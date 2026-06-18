/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 # Slicers Awesome Dump
 # Module: sad.c
 #
 # History:
 #    2.5.4  Added configuration for WT (if ST32 does not exist)
 #    2.5.3  Updated/fixed a few compile warnings.
 #    2.5.2  Changed to use CMake and build on Mac and Linux.
 #    2.5.1  Change -6r compile switch to -4r for AMD/Win64 compatability. -JK
 #    2.5.0  Added --offset-divisor option to change offset numbers. - JK
 #    2.4.0  Added alternate text processing of numeric parameters. (i.e. Hex, Oct, etc) -JK
 #    2.3.1  Fixed up the code a little to allow compulation on Linux -JK
 #    2.3.0  Added --show-spaces option. (used only for --ascii) -JK
 #    2.2.3  Fixed --signed output. (was not really signed!) -JK
 #    2.2.2  Fixed FileIsBin() function, always returned false :(   -JK
 #    2.2.1  Fixed some lint errors.  -JK
 #    2.2.0  Added --float-width and --float-precision options
 #    2.1.2  Fixed 64bit binary text bug. -JK
 #    2.1.1  Added read options from cfg and/or env. -JK
 #    2.1.0  Added --binary option. -JK
 #    2.0.11 Changed floating point rounding scheme. -JK
 #    2.0.10 Fixed floating point and double error.  Incorrectly calcuted pointer sizes and displayed
 #           wrong values.
 #    2.0.9  Fixed -k option on float and double -JK
 #    2.0.8  Help file had options -F and -H reversed.
 #    2.0.7  Fixed duplicate error where first dup line was not found. -JK
 #    2.0.6  Spelling fixes.
 #    2.0.5  Fixed stdin open error. Now realy does open in binary. -JK
 #    2.0.4  Added suppport for starting offset and or count. -JK
 #    2.0.3  Added support for partial dumps (--count) option. -JK
 #    2.0.2  Fixed offset printing (default is always hex) option to show in
 #           other formats -JK
 #    2.0.1  Added float and signed support. -JK
 #
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/****************/
/* Lint Options */
/****************/
/*lint -save*/
/*lint +fll   Allow long long*/
/*lint -e557  unrecognized format*/
/*lint -e716  Allow while(1) infinte loop */
/*lint -e774  Allow while loop to always be true*/
/*lint -e661*/

#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__DJGPP__) || defined(__UNIXLIKE__)
  #include <errno.h>
  #define _access access
#endif

#if defined(__BORLANDC__) || defined(_MSC_VER)
#  include <io.h>
#else
#  include <unistd.h>
#endif
#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>

#include "jkcommon.h"
#include "getopt.h"
#include "sad.h"
#include "version.h"
#include "dprintf.h"
#include "binstr.h"
#include "makeargs.h"

#ifdef __DOS16__
char system_command[MAXSTRING];
#endif


/*globals and options*/
uint16 dump_mode;
char   pager_program[MAXSTRING];
FILE   *output_file;
FILE   *input_file;
int    columns;
int    data_width;
int32  data_end_count;
int32  data_start_count;
#ifdef DEBUG
bool   print_debug_messages;
#endif
bool   use_pager;
bool   show_offset;
bool   show_ascii;
bool   highlight;
bool   no_duplicate;
bool   show_eof_marker;
bool   big_endian;
bool   mode_selected;
bool   width_selected;
bool   columns_selected;
bool   show_spaces;
int    offset_display;
uint32_t offset_divisor = 1;

int    float_width = 0;
int    float_precision = 0;

static char *pname;

/*local prototypes*/
static bool dump_file(FILE *infile_ptr, FILE *outfile_ptr);
static bool dump_file_ascii( FILE *ifile, FILE *ofile );
static bool dump_file_asciifixed( FILE *ifile, FILE *ofile, uint32 cur_offset );
static bool dump_file_binary( FILE *ifile, FILE *ofile, uint32 offset);
static bool dump_file_control( FILE *ifile, FILE *ofile );
static bool dump_file_dec( FILE *ifile, FILE *ofile, uint32 offset);
static bool dump_file_hex( FILE *ifile, FILE *ofile, uint32 offset);
static bool dump_file_oct( FILE *ifile, FILE *ofile, uint32 offset);
static bool dump_file_signed( FILE *ifile, FILE *ofile, uint32 offset);
static bool dump_file_float( FILE *ifile, FILE *ofile, int abs_data_width, uint32 offset);
static bool file_is_bin( char *filename_sz );
static int  get_input( FILE *fp );
static void print_offset( FILE *outfp, uint32 offvalue );
static bool process_options(int argc, char **argv);
static bool write_fmtline_bin(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof);
static bool write_fmtline_dec(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof );
static bool write_fmtline_hex(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof);
static bool write_fmtline_oct(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof);
static bool write_fmtline_signed(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof);
static bool write_fmtline_float(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof);
static void show_help( void );
static void show_version( void );

uint64_t string_to_uint64(const char *val, bool *valid);

int main( int argc, char **argv )
{
    char    *p;
    int     index;
    int     ex_argc;
    char    **ex_argv;
    char    cfg_file[MAXSTRING];

    /* strip off everything for program name */
    pname = argv[0];
    p = pname;
    while ((bool)*++p) *p = (char)tolower(*p);
    #if defined(__DOSLIKE__)
    p=strchr(pname,'.');
    #endif
    if (p!=NULL) *p= (char)0;
    #ifndef _UNIXSTYLE_PATHS
        while((p=strchr(pname,'\\')) != 0)pname=p+1;
    #else
        while((p=strchr(pname,'/'))  != 0)pname=p+1;
    #endif

    /******************\
       Setup defaults
    \******************/
    #if defined(DEBUG)
        print_debug_messages = true;
    #endif
    dump_mode = DUMP_MODE_ASCII;
    use_pager = false;
    input_file = stdin;
    columns = DEFAULT_COLUMNS_HEX;
    data_width = DEFAULT_WIDTH;
    show_offset = true;
    show_ascii = true;
    highlight = true;
    no_duplicate = false;
    show_eof_marker = true;
    big_endian = false;
    mode_selected = false;
    width_selected = false;
    columns_selected = false;
    show_spaces = false;
    offset_display = OFFSET_HEX;
    data_end_count = 0;
    data_start_count = 0;


    /* Get pager from environment */
    p = getenv("PAGER");
    if (p == NULL)
    {
        #if defined(__DOSLIKE__)
            strcpy(pager_program, "more");
        #else
            strcpy(pager_program, "less");
        #endif
    }
    else
        strcpy(pager_program, p);


    /*********************************************/
    /* Get options from config file if it exists */
    /*********************************************/
    dbprintf("%s: debug: checking for options...\n", pname);

    cfg_file[0] = 0;
    p = getenv("HOME");
    if (p != NULL)
    {
        dbprintf("%s: debug: `HOME' variable found, checking for config\n", pname);
        #if !defined(_UNIXSTYLE_PATHS)
            sprintf(cfg_file, "%s\\sad.cfg", p );
        #else
            sprintf(cfg_file, "%s/sad.cfg", p );
        #endif
        if (!FILE_EXISTS(cfg_file))
            cfg_file[0] = 0;
    } /* if (p != NULL) */

    /* First check for WTDIR variable */
    if (strlen(cfg_file) == 0)
    {
        p = getenv("WTDIR");
        if (p != NULL)
        {
            dbprintf("%s: debug: `WDIR' variable found, checking for config\n", pname);
            #if !defined(_UNIXSTYLE_PATHS)
            sprintf(cfg_file, "%s\\cfg\\sad.cfg", p );
            #else
            sprintf(cfg_file, "%s/cfg/sad.cfg", p );
            #endif
            if (!FILE_EXISTS(cfg_file))
                cfg_file[0] = 0;
        } /* if (p != NULL) */
    } /* else */

    if (strlen(cfg_file) == 0)
    {
        p = getenv("ST32");
        if (p != NULL)
        {
            dbprintf("%s: debug: `ST32' variable found, checking for config\n", pname);
            #if !defined(_UNIXSTYLE_PATHS)
            sprintf(cfg_file, "%s\\config\\sad.cfg", p );
            #else
            sprintf(cfg_file, "%s/config/sad.cfg", p );
            #endif
            if (!FILE_EXISTS(cfg_file))
                cfg_file[0] = 0;
        } /* if (p != NULL) */
    } /* else */

    if (strlen(cfg_file) > 0)
    {
        dbprintf("%s: debug: `%s' found, getting arguments...\n", pname, cfg_file);
        ex_argc = fmakeargs(cfg_file, &ex_argv);
        if (ex_argc != 0)
        {
            if (!process_options(ex_argc, ex_argv)) return(1);
        } /* if (ex_argc != 0) */
        else
            dbprintf("%s: debug: no cfg arguments found.\n", pname);
        free_makeargs(ex_argc, ex_argv);
    } /* if (strlen(cfg_file) > 0) */
    else
        dbprintf("%s: debug: no cfg file found.\n", pname);


    /*********************************************/
    /* Get options from ENV variable if it exits */
    /*********************************************/
    p = getenv(pname);
    if (p != NULL)
    {
        dbprintf("%s: debug: `%s' env found, getting arguments...\n", pname, pname);
        ex_argc = env_makeargs(pname, &ex_argv);
        if (ex_argc != 0)
            if (!process_options(ex_argc, ex_argv)) return(1);
        free_makeargs(ex_argc, ex_argv);
    } /* if (p != NULL) */
    else
        dbprintf("%s: debug: no env var found.\n", pname);

    /*********************************************/
    /* Process the command line options */
    /*********************************************/
    if (!process_options(argc, argv))
        return(1);


    if (columns == 0)
    {
        fprintf(stderr, "%s: error: `columns' can not be zero.\n", pname);
        return(1);
    } /* if (columns == 0) */

    if (data_width == 0)
    {
        fprintf(stderr, "%s: error: `width' can not be zero.\n", pname);
        return(1);
    } /* if (columns == 0) */


    /* In some cases a dumb person could specify a width with */
    /* a float or double... We want to override any width selections */
    switch (dump_mode)
    {
        case DUMP_MODE_FLOAT:
            data_width = sizeof(float);
            break;
        case DUMP_MODE_DOUBLE:
            data_width = sizeof(double);
            break;
        default:
            break;
    } /* switch (dump_mode) */

#if !defined(__DOS16__)
    /* determine output */
    if (use_pager)
    {
        output_file = popen(pager_program, "w");
        if (output_file == NULL)
        {
            perror("pipe error");
            exit(1);
        } /* if (output_file == NULL) */
    } /* if (use_pager) */
    else
    {
        output_file = stdout;
    } /* else */
#else
    if (use_pager)
    {
            output_file = fopen( TMP_OUTFILE, "w" );
            if ( output_file == NULL )
            {
                perror("Error opening TMP_OUTFILE");
                exit(1);
            }

    } /* if (use_pager) */
    else
        output_file = stdout;
#endif


    dbprintf("!!debug: dump mode is %#x\n", dump_mode );
    if (optind == argc)
    {
        /* Use STDIN */
        dbprintf("!!debug: No non-opt args. (use stdin)\n");
        input_file = stdin;
        if ( isatty(fileno(input_file)) )
        {
            #if defined(__DOSLIKE__)
                fprintf(stderr, "Reading from stdin ...(use ^C to exit)\n");
            #else
                fprintf(stderr, "Reading from stdin ...(use ^D to exit)\n");
            #endif
        } /* if ( isatty(fileno(input_file)) ) */

        #if defined(__QNX__) || defined(__UNIXLIKE__)
          setvbuf(input_file, NULL, _IONBF, 0);
        #else
           (void)setmode(fileno(stdin), O_BINARY);
        #endif
        (void) dump_file(input_file, output_file);


    } /* if (optind == argc) */
    else
    {
        for (index = optind; index < argc; index++)
        {
            dbprintf("!!debug: Non-option argument %s\n", argv[index]);
            dbprintf("!!debug: mode_selected = %d\n", mode_selected);
            if (!mode_selected)
            {
                dbprintf("!!debug: No mode selected, trying to determine mode.\n");
                if (file_is_bin(argv[index]))
                {
                    dump_mode = DUMP_MODE_HEX;
                    dbprintf("!!debug: file is binary, dump mode is now %#x\n", dump_mode );
                } /* if (file_is_bin(argv[index])) */
            }
            /* open the file */
            input_file = fopen(argv[index], "rb");
            if (NULL == input_file)
            {
                fprintf(stderr, "%s: error opening input file <%s> : %s\n", pname, argv[index], strerror(errno));
                break;
            } /* if (NULL == input_file) */
            dbprintf("!!debug: dump mode is %#x\n", dump_mode );

            if (!dump_file(input_file, output_file))
            {
                fprintf(stderr, "%s: error processing file <%s>\n", pname, argv[index]);
                (void)fclose(input_file);
                break;
            } /* if (!dump_file(input_file, output_file)) */

            (void)fclose(input_file);

        } /* for (index = optind; index < argc; index++) */
    } /* else */

    #if !defined(__DOS16__)
        if (use_pager)
            (void)pclose(output_file);
    #else
        if (use_pager)
        {
            fclose( output_file );
            sprintf(system_command, "%s %s", pager_program, TMP_OUTFILE );
            system(system_command);
            remove( TMP_OUTFILE );
        } /* if (use_pager) */
    #endif

    return(0);

} /* int main( int argc, char **argv ) */



/*****************************************************************************\
 Function: dump_file()

 Description: Dumps a file
    Files must be opened before calling dump file, and pointers must be valid!

 Returns:
\*****************************************************************************/
bool dump_file(FILE *infile_ptr, FILE *outfile_ptr)
{
    uint32 dump_file_offset;

    dump_file_offset = 0;

    if (data_start_count != 0)
    {
        /* ignore everything to the starting count */
        int64  total_bytes;

        total_bytes = data_start_count * data_width;
        while (dump_file_offset < total_bytes)
        {
            /*lint -esym(550,junk)*/
            int junk;
            if ((junk = fgetc(infile_ptr)) == EOF) return true;
            ++dump_file_offset;
        } /* while (current_total < total_bytes) */

    } /* if (data_start_count != 0) */


    switch(dump_mode)
    {
        case DUMP_MODE_HEX:
            return (dump_file_hex(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_ASCII:
            return (dump_file_ascii(infile_ptr, outfile_ptr));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_FASCII:
            return (dump_file_asciifixed(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_CTRL:
            return (dump_file_control(infile_ptr, outfile_ptr));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_DEC:
            return (dump_file_dec(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_OCT:
            return (dump_file_oct(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_SIGNED:
            return (dump_file_signed(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_FLOAT:
            return (dump_file_float(infile_ptr, outfile_ptr, sizeof(float), dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_DOUBLE:
            return (dump_file_float(infile_ptr, outfile_ptr, sizeof(double), dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_BINARY:
            return (dump_file_binary(infile_ptr, outfile_ptr, dump_file_offset));
            /*NOTREACHED*/
            break;

        case DUMP_MODE_UNKNOWN:
            /*FALLTHRU*/
        default:
            fprintf(stderr, "%s: dump error: unknown mode <%#x>.\n", pname, dump_mode);
            return false;
            /*NOTREACHED*/
            break;
    } /* switch(dump_mode) */


    /*NOTREACHED*/
    return true;

} /* bool dump_file(FILE *infile_ptr, FILE *outfile_ptr) */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


bool dump_file_ascii( FILE *ifile, FILE *ofile )
{
    /* pointers to files must be opened and valid */

    int           in_char = 0;

    while ( in_char != EOF )
    {
        if ( (in_char = get_input( ifile )) == EOF )
        {
            if (show_eof_marker)
                fprintf(ofile, "%ceof%c", E_BEGIN, E_END);
            fflush(ofile);
            continue;
        }

        if ( in_char < (int)(sizeof( CtrlTable) / sizeof(char*)) )
        {
            /* in_char is in CtrlTable */
            fprintf(ofile, "%c%s%c", B_BEGIN, CtrlTable[ in_char ], B_END);
            if ( in_char == CH_NL )
                fprintf(ofile,"\n");
            fflush(ofile);
            continue;
        }

        if ( in_char == CH_DEL )
        {
            fprintf(ofile, "%cdel%c", B_BEGIN, B_END);
            continue;
        }

        if (!isprint( in_char ) )
        {
            fprintf(ofile, "%c0x%02X%c", B_BEGIN, in_char, B_END );
        }
        else
        {
            if ((in_char == CH_SPACE) && show_spaces)
                putc('\xB7',ofile);
            else
                if (highlight)
                    fprintf( ofile, "%c\b%c", in_char, in_char );
                else
                    putc( in_char, ofile );

        }

    }

    return true;
}


bool dump_file_asciifixed( FILE *ifile, FILE *ofile, uint32 cur_offset )
{
    /* pointers to files must be opened and valid */

    int       in_char = 0;
    int       ascii_count = 0;

    while ( in_char != EOF )
    {
        if ( ascii_count == DEFAULT_COLUMNS_ASCII )
        {
            fprintf(ofile,"\n");
            ascii_count = 0;
        }

        if ( (in_char = get_input( ifile )) == EOF )
        {
            if (show_eof_marker)
                fprintf(ofile, "%ceof%c ", E_BEGIN, E_END);
            fflush(ofile);
            continue;
        }

        if ( ascii_count == 0 )
        {
            if (show_offset) print_offset(ofile, cur_offset);
            cur_offset += DEFAULT_COLUMNS_ASCII;
        }

        ++ascii_count;

        if ( in_char < (int)(sizeof( CtrlTable) / sizeof(char*)) )
        {
            /* in_char is in CtrlTable */
            fprintf(ofile, "%c%3s%c ", B_BEGIN, CtrlTable[ in_char ], B_END);
            fflush(ofile);
            continue;
        }

        if ( in_char == CH_DEL )
        {
            fprintf(ofile, "%cdel%c ", B_BEGIN, B_END);
            continue;
        }

        if (!isprint( in_char ) )
        {
            fprintf(ofile, "%c0x%02X%c", B_BEGIN, in_char, B_END );
        }
        else
        {
            if (highlight)
                fprintf( ofile, "[  %c\b%c ]", in_char, in_char );
            else
                fprintf(ofile, "[  %c ]", in_char );
        }
    }

    return true;

}

/*****************************************************************************\
 Function: dump_file_binary()

 Description: Dumps a file in binary

 Returns:
\*****************************************************************************/
bool dump_file_binary( FILE *ifile, FILE *ofile, uint32 offset )
{
    int     in_char;
    char    *linebuf;
    char    *lastlinebuf;
    uint32  data_per_line;
    uint32  line_buf_size;
    uint32  data_count;
    bool    first_line;
    bool    duplicate;
    bool    eof_flag;


    if (!columns_selected)
    {
        switch (data_width)
        {
            case 1:
                columns = DEFAULT_COLUMNS_BIN;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2BIN;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4BIN;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8BIN;
                break;
            default:
                columns = DEFAULT_COLUMNS_BIN;
                break;
        }
    }

    first_line = true;
    duplicate = false;
    eof_flag = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;



    linebuf = (char *)malloc(line_buf_size);
    if (linebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char *)malloc(line_buf_size);
    if (lastlinebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */


    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1)
    {
        /*fill line buffer*/

        in_char = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;

        if (eof_flag || (data_count == (data_per_line-1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line)
            {
                if (!write_fmtline_bin(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else
            {
                if (!no_duplicate && (0 == memcmp(linebuf,lastlinebuf, line_buf_size)))
                {
                    if (!duplicate)
                    {
                        fprintf(ofile,"%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else
                {
                    duplicate = false;
                    if (!write_fmtline_bin(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count+1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF)
            break;

    } /* while (1) */

    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_binary() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


bool dump_file_control( FILE *ifile, FILE *ofile )
{
    /* pointers to files must be opened and valid */

    int       in_char = 0;


    while ( in_char != EOF )
    {
        if ( (in_char = get_input( ifile )) == EOF )
        {
            if (show_eof_marker)
                fprintf(ofile, "%ceof%c", E_BEGIN, E_END );
            fflush( ofile );
            continue;
        }

        if ( in_char < (int)(sizeof( CCTable) / sizeof(char*)) )
        {
            /* in_char is in CtrlTable */
            fprintf(ofile, "%c%s%c", B_BEGIN, CCTable[ in_char ], B_END );
            if ( in_char == CH_NL )
                fprintf(ofile,"\n");
            fflush(ofile);
            continue;
        }

        if ( in_char == CH_DEL )
        {
            fprintf(ofile, "%cdel%c", B_BEGIN, B_END );
            continue;
        }

        if (!isprint( in_char ) )
        {
            fprintf(ofile, "%c0x%02X%c", B_BEGIN, in_char, B_END );
        }
        else
        {
            if (highlight)
                fprintf( ofile, "%c\b%c", in_char, in_char );
            else
                putc( in_char, ofile );
        }
    }

    fflush( ofile );
    return true;
}



bool dump_file_dec( FILE *ifile, FILE *ofile, uint32 offset )
{
    int     in_char;
    char    *linebuf;
    char    *lastlinebuf;
    uint32  data_per_line;
    uint32  line_buf_size;
    uint32  data_count;
    bool    first_line;
    bool    duplicate;
    bool    eof_flag;


    if (!columns_selected)
    {
        switch (data_width)
        {
            case 1:
                columns = DEFAULT_COLUMNS_DEC;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2DEC;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4DEC;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8DEC;
                break;
            default:
                columns = DEFAULT_COLUMNS_DEC;
                break;
        }
    }

    first_line = true;
    duplicate = false;
    eof_flag = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;



    linebuf = (char *)malloc(line_buf_size);
    if (linebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char *)malloc(line_buf_size);
    if (lastlinebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */


    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1)
    {
        /*fill line buffer*/

        in_char = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;



        if (eof_flag || (data_count == (data_per_line-1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line)
            {
                if (!write_fmtline_dec(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else
            {
                if (!no_duplicate && (0 == memcmp(linebuf,lastlinebuf, line_buf_size)))
                {
                    if (!duplicate)
                    {
                        fprintf(ofile,"%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else
                {
                    duplicate = false;
                    if (!write_fmtline_dec(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count+1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF)
            break;

    } /* while (1) */



    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_dec() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


bool dump_file_float( FILE *ifile, FILE *ofile, int abs_data_width, uint32 offset )
{
    int     in_char;
    char    *linebuf;
    char    *lastlinebuf;
    uint32  data_per_line;
    uint32  line_buf_size;
    uint32  data_count;
    bool    first_line;
    bool    duplicate;
    bool    eof_flag;

    if (!columns_selected)
    {
        switch (abs_data_width)
        {
            case sizeof(float):
                columns = DEFAULT_COLUMNS_FLOAT;
                break;
            case sizeof(double):
                columns = DEFAULT_COLUMNS_DOUBLE;
                break;
            default:
                fprintf(stderr, "%s: unsupported data with for type float - %d\n", pname, abs_data_width);
                break;
        } /* switch (abs_data_width) */
    } /* if (!columns_selected) */

    first_line = true;
    duplicate = false;
    eof_flag = false;
    line_buf_size = (unsigned)columns * (unsigned)abs_data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)abs_data_width;



    linebuf = (char *)malloc(line_buf_size);
    if (linebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char *)malloc(line_buf_size);
    if (lastlinebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */


    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1)
    {
        /*fill line buffer*/

        in_char = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;



        if (eof_flag || (data_count == (data_per_line-1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line)
            {
                if (!write_fmtline_float(ofile, offset, linebuf, data_count+1, abs_data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else
            {
                if (!no_duplicate && (0 == memcmp(linebuf,lastlinebuf, line_buf_size)))
                {
                    if (!duplicate)
                    {
                        fprintf(ofile,"%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else
                {
                    duplicate = false;
                    if (!write_fmtline_float(ofile, offset, linebuf, data_count+1, abs_data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count+1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF)
            break;

    } /* while (1) */



    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_float() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


/*****************************************************************************\
 Function: dump_file_hex()

 Description: Dumps a file in hex

 Returns:
\*****************************************************************************/
bool dump_file_hex( FILE *ifile, FILE *ofile, uint32 offset )
{
    int     in_char;
    char    *linebuf;
    char    *lastlinebuf;
    uint32  data_per_line;
    uint32  line_buf_size;
    uint32  data_count;
    bool    first_line;
    bool    duplicate;
    bool    eof_flag;


    if (!columns_selected)
    {
        switch (data_width)
        {
            case 1:
                columns = DEFAULT_COLUMNS_HEX;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2HEX;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4HEX;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8HEX;
                break;
            default:
                columns = DEFAULT_COLUMNS_HEX;
                break;
        }
    }

    first_line = true;
    duplicate = false;
    eof_flag = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;



    linebuf = (char *)malloc(line_buf_size);
    if (linebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char *)malloc(line_buf_size);
    if (lastlinebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */


    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1)
    {
        /*fill line buffer*/

        in_char = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;



        if (eof_flag || (data_count == (data_per_line-1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line)
            {
                if (!write_fmtline_hex(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else
            {
                if (!no_duplicate && (0 == memcmp(linebuf,lastlinebuf, line_buf_size)))
                {
                    if (!duplicate)
                    {
                        fprintf(ofile,"%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else
                {
                    duplicate = false;
                    if (!write_fmtline_hex(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count+1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF)
            break;

    } /* while (1) */



    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_hex() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


bool dump_file_oct( FILE *ifile, FILE *ofile, uint32 offset )
{
    int     in_char;
    char    *linebuf;
    char    *lastlinebuf;
    uint32  data_per_line;
    uint32  line_buf_size;
    uint32  data_count;
    bool    first_line;
    bool    duplicate;
    bool    eof_flag;


    if (!columns_selected)
    {
        switch (data_width)
        {
            case 1:
                columns = DEFAULT_COLUMNS_OCT;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2OCT;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4OCT;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8OCT;
                break;
            default:
                columns = DEFAULT_COLUMNS_OCT;
                break;
        }
    }

    first_line = true;
    duplicate = false;
    eof_flag = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;



    linebuf = (char *)malloc(line_buf_size);
    if (linebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char *)malloc(line_buf_size);
    if (lastlinebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */


    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1)
    {
        /*fill line buffer*/

        in_char = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;



        if (eof_flag || (data_count == (data_per_line-1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line)
            {
                if (!write_fmtline_oct(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else
            {
                if (!no_duplicate && (0 == memcmp(linebuf,lastlinebuf, line_buf_size)))
                {
                    if (!duplicate)
                    {
                        fprintf(ofile,"%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else
                {
                    duplicate = false;
                    if (!write_fmtline_oct(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count+1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF)
            break;

    } /* while (1) */



    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_oct() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


bool dump_file_signed( FILE *ifile, FILE *ofile, uint32 offset )
{
    int     in_char;
    char    *linebuf;
    char    *lastlinebuf;
    uint32  data_per_line;
    uint32  line_buf_size;
    uint32  data_count;
    bool    first_line;
    bool    duplicate;
    bool    eof_flag;


    if (!columns_selected)
    {
        switch (data_width)
        {
            case 1:
                columns = DEFAULT_COLUMNS_SND;
                break;
            case 2:
                columns = DEFAULT_COLUMNS_2SND;
                break;
            case 4:
                columns = DEFAULT_COLUMNS_4SND;
                break;
            case 8:
                columns = DEFAULT_COLUMNS_8SND;
                break;
            default:
                columns = DEFAULT_COLUMNS_SND;
                break;
        }
    }

    first_line = true;
    duplicate = false;
    eof_flag = false;
    line_buf_size = (unsigned)columns * (unsigned)data_width * sizeof(in_char);
    data_per_line = (unsigned)columns * (unsigned)data_width;



    linebuf = (char *)malloc(line_buf_size);
    if (linebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        return false;
    } /* if (linebuf == NULL) */
    lastlinebuf = (char *)malloc(line_buf_size);
    if (lastlinebuf == NULL)
    {
        fprintf(stderr, "%s: memory allocation error: module %s, line %d\n", pname, __FILE__, __LINE__);
        free(linebuf);
        return false;
    } /* if (linebuf == NULL) */


    memset(linebuf, 0x0, line_buf_size);
    memset(lastlinebuf, 0x0, line_buf_size);

    data_count = 0;

    while (1)
    {
        /*fill line buffer*/

        in_char = get_input(ifile);
        eof_flag = (bool)(in_char == EOF);
        if ((data_count < data_per_line) && !eof_flag)
            linebuf[data_count] = (char)in_char;



        if (eof_flag || (data_count == (data_per_line-1))) /* done with line or completely done */
        {
            /* check for duplicate line */
            if (first_line)
            {
                if (!write_fmtline_signed(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                    break;
                first_line = false;
            } /* if (first_line) */
            else
            {
                if (!no_duplicate && (0 == memcmp(linebuf,lastlinebuf, line_buf_size)))
                {
                    if (!duplicate)
                    {
                        fprintf(ofile,"%c\n", DUP_CHAR);
                        duplicate = true;
                    } /* if (!duplicate) */

                } /* if (0 == memcmp(linebuf,lastlinebuf, data_per_line)) */
                else
                {
                    duplicate = false;
                    if (!write_fmtline_signed(ofile, offset, linebuf, data_count+1, data_width, eof_flag))
                        break;
                } /* else */

            } /* if (!duplicate && !first_line) */

            memcpy(lastlinebuf, linebuf, line_buf_size);
            memset(linebuf, 0x0, line_buf_size);
            offset += (data_count+1);
            data_count = 0;
        } /* if ((in_char == EOF) || (data_count == (data_per_line-1))) */
        else
            ++data_count;

        if (in_char == EOF)
            break;

    } /* while (1) */



    free(linebuf);
    free(lastlinebuf);
    return true;

} /* dump_file_signed() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


bool file_is_bin( char *filename_sz )
{
    FILE    *posbin_fl;
    bool    retflag_f = false;
    int     iChIn;

    posbin_fl = fopen( filename_sz, "r" );
    if ( posbin_fl == NULL )
        return( false );

    while( (iChIn = fgetc(posbin_fl)) != EOF )
    {
        if ( iChIn < 0x7 || iChIn > 0x7E)
        {
            retflag_f = true;
            break;
        }
    }

    fclose( posbin_fl );
    return( retflag_f );
}



/*****************************************************************************\
 Function: get_input()

 Description: Gets input chars from "fp", used to control amount read in.
 Use global byte counts to control how much is read in.
 EOF is returned when the maximum byte count is hit.  If EOF is returned before
 actual EOF we need to make sure the EOF marker is not displayed, so
 show_eof_marker should be set to false.

 Returns:
\*****************************************************************************/
int get_input( FILE *fp )
{
    static uint64 char_count = 0;

    if (data_end_count == 0)
    {
        return(fgetc(fp));
    } /* if (data_count == 0) */
    else
    {
        int input_char;

        input_char = fgetc(fp);
        ++char_count;
        if (char_count <= (uint64)((unsigned)data_end_count * (unsigned)data_width))
        {
            return(input_char);
        } /* if (char_count < (data_count * data_width)) */
        else
        {
            if (input_char != EOF) /* we have all the requested data, but theres more */
            {
                show_eof_marker = false;  /*not really eof, so we don't want to show it */
                return((int)EOF);
            } /* if (input_char != EOF) */
        } /* else */

    } /* else */

    dbprintf("!! DEBUG WARNING: should never have reached this line. Module %s, line %d\n", __FILE__, __LINE__);
    return((int)EOF);

} /* get_input() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/



/*****************************************************************************\
 Function: print_offset()

 Description: Prints the offset

 Returns:
\*****************************************************************************/
void print_offset( FILE *outfp, uint32 offvalue )
{
    unsigned long int offset_value = offvalue;

    if (offset_divisor == 0)
        offset_divisor = 1; // Dont want any division by zero.

    offset_value /= offset_divisor;

    switch (offset_display)
    {
        case OFFSET_HEX:
            fprintf(outfp, "%08lX: ", offset_value);
            break;
        case OFFSET_DEC:
            fprintf(outfp, "%010lud: ", offset_value);
            break;
        case OFFSET_OCT:
            fprintf(outfp, "%011loo: ", offset_value);
            break;
        default:
            fprintf(outfp, "%08lX: ", offset_value);
            break;
    }

} /* print_offset() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/



/*****************************************************************************\
 Function: process_options()

 Description: Process command line options

 Returns:
\*****************************************************************************/
bool process_options(int argc, char **argv)
{
    int      opt;
    int32    bytes;
    bool     okay;

    optind = 0;

    /* Check command line options */
    while (true)
    {
        int option_index = 0;
        /*@ignore@*/
        static struct option long_options[] =
        {
            {"ascii", no_argument, 0, 'a'},
            {"ascii-formatted", no_argument, 0, 'A'},
            {"binary", no_argument, 0, 'b'},
            {"formatted-ascii", no_argument, 0, 'A'},
            {"big-endian", no_argument, 0, 'B'},
            {"little-endian", no_argument, 0, 'L'},
            {"columns", required_argument, 0, 'n'},
            {"control", no_argument, 0, 'c'},
            {"decimal", no_argument, 0, 'd'},
            {"show-spaces", no_argument, 0, 'e'},
            {"signed", no_argument, 0, 's'},
            {"hex", no_argument, 0, 'x'},
            {"float", no_argument, 0, 'f'},
            {"double", no_argument, 0, 'l'},
            {"no-ascii", no_argument, 0, 'I'},
            {"no-eof-marker", no_argument, 0, 'E'},
            {"no-highlight", no_argument, 0, 'H'},
            {"no-offset", no_argument, 0, 'F'},
            {"octal", no_argument, 0, 'o'},
            {"pager", optional_argument, 0, 'p'},
            {"show-duplicate", no_argument, 0, 'U'},
            {"width", required_argument, 0, 'w'},
            {"float-width", required_argument, 0, 'W'},
            {"float-precision", required_argument, 0, 'P'},
            {"bytes", no_argument, 0, '1'},
            {"8bit", no_argument, 0, '1'},
            {"words", no_argument, 0, '2'},
            {"16bit", no_argument, 0, '2'},
            {"dwords", no_argument, 0, '4'},
            {"32bit", no_argument, 0, '4'},
            {"qwords", no_argument, 0, '8'},
            {"64bit", no_argument, 0, '8'},
            {"offset-oct", no_argument, 0, 'q'},
            {"offset-dec", no_argument, 0, 'z'},
            {"offset-divisor", required_argument, 0, 'V'},
            {"start-count", required_argument, 0, 'j'},
            {"start-offset", required_argument, 0, 'J'},
            {"count", required_argument, 0, 'k'},
            {"end-offset", required_argument, 0, 'K'},

#ifdef DEBUG
            {"debug", no_argument, 0, 'D'},
#endif
            {"help", no_argument,0, '?'},
            {"version",no_argument,0,'v'},
            {0, 0, 0, 0}
        };
        /*@end@*/
        opt = getopt_long(argc,argv, "bk:j:J:K:flscaAxdeo?Dvp::P:w:W:n:IFHUELB1248qzV:", long_options, &option_index);

        if (opt == -1)
            break;

        switch(opt)
        {
            case 0:
                break;

            case 'x':
                dump_mode = DUMP_MODE_HEX;
                mode_selected = true;
                break;
            case 'c':
                dump_mode = DUMP_MODE_CTRL;
                mode_selected = true;
                break;
            case 'a':
                dump_mode = DUMP_MODE_ASCII;
                mode_selected = true;
                break;
            case 'A':
                dump_mode = DUMP_MODE_FASCII;
                mode_selected = true;
                break;
            case 'b':
                dump_mode = DUMP_MODE_BINARY;
                mode_selected = true;
                break;
            case 'd':
                dump_mode = DUMP_MODE_DEC;
                mode_selected = true;
                break;
            case 'o':
                dump_mode = DUMP_MODE_OCT;
                mode_selected = true;
                break;
            case 's':
                dump_mode = DUMP_MODE_SIGNED;
                mode_selected = true;
                break;
            case 'f':
                dump_mode = DUMP_MODE_FLOAT;
                data_width = sizeof(float);
                mode_selected = true;
                break;
            case 'l':
                dump_mode = DUMP_MODE_DOUBLE;
                data_width = sizeof(double);
                mode_selected = true;
                break;

            case 'q':
                offset_display = OFFSET_OCT;
                break;
            case 'z':
                offset_display = OFFSET_DEC;
                break;

            case '1':
                data_width = 1;
                width_selected = true;
                break;
            case '2':
                data_width = 2;
                width_selected = true;
                break;
            case '4':
                data_width = 4;
                width_selected = true;
                break;
            case '8':
                data_width = 8;
                width_selected = true;
                break;

            case 'p':
                use_pager = true;
                if (optarg)
                    strcpy(pager_program, optarg);
                break;
#ifdef DEBUG
            case 'D':
                print_debug_messages = true;
                break;
#endif
            case 'I':
                show_ascii = false;
                break;
            case 'F':
                show_offset = false;
                break;
            case 'H':
                highlight = false;
                break;
            case 'U':
                no_duplicate = true;
                break;
            case 'E':
                show_eof_marker = false;
                break;
            case 'e':
                show_spaces = true;
                break;
            case 'L':
                /*FALL THRU*/
            case 'B':
                big_endian = true;
                break;

            case 'V':
                offset_divisor = (uint32_t)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting offset divisor string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                if (offset_divisor == 0)
                {
                    fprintf(stderr, "\a%s: warning: `offset-divisor' can not be zero, defaulting to 1.\n", pname);
                    offset_divisor = 1;
                }
                break;
            case 'w':
                //data_width = atoi(optarg);
                data_width = (int)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting data width string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                width_selected = true;
                break;

            case 'W':
                //float_width = atoi(optarg);
                float_width = (int)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting float width string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case 'P':
                //float_precision = atoi(optarg);
                float_precision = (int)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting float precision string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case 'n':
                //columns = atoi(optarg);
                columns = (int)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting columns string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                columns_selected = true;
                break;

            case 'J':
                //bytes = atol(optarg);
                bytes = (long)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting bytes string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                if (data_width != 0)
                    data_start_count = bytes / data_width;
                break;
            case 'j':
                //data_start_count = atol(optarg);
                data_start_count = (long)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting data start count string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case 'K':
                //bytes = atol(optarg);
                bytes = (long)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting bytes string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                if (data_width != 0)
                    data_end_count = bytes / data_width;
                break;
            case 'k':
                //data_end_count = atol(optarg);
                data_end_count = (long)string_to_uint64(optarg,&okay);
                if (!okay)
                {
                    fprintf(stderr, "Error converting data end count string: `%s'\n", optarg);
                    fprintf(stderr, "Program terminated.");
                    exit(7);
                }
                break;

            case '?':
                /*FALLTHRU*/
            case ':':
                show_help();
                exit(0);
                /*NOTREACHED*/
                break;
            case 'v':
                show_version();
                exit(0);
                /*NOTREACHED*/
                break;

            default:
                break;
        } /*switch*/

    } /* while command line args */

    return true;

} /* process_options() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


/*****************************************************************************\
 Function: show_help()

 Description: Shows help message

 Returns:
\*****************************************************************************/
void show_help( void )
{
    FILE *fp;

    #if !defined(__DOS16__)
    if (use_pager)
    {
        fp = popen(pager_program, "w");
        if (fp == NULL)
        {
            perror("pipe error");
            exit(1);
        } /* if (output_file == NULL) */
    } /* if (use_pager) */
    else
    {
        fp = stdout;
    } /* else */
    #else
    fp = stdout;
    #endif

    fprintf(fp, "Slicers Awesome Dump - version %s%s%s\n", SAD_MAJOR_VER, SAD_MINOR_VER, SAD_REVISION);
    fprintf(fp, "Usage: %s [options] [file(s)]\n", pname );
    fprintf(fp, "Options:\n");
    fprintf(fp, "   -a,   --ascii            Display in ASCII format.\n");
    fprintf(fp, "   -A,   --formatted-ascii  Display in \"formatted\" ASCII.\n");
    fprintf(fp, "   -b,   --binary           Display in binary `text'.\n");
    fprintf(fp, "   -c,   --control          Display text and control characters.\n");
    fprintf(fp, "   -d,   --decimal          Display in decimal format.\n");
    fprintf(fp, "   -o,   --octal            Display in octal format.\n");
    fprintf(fp, "   -s,   --signed           Display in signed decimal format.\n");
    fprintf(fp, "   -x,   --hex              Display in hex format.\n");
    fprintf(fp, "   -f,   --float            Display in floating point format.\n");
    fprintf(fp, "   -l,   --double           Display in double floating point format.\n");

    fprintf(fp, "   -W<n>,--float-width=<n>  Character width of floating point number.\n");
    fprintf(fp, "   -P<n>,--float-precision  Precision of floating point number..\n");

    #if !defined(__BIG_ENDIAN__)
        fprintf(fp, "   -B,   --big-endian       Use \"big endian\" byte ordering.\n");
    #else
        fprintf(fp, "   -L,   --little-endian    Use \"little endian\" byte ordering.\n");
    #endif
    fprintf(fp, "   -H,   --no-highlight     Do not add highlight information.\n");
    fprintf(fp, "   -F,   --no-offset        Do not show offset.\n");
    fprintf(fp, "   -I,   --no-ascii         Do not show ascii representation of data.\n");
    fprintf(fp, "   -E,   --no-eof-marker    Do not show end of file marker.\n");
    fprintf(fp, "   -U,   --show-duplicate   Show duplicate lines.\n");
    fprintf(fp, "   -e,   --show-spaces      Show 'visible' spaces in ascii mode.\n");
    fprintf(fp, "   -q,   --offset-oct       Show offsets in octal (default is hex).\n");
    fprintf(fp, "   -z,   --offset-dec       Show offsets in decimal (default is hex).\n");
    fprintf(fp, "   -V,   --offset-divisor   Divide the offset by given value. (default is 1).\n");

    fprintf(fp, "   -p,   --pager[=PAGER]    Send output to pager.\n");

    fprintf(fp, "   -n<n>,--columns=<n>      Number of columns to display per line.\n");
    fprintf(fp, "   -w<n>,--width=<n>        Data width in bytes to display, default is %d.\n", data_width);
    fprintf(fp, "   -1,   --bytes,--8bit     Display in bytes (same as --width=1).\n");
    fprintf(fp, "   -2,   --words,--16bit    Display in words (same as --width=2).\n");
    fprintf(fp, "   -4,   --dwords,--32bit   Display in double words (same as --width=4).\n");
    fprintf(fp, "   -8,   --qwords,--64bit   Display in quad words (same as --width=8).\n");

    fprintf(fp, "   -j<n>,--start-count=<n>  Starting item to begin dump (of size data width).\n");
    fprintf(fp, "   -J<n>,--start-offset=<n> Starting offset in bytes to begin dump.\n");
    fprintf(fp, "   -k<n>,--count=<n>        Number of items to dump (of data width).\n");
    fprintf(fp, "   -K<n>,--end-offset=<n>   Ending offset in bytes.\n");


#ifdef DEBUG
    fprintf(fp, "   -D,   --debug            Show debugging messages.\n");
#endif
    fprintf(fp, "   -?,   --help             Show this message.\n");
    fprintf(fp, "   -v,   --version          Show version information\n");
    fprintf(fp, "\nNote: When entering numbers, default is decimal (base 10). To enter numbers in other bases:");
    fprintf(fp, "\nHexadecimal=0xNNNN, Octal=0oNNNN, Binary=0bNNNN\n");
    fprintf(fp, "\n");
    fprintf(fp, "Report issues to <john@yellowsocks.com>\n");

    #if !defined(__DOS16__)
        if (use_pager)
            (void) pclose(fp);
    #endif

} /* show_help() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


/*****************************************************************************\
 Function: show_version()

 Description: Shows version info

 Returns:
\*****************************************************************************/
void show_version( void )
{
    fprintf(stderr, "'%s' version %s%s%s", pname, SAD_MAJOR_VER, SAD_MINOR_VER, SAD_REVISION);
    #if defined(__ST32__)
        fprintf(stderr, " for Win32.\n");
    #elif defined(__DOS16__)
        fprintf(stderr, " for DOS.\n");
    #elif defined(__LINUX__)
        fprintf(stderr, " for linux.\n");
    #elif defined(__APPLE__)
        fprintf(stderr, " for Apple.\n");
    #elif defined(__QNX__)
        fprintf(stderr, " for QNX.");
        #if defined(__BIG_ENDIAN__)
            fprintf(stderr, " (big-endian)\n");
        #else
            fprintf(stderr, " (little-endian)\n");
        #endif
    #elif defined(__UNIXLIKE__)
        fprintf(stderr, " for unix.\n");
    #else
        fprintf(stderr, "\n");
    #endif

    fprintf(stderr, "Copyright (c) 2006.  John Kiernan\n");
    fprintf(stderr, "Last compiled on %s at %s\n", __DATE__, __TIME__);
    /*attempt to show compiler*/
    #ifdef __COMPILER__
        fprintf(stderr,"Compiled using %s\n", (char *)_MAC_STR(__COMPILER__));
    #endif
} /* show_version() */
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/


bool write_fmtline_bin(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof )
{
    BYTE   *p_byte;
    uint16 *p_word;
    uint32 *p_dword;
    uint64 *p_qword;
    int     i,j;
    bool    partial_line;
    int     elements;
    char    data_str[MAXSTRING];
    char    *ascii_str;
    uint32  ascii_buf_size;
    BYTE    tmp_char;
    uint32  byte_count;
    uint32  bytes_per_line;
    char    bin_string[MAX_BIN_STRING];

    if (data_size==0) return false;
    partial_line = false;
    byte_count = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker )
        return true;

    bytes_per_line = (((unsigned)columns*(unsigned)data_width*8) + (unsigned)columns);
    if (columns%2) --bytes_per_line;

    if (elements < columns)
        partial_line = true;


    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str = (char *)malloc(ascii_buf_size);
    if (ascii_str == NULL)
    {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname, __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0] = 0;
    ascii_str[0] = 0;

    switch (data_size)
    {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                sprintf(data_str, "%s", binstr((uint64)*p_byte++, bin_string, 1));
                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*8;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%s", binstr((uint64)*p_word++, bin_string, 2));
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%s", binstr((uint64)BYTE_SWAP_16(*p_word), bin_string, 2));
                    ++p_word;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*8;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */


                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%s", binstr((uint64)*p_dword++,bin_string,4));
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%s", binstr((uint64)BYTE_SWAP_32(*p_dword), bin_string, 4));
                    ++p_dword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*8;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%s", binstr(*p_qword, bin_string, 8));
                    ++p_qword;
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%s", binstr(BYTE_SWAP_64(*p_qword),bin_string,8));
                    ++p_qword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*8;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname, data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

} /* bool write_fmtline_bin(uint32 current_offset, void *buffer, uint32 count, uint32 data_size) */



bool write_fmtline_hex(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof )
{
    BYTE   *p_byte;
    uint16 *p_word;
    uint32 *p_dword;
    uint64 *p_qword;
    int     i,j;
    bool    partial_line;
    int     elements;
    char    data_str[MAXSTRING];
    char    *ascii_str;
    uint32  ascii_buf_size;
    BYTE    tmp_char;
    uint32  byte_count;
    uint32  bytes_per_line;


    if (data_size==0) return false;
    partial_line = false;
    byte_count = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker )
        return true;

    bytes_per_line = (((unsigned)columns*(unsigned)data_width*2) + (unsigned)columns);
    if (columns%2) --bytes_per_line;

    if (elements < columns)
        partial_line = true;


    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str = (char *)malloc(ascii_buf_size);
    if (ascii_str == NULL)
    {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname, __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0] = 0;
    ascii_str[0] = 0;

    switch (data_size)
    {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                sprintf(data_str, "%02X", *p_byte++);
                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*2;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%04X", *p_word++);
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%04X", BYTE_SWAP_16(*p_word));
                    ++p_word;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*2;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */


                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%08lX", *p_dword++);
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%08lX", BYTE_SWAP_32(*p_dword));
                    ++p_dword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*2;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%08lX%08lX", (uint32)(*p_qword >> 32 ), (uint32)(*p_qword & 0xffffffff));
                    ++p_qword;
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%08lX%08lX", (uint32)BYTE_SWAP_32((*p_qword & 0xffffffff)), (uint32)BYTE_SWAP_32((*p_qword >> 32)) );
                    ++p_qword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)data_size*2;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname, data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

} /* bool write_fmtline_hex(uint32 current_offset, void *buffer, uint32 count, uint32 data_size) */



bool write_fmtline_dec(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof )
{
    BYTE   *p_byte;
    uint16 *p_word;
    uint32 *p_dword;
    uint64 *p_qword;
    int     i,j;
    bool    partial_line;
    int     elements;
    char    data_str[MAXSTRING];
    char    *ascii_str;
    uint32  ascii_buf_size;
    BYTE    tmp_char;
    uint32  byte_count;
    uint32  bytes_per_line;
    int     factor;


    if (data_size==0) return false;
    partial_line = false;
    byte_count = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker )
        return true;

    factor = (int)((data_width*2.5)+0.5);
    bytes_per_line = (uint32)((columns*factor) + columns);
    if (columns%2) --bytes_per_line;

    if (elements < columns)
        partial_line = true;


    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str = (char *)malloc(ascii_buf_size);
    if (ascii_str == NULL)
    {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname, __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0] = 0;
    ascii_str[0] = 0;

    switch (data_size)
    {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                sprintf(data_str, "%03u", *p_byte++);
                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%5u", *p_word++);
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%5u", BYTE_SWAP_16(*p_word));
                    ++p_word;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */


                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%10lu", *p_dword++);
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%10lu", BYTE_SWAP_32(*p_dword));
                    ++p_dword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    #if !defined(__GNUC__)
                    sprintf(data_str, "%20I64u", *p_qword);
                    #else
                    sprintf(data_str, "%20llu", *p_qword);
                    #endif
                    ++p_qword;
                } /* if (little-endian) */
                else
                {
                    #if !defined(__GNUC__)
                    sprintf(data_str, "%20I64u", BYTE_SWAP_64(*p_qword));
                    #else
                    sprintf(data_str, "%20llu", BYTE_SWAP_64(*p_qword));
                    #endif
                    ++p_qword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += (unsigned)factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if ((bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=(int)byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname, data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

}


bool write_fmtline_oct(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof )
{
    BYTE   *p_byte;
    uint16 *p_word;
    uint32 *p_dword;
    uint64 *p_qword;
    int     i,j;
    bool    partial_line;
    int     elements;
    char    data_str[MAXSTRING];
    char    *ascii_str;
    uint32  ascii_buf_size;
    BYTE    tmp_char;
    int     byte_count;
    uint32  bytes_per_line;
    int     factor;


    if (data_size==0) return false;
    partial_line = false;
    byte_count = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker )
        return true;

    factor = (int)((data_width*2.75)+0.5);
    bytes_per_line = (((unsigned)columns*(unsigned)factor) + (unsigned)columns);
    if (columns%2) --bytes_per_line;

    if (elements < columns)
        partial_line = true;


    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str = (char *)malloc(ascii_buf_size);
    if (ascii_str == NULL)
    {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname, __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0] = 0;
    ascii_str[0] = 0;

    switch (data_size)
    {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                sprintf(data_str, "%03o", *p_byte++);
                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%6o", *p_word++);
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%6o", BYTE_SWAP_16(*p_word));
                    ++p_word;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */


                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%11lo", *p_dword++);
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "%11lo", BYTE_SWAP_32(*p_dword));
                    ++p_dword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    #if !defined(__GNUC__)
                    sprintf(data_str, "%22I64o", *p_qword);
                    #else
                    sprintf(data_str, "%22llo", *p_qword);
                    #endif
                    ++p_qword;
                } /* if (little-endian) */
                else
                {
                    #if !defined(__GNUC__)
                    sprintf(data_str, "%22I64o", BYTE_SWAP_64(*p_qword));
                    #else
                    sprintf(data_str, "%22llo", BYTE_SWAP_64(*p_qword));
                    #endif
                    ++p_qword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname, data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

}


bool write_fmtline_signed(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof )
{
    BYTE   *p_byte;
    uint16 *p_word;
    uint32 *p_dword;
    uint64 *p_qword;
    int     i,j;
    bool    partial_line;
    int     elements;
    char    data_str[MAXSTRING];
    char    *ascii_str;
    uint32  ascii_buf_size;
    BYTE    tmp_char;
    int     byte_count;
    uint32  bytes_per_line;
    int     factor;


    if (data_size==0) return false;
    partial_line = false;
    byte_count = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker )
        return true;

    factor = (int)((data_width*2.5)+1.5);
    bytes_per_line =(uint32)((columns*factor) + columns);
    if (columns%2) --bytes_per_line;

    if (elements < columns)
        partial_line = true;


    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str = (char *)malloc(ascii_buf_size);
    if (ascii_str == NULL)
    {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname, __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0] = 0;
    ascii_str[0] = 0;

    switch (data_size)
    {
        case 1:
            if (show_offset) print_offset(fp, current_offset);
            p_byte = (BYTE *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                tmp_char = (unsigned char)*p_byte;
                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                sprintf(data_str, "%4d",  (int)(*(signed char *)p_byte++));
                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "%4s","@");
                    byte_count+=4;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 2:
            if (show_offset) print_offset(fp, current_offset);
            p_word = (uint16 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_word;
                else
                    tmp_char = (unsigned char)BYTE_SWAP_16(*p_word);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "% 6d", (int16)(*(int16 *)p_word++));
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "% 6d", (int16)BYTE_SWAP_16(*(int16 *)p_word));
                    ++p_word;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */


                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, " %ceof%c", E_BEGIN, E_END);
                    byte_count+=6;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "%6s","@");
                    byte_count+=6;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 4:
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "% 11ld", (int32)(*(int32 *)p_dword++));
                } /* if (little-endian) */
                else
                {
                    sprintf(data_str, "% 11ld", (int32)BYTE_SWAP_32(*(int32 *)p_dword));
                    ++p_dword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, " %ceof%c%5s", E_BEGIN, E_END, " ");
                    byte_count+=11;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "%11s","@");
                    byte_count+=11;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    #if !defined(__GNUC__)
                    sprintf(data_str, "% 21I64d", (int64)(*(int64 *)p_qword));
                    #else
                    sprintf(data_str, "% 21lld", (int64)(*(int64 *)p_qword));
                    #endif
                    ++p_qword;
                } /* if (little-endian) */
                else
                {
                    #if !defined(__GNUC__)
                    sprintf(data_str, "% 21I64d", (int64)BYTE_SWAP_64(*(int64 *)p_qword));
                    #else
                    sprintf(data_str, "% 21lld", (int64)BYTE_SWAP_64(*(int64 *)p_qword));
                    #endif
                    ++p_qword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, " %ceof%c%15s",E_BEGIN, E_END, " ");
                    byte_count+=21;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "%21s","@");
                    byte_count+=21;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d>\n", pname, data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

}


bool write_fmtline_float(FILE *fp, uint32 current_offset, void *buffer, uint32 count, int data_size, bool iseof )
{
    uint32 *p_dword;
    uint64 *p_qword;
    int     i,j;
    bool    partial_line;
    int     elements;
    char    data_str[MAXSTRING];
    char    *ascii_str;
    uint32  ascii_buf_size;
    BYTE    tmp_char;
    int     byte_count;
    uint32  bytes_per_line;
    int     factor;


    if (data_size==0) return false;
    partial_line = false;
    byte_count = 0;

    if (iseof) --count;
    elements = (int)count / data_size;

    if (elements < 1 && !show_eof_marker )
        return true;

    if ((unsigned)data_size == sizeof(float) )
        factor = 14;
    else
        factor = 15;

    bytes_per_line = (((unsigned)columns*(unsigned)factor) + (unsigned)columns);
    if (columns%2) --bytes_per_line;

    if (elements < columns)
        partial_line = true;


    ascii_buf_size = ((unsigned)elements * sizeof(tmp_char)) + 1;
    ascii_str = (char *)malloc(ascii_buf_size);
    if (ascii_str == NULL)
    {
        fprintf(stderr, "%s: memory allocation error, module %s, line %d\n", pname, __FILE__, (__LINE__ - 3));
        return false;
    }

    data_str[0] = 0;
    ascii_str[0] = 0;

    if (float_precision == 0) float_precision = 6;

    switch (data_size)
    {
        case 4:
            if (float_width == 0) float_width = 14;
            if (show_offset) print_offset(fp, current_offset);
            p_dword = (uint32 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_dword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_32(*p_dword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    sprintf(data_str, "%*.*g", float_width, float_precision, (float)(*((float *)p_dword)));
                    ++p_dword;
                } /* if (little-endian) */
                else
                {
                    *p_dword = BYTE_SWAP_32(*p_dword);
                    sprintf(data_str, "%*.*g", float_width, float_precision, (float)(*((float *)p_dword)));
                    ++p_dword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        case 8:
            if (float_width == 0) float_width = 15;
            if (show_offset) print_offset(fp, current_offset);
            p_qword = (uint64 *)buffer;
            for (i=0;i<(int)elements;i++)
            {
                if (!big_endian)
                    tmp_char = (unsigned char)*p_qword;
                else
                    tmp_char = (unsigned char) BYTE_SWAP_64(*p_qword);

                if (isprint((int)tmp_char))
                    ascii_str[i] = (char)tmp_char;
                else
                    ascii_str[i] = '.';
                ascii_str[i+1] = 0;

                if (!big_endian)
                {
                    #if !defined(__GNUC__)
                    sprintf(data_str, "%*.*I64g",float_width, float_precision,  (double)( *((double *)p_qword)));
                    #else
                    sprintf(data_str, "%*.*g",float_width, float_precision,  (double)( *((double *)p_qword)));
                    #endif
                    ++p_qword;
                } /* if (little-endian) */
                else
                {
                    *p_qword = BYTE_SWAP_64(*p_qword);
                    #if !defined(__GNUC__)
                    sprintf(data_str, "%*.*I64g",float_width, float_precision,  (double)( *((double *)p_qword)));
                    #else
                    sprintf(data_str, "%*.*g",float_width, float_precision,  (double)( *((double *)p_qword)));
                    #endif
                    ++p_qword;
                } /* else */

                if (highlight)
                {
                    for (j=0;j<(int)strlen(data_str);j++)
                    {
                        if (data_str[j] != CH_SPACE)
                            fprintf(fp, "%c\b%c", data_str[j], data_str[j]);
                        else
                            fprintf(fp, "%c", data_str[j]);
                    } /* for (j=0;j<strlen(data_str);j++) */
                } /* if (highlight) */
                else
                    fprintf(fp, "%s", data_str);

                byte_count += factor;
                if (columns%2 == 0)
                {
                    if (i == (((int)columns/2)-1) )
                    {
                        fprintf(fp, "  ");
                        byte_count+=2;
                    } /* if (i == ((elements/2)-1)) */
                    else if ((i != (int)elements-1) || partial_line)
                    {
                        fprintf(fp, " ");
                        byte_count++;
                    } /* else */

                } /* if (elements%2 == 0) */
                else if ((i != (int)elements-1) || partial_line)
                {
                    fprintf(fp, " ");
                    byte_count++;
                } /* else */



            } /* for (i=0;i<elements;i++) */

            if (iseof && show_eof_marker)
            {
                if (((int)bytes_per_line - byte_count) >= 5)
                {
                    fprintf(fp, "%ceof%c", E_BEGIN, E_END);
                    byte_count+=5;
                } /* if ((bytes_per_line - byte_count) >= 5) */
                else
                {
                    fprintf(fp, "@");
                    ++byte_count;
                } /* else */
            } /* if (partial_line) */

            if (partial_line)
                for (i=byte_count;i<(int)bytes_per_line;i++) fprintf(fp, " ");

            if (show_ascii)
                fprintf(fp, " %s\n", ascii_str);
            else
                fprintf(fp, "\n");

            break;

        default:
            fprintf(stderr, "%s: unsupported data size specified <%d> for format.\n", pname, data_size);
            free(ascii_str);
            return false;
            /*NOTREACHED*/
            break;

    } /* switch (data_size) */

    free(ascii_str);
    return true;

}


/*lint -restore*/
