#ifndef UTILS_H_
#define UTILS_H_

#include <stm32f10x_conf.h>
#include <rtthread.h>

/* system status */
typedef enum{
    SYSTEM_STATUS_INIT,
    SYSTEM_STATUS_RUN,
    SYSTEM_STATUS_FAULT,
    SYSTEM_STATUS_SLEEP,
}SystemStatus;

void Delay(vu32 nCount);
void system_go_to_fault_status(void);
void set_system_status(SystemStatus status);
SystemStatus get_system_status(void);

#endif /* UTILS_H_ */
