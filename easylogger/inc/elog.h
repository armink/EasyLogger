/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
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
 * Function: It is an head file for this library. You can see all be called functions.
 * Created on: 2015-04-28
 */

#ifndef __ELOG_H__
#define __ELOG_H__

#include <elog_cfg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* output log's level */
#define ELOG_LVL_ASSERT                      0
#define ELOG_LVL_ERROR                       1
#define ELOG_LVL_WARN                        2
#define ELOG_LVL_INFO                        3
#define ELOG_LVL_DEBUG                       4
#define ELOG_LVL_VERBOSE                     5

/* the output silent level and all level for filter setting */
#define ELOG_FILTER_LVL_SILENT               ELOG_LVL_ASSERT
#define ELOG_FILTER_LVL_ALL                  ELOG_LVL_VERBOSE

/* output log's level total number */
#define ELOG_LVL_TOTAL_NUM                   6

/* EasyLogger software version number */
#define ELOG_SW_VERSION                      "2.2.99"

/* EasyLogger assert for developer. */
#ifdef ELOG_ASSERT_ENABLE
    #define ELOG_ASSERT(EXPR)                                                 \
    if (!(EXPR))                                                              \
    {                                                                         \
        if (elog_assert_hook == NULL) {                                       \
            elog_a("elog", "(%s) has assert failed at %s:%ld.", #EXPR, __FUNCTION__, __LINE__); \
            while (1);                                                        \
        } else {                                                              \
            elog_assert_hook(#EXPR, __FUNCTION__, __LINE__);                  \
        }                                                                     \
    }
#else
    #define ELOG_ASSERT(EXPR)                    ((void)0);
#endif

#ifndef ELOG_OUTPUT_ENABLE
    #define elog_assert(tag, ...)
    #define elog_error(tag, ...)
    #define elog_warn(tag, ...)
    #define elog_info(tag, ...)
    #define elog_debug(tag, ...)
    #define elog_verbose(tag, ...)
#else /* ELOG_OUTPUT_ENABLE */
    #if ELOG_OUTPUT_LVL >= ELOG_LVL_ASSERT
        #define elog_assert(tag, ...) \
                elog_output(ELOG_LVL_ASSERT, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #else
        #define elog_assert(tag, ...)
    #endif /* ELOG_OUTPUT_LVL >= ELOG_LVL_ASSERT */

    #if ELOG_OUTPUT_LVL >= ELOG_LVL_ERROR
        #define elog_error(tag, ...) \
                elog_output(ELOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #else
        #define elog_error(tag, ...)
    #endif /* ELOG_OUTPUT_LVL >= ELOG_LVL_ERROR */

    #if ELOG_OUTPUT_LVL >= ELOG_LVL_WARN
        #define elog_warn(tag, ...) \
                elog_output(ELOG_LVL_WARN, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #else
        #define elog_warn(tag, ...)
    #endif /* ELOG_OUTPUT_LVL >= ELOG_LVL_WARN */

    #if ELOG_OUTPUT_LVL >= ELOG_LVL_INFO
        #define elog_info(tag, ...) \
                elog_output(ELOG_LVL_INFO, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #else
        #define elog_info(tag, ...)
    #endif /* ELOG_OUTPUT_LVL >= ELOG_LVL_INFO */

    #if ELOG_OUTPUT_LVL >= ELOG_LVL_DEBUG
        #define elog_debug(tag, ...) \
                elog_output(ELOG_LVL_DEBUG, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #else
        #define elog_debug(tag, ...)
    #endif /* ELOG_OUTPUT_LVL >= ELOG_LVL_DEBUG */

    #if ELOG_OUTPUT_LVL == ELOG_LVL_VERBOSE
        #define elog_verbose(tag, ...) \
                elog_output(ELOG_LVL_VERBOSE, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #else
        #define elog_verbose(tag, ...)
    #endif /* ELOG_OUTPUT_LVL == ELOG_LVL_VERBOSE */
#endif /* ELOG_OUTPUT_ENABLE */

/* all formats index */
typedef enum {
    ELOG_FMT_LVL    = 1 << 0, /**< level */
    ELOG_FMT_TAG    = 1 << 1, /**< tag */
    ELOG_FMT_TIME   = 1 << 2, /**< current time */
    ELOG_FMT_P_INFO = 1 << 3, /**< process info */
    ELOG_FMT_T_INFO = 1 << 4, /**< thread info */
    ELOG_FMT_DIR    = 1 << 5, /**< file directory and name */
    ELOG_FMT_FUNC   = 1 << 6, /**< function name */
    ELOG_FMT_LINE   = 1 << 7, /**< line number */
} ElogFmtIndex;

/* macro definition for all formats */
#define ELOG_FMT_ALL    (ELOG_FMT_LVL|ELOG_FMT_TAG|ELOG_FMT_TIME|ELOG_FMT_P_INFO|ELOG_FMT_T_INFO| \
    ELOG_FMT_DIR|ELOG_FMT_FUNC|ELOG_FMT_LINE)

/* output log's tag filter */
typedef struct {
    uint8_t level;
    char tag[ELOG_FILTER_TAG_MAX_LEN + 1];
    bool tag_use_flag; /**< false : tag is no used   true: tag is used */
} ElogTagLvlFilter, *ElogTagLvlFilter_t;

/* output log's filter */
typedef struct {
    uint8_t level;
    char tag[ELOG_FILTER_TAG_MAX_LEN + 1];
    char keyword[ELOG_FILTER_KW_MAX_LEN + 1];
    ElogTagLvlFilter tag_lvl[ELOG_FILTER_TAG_LVL_MAX_NUM];
} ElogFilter, *ElogFilter_t;

/* easy logger */
typedef struct {
    ElogFilter filter;
    size_t enabled_fmt_set[ELOG_LVL_TOTAL_NUM];
    bool init_ok;
    bool output_enabled;
    bool output_lock_enabled;
    bool output_is_locked_before_enable;
    bool output_is_locked_before_disable;

#ifdef ELOG_COLOR_ENABLE
    bool text_color_enabled;
#endif

}EasyLogger, *EasyLogger_t;

/* EasyLogger error code */
typedef enum {
    ELOG_NO_ERR,
} ElogErrCode;

/* elog.c */
ElogErrCode elog_init(void);
void elog_deinit(void);
void elog_start(void);
void elog_stop(void);
void elog_set_output_enabled(bool enabled);
bool elog_get_output_enabled(void);
void elog_set_text_color_enabled(bool enabled);
bool elog_get_text_color_enabled(void);
void elog_set_fmt(uint8_t level, size_t set);
void elog_set_filter(uint8_t level, const char *tag, const char *keyword);
void elog_set_filter_lvl(uint8_t level);
void elog_set_filter_tag(const char *tag);
void elog_set_filter_kw(const char *keyword);
void elog_set_filter_tag_lvl(const char *tag, uint8_t level);
uint8_t elog_get_filter_tag_lvl(const char *tag);
void elog_raw(const char *format, ...);
void elog_output(uint8_t level, const char *tag, const char *file, const char *func,
        const long line, const char *format, ...);
void elog_output_lock_enabled(bool enabled);
extern void (*elog_assert_hook)(const char* expr, const char* func, size_t line);
void elog_assert_set_hook(void (*hook)(const char* expr, const char* func, size_t line));
int8_t elog_find_lvl(const char *log);
const char *elog_find_tag(const char *log, uint8_t lvl, size_t *tag_len);
void elog_hexdump(const char *name, uint8_t width, const void *buf, uint16_t size);

#define elog_a(tag, ...)     elog_assert(tag, __VA_ARGS__)
#define elog_e(tag, ...)     elog_error(tag, __VA_ARGS__)
#define elog_w(tag, ...)     elog_warn(tag, __VA_ARGS__)
#define elog_i(tag, ...)     elog_info(tag, __VA_ARGS__)
#define elog_d(tag, ...)     elog_debug(tag, __VA_ARGS__)
#define elog_v(tag, ...)     elog_verbose(tag, __VA_ARGS__)

/**
 * log API short definition
 * NOTE: The `LOG_TAG` and `LOG_LVL` must defined before including the <elog.h> when you want to use log_x API.
 */
#if !defined(LOG_TAG)
    #define LOG_TAG          "NO_TAG"
#endif
#if !defined(LOG_LVL)
    #define LOG_LVL          ELOG_LVL_VERBOSE
#endif
#if LOG_LVL >= ELOG_LVL_ASSERT
    #define log_a(...)       elog_a(LOG_TAG, __VA_ARGS__)
#else
    #define log_a(...)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_ERROR
    #define log_e(...)       elog_e(LOG_TAG, __VA_ARGS__)
#else
    #define log_e(...)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_WARN
    #define log_w(...)       elog_w(LOG_TAG, __VA_ARGS__)
#else
    #define log_w(...)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_INFO
    #define log_i(...)       elog_i(LOG_TAG, __VA_ARGS__)
#else
    #define log_i(...)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_DEBUG
    #define log_d(...)       elog_d(LOG_TAG, __VA_ARGS__)
#else
    #define log_d(...)       ((void)0);
#endif
#if LOG_LVL >= ELOG_LVL_VERBOSE
    #define log_v(...)       elog_v(LOG_TAG, __VA_ARGS__)
#else
    #define log_v(...)       ((void)0);
#endif

/* assert API short definition */
#if !defined(assert)
    #define assert           ELOG_ASSERT
#endif

/* elog_buf.c */
void elog_buf_enabled(bool enabled);
void elog_flush(void);

/* elog_async.c */
void elog_async_enabled(bool enabled);
size_t elog_async_get_log(char *log, size_t size);
size_t elog_async_get_line_log(char *log, size_t size);

/* elog_utils.c */
size_t elog_strcpy(size_t cur_len, char *dst, const char *src);
size_t elog_cpyln(char *line, const char *log, size_t len);
void *elog_memcpy(void *dst, const void *src, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* __ELOG_H__ */
