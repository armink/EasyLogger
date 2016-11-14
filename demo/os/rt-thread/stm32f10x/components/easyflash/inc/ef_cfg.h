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
 * Function: It is the configure head file for this library.
 * Created on: 2015-07-14
 */


#ifndef EF_CFG_H_
#define EF_CFG_H_

#include <stm32f10x_conf.h>

/* using ENV function */
#define EF_USING_ENV
/* using wear leveling mode for ENV */
/* #define EF_ENV_USING_WL_MODE */
/* using power fail safeguard mode for ENV */
/* #define EF_ENV_USING_PFS_MODE */

/* using IAP function */
#define EF_USING_IAP

/* using save log function */
#define EF_USING_LOG

/* page size for stm32 flash */
#if defined(STM32F10X_LD) || defined(STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
#define PAGE_SIZE     1024
#else
#define PAGE_SIZE     2048
#endif

/* the minimum size of flash erasure */
#define EF_ERASE_MIN_SIZE         PAGE_SIZE              /* it is one page for STM3210x */

/**
 *
 * This all Backup Area Flash storage index. All used flash area configure is under here.
 * |----------------------------|   Storage Size
 * | Environment variables area |   ENV area size @see ENV_AREA_SIZE
 * |      1.system section      |   ENV_SYSTEM_SIZE
 * |      2:data section        |   ENV_AREA_SIZE - ENV_SYSTEM_SIZE
 * |----------------------------|
 * |      Saved log area        |   Log area size @see LOG_AREA_SIZE
 * |----------------------------|
 * |(IAP)Downloaded application |   IAP already downloaded application, unfixed size
 * |----------------------------|
 *
 * @note all area size must be aligned with EF_ERASE_MIN_SIZE
 * @note EasyFlash will use ram to buffered the ENV. At some time flash's EF_ERASE_MIN_SIZE is so big,
 *       and you want use ENV size is less than it. So you must defined ENV_USER_SETTING_SIZE for ENV.
 * @note ENV area size has some limitations in different modes.
 *       1.Normal mode: no more limitations
 *       2.Wear leveling mode: system section will used an flash section and the data section will used at least 2 flash sections
 *       3.Power fail safeguard mode: ENV area will has an backup. It is twice as normal mode.
 *       4.wear leveling and power fail safeguard mode: The required capacity will be 2 times the total capacity in wear leveling mode.
 *       For example:
 *       The EF_ERASE_MIN_SIZE is 128K and the ENV_USER_SETTING_SIZE: 2K. The ENV_AREA_SIZE in different mode you can define
 *       1.Normal mode: 1*EF_ERASE_MIN_SIZE
 *       2.Wear leveling mode: 3*EF_ERASE_MIN_SIZE (It has 2 data section to store ENV. So ENV can erase at least 200,000 times)
 *       3.Power fail safeguard mode: 2*EF_ERASE_MIN_SIZE
 *       4.Wear leveling and power fail safeguard mode: 6*EF_ERASE_MIN_SIZE
 * @note the log area size must be more than twice of EF_ERASE_MIN_SIZE
 */
/* backup area start address */
#define EF_START_ADDR             (FLASH_BASE + 80 * 1024) /* from the chip position: 80KB */
/* the user setting size of ENV, must be word alignment */
#define ENV_USER_SETTING_SIZE     (2 * 1024)
#ifndef EF_ENV_USING_PFS_MODE
    #ifndef EF_ENV_USING_WL_MODE
        /* ENV area total bytes size in normal mode. */
        #define ENV_AREA_SIZE          (1 * EF_ERASE_MIN_SIZE)      /* 2K */
    #else
        /* ENV area total bytes size in wear leveling mode. */
        #define ENV_AREA_SIZE          (4 * EF_ERASE_MIN_SIZE)      /* 8K */
    #endif
#else
    #ifndef EF_ENV_USING_WL_MODE
        /* ENV area total bytes size in power fail safeguard mode. */
        #define ENV_AREA_SIZE          (2 * EF_ERASE_MIN_SIZE)      /* 4K */
    #else
        /* ENV area total bytes size in wear leveling and power fail safeguard mode. */
        #define ENV_AREA_SIZE          (8 * EF_ERASE_MIN_SIZE)      /* 16K */
    #endif
#endif
/* saved log area size */
#define LOG_AREA_SIZE             (10 * EF_ERASE_MIN_SIZE)      /* 20K */

/* print debug information of flash */
#define PRINT_DEBUG

#endif /* EF_CFG_H_ */
