/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
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
 * Function: Portable interface for EasyLogger's flash log pulgin.
 * Created on: 2015-07-28
 */

#include "elog_flash.h"

/**
 * EasyLogger flash log pulgin port initialize
 *
 * @return result
 */
ElogErrCode elog_flash_port_init(void) {
    ElogErrCode result = ELOG_NO_ERR;
    
    /* add your code here */

    return result;
}

/**
 * output flash saved log port interface
 *
 * @param log flash saved log
 * @param size log size
 */
void elog_flash_port_output(const char *log, size_t size) {
    
    /* add your code here */
    
}

/**
 * flash log lock
 */
void elog_flash_port_lock(void) {
    
    /* add your code here */
    
}

/**
 * flash log unlock
 */
void elog_flash_port_unlock(void) {
    
    /* add your code here */
    
}