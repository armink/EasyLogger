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
 * Function: It is an head file for flash log plugin. You can see all be called functions.
 * Created on: 2015-06-05
 */

#ifndef __ELOG_FLASH_H__
#define __ELOG_FLASH_H__

#include <elog.h>
#include <elog_flash_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(ELOG_FLASH_BUF_SIZE)
    #error "Please configure RAM buffer size (in elog_flash_cfg.h)"
#endif

/* EasyLogger flash log plugin's software version number */
#define ELOG_FLASH_SW_VERSION                "V2.0.1"

/* elog_flash.c */
ElogErrCode elog_flash_init(void);
void elog_flash_output(size_t pos, size_t size);
void elog_flash_output_all(void);
void elog_flash_output_recent(size_t size);
void elog_flash_set_filter(uint8_t level,const char *tag,const char *keyword);
void elog_flash_write(const char *log, size_t size);
void elog_flash_clean(void);
void elog_flash_lock_enabled(bool enabled);

#ifdef ELOG_FLASH_USING_BUF_MODE
void elog_flash_flush(void);
#endif

/* elog_flash_port.c */
ElogErrCode elog_flash_port_init(void);
void elog_flash_port_output(const char *log, size_t size);
void elog_flash_port_lock(void);
void elog_flash_port_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELOG_FLASH_H__ */
