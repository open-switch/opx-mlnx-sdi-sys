/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * Core SDI framework which provides core APIs that work on entity.
 ***************************************************************************************/

#include "sdi_common.h"
#include "sdi_entity.h"

#define SDI_DEVICE_CONFIG_FILE "/etc/opx/sdi/device.xml"


static std_dll_head entity_list;

/* Note: Names must be in the same order as defined for enum sdi_entity_type_t */
static const char * sdi_entity_names[] = {
    "SDI_ENTITY_SYSTEM_BOARD",
    "SDI_ENTITY_FAN_TRAY",
    "SDI_ENTITY_PSU_TRAY"
};

/* Note: Names must be in the same order as defined for enum sdi_resource_type_t */
static const char * sdi_resource_names[] = {
    "SDI_RESOURCE_TEMPERATURE",
    "SDI_RESOURCE_FAN",
    "SDI_RESOURCE_LED",
    "SDI_RESOURCE_DIGIT_DISPLAY_LED",
    "SDI_RESOURCE_ENTITY_INFO",
    "SDI_RESOURCE_UPGRADABLE_PLD",
    "SDI_RESOURCE_MEDIA"
};

/**
 * Returns first entity from the list.
 *
 * return First entity node.
 */
static sdi_entity_node_t * sdi_entity_find_first(void)
{
    return (sdi_entity_node_t*)std_dll_getfirst(&entity_list);
}

/**
 * Returns entity node, next to given node from the entity list.
 *
 * node[in] - Entity node.
 *
 * return Next to given node from the entity list.
 */
static sdi_entity_node_t * sdi_entity_find_next(sdi_entity_node_t *node)
{
    return (sdi_entity_node_t*)std_dll_getnext(&entity_list, (std_dll*)node);
}

/**
 * Gets the entity type based on name. Entity name can be retrieved from config file.
 *
 * entity_name[in] - entity name.
 *
 * return Entity type.
 */
static sdi_entity_type_t sdi_entity_string_to_type(const char *entity_name)
{
    int entity_index = -1;
    int max_sdi_entities = (sizeof(sdi_entity_names) / sizeof(sdi_entity_names[0]));

    STD_ASSERT(entity_name != NULL);

    entity_index = dn_std_string_to_enum(sdi_entity_names, max_sdi_entities, entity_name);

    if (entity_index < STD_ERR_OK) {
        /* Could not find entity type, means SDI entity-list is corrupted, hence assert */
        STD_ASSERT(false);
    }

    return (sdi_entity_type_t)entity_index;
}

/**
 * Creates structure to hold entity information, and return a handle to it.
 *
 * type[in] - type of the entity that has to be created.
 * instance[in] - instance of the entity
 * name[in] - Name of the entity
 *
 * return Handle to the entity created if successful else return NULL.
 */
static sdi_entity_hdl_t sdi_entity_create(sdi_entity_type_t type, uint_t instance, const char *name)
{
    STD_ASSERT(name != NULL);

    sdi_entity_priv_hdl_t entity_hdl = (sdi_entity_priv_hdl_t)calloc(1, sizeof(struct sdi_entity));

    STD_ASSERT(entity_hdl != NULL);

    strncpy(entity_hdl->name, name, SDI_MAX_NAME_LEN);
    entity_hdl->type = type;
    entity_hdl->instance = instance;
    entity_hdl->presence.type = SDI_ENTITY_FIXED;
    entity_hdl->status.is_supported = false;
    entity_hdl->power.is_supported = false;
    entity_hdl->entity_info_hdl = NULL;

    entity_hdl->resource_list = (std_dll_head*)calloc(1, sizeof(std_dll_head));
    STD_ASSERT(entity_hdl->resource_list != NULL);

    std_dll_init(entity_hdl->resource_list);

    return (sdi_entity_hdl_t)entity_hdl;
}

/**
 * Gets the resource type based on name. Resource name can be retrieved from config file.
 *
 * resource_name[in] - resource name.
 *
 * return Entity type.
 */
static sdi_resource_type_t sdi_resource_string_to_type(const char *resource_name)
{
    int entity_index = -1;
    int max_sdi_resources = (sizeof(sdi_resource_names) / sizeof(sdi_resource_names[0]));

    STD_ASSERT(resource_name != NULL);

    entity_index = dn_std_string_to_enum(sdi_resource_names, max_sdi_resources, resource_name);

    if (entity_index < STD_ERR_OK) {
        /* Could not find resource type, means SDI resource-list is corrupted, hence assert */
        STD_ASSERT(false);
    }

    return (sdi_resource_type_t)entity_index;
}

/**
 * Adds the specified resource to specified entity.
 *
 * ehdl[in] -  handle of the entity to which the resource has to be added.
 * resource[in] - handle of the resource which needs to be added to entity.
 * name[in] - Name of the resource by which it known within this entity.
 *
 * return None.
 */
static void sdi_entity_add_resource(sdi_entity_hdl_t ehdl, sdi_resource_hdl_t resource, const char *name)
{
    sdi_entity_resource_node_t *newnode = NULL;

    STD_ASSERT(name != NULL);

    newnode = (sdi_entity_resource_node_t*)calloc(1, sizeof(sdi_entity_resource_node_t));
    STD_ASSERT(newnode != NULL);

    strncpy(((sdi_resource_priv_hdl_t)resource)->alias, name,
            sizeof(((sdi_resource_priv_hdl_t)resource)->alias));

    newnode->hdl = resource;

    std_dll_insertatback(((sdi_entity_priv_hdl_t)ehdl)->resource_list, (std_dll*)newnode);
}

/**
 * Registers settings for the specified resource.
 *
 * hdl[in] - handle of the resource.
 * entity_hdl[in] - handle of the entity, to which info resource belongs.
 * st_node[in] - config node for the resources settings.
 *
 * return None.
 */
static void sdi_resource_register_settings(sdi_resource_priv_hdl_t hdl,
                                           sdi_entity_priv_hdl_t   entity_hdl,
                                           std_config_node_t       st_node)
{
    std_config_node_t node = NULL;
    bool              settings_found = false;

    /* Find settings for the specified resource */
    for (node = std_config_get_child(st_node); (node != NULL); node = std_config_next_node(node)) {
        if (strncmp(hdl->reference, std_config_attr_get(node, "name"), sizeof(hdl->reference)) == 0) {
            settings_found = true;
            break;
        }
    }

    STD_ASSERT(settings_found != false);

    /* Register settings for the specified resource */
    switch (hdl->type) {
    case SDI_RESOURCE_ENTITY_INFO:
        sdi_info_register_settings(hdl, entity_hdl, node);
        break;

    case SDI_RESOURCE_LED:
        sdi_led_register_settings(hdl, node);
        break;

    case SDI_RESOURCE_FAN:
        sdi_fan_register_settings(hdl, node);
        break;

    case SDI_RESOURCE_TEMPERATURE:
        sdi_temp_register_settings(hdl, node);
        break;

    case SDI_RESOURCE_MEDIA:
        sdi_media_register_settings(hdl, node);
        break;

    default:
        /* Resource type is invalid, means SDI resource info is corrupted, hence assert */
        STD_ASSERT(false);
    }
}

/**
 * Adds resources to the entity and registers them.
 *
 * param node[in] - Entity node whose attribute values needs to be determined.
 * param st_node[in] - "settings" config node for the entity.
 * param entity_hdl[in] - Handler of entity which needs to add resources.
 *
 * return None.
 */
static void sdi_entity_register_resources(std_config_node_t node,
                                          std_config_node_t st_node,
                                          sdi_entity_hdl_t  entity_hdl)
{
    std_config_node_t  resource = NULL;
    sdi_resource_hdl_t res_hdl = NULL;
    char              *resource_name = NULL;
    char              *resource_reference = NULL;
    char              *resource_type = NULL;

    for ((resource = std_config_get_child(node));
         (resource != NULL);
         (resource = std_config_next_node(resource))) {
        STD_ASSERT((resource_reference = std_config_attr_get(resource, "reference")) != NULL);
        STD_ASSERT((resource_name = std_config_attr_get(resource, "name")) != NULL);
        STD_ASSERT((resource_type = std_config_attr_get(resource, "type")) != NULL);

        sdi_resource_priv_hdl_t resource_hdl = (sdi_resource_priv_hdl_t)calloc(1, sizeof(struct sdi_resource));
        STD_ASSERT(resource_hdl != NULL);

        strncpy(resource_hdl->name, resource_name, sizeof(resource_hdl->name));
        strncpy(resource_hdl->reference, resource_reference, sizeof(resource_hdl->reference));
        resource_hdl->type = sdi_resource_string_to_type(resource_type);
        sdi_resource_register_settings(resource_hdl, (sdi_entity_priv_hdl_t)entity_hdl, st_node);

        res_hdl = (sdi_resource_hdl_t)(resource_hdl);

        /* In case of ENTITY_INFO, initialize the entity_info handler of entity */
        if (sdi_resource_type_get(res_hdl) == SDI_RESOURCE_ENTITY_INFO) {
            ((sdi_entity_priv_hdl_t)entity_hdl)->entity_info_hdl = res_hdl;
        }

        sdi_entity_add_resource(entity_hdl, res_hdl, resource_name);
    }
}

/**
 * Adds entity handler to the entity list.
 *
 * hdl[in] - Handler of entity which needs to be added to entity list.
 *
 * return None.
 */
static void sdi_add_entity(sdi_entity_hdl_t entity_hdl)
{
    sdi_entity_node_t *node = NULL;

    node = (sdi_entity_node_t*)calloc(1, sizeof(sdi_entity_node_t));
    STD_ASSERT(node != NULL);

    node->entity_hdl = entity_hdl;
    std_dll_insertatfront(&entity_list, (std_dll*)node);
}

/**
 * Gets the child config node by the specified name.
 *
 * node[in] - parent config node.
 * name[in] - name of child config node.
 *
 * return child config node or NULL it doesn't exist.
 */
static std_config_node_t sdi_settings_get_child_by_name(std_config_node_t node, const char *name)
{
    std_config_node_t child = NULL;
    bool              found = false;

    STD_ASSERT(node != NULL);
    STD_ASSERT(name != NULL);

    for (child = std_config_get_child(node); (child != NULL); child = std_config_next_node(child)) {
        if (strncmp(name, std_config_attr_get(child, "name"), strlen(name)) == 0) {
            found = true;
            break;
        }
    }

    return (found == true) ? child : NULL;
}

/**
 * Registers "presence" settings for the swappable entity.
 *
 * hdl[in] - handler of the specified entity.
 * entity_node[in] - config node for the entity settings.
 * pres_name[in] - "presence" settings name.
 *
 * return None.
 */
static void sdi_entity_presence_register(sdi_entity_priv_hdl_t hdl,
                                         std_config_node_t     entity_node,
                                         const char           *pres_name)
{
    std_config_node_t node = NULL;
    char             *name = NULL;
    char             *path = NULL;
    char             *stat_present = NULL;
    char             *stat_not_present = NULL;

    STD_ASSERT(hdl != NULL);
    STD_ASSERT(hdl->presence.type != SDI_ENTITY_FIXED);

    node = sdi_settings_get_child_by_name(entity_node, pres_name);
    STD_ASSERT(node != NULL);

    STD_ASSERT((name = std_config_attr_get(node, "name")) != NULL);
    STD_ASSERT((path = std_config_attr_get(node, "path")) != NULL);
    STD_ASSERT((stat_present = std_config_attr_get(node, "present")) != NULL);
    STD_ASSERT((stat_not_present = std_config_attr_get(node, "not_present")) != NULL);

    strncpy(hdl->presence.name, name, sizeof(hdl->presence.name));
    strncpy(hdl->presence.path, path, sizeof(hdl->presence.path));
    strncpy(hdl->presence.present, stat_present, sizeof(hdl->presence.present));
    strncpy(hdl->presence.not_present, stat_not_present, sizeof(hdl->presence.not_present));
}

/**
 * Registers "fault status" settings for the swappable entity.
 *
 * hdl[in] - handler of the specified entity.
 * entity_node[in] - config node for the entity settings.
 * pres_name[in] - "fault status" settings name.
 *
 * return None.
 */
static void sdi_entity_fault_register(sdi_entity_priv_hdl_t hdl, std_config_node_t entity_node, const char *fault_name)
{
    std_config_node_t node = NULL;
    char             *name = NULL;
    char             *path = NULL;
    char             *stat_ok = NULL;
    char             *stat_fault = NULL;

    STD_ASSERT(hdl != NULL);
    STD_ASSERT(hdl->status.is_supported != false);

    node = sdi_settings_get_child_by_name(entity_node, fault_name);
    STD_ASSERT(node != NULL);

    STD_ASSERT((name = std_config_attr_get(node, "name")) != NULL);
    STD_ASSERT((path = std_config_attr_get(node, "path")) != NULL);
    STD_ASSERT((stat_ok = std_config_attr_get(node, "ok")) != NULL);
    STD_ASSERT((stat_fault = std_config_attr_get(node, "fault")) != NULL);

    strncpy(hdl->status.name, name, sizeof(hdl->presence.name));
    strncpy(hdl->status.path, path, sizeof(hdl->presence.path));
    strncpy(hdl->status.ok, stat_ok, sizeof(hdl->status.ok));
    strncpy(hdl->status.fault, stat_fault, sizeof(hdl->status.fault));
}

/**
 * Registers "power" settings for the PSU entity.
 *
 * hdl[in] - handler of the specified entity.
 * entity_node[in] - config node for the entity settings.
 *
 * return None.
 */
static void sdi_entity_power_register(sdi_entity_priv_hdl_t hdl, std_config_node_t entity_node)
{
    std_config_node_t node = NULL;
    char             *type = NULL;
    char             *name = NULL;
    char             *path = NULL;
    char             *present = NULL;
    char             *not_present = NULL;

    STD_ASSERT(hdl != NULL);
    STD_ASSERT(hdl->power.is_supported != false);

    /* Get power type (AC/DC) */
    STD_ASSERT((type = std_config_attr_get(entity_node, "type")) != NULL);
    if (strncmp("AC", type, strlen("AC")) == 0) {
        hdl->power.type.ac_power = true;
    } else if (strncmp("DC", type, strlen("DC")) == 0) {
        hdl->power.type.dc_power = true;
    } else {
        STD_ASSERT(false);
    }

    for (node = std_config_get_child(entity_node); (node != NULL); node = std_config_next_node(node)) {
        if (strcmp("power", std_config_name_get(node)) == 0) {
            /* Get "power status" settings */
            STD_ASSERT((name = std_config_attr_get(node, "name")) != NULL);
            STD_ASSERT((path = std_config_attr_get(node, "path")) != NULL);
            STD_ASSERT((present = std_config_attr_get(node, "present")) != NULL);
            STD_ASSERT((not_present = std_config_attr_get(node, "not_present")) != NULL);

            strncpy(hdl->power.status_name, name, sizeof(hdl->power.status_name));
            strncpy(hdl->power.status_path, path, sizeof(hdl->power.status_path));
            strncpy(hdl->power.status_present, present, sizeof(hdl->power.status_present));
            strncpy(hdl->power.status_not_present, not_present, sizeof(hdl->power.status_not_present));
        } else if (strcmp("rating", std_config_name_get(node))) {
            /* Get "power rating" settings */
            STD_ASSERT((name = std_config_attr_get(node, "name")) != NULL);
            STD_ASSERT((path = std_config_attr_get(node, "path")) != NULL);

            strncpy(hdl->power.rating_name, name, sizeof(hdl->power.rating_name));
            strncpy(hdl->power.rating_path, path, sizeof(hdl->power.rating_path));
        }
    }
}

/**
 * Registers "power control" settings for the entity.
 *
 * hdl[in] - handler of the specified entity.
 * entity_node[in] - config node for the entity settings.
 * pwr_hdl_name[in] - the "power control" settings name.
 *
 * return None.
 */
static void sdi_entity_pwrctl_register(sdi_entity_priv_hdl_t hdl,
                                       std_config_node_t     entity_node,
                                       const char           *pwr_hdl_name)
{
    std_config_node_t node = NULL;
    char             *path = NULL;
    char             *reset_attr = NULL;
    char             *power_hdl_attr = NULL;

    STD_ASSERT(hdl != NULL);

    node = sdi_settings_get_child_by_name(entity_node, pwr_hdl_name);
    STD_ASSERT(node != NULL);

    STD_ASSERT((path = std_config_attr_get(node, "path")) != NULL);
    strncpy(hdl->power_ctl.path, path, sizeof(hdl->power_ctl.path));

    /* Register reset related settings */
    if ((reset_attr = std_config_attr_get(node, "reset")) != NULL) {
        strncpy(hdl->power_ctl.reset, reset_attr, sizeof(hdl->power_ctl.reset));
    } else {
        *(hdl->power_ctl.reset) = "\0";
    }

    /* Register power control related settings */
    if ((power_hdl_attr = std_config_attr_get(node, "powerhdl")) != NULL) {
        strncpy(hdl->power_ctl.powerhdl, power_hdl_attr, sizeof(hdl->power_ctl.powerhdl));
    } else {
        *(hdl->power_ctl.powerhdl) = "\0";
    }

    if ((power_hdl_attr = std_config_attr_get(node, "power_on")) != NULL) {
        strncpy(hdl->power_ctl.power_on, power_hdl_attr, sizeof(hdl->power_ctl.power_on));
    } else {
        *(hdl->power_ctl.power_on) = "\0";
    }

    if ((power_hdl_attr = std_config_attr_get(node, "power_off")) != NULL) {
        strncpy(hdl->power_ctl.power_off, power_hdl_attr, sizeof(hdl->power_ctl.power_off));
    } else {
        *(hdl->power_ctl.power_off) = "\0";
    }
}

/**
 * Allocates and Initializes the data structures for a given node and adds it to entity
 * list.
 *
 * node[in] - Node whose attribute values needs to be determined.
 *
 * return None.
 */
static void sdi_register_entity(std_config_node_t node, std_config_node_t settings_root)
{
    const char       *entity_name = std_config_name_get(node);
    char             *alias_name = NULL;
    char              alias[SDI_MAX_NAME_LEN];
    char             *config_attr = NULL;
    char             *entity_presence = NULL;
    char             *entity_fault = NULL;
    char             *entity_power_ctl = NULL;
    uint_t            instance = 0;
    sdi_entity_type_t entity_type = 0;
    sdi_entity_hdl_t  entity_hdl = NULL;
    std_config_node_t settings_node = NULL;

    memset(alias, '\0', sizeof(alias));

    /* Get instance value */
    STD_ASSERT((config_attr = std_config_attr_get(node, "instance")) != NULL);
    instance = atoi(config_attr);

    /* Get alias value */
    alias_name = std_config_attr_get(node, "alias");
    if (alias_name == NULL) {
        snprintf(alias, SDI_MAX_NAME_LEN, "%s-%u", entity_name, instance);
    } else {
        strncpy(alias, alias_name, sizeof(alias));
    }

    /* Get type value */
    STD_ASSERT((config_attr = std_config_attr_get(node, "type")) != NULL);
    entity_type = sdi_entity_string_to_type(config_attr);

    STD_ASSERT((entity_presence = std_config_attr_get(node, "presence")) != NULL);

    SDI_TRACEMSG_LOG("\nRegistering entity: %s@%d\n", config_attr, instance);

    entity_hdl = sdi_entity_create(entity_type, instance, alias);
    STD_ASSERT(entity_hdl);

    settings_node = sdi_settings_get_child_by_name(settings_root, alias_name);
    STD_ASSERT(settings_node != NULL);

    /* Register "presence" related settings */
    if (strncmp(entity_presence, "fixed", sizeof(entity_presence)) == 0) {
        ((sdi_entity_priv_hdl_t)entity_hdl)->presence.type = SDI_ENTITY_FIXED;
    } else {
        ((sdi_entity_priv_hdl_t)entity_hdl)->presence.type = SDI_ENTITY_SWAPPABLE;
        sdi_entity_presence_register((sdi_entity_priv_hdl_t)entity_hdl, settings_node, entity_presence);
    }

    /* Register "fault status" related settings */
    if ((entity_fault = std_config_attr_get(node, "fault")) != NULL) {
        ((sdi_entity_priv_hdl_t)entity_hdl)->status.is_supported = true;
        sdi_entity_fault_register((sdi_entity_priv_hdl_t)entity_hdl, settings_node, entity_fault);
    } else {
        ((sdi_entity_priv_hdl_t)entity_hdl)->status.is_supported = false;
    }

    /* Register power related settings for PSU */
    if (((sdi_entity_priv_hdl_t)entity_hdl)->type == SDI_ENTITY_PSU_TRAY) {
        ((sdi_entity_priv_hdl_t)entity_hdl)->power.is_supported = true;
        sdi_entity_power_register((sdi_entity_priv_hdl_t)entity_hdl, settings_node);
    } else {
        ((sdi_entity_priv_hdl_t)entity_hdl)->power.is_supported = false;
    }

    /* Register "power control" related settings */
    if ((entity_power_ctl = std_config_attr_get(node, "power_ctl")) != NULL) {
        sdi_entity_pwrctl_register((sdi_entity_priv_hdl_t)entity_hdl, settings_node, entity_power_ctl);
    } else {
        *(((sdi_entity_priv_hdl_t)entity_hdl)->power_ctl.reset) = "\0";
        *(((sdi_entity_priv_hdl_t)entity_hdl)->power_ctl.powerhdl) = "\0";
        *(((sdi_entity_priv_hdl_t)entity_hdl)->power_ctl.power_on) = "\0";
        *(((sdi_entity_priv_hdl_t)entity_hdl)->power_ctl.power_off) = "\0";
    }

    sdi_entity_register_resources(node, settings_node, entity_hdl);
    sdi_add_entity(entity_hdl);
}

/**
 * Initializes internal data structures for the entity and creates entity-db.
 *
 * entity_cfg_file[in] - Entity config file which has information about the devices on
 * each entity.
 *
 * return None.
 */
void sdi_register_entities(const char * entity_cfg_file)
{
    std_config_hdl_t  cfg_hdl = NULL;
    std_config_node_t root = NULL;
    std_config_node_t entity = NULL;
    std_config_hdl_t  settings_hdl = NULL;
    std_config_node_t settings_node = NULL;

    STD_ASSERT(entity_cfg_file != NULL);

    cfg_hdl = std_config_load(entity_cfg_file);
    root = std_config_get_root(cfg_hdl);

    STD_ASSERT(root != NULL);

    /* Load "settings" config file and find config node for the entity */
    settings_hdl = std_config_load(SDI_DEVICE_CONFIG_FILE);
    settings_node = std_config_get_root(settings_hdl);
    STD_ASSERT(settings_node != NULL);

    std_dll_init(&entity_list);

    for (entity = std_config_get_child(root); (entity != NULL); entity = std_config_next_node(entity)) {
        SDI_TRACEMSG_LOG("Found entity: %s\n", std_config_name_get(entity));

        sdi_register_entity(entity, settings_node);
    }

    std_config_unload(cfg_hdl);
    std_config_unload(settings_hdl);
}

/**
 * Iterates on entity list and runs specified function on every entity.
 *
 * hdl[in] - entity handle
 * fn[in] - function that would be called for each entity
 * user_data[in] - user data that will be passed to the function
 *
 * return None.
 */
void sdi_entity_for_each(void (*fn)(sdi_entity_hdl_t hdl, void *user_data), void *user_data)
{
    sdi_entity_node_t *hdl = NULL;

    for (hdl = sdi_entity_find_first(); (hdl != NULL);
         hdl = sdi_entity_find_next(hdl)) {
        (*fn)(hdl->entity_hdl, user_data);
    }
}

/**
 * Returns name of the given entity.
 *
 * hdl[in] - handle to the entity whose name has to be returned.
 *
 * return Name of the given entity.
 */
const char * sdi_entity_name_get(sdi_entity_hdl_t hdl)
{
    return ((sdi_entity_priv_hdl_t)hdl)->name;
}

/**
 * Returns type of the given entity.
 *
 * hdl[in] - handle to the entity whose type has to be returned.
 *
 * return Type of the given entity.
 */
sdi_entity_type_t sdi_entity_type_get(sdi_entity_hdl_t hdl)
{
    return ((sdi_entity_priv_hdl_t)hdl)->type;
}

/**
 * Retrieves number of entities supported by system of given type.
 *
 * etype[in] - entity type.
 *
 * return number of entities of the specified type.
 */
uint_t sdi_entity_count_get(sdi_entity_type_t etype)
{
    uint_t                entity_count = 0;
    sdi_entity_node_t    *node = NULL;
    sdi_entity_priv_hdl_t entity_hdl = NULL;

    for (node = sdi_entity_find_first(); (node != NULL);
         node = sdi_entity_find_next(node)) {
        entity_hdl = (sdi_entity_priv_hdl_t)node->entity_hdl;

        if (entity_hdl->type == etype) {
            entity_count++;
        }
    }

    return entity_count;
}

/**
 * Retrieves the handle of the specified entity by given type and instance.
 *
 * etype[in] - Type of entity
 * instance[in] - Instance of the entity of specified type that has to be retrieved.
 *
 * return the handle to the specified entity, NULL if entity is not found.
 */
sdi_entity_hdl_t sdi_entity_lookup(sdi_entity_type_t etype, uint_t instance)
{
    sdi_entity_node_t    *hdl = NULL;
    sdi_entity_priv_hdl_t entity_hdl = NULL;

    for (hdl = sdi_entity_find_first(); (hdl != NULL);
         hdl = sdi_entity_find_next(hdl)) {
        entity_hdl = (sdi_entity_priv_hdl_t)hdl->entity_hdl;

        if ((entity_hdl->type == etype) && (entity_hdl->instance == instance)) {
            return hdl->entity_hdl;
        }
    }

    return NULL;
}

/**
 * Retrieves number of resources of given type within given entity.
 *
 * hdl[in] - handle to the entity whose information has to be retrieved.
 * resource_type[in] - type of resource. Example, temperature, fan etc.
 *
 * return the number of resources of given type within given entity.
 */
uint_t sdi_entity_resource_count_get(sdi_entity_hdl_t hdl, sdi_resource_type_t resource_type)
{
    uint_t                      resource_count = 0;
    sdi_entity_resource_node_t *node = NULL;
    sdi_entity_priv_hdl_t       entity_hdl = NULL;
    sdi_resource_priv_hdl_t     resource_hdl = NULL;
    std_dll_head               *resource_head = NULL;

    STD_ASSERT(hdl != NULL);

    entity_hdl = (sdi_entity_priv_hdl_t)hdl;
    resource_head = (std_dll_head*)entity_hdl->resource_list;
    STD_ASSERT(resource_head != NULL);

    for ((node = (sdi_entity_resource_node_t*)std_dll_getfirst(resource_head));
         (node != NULL);
         (node = (sdi_entity_resource_node_t*)std_dll_getnext(resource_head, (std_dll*)node))) {
        resource_hdl = (sdi_resource_priv_hdl_t)node->hdl;

        if (resource_hdl->type == resource_type) {
            resource_count++;
        }
    }

    return resource_count;
}

/**
 * Retrieves the handle of the resource whose name is known.
 *
 * hdl[in] - handle to the entity whose information has to be retrieved.
 * resource[in] - The type of resource that needs to be looked up.
 * alias[in] - the name of the alias. example, "BOOT_STATUS" led.
 *
 * return - handle to found resource, else returns NULL.
 */
sdi_resource_hdl_t sdi_entity_resource_lookup(sdi_entity_hdl_t hdl, sdi_resource_type_t resource, const char *alias)
{
    sdi_entity_resource_node_t *node = NULL;
    sdi_entity_priv_hdl_t       entity_hdl = NULL;
    sdi_resource_priv_hdl_t     resource_hdl = NULL;
    std_dll_head               *resource_head = NULL;

    STD_ASSERT(hdl != NULL);

    entity_hdl = (sdi_entity_priv_hdl_t)hdl;
    resource_head = (std_dll_head*)entity_hdl->resource_list;
    STD_ASSERT(resource_head != NULL);

    for ((node = (sdi_entity_resource_node_t*)std_dll_getfirst(resource_head));
         (node != NULL);
         (node = (sdi_entity_resource_node_t*)std_dll_getnext(resource_head, (std_dll*)node))) {
        resource_hdl = (sdi_resource_priv_hdl_t)node->hdl;
        if (strncmp(resource_hdl->alias, alias, strlen(alias)) == 0) {
            return node->hdl;
        }
    }

    return NULL;
}

/**
 * Retrieves the alias name of the given resource.
 *
 * resource_hdl[in] - handle to the resource whose name has to be retrieved.
 *
 * return the alias name of the resource. example, "BOOT_STATUS" led.
 */
const char * sdi_resource_alias_get(sdi_resource_hdl_t resource_hdl)
{
    return ((sdi_resource_priv_hdl_t)resource_hdl)->alias;
}

/**
 * Iterates on each resource and run specified function.
 *
 * hdl[in] - Entity handle.
 * fn[in] - function that would be called for each resource.
 * user_data[in] - user data that will be passed to the function.
 *
 * return None.
 */
void sdi_entity_for_each_resource(sdi_entity_hdl_t hdl, void (*fn)(sdi_resource_hdl_t hdl,
                                                                   void *user_data), void *user_data)
{
    sdi_entity_resource_node_t *node;
    sdi_entity_priv_hdl_t       entity_hdl = (sdi_entity_priv_hdl_t)hdl;
    std_dll_head               *resource_head = (std_dll_head*)entity_hdl->resource_list;

    STD_ASSERT(fn != NULL);

    for ((node = (sdi_entity_resource_node_t*)std_dll_getfirst(resource_head));
         (node);
         (node = (sdi_entity_resource_node_t*)std_dll_getnext(resource_head, (std_dll*)node))) {
        (*fn)(node->hdl, user_data);
    }
}

/**
 * Initializes the specified entity.
 * Upon Initialization, default configurations as specified for platform would
 * be applied.
 *
 * param[in] hdl - handle to the entity whose information has to be initialized.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_entity_init(sdi_entity_hdl_t hdl)
{
    bool        presence = false;
    t_std_error rc = STD_ERR_OK;

    STD_ASSERT(hdl != NULL);

    (void)sdi_entity_presence_get(hdl, &presence);
    if (presence != true) {
        rc = EPERM;
    }

    if (rc != STD_ERR_OK) {
        SDI_ERRMSG_LOG("Entity (%s): Init failed (rc=%d).\n",
                       ((sdi_entity_priv_hdl_t)hdl)->name, rc);
    }

    return rc;
}

/**
 * Returns the type of resource from resource handler.
 *
 * param[in] hdl - resource handle.
 *
 * return type of resource.
 */
sdi_resource_type_t sdi_resource_type_get(sdi_resource_hdl_t hdl)
{
    return ((sdi_resource_priv_hdl_t)hdl)->type;
}

/**
 * @TODO: Below two functions are added as a interim solution for PAS
 * compilation.These functions will be removed once the PAS team clean-up
 * their code.
 */

/**
 * Retrieve the handle of first resource of the specified type within the entity.
 * hdl[in] -  handle to the entity whose information has to be retrieved.
 * resource[in] - The type of resource that needs to be looked up.
 * return - if a resource maching the criteria is found, returns handle to it.
 *          else returns NULL.
 */
sdi_resource_hdl_t sdi_entity_get_first_resource(sdi_entity_hdl_t hdl, sdi_resource_type_t resource)
{
    return NULL;
}
/**
 * Retrieve the handle of next resource of the specified type within the entity.
 * hdl[in] - handle to the entity whose information has to be retrieved.
 * resource[in] - The type of resource that needs to be looked up.
 * return - if a resource maching the criteria is found, returns handle to it.
 *          else returns NULL.
 */
sdi_resource_hdl_t sdi_entity_get_next_resource(sdi_resource_hdl_t hdl, sdi_resource_type_t resource)
{
    return NULL;
}
