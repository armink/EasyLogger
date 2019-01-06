#ifndef __ELOG_FILE__H__
#define __ELOG_FILE__H__

#include <elog.h>

/* EasyLogger file log plugin's software version number */  
#define ELOG_FILE_SW_VERSION                "V1.0.0"

#ifdef __cplusplus
extern "C" {
#endif
	/* elog_file.c */
	ElogErrCode elog_file_init(void);
	void elog_file_write(const char *log, size_t size);
	void elog_file_deinit(void);

	/* elog_file_port.c */
	ElogErrCode elog_flash_port_init(void);
	void elog_file_port_write(const char *log, size_t size);
	void elog_file_port_lock(void);
	void elog_file_port_unlock(void);
	void elog_file_port_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
