/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/******************************************************************************
 * \file sdi_common.h
 * \brief Common header
 *****************************************************************************/
#ifndef __SDI_COMMON_H
#define __SDI_COMMON_H

#include "std_llist.h"
#include "std_config_node.h"
#include "std_assert.h"
#include "std_utils.h"
#include "std_time_tools.h"
#include "std_bit_ops.h"
#include "std_error_codes.h"
#include "std_type_defs.h"
#include "event_log.h"
#include "sdi_entity_info.h"
#include <stdio.h>
#include <stdlib.h>

#define SDI_MAX_NAME_LEN NAME_MAX
#define SDI_MOD          "SDI_SYS_MODULE"

#define SDI_ERRNO_LOG()                    EV_LOG_ERRNO(ev_log_t_BOARD, 3, SDI_MOD, errno)
#define SDI_ERRMSG_LOG(format, args ...)   EV_LOG_ERR(ev_log_t_BOARD, 3, SDI_MOD, format, args)
#define SDI_TRACEMSG_LOG(format, args ...) EV_LOG_TRACE(ev_log_t_BOARD, 3, SDI_MOD, format, args)
#define SDI_ERRNO STD_ERR_MK(e_std_err_BOARD, e_std_err_code_FAIL, errno)
#define SDI_ERRCODE(errcode) STD_ERR_MK(e_std_err_BOARD, e_std_err_code_FAIL, errcode)
#define STD_ERR_UNIMPLEMENTED STD_ERR_MK(e_std_err_BOARD, e_std_err_code_FAIL, ENOSYS)


/**
 * @defgroup sdi_entity_presence_t
 * List of the entity presence type.
 */
typedef enum {
    SDI_ENTITY_FIXED,
    SDI_ENTITY_SWAPPABLE
} sdi_entity_presence_type_t;

/**
 * @struct sdi_entity_presence_t
 * Used to hold presence info for the entity.
 */
typedef struct sdi_entity_presence_s {
    sdi_entity_presence_type_t type;    /**< presence type of the entity */
    char                       path[PATH_MAX]; /**< path to the "presence" SysFs attribute */
    char                       name[SDI_MAX_NAME_LEN]; /**< name of the "presence" SysFs attribute */
    char                       present[SDI_MAX_NAME_LEN]; /**< value for the "present" state */
    char                       not_present[SDI_MAX_NAME_LEN]; /**< value for the "not present" state */
} sdi_entity_presence_t;

/**
 * @struct sdi_entity_status_t
 * Used to hold fault status info for the entity.
 */
typedef struct sdi_entity_status_s {
    bool is_supported;            /**< flag to check whether "fault status" attribute is supported */
    char path[PATH_MAX];          /**< path to the "fault status" SysFs attribute */
    char name[SDI_MAX_NAME_LEN];  /**< name of the "fault status" SysFs attribute */
    char ok[SDI_MAX_NAME_LEN];    /**< value for the "ok" status */
    char fault[SDI_MAX_NAME_LEN]; /**< value for the "fault" status */
} sdi_entity_status_t;

/**
 * @struct sdi_entity_power_t
 * Used to hold power info for the PSU entity.
 */
typedef struct sdi_entity_power_s {
    bool             is_supported;             /**< flag to check whether "power" attribute is supported */
    sdi_power_type_t type;                     /**< supported power types (AC and/or DC) */
    char             status_path[PATH_MAX];    /**< path to the "power status" SysFs attribute */
    char             status_name[SDI_MAX_NAME_LEN]; /**< name of the "power status" SysFs attribute */
    char             status_present[SDI_MAX_NAME_LEN]; /**< value for the "present" power status */
    char             status_not_present[SDI_MAX_NAME_LEN]; /**< value for the "not present" power status */
    char             rating_path[PATH_MAX];    /**< path to the "power rating" SysFs attribute */
    char             rating_name[SDI_MAX_NAME_LEN]; /**< name of the "power rating" SysFs attribute */
} sdi_entity_power_t;

/**
 * @struct sdi_entity_powerctl_t
 * Used to hold the power control info for the entity.
 */
typedef struct sdi_entity_powerctl_s {
    char path[PATH_MAX];                /**< path to the SysFs attributes for power reset or control*/
    char reset[SDI_MAX_NAME_LEN];       /**< name of the SysFs attribute for component reset */
    char powerhdl[SDI_MAX_NAME_LEN];    /**< name of the SysFs attribute for component power on/off operations */
    char power_on[SDI_MAX_NAME_LEN];    /**< value for the "ON" power status */
    char power_off[SDI_MAX_NAME_LEN];   /**< value for the "OFF" power status */
} sdi_entity_powerctl_t;


/**
 * @struct sdi_entity
 * Entity data structure which contains details of an entity.
 */
struct sdi_entity {
    sdi_entity_type_t     type;             /**< type of the entity */
    uint_t                instance;         /**< instance of the entity */
    sdi_entity_presence_t presence;         /**< entity presence info */
    sdi_entity_status_t   status;           /**< entity fault status info */
    sdi_entity_power_t    power;            /**< entity power info */
    char                  name[SDI_MAX_NAME_LEN]; /**< name of the entity */
    sdi_resource_hdl_t    entity_info_hdl;  /**< entity_info handler of the entity */
    std_dll_head         *resource_list;    /**< list of resources that are part of this entity */
    sdi_entity_powerctl_t power_ctl;        /**< entity reset and power control */
};

/** An opaque handle to entity. */
typedef struct sdi_entity *sdi_entity_priv_hdl_t;

/**
 * @struct sdi_entity_node_t
 * Used to hold entity related data.
 */
typedef struct sdi_entity_node {
    std_dll          node;       /**< node to an entity */
    sdi_entity_hdl_t entity_hdl; /**< entity specific data */
} sdi_entity_node_t;

/**
 * @struct sdi_resource
 * Resource data structure which contains details of the resource.
 */
struct sdi_resource {
    char                name[SDI_MAX_NAME_LEN]; /**< name of the resource */
    sdi_resource_type_t type;         /**< type of the resource */
    char                alias[SDI_MAX_NAME_LEN]; /**< alias name of the resource */
    char                reference[SDI_MAX_NAME_LEN]; /**< reference name of the resource */
    void               *settings;     /**< pointer to settings of the resource */
};

/** An opaque handle to resource. */
typedef struct sdi_resource *sdi_resource_priv_hdl_t;

/**
 * @struct sdi_entity_resource_node_t
 * Used to hold resource related data.
 */
typedef struct sdi_entity_resource_node {
    std_dll            node; /**< node to an resource */
    sdi_resource_hdl_t hdl; /**< resource specific data */
} sdi_entity_resource_node_t;

/**
 * Initializes internal data structures for the entity and creates entity-db.
 *
 * entity_cfg_file[in] - Entity config file which has information about the devices on
 * each entity.
 *
 * return None.
 */
void sdi_register_entities(const char * entity_cfg_file);

/**
 * Registers settings for the specified LED resource.
 *
 * hdl[in] - handle of the resource.
 * root[in] - config node for the resource settings.
 *
 * return None.
 */
void sdi_led_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t led_node);

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
                                std_config_node_t       info_node);

/**
 * Registers settings for the specified FAN resource.
 *
 * hdl[in] - handle of the resource.
 * fan_node[in] - config node for the FAN resource settings.
 *
 * return None.
 */
void sdi_fan_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t fan_node);


/*
 * Gets the maximum speed of the fan refered by resource.
 *
 * [in] hdl - resource handle of the fan
 * [out] speed - maximum speed(in RPM)
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_fan_max_speed_get(sdi_resource_hdl_t hdl, uint_t *max_speed);

/**
 * Registers settings for the specified thermal sensor resource.
 *
 * hdl[in] - handle of the resource.
 * temp_node[in] - config node for the resource settings.
 *
 * return None.
 */
void sdi_temp_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t temp_node);

/**
 * Registers settings for the specified media resource.
 *
 * hdl[in] - handle of the resource.
 * media_node[in] - config node for the resource settings.
 *
 * return None.
 */
void sdi_media_register_settings(sdi_resource_priv_hdl_t hdl, std_config_node_t media_node);

#endif /* __SDI_COMMON_H */
