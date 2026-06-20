/*******************************************************************************
 * SAD - Output Formatters Module
 *
 * Low-level formatters converting binary data to text representations
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
#include "formatters.h"

#include "binstr.h"
#include "jkcommon.h"
#include "platform.h"
#include "sad.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External variables (defined in sad.c) */
extern char* pname;

void print_offset(FILE* outfp, uint32_t offvalue)
{
    unsigned long int offset_value = offvalue;

    if (offset_divisor == 0) offset_divisor = 1; // Dont want any division by zero.

    offset_value /= offset_divisor;

    switch (offset_display) {
        case OFFSET_HEX:
            fprintf(outfp, "%08lX: ", offset_value);
            break;
        case OFFSET_DEC:
            fprintf(outfp, "%010lud: ", offset_value);
            break;
        case OFFSET_OCT:
            fprintf(outfp, "%011loo: ", offset_value);
            break;
        default:
            fprintf(outfp, "%08lX: ", offset_value);
            break;
    }

} /* print_offset() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
bool write_fmtline_bin(FILE* fp, uint32_t current_offset, void* buffer, uint32_t count,
                       int data_size, bool iseof)
{
    BYTE*     p_byte;
    uint16_t* p_word;
    uint32_t* p_dword;
    uint64_t* p_qword;
    int       i, j;
    bool      partial_line;
    int       elements;
    char      data_str[MAXSTRING];
    char*     ascii_str;
    uint32_t  ascii_buf_size;
    BYTE      tmp_char;
    uint32_t  byte_count;
    uint32_t  bytes_per_line;
    char      bin_string[MAX_BIN_STRING];

    if (data_size == 0) return false;
    partial_line = false;
    byte_count   = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker) return true;

    bytes_per_line
        = (((unsigned)columns * (unsigned)data_width * 8) + (unsigned)columns);
    if (columns % 2) --bytes_per_line;

    if (elements < columns) partial_line = true;

    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str      = (char*)malloc(ascii_buf_size);
    if (ascii_str == NULL) {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname,
                __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0]  = 0;
    ascii_str[0] = 0;

    switch (data_size) {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE*)buffer;
            for (i = 0; i < (int)elements; i++) {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                sprintf(data_str, "%s", binstr((uint64_t)*p_byte++, bin_string, 1));
                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 8;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                byte_count += 5;
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%s", binstr((uint64_t)*p_word++, bin_string, 2));
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%s",
                            binstr((uint64_t)BYTE_SWAP_16(*p_word), bin_string, 2));
                    ++p_word;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 8;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                byte_count += 5;
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%s",
                            binstr((uint64_t)*p_dword++, bin_string, 4));
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%s",
                            binstr((uint64_t)BYTE_SWAP_32(*p_dword), bin_string, 4));
                    ++p_dword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 8;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                byte_count += 5;
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%s", binstr(*p_qword, bin_string, 8));
                    ++p_qword;
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%s",
                            binstr(BYTE_SWAP_64(*p_qword), bin_string, 8));
                    ++p_qword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 8;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                byte_count += 5;
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname,
                    data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

} /* bool write_fmtline_bin(uint32_t current_offset, void *buffer, uint32_t count,
     uint32_t data_size) */

bool write_fmtline_hex(FILE* fp, uint32_t current_offset, void* buffer, uint32_t count,
                       int data_size, bool iseof)
{
    BYTE*     p_byte;
    uint16_t* p_word;
    uint32_t* p_dword;
    uint64_t* p_qword;
    int       i, j;
    bool      partial_line;
    int       elements;
    char      data_str[MAXSTRING];
    char*     ascii_str;
    uint32_t  ascii_buf_size;
    BYTE      tmp_char;
    uint32_t  byte_count;
    uint32_t  bytes_per_line;

    if (data_size == 0) return false;
    partial_line = false;
    byte_count   = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker) return true;

    bytes_per_line
        = (((unsigned)columns * (unsigned)data_width * 2) + (unsigned)columns);
    if (columns % 2) --bytes_per_line;

    if (elements < columns) partial_line = true;

    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str      = (char*)malloc(ascii_buf_size);
    if (ascii_str == NULL) {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname,
                __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0]  = 0;
    ascii_str[0] = 0;

    switch (data_size) {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE*)buffer;
            for (i = 0; i < (int)elements; i++) {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                sprintf(data_str, "%02X", *p_byte++);
                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 2;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%04X", *p_word++);
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%04X", BYTE_SWAP_16(*p_word));
                    ++p_word;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 2;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%08lX", *p_dword++);
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%08lX", BYTE_SWAP_32(*p_dword));
                    ++p_dword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 2;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%08lX%08lX", (uint32_t)(*p_qword >> 32),
                            (uint32_t)(*p_qword & 0xffffffff));
                    ++p_qword;
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%08lX%08lX",
                            (uint32_t)BYTE_SWAP_32((*p_qword & 0xffffffff)),
                            (uint32_t)BYTE_SWAP_32((*p_qword >> 32)));
                    ++p_qword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size * 2;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname,
                    data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

} /* bool write_fmtline_hex(uint32_t current_offset, void *buffer, uint32_t count,
     uint32_t data_size) */

bool write_fmtline_dec(FILE* fp, uint32_t current_offset, void* buffer, uint32_t count,
                       int data_size, bool iseof)
{
    BYTE*     p_byte;
    uint16_t* p_word;
    uint32_t* p_dword;
    uint64_t* p_qword;
    int       i, j;
    bool      partial_line;
    int       elements;
    char      data_str[MAXSTRING];
    char*     ascii_str;
    uint32_t  ascii_buf_size;
    BYTE      tmp_char;
    uint32_t  byte_count;
    uint32_t  bytes_per_line;
    int       factor;

    if (data_size == 0) return false;
    partial_line = false;
    byte_count   = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker) return true;

    factor         = (int)((data_width * 2.5) + 0.5);
    bytes_per_line = (uint32_t)((columns * factor) + columns);
    if (columns % 2) --bytes_per_line;

    if (elements < columns) partial_line = true;

    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str      = (char*)malloc(ascii_buf_size);
    if (ascii_str == NULL) {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname,
                __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0]  = 0;
    ascii_str[0] = 0;

    switch (data_size) {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE*)buffer;
            for (i = 0; i < (int)elements; i++) {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                sprintf(data_str, "%03u", *p_byte++);
                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%5u", *p_word++);
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%5u", BYTE_SWAP_16(*p_word));
                    ++p_word;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%10lu", *p_dword++);
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%10lu", BYTE_SWAP_32(*p_dword));
                    ++p_dword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
#if !defined(__GNUC__)
                    sprintf(data_str, "%20I64u", *p_qword);
#else
                    sprintf(data_str, "%20llu", *p_qword);
#endif
                    ++p_qword;
                } /* if (little-endian) */
                else {
#if !defined(__GNUC__)
                    sprintf(data_str, "%20I64u", BYTE_SWAP_64(*p_qword));
#else
                    sprintf(data_str, "%20llu", BYTE_SWAP_64(*p_qword));
#endif
                    ++p_qword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if ((bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = (int)byte_count; i < (int)bytes_per_line; i++)
                    fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname,
                    data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;
}

bool write_fmtline_oct(FILE* fp, uint32_t current_offset, void* buffer, uint32_t count,
                       int data_size, bool iseof)
{
    BYTE*     p_byte;
    uint16_t* p_word;
    uint32_t* p_dword;
    uint64_t* p_qword;
    int       i, j;
    bool      partial_line;
    int       elements;
    char      data_str[MAXSTRING];
    char*     ascii_str;
    uint32_t  ascii_buf_size;
    BYTE      tmp_char;
    int       byte_count;
    uint32_t  bytes_per_line;
    int       factor;

    if (data_size == 0) return false;
    partial_line = false;
    byte_count   = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker) return true;

    factor         = (int)((data_width * 2.75) + 0.5);
    bytes_per_line = (((unsigned)columns * (unsigned)factor) + (unsigned)columns);
    if (columns % 2) --bytes_per_line;

    if (elements < columns) partial_line = true;

    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str      = (char*)malloc(ascii_buf_size);
    if (ascii_str == NULL) {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname,
                __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0]  = 0;
    ascii_str[0] = 0;

    switch (data_size) {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE*)buffer;
            for (i = 0; i < (int)elements; i++) {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                sprintf(data_str, "%03o", *p_byte++);
                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%6o", *p_word++);
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%6o", BYTE_SWAP_16(*p_word));
                    ++p_word;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%11lo", *p_dword++);
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "%11lo", BYTE_SWAP_32(*p_dword));
                    ++p_dword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
#if !defined(__GNUC__)
                    sprintf(data_str, "%22I64o", *p_qword);
#else
                    sprintf(data_str, "%22llo", *p_qword);
#endif
                    ++p_qword;
                } /* if (little-endian) */
                else {
#if !defined(__GNUC__)
                    sprintf(data_str, "%22I64o", BYTE_SWAP_64(*p_qword));
#else
                    sprintf(data_str, "%22llo", BYTE_SWAP_64(*p_qword));
#endif
                    ++p_qword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname,
                    data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;
}

bool write_fmtline_signed(FILE* fp, uint32_t current_offset, void* buffer,
                          uint32_t count, int data_size, bool iseof)
{
    BYTE*     p_byte;
    uint16_t* p_word;
    uint32_t* p_dword;
    uint64_t* p_qword;
    int       i, j;
    bool      partial_line;
    int       elements;
    char      data_str[MAXSTRING];
    char*     ascii_str;
    uint32_t  ascii_buf_size;
    BYTE      tmp_char;
    int       byte_count;
    uint32_t  bytes_per_line;
    int       factor;

    if (data_size == 0) return false;
    partial_line = false;
    byte_count   = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker) return true;

    factor         = (int)((data_width * 2.5) + 1.5);
    bytes_per_line = (uint32_t)((columns * factor) + columns);
    if (columns % 2) --bytes_per_line;

    if (elements < columns) partial_line = true;

    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str      = (char*)malloc(ascii_buf_size);
    if (ascii_str == NULL) {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname,
                __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0]  = 0;
    ascii_str[0] = 0;

    switch (data_size) {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE*)buffer;
            for (i = 0; i < (int)elements; i++) {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                sprintf(data_str, "%4d", (int)(*(signed char*)p_byte++));
                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "%4s", "@");
                    byte_count += 4;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "% 6d", (int16_t)(*(int16_t*)p_word++));
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "% 6d", (int16_t)BYTE_SWAP_16(*(int16_t*)p_word));
                    ++p_word;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, " %ceof%c", E_BEGIN, E_END);
                    byte_count += 6;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "%6s", "@");
                    byte_count += 6;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "% 11ld", (int32_t)(*(int32_t*)p_dword++));
                } /* if (little-endian) */
                else {
                    sprintf(data_str, "% 11ld",
                            (int32_t)BYTE_SWAP_32(*(int32_t*)p_dword));
                    ++p_dword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, " %ceof%c%5s", E_BEGIN, E_END, " ");
                    byte_count += 11;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "%11s", "@");
                    byte_count += 11;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
#if !defined(__GNUC__)
                    sprintf(data_str, "% 21I64d", (int64_t)(*(int64_t*)p_qword));
#else
                    sprintf(data_str, "% 21lld", (int64_t)(*(int64_t*)p_qword));
#endif
                    ++p_qword;
                } /* if (little-endian) */
                else {
#if !defined(__GNUC__)
                    sprintf(data_str, "% 21I64d",
                            (int64_t)BYTE_SWAP_64(*(int64_t*)p_qword));
#else
                    sprintf(data_str, "% 21lld",
                            (int64_t)BYTE_SWAP_64(*(int64_t*)p_qword));
#endif
                    ++p_qword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, " %ceof%c%15s", E_BEGIN, E_END, " ");
                    byte_count += 21;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "%21s", "@");
                    byte_count += 21;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname,
                    data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;
}

bool write_fmtline_float(FILE* fp, uint32_t current_offset, void* buffer,
                         uint32_t count, int data_size, bool iseof)
{
    uint32_t* p_dword;
    uint64_t* p_qword;
    int       i, j;
    bool      partial_line;
    int       elements;
    char      data_str[MAXSTRING];
    char*     ascii_str;
    uint32_t  ascii_buf_size;
    BYTE      tmp_char;
    int       byte_count;
    uint32_t  bytes_per_line;
    int       factor;

    if (data_size == 0) return false;
    partial_line = false;
    byte_count   = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker) return true;

    if ((unsigned)data_size == sizeof(float))
        factor = 14;
    else
        factor = 15;

    bytes_per_line = (((unsigned)columns * (unsigned)factor) + (unsigned)columns);
    if (columns % 2) --bytes_per_line;

    if (elements < columns) partial_line = true;

    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str      = (char*)malloc(ascii_buf_size);
    if (ascii_str == NULL) {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname,
                __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0]  = 0;
    ascii_str[0] = 0;

    if (float_precision == 0) float_precision = 6;

    switch (data_size) {
        case 4:
            if (float_width == 0) float_width = 14;
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%*.*g", float_width, float_precision,
                            (float)(*((float*)p_dword)));
                    ++p_dword;
                } /* if (little-endian) */
                else {
                    *p_dword = BYTE_SWAP_32(*p_dword);
                    sprintf(data_str, "%*.*g", float_width, float_precision,
                            (float)(*((float*)p_dword)));
                    ++p_dword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (float_width == 0) float_width = 15;
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64_t*)buffer;
            for (i = 0; i < (int)elements; i++) {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i + 1] = 0;

                if (!big_endian) {
                    sprintf(data_str, "%*.*g", float_width, float_precision,
                            (double)(*((double*)p_qword)));
                    ++p_qword;
                } /* if (little-endian) */
                else {
                    *p_qword = BYTE_SWAP_64(*p_qword);
                    sprintf(data_str, "%*.*g", float_width, float_precision,
                            (double)(*((double*)p_qword)));
                    ++p_qword;
                } /* else */

                if (highlight) {
                    for (j = 0; j < (int)strlen(data_str); j++) {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns % 2 == 0) {
                    if (i == (((int)columns / 2) - 1)) {
                        fprintf(fp, "  ");
                        byte_count += 2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements - 1) || partial_line) {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements - 1) || partial_line) {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */

            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker) {
                if (((int)bytes_per_line - byte_count) >= 5) {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count += 5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i = byte_count; i < (int)bytes_per_line; i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d> for format.\n",
                    pname, data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;
}

/*lint -restore*/
