/*******************************************************************************
 * SAD - Debug Printf Module
 *
 * Debug printf functions for conditional debug output
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/
#include "dprintf.h"

#include "jkcommon.h"

#include <stdarg.h>
#include <stdio.h>

/*****************************************************************************\
 Function: dbprintf()

 Description: debugging printf. only printed it debug value is set .

 Returns:
\*****************************************************************************/
void dbprintf(char* format_sz, ...)
{
#ifdef DEBUG
    extern bool print_debug_messages;
    va_list     arg_list;

    /* Display only if print_yes is set */
    if (print_debug_messages) {
        va_start(arg_list, format_sz);
        (void)vprintf(format_sz, arg_list);
        va_end(arg_list);
        (void)fflush(stdout);
    }
#endif
} /* dprintf() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/