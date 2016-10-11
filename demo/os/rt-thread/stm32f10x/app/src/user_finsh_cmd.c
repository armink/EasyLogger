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
#include "elog_flash.h"
#include "easyflash.h"

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

void setenv(uint8_t argc, char **argv) {
    uint8_t i;
    char c_value = NULL;
    char *value = &c_value;
    if (argc > 3) {
        /* environment variable value string together */
        for (i = 0; i < argc - 2; i++) {
            argv[2 + i][rt_strlen(argv[2 + i])] = ' ';
        }
    }
    if (argc == 1) {
        ef_set_env(value, value);
    } else if (argc == 2) {
        ef_set_env(argv[1], value);
    } else {
        ef_set_env(argv[1], argv[2]);
    }
}
MSH_CMD_EXPORT(setenv, Set an envrionment variable.);

void printenv(uint8_t argc, char **argv) {
    ef_print_env();
}
MSH_CMD_EXPORT(printenv, Print all envrionment variables.);

void saveenv(uint8_t argc, char **argv) {
    ef_save_env();
}
MSH_CMD_EXPORT(saveenv, Save all envrionment variables to flash.);

void getvalue(uint8_t argc, char **argv) {
    char *value = NULL;
    value = ef_get_env(argv[1]);
    if (value) {
        rt_kprintf("The %s value is %s.\n", argv[1], value);
    } else {
        rt_kprintf("Can't find %s.\n", argv[1]);
    }
}
MSH_CMD_EXPORT(getvalue, Get an envrionment variable by name.);

static void elog(uint8_t argc, char **argv) {
    if (argc > 1) {
        if (!strcmp(argv[1], "on") || !strcmp(argv[1], "ON")) {
            elog_set_output_enabled(true);
        } else if (!strcmp(argv[1], "off") || !strcmp(argv[1], "OFF")) {
            elog_set_output_enabled(false);
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

static void elog_flash(uint8_t argc, char **argv) {
    if (argc >= 2) {
        if (!strcmp(argv[1], "read")) {
            if (argc >= 3) {
                elog_flash_output_recent(atol(argv[2]));
            }else {
                elog_flash_output_all();
            }
        } else if (!strcmp(argv[1], "clean")) {
            elog_flash_clean();
        } else if (!strcmp(argv[1], "flush")) {

#ifdef ELOG_FLASH_USING_BUF_MODE
            elog_flash_flush();
#else
            rt_kprintf("EasyLogger flash log buffer mode is not open.\n");
#endif

        } else {
            rt_kprintf("Please input elog_flash {<read>, <clean>, <flush>}.\n");
        }
    } else {
        rt_kprintf("Please input elog_flash {<read>, <clean>, <flush>}.\n");
    }
}
MSH_CMD_EXPORT(elog_flash, EasyLogger <read> <clean> <flush> flash log);
