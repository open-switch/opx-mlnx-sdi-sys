/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/******************************************************************************
 * \file sdi_eeprom_utils.h
 * \brief EEPROM util functions
 *****************************************************************************/
#ifndef __SDI_EEPROM__UTILS_H
#define __SDI_EEPROM__UTILS_H

#include "sdi_common.h"

#define SDI_ONIE_ID_STRING_SIZE 8

#define EEPROM_FAN_MLNX_MULTIPLIER            16 /**< Multiplier to calculate offset for specific block */
#define EEPROM_FAN_MLNX_SANITY_OFFSET         8  /**< Sanity string offset */
#define EEPROM_FAN_MLNX_BLOCK1_START          12 /**< Start of block 1 */
#define EEPROM_FAN_MLNX_BLOCK1_TYPE           1  /**< Type of the block 1 */
#define EEPROM_FAN_MLNX_BLOCK1_SERIAL_OFFSET  8  /**< Serial number offset from block 1 */
#define EEPROM_FAN_MLNX_BLOCK1_SERIAL_LEN     24 /**< Serial number length */
#define EEPROM_FAN_MLNX_BLOCK1_PART_OFFSET    32 /**< Part number offset from block 1 */
#define EEPROM_FAN_MLNX_BLOCK1_PART_LEN       20 /**< Part number length */
#define EEPROM_FAN_MLNX_BLOCK1_REV_OFFSET     52 /**< Revision offset from block 1 */
#define EEPROM_FAN_MLNX_BLOCK1_REV_LEN        4  /**< Revision length */
#define EEPROM_FAN_MLNX_BLOCK1_PRODUCT_OFFSET 60 /**< Product name offset from block 1 */
#define EEPROM_FAN_MLNX_BLOCK1_PRODUCT_LEN    64 /**< Product name length */
#define EEPROM_FAN_MLNX_BLOCK2_START          14 /**< Start of block 2 */
#define EEPROM_FAN_MLNX_BLOCK2_TYPE           5  /**< Type of block 2 */
#define EEPROM_FAN_MLNX_BLOCK2_FAN_OFFSET     14 /**< Fan direction offset from brom block 2 */
#define EEPROM_FAN_MLNX_BLOCK2_FAN_NORMAL     1  /**< Normal direction of fan */
#define EEPROM_FAN_MLNX_BLOCK2_FAN_REVERSE    2  /**< Reverse direction of fan */

#define EEPROM_PSU_MLNX_SERIAL_LEN 24 /**< Serial number length */
#define EEPROM_PSU_MLNX_PART_LEN   20 /**< Part number length */
#define EEPROM_PSU_MLNX_REV_LEN    4  /**< Revision length */

/**
 * @defgroup sdi_eeprom_type_t
 * List of supproted EEPROM types.
 */
typedef enum {
    SDI_EEPROM_SYS_ONIE, /**< ONIE system EEPROM */
    SDI_EEPROM_FAN_MLNX, /**< Mellanox fan EEPROM */
    SDI_EEPROM_PSU_MLNX  /**< Mellanox PSU EEPROM */
} sdi_eeprom_type_t;


/**
 * @defgroup sdi_eeprom_sys_onie_type_t
 * List of ONIE EEPROM type codes.
 */
typedef enum {
    SDI_EEPROM_SYS_ONIE_PRODUCT_NAME = 0x21,     /**< ASCII string containing the product name */
    SDI_EEPROM_SYS_ONIE_PART_NUMBER = 0x22,      /**< ASCII string containing the vendor's part number */
    SDI_EEPROM_SYS_ONIE_SERIAL_NUMBER = 0x23,    /**< ASCII string containing the serial number of the device */
    SDI_EEPROM_SYS_ONIE_BASE_MAC_ADDR = 0x24,    /**< Six bytes containing the base MAC address of the device */
    SDI_EEPROM_SYS_ONIE_MANUFACTURE_DATE = 0x25, /**< ASCII string that specifies when the device was manufactured */
    SDI_EEPROM_SYS_ONIE_DEVICE_VERSION = 0x26,   /**< Single byte indicating the version of the device */
    SDI_EEPROM_SYS_ONIE_LABEL_REVISION = 0x27,   /**< ASCII string containing the label revision */
    SDI_EEPROM_SYS_ONIE_PLATFORM_NAME = 0x28,    /**< ASCII string which identifies a CPU subsystem */
    SDI_EEPROM_SYS_ONIE_VERSION = 0x29,          /**< ASCII string containing the version of the ONIE */
    SDI_EEPROM_SYS_ONIE_NUM_MACS = 0x2a,         /**< Number of MACs (2 bytes in big-endian format) */
    SDI_EEPROM_SYS_ONIE_MANUFACTURER = 0x2b,     /**< ASCII string containing the name manufacturer */
    SDI_EEPROM_SYS_ONIE_COUNTRY_CODE = 0x2c,     /**< Two-byte ASCII string containing code of the country */
    SDI_EEPROM_SYS_ONIE_VENDOR = 0x2d,           /**< ASCII string containing the name of the vendor */
    SDI_EEPROM_SYS_ONIE_DIAG_VERSION = 0x2e,     /**< An ASCII string containing the version of the diagnostic software */
    SDI_EEPROM_SYS_ONIE_SERVICE_TAG = 0x2f,      /**< An ASCII string containing a vendor defined service tag */
    SDI_EEPROM_SYS_ONIE_VENDOR_EXTENSION = 0xfd, /**< This type code allows vendors to include extra information */
    SDI_EEPROM_SYS_ONIE_CRC32 = 0xfe             /**< Four-byte CRC which covers the EEPROM contents */
} sdi_eeprom_sys_onie_type_t;

/**
 * @struct sdi_eeprom_onie_tlv_t
 * Structure for representing the ONIE EEPROM fields in TLV format.
 */
typedef struct {
    uint8_t type;     /**< Type of the value field */
    uint8_t length;   /**< Number of bytes in the value field */
    uint8_t value[0]; /**< Value for specified type code */
} __attribute__((packed)) sdi_eeprom_onie_tlv_t;

/**
 * @struct sdi_eeprom_onie_header_t
 * Structure for representing the ONIE EEPROM header.
 */
typedef struct {
    char     id_string[SDI_ONIE_ID_STRING_SIZE]; /**< ID String */
    uint8_t  hdr_version;                    /**< TLV Header Version */
    uint16_t total_len;                      /**< Total Number of bytes of data */
} __attribute__((packed)) sdi_eeprom_onie_header_t;

/**
 * Gets the EEPROM type based on name.
 *
 * eeprom_name[in] - EEPROM type in string format.
 *
 * return EEPROM type.
 */
sdi_eeprom_type_t sdi_eeprom_string_to_type(const char *eeprom_name);

/**
 * Fills entity_info structure with the info from ONIE EEPROM system info.
 *
 * buf[in] - ONIE EEPROM raw data.
 * size[in] - size of ONIE EEPROM raw data.
 * entity_info[out] - entity_info structure to fill.
 *
 * return None
 */
t_std_error sdi_eeprom_sys_onie_get(char *buf, size_t size, sdi_entity_info_t *entity_info);

/**
 * Fills entity_info structure with the info from Mellanox FAN EEPROM.
 *
 * buf[in] - Mellanox FAN EEPROM raw data.
 * size[in] - size of Mellanox FAN EEPROM raw data.
 * entity_info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_eeprom_fan_mlnx_get(char *buf, size_t size, sdi_entity_info_t *entity_info);

/**
 * Fills entity_info structure with the info from Mellanox PSU EEPROM.
 *
 * buf[in] - Mellanox PSU EEPROM raw data.
 * size[in] - size of Mellanox PSU EEPROM raw data.
 * entity_info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_eeprom_psu_mlnx_get(char *buf, size_t size, sdi_entity_info_t *entity_info);

#endif /* __SDI_EEPROM__UTILS_H */
