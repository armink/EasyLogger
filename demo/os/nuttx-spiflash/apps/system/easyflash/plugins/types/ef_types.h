/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2016, Armink, <armink.ztl@gmail.com>
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
 * Function: It is an head file for this plugin. You can see all be called functions.
 * Created on: 2015-12-16
 */

#ifndef EF_TYPES_H_
#define EF_TYPES_H_

#include <easyflash.h>
#include <stdbool.h>
#include "struct2json\inc\s2j.h"

/* EasyFlash types plugin's software version number */
#define EF_TYPES_SW_VERSION                      "0.11.03"

typedef cJSON *(*ef_types_set_cb)(void* struct_obj);
typedef void *(*ef_types_get_cb)(cJSON* json_obj);

void ef_types_init(S2jHook *hook);
bool ef_get_bool(const char *key);
char ef_get_char(const char *key);
short ef_get_short(const char *key);
int ef_get_int(const char *key);
long ef_get_long(const char *key);
float ef_get_float(const char *key);
double ef_get_double(const char *key);
void ef_get_bool_array(const char *key, bool *value);
void ef_get_char_array(const char *key, char *value);
void ef_get_short_array(const char *key, short *value);
void ef_get_int_array(const char *key, int *value);
void ef_get_long_array(const char *key, long *value);
void ef_get_float_array(const char *key, float *value);
void ef_get_double_array(const char *key, double *value);
void ef_get_string_array(const char *key, char **value);
void *ef_get_struct(const char *key, ef_types_get_cb get_cb);
EfErrCode ef_set_bool(const char *key, bool value);
EfErrCode ef_set_char(const char *key, char value);
EfErrCode ef_set_short(const char *key, short value);
EfErrCode ef_set_int(const char *key, int value);
EfErrCode ef_set_long(const char *key, long value);
EfErrCode ef_set_float(const char *key, float value);
EfErrCode ef_set_double(const char *key, double value);
EfErrCode ef_set_bool_array(const char *key, bool *value, size_t len);
EfErrCode ef_set_char_array(const char *key, char *value, size_t len);
EfErrCode ef_set_short_array(const char *key, short *value, size_t len);
EfErrCode ef_set_int_array(const char *key, int *value, size_t len);
EfErrCode ef_set_long_array(const char *key, long *value, size_t len);
EfErrCode ef_set_float_array(const char *key, float *value, size_t len);
EfErrCode ef_set_double_array(const char *key, double *value, size_t len);
EfErrCode ef_set_string_array(const char *key, char **value, size_t len);
EfErrCode ef_set_struct(const char *key, void *value, ef_types_set_cb set_cb);

#endif /* EF_TYPES_H_ */
