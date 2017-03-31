/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * sdi_thermal.c
 * API implementation for thermal related functionalities
 ***************************************************************************************/

#include "sdi_thermal.h"
#include "sdi_common.h"

#define TEMP_THRESH_UNSUP INT_MIN /**< value, which specifies that threshold is unsupported */
#define DEGREE_DIVIDER    1000 /* divider to convert millidegrees to degrees (Celsius) */

/**
 * @struct sdi_temp_settings_t
 * Used to hold settings for the thermal sensor resource.
 */
typedef struct sdi_temp_settings_s {
    char name[SDI_MAX_NAME_LEN]; /**< name of the temperature SysFs attribute */
    char path[PATH_MAX];         /**< path to the temperature SysFs attribute */
    int  low_thresh;             /**< low threshold for the thermal sensor */
    int  high_thresh;            /**< high threshold for the thermal sensor */
} sdi_temp_settings_t;

/**
 * Registers settings for the specified thermal sensor resource.
 *
 * hdl[in] - handle of the resource.
 * temp_node[in] - config node for the resource settings.
 *
 * return None.
 */
void sdi_temp_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t temp_node)
{
    char                *name = NULL;
    char                *path = NULL;
    char                *attr = NULL;
    std_config_node_t    thresholds_node = NULL;
    sdi_temp_settings_t *settings = NULL;

    memset(&settings, 0, sizeof(settings));

    STD_ASSERT((name = std_config_attr_get(temp_node, "name")) != NULL);
    STD_ASSERT((path = std_config_attr_get(temp_node, "path")) != NULL);

    thresholds_node = std_config_get_child(temp_node);

    settings = (sdi_temp_settings_t*)calloc(1, sizeof(sdi_temp_settings_t));
    STD_ASSERT(settings != NULL);

    strncpy(settings->name, name, sizeof(settings->name));
    strncpy(settings->path, path, sizeof(settings->path));

    if (thresholds_node != NULL) {
        if ((attr = std_config_attr_get(thresholds_node, "low")) != NULL) {
            settings->low_thresh = atoi(attr);
        }

        if ((attr = std_config_attr_get(thresholds_node, "high")) != NULL) {
            settings->high_thresh = atoi(attr);
        }
    } else {
        settings->low_thresh = TEMP_THRESH_UNSUP;
        settings->high_thresh = TEMP_THRESH_UNSUP;
    }

    hdl->settings = (void*)settings;
}

/*
 * API implementation to retrieve the temperature of the chip refered by resource.
 *
 * resource_hdl[in] - resource handle of the chip
 * temp[out] - temperature value is returned in this
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_temperature_get(sdi_resource_hdl_t resource_hdl, int *temp)
{
    t_std_error             rc = STD_ERR_OK;
    sdi_resource_priv_hdl_t hdl = NULL;
    sdi_temp_settings_t    *settings = NULL;

    STD_ASSERT((hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_temp_settings_t*)hdl->settings) != NULL);
    STD_ASSERT(temp != NULL);

    if (hdl->type != SDI_RESOURCE_TEMPERATURE) {
        return SDI_ERRCODE(EPERM);
    }


    if ((rc = sdi_sysfs_attr_int_get(settings->path, settings->name, temp)) == STD_ERR_OK) {
        *temp /= DEGREE_DIVIDER;
    }

    return rc;
}

/*
 * API implementation to retrieve the temperature thresholds of the chip refered by resource.
 *
 * [in] resource_hdl - resource handle of the chip.
 * [in] threshold_type - type of the threshold.
 * [out] val - threshold value is returned in this
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_temperature_threshold_get(sdi_resource_hdl_t resource_hdl, sdi_threshold_t threshold_type,  int *val)
{
    sdi_resource_priv_hdl_t hdl = NULL;
    sdi_temp_settings_t    *settings = NULL;

    STD_ASSERT((hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_temp_settings_t*)hdl->settings) != NULL);
    STD_ASSERT(val != NULL);

    if (hdl->type != SDI_RESOURCE_TEMPERATURE) {
        return SDI_ERRCODE(EPERM);
    }

    switch (threshold_type) {
    case SDI_LOW_THRESHOLD:
        if (settings->low_thresh == TEMP_THRESH_UNSUP) {
            return SDI_ERRCODE(EOPNOTSUPP);
        }
        *val = settings->low_thresh;
        break;

    case SDI_HIGH_THRESHOLD:
        if (settings->low_thresh == TEMP_THRESH_UNSUP) {
            return SDI_ERRCODE(EOPNOTSUPP);
        }
        *val = settings->high_thresh;
        break;

    default:
        return SDI_ERRCODE(EPERM);
    }

    return STD_ERR_OK;
}

/*
 * API implementation to set the temperature thresholds of the chip refered by resource.
 *
 * resource_hdl[in] - resource handle of the chip.
 * threshold_type[in] - type of the threshold.
 * val[in] - threshold value is returned in this.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_temperature_threshold_set(sdi_resource_hdl_t resource_hdl, sdi_threshold_t threshold_type, int val)
{
    sdi_resource_priv_hdl_t hdl = NULL;
    sdi_temp_settings_t    *settings = NULL;

    STD_ASSERT((hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_temp_settings_t*)hdl->settings) != NULL);

    if (hdl->type != SDI_RESOURCE_TEMPERATURE) {
        return SDI_ERRCODE(EPERM);
    }

    switch (threshold_type) {
    case SDI_LOW_THRESHOLD:
        if (settings->low_thresh == TEMP_THRESH_UNSUP) {
            return SDI_ERRCODE(EOPNOTSUPP);
        }
        settings->low_thresh = val;
        break;

    case SDI_HIGH_THRESHOLD:
        if (settings->low_thresh == TEMP_THRESH_UNSUP) {
            return SDI_ERRCODE(EOPNOTSUPP);
        }
        settings->high_thresh = val;
        break;

    default:
        return SDI_ERRCODE(EPERM);
    }

    return STD_ERR_OK;
}

/*
 * API implementation to retrieve the fault status of the chip refered by resource.
 *
 * resource_hdl[in] - resource handle of the chip.
 * alert_on[out] - fault status is returned in this.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_temperature_status_get(sdi_resource_hdl_t resource_hdl, bool *alert_on)
{
    sdi_resource_priv_hdl_t hdl = NULL;
    sdi_temp_settings_t    *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    int                     temp = 0;

    STD_ASSERT((hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_temp_settings_t*)hdl->settings) != NULL);
    STD_ASSERT(alert_on != NULL);

    if (hdl->type != SDI_RESOURCE_TEMPERATURE) {
        return SDI_ERRCODE(EPERM);
    }

    *alert_on = false;

    if ((rc = sdi_sysfs_attr_int_get(settings->path, settings->name, &temp)) == STD_ERR_OK) {
        if (((settings->low_thresh != TEMP_THRESH_UNSUP) && (temp < settings->low_thresh)) ||
            ((settings->high_thresh != TEMP_THRESH_UNSUP) && (temp < settings->high_thresh))) {
            *alert_on = true;
        }
    }

    return rc;
}
