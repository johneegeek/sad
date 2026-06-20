/*******************************************************************************
 * SAD - Options Module Header
 *
 * Command-line option processing and configuration
 *
 * Copyright (c) 1997-2026 John Kiernan
 * Licensed under MIT License - see LICENSE file for details
 ******************************************************************************/
#ifndef _OPTIONS_H
#define _OPTIONS_H

#include <stdbool.h>

/*****************************************************************************\
 Function: process_options()

 Description: Processes command-line options and sets global configuration
              variables accordingly

 Parameters:
   argc - Argument count
   argv - Argument vector

 Returns: true on success, false on failure (invalid options)
\*****************************************************************************/
bool process_options(int argc, char** argv);

#endif /* !_OPTIONS_H */
