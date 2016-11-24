/*
 * This file is part of the EasyFlash Library.
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
 * Function: Save logs to flash.
 * Created on: 2015-06-04
 */

#include <easyflash.h>

#ifdef EF_USING_LOG

/* the stored logs start address and end address. It's like a ring buffer which implement by flash. */
static uint32_t log_start_addr = 0, log_end_addr = 0;
/* saved log area address for flash */
static uint32_t log_area_start_addr = 0;
/* initialize OK flag */
static bool init_ok = false;

static void find_start_and_end_addr(void);
static uint32_t get_next_flash_sec_addr(uint32_t cur_addr);

/**
 * The flash save log function initialize.
 *
 * @return result
 */
EfErrCode ef_log_init(void) {
    EfErrCode result = EF_NO_ERR;

    EF_ASSERT(LOG_AREA_SIZE);
    EF_ASSERT(EF_ERASE_MIN_SIZE);
    /* the log area size must be an integral multiple of erase minimum size. */
    EF_ASSERT(LOG_AREA_SIZE % EF_ERASE_MIN_SIZE == 0);
    /* the log area size must be more than twice of EF_ERASE_MIN_SIZE */
    EF_ASSERT(LOG_AREA_SIZE / EF_ERASE_MIN_SIZE >= 2);

#ifdef EF_USING_ENV
    log_area_start_addr = EF_START_ADDR + ENV_AREA_SIZE;
#else
    log_area_start_addr = EF_START_ADDR;
#endif

    /* find the log store start address and end address */
    find_start_and_end_addr();
    /* initialize OK */
    init_ok = true;

    return result;
}

/**
 * Find the log store start address and end address.
 * It's like a ring buffer which implement by flash.
 * The flash log area has two state when find start address and end address.
 *                       state 1                                state 2
 *                   |============|                         |============|
 * log area start--> |############| <-- start address       |############| <-- end address
 *                   |############|                         |   empty    |
 *                   |------------|                         |------------|
 *                   |############|                         |############| <-- start address
 *                   |############|                         |############|
 *                   |------------|                         |------------|
 *                   |     .      |                         |     .      |
 *                   |     .      |                         |     .      |
 *                   |     .      |                         |     .      |
 *                   |------------|                         |------------|
 *                   |############| <-- end address         |############|
 *                   |   empty    |                         |############|
 *  log area end --> |============|                         |============|
 *
 *  LOG_AREA_SIZE = log area end - log area star
 *
 */
static void find_start_and_end_addr(void) {
    size_t cur_size = 0;
    EfSecrorStatus cur_sec_status, last_sec_status;
    uint32_t cur_using_sec_addr = 0;
    /* all status sector counts */
    size_t empty_sec_counts = 0, using_sec_counts = 0, full_sector_counts = 0;
    /* total sector number */
    size_t total_sec_num = LOG_AREA_SIZE / EF_ERASE_MIN_SIZE;
    /* see comment of find_start_and_end_addr function */
    uint8_t cur_log_sec_state = 0;

    /* get the first sector status */
    cur_sec_status = ef_get_sector_status(log_area_start_addr, EF_ERASE_MIN_SIZE);
    last_sec_status = cur_sec_status;

    for (cur_size = EF_ERASE_MIN_SIZE; cur_size < LOG_AREA_SIZE; cur_size += EF_ERASE_MIN_SIZE) {
        /* get current sector status */
        cur_sec_status = ef_get_sector_status(log_area_start_addr + cur_size, EF_ERASE_MIN_SIZE);
        /* compare last and current status */
        switch (last_sec_status) {
        case EF_SECTOR_EMPTY: {
            switch (cur_sec_status) {
            case EF_SECTOR_EMPTY:
                break;
            case EF_SECTOR_USING:
                EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                ef_log_clean();
                return;
            case EF_SECTOR_FULL:
                EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                ef_log_clean();
                return;
            }
            empty_sec_counts++;
            break;
        }
        case EF_SECTOR_USING: {
            switch (cur_sec_status) {
            case EF_SECTOR_EMPTY:
                /* like state 1 */
                cur_log_sec_state = 1;
                log_start_addr = log_area_start_addr;
                cur_using_sec_addr = log_area_start_addr + cur_size - EF_ERASE_MIN_SIZE;
                break;
            case EF_SECTOR_USING:
                EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                ef_log_clean();
                return;
            case EF_SECTOR_FULL:
                /* like state 2 */
                cur_log_sec_state = 2;
                log_start_addr = log_area_start_addr + cur_size;
                cur_using_sec_addr = log_area_start_addr + cur_size - EF_ERASE_MIN_SIZE;
                break;
            }
            using_sec_counts++;
            break;
        }
        case EF_SECTOR_FULL: {
            switch (cur_sec_status) {
            case EF_SECTOR_EMPTY:
                /* like state 1 */
                if (cur_log_sec_state == 2) {
                    EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                    ef_log_clean();
                    return;
                } else {
                    cur_log_sec_state = 1;
                    log_start_addr = log_area_start_addr;
                    /* word alignment */
                    log_end_addr = log_area_start_addr + cur_size - 4;
                    cur_using_sec_addr = log_area_start_addr + cur_size - EF_ERASE_MIN_SIZE;
                }
                break;
            case EF_SECTOR_USING:
                if(total_sec_num <= 2) {
                    /* like state 1 */
                    cur_log_sec_state = 1;
                    log_start_addr = log_area_start_addr;
                    cur_using_sec_addr = log_area_start_addr + cur_size;
                } else {
                    /* like state 2 when the sector is the last one */
                    if (cur_size + EF_ERASE_MIN_SIZE >= LOG_AREA_SIZE) {
                        cur_log_sec_state = 2;
                        log_start_addr = log_area_start_addr + cur_size;
                        cur_using_sec_addr = log_area_start_addr + cur_size - EF_ERASE_MIN_SIZE;
                    }
                }
                break;
            case EF_SECTOR_FULL:
                break;
            }
            full_sector_counts++;
            break;
        }
        }
        last_sec_status = cur_sec_status;
    }

    /* the last sector status counts */
    if (cur_sec_status == EF_SECTOR_EMPTY) {
        empty_sec_counts++;
    } else if (cur_sec_status == EF_SECTOR_USING) {
        using_sec_counts++;
    } else if (cur_sec_status == EF_SECTOR_FULL) {
        full_sector_counts++;
    }

    if (using_sec_counts > 1) {
        EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
        ef_log_clean();
        return;
    } else if (empty_sec_counts == total_sec_num) {
        log_start_addr = log_end_addr = log_area_start_addr;
    } else if (full_sector_counts == total_sec_num) {
        /* this state is almost impossible */
        EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
        ef_log_clean();
        return;
    } else if (((cur_log_sec_state == 1) && (cur_using_sec_addr != 0))
            || (cur_log_sec_state == 2)) {
        /* find the end address */
        log_end_addr = ef_find_sec_using_end_addr(cur_using_sec_addr, EF_ERASE_MIN_SIZE);
    }
}

/**
 * Get log used flash total size.
 *
 * @return log used flash total size
 */
size_t ef_log_get_used_size(void) {
    EF_ASSERT(log_start_addr);
    EF_ASSERT(log_end_addr);

    /* must be call this function after initialize OK */
    EF_ASSERT(init_ok);

    if (log_start_addr < log_end_addr) {
        return log_end_addr - log_start_addr + 4;
    } else if (log_start_addr > log_end_addr) {
        return LOG_AREA_SIZE - (log_start_addr - log_end_addr) + 4;
    } else {
        return 0;
    }
}

/**
 * Read log from flash.
 *
 * @param index index for saved log.
 *        Minimum index is 0.
 *        Maximum index is log used flash total size - 1.
 * @param log the log which will read from flash
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_log_read(size_t index, uint32_t *log, size_t size) {
    EfErrCode result = EF_NO_ERR;
    size_t cur_using_size = ef_log_get_used_size();
    size_t read_size_temp = 0;

    EF_ASSERT(size % 4 == 0);
    EF_ASSERT(index + size <= cur_using_size);
    /* must be call this function after initialize OK */
    EF_ASSERT(init_ok);

    if (!size) {
        return result;
    }

    if (log_start_addr < log_end_addr) {
        result = ef_port_read(log_area_start_addr + index, log, size);
    } else if (log_start_addr > log_end_addr) {
        if (log_start_addr + index + size <= log_area_start_addr + LOG_AREA_SIZE) {
            /*                          Flash log area
             *                         |--------------|
             * log_area_start_addr --> |##############|
             *                         |##############|
             *                         |##############|
             *                         |--------------|
             *                         |##############|
             *                         |##############|
             *                         |##############| <-- log_end_addr
             *                         |--------------|
             *      log_start_addr --> |##############|
             *          read start --> |**************| <-- read end
             *                         |##############|
             *                         |--------------|
             *
             * read from (log_start_addr + index) to (log_start_addr + index + size)
             */
            result = ef_port_read(log_start_addr + index, log, size);
        } else if (log_start_addr + index < log_area_start_addr + LOG_AREA_SIZE) {
            /*                          Flash log area
             *                         |--------------|
             * log_area_start_addr --> |**************| <-- read end
             *                         |##############|
             *                         |##############|
             *                         |--------------|
             *                         |##############|
             *                         |##############|
             *                         |##############| <-- log_end_addr
             *                         |--------------|
             *      log_start_addr --> |##############|
             *          read start --> |**************|
             *                         |**************|
             *                         |--------------|
             * read will by 2 steps
             * step1: read from (log_start_addr + index) to flash log area end address
             * step2: read from flash log area start address to read size's end address
             */
            read_size_temp = (log_area_start_addr + LOG_AREA_SIZE) - (log_start_addr + index);
            result = ef_port_read(log_start_addr + index, log, read_size_temp);
            if (result == EF_NO_ERR) {
                result = ef_port_read(log_area_start_addr, log + read_size_temp,
                        size - read_size_temp);
            }
        } else {
            /*                          Flash log area
             *                         |--------------|
             * log_area_start_addr --> |##############|
             *          read start --> |**************|
             *                         |**************| <-- read end
             *                         |--------------|
             *                         |##############|
             *                         |##############|
             *                         |##############| <-- log_end_addr
             *                         |--------------|
             *      log_start_addr --> |##############|
             *                         |##############|
             *                         |##############|
             *                         |--------------|
             * read from (log_start_addr + index - LOG_AREA_SIZE) to read size's end address
             */
            result = ef_port_read(log_start_addr + index - LOG_AREA_SIZE, log, size);
        }
    }

    return result;
}

/**
 * Write log to flash.
 *
 * @param log the log which will be write to flash
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_log_write(const uint32_t *log, size_t size) {
    EfErrCode result = EF_NO_ERR;
    size_t cur_using_size = ef_log_get_used_size(), write_size = 0, writable_size = 0;
    uint32_t write_addr, erase_addr;

    EF_ASSERT(size % 4 == 0);
    /* must be call this function after initialize OK */
    EF_ASSERT(init_ok);

    /* write address is after log end address when LOG AREA isn't empty */
    if (log_start_addr != log_end_addr) {
        write_addr = log_end_addr + 4;
    } else {
        write_addr = log_start_addr;
    }
    /* write the already erased but not used area */
    writable_size = EF_ERASE_MIN_SIZE - ((write_addr - log_area_start_addr) % EF_ERASE_MIN_SIZE);
    if ((writable_size != EF_ERASE_MIN_SIZE) || (log_start_addr == log_end_addr)) {
        if (size > writable_size) {
            result = ef_port_write(write_addr, log, writable_size);
            if (result != EF_NO_ERR) {
                goto exit;
            }
            write_size += writable_size;
        } else {
            result = ef_port_write(write_addr, log, size);
            log_end_addr = write_addr + size - 4;
            goto exit;
        }
    }
    /* erase and write remain log */
    while (true) {
        /* calculate next available sector address */
        erase_addr = write_addr = get_next_flash_sec_addr(write_addr - 4);
        /* move the flash log start address to next available sector address */
        if (log_start_addr == erase_addr) {
            log_start_addr = get_next_flash_sec_addr(log_start_addr);
        }
        /* erase sector */
        result = ef_port_erase(erase_addr, EF_ERASE_MIN_SIZE);
        if (result == EF_NO_ERR) {
            if (size - write_size > EF_ERASE_MIN_SIZE) {
                result = ef_port_write(write_addr, log + write_size / 4, EF_ERASE_MIN_SIZE);
                if (result != EF_NO_ERR) {
                    goto exit;
                }
                log_end_addr = write_addr + EF_ERASE_MIN_SIZE - 4;
                write_size += EF_ERASE_MIN_SIZE;
                write_addr += EF_ERASE_MIN_SIZE;
            } else {
                result = ef_port_write(write_addr, log + write_size / 4, size - write_size);
                if (result != EF_NO_ERR) {
                    goto exit;
                }
                log_end_addr = write_addr + (size - write_size) - 4;
                break;
            }
        } else {
            goto exit;
        }
    }

exit:
    return result;
}

/**
 * Get next flash sector address.The log total sector like ring buffer which implement by flash.
 *
 * @param cur_addr cur flash address
 *
 * @return next flash sector address
 */
static uint32_t get_next_flash_sec_addr(uint32_t cur_addr) {
    size_t cur_sec_id = (cur_addr - log_area_start_addr) / EF_ERASE_MIN_SIZE;
    size_t sec_total_num = LOG_AREA_SIZE / EF_ERASE_MIN_SIZE;
    if (cur_sec_id + 1 >= sec_total_num) {
        /* return to ring head */
        return log_area_start_addr;
    } else {
        return log_area_start_addr + (cur_sec_id + 1) * EF_ERASE_MIN_SIZE;
    }
}

/**
 * Clean all log which in flash.
 *
 * @return result
 */
EfErrCode ef_log_clean(void) {
    EfErrCode result = EF_NO_ERR;

    EF_ASSERT(log_area_start_addr);

    /* clean address */
    log_start_addr = log_end_addr = log_area_start_addr;
    /* erase log flash area */
    result = ef_port_erase(log_area_start_addr, LOG_AREA_SIZE);

    return result;
}

#endif /* EF_USING_LOG */
