/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 Output Formatters Module

 This module contains the low-level formatting functions that convert binary
 data into various text representations (hex, decimal, octal, binary, etc.)
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef _FORMATTERS_H
#define _FORMATTERS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* External global variables used by formatters (defined in sad.c) */
extern int      columns;
extern int      data_width;
extern bool     show_offset;
extern bool     show_ascii;
extern bool     big_endian;
extern int      offset_display;
extern uint32_t offset_divisor;
extern int      float_width;
extern int      float_precision;
extern bool     show_eof_marker;
extern bool     highlight;
extern bool     no_duplicate;
extern bool     columns_selected;

/*****************************************************************************\
 Function: print_offset()

 Description: Prints the offset value in the appropriate format (hex/dec/oct)

 Parameters:
   outfp     - Output file pointer
   offvalue  - Offset value to print

 Returns: void
\*****************************************************************************/
void print_offset(FILE *outfp, uint32_t offvalue);

/*****************************************************************************\
 Function: write_fmtline_bin()

 Description: Formats and writes a line of binary text output

 Parameters:
   fp              - Output file pointer
   current_offset  - Current byte offset in file
   buffer          - Data buffer to format
   count           - Number of elements to format
   data_size       - Size of each element (1, 2, 4, or 8 bytes)
   iseof           - True if this is the last line (end of file)

 Returns: true on success, false on failure
\*****************************************************************************/
bool write_fmtline_bin(FILE *fp, uint32_t current_offset, void *buffer,
                       uint32_t count, int data_size, bool iseof);

/*****************************************************************************\
 Function: write_fmtline_hex()

 Description: Formats and writes a line of hexadecimal output

 Parameters:
   fp              - Output file pointer
   current_offset  - Current byte offset in file
   buffer          - Data buffer to format
   count           - Number of elements to format
   data_size       - Size of each element (1, 2, 4, or 8 bytes)
   iseof           - True if this is the last line (end of file)

 Returns: true on success, false on failure
\*****************************************************************************/
bool write_fmtline_hex(FILE *fp, uint32_t current_offset, void *buffer,
                       uint32_t count, int data_size, bool iseof);

/*****************************************************************************\
 Function: write_fmtline_dec()

 Description: Formats and writes a line of decimal output

 Parameters:
   fp              - Output file pointer
   current_offset  - Current byte offset in file
   buffer          - Data buffer to format
   count           - Number of elements to format
   data_size       - Size of each element (1, 2, 4, or 8 bytes)
   iseof           - True if this is the last line (end of file)

 Returns: true on success, false on failure
\*****************************************************************************/
bool write_fmtline_dec(FILE *fp, uint32_t current_offset, void *buffer,
                       uint32_t count, int data_size, bool iseof);

/*****************************************************************************\
 Function: write_fmtline_oct()

 Description: Formats and writes a line of octal output

 Parameters:
   fp              - Output file pointer
   current_offset  - Current byte offset in file
   buffer          - Data buffer to format
   count           - Number of elements to format
   data_size       - Size of each element (1, 2, 4, or 8 bytes)
   iseof           - True if this is the last line (end of file)

 Returns: true on success, false on failure
\*****************************************************************************/
bool write_fmtline_oct(FILE *fp, uint32_t current_offset, void *buffer,
                       uint32_t count, int data_size, bool iseof);

/*****************************************************************************\
 Function: write_fmtline_signed()

 Description: Formats and writes a line of signed decimal output

 Parameters:
   fp              - Output file pointer
   current_offset  - Current byte offset in file
   buffer          - Data buffer to format
   count           - Number of elements to format
   data_size       - Size of each element (1, 2, 4, or 8 bytes)
   iseof           - True if this is the last line (end of file)

 Returns: true on success, false on failure
\*****************************************************************************/
bool write_fmtline_signed(FILE *fp, uint32_t current_offset, void *buffer,
                          uint32_t count, int data_size, bool iseof);

/*****************************************************************************\
 Function: write_fmtline_float()

 Description: Formats and writes a line of floating-point output

 Parameters:
   fp              - Output file pointer
   current_offset  - Current byte offset in file
   buffer          - Data buffer to format
   count           - Number of elements to format
   data_size       - Size of each element (4 for float, 8 for double)
   iseof           - True if this is the last line (end of file)

 Returns: true on success, false on failure
\*****************************************************************************/
bool write_fmtline_float(FILE *fp, uint32_t current_offset, void *buffer,
                         uint32_t count, int data_size, bool iseof);

#endif /* !_FORMATTERS_H */
