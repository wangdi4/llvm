//===-------- interface.cpp - Target independent OpenMP target RTL --------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementation of the interface to be used by Clang during the codegen of a
// target region.
//
//===----------------------------------------------------------------------===//

#if INTEL_CUSTOMIZATION
#include "omptarget-tools.h"
#include "xpti_registry.h"
#endif // INTEL_CUSTOMIZATION

#include "device.h"
#include "omptarget.h"
#include "private.h"
#include "rtl.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#if INTEL_COLLAB
#include <string.h>
#endif  // INTEL_COLLAB

extern bool isOffloadDisabled();

#if INTEL_COLLAB
//static uint32_t useSingleQueue = 0;

static int64_t GetEncodedDeviceID(int64_t &DeviceID) {
  if (DeviceID == OFFLOAD_DEVICE_DEFAULT)
    return omp_get_default_device();

  int64_t EncodedID = DeviceID;
  if (EncodedID < 0) {
    // DeviceID is already encoded (e.g., subdevice clause)
    DeviceID = EXTRACT_BITS(EncodedID, 31, 0);
  } else if (PM->RootDeviceID >= 0) {
    // DeviceID is sub-device ID -- move it to "start" bits and replace it with
    // the stored root device ID.
    EncodedID = PM->SubDeviceMask | (DeviceID << 48);
    DeviceID = PM->RootDeviceID;
  }

  return EncodedID;
}
#endif // INTEL_COLLAB

////////////////////////////////////////////////////////////////////////////////
/// adds requires flags
EXTERN void __tgt_register_requires(int64_t Flags) {
  TIMESCOPE();
  PM->RTLs.registerRequires(Flags);
}

////////////////////////////////////////////////////////////////////////////////
/// adds a target shared library to the target execution image
EXTERN void __tgt_register_lib(__tgt_bin_desc *Desc) {
  TIMESCOPE();
  std::call_once(PM->RTLs.InitFlag, &RTLsTy::loadRTLs, &PM->RTLs);
  for (auto &RTL : PM->RTLs.AllRTLs) {
    if (RTL.register_lib) {
      if ((*RTL.register_lib)(Desc) != OFFLOAD_SUCCESS) {
        DP("Could not register library with %s", RTL.RTLName.c_str());
      }
    }
  }
  PM->RTLs.registerLib(Desc);
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize all available devices without registering any image
EXTERN void __tgt_init_all_rtls() { PM->RTLs.initAllRTLs(); }

////////////////////////////////////////////////////////////////////////////////
/// unloads a target shared library
EXTERN void __tgt_unregister_lib(__tgt_bin_desc *Desc) {
  TIMESCOPE();
  PM->RTLs.unregisterLib(Desc);
  for (auto &RTL : PM->RTLs.UsedRTLs) {
    if (RTL->unregister_lib) {
      if ((*RTL->unregister_lib)(Desc) != OFFLOAD_SUCCESS) {
        DP("Could not register library with %s", RTL->RTLName.c_str());
      }
    }
  }
}

/// creates host-to-target data mapping, stores it in the
/// libomptarget.so internal structure (an entry in a stack of data maps)
/// and passes the data to the device.
EXTERN void __tgt_target_data_begin(int64_t DeviceId, int32_t ArgNum,
                                    void **ArgsBase, void **Args,
                                    int64_t *ArgSizes, int64_t *ArgTypes) {
  TIMESCOPE();
  __tgt_target_data_begin_mapper(nullptr, DeviceId, ArgNum, ArgsBase, Args,
                                 ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN void __tgt_target_data_begin_nowait(int64_t DeviceId, int32_t ArgNum,
                                           void **ArgsBase, void **Args,
                                           int64_t *ArgSizes, int64_t *ArgTypes,
                                           int32_t DepNum, void *DepList,
                                           int32_t NoAliasDepNum,
                                           void *NoAliasDepList) {
  TIMESCOPE();

  __tgt_target_data_begin_mapper(nullptr, DeviceId, ArgNum, ArgsBase, Args,
                                 ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN void __tgt_target_data_begin_mapper(ident_t *Loc, int64_t DeviceId,
                                           int32_t ArgNum, void **ArgsBase,
                                           void **Args, int64_t *ArgSizes,
                                           int64_t *ArgTypes,
                                           map_var_info_t *ArgNames,
                                           void **ArgMappers) {
  TIMESCOPE_WITH_IDENT(Loc);

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_COLLAB

  DP("Entering data begin region for device %" PRId64 " with %d mappings\n",
     DeviceId, ArgNum);
  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return;
  }

  DeviceTy &Device = *PM->Devices[DeviceId];

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(Loc, DeviceId, ArgNum, ArgSizes, ArgTypes, ArgNames,
                         "Entering OpenMP data region");
#ifdef OMPTARGET_DEBUG
  for (int I = 0; I < ArgNum; ++I) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       I, DPxPTR(ArgsBase[I]), DPxPTR(Args[I]), ArgSizes[I], ArgTypes[I],
       (ArgNames) ? getNameFromMapping(ArgNames[I]).c_str() : "unknown");
  }
#endif

#if INTEL_COLLAB
  Device.pushSubDevice(encodedId, DeviceId);
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataEnterBegin(DeviceId));
#endif // INTEL_CUSTOMIZATION

  AsyncInfoTy AsyncInfo(Device);
  int Rc = targetDataBegin(Loc, Device, ArgNum, ArgsBase, Args, ArgSizes,
                           ArgTypes, ArgNames, ArgMappers, AsyncInfo);
  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();
  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataEnterEnd(DeviceId));
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  if (encodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_COLLAB

}

EXTERN void __tgt_target_data_begin_nowait_mapper(
    ident_t *Loc, int64_t DeviceId, int32_t ArgNum, void **ArgsBase,
    void **Args, int64_t *ArgSizes, int64_t *ArgTypes, map_var_info_t *ArgNames,
    void **ArgMappers, int32_t DepNum, void *DepList, int32_t NoAliasDepNum,
    void *NoAliasDepList) {
  TIMESCOPE_WITH_IDENT(Loc);

  __tgt_target_data_begin_mapper(Loc, DeviceId, ArgNum, ArgsBase, Args,
                                 ArgSizes, ArgTypes, ArgNames, ArgMappers);
}

/// passes data from the target, releases target memory and destroys
/// the host-target mapping (top entry from the stack of data maps)
/// created by the last __tgt_target_data_begin.
EXTERN void __tgt_target_data_end(int64_t DeviceId, int32_t ArgNum,
                                  void **ArgsBase, void **Args,
                                  int64_t *ArgSizes, int64_t *ArgTypes) {
  TIMESCOPE();
  __tgt_target_data_end_mapper(nullptr, DeviceId, ArgNum, ArgsBase, Args,
                               ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN void __tgt_target_data_end_nowait(int64_t DeviceId, int32_t ArgNum,
                                         void **ArgsBase, void **Args,
                                         int64_t *ArgSizes, int64_t *ArgTypes,
                                         int32_t DepNum, void *DepList,
                                         int32_t NoAliasDepNum,
                                         void *NoAliasDepList) {
  TIMESCOPE();

  __tgt_target_data_end_mapper(nullptr, DeviceId, ArgNum, ArgsBase, Args,
                               ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN void __tgt_target_data_end_mapper(ident_t *Loc, int64_t DeviceId,
                                         int32_t ArgNum, void **ArgsBase,
                                         void **Args, int64_t *ArgSizes,
                                         int64_t *ArgTypes,
                                         map_var_info_t *ArgNames,
                                         void **ArgMappers) {
  TIMESCOPE_WITH_IDENT(Loc);

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_COLLAB

  DP("Entering data end region with %d mappings\n", ArgNum);
  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return;
  }

  DeviceTy &Device = *PM->Devices[DeviceId];

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(Loc, DeviceId, ArgNum, ArgSizes, ArgTypes, ArgNames,
                         "Exiting OpenMP data region");
#ifdef OMPTARGET_DEBUG
  for (int I = 0; I < ArgNum; ++I) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       I, DPxPTR(ArgsBase[I]), DPxPTR(Args[I]), ArgSizes[I], ArgTypes[I],
       (ArgNames) ? getNameFromMapping(ArgNames[I]).c_str() : "unknown");
  }
#endif

#if INTEL_COLLAB
  Device.pushSubDevice(encodedId, DeviceId);
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataExitBegin(DeviceId));
#endif // INTEL_CUSTOMIZATION

  AsyncInfoTy AsyncInfo(Device);
  int Rc = targetDataEnd(Loc, Device, ArgNum, ArgsBase, Args, ArgSizes,
                         ArgTypes, ArgNames, ArgMappers, AsyncInfo);
  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();
  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataExitEnd(DeviceId));
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  if (encodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_COLLAB

}

EXTERN void __tgt_target_data_end_nowait_mapper(
    ident_t *Loc, int64_t DeviceId, int32_t ArgNum, void **ArgsBase,
    void **Args, int64_t *ArgSizes, int64_t *ArgTypes, map_var_info_t *ArgNames,
    void **ArgMappers, int32_t DepNum, void *DepList, int32_t NoAliasDepNum,
    void *NoAliasDepList) {
  TIMESCOPE_WITH_IDENT(Loc);

  __tgt_target_data_end_mapper(Loc, DeviceId, ArgNum, ArgsBase, Args, ArgSizes,
                               ArgTypes, ArgNames, ArgMappers);
}

EXTERN void __tgt_target_data_update(int64_t DeviceId, int32_t ArgNum,
                                     void **ArgsBase, void **Args,
                                     int64_t *ArgSizes, int64_t *ArgTypes) {
  TIMESCOPE();
  __tgt_target_data_update_mapper(nullptr, DeviceId, ArgNum, ArgsBase, Args,
                                  ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN void __tgt_target_data_update_nowait(
    int64_t DeviceId, int32_t ArgNum, void **ArgsBase, void **Args,
    int64_t *ArgSizes, int64_t *ArgTypes, int32_t DepNum, void *DepList,
    int32_t NoAliasDepNum, void *NoAliasDepList) {
  TIMESCOPE();

  __tgt_target_data_update_mapper(nullptr, DeviceId, ArgNum, ArgsBase, Args,
                                  ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN void __tgt_target_data_update_mapper(ident_t *Loc, int64_t DeviceId,
                                            int32_t ArgNum, void **ArgsBase,
                                            void **Args, int64_t *ArgSizes,
                                            int64_t *ArgTypes,
                                            map_var_info_t *ArgNames,
                                            void **ArgMappers) {
  TIMESCOPE_WITH_IDENT(Loc);

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

  DP("Entering data update with %d mappings\n", ArgNum);

#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_COLLAB

  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return;
  }

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(Loc, DeviceId, ArgNum, ArgSizes, ArgTypes, ArgNames,
                         "Updating OpenMP data");

  DeviceTy &Device = *PM->Devices[DeviceId];
  AsyncInfoTy AsyncInfo(Device);

#if INTEL_COLLAB
  Device.pushSubDevice(encodedId, DeviceId);
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataUpdateBegin(DeviceId));
#endif // INTEL_CUSTOMIZATION

  int Rc = targetDataUpdate(Loc, Device, ArgNum, ArgsBase, Args, ArgSizes,
                            ArgTypes, ArgNames, ArgMappers, AsyncInfo);
  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();
  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataUpdateEnd(DeviceId));
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  if (encodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_COLLAB

}

EXTERN void __tgt_target_data_update_nowait_mapper(
    ident_t *Loc, int64_t DeviceId, int32_t ArgNum, void **ArgsBase,
    void **Args, int64_t *ArgSizes, int64_t *ArgTypes, map_var_info_t *ArgNames,
    void **ArgMappers, int32_t DepNum, void *DepList, int32_t NoAliasDepNum,
    void *NoAliasDepList) {
  TIMESCOPE_WITH_IDENT(Loc);

  __tgt_target_data_update_mapper(Loc, DeviceId, ArgNum, ArgsBase, Args,
                                  ArgSizes, ArgTypes, ArgNames, ArgMappers);
}

EXTERN int __tgt_target(int64_t DeviceId, void *HostPtr, int32_t ArgNum,
                        void **ArgsBase, void **Args, int64_t *ArgSizes,
                        int64_t *ArgTypes) {
  TIMESCOPE();
  return __tgt_target_mapper(nullptr, DeviceId, HostPtr, ArgNum, ArgsBase, Args,
                             ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN int __tgt_target_nowait(int64_t DeviceId, void *HostPtr, int32_t ArgNum,
                               void **ArgsBase, void **Args, int64_t *ArgSizes,
                               int64_t *ArgTypes, int32_t DepNum, void *DepList,
                               int32_t NoAliasDepNum, void *NoAliasDepList) {
  TIMESCOPE();

  return __tgt_target_mapper(nullptr, DeviceId, HostPtr, ArgNum, ArgsBase, Args,
                             ArgSizes, ArgTypes, nullptr, nullptr);
}

EXTERN int __tgt_target_mapper(ident_t *Loc, int64_t DeviceId, void *HostPtr,
                               int32_t ArgNum, void **ArgsBase, void **Args,
                               int64_t *ArgSizes, int64_t *ArgTypes,
                               map_var_info_t *ArgNames, void **ArgMappers) {
  TIMESCOPE_WITH_IDENT(Loc);
<<<<<<< HEAD

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_COLLAB

  DP("Entering target region with entry point " DPxMOD " and device Id %" PRId64
     "\n",
     DPxPTR(HostPtr), DeviceId);
  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return OMP_TGT_FAIL;
  }

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(Loc, DeviceId, ArgNum, ArgSizes, ArgTypes, ArgNames,
                         "Entering OpenMP kernel");
#ifdef OMPTARGET_DEBUG
  for (int I = 0; I < ArgNum; ++I) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       I, DPxPTR(ArgsBase[I]), DPxPTR(Args[I]), ArgSizes[I], ArgTypes[I],
       (ArgNames) ? getNameFromMapping(ArgNames[I]).c_str() : "unknown");
  }
#endif

#if INTEL_COLLAB
  // Push device encoding
  PM->Devices[DeviceId]->pushSubDevice(encodedId, DeviceId);
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetBegin(DeviceId));
#endif // INTEL_CUSTOMIZATION

  DeviceTy &Device = *PM->Devices[DeviceId];
  AsyncInfoTy AsyncInfo(Device);
  int Rc =
      target(Loc, Device, HostPtr, ArgNum, ArgsBase, Args, ArgSizes, ArgTypes,
             ArgNames, ArgMappers, 0, 0, false /*team*/, AsyncInfo);
  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();
  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetEnd(DeviceId));
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  if (encodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_COLLAB

  assert(Rc == OFFLOAD_SUCCESS && "__tgt_target_mapper unexpected failure!");
  return OMP_TGT_SUCCESS;
=======
  __tgt_kernel_arguments KernelArgs{1,        ArgNum,   ArgsBase, Args,
                                    ArgSizes, ArgTypes, ArgNames, ArgMappers};
  return __tgt_target_kernel(Loc, DeviceId, -1, 0, HostPtr, &KernelArgs);
>>>>>>> ad23e4d85fb39e99ff61f588bad480b824d9d1df
}

EXTERN int __tgt_target_nowait_mapper(
    ident_t *Loc, int64_t DeviceId, void *HostPtr, int32_t ArgNum,
    void **ArgsBase, void **Args, int64_t *ArgSizes, int64_t *ArgTypes,
    map_var_info_t *ArgNames, void **ArgMappers, int32_t DepNum, void *DepList,
    int32_t NoAliasDepNum, void *NoAliasDepList) {
  TIMESCOPE_WITH_IDENT(Loc);

  return __tgt_target_mapper(Loc, DeviceId, HostPtr, ArgNum, ArgsBase, Args,
                             ArgSizes, ArgTypes, ArgNames, ArgMappers);
}

EXTERN int __tgt_target_teams(int64_t DeviceId, void *HostPtr, int32_t ArgNum,
                              void **ArgsBase, void **Args, int64_t *ArgSizes,
                              int64_t *ArgTypes, int32_t TeamNum,
                              int32_t ThreadLimit) {
  TIMESCOPE();
  return __tgt_target_teams_mapper(nullptr, DeviceId, HostPtr, ArgNum, ArgsBase,
                                   Args, ArgSizes, ArgTypes, nullptr, nullptr,
                                   TeamNum, ThreadLimit);
}

EXTERN int __tgt_target_teams_nowait(int64_t DeviceId, void *HostPtr,
                                     int32_t ArgNum, void **ArgsBase,
                                     void **Args, int64_t *ArgSizes,
                                     int64_t *ArgTypes, int32_t TeamNum,
                                     int32_t ThreadLimit, int32_t DepNum,
                                     void *DepList, int32_t NoAliasDepNum,
                                     void *NoAliasDepList) {
  TIMESCOPE();

  return __tgt_target_teams_mapper(nullptr, DeviceId, HostPtr, ArgNum, ArgsBase,
                                   Args, ArgSizes, ArgTypes, nullptr, nullptr,
                                   TeamNum, ThreadLimit);
}

EXTERN int __tgt_target_teams_mapper(ident_t *Loc, int64_t DeviceId,
                                     void *HostPtr, int32_t ArgNum,
                                     void **ArgsBase, void **Args,
                                     int64_t *ArgSizes, int64_t *ArgTypes,
                                     map_var_info_t *ArgNames,
                                     void **ArgMappers, int32_t TeamNum,
                                     int32_t ThreadLimit) {
<<<<<<< HEAD

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_COLLAB

=======
  TIMESCOPE_WITH_IDENT(Loc);
  __tgt_kernel_arguments KernelArgs{1,        ArgNum,   ArgsBase, Args,
                                    ArgSizes, ArgTypes, ArgNames, ArgMappers};
  return __tgt_target_kernel(Loc, DeviceId, TeamNum, ThreadLimit, HostPtr,
                             &KernelArgs);
}

/// Implements a kernel entry that executes the target region on the specified
/// device.
///
/// \param Loc Source location associated with this target region.
/// \param DeviceId The device to execute this region, -1 indicated the default.
/// \param NumTeams Number of teams to launch the region with, -1 indicates a
///                 non-teams region and 0 indicates it was unspecified.
/// \param ThreadLimit Limit to the number of threads to use in the kernel
///                    launch, 0 indicates it was unspecified.
/// \param HostPtr  The pointer to the host function registered with the kernel.
/// \param Args     All arguments to this kernel launch (see struct definition).
EXTERN int __tgt_target_kernel(ident_t *Loc, int64_t DeviceId, int32_t NumTeams,
                               int32_t ThreadLimit, void *HostPtr,
                               __tgt_kernel_arguments *Args) {
  TIMESCOPE_WITH_IDENT(Loc);
>>>>>>> ad23e4d85fb39e99ff61f588bad480b824d9d1df
  DP("Entering target region with entry point " DPxMOD " and device Id %" PRId64
     "\n",
     DPxPTR(HostPtr), DeviceId);
  if (Args->Version != 1) {
    DP("Unexpected ABI version: %d\n", Args->Version);
  }
  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return OMP_TGT_FAIL;
  }

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(Loc, DeviceId, Args->NumArgs, Args->ArgSizes,
                         Args->ArgTypes, Args->ArgNames,
                         "Entering OpenMP kernel");
#ifdef OMPTARGET_DEBUG
  for (int I = 0; I < Args->NumArgs; ++I) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       I, DPxPTR(Args->ArgBasePtrs[I]), DPxPTR(Args->ArgPtrs[I]),
       Args->ArgSizes[I], Args->ArgTypes[I],
       (Args->ArgNames) ? getNameFromMapping(Args->ArgNames[I]).c_str()
                        : "unknown");
  }
#endif

<<<<<<< HEAD
#if INTEL_COLLAB
  // Push device encoding
  PM->Devices[DeviceId]->pushSubDevice(encodedId, DeviceId);
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetBegin(DeviceId));
#endif // INTEL_CUSTOMIZATION
=======
  bool IsTeams = NumTeams != -1;
  if (!IsTeams)
    NumTeams = 0;
>>>>>>> ad23e4d85fb39e99ff61f588bad480b824d9d1df

  DeviceTy &Device = *PM->Devices[DeviceId];
  AsyncInfoTy AsyncInfo(Device);
  int Rc = target(Loc, Device, HostPtr, Args->NumArgs, Args->ArgBasePtrs,
                  Args->ArgPtrs, Args->ArgSizes, Args->ArgTypes, Args->ArgNames,
                  Args->ArgMappers, NumTeams, ThreadLimit, IsTeams, AsyncInfo);
  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();
  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);
<<<<<<< HEAD

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetEnd(DeviceId));
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  if (encodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_COLLAB

  assert(Rc == OFFLOAD_SUCCESS &&
         "__tgt_target_teams_mapper unexpected failure!");
=======
  assert(Rc == OFFLOAD_SUCCESS && "__tgt_target_kernel unexpected failure!");
>>>>>>> ad23e4d85fb39e99ff61f588bad480b824d9d1df
  return OMP_TGT_SUCCESS;
}

EXTERN int __tgt_target_kernel_nowait(
    ident_t *Loc, int64_t DeviceId, int32_t NumTeams, int32_t ThreadLimit,
    void *HostPtr, __tgt_kernel_arguments *Args, int32_t DepNum, void *DepList,
    int32_t NoAliasDepNum, void *NoAliasDepList) {
  TIMESCOPE_WITH_IDENT(Loc);

  return __tgt_target_kernel(Loc, DeviceId, NumTeams, ThreadLimit, HostPtr,
                             Args);
}

// Get the current number of components for a user-defined mapper.
EXTERN int64_t __tgt_mapper_num_components(void *RtMapperHandle) {
  TIMESCOPE();
  auto *MapperComponentsPtr = (struct MapperComponentsTy *)RtMapperHandle;
  int64_t Size = MapperComponentsPtr->Components.size();
  DP("__tgt_mapper_num_components(Handle=" DPxMOD ") returns %" PRId64 "\n",
     DPxPTR(RtMapperHandle), Size);
  return Size;
}

// Push back one component for a user-defined mapper.
EXTERN void __tgt_push_mapper_component(void *RtMapperHandle, void *Base,
                                        void *Begin, int64_t Size, int64_t Type,
                                        void *Name) {
  TIMESCOPE();
  DP("__tgt_push_mapper_component(Handle=" DPxMOD
     ") adds an entry (Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
     ", Type=0x%" PRIx64 ", Name=%s).\n",
     DPxPTR(RtMapperHandle), DPxPTR(Base), DPxPTR(Begin), Size, Type,
     (Name) ? getNameFromMapping(Name).c_str() : "unknown");
  auto *MapperComponentsPtr = (struct MapperComponentsTy *)RtMapperHandle;
  MapperComponentsPtr->Components.push_back(
      MapComponentInfoTy(Base, Begin, Size, Type, Name));
}

EXTERN void __kmpc_push_target_tripcount(int64_t DeviceId,
                                         uint64_t LoopTripcount) {
  __kmpc_push_target_tripcount_mapper(nullptr, DeviceId, LoopTripcount);
}

EXTERN void __kmpc_push_target_tripcount_mapper(ident_t *Loc, int64_t DeviceId,
                                                uint64_t LoopTripcount) {
  TIMESCOPE_WITH_IDENT(Loc);
  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return;
  }

  DP("__kmpc_push_target_tripcount(%" PRId64 ", %" PRIu64 ")\n", DeviceId,
     LoopTripcount);
  PM->TblMapMtx.lock();
  PM->Devices[DeviceId]->LoopTripCnt.emplace(__kmpc_global_thread_num(NULL),
                                             LoopTripcount);
  PM->TblMapMtx.unlock();
}

#if INTEL_COLLAB
EXTERN int32_t __tgt_is_device_available(int64_t device_num,
                                         void *device_type) {
  device_num = EXTRACT_BITS(device_num, 31, 0);
  if (checkDeviceAndCtors(device_num, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    handleTargetOutcome(false, nullptr);
    return false;
  }

  return PM->Devices[device_num]->isSupportedDevice(device_type);
}

EXTERN char *__tgt_get_device_name(
    int64_t device_num, char *buffer, size_t buffer_max_size) {
  DP("Call to __tgt_get_device_name with device_num %" PRId64 " and "
     "buffer_max_size %zu.\n",
     device_num, buffer_max_size);

  if (!buffer || buffer_max_size == 0 || isOffloadDisabled())
    return NULL;

  if (checkDeviceAndCtors(device_num, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    handleTargetOutcome(false, nullptr);
    return NULL;
  }

  DP("Querying device for its name.\n");

  DeviceTy &Device = *PM->Devices[device_num];
  return Device.get_device_name(buffer, buffer_max_size);
}

EXTERN char *__tgt_get_device_rtl_name(
    int64_t device_num, char *buffer, size_t buffer_max_size) {
  DP("Call to __tgt_get_device_rtl_name with device_num %" PRId64 " and "
     "buffer_max_size %zu.\n",
     device_num, buffer_max_size);

  if (!buffer || buffer_max_size == 0 || isOffloadDisabled())
    return NULL;


  if (checkDeviceAndCtors(device_num, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    handleTargetOutcome(false, nullptr);
    return NULL;
  }

  const RTLInfoTy *RTL = PM->Devices[device_num]->RTL;
  assert(RTL && "Device with uninitialized RTL.");
  strncpy(buffer, RTL->RTLConstName, buffer_max_size - 1);
  buffer[buffer_max_size - 1] = '\0';
  return buffer;
}

// Begin INTEL DISPATCH extension
// The following code upto "End INTEL DISPATCH extension should be deleted
// once we obsolete INTEL DISPATCH extension
EXTERN void __tgt_offload_proxy_task_complete_ooo(void *interop_obj) {

  DP("Call to __tgt_offload_proxy_task_complete_ooo interop obj " DPxMOD "\n",
      DPxPTR(interop_obj));
  void *async_obj =  ((__tgt_interop *) interop_obj)->intel_tmp_ext->async_obj;
  (void)__tgt_release_interop(interop_obj);
  __kmpc_proxy_task_completed_ooo(async_obj);
}

// TEMPORARY Remove once Intel interop extension is obsoleted
// Declared in api.cpp
EXTERN omp_intptr_t omp_get_interop_int(const omp_interop_t interop,
    omp_interop_property_t property_id, int *ret_code);
EXTERN void *omp_get_interop_ptr(const omp_interop_t interop,
    omp_interop_property_t property_id, int *ret_code);

 // Use single queue for Intel dispatch when is_async is false
static std::map<int32_t, void *> interop_obj_queue_lists;

// END TEMPORARY

EXTERN void *__tgt_create_interop_obj(
    int64_t device_code, bool is_async, void *async_obj) {
  int64_t device_id = EXTRACT_BITS(device_code, 31, 0);
  int plugin_type;
  omp_interop_t Interop;

  bool queue_found = interop_obj_queue_lists.find(device_id) != interop_obj_queue_lists.end();
  
  if (is_async || !queue_found)
     Interop = __tgt_create_interop(device_id,
                              OMP_INTEROP_CONTEXT_TARGETSYNC, 0, NULL);
  else
     Interop = __tgt_create_interop(device_id,
                              OMP_INTEROP_CONTEXT_TARGET, 0, NULL);
  if (!Interop)
    return NULL;

  // Save the queue created for reuse  later
  if (!is_async && !queue_found) {
     // Save the queue for reuse and set TargetSync to NULL so the queue
     // is not destroyed in plugin when interop obj is released.
     int ret_code = OFFLOAD_FAIL;
     interop_obj_queue_lists[device_id]  = (void *) omp_get_interop_ptr(Interop, omp_ipr_targetsync, &ret_code);
     ((__tgt_interop *)Interop)->TargetSync= NULL;
  }

  __tgt_interop_obj *intel_ext_obj =
      (__tgt_interop_obj *)malloc(sizeof(__tgt_interop_obj));
  if (!intel_ext_obj)
    return NULL;

  ((__tgt_interop *)Interop)->intel_tmp_ext = intel_ext_obj;

  intel_ext_obj->is_async = is_async;
  intel_ext_obj->async_obj = async_obj;
  intel_ext_obj->async_handler = &__tgt_offload_proxy_task_complete_ooo;
  int ret_code = OFFLOAD_FAIL;
  intel_ext_obj->device_id =
      omp_get_interop_int(Interop, omp_ipr_device_num, &ret_code);
  plugin_type = (int)omp_get_interop_int(Interop, omp_ipr_fr_id, &ret_code);
  if (plugin_type == 6)
    plugin_type = INTEROP_PLUGIN_LEVEL0;
  else if (plugin_type == 3)
    plugin_type = INTEROP_PLUGIN_OPENCL;
  else
    DP("%d does not support interop plugin type \n", plugin_type);
  intel_ext_obj->plugin_interface = plugin_type;
  return Interop;
}

EXTERN int __tgt_release_interop_obj(void *interop_obj) {
  DP("Call to __tgt_release_interop_obj with interop_obj " DPxMOD "\n",
     DPxPTR(interop_obj));

  free(static_cast<__tgt_interop *>(interop_obj)->intel_tmp_ext);
  return __tgt_release_interop((omp_interop_t) interop_obj);

}

EXTERN int __tgt_set_interop_property(
    void *interop_obj, int32_t property_id, void *property_value) {
  DP("Call to __tgt_set_interop_property with interop_obj " DPxMOD
     ", property_id %" PRId32 "\n", DPxPTR(interop_obj), property_id);

  if (isOffloadDisabled() || !interop_obj || !property_value) {
    return OFFLOAD_FAIL;
  }

  __tgt_interop_obj * Interop = (__tgt_interop_obj *) ((__tgt_interop *) interop_obj)->intel_tmp_ext;

  // Currently we support setting async object only
  switch (property_id) {
  case INTEROP_ASYNC_OBJ:
    if (Interop->async_obj) {
       DP("Updating async obj is not allowed" PRId32 "\n");
       return OFFLOAD_FAIL;
    }
    Interop->async_obj = property_value;
    break;
  default:
    DP("Invalid interop property name " PRId32 "\n");
    return OFFLOAD_FAIL;
  }
  
  return OFFLOAD_SUCCESS;
}

EXTERN int __tgt_get_interop_property(
    void *interop_obj, int32_t property_id, void **property_value) {

  DP("Call to __tgt_get_interop_property with interop_obj " DPxMOD
     ", property_id %" PRId32 "\n", DPxPTR(interop_obj), property_id);

  int ret_code = OFFLOAD_FAIL;

  __tgt_interop_obj *ext_obj = ((__tgt_interop *)interop_obj)->intel_tmp_ext;
  switch (property_id) {
  case INTEROP_DEVICE_ID:
    *property_value = (void *)&ext_obj->device_id;
    ret_code = OFFLOAD_SUCCESS;
    break;
  case INTEROP_IS_ASYNC:
    *property_value = (void *)&ext_obj->is_async;
    ret_code = OFFLOAD_SUCCESS;
    break;
  case INTEROP_ASYNC_OBJ:
    *property_value = (void *)ext_obj->async_obj;
    ret_code = OFFLOAD_SUCCESS;
    break;
  case INTEROP_ASYNC_CALLBACK:
    *property_value = (void *)ext_obj->async_handler;
    break;
  case INTEROP_PLUGIN_INTERFACE:
    *property_value = (void *)&ext_obj->plugin_interface;
    ret_code = OFFLOAD_SUCCESS;
    break;
  case INTEROP_OFFLOAD_QUEUE:
    if (ext_obj->is_async)
       *property_value = (void *) omp_get_interop_ptr(interop_obj, omp_ipr_targetsync, &ret_code);
    else
       *property_value = interop_obj_queue_lists.at(ext_obj->device_id);
    break;
  case INTEROP_PLATFORM_HANDLE:
    // FOr level 0 return PLATFORM_HANDLE  for  OpenCL return CONTEXT_HANDLE
    if (ext_obj->plugin_interface == INTEROP_PLUGIN_LEVEL0) { 
       *property_value = (void *) omp_get_interop_ptr(interop_obj, omp_ipr_platform, &ret_code);
       break;
    }
    [[fallthrough]];
  case INTEROP_CONTEXT_HANDLE:
    *property_value = (void *) omp_get_interop_ptr(interop_obj, omp_ipr_device_context, &ret_code);
    break;
  case INTEROP_DEVICE_HANDLE:
    *property_value = (void *) omp_get_interop_ptr(interop_obj, omp_ipr_device, &ret_code);
    break;
  default:
    DP("Invalid interop property name " PRId32 "\n");
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

// End INTEL DISPATCH extension

#if INTEL_CUSTOMIZATION
EXTERN omp_interop_t __tgt_create_interop(
    int64_t device_num, int32_t interop_type, int32_t num_prefers,
    int32_t *prefer_ids) {
  DP("Call to %s with device_num %" PRId64 ", interop_type %" PRId32
     ", num_prefers %" PRId32 ", prefer_ids " DPxMOD "\n",
     __func__, device_num, interop_type, num_prefers, DPxPTR(prefer_ids));

  if (isOffloadDisabled())
    return omp_interop_none;

  omp_interop_t Interop = omp_interop_none;

  // Now, try to create an interop with device_num.
  if (device_num == OFFLOAD_DEVICE_DEFAULT)
    device_num = omp_get_default_device();

  if (deviceIsReady(device_num)) {
    Interop = PM->Devices[device_num]->createInterop(interop_type, num_prefers,
                                                    prefer_ids);
    DP("Created an interop " DPxMOD " from device_num %" PRId64 "\n",
       DPxPTR(Interop), device_num);
  }

  return Interop;
}

EXTERN int __tgt_release_interop(omp_interop_t interop) {
  DP("Call to %s with interop " DPxMOD "\n", __func__, DPxPTR(interop));

  if (isOffloadDisabled() || !interop)
    return OFFLOAD_FAIL;

  __tgt_interop *TgtInterop = static_cast<__tgt_interop *>(interop);
  int64_t DeviceNum = TgtInterop->DeviceNum;

  if (!deviceIsReady(DeviceNum)) {
    DP("Device %" PRId64 " is not ready when releasing an interop " DPxMOD "\n",
       DeviceNum, DPxPTR(interop));
    return OFFLOAD_FAIL;
  }

  return PM->Devices[DeviceNum]->releaseInterop(TgtInterop);
}

EXTERN int __tgt_use_interop(omp_interop_t interop) {
  DP("Call to %s with interop " DPxMOD "\n", __func__, DPxPTR(interop));

  if (isOffloadDisabled() || !interop)
    return OFFLOAD_FAIL;

  __tgt_interop *TgtInterop = static_cast<__tgt_interop *>(interop);
  int64_t DeviceNum = TgtInterop->DeviceNum;

  if (!deviceIsReady(DeviceNum)) {
    DP("Device %" PRId64 " is not ready when using an interop " DPxMOD "\n",
       DeviceNum, DPxPTR(interop));
    return OFFLOAD_FAIL;
  }

  if (!TgtInterop->TargetSync)
    return OFFLOAD_SUCCESS;

  return PM->Devices[DeviceNum]->useInterop(TgtInterop);
}
#endif // INTEL_CUSTOMIZATION

EXTERN int __tgt_get_target_memory_info(
    void *interop_obj, int32_t num_ptrs, void *tgt_ptrs, void *ptr_info) {
  DP("Call to __tgt_get_target_memory_info with interop_obj " DPxMOD
     ", num_ptrs %" PRId32 ", tgt_ptrs " DPxMOD ", ptr_info " DPxMOD
     "\n", DPxPTR(interop_obj), num_ptrs, DPxPTR(tgt_ptrs), DPxPTR(ptr_info));

  if (isOffloadDisabled() || !interop_obj || !tgt_ptrs || !ptr_info ||
      num_ptrs <= 0) {
    return OFFLOAD_FAIL;
  }

  __tgt_interop_obj *obj = static_cast<__tgt_interop_obj *>(interop_obj);
  DeviceTy &Device = *PM->Devices[obj->device_id];
  return Device.get_data_alloc_info(num_ptrs, tgt_ptrs, ptr_info);
}

#if INTEL_CUSTOMIZATION
EXTERN void __tgt_push_code_location(const char *location, void *codeptr_ra) {
  OmptGlobal->getTrace().pushCodeLocation(location, codeptr_ra);
  // Temporary workaround since code location directly passed with __tgt*
  // entries is incorrect.
  XPTIRegistry->pushCodeLocation(location);
}
#endif // INTEL_CUSTOMIZATION

EXTERN int __tgt_get_num_devices(void) {
  return omp_get_num_devices();
}

EXTERN void __tgt_add_build_options(
    const char *compile_options, const char *link_options) {

  int64_t device_num = omp_get_default_device();

  if (!deviceIsReady(device_num)) {
    REPORT("Device %" PRId64 " is not ready.\n", device_num);
    return;
  }

  auto RTLInfo = PM->Devices[device_num]->RTL;
  if (RTLInfo->add_build_options)
    RTLInfo->add_build_options(compile_options, link_options);
}

EXTERN int __tgt_target_supports_per_hw_thread_scratch(int64_t device_num) {
  if (checkDeviceAndCtors(device_num, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    handleTargetOutcome(false, nullptr);
    return 0;
  }

  return PM->Devices[device_num]->supportsPerHWThreadScratch();
}

EXTERN void *__tgt_target_alloc_per_hw_thread_scratch(
    int64_t device_num, size_t obj_size, int32_t alloc_kind) {
  if (obj_size == 0)
    return nullptr;

  if (checkDeviceAndCtors(device_num, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    handleTargetOutcome(false, nullptr);
    return nullptr;
  }

  return PM->Devices[device_num]->allocPerHWThreadScratch(obj_size, alloc_kind);
}

EXTERN void __tgt_target_free_per_hw_thread_scratch(
    int64_t device_num, void *ptr) {
  if (!ptr)
    return;

  if (checkDeviceAndCtors(device_num, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    handleTargetOutcome(false, nullptr);
    return;
  }

  return PM->Devices[device_num]->freePerHWThreadScratch(ptr);
}
#endif // INTEL_COLLAB
EXTERN void __tgt_set_info_flag(uint32_t NewInfoLevel) {
  std::atomic<uint32_t> &InfoLevel = getInfoLevelInternal();
  InfoLevel.store(NewInfoLevel);
  for (auto &R : PM->RTLs.AllRTLs) {
    if (R.set_info_flag)
      R.set_info_flag(NewInfoLevel);
  }
}

EXTERN int __tgt_print_device_info(int64_t DeviceId) {
  return PM->Devices[DeviceId]->printDeviceInfo(
      PM->Devices[DeviceId]->RTLDeviceID);
}
