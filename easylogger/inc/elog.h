/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Function: Is is an head file for this library. You can see all be called functions.
 * Created on: 2015-04-28
 */

#ifndef __ELOG_H__
#define __ELOG_H__

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
/* setting static output log level. default is verbose */
#define ELOG_OUTPUT_LVL                      ELOG_LVL_VERBOSE
/* enable log output. default open this macro */
#define ELOG_OUTPUT_ENABLE
/* using output to file mode */
#define ELOG_USING_OUTPUT_FILE
/* output line number max length */
#define ELOG_LINE_NUM_MAX_LEN                5
/* log buffer size */
#define ELOG_BUF_SIZE                        256
/* output filter's tag max length */
#define ELOG_FILTER_TAG_MAX_LEN              16
/* output filter's keyword max length */
#define ELOG_FILTER_KW_MAX_LEN               16
/* EasyLogger software version number */
#define ELOG_SW_VERSION                      "0.06.09"

/* EasyLogger assert for developer. */
#define ELOG_ASSERT(EXPR)                                                   \
if (!(EXPR))                                                                \
{                                                                           \
    elog_a("elog", "(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__); \
    while (1);                                                              \
}

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

/* output log's filter */
typedef struct {
    uint8_t level;
    char tag[ELOG_FILTER_TAG_MAX_LEN + 1];
    char keyword[ELOG_FILTER_KW_MAX_LEN + 1];
} ElogFilter, *ElogFilter_t;

/* easy logger */
typedef struct {
    ElogFilter filter;
    size_t enabled_fmt_set;
    bool output_enabled;
}EasyLogger, *EasyLogger_t;

/* EasyLogger error code */
typedef enum {
    ELOG_NO_ERR,
} ElogErrCode;

/* elog.c */
ElogErrCode elog_init(void);
void elog_start(void);
void elog_set_output_enabled(bool enabled);
bool elog_get_output_enabled(void);
void elog_set_fmt(size_t set);
void elog_set_filter(uint8_t level, const char *tag, const char *keyword);
void elog_set_filter_lvl(uint8_t level);
void elog_set_filter_tag(const char *tag);
void elog_set_filter_kw(const char *keyword);
void elog_raw(const char *format, ...);
void elog_output(uint8_t level, const char *tag, const char *file, const char *func,
        const long line, const char *format, ...);

#ifndef ELOG_OUTPUT_ENABLE

#define elog_a(tag, ...)
#define elog_e(tag, ...)
#define elog_w(tag, ...)
#define elog_i(tag, ...)
#define elog_d(tag, ...)
#define elog_v(tag, ...)

#else /* ELOG_OUTPUT_ENABLE */

#if ELOG_OUTPUT_LVL >= ELOG_LVL_ASSERT
#define elog_a(tag, ...) \
        elog_output(ELOG_LVL_ASSERT, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define elog_a(tag, ...)
#endif

#if ELOG_OUTPUT_LVL >= ELOG_LVL_ERROR
#define elog_e(tag, ...) \
        elog_output(ELOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define elog_e(tag, ...)
#endif

#if ELOG_OUTPUT_LVL >= ELOG_LVL_WARN
#define elog_w(tag, ...) \
        elog_output(ELOG_LVL_WARN, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define elog_w(tag, ...)
#endif

#if ELOG_OUTPUT_LVL >= ELOG_LVL_INFO
#define elog_i(tag, ...) \
        elog_output(ELOG_LVL_INFO, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define elog_i(tag, ...)
#endif

#if ELOG_OUTPUT_LVL >= ELOG_LVL_DEBUG
#define elog_d(tag, ...) \
        elog_output(ELOG_LVL_DEBUG, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define elog_d(tag, ...)
#endif

#if ELOG_OUTPUT_LVL == ELOG_LVL_VERBOSE
#define elog_v(tag, ...) \
        elog_output(ELOG_LVL_VERBOSE, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

#endif /* ELOG_OUTPUT_ENABLE */

/* elog_utils.c */
size_t elog_strcpy(size_t cur_len, char *dst, const char *src);

/* elog_port.c */
ElogErrCode elog_port_init(void);
void elog_port_output(const char *output, size_t size);
void elog_port_output_lock(void);
void elog_port_output_unlock(void);
const char *elog_port_get_time(void);
const char *elog_port_get_p_info(void);
const char *elog_port_get_t_info(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELOG_H__ */
