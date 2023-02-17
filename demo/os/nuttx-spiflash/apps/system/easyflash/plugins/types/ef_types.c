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
 * Function: Types plugin source code for this library.
 * Created on: 2015-12-16
 */

#include "ef_types.h"
#include <stdlib.h>
#include <stdio.h>

/**
 *  array support types
 */
typedef enum {
    EF_ARRAY_TYPES_BOOL,
    EF_ARRAY_TYPES_CHAR,
    EF_ARRAY_TYPES_SHORT,
    EF_ARRAY_TYPES_INT,
    EF_ARRAY_TYPES_LONG,
    EF_ARRAY_TYPES_FLOAT,
    EF_ARRAY_TYPES_DOUBLE,
    EF_ARRAY_TYPES_STRING,
} ef_array_types;

/**
 * EasyFlash types plugin initialize.
 *
 * @param hook Memory management hook function.
 *             If hook is null or not call this function, then use free and malloc of C library.
 */
void ef_types_init(S2jHook *hook) {
    s2j_init(hook);
}

bool ef_get_bool(const char *key) {
    char *value = ef_get_env(key);
    if(value) {
        return atoi(value) == 0 ? false : true;
    } else {
        EF_INFO("Couldn't find this ENV(%s)!\n", key);
        return false;
    }
}

char ef_get_char(const char *key) {
    return ef_get_long(key);
}

short ef_get_short(const char *key) {
    return ef_get_long(key);
}

int ef_get_int(const char *key) {
    return ef_get_long(key);
}

long ef_get_long(const char *key) {
    char *value = ef_get_env(key);
    if(value) {
        return atol(value);
    } else {
        EF_INFO("Couldn't find this ENV(%s)!\n", key);
        return NULL;
    }
}

float ef_get_float(const char *key) {
    return ef_get_double(key);
}

double ef_get_double(const char *key) {
    char *value = ef_get_env(key);
    if(value) {
        return atof(value);
    } else {
        EF_INFO("Couldn't find this ENV(%s)!\n", key);
        return NULL;
    }
}

/**
 * get array ENV value
 *
 * @param key ENV name
 * @param value returned ENV value
 * @param types ENV array's type
 */
static void ef_get_array(const char *key, void *value, ef_array_types types) {
    char *char_value = ef_get_env(key);
    cJSON *array;
    size_t size, i;

    EF_ASSERT(value);

    if (char_value) {
        array = cJSON_Parse(char_value);
        if (array) {
            size = cJSON_GetArraySize(array);
            for (i = 0; i < size; i++) {
                switch (types) {
                case EF_ARRAY_TYPES_BOOL: {
                    *((bool *) value + i) = cJSON_GetArrayItem(array, i)->valueint;
                    break;
                }
                case EF_ARRAY_TYPES_CHAR: {
                    *((char *) value + i) = cJSON_GetArrayItem(array, i)->valueint;
                    break;
                }
                case EF_ARRAY_TYPES_SHORT: {
                    *((short *) value + i) = cJSON_GetArrayItem(array, i)->valueint;
                    break;
                }
                case EF_ARRAY_TYPES_INT: {
                    *((int *) value + i) = cJSON_GetArrayItem(array, i)->valueint;
                    break;
                }
                case EF_ARRAY_TYPES_LONG: {
                    *((long *) value + i) = cJSON_GetArrayItem(array, i)->valueint;
                    break;
                }
                case EF_ARRAY_TYPES_FLOAT: {
                    *((float *) value + i) = cJSON_GetArrayItem(array, i)->valuedouble;
                    break;
                }
                case EF_ARRAY_TYPES_DOUBLE: {
                    *((double *) value + i) = cJSON_GetArrayItem(array, i)->valuedouble;
                    break;
                }
                case EF_ARRAY_TYPES_STRING: {
                    *((char **) value + i) = cJSON_GetArrayItem(array, i)->valuestring;
                    break;
                }
                }
            }
        } else {
            EF_INFO("This ENV(%s) value type has error!\n", key);
        }
        cJSON_Delete(array);
    } else {
        EF_INFO("Couldn't find this ENV(%s)!\n", key);
    }
}

void ef_get_bool_array(const char *key, bool *value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_BOOL);
}

void ef_get_char_array(const char *key, char *value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_CHAR);
}

void ef_get_short_array(const char *key, short *value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_SHORT);
}

void ef_get_int_array(const char *key, int *value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_INT);
}

void ef_get_long_array(const char *key, long *value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_LONG);
}

void ef_get_float_array(const char *key, float *value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_FLOAT);
}

void ef_get_double_array(const char *key, double *value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_DOUBLE);
}

void ef_get_string_array(const char *key, char **value) {
    ef_get_array(key, value, EF_ARRAY_TYPES_STRING);
}

/**
 * get structure ENV value
 *
 * @param key ENV name
 * @param get_cb get structure callback function.
 *        You can use json to structure function which in the struct2json lib(https://github.com/armink/struct2json).
 *
 * @return value returned structure ENV value pointer. @note The returned value will malloc new ram.
 *         You must free the value then used finish.
 */
void *ef_get_struct(const char *key, ef_types_get_cb get_cb) {
    char *char_value = ef_get_env(key);
    cJSON *json_value = cJSON_Parse(char_value);
    void *value = NULL;

    if (json_value) {
        value = get_cb(json_value);
        cJSON_Delete(json_value);
    }
    return value;
}

EfErrCode ef_set_bool(const char *key, bool value) {
    char char_value[2] = { 0 };
    if (!value) {
        strcpy(char_value, "0");
    } else {
        strcpy(char_value, "1");
    }
    return ef_set_env(key, char_value);
}

EfErrCode ef_set_char(const char *key, char value) {
    return ef_set_long(key, value);
}

EfErrCode ef_set_short(const char *key, short value) {
    return ef_set_long(key, value);
}

EfErrCode ef_set_int(const char *key, int value) {
    return ef_set_long(key, value);
}

EfErrCode ef_set_long(const char *key, long value) {
    char char_value[21] = { 0 };

    snprintf(char_value, 20, "%ld", value);

    return ef_set_env(key, char_value);
}

EfErrCode ef_set_float(const char *key, float value) {
    return ef_set_double(key, value);
}

EfErrCode ef_set_double(const char *key, double value) {
    char char_value[21] = { 0 };

    snprintf(char_value, 20, "%lf", value);

    return ef_set_env(key, char_value);
}

/**
 * set array ENV value
 *
 * @param key ENV name
 * @param value ENV value
 * @param len array length
 * @param types ENV array's type
 *
 * @return ENV set result
 */
static EfErrCode ef_set_array(const char *key, void *value, size_t len, ef_array_types types) {
    char *char_value = NULL;
    cJSON *array = NULL, *array_item = NULL;
    size_t i;
    EfErrCode result = EF_NO_ERR;

    EF_ASSERT(value);

    array = cJSON_CreateArray();
    if (array) {
        for (i = 0; i < len; i++) {
            switch (types) {
            case EF_ARRAY_TYPES_BOOL: {
                array_item = cJSON_CreateBool(*((bool *) value + i));
                break;
            }
            case EF_ARRAY_TYPES_CHAR: {
                array_item = cJSON_CreateNumber(*((char *) value + i));
                break;
            }
            case EF_ARRAY_TYPES_SHORT: {
                array_item = cJSON_CreateNumber(*((short *) value + i));
                break;
            }
            case EF_ARRAY_TYPES_INT: {
                array_item = cJSON_CreateNumber(*((int *) value + i));
                break;
            }
            case EF_ARRAY_TYPES_LONG: {
                array_item = cJSON_CreateNumber(*((long *) value + i));
                break;
            }
            case EF_ARRAY_TYPES_FLOAT: {
                array_item = cJSON_CreateNumber(*((float *) value + i));
                break;
            }
            case EF_ARRAY_TYPES_DOUBLE: {
                array_item = cJSON_CreateNumber(*((double *) value + i));
                break;
            }
            case EF_ARRAY_TYPES_STRING: {
                array_item = cJSON_CreateString(*((char **) value + i));
                break;
            }
            default:
                /* the types parameter has error */
                EF_ASSERT(0);
            }
            if (array_item) {
                cJSON_AddItemToArray(array, array_item);
            } else {
                result = EF_ENV_FULL;
                EF_INFO("Memory full!\n", key);
                break;
            }
        }
        char_value = cJSON_PrintUnformatted(array);
        if (char_value) {
            result = ef_set_env(key, char_value);
            s2jHook.free_fn(char_value);
        } else {
            result = EF_ENV_FULL;
            EF_INFO("Memory full!\n", key);
        }
        cJSON_Delete(array);
    } else {
        result = EF_ENV_FULL;
        EF_INFO("Memory full!\n", key);
    }
    return result;
}

EfErrCode ef_set_bool_array(const char *key, bool *value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_BOOL);
}

EfErrCode ef_set_char_array(const char *key, char *value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_CHAR);
}

EfErrCode ef_set_short_array(const char *key, short *value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_SHORT);
}

EfErrCode ef_set_int_array(const char *key, int *value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_INT);
}

EfErrCode ef_set_long_array(const char *key, long *value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_LONG);
}

EfErrCode ef_set_float_array(const char *key, float *value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_FLOAT);
}

EfErrCode ef_set_double_array(const char *key, double *value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_DOUBLE);
}

EfErrCode ef_set_string_array(const char *key, char **value, size_t len) {
    return ef_set_array(key, value, len, EF_ARRAY_TYPES_STRING);
}

/**
 * set structure ENV value
 *
 * @param key ENV name
 * @param value structure ENV value pointer
 * @param get_cb set structure callback function.
 *        You can use structure to json function which in the struct2json lib(https://github.com/armink/struct2json).
 */
EfErrCode ef_set_struct(const char *key, void *value, ef_types_set_cb set_cb) {
    EfErrCode result = EF_NO_ERR;
    cJSON *json_value = set_cb(value);
    char *char_value = cJSON_PrintUnformatted(json_value);

    result = ef_set_env(key, char_value);

    cJSON_Delete(json_value);
    s2jHook.free_fn(char_value);

    return result;
}
