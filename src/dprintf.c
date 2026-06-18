/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 # Debugging printf functions
 # Module: dprintf.c
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
