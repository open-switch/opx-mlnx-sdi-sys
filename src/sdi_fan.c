/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * sdi_fan.c
 * API Implementation for FAN resource related functionalities.
 ***************************************************************************************/

#include "sdi_fan.h"
#include "sdi_common.h"
#include "sdi_sysfs_utils.h"


/**
 * @struct sdi_fan_speed_t
 * Used to hold settings for the "fan speed" SysFs attribute.
 */
typedef struct sdi_fan_speed_s {
    char   set[SDI_MAX_NAME_LEN];   /**< name of the fan "set speed" SysFs attribute */
    char   get[SDI_MAX_NAME_LEN];   /**< name of the fan "get speed" SysFs attribute */
    char   max_get[SDI_MAX_NAME_LEN]; /**< name of the fan "get max speed" SysFs attribute */
    uint_t max_pwm;                 /**< maximum speed value in PWM format */
    uint_t max_rpm;                 /**< maximum speed value in RPM format */
} sdi_fan_speed_t;

/**
 * @struct sdi_fan_status_t
 * Used to hold settings for the fan "fault status" SysFs attribute.
 */
typedef struct sdi_fan_status_s {
    char get[SDI_MAX_NAME_LEN];   /**< name of the fan "fault status" SysFs attribute */
    char fault[SDI_MAX_NAME_LEN]; /**< value of the "Fault" status */
} sdi_fan_status_t;

/**
 * @struct sdi_fan_settings_t
 * Used to hold FAN related settings.
 */
typedef struct sdi_fan_settings_s {
    char             name[SDI_MAX_NAME_LEN]; /**< name of the fan SysFs attribute */
    char             path[PATH_MAX]; /**< path to the fan SysFs attributes */
    sdi_fan_speed_t  speed;      /**< settings for the fan speed SysFs attributes */
    sdi_fan_status_t status;     /**< settings for the fan fault status SysFs attribute */
} sdi_fan_settings_t;

/**
 * Registers settings for the specified FAN resource.
 *
 * hdl[in] - handle of the resource.
 * fan_node[in] - config node for the FAN resource settings.
 *
 * return None.
 */
void sdi_fan_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t fan_node)
{
    char               *name = NULL;
    char               *path = NULL;
    char               *type = NULL;
    char               *attr = NULL;
    std_config_node_t   node = NULL;
    sdi_fan_settings_t *settings = NULL;

    STD_ASSERT(hdl != NULL);
    STD_ASSERT(fan_node != NULL);

    STD_ASSERT((name = std_config_attr_get(fan_node, "name")) != NULL);
    STD_ASSERT((path = std_config_attr_get(fan_node, "path")) != NULL);

    settings = (sdi_fan_settings_t*)calloc(1, sizeof(sdi_fan_settings_t));
    STD_ASSERT(settings != NULL);

    strncpy(settings->name, name, sizeof(settings->name));
    strncpy(settings->path, path, sizeof(settings->path));

    for (node = std_config_get_child(fan_node); (node != NULL); node = std_config_next_node(node)) {
        if (strncmp(std_config_name_get(node), "speed", sizeof("speed")) == 0) {
            if ((attr = std_config_attr_get(node, "set")) != NULL) {
                strncpy(settings->speed.set, attr, sizeof(settings->speed.set));
            }

            if ((attr = std_config_attr_get(node, "get")) != NULL) {
                strncpy(settings->speed.get, attr, sizeof(settings->speed.get));
            }

            if ((attr = std_config_attr_get(node, "max_get")) != NULL) {
                strncpy(settings->speed.max_get, attr, sizeof(settings->speed.max_get));
            }

            if ((attr = std_config_attr_get(node, "max_pwm")) != NULL) {
                settings->speed.max_pwm = atoi(attr);
            }

            if ((attr = std_config_attr_get(node, "max_rpm")) != NULL) {
                settings->speed.max_rpm = atoi(attr);
            }
        } else if (strncmp(std_config_name_get(node), "status", sizeof("status")) == 0) {
            if ((attr = std_config_attr_get(node, "get")) != NULL) {
                strncpy(settings->status.get, attr, sizeof(settings->status.get));
            }

            if ((attr = std_config_attr_get(node, "fault")) != NULL) {
                strncpy(settings->status.fault, attr, sizeof(settings->status.fault));
            }
        }
    }

    hdl->settings = (void*)settings;
}

/*
 * Gets the maximum speed of the fan referred by resource.
 *
 * [in] hdl - resource handle of the fan
 * [out] speed - maximum speed(in RPM)
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_fan_max_speed_get(sdi_resource_hdl_t hdl, uint_t *max_speed)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_fan_settings_t     *settings = NULL;

    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)hdl) != NULL);
    STD_ASSERT((settings = (sdi_fan_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_FAN) {
        return SDI_ERRCODE(EPERM);
    }

    if (strlen(settings->speed.max_get) == 0) {
        if (settings->speed.max_rpm > 0) {
            *max_speed = settings->speed.max_rpm;
        } else {
            return SDI_ERRCODE(EPERM);
        }
    }

    return sdi_sysfs_attr_uint_get(settings->path, settings->speed.max_get, max_speed);
}

/*
 * API implementation to retrieve the speed of the fan referred by resource.
 *
 * [in] hdl - resource handle of the fan
 * [out] speed - speed(in RPM) is returned in this
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_fan_speed_get(sdi_resource_hdl_t hdl, uint_t *speed)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_fan_settings_t     *settings = NULL;

    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)hdl) != NULL);
    STD_ASSERT((settings = (sdi_fan_settings_t*)priv_hdl->settings) != NULL);

    if ((priv_hdl->type != SDI_RESOURCE_FAN) || (strlen(settings->speed.get) == 0)) {
        return SDI_ERRCODE(EPERM);
    }

    return sdi_sysfs_attr_uint_get(settings->path, settings->speed.get, speed);
}

/*
 * API implementation to set the speed of the fan(in RPM) refered by resource.
 *
 * [in] hdl - resource handle of the fan
 * [in] speed - speed(in RPM) to be set
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_fan_speed_set(sdi_resource_hdl_t hdl, uint_t speed)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_fan_settings_t     *settings = NULL;
    const uint_t            percent = 100;
    uint_t                  pwm_speed = 0;
    uint_t                  max_speed = 0;

    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)hdl) != NULL);
    STD_ASSERT((settings = (sdi_fan_settings_t*)priv_hdl->settings) != NULL);

    if (priv_hdl->type != SDI_RESOURCE_FAN) {
        return SDI_ERRCODE(EPERM);
    }

    if ((strlen(settings->speed.max_get) == 0) || (strlen(settings->speed.set) == 0)) {
        return SDI_ERRCODE(EPERM);
    }

    if (sdi_sysfs_attr_uint_get(settings->path, settings->speed.max_get, &max_speed) != STD_ERR_OK) {
        return SDI_ERRCODE(EPERM);
    }

    pwm_speed = settings->speed.max_pwm * (speed * percent / max_speed) / percent;

    return sdi_sysfs_attr_uint_set(settings->path, settings->speed.set, pwm_speed);
}

/*
 * API implementation to retrieve the fault status of the fan refered by resource.
 *
 * [in] hdl - resource handle of the fan
 * [out] status - fan's fault status is returned in this
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_fan_status_get(sdi_resource_hdl_t hdl, bool *status)
{
    sdi_resource_priv_hdl_t priv_hdl = NULL;
    sdi_fan_settings_t     *settings = NULL;
    t_std_error             rc = STD_ERR_OK;
    char                    tmp_status[SDI_MAX_NAME_LEN] = {0};

    STD_ASSERT((priv_hdl = (sdi_resource_priv_hdl_t)hdl) != NULL);
    STD_ASSERT((settings = (sdi_fan_settings_t*)priv_hdl->settings) != NULL);

    if ((priv_hdl->type != SDI_RESOURCE_FAN) || (strlen(settings->status.get) == 0)) {
        return SDI_ERRCODE(EPERM);
    }

    *status = true;

    rc = sdi_sysfs_attr_str_get(settings->path, settings->status.get, tmp_status);
    if (rc == STD_ERR_OK) {
        if (strncmp(settings->status.fault, tmp_status, sizeof(settings->status.fault)) != 0) {
            *status = false;
        }
    }

    return rc;
}
