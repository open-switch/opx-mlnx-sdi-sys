/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/**************************************************************************************
 * SysFs util functions to get and set attributes of resources.
 ***************************************************************************************/

#include "sdi_common.h"
#include "sdi_sysfs_utils.h"
#include <stdio.h>
#include <unistd.h>


/**
 * Sets string value for SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[in] - value which should be set.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_str_set(const char *path, const char *attr, const char *val)
{
    FILE *f = NULL;
    char  full_path[PATH_MAX] = {0};

    if ((path == NULL) || (attr == NULL) || (val == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    snprintf(full_path, sizeof(full_path) - 1, "%s%s", path, attr);

    if (access(full_path, F_OK) == -1) {
        return SDI_ERRNO;
    }

    if ((f = fopen(full_path, "w")) == NULL) {
        return SDI_ERRNO;
    }

    fprintf(f, "%s", val);

    fclose(f);

    return STD_ERR_OK;
}

/**
 * Gets the string value from SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[out] - retrieved value.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_str_get(const char *path, const char *attr, char *val)
{
    FILE *f = NULL;
    char  full_path[PATH_MAX] = {0};

    if ((path == NULL) || (attr == NULL) || (val == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    snprintf(full_path, sizeof(full_path) - 1, "%s%s", path, attr);

    if (access(full_path, F_OK) == -1) {
        return SDI_ERRNO;
    }

    if ((f = fopen(full_path, "r")) == NULL) {
        return SDI_ERRNO;
    }

    fscanf(f, "%s", val);

    fclose(f);

    return STD_ERR_OK;
}

/**
 * Sets the unsigned integer value for SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[in] - value which should be set.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_uint_set(const char *path, const char *attr, uint_t val)
{
    FILE *f = NULL;
    char  full_path[PATH_MAX] = {0};

    if ((path == NULL) || (attr == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    snprintf(full_path, sizeof(full_path) - 1, "%s%s", path, attr);

    if (access(full_path, F_OK) == -1) {
        return SDI_ERRNO;
    }

    if ((f = fopen(full_path, "w")) == NULL) {
        return SDI_ERRNO;
    }

    fprintf(f, "%d", val);

    fclose(f);

    return STD_ERR_OK;
}

/**
 * Gets the unsigned integer value from SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[out] - retrieved value.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_uint_get(const char *path, const char *attr, uint_t *val)
{
    FILE *f = NULL;
    char  full_path[PATH_MAX] = {0};

    if ((path == NULL) || (attr == NULL) || (val == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    snprintf(full_path, sizeof(full_path) - 1, "%s%s", path, attr);

    if (access(full_path, F_OK) == -1) {
        return SDI_ERRNO;
    }

    if ((f = fopen(full_path, "r")) == NULL) {
        return SDI_ERRNO;
    }

    fscanf(f, "%u", val);

    fclose(f);

    return STD_ERR_OK;
}

/**
 * Gets the integer value from SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[out] - retrieved value.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_int_get(const char *path, const char *attr, int *val)
{
    FILE *f = NULL;
    char  full_path[PATH_MAX] = {0};

    if ((path == NULL) || (attr == NULL) || (val == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    snprintf(full_path, sizeof(full_path) - 1, "%s%s", path, attr);

    if (access(full_path, F_OK) == -1) {
        return SDI_ERRNO;
    }

    if ((f = fopen(full_path, "r")) == NULL) {
        return SDI_ERRNO;
    }

    fscanf(f, "%d", val);

    fclose(f);

    return STD_ERR_OK;
}

/**
 * Gets the size in bytes of raw data in the specified SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * size[in] - size of the raw data from specified SysFs attribute.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_data_size_get(const char *path, const char *attr, size_t *size)
{
    t_std_error rc = STD_ERR_OK;
    FILE       *f = NULL;
    char        full_path[PATH_MAX] = {0};

    if ((path == NULL) || (attr == NULL) || (size == NULL)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    snprintf(full_path, sizeof(full_path) - 1, "%s%s", path, attr);

    if ((f = fopen(full_path, "rb")) == NULL) {
        return SDI_ERRNO;
    }

    rc = fseek(f, 0, SEEK_END);
    if (rc == 0) {
        *size = ftell(f);
        rc = fseek(f, 0, SEEK_SET);
    }

    fclose(f);

    return rc;
}

/**
 * Gets the raw data from the specified SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * size[in] - size the of buffer, where raw data should be stored.
 * buf[out] - buffer, where raw data should be stored.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_data_get(const char *path, const char *attr, size_t size, char *buf)
{
    t_std_error rc = STD_ERR_OK;
    FILE       *f = NULL;
    char        full_path[PATH_MAX] = {0};

    if ((path == NULL) || (attr == NULL) || (buf == NULL) || (size == 0)) {
        return SDI_ERRCODE(EINVAL); /* Invalid argument */
    }

    snprintf(full_path, sizeof(full_path) - 1, "%s%s", path, attr);

    if ((f = fopen(full_path, "rb")) == NULL) {
        return SDI_ERRNO;
    }

    if (fread(buf, sizeof(*buf), size, f) != size) {
        if (feof(f) || ferror(f)) {
            rc = SDI_ERRCODE(EIO); /* I/O error */
        }
    }

    fclose(f);

    return rc;
}
