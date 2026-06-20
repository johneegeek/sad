/*******************************************************************************
 * SAD - Utils Module
 *
 * General utility functions used throughout the application
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/

#include "utils.h"

#include "dprintf.h"
#include "jkcommon.h"
#include "platform.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* External variables (defined in sad.c) */
extern int     data_width;
extern int32_t data_end_count;
extern bool    show_eof_marker;

/*****************************************************************************\
 Function: file_is_bin()

 Description: Determines if a file is binary or text

 Returns: true if file is binary, false if text
\*****************************************************************************/
bool file_is_bin(char* filename_sz)
{
    FILE* posbin_fl;
    bool  retflag_f = false;
    int   iChIn;

    posbin_fl = fopen(filename_sz, "r");
    if (posbin_fl == NULL) return (false);

    while ((iChIn = fgetc(posbin_fl)) != EOF) {
        if (iChIn < 0x7 || iChIn > 0x7E) {
            retflag_f = true;
            break;
        }
    }

    fclose(posbin_fl);
    return (retflag_f);
}

/*****************************************************************************\
 Function: get_input()

 Description: Gets input chars from "fp", used to control amount read in.
 Use global byte counts to control how much is read in.
 EOF is returned when the maximum byte count is hit.  If EOF is returned before
 actual EOF we need to make sure the EOF marker is not displayed, so
 show_eof_marker should be set to false.

 Returns:
\*****************************************************************************/
int get_input(FILE* fp)
{
    static uint64_t char_count = 0;

    if (data_end_count == 0) { return (fgetc(fp)); } /* if (data_count == 0) */
    else {
        int input_char;

        input_char = fgetc(fp);
        ++char_count;
        if (char_count <= (uint64_t)((unsigned)data_end_count * (unsigned)data_width)) {
            return (input_char);
        } /* if (char_count < (data_count * data_width)) */
        else {
            if (input_char != EOF) /* we have all the requested data, but theres more */
            {
                show_eof_marker
                    = false; /*not really eof, so we don't want to show it */
                return ((int)EOF);
            } /* if (input_char != EOF) */
        }     /* else */

    } /* else */

    dbprintf(
        "!! DEBUG WARNING: should never have reached this line. Module %s, line %d\n",
        __FILE__, __LINE__);
    return ((int)EOF);

} /* get_input() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
