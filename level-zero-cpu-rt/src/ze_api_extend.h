/*
 *
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 * @file ze_api_extend.h
 *
 * @brief Contain embargoed definitions for zesim
 *
 */
#ifndef _ZE_API_EXTENTD_H
#define _ZE_API_EXTENTD_H
#if defined(__cplusplus)
#pragma once
#endif
#if !defined(_ZE_API_H)
#pragma message("warning: this file is not intended to be included directly")
#endif

#if defined(__cplusplus)
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
/// @brief Set large GRF mode for the kernels in a module
///
/// @details
///     -
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
///     - This API is a workaround for CMRT CISA binary only as compiler does
///       not provide the GRF mode information in CISA bianry.
///     - This API is not supported for L0 binary.
///
/// @returns
///     - ::ZE_RESULT_SUCCESS
///     - ::ZE_RESULT_ERROR_UNINITIALIZED
///     - ::ZE_RESULT_ERROR_DEVICE_LOST
///     - ::ZE_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hKernel`
ZE_APIEXPORT ze_result_t ZE_APICALL zeSimModuleSetLargeGRFMode(
    ze_module_handle_t hModule, ///< [in] handle of the module object
    bool isLargeGRFMode ///< [in] true is for large GRF mode, false is for
                        ///< regular GRF mode
);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // _ZE_API_EXTENTD_H
