/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015-2019, Qintl, <qintl_linux@163.com>
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
 * Function: Save log to file.
 * Created on: 2019-01-05
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#include <file/elog_file.h>
#include <file/elog_file_cfg.h>
/* initialize OK flag */
static bool init_ok = false;
static FILE *fp;
static int fd;
static Elog_File_Cfg file;

static void elog_file_config_init(Elog_File_Cfg *file);

ElogErrCode elog_file_init(void) 
{
    ElogErrCode result = ELOG_NO_ERR;
    if (init_ok)
        goto __exit;

    elog_file_config_init(&file);

    fp = fopen(file.name, "a+");
    if (fp)
        fd = fileno(fp);
    else
        fd = -1;

    elog_file_port_init();

    init_ok = true;
__exit:
    return result;	
}

void elog_file_write(const char *log, size_t size)
{
    ELOG_ASSERT(init_ok);
    ELOG_ASSERT(log);
    struct stat statbuf;

    statbuf.st_size = 0;
    fstat(fd, &statbuf);


    if (unlikely(statbuf.st_size > file.max_size))
        return ;

    elog_file_port_lock();

    fwrite(log, size, 1, fp);
#ifdef ELOG_FILE_FLUSH_CAHCE_ENABLE
    fflush(fp);
    fsync(fd);
#endif 

    elog_file_port_unlock();
}

void elog_file_deinit(void)
{
    ELOG_ASSERT(init_ok);

    elog_file_port_deinit();
    fclose(fp);
}

static void elog_file_config_init(Elog_File_Cfg *file)
{
    file->name = ELOG_FILE_NAME;
    file->max_size = ELOG_FILE_MAX_SIZE;
}
