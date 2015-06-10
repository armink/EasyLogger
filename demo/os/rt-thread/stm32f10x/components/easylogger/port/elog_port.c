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
 * Function: Portable interface for RT-Thread.
 * Created on: 2015-04-28
 */

#include "elog.h"
#include <rthw.h>
#include <rtthread.h>

static struct rt_semaphore output_lock;

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;

    rt_sem_init(&output_lock, "elog lock", 1, RT_IPC_FLAG_PRIO);

    return result;
}

/**
 * output log port interface
 */
void elog_port_output(const char *output, size_t size) {
    /* output to terminal */
    rt_kprintf("%.*s", size, output);
    //TODO output to flash
}

/**
 * output lock
 */
void elog_port_output_lock(void) {
    rt_sem_take(&output_lock, RT_WAITING_FOREVER);
}

/**
 * output unlock
 */
void elog_port_output_unlock(void) {
    rt_sem_release(&output_lock);
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    static char cur_system_time[16] = { 0 };
    rt_snprintf(cur_system_time, 16, "tick:%010d", rt_tick_get());
    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    return "";
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    return rt_thread_self()->name;
}
