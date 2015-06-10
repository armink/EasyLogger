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
 * Function: Is is an head file for flash log plugin. You can see all be called functions.
 * Created on: 2015-06-05
 */

#ifndef __ELOG_FLASH_H__
#define __ELOG_FLASH_H__

#include "elog.h"

#ifdef __cplusplus
extern "C" {
#endif

/* EasyLogger flash save plugin's using buffer mode */
#define ELOG_FLASH_USING_BUF_MODE
/* EasyLogger flash save plugin's RAM buffer size */
#define ELOG_FLASH_BUF_SIZE                  1024
/* EasyLogger flash save plugin's software version number */
#define ELOG_FLASH_SW_VERSION                "0.06.09"

/* elog_flash.c */
ElogErrCode elog_flash_init(void);
void elog_flash_outout_all(void);
void elog_flash_set_filter(uint8_t level,const char *tag,const char *keyword);
void elog_flash_write(const char *log, size_t size);
void elog_flash_clean(void);

#ifdef ELOG_FLASH_USING_BUF_MODE
void elog_flash_flush(void);
#endif

/* elog_port.c */
void elog_flash_port_output(const char *output, size_t size);
void elog_flash_port_lock(void);
void elog_flash_port_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELOG_FLASH_H__ */
