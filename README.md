# SneezyMUD

A classic DikuMUD-based text MUD (Multi-User Dungeon) game server written in C.

## Prerequisites

- **Clang** (C17 compiler with sanitizer runtime)
- **CMake** 3.20 or newer
- **Ninja** build system
- Standard C library with crypt support

On Ubuntu/Debian:
```bash
sudo apt install clang cmake ninja-build libclang-rt-dev
```

Note: `libclang-rt-dev` provides the sanitizer runtime libraries (ASan/UBSan/LeakSan) which are enabled by default.

## Quick Start

**1. Build the server**

Using Make (recommended):
```bash
make build
```

Or using CMake directly:
```bash
cmake --preset dev
cmake --build build
```

The compiled `sneezy` binary will be copied to the project root.

**2. Run the server**

```bash
make run
```

Or run directly:
```bash
./sneezy
```

By default, the server runs on port **4000**. Use `make PORT=5000 run` for a custom port.

**3. Connect to the game**

Use any telnet client:
```bash
telnet localhost 4000
```

## Command-Line Options

```
./sneezy [-l] [-s] [-d pathname] [port]
```

- `-l` - Lawful mode
- `-s` - Suppress special routines (mob AI/special procs)
- `-d pathname` - Data directory (default: `lib/`)
- `port` - Port number (default: 4000)

## Development

**Common commands (using Make):**
```bash
make help       # Show all available commands
make build      # Build debug version (default)
make release    # Build optimized version
make format     # Format code with clang-format
make clean      # Remove build directory
make run        # Build and run server
make report     # Clean build with error/warning report
```

**Format code:**
```bash
make format
```

Or with CMake:
```bash
cmake --build build --target format
```

**Build configurations:**
- `dev` - Debug build with sanitizers (default)
- `release` - Optimized build with sanitizers

**Check code quality:**
```bash
make report                    # Clean build with detailed error/warning report
./scripts/build-report.sh      # Run directly with dev preset
./scripts/build-report.sh release  # Run with release preset
```

This performs a clean build and generates a summary of all compilation errors, warnings, and runtime errors, grouped by type for easy review.

**Disable sanitizers** (if libclang-rt-dev is unavailable):
```bash
cmake --preset dev -DENABLE_SANITIZERS=OFF
cmake --build build
```

For detailed architecture and development guidance, see [CLAUDE.md](CLAUDE.md).

## Project Structure

```
code/          - C source files
lib/           - Game data files (world, help, player saves)
  ├── areas/   - Zone files
  ├── help/    - Help system
  └── players  - Player save file
cmake/         - CMake configuration modules
scripts/       - Build and development scripts
build/         - Build output (generated)
```

## License

See [LICENSE](LICENSE) for details.
