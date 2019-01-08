/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 creates string representation of binary from and integer
 $Id: binstr.h 4 2006-10-26 21:27:09Z jkiernan $
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef _BINSTR_H
#define _BINSTR_H

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
extern char *binstr( uint64 value, void *buffer, size_t bytes );



#endif /* !_BINSTR_H */
