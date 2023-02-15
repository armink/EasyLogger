/*
 * This file is part of the EasyFlash Library.
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
 * Function: Save logs to flash.
 * Created on: 2015-06-04
 */

#include <easyflash.h>

#ifdef EF_USING_LOG

#if defined(EF_USING_LOG) && !defined(LOG_AREA_SIZE)
#error "Please configure log area size (in ef_cfg.h)"
#endif

/* magic code on every sector header. 'EF' is 0xEF30EF30 */
#define LOG_SECTOR_MAGIC               0xEF30EF30
/* sector header size, includes the sector magic code and status magic code */
#define LOG_SECTOR_HEADER_SIZE         12
/* sector header word size,what is equivalent to the total number of sectors header index */
#define LOG_SECTOR_HEADER_WORD_SIZE    3

/**
 * Sector status magic code
 * The sector status is 8B after LOG_SECTOR_MAGIC at every sector header.
 * ==============================================
 * |           header(12B)            | status |
 * ----------------------------------------------
 * | 0xEF30EF30 0xFFFFFFFF 0xFFFFFFFF |  empty |
 * | 0xEF30EF30 0xFEFEFEFE 0xFFFFFFFF |  using |
 * | 0xEF30EF30 0xFEFEFEFE 0xFCFCFCFC |  full  |
 * ==============================================
 *
 * State transition relationship: empty->using->full
 * The FULL status will change to EMPTY after sector clean.
 */
#define SECTOR_STATUS_MAGIC_EMPUT     0xFFFFFFFF
#define SECTOR_STATUS_MAGIC_USING     0xFEFEFEFE
#define SECTOR_STATUS_MAGIC_FULL      0xFCFCFCFC

typedef enum {
    SECTOR_STATUS_EMPUT,
    SECTOR_STATUS_USING,
    SECTOR_STATUS_FULL,
    SECTOR_STATUS_HEADER_ERROR,
} SectorStatus;

typedef enum {
    SECTOR_HEADER_MAGIC_INDEX,
    SECTOR_HEADER_USING_INDEX,
    SECTOR_HEADER_FULL_INDEX,
} SectorHeaderIndex;

/* the stored logs start address and end address. It's like a ring buffer implemented on flash. */
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
 * Get flash sector current status.
 *
 * @param addr sector address, this function will auto calculate the sector header address by this address.
 *
 * @return the flash sector current status
 */
static SectorStatus get_sector_status(uint32_t addr) {
    uint32_t header_buf[LOG_SECTOR_HEADER_WORD_SIZE] = {0}, header_addr = 0;
    uint32_t sector_header_magic = 0;
    uint32_t status_full_magic = 0, status_use_magic = 0;

    /* calculate the sector header address */
    header_addr = addr & (~(EF_ERASE_MIN_SIZE - 1));

    if (ef_port_read(header_addr, header_buf, sizeof(header_buf)) == EF_NO_ERR) {
        sector_header_magic = header_buf[SECTOR_HEADER_MAGIC_INDEX];
        status_use_magic = header_buf[SECTOR_HEADER_USING_INDEX];
        status_full_magic = header_buf[SECTOR_HEADER_FULL_INDEX];
    } else {
        EF_DEBUG("Error: Read sector header data error.\n");
        return SECTOR_STATUS_HEADER_ERROR;
    }

    /* compare header magic code */
    if(sector_header_magic == LOG_SECTOR_MAGIC){
        if((status_use_magic == SECTOR_STATUS_MAGIC_EMPUT) && (status_full_magic == SECTOR_STATUS_MAGIC_EMPUT)) {
            return SECTOR_STATUS_EMPUT;
        } else if((status_use_magic == SECTOR_STATUS_MAGIC_USING) && (status_full_magic == SECTOR_STATUS_MAGIC_EMPUT)) {
             return SECTOR_STATUS_USING;
        } else if((status_use_magic == SECTOR_STATUS_MAGIC_USING) && (status_full_magic == SECTOR_STATUS_MAGIC_FULL)) {
             return SECTOR_STATUS_FULL;
        } else {
            return SECTOR_STATUS_HEADER_ERROR;
        }
    } else {
        return SECTOR_STATUS_HEADER_ERROR;
    }

}

/**
 * Write flash sector current status.
 *
 * @param addr sector address, this function will auto calculate the sector header address by this address.
 * @param status sector cur status
 *
 * @return result
 */
static EfErrCode write_sector_status(uint32_t addr, SectorStatus status) {
    uint32_t header, header_addr = 0;

    /* calculate the sector header address */
    header_addr = addr & (~(EF_ERASE_MIN_SIZE - 1));

    /* calculate the sector staus magic */
    switch (status) {
    case SECTOR_STATUS_EMPUT: {
        header = LOG_SECTOR_MAGIC;
        return ef_port_write(header_addr, &header, sizeof(header));
    }
    case SECTOR_STATUS_USING: {
        header = SECTOR_STATUS_MAGIC_USING;
        return ef_port_write(header_addr + sizeof(header), &header, sizeof(header));
    }
    case SECTOR_STATUS_FULL: {
        header = SECTOR_STATUS_MAGIC_FULL;
        return ef_port_write(header_addr + sizeof(header) * 2, &header, sizeof(header));
    }
    default:
        return EF_WRITE_ERR;
    }
}

/**
 * Find the current flash sector using end address by continuous 0xFF.
 *
 * @param addr sector address
 *
 * @return current flash sector using end address
 */
static uint32_t find_sec_using_end_addr(uint32_t addr) {
/* read section data buffer size */
#define READ_BUF_SIZE                32

    uint32_t sector_start = addr, data_start = addr, continue_ff = 0, read_buf_size = 0, i;
    uint8_t buf[READ_BUF_SIZE];

    EF_ASSERT(READ_BUF_SIZE % 4 == 0);
    /* calculate the sector start and data start address */
    sector_start = addr & (~(EF_ERASE_MIN_SIZE - 1));
    data_start = sector_start + LOG_SECTOR_HEADER_SIZE;

    /* counts continuous 0xFF which is end of sector */
    while (data_start < sector_start + EF_ERASE_MIN_SIZE) {
        if (data_start + READ_BUF_SIZE < sector_start + EF_ERASE_MIN_SIZE) {
            read_buf_size = READ_BUF_SIZE;
        } else {
            read_buf_size = sector_start + EF_ERASE_MIN_SIZE - data_start;
        }
        ef_port_read(data_start, (uint32_t *)buf, read_buf_size);
        for (i = 0; i < read_buf_size; i++) {
            if (buf[i] == 0xFF) {
                continue_ff++;
            } else {
                continue_ff = 0;
            }
        }
        data_start += read_buf_size;
    }
    /* calculate current flash sector using end address */
    if (continue_ff >= EF_ERASE_MIN_SIZE - LOG_SECTOR_HEADER_SIZE) {
        /* from 0 to sec_size all sector is 0xFF, so the sector is empty */
        return sector_start + LOG_SECTOR_HEADER_SIZE;
    } else if (continue_ff >= 4) {
        /* form end_addr - 4 to sec_size length all area is 0xFF, so it's used part of the sector.
         * the address must be word alignment. */
        if (continue_ff % 4 != 0) {
            continue_ff = (continue_ff / 4 + 1) * 4;
        }
        return sector_start + EF_ERASE_MIN_SIZE - continue_ff;
    } else {
        /* all sector not has continuous 0xFF, so the sector is full */
        return sector_start + EF_ERASE_MIN_SIZE;
    }
}

/**
 * Find the log store start address and end address.
 * It's like a ring buffer implemented on flash.
 * The flash log area can be in two states depending on start address and end address:
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
    SectorStatus cur_sec_status, last_sec_status;
    uint32_t cur_using_sec_addr = 0;
    /* all status sector counts */
    size_t empty_sec_counts = 0, using_sec_counts = 0, full_sector_counts = 0;
    /* total sector number */
    size_t total_sec_num = LOG_AREA_SIZE / EF_ERASE_MIN_SIZE;
    /* see comment of find_start_and_end_addr function */
    uint8_t cur_log_sec_state = 0;

    /* get the first sector status */
    cur_sec_status = get_sector_status(log_area_start_addr);
    last_sec_status = cur_sec_status;

    for (cur_size = EF_ERASE_MIN_SIZE; cur_size < LOG_AREA_SIZE; cur_size += EF_ERASE_MIN_SIZE) {
        /* get current sector status */
        cur_sec_status = get_sector_status(log_area_start_addr + cur_size);
        /* compare last and current status */
        switch (last_sec_status) {
        case SECTOR_STATUS_EMPUT: {
            switch (cur_sec_status) {
            case SECTOR_STATUS_EMPUT:
                break;
            case SECTOR_STATUS_USING:
                EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                ef_log_clean();
                return;
            case SECTOR_STATUS_FULL:
                EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                ef_log_clean();
                return;
            }
            empty_sec_counts++;
            break;
        }
        case SECTOR_STATUS_USING: {
            switch (cur_sec_status) {
            case SECTOR_STATUS_EMPUT:
                /* like state 1 */
                cur_log_sec_state = 1;
                log_start_addr = log_area_start_addr;
                cur_using_sec_addr = log_area_start_addr + cur_size - EF_ERASE_MIN_SIZE;
                break;
            case SECTOR_STATUS_USING:
                EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                ef_log_clean();
                return;
            case SECTOR_STATUS_FULL:
                /* like state 2 */
                cur_log_sec_state = 2;
                log_start_addr = log_area_start_addr + cur_size;
                cur_using_sec_addr = log_area_start_addr + cur_size - EF_ERASE_MIN_SIZE;
                break;
            }
            using_sec_counts++;
            break;
        }
        case SECTOR_STATUS_FULL: {
            switch (cur_sec_status) {
            case SECTOR_STATUS_EMPUT:
                /* like state 1 */
                if (cur_log_sec_state == 2) {
                    EF_DEBUG("Error: Log area error! Now will clean all log area.\n");
                    ef_log_clean();
                    return;
                } else {
                    cur_log_sec_state = 1;
                    log_start_addr = log_area_start_addr;
                    log_end_addr = log_area_start_addr + cur_size;
                    cur_using_sec_addr = log_area_start_addr + cur_size - EF_ERASE_MIN_SIZE;
                }
                break;
            case SECTOR_STATUS_USING:
                if(total_sec_num <= 2) {
                    /* like state 1 */
                    cur_log_sec_state = 1;
                    log_start_addr = log_area_start_addr;
                    cur_using_sec_addr = log_area_start_addr + cur_size;
                } else {
                    /* like state 2 when the sector is the last one */
                    if (cur_size + EF_ERASE_MIN_SIZE >= LOG_AREA_SIZE) {
                        cur_log_sec_state = 2;
                        log_start_addr = get_next_flash_sec_addr(log_area_start_addr + cur_size);
                        cur_using_sec_addr = log_area_start_addr + cur_size;
                    }
                }
                break;
            case SECTOR_STATUS_FULL:
                break;
            }
            full_sector_counts++;
            break;
        }
        case SECTOR_STATUS_HEADER_ERROR:
            EF_DEBUG("Error: Log sector header error! Now will clean all log area.\n");
            ef_log_clean();
            return;
        }
        last_sec_status = cur_sec_status;
    }

    /* the last sector status counts */
    if (cur_sec_status == SECTOR_STATUS_EMPUT) {
        empty_sec_counts++;
    } else if (cur_sec_status == SECTOR_STATUS_USING) {
        using_sec_counts++;
    } else if (cur_sec_status == SECTOR_STATUS_FULL) {
        full_sector_counts++;
    } else if (cur_sec_status == SECTOR_STATUS_HEADER_ERROR) {
        EF_DEBUG("Error: Log sector header error! Now will clean all log area.\n");
        ef_log_clean();
        return;
    }

    if (using_sec_counts != 1) {
        /* this state is almost impossible */
        EF_DEBUG("Error: There must be only one sector status is USING! Now will clean all log area.\n");
        ef_log_clean();
    } else {
        /* find the end address */
        log_end_addr = find_sec_using_end_addr(cur_using_sec_addr);
    }

}

/**
 * Get log used flash total size.
 *
 * @return log used flash total size. @note NOT contain sector headers
 */
size_t ef_log_get_used_size(void) {
    size_t header_total_num = 0, physical_size = 0;
    /* must be call this function after initialize OK */
    if (!init_ok) {
        return 0;
    }

    if (log_start_addr < log_end_addr) {
        physical_size = log_end_addr - log_start_addr;
    } else {
        physical_size = LOG_AREA_SIZE - (log_start_addr - log_end_addr);
    }

    header_total_num = physical_size / EF_ERASE_MIN_SIZE + 1;

    return physical_size - header_total_num * LOG_SECTOR_HEADER_SIZE;
}

/**
 * Sequential reading log data. It will ignore sector headers.
 *
 * @param addr address
 * @param log log buffer
 * @param size log size, not contain sector headers.
 *
 * @return result
 */
static EfErrCode log_seq_read(uint32_t addr, uint32_t *log, size_t size) {
    EfErrCode result = EF_NO_ERR;
    size_t read_size = 0, read_size_temp = 0;

    while (size) {
        /* move to sector data address */
        if ((addr + read_size) % EF_ERASE_MIN_SIZE == 0) {
            addr += LOG_SECTOR_HEADER_SIZE;
        }
        /* calculate current sector last data size */
        read_size_temp = EF_ERASE_MIN_SIZE - (addr % EF_ERASE_MIN_SIZE);
        if (size < read_size_temp) {
            read_size_temp = size;
        }
        result = ef_port_read(addr + read_size, log + read_size / 4, read_size_temp);
        if (result != EF_NO_ERR) {
            return result;
        }
        read_size += read_size_temp;
        size -= read_size_temp;
    }

    return result;
}

/**
 * Calculate flash physical address by log index.
 *
 * @param index log index
 *
 * @return flash physical address
 */
static uint32_t log_index2addr(size_t index) {
    size_t header_total_offset = 0;
    /* total include sector number */
    size_t sector_num = index / (EF_ERASE_MIN_SIZE - LOG_SECTOR_HEADER_SIZE) + 1;

    header_total_offset = sector_num * LOG_SECTOR_HEADER_SIZE;
    if (log_start_addr < log_end_addr) {
        return log_start_addr + index + header_total_offset;
    } else {
        if (log_start_addr + index + header_total_offset < log_area_start_addr + LOG_AREA_SIZE) {
            return log_start_addr + index + header_total_offset;
        } else {
            return log_start_addr + index + header_total_offset - LOG_AREA_SIZE;

        }
    }
}

/**
 * Read log from flash.
 *
 * @param index index for saved log.
 *        Minimum index is 0.
 *        Maximum index is ef_log_get_used_size() - 1.
 * @param log the log which will read from flash
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_log_read(size_t index, uint32_t *log, size_t size) {
    EfErrCode result = EF_NO_ERR;
    size_t cur_using_size = ef_log_get_used_size();
    size_t read_size_temp = 0;
    size_t header_total_num = 0;

    if (!size) {
        return result;
    }

    EF_ASSERT(size % 4 == 0);
    EF_ASSERT(index < cur_using_size);

    if (index + size > cur_using_size) {
        EF_DEBUG("Warning: Log read size out of bound. Cut read size.\n");
        size = cur_using_size - index;
    }
    /* must be call this function after initialize OK */
    if (!init_ok) {
        return EF_ENV_INIT_FAILED;
    }

    if (log_start_addr < log_end_addr) {
        log_seq_read(log_index2addr(index), log, size);
    } else {
        if (log_index2addr(index) + size <= log_area_start_addr + LOG_AREA_SIZE) {
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
             * read from (log_start_addr + log_index2addr(index)) to (log_start_addr + index + log_index2addr(index))
             */
            result = log_seq_read(log_index2addr(index), log, size);
        } else if (log_index2addr(index) < log_area_start_addr + LOG_AREA_SIZE) {
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
             * step1: read from (log_start_addr + log_index2addr(index)) to flash log area end address
             * step2: read from flash log area start address to read size's end address
             */
            read_size_temp = (log_area_start_addr + LOG_AREA_SIZE) - log_index2addr(index);
            header_total_num = read_size_temp / EF_ERASE_MIN_SIZE;
            /* Minus some ignored bytes */
            read_size_temp -= header_total_num * LOG_SECTOR_HEADER_SIZE;
            result = log_seq_read(log_index2addr(index), log, read_size_temp);
            if (result == EF_NO_ERR) {
                result = log_seq_read(log_area_start_addr, log + read_size_temp / 4, size - read_size_temp);
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
             * read from (log_start_addr + log_index2addr(index) - LOG_AREA_SIZE) to read size's end address
             */
            result = log_seq_read(log_index2addr(index) - LOG_AREA_SIZE, log, size);
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
    size_t write_size = 0, writable_size = 0;
    uint32_t write_addr = log_end_addr, erase_addr;
    SectorStatus sector_status;

    EF_ASSERT(size % 4 == 0);
    /* must be call this function after initialize OK */
    if (!init_ok) {
        return EF_ENV_INIT_FAILED;
    }

    if ((sector_status = get_sector_status(write_addr)) == SECTOR_STATUS_HEADER_ERROR) {
        return EF_WRITE_ERR;
    }
    /* write some log when current sector status is USING and EMPTY */
    if ((sector_status == SECTOR_STATUS_USING) || (sector_status == SECTOR_STATUS_EMPUT)) {
        /* write the already erased but not used area */
        writable_size = EF_ERASE_MIN_SIZE - ((write_addr - log_area_start_addr) % EF_ERASE_MIN_SIZE);
        if (size >= writable_size) {
            result = ef_port_write(write_addr, log, writable_size);
            if (result != EF_NO_ERR) {
                goto exit;
            }
            /* change the current sector status to FULL */
            result = write_sector_status(write_addr, SECTOR_STATUS_FULL);
            if (result != EF_NO_ERR) {
                goto exit;
            }
            write_size += writable_size;
        } else {
            result = ef_port_write(write_addr, log, size);
            log_end_addr = write_addr + size;
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
        if (result != EF_NO_ERR) {
            goto exit;
        }
        /* change the sector status to EMPTY and USING when write begin sector start address */
        result = write_sector_status(write_addr, SECTOR_STATUS_EMPUT);
        result = write_sector_status(write_addr, SECTOR_STATUS_USING);
        if (result == EF_NO_ERR) {
            write_addr += LOG_SECTOR_HEADER_SIZE;
        } else {
            goto exit;
        }
        /* calculate current sector writable data size */
        writable_size = EF_ERASE_MIN_SIZE - LOG_SECTOR_HEADER_SIZE;
        if (size - write_size >= writable_size) {
            result = ef_port_write(write_addr, log + write_size / 4, writable_size);
            if (result != EF_NO_ERR) {
                goto exit;
            }
            /* change the current sector status to FULL */
            result = write_sector_status(write_addr, SECTOR_STATUS_FULL);
            if (result != EF_NO_ERR) {
                goto exit;
            }
            log_end_addr = write_addr + writable_size;
            write_size += writable_size;
            write_addr += writable_size;
        } else {
            result = ef_port_write(write_addr, log + write_size / 4, size - write_size);
            if (result != EF_NO_ERR) {
                goto exit;
            }
            log_end_addr = write_addr + (size - write_size);
            break;
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
    uint32_t write_addr = log_area_start_addr;

    /* clean address */
    log_start_addr = log_area_start_addr;
    log_end_addr = log_start_addr + LOG_SECTOR_HEADER_SIZE;
    /* erase log flash area */
    result = ef_port_erase(log_area_start_addr, LOG_AREA_SIZE);
    if (result != EF_NO_ERR) {
        goto exit;
    }
    /* setting first sector is EMPTY to USING */
    write_sector_status(write_addr, SECTOR_STATUS_EMPUT);
    write_sector_status(write_addr, SECTOR_STATUS_USING);
    if (result != EF_NO_ERR) {
        goto exit;
    }
    write_addr += EF_ERASE_MIN_SIZE;
    /* add sector header */
    while (true) {
        write_sector_status(write_addr, SECTOR_STATUS_EMPUT);
        if (result != EF_NO_ERR) {
            goto exit;
        }
        write_addr += EF_ERASE_MIN_SIZE;
        if (write_addr >= log_area_start_addr + LOG_AREA_SIZE) {
            break;
        }
    }

exit:
    return result;
}

#endif /* EF_USING_LOG */
