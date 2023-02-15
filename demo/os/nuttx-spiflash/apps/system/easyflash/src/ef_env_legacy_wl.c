/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2018, Armink, <armink.ztl@gmail.com>
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
 * Function: Environment variables operating interface. (wear leveling mode)
 * Created on: 2015-02-11
 */

#include <easyflash.h>
#include <string.h>
#include <stdlib.h>

#if defined(EF_USING_ENV) && defined(EF_ENV_USING_LEGACY_MODE)

#ifdef EF_ENV_USING_WL_MODE

#if defined(EF_USING_ENV) && (!defined(ENV_USER_SETTING_SIZE) || !defined(ENV_AREA_SIZE))
#error "Please configure user setting ENV size or ENV area size (in ef_cfg.h)"
#endif

/**
 * ENV area has 2 sections
 * 1. System section
 *    Storage ENV current using data section address.
 *    Units: Word. Total size: @see EF_ERASE_MIN_SIZE.
 * 2. Data section
 *    The data section storage ENV's parameters and detail.
 *    When an exception has occurred on flash erase or write. The current using data section
 *    address will move to next available position. This position depends on EF_ERASE_MIN_SIZE.
 *    2.1 ENV parameters part
 *        It storage ENV's parameters.
 *    2.2 ENV detail part
 *        It storage all ENV. Storage format is key=value\0.
 *        All ENV must be 4 bytes alignment. The remaining part must fill '\0'.
 *
 * @note Word = 4 Bytes in this file
 * @note It will has two ENV areas(Area0, Area1) in data section when used power fail safeguard mode.
 */

/* flash ENV parameters part index and size */
enum {
    /* data section ENV detail part end address index */
    ENV_PARAM_PART_INDEX_END_ADDR = 0,

#ifdef EF_ENV_USING_PFS_MODE
    /* saved count for ENV area */
    ENV_PARAM_PART_INDEX_SAVED_COUNT,
#endif

#ifdef EF_ENV_AUTO_UPDATE
    /* current version number for ENV */
    ENV_PARAM_INDEX_VER_NUM,
#endif

    /* data section CRC32 code index */
    ENV_PARAM_PART_INDEX_DATA_CRC,
    /* ENV parameters part word size */
    ENV_PARAM_PART_WORD_SIZE,
    /* ENV parameters part byte size */
    ENV_PARAM_PART_BYTE_SIZE = ENV_PARAM_PART_WORD_SIZE * 4,
};

/* default ENV set, must be initialized by user */
static ef_env const *default_env_set;
/* default ENV set size, must be initialized by user */
static size_t default_env_set_size = 0;
/* flash ENV data section size */
static size_t env_data_section_size = 0;
/* ENV ram cache */
static uint32_t env_cache[ENV_USER_SETTING_SIZE / 4] = { 0 };
/* ENV start address in flash */
static uint32_t env_start_addr = 0;
/* current using data section address */
static uint32_t cur_using_data_addr = 0;
/* ENV ram cache has changed when ENV created, deleted and changed value. */
static bool env_cache_changed = false;
/* initialize OK flag */
static bool init_ok = false;

#ifdef EF_ENV_USING_PFS_MODE
/* next save ENV area address */
static uint32_t next_save_area_addr = 0;
#endif

static uint32_t get_env_start_addr(void);
static uint32_t get_cur_using_data_addr(void);
static uint32_t get_env_detail_addr(void);
static uint32_t get_env_detail_end_addr(void);
static void set_cur_using_data_addr(uint32_t using_data_addr);
static void set_env_detail_end_addr(uint32_t end_addr);
static EfErrCode write_env(const char *key, const char *value);
static char *find_env(const char *key);
static size_t get_env_detail_size(void);
static size_t get_env_user_used_size(void);
static EfErrCode create_env(const char *key, const char *value);
static EfErrCode del_env(const char *key);
static EfErrCode save_cur_using_data_addr(uint32_t cur_data_addr);
static uint32_t calc_env_crc(void);
static bool env_crc_is_ok(void);
#ifdef EF_ENV_AUTO_UPDATE
static EfErrCode env_auto_update(void);
#endif

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

    EF_ASSERT(ENV_AREA_SIZE);
    EF_ASSERT(ENV_USER_SETTING_SIZE);
    /* must be word alignment for ENV */
    EF_ASSERT(ENV_USER_SETTING_SIZE % 4 == 0);
    EF_ASSERT(ENV_AREA_SIZE % 4 == 0);
    EF_ASSERT(default_env);
    EF_ASSERT(default_env_size < ENV_USER_SETTING_SIZE);

#ifndef EF_ENV_USING_PFS_MODE
    /* system section size is erase_min_size, so last part is data section */
    env_data_section_size = ENV_AREA_SIZE - EF_ERASE_MIN_SIZE;
#else
    /* system section size is erase_min_size, so last part is data section */
    env_data_section_size = ENV_AREA_SIZE / 2 - EF_ERASE_MIN_SIZE;
    EF_ASSERT((ENV_AREA_SIZE / EF_ERASE_MIN_SIZE) % 2 == 0);
#endif
    EF_ASSERT(env_data_section_size >= ENV_USER_SETTING_SIZE);
    /* the ENV data section size should be an integral multiple of erase minimum size. */
    EF_ASSERT(env_data_section_size % EF_ERASE_MIN_SIZE == 0);


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

/**
 * ENV set default.
 *
 * @return result
 */
EfErrCode ef_env_set_default(void) {
    EfErrCode result = EF_NO_ERR;
    size_t i;

    EF_ASSERT(default_env_set);
    EF_ASSERT(default_env_set_size);

    /* lock the ENV cache */
    ef_port_env_lock();

    /* set ENV detail part end address is at ENV detail part start address */
    set_env_detail_end_addr(get_env_detail_addr());

#ifdef EF_ENV_USING_PFS_MODE
    /* set saved count to default 0 */
    env_cache[ENV_PARAM_PART_INDEX_SAVED_COUNT] = 0;
#endif

#ifdef EF_ENV_AUTO_UPDATE
    /* initialize version number */
    env_cache[ENV_PARAM_INDEX_VER_NUM] = EF_ENV_VER_NUM;
#endif

    /* create default ENV */
    for (i = 0; i < default_env_set_size; i++) {
        create_env(default_env_set[i].key, default_env_set[i].value);
    }

    /* unlock the ENV cache */
    ef_port_env_unlock();

    result = ef_save_env();

#ifdef EF_ENV_USING_PFS_MODE
    /* reset other PFS area's data */
    if (result == EF_NO_ERR) {
        env_cache_changed = true;
        result = ef_save_env();
    }
#endif

    return result;
}

/**
 * Get ENV start address in flash.
 *
 * @return ENV start address in flash
 */
static uint32_t get_env_start_addr(void) {
    return env_start_addr;
}
/**
 * Get current using data section address.
 *
 * @return current using data section address
 */
static uint32_t get_cur_using_data_addr(void) {
    return cur_using_data_addr;
}

/**
 * Set current using data section address.
 *
 * @param using_data_addr current using data section address
 */
static void set_cur_using_data_addr(uint32_t using_data_addr) {
    cur_using_data_addr = using_data_addr;
}

/**
 * Get ENV detail part start address.
 *
 * @return detail part start address
 */
static uint32_t get_env_detail_addr(void) {
    return get_cur_using_data_addr() + ENV_PARAM_PART_BYTE_SIZE;
}

/**
 * Get ENV detail part end address.
 * It's the first word in ENV.
 *
 * @return ENV end address
 */
static uint32_t get_env_detail_end_addr(void) {
    /* it is the first word */
    return env_cache[ENV_PARAM_PART_INDEX_END_ADDR];
}

/**
 * Set ENV detail part end address.
 * It's the first word in ENV.
 *
 * @param end_addr ENV end address
 */
static void set_env_detail_end_addr(uint32_t end_addr) {
    env_cache[ENV_PARAM_PART_INDEX_END_ADDR] = end_addr;
}

/**
 * Get current ENV detail part size.
 *
 * @return size
 */
static size_t get_env_detail_size(void) {
    if (get_env_detail_end_addr() > get_env_detail_addr()) {
        return get_env_detail_end_addr() - get_env_detail_addr();
    } else {
        return 0;
    }
}

/**
 * Get current user used ENV size.
 *
 * @see ENV_USER_SETTING_SIZE
 *
 * @return size
 */
/* must be initialized */
static size_t get_env_user_used_size(void) {
    if (get_env_detail_end_addr() > get_cur_using_data_addr()) {
        return get_env_detail_end_addr() - get_cur_using_data_addr();
    } else {
        return 0;
    }
}

/**
 * Get current ENV already write bytes.
 *
 * @return write bytes
 */
size_t ef_get_env_write_bytes(void) {
#ifndef EF_ENV_USING_PFS_MODE
    return get_env_detail_end_addr() - get_env_start_addr();
#else
    return EF_ERASE_MIN_SIZE + get_env_detail_end_addr() - get_cur_using_data_addr();
#endif
}

/**
 * Write an ENV at the end of cache.
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
static EfErrCode write_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;
    size_t ker_len = strlen(key), value_len = strlen(value), env_str_len;
    char *env_cache_bak = (char *)env_cache;

    /* calculate ENV storage length, contain '=' and '\0'. */
    env_str_len = ker_len + value_len + 2;
    if (env_str_len % 4 != 0) {
        env_str_len = (env_str_len / 4 + 1) * 4;
    }
    /* check capacity of ENV  */
    if (env_str_len + get_env_user_used_size() >= ENV_USER_SETTING_SIZE) {
        return EF_ENV_FULL;
    }
    /* calculate current ENV ram cache end address */
    env_cache_bak += ENV_PARAM_PART_BYTE_SIZE + get_env_detail_size();
    /* copy key name */
    memcpy(env_cache_bak, key, ker_len);
    env_cache_bak += ker_len;
    /* copy equal sign */
    *env_cache_bak = '=';
    env_cache_bak++;
    /* copy value */
    memcpy(env_cache_bak, value, value_len);
    env_cache_bak += value_len;
    /* fill '\0' for string end sign */
    *env_cache_bak = '\0';
    env_cache_bak ++;
    /* fill '\0' for word alignment */
    memset(env_cache_bak, 0, env_str_len - (ker_len + value_len + 2));
    set_env_detail_end_addr(get_env_detail_end_addr() + env_str_len);
    /* ENV ram cache has changed */
    env_cache_changed = true;

    return result;
}

/**
 * Find ENV.
 *
 * @param key ENV name
 *
 * @return found ENV in ram cache
 */
static char *find_env(const char *key) {
    char *env_start, *env_end, *env, *found_env = NULL;
    size_t key_len = strlen(key), env_len;

    if ((key == NULL) || (*key == '\0')) {
        EF_INFO("Flash ENV name must be not empty!\n");
        return NULL;
    }

    /* from data section start to data section end */
    env_start = (char *) ((char *) env_cache + ENV_PARAM_PART_BYTE_SIZE);
    env_end = (char *) ((char *) env_cache + ENV_PARAM_PART_BYTE_SIZE + get_env_detail_size());

    /* ENV is null */
    if (env_start == env_end) {
        return NULL;
    }

    env = env_start;
    while (env < env_end) {
        /* the key length must be equal */
        if (!strncmp(env, key, key_len) && (env[key_len] == '=')) {
            found_env = env;
            break;
        } else {
            /* calculate ENV length, contain '\0'. */
            env_len = strlen(env) + 1;
            /* next ENV and word alignment */
            if (env_len % 4 == 0) {
                env += env_len;
            } else {
                env += (env_len / 4 + 1) * 4;
            }
        }
    }

    return found_env;
}

/**
 * If the ENV is not exist, create it.
 * @see flash_write_env
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
static EfErrCode create_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;

    EF_ASSERT(key);
    EF_ASSERT(value);

    if ((key == NULL) || (*key == '\0')) {
        EF_INFO("Flash ENV name must be not empty!\n");
        return EF_ENV_NAME_ERR;
    }

    if (strchr(key, '=')) {
        EF_INFO("Flash ENV name can't contain '='.\n");
        return EF_ENV_NAME_ERR;
    }

    /* find ENV */
    if (find_env(key)) {
        EF_INFO("The name of \"%s\" is already exist.\n", key);
        return EF_ENV_NAME_EXIST;
    }
    /* write ENV at the end of cache */
    result = write_env(key, value);

    return result;
}

/**
 * Delete an ENV in cache.
 *
 * @param key ENV name
 *
 * @return result
 */
static EfErrCode del_env(const char *key) {
    EfErrCode result = EF_NO_ERR;
    char *del_env = NULL;
    size_t del_env_length, remain_env_length;

    EF_ASSERT(key);

    if ((key == NULL) || (*key == '\0')) {
        EF_INFO("Flash ENV name must be not NULL!\n");
        return EF_ENV_NAME_ERR;
    }

    if (strchr(key, '=')) {
        EF_INFO("Flash ENV name or value can't contain '='.\n");
        return EF_ENV_NAME_ERR;
    }

    /* find ENV */
    del_env = find_env(key);

    if (!del_env) {
        EF_INFO("Not find \"%s\" in ENV.\n", key);
        return EF_ENV_NAME_ERR;
    }
    del_env_length = strlen(del_env);
    /* '\0' also must be as ENV length */
    del_env_length ++;
    /* the address must multiple of 4 */
    if (del_env_length % 4 != 0) {
        del_env_length = (del_env_length / 4 + 1) * 4;
    }
    /* calculate remain ENV length */
    remain_env_length = get_env_detail_size()
                        - (((uint32_t) del_env + del_env_length) - ((uint32_t) env_cache + ENV_PARAM_PART_BYTE_SIZE));
    /* remain ENV move forward */
    memcpy(del_env, del_env + del_env_length, remain_env_length);
    /* reset ENV end address */
    set_env_detail_end_addr(get_env_detail_end_addr() - del_env_length);
    /* ENV ram cache has changed */
    env_cache_changed = true;

    return result;
}

/**
 * Set an ENV.If it value is NULL, delete it.
 * If not find it in ENV table, then create it.
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
EfErrCode ef_set_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;
    char *old_env, *old_value;

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return EF_ENV_INIT_FAILED;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    /* if ENV value is NULL, delete it */
    if (value == NULL) {
        result = del_env(key);
    } else {
        old_env = find_env(key);
        /* If find this ENV, then compare the new value and old value. */
        if (old_env) {
            /* find the old value address */
            old_env = strchr(old_env, '=');
            old_value = old_env + 1;
            /* If it is changed then delete it and recreate it  */
            if (strcmp(old_value, value)) {
                result = del_env(key);
                if (result == EF_NO_ERR) {
                    result = create_env(key, value);
                }
            }
        } else {
            result = create_env(key, value);
        }
    }
    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

/**
 * Del an ENV.
 *
 * @param key ENV name
 *
 * @return result
 */
EfErrCode ef_del_env(const char *key) {
    EfErrCode result = EF_NO_ERR;

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return EF_ENV_INIT_FAILED;
    }

    /* lock the ENV cache */
    ef_port_env_lock();

    result = del_env(key);

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return result;
}

/**
 * Get an ENV value by key name.
 *
 * @param key ENV name
 *
 * @return value
 */
char *ef_get_env(const char *key) {
    char *env = NULL, *value = NULL;

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return NULL;
    }

    /* find ENV */
    env = find_env(key);

    if (env == NULL) {
        return NULL;
    }
    /* get value address */
    value = strchr(env, '=');
    if (value != NULL) {
        /* the equal sign next character is value */
        value++;
    }
    return value;
}
/**
 * Print ENV.
 */
void ef_print_env(void) {
    uint32_t *env_cache_detail_addr = env_cache + ENV_PARAM_PART_WORD_SIZE, *env_cache_end_addr =
                                          (uint32_t *) (env_cache + ENV_PARAM_PART_WORD_SIZE + get_env_detail_size() / 4);
    uint8_t j;
    char c;

    if (!init_ok) {
        EF_INFO("ENV isn't initialize OK.\n");
        return;
    }

    for (; env_cache_detail_addr < env_cache_end_addr; env_cache_detail_addr += 1) {
        for (j = 0; j < 4; j++) {
            c = (*env_cache_detail_addr) >> (8 * j);
            ef_print("%c", c);
            if (c == '\0') {
                ef_print("\n");
                break;
            }
        }
    }

#ifndef EF_ENV_USING_PFS_MODE
    ef_print("\nmode: wear leveling\n");
    ef_print("size: %ld/%ld bytes, write bytes %ld/%ld.\n", get_env_user_used_size(), ENV_USER_SETTING_SIZE,
             ef_get_env_write_bytes(), ENV_AREA_SIZE);
#else
    ef_print("\nmode: wear leveling and power fail safeguard\n");
    ef_print("size: %ld/%ld bytes, write bytes %ld/%ld.\n", get_env_user_used_size(), ENV_USER_SETTING_SIZE,
             ef_get_env_write_bytes(), ENV_AREA_SIZE / 2);
    ef_print("saved count: %ld\n", env_cache[ENV_PARAM_PART_INDEX_SAVED_COUNT]);
#endif

#ifdef EF_ENV_AUTO_UPDATE
    ef_print("ver num: %d\n", env_cache[ENV_PARAM_INDEX_VER_NUM]);
#endif
}

/**
 * Load flash ENV to ram.
 *
 * @return result
 */
#ifndef EF_ENV_USING_PFS_MODE
EfErrCode ef_load_env(void) {
    EfErrCode result = EF_NO_ERR;
    uint32_t *env_cache_bak, env_end_addr, using_data_addr;

    /* read current using data section address */
    ef_port_read(get_env_start_addr(), &using_data_addr, 4);
    /* if ENV is not initialize or flash has dirty data, set default for it */
    if ((using_data_addr == 0xFFFFFFFF)
            || (using_data_addr > get_env_start_addr() + ENV_AREA_SIZE)
            || (using_data_addr < get_env_start_addr() + EF_ERASE_MIN_SIZE)) {
        /* initialize current using data section address */
        set_cur_using_data_addr(get_env_start_addr() + EF_ERASE_MIN_SIZE);
        /* save current using data section address to flash*/
        if ((result = save_cur_using_data_addr(get_cur_using_data_addr())) == EF_NO_ERR) {
            /* set default ENV */
            result = ef_env_set_default();
        }
    } else {
        /* set current using data section address */
        set_cur_using_data_addr(using_data_addr);
        /* read ENV detail part end address from flash */
        ef_port_read(get_cur_using_data_addr() + ENV_PARAM_PART_INDEX_END_ADDR * 4, &env_end_addr, 4);
        /* if ENV end address has error, set default for ENV */
        if (env_end_addr > get_env_start_addr() + ENV_AREA_SIZE) {
            /* initialize current using data section address */
            set_cur_using_data_addr(get_env_start_addr() + EF_ERASE_MIN_SIZE);
            /* save current using data section address to flash*/
            if ((result = save_cur_using_data_addr(get_cur_using_data_addr())) == EF_NO_ERR) {
                EF_INFO("Warning: ENV end address has error. Set it to default.\n");
                result = ef_env_set_default();
            }
        } else {
            /* set ENV detail part end address */
            set_env_detail_end_addr(env_end_addr);

            env_cache_bak = env_cache + ENV_PARAM_PART_WORD_SIZE;
            /* read all ENV from flash */
            ef_port_read(get_env_detail_addr(), env_cache_bak, get_env_detail_size());
            /* read ENV CRC code from flash */
            ef_port_read(get_cur_using_data_addr() + ENV_PARAM_PART_INDEX_DATA_CRC * 4,
                         &env_cache[ENV_PARAM_PART_INDEX_DATA_CRC], 4);
            /* if ENV CRC32 check is fault, set default for it */
            if (!env_crc_is_ok()) {
                EF_INFO("Warning: ENV CRC check failed. Set it to default.\n");
                result = ef_env_set_default();
            }
        }

    }
    return result;
}
#else
EfErrCode ef_load_env(void) {
    EfErrCode result = EF_NO_ERR;
    /* ENV area0 current using address default value */
    uint32_t area0_default_cur_using_addr = get_env_start_addr() + EF_ERASE_MIN_SIZE;
    /* ENV area1 current using address default value */
    uint32_t area1_default_cur_using_addr = area0_default_cur_using_addr + ENV_AREA_SIZE / 2;
    uint32_t area0_cur_using_addr, area1_cur_using_addr, area0_end_addr, area1_end_addr;
    uint32_t area0_crc, area1_crc, area0_saved_count, area1_saved_count;
    bool area0_is_valid = true, area1_is_valid = true;

    /* read ENV area0 and area1 current using data section address */
    ef_port_read(get_env_start_addr(), &area0_cur_using_addr, 4);
    ef_port_read(get_env_start_addr() + ENV_AREA_SIZE / 2, &area1_cur_using_addr, 4);
    /* if ENV is not initialize or flash has dirty data, set it isn't valid */
    if ((area0_cur_using_addr == 0xFFFFFFFF)
            || (area0_cur_using_addr > get_env_start_addr() + ENV_AREA_SIZE / 2)
            || (area0_cur_using_addr < get_env_start_addr() + EF_ERASE_MIN_SIZE)) {
        area0_is_valid = false;
    }
    if ((area1_cur_using_addr == 0xFFFFFFFF)
            || (area1_cur_using_addr > get_env_start_addr() + ENV_AREA_SIZE)
            || (area1_cur_using_addr < get_env_start_addr() + ENV_AREA_SIZE / 2 + EF_ERASE_MIN_SIZE)) {
        area1_is_valid = false;
    }
    /* check area0 end address when it is valid */
    if (area0_is_valid) {
        /* read ENV area end address from flash */
        ef_port_read(area0_cur_using_addr + ENV_PARAM_PART_INDEX_END_ADDR * 4, &area0_end_addr, 4);
        if ((area0_end_addr == 0xFFFFFFFF) || (area0_end_addr < area0_cur_using_addr)
                || (area0_end_addr > area0_cur_using_addr + ENV_USER_SETTING_SIZE)) {
            area0_is_valid = false;
        }
    }
    /* check area1 end address when it is valid */
    if (area1_is_valid) {
        /* read ENV area end address from flash */
        ef_port_read(area1_cur_using_addr + ENV_PARAM_PART_INDEX_END_ADDR * 4, &area1_end_addr, 4);
        if ((area1_end_addr == 0xFFFFFFFF) || (area1_end_addr < area1_cur_using_addr)
                || (area1_end_addr > area1_cur_using_addr + ENV_USER_SETTING_SIZE)) {
            area1_is_valid = false;
        }
    }
    /* check area0 CRC when it is valid */
    if (area0_is_valid) {
        /* read ENV area0 crc32 code from flash */
        ef_port_read(area0_cur_using_addr + ENV_PARAM_PART_INDEX_DATA_CRC * 4, &area0_crc, 4);
        /* read ENV from ENV area0 */
        ef_port_read(area0_cur_using_addr, env_cache, area0_end_addr - area0_cur_using_addr);
        /* current using data section address is area0 current using data section address */
        set_cur_using_data_addr(area0_cur_using_addr);
        if (!env_crc_is_ok()) {
            area0_is_valid = false;
        }
    }
    /* check area1 CRC when it is valid */
    if (area1_is_valid) {
        /* read ENV area1 crc32 code from flash */
        ef_port_read(area1_cur_using_addr + ENV_PARAM_PART_INDEX_DATA_CRC * 4, &area1_crc, 4);
        /* read ENV from ENV area1 */
        ef_port_read(area1_cur_using_addr, env_cache, area1_end_addr - area1_cur_using_addr);
        /* current using data section address is area1 current using data section address */
        set_cur_using_data_addr(area1_cur_using_addr);
        if (!env_crc_is_ok()) {
            area1_is_valid = false;
        }
    }
    /* all ENV area CRC is OK then compare saved count */
    if (area0_is_valid && area1_is_valid) {
        /* read ENV area saved count from flash */
        ef_port_read(area0_cur_using_addr + ENV_PARAM_PART_INDEX_SAVED_COUNT * 4,
                     &area0_saved_count, 4);
        ef_port_read(area1_cur_using_addr + ENV_PARAM_PART_INDEX_SAVED_COUNT * 4,
                     &area1_saved_count, 4);
        /* the bigger saved count area is valid */
        if ((area0_saved_count > area1_saved_count) || ((area0_saved_count == 0) && (area1_saved_count == 0xFFFFFFFF))) {
            area1_is_valid = false;
        } else {
            area0_is_valid = false;
        }
    }
    if (area0_is_valid) {
        /* current using data section address is area0 current using data section address */
        set_cur_using_data_addr(area0_cur_using_addr);
        /* next save ENV area address is area1 current using address value */
        next_save_area_addr = area1_cur_using_addr;
        /* read all ENV from area0 */
        ef_port_read(area0_cur_using_addr, env_cache, area0_end_addr - area0_cur_using_addr);
    } else if (area1_is_valid) {
        /* already read data section and set_cur_using_data_addr above current code,
         * so just set next save ENV area address is area0 current using address value */
        next_save_area_addr = area0_cur_using_addr;
    } else {
        /* current using data section address is area1 current using address default value */
        set_cur_using_data_addr(area1_default_cur_using_addr);
        /* next save ENV area address default is area0 current using address default value */
        next_save_area_addr = area0_default_cur_using_addr;
        /* save current using data section address to flash*/
        if (((result = save_cur_using_data_addr(area0_default_cur_using_addr)) == EF_NO_ERR)
                && ((result = save_cur_using_data_addr(area1_default_cur_using_addr)) == EF_NO_ERR)) {
            /* set the ENV to default */
            result = ef_env_set_default();
        }
    }
    return result;
}
#endif

/**
 * Save ENV to flash.
 */
EfErrCode ef_save_env(void) {
    EfErrCode result = EF_NO_ERR;
    uint32_t cur_using_addr_bak, move_offset_addr;
    size_t env_used_size = get_env_user_used_size();
    uint32_t data_sec_end_addr;

    /* ENV ram cache has not changed don't need to save */
    if (!env_cache_changed) {
        return result;
    }

#ifndef EF_ENV_USING_PFS_MODE
    data_sec_end_addr = get_env_start_addr() + ENV_AREA_SIZE - 4;
    cur_using_addr_bak = get_cur_using_data_addr();
#else
    cur_using_addr_bak = next_save_area_addr;
    /* replace next_save_area_addr with cur_using_data_addr */
    next_save_area_addr = get_cur_using_data_addr();
    set_cur_using_data_addr(cur_using_addr_bak);
    /* change the ENV detail end address to next save area address */
    set_env_detail_end_addr(get_cur_using_data_addr() + env_used_size);
    /* area0 or area1 */
    if (get_cur_using_data_addr() < get_env_start_addr() + ENV_AREA_SIZE / 2) {
        data_sec_end_addr = get_env_start_addr() + ENV_AREA_SIZE / 2 - 4;
    } else {
        data_sec_end_addr = get_env_start_addr() + ENV_AREA_SIZE - 4;
    }
    /* ENV area saved count +1 */
    env_cache[ENV_PARAM_PART_INDEX_SAVED_COUNT]++;
#endif

    /* wear leveling process, automatic move ENV to next available position */
    while (get_cur_using_data_addr() + env_used_size < data_sec_end_addr) {
        /* calculate and cache CRC32 code */
        env_cache[ENV_PARAM_PART_INDEX_DATA_CRC] = calc_env_crc();
        /* erase ENV */
        result = ef_port_erase(get_cur_using_data_addr(), env_used_size);
        switch (result) {
        case EF_NO_ERR: {
            EF_DEBUG("Erased ENV OK.\n");
            break;
        }
        case EF_ERASE_ERR: {
            EF_INFO("Warning: Erased ENV fault! Start address is 0x%08X, size is %ld.\n",
                    get_cur_using_data_addr(), env_used_size);
            EF_INFO("Moving ENV to next available position.\n");
            /* Calculate move offset address.
             * Current strategy is optimistic. It will offset the flash erasure minimum size.
             */
            move_offset_addr = EF_ERASE_MIN_SIZE;
            /* calculate and set next available data section address */
            set_cur_using_data_addr(get_cur_using_data_addr() + move_offset_addr);
            /* calculate and set next available ENV detail part end address */
            set_env_detail_end_addr(get_env_detail_end_addr() + move_offset_addr);
            continue;
        }
        }
        /* write ENV to flash */
        result = ef_port_write(get_cur_using_data_addr(), env_cache, env_used_size);
        switch (result) {
        case EF_NO_ERR: {
            EF_DEBUG("Saved ENV OK.\n");
            break;
        }
        case EF_WRITE_ERR: {
            EF_INFO("Warning: Saved ENV fault! Start address is 0x%08X, size is %ld.\n",
                    get_cur_using_data_addr(), env_used_size);
            EF_INFO("Moving ENV to next available position.\n");
            /* Calculate move offset address.
             * Current strategy is optimistic. It will offset the flash erasure minimum size.
             */
            move_offset_addr = EF_ERASE_MIN_SIZE;
            /* calculate and set next available data section address */
            set_cur_using_data_addr(get_cur_using_data_addr() + move_offset_addr);
            /* calculate and set next available ENV detail part end address */
            set_env_detail_end_addr(get_env_detail_end_addr() + move_offset_addr);
            continue;
        }
        }
        /* save ENV success */
        if (result == EF_NO_ERR) {
            break;
        }
    }

    if (get_cur_using_data_addr() + env_used_size < data_sec_end_addr) {
        /* current using data section address has changed, save it */
        if (get_cur_using_data_addr() != cur_using_addr_bak) {
            result = save_cur_using_data_addr(get_cur_using_data_addr());
        }
    } else {
        result = EF_ENV_FULL;
        EF_INFO("Error: The flash has no available space to save ENV.\n");
    }

    env_cache_changed = false;

    return result;
}

/**
 * Calculate the cached ENV CRC32 value.
 *
 * @return CRC32 value
 */
static uint32_t calc_env_crc(void) {
    uint32_t crc32 = 0;

    /* Calculate the ENV end address and all ENV data CRC32.
     * The 4 is ENV end address bytes size. */
    crc32 = ef_calc_crc32(crc32, &env_cache[ENV_PARAM_PART_INDEX_END_ADDR], 4);
    crc32 = ef_calc_crc32(crc32, &env_cache[ENV_PARAM_PART_WORD_SIZE], get_env_detail_size());
    EF_DEBUG("Calculate ENV CRC32 number is 0x%08X.\n", crc32);

    return crc32;
}

/**
 * Check the ENV CRC32
 *
 * @return true is ok
 */
static bool env_crc_is_ok(void) {
    if (calc_env_crc() == env_cache[ENV_PARAM_PART_INDEX_DATA_CRC]) {
        EF_DEBUG("Verify ENV CRC32 result is OK.\n");
        return true;
    } else {
        return false;
    }
}

/**
 * Save current using data section address to flash.
 *
 * @param cur_data_addr current using data section address
 *
 * @return result
 */
#ifndef EF_ENV_USING_PFS_MODE
static EfErrCode save_cur_using_data_addr(uint32_t cur_data_addr) {
    EfErrCode result = EF_NO_ERR;

    /* erase ENV system section */
    result = ef_port_erase(get_env_start_addr(), 4);
    if (result == EF_NO_ERR) {
        /* write current using data section address to flash */
        result = ef_port_write(get_env_start_addr(), &cur_data_addr, 4);
        if (result == EF_WRITE_ERR) {
            EF_INFO("Error: Write system section fault! Start address is 0x%08X, size is %ld.\n",
                    get_env_start_addr(), 4);
            EF_INFO("Note: The ENV can not be used.\n");
        }
    } else {
        EF_INFO("Error: Erased system section fault! Start address is 0x%08X, size is %ld.\n",
                get_env_start_addr(), 4);
        EF_INFO("Note: The ENV can not be used\n");
    }
    return result;
}
#else
static EfErrCode save_cur_using_data_addr(uint32_t cur_data_addr) {
    EfErrCode result = EF_NO_ERR;
    uint32_t cur_system_sec_addr;

    if (cur_data_addr < get_env_start_addr() + ENV_AREA_SIZE / 2) {
        /* current using system section is in ENV area0 */
        cur_system_sec_addr = get_env_start_addr();
    } else {
        /* current using system section is in ENV area1 */
        cur_system_sec_addr = get_env_start_addr() + ENV_AREA_SIZE / 2;
    }
    /* erase ENV system section */
    result = ef_port_erase(cur_system_sec_addr, 4);
    if (result == EF_NO_ERR) {
        /* write area0 and area1 current using data section address to flash */
        result = ef_port_write(cur_system_sec_addr, &cur_data_addr, 4);
        if (result == EF_WRITE_ERR) {
            EF_INFO("Error: Write system section fault! Start address is 0x%08X, size is %ld.\n",
                    cur_system_sec_addr, 4);
            EF_INFO("Note: The ENV can not be used.\n");
        }
    } else {
        EF_INFO("Error: Erased system section fault! Start address is 0x%08X, size is %ld.\n",
                cur_system_sec_addr, 4);
        EF_INFO("Note: The ENV can not be used\n");
    }
    return result;
}
#endif

/**
 * Set and save an ENV. If set ENV is success then will save it.
 *
 * @param key ENV name
 * @param value ENV value
 *
 * @return result
 */
EfErrCode ef_set_and_save_env(const char *key, const char *value) {
    EfErrCode result = EF_NO_ERR;

    result = ef_set_env(key, value);

    if (result == EF_NO_ERR) {
        result = ef_save_env();
    }

    return result;
}

/**
 * Del and save an ENV. If del ENV is success then will save it.
 *
 * @param key ENV name
 *
 * @return result
 */
EfErrCode ef_del_and_save_env(const char *key) {
    EfErrCode result = EF_NO_ERR;

    result = ef_del_env(key);

    if (result == EF_NO_ERR) {
        result = ef_save_env();
    }

    return result;
}

#ifdef EF_ENV_AUTO_UPDATE
/**
 * Auto update ENV to latest default when current EF_ENV_VER is changed.
 *
 * @return result
 */
static EfErrCode env_auto_update(void)
{
    size_t i;

    /* lock the ENV cache */
    ef_port_env_lock();

    /* read ENV version number from flash*/
    ef_port_read(get_cur_using_data_addr() + ENV_PARAM_INDEX_VER_NUM * 4,
                 &env_cache[ENV_PARAM_INDEX_VER_NUM] , 4);

    /* check version number */
    if (env_cache[ENV_PARAM_INDEX_VER_NUM] != EF_ENV_VER_NUM) {
        env_cache_changed = true;
        /* update version number */
        env_cache[ENV_PARAM_INDEX_VER_NUM] = EF_ENV_VER_NUM;
        /* add a new ENV when it's not found */
        for (i = 0; i < default_env_set_size; i++) {
            if (find_env(default_env_set[i].key) == NULL) {
                create_env(default_env_set[i].key, default_env_set[i].value);
            }
        }
    }

    /* unlock the ENV cache */
    ef_port_env_unlock();

    return ef_save_env();
}
#endif /* EF_ENV_AUTO_UPDATE */

#endif /* EF_ENV_USING_WL_MODE */

#endif /* defined(EF_USING_ENV) && defined(EF_ENV_USING_LEGACY_MODE) */
