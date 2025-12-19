#!/bin/bash
# build-report.sh - Clean build with error/warning reporting

set -e

# Colors for output
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
LOG_FILE="/tmp/sneezy-build.log"
PRESET="${1:-dev}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}SneezyMUD Clean Build Report${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Clean previous build
echo -e "${BLUE}[1/3] Cleaning previous build...${NC}"
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    echo "  ✓ Removed $BUILD_DIR directory"
fi
if [ -f "compile_commands.json" ]; then
    rm -f "compile_commands.json"
    echo "  ✓ Removed compile_commands.json"
fi
echo ""

# Configure
echo -e "${BLUE}[2/3] Configuring with preset: $PRESET${NC}"
if cmake --preset "$PRESET" > "$LOG_FILE" 2>&1; then
    echo "  ✓ Configuration successful"
else
    echo -e "${RED}  ✗ Configuration failed${NC}"
    echo ""
    echo -e "${RED}Configuration errors:${NC}"
    cat "$LOG_FILE"
    exit 1
fi
echo ""

# Build and capture output
echo -e "${BLUE}[3/3] Building...${NC}"
BUILD_START=$(date +%s)
if cmake --build "$BUILD_DIR" > "$LOG_FILE" 2>&1; then
    BUILD_STATUS=0
    echo -e "  ${GREEN}✓ Build successful${NC}"
else
    BUILD_STATUS=1
    echo -e "  ${RED}✗ Build failed${NC}"
fi
BUILD_END=$(date +%s)
BUILD_TIME=$((BUILD_END - BUILD_START))
echo "  Build time: ${BUILD_TIME}s"
echo ""

# Parse warnings and errors
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Build Report${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Count errors and warnings
ERROR_COUNT=$(grep -c "error:" "$LOG_FILE" 2>/dev/null || echo "0")
ERROR_COUNT=$(echo "$ERROR_COUNT" | tr -d '\n')
WARNING_COUNT=$(grep -c "warning:" "$LOG_FILE" 2>/dev/null || echo "0")
WARNING_COUNT=$(echo "$WARNING_COUNT" | tr -d '\n')
RUNTIME_ERROR_COUNT=$(grep -c "runtime error:" "$LOG_FILE" 2>/dev/null || echo "0")
RUNTIME_ERROR_COUNT=$(echo "$RUNTIME_ERROR_COUNT" | tr -d '\n')

# Summary
echo -e "${BLUE}Summary:${NC}"
echo "  Errors:          $ERROR_COUNT"
echo "  Warnings:        $WARNING_COUNT"
echo "  Runtime Errors:  $RUNTIME_ERROR_COUNT"
echo ""

# Show errors if any
if [ "$ERROR_COUNT" -gt 0 ]; then
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}ERRORS (${ERROR_COUNT} total)${NC}"
    echo -e "${RED}========================================${NC}"
    grep --color=never "error:" "$LOG_FILE" | head -50
    echo ""
fi

# Show runtime errors if any
if [ "$RUNTIME_ERROR_COUNT" -gt 0 ]; then
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}RUNTIME ERRORS (${RUNTIME_ERROR_COUNT} total)${NC}"
    echo -e "${RED}========================================${NC}"
    grep --color=never "runtime error:" "$LOG_FILE" | head -50
    echo ""
fi

# Show warnings if any
if [ "$WARNING_COUNT" -gt 0 ]; then
    echo -e "${YELLOW}========================================${NC}"
    echo -e "${YELLOW}WARNINGS (${WARNING_COUNT} total)${NC}"
    echo -e "${YELLOW}========================================${NC}"

    # Group warnings by type
    echo -e "${YELLOW}Warning breakdown:${NC}"
    grep "warning:" "$LOG_FILE" | sed 's/.*warning: //' | sed 's/ \[.*//' | sort | uniq -c | sort -rn | head -20
    echo ""

    echo -e "${YELLOW}First 30 warnings:${NC}"
    grep --color=never "warning:" "$LOG_FILE" | head -30
    echo ""

    if [ "$WARNING_COUNT" -gt 30 ]; then
        echo -e "${YELLOW}... and $((WARNING_COUNT - 30)) more warnings${NC}"
        echo ""
    fi
fi

# Show suggestions
if [ "$ERROR_COUNT" -eq 0 ] && [ "$WARNING_COUNT" -eq 0 ] && [ "$RUNTIME_ERROR_COUNT" -eq 0 ]; then
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}✓ Clean build! No errors or warnings.${NC}"
    echo -e "${GREEN}========================================${NC}"
else
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}Next Steps${NC}"
    echo -e "${BLUE}========================================${NC}"

    if [ "$ERROR_COUNT" -gt 0 ]; then
        echo "1. Fix compilation errors first (see above)"
    fi

    if [ "$RUNTIME_ERROR_COUNT" -gt 0 ]; then
        echo "2. Fix runtime errors (sanitizer issues)"
    fi

    if [ "$WARNING_COUNT" -gt 0 ]; then
        echo "3. Address compiler warnings"
    fi

    echo ""
    echo "Full build log: $LOG_FILE"
fi

echo ""
exit $BUILD_STATUS
