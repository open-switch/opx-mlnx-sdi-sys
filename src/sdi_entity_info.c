/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * brief   Entity Info Functionality provider.
 *         Currently entity_info providing support for read operations.
 ***************************************************************************************/

#include "sdi_common.h"
#include "sdi_entity_info.h"
#include "sdi_eeprom_utils.h"
#include <string.h>


/**
 * @struct sdi_info_settings_t
 * Used to hold info resource related settings.
 */
typedef struct sdi_info_settings_s {
    char                  path[PATH_MAX]; /**< path to the EEPROM SysFs attribute */
    char                  name[SDI_MAX_NAME_LEN]; /**< name of the EEPROM SysFs attribute */
    sdi_eeprom_type_t     type;       /**< type of the EEPROM raw data */
    sdi_entity_priv_hdl_t entity_hdl; /**< handle of the entity, to which this info resource belongs */
} sdi_info_settings_t;

/**
 * Registers settings for the specified EEPROM info resource.
 *
 * hdl[in] - handle of the resource.
 * entity_hdl[in] - handle of the entity, to which info resource belongs.
 * node[in] - config node for the info resource settings.
 *
 * return None.
 */
void sdi_info_register_settings(sdi_resource_priv_hdl_t hdl,
                                sdi_entity_priv_hdl_t   entity_hdl,
                                std_config_node_t       info_node)
{
    char                *name = NULL;
    char                *path = NULL;
    char                *type = NULL;
    sdi_info_settings_t *settings = NULL;

    STD_ASSERT(hdl != NULL);
    STD_ASSERT(entity_hdl != NULL);
    STD_ASSERT(info_node != NULL);

    STD_ASSERT((name = std_config_attr_get(info_node, "name")) != NULL);
    STD_ASSERT((path = std_config_attr_get(info_node, "path")) != NULL);
    STD_ASSERT((type = std_config_attr_get(info_node, "type")) != NULL);

    settings = (sdi_info_settings_t*)calloc(1, sizeof(sdi_info_settings_t));
    STD_ASSERT(settings != NULL);

    strncpy(settings->name, name, sizeof(settings->name));
    strncpy(settings->path, path, sizeof(settings->path));
    settings->type = sdi_eeprom_string_to_type(type);
    settings->entity_hdl = entity_hdl;

    hdl->settings = (void*)settings;
}

/**
 * Calculates the maximum speed for specific fan-tray.
 *
 * This function should be called for each fan in fan-tray.
 * It takes maximum speed for previous fan and reads for current one.
 * Then the less value is returned as new maximum fan speed.
 *
 * hdl[in] - handle of the resource.
 * data[in/out] - calculated maximum speed.
 *
 * return none.
 *
 * note
 */
static void sdi_fan_max_speed(sdi_resource_hdl_t hdl, void *data)
{
    uint_t speed = 0;
    uint_t max_speed = 0;

    if ((hdl == NULL) || (data == NULL)) {
        return;
    }

    max_speed = *((uint_t*)data);

    if (sdi_resource_type_get(hdl) == SDI_RESOURCE_FAN) {
        if (sdi_fan_max_speed_get(hdl, &speed) != STD_ERR_OK) {
            return;
        }

        if ((max_speed == 0) || (speed < max_speed)) {
            *((uint_t*)data) = speed;
        }
    }
}

/**
 * Fills the "info" structure for the specified fan tray.
 *
 * hdl[in] - handle of the fan tray entity.
 * info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
static t_std_error sdi_fan_info_fill(sdi_entity_priv_hdl_t hdl, sdi_entity_info_t *info)
{
    uint_t fan_num = 0;
    uint_t fan_max_speed = 0;

    if ((hdl == NULL) || (info == NULL)) {
        return SDI_ERRCODE(EINVAL);
    }

    fan_num = sdi_entity_resource_count_get((sdi_entity_hdl_t)hdl, SDI_RESOURCE_FAN);
    sdi_entity_for_each_resource((sdi_entity_hdl_t)hdl, sdi_fan_max_speed, &fan_max_speed);

    if ((fan_num == 0) || (fan_max_speed == 0)) {
        return SDI_ERRCODE(-1);
    }

    if (info->num_fans != fan_num) {
        info->num_fans = fan_num;
    }

    if (info->max_speed != fan_max_speed) {
        info->max_speed = fan_max_speed;
    }

    return STD_ERR_OK;
}

/**
 * Fills the "info" structure for the specified PSU tray.
 *
 * hdl[in] - handle of the PSU tray entity.
 * info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
static t_std_error sdi_psu_info_fill(sdi_entity_priv_hdl_t hdl, sdi_entity_info_t *info)
{
    const uint_t volt_divider = 1000; /* divider for converting millivolts to volts */
    t_std_error  rc = STD_ERR_OK;
    uint_t       power_rating = 0;
    uint_t       fan_max_speed = 0;

    STD_ASSERT(hdl != NULL);
    STD_ASSERT(info != NULL);

    if (hdl->type != SDI_ENTITY_PSU_TRAY) {
        return SDI_ERRCODE(EPERM);
    }

    info->power_type = hdl->power.type;
    info->num_fans = sdi_entity_resource_count_get((sdi_entity_hdl_t)hdl, SDI_RESOURCE_FAN);
    sdi_entity_for_each_resource((sdi_entity_hdl_t)hdl, sdi_fan_max_speed, &fan_max_speed);
    info->max_speed = fan_max_speed;

    rc = sdi_sysfs_attr_uint_get(hdl->power.rating_path, hdl->power.rating_name, &power_rating);
    if (rc == STD_ERR_OK) {
        info->power_rating = power_rating / volt_divider;
    }

    return rc;
}

/**
 * Get the "info" structure for the entity.
 *
 * settings[in] - settings info for the entity.
 * info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
static t_std_error sdi_entity_info_get(sdi_info_settings_t *settings, sdi_entity_info_t *info)
{
    char       *buf = NULL;
    size_t      buf_size = 0;
    t_std_error rc = STD_ERR_OK;

    if ((settings == NULL) || (info == NULL)) {
        return SDI_ERRCODE(EINVAL);
    }

    /* Get the size of the EEPROM raw data */
    rc = sdi_sysfs_attr_data_size_get(settings->path, settings->name, &buf_size);
    if ((rc != STD_ERR_OK) || (buf_size == 0)) {
        SDI_ERRMSG_LOG("%s:%d Cannot get size of EEPROM raw data (error:%d).",
                       __FUNCTION__, __LINE__, rc);
        return SDI_ERRCODE(-1);
    }

    buf = (char*)calloc(buf_size, sizeof(*buf));
    if (buf == NULL) {
        return SDI_ERRCODE(ENOMEM);
    }

    /* Read raw data from the EEPROM */
    rc = sdi_sysfs_attr_data_get(settings->path, settings->name, buf_size, buf);
    if (rc != STD_ERR_OK) {
        free(buf);
        SDI_ERRMSG_LOG("%s:%d Cannot read EEPROM raw data (error:%d).", __FUNCTION__, __LINE__, rc);
        return SDI_ERRCODE(rc);
    }

    memset(info, 0, sizeof(*info));

    /* Parse EEPROM raw data and fill in the entity info structure */
    switch (settings->type) {
    case SDI_EEPROM_SYS_ONIE:
        rc = sdi_eeprom_sys_onie_get(buf, buf_size, info);
        break;

    case SDI_EEPROM_FAN_MLNX:
        rc = sdi_eeprom_fan_mlnx_get(buf, buf_size, info);
        break;

    case SDI_EEPROM_PSU_MLNX:
        rc = sdi_eeprom_psu_mlnx_get(buf, buf_size, info);
        break;

    default:
        rc = SDI_ERRCODE(EOPNOTSUPP);
    }

    free(buf);
    return rc;
}

/**
 * Fills the "info" structure for the entity.
 *
 * This function should be called only for present entities.
 *
 * settings[in] - settings info for the entity.
 * info[out] - entity_info structure to fill.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
static t_std_error sdi_entity_info_fill(sdi_info_settings_t *settings, sdi_entity_info_t *info)
{
    t_std_error           rc = STD_ERR_OK;
    sdi_entity_priv_hdl_t hdl = NULL;
    sdi_entity_info_t     tmp_info;

    memset(&tmp_info, 0, sizeof(tmp_info));

    if ((settings == NULL) || (info == NULL)) {
        return SDI_ERRCODE(EINVAL);
    }

    if ((rc = sdi_entity_info_get(settings, info)) != STD_ERR_OK) {
        return SDI_ERRCODE(rc);
    }

    /* Get vendor, platform names and service tag from the system board info,
     *  since these fields are common and only system board contains them. */
    if (settings->entity_hdl->type != SDI_ENTITY_SYSTEM_BOARD) {
        hdl = (sdi_entity_priv_hdl_t)sdi_entity_lookup(SDI_ENTITY_SYSTEM_BOARD, 1);
        rc = sdi_entity_info_read(hdl->entity_info_hdl, &tmp_info);
        if ((hdl != NULL) && (rc == STD_ERR_OK)) {
            strncpy(info->vendor_name, tmp_info.vendor_name, sizeof(info->vendor_name));
            strncpy(info->service_tag, tmp_info.service_tag, sizeof(info->service_tag));
            strncpy(info->platform_name, tmp_info.platform_name, sizeof(info->platform_name));
        }
    }

    if (settings->entity_hdl->type == SDI_ENTITY_FAN_TRAY) {
        /* Get number of fans and max speed for the fan tray. */
        rc = sdi_fan_info_fill(settings->entity_hdl, info);
    } else if (settings->entity_hdl->type == SDI_ENTITY_PSU_TRAY) {
        /* Get info for the PSU tray. */
        rc = sdi_psu_info_fill(settings->entity_hdl, info);
    }

    return rc;
}

/**
 * brief - Read the entity info.
 *
 * param[in]   resource_hdl - resource .
 * param[out] entity_info - info to fill.
 *
 * return STD_ERR_OK for success and the respective error code in case of failure.
 */
t_std_error sdi_entity_info_read(sdi_resource_hdl_t resource_hdl, sdi_entity_info_t *entity_info)
{
    sdi_resource_priv_hdl_t hdl = NULL;
    sdi_info_settings_t    *settings = NULL;

    STD_ASSERT((hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_info_settings_t*)hdl->settings) != NULL);

    if (hdl->type != SDI_RESOURCE_ENTITY_INFO) {
        return SDI_ERRCODE(EPERM);
    }

    return sdi_entity_info_fill(settings, entity_info);
}
