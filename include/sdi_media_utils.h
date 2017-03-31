/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/******************************************************************************
 * \file sdi_media_utils.h
 * \brief Media util functions
 *****************************************************************************/
#ifndef __SDI_MEDIA__UTILS_H
#define __SDI_MEDIA__UTILS_H

#include "sdi_common.h"
#include "sdi_media.h"
#include <sx/sxd/sxd_dpt.h>
#include <sx/sxd/sxd_access_register.h>

#define SXD_DEVICE_ID    1
#define DEFAULT_ETH_SWID 0

#define CABLE_I2C_ADDR 0x50

#define MCIA_DATA_BATCH_SIZE (sizeof(uint32_t) * 12)

#define SDI_MEDIA_ID_TYPE_SFP       0x3
#define SDI_MEDIA_ID_TYPE_QSFP      0xc
#define SDI_MEDIA_ID_TYPE_QSFP_PLUS 0xd
#define SDI_MEDIA_ID_TYPE_QSFP_28   0x11

/**
 * @defgroup sdi_media_buf_size_t
 * List of the size values for buffer in bytes.
 */
typedef enum {
    SDI_MEDIA_BUF_SIZE_1 = 1,
    SDI_MEDIA_BUF_SIZE_2 = 2,
    SDI_MEDIA_BUF_SIZE_4 = 4,
    SDI_MEDIA_BUF_SIZE_8 = 8
} sdi_media_buf_size_t;

/**
 * @struct sdi_media_reg_info_t
 * Used to hold register info for the media resource.
 */
typedef struct sdi_media_reg_info_s {
    uint16_t addr;
    uint16_t size;
} sdi_media_reg_info_t;

/**
 * Get identifier type of media module.
 *
 * module_id[in] - media module ID.
 * identifier_type[out] - identifier type for the given module.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_media_identifier_get(uint8_t module_id, uint32_t *identifier_type);

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
t_std_error sdi_media_info_get(uint8_t module_id, uint8_t page, uint16_t addr, uint16_t size, uint8_t *buf);

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
t_std_error sdi_media_info_set(uint8_t module_id, uint8_t page, uint16_t addr, uint16_t size, uint8_t *buf);

/******************************************************************************
 * SFP related definitions
 *****************************************************************************/

/**
 * @defgroup sdi_sfp_pages_t
 * List of the supported SFP pages.
 */
typedef enum {
    SDI_SFP_PAGE_0 = 0,
    SDI_SFP_PAGE_2 = 2
} sdi_sfp_pages_t;

/**
 * @defgroup sdi_sfp_page0_addr_t
 * List of the addresses for SFP page 0.
 */
typedef enum {
    SFP_IDENTIFIER_ADDR = 0,
    SFP_EXT_IDENTIFIER_ADDR = 1,
    SFP_CONNECTOR_ADDR = 2,
    SFP_COMPLIANCE_CODE_ADDR = 3,
    SFP_ENCODING_TYPE_ADDR = 11,
    SFP_NM_BITRATE_ADDR = 12,
    SFP_LENGTH_SMF_KM_ADDR = 14,
    SFP_LENGTH_SMF_ADDR = 15,
    SFP_LENGTH_OM2_ADDR = 16,
    SFP_LENGTH_OM1_ADDR = 17,
    SFP_LENGTH_CABLE_ASSEMBLY_ADDR = 18,
    SFP_LENGTH_OM3_ADDR = 19,
    SFP_VENDOR_NAME_ADDR = 20,
    SFP_EXT_COMPLIANCE_CODE_ADDR = 36,
    SFP_VENDOR_OUI_ADDR = 37,
    SFP_VENDOR_PN_ADDR = 40,
    SFP_VENDOR_REVISION_ADDR = 56,
    SFP_WAVELENGTH_ADDR = 60,
    SFP_CC_BASE_ADDR = 63,
    SFP_OPTIONS_ADDR = 64,
    SFP_MAX_BITRATE_ADDR = 66,
    SFP_MIN_BITRATE_ADDR = 67,
    SFP_VENDOR_SN_ADDR = 68,
    SFP_VENDOR_DATE_ADDR = 84,
    SFP_DIAG_MON_TYPE_ADDR = 92,
    SFP_ENHANCED_OPTIONS_ADDR = 93,
    SFP_CC_EXT_ADDR = 95,
    SFP_DELL_PRODUCT_ID_ADDR = 96,
} sdi_sfp_page0_addr_t;

/* SFP info entries. Should be defined in the same order of sdi_media_param_type_t */
static sdi_media_reg_info_t sdi_sfp_info[] = {
    { SFP_WAVELENGTH_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { 0, 0 },
    { 0, 0 },
    { SFP_CC_BASE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_CC_EXT_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_CONNECTOR_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_ENCODING_TYPE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_NM_BITRATE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_IDENTIFIER_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_EXT_IDENTIFIER_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_LENGTH_SMF_KM_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_LENGTH_OM1_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_LENGTH_OM2_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_LENGTH_OM3_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_LENGTH_CABLE_ASSEMBLY_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_LENGTH_SMF_ADDR, SDI_MEDIA_BUF_SIZE_1},
    { SFP_OPTIONS_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_ENHANCED_OPTIONS_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_DIAG_MON_TYPE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { 0, 0 },
    { SFP_MAX_BITRATE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_MIN_BITRATE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { SFP_EXT_COMPLIANCE_CODE_ADDR, SDI_MEDIA_BUF_SIZE_1}
};

/* SFP vendor info entries. Should be defined in the same order of sdi_media_vendor_info_type_t */
static sdi_media_reg_info_t sdi_sfp_vendor_info[] = {
    { SFP_VENDOR_NAME_ADDR, SDI_MEDIA_MAX_VENDOR_NAME_LEN - 1 },
    { SFP_VENDOR_OUI_ADDR, SDI_MEDIA_MAX_VENDOR_OUI_LEN - 1 },
    { SFP_VENDOR_SN_ADDR, SDI_MEDIA_MAX_VENDOR_SERIAL_NUMBER_LEN - 1 },
    { SFP_VENDOR_DATE_ADDR, SDI_MEDIA_MAX_VENDOR_DATE_LEN - 1 },
    { SFP_VENDOR_PN_ADDR, SDI_MEDIA_MAX_VENDOR_PART_NUMBER_LEN - 1 },
    { SFP_VENDOR_REVISION_ADDR, SDI_MEDIA_MAX_VENDOR_REVISION_LEN - 1 }
};

/**
 * @defgroup sdi_sfp_page2_addr_t
 * List of the addresses for SFP page 2.
 */
typedef enum {
    SFP_TEMP_HIGH_ALARM_THRESHOLD_ADDR = 0,
    SFP_TEMP_LOW_ALARM_THRESHOLD_ADDR = 2,
    SFP_TEMP_HIGH_WARNING_THRESHOLD_ADDR = 4,
    SFP_TEMP_LOW_WARNING_THRESHOLD_ADDR = 6,
    SFP_VOLT_HIGH_ALARM_THRESHOLD_ADDR = 8,
    SFP_VOLT_LOW_ALARM_THRESHOLD_ADDR = 10,
    SFP_VOLT_HIGH_WARNING_THRESHOLD_ADDR = 12,
    SFP_VOLT_LOW_WARNING_THRESHOLD_ADDR = 14,
    SFP_BIAS_HIGH_ALARM_THRESHOLD_ADDR = 16,
    SFP_BIAS_LOW_ALARM_THRESHOLD_ADDR = 18,
    SFP_BIAS_HIGH_WARNING_THRESHOLD_ADDR = 20,
    SFP_BIAS_LOW_WARNING_THRESHOLD_ADDR = 22,
    SFP_TX_PWR_HIGH_ALARM_THRESHOLD_ADDR = 24,
    SFP_TX_PWR_LOW_ALARM_THRESHOLD_ADDR = 26,
    SFP_TX_PWR_HIGH_WARNING_THRESHOLD_ADDR = 28,
    SFP_TX_PWR_LOW_WARNING_THRESHOLD_ADDR = 30,
    SFP_RX_PWR_HIGH_ALARM_THRESHOLD_ADDR = 32,
    SFP_RX_PWR_LOW_ALARM_THRESHOLD_ADDR = 34,
    SFP_RX_PWR_HIGH_WARNING_THRESHOLD_ADDR = 36,
    SFP_RX_PWR_LOW_WARNING_THRESHOLD_ADDR = 38,
    SFP_CALIB_RX_POWER_CONST_START_ADDR = 56,
    SFP_CALIB_TX_BIAS_SLOPE_ADDR = 76,
    SFP_CALIB_TX_BIAS_CONST_ADDR = 78,
    SFP_CALIB_TX_POWER_SLOPE_ADDR = 80,
    SFP_CALIB_TX_POWER_CONST_ADDR = 82,
    SFP_CALIB_TEMP_SLOPE_ADDR = 84,
    SFP_CALIB_TEMP_CONST_ADDR = 86,
    SFP_CALIB_VOLT_SLOPE_ADDR = 88,
    SFP_CALIB_VOLT_CONST_ADDR = 90,
    SFP_TEMPERATURE_ADDR = 96,
    SFP_VOLTAGE_ADDR = 98,
    SFP_TX_BIAS_CURRENT_ADDR = 100,
    SFP_TX_OUTPUT_POWER_ADDR = 102,
    SFP_RX_INPUT_POWER_ADDR = 104,
    SFP_OPTIONAL_STATUS_CONTROL_ADDR = 110,
    SFP_ALARM_STATUS_1_ADDR = 112,
    SFP_ALARM_STATUS_2_ADDR = 113,
    SFP_WARNING_STATUS_1_ADDR = 116,
    SFP_WARNING_STATUS_2_ADDR = 117,
    SFP_TARGET_WAVELENGTH_ADDR = 146
} sdi_sfp_page2_addr_t;

#define SFP_TEMP_HIGH_ALARM_BIT (1 << 7)
#define SFP_TEMP_LOW_ALARM_BIT  (1 << 6)
#define SFP_VOLT_HIGH_ALARM_BIT (1 << 5)
#define SFP_VOLT_LOW_ALARM_BIT  (1 << 4)

#define SFP_TEMP_HIGH_WARNING_BIT (1 << 7)
#define SFP_TEMP_LOW_WARNING_BIT  (1 << 6)
#define SFP_VOLT_HIGH_WARNING_BIT (1 << 5)
#define SFP_VOLT_LOW_WARNING_BIT  (1 << 4)

#define SFP_TEMP_HIGH_ALARM_BIT (1 << 7)
#define SFP_TEMP_LOW_ALARM_BIT  (1 << 6)
#define SFP_VOLT_HIGH_ALARM_BIT (1 << 5)
#define SFP_VOLT_LOW_ALARM_BIT  (1 << 4)

#define SFP_TX_BIAS_HIGH_ALARM_BIT (1 << 3)
#define SFP_TX_BIAS_LOW_ALARM_BIT  (1 << 2)
#define SFP_TX_PWR_HIGH_ALARM_BIT  (1 << 1)
#define SFP_TX_PWR_LOW_ALARM_BIT   (1)

#define SFP_RX_PWR_HIGH_ALARM_BIT (1 << 7)
#define SFP_RX_PWR_LOW_ALARM_BIT  (1 << 6)

#define SFP_TEMP_HIGH_WARNING_BIT    (1 << 7)
#define SFP_TEMP_LOW_WARNING_BIT     (1 << 6)
#define SFP_VOLT_HIGH_WARNING_BIT    (1 << 5)
#define SFP_VOLT_LOW_WARNING_BIT     (1 << 4)
#define SFP_TX_BIAS_HIGH_WARNING_BIT (1 << 3)
#define SFP_TX_BIAS_LOW_WARNING_BIT  (1 << 2)
#define SFP_TX_PWR_HIGH_WARNING_BIT  (1 << 1)
#define SFP_TX_PWR_LOW_WARNING_BIT   (1)

#define SFP_RX_PWR_HIGH_WARNING_BIT (1 << 7)
#define SFP_RX_PWR_LOW_WARNING_BIT  (1 << 6)

#define SFP_RX_LOSS_STATE_BIT         (1 << 1)
#define SFP_TX_FAULT_STATE_BIT        (1 << 2)
#define SFP_SOFT_TX_DISABLE_STATE_BIT (1 << 6)
#define SFP_TX_DISABLE_STATE_BIT      (1 << 7)

#define SFP_ALARM_SUPPORT_BIT (1 << 7)

#define SFP_DIAG_MON_SUPPORT_BIT (1 << 6)

#define SFP_RATE_SELECT_BIT (1 << 1)

/* SFP thresholds entries. Should be defined in the same order of sdi_media_threshold_type_t */
static sdi_media_reg_info_t sdi_sfp_thresholds[] = {
    { SFP_TEMP_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_TEMP_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_TEMP_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_TEMP_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_VOLT_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_VOLT_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_VOLT_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_VOLT_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_RX_PWR_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_RX_PWR_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_RX_PWR_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_RX_PWR_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_BIAS_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_BIAS_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_BIAS_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_BIAS_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_TX_PWR_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_TX_PWR_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_TX_PWR_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { SFP_TX_PWR_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 }
};

/******************************************************************************
 * QSFP related definitions
 *****************************************************************************/

/**
 * @defgroup sdi_qsfp_channel_t
 * List of the channel values for QSFP.
 */
enum {
    SDI_QSFP_CHANNEL1,
    SDI_QSFP_CHANNEL2,
    SDI_QSFP_CHANNEL3,
    SDI_QSFP_CHANNEL4,
} sdi_qsfp_channel_t;

/**
 * @defgroup sdi_qsfp_page_t
 * List of the supported QSFP pages.
 */
typedef enum {
    SDI_QSFP_PAGE_0,
    SDI_QSFP_PAGE_1,
    SDI_QSFP_PAGE_2,
    SDI_QSFP_PAGE_3
} sdi_qsfp_page_t;

/**
 * @defgroup qsfp_page0_addr_t
 * List of addresses for QSFP page 0.
 */
typedef enum {
    QSFP_STATUS_INDICATOR_ADDR = 2,
    QSFP_CHANNEL_LOS_INDICATOR_ADDR = 3,
    QSFP_CHANNEL_TXFAULT_ADDR = 4,
    QSFP_TEMP_INTERRUPT_ADDR = 6,
    QSFP_VOLT_INTERRUPT_ADDR = 7,
    QSFP_RX12_POWER_INTERRUPT_ADDR = 9,
    QSFP_RX34_POWER_INTERRUPT_ADDR = 10,
    QSFP_TX12_BIAS_INTERRUPT_ADDR = 11,
    QSFP_TX34_BIAS_INTERRUPT_ADDR = 12,
    QSFP_TEMPERATURE_ADDR = 22,
    QSFP_VOLTAGE_ADDR = 26,
    QSFP_RX1_POWER_ADDR = 34,
    QSFP_RX2_POWER_ADDR = 36,
    QSFP_RX3_POWER_ADDR = 38,
    QSFP_RX4_POWER_ADDR = 40,
    QSFP_TX1_POWER_BIAS_ADDR = 42,
    QSFP_TX2_POWER_BIAS_ADDR = 44,
    QSFP_TX3_POWER_BIAS_ADDR = 46,
    QSFP_TX4_POWER_BIAS_ADDR = 48,
    QSFP_TX_CONTROL_ADDR = 86,
    QSFP_CDR_CONTROL_ADDR = 98,
    QSFP_PAGE_SELECT_BYTE_ADDR = 127,
    QSFP_IDENTIFIER_ADDR = 128,
    QSFP_EXT_IDENTIFIER_ADDR = 129,
    QSFP_CONNECTOR_ADDR = 130,
    QSFP_COMPLIANCE_CODE_ADDR = 131,
    QSFP_ENCODING_TYPE_ADDR = 139,
    QSFP_NM_BITRATE_ADDR = 140,
    QSFP_LENGTH_SMF_KM_ADDR = 142,
    QSFP_LENGTH_OM3_ADDR = 143,
    QSFP_LENGTH_OM2_ADDR = 144,
    QSFP_LENGTH_OM1_ADDR = 145,
    QSFP_LENGTH_CABLE_ASSEMBLY_ADDR = 146,
    QSFP_DEVICE_TECH_ADDR = 147,
    QSFP_VENDOR_NAME_ADDR = 148,
    QSFP_VENDOR_OUI_ADDR = 165,
    QSFP_VENDOR_PN_ADDR = 168,
    QSFP_VENDOR_REVISION_ADDR = 184,
    QSFP_WAVELENGTH_ADDR = 186,
    QSFP_WAVELENGTH_TOLERANCE_ADDR = 188,
    QSFP_MAX_CASE_TEMP_ADDR = 190,
    QSFP_CC_BASE_ADDR = 191,
    QSFP_OPTIONS1_ADDR = 192,
    QSFP_OPTIONS2_ADDR = 193,
    QSFP_OPTIONS3_ADDR = 194,
    QSFP_OPTIONS4_ADDR = 195,
    QSFP_VENDOR_SN_ADDR = 196,
    QSFP_VENDOR_DATE_ADDR = 212,
    QSFP_DIAG_MON_TYPE_ADDR = 220,
    QSFP_ENHANCED_OPTIONS_ADDR = 221,
    QSFP_CC_EXT_ADDR = 223
} qsfp_page0_addr_t;

#define QSFP_TEMP_HIGH_ALARM_BIT   (1 << 7)
#define QSFP_TEMP_LOW_ALARM_BIT    (1 << 6)
#define QSFP_TEMP_HIGH_WARNING_BIT (1 << 5)
#define QSFP_TEMP_LOW_WARNING_BIT  (1 << 4)

#define QSFP_VOLT_HIGH_ALARM_BIT   (1 << 7)
#define QSFP_VOLT_LOW_ALARM_BIT    (1 << 6)
#define QSFP_VOLT_HIGH_WARNING_BIT (1 << 5)
#define QSFP_VOLT_LOW_WARNING_BIT  (1 << 4)

#define QSFP_RX13_POWER_HIGH_ALARM_BIT   (1 << 7)
#define QSFP_RX13_POWER_LOW_ALARM_BIT    (1 << 6)
#define QSFP_RX13_POWER_HIGH_WARNING_BIT (1 << 5)
#define QSFP_RX13_POWER_LOW_WARNING_BIT  (1 << 4)
#define QSFP_RX24_POWER_HIGH_ALARM_BIT   (1 << 3)
#define QSFP_RX24_POWER_LOW_ALARM_BIT    (1 << 2)
#define QSFP_RX24_POWER_HIGH_WARNING_BIT (1 << 1)
#define QSFP_RX24_POWER_LOW_WARNING_BIT  (1 << 0)

#define QSFP_TX13_BIAS_HIGH_ALARM_BIT   (1 << 7)
#define QSFP_TX13_BIAS_LOW_ALARM_BIT    (1 << 6)
#define QSFP_TX13_BIAS_HIGH_WARNING_BIT (1 << 5)
#define QSFP_TX13_BIAS_LOW_WARNING_BIT  (1 << 4)
#define QSFP_TX24_BIAS_HIGH_ALARM_BIT   (1 << 3)
#define QSFP_TX24_BIAS_LOW_ALARM_BIT    (1 << 2)
#define QSFP_TX24_BIAS_HIGH_WARNING_BIT (1 << 1)
#define QSFP_TX24_BIAS_LOW_WARNING_BIT  (1 << 0)

#define QSFP_TX_DISABLE_BIT  (1 << 4)
#define QSFP_RATE_SELECT_BIT (1 << 5)

#define QSFP_FLAT_MEM_BIT (1 << 2)

/* QSFP info entries. Should be defined in the same order of sdi_media_param_type_t */
static sdi_media_reg_info_t sdi_qsfp_info[] = {
    { QSFP_WAVELENGTH_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_WAVELENGTH_TOLERANCE_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_MAX_CASE_TEMP_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_CC_BASE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_CC_EXT_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_CONNECTOR_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_ENCODING_TYPE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_NM_BITRATE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_IDENTIFIER_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_EXT_IDENTIFIER_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_LENGTH_SMF_KM_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_LENGTH_OM1_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_LENGTH_OM2_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_LENGTH_OM3_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_LENGTH_CABLE_ASSEMBLY_ADDR, SDI_MEDIA_BUF_SIZE_1  },
    { 0, 0},
    { QSFP_OPTIONS1_ADDR, SDI_MEDIA_BUF_SIZE_4 },
    { QSFP_ENHANCED_OPTIONS_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_DIAG_MON_TYPE_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { QSFP_DEVICE_TECH_ADDR, SDI_MEDIA_BUF_SIZE_1 },
    { 0, 0},
    { 0, 0},
    { 0, 0}
};

/* QSFP vendor info entries. Should be defined in the same order of sdi_media_vendor_info_type_t */
static sdi_media_reg_info_t sdi_qsfp_vendor_info[] = {
    { QSFP_VENDOR_NAME_ADDR, SDI_MEDIA_MAX_VENDOR_NAME_LEN - 1 },
    { QSFP_VENDOR_OUI_ADDR, SDI_MEDIA_MAX_VENDOR_OUI_LEN - 1 },
    { QSFP_VENDOR_SN_ADDR, SDI_MEDIA_MAX_VENDOR_SERIAL_NUMBER_LEN - 1 },
    { QSFP_VENDOR_DATE_ADDR, SDI_MEDIA_MAX_VENDOR_DATE_LEN - 1 },
    { QSFP_VENDOR_PN_ADDR, SDI_MEDIA_MAX_VENDOR_PART_NUMBER_LEN - 1 },
    { QSFP_VENDOR_REVISION_ADDR, SDI_MEDIA_MAX_VENDOR_REVISION_LEN - 1 }
};

/**
 * @defgroup qsfp_page3_addr_t
 * List of addresses for QSFP page 3.
 */
typedef enum {
    QSFP_TEMP_HIGH_ALARM_THRESHOLD_ADDR = 128,
    QSFP_TEMP_LOW_ALARM_THRESHOLD_ADDR = 130,
    QSFP_TEMP_HIGH_WARNING_THRESHOLD_ADDR = 132,
    QSFP_TEMP_LOW_WARNING_THRESHOLD_ADDR = 134,
    QSFP_VOLT_HIGH_ALARM_THRESHOLD_ADDR = 144,
    QSFP_VOLT_LOW_ALARM_THRESHOLD_ADDR = 146,
    QSFP_VOLT_HIGH_WARNING_THRESHOLD_ADDR = 148,
    QSFP_VOLT_LOW_WARNING_THRESHOLD_ADDR = 150,
    QSFP_RX_PWR_HIGH_ALARM_THRESHOLD_ADDR = 176,
    QSFP_RX_PWR_LOW_ALARM_THRESHOLD_ADDR = 178,
    QSFP_RX_PWR_HIGH_WARNING_THRESHOLD_ADDR = 180,
    QSFP_RX_PWR_LOW_WARNING_THRESHOLD_ADDR = 182,
    QSFP_TX_BIAS_HIGH_ALARM_THRESHOLD_ADDR = 184,
    QSFP_TX_BIAS_LOW_ALARM_THRESHOLD_ADDR = 186,
    QSFP_TX_BIAS_HIGH_WARNING_THRESHOLD_ADDR = 188,
    QSFP_TX_BIAS_LOW_WARNING_THRESHOLD_ADDR = 190
} qsfp_page3_addr_t;

/* QSFP thresholds entries. Should be defined in the same order of sdi_media_threshold_type_t */
static sdi_media_reg_info_t sdi_qsfp_thresholds[] = {
    { QSFP_TEMP_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_TEMP_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_TEMP_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_TEMP_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_VOLT_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_VOLT_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_VOLT_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_VOLT_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_RX_PWR_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_RX_PWR_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_RX_PWR_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_RX_PWR_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_TX_BIAS_HIGH_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_TX_BIAS_LOW_ALARM_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_TX_BIAS_HIGH_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { QSFP_TX_BIAS_LOW_WARNING_THRESHOLD_ADDR, SDI_MEDIA_BUF_SIZE_2 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 }
};

#endif /* __SDI_MEDIA__UTILS_H */
