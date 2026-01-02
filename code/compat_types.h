/* ************************************************************************
 * file: compat_types.h - Binary file compatibility types
 *
 * This header provides fixed-size types for binary file I/O to maintain
 * compatibility with player files created on 32-bit systems (circa 1990s).
 *
 * On modern 64-bit systems:
 *   - time_t is 8 bytes (was 4)
 *   - long is 8 bytes (was 4)
 *   - pointers are 8 bytes (was 4)
 *
 * These compat types ensure binary files remain readable across platforms.
 ************************************************************************ */

#pragma once

#include <stdint.h>

/* Fixed-size types matching original 32-bit binary format */
using compat_time = int32_t;   /* time_t was 32-bit */
using compat_long = int32_t;   /* long was 32-bit */
using compat_ulong = uint32_t; /* unsigned long was 32-bit */
using compat_ptr = int32_t;    /* pointers were 32-bit (placeholder in files) */

/* Conversion macros for clarity */
#define TIME_TO_COMPAT(t) ((compat_time)(t))
#define COMPAT_TO_TIME(c) ((time_t)(c))
#define LONG_TO_COMPAT(l) ((compat_long)(l))
#define COMPAT_TO_LONG(c) ((long)(c))
#define ULONG_TO_COMPAT(l) ((compat_ulong)(l))
#define COMPAT_TO_ULONG(c) ((unsigned long)(c))
