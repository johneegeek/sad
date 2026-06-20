/*******************************************************************************
 * SAD - Binary String Module
 *
 * Binary string representation utilities for converting integers to binary
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
#include "jkcommon.h"
#include "platform.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BS_BITS_PER_BYTE 8

/*local prototype*/

char* binstr(uint64_t value, void* buffer, size_t bytes);

/* implementation */

/*****************************************************************************\
 Function: binstr()

 Description: converts integer to string representation of binary

 Returns:
   binary string of value.
\*****************************************************************************/

char* binstr(uint64_t value, void* buffer, size_t bytes)
{
    BYTE*        p; /* position pointer of buffer */
    BYTE         bits;
    register int n;

    bits = (BYTE)(bytes * BS_BITS_PER_BYTE);

    if (bytes > sizeof(value)) /* more bytes than value has! */
        return (NULL);

    p    = (BYTE*)buffer + bits;
    *p-- = 0;

    for (n = 0; n < bits; n++) *p-- = (BYTE)((value & ((uint64_t)1 << n)) ? '1' : '0');

    return (buffer);

} /* binstr() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#ifdef _TEST
int main(void)
{
    int   x;
    char* bin_string[33];

    printf("binstr test:\n");
    for (x = 0; x < 256; x++) {
        printf("(%3d) : %s\n", x, binstr(x, bin_string, 2));
    } /* for */

    return (0);
}
#endif
