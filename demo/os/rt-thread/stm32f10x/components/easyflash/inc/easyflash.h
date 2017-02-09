/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2014-2016, Armink, <armink.ztl@gmail.com>
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
 * Created on: 2014-09-10
 */


#ifndef EASYFLASH_H_
#define EASYFLASH_H_

#include <ef_cfg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(EF_USING_ENV) && (!defined(ENV_USER_SETTING_SIZE) || !defined(ENV_AREA_SIZE))
    #error "Please configure user setting ENV size or ENV area size (in ef_cfg.h)"
#endif

#if defined(EF_USING_LOG) && !defined(LOG_AREA_SIZE)
    #error "Please configure log area size (in ef_cfg.h)"
#endif

#if !defined(EF_START_ADDR)
    #error "Please configure backup area start address (in ef_cfg.h)"
#endif

#if !defined(EF_ERASE_MIN_SIZE)
    #error "Please configure minimum size of flash erasure (in ef_cfg.h)"
#endif

/* EasyFlash debug print function. Must be implement by user. */
#define EF_DEBUG(...) ef_log_debug(__FILE__, __LINE__, __VA_ARGS__)
/* EasyFlash routine print function. Must be implement by user. */
#define EF_INFO(...)  ef_log_info(__VA_ARGS__)
/* EasyFlash assert for developer. */
#define EF_ASSERT(EXPR)                                                       \
if (!(EXPR))                                                                  \
{                                                                             \
    EF_DEBUG("(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__);         \
    while (1);                                                                \
}
/* EasyFlash software version number */
#define EF_SW_VERSION                "2.12.08"

typedef struct _ef_env{
    char *key;
    char *value;
}ef_env, *ef_env_t;

/* EasyFlash error code */
typedef enum {
    EF_NO_ERR,
    EF_ERASE_ERR,
    EF_WRITE_ERR,
    EF_ENV_NAME_ERR,
    EF_ENV_NAME_EXIST,
    EF_ENV_FULL,
    EF_ENV_INIT_FAILED,
} EfErrCode;

/* the flash sector current status */
typedef enum {
    EF_SECTOR_EMPTY,
    EF_SECTOR_USING,
    EF_SECTOR_FULL,
}EfSecrorStatus;

/* easyflash.c */
EfErrCode easyflash_init(void);

#ifdef EF_USING_ENV
/* ef_env.c ef_env_wl.c */
EfErrCode ef_load_env(void);
void ef_print_env(void);
char *ef_get_env(const char *key);
EfErrCode ef_set_env(const char *key, const char *value);
EfErrCode ef_save_env(void);
EfErrCode ef_env_set_default(void);
size_t ef_get_env_write_bytes(void);
EfErrCode ef_set_and_save_env(const char *key, const char *value);
#endif

#ifdef EF_USING_IAP
/* ef_iap.c */
EfErrCode ef_erase_bak_app(size_t app_size);
EfErrCode ef_erase_user_app(uint32_t user_app_addr, size_t user_app_size);
EfErrCode ef_erase_bl(uint32_t bl_addr, size_t bl_size);
EfErrCode ef_write_data_to_bak(uint8_t *data, size_t size, size_t *cur_size,
        size_t total_size);
EfErrCode ef_copy_app_from_bak(uint32_t user_app_addr, size_t app_size);
EfErrCode ef_copy_bl_from_bak(uint32_t bl_addr, size_t bl_size);
#endif

#ifdef EF_USING_LOG
/* ef_log.c */
EfErrCode ef_log_read(size_t index, uint32_t *log, size_t size);
EfErrCode ef_log_write(const uint32_t *log, size_t size);
EfErrCode ef_log_clean(void);
size_t ef_log_get_used_size(void);
#endif

/* ef_utils.c */
uint32_t ef_calc_crc32(uint32_t crc, const void *buf, size_t size);
EfSecrorStatus ef_get_sector_status(uint32_t addr, size_t sec_size);
uint32_t ef_find_sec_using_end_addr(uint32_t addr, size_t sec_size);

/* ef_port.c */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size);
EfErrCode ef_port_erase(uint32_t addr, size_t size);
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size);
void ef_port_env_lock(void);
void ef_port_env_unlock(void);
void ef_log_debug(const char *file, const long line, const char *format, ...);
void ef_log_info(const char *format, ...);
void ef_print(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* EASYFLASH_H_ */
