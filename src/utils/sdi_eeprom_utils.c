/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * EEPROM util functions to get and set attributes of resources.
 ***************************************************************************************/

#include "sdi_eeprom_utils.h"


/* Note: Names must be in the same order as defined for enum sdi_eeprom_type_t */
static const char * sdi_eeprom_names[] = {
    "SDI_EEPROM_SYS_ONIE",
    "SDI_EEPROM_FAN_MLNX",
    "SDI_EEPROM_PSU_MLNX"
};

/**
 * Gets the EEPROM type based on name.
 *
 * eeprom_name[in] - EEPROM type in string format.
 *
 * return EEPROM type.
 */
sdi_eeprom_type_t sdi_eeprom_string_to_type(const char *eeprom_name)
{
    int          eeprom_index = -1;
    unsigned int max_num = (sizeof(sdi_eeprom_names) / sizeof(sdi_eeprom_names[0]));

    STD_ASSERT(eeprom_name != NULL);

    eeprom_index = dn_std_string_to_enum(sdi_eeprom_names, max_num, eeprom_name);

    if (eeprom_index < STD_ERR_OK) {
        /* Could not find entity type, means SDI entity-list is corrupted, hence assert */
        STD_ASSERT(false);
    }

    return (sdi_eeprom_type_t)eeprom_index;
}

/**
 * Fills entity_info structure with the info from ONIE EEPROM system info.
 *
 * buf[in] - ONIE EEPROM raw data.
 * size[in] - size of ONIE EEPROM raw data.
 * entity_info[out] - entity_info structure to fill.
 *
 * return None
 */
t_std_error sdi_eeprom_sys_onie_get(char *buf, size_t size, sdi_entity_info_t *entity_info)
{
    int                    tlv_len = 0;
    sdi_eeprom_onie_tlv_t *tlv = NULL;

    if ((buf == NULL) || (entity_info == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    /* Check whether size of buffer is correct */
    tlv_len = ntohs(((sdi_eeprom_onie_header_t*)buf)->total_len);
    if (tlv_len >= size) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    buf += sizeof(sdi_eeprom_onie_header_t);

    memset(entity_info, 0, sizeof(sdi_entity_info_t));

    /* Set default values */
    strncpy(entity_info->service_tag, "N/A", sizeof(entity_info->service_tag));
    strncpy(entity_info->hw_revision, "0", sizeof(entity_info->hw_revision));

    while (tlv_len > 0) {
        tlv = (sdi_eeprom_onie_tlv_t*)buf;

        switch (tlv->type) {
        case SDI_EEPROM_SYS_ONIE_PRODUCT_NAME:
            if (tlv->length < sizeof(entity_info->prod_name)) {
                safestrncpy(entity_info->prod_name, tlv->value, tlv->length + 1);
            }
            break;

        case SDI_EEPROM_SYS_ONIE_PART_NUMBER:
            if (tlv->length < sizeof(entity_info->part_number)) {
                safestrncpy(entity_info->part_number, tlv->value, tlv->length + 1);
            }
            break;

        case SDI_EEPROM_SYS_ONIE_SERIAL_NUMBER:
            if (tlv->length < sizeof(entity_info->ppid)) {
                safestrncpy(entity_info->ppid, tlv->value, tlv->length + 1);
            }
            break;

        case SDI_EEPROM_SYS_ONIE_BASE_MAC_ADDR:
            if (tlv->length <= sizeof(entity_info->base_mac)) {
                memcpy(entity_info->base_mac, tlv->value, tlv->length);
            }
            break;

        case SDI_EEPROM_SYS_ONIE_LABEL_REVISION:
            if (tlv->length < sizeof(entity_info->hw_revision)) {
                safestrncpy(entity_info->hw_revision, tlv->value, tlv->length + 1);
            }
            break;

        case SDI_EEPROM_SYS_ONIE_PLATFORM_NAME:
            if (tlv->length < sizeof(entity_info->platform_name)) {
                safestrncpy(entity_info->platform_name, tlv->value, tlv->length + 1);
            }
            break;

        case SDI_EEPROM_SYS_ONIE_NUM_MACS:
            entity_info->mac_size = ntohs(tlv->value[1] | (tlv->value[0] << 8));
            break;

        case SDI_EEPROM_SYS_ONIE_MANUFACTURER:
            if (tlv->length < sizeof(entity_info->vendor_name)) {
                safestrncpy(entity_info->vendor_name, tlv->value, tlv->length + 1);
            }
            break;

        case SDI_EEPROM_SYS_ONIE_SERVICE_TAG:
            if (tlv->length < sizeof(entity_info->service_tag)) {
                safestrncpy(entity_info->service_tag, tlv->value, tlv->length + 1);
            }
            break;

        default:
            /* Do not return an error intentionally, since not all types are required,
             *  so if we go here it doesn't mean that error occurred.
             */
            break;
        }

        tlv_len -= tlv->length + sizeof(tlv->length) + sizeof(tlv->type);
        buf += tlv->length + sizeof(tlv->length) + sizeof(tlv->type);
    }

    return STD_ERR_OK;
}

/**
 * Fills entity_info structure with the info from Mellanox FAN EEPROM.
 *
 * buf[in] - Mellanox FAN EEPROM raw data.
 * size[in] - size of Mellanox FAN EEPROM raw data.
 * entity_info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_eeprom_fan_mlnx_get(char *buf, size_t size, sdi_entity_info_t *entity_info)
{
    const char  sanity_checker[] = "MLNX";
    uint8_t     offset = 0;
    size_t      len = 0;
    t_std_error rc = STD_ERR_OK;

    if ((buf == NULL) || (entity_info == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    /* Check whether size of buffer is correct  */
    offset = buf[EEPROM_FAN_MLNX_BLOCK2_START] * EEPROM_FAN_MLNX_MULTIPLIER + EEPROM_FAN_MLNX_BLOCK2_FAN_OFFSET;
    if (offset >= size) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    /* Sanity check */
    if (strncmp(sanity_checker, &buf[EEPROM_FAN_MLNX_SANITY_OFFSET], strlen(sanity_checker)) != 0) {
        SDI_ERRMSG_LOG("%s:%d Sanity check failed.", __FUNCTION__, __LINE__);
        return SDI_ERRCODE(-1);
    }

    /* Check EEPROM block 1 type  */
    if (buf[EEPROM_FAN_MLNX_BLOCK1_START + 1] != EEPROM_FAN_MLNX_BLOCK1_TYPE) {
        SDI_ERRMSG_LOG("%s:%d Type of block 1 is incorrect.", __FUNCTION__, __LINE__);
        return SDI_ERRCODE(-1);
    }

    /* Get serial number */
    offset = buf[EEPROM_FAN_MLNX_BLOCK1_START] * EEPROM_FAN_MLNX_MULTIPLIER + EEPROM_FAN_MLNX_BLOCK1_SERIAL_OFFSET;
    if (EEPROM_FAN_MLNX_BLOCK1_SERIAL_LEN > sizeof(entity_info->ppid)) {
        len = sizeof(entity_info->part_number);
    } else {
        len = EEPROM_FAN_MLNX_BLOCK1_SERIAL_LEN;
    }
    safestrncpy(entity_info->ppid, &buf[offset], len);

    /* Get part number */
    offset = buf[EEPROM_FAN_MLNX_BLOCK1_START] * EEPROM_FAN_MLNX_MULTIPLIER + EEPROM_FAN_MLNX_BLOCK1_PART_OFFSET;
    if (EEPROM_FAN_MLNX_BLOCK1_PART_LEN >= sizeof(entity_info->part_number)) {
        len = sizeof(entity_info->part_number);
    } else {
        len = EEPROM_FAN_MLNX_BLOCK1_PART_LEN;
    }
    safestrncpy(entity_info->part_number, &buf[offset], len);

    /* Get HW revision */
    offset = buf[EEPROM_FAN_MLNX_BLOCK1_START] * EEPROM_FAN_MLNX_MULTIPLIER + EEPROM_FAN_MLNX_BLOCK1_REV_OFFSET;
    if (EEPROM_FAN_MLNX_BLOCK1_REV_LEN >= sizeof(entity_info->hw_revision)) {
        len = sizeof(entity_info->hw_revision);
    } else {
        len = EEPROM_FAN_MLNX_BLOCK1_REV_LEN;
    }
    safestrncpy(entity_info->hw_revision, &buf[offset], len);

    /* Get product name */
    offset = buf[EEPROM_FAN_MLNX_BLOCK1_START] * EEPROM_FAN_MLNX_MULTIPLIER + EEPROM_FAN_MLNX_BLOCK1_PRODUCT_OFFSET;
    if (EEPROM_FAN_MLNX_BLOCK1_PRODUCT_LEN >= sizeof(entity_info->prod_name)) {
        len = sizeof(entity_info->prod_name);
    } else {
        len = EEPROM_FAN_MLNX_BLOCK1_PRODUCT_LEN;
    }
    safestrncpy(entity_info->prod_name, &buf[offset], len);

    /* Check EEPROM block 2 type  */
    if (buf[EEPROM_FAN_MLNX_BLOCK2_START + 1] != EEPROM_FAN_MLNX_BLOCK2_TYPE) {
        SDI_ERRMSG_LOG("%s:%d Type of block 2 is incorrect.", __FUNCTION__, __LINE__);
        return SDI_ERRCODE(-1);
    }

    /* Get fan direction */
    offset = buf[EEPROM_FAN_MLNX_BLOCK2_START] * EEPROM_FAN_MLNX_MULTIPLIER + EEPROM_FAN_MLNX_BLOCK2_FAN_OFFSET;
    if (offset >= size) {
        return SDI_ERRCODE(-1);
    }

    switch (buf[offset]) {
    case EEPROM_FAN_MLNX_BLOCK2_FAN_NORMAL:
        entity_info->air_flow = SDI_PWR_AIR_FLOW_NORMAL;
        break;

    case EEPROM_FAN_MLNX_BLOCK2_FAN_REVERSE:
        entity_info->air_flow = SDI_PWR_AIR_FLOW_REVERSE;
        break;

    default:
        SDI_ERRMSG_LOG("%s:%d Type of fan air flow direction is incorrect.", __FUNCTION__, __LINE__);
        return SDI_ERRCODE(-1);
    }

    return STD_ERR_OK;
}

/**
 * Fills entity_info structure with the info from Mellanox PSU EEPROM.
 *
 * buf[in] - Mellanox PSU EEPROM raw data.
 * size[in] - size of Mellanox PSU EEPROM raw data.
 * entity_info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_eeprom_psu_mlnx_get(char *buf, size_t size, sdi_entity_info_t *entity_info)
{
    const char sanity_checker[] = "MLNX";
    bool       sanity_found = false;
    size_t     index = 0;
    size_t     len = 0;

    if ((buf == NULL) || (entity_info == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    /* Sanity check */
    while (index < (size - strlen(sanity_checker))) {
        if (strncmp(sanity_checker, &buf[index], strlen(sanity_checker)) == 0) {
            sanity_found = true;
            break;
        }

        index++;
    }

    if (sanity_found != true) {
        SDI_ERRMSG_LOG("%s:%d Sanity check failed.", __FUNCTION__, __LINE__);
        return SDI_ERRCODE(-1);
    }

    /* Set default values */
    strncpy(entity_info->prod_name, "N/A", sizeof(entity_info->prod_name));

    /* Get serial number */
    index += strlen(sanity_checker);

    if (EEPROM_PSU_MLNX_SERIAL_LEN >= sizeof(entity_info->ppid)) {
        len = sizeof(entity_info->ppid);
    } else {
        len = EEPROM_PSU_MLNX_SERIAL_LEN;
    }

    if ((index + len) > size) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    safestrncpy(entity_info->ppid, &buf[index], len);

    /* Get part number */
    index += EEPROM_PSU_MLNX_SERIAL_LEN;

    if (EEPROM_PSU_MLNX_PART_LEN >= sizeof(entity_info->part_number)) {
        len = sizeof(entity_info->part_number);
    } else {
        len = EEPROM_PSU_MLNX_PART_LEN;
    }

    if ((index + len) > size) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    safestrncpy(entity_info->part_number, &buf[index], len);

    /* Get HW revision */
    index += EEPROM_PSU_MLNX_PART_LEN;

    if (EEPROM_PSU_MLNX_REV_LEN >= sizeof(entity_info->hw_revision)) {
        len = sizeof(entity_info->hw_revision);
    } else {
        len = EEPROM_PSU_MLNX_REV_LEN;
    }

    if ((index + len) > size) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    safestrncpy(entity_info->hw_revision, &buf[index], len);

    return STD_ERR_OK;
}
