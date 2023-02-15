/*
 * This file is part of the struct2json Library.
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
 * Function: Initialize interface for this library.
 * Created on: 2015-10-14
 */

#include <s2j.h>
#include <stdlib.h>

S2jHook s2jHook = {
        .malloc_fn = malloc,
        .free_fn = free,
};

/**
 * struct2json library initialize
 * @note It will initialize cJSON library hooks.
 */
void s2j_init(S2jHook *hook) {
    /* initialize cJSON library */
    cJSON_InitHooks((cJSON_Hooks *)hook);
    /* initialize hooks */
    if (hook) {
        s2jHook.malloc_fn = (hook->malloc_fn) ? hook->malloc_fn : malloc;
        s2jHook.free_fn = (hook->free_fn) ? hook->free_fn : free;
    } else {
        hook->malloc_fn = malloc;
        hook->free_fn = free;
    }
}
