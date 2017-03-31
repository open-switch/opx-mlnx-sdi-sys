/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * Media util functions to get and set attributes of resources.
 ***************************************************************************************/

#include "sdi_media_utils.h"


/**
 * Util function to get the data from MCIA register.
 *
 * module_id[in] - media module ID.
 * i2c_adrr[in] - I2C device address.
 * page[in] - memory page number.
 * addr[in] - memory address.
 * size[in] - size of data to get.
 * reg[out] - pointer to MCIA structure.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
static sxd_status_t sxd_mcia_reg_get(uint8_t             module_id,
                                     uint8_t             i2c_adrr,
                                     uint8_t             page,
                                     uint16_t            addr,
                                     uint16_t            size,
                                     struct ku_mcia_reg* reg)
{
    sxd_reg_meta_t reg_meta;
    sxd_status_t   status = SXD_STATUS_SUCCESS;

    if (reg == NULL) {
        return SXD_STATUS_ERROR;
    }

    memset(&reg_meta, 0, sizeof(reg_meta));
    memset(reg, 0, sizeof(*reg));

    reg_meta.access_cmd = SXD_ACCESS_CMD_GET;
    reg_meta.dev_id = SXD_DEVICE_ID;
    reg_meta.swid = DEFAULT_ETH_SWID;
    reg->i2c_device_address = i2c_adrr;
    reg->page_number = page;
    reg->device_address = addr;
    reg->size = size;
    reg->module = module_id;

    status = sxd_access_reg_mcia(reg, &reg_meta, 1, NULL, NULL);
    if (status != SXD_STATUS_SUCCESS) {
        SDI_ERRMSG_LOG("Failed read MCIA register (i2c_addr:%x page_number:%x offset:%x size:%d).",
                       i2c_adrr, page, addr, size);
        return status;
    }

    reg->dword_0 = ntohl(reg->dword_0);
    reg->dword_1 = ntohl(reg->dword_1);
    reg->dword_2 = ntohl(reg->dword_2);
    reg->dword_3 = ntohl(reg->dword_3);
    reg->dword_4 = ntohl(reg->dword_4);
    reg->dword_5 = ntohl(reg->dword_5);
    reg->dword_6 = ntohl(reg->dword_6);
    reg->dword_7 = ntohl(reg->dword_7);
    reg->dword_8 = ntohl(reg->dword_8);
    reg->dword_9 = ntohl(reg->dword_9);
    reg->dword_10 = ntohl(reg->dword_10);
    reg->dword_11 = ntohl(reg->dword_11);

    return status;
}

/**
 * Util function to set the data to MCIA register.
 *
 * module_id[in] - media module ID.
 * i2c_adrr[in] - I2C device address.
 * page[in] - memory page number.
 * addr[in] - memory address.
 * size[in] - size of data to set.
 * reg[out] - pointer to MCIA structure.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
static sxd_status_t sxd_mcia_reg_set(uint8_t             module_id,
                                     uint8_t             i2c_adrr,
                                     uint8_t             page,
                                     uint16_t            addr,
                                     uint16_t            size,
                                     struct ku_mcia_reg* reg)
{
    sxd_reg_meta_t reg_meta;
    sxd_status_t   status = SXD_STATUS_SUCCESS;

    if (reg == NULL) {
        return SXD_STATUS_ERROR;
    }

    memset(&reg_meta, 0, sizeof(reg_meta));

    reg_meta.access_cmd = SXD_ACCESS_CMD_SET;
    reg_meta.dev_id = SXD_DEVICE_ID;
    reg_meta.swid = DEFAULT_ETH_SWID;
    reg->i2c_device_address = i2c_adrr;
    reg->page_number = page;
    reg->device_address = addr;
    reg->size = size;
    reg->module = module_id;

    reg->dword_0 = htonl(reg->dword_0);
    reg->dword_1 = htonl(reg->dword_1);
    reg->dword_2 = htonl(reg->dword_2);
    reg->dword_3 = htonl(reg->dword_3);
    reg->dword_4 = htonl(reg->dword_4);
    reg->dword_5 = htonl(reg->dword_5);
    reg->dword_6 = htonl(reg->dword_6);
    reg->dword_7 = htonl(reg->dword_7);
    reg->dword_8 = htonl(reg->dword_8);
    reg->dword_9 = htonl(reg->dword_9);
    reg->dword_10 = htonl(reg->dword_10);
    reg->dword_11 = htonl(reg->dword_11);

    status = sxd_access_reg_mcia(reg, &reg_meta, 1, NULL, NULL);
    if (status != SXD_STATUS_SUCCESS) {
        SDI_ERRMSG_LOG("Failed read MCIA register (i2c_addr:%x page_number:%x offset:%x size:%d).",
                       i2c_adrr, page, addr, size);
        return status;
    }

    return status;
}

/**
 * Get identifier type of media module.
 *
 * module_id[in] - media module ID.
 * identifier_type[out] - identifier type for the given module.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_media_identifier_get(uint8_t module_id, uint32_t *identifier_type)
{
    sxd_status_t       status = SXD_STATUS_SUCCESS;
    struct ku_mcia_reg reg;

    status = sxd_mcia_reg_get(module_id, CABLE_I2C_ADDR, 0, 0, sizeof(*identifier_type), &reg);
    if (status != SXD_STATUS_SUCCESS) {
        return SDI_ERRCODE(-1);
    }

    *identifier_type = reg.dword_0 & 0xff;

    return STD_ERR_OK;
}

/**
 * Get info from media module register.
 *
 * module_id[in] - media module ID.
 * page[in] - memory page number.
 * addr[in] - memory address.
 * size[in] - size of data to get.
 * buf[out] - buffer to store the data.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_media_info_get(uint8_t module_id, uint8_t page, uint16_t addr, uint16_t size, uint8_t *buf)
{
    struct ku_mcia_reg reg;
    sxd_status_t       status = SXD_STATUS_SUCCESS;
    uint16_t           batch_count = size / MCIA_DATA_BATCH_SIZE;
    uint16_t           tail_size = size % MCIA_DATA_BATCH_SIZE;
    uint16_t           i = 0;

    memset(&reg, 0, sizeof(reg));

    if (buf == NULL) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    for (i = 0; i < batch_count; i++) {
        status = sxd_mcia_reg_get(module_id, CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * i),
                                  MCIA_DATA_BATCH_SIZE, &reg);
        if (status != SXD_STATUS_SUCCESS) {
            SDI_ERRMSG_LOG("Failed read MCIA batch (i2c_addr:%x page_number:%x offset:%x size:%d).",
                           CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * i), MCIA_DATA_BATCH_SIZE);
            return SDI_ERRCODE(-1);
        }

        memcpy(buf + (MCIA_DATA_BATCH_SIZE * i), (uint8_t*)&reg.dword_0, MCIA_DATA_BATCH_SIZE);
    }

    if (tail_size == 0) {
        return STD_ERR_OK;
    }

    status = sxd_mcia_reg_get(module_id, CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * batch_count),
                              tail_size, &reg);
    if (status != SXD_STATUS_SUCCESS) {
        SDI_ERRMSG_LOG("Failed read MCIA batch (i2c_addr:%x page_number:%x offset:%x size:%d).",
                       CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * batch_count), tail_size);
        return SDI_ERRCODE(-1);
    }

    memcpy(buf + (MCIA_DATA_BATCH_SIZE * batch_count), (uint8_t*)&reg.dword_0, tail_size);

    return STD_ERR_OK;
}

/**
 * Set info to media module register.
 *
 * module_id[in] - media module ID.
 * page[in] - memory page number.
 * addr[in] - memory address.
 * size[in] - size of data to set.
 * buf[out] - data to be set.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_media_info_set(uint8_t module_id, uint8_t page, uint16_t addr, uint16_t size, uint8_t *buf)
{
    struct ku_mcia_reg reg;
    sxd_status_t       status = SXD_STATUS_SUCCESS;
    uint16_t           batch_count = size / MCIA_DATA_BATCH_SIZE;
    uint16_t           tail_size = size % MCIA_DATA_BATCH_SIZE;
    uint16_t           i = 0;

    memset(&reg, 0, sizeof(reg));

    if (buf == NULL) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    for (i = 0; i < batch_count; i++) {
        memcpy((uint8_t*)&reg.dword_0, buf + (MCIA_DATA_BATCH_SIZE * i), MCIA_DATA_BATCH_SIZE);

        status = sxd_mcia_reg_set(module_id, CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * i),
                                  MCIA_DATA_BATCH_SIZE, &reg);
        if (status != SXD_STATUS_SUCCESS) {
            SDI_ERRMSG_LOG("Failed write MCIA batch (i2c_addr:%x page_number:%x offset:%x size:%d).",
                           CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * i), MCIA_DATA_BATCH_SIZE);
            return SDI_ERRCODE(-1);
        }
    }

    if (tail_size == 0) {
        return STD_ERR_OK;
    }

    memcpy((uint8_t*)&reg.dword_0, buf + (MCIA_DATA_BATCH_SIZE * batch_count), tail_size);

    status = sxd_mcia_reg_set(module_id, CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * batch_count),
                              tail_size, &reg);
    if (status != SXD_STATUS_SUCCESS) {
        SDI_ERRMSG_LOG("Failed write MCIA batch (i2c_addr:%x page_number:%x offset:%x size:%d).",
                       CABLE_I2C_ADDR, page, addr + (MCIA_DATA_BATCH_SIZE * batch_count), tail_size);
        return SDI_ERRCODE(-1);
    }

    return STD_ERR_OK;
}
