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
 * Function: nuttx demo.
 * Created on: 2023-02-15
 */

#define LOG_TAG    "elogdemo"

#include <elog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <elog_flash.h>
#include <easyflash.h>


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * Env demo.
 */
static void test_env(void) {
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
//    assert_param(c_old_boot_times);
    i_boot_times = atol(c_old_boot_times);
    /* boot count +1 */
    i_boot_times ++;
    printf("The system now boot %d times\n\r", i_boot_times);
    /* interger to string */
    sprintf(c_new_boot_times,"%ld", i_boot_times);
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);
    ef_save_env();
}

/**
 * EasyLogger demo
 */
static void test_elog(void) {
    uint8_t buf[256]= {0};
    int i = 0;

    for (i = 0; i < sizeof(buf); i++)
    {
        buf[i] = i;
    }
    while(true) {
        /* test log output for all level */
        log_a("Hello EasyLogger!");
        log_e("Hello EasyLogger!");
        log_w("Hello EasyLogger!");
        log_i("Hello EasyLogger!");
        log_d("Hello EasyLogger!");
        log_v("Hello EasyLogger!");
//        elog_raw("Hello EasyLogger!");
        elog_hexdump("test", 16, buf, sizeof(buf));
        break;
    }
}

static void easylogger_demo(void) {
    /* close printf buffer */
    setbuf(stdout, NULL);

    /* initialize EasyFlash and EasyLogger */
    if ((easyflash_init() == EF_NO_ERR)&&(elog_init() == ELOG_NO_ERR)) {
        /* set EasyLogger log format */
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
        elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
        elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    #ifdef ELOG_COLOR_ENABLE
        elog_set_text_color_enabled(true);
    #endif 
        /* initialize EasyLogger Flash plugin */
        elog_flash_init();
        /* start EasyLogger */
        elog_start();

        /* dynamic set enable or disable for output logs (true or false) */
    //    elog_set_output_enabled(false);
        /* dynamic set output logs's level (from ELOG_LVL_ASSERT to ELOG_LVL_VERBOSE) */
    //    elog_set_filter_lvl(ELOG_LVL_WARN);
        /* dynamic set output logs's filter for tag */
    //    elog_set_filter_tag("main");
        /* dynamic set output logs's filter for keyword */
    //    elog_set_filter_kw("Hello");
        /* dynamic set output logs's tag filter */
    //    elog_set_filter_tag_lvl("main", ELOG_LVL_WARN);

        /* test logger output */
        test_env();
        test_elog();
    }
    else{
        printf("easyflash_init or elog_init init fail\n");
    }

    return EXIT_SUCCESS;
}


/****************************************************************************
 * elog_demo
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
  printf("Hello, EasyLogger!!\n");
  easylogger_demo();
  return 0;
}

