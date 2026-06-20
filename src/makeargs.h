/*******************************************************************************
 * SAD - Makeargs Module Header
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
#ifndef _MAKEARGS_H
#define _MAKEARGS_H

#ifdef __cplusplus
extern "C" {
#endif

    char ma_comment_char = '#';

    /* external prototypes */
    extern int  cat_args(int* dargc, char*** dargv, const int sargc, char** sargv);
    extern void free_makeargs(int margc, char** margv);
    extern int  env_makeargs(const char* env_str, char*** nargv);
    extern int  fmakeargs(const char* filename, char*** nargv);

#ifdef __cplusplus
}
#endif
#endif /* !_MAKEARGS_H */
