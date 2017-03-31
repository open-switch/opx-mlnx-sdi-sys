/*
 * Copyright Mellanox Technologies, Ltd. 2001-2017.
 * This software product is licensed under Apache version 2, as detailed in
 * the LICENSE file.
 */

/**************************************************************************************
 *  Implementation of public APIs for reading and writing NVRAM.
 ***************************************************************************************/

#include "sdi_nvram.h"
#include "sdi_common.h"


/**
 * Returns size (bytes) of NVRAM
 *
 * resource_hdl[in] - handle of the NVRAM resource that is of interest
 * size[out] - size of NVRAM, in bytes
 *
 * return - STD_ERR_OK on success and standard error on failure
 */
t_std_error sdi_nvram_size(sdi_resource_hdl_t resource_hdl, uint_t *size)
{
    *size = 0;

    return SDI_ERRCODE(EOPNOTSUPP);
}

/**
 * Reads NVRAM
 *
 * resource_hdl[in] - handle of the NVRAM resource that is of interest
 * buf[out] - buffer to fill with NVRAM data
 * ofs[in] - offset (byes) into NVRAM to read
 * len[in] - number of bytes to read
 *
 * @return - STD_ERR_OK on success and standard error on failure
 */
t_std_error sdi_nvram_read(sdi_resource_hdl_t resource_hdl, uint8_t *buf, uint_t ofs, uint_t len)
{
    return SDI_ERRCODE(EOPNOTSUPP);
}

/**
 * Writes NVRAM
 *
 * resource_hdl[in] - handle of the NVRAM resource that is of interest
 * buf[in] - buffer of data to write to NVRAM
 * ofs[in] - offset (byes) into NVRAM to write
 * len[in] - number of bytes to write
 *
 * @return - STD_ERR_OK on success and standard error on failure
 */
t_std_error sdi_nvram_write(sdi_resource_hdl_t resource_hdl, uint8_t *buf, uint_t ofs, uint_t len)
{
    return SDI_ERRCODE(EOPNOTSUPP);
}
