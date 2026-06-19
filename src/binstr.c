/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 # Displays numbers as "binary" strings
 # Module: binstr.c
 # $Id: binstr.c 4 2006-10-26 21:27:09Z jkiernan $
 #
 # Created: 12/28/1999
 #
 # History:
 #
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include "jkcommon.h"
                 
                        
#define BS_BITS_PER_BYTE    8

/*local prototype*/

char *binstr( uint64 value, void *buffer, size_t bytes );


/* implementation */

/*****************************************************************************\
 Function: binstr()

 Description: converts integer to string representation of binary

 Returns:
   binary string of value.
\*****************************************************************************/

char *binstr( uint64 value, void *buffer, size_t bytes )
{
    BYTE        *p; /* position pointer of buffer */
    BYTE        bits;
    register int n;

    bits = (BYTE)(bytes * BS_BITS_PER_BYTE);

    if ( bytes > sizeof(value) ) /* more bytes than value has! */
        return(NULL);

    p = (BYTE *)buffer + bits;
    *p-- = 0;

    for( n=0; n < bits; n++ )
        *p-- = (BYTE)((value & ((uint64)1 << n)) ? '1' : '0');

    return( buffer );

} /* binstr() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


#ifdef _TEST
int main( void )
{
    int     x;
    char    *bin_string[33];

    printf("binstr test:\n");
    for ( x=0; x<256; x++)
    {
        printf("(%3d) : %s\n", x, binstr(x, bin_string, 2) );
    } /* for */

    return(0);
}
#endif
