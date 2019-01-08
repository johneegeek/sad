/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 # Debugging printf functions
 # Module: dprintf.c
 # $Header: C:\\st32_dev\\source_files\\sad\\RCS\\dprintf.c,v 1.0 2002-07-22 09:53:51-05 john Exp john $
 #
 # Created: 7/11/2002
 # Copyright Bio-Imaging Research, Inc.
 #
 # History:
 #
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#include <stdio.h>
#include <stdarg.h>
#include "jkcommon.h"
#include "dprintf.h"

                                 
/*****************************************************************************\
 Function: dbprintf()

 Description: debugging printf. only printed it debug value is set .

 Returns:
\*****************************************************************************/
void dbprintf( char *format_sz, ... )
{
#ifdef DEBUG
    extern bool print_debug_messages;
    va_list arg_list;

    /* Display only if print_yes is set */
    if ( print_debug_messages ) 
    {
        va_start( arg_list, format_sz );
        (void) vprintf(format_sz, arg_list );
        va_end( arg_list );
        (void)fflush(stdout);
    }
#endif
} /* dprintf() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
