/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * Implementation of generic entity  and resource API.
 ***************************************************************************************/

#include "sdi_entity.h"
#include "sdi_common.h"


/**
 * Retrieve presence status of given entity.
 *
 * entity_hdl[in] - handle to the entity whose information has to be retrieved.
 * presence[out]    - true if entity is present, false otherwise
 *
 * return STD_ERR_OK on success and standard error on failure
 */
t_std_error sdi_entity_presence_get(sdi_entity_hdl_t entity_hdl, bool *presence)
{
    t_std_error           rc = STD_ERR_OK;
    sdi_entity_priv_hdl_t hdl = NULL;
    char                  pres[SDI_MAX_NAME_LEN] = {0};

    STD_ASSERT(entity_hdl != NULL);
    STD_ASSERT(presence != NULL);

    hdl = (sdi_entity_priv_hdl_t)entity_hdl;

    *presence = false;

    if (hdl->presence.type == SDI_ENTITY_FIXED) {
        *presence = true;
    } else {
        rc = sdi_sysfs_attr_str_get(hdl->presence.path, hdl->presence.name, pres);
        if ((rc == STD_ERR_OK) && (strncmp(hdl->presence.present, pres, sizeof(hdl->presence.present)) == 0)) {
            *presence = true;
        }
    }

    return rc;
}

/**
 * Checks the fault status for a given entity
 *
 * entity_hdl[in] - handle to the entity whose information has to be retrieved.
 * fault[out] - true if entity has any fault, false otherwise.
 *
 * return STD_ERR_OK on success and standard error on failure
 */
t_std_error sdi_entity_fault_status_get(sdi_entity_hdl_t entity_hdl, bool *fault)
{
    t_std_error           rc = STD_ERR_OK;
    sdi_entity_priv_hdl_t hdl = NULL;
    char                  status[SDI_MAX_NAME_LEN] = {0};

    STD_ASSERT(entity_hdl != NULL);
    STD_ASSERT(fault != NULL);

    hdl = (sdi_entity_priv_hdl_t)entity_hdl;

    *fault = false;

    if (hdl->status.is_supported == true) {
        rc = sdi_sysfs_attr_str_get(hdl->status.path, hdl->status.name, status);
        if ((rc != STD_ERR_OK) || (strncmp(hdl->status.fault, status, sizeof(hdl->status.fault)) == 0)) {
            *fault = true;
        }
    }

    return rc;
}

/**
 * Checks the psu output power status for a given psu
 *
 * entity_hdl[in] - handle to the psu entity whose information has to be retrieved.
 * status[out] - true if psu output status is good , false otherwise.
 *
 * return STD_ERR_OK on success , standard error on failure
 */
t_std_error sdi_entity_psu_output_power_status_get(sdi_entity_hdl_t entity_hdl, bool *status)
{
    t_std_error           rc = SDI_ERRCODE(EPERM);
    sdi_entity_priv_hdl_t hdl = (sdi_entity_priv_hdl_t)entity_hdl;
    char                  pwr_status[SDI_MAX_NAME_LEN] = {0};

    STD_ASSERT(status != NULL);
    STD_ASSERT(hdl != NULL);
    STD_ASSERT(hdl->type == SDI_ENTITY_PSU_TRAY);

    *status = false;

    if (hdl->power.is_supported == true) {
        rc = sdi_sysfs_attr_str_get(hdl->power.status_path, hdl->power.status_name, pwr_status);
        if ((rc == STD_ERR_OK) &&
            (strncmp(hdl->power.status_present, pwr_status, sizeof(hdl->power.status_present)) == 0)) {
            *status = true;
        }
    }

    return rc;
}
