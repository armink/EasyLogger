/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2014-2019, Armink, <armink.ztl@gmail.com>
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
 * Created on: 2014-09-09
 */

/*
 *
 * This all Backup Area Flash storage index. All used flash area configure is under here.
 * |----------------------------|   Storage Size
 * | Environment variables area |   ENV area size @see ENV_AREA_SIZE
 * |----------------------------|
 * |      Saved log area        |   Log area size @see LOG_AREA_SIZE
 * |----------------------------|
 * |(IAP)Downloaded application |   IAP already downloaded application, unfixed size
 * |----------------------------|
 *
 * @note all area sizes must be aligned with EF_ERASE_MIN_SIZE
 *
 * The EasyFlash add the NG (Next Generation) mode start from V4.0. All old mode before V4.0, called LEGACY mode.
 *
 * - NG (Next Generation) mode is default mode from V4.0. It's easy to settings, only defined the ENV_AREA_SIZE.
 * - The LEGACY mode has been DEPRECATED. It is NOT RECOMMENDED to continue using.
 *   Beacuse it will use ram to buffer the ENV and spend more flash erase times.
 *   If you want use it please using the V3.X version.
 */

#include <easyflash.h>

#if !defined(EF_START_ADDR)
#error "Please configure backup area start address (in ef_cfg.h)"
#endif

#if !defined(EF_ERASE_MIN_SIZE)
#error "Please configure minimum size of flash erasure (in ef_cfg.h)"
#endif

/**
 * EasyFlash system initialize.
 *
 * @return result
 */
EfErrCode easyflash_init(void) {
    extern EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size);
    extern EfErrCode ef_env_init(ef_env const *default_env, size_t default_env_size);
    extern EfErrCode ef_iap_init(void);
    extern EfErrCode ef_log_init(void);

    size_t default_env_set_size = 0;
    const ef_env *default_env_set;
    EfErrCode result = EF_NO_ERR;
    static bool init_ok = false;

    if (init_ok) {
        return EF_NO_ERR;
    }

    result = ef_port_init(&default_env_set, &default_env_set_size);

#ifdef EF_USING_ENV
    if (result == EF_NO_ERR) {
        result = ef_env_init(default_env_set, default_env_set_size);
    }
#endif

#ifdef EF_USING_IAP
    if (result == EF_NO_ERR) {
        result = ef_iap_init();
    }
#endif

#ifdef EF_USING_LOG
    if (result == EF_NO_ERR) {
        result = ef_log_init();
    }
#endif

    if (result == EF_NO_ERR) {
        init_ok = true;
        EF_INFO("EasyFlash V%s is initialize success.\n", EF_SW_VERSION);
    } else {
        EF_INFO("EasyFlash V%s is initialize fail.\n", EF_SW_VERSION);
    }
    EF_INFO("You can get the latest version on https://github.com/armink/EasyFlash .\n");

    return result;
}
