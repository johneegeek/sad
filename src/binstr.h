/*******************************************************************************
 * SAD - Binary String Module Header
 *
 * Binary string representation utilities for converting integers to binary
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/
#ifndef _BINSTR_H
#define _BINSTR_H

#include <stdint.h>

/* prototypes */

/*****************************************************************************\
 Function: binstr()

 Description: converts integer to string representation of binary

    value is the integer value
    buffer is an initialized area of memory big enough to store requested data
    bytes is the number of bytes to display/convert

 Returns:
   binary string of value.
\*****************************************************************************/
extern char* binstr(uint64_t value, void* buffer, size_t bytes);

#endif /* !_BINSTR_H */
