/*******************************************************************************
 * SAD - Options Module
 *
 * Command-line option processing and configuration
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

#include "options.h"

#include "getopt.h"
#include "help.h"
#include "jkcommon.h"
#include "platform.h"
#include "sad.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External variables (defined in sad.c) */
extern char*    pname;
extern uint16_t dump_mode;
extern char     pager_program[];
extern FILE*    output_file;
extern int      columns;
extern int      data_width;
extern int32_t  data_end_count;
extern int32_t  data_start_count;
extern bool     use_pager;
extern bool     show_offset;
extern bool     show_ascii;
extern bool     highlight;
extern bool     no_duplicate;
extern bool     show_eof_marker;
extern bool     big_endian;
extern bool     mode_selected;
extern bool     width_selected;
extern bool     columns_selected;
extern bool     show_spaces;
extern int      offset_display;
extern uint32_t offset_divisor;
extern int      float_width;
extern int      float_precision;

/* External function */
extern uint64_t string_to_uint64(const char* val, bool* valid);

/*****************************************************************************\
 Function: process_options()

 Description: Process command line options

 Returns:
\*****************************************************************************/
bool process_options(int argc, char** argv)
{
    int     opt;
    int32_t bytes;
    bool    okay;

    optind = 0;

    /* Check command line options */
    while (true) {
        int                  option_index   = 0;
        /*@ignore@*/
        static struct option long_options[] = {
            {          "ascii",       no_argument, 0, 'a'},
            {"ascii-formatted",       no_argument, 0, 'A'},
            {         "binary",       no_argument, 0, 'b'},
            {"formatted-ascii",       no_argument, 0, 'A'},
            {     "big-endian",       no_argument, 0, 'B'},
            {  "little-endian",       no_argument, 0, 'L'},
            {        "columns", required_argument, 0, 'n'},
            {        "control",       no_argument, 0, 'c'},
            {        "decimal",       no_argument, 0, 'd'},
            {    "show-spaces",       no_argument, 0, 'e'},
            {         "signed",       no_argument, 0, 's'},
            {            "hex",       no_argument, 0, 'x'},
            {          "float",       no_argument, 0, 'f'},
            {         "double",       no_argument, 0, 'l'},
            {       "no-ascii",       no_argument, 0, 'I'},
            {  "no-eof-marker",       no_argument, 0, 'E'},
            {   "no-highlight",       no_argument, 0, 'H'},
            {      "no-offset",       no_argument, 0, 'F'},
            {          "octal",       no_argument, 0, 'o'},
            {          "pager", optional_argument, 0, 'p'},
            { "show-duplicate",       no_argument, 0, 'U'},
            {          "width", required_argument, 0, 'w'},
            {    "float-width", required_argument, 0, 'W'},
            {"float-precision", required_argument, 0, 'P'},
            {          "bytes",       no_argument, 0, '1'},
            {           "8bit",       no_argument, 0, '1'},
            {          "words",       no_argument, 0, '2'},
            {          "16bit",       no_argument, 0, '2'},
            {         "dwords",       no_argument, 0, '4'},
            {          "32bit",       no_argument, 0, '4'},
            {         "qwords",       no_argument, 0, '8'},
            {          "64bit",       no_argument, 0, '8'},
            {     "offset-oct",       no_argument, 0, 'q'},
            {     "offset-dec",       no_argument, 0, 'z'},
            { "offset-divisor", required_argument, 0, 'V'},
            {    "start-count", required_argument, 0, 'j'},
            {   "start-offset", required_argument, 0, 'J'},
            {          "count", required_argument, 0, 'k'},
            {     "end-offset", required_argument, 0, 'K'},

#ifdef DEBUG
            {          "debug",       no_argument, 0, 'D'},
#endif
            {           "help",       no_argument, 0, '?'},
            {        "version",       no_argument, 0, 'v'},
            {                0,                 0, 0,   0}
        };
        /*@end@*/
        opt = getopt_long(argc, argv,
                          "bk:j:J:K:flscaAxdeo?Dvp::P:w:W:n:IFHUELB1248qzV:",
                          long_options, &option_index);

        if (opt == -1) break;

        switch (opt) {
            case 0:
                break;

            case 'x':
                dump_mode     = DUMP_MODE_HEX;
                mode_selected = true;
                break;
            case 'c':
                dump_mode     = DUMP_MODE_CTRL;
                mode_selected = true;
                break;
            case 'a':
                dump_mode     = DUMP_MODE_ASCII;
                mode_selected = true;
                break;
            case 'A':
                dump_mode     = DUMP_MODE_FASCII;
                mode_selected = true;
                break;
            case 'b':
                dump_mode     = DUMP_MODE_BINARY;
                mode_selected = true;
                break;
            case 'd':
                dump_mode     = DUMP_MODE_DEC;
                mode_selected = true;
                break;
            case 'o':
                dump_mode     = DUMP_MODE_OCT;
                mode_selected = true;
                break;
            case 's':
                dump_mode     = DUMP_MODE_SIGNED;
                mode_selected = true;
                break;
            case 'f':
                dump_mode     = DUMP_MODE_FLOAT;
                data_width    = sizeof(float);
                mode_selected = true;
                break;
            case 'l':
                dump_mode     = DUMP_MODE_DOUBLE;
                data_width    = sizeof(double);
                mode_selected = true;
                break;

            case 'q':
                offset_display = OFFSET_OCT;
                break;
            case 'z':
                offset_display = OFFSET_DEC;
                break;

            case '1':
                data_width     = 1;
                width_selected = true;
                break;
            case '2':
                data_width     = 2;
                width_selected = true;
                break;
            case '4':
                data_width     = 4;
                width_selected = true;
                break;
            case '8':
                data_width     = 8;
                width_selected = true;
                break;

            case 'p':
                use_pager = true;
                if (optarg) strcpy(pager_program, optarg);
                break;
#ifdef DEBUG
            case 'D':
                print_debug_messages = true;
                break;
#endif
            case 'I':
                show_ascii = false;
                break;
            case 'F':
                show_offset = false;
                break;
            case 'H':
                highlight = false;
                break;
            case 'U':
                no_duplicate = true;
                break;
            case 'E':
                show_eof_marker = false;
                break;
            case 'e':
                show_spaces = true;
                break;
            case 'L':
                /*FALL THRU*/
            case 'B':
                big_endian = true;
                break;

            case 'V':
                offset_divisor = (uint32_t)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting offset divisor string: `%s'\n",
                            optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                if (offset_divisor == 0) {
                    fprintf(stderr,
                            "\a%s: warning: `offset-divisor' can not be zero, "
                            "defaulting to 1.\n",
                            pname);
                    offset_divisor = 1;
                }
                break;
            case 'w':
                // data_width = atoi(optarg);
                data_width = (int)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting data width string: `%s'\n",
                            optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                width_selected = true;
                break;

            case 'W':
                // float_width = atoi(optarg);
                float_width = (int)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting float width string: `%s'\n",
                            optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case 'P':
                // float_precision = atoi(optarg);
                float_precision = (int)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting float precision string: `%s'\n",
                            optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case 'n':
                // columns = atoi(optarg);
                columns = (int)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting columns string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                columns_selected = true;
                break;

            case 'J':
                // bytes = atol(optarg);
                bytes = (long)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting bytes string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                if (data_width != 0) data_start_count = bytes / data_width;
                break;
            case 'j':
                // data_start_count = atol(optarg);
                data_start_count = (long)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting data start count string: `%s'\n",
                            optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case 'K':
                // bytes = atol(optarg);
                bytes = (long)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting bytes string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                if (data_width != 0) data_end_count = bytes / data_width;
                break;
            case 'k':
                // data_end_count = atol(optarg);
                data_end_count = (long)string_to_uint64(optarg, &okay);
                if (!okay) {
                    fprintf(stderr, "Error converting data end count string: `%s'\n",
                            optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case '?':
                /*FALLTHRU*/
            case ':':
                show_help();
                exit(0);
                /*NOTREACHED*/
                break;
            case 'v':
                show_version();
                exit(0);
                /*NOTREACHED*/
                break;

            default:
                break;
        } /*switch*/

    } /* while command line args */

    return true;

} /* process_options() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/*****************************************************************************\
 Function: show_help()

 Description: Shows help message

 Returns:
\*****************************************************************************/
