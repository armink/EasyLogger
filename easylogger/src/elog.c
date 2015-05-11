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
 * Function: Initialize function and other general function.
 * Created on: 2015-04-28
 */

#include "elog.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

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
    bool_t output_enabled;
}EasyLogger, *EasyLogger_t;

/* EasyLogger object */
static EasyLogger elog;
/* log buffer */
static char log_buf[ELOG_BUF_SIZE] = { 0 };
/* log tag */
static const char *tag = "ELOG";
/* level output info */
static const char *level_output_info[] = {
        "A/",
        "E/",
        "W/",
        "I/",
        "D/",
        "V/",
};
static bool_t get_fmt_enabled(size_t set);

/**
 * EasyLogger initialize.
 *
 * @return result
 */
ElogErrCode elog_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    /* port initialize */
    result = elog_port_init();
    /* set level is ELOG_LVL_VERBOSE */
    elog_set_filter_lvl(ELOG_LVL_VERBOSE);
    /* enable output */
    elog_set_output_enabled(TRUE);

    if (result == ELOG_NO_ERR) {
        elog_d(tag, "EasyLogger V%s is initialize success.", ELOG_SW_VERSION);
    } else {
        elog_d(tag, "EasyLogger V%s is initialize fail.", ELOG_SW_VERSION);
    }
    return result;
}

/**
 * set output enable or disable
 *
 * @param enabled TRUE: enable FALSE: disable
 */
void elog_set_output_enabled(bool_t enabled) {
    ELOG_ASSERT((enabled == FALSE) || (enabled == TRUE));

    elog.output_enabled = enabled;
}

/**
 * get output is enable or disable
 *
 * @return enable or disable
 */
bool_t elog_get_output_enabled(void) {
    return elog.output_enabled;
}

/**
 * set log output format. only enable or disable
 *
 * @param set format set
 */
void elog_set_fmt(size_t set) {
    elog.enabled_fmt_set = set;
}

/**
 * set log filter all parameter
 *
 * @param level level
 * @param tag tag
 * @param keyword keyword
 */
void elog_set_filter(uint8_t level, const char *tag, const char *keyword) {
    ELOG_ASSERT(level <= ELOG_LVL_VERBOSE);

    elog_set_filter_lvl(level);
    elog_set_filter_tag(tag);
    elog_set_filter_kw(keyword);
}

/**
 * set log filter's level
 *
 * @param level level
 */
void elog_set_filter_lvl(uint8_t level) {
    ELOG_ASSERT(level <= ELOG_LVL_VERBOSE);

    elog.filter.level = level;
}

/**
 * set log filter's tag
 *
 * @param tag tag
 */
void elog_set_filter_tag(const char *tag) {
    strncpy(elog.filter.tag, tag, ELOG_FILTER_TAG_MAX_LEN);
}

/**
 * set log filter's keyword
 *
 * @param keyword keyword
 */
void elog_set_filter_kw(const char *keyword) {
    strncpy(elog.filter.keyword, keyword, ELOG_FILTER_KW_MAX_LEN);
}

/**
 * output RAW format log
 *
 * @param format output format
 * @param ... args
 */
void elog_raw(const char *format, ...) {
    va_list args;
    int fmt_result;

    /* check output enabled */
    if (!elog.output_enabled) {
        return;
    }

    /* args point to the first variable parameter */
    va_start(args, format);

    /* lock output */
    elog_port_output_lock();

    /* package log data to buffer */
    fmt_result = vsnprintf(log_buf, ELOG_BUF_SIZE, format, args);

    /* output converted log */
    if ((fmt_result > -1) && (fmt_result <= ELOG_BUF_SIZE)) {
        /* output log */
        elog_port_output(log_buf, fmt_result);
    } else {
        /* output log */
        elog_port_output(log_buf, ELOG_BUF_SIZE);
    }

    /* unlock output */
    elog_port_output_unlock();

    va_end(args);
}

/**
 * output the log
 *
 * @param level level
 * @param tag tag
 * @param file file name
 * @param func function name
 * @param line line number
 * @param format output format
 * @param ... args
 *
 */
void elog_output(uint8_t level, const char *tag, const char *file, const char *func,
        const long line, const char *format, ...) {
    size_t tag_len = strlen(tag), log_len = 0;
    char line_num[ELOG_LINE_NUM_MAX_LEN + 1] = { 0 };
    char tag_sapce[ELOG_FILTER_TAG_MAX_LEN / 2 + 1] = { 0 };
    va_list args;
    int fmt_result;

    ELOG_ASSERT(level <= ELOG_LVL_VERBOSE);

    /* check output enabled */
    if (!elog.output_enabled) {
        return;
    }

    /* level filter */
    if (level > elog.filter.level) {
        return;
    } else if (!strstr(tag, elog.filter.tag)) { /* tag filter */
        //TODO 可以考虑采用KMP及朴素模式匹配字符串，提升性能
        return;
    }

    /* args point to the first variable parameter */
    va_start(args, format);

    /* lock output */
    elog_port_output_lock();
    /* package level info */
    if (get_fmt_enabled(ELOG_FMT_LVL)) {
        log_len += elog_strcpy(log_len, log_buf + log_len, level_output_info[level]);
    }
    /* package tag info */
    if (get_fmt_enabled(ELOG_FMT_TAG)) {
        log_len += elog_strcpy(log_len, log_buf + log_len, tag);
        /* if the tag length is less than 50% ELOG_FILTER_TAG_MAX_LEN, then fill space */
        if (tag_len <= ELOG_FILTER_TAG_MAX_LEN / 2) {
             memset(tag_sapce, ' ', ELOG_FILTER_TAG_MAX_LEN / 2 - tag_len);
             log_len += elog_strcpy(log_len, log_buf + log_len, tag_sapce);
        }
        log_len += elog_strcpy(log_len, log_buf + log_len, " ");
    }
    /* package time, process and thread info */
    if (get_fmt_enabled(ELOG_FMT_TIME) || get_fmt_enabled(ELOG_FMT_P_INFO)
            || get_fmt_enabled(ELOG_FMT_T_INFO)) {
        log_len += elog_strcpy(log_len, log_buf + log_len, "[");
        /* package time info */
        if (get_fmt_enabled(ELOG_FMT_TIME)) {
            log_len += elog_strcpy(log_len, log_buf + log_len, elog_port_get_time());
            if (get_fmt_enabled(ELOG_FMT_P_INFO) || get_fmt_enabled(ELOG_FMT_T_INFO)) {
                log_len += elog_strcpy(log_len, log_buf + log_len, " ");
            }
        }
        /* package process info */
        if (get_fmt_enabled(ELOG_FMT_P_INFO)) {
            log_len += elog_strcpy(log_len, log_buf + log_len, elog_port_get_p_info());
            if (get_fmt_enabled(ELOG_FMT_T_INFO)) {
                log_len += elog_strcpy(log_len, log_buf + log_len, " ");
            }
        }
        /* package thread info */
        if (get_fmt_enabled(ELOG_FMT_T_INFO)) {
            log_len += elog_strcpy(log_len, log_buf + log_len, elog_port_get_t_info());
        }
        log_len += elog_strcpy(log_len, log_buf + log_len, "] ");
    }
    /* package file directory and name, function name and line number info */
    if (get_fmt_enabled(ELOG_FMT_DIR) || get_fmt_enabled(ELOG_FMT_FUNC)
            || get_fmt_enabled(ELOG_FMT_LINE)) {
        log_len += elog_strcpy(log_len, log_buf + log_len, "(");
        /* package time info */
        if (get_fmt_enabled(ELOG_FMT_DIR)) {
            log_len += elog_strcpy(log_len, log_buf + log_len, file);
            if (get_fmt_enabled(ELOG_FMT_FUNC)) {
                log_len += elog_strcpy(log_len, log_buf + log_len, " ");
            } else if (get_fmt_enabled(ELOG_FMT_LINE)) {
                log_len += elog_strcpy(log_len, log_buf + log_len, ":");
            }
        }
        /* package process info */
        if (get_fmt_enabled(ELOG_FMT_FUNC)) {
            log_len += elog_strcpy(log_len, log_buf + log_len, func);
            if (get_fmt_enabled(ELOG_FMT_LINE)) {
                log_len += elog_strcpy(log_len, log_buf + log_len, ":");
            }
        }
        /* package thread info */
        if (get_fmt_enabled(ELOG_FMT_LINE)) {
            //TODO snprintf资源占用可能较高，待优化
            snprintf(line_num, ELOG_LINE_NUM_MAX_LEN, "%ld", line);
            log_len += elog_strcpy(log_len, log_buf + log_len, line_num);
        }
        log_len += elog_strcpy(log_len, log_buf + log_len, ") ");
    }

    /* add space and colon sign */
    if (log_len != 0) {
        log_len += elog_strcpy(log_len, log_buf + log_len, ": ");
    }

    /* package other log data to buffer. CRLF length is 2. */
    fmt_result = vsnprintf(log_buf + log_len, ELOG_BUF_SIZE - log_len - 2, format, args);

    va_end(args);

    /* keyword filter */
    if (!strstr(log_buf, elog.filter.keyword)) {
        //TODO 可以考虑采用KMP及朴素模式匹配字符串，提升性能
        /* unlock output */
        elog_port_output_unlock();
        return;
    }

    /* package end sign( CRLF and '\0' ) */
    if ((fmt_result > -1) && (fmt_result + log_len + 2 < ELOG_BUF_SIZE)) {
        log_len += fmt_result;
        /* add CRLF */
        log_len += elog_strcpy(log_len, log_buf + log_len, "\r\n");
        /* add '\0' */
        log_buf[log_len++] = '\0';

    } else {
        /* add CRLF */
        log_buf[ELOG_BUF_SIZE - 3] = '\r';
        log_buf[ELOG_BUF_SIZE - 2] = '\n';
        /* add log end sign */
        log_buf[ELOG_BUF_SIZE - 1] = '\0';
    }

    /* output log */
    elog_port_output(log_buf, log_len);

    /* unlock output */
    elog_port_output_unlock();
}

/**
 * get format enabled
 *
 * @param set format set
 *
 * @return enable or disable
 */
static bool_t get_fmt_enabled(size_t set) {
    if (elog.enabled_fmt_set & set) {
        return TRUE;
    } else {
        return FALSE;
    }
}
