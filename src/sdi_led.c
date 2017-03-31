/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 *  Implementation of LED resource API.
 ***************************************************************************************/

#include "sdi_led.h"
#include "sdi_common.h"
#include "sdi_sysfs_utils.h"


/**
 * @struct sdi_led_settings_t
 * Used to hold LED related settings.
 */
typedef struct sdi_led_settings_s {
    char sysfs_name[SDI_MAX_NAME_LEN]; /**< SysFs name of the LED */
    char sysfs_path[PATH_MAX];         /**< SysFs path of the LED */
    char state_off[SDI_MAX_NAME_LEN];  /**< SysFs value for the LED's "off" state */
    char state_on[SDI_MAX_NAME_LEN];   /**< SysFs value for the LED's "on" state */
} sdi_led_settings_t;

/**
 * Registers settings for the specified LED resource.
 *
 * hdl[in] - handle of the resource.
 * root[in] - config node for the resource settings.
 *
 * return None.
 */
void sdi_led_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t led_node)
{
    char               *name = NULL;
    char               *path = NULL;
    char               *state_off = NULL;
    char               *state_on = NULL;
    std_config_node_t   state_node = NULL;
    sdi_led_settings_t *settings = NULL;

    memset(&settings, 0, sizeof(settings));

    STD_ASSERT((name = std_config_attr_get(led_node, "name")) != NULL);
    STD_ASSERT((path = std_config_attr_get(led_node, "path")) != NULL);

    state_node = std_config_get_child(led_node);

    STD_ASSERT((state_off = std_config_attr_get(state_node, "off")) != NULL);
    STD_ASSERT((state_on = std_config_attr_get(state_node, "on")) != NULL);

    settings = (sdi_led_settings_t*)calloc(1, sizeof(sdi_led_settings_t));
    STD_ASSERT(settings != NULL);

    strncpy(settings->sysfs_name, name, sizeof(settings->sysfs_name));
    strncpy(settings->sysfs_path, path, sizeof(settings->sysfs_path));
    strncpy(settings->state_off, state_off, sizeof(settings->state_off));
    strncpy(settings->state_on, state_on, sizeof(settings->state_on));

    hdl->settings = (void*)settings;
}

/**
 * Turn-on the specified LED
 *
 * resource_hdl[in] - Handle of the Resource
 *
 * return t_std_error
 */
t_std_error sdi_led_on(sdi_resource_hdl_t resource_hdl)
{
    sdi_resource_priv_hdl_t hdl = NULL;
    sdi_led_settings_t     *settings = NULL;

    STD_ASSERT((hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_led_settings_t*)hdl->settings) != NULL);

    if (hdl->type != SDI_RESOURCE_LED) {
        return SDI_ERRCODE(EPERM);
    }

    return sdi_sysfs_attr_str_set(settings->sysfs_path, settings->sysfs_name, settings->state_on);
}

/**
 * Turn-off the specified LED
 *
 * resource_hdl[in] - Handle of the resource
 *
 * return t_std_error
 */
t_std_error sdi_led_off(sdi_resource_hdl_t resource_hdl)
{
    sdi_resource_priv_hdl_t hdl = NULL;
    sdi_led_settings_t     *settings = NULL;

    STD_ASSERT((hdl = (sdi_resource_priv_hdl_t)resource_hdl) != NULL);
    STD_ASSERT((settings = (sdi_led_settings_t*)hdl->settings) != NULL);

    if (hdl->type != SDI_RESOURCE_LED) {
        return SDI_ERRCODE(EPERM);
    }

    return sdi_sysfs_attr_str_set(settings->sysfs_path, settings->sysfs_name, settings->state_off);
}

/**
 * Turn-on the digital display LED
 *
 * resource_hdl[in] - Handle of the LED
 *
 * return t_std_error
 */
t_std_error sdi_digital_display_led_on(sdi_resource_hdl_t resource_hdl)
{
    return SDI_ERRCODE(EPERM);
}

/**
 * Turn-off the digital display LED
 *
 * resource_hdl[in] - Handle of the LED
 *
 * return t_std_error
 */
t_std_error sdi_digital_display_led_off(sdi_resource_hdl_t resource_hdl)
{
    return SDI_ERRCODE(EPERM);
}

/**
 * Sets the specified value in the digital_display_led.
 *
 * hdl[in]           : Handle of the resource
 * display_string[in]: Value to be displayed
 *
 * return t_std_error
 */
t_std_error sdi_digital_display_led_set(sdi_resource_hdl_t hdl, const char *display_string)
{
    return SDI_ERRCODE(EPERM);
}
