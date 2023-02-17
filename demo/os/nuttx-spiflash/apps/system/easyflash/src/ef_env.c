/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2019, Armink, <armink.ztl@gmail.com>
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
 * Function: Environment variables operating interface. This is the Next Generation version.
 * Created on: 2019-02-02
 */

#include <string.h>
#include <easyflash.h>

#if defined(EF_USING_ENV) && !defined(EF_ENV_USING_LEGACY_MODE)

#ifndef EF_WRITE_GRAN
#error "Please configure flash write granularity (in ef_cfg.h)"
#endif

#if EF_WRITE_GRAN != 1 && EF_WRITE_GRAN != 8 && EF_WRITE_GRAN != 32 && EF_WRITE_GRAN != 64
#error "the write gran can be only setting as 1, 8, 32 and 64"
#endif

/* magic word(`E`, `F`, `4`, `0`) */
#define SECTOR_MAGIC_WORD                        0x30344645
/* magic word(`K`, `V`, `4`, `0`) */
#define ENV_MAGIC_WORD                           0x3034564B

/* the using status sector table length */
#ifndef USING_SECTOR_TABLE_LEN
#define USING_SECTOR_TABLE_LEN                   4
#endif

/* the string ENV value buffer size for legacy ef_get_env() function */
#ifndef EF_STR_ENV_VALUE_MAX_SIZE
#define EF_STR_ENV_VALUE_MAX_SIZE                128
#endif

/* the sector remain threshold before full status */
#ifndef EF_SEC_REMAIN_THRESHOLD
#define EF_SEC_REMAIN_THRESHOLD                  (ENV_HDR_DATA_SIZE + EF_ENV_NAME_MAX)
#endif

/* the total remain empty sector threshold before GC */
#ifndef EF_GC_EMPTY_SEC_THRESHOLD
#define EF_GC_EMPTY_SEC_THRESHOLD                1
#endif

/* the ENV cache table size, it will improve ENV search speed when using cache */
#ifndef EF_ENV_CACHE_TABLE_SIZE
#define EF_ENV_CACHE_TABLE_SIZE                  16
#endif

/* the sector cache table size, it will improve ENV save speed when using cache */
#ifndef EF_SECTOR_CACHE_TABLE_SIZE
#define EF_SECTOR_CACHE_TABLE_SIZE               4
#endif

#if EF_ENV_CACHE_TABLE_SIZE > 0xFFFF
#error "The ENV cache table size must less than 0xFFFF"
#endif

#if (EF_ENV_CACHE_TABLE_SIZE > 0) && (EF_SECTOR_CACHE_TABLE_SIZE > 0)
#define EF_ENV_USING_CACHE
#endif

/* the sector is not combined value */
#define SECTOR_NOT_COMBINED                      0xFFFFFFFF
/* the next address is get failed */
#define FAILED_ADDR                              0xFFFFFFFF

/* Return the most contiguous size aligned at specified width. RT_ALIGN(13, 4)
 * would return 16.
 */
#define EF_ALIGN(size, align)                    (((size) + (align) - 1) & ~((align) - 1))
/* align by write granularity */
#define EF_WG_ALIGN(size)                        (EF_ALIGN(size, (EF_WRITE_GRAN + 7)/8))
/**
 * Return the down number of aligned at specified width. RT_ALIGN_DOWN(13, 4)
 * would return 12.
 */
#define EF_ALIGN_DOWN(size, align)               ((size) & ~((align) - 1))
/* align down by write granularity */
#define EF_WG_ALIGN_DOWN(size)                   (EF_ALIGN_DOWN(size, (EF_WRITE_GRAN + 7)/8))

#if (EF_WRITE_GRAN == 1)
#define STATUS_TABLE_SIZE(status_number)         ((status_number * EF_WRITE_GRAN + 7)/8)
#else
#define STATUS_TABLE_SIZE(status_number)         (((status_number - 1) * EF_WRITE_GRAN + 7)/8)
#endif

#define STORE_STATUS_TABLE_SIZE                  STATUS_TABLE_SIZE(SECTOR_STORE_STATUS_NUM)
#define DIRTY_STATUS_TABLE_SIZE                  STATUS_TABLE_SIZE(SECTOR_DIRTY_STATUS_NUM)
#define ENV_STATUS_TABLE_SIZE                    STATUS_TABLE_SIZE(ENV_STATUS_NUM)

#define SECTOR_SIZE                              EF_ERASE_MIN_SIZE
#define SECTOR_NUM                               (ENV_AREA_SIZE / (EF_ERASE_MIN_SIZE))

#if (SECTOR_NUM < 2)
#error "The sector number must lager then or equal to 2"
#endif

#if (EF_GC_EMPTY_SEC_THRESHOLD == 0 || EF_GC_EMPTY_SEC_THRESHOLD >= SECTOR_NUM)
#error "There is at least one empty sector for GC."
#endif

#define SECTOR_HDR_DATA_SIZE                     (EF_WG_ALIGN(sizeof(struct sector_hdr_data)))
#define SECTOR_DIRTY_OFFSET                      ((unsigned long)(&((struct sector_hdr_data *)0)->status_table.dirty))
#define ENV_HDR_DATA_SIZE                        (EF_WG_ALIGN(sizeof(struct env_hdr_data)))
#define ENV_MAGIC_OFFSET                         ((unsigned long)(&((struct env_hdr_data *)0)->magic))
#define ENV_LEN_OFFSET                           ((unsigned long)(&((struct env_hdr_data *)0)->len))
#define ENV_NAME_LEN_OFFSET                      ((unsigned long)(&((struct env_hdr_data *)0)->name_len))

#define VER_NUM_ENV_NAME                         "__ver_num__"

enum sector_store_status {
    SECTOR_STORE_UNUSED,
    SECTOR_STORE_EMPTY,
    SECTOR_STORE_USING,
    SECTOR_STORE_FULL,
    SECTOR_STORE_STATUS_NUM,
};
typedef enum sector_store_status sector_store_status_t;

enum sector_dirty_status {
    SECTOR_DIRTY_UNUSED,
    SECTOR_DIRTY_FALSE,
    SECTOR_DIRTY_TRUE,
    SECTOR_DIRTY_GC,
    SECTOR_DIRTY_STATUS_NUM,
};
typedef enum sector_dirty_status sector_dirty_status_t;

struct sector_hdr_data {
    struct {
        uint8_t store[STORE_STATUS_TABLE_SIZE];  /**< sector store status @see sector_store_status_t */
        uint8_t dirty[DIRTY_STATUS_TABLE_SIZE];  /**< sector dirty status @see sector_dirty_status_t */
    } status_table;
    uint32_t magic;                              /**< magic word(`E`, `F`, `4`, `0`) */
    uint32_t combined;                           /**< the combined next sector number, 0xFFFFFFFF: not combined */
    uint32_t reserved;
};
typedef struct sector_hdr_data *sector_hdr_data_t;

struct sector_meta_data {
    bool check_ok;                               /**< sector header check is OK */
    struct {
        sector_store_status_t store;             /**< sector store status @see sector_store_status_t */
        sector_dirty_status_t dirty;             /**< sector dirty status @see sector_dirty_status_t */
    } status;
    uint32_t addr;                               /**< sector start address */
    uint32_t magic;                              /**< magic word(`E`, `F`, `4`, `0`) */
    uint32_t combined;                           /**< the combined next sector number, 0xFFFFFFFF: not combined */
    size_t remain;                               /**< remain size */
    uint32_t empty_env;                          /**< the next empty ENV node start address */
};
typedef struct sector_meta_data *sector_meta_data_t;

struct env_hdr_data {
    uint8_t status_table[ENV_STATUS_TABLE_SIZE]; /**< ENV node status, @see node_status_t */
    uint32_t magic;                              /**< magic word(`K`, `V`, `4`, `0`) */
    uint32_t len;                                /**< ENV node total length (header + name + value), must align by EF_WRITE_GRAN */
    uint32_t crc32;                              /**< ENV node crc32(name_len + data_len + name + value) */
    uint8_t name_len;                            /**< name length */
    uint32_t value_len;                          /**< value length */
};
typedef struct env_hdr_data *env_hdr_data_t;

struct env_cache_node {
    uint16_t name_crc;                           /**< ENV name's CRC32 low 16bit value */
    uint16_t active;                             /**< ENV node access active degree */
    uint32_t addr;                               /**< ENV node address */
};
typedef struct env_cache_node *env_cache_node_t;

struct sector_cache_node {
    uint32_t addr;                               /**< sector start address */
    uint32_t empty_addr;                         /**< sector empty address */
};
typedef struct sector_cache_node *sector_cache_node_t;

static void gc_collect(void);

/* ENV start address in flash */
static uint32_t env_start_addr = 0;
/* default ENV set, must be initialized by user */
static ef_env const *default_env_set;
/* default ENV set size, must be initialized by user */
static size_t default_env_set_size = 0;
/* initialize OK flag */
static bool init_ok = false;
/* request a GC check */
static bool gc_request = false;
/* is in recovery check status when first reboot */
static bool in_recovery_check = false;

#ifdef EF_ENV_USING_CACHE
/* ENV cache table */
struct env_cache_node env_cache_table[EF_ENV_CACHE_TABLE_SIZE] = { 0 };
/* sector cache table, it caching the sector info which status is current using */
struct sector_cache_node sector_cache_table[EF_SECTOR_CACHE_TABLE_SIZE] = { 0 };
#endif /* EF_ENV_USING_CACHE */

static size_t set_status(uint8_t status_table[], size_t status_num, size_t status_index)
{
    size_t byte_index = ~0UL;
    /*
     * | write garn |       status0       |       status1       |      status2         |
     * ---------------------------------------------------------------------------------
     * |    1bit    | 0xFF                | 0x7F                |  0x3F                |
     * |    8bit    | 0xFFFF              | 0x00FF              |  0x0000              |
     * |   32bit    | 0xFFFFFFFF FFFFFFFF | 0x00FFFFFF FFFFFFFF |  0x00FFFFFF 00FFFFFF |
     */
    memset(status_table, 0xFF, STATUS_TABLE_SIZE(status_num));
    if (status_index > 0) {
#if (EF_WRITE_GRAN == 1)
        byte_index = (status_index - 1) / 8;
        status_table[byte_index] &= ~(0x80 >> ((status_index - 1) % 8));
#else
        byte_index = (status_index - 1) * (EF_WRITE_GRAN / 8);
        status_table[byte_index] = 0x00;
#endif /* EF_WRITE_GRAN == 1 */
    }

    return byte_index;
}

static size_t get_status(uint8_t status_table[], size_t status_num)
{
    size_t i = 0, status_num_bak = --status_num;

    while (status_num --) {
        /* get the first 0 position from end address to start address */
#if (EF_WRITE_GRAN == 1)
        if ((status_table[status_num / 8] & (0x80 >> (status_num % 8))) == 0x00) {
            break;
        }
#else /*  (EF_WRITE_GRAN == 8) ||  (EF_WRITE_GRAN == 32) ||  (EF_WRITE_GRAN == 64) */
        if (status_table[status_num * EF_WRITE_GRAN / 8] == 0x00) {
            break;
        }
#endif /* EF_WRITE_GRAN == 1 */
        i++;
    }

    return status_num_bak - i;
}

static EfErrCode write_status(uint32_t addr, uint8_t status_table[], size_t status_num, size_t status_index)
{
    EfErrCode result = EF_NO_ERR;
    size_t byte_index;

    EF_ASSERT(status_index < status_num);
    EF_ASSERT(status_table);

    /* set the status first */
    byte_index = set_status(status_table, status_num, status_index);

    /* the first status table value is all 1, so no need to write flash */
    if (byte_index == ~0UL) {
        return EF_NO_ERR;
    }
#if (EF_WRITE_GRAN == 1)
    result = ef_port_write(addr + byte_index, (uint32_t *)&status_table[byte_index], 1);
#else /*  (EF_WRITE_GRAN == 8) ||  (EF_WRITE_GRAN == 32) ||  (EF_WRITE_GRAN == 64) */
    /* write the status by write granularity
     * some flash (like stm32 onchip) NOT supported repeated write before erase */
    result = ef_port_write(addr + byte_index, (uint32_t *) &status_table[byte_index], EF_WRITE_GRAN / 8);
#endif /* EF_WRITE_GRAN == 1 */

    return result;
}

static size_t read_status(uint32_t addr, uint8_t status_table[], size_t total_num)
{
    EF_ASSERT(status_table);

    ef_port_read(addr, (uint32_t *) status_table, STATUS_TABLE_SIZE(total_num));

    return get_status(status_table, total_num);
}

#ifdef EF_ENV_USING_CACHE
/*
 * It's only caching the current using status sector's empty_addr
 */
static void update_sector_cache(uint32_t sec_addr, uint32_t empty_addr)
{
    size_t i, empty_index = EF_SECTOR_CACHE_TABLE_SIZE;

    for (i = 0; i < EF_SECTOR_CACHE_TABLE_SIZE; i++) {
        if ((empty_addr > sec_addr) && (empty_addr < sec_addr + SECTOR_SIZE)) {
            /* update the sector empty_addr in cache */
            if (sector_cache_table[i].addr == sec_addr) {
                sector_cache_table[i].addr = sec_addr;
                sector_cache_table[i].empty_addr = empty_addr;
                return;
            } else if ((sector_cache_table[i].addr == FAILED_ADDR) && (empty_index == EF_SECTOR_CACHE_TABLE_SIZE)) {
                empty_index = i;
            }
        } else if (sector_cache_table[i].addr == sec_addr) {
            /* delete the sector which status is not current using */
            sector_cache_table[i].addr = FAILED_ADDR;
            return;
        }
    }
    /* add the sector empty_addr to cache */
    if (empty_index < EF_SECTOR_CACHE_TABLE_SIZE) {
        sector_cache_table[empty_index].addr = sec_addr;
        sector_cache_table[empty_index].empty_addr = empty_addr;
    }
}

/*
 * Get sector info from cache. It's return true when cache is hit.
 */
static bool get_sector_from_cache(uint32_t sec_addr, uint32_t *empty_addr)
{
    size_t i;

    for (i = 0; i < EF_SECTOR_CACHE_TABLE_SIZE; i++) {
        if (sector_cache_table[i].addr == sec_addr) {
            if (empty_addr) {
                *empty_addr = sector_cache_table[i].empty_addr;
            }
            return true;
        }
    }

    return false;
}

static void update_env_cache(const char *name, size_t name_len, uint32_t addr)
{
    size_t i, empty_index = EF_ENV_CACHE_TABLE_SIZE, min_activity_index = EF_ENV_CACHE_TABLE_SIZE;
    uint16_t name_crc = (uint16_t) (ef_calc_crc32(0, name, name_len) >> 16), min_activity = 0xFFFF;

    for (i = 0; i < EF_ENV_CACHE_TABLE_SIZE; i++) {
        if (addr != FAILED_ADDR) {
            /* update the ENV address in cache */
            if (env_cache_table[i].name_crc == name_crc) {
                env_cache_table[i].addr = addr;
                return;
            } else if ((env_cache_table[i].addr == FAILED_ADDR) && (empty_index == EF_ENV_CACHE_TABLE_SIZE)) {
                empty_index = i;
            } else if (env_cache_table[i].addr != FAILED_ADDR) {
                if (env_cache_table[i].active > 0) {
                    env_cache_table[i].active--;
                }
                if (env_cache_table[i].active < min_activity) {
                    min_activity_index = i;
                    min_activity = env_cache_table[i].active;
                }
            }
        } else if (env_cache_table[i].name_crc == name_crc) {
            /* delete the ENV */
            env_cache_table[i].addr = FAILED_ADDR;
            env_cache_table[i].active = 0;
            return;
        }
    }
    /* add the ENV to cache, using LRU (Least Recently Used) like algorithm */
    if (empty_index < EF_ENV_CACHE_TABLE_SIZE) {
        env_cache_table[empty_index].addr = addr;
        env_cache_table[empty_index].name_crc = name_crc;
        env_cache_table[empty_index].active = 0;
    } else if (min_activity_index < EF_ENV_CACHE_TABLE_SIZE) {
        env_cache_table[min_activity_index].addr = addr;
        env_cache_table[min_activity_index].name_crc = name_crc;
        env_cache_table[min_activity_index].active = 0;
    }
}

/*
 * Get ENV info from cache. It's return true when cache is hit.
 */
static bool get_env_from_cache(const char *name, size_t name_len, uint32_t *addr)
{
    size_t i;
    uint16_t name_crc = (uint16_t) (ef_calc_crc32(0, name, name_len) >> 16);

    for (i = 0; i < EF_ENV_CACHE_TABLE_SIZE; i++) {
        if ((env_cache_table[i].addr != FAILED_ADDR) && (env_cache_table[i].name_crc == name_crc)) {
            char saved_name[EF_ENV_NAME_MAX];
            /* read the ENV name in flash */
            ef_port_read(env_cache_table[i].addr + ENV_HDR_DATA_SIZE, (uint32_t *) saved_name, EF_ENV_NAME_MAX);
            if (!strncmp(name, saved_name, name_len)) {
                *addr = env_cache_table[i].addr;
                if (env_cache_table[i].active >= 0xFFFF - EF_ENV_CACHE_TABLE_SIZE) {
                    env_cache_table[i].active = 0xFFFF;
                } else {
                    env_cache_table[i].active += EF_ENV_CACHE_TABLE_SIZE;
                }
                return true;
            }
        }
    }

    return false;
}
#endif /* EF_ENV_USING_CACHE */

/*
 * find the continue 0xFF flash address to end address
 */
static uint32_t continue_ff_addr(uint32_t start, uint32_t end)
{
    uint8_t buf[32], last_data = 0x00;
    size_t i, addr = start, read_size;

    for (; start < end; start += sizeof(buf)) {
        if (start + sizeof(buf) < end) {
            read_size = sizeof(buf);
        } else {
            read_size = end - start;
        }
        ef_port_read(start, (uint32_t *) buf, read_size);
        for (i = 0; i < read_size; i++) {
            if (last_data != 0xFF && buf[i] == 0xFF) {
                addr = start + i;
            }
            last_data = buf[i];
        }
    }

    if (last_data == 0xFF) {
        return EF_WG_ALIGN(addr);
    } else {
        return end;
    }
}

/*
 * find the next ENV address by magic word on the flash
 */
static uint32_t find_next_env_addr(uint32_t start, uint32_t end)
{
    uint8_t buf[32];
    uint32_t start_bak = start, i;
    uint32_t magic;

#ifdef EF_ENV_USING_CACHE
    uint32_t empty_env;

    if (get_sector_from_cache(EF_ALIGN_DOWN(start, SECTOR_SIZE), &empty_env) && start == empty_env) {
        return FAILED_ADDR;
    }
#endif /* EF_ENV_USING_CACHE */

    for (; start < end; start += (sizeof(buf) - sizeof(uint32_t))) {
        ef_port_read(start, (uint32_t *) buf, sizeof(buf));
        for (i = 0; i < sizeof(buf) - sizeof(uint32_t) && start + i < end; i++) {
#ifndef EF_BIG_ENDIAN            /* Little Endian Order */
            magic = buf[i] + (buf[i + 1] << 8) + (buf[i + 2] << 16) + (buf[i + 3] << 24);
#else                       /* Big Endian Order */
            magic = buf[i + 3] + (buf[i + 2] << 8) + (buf[i + 1] << 16) + (buf[i] << 24);
#endif
            if (magic == ENV_MAGIC_WORD && (start + i - ENV_MAGIC_OFFSET) >= start_bak) {
                return start + i - ENV_MAGIC_OFFSET;
            }
        }
    }

    return FAILED_ADDR;
}

static uint32_t get_next_env_addr(sector_meta_data_t sector, env_node_obj_t pre_env)
{
    uint32_t addr = FAILED_ADDR;

    if (sector->status.store == SECTOR_STORE_EMPTY) {
        return FAILED_ADDR;
    }

    if (pre_env->addr.start == FAILED_ADDR) {
        /* the first ENV address */
        addr = sector->addr + SECTOR_HDR_DATA_SIZE;
    } else {
        if (pre_env->addr.start <= sector->addr + SECTOR_SIZE) {
            if (pre_env->crc_is_ok) {
                addr = pre_env->addr.start + pre_env->len;
            } else {
                /* when pre_env CRC check failed, maybe the flash has error data
                 * find_next_env_addr after pre_env address */
                addr = pre_env->addr.start + EF_WG_ALIGN(1);
            }
            /* check and find next ENV address */
            addr = find_next_env_addr(addr, sector->addr + SECTOR_SIZE - SECTOR_HDR_DATA_SIZE);

            if (addr > sector->addr + SECTOR_SIZE || pre_env->len == 0) {
                //TODO ��������ģʽ
                return FAILED_ADDR;
            }
        } else {
            /* no ENV */
            return FAILED_ADDR;
        }
    }

    return addr;
}

static EfErrCode read_env(env_node_obj_t env)
{
    struct env_hdr_data env_hdr;
    uint8_t buf[32];
    uint32_t calc_crc32 = 0, crc_data_len, env_name_addr;
    EfErrCode result = EF_NO_ERR;
    size_t len, size;
    /* read ENV header raw data */
    ef_port_read(env->addr.start, (uint32_t *)&env_hdr, sizeof(struct env_hdr_data));
    env->status = (env_status_t) get_status(env_hdr.status_table, ENV_STATUS_NUM);
    env->len = env_hdr.len;

    if (env->len == ~0UL || env->len > ENV_AREA_SIZE || env->len < ENV_NAME_LEN_OFFSET) {
        /* the ENV length was not write, so reserved the meta data for current ENV */
        env->len = ENV_HDR_DATA_SIZE;
        if (env->status != ENV_ERR_HDR) {
            env->status = ENV_ERR_HDR;
            EF_DEBUG("Error: The ENV @0x%08X length has an error.\n", env->addr.start);
            write_status(env->addr.start, env_hdr.status_table, ENV_STATUS_NUM, ENV_ERR_HDR);
        }
        env->crc_is_ok = false;
        return EF_READ_ERR;
    } else if (env->len > SECTOR_SIZE - SECTOR_HDR_DATA_SIZE && env->len < ENV_AREA_SIZE) {
        //TODO ��������ģʽ������д�볤��û��д������
        EF_ASSERT(0);
    }

    /* CRC32 data len(header.name_len + header.value_len + name + value) */
    crc_data_len = env->len - ENV_NAME_LEN_OFFSET;
    /* calculate the CRC32 value */
    for (len = 0, size = 0; len < crc_data_len; len += size) {
        if (len + sizeof(buf) < crc_data_len) {
            size = sizeof(buf);
        } else {
            size = crc_data_len - len;
        }

        ef_port_read(env->addr.start + ENV_NAME_LEN_OFFSET + len, (uint32_t *) buf, EF_WG_ALIGN(size));
        calc_crc32 = ef_calc_crc32(calc_crc32, buf, size);
    }
    /* check CRC32 */
    if (calc_crc32 != env_hdr.crc32) {
        env->crc_is_ok = false;
        result = EF_READ_ERR;
    } else {
        env->crc_is_ok = true;
        /* the name is behind aligned ENV header */
        env_name_addr = env->addr.start + ENV_HDR_DATA_SIZE;
        ef_port_read(env_name_addr, (uint32_t *) env->name, EF_WG_ALIGN(env_hdr.name_len));
        /* the value is behind aligned name */
        env->addr.value = env_name_addr + EF_WG_ALIGN(env_hdr.name_len);
        env->value_len = env_hdr.value_len;
        env->name_len = env_hdr.name_len;
    }

    return result;
}

static EfErrCode read_sector_meta_data(uint32_t addr, sector_meta_data_t sector, bool traversal)
{
    EfErrCode result = EF_NO_ERR;
    struct sector_hdr_data sec_hdr;

    EF_ASSERT(addr % SECTOR_SIZE == 0);
    EF_ASSERT(sector);

    /* read sector header raw data */
    ef_port_read(addr, (uint32_t *)&sec_hdr, sizeof(struct sector_hdr_data));

    sector->addr = addr;
    sector->magic = sec_hdr.magic;
    /* check magic word */
    if (sector->magic != SECTOR_MAGIC_WORD) {
        sector->check_ok = false;
        sector->combined = SECTOR_NOT_COMBINED;
        return EF_ENV_INIT_FAILED;
    }
    sector->check_ok = true;
    /* get other sector meta data */
    sector->combined = sec_hdr.combined;
    sector->status.store = (sector_store_status_t) get_status(sec_hdr.status_table.store, SECTOR_STORE_STATUS_NUM);
    sector->status.dirty = (sector_dirty_status_t) get_status(sec_hdr.status_table.dirty, SECTOR_DIRTY_STATUS_NUM);
    /* traversal all ENV and calculate the remain space size */
    if (traversal) {
        sector->remain = 0;
        sector->empty_env = sector->addr + SECTOR_HDR_DATA_SIZE;
        if (sector->status.store == SECTOR_STORE_EMPTY) {
            sector->remain = SECTOR_SIZE - SECTOR_HDR_DATA_SIZE;
        } else if (sector->status.store == SECTOR_STORE_USING) {
            struct env_node_obj env_meta;

#ifdef EF_ENV_USING_CACHE
            if (get_sector_from_cache(addr, &sector->empty_env)) {
                sector->remain = SECTOR_SIZE - (sector->empty_env - sector->addr);
                return result;
            }
#endif /* EF_ENV_USING_CACHE */

            sector->remain = SECTOR_SIZE - SECTOR_HDR_DATA_SIZE;
            env_meta.addr.start = FAILED_ADDR;
            while ((env_meta.addr.start = get_next_env_addr(sector, &env_meta)) != FAILED_ADDR) {
                read_env(&env_meta);
                if (!env_meta.crc_is_ok) {
                    if (env_meta.status != ENV_PRE_WRITE && env_meta.status!= ENV_ERR_HDR) {
                        EF_INFO("Error: The ENV (@0x%08X) CRC32 check failed!\n", env_meta.addr.start);
                        sector->remain = 0;
                        result = EF_READ_ERR;
                        break;
                    }
                }
                sector->empty_env += env_meta.len;
                sector->remain -= env_meta.len;
            }
            /* check the empty ENV address by read continue 0xFF on flash  */
            {
                uint32_t ff_addr;

                ff_addr = continue_ff_addr(sector->empty_env, sector->addr + SECTOR_SIZE);
                /* check the flash data is clean */
                if (sector->empty_env != ff_addr) {
                    /* update the sector information */
                    sector->empty_env = ff_addr;
                    sector->remain = SECTOR_SIZE - (ff_addr - sector->addr);
                }
            }

#ifdef EF_ENV_USING_CACHE
            update_sector_cache(sector->addr, sector->empty_env);
#endif
        }
    }

    return result;
}

static uint32_t get_next_sector_addr(sector_meta_data_t pre_sec)
{
    uint32_t next_addr;

    if (pre_sec->addr == FAILED_ADDR) {
        return env_start_addr;
    } else {
        /* check ENV sector combined */
        if (pre_sec->combined == SECTOR_NOT_COMBINED) {
            next_addr = pre_sec->addr + SECTOR_SIZE;
        } else {
            next_addr = pre_sec->addr + pre_sec->combined * SECTOR_SIZE;
        }
        /* check range */
        if (next_addr < env_start_addr + ENV_AREA_SIZE) {
            return next_addr;
        } else {
            /* no sector */
            return FAILED_ADDR;
        }
    }
}

static void env_iterator(env_node_obj_t env, void *arg1, void *arg2,
        bool (*callback)(env_node_obj_t env, void *arg1, void *arg2))
{
    struct sector_meta_data sector;
    uint32_t sec_addr;

    sector.addr = FAILED_ADDR;
    /* search all sectors */
    while ((sec_addr = get_next_sector_addr(&sector)) != FAILED_ADDR) {
        if (read_sector_meta_data(sec_addr, &sector, false) != EF_NO_ERR) {
            continue;
        }
        if (callback == NULL) {
            continue;
        }
        /* sector has ENV */
        if (sector.status.store == SECTOR_STORE_USING || sector.status.store == SECTOR_STORE_FULL) {
            env->addr.start = FAILED_ADDR;
            /* search all ENV */
            while ((env->addr.start = get_next_env_addr(&sector, env)) != FAILED_ADDR) {
                read_env(env);
                /* iterator is interrupted when callback return true */
                if (callback(env, arg1, arg2)) {
                    return;
                }
            }
        }
    }
}

static bool find_env_cb(env_node_obj_t env, void *arg1, void *arg2)
{
    const char *key = arg1;
    bool *find_ok = arg2;
    size_t key_len = strlen(key);

    if (key_len != env->name_len) {
        return false;
    }
    /* check ENV */
    if (env->crc_is_ok && env->status == ENV_WRITE && !strncmp(env->name, key, key_len)) {
        *find_ok = true;
        return true;
    }
    return false;
}

static bool find_env_no_cache(const char *key, env_node_obj_t env)
{
    bool find_ok = false;

    env_iterator(env, (void *)key, &find_ok, find_env_cb);

    return find_ok;
}

static bool find_env(const char *key, env_node_obj_t env)
{
    bool find_ok = false;

#ifdef EF_ENV_USING_CACHE
    size_t key_len = strlen(key);

    if (get_env_from_cache(key, key_len, &env->addr.start)) {
        read_env(env);
        return true;
    }
#endif /* EF_ENV_USING_CACHE */

    find_ok = find_env_no_cache(key, env);

#ifdef EF_ENV_USING_CACHE
    if (find_ok) {
        update_env_cache(key, key_len, env->addr.start);
    }
#endif /* EF_ENV_USING_CACHE */

    return find_ok;
}

static bool ef_is_str(uint8_t *value, size_t len)
{
#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')
    size_t i;

    for (i = 0; i < len; i++) {
        if (!__is_print(value[i])) {
            return false;
        }
    }
    return true;
}

static size_t get_env(const char *key, void *value_buf, size_t buf_len, size_t *value_len)
{
    struct env_node_obj env;
    size_t read_len = 0;

    if (find_env(key, &env)) {
        if (value_len) {
            *value_len = env.value_len;
        }
        if (buf_len > env.value_len) {
            read_len = env.value_len;
        } else {
            read_len = buf_len;
        }
        if (value_buf){
            ef_port_read(env.addr.value, (uint32_t *) value_buf, read_len);
        }
    } else if (value_len) {
        *value_len = 0;
    }

    return read_len;
}

/**
 * Get a ENV object by key name
 *
 * @param key ENV name
 * @param env ENV object
 *
 * @return TRUE: find the ENV is OK, else return false
 */
bool ef_get_env_obj(const char *key, env_node_obj_t env)
{
    bool find_ok = false;

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return 0;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    find_ok = find_env(key, env);

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return find_ok;
}

/**
 * Get a blob ENV value by key name.
 *
 * @param key ENV name
 * @param value_buf ENV blob buffer
 * @param buf_len ENV blob buffer length
 * @param saved_value_len return the length of the value saved on the flash, 0: NOT found
 *
 * @return the actually get size on successful
 */
size_t ef_get_env_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len)
{
    size_t read_len = 0;

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return 0;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    read_len = get_env(key, value_buf, buf_len, saved_value_len);

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return read_len;
}

/**
 * Get an ENV value by key name.
 *
 * @note this function is NOT supported reentrant
 * @note this function is DEPRECATED
 *
 * @param key ENV name
 *
 * @return value
 */
char *ef_get_env(const char *key)
{
    static char value[EF_STR_ENV_VALUE_MAX_SIZE + 1];
    size_t get_size;

    if ((get_size = ef_get_env_blob(key, value, EF_STR_ENV_VALUE_MAX_SIZE, NULL)) > 0) {
        /* the return value must be string */
        if (ef_is_str((uint8_t *)value, get_size)) {
            value[get_size] = '\0';
            return value;
        } else {
            EF_INFO("Warning: The ENV value isn't string. Could not be returned\n");
            return NULL;
        }
    }

    return NULL;
}

/**
 * read the ENV value by ENV object
 *
 * @param env ENV object
 * @param value_buf the buffer for store ENV value
 * @param buf_len buffer length
 *
 * @return the actually read size on successful
 */
size_t ef_read_env_value(env_node_obj_t env, uint8_t *value_buf, size_t buf_len)
{
    size_t read_len = 0;

    EF_ASSERT(env);
    EF_ASSERT(value_buf);

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return 0;
    }

    if (env->crc_is_ok) {
        /* lock the ENV cache */
        ef_port_env_lock();

        if (buf_len > env->value_len) {
            read_len = env->value_len;
        } else {
            read_len = buf_len;
        }

        ef_port_read(env->addr.value, (uint32_t *) value_buf, read_len);
        /* unlock the ENV cache */
        ef_port_env_unlock();
    }

    return read_len;
}

static EfErrCode write_env_hdr(uint32_t addr, env_hdr_data_t env_hdr) {
    EfErrCode result = EF_NO_ERR;
    /* write the status will by write granularity */
    result = write_status(addr, env_hdr->status_table, ENV_STATUS_NUM, ENV_PRE_WRITE);
    if (result != EF_NO_ERR) {
        return result;
    }
    /* write other header data */
    result = ef_port_write(addr + ENV_MAGIC_OFFSET, &env_hdr->magic, sizeof(struct env_hdr_data) - ENV_MAGIC_OFFSET);

    return result;
}

static EfErrCode format_sector(uint32_t addr, uint32_t combined_value)
{
    EfErrCode result = EF_NO_ERR;
    struct sector_hdr_data sec_hdr;

    EF_ASSERT(addr % SECTOR_SIZE == 0);

    result = ef_port_erase(addr, SECTOR_SIZE);
    if (result == EF_NO_ERR) {
        /* initialize the header data */
        memset(&sec_hdr, 0xFF, sizeof(struct sector_hdr_data));
        set_status(sec_hdr.status_table.store, SECTOR_STORE_STATUS_NUM, SECTOR_STORE_EMPTY);
        set_status(sec_hdr.status_table.dirty, SECTOR_DIRTY_STATUS_NUM, SECTOR_DIRTY_FALSE);
        sec_hdr.magic = SECTOR_MAGIC_WORD;
        sec_hdr.combined = combined_value;
        sec_hdr.reserved = 0xFFFFFFFF;
        /* save the header */
        result = ef_port_write(addr, (uint32_t *)&sec_hdr, sizeof(struct sector_hdr_data));

#ifdef EF_ENV_USING_CACHE
        /* delete the sector cache */
        update_sector_cache(addr, addr + SECTOR_SIZE);
#endif /* EF_ENV_USING_CACHE */
    }

    return result;
}

static EfErrCode update_sec_status(sector_meta_data_t sector, size_t new_env_len, bool *is_full)
{
    uint8_t status_table[STORE_STATUS_TABLE_SIZE];
    EfErrCode result = EF_NO_ERR;
    /* change the current sector status */
    if (sector->status.store == SECTOR_STORE_EMPTY) {
        /* change the sector status to using */
        result = write_status(sector->addr, status_table, SECTOR_STORE_STATUS_NUM, SECTOR_STORE_USING);
    } else if (sector->status.store == SECTOR_STORE_USING) {
        /* check remain size */
        if (sector->remain < EF_SEC_REMAIN_THRESHOLD || sector->remain - new_env_len < EF_SEC_REMAIN_THRESHOLD) {
            /* change the sector status to full */
            result = write_status(sector->addr, status_table, SECTOR_STORE_STATUS_NUM, SECTOR_STORE_FULL);

#ifdef EF_ENV_USING_CACHE
            /* delete the sector cache */
            update_sector_cache(sector->addr, sector->addr + SECTOR_SIZE);
#endif /* EF_ENV_USING_CACHE */

            if (is_full) {
                *is_full = true;
            }
        } else if (is_full) {
            *is_full = false;
        }
    }

    return result;
}

static void sector_iterator(sector_meta_data_t sector, sector_store_status_t status, void *arg1, void *arg2,
        bool (*callback)(sector_meta_data_t sector, void *arg1, void *arg2), bool traversal_env) {
    uint32_t sec_addr;

    /* search all sectors */
    sector->addr = FAILED_ADDR;
    while ((sec_addr = get_next_sector_addr(sector)) != FAILED_ADDR) {
        read_sector_meta_data(sec_addr, sector, false);
        if (status == SECTOR_STORE_UNUSED || status == sector->status.store) {
            if (traversal_env) {
                read_sector_meta_data(sec_addr, sector, traversal_env);
            }
            /* iterator is interrupted when callback return true */
            if (callback && callback(sector, arg1, arg2)) {
                return;
            }
        }
    }
}

static bool sector_statistics_cb(sector_meta_data_t sector, void *arg1, void *arg2)
{
    size_t *empty_sector = arg1, *using_sector = arg2;

    if (sector->check_ok && sector->status.store == SECTOR_STORE_EMPTY) {
        (*empty_sector)++;
    } else if (sector->check_ok && sector->status.store == SECTOR_STORE_USING) {
        (*using_sector)++;
    }

    return false;
}

static bool alloc_env_cb(sector_meta_data_t sector, void *arg1, void *arg2)
{
    size_t *env_size = arg1;
    uint32_t *empty_env = arg2;

    /* 1. sector has space
     * 2. the NO dirty sector
     * 3. the dirty sector only when the gc_request is false */
    if (sector->check_ok && sector->remain > *env_size
            && ((sector->status.dirty == SECTOR_DIRTY_FALSE)
                    || (sector->status.dirty == SECTOR_DIRTY_TRUE && !gc_request))) {
        *empty_env = sector->empty_env;
        return true;
    }

    return false;
}

static uint32_t alloc_env(sector_meta_data_t sector, size_t env_size)
{
    uint32_t empty_env = FAILED_ADDR;
    size_t empty_sector = 0, using_sector = 0;

    /* sector status statistics */
    sector_iterator(sector, SECTOR_STORE_UNUSED, &empty_sector, &using_sector, sector_statistics_cb, false);
    if (using_sector > 0) {
        /* alloc the ENV from the using status sector first */
        sector_iterator(sector, SECTOR_STORE_USING, &env_size, &empty_env, alloc_env_cb, true);
    }
    if (empty_sector > 0 && empty_env == FAILED_ADDR) {
        if (empty_sector > EF_GC_EMPTY_SEC_THRESHOLD || gc_request) {
            sector_iterator(sector, SECTOR_STORE_EMPTY, &env_size, &empty_env, alloc_env_cb, true);
        } else {
            /* no space for new ENV now will GC and retry */
            EF_DEBUG("Trigger a GC check after alloc ENV failed.\n");
            gc_request = true;
        }
    }

    return empty_env;
}

static EfErrCode del_env(const char *key, env_node_obj_t old_env, bool complete_del) {
    EfErrCode result = EF_NO_ERR;
    uint32_t dirty_status_addr;
    static bool last_is_complete_del = false;

#if (ENV_STATUS_TABLE_SIZE >= DIRTY_STATUS_TABLE_SIZE)
    uint8_t status_table[ENV_STATUS_TABLE_SIZE];
#else
    uint8_t status_table[DIRTY_STATUS_TABLE_SIZE];
#endif

    /* need find ENV */
    if (!old_env) {
        struct env_node_obj env;
        /* find ENV */
        if (find_env(key, &env)) {
            old_env = &env;
        } else {
            EF_DEBUG("Not found '%s' in ENV.\n", key);
            return EF_ENV_NAME_ERR;
        }
    }
    /* change and save the new status */
    if (!complete_del) {
        result = write_status(old_env->addr.start, status_table, ENV_STATUS_NUM, ENV_PRE_DELETE);
        last_is_complete_del = true;
    } else {
        result = write_status(old_env->addr.start, status_table, ENV_STATUS_NUM, ENV_DELETED);

        if (!last_is_complete_del && result == EF_NO_ERR) {
#ifdef EF_ENV_USING_CACHE
            /* delete the ENV in flash and cache */
            if (key != NULL) {
                /* when using del_env(key, NULL, true) or del_env(key, env, true) in ef_del_env() and set_env() */
                update_env_cache(key, strlen(key), FAILED_ADDR);
            } else if (old_env != NULL) {
                /* when using del_env(NULL, env, true) in move_env() */
                update_env_cache(old_env->name, old_env->name_len, FAILED_ADDR);
            }
#endif /* EF_ENV_USING_CACHE */
        }

        last_is_complete_del = false;
    }

    dirty_status_addr = EF_ALIGN_DOWN(old_env->addr.start, SECTOR_SIZE) + SECTOR_DIRTY_OFFSET;
    /* read and change the sector dirty status */
    if (result == EF_NO_ERR
            && read_status(dirty_status_addr, status_table, SECTOR_DIRTY_STATUS_NUM) == SECTOR_DIRTY_FALSE) {
        result = write_status(dirty_status_addr, status_table, SECTOR_DIRTY_STATUS_NUM, SECTOR_DIRTY_TRUE);
    }

    return result;
}

/*
 * move the ENV to new space
 */
static EfErrCode move_env(env_node_obj_t env)
{
    EfErrCode result = EF_NO_ERR;
    uint8_t status_table[ENV_STATUS_TABLE_SIZE];
    uint32_t env_addr;
    struct sector_meta_data sector;

    /* prepare to delete the current ENV */
    if (env->status == ENV_WRITE) {
        del_env(NULL, env, false);
    }

    if ((env_addr = alloc_env(&sector, env->len)) != FAILED_ADDR) {
        if (in_recovery_check) {
            struct env_node_obj env_bak;
            char name[EF_ENV_NAME_MAX + 1] = { 0 };
            strncpy(name, env->name, env->name_len);
            /* check the ENV in flash is already create success */
            if (find_env_no_cache(name, &env_bak)) {
                /* already create success, don't need to duplicate */
                result = EF_NO_ERR;
                goto __exit;
            }
        }
    } else {
        return EF_ENV_FULL;
    }
    /* start move the ENV */
    {
        uint8_t buf[32];
        size_t len, size, env_len = env->len;

        /* update the new ENV sector status first */
        update_sec_status(&sector, env->len, NULL);

        write_status(env_addr, status_table, ENV_STATUS_NUM, ENV_PRE_WRITE);
        env_len -= ENV_MAGIC_OFFSET;
        for (len = 0, size = 0; len < env_len; len += size) {
            if (len + sizeof(buf) < env_len) {
                size = sizeof(buf);
            } else {
                size = env_len - len;
            }
            ef_port_read(env->addr.start + ENV_MAGIC_OFFSET + len, (uint32_t *) buf, EF_WG_ALIGN(size));
            result = ef_port_write(env_addr + ENV_MAGIC_OFFSET + len, (uint32_t *) buf, size);
        }
        write_status(env_addr, status_table, ENV_STATUS_NUM, ENV_WRITE);

#ifdef EF_ENV_USING_CACHE
        update_sector_cache(EF_ALIGN_DOWN(env_addr, SECTOR_SIZE),
                env_addr + ENV_HDR_DATA_SIZE + EF_WG_ALIGN(env->name_len) + EF_WG_ALIGN(env->value_len));
        update_env_cache(env->name, env->name_len, env_addr);
#endif /* EF_ENV_USING_CACHE */
    }

    EF_DEBUG("Moved the ENV (%.*s) from 0x%08X to 0x%08X.\n", env->name_len, env->name, env->addr.start, env_addr);

__exit:
    del_env(NULL, env, true);

    return result;
}

static uint32_t new_env(sector_meta_data_t sector, size_t env_size)
{
    bool already_gc = false;
    uint32_t empty_env = FAILED_ADDR;

__retry:

    if ((empty_env = alloc_env(sector, env_size)) == FAILED_ADDR && gc_request && !already_gc) {
        EF_DEBUG("Warning: Alloc an ENV (size %d) failed when new ENV. Now will GC then retry.\n", env_size);
        gc_collect();
        already_gc = true;
        goto __retry;
    }

    return empty_env;
}

static uint32_t new_env_by_kv(sector_meta_data_t sector, size_t key_len, size_t buf_len)
{
    size_t env_len = ENV_HDR_DATA_SIZE + EF_WG_ALIGN(key_len) + EF_WG_ALIGN(buf_len);

    return new_env(sector, env_len);
}

static bool gc_check_cb(sector_meta_data_t sector, void *arg1, void *arg2)
{
    size_t *empty_sec = arg1;

    if (sector->check_ok) {
        *empty_sec = *empty_sec + 1;
    }

    return false;

}

static bool do_gc(sector_meta_data_t sector, void *arg1, void *arg2)
{
    struct env_node_obj env;

    if (sector->check_ok && (sector->status.dirty == SECTOR_DIRTY_TRUE || sector->status.dirty == SECTOR_DIRTY_GC)) {
        uint8_t status_table[DIRTY_STATUS_TABLE_SIZE];
        /* change the sector status to GC */
        write_status(sector->addr + SECTOR_DIRTY_OFFSET, status_table, SECTOR_DIRTY_STATUS_NUM, SECTOR_DIRTY_GC);
        /* search all ENV */
        env.addr.start = FAILED_ADDR;
        while ((env.addr.start = get_next_env_addr(sector, &env)) != FAILED_ADDR) {
            read_env(&env);
            if (env.crc_is_ok && (env.status == ENV_WRITE || env.status == ENV_PRE_DELETE)) {
                /* move the ENV to new space */
                if (move_env(&env) != EF_NO_ERR) {
                    EF_DEBUG("Error: Moved the ENV (%.*s) for GC failed.\n", env.name_len, env.name);
                }
            }
        }
        format_sector(sector->addr, SECTOR_NOT_COMBINED);
        EF_DEBUG("Collect a sector @0x%08X\n", sector->addr);
    }

    return false;
}

/*
 * The GC will be triggered on the following scene:
 * 1. alloc an ENV when the flash not has enough space
 * 2. write an ENV then the flash not has enough space
 */
static void gc_collect(void)
{
    struct sector_meta_data sector;
    size_t empty_sec = 0;

    /* GC check the empty sector number */
    sector_iterator(&sector, SECTOR_STORE_EMPTY, &empty_sec, NULL, gc_check_cb, false);

    /* do GC collect */
    EF_DEBUG("The remain empty sector is %d, GC threshold is %d.\n", empty_sec, EF_GC_EMPTY_SEC_THRESHOLD);
    if (empty_sec <= EF_GC_EMPTY_SEC_THRESHOLD) {
        sector_iterator(&sector, SECTOR_STORE_UNUSED, NULL, NULL, do_gc, false);
    }

    gc_request = false;
}

static EfErrCode align_write(uint32_t addr, const uint32_t *buf, size_t size)
{
    EfErrCode result = EF_NO_ERR;
    size_t align_remain;

#if (EF_WRITE_GRAN / 8 > 0)
    uint8_t align_data[EF_WRITE_GRAN / 8];
    size_t align_data_size = sizeof(align_data);
#else
    /* For compatibility with C89 */
    uint8_t align_data_u8, *align_data = &align_data_u8;
    size_t align_data_size = 1;
#endif

    memset(align_data, 0xFF, align_data_size);
    align_remain = EF_WG_ALIGN_DOWN(size);//use align_remain temporary to save aligned size.

    if(align_remain > 0){//it may be 0 in this function.
        result = ef_port_write(addr, buf, align_remain);
    }

    align_remain = size - align_remain;
    if (result == EF_NO_ERR && align_remain) {
        memcpy(align_data, (uint8_t *)buf + EF_WG_ALIGN_DOWN(size), align_remain);
        result = ef_port_write(addr + EF_WG_ALIGN_DOWN(size), (uint32_t *) align_data, align_data_size);
    }

    return result;
}

static EfErrCode create_env_blob(sector_meta_data_t sector, const char *key, const void *value, size_t len)
{
    EfErrCode result = EF_NO_ERR;
    struct env_hdr_data env_hdr;
    bool is_full = false;
    uint32_t env_addr = sector->empty_env;

    if (strlen(key) > EF_ENV_NAME_MAX) {
        EF_INFO("Error: The ENV name length is more than %d\n", EF_ENV_NAME_MAX);
        return EF_ENV_NAME_ERR;
    }

    memset(&env_hdr, 0xFF, sizeof(struct env_hdr_data));
    env_hdr.magic = ENV_MAGIC_WORD;
    env_hdr.name_len = strlen(key);
    env_hdr.value_len = len;
    env_hdr.len = ENV_HDR_DATA_SIZE + EF_WG_ALIGN(env_hdr.name_len) + EF_WG_ALIGN(env_hdr.value_len);

    if (env_hdr.len > SECTOR_SIZE - SECTOR_HDR_DATA_SIZE) {
        EF_INFO("Error: The ENV size is too big\n");
        return EF_ENV_FULL;
    }

    if (env_addr != FAILED_ADDR || (env_addr = new_env(sector, env_hdr.len)) != FAILED_ADDR) {
        size_t align_remain;
        /* update the sector status */
        if (result == EF_NO_ERR) {
            result = update_sec_status(sector, env_hdr.len, &is_full);
        }
        if (result == EF_NO_ERR) {
            uint8_t ff = 0xFF;
            /* start calculate CRC32 */
            env_hdr.crc32 = ef_calc_crc32(0, &env_hdr.name_len, ENV_HDR_DATA_SIZE - ENV_NAME_LEN_OFFSET);
            env_hdr.crc32 = ef_calc_crc32(env_hdr.crc32, key, env_hdr.name_len);
            align_remain = EF_WG_ALIGN(env_hdr.name_len) - env_hdr.name_len;
            while (align_remain--) {
                env_hdr.crc32 = ef_calc_crc32(env_hdr.crc32, &ff, 1);
            }
            env_hdr.crc32 = ef_calc_crc32(env_hdr.crc32, value, env_hdr.value_len);
            align_remain = EF_WG_ALIGN(env_hdr.value_len) - env_hdr.value_len;
            while (align_remain--) {
                env_hdr.crc32 = ef_calc_crc32(env_hdr.crc32, &ff, 1);
            }
            /* write ENV header data */
            result = write_env_hdr(env_addr, &env_hdr);

        }
        /* write key name */
        if (result == EF_NO_ERR) {
            result = align_write(env_addr + ENV_HDR_DATA_SIZE, (uint32_t *) key, env_hdr.name_len);

#ifdef EF_ENV_USING_CACHE
            if (!is_full) {
                update_sector_cache(sector->addr,
                        env_addr + ENV_HDR_DATA_SIZE + EF_WG_ALIGN(env_hdr.name_len) + EF_WG_ALIGN(env_hdr.value_len));
            }
            update_env_cache(key, env_hdr.name_len, env_addr);
#endif /* EF_ENV_USING_CACHE */
        }
        /* write value */
        if (result == EF_NO_ERR) {
            result = align_write(env_addr + ENV_HDR_DATA_SIZE + EF_WG_ALIGN(env_hdr.name_len), value,
                    env_hdr.value_len);
        }
        /* change the ENV status to ENV_WRITE */
        if (result == EF_NO_ERR) {
            result = write_status(env_addr, env_hdr.status_table, ENV_STATUS_NUM, ENV_WRITE);
        }
        /* trigger GC collect when current sector is full */
        if (result == EF_NO_ERR && is_full) {
            EF_DEBUG("Trigger a GC check after created ENV.\n");
            gc_request = true;
        }
    } else {
        result = EF_ENV_FULL;
    }

    return result;
}

/**
 * Delete an ENV.
 *
 * @param key ENV name
 *
 * @return result
 */
EfErrCode ef_del_env(const char *key)
{
    EfErrCode result = EF_NO_ERR;

    if (!init_ok) {
        EF_INFO("Error: ENV isn't initialize OK.\n");
        return EF_ENV_INIT_FAILED;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    result = del_env(key, NULL, true);

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

/**
 * The same to ef_del_env on this mode
 * It's compatibility with older versions (less then V4.0).
 *
 * @note this function is DEPRECATED
 *
 * @param key ENV name
 *
 * @return result
 */
EfErrCode ef_del_and_save_env(const char *key)
{
    return ef_del_env(key);
}

static EfErrCode set_env(const char *key, const void *value_buf, size_t buf_len)
{
    EfErrCode result = EF_NO_ERR;
    static struct env_node_obj env;
    static struct sector_meta_data sector;
    bool env_is_found = false;

    if (value_buf == NULL) {
        result = del_env(key, NULL, true);
    } else {
        /* make sure the flash has enough space */
        if (new_env_by_kv(&sector, strlen(key), buf_len) == FAILED_ADDR) {
            return EF_ENV_FULL;
        }
        env_is_found = find_env(key, &env);
        /* prepare to delete the old ENV */
        if (env_is_found) {
            result = del_env(key, &env, false);
        }
        /* create the new ENV */
        if (result == EF_NO_ERR) {
            result = create_env_blob(&sector, key, value_buf, buf_len);
        }
        /* delete the old ENV */
        if (env_is_found && result == EF_NO_ERR) {
            result = del_env(key, &env, true);
        }
        /* process the GC after set ENV */
        if (gc_request) {
            gc_collect();
        }
    }

    return result;
}

/**
 * Set a blob ENV. If it value is NULL, delete it.
 * If not find it in flash, then create it.
 *
 * @param key ENV name
 * @param value ENV value
 * @param len ENV value length
 *
 * @return result
 */
EfErrCode ef_set_env_blob(const char *key, const void *value_buf, size_t buf_len)
{
    EfErrCode result = EF_NO_ERR;


    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return EF_ENV_INIT_FAILED;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    result = set_env(key, value_buf, buf_len);

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

/**
 * Set a string ENV. If it value is NULL, delete it.
 * If not find it in flash, then create it.
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
EfErrCode ef_set_env(const char *key, const char *value)
{
    return ef_set_env_blob(key, value, strlen(value));
}

/**
 * The same to ef_set_env on this mode.
 * It's compatibility with older versions (less then V4.0).
 *
 * @note this function is DEPRECATED
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
EfErrCode ef_set_and_save_env(const char *key, const char *value)
{
    return ef_set_env_blob(key, value, strlen(value));
}

/**
 * Save ENV to flash.
 *
 * @note this function is DEPRECATED
 */
EfErrCode ef_save_env(void)
{
    /* do nothing not cur mode */
    return EF_NO_ERR;
}

/**
 * ENV set default.
 *
 * @return result
 */
EfErrCode ef_env_set_default(void)
{
    EfErrCode result = EF_NO_ERR;
    uint32_t addr, i, value_len;
    struct sector_meta_data sector;

    EF_ASSERT(default_env_set);
    EF_ASSERT(default_env_set_size);

    /* lock the ENV cache */
    ef_port_env_lock();
    /* format all sectors */
    for (addr = env_start_addr; addr < env_start_addr + ENV_AREA_SIZE; addr += SECTOR_SIZE) {
        result = format_sector(addr, SECTOR_NOT_COMBINED);
        if (result != EF_NO_ERR) {
            goto __exit;
        }
    }
    /* create default ENV */
    for (i = 0; i < default_env_set_size; i++) {
        /* It seems to be a string when value length is 0.
         * This mechanism is for compatibility with older versions (less then V4.0). */
        if (default_env_set[i].value_len == 0) {
            value_len = strlen(default_env_set[i].value);
        } else {
            value_len = default_env_set[i].value_len;
        }
        sector.empty_env = FAILED_ADDR;
        create_env_blob(&sector, default_env_set[i].key, default_env_set[i].value, value_len);
        if (result != EF_NO_ERR) {
            goto __exit;
        }
    }

__exit:
    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

static bool print_env_cb(env_node_obj_t env, void *arg1, void *arg2)
{
    bool value_is_str = true, print_value = false;
    size_t *using_size = arg1;

    if (env->crc_is_ok) {
        /* calculate the total using flash size */
        *using_size += env->len;
        /* check ENV */
        if (env->status == ENV_WRITE) {
            ef_print("%.*s=", env->name_len, env->name);

            if (env->value_len < EF_STR_ENV_VALUE_MAX_SIZE ) {
                uint8_t buf[32];
                size_t len, size;
__reload:
                /* check the value is string */
                for (len = 0, size = 0; len < env->value_len; len += size) {
                    if (len + sizeof(buf) < env->value_len) {
                        size = sizeof(buf);
                    } else {
                        size = env->value_len - len;
                    }
                    ef_port_read(env->addr.value + len, (uint32_t *) buf, EF_WG_ALIGN(size));
                    if (print_value) {
                        ef_print("%.*s", size, buf);
                    } else if (!ef_is_str(buf, size)) {
                        value_is_str = false;
                        break;
                    }
                }
            } else {
                value_is_str = false;
            }
            if (value_is_str && !print_value) {
                print_value = true;
                goto __reload;
            } else if (!value_is_str) {
                ef_print("blob @0x%08X %dbytes", env->addr.value, env->value_len);
            }
            ef_print("\n");
        }
    }

    return false;
}


/**
 * Print ENV.
 */
void ef_print_env(void)
{
    struct env_node_obj env;
    size_t using_size = 0;

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    env_iterator(&env, &using_size, NULL, print_env_cb);

    ef_print("\nmode: next generation\n");
    ef_print("size: %lu/%lu bytes.\n", using_size + (SECTOR_NUM - EF_GC_EMPTY_SEC_THRESHOLD) * SECTOR_HDR_DATA_SIZE,
            ENV_AREA_SIZE - SECTOR_SIZE * EF_GC_EMPTY_SEC_THRESHOLD);

    /* unlock the ENV cache */
    ef_port_env_unlock();
}

#ifdef EF_ENV_AUTO_UPDATE
/*
 * Auto update ENV to latest default when current EF_ENV_VER_NUM is changed.
 */
static void env_auto_update(void)
{
    size_t saved_ver_num, setting_ver_num = EF_ENV_VER_NUM;

    if (get_env(VER_NUM_ENV_NAME, &saved_ver_num, sizeof(size_t), NULL) > 0) {
        /* check version number */
        if (saved_ver_num != setting_ver_num) {
            struct env_node_obj env;
            size_t i, value_len;
            struct sector_meta_data sector;
            EF_DEBUG("Update the ENV from version %d to %d.\n", saved_ver_num, setting_ver_num);
            for (i = 0; i < default_env_set_size; i++) {
                /* add a new ENV when it's not found */
                if (!find_env(default_env_set[i].key, &env)) {
                    /* It seems to be a string when value length is 0.
                     * This mechanism is for compatibility with older versions (less then V4.0). */
                    if (default_env_set[i].value_len == 0) {
                        value_len = strlen(default_env_set[i].value);
                    } else {
                        value_len = default_env_set[i].value_len;
                    }
                    sector.empty_env = FAILED_ADDR;
                    create_env_blob(&sector, default_env_set[i].key, default_env_set[i].value, value_len);
                }
            }
        } else {
            /* version number not changed now return */
            return;
        }
    }

    set_env(VER_NUM_ENV_NAME, &setting_ver_num, sizeof(size_t));
}
#endif /* EF_ENV_AUTO_UPDATE */

static bool check_sec_hdr_cb(sector_meta_data_t sector, void *arg1, void *arg2)
{
    if (!sector->check_ok) {
        size_t *failed_count = arg1;

        EF_INFO("Warning: Sector header check failed. Format this sector (0x%08x).\n", sector->addr);
        (*failed_count) ++;
        format_sector(sector->addr, SECTOR_NOT_COMBINED);
    }

    return false;
}

static bool check_and_recovery_gc_cb(sector_meta_data_t sector, void *arg1, void *arg2)
{
    if (sector->check_ok && sector->status.dirty == SECTOR_DIRTY_GC) {
        /* make sure the GC request flag to true */
        gc_request = true;
        /* resume the GC operate */
        gc_collect();
    }

    return false;
}

static bool check_and_recovery_env_cb(env_node_obj_t env, void *arg1, void *arg2)
{
    /* recovery the prepare deleted ENV */
    if (env->crc_is_ok && env->status == ENV_PRE_DELETE) {
        EF_INFO("Found an ENV (%.*s) which has changed value failed. Now will recovery it.\n", env->name_len, env->name);
        /* recovery the old ENV */
        if (move_env(env) == EF_NO_ERR) {
            EF_DEBUG("Recovery the ENV successful.\n");
        } else {
            EF_DEBUG("Warning: Moved an ENV (size %d) failed when recovery. Now will GC then retry.\n", env->len);
            return true;
        }
    } else if (env->status == ENV_PRE_WRITE) {
        uint8_t status_table[ENV_STATUS_TABLE_SIZE];
        /* the ENV has not write finish, change the status to error */
        //TODO �����쳣������״̬װ��ͼ
        write_status(env->addr.start, status_table, ENV_STATUS_NUM, ENV_ERR_HDR);
        return true;
    }

    return false;
}

/**
 * Check and load the flash ENV meta data.
 *
 * @return result
 */
EfErrCode ef_load_env(void)
{
    EfErrCode result = EF_NO_ERR;
    struct env_node_obj env;
    struct sector_meta_data sector;
    size_t check_failed_count = 0;

    in_recovery_check = true;
    /* check all sector header */
    sector_iterator(&sector, SECTOR_STORE_UNUSED, &check_failed_count, NULL, check_sec_hdr_cb, false);
    /* all sector header check failed */
    if (check_failed_count == SECTOR_NUM) {
        EF_INFO("Warning: All sector header check failed. Set it to default.\n");
        ef_env_set_default();
    }

    /* lock the ENV cache */
    ef_port_env_lock();
    /* check all sector header for recovery GC */
    sector_iterator(&sector, SECTOR_STORE_UNUSED, NULL, NULL, check_and_recovery_gc_cb, false);

__retry:
    /* check all ENV for recovery */
    env_iterator(&env, NULL, NULL, check_and_recovery_env_cb);
    if (gc_request) {
        gc_collect();
        goto __retry;
    }

    in_recovery_check = false;

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

/**
 * Flash ENV initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV set size
 *
 * @return result
 */
EfErrCode ef_env_init(ef_env const *default_env, size_t default_env_size) {
    EfErrCode result = EF_NO_ERR;

#ifdef EF_ENV_USING_CACHE
    size_t i;
#endif

    EF_ASSERT(default_env);
    EF_ASSERT(ENV_AREA_SIZE);
    /* must be aligned with erase_min_size */
    EF_ASSERT(ENV_AREA_SIZE % EF_ERASE_MIN_SIZE == 0);
    /* sector number must be greater than or equal to 2 */
    EF_ASSERT(SECTOR_NUM >= 2);
    /* must be aligned with write granularity */
    EF_ASSERT((EF_STR_ENV_VALUE_MAX_SIZE * 8) % EF_WRITE_GRAN == 0);

    if (init_ok) {
        return EF_NO_ERR;
    }

#ifdef EF_ENV_USING_CACHE
    for (i = 0; i < EF_SECTOR_CACHE_TABLE_SIZE; i++) {
        sector_cache_table[i].addr = FAILED_ADDR;
    }
    for (i = 0; i < EF_ENV_CACHE_TABLE_SIZE; i++) {
        env_cache_table[i].addr = FAILED_ADDR;
    }
#endif /* EF_ENV_USING_CACHE */

    env_start_addr = EF_START_ADDR;
    default_env_set = default_env;
    default_env_set_size = default_env_size;

    EF_DEBUG("ENV start address is 0x%08X, size is %d bytes.\n", EF_START_ADDR, ENV_AREA_SIZE);

    result = ef_load_env();

#ifdef EF_ENV_AUTO_UPDATE
    if (result == EF_NO_ERR) {
        env_auto_update();
    }
#endif

    if (result == EF_NO_ERR) {
        init_ok = true;
    }

    return result;
}

#endif /* defined(EF_USING_ENV) && !defined(EF_ENV_USING_LEGACY_MODE) */
