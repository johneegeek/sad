/*******************************************************************************
 * SAD - Debug Printf Module Header
 *
 * Debug printf functions for conditional debug output
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
#ifndef _DPRINTF_H
#define _DPRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

    /*prototypes*/
    void dbprintf(char* format_sz, ...);

#ifdef __cplusplus
}
#endif

#endif /* !_DPRINTF_H */
