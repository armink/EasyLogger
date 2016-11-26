/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Some utils for this library.
 * Created on: 2015-04-28
 */

#include <elog.h>
#include <string.h>

#define LOG_TAG    "elog.utils"
#define assert     ELOG_ASSERT
#define log_e(...) elog_e(LOG_TAG, __VA_ARGS__)
#define log_w(...) elog_w(LOG_TAG, __VA_ARGS__)

#ifdef ELOG_DEBUG
    #define log_d(...) elog_d(LOG_TAG, __VA_ARGS__)
    #define log_v(...) elog_v(LOG_TAG, __VA_ARGS__)
#else
    #define log_d(...)
    #define log_v(...)
#endif

/**
 * another copy string function
 *
 * @param cur_len current copied log length, max size is ELOG_LINE_BUF_SIZE
 * @param dst destination
 * @param src source
 *
 * @return copied length
 */
size_t elog_strcpy(size_t cur_len, char *dst, const char *src) {
    const char *src_old = src;

    assert(dst);
    assert(src);

    while (*src != 0) {
        /* make sure destination has enough space */
        if (cur_len++ <= ELOG_LINE_BUF_SIZE) {
            *dst++ = *src++;
        } else {
            break;
        }
    }
    return src - src_old;
}

/**
 * Copy line log split by newline sign. It will copy all log when the newline sign isn't find.
 *
 * @param line line log buffer
 * @param log origin log buffer
 * @param len origin log buffer length
 *
 * @return copy size
 */
size_t elog_cpyln(char *line, const char *log, size_t len) {
    size_t newline_len = strlen(ELOG_NEWLINE_SIGN), copy_size = 0;

    assert(log);
    assert(line);

    while (len--) {
        *line++ = *log++;
        copy_size++;
        if (copy_size >= newline_len && !strncmp(log - newline_len, ELOG_NEWLINE_SIGN, newline_len)) {
            break;
        }
    }
    return copy_size;
}
