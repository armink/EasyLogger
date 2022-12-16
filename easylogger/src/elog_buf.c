/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2016, Armink, <armink.ztl@gmail.com>
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
 * Function: Logs buffered output.
 * Created on: 2016-11-09
 */

#include <elog.h>
#include <string.h>

#ifdef ELOG_BUF_OUTPUT_ENABLE
#if !defined(ELOG_BUF_OUTPUT_BUF_SIZE)
    #error "Please configure buffer size for buffered output mode (in elog_cfg.h)"
#endif

/* buffered output mode's buffer */
static char log_buf[ELOG_BUF_OUTPUT_BUF_SIZE] = { 0 };
/* log buffer current write size */
static size_t buf_write_size = 0;
/* buffered output mode enabled flag */
static bool is_enabled = false;

extern void elog_port_output(const char *log, size_t size);
extern void elog_output_lock(void);
extern void elog_output_unlock(void);

/**
 * output buffered logs when buffer is full
 *
 * @param log will be buffered line's log
 * @param size log size
 */
void elog_buf_output(const char *log, size_t size) {
    size_t write_size = 0, write_index = 0;

    if (!is_enabled) {
        elog_port_output(log, size);
        return;
    }

    while (true) {
        if (buf_write_size + size > ELOG_BUF_OUTPUT_BUF_SIZE) {
            write_size = ELOG_BUF_OUTPUT_BUF_SIZE - buf_write_size;
            memcpy(log_buf + buf_write_size, log + write_index, write_size);
            write_index += write_size;
            size -= write_size;
            /* output log */
            elog_port_output(log_buf, ELOG_BUF_OUTPUT_BUF_SIZE);
            /* reset write index */
            buf_write_size = 0;
        } else {
            memcpy(log_buf + buf_write_size, log + write_index, size);
            buf_write_size += size;
            break;
        }
    }
}

/**
 * flush all buffered logs to output device
 */
void elog_flush(void) {
    if (buf_write_size == 0)
        return;
    /* lock output */
    elog_output_lock();
    /* output log */
    elog_port_output(log_buf, buf_write_size);
    /* reset write index */
    buf_write_size = 0;
    /* unlock output */
    elog_output_unlock();
}

/**
 * enable or disable buffered output mode
 * the log will be output directly when mode is disabled
 *
 * @param enabled true: enabled, false: disabled
 */
void elog_buf_enabled(bool enabled) {
    is_enabled = enabled;
}
#endif /* ELOG_BUF_OUTPUT_ENABLE */
