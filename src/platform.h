/*******************************************************************************
 * SAD - Platform Header
 *
 * Platform-specific type definitions and macros
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
#ifndef H_PLATFORM_H
#define H_PLATFORM_H

#ifndef __DOSLIKE__
#if defined(__WIN32__) || defined(__ST32__)
#define __DOSLIKE__
#endif
#endif

#if defined(__DJGPP__)
#define __DOS16__
#endif

#if defined(__linux__) || defined(__QNX__) || defined(__LINUX__) || defined(__APPLE__)
#define __UNIXLIKE__
#define _UNIXSTYLE_PATHS
#endif

#if defined(__WATCOMC__) || defined(__APPLE__)
#define _NEED_STRING_H
#endif

#endif
