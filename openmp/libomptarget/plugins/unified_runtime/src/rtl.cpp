#if INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//===--- Target RTLs Implementation ---------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// RTL for SPIR-V/UnifiedRuntime
//
//===----------------------------------------------------------------------===//

#include "omptargetplugin.h"
#include <cstdint>
#include <ur_api.h>

///
/// Required interface to be a valid plugin. See ../../../src/rtl.cpp.
///
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) { return 1; }

int32_t __tgt_rtl_number_of_devices(void) {
  // Just try to use one of the UR API.
  auto RC = urInit(UR_DEVICE_INIT_FLAG_GPU);
  (void)RC;
  return 0;
}

int32_t __tgt_rtl_init_device(int32_t DeviceId) { return OFFLOAD_FAIL; }

__tgt_target_table *__tgt_rtl_load_binary(int32_t DeviceId,
                                          __tgt_device_image *Image) {
  return nullptr;
}

void *__tgt_rtl_data_alloc(int32_t DeviceId, int64_t Size, void *HstPtr,
                           int32_t Kind) {
  return nullptr;
}

int32_t __tgt_rtl_data_submit(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                              int64_t Size) {
  return OFFLOAD_FAIL;
}

int32_t __tgt_rtl_data_retrieve(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                                int64_t Size) {
  return OFFLOAD_FAIL;
}

int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr, int32_t Kind) {
  return OFFLOAD_FAIL;
}

int32_t __tgt_rtl_launch_kernel(int32_t DeviceId, void *Entry, void **Args,
                                ptrdiff_t *Offsets, KernelArgsTy *KernelArgs,
                                __tgt_async_info *AsyncInfo) {
  return OFFLOAD_FAIL;
}

#endif // INTEL_CUSTOMIZATION
