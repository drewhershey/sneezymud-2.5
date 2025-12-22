# Binary File Compatibility: 32-bit and Endianness Issues

This document records the discoveries made while modernizing this 1990s-era MUD codebase to run on modern 64-bit x86 Linux systems.

## Background

SneezyMUD was originally developed in the early 1990s when:
- 32-bit systems were the norm
- Sun SPARC workstations (big-endian) were common for university servers
- C's `int`, `long`, `time_t`, and pointers were typically 4 bytes

The codebase uses binary file formats where C structs are written directly to disk with `fwrite()` and read back with `fread()`. This approach creates two categories of compatibility issues when running on modern systems.

## Issue 1: 32-bit vs 64-bit Data Type Sizes

### The Problem

On modern 64-bit Linux systems:
- `time_t` is 8 bytes (was 4 bytes on 32-bit)
- `long` is 8 bytes (was 4 bytes on 32-bit)
- Pointers are 8 bytes (were 4 bytes on 32-bit)

When the code attempts to read binary files created on 32-bit systems, the struct field offsets don't align, causing:
- Corrupted data when reading
- Files appearing empty or having wrong record counts
- Crashes from invalid pointer values

### Affected Structures

| Structure | File | Problematic Fields |
|-----------|------|-------------------|
| `char_file_u` | `lib/players` | `time_t birth`, `time_t last_logon`, `unsigned long act`, `affected_type[25]` (contains `long` + pointer) |
| `obj_file_elem` | `lib/rent/*` | `long bitvector` |
| `obj_file_u` | `lib/rent/*` | `long last_update`, `long minimum_stay` |
| `header_block_type_d` | `lib/mail` | `long block_type`, `long next_block`, `long mail_time` |
| `data_block_type_d` | `lib/mail` | `long block_type`, `long next_block` |

### The Solution

Created `code/compat_types.h` with fixed-size typedefs that match the original 32-bit layout:

```c
typedef int32_t  compat_time;   // was time_t (4 bytes on 32-bit)
typedef int32_t  compat_long;   // was long (4 bytes on 32-bit)
typedef uint32_t compat_ulong;  // was unsigned long
typedef uint32_t compat_ptr;    // was pointer (stored but not dereferenced)
```

All file I/O structs were updated to use these fixed-size types, ensuring the binary layout matches regardless of the host architecture.

### Example: struct char_file_u

Original problematic layout:
```c
struct char_file_u {
  char name[20];
  char pwd[11];
  // ...
  time_t birth;           // 8 bytes on 64-bit, was 4 bytes
  time_t last_logon;      // 8 bytes on 64-bit, was 4 bytes
  unsigned long act;      // 8 bytes on 64-bit, was 4 bytes
  struct affected_type affected_by[MAX_AFFECT];  // contains long + pointer
  // ...
};
```

Fixed layout with compat types:
```c
struct char_file_u {
  char name[20];
  char pwd[11];
  // ...
  compat_time birth;           // Always 4 bytes
  compat_time last_logon;      // Always 4 bytes
  compat_ulong act;            // Always 4 bytes
  struct affected_type_file affected_by[MAX_AFFECT];  // Uses compat types
  // ...
};
```

### Verification

After applying compat types, `sizeof(struct char_file_u)` is 1264 bytes, matching the original 32-bit layout. The `lib/players` file with 351 records reads correctly.

## Issue 2: Big-Endian vs Little-Endian

### The Problem

Different CPU architectures store multi-byte integers in different byte orders:
- **Big-endian** (Sun SPARC, PowerPC): Most significant byte first
- **Little-endian** (x86, x86-64): Least significant byte first

The value `0x00000384` (900 decimal):
- Big-endian bytes: `00 00 03 84`
- Little-endian bytes: `84 03 00 00`

When a little-endian system reads a big-endian file, integer values are completely wrong.

### Discovery Process

Initial symptoms:
- Mail file showed garbage `next_block` values like `-2080178176`
- Message chains were broken, text was truncated

Investigation revealed:
- Raw hex bytes `00 00 03 84` were being read as little-endian `0x84030000`
- The file was created on a Sun SPARC (big-endian) system
- Byte-swapping the value: `0x84030000` → `0x00000384` = 900 (valid byte offset!)

### Affected Files

| File | Original System | Status |
|------|-----------------|--------|
| `lib/mail` | Sun SPARC (big-endian) | Converted to little-endian |
| `lib/3095.messages` | Sun SPARC (big-endian) | Converted to little-endian |
| `lib/3097.messages` | Sun SPARC (big-endian) | Converted to little-endian |
| `lib/3098.messages` | Sun SPARC (big-endian) | Converted to little-endian |
| `lib/25102.messages` | Sun SPARC (big-endian) | Converted to little-endian |
| `lib/3099.messages` | x86 (little-endian, from 2002) | No conversion needed |
| `lib/players` | x86 (little-endian) | No conversion needed |

### The Solution

Created one-time conversion tools in the `tools/` directory:
- `convert_mail_endian.c` - Converts mail file from big-endian to little-endian
- `convert_board_endian.c` - Converts board message files from big-endian to little-endian

These tools byte-swap all integer fields while preserving text data.

### Conversion Verification

After conversion, the mail file correctly shows:
- 52 messages from April-November 1993
- Proper dates (not garbage like "year 2072")
- Complete message text (not truncated)
- Valid `next_block` chain pointers

Original big-endian files backed up with `.bigendian.bak` extension.

## Mail File Format

The mail system uses a block-based format where each block is 100 bytes:

```c
#define BLOCK_SIZE 100

struct header_block_type_d {
  compat_long block_type;      // HEADER_BLOCK (2) or LAST_BLOCK (-2)
  compat_long next_block;      // Byte offset to next block, or -2 for end
  compat_long mail_time;       // Unix timestamp
  char from[20];               // Sender name
  char to[20];                 // Recipient name
  char txt[HEADER_BLOCK_DATASIZE];  // Message start (43 bytes after compat fix)
};

struct data_block_type_d {
  compat_long block_type;      // DATA_BLOCK (1)
  compat_long next_block;      // Byte offset to next block, or -2 for end
  char txt[DATA_BLOCK_DATASIZE];  // Continuation text (92 bytes)
};
```

**Key insight**: The `next_block` field stores a **byte offset** into the file, not a block index.

## Board Message File Format

Board message files (`.messages`) use a simpler format:

```
[4-byte int: message_count]
For each message:
  [4-byte int: header_length]
  [header_text]
  [4-byte int: body_length]
  [body_text]
```

All integers were big-endian in the original files.

## Code Changes Summary

### New Files
- `code/compat_types.h` - Fixed-size type definitions

### Modified Files
- `code/structs.h` - Updated `char_file_u`, `obj_file_elem`, `obj_file_u` to use compat types; added `affected_type_file` and `obj_affected_type_file` for file I/O
- `code/mail.h` - Updated mail structs to use `compat_long`
- `code/db.c` - Added field-by-field conversion between runtime and file structs for affected types
- `code/reception.c` - Added conversion for object affected types
- `code/interpreter.c` - Added conversion when loading objects
- `code/board.c` - Fixed double-free bug, re-enabled board loading (was disabled due to endianness issues)

### Conversion Tools
- `tools/convert_mail_endian.c`
- `tools/convert_board_endian.c`

## Historical Context

The timestamps in the recovered data provide a window into the MUD's history:

- **Mail messages**: April-November 1993
- **Board posts**: May-November 1993
- **Board 3099**: Created in 2002 (different era, already little-endian)

The original server was likely a Sun SPARC workstation at a university, common for MUD hosting in the early 1990s. At some point, the players file and some other data were migrated to an x86 system, while the mail and board files retained their original big-endian format.

## Lessons Learned

1. **Never write raw structs to disk** if cross-platform or long-term compatibility matters. Use explicit serialization with fixed-size types and defined byte order.

2. **Document binary file formats** independently of the code, including exact byte layouts and endianness.

3. **Include magic numbers and version fields** in binary file headers to detect format mismatches.

4. **Prefer text-based formats** (JSON, YAML) or databases (SQLite) for modern applications—they're self-describing and portable.

5. **Keep conversion tools** even after migration—you may need to process old backups or archives in the future.

## Future Considerations

### Year 2038 Problem

The `compat_time` type is `int32_t`, which will overflow on January 19, 2038. For a nostalgic MUD server, this is likely acceptable, but for long-term use:
- Consider migrating to a database (SQLite recommended)
- Or implement a new file format with 64-bit timestamps

### Password Security

The current password system uses DES crypt (1970s vintage):
- Limited to 8 character passwords
- Easily crackable with modern hardware
- Telnet transmits passwords in plaintext

For a private nostalgia server this is acceptable, but consider:
- Upgrading to SHA-512 or bcrypt
- Using stunnel or TLS wrapper for encrypted connections
