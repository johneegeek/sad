/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 Sad Header File
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef _SAD_H
#define _SAD_H

#include "platform.h"

static const char B_BEGIN = '<';
static const char B_END = '>';
static const char E_BEGIN = '*';
static const char E_END = '*';
static const char DUP_CHAR = '*';

/* External declarations for control character tables (defined in sad.c) */
extern char *CtrlTable[];
extern char *CCTable[];
extern char *H_CCTable[];

#define CH_DEL   0x7F
#define CH_NL    0x0A
#define CH_SPACE 0x20

#define TMP_OUTFILE "_sad_"

#define MAXSTRING 1024
#define MAX_BIN_STRING 65

#define DEFAULT_COLUMNS_HEX   16
#define DEFAULT_COLUMNS_2HEX  8
#define DEFAULT_COLUMNS_4HEX  6
#define DEFAULT_COLUMNS_8HEX  3

#define DEFAULT_COLUMNS_DEC   10
#define DEFAULT_COLUMNS_2DEC  8
#define DEFAULT_COLUMNS_4DEC  4
#define DEFAULT_COLUMNS_8DEC  2

#define DEFAULT_COLUMNS_OCT   8
#define DEFAULT_COLUMNS_2OCT  8
#define DEFAULT_COLUMNS_4OCT  4
#define DEFAULT_COLUMNS_8OCT  2

#define DEFAULT_COLUMNS_SND   10
#define DEFAULT_COLUMNS_2SND  8
#define DEFAULT_COLUMNS_4SND  4
#define DEFAULT_COLUMNS_8SND  2

#define DEFAULT_COLUMNS_BIN   6
#define DEFAULT_COLUMNS_2BIN  3
#define DEFAULT_COLUMNS_4BIN  2
#define DEFAULT_COLUMNS_8BIN  1

#define DEFAULT_COLUMNS_ASCII 10

#define DEFAULT_COLUMNS_FLOAT  4
#define DEFAULT_COLUMNS_DOUBLE 3

#define DEFAULT_WIDTH 1

/* Modes */
#define DUMP_MODE_HEX       0x0
#define DUMP_MODE_ASCII     0x1
#define DUMP_MODE_FASCII    0x2
#define DUMP_MODE_CTRL      0x3
#define DUMP_MODE_DEC       0x4
#define DUMP_MODE_OCT       0x5
#define DUMP_MODE_SIGNED    0x6
#define DUMP_MODE_FLOAT     0x7
#define DUMP_MODE_DOUBLE    0x8
#define DUMP_MODE_BINARY    0x9
#define DUMP_MODE_UNKNOWN   0xFFFF


#define OFFSET_HEX  0x1
#define OFFSET_DEC  0x2
#define OFFSET_OCT  0x3

#if !defined(__GNUC__)
    #define popen _popen
    #define pclose _pclose
#endif

#define FILE_EXISTS(f) (_access(f,0)==0)

#define BYTE_SWAP_32(data) \
((((data) & 0x000000ff) << 24) | (((data) & 0x0000ff00) << 8) | (((data) & 0x00ff0000) >> 8) | (((data) & 0xff000000) >> 24))

#define BYTE_SWAP_16(data) \
   ((((data) & 0x00FF) << 8) | (((data) & 0xFF00) >> 8))

#ifdef __GNUC__
#define BYTE_SWAP_64(data) \
((((data)&0x00000000000000ffLL)<<56)|(((data)&0x000000000000ff00LL)<<40)|(((data)&0x0000000000ff0000LL)<< 24)|(((data)&0x00000000ff000000LL)<< 8)| \
(((data)&0x000000ff00000000LL)>>8)|(((data)&0x0000ff0000000000LL)>>24)|(((data)&0x00ff000000000000LL)>>40)|(((data)&0xff00000000000000LL)>>56))
#else
#define BYTE_SWAP_64(data) \
((((data)&0x00000000000000ff)<<56)|(((data)&0x000000000000ff00)<<40)|(((data)&0x0000000000ff0000)<< 24)|(((data)&0x00000000ff000000)<< 8)| \
(((data)&0x000000ff00000000)>>8)|(((data)&0x0000ff0000000000)>>24)|(((data)&0x00ff000000000000)>>40)|(((data)&0xff00000000000000)>>56))
#endif


#define _MAC_STR(z) _MAC_TMP(z)
#define _MAC_TMP(z) #z


#endif /* !_SAD_H */
