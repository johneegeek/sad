/*******************************************************************************
 * SAD - Dumpers Module Header
 *
 * High-level dump orchestration - reads files and calls formatters
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
#ifndef _DUMPERS_H
#define _DUMPERS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* External global variables used by dumpers (defined in sad.c) */
extern uint16_t dump_mode;
extern int      columns;
extern int      data_width;
extern int32_t  data_end_count;
extern int32_t  data_start_count;
extern bool     show_eof_marker;
extern bool     show_spaces;

/*****************************************************************************\
 Function: dump_file()

 Description: Main dump dispatcher - calls appropriate dump_file_* function
              based on the selected dump mode

 Parameters:
   infile_ptr  - Input file pointer (must be opened)
   outfile_ptr - Output file pointer (must be opened)

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file(FILE* infile_ptr, FILE* outfile_ptr);

/*****************************************************************************\
 Function: dump_file_ascii()

 Description: Dumps file in ASCII mode with control character names

 Parameters:
   ifile - Input file pointer
   ofile - Output file pointer

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_ascii(FILE* ifile, FILE* ofile);

/*****************************************************************************\
 Function: dump_file_asciifixed()

 Description: Dumps file in fixed ASCII mode

 Parameters:
   ifile      - Input file pointer
   ofile      - Output file pointer
   cur_offset - Current byte offset

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_asciifixed(FILE* ifile, FILE* ofile, uint32_t cur_offset);

/*****************************************************************************\
 Function: dump_file_binary()

 Description: Dumps file as binary text (0s and 1s)

 Parameters:
   ifile  - Input file pointer
   ofile  - Output file pointer
   offset - Starting byte offset

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_binary(FILE* ifile, FILE* ofile, uint32_t offset);

/*****************************************************************************\
 Function: dump_file_control()

 Description: Dumps file with control characters displayed

 Parameters:
   ifile - Input file pointer
   ofile - Output file pointer

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_control(FILE* ifile, FILE* ofile);

/*****************************************************************************\
 Function: dump_file_dec()

 Description: Dumps file as decimal numbers

 Parameters:
   ifile  - Input file pointer
   ofile  - Output file pointer
   offset - Starting byte offset

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_dec(FILE* ifile, FILE* ofile, uint32_t offset);

/*****************************************************************************\
 Function: dump_file_float()

 Description: Dumps file as floating-point numbers

 Parameters:
   ifile          - Input file pointer
   ofile          - Output file pointer
   abs_data_width - Absolute data width (4 for float, 8 for double)
   offset         - Starting byte offset

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_float(FILE* ifile, FILE* ofile, int abs_data_width, uint32_t offset);

/*****************************************************************************\
 Function: dump_file_hex()

 Description: Dumps file as hexadecimal

 Parameters:
   ifile  - Input file pointer
   ofile  - Output file pointer
   offset - Starting byte offset

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_hex(FILE* ifile, FILE* ofile, uint32_t offset);

/*****************************************************************************\
 Function: dump_file_oct()

 Description: Dumps file as octal numbers

 Parameters:
   ifile  - Input file pointer
   ofile  - Output file pointer
   offset - Starting byte offset

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_oct(FILE* ifile, FILE* ofile, uint32_t offset);

/*****************************************************************************\
 Function: dump_file_signed()

 Description: Dumps file as signed decimal numbers

 Parameters:
   ifile  - Input file pointer
   ofile  - Output file pointer
   offset - Starting byte offset

 Returns: true on success, false on failure
\*****************************************************************************/
bool dump_file_signed(FILE* ifile, FILE* ofile, uint32_t offset);

#endif /* !_DUMPERS_H */
