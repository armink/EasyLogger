/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015-2017, Armink, <armink.ztl@gmail.com>
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
 * Function: Save log to flash. Must use EasyFlash(https://github.com/armink/EasyFlash) library.
 * Created on: 2015-06-05
 */

#define LOG_TAG    "elog.flash"

#include "elog_flash.h"
#include <easyflash.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ELOG_FLASH_USING_BUF_MODE
/* flash log buffer */
static char log_buf[ELOG_FLASH_BUF_SIZE] = { 0 };
/* current flash log buffer write position  */
static size_t cur_buf_size = 0;
#endif

/* initialize OK flag */
static bool init_ok = false;
/* the flash log buffer lock enable or disable. default is enable */
static bool log_buf_lock_enabled = true;
/* the flash log buffer is locked before enable. */
static bool log_buf_is_locked_before_enable = false;
/* the flash log buffer is locked before disable. */
static bool log_buf_is_locked_before_disable = false;
static void log_buf_lock(void);
static void log_buf_unlock(void);

/**
 * EasyLogger flash log plugin initialize.
 *
 * @return result
 */
ElogErrCode elog_flash_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    /* buffer size must be word alignment */
    ELOG_ASSERT(ELOG_FLASH_BUF_SIZE % 4 == 0);

#ifdef ELOG_FLASH_USING_BUF_MODE
    /* initialize current flash log buffer write position */
    cur_buf_size = 0;
#endif

    /* port initialize */
    elog_flash_port_init();
    /* initialize OK */
    init_ok = true;

    return result;
}

/**
 * Read and output log which saved in flash.
 *
 * @param index index for saved log. @note It will auto word alignment.
 *        Minimum index is 0.
 *        Maximum index is log used flash total size - 1.
 * @param size
 */
void elog_flash_output(size_t index, size_t size) {
    /* 128 bytes buffer */
    uint32_t buf[32] = { 0 };
    size_t log_total_size = ef_log_get_used_size();
    size_t buf_size = sizeof(buf);
    size_t read_size = 0, read_overage_size = 0;

    /* word alignment for index */
    index = index / 4 * 4;
    if (index + size > log_total_size) {
        log_i("The output position and size is out of bound. The max size is %d.", log_total_size);
        return;
    }
    /* must be call this function after initialize OK */
    ELOG_ASSERT(init_ok);
    /* lock flash log buffer */
    log_buf_lock();
    /* output all flash saved log. It will use filter */
    while (true) {
        if (read_size + buf_size < size) {
            ef_log_read(index + read_size, buf, buf_size);
            elog_flash_port_output((const char*)buf, buf_size);
            read_size += buf_size;
        } else {
            /* flash read is word alignment */
            if ((size - read_size) % 4 == 0) {
                read_overage_size = 0;
            } else {
                read_overage_size = 4 - ((size - read_size) % 4);
            }
            ef_log_read(index + read_size, buf, size - read_size + read_overage_size);
            elog_flash_port_output((const char*) buf + read_overage_size, size - read_size);
            /* output newline sign */
            elog_flash_port_output(ELOG_NEWLINE_SIGN, strlen(ELOG_NEWLINE_SIGN));
            break;
        }
    }
    /* unlock flash log buffer */
    log_buf_unlock();
}

/**
 * Read and output all log which saved in flash.
 */
void elog_flash_output_all(void) {
    elog_flash_output(0, ef_log_get_used_size());
}

/**
 * Read and output recent log which saved in flash.
 *
 * @param size recent log size
 */
void elog_flash_output_recent(size_t size) {
    size_t max_size = ef_log_get_used_size();

    if (size == 0) {
        return;
    }

    if (size > max_size) {
        log_i("The output size is out of bound. The max size is %d.", max_size);
    } else {
        elog_flash_output(max_size - size, size);
    }
}

/**
 * Write log to flash. The flash write use buffer mode.
 *
 * @param log log
 * @param size log size
 */
void elog_flash_write(const char *log, size_t size) {

#ifdef ELOG_FLASH_USING_BUF_MODE
    size_t write_size = 0, write_index = 0;
#else
    size_t write_size_temp = 0;
    EfErrCode result = EF_NO_ERR;
    /* write some '\r' for word alignment */
    char write_overage_c[4] = { '\r', '\r', '\r', '\r' };
#endif

    /* must be call this function after initialize OK */
    ELOG_ASSERT(init_ok);

    /* lock flash log buffer */
    log_buf_lock();

#ifdef ELOG_FLASH_USING_BUF_MODE
    while (true) {
        if (cur_buf_size + size > ELOG_FLASH_BUF_SIZE) {
            write_size = ELOG_FLASH_BUF_SIZE - cur_buf_size;
            elog_memcpy(log_buf + cur_buf_size, log + write_index, write_size);
            write_index += write_size;
            size -= write_size;
            cur_buf_size += write_size;
            /* unlock flash log buffer */
            log_buf_unlock();
            /* write all buffered log to flash, cur_buf_size will reset */
            elog_flash_flush();
            /* lock flash log buffer */
            log_buf_lock();
        } else {
            elog_memcpy(log_buf + cur_buf_size, log + write_index, size);
            cur_buf_size += size;
            break;
        }
    }
#else
    /* calculate the word alignment write size */
    write_size_temp = size / 4 * 4;
    /* write log to flash */
    result = ef_log_write((uint32_t *) log, write_size_temp);
    /* write last word alignment data */
    if ((result == EF_NO_ERR) && (write_size_temp != size)) {
        elog_memcpy(write_overage_c, log + write_size_temp, size - write_size_temp);
        ef_log_write((uint32_t *) write_overage_c, 4);
    }
#endif

    /* unlock flash log buffer */
    log_buf_unlock();
}

#ifdef ELOG_FLASH_USING_BUF_MODE
/**
 * write all buffered log to flash
 */
void elog_flash_flush(void) {
    size_t write_overage_size = 0;

    /* must be call this function after initialize OK */
    ELOG_ASSERT(init_ok);
    /* lock flash log buffer */
    log_buf_lock();
    /* flash write is word alignment */
    if (cur_buf_size % 4 != 0) {
        write_overage_size = 4 - (cur_buf_size % 4);
    }
    /* fill '\r' for word alignment */
    memset(log_buf + cur_buf_size, '\r', write_overage_size);
    /* write all buffered log to flash */
    ef_log_write((uint32_t *) log_buf, cur_buf_size + write_overage_size);
    /* reset position */
    cur_buf_size = 0;
    /* unlock flash log buffer */
    log_buf_unlock();
}
#endif

/**
 * clean all log which in flash and ram buffer
 */
void elog_flash_clean(void) {
    EfErrCode clean_result = EF_NO_ERR;

    /* must be call this function after initialize OK */
    ELOG_ASSERT(init_ok);
    /* lock flash log buffer */
    log_buf_lock();
    /* clean all log which in flash */
    clean_result = ef_log_clean();

#ifdef ELOG_FLASH_USING_BUF_MODE
    /* reset position */
    cur_buf_size = 0;
#endif

    /* unlock flash log buffer */
    log_buf_unlock();

    if(clean_result == EF_NO_ERR) {
        log_i("All logs which in flash is clean OK.");
    } else {
        log_e("Clean logs which in flash has an error!");
    }
}

/**
 * enable or disable flash plugin lock
 * @note disable this lock is not recommended except you want output system exception log
 *
 * @param enabled true: enable  false: disable
 */
void elog_flash_lock_enabled(bool enabled) {
    log_buf_lock_enabled = enabled;
    /* it will re-lock or re-unlock before log buffer lock enable */
    if (log_buf_lock_enabled) {
        if (!log_buf_is_locked_before_disable && log_buf_is_locked_before_enable) {
            /* the log buffer lock is unlocked before disable, and the lock will unlocking after enable */
            elog_flash_port_lock();
        } else if (log_buf_is_locked_before_disable && !log_buf_is_locked_before_enable) {
            /* the log buffer lock is locked before disable, and the lock will locking after enable */
            elog_flash_port_unlock();
        }
    }
}

/**
 * lock flash log buffer
 */
static void log_buf_lock(void) {
    if (log_buf_lock_enabled) {
        elog_flash_port_lock();
        log_buf_is_locked_before_disable = true;
    } else {
        log_buf_is_locked_before_enable = true;
    }
}
/**
 * unlock flash log buffer
 */
static void log_buf_unlock(void) {
    if (log_buf_lock_enabled) {
        elog_flash_port_unlock();
        log_buf_is_locked_before_disable = false;
    } else {
        log_buf_is_locked_before_enable = false;
    }
}
