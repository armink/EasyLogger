#include "app_task.h"
#include <stdlib.h>
#include <stdio.h>
#include "bsp.h"
#include "elog.h"
#include "finsh.h"
#include "shell.h"
#include "cpuusage.h"
#include "delay_conf.h"
#include "utils.h"

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#elif __ICCARM__
#pragma section="HEAP"
#else
extern int __bss_end;
#endif

#define log_a(...) elog_a("main.test.a", __VA_ARGS__)
#define log_e(...) elog_e("main.test.e", __VA_ARGS__)
#define log_w(...) elog_w("main.test.w", __VA_ARGS__)
#define log_i(...) elog_i("main.test.i", __VA_ARGS__)
#define log_d(...) elog_d("main.test.d", __VA_ARGS__)
#define log_v(...) elog_v("main.test.v", __VA_ARGS__)

#define thread_sys_monitor_prio                30

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t thread_sys_monitor_stack[512];

struct rt_thread thread_sys_monitor;

static void test_elog(void);

/**
 * System monitor thread.
 *
 * @param parameter parameter
 */
void thread_entry_sys_monitor(void* parameter)
{
    while (1)
    {
        if(get_system_status() == SYSTEM_STATUS_RUN){
            /* elog test */
            test_elog();
            LED_RUN_ON;
            rt_thread_delay(DELAY_SYS_RUN_LED_ON);
            LED_RUN_OFF;
            rt_thread_delay(DELAY_SYS_RUN_LED_OFF);
        } else if (get_system_status() == SYSTEM_STATUS_INIT){
            LED_RUN_ON;
            rt_thread_delay(DELAY_SYS_INIT_LED_ON);
            LED_RUN_OFF;
            rt_thread_delay(DELAY_SYS_INIT_LED_OFF);
        } else if (get_system_status() == SYSTEM_STATUS_FAULT){
            LED_RUN_ON;
            rt_thread_delay(DELAY_SYS_FAULT_LED_ON);
            LED_RUN_OFF;
            rt_thread_delay(DELAY_SYS_FAULT_LED_OFF);
        }
        IWDG_Feed();
    }
}

/**
 * Elog demo
 */
static void test_elog(void) {
    log_a("Hello EasyLogger!");
    log_e("Hello EasyLogger!");
    log_w("Hello EasyLogger!");
    log_i("Hello EasyLogger!");
    log_d("Hello EasyLogger!");
    log_v("Hello EasyLogger!");
    //elog_raw("Hello EasyLogger!");
}

/**
 * System initialization thread.
 *
 * @param parameter parameter
 */
void sys_init_thread(void* parameter){
	set_system_status(SYSTEM_STATUS_INIT);

    /* EasyLogger initialization */
    if (elog_init() == ELOG_NO_ERR) {
        /* set enabled format */
        elog_set_fmt(ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME /*| ELOG_FMT_P_INFO*/ | ELOG_FMT_T_INFO | ELOG_FMT_DIR
                /*| ELOG_FMT_FUNC*/ | ELOG_FMT_LINE);
        /* initialize OK and switch to running status */
        set_system_status(SYSTEM_STATUS_RUN);
    } else {
        /* initialize fail and switch to fault status */
        set_system_status(SYSTEM_STATUS_FAULT);
    }

    rt_thread_delete(rt_thread_self());
}

int rt_application_init(void)
{
    rt_thread_t init_thread = NULL;
    rt_thread_init(&thread_sys_monitor,
                   "sys_monitor",
                   thread_entry_sys_monitor,
                   RT_NULL,
                   thread_sys_monitor_stack,
                   sizeof(thread_sys_monitor_stack),
                   thread_sys_monitor_prio, 5);
    rt_thread_startup(&thread_sys_monitor);

    init_thread = rt_thread_create("sys init", sys_init_thread,
            NULL, 512, 10, 10);
    if (init_thread != NULL) {
        rt_thread_startup(init_thread);
    }
    return 0;
}

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
    /* init board */
    rt_hw_board_init();

    /* show version */
    rt_show_version();

    /* init tick */
    rt_system_tick_init();

    /* init kernel object */
    rt_system_object_init();

    /* init timer system */
    rt_system_timer_init();

#ifdef RT_USING_HEAP
#ifdef __CC_ARM
    rt_system_heap_init((void*)&Image$$RW_IRAM1$$ZI$$Limit, (void*)STM32_SRAM_END);
#elif __ICCARM__
    rt_system_heap_init(__segment_end("HEAP"), (void*)STM32_SRAM_END);
#else
    /* init memory system */
    rt_system_heap_init((void*)&__bss_end, (void*)STM32_SRAM_END);
#endif
#endif

    /* init scheduler system */
    rt_system_scheduler_init();

    /* initialize timer */
    rt_system_timer_init();

    /* init timer thread */
    rt_system_timer_thread_init();

    /* Add CPU usage to system */
    cpu_usage_init();

#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

#ifdef  RT_USING_FINSH
    /* init finsh */
    finsh_system_init();
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

    /* init application */
    rt_application_init();

    /* init idle thread */
    rt_thread_idle_init();

    /* start scheduler */
    rt_system_scheduler_start();

    /* never reach here */
    return;
}

