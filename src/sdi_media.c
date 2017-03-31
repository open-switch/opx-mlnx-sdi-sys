/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/*******************************************************************************
 * Implementation of Media resource API.
 ******************************************************************************/

#include "sdi_media.h"
#include "sdi_common.h"
#include "sdi_media_utils.h"
#include <sx/sxd/sxd_dpt.h>
#include <sx/sxd/sxd_access_register.h>


/**
 * @struct sdi_media_settings_t
 * Used to hold settings for the media resource.
 */
typedef struct sdi_media_settings_s {
    char    name[SDI_MAX_NAME_LEN];     /**< name of the media SysFs attribute */
    char    path[PATH_MAX];             /**< path to the media SysFs attribute */
    char    status[SDI_MAX_NAME_LEN];   /**< name of the media "present status" SysFs attribute */
    char    not_present[SDI_MAX_NAME_LEN]; /**< value of the "Not present" status */
    uint8_t module;                     /**< media module ID */
} sdi_media_settings_t;

/**
 * Registers settings for the specified media resource.
 *
 * hdl[in] - handle of the resource.
 * media_node[in] - config node for the resource settings.
 *
 * return None.
 */
void sdi_media_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t media_node)
{
    char                 *name = NULL;
    char                 *path = NULL;
    char                 *status = NULL;
    char                 *not_present = NULL;
    char                 *module = NULL;
    sdi_media_settings_t *settings = NULL;

    memset(&settings, 0, sizeof(settings));

    STD_ASSERT(hdl != NULL);
    STD_ASSERT(media_node != NULL);

    STD_ASSERT((name = std_config_attr_get(media_node, "name")) != NULL);
    STD_ASSERT((path = std_config_attr_get(media_node, "path")) != NULL);
    STD_ASSERT((status = std_config_attr_get(media_node, "status")) != NULL);
    STD_ASSERT((not_present = std_config_attr_get(media_node, "not_present")) != NULL);
    STD_ASSERT((module = std_config_attr_get(media_node, "module")) != NULL);

    settings = (sdi_media_settings_t*)calloc(1, sizeof(sdi_media_settings_t));
    STD_ASSERT(settings != NULL);

    strncpy(settings->name, name, sizeof(settings->name));
    strncpy(settings->path, path, sizeof(settings->path));
    strncpy(settings->status, status, sizeof(settings->status));
    strncpy(settings->not_present, not_present, sizeof(settings->not_present));
    settings->module = atoi(module);

    hdl->settings = (void*)settings;

    if (sxd_access_reg_init(0, NULL, SX_VERBOSITY_LEVEL_INFO) != SXD_STATUS_SUCCESS) {
        STD_ASSERT(false);
    }
}

/**
 * Get the present status of the specific media
 *
 * resource_hdl[in] - Handle of the resource
 * pres[out]        - "true" if module is present else "false"
 *
 * return t_std_error
 */
t_std_error sdi_media_presence_get(sdi_resource_hdl_t resource_hdl, bool *pres)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    char                    status[SDI_MAX_NAME_LEN] = {0};

    STD_ASSERT(pres != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    *pres = false;

    rc = sdi_sysfs_attr_str_get(settings->path, settings->status, status);
    if (rc == STD_ERR_OK) {
        if (strncmp(settings->not_present, status, sizeof(settings->not_present)) != 0) {
            *pres = true;
        }
    }

    return rc;
}

/**
 * Gets the required module monitors(temperature and voltage) alarm status
 *
 * resource_hdl[in] - Handle of the resource
 * flags[in]        - flags for status that are of interest
 * status[out]      - returns the set of status flags
 *
 * return t_std_error
 */
t_std_error sdi_media_module_monitor_status_get(sdi_resource_hdl_t resource_hdl, uint_t flags, uint_t *status)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf = 0;

    STD_ASSERT(status != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        /* Get temperature alarm and warning status */
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_TEMP_INTERRUPT_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_STATUS_TEMP_HIGH_ALARM) && (buf & QSFP_TEMP_HIGH_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_HIGH_ALARM;
            }
            if ((flags & SDI_MEDIA_STATUS_TEMP_LOW_ALARM) && (buf & QSFP_TEMP_LOW_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_LOW_ALARM;
            }
            if ((flags & SDI_MEDIA_STATUS_TEMP_HIGH_WARNING) && (buf & QSFP_TEMP_HIGH_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_HIGH_WARNING;
            }
            if ((flags & SDI_MEDIA_STATUS_TEMP_LOW_WARNING) && (buf & QSFP_TEMP_LOW_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_LOW_WARNING;
            }
        } else {
            return rc;
        }
        /* Get voltage alarm and warning status */
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_VOLT_INTERRUPT_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_STATUS_VOLT_HIGH_ALARM) && (buf & QSFP_VOLT_HIGH_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_HIGH_ALARM;
            }
            if ((flags & SDI_MEDIA_STATUS_VOLT_LOW_ALARM) && (buf & QSFP_VOLT_LOW_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_LOW_ALARM;
            }
            if ((flags & SDI_MEDIA_STATUS_VOLT_HIGH_WARNING) && (buf & QSFP_VOLT_HIGH_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_HIGH_WARNING;
            }
            if ((flags & SDI_MEDIA_STATUS_VOLT_LOW_WARNING) && (buf & QSFP_VOLT_LOW_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_LOW_WARNING;
            }
        } else {
            return rc;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        /* Get temperature and voltage alarm status */
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_ALARM_STATUS_1_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_STATUS_TEMP_HIGH_ALARM) && (buf & SFP_TEMP_HIGH_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_HIGH_ALARM;
            }
            if ((flags & SDI_MEDIA_STATUS_TEMP_LOW_ALARM) && (buf & SFP_TEMP_LOW_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_LOW_ALARM;
            }
            if ((flags & SDI_MEDIA_STATUS_VOLT_HIGH_ALARM) && (buf & SFP_VOLT_HIGH_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_HIGH_ALARM;
            }
            if ((flags & SDI_MEDIA_STATUS_VOLT_LOW_ALARM) && (buf & SFP_VOLT_LOW_ALARM_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_LOW_ALARM;
            }
        } else {
            return rc;
        }
        /* Get temperature and voltage warning status */
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_WARNING_STATUS_1_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_STATUS_TEMP_HIGH_WARNING) && (buf & SFP_TEMP_HIGH_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_HIGH_WARNING;
            }
            if ((flags & SDI_MEDIA_STATUS_TEMP_LOW_WARNING) && (buf & SFP_TEMP_LOW_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_TEMP_LOW_WARNING;
            }
            if ((flags & SDI_MEDIA_STATUS_VOLT_HIGH_WARNING) && (buf & SFP_VOLT_HIGH_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_HIGH_WARNING;
            }
            if ((flags & SDI_MEDIA_STATUS_VOLT_LOW_WARNING) && (buf & SFP_VOLT_LOW_WARNING_BIT)) {
                *status |= SDI_MEDIA_STATUS_VOLT_LOW_WARNING;
            }
        } else {
            return rc;
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Get the required channel monitoring(rx_power and tx_bias) alarm status of media.
 *
 * resource_hdl[in] - Handle of the resource
 * channel[in]      - channel number that is of interest, it should be '0' if
 *                    only one channel is present
 * flags[in]        - flags for channel status
 * status[out]      - returns the set of status flags which are asserted.
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_channel_monitor_status_get(sdi_resource_hdl_t resource_hdl,
                                                 uint_t             channel,
                                                 uint_t             flags,
                                                 uint_t            *status)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf;
    uint16_t                addr = 0;

    STD_ASSERT(status != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        /* Get RX power alarm and warning status */
        if ((channel == SDI_QSFP_CHANNEL1) || (channel == SDI_QSFP_CHANNEL2)) {
            addr = QSFP_RX12_POWER_INTERRUPT_ADDR;
        } else if ((channel == SDI_QSFP_CHANNEL3) || (channel == SDI_QSFP_CHANNEL4)) {
            addr = QSFP_RX34_POWER_INTERRUPT_ADDR;
        }
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, addr, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if ((flags & SDI_MEDIA_RX_PWR_HIGH_ALARM) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_RX13_POWER_HIGH_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_RX24_POWER_HIGH_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_RX13_POWER_HIGH_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_RX24_POWER_HIGH_ALARM_BIT)))) {
            *status |= SDI_MEDIA_RX_PWR_HIGH_ALARM;
        }
        if ((flags & SDI_MEDIA_RX_PWR_LOW_ALARM) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_RX13_POWER_LOW_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_RX24_POWER_LOW_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_RX13_POWER_LOW_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_RX24_POWER_LOW_ALARM_BIT)))) {
            *status |= SDI_MEDIA_RX_PWR_LOW_ALARM;
        }
        if ((flags & SDI_MEDIA_RX_PWR_HIGH_WARNING) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_RX13_POWER_HIGH_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_RX24_POWER_HIGH_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_RX13_POWER_HIGH_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_RX24_POWER_HIGH_WARNING_BIT)))) {
            *status |= SDI_MEDIA_RX_PWR_HIGH_WARNING;
        }
        if ((flags & SDI_MEDIA_RX_PWR_LOW_WARNING) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_RX13_POWER_LOW_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_RX24_POWER_LOW_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_RX13_POWER_LOW_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_RX24_POWER_LOW_WARNING_BIT)))) {
            *status |= SDI_MEDIA_RX_PWR_LOW_WARNING;
        }

        /* Get TX BIAS alarm and warning status */
        if ((channel == SDI_QSFP_CHANNEL1) || (channel == SDI_QSFP_CHANNEL2)) {
            addr = QSFP_TX12_BIAS_INTERRUPT_ADDR;
        } else if ((channel == SDI_QSFP_CHANNEL3) || (channel == SDI_QSFP_CHANNEL4)) {
            addr = QSFP_TX34_BIAS_INTERRUPT_ADDR;
        }
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, addr, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if ((flags & SDI_MEDIA_TX_BIAS_HIGH_ALARM) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_TX13_BIAS_HIGH_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_TX24_BIAS_HIGH_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_TX13_BIAS_HIGH_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_TX24_BIAS_HIGH_ALARM_BIT)))) {
            *status |= SDI_MEDIA_TX_BIAS_HIGH_ALARM;
        }
        if ((flags & SDI_MEDIA_TX_BIAS_LOW_ALARM) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_TX13_BIAS_LOW_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_TX24_BIAS_LOW_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_TX13_BIAS_LOW_ALARM_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_TX24_BIAS_LOW_ALARM_BIT)))) {
            *status |= SDI_MEDIA_TX_BIAS_LOW_ALARM;
        }
        if ((flags & SDI_MEDIA_TX_BIAS_HIGH_WARNING) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_TX13_BIAS_HIGH_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_TX24_BIAS_HIGH_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_TX13_BIAS_HIGH_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_TX24_BIAS_HIGH_WARNING_BIT)))) {
            *status |= SDI_MEDIA_TX_BIAS_HIGH_WARNING;
        }
        if ((flags & SDI_MEDIA_TX_BIAS_LOW_WARNING) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & QSFP_TX13_BIAS_LOW_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & QSFP_TX24_BIAS_LOW_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & QSFP_TX13_BIAS_LOW_WARNING_BIT)) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & QSFP_TX24_BIAS_LOW_WARNING_BIT)))) {
            *status |= SDI_MEDIA_TX_BIAS_LOW_WARNING;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        /* Get TX bias and TX power alarm  status */
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_ALARM_STATUS_1_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_TX_BIAS_HIGH_ALARM) && (buf & SFP_TX_BIAS_HIGH_ALARM_BIT)) {
                *status |= SDI_MEDIA_TX_BIAS_HIGH_ALARM;
            }
            if ((flags & SDI_MEDIA_TX_BIAS_LOW_ALARM) && (buf & SFP_TX_BIAS_LOW_ALARM_BIT)) {
                *status |= SDI_MEDIA_TX_BIAS_LOW_ALARM;
            }
            if ((flags & SDI_MEDIA_TX_PWR_HIGH_ALARM) && (buf & SFP_TX_PWR_HIGH_ALARM_BIT)) {
                *status |= SDI_MEDIA_TX_PWR_HIGH_ALARM;
            }
            if ((flags & SDI_MEDIA_TX_PWR_LOW_ALARM) && (buf & SFP_TX_PWR_LOW_ALARM_BIT)) {
                *status |= SDI_MEDIA_TX_PWR_LOW_ALARM;
            }
        } else {
            return rc;
        }

        /* Get RX power alarm status */
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_ALARM_STATUS_2_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_RX_PWR_HIGH_ALARM) && (buf & SFP_RX_PWR_HIGH_ALARM_BIT)) {
                *status |= SDI_MEDIA_RX_PWR_HIGH_ALARM;
            }
            if ((flags & SDI_MEDIA_RX_PWR_LOW_ALARM) && (buf & SFP_RX_PWR_LOW_ALARM_BIT)) {
                *status |= SDI_MEDIA_RX_PWR_LOW_ALARM;
            }
        } else {
            return rc;
        }

        /* Get TX bias and TX power warning status */
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_WARNING_STATUS_1_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_TX_BIAS_HIGH_WARNING) && (buf & SFP_TX_BIAS_HIGH_WARNING_BIT)) {
                *status |= SDI_MEDIA_TX_BIAS_HIGH_WARNING;
            }
            if ((flags & SDI_MEDIA_TX_BIAS_LOW_WARNING) && (buf & SFP_TX_BIAS_LOW_WARNING_BIT)) {
                *status |= SDI_MEDIA_TX_BIAS_LOW_WARNING;
            }
            if ((flags & SDI_MEDIA_TX_PWR_HIGH_WARNING) && (buf & SFP_TX_PWR_HIGH_WARNING_BIT)) {
                *status |= SDI_MEDIA_TX_PWR_HIGH_WARNING;
            }
            if ((flags & SDI_MEDIA_TX_PWR_LOW_WARNING) && (buf & SFP_TX_PWR_LOW_WARNING_BIT)) {
                *status |= SDI_MEDIA_TX_PWR_LOW_WARNING;
            }
        } else {
            return rc;
        }

        /* Get RX power warning status */
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_WARNING_STATUS_2_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_RX_PWR_HIGH_WARNING) && (buf & SFP_RX_PWR_HIGH_WARNING_BIT)) {
                *status |= SDI_MEDIA_RX_PWR_HIGH_WARNING;
            }
            if ((flags & SDI_MEDIA_RX_PWR_LOW_WARNING) && (buf & SFP_RX_PWR_LOW_WARNING_BIT)) {
                *status |= SDI_MEDIA_RX_PWR_LOW_WARNING;
            }
        } else {
            return rc;
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Get the required channel status of the specific media.
 *
 * resource_hdl[in] - Handle of the resource
 * channel[in]      - channel number that is of interest, it should be '0' if
 *                    only one channel is present
 * flags[in]        - flags for channel status
 * status[out]      - returns the set of status flags which are asserted.
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_channel_status_get(sdi_resource_hdl_t resource_hdl, uint_t channel, uint_t flags, uint_t *status)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf = 0;

    STD_ASSERT(status != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        /* Get TX disable status */
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_TX_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }
        if ((flags & SDI_MEDIA_STATUS_TXDISABLE) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & (0x1 << SDI_QSFP_CHANNEL1))) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & (0x1 << SDI_QSFP_CHANNEL2))) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & (0x1 << SDI_QSFP_CHANNEL3))) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & (0x1 << SDI_QSFP_CHANNEL4))))) {
            *status |= SDI_MEDIA_STATUS_TXDISABLE;
        }

        /* Get TX fault status */
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_CHANNEL_TXFAULT_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }
        if ((flags & SDI_MEDIA_STATUS_TXFAULT) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & (0x1 << SDI_QSFP_CHANNEL1))) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & (0x1 << SDI_QSFP_CHANNEL2))) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & (0x1 << SDI_QSFP_CHANNEL3))) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & (0x1 << SDI_QSFP_CHANNEL4))))) {
            *status |= SDI_MEDIA_STATUS_TXFAULT;
        }

        /* Get TXLOSS and RXLOSS status */
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_CHANNEL_LOS_INDICATOR_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }
        if ((flags & SDI_MEDIA_STATUS_TXLOSS) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & (0x10 << SDI_QSFP_CHANNEL1))) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & (0x10 << SDI_QSFP_CHANNEL2))) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & (0x10 << SDI_QSFP_CHANNEL3))) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & (0x10 << SDI_QSFP_CHANNEL4))))) {
            *status |= SDI_MEDIA_STATUS_TXLOSS;
        }
        if ((flags & SDI_MEDIA_STATUS_RXLOSS) &&
            (((channel == SDI_QSFP_CHANNEL1) && (buf & (0x1 << SDI_QSFP_CHANNEL1))) ||
             ((channel == SDI_QSFP_CHANNEL2) && (buf & (0x1 << SDI_QSFP_CHANNEL2))) ||
             ((channel == SDI_QSFP_CHANNEL3) && (buf & (0x1 << SDI_QSFP_CHANNEL3))) ||
             ((channel == SDI_QSFP_CHANNEL4) && (buf & (0x1 << SDI_QSFP_CHANNEL4))))) {
            *status |= SDI_MEDIA_STATUS_RXLOSS;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_OPTIONAL_STATUS_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if ((flags & SDI_MEDIA_STATUS_TXDISABLE) && (buf & SFP_TX_DISABLE_STATE_BIT)) {
                *status |= SDI_MEDIA_STATUS_TXDISABLE;
            }
            if ((flags & SDI_MEDIA_STATUS_TXFAULT) && (buf & SFP_TX_FAULT_STATE_BIT)) {
                *status |= SDI_MEDIA_STATUS_TXFAULT;
            }
            if ((flags & SDI_MEDIA_STATUS_RXLOSS) && (buf & SFP_RX_LOSS_STATE_BIT)) {
                *status |= SDI_MEDIA_STATUS_RXLOSS;
            }
        } else {
            return rc;
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Disable/Enable the transmitter of the specific media.
 *
 * resource_hdl[in] - handle of the media resource
 * channel[in]      - channel number that is of interest and should be 0 only
 *                    one channel is present
 * enable[in]       - "false" to disable and "true" to enable
 *
 * @return          - standard t_std_error
 */
t_std_error sdi_media_tx_control(sdi_resource_hdl_t resource_hdl, uint_t channel, bool enable)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf = 0;

    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_TX_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (enable == true) {
            buf &= ~(1 << channel);
        } else {
            buf |= (1 << channel);
        }

        rc = sdi_media_info_set(settings->module, SDI_QSFP_PAGE_0, QSFP_TX_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_OPTIONAL_STATUS_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (enable) {
            buf &= ~SFP_SOFT_TX_DISABLE_STATE_BIT;
        } else {
            buf |= SFP_SOFT_TX_DISABLE_STATE_BIT;
        }

        rc = sdi_media_info_set(settings->module, SDI_SFP_PAGE_2, SFP_OPTIONAL_STATUS_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * For getting transmitter status(enabled/disabled) on the specified channel
 *
 * resource_hdl[in] - handle of the media resource
 * channel[in]      - channel number
 * status[out]      -  transmitter status-> "true" if enabled, else "false"
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_tx_control_status_get(sdi_resource_hdl_t resource_hdl, uint_t channel, bool *status)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf = 0;

    STD_ASSERT(status != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_TX_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (((channel == SDI_QSFP_CHANNEL1) && (buf & (0x1 << SDI_QSFP_CHANNEL1))) ||
            ((channel == SDI_QSFP_CHANNEL2) && (buf & (0x1 << SDI_QSFP_CHANNEL2))) ||
            ((channel == SDI_QSFP_CHANNEL3) && (buf & (0x1 << SDI_QSFP_CHANNEL3))) ||
            ((channel == SDI_QSFP_CHANNEL4) && (buf & (0x1 << SDI_QSFP_CHANNEL4)))) {
            *status = false;
        } else {
            *status = true;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_OPTIONAL_STATUS_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc == STD_ERR_OK) {
            if (buf & SFP_TX_DISABLE_STATE_BIT) {
                *status = false;
            } else {
                *status = true;
            }
        } else {
            return rc;
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
        break;
    }

    return rc;
}

/**
 * Disable/Enable the cdr of the specific media.
 *
 * resource_hdl[in] - handle of the media resource
 * channel[in]      - channel number that is of interest and should be 0 only
 *                    one channel is present
 * enable[in]       - "false" to disable and "true" to enable
 *
 * @return          - standard t_std_error
 */
t_std_error sdi_media_cdr_status_set(sdi_resource_hdl_t resource_hdl, uint_t channel, bool enable)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf = 0;

    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_CDR_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (enable == true) {
            buf |= (0x1 << channel);
            buf |= (0x10 << channel);
        } else {
            buf &= ~(0x1 << channel);
            buf &= ~(0x10 << channel);
        }

        rc = sdi_media_info_set(settings->module, SDI_QSFP_PAGE_0, QSFP_CDR_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        return SDI_ERRCODE(EOPNOTSUPP);     /* Unsupported on SFP */

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * For getting transmitter status(enabled/disabled) on the specified channel
 *
 * resource_hdl[in] - handle of the media resource
 * channel[in]      - channel number
 * status[out]      -  transmitter status-> "true" if enabled, else "false"
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_cdr_status_get(sdi_resource_hdl_t resource_hdl, uint_t channel, bool *status)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf = 0;

    STD_ASSERT(status != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_CDR_CONTROL_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if ((buf & (0x1 << channel)) || (buf & (0x10 << channel))) {
            *status = true;
        } else {
            *status = false;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        return SDI_ERRCODE(EOPNOTSUPP);     /* Unsupported on SFP */

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Get the maximum speed that can be supported by a specific media resource
 *
 * resource_hdl[in] - handle of the media resource
 * speed[out]       - maximum speed that can be supported by media device
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_speed_get(sdi_resource_hdl_t resource_hdl, sdi_media_speed_t *speed)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;

    STD_ASSERT(speed != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
        *speed = SDI_MEDIA_SPEED_40G;
        break;

    case SDI_MEDIA_ID_TYPE_QSFP_28:
        *speed = SDI_MEDIA_SPEED_100G;
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        *speed = SDI_MEDIA_SPEED_10G;
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Checks whether the specified media is qualified by DELL or not
 *
 * resource_hdl[in] - handle of the media resource
 * status[out]      - "true" if media is qualified by DELL else "false"
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_is_dell_qualified(sdi_resource_hdl_t resource_hdl, bool *status)
{
    *status = false;

    return STD_ERR_OK;
}

/**
 * Reads the requested parameter value from eeprom
 *
 * resource_hdl[in] - handle of the media resource
 * param[in]        - parametr type that is of interest(e.g wavelength, maximum
 *                    case temperature etc)
 * value[out]       - parameter value which is read from eeprom
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_parameter_get(sdi_resource_hdl_t resource_hdl, sdi_media_param_type_t param, uint_t *value)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint32_t                buf = 0;

    STD_ASSERT(value != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, sdi_qsfp_info[param].addr,
                                sdi_qsfp_info[param].size, (uint8_t*)&buf);
        if (rc == STD_ERR_OK) {
            *value = (uint_t)buf;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_0, sdi_sfp_info[param].addr,
                                sdi_sfp_info[param].size, (uint8_t*)&buf);
        if (rc == STD_ERR_OK) {
            *value = (uint_t)buf;
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Read the requested vendor information of a specific media resource
 *
 * resource_hdl[in]     - handle of the media resource
 * vendor_info_type[in] - vendor information that is of interest.
 * vendor_info[out]     - vendor information which is read from eeprom
 * buf_size[in]         - size of the input buffer(vendor_info)
 *
 * return               - standard t_std_error
 */
t_std_error sdi_media_vendor_info_get(sdi_resource_hdl_t           resource_hdl,
                                      sdi_media_vendor_info_type_t vendor_info_type,
                                      char                        *vendor_info,
                                      size_t                       buf_size)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    size_t                  size = 0;

    STD_ASSERT(vendor_info != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    memset(vendor_info, 0, buf_size);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        size =
            (sdi_qsfp_vendor_info[vendor_info_type].size <
             buf_size) ? sdi_qsfp_vendor_info[vendor_info_type].size : buf_size;
        rc = sdi_media_info_get(settings->module,
                                SDI_QSFP_PAGE_0,
                                sdi_qsfp_vendor_info[vendor_info_type].addr,
                                size,
                                vendor_info);
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        size =
            (sdi_sfp_vendor_info[vendor_info_type].size <
             buf_size) ? sdi_sfp_vendor_info[vendor_info_type].size : buf_size;
        rc = sdi_media_info_get(settings->module,
                                SDI_SFP_PAGE_0,
                                sdi_sfp_vendor_info[vendor_info_type].addr,
                                size,
                                vendor_info);
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Read the transceiver compliance code of a specific media resource
 *
 * resource_hdl[in]     - handle of the media resource
 * transceiver_info[out]- transceiver compliance code which is read from eeprom
 *
 * return               - standard t_std_error
 */
t_std_error sdi_media_transceiver_code_get(sdi_resource_hdl_t             resource_hdl,
                                           sdi_media_transceiver_descr_t *transceiver_info)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf[SDI_MEDIA_BUF_SIZE_8] = {0};

    STD_ASSERT(transceiver_info != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    memset(transceiver_info, 0, sizeof(*transceiver_info));

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc =
            sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_COMPLIANCE_CODE_ADDR, SDI_MEDIA_BUF_SIZE_8,
                               buf);
        if (rc == STD_ERR_OK) {
            memcpy(transceiver_info, buf, sizeof(*transceiver_info));
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_0, SFP_COMPLIANCE_CODE_ADDR, SDI_MEDIA_BUF_SIZE_8, buf);
        if (rc == STD_ERR_OK) {
            memcpy(transceiver_info, buf, sizeof(*transceiver_info));
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Read the dell product information
 *
 * resource_hdl[in] - Handle of the resource
 * info[out] - dell product information
 *
 * return - standard t_std_error
 */
t_std_error sdi_media_dell_product_info_get(sdi_resource_hdl_t resource_hdl, sdi_media_dell_product_info_t *info)
{
    return STD_ERR_OK;
}

/**
 * Get the alarm and warning threshold values for a given optics
 *
 * resource_hdl[in] - Handle of the resource
 * threshold_type[in] - type of threshold
 * value[out] - threshold value
 *
 * return - standard t_std_error
 */
t_std_error sdi_media_threshold_get(sdi_resource_hdl_t         resource_hdl,
                                    sdi_media_threshold_type_t threshold_type,
                                    float                     *value)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf[SDI_MEDIA_BUF_SIZE_2];

    memset(buf, 0, sizeof(buf));

    STD_ASSERT(value != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_3, sdi_qsfp_thresholds[threshold_type].addr,
                                sdi_qsfp_thresholds[threshold_type].size, buf);
        if (rc == STD_ERR_OK) {
            *value = *((float*)buf);
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, sdi_sfp_thresholds[threshold_type].addr,
                                sdi_sfp_thresholds[threshold_type].size, buf);
        if (rc == STD_ERR_OK) {
            *value = *((float*)buf);
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Read the threshold values for module monitors like temperature and voltage
 *
 * resource_hdl[in] - Handle of the resource
 * threshold_type[in] - type of threshold
 * value[out] - threshold value
 *
 * return - standard t_std_error
 *
 * TODO: depricated API. Need to remove once upper layers adopted new api
 * sdi_media_threshold_get
 */
t_std_error sdi_media_module_monitor_threshold_get(sdi_resource_hdl_t resource_hdl,
                                                   uint_t             threshold_type,
                                                   uint_t            *value)
{
    return SDI_ERRCODE(EOPNOTSUPP);
}

/**
 * Read the threshold values for channel monitors like rx-ower and tx-bias
 *
 * resource_hdl[in] - Handle of the resource
 * threshold_type[in] - type of threshold
 * value[out] - threshold value
 *
 * return - standard t_std_error
 *
 * TODO: depricated API. Need to remove once upper layers adopted new api
 * sdi_media_threshold_get
 */
t_std_error sdi_media_channel_monitor_threshold_get(sdi_resource_hdl_t resource_hdl,
                                                    uint_t             threshold_type,
                                                    uint_t            *value)
{
    return SDI_ERRCODE(EOPNOTSUPP);
}

/**
 * Enable/Disable the module control parameters like low power mode and reset
 * control
 *
 * resource_hdl[in] - handle of the resource
 * ctrl_type[in]    - module control type(LP mode/reset)
 * enable[in]       - "true" to enable and "false" to disable
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_module_control(sdi_resource_hdl_t           resource_hdl,
                                     sdi_media_module_ctrl_type_t ctrl_type,
                                     bool                         enable)
{
    return STD_ERR_OK;
}

/**
 * Enable/Disable Auto neg on SFP PHY
 *
 * @param[in] resource_hdl - handle to media
 * @param[in] channel - channel number that is of interest. channel numbers
 * should start with 0(e.g. For qsfp valid channel number range is 0 to 3) and
 * channel number should be '0' if only one channel is present
 * @param[in] type - media type which is present in front panel port
 * @param[in] enable - true-enable Autonge,false-disable Autoneg.
 *
 * @return - standard @ref t_std_error
 *
 */
t_std_error sdi_media_phy_autoneg_set(sdi_resource_hdl_t resource_hdl,
                                      uint_t             channel,
                                      sdi_media_type_t   type,
                                      bool               enable)
{
    return STD_ERR_OK;
}

/**
 * set mode on  SFP PHY
 *
 * @param[in] resource_hdl - handle to media
 * @param[in] channel - channel number that is of interest. channel numbers
 * should start with 0(e.g. For qsfp valid channel number range is 0 to 3) and
 * channel number should be '0' if only one channel is present
 * @param[in] type - media type which is present in front panel port
 * @param[in] mode - SGMII/MII/GMII, Should be of type Refer @ref sdi_media_mode_t.
 *
 * @return - standard @ref t_std_error
 *
 */
t_std_error sdi_media_phy_mode_set(sdi_resource_hdl_t resource_hdl,
                                   uint_t             channel,
                                   sdi_media_type_t   type,
                                   sdi_media_mode_t   mode)
{
    return STD_ERR_OK;
}

/**
 * set speed on  SFP PHY
 *
 * @param[in] resource_hdl - handle to media
 * @param[in] channel - channel number that is of interest. channel numbers
 * should start with 0(e.g. For qsfp valid channel number range is 0 to 3) and
 * channel number should be '0' if only one channel is present
 * @param[in] type - media type which is present in front panel port
 * @param[in] speed - phy supported speed's. Should be of type @ref sdi_media_speed_t .
 * @count[in] count - count for number of phy supported speed's 10/100/1000.
 *
 * @return - standard @ref t_std_error
 *
 */
t_std_error sdi_media_phy_speed_set(sdi_resource_hdl_t resource_hdl,
                                    uint_t             channel,
                                    sdi_media_type_t   type,
                                    sdi_media_speed_t *speed,
                                    uint_t             count)
{
    return STD_ERR_OK;
}

/**
 * Get the status of module control parameters like low power mode and reset
 * status
 *
 * resource_hdl[in] - handle of the resource
 * ctrl_type[in]    - module control type(LP mode/reset)
 * status[out]      - "true" if enabled else "false"
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_module_control_status_get(sdi_resource_hdl_t           resource_hdl,
                                                sdi_media_module_ctrl_type_t ctrl_type,
                                                bool                        *status)
{
    return STD_ERR_OK;
}

/**
 * Retrieve module monitors assoicated with the specified media
 *
 * resource_hdl[in] - handle of the media resource
 * monitor[in]      - monitor which needs to be retrieved(TEMPERATURE/VOLTAGE)
 * value[out]       - Value of the monitor
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_module_monitor_get(sdi_resource_hdl_t         resource_hdl,
                                         sdi_media_module_monitor_t monitor,
                                         float                     *value)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf[SDI_MEDIA_BUF_SIZE_2];

    memset(buf, 0, sizeof(buf));

    STD_ASSERT(value != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        if (monitor == SDI_MEDIA_TEMP) {
            rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_TEMPERATURE_ADDR, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else if (monitor == SDI_MEDIA_VOLT) {
            rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_VOLTAGE_ADDR, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else {
            return SDI_ERRCODE(EINVAL);     /* Invalid argument */
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        if (monitor == SDI_MEDIA_TEMP) {
            rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_TEMPERATURE_ADDR, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else if (monitor == SDI_MEDIA_VOLT) {
            rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_2, SFP_VOLTAGE_ADDR, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else {
            return SDI_ERRCODE(EINVAL);         /* Invalid argument */
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Retrieve channel monitors assoicated with the specified media.
 *
 * resource_hdl[in] - handle of the media resource
 * channel[in]      - channel whose monitor has to be retreived and should be 0,
 *                    if only one channel is present.
 * monitor[in]      - monitor which needs to be retrieved.
 * value[out]       - Value of the monitor
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_channel_monitor_get(sdi_resource_hdl_t          resource_hdl,
                                          uint_t                      channel,
                                          sdi_media_channel_monitor_t monitor,
                                          float                      *value)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint16_t                addr = 0;
    uint8_t                 buf[SDI_MEDIA_BUF_SIZE_2];

    memset(buf, 0, sizeof(buf));

    STD_ASSERT(value != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        if (monitor == SDI_MEDIA_INTERNAL_RX_POWER_MONITOR) {
            if (channel == SDI_QSFP_CHANNEL1) {
                addr = QSFP_RX1_POWER_ADDR;
            } else if (channel == SDI_QSFP_CHANNEL2) {
                addr = QSFP_RX2_POWER_ADDR;
            } else if (channel == SDI_QSFP_CHANNEL3) {
                addr = QSFP_RX3_POWER_ADDR;
            } else if (channel == SDI_QSFP_CHANNEL4) {
                addr = QSFP_RX4_POWER_ADDR;
            } else {
                return SDI_ERRCODE(EINVAL);
            }
            rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, addr, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else if (monitor == SDI_MEDIA_INTERNAL_TX_BIAS_CURRENT) {
            if (channel == SDI_QSFP_CHANNEL1) {
                addr = QSFP_TX1_POWER_BIAS_ADDR;
            } else if (channel == SDI_QSFP_CHANNEL2) {
                addr = QSFP_TX2_POWER_BIAS_ADDR;
            } else if (channel == SDI_QSFP_CHANNEL3) {
                addr = QSFP_TX3_POWER_BIAS_ADDR;
            } else if (channel == SDI_QSFP_CHANNEL4) {
                addr = QSFP_TX4_POWER_BIAS_ADDR;
            } else {
                return SDI_ERRCODE(EINVAL);
            }
            rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, addr, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else if (monitor == SDI_MEDIA_INTERNAL_TX_OUTPUT_POWER) {
            return SDI_ERRCODE(EOPNOTSUPP);     /* Unsupported on QSFP */
        } else {
            return SDI_ERRCODE(EINVAL);     /* Invalid argument */
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        if (monitor == SDI_MEDIA_INTERNAL_RX_POWER_MONITOR) {
            rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_0, SFP_RX_INPUT_POWER_ADDR, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else if (monitor == SDI_MEDIA_INTERNAL_TX_BIAS_CURRENT) {
            rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_0, SFP_TX_BIAS_CURRENT_ADDR, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else if (SDI_MEDIA_INTERNAL_TX_OUTPUT_POWER) {
            rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_0, SFP_TX_OUTPUT_POWER_ADDR, sizeof(buf), buf);
            if (rc == STD_ERR_OK) {
                *value = *((float*)buf);
            }
        } else {
            return SDI_ERRCODE(EINVAL);         /* Invalid argument */
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Read data from media
 *
 * resource_hdl[in] - handle of the media resource
 * offset[in]       - offset from which data to be read
 * data[out]        - buffer for data to be read
 * data_len[in]     - length of the data to be read
 *
 * return           - standard t_std_error
 */
t_std_error sdi_media_read(sdi_resource_hdl_t resource_hdl, uint_t offset, uint8_t *data, size_t data_len)
{
    return STD_ERR_UNIMPLEMENTED;
}

/**
 * Write data to media
 *
 * resource_hdl[in] - handle of the media resource
 * offset[in]       - offset to which data to be write
 * data[in]         - input buffer which contains data to be written
 * data_len[in]     - length of the data to be write
 *
 * return standard t_std_error
 */
t_std_error sdi_media_write(sdi_resource_hdl_t resource_hdl, uint_t offset, uint8_t *data, size_t data_len)
{
    return STD_ERR_UNIMPLEMENTED;
}

/**
 * Get the optional feature support status on a given optics
 *
 * resource_hdl[in] - handle of the media resource
 * feature_support[out] - feature support flags. Flag will be set to "true" if
 * feature is supported else "false"
 *
 * return - standard t_std_error
 */
t_std_error sdi_media_feature_support_status_get(sdi_resource_hdl_t             resource_hdl,
                                                 sdi_media_supported_feature_t *feature_support)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_media_settings_t   *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    uint32_t                identifier_type = 0;
    uint8_t                 buf = 0;

    memset(feature_support, 0, sizeof(*feature_support));

    STD_ASSERT(feature_support != NULL);
    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_media_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_MEDIA) {
        return SDI_ERRCODE(EPERM);
    }

    if ((rc = sdi_media_identifier_get(settings->module, &identifier_type)) != STD_ERR_OK) {
        return rc;
    }

    switch (identifier_type) {
    case SDI_MEDIA_ID_TYPE_QSFP:
    case SDI_MEDIA_ID_TYPE_QSFP_PLUS:
    case SDI_MEDIA_ID_TYPE_QSFP_28:
        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_STATUS_INDICATOR_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (buf & QSFP_FLAT_MEM_BIT) {
            feature_support->qsfp_features.paging_support_status = true;
        }

        rc = sdi_media_info_get(settings->module, SDI_QSFP_PAGE_0, QSFP_OPTIONS4_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (buf & QSFP_TX_DISABLE_BIT) {
            feature_support->qsfp_features.tx_control_support_status = true;
        }
        if (buf & QSFP_RATE_SELECT_BIT) {
            feature_support->qsfp_features.rate_select_status = true;
        }
        break;

    case SDI_MEDIA_ID_TYPE_SFP:
        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_0, SFP_ENHANCED_OPTIONS_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (buf & SFP_ALARM_SUPPORT_BIT) {
            feature_support->sfp_features.alarm_support_status = true;
        }
        if (buf & SFP_RATE_SELECT_BIT) {
            feature_support->sfp_features.rate_select_status = true;
        }

        rc = sdi_media_info_get(settings->module, SDI_SFP_PAGE_0, SFP_DIAG_MON_TYPE_ADDR, sizeof(buf), &buf);
        if (rc != STD_ERR_OK) {
            return rc;
        }

        if (buf & SFP_DIAG_MON_SUPPORT_BIT) {
            feature_support->sfp_features.diag_mntr_support_status = true;
        }
        break;

    default:
        SDI_ERRMSG_LOG("Invalid identifier type %x of media module %u.", identifier_type, settings->module);
        return SDI_ERRCODE(-1);
    }

    return rc;
}

/**
 * Set the port LED based on the speed settings of the port.
 *
 * resource_hdl[in] - Handle of the resource
 * channel[in] - Channel number. Should be 0, if only one channel is present
 * speed[in] - LED mode setting is derived from speed
 *
 * return - standard t_std_error
 */
t_std_error sdi_media_led_set(sdi_resource_hdl_t resource_hdl, uint_t channel, sdi_media_speed_t speed)
{
    return SDI_ERRCODE(EOPNOTSUPP);
}

/*
 * @brief initialize plugedin  module
 *
 * @param[in] resource_hdl - handle to the qsfp
 * @pres[in]      - presence status
 *
 * @return - standard @ref t_std_error
 */
t_std_error sdi_media_module_init(sdi_resource_hdl_t resource_hdl, bool pres)
{
    return STD_ERR_OK;
}

/*
 * @brief Set wavelength for tunable media
 *
 * @param[in]  - resource_hdl - handle to the front panel port
 * @param[in]  - wavelength value
 */
t_std_error sdi_media_wavelength_set(sdi_resource_hdl_t resource_hdl, float value)
{
    return SDI_ERRCODE(EOPNOTSUPP);
}
