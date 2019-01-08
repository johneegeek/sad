/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 Get options from file
 $Header:$
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef _MAKEARGS_H
#define _MAKEARGS_H

#ifdef __cplusplus
extern "C" {
#endif

char ma_comment_char = '#';

/* external prototypes */
extern int cat_args(int *dargc, char ***dargv, const int sargc, char **sargv);
extern void free_makeargs(int margc, char **margv);
extern int env_makeargs(const char *env_str, char ***nargv);
extern int fmakeargs(const char *filename, char ***nargv);

#ifdef __cplusplus
}
#endif
#endif /* !_MAKEARGS_H */
