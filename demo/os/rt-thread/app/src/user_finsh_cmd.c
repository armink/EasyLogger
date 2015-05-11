/*
 * user_finsh_cmd.c
 *
 *  Created on: 2013Äê12ÔÂ7ÈÕ
 *      Author: Armink
 */
#include <rthw.h>
#include <rtthread.h>
#include <stm32f10x_conf.h>
#include <finsh.h>
#include "cpuusage.h"
#include "elog.h"

static void reboot(uint8_t argc, char **argv) {
    NVIC_SystemReset();
}
MSH_CMD_EXPORT(reboot, Reboot System);

static void get_cpuusage(void) {
    uint8_t cpu_usage_major, cpu_usage_minor;
    
    cpu_usage_get(&cpu_usage_major, &cpu_usage_minor);
    rt_kprintf("The CPU usage is %d.%d% now.\n", cpu_usage_major, cpu_usage_minor);
}
MSH_CMD_EXPORT(get_cpuusage, Get control board cpu usage);

static void elog(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (!strcmp(argv[1], "on") || !strcmp(argv[1], "ON")) {
            elog_set_output_enabled(TRUE);
        } else if (!strcmp(argv[1], "off") || !strcmp(argv[1], "OFF")) {
            elog_set_output_enabled(FALSE);
        } else {
            rt_kprintf("Please input elog on or elog off.\n");
        }
    } else {
        rt_kprintf("Please input elog on or elog off.\n");
    }
}
MSH_CMD_EXPORT(elog, EasyLogger output enabled [on/off]);

static void elog_lvl(uint8_t argc, char **argv) {
    if (argc > 1) {
        if ((atoi(argv[1]) <= ELOG_LVL_VERBOSE) && (atoi(argv[1]) >= 0)) {
            elog_set_filter_lvl(atoi(argv[1]));
        } else {
            rt_kprintf("Please input correct level(0-5).\n");
        }
    } else {
        rt_kprintf("Please input level.\n");
    }
}
MSH_CMD_EXPORT(elog_lvl, Set EasyLogger filter level);

static void elog_tag(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (rt_strlen(argv[1]) <= ELOG_FILTER_TAG_MAX_LEN) {
            elog_set_filter_tag(argv[1]);
        } else {
            rt_kprintf("The tag length is too long. Max is %d.\n", ELOG_FILTER_TAG_MAX_LEN);
        }
    } else {
        elog_set_filter_tag("");
    }
}
MSH_CMD_EXPORT(elog_tag, Set EasyLogger filter tag);

static void elog_kw(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (rt_strlen(argv[1]) <= ELOG_FILTER_KW_MAX_LEN) {
            elog_set_filter_kw(argv[1]);
        } else {
            rt_kprintf("The keyword length is too long. Max is %d.\n", ELOG_FILTER_KW_MAX_LEN);
        }
    } else {
        elog_set_filter_kw("");
    }
}
MSH_CMD_EXPORT(elog_kw, Set EasyLogger filter keyword);
