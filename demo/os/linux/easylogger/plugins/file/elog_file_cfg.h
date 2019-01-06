#ifndef _ELOG_FILE_CFG_H_
#define _ELOG_FILE_CFG_H_

/* EasyLogger file log plugin's using file name */
#define ELOG_FILE_NAME      "/tmp/elog_file.log"

/* EasyLogger file log plugin's using file max size */
#define ELOG_FILE_MAX_SIZE  (10 * 1024 * 1024)

/* EasyLogger file log plugin's using semaphore key */
#define ELOG_FILE_SEM_KEY   ((key_t)0x19910612)

#endif /* _ELOG_FILE_CFG_H_ */
