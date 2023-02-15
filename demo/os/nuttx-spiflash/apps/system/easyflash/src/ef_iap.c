/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2017, Armink, <armink.ztl@gmail.com>
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
 * Function: IAP(In-Application Programming) operating interface.
 * Created on: 2015-01-05
 */

#include <easyflash.h>

#ifdef EF_USING_IAP

/* IAP section backup application section start address in flash */
static uint32_t bak_app_start_addr = 0;

/**
 * Flash IAP function initialize.
 *
 * @return result
 */
EfErrCode ef_iap_init(void) {
    EfErrCode result = EF_NO_ERR;

    bak_app_start_addr = EF_START_ADDR ;

#if defined(EF_USING_ENV)
    bak_app_start_addr += ENV_AREA_SIZE;
#endif

#if defined(EF_USING_LOG)
    bak_app_start_addr += LOG_AREA_SIZE;
#endif

    return result;
}

/**
 * Erase backup area application data.
 *
 * @param app_size application size
 *
 * @return result
 */
EfErrCode ef_erase_bak_app(size_t app_size) {
    EfErrCode result = EF_NO_ERR;

    result = ef_port_erase(ef_get_bak_app_start_addr(), app_size);
    switch (result) {
    case EF_NO_ERR: {
        EF_INFO("Erased backup area application OK.\n");
        break;
    }
    case EF_ERASE_ERR: {
        EF_INFO("Warning: Erase backup area application fault!\n");
        /* will return when erase fault */
        return result;
    }
    }

    return result;
}

/**
 * Erase user old application by using specified erase function.
 *
 * @param user_app_addr application entry address
 * @param app_size application size
 * @param app_erase user specified application erase function
 *
 * @return result
 */
EfErrCode ef_erase_spec_user_app(uint32_t user_app_addr, size_t app_size,
        EfErrCode (*app_erase)(uint32_t addr, size_t size)) {
    EfErrCode result = EF_NO_ERR;

    result = app_erase(user_app_addr, app_size);
    switch (result) {
    case EF_NO_ERR: {
        EF_INFO("Erased user application OK.\n");
        break;
    }
    case EF_ERASE_ERR: {
        EF_INFO("Warning: Erase user application fault!\n");
        /* will return when erase fault */
        return result;
    }
    }

    return result;
}

/**
 * Erase user old application by using default `ef_port_erase` function.
 *
 * @param user_app_addr application entry address
 * @param app_size application size
 *
 * @return result
 */
EfErrCode ef_erase_user_app(uint32_t user_app_addr, size_t app_size) {
    return ef_erase_spec_user_app(user_app_addr, app_size, ef_port_erase);
}

/**
 * Erase old bootloader
 *
 * @param bl_addr bootloader entry address
 * @param bl_size bootloader size
 *
 * @return result
 */
EfErrCode ef_erase_bl(uint32_t bl_addr, size_t bl_size) {
    EfErrCode result = EF_NO_ERR;

    result = ef_port_erase(bl_addr, bl_size);
    switch (result) {
    case EF_NO_ERR: {
        EF_INFO("Erased bootloader OK.\n");
        break;
    }
    case EF_ERASE_ERR: {
        EF_INFO("Warning: Erase bootloader fault!\n");
        /* will return when erase fault */
        return result;
    }
    }

    return result;
}

/**
 * Write data of application to backup area.
 *
 * @param data a part of application
 * @param size data size
 * @param cur_size current write application size
 * @param total_size application total size
 *
 * @return result
 */
EfErrCode ef_write_data_to_bak(uint8_t *data, size_t size, size_t *cur_size,
        size_t total_size) {
    EfErrCode result = EF_NO_ERR;

    /* make sure don't write excess data */
    if (*cur_size + size > total_size) {
        size = total_size - *cur_size;
    }

    result = ef_port_write(ef_get_bak_app_start_addr() + *cur_size, (uint32_t *) data, size);
    switch (result) {
    case EF_NO_ERR: {
        *cur_size += size;
        EF_DEBUG("Write data to backup area OK.\n");
        break;
    }
    case EF_WRITE_ERR: {
        EF_INFO("Warning: Write data to backup area fault!\n");
        break;
    }
    }

    return result;
}

/**
 * Copy backup area application to application entry by using specified write function.
 *
 * @param user_app_addr application entry address
 * @param app_size application size
 * @param app_write user specified application write function
 *
 * @return result
 */
EfErrCode ef_copy_spec_app_from_bak(uint32_t user_app_addr, size_t app_size,
        EfErrCode (*app_write)(uint32_t addr, const uint32_t *buf, size_t size)) {
    size_t cur_size;
    uint32_t app_cur_addr, bak_cur_addr;
    EfErrCode result = EF_NO_ERR;
    /* 32 words size buffer */
    uint32_t buff[32];

    /* cycle copy data */
    for (cur_size = 0; cur_size < app_size; cur_size += sizeof(buff)) {
        app_cur_addr = user_app_addr + cur_size;
        bak_cur_addr = ef_get_bak_app_start_addr() + cur_size;
        ef_port_read(bak_cur_addr, buff, sizeof(buff));
        result = app_write(app_cur_addr, buff, sizeof(buff));
        if (result != EF_NO_ERR) {
            break;
        }
    }

    switch (result) {
    case EF_NO_ERR: {
        EF_INFO("Write data to application entry OK.\n");
        break;
    }
    case EF_WRITE_ERR: {
        EF_INFO("Warning: Write data to application entry fault!\n");
        break;
    }
    }

    return result;
}

/**
 * Copy backup area application to application entry by using default `ef_port_write` function.
 *
 * @param user_app_addr application entry address
 * @param app_size application size
 *
 * @return result
 */
EfErrCode ef_copy_app_from_bak(uint32_t user_app_addr, size_t app_size) {
    return ef_copy_spec_app_from_bak(user_app_addr, app_size, ef_port_write);
}

/**
 * Copy backup area bootloader to bootloader entry.
 *
 * @param bl_addr bootloader entry address
 * @param bl_size bootloader size
 *
 * @return result
 */
EfErrCode ef_copy_bl_from_bak(uint32_t bl_addr, size_t bl_size) {
    size_t cur_size;
    uint32_t bl_cur_addr, bak_cur_addr;
    EfErrCode result = EF_NO_ERR;
    /* 32 words buffer */
    uint32_t buff[32];

    /* cycle copy data by 32bytes buffer */
    for (cur_size = 0; cur_size < bl_size; cur_size += sizeof(buff)) {
        bl_cur_addr = bl_addr + cur_size;
        bak_cur_addr = ef_get_bak_app_start_addr() + cur_size;
        ef_port_read(bak_cur_addr, buff, sizeof(buff));
        result = ef_port_write(bl_cur_addr, buff, sizeof(buff));
        if (result != EF_NO_ERR) {
            break;
        }
    }

    switch (result) {
    case EF_NO_ERR: {
        EF_INFO("Write data to bootloader entry OK.\n");
        break;
    }
    case EF_WRITE_ERR: {
        EF_INFO("Warning: Write data to bootloader entry fault!\n");
        break;
    }
    }

    return result;
}

/**
 * Get IAP section start address in flash.
 *
 * @return size
 */
uint32_t ef_get_bak_app_start_addr(void) {
    return bak_app_start_addr;
}

#endif /* EF_USING_IAP */
