/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 Common defines
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef _JKCOMMON_H
#define _JKCOMMON_H

#if !defined (__bool_true_false_are_defined) && !defined(_BOOL_TRUE_FALSE_ALREADY_DEFINED) && !defined(__cplusplus)
    /* We have <stdbool.h>. or have already definded bool */
    #define _BOOL_TRUE_FALSE_ALREADY_DEFINED
    /*@ignore@*/ /*bug in splint?*/
    /*@-booltype bool@*/
    /*@-boolfalse false@*/
    /*@-booltrue true@*/
    /*@end@*/
    typedef enum _boolean_type {false,true} bool;

    #if !defined(_WINDOWS_)
    /*@ignore@*/
    typedef enum _BOOLEAN_TYPE {FALSE,TRUE} BOOL;
    /*@end@*/
    #endif

#endif

#ifndef __COMPILER__
#ifdef __GNUC__
    #define __COMPILER__ "GNU C/C++"
    #define __COMPILER_VER__ __GNUC__
#endif
#ifdef _MSC_VER
    #define __COMPILER__ "Microsoft C/C++"
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
    #define __COMPILER__ "Borland C/C++"
	#define __COMPILER_VER__ __BORLANDC__
#endif
#endif


#ifndef _INT_TYPES_DEFINED
#define _INT_TYPES_DEFINED
    typedef short int             int16;
    typedef unsigned short int    uint16;

    #if defined(WIN32) || defined(_WINDOWS_)
        typedef int               int32;
        typedef unsigned int      uint32;
    #else
        typedef long int          int32;
        typedef unsigned long int uint32;
    #endif

    /* NOTE: for 64bit integers, use %I64u or %I64d in printfs to display*/
    #if defined(__GNUC__) || defined(S_SPLINT_S)
        typedef long long int           int64;
        typedef unsigned long long int  uint64;
    #endif
    #if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__)
        typedef __int64                 int64;
        typedef unsigned __int64        uint64;
    #endif
#endif

/* BYTE Type */
#if !defined(_WINDOWS_) && !defined(_WINDEF_) && !defined(_BYTE_DEFINED)
    #define _BYTE_DEFINED
    typedef unsigned char BYTE;
    typedef BYTE *PBYTE;
#endif /* !_BYTE_DEFINED */

#undef _D
#ifdef DEBUG
    #define _D(exp) exp
#else
    #define _D(exp) ((void)0)
#endif

#endif /* !_JKCOMMON_H */
