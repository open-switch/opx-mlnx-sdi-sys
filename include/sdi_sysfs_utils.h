/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */


/******************************************************************************
 * \file sdi_sysfs_utils.h
 * \brief SysFs util functions to get and set attributes of resources
 *****************************************************************************/
#ifndef __SDI_SYSFS__UTILS_H
#define __SDI_SYSFS__UTILS_H

#include "sdi_common.h"


/**
 * Sets string value for SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[in] - value which should be set.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_str_set(const char *path, const char *attr, const char *val);

/**
 * Gets the string value from SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[out] - retrieved value.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_str_get(const char *path, const char *attr, char *val);

/**
 * Sets the unsigned integer value for SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[in] - value which should be set.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_uint_set(const char *path, const char *attr, uint_t val);

/**
 * Gets the unsigned integer value from SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[out] - retrieved value.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_uint_get(const char *path, const char *attr, uint_t *val);

/**
 * Gets the integer value from SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * val[out] - retrieved value.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_int_get(const char *path, const char *attr, int *val);

/**
 * Gets the size in bytes of raw data in the specified SysFs attribute.
 *
 * path[in] - path to SysFs attribute.
 * attr[in] - name of SysFs attribute.
 * size[in] - size of the raw data from specified SysFs attribute.
 *
 * return STD_ERR_OK on success and standard error on failure.
 */
t_std_error sdi_sysfs_attr_data_size_get(const char *path, const char *attr, size_t *size);

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
t_std_error sdi_sysfs_attr_data_get(const char *path, const char *attr, size_t size, char *buf);

#endif /* __SDI_SYSFS__UTILS_H */
