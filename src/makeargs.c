/*******************************************************************************
 * SAD - Makeargs Module
 *
 * Configuration file and environment variable parsing
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

#include "platform.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__WATCOMC__)
#include <sys/types.h>
#endif
#include <sys/stat.h>

#if defined(__DJGPP__) || defined(__UNIXLIKE__)
#include <errno.h>
#endif

static char* module_name = "makeargs";

#define CHECK_MEM_ERROR(ptr)                                                        \
    if (ptr == NULL) {                                                              \
        fprintf(stderr, "\nfatal: memory allocation error: module %s, line %d\n\a", \
                __FILE__, __LINE__ - 1);                                            \
        abort();                                                                    \
    }

extern char ma_comment_char;

/*local protos*/
static void strip_comments_cr(void* buffer, unsigned long buf_size);
extern void free_makeargs(int margc, char** margv);

int cat_args(int* dargc, char*** dargv, const int sargc, char** sargv)
{
    int    count;
    int    i;
    char **pargv, **des;

    count = *dargc;
    des   = *dargv;

    /* Create new storage location and copy everyting to it*/
    /* Then free the one that was passed in. */
    pargv = (char**)malloc(sizeof(char*) * count);

    /* arg[0] gets set to the new arg[0] (*/
    pargv[0] = (char*)malloc(strlen(sargv[0]) * sizeof(char) + 1);
    CHECK_MEM_ERROR(pargv[0]);
    strcpy(pargv[0], sargv[0]);

    for (i = 1; i < count; i++) {
        pargv[i] = (char*)malloc(strlen(des[i]) * sizeof(char) + 1);
        CHECK_MEM_ERROR(pargv[i]);
        strcpy(pargv[i], des[i]);
    } /* for (i=0;i<count;i++) */
    free_makeargs(*dargc, *dargv);

    /* copy source args over */
    for (i = 1; i < sargc; i++) {
        pargv = (char**)realloc(pargv, sizeof(char*) * (count + 1));
        CHECK_MEM_ERROR(pargv);
        pargv[count] = (char*)malloc(strlen(sargv[i]) * sizeof(char) + 1);
        CHECK_MEM_ERROR(pargv[count]);
        strcpy(pargv[count], sargv[i]);
        ++count;
    }

    *dargv = pargv;
    *dargc = count;
    return (count);

} /* int cat_args(char ***newargv, const int sargc, const char **sargv) */

int env_makeargs(const char* env_str, char*** nargv)
{
    char*         scratch;
    char*         tmp_string;
    char*         tmp;
    unsigned long scratch_size;
    int           count;
    char**        pargv;

    count = 0;

    tmp_string = getenv(env_str);
    /* make a copy of string returned, because you cant modify it */
    if (tmp_string == NULL) return 0;

    scratch = (char*)malloc(strlen(tmp_string) * sizeof(char) + 1);
    CHECK_MEM_ERROR(scratch);
    strcpy(scratch, tmp_string);
    scratch_size = strlen(scratch);
    if (scratch_size == 0) return 0;

    strip_comments_cr(scratch, scratch_size);

#ifdef DEBUG_MAKEARGS
    printf("DEBUG: %s: [%s]\n", module_name, scratch);
#endif

    /* allocate array space */
    pargv = (char**)malloc(sizeof(char*) * 1);
    CHECK_MEM_ERROR(pargv);
    pargv[0] = (char*)malloc(strlen(env_str) * sizeof(char) + 1);
    CHECK_MEM_ERROR(pargv[0]);
    strcpy(pargv[0], env_str);
    ++count;

    tmp = strtok(scratch, " ");
    while (tmp != NULL) {
        pargv = (char**)realloc(pargv, sizeof(char*) * (count + 1));
        CHECK_MEM_ERROR(pargv);
        pargv[count] = (char*)malloc(strlen(tmp) * sizeof(char) + 1);
        CHECK_MEM_ERROR(pargv[count]);
        strcpy(pargv[count], tmp);
        tmp = strtok(NULL, " ");
        ++count;
    }

    *nargv = pargv;
    free(scratch);
    return (count);
}

int fmakeargs(const char* filename, char*** nargv)
{
    FILE*         fp;
    struct stat   fbuf;
    char*         scratch;
    char*         tmp;
    unsigned long scratch_size;
    unsigned long data_read;
    int           count;
    char**        pargv;

    count = 0;

    if (-1 == stat(filename, &fbuf)) {
        fprintf(stderr, "\n%s: error accessing file %s : %s\n", module_name, filename,
                strerror(errno));
        return (0);
    }

    scratch_size = fbuf.st_size;
    scratch      = (char*)malloc(scratch_size);
    CHECK_MEM_ERROR(scratch);

    memset(scratch, 0x0, scratch_size);

    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "\n%s: error opening file %s : %s\n", module_name, filename,
                strerror(errno));
        free(scratch);
        return (0);
    } /* if (fp == NULL) */

    if (0 == (data_read = fread(scratch, 1, scratch_size, fp))) {
        fprintf(stderr, "%s: unable to read from file %s\n", module_name, filename);
        fclose(fp);
        free(scratch);
        return (0);
    } /* if (1 != fread(scratch, scratch_size, 1, fp)) */

    fclose(fp);

    strip_comments_cr(scratch, data_read);

#ifdef DEBUG_MAKEARGS
    printf("DEBUG: %s: [%s]\n", module_name, scratch);
#endif

    /* allocate array space */
    pargv = (char**)malloc(sizeof(char*) * 1);
    CHECK_MEM_ERROR(pargv);
    pargv[0] = (char*)malloc(strlen(filename) * sizeof(char) + 1);
    CHECK_MEM_ERROR(pargv[0]);
    strcpy(pargv[0], filename);
    ++count;

    tmp = strtok(scratch, " ");
    while (tmp != NULL) {
        pargv = (char**)realloc(pargv, sizeof(char*) * (count + 1));
        CHECK_MEM_ERROR(pargv);
        pargv[count] = (char*)malloc(strlen(tmp) * sizeof(char) + 1);
        CHECK_MEM_ERROR(pargv[count]);
        strcpy(pargv[count], tmp);
        tmp = strtok(NULL, " ");
        ++count;
    }
    free(scratch);

    *nargv = pargv;
    return (count);
}

void free_makeargs(int margc, char** margv)
{
    int i;

    if (margc == 0) return;
    if (margv == NULL) return;

    for (i = 0; i < margc; i++) free(margv[i]);

    free(margv);

} /* void free_args_from_file(void) */

static void strip_comments_cr(void* buffer, unsigned long buf_size)
{
    char*         tmp_buf;
    char *        s, *d;
    int           in_comment;
    unsigned long i;

    in_comment = 0;

    tmp_buf = (char*)malloc(buf_size);
    CHECK_MEM_ERROR(tmp_buf);

    memcpy(tmp_buf, buffer, buf_size);
    memset(buffer, 0x0, buf_size);

    s = tmp_buf;
    d = buffer;

    for (i = 0; i < buf_size; i++) {
        if ((*s != '\n') && (*s != '\r')) {
            if (*s == ma_comment_char)
                in_comment = 1;
            else if (!in_comment)
                *d++ = *s;
        }
        else {
            in_comment = 0;
            *d++       = ' '; /* replace cr's with spaces */
        }
        ++s;
    }

    free(tmp_buf);

} /* void remove_comments(void *buffer, unsigned long buf_size) */
