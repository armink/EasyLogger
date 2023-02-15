/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2019-2020, Armink, <armink.ztl@gmail.com>
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
 * Function: It is the definitions head file for this library.
 * Created on: 2019-11-20
 */

#ifndef EF_DEF_H_
#define EF_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* EasyFlash software version number */
#define EF_SW_VERSION                  "4.1.99"
#define EF_SW_VERSION_NUM              0x40199

/*
 * ENV version number defined by user.
 * Please change it when your firmware add a new ENV to default_env_set.
 */
#ifndef EF_ENV_VER_NUM
#define EF_ENV_VER_NUM                 0
#endif

/* the ENV max name length must less then it */
#ifndef EF_ENV_NAME_MAX
#define EF_ENV_NAME_MAX                          32
#endif

/* EasyFlash debug print function. Must be implement by user. */
#ifdef PRINT_DEBUG
#define EF_DEBUG(...) ef_log_debug(__FILE__, __LINE__, __VA_ARGS__)
#else
#define EF_DEBUG(...)
#endif
/* EasyFlash routine print function. Must be implement by user. */
#define EF_INFO(...)  ef_log_info(__VA_ARGS__)
/* EasyFlash assert for developer. */
#define EF_ASSERT(EXPR)                                                       \
if (!(EXPR))                                                                  \
{                                                                             \
    EF_DEBUG("(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__);         \
    while (1);                                                                \
}

typedef struct _ef_env {
    char *key;
    void *value;
    size_t value_len;
} ef_env, *ef_env_t;

/* EasyFlash error code */
typedef enum {
    EF_NO_ERR,
    EF_ERASE_ERR,
    EF_READ_ERR,
    EF_WRITE_ERR,
    EF_ENV_NAME_ERR,
    EF_ENV_NAME_EXIST,
    EF_ENV_FULL,
    EF_ENV_INIT_FAILED,
} EfErrCode;

/* the flash sector current status */
typedef enum {
    EF_SECTOR_EMPTY,
    EF_SECTOR_USING,
    EF_SECTOR_FULL,
} EfSecrorStatus;

enum env_status {
    ENV_UNUSED,
    ENV_PRE_WRITE,
    ENV_WRITE,
    ENV_PRE_DELETE,
    ENV_DELETED,
    ENV_ERR_HDR,
    ENV_STATUS_NUM,
};
typedef enum env_status env_status_t;

struct env_node_obj {
    env_status_t status;                         /**< ENV node status, @see node_status_t */
    bool crc_is_ok;                              /**< ENV node CRC32 check is OK */
    uint8_t name_len;                            /**< name length */
    uint32_t magic;                              /**< magic word(`K`, `V`, `4`, `0`) */
    uint32_t len;                                /**< ENV node total length (header + name + value), must align by EF_WRITE_GRAN */
    uint32_t value_len;                          /**< value length */
    char name[EF_ENV_NAME_MAX];                  /**< name */
    struct {
        uint32_t start;                          /**< ENV node start address */
        uint32_t value;                          /**< value start address */
    } addr;
};
typedef struct env_node_obj *env_node_obj_t;

#ifdef __cplusplus
}
#endif

#endif /* EF_DEF_H_ */
