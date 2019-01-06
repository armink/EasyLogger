#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#include "elog_file.h"

/* initialize OK flag */
static bool init_ok = false;

ElogErrCode elog_file_init(void) 
{
    ElogErrCode result = ELOG_NO_ERR;

    elog_file_port_init();

    init_ok = true;
    return result;	
}

void elog_file_write(const char *log, size_t size)
{
    ELOG_ASSERT(init_ok);
    ELOG_ASSERT(log);

    elog_file_port_lock();   
    elog_file_port_write(log, size);
    elog_file_port_unlock();   
}

void elog_file_deinit(void)
{
    elog_file_port_deinit();
}
