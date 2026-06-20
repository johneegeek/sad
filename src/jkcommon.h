/*******************************************************************************
 * SAD - Common Header
 *
 * Common type definitions and compiler detection
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/
#ifndef _JKCOMMON_H
#define _JKCOMMON_H

#if !defined(__bool_true_false_are_defined) \
    && !defined(_BOOL_TRUE_FALSE_ALREADY_DEFINED) && !defined(__cplusplus)
/* We have <stdbool.h>. or have already definded bool */
#define _BOOL_TRUE_FALSE_ALREADY_DEFINED
/*@ignore@*/ /*bug in splint?*/
/*@-booltype bool@*/
/*@-boolfalse false@*/
/*@-booltrue true@*/
/*@end@*/
typedef enum _boolean_type { false, true } bool;

#if !defined(_WINDOWS_)
/*@ignore@*/
typedef enum _BOOLEAN_TYPE { FALSE, TRUE } BOOL;
/*@end@*/
#endif

#endif

#ifndef __COMPILER__
#ifdef __GNUC__
#define __COMPILER__     "GNU C/C++"
#define __COMPILER_VER__ __GNUC__
#endif
#ifdef _MSC_VER
#define __COMPILER__     "Microsoft C/C++"
#define __COMPILER_VER__ _MSC_VER
#ifndef __MICROSOFTC__
#define __MICROSOFTC__ _MSC_VER
#endif
#endif
#ifdef __WATCOMC__
#if __WATCOMC__ >= 1200
#define __COMPILER__ "Open Watcom C/C++"
#else
#define __COMPILER__ "Watcom C/C++"
#endif
#define __COMPILER_VER__ __WATCOMC__
#endif
#ifdef __BORLANDC__
#define __COMPILER__     "Borland C/C++"
#define __COMPILER_VER__ __BORLANDC__
#endif
#endif

/* Legacy type definitions removed - now using standard <stdint.h> types:
 * Use uint8_t, uint16_t, uint32_t, uint64_t instead of uint8, uint16, uint32, uint64
 * Use int8_t, int16_t, int32_t, int64_t instead of int8, int16, int32, int64
 */
#include <stdint.h>

/* BYTE Type */
#if !defined(_WINDOWS_) && !defined(_WINDEF_) && !defined(_BYTE_DEFINED)
#define _BYTE_DEFINED
typedef unsigned char BYTE;
typedef BYTE*         PBYTE;
#endif /* !_BYTE_DEFINED */

#undef _D
#ifdef DEBUG
#define _D(exp) exp
#else
#define _D(exp) ((void)0)
#endif

#endif /* !_JKCOMMON_H */
