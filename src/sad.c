/*******************************************************************************
 * SAD - Main Program
 *
 * Program entry point, initialization, file I/O, and pager integration
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__DJGPP__) || defined(__UNIXLIKE__)
#include <errno.h>
#define _access access
#endif

#if defined(__BORLANDC__) || defined(_MSC_VER)
#include <io.h>
#else
#include <unistd.h>
#endif
#include "binstr.h"
#include "dprintf.h"
#include "dumpers.h"
#include "formatters.h"
#include "getopt.h"
#include "help.h"
#include "jkcommon.h"
#include "makeargs.h"
#include "options.h"
#include "sad.h"
#include "utils.h"
#include "version.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __DOS16__
char system_command[MAXSTRING];
#endif

/* Control character tables */
char* CtrlTable[]
    = {"nul", "soh", "stx", "etx", "eot", "enq", "ack", "bel", "bs",  "ht",  "nl",
       "vt",  "np",  "cr",  "so",  "si",  "dle", "dc1", "dc2", "dc3", "dc4", "nak",
       "syn", "etb", "can", "em",  "sub", "esc", "fs",  "gs",  "rs",  "us"};

char* CCTable[] = {"^@", "^A", "^B", "^C", "^D", "^E", "^F",  "^G", "^H", "^I", "^J",
                   "^K", "^L", "^M", "^N", "^O", "^P", "^Q",  "^R", "^S", "^T", "^U",
                   "^V", "^W", "^X", "^Y", "^Z", "^[", "^\\", "^]", "^^", "^_"};

char* H_CCTable[]
    = {"^\b^@\b@", "^\b^A\bA", "^\b^B\bB", "^\b^C\bC", "^\b^D\bD",   "^\b^E\bE",
       "^\b^F\bF", "^\b^G\bG", "^\b^H\bH", "^\b^I\bI", "^\b^J\bJ",   "^\b^K\bK",
       "^\b^L\bL", "^\b^M\bM", "^\b^N\bN", "^\b^O\bO", "^\b^P\bP",   "^\b^Q\bQ",
       "^\b^R\bR", "^\b^S\bS", "^\b^T\bT", "^\b^U\bU", "^\b^V\bV",   "^\b^W\bW",
       "^\b^X\bX", "^\b^Y\bY", "^\b^Z\bZ", "^\b^[\b[", "^\b^\\\b\\", "^\b^]\b]",
       "^\b^^\b^", "^\b^_\b_"};

/*globals and options*/
uint16_t dump_mode;
char     pager_program[MAXSTRING];
FILE*    output_file;
FILE*    input_file;
int      columns;
int      data_width;
int32_t  data_end_count;
int32_t  data_start_count;
#ifdef DEBUG
bool print_debug_messages;
#endif
bool     use_pager;
bool     show_offset;
bool     show_ascii;
bool     highlight;
bool     no_duplicate;
bool     show_eof_marker;
bool     big_endian;
bool     mode_selected;
bool     width_selected;
bool     columns_selected;
bool     show_spaces;
int      offset_display;
uint32_t offset_divisor = 1;

int float_width     = 0;
int float_precision = 0;

char* pname;

/*local prototypes*/
/* All functions now in separate modules */

uint64_t string_to_uint64(const char* val, bool* valid);

/**
 * Try to find config file at a specific location
 * @param cfg_file Buffer to store the config file path
 * @param env_var Environment variable name to check
 * @param path_suffix Path suffix to append (e.g., "sad.cfg", "cfg/sad.cfg")
 * @return true if config file found, false otherwise
 */
static bool try_config_location(char* cfg_file, const char* env_var,
                                const char* path_suffix)
{
    char* p = getenv(env_var);
    if (p != NULL) {
        dbprintf("%s: debug: `%s' variable found, checking for config\n", pname,
                 env_var);
#if !defined(_UNIXSTYLE_PATHS)
        /* On Windows, build path with backslashes */
        sprintf(cfg_file, "%s\\%s", p, path_suffix);
        /* Replace forward slashes with backslashes in the suffix portion */
        char* slash;
        while ((slash = strchr(cfg_file, '/')) != NULL) {
            *slash = '\\';
        }
#else
        sprintf(cfg_file, "%s/%s", p, path_suffix);
#endif
        if (FILE_EXISTS(cfg_file)) { return true; }
    }
    return false;
}

int main(int argc, char** argv)
{
    char*  p;
    int    index;
    int    ex_argc;
    char** ex_argv;
    char   cfg_file[MAXSTRING];

    /* strip off everything for program name */
    pname = argv[0];
    p     = pname;
    while ((bool)*++p) *p = (char)tolower(*p);
#if defined(__DOSLIKE__)
    p = strchr(pname, '.');
#endif
    if (p != NULL) *p = (char)0;
#ifndef _UNIXSTYLE_PATHS
    while ((p = strchr(pname, '\\')) != 0) pname = p + 1;
#else
    while ((p = strchr(pname, '/')) != 0) pname = p + 1;
#endif

/******************\
   Setup defaults
\******************/
#if defined(DEBUG)
    print_debug_messages = true;
#endif
    dump_mode        = DUMP_MODE_ASCII;
    use_pager        = false;
    input_file       = stdin;
    columns          = DEFAULT_COLUMNS_HEX;
    data_width       = DEFAULT_WIDTH;
    show_offset      = true;
    show_ascii       = true;
    highlight        = true;
    no_duplicate     = false;
    show_eof_marker  = true;
    big_endian       = false;
    mode_selected    = false;
    width_selected   = false;
    columns_selected = false;
    show_spaces      = false;
    offset_display   = OFFSET_HEX;
    data_end_count   = 0;
    data_start_count = 0;

    /* Get pager from environment */
    p = getenv("PAGER");
    if (p == NULL) {
#if defined(__DOSLIKE__)
        strcpy(pager_program, "more");
#else
        strcpy(pager_program, "less");
#endif
    }
    else
        strcpy(pager_program, p);

    /*********************************************/
    /* Get options from config file if it exists */
    /*********************************************/
    dbprintf("%s: debug: checking for options...\n", pname);

    cfg_file[0] = 0;
    if (try_config_location(cfg_file, "HOME", "sad.cfg")
        || try_config_location(cfg_file, "XDG_CONFIG_HOME", "sad/sad.cfg")
        || try_config_location(cfg_file, "WTDIR", "cfg/sad.cfg")) {
        dbprintf("%s: debug: `%s' found, getting arguments...\n", pname, cfg_file);
        ex_argc = fmakeargs(cfg_file, &ex_argv);
        if (ex_argc != 0) {
            if (!process_options(ex_argc, ex_argv)) return (1);
        } /* if (ex_argc != 0) */
        else
            dbprintf("%s: debug: no cfg arguments found.\n", pname);
        free_makeargs(ex_argc, ex_argv);
    } /* if config file found */
    else {
        dbprintf("%s: debug: no cfg file found.\n", pname);
    }

    /*********************************************/
    /* Get options from ENV variable if it exits */
    /*********************************************/
    p = getenv(pname);
    if (p != NULL) {
        dbprintf("%s: debug: `%s' env found, getting arguments...\n", pname, pname);
        ex_argc = env_makeargs(pname, &ex_argv);
        if (ex_argc != 0)
            if (!process_options(ex_argc, ex_argv)) return (1);
        free_makeargs(ex_argc, ex_argv);
    } /* if (p != NULL) */
    else
        dbprintf("%s: debug: no env var found.\n", pname);

    /*********************************************/
    /* Process the command line options */
    /*********************************************/
    if (!process_options(argc, argv)) return (1);

    if (columns == 0) {
        fprintf(stderr, "%s: error: `columns' can not be zero.\n", pname);
        return (1);
    } /* if (columns == 0) */

    if (data_width == 0) {
        fprintf(stderr, "%s: error: `width' can not be zero.\n", pname);
        return (1);
    } /* if (columns == 0) */

    /* In some cases a dumb person could specify a width with */
    /* a float or double... We want to override any width selections */
    switch (dump_mode) {
        case DUMP_MODE_FLOAT:
            data_width = sizeof(float);
            break;
        case DUMP_MODE_DOUBLE:
            data_width = sizeof(double);
            break;
        default:
            break;
    } /* switch (dump_mode) */

#if !defined(__DOS16__)
    /* determine output */
    if (use_pager) {
        output_file = popen(pager_program, "w");
        if (output_file == NULL) {
            perror("pipe error");
            exit(1);
        } /* if (output_file == NULL) */
    } /* if (use_pager) */
    else {
        output_file = stdout;
    } /* else */
#else
    if (use_pager) {
        output_file = fopen(TMP_OUTFILE, "w");
        if (output_file == NULL) {
            perror("Error opening TMP_OUTFILE");
            exit(1);
        }

    } /* if (use_pager) */
    else
        output_file = stdout;
#endif

    dbprintf("!!debug: dump mode is %#x\n", dump_mode);
    if (optind == argc) {
        /* Use STDIN */
        dbprintf("!!debug: No non-opt args. (use stdin)\n");
        input_file = stdin;
        if (isatty(fileno(input_file))) {
#if defined(__DOSLIKE__)
            fprintf(stderr, "Reading from stdin ...(use ^C to exit)\n");
#else
            fprintf(stderr, "Reading from stdin ...(use ^D to exit)\n");
#endif
        } /* if ( isatty(fileno(input_file)) ) */

#if defined(__QNX__) || defined(__UNIXLIKE__)
        setvbuf(input_file, NULL, _IONBF, 0);
#else
        (void)setmode(fileno(stdin), O_BINARY);
#endif
        (void)dump_file(input_file, output_file);

    } /* if (optind == argc) */
    else {
        for (index = optind; index < argc; index++) {
            dbprintf("!!debug: Non-option argument %s\n", argv[index]);
            dbprintf("!!debug: mode_selected = %d\n", mode_selected);
            if (!mode_selected) {
                dbprintf("!!debug: No mode selected, trying to determine mode.\n");
                if (file_is_bin(argv[index])) {
                    dump_mode = DUMP_MODE_HEX;
                    dbprintf("!!debug: file is binary, dump mode is now %#x\n",
                             dump_mode);
                } /* if (file_is_bin(argv[index])) */
            }
            /* open the file */
            input_file = fopen(argv[index], "rb");
            if (NULL == input_file) {
                fprintf(stderr, "%s: error opening input file <%s> : %s\n", pname,
                        argv[index], strerror(errno));
                break;
            } /* if (NULL == input_file) */
            dbprintf("!!debug: dump mode is %#x\n", dump_mode);

            if (!dump_file(input_file, output_file)) {
                fprintf(stderr, "%s: error processing file <%s>\n", pname, argv[index]);
                (void)fclose(input_file);
                break;
            } /* if (!dump_file(input_file, output_file)) */

            (void)fclose(input_file);

        } /* for (index = optind; index < argc; index++) */
    } /* else */

#if !defined(__DOS16__)
    if (use_pager) (void)pclose(output_file);
#else
    if (use_pager) {
        fclose(output_file);
        sprintf(system_command, "%s %s", pager_program, TMP_OUTFILE);
        system(system_command);
        remove(TMP_OUTFILE);
    } /* if (use_pager) */
#endif

    return (0);

} /* int main( int argc, char **argv ) */
