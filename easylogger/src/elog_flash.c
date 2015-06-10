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
 * Function: Save log to flash. Must use EasyFlash(https://github.com/armink/EasyFlash) library.
 * Created on: 2015-06-05
 */

#include "elog_flash.h"
#include "flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define log_a(...) elog_a("elog.flash", __VA_ARGS__)
#define log_e(...) elog_e("elog.flash", __VA_ARGS__)
#define log_w(...) elog_w("elog.flash", __VA_ARGS__)
#define log_i(...) elog_i("elog.flash", __VA_ARGS__)
#define log_d(...) elog_d("elog.flash", __VA_ARGS__)
#define log_v(...) elog_v("elog.flash", __VA_ARGS__)

#ifdef ELOG_FLASH_USING_BUF_MODE
/* flash log buffer */
static char log_buf[ELOG_FLASH_BUF_SIZE] = { 0 };
/* current flash log buffer write position  */
static size_t cur_buf_size = 0;
#endif

/* initialize OK flag */
static bool init_ok = false;

/**
 * EasyLogger flash save plugin initialize.
 *
 * @return result
 */
ElogErrCode elog_flash_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    /* buffer size must be word alignment */
    ELOG_ASSERT(ELOG_FLASH_BUF_SIZE % 4 == 0)

#ifdef ELOG_FLASH_USING_BUF_MODE
    /* initialize current flash log buffer write position */
    cur_buf_size = 0;
#endif

    /* initialize OK */
    init_ok = true;

    return result;
}

/**
 * Read and output log which saved in flash.
 *
 * @param index index for saved log.
 *        Minimum index is 0.
 *        Maximum index is log used flash total size - 1.
 * @param size
 */
void elog_flash_outout(size_t index, size_t size) {
    /* 128 bytes buffer */
    uint32_t buf[32] = { 0 };
    size_t log_total_size = flash_log_get_used_size();
    size_t buf_szie = sizeof(buf);
    size_t read_size = 0, read_overage_size = 0;

    if (index + size > log_total_size) {
        log_i("The output position and size is out of bound. The max size is %d.", log_total_size);
        return;
    }

    /* must be call this function after initialize OK */
    ELOG_ASSERT(init_ok);
    /* lock flash log buffer */
    elog_flash_port_lock();
    /* Output all flash saved log. It will use filter */
    while (true) {
        if (index + read_size + buf_szie < log_total_size) {
            flash_log_read(index + read_size, buf, buf_szie);
            elog_flash_port_output((const char*)buf, buf_szie);
            read_size += buf_szie;
        } else {
            /* flash read is word alignment */
            if ((log_total_size - index - read_size) % 4 == 0) {
                read_overage_size = 0;
            } else {
                read_overage_size = 4 - ((log_total_size - index - read_size) % 4);
            }
            flash_log_read(index + read_size - read_overage_size, buf,
                    log_total_size - index - read_size + read_overage_size);
            elog_flash_port_output((const char*) buf + read_overage_size,
                    log_total_size - index - read_size);
            //TODO CRLF 后期需要统一在头文件宏定义
            elog_flash_port_output("\r\n", 2);
            break;
        }
    }
    /* unlock flash log buffer */
    elog_flash_port_unlock();
}

/**
 * Read and output all log which saved in flash.
 */
void elog_flash_outout_all(void) {
    elog_flash_outout(0, flash_log_get_used_size());
}

/**
 * Read and output recent log which saved in flash.
 *
 * @param size recent log size
 */
void elog_flash_outout_recent(size_t size) {
    size_t max_size = flash_log_get_used_size();
    if (size > max_size) {
        log_i("The output size is out of bound. The max size is %d.", max_size);
    } else {
        elog_flash_outout(max_size - size, size);
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
    FlashErrCode result = FLASH_NO_ERR;
    /* write some '\r' for word alignment */
    char write_overage_c[4] = { '\r', '\r', '\r', '\r' };
#endif

    /* must be call this function after initialize OK */
    ELOG_ASSERT(init_ok);

    /* lock flash log buffer */
    elog_flash_port_lock();

#ifdef ELOG_FLASH_USING_BUF_MODE
    while (true) {
        if (cur_buf_size + size > ELOG_FLASH_BUF_SIZE) {
            write_size = ELOG_FLASH_BUF_SIZE - cur_buf_size;
            memcpy(log_buf + cur_buf_size, log + write_index, write_size);
            write_index += write_size;
            size -= write_size;
            cur_buf_size += write_size;
            /* unlock flash log buffer */
            elog_flash_port_unlock();
            /* write all buffered log to flash, cur_buf_size will reset */
            elog_flash_flush();
            /* lock flash log buffer */
            elog_flash_port_lock();
        } else {
            memcpy(log_buf + cur_buf_size, log + write_index, size);
            cur_buf_size += size;
            break;
        }
    }
#else
    /* calculate the word alignment write size */
    write_size_temp = size / 4 * 4;
    /* write log to flash */
    result = flash_log_write((uint32_t *) log, write_size_temp);
    /* write last word alignment data */
    if ((result == FLASH_NO_ERR) && (write_size_temp != size)) {
        memcpy(write_overage_c, log + write_size_temp, size - write_size_temp);
        flash_log_write((uint32_t *) write_overage_c, 4);
    }
#endif

    /* unlock flash log buffer */
    elog_flash_port_unlock();
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
    elog_flash_port_lock();
    /* flash write is word alignment */
    if (cur_buf_size % 4 != 0) {
        write_overage_size = 4 - (cur_buf_size % 4);
    }
    /* fill '\r' for word alignment */
    memset(log_buf + cur_buf_size, '\r', write_overage_size);
    /* write all buffered log to flash */
    flash_log_write((uint32_t *) log_buf, cur_buf_size + write_overage_size);
    /* reset position */
    cur_buf_size = 0;
    /* unlock flash log buffer */
    elog_flash_port_unlock();
}
#endif

/**
 * clean all log which in flash and ram buffer
 */
void elog_flash_clean(void) {
    FlashErrCode clean_result = FLASH_NO_ERR;

    /* must be call this function after initialize OK */
    ELOG_ASSERT(init_ok);
    /* lock flash log buffer */
    elog_flash_port_lock();
    /* clean all log which in flash */
    clean_result = flash_log_clean();

#ifdef ELOG_FLASH_USING_BUF_MODE
    /* reset position */
    cur_buf_size = 0;
#endif

    /* unlock flash log buffer */
    elog_flash_port_unlock();

    if(clean_result == FLASH_NO_ERR) {
        log_i("All logs which in flash is clean OK.");
    } else {
        log_e("Clean logs which in flash has an error!");
    }
}
