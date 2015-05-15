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
 * Function: Some utils for this library.
 * Created on: 2015-04-28
 */

#include "elog.h"

/**
 * another copy string function
 *
 * @param cur_len current copied log length, max size is ELOG_BUF_SIZE
 * @param dst destination
 * @param src source
 *
 * @return copied length
 */
size_t elog_strcpy(size_t cur_len, char *dst, const char *src) {
    const char *src_old = src;
    while (*src != 0) {
        /* make sure destination has enough space */
        if (cur_len++ <= ELOG_BUF_SIZE) {
            *dst++ = *src++;
        } else {
            break;
        }
    }
    return src - src_old;
}
