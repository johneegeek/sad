/*******************************************************************************
 * SAD - Utils Module Header
 *
 * General utility functions used throughout the application
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/
#ifndef _UTILS_H
#define _UTILS_H

#include <stdbool.h>
#include <stdio.h>

/*****************************************************************************\
 Function: file_is_bin()

 Description: Determines if a file is binary or text by examining its contents

 Parameters:
   filename_sz - Path to the file to check

 Returns: true if file appears to be binary, false if it appears to be text
\*****************************************************************************/
bool file_is_bin(char* filename_sz);

/*****************************************************************************\
 Function: get_input()

 Description: Gets input characters from a file pointer, respecting global
              byte count limits (data_end_count). Returns EOF when the
              maximum byte count is reached.

 Parameters:
   fp - File pointer to read from

 Returns: Character read, or EOF
\*****************************************************************************/
int get_input(FILE* fp);

#endif /* !_UTILS_H */
