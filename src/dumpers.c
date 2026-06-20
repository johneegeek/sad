/*******************************************************************************
 * SAD - Dumpers Module
 *
 * High-level dump orchestration - reads files and calls formatters
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/

#include "dumpers.h"

#include "formatters.h"
#include "jkcommon.h"
#include "platform.h"
#include "sad.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External variables and functions (defined in sad.c) */
extern char* pname;

/* Forward declarations for functions still in sad.c */
extern int get_input(FILE* fp);

bool dump_file(FILE* infile_ptr, FILE* outfile_ptr)
{
    uint32_t dump_file_offset;

    dump_file_offset = 0;

    if (data_start_count != 0) {
        /* ignore everything to the starting count */
        int64_t total_bytes;

        total_bytes = data_start_count * data_width;
        while (dump_file_offset < total_bytes) {
            /*lint -esym(550,junk)*/
            int junk;
            if ((junk = fgetc(infile_ptr)) == EOF) return true;
            ++dump_file_offset;
        } /* while (current_total < total_bytes) */

    } /* if (data_start_count != 0) */

    switch (dump_mode) {
        case DUMP_MODE_HEX:
            return (dump_file_hex(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_ASCII:
            return (dump_file_ascii(infile_ptr, outfile_ptr));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_FASCII:
            return (dump_file_asciifixed(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_CTRL:
            return (dump_file_control(infile_ptr, outfile_ptr));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_DEC:
            return (dump_file_dec(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_OCT:
            return (dump_file_oct(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_SIGNED:
            return (dump_file_signed(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_FLOAT:
            return (dump_file_float(infile_ptr, outfile_ptr, sizeof(float),
                                    dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_DOUBLE:
            return (dump_file_float(infile_ptr, outfile_ptr, sizeof(double),
                                    dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_BINARY:
            return (dump_file_binary(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_UNKNOWN:
            /*FALLTHRU*/
        default:
            fprintf(stderr, "%s: dump error: unknown mode <%#x>.\n", pname, dump_mode);
            return false;
            /*NOTREACHED*/
            break;
    } /* switch(dump_mode) */

    /*NOTREACHED*/
    return true;

} /* bool dump_file(FILE *infile_ptr, FILE *outfile_ptr) */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

bool dump_file_ascii(FILE* ifile, FILE* ofile)
{
    /* pointers to files must be opened and valid */

    int in_char = 0;

    while (in_char != EOF) {
        if ((in_char = get_input(ifile)) == EOF) {
            if (show_eof_marker) fprintf(ofile, "%ceof%c", E_BEGIN, E_END);
            fflush(ofile);
            continue;
        }

        if (in_char < 32) /* Control characters 0-31 */
        {
            /* in_char is in CtrlTable */
            fprintf(ofile, "%c%s%c", B_BEGIN, CtrlTable[in_char], B_END);
            if (in_char == CH_NL) fprintf(ofile, "\n");
            fflush(ofile);
            continue;
        }

        if (in_char == CH_DEL) {
            fprintf(ofile, "%cdel%c", B_BEGIN, B_END);
            continue;
        }

        if (!isprint(in_char)) {
            fprintf(ofile, "%c0x%02X%c", B_BEGIN, in_char, B_END);
        }
        else {
            if ((in_char == CH_SPACE) && show_spaces)
                putc('\xB7', ofile);
            else if (highlight)
                fprintf(ofile, "%c\b%c", in_char, in_char);
            else
                putc(in_char, ofile);
        }
    }

    return true;
}

bool dump_file_asciifixed(FILE* ifile, FILE* ofile, uint32_t cur_offset)
{
    /* pointers to files must be opened and valid */

    int in_char     = 0;
    int ascii_count = 0;

    while (in_char != EOF) {
        if (ascii_count == DEFAULT_COLUMNS_ASCII) {
            fprintf(ofile, "\n");
            ascii_count = 0;
        }

        if ((in_char = get_input(ifile)) == EOF) {
            if (show_eof_marker) fprintf(ofile, "%ceof%c ", E_BEGIN, E_END);
            fflush(ofile);
            continue;
        }

        if (ascii_count == 0) {
            if (show_offset) print_offset(ofile, cur_offset);
            cur_offset += DEFAULT_COLUMNS_ASCII;
        }

        ++ascii_count;

        if (in_char < 32) /* Control characters 0-31 */
        {
            /* in_char is in CtrlTable */
            fprintf(ofile, "%c%3s%c ", B_BEGIN, CtrlTable[in_char], B_END);
            fflush(ofile);
            continue;
        }

        if (in_char == CH_DEL) {
            fprintf(ofile, "%cdel%c ", B_BEGIN, B_END);
            continue;
        }

        if (!isprint(in_char)) {
            fprintf(ofile, "%c0x%02X%c", B_BEGIN, in_char, B_END);
        }
        else {
            if (highlight)
                fprintf(ofile, "[  %c\b%c ]", in_char, in_char);
            else
                fprintf(ofile, "[  %c ]", in_char);
        }
    }

    return true;
}

/*****************************************************************************\
 Function: dump_file_binary()

 Description: Dumps a file in binary

 Returns:
\*****************************************************************************/
bool dump_file_binary(FILE* ifile, FILE* ofile, uint32_t offset)
{
    int      in_char;
    char*    linebuf;
    char*    lastlinebuf;
    uint32_t data_per_line;
    uint32_t line_buf_size;
    uint32_t data_count;
    bool     first_line;
    bool     duplicate;
    bool     eof_flag;

    if (!columns_selected) {
        switch (data_width) {
            case 1:
                columns = DEFAULT_COLUMNS_BIN;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2BIN;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4BIN;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8BIN;
                break;
            default:
                columns = DEFAULT_COLUMNS_BIN;
                break;
        }
    }

    first_line    = true;
    duplicate     = false;
    eof_flag      = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;

    linebuf = (char*)malloc(line_buf_size);
    if (linebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char*)malloc(line_buf_size);
    if (lastlinebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */

    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1) {
        /*fill line buffer*/

        in_char  = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;

        if (eof_flag
            || (data_count
                == (data_per_line - 1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line) {
                if (!write_fmtline_bin(ofile, offset, linebuf, data_count + 1,
                                       data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else {
                if (!no_duplicate
                    && (0 == memcmp(linebuf, lastlinebuf, line_buf_size))) {
                    if (!duplicate) {
                        fprintf(ofile, "%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else {
                    duplicate = false;
                    if (!write_fmtline_bin(ofile, offset, linebuf, data_count + 1,
                                           data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count + 1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF) break;

    } /* while (1) */

    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_binary() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

bool dump_file_control(FILE* ifile, FILE* ofile)
{
    /* pointers to files must be opened and valid */

    int in_char = 0;

    while (in_char != EOF) {
        if ((in_char = get_input(ifile)) == EOF) {
            if (show_eof_marker) fprintf(ofile, "%ceof%c", E_BEGIN, E_END);
            fflush(ofile);
            continue;
        }

        if (in_char < 32) /* Control characters 0-31 */
        {
            /* in_char is in CtrlTable */
            fprintf(ofile, "%c%s%c", B_BEGIN, CCTable[in_char], B_END);
            if (in_char == CH_NL) fprintf(ofile, "\n");
            fflush(ofile);
            continue;
        }

        if (in_char == CH_DEL) {
            fprintf(ofile, "%cdel%c", B_BEGIN, B_END);
            continue;
        }

        if (!isprint(in_char)) {
            fprintf(ofile, "%c0x%02X%c", B_BEGIN, in_char, B_END);
        }
        else {
            if (highlight)
                fprintf(ofile, "%c\b%c", in_char, in_char);
            else
                putc(in_char, ofile);
        }
    }

    fflush(ofile);
    return true;
}

bool dump_file_dec(FILE* ifile, FILE* ofile, uint32_t offset)
{
    int      in_char;
    char*    linebuf;
    char*    lastlinebuf;
    uint32_t data_per_line;
    uint32_t line_buf_size;
    uint32_t data_count;
    bool     first_line;
    bool     duplicate;
    bool     eof_flag;

    if (!columns_selected) {
        switch (data_width) {
            case 1:
                columns = DEFAULT_COLUMNS_DEC;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2DEC;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4DEC;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8DEC;
                break;
            default:
                columns = DEFAULT_COLUMNS_DEC;
                break;
        }
    }

    first_line    = true;
    duplicate     = false;
    eof_flag      = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;

    linebuf = (char*)malloc(line_buf_size);
    if (linebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char*)malloc(line_buf_size);
    if (lastlinebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */

    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1) {
        /*fill line buffer*/

        in_char  = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;

        if (eof_flag
            || (data_count
                == (data_per_line - 1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line) {
                if (!write_fmtline_dec(ofile, offset, linebuf, data_count + 1,
                                       data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else {
                if (!no_duplicate
                    && (0 == memcmp(linebuf, lastlinebuf, line_buf_size))) {
                    if (!duplicate) {
                        fprintf(ofile, "%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else {
                    duplicate = false;
                    if (!write_fmtline_dec(ofile, offset, linebuf, data_count + 1,
                                           data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count + 1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF) break;

    } /* while (1) */

    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_dec() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

bool dump_file_float(FILE* ifile, FILE* ofile, int abs_data_width, uint32_t offset)
{
    int      in_char;
    char*    linebuf;
    char*    lastlinebuf;
    uint32_t data_per_line;
    uint32_t line_buf_size;
    uint32_t data_count;
    bool     first_line;
    bool     duplicate;
    bool     eof_flag;

    if (!columns_selected) {
        switch (abs_data_width) {
            case sizeof(float):
                columns = DEFAULT_COLUMNS_FLOAT;
                break;
            case sizeof(double):
                columns = DEFAULT_COLUMNS_DOUBLE;
                break;
            default:
                fprintf(stderr, "%s: unsupported data with for type float - %d\n",
                        pname, abs_data_width);
                break;
        } /* switch (abs_data_width) */
    }     /* if (!columns_selected) */

    first_line    = true;
    duplicate     = false;
    eof_flag      = false;
    line_buf_size = (unsigned)columns * (unsigned)abs_data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)abs_data_width;

    linebuf = (char*)malloc(line_buf_size);
    if (linebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char*)malloc(line_buf_size);
    if (lastlinebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */

    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1) {
        /*fill line buffer*/

        in_char  = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;

        if (eof_flag
            || (data_count
                == (data_per_line - 1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line) {
                if (!write_fmtline_float(ofile, offset, linebuf, data_count + 1,
                                         abs_data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else {
                if (!no_duplicate
                    && (0 == memcmp(linebuf, lastlinebuf, line_buf_size))) {
                    if (!duplicate) {
                        fprintf(ofile, "%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else {
                    duplicate = false;
                    if (!write_fmtline_float(ofile, offset, linebuf, data_count + 1,
                                             abs_data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count + 1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF) break;

    } /* while (1) */

    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_float() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/*****************************************************************************\
 Function: dump_file_hex()

 Description: Dumps a file in hex

 Returns:
\*****************************************************************************/
bool dump_file_hex(FILE* ifile, FILE* ofile, uint32_t offset)
{
    int      in_char;
    char*    linebuf;
    char*    lastlinebuf;
    uint32_t data_per_line;
    uint32_t line_buf_size;
    uint32_t data_count;
    bool     first_line;
    bool     duplicate;
    bool     eof_flag;

    if (!columns_selected) {
        switch (data_width) {
            case 1:
                columns = DEFAULT_COLUMNS_HEX;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2HEX;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4HEX;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8HEX;
                break;
            default:
                columns = DEFAULT_COLUMNS_HEX;
                break;
        }
    }

    first_line    = true;
    duplicate     = false;
    eof_flag      = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;

    linebuf = (char*)malloc(line_buf_size);
    if (linebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char*)malloc(line_buf_size);
    if (lastlinebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */

    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1) {
        /*fill line buffer*/

        in_char  = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;

        if (eof_flag
            || (data_count
                == (data_per_line - 1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line) {
                if (!write_fmtline_hex(ofile, offset, linebuf, data_count + 1,
                                       data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else {
                if (!no_duplicate
                    && (0 == memcmp(linebuf, lastlinebuf, line_buf_size))) {
                    if (!duplicate) {
                        fprintf(ofile, "%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else {
                    duplicate = false;
                    if (!write_fmtline_hex(ofile, offset, linebuf, data_count + 1,
                                           data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count + 1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF) break;

    } /* while (1) */

    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_hex() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

bool dump_file_oct(FILE* ifile, FILE* ofile, uint32_t offset)
{
    int      in_char;
    char*    linebuf;
    char*    lastlinebuf;
    uint32_t data_per_line;
    uint32_t line_buf_size;
    uint32_t data_count;
    bool     first_line;
    bool     duplicate;
    bool     eof_flag;

    if (!columns_selected) {
        switch (data_width) {
            case 1:
                columns = DEFAULT_COLUMNS_OCT;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2OCT;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4OCT;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8OCT;
                break;
            default:
                columns = DEFAULT_COLUMNS_OCT;
                break;
        }
    }

    first_line    = true;
    duplicate     = false;
    eof_flag      = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;

    linebuf = (char*)malloc(line_buf_size);
    if (linebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char*)malloc(line_buf_size);
    if (lastlinebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */

    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1) {
        /*fill line buffer*/

        in_char  = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;

        if (eof_flag
            || (data_count
                == (data_per_line - 1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line) {
                if (!write_fmtline_oct(ofile, offset, linebuf, data_count + 1,
                                       data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else {
                if (!no_duplicate
                    && (0 == memcmp(linebuf, lastlinebuf, line_buf_size))) {
                    if (!duplicate) {
                        fprintf(ofile, "%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else {
                    duplicate = false;
                    if (!write_fmtline_oct(ofile, offset, linebuf, data_count + 1,
                                           data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count + 1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF) break;

    } /* while (1) */

    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_oct() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

bool dump_file_signed(FILE* ifile, FILE* ofile, uint32_t offset)
{
    int      in_char;
    char*    linebuf;
    char*    lastlinebuf;
    uint32_t data_per_line;
    uint32_t line_buf_size;
    uint32_t data_count;
    bool     first_line;
    bool     duplicate;
    bool     eof_flag;

    if (!columns_selected) {
        switch (data_width) {
            case 1:
                columns = DEFAULT_COLUMNS_SND;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2SND;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4SND;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8SND;
                break;
            default:
                columns = DEFAULT_COLUMNS_SND;
                break;
        }
    }

    first_line    = true;
    duplicate     = false;
    eof_flag      = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;

    linebuf = (char*)malloc(line_buf_size);
    if (linebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char*)malloc(line_buf_size);
    if (lastlinebuf == NULL) {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname,
                __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */

    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1) {
        /*fill line buffer*/

        in_char  = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;

        if (eof_flag
            || (data_count
                == (data_per_line - 1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line) {
                if (!write_fmtline_signed(ofile, offset, linebuf, data_count + 1,
                                          data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else {
                if (!no_duplicate
                    && (0 == memcmp(linebuf, lastlinebuf, line_buf_size))) {
                    if (!duplicate) {
                        fprintf(ofile, "%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else {
                    duplicate = false;
                    if (!write_fmtline_signed(ofile, offset, linebuf, data_count + 1,
                                              data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count + 1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF) break;

    } /* while (1) */

    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_signed() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
