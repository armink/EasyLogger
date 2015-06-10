#include "utils.h"

/* current system status */
static SystemStatus cur_system_status = SYSTEM_STATUS_INIT;


/**
 * System go to fault status.
 */
void system_go_to_fault_status(void){
    cur_system_status = SYSTEM_STATUS_FAULT;
}

/**
 * Set current system status.
 *
 * @param status system status
 */
void set_system_status(SystemStatus status){
    cur_system_status = status;
}

/**
 * Get current system status.
 *
 * @return current system status
 */
SystemStatus get_system_status(void){
    return cur_system_status;
}
