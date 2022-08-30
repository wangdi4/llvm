/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */

#if INTEL_CUSTOMIZATION

//===--- Target SYCL WRAPPER RTLs Implementation --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// RTL for SYCL WRAPPER for OpenMP interop object
//
//===----------------------------------------------------------------------===//

#include <string>
#include "omptarget.h"
#include "Debug.h"

#include <ze_api.h>
#include <sycl/ext/oneapi/backend/level_zero.hpp>
#include <vector>

#define STR(x) #x
#define TO_STRING(x) STR(x)

#define TARGET_NAME SYCL_WRAPPER
#define DEBUG_PREFIX "Target " GETNAME(TARGET_NAME) " RTL"

int DebugLevel = getDebugLevel();

using namespace sycl;

class SyclWrapperTy {
public:
  void *ZeDevice;               // Level0 device
  void *ZeQueue;                // Level0 queue
  sycl::platform SyclPlatform;
  sycl::device SyclDevice;
  sycl::context SyclContext;
  sycl::queue SyclQueue;
  omp_interop_t interop;        // Original openmp interop
};

std::vector<SyclWrapperTy *> SyclWrappers;

EXTERN void *__tgt_sycl_get_interop(void *zedevice) {

  for (auto obj : SyclWrappers)
    if (obj->ZeDevice == zedevice)
      return static_cast<void *>(obj);
  return NULL;
}

EXTERN void __tgt_sycl_create_interop_wrapper(omp_interop_t interop) {

  ze_driver_handle_t ZePlatform;
  ze_context_handle_t ZeContext;
  void *ZeDevice;
  void *ZeQueue;

  __tgt_interop *TgtInterop = static_cast<__tgt_interop *>(interop);

  ZePlatform = static_cast<ze_driver_handle_t>(TgtInterop->Platform);
  ZeContext = static_cast<ze_context_handle_t>(TgtInterop->DeviceContext);
  ZeDevice = TgtInterop->Device;
  ZeQueue = TgtInterop->TargetSync;

  SyclWrapperTy *SyclWrapperObj = new SyclWrapperTy;
  SyclWrapperObj->SyclPlatform = sycl::ext::oneapi::level_zero::make_platform(
      reinterpret_cast<pi_native_handle>(ZePlatform));

  SyclWrapperObj->ZeDevice = ZeDevice;
  SyclWrapperObj->SyclDevice = sycl::ext::oneapi::level_zero::make_device(
      SyclWrapperObj->SyclPlatform,
      reinterpret_cast<pi_native_handle>(ZeDevice));

  SyclWrapperObj->SyclContext =
      sycl::make_context<sycl::backend::ext_oneapi_level_zero>(
          {ZeContext,
           {SyclWrapperObj->SyclDevice},
           sycl::ext::oneapi::level_zero::ownership::keep});

  SyclWrapperObj->ZeQueue = ZeQueue;
  ze_command_queue_handle_t ZeQueueT =
      static_cast<ze_command_queue_handle_t>(ZeQueue);
  SyclWrapperObj->SyclQueue =
      sycl::make_queue<sycl::backend::ext_oneapi_level_zero>(
          {ZeQueueT, sycl::ext::oneapi::level_zero::ownership::keep},
          SyclWrapperObj->SyclContext);

  SyclWrapperObj->interop = interop;
  SyclWrappers.push_back(SyclWrapperObj);

  // Update interop object by replacing  level0 with sycl
  TgtInterop->Platform = static_cast<void *>(&SyclWrapperObj->SyclPlatform);
  TgtInterop->DeviceContext = static_cast<void *>(&SyclWrapperObj->SyclContext);
  TgtInterop->Device = static_cast<void *>(&SyclWrapperObj->SyclDevice);
  TgtInterop->TargetSync = static_cast<void *>(&SyclWrapperObj->SyclQueue);
  TgtInterop->FrId = 4;
  TgtInterop->FrName = GETNAME(sycl);

  DP("Created sycl wrapper " DPxMOD " for interop " DPxMOD "\n",
     DPxPTR(interop), DPxPTR(SyclWrapperObj));
}

EXTERN void __tgt_sycl_delete_interop_wrapper(omp_interop_t interop) {
  for (auto obj : SyclWrappers)
    if (obj->interop == interop) {
      delete obj;
      DP("Deleted sycl wrapper for interop " DPxMOD "\n", DPxPTR(interop));
      return;
    }
  DP("ERROR: Could not find sycl wrapper " DPxMOD "\n", DPxPTR(interop));
}

EXTERN void __tgt_sycl_delete_all_interop_wrapper() {
  for (auto obj : SyclWrappers)
    delete obj;
}

#endif // INTEL_CUSTOMIZATION
