/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * Implementation of entity reset and power status control APIs.
 ***************************************************************************************/

#include "sdi_entity.h"
#include "sdi_common.h"
#include "std_assert.h"

/**
 * Reset the specified entity.
 * Reset of entity results in reset of resources and devices as per the reset type.
 * Upon reset, default configurations as specified for platform would be applied.
 * param[in] hdl - handle to the entity whose information has to be retrieved.
 * param[in] type - type of reset to perform.
 * return STD_ERR_OK on success and standard error on failure
 */
t_std_error sdi_entity_reset(sdi_entity_hdl_t hdl, sdi_reset_type_t type)
{
    t_std_error           rc = STD_ERR_OK;
    sdi_entity_priv_hdl_t entity_priv_hdl = NULL;

    STD_ASSERT((entity_priv_hdl = (sdi_entity_priv_hdl_t)hdl) != NULL);

    /* Check this entity supports reset for this type */
    /* Mlnx switches support only cold reset type */
    if ((type != COLD_RESET) || (strlen(entity_priv_hdl->power_ctl.reset) <= 0)) {
        return SDI_ERRCODE(EOPNOTSUPP);
    }

    /* Perform the entity reset */
    rc = sdi_sysfs_attr_uint_set(entity_priv_hdl->power_ctl.path, entity_priv_hdl->power_ctl.reset, 1);

    return rc;
}

/**
 * Change/Control the power status for the specified entity.
 *
 * param[in] hdl - handle to the entity whose information has to be retrieved.
 * param[in] enable - power state to enable / disable
 * return STD_ERR_OK on success and standard error on failure
 */
t_std_error sdi_entity_power_status_control(sdi_entity_hdl_t hdl, bool enable)
{
    char                 *val = NULL;
    t_std_error           rc = STD_ERR_OK;
    sdi_entity_priv_hdl_t entity_priv_hdl = NULL;

    STD_ASSERT((entity_priv_hdl = (sdi_entity_priv_hdl_t)hdl) != NULL);

    /* Check this entity supports power on/off operations */
    if (strlen(entity_priv_hdl->power_ctl.powerhdl) <= 0) {
        return SDI_ERRCODE(EOPNOTSUPP);
    }

    /* Get the power state value for platform */
    val = enable ? entity_priv_hdl->power_ctl.power_on : entity_priv_hdl->power_ctl.power_off;
    STD_ASSERT(strlen(val) > 0);

    /* Set the power state */
    rc = sdi_sysfs_attr_str_set(entity_priv_hdl->power_ctl.path, entity_priv_hdl->power_ctl.powerhdl, val);

    return rc;
}
