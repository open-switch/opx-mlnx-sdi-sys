/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * SDI Framework core functions.
 ***************************************************************************************/

#include "sdi_common.h"

/**
 * @def Attirbute used to get entity config file path.
 */
#define SDI_ENTITY_CONFIG_FILE "/etc/opx/sdi/entity.xml"


/**
 * Initializes the specified entity.
 *
 * param[in] hdl - handle to the entity whose information has to be initialised.
 * param[out] init_status - entity initialization status.
 *
 * return None
 */
static void sdi_sys_entity_init(sdi_entity_hdl_t hdl, void *init_status)
{
    *((t_std_error*)init_status) = sdi_entity_init(hdl);
}

/**
 * Initializes the SDI sub-system which creates resources and entities.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sys_init(void)
{
    t_std_error rc = STD_ERR_OK;

    sdi_register_entities(SDI_ENTITY_CONFIG_FILE);

    /* Initialise each entity */
    sdi_entity_for_each(&sdi_sys_entity_init, &rc);
    if (rc != STD_ERR_OK) {
        SDI_ERRMSG_LOG("At least one Entity failed in the init (rc=%d)\n", rc);
    }

    return rc;
}
