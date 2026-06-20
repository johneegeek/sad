/*******************************************************************************
 * SAD - Help Module
 *
 * Help and version information display
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/

#include "help.h"

#include "platform.h"
#include "sad.h"
#include "version.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External variables (defined in sad.c) */
extern char* pname;
extern bool  use_pager;
extern char  pager_program[];
extern int   data_width;

/*****************************************************************************\
 Function: show_help()

 Description: Shows help message

 Returns:
\*****************************************************************************/
void show_help(void)
{
    FILE* fp;

#if !defined(__DOS16__)
    if (use_pager) {
        fp = popen(pager_program, "w");
        if (fp == NULL) {
            perror("pipe error");
            exit(1);
        } /* if (output_file == NULL) */
    } /* if (use_pager) */
    else {
        fp = stdout;
    } /* else */
#else
    fp = stdout;
#endif

    fprintf(fp, "Slightly Awesome Dump - version %s\n", SAD_VERSION);
    fprintf(fp, "Usage: %s [options] [file(s)]\n", pname);
    fprintf(fp, "Options:\n");
    fprintf(fp, "   -a,   --ascii            Display in ASCII format.\n");
    fprintf(fp, "   -A,   --formatted-ascii  Display in \"formatted\" ASCII.\n");
    fprintf(fp, "   -b,   --binary           Display in binary `text'.\n");
    fprintf(fp, "   -c,   --control          Display text and control characters.\n");
    fprintf(fp, "   -d,   --decimal          Display in decimal format.\n");
    fprintf(fp, "   -o,   --octal            Display in octal format.\n");
    fprintf(fp, "   -s,   --signed           Display in signed decimal format.\n");
    fprintf(fp, "   -x,   --hex              Display in hex format.\n");
    fprintf(fp, "   -f,   --float            Display in floating point format.\n");
    fprintf(fp,
            "   -l,   --double           Display in double floating point format.\n");

    fprintf(fp,
            "   -W<n>,--float-width=<n>  Character width of floating point number.\n");
    fprintf(fp, "   -P<n>,--float-precision  Precision of floating point number..\n");

#if !defined(__BIG_ENDIAN__)
    fprintf(fp, "   -B,   --big-endian       Use \"big endian\" byte ordering.\n");
#else
    fprintf(fp, "   -L,   --little-endian    Use \"little endian\" byte ordering.\n");
#endif
    fprintf(fp, "   -H,   --no-highlight     Do not add highlight information.\n");
    fprintf(fp, "   -F,   --no-offset        Do not show offset.\n");
    fprintf(fp,
            "   -I,   --no-ascii         Do not show ascii representation of data.\n");
    fprintf(fp, "   -E,   --no-eof-marker    Do not show end of file marker.\n");
    fprintf(fp, "   -U,   --show-duplicate   Show duplicate lines.\n");
    fprintf(fp, "   -e,   --show-spaces      Show 'visible' spaces in ascii mode.\n");
    fprintf(fp,
            "   -q,   --offset-oct       Show offsets in octal (default is hex).\n");
    fprintf(fp,
            "   -z,   --offset-dec       Show offsets in decimal (default is hex).\n");
    fprintf(fp, "   -V,   --offset-divisor   Divide the offset by given value. "
                "(default is 1).\n");

    fprintf(fp, "   -p,   --pager[=PAGER]    Send output to pager.\n");

    fprintf(fp, "   -n<n>,--columns=<n>      Number of columns to display per line.\n");
    fprintf(
        fp,
        "   -w<n>,--width=<n>        Data width in bytes to display, default is %d.\n",
        data_width);
    fprintf(fp, "   -1,   --bytes,--8bit     Display in bytes (same as --width=1).\n");
    fprintf(fp, "   -2,   --words,--16bit    Display in words (same as --width=2).\n");
    fprintf(
        fp,
        "   -4,   --dwords,--32bit   Display in double words (same as --width=4).\n");
    fprintf(fp,
            "   -8,   --qwords,--64bit   Display in quad words (same as --width=8).\n");

    fprintf(fp, "   -j<n>,--start-count=<n>  Starting item to begin dump (of size data "
                "width).\n");
    fprintf(fp,
            "   -J<n>,--start-offset=<n> Starting offset in bytes to begin dump.\n");
    fprintf(fp,
            "   -k<n>,--count=<n>        Number of items to dump (of data width).\n");
    fprintf(fp, "   -K<n>,--end-offset=<n>   Ending offset in bytes.\n");

#ifdef DEBUG
    fprintf(fp, "   -D,   --debug            Show debugging messages.\n");
#endif
    fprintf(fp, "   -?,   --help             Show this message.\n");
    fprintf(fp, "   -v,   --version          Show version information\n");
    fprintf(fp, "\nNote: When entering numbers, default is decimal (base 10). To enter "
                "numbers in other bases:");
    fprintf(fp, "\nHexadecimal=0xNNNN, Octal=0oNNNN, Binary=0bNNNN\n");
    fprintf(fp, "\n");
    fprintf(fp, "Report issues to <john@yellowsocks.com>\n");
    fprintf(fp, "    or github.com/johneegeek/sad/issues\n");

#if !defined(__DOS16__)
    if (use_pager) (void)pclose(fp);
#endif

} /* show_help() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/*****************************************************************************\
 Function: show_version()

 Description: Shows version info

 Returns:
\*****************************************************************************/
void show_version(void)
{
    fprintf(stderr, "'%s' version %s", pname, SAD_VERSION);
#if defined(__WIN32__) || defined(_MSC_VER) || defined(_WIN64)
    fprintf(stderr, " for Windows.\n");
#elif defined(__ST32__)
    fprintf(stderr, " for ST32.\n");
#elif defined(__DOS16__)
    fprintf(stderr, " for DOS.\n");
#elif defined(__LINUX__) || defined(__linux__)
    fprintf(stderr, " for Linux.\n");
#elif defined(__APPLE__)
    fprintf(stderr, " for Apple.\n");
#elif defined(__QNX__)
    fprintf(stderr, " for QNX.");
#if defined(__BIG_ENDIAN__)
    fprintf(stderr, " (big-endian)\n");
#else
    fprintf(stderr, " (little-endian)\n");
#endif
#elif defined(__UNIXLIKE__)
    fprintf(stderr, " for Unix.\n");
#else
    fprintf(stderr, "\n");
#endif
    fprintf(stderr, "Copyright (c) 1997-2026.  John Kiernan\n");
    fprintf(stderr, "Last compiled on %s (%s)\n", BUILD_DATE, COMPILER_INFO);
    fprintf(stderr, "git sha: %s\n", SAD_GIT_SHA);
} /* show_version() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
