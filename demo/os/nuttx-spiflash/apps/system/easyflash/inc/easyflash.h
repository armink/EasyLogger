/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2014-2019, Armink, <armink.ztl@gmail.com>
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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ef_cfg.h>
#include <ef_def.h>

#ifdef __cplusplus
extern "C" {
#endif

/* easyflash.c */
EfErrCode easyflash_init(void);

#ifdef EF_USING_ENV
/* only supported on ef_env.c */
size_t ef_get_env_blob(const char *key, void *value_buf, size_t buf_len, size_t *saved_value_len);
bool ef_get_env_obj(const char *key, env_node_obj_t env);
size_t ef_read_env_value(env_node_obj_t env, uint8_t *value_buf, size_t buf_len);
EfErrCode ef_set_env_blob(const char *key, const void *value_buf, size_t buf_len);

/* ef_env.c, ef_env_legacy_wl.c and ef_env_legacy.c */
EfErrCode ef_load_env(void);
void ef_print_env(void);
char *ef_get_env(const char *key);
EfErrCode ef_set_env(const char *key, const char *value);
EfErrCode ef_del_env(const char *key);
EfErrCode ef_save_env(void);
EfErrCode ef_env_set_default(void);
size_t ef_get_env_write_bytes(void);
EfErrCode ef_set_and_save_env(const char *key, const char *value);
EfErrCode ef_del_and_save_env(const char *key);
#endif

#ifdef EF_USING_IAP
/* ef_iap.c */
EfErrCode ef_erase_bak_app(size_t app_size);
EfErrCode ef_erase_user_app(uint32_t user_app_addr, size_t user_app_size);
EfErrCode ef_erase_spec_user_app(uint32_t user_app_addr, size_t app_size,
                                 EfErrCode (*app_erase)(uint32_t addr, size_t size));
EfErrCode ef_erase_bl(uint32_t bl_addr, size_t bl_size);
EfErrCode ef_write_data_to_bak(uint8_t *data, size_t size, size_t *cur_size,
                               size_t total_size);
EfErrCode ef_copy_app_from_bak(uint32_t user_app_addr, size_t app_size);
EfErrCode ef_copy_spec_app_from_bak(uint32_t user_app_addr, size_t app_size,
                                    EfErrCode (*app_write)(uint32_t addr, const uint32_t *buf, size_t size));
EfErrCode ef_copy_bl_from_bak(uint32_t bl_addr, size_t bl_size);
uint32_t ef_get_bak_app_start_addr(void);
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
