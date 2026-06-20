/*******************************************************************************
 * SAD - Options Module Header
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
