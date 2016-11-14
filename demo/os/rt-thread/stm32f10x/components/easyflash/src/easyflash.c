/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2014-2016, Armink, <armink.ztl@gmail.com>
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

/**
 *
 * This all Backup Area Flash storage index. All used flash area configure is under here.
 * |----------------------------|   Storage Size
 * | Environment variables area |   ENV area size @see ENV_AREA_SIZE
 * |      1.system section      |   ENV system section size
 * |      2:data section        |   ENV_AREA_SIZE - ENV system section size
 * |----------------------------|
 * |      Saved log area        |   Log area size @see LOG_AREA_SIZE
 * |----------------------------|
 * |(IAP)Downloaded application |   IAP already downloaded application, unfixed size
 * |----------------------------|
 *
 * @note all area size must be aligned with EF_ERASE_MIN_SIZE
 * @note EasyFlash will use ram to buffered the ENV. At some time flash's EF_ERASE_MIN_SIZE is so big,
 *       and you want use ENV size is less than it. So you must defined ENV_USER_SETTING_SIZE for ENV.
 * @note ENV area size has some limitations in different modes.
 *       1.Normal mode: no more limitations
 *       2.Wear leveling mode: system section will used an flash section and the data section will used at least 2 flash sections
 *       3.Power fail safeguard mode: ENV area will has an backup. It is twice as normal mode.
 *       4.wear leveling and power fail safeguard mode: The required capacity will be 2 times the total capacity in wear leveling mode.
 *       For example:
 *       The EF_ERASE_MIN_SIZE is 128K and the ENV_USER_SETTING_SIZE: 2K. The ENV_AREA_SIZE in different mode you can define
 *       1.Normal mode: 1*EF_ERASE_MIN_SIZE
 *       2.Wear leveling mode: 3*EF_ERASE_MIN_SIZE (It has 2 data section to store ENV. So ENV can erase at least 200,000 times)
 *       3.Power fail safeguard mode: 2*EF_ERASE_MIN_SIZE
 *       4.Wear leveling and power fail safeguard mode: 6*EF_ERASE_MIN_SIZE
 * @note the log area size must be more than twice of EF_ERASE_MIN_SIZE
 */
#include <easyflash.h>

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
        EF_DEBUG("EasyFlash V%s is initialize success.\n", EF_SW_VERSION);
    } else {
        EF_DEBUG("EasyFlash V%s is initialize fail.\n", EF_SW_VERSION);
    }
    EF_DEBUG("You can get the latest version on https://github.com/armink/EasyFlash .\n");

    return result;
}
