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
#include "xpti_registry.h"
#endif // INTEL_CUSTOMIZATION

#include "OmptCallback.h"
#include "OmptInterface.h"
#include "device.h"
#include "omptarget.h"
#include "private.h"
#include "rtl.h"

#include "Utilities.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <type_traits>

#if INTEL_CUSTOMIZATION
#include <string.h>
extern bool isOffloadDisabled();
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
#endif // INTEL_CUSTOMIZATION
#ifdef OMPT_SUPPORT
using namespace llvm::omp::target::ompt;
#endif

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
#if INTEL_CUSTOMIZATION
  // Keep the old behavior since moving this to global constructor does not work
  // well with our software stack.
  std::call_once(PM->RTLs.InitFlag, &RTLsTy::loadRTLs, &PM->RTLs);
#else  // INTEL_CUSTOMIZATION
  if (PM->maybeDelayRegisterLib(Desc))
    return;
#endif // INTEL_CUSTOMIZATION

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

template <typename TargetAsyncInfoTy>
static inline void
targetData(ident_t *Loc, int64_t DeviceId, int32_t ArgNum, void **ArgsBase,
           void **Args, int64_t *ArgSizes, int64_t *ArgTypes,
           map_var_info_t *ArgNames, void **ArgMappers,
           TargetDataFuncPtrTy TargetDataFunction, const char *RegionTypeMsg,
           const char *RegionName) {
  static_assert(std::is_convertible_v<TargetAsyncInfoTy, AsyncInfoTy>,
                "TargetAsyncInfoTy must be convertible to AsyncInfoTy.");

  TIMESCOPE_WITH_RTM_AND_IDENT(RegionTypeMsg, Loc);

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  int64_t EncodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_CUSTOMIZATION

  DP("Entering data %s region for device %" PRId64 " with %d mappings\n",
     RegionName, DeviceId, ArgNum);

  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return;
  }

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(Loc, DeviceId, ArgNum, ArgSizes, ArgTypes, ArgNames,
                         RegionTypeMsg);
#ifdef OMPTARGET_DEBUG
  for (int I = 0; I < ArgNum; ++I) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       I, DPxPTR(ArgsBase[I]), DPxPTR(Args[I]), ArgSizes[I], ArgTypes[I],
       (ArgNames) ? getNameFromMapping(ArgNames[I]).c_str() : "unknown");
  }
#endif

#if INTEL_CUSTOMIZATION
  PM->Devices[DeviceId]->pushSubDevice(EncodedId, DeviceId);
#endif // INTEL_CUSTOMIZATION

  DeviceTy &Device = *PM->Devices[DeviceId];
  TargetAsyncInfoTy TargetAsyncInfo(Device);
  AsyncInfoTy &AsyncInfo = TargetAsyncInfo;

  /// RAII to establish tool anchors before and after data begin / end / update
  OMPT_IF_BUILT(assert((TargetDataFunction == targetDataBegin ||
                        TargetDataFunction == targetDataEnd ||
                        TargetDataFunction == targetDataUpdate) &&
                       "Encountered unexpected TargetDataFunction during "
                       "execution of targetData");
                auto CallbackFunctions =
                    (TargetDataFunction == targetDataBegin)
                        ? RegionInterface.getCallbacks<ompt_target_enter_data>()
                    : (TargetDataFunction == targetDataEnd)
                        ? RegionInterface.getCallbacks<ompt_target_exit_data>()
                        : RegionInterface.getCallbacks<ompt_target_update>();
                InterfaceRAII TargetDataRAII(CallbackFunctions, DeviceId,
                                             OMPT_GET_RETURN_ADDRESS(0));)

  int Rc = OFFLOAD_SUCCESS;
  Rc = TargetDataFunction(Loc, Device, ArgNum, ArgsBase, Args, ArgSizes,
                          ArgTypes, ArgNames, ArgMappers, AsyncInfo,
                          false /* FromMapper */);

  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();

  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);

#if INTEL_CUSTOMIZATION
  if (EncodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_CUSTOMIZATION
}

/// creates host-to-target data mapping, stores it in the
/// libomptarget.so internal structure (an entry in a stack of data maps)
/// and passes the data to the device.
EXTERN void __tgt_target_data_begin_mapper(ident_t *Loc, int64_t DeviceId,
                                           int32_t ArgNum, void **ArgsBase,
                                           void **Args, int64_t *ArgSizes,
                                           int64_t *ArgTypes,
                                           map_var_info_t *ArgNames,
                                           void **ArgMappers) {

  targetData<AsyncInfoTy>(Loc, DeviceId, ArgNum, ArgsBase, Args, ArgSizes,
                          ArgTypes, ArgNames, ArgMappers, targetDataBegin,
                          "Entering OpenMP data region with being_mapper",
                          "begin");
}

EXTERN void __tgt_target_data_begin_nowait_mapper(
    ident_t *Loc, int64_t DeviceId, int32_t ArgNum, void **ArgsBase,
    void **Args, int64_t *ArgSizes, int64_t *ArgTypes, map_var_info_t *ArgNames,
    void **ArgMappers, int32_t DepNum, void *DepList, int32_t NoAliasDepNum,
    void *NoAliasDepList) {

  targetData<TaskAsyncInfoWrapperTy>(
      Loc, DeviceId, ArgNum, ArgsBase, Args, ArgSizes, ArgTypes, ArgNames,
      ArgMappers, targetDataBegin,
      "Entering OpenMP data region with being_nowait_mapper", "begin");
}

/// passes data from the target, releases target memory and destroys
/// the host-target mapping (top entry from the stack of data maps)
/// created by the last __tgt_target_data_begin.
EXTERN void __tgt_target_data_end_mapper(ident_t *Loc, int64_t DeviceId,
                                         int32_t ArgNum, void **ArgsBase,
                                         void **Args, int64_t *ArgSizes,
                                         int64_t *ArgTypes,
                                         map_var_info_t *ArgNames,
                                         void **ArgMappers) {

  targetData<AsyncInfoTy>(Loc, DeviceId, ArgNum, ArgsBase, Args, ArgSizes,
                          ArgTypes, ArgNames, ArgMappers, targetDataEnd,
                          "Exiting OpenMP data region with end_mapper", "end");
}

EXTERN void __tgt_target_data_end_nowait_mapper(
    ident_t *Loc, int64_t DeviceId, int32_t ArgNum, void **ArgsBase,
    void **Args, int64_t *ArgSizes, int64_t *ArgTypes, map_var_info_t *ArgNames,
    void **ArgMappers, int32_t DepNum, void *DepList, int32_t NoAliasDepNum,
    void *NoAliasDepList) {

  targetData<TaskAsyncInfoWrapperTy>(
      Loc, DeviceId, ArgNum, ArgsBase, Args, ArgSizes, ArgTypes, ArgNames,
      ArgMappers, targetDataEnd,
      "Exiting OpenMP data region with end_nowait_mapper", "end");
}

EXTERN void __tgt_target_data_update_mapper(ident_t *Loc, int64_t DeviceId,
                                            int32_t ArgNum, void **ArgsBase,
                                            void **Args, int64_t *ArgSizes,
                                            int64_t *ArgTypes,
                                            map_var_info_t *ArgNames,
                                            void **ArgMappers) {

  targetData<AsyncInfoTy>(
      Loc, DeviceId, ArgNum, ArgsBase, Args, ArgSizes, ArgTypes, ArgNames,
      ArgMappers, targetDataUpdate,
      "Updating data within the OpenMP data region with update_mapper",
      "update");
}

EXTERN void __tgt_target_data_update_nowait_mapper(
    ident_t *Loc, int64_t DeviceId, int32_t ArgNum, void **ArgsBase,
    void **Args, int64_t *ArgSizes, int64_t *ArgTypes, map_var_info_t *ArgNames,
    void **ArgMappers, int32_t DepNum, void *DepList, int32_t NoAliasDepNum,
    void *NoAliasDepList) {
  targetData<TaskAsyncInfoWrapperTy>(
      Loc, DeviceId, ArgNum, ArgsBase, Args, ArgSizes, ArgTypes, ArgNames,
      ArgMappers, targetDataUpdate,
      "Updating data within the OpenMP data region with update_nowait_mapper",
      "update");
}

static KernelArgsTy *upgradeKernelArgs(KernelArgsTy *KernelArgs,
                                       KernelArgsTy &LocalKernelArgs,
                                       int32_t NumTeams, int32_t ThreadLimit) {
  if (KernelArgs->Version > 2)
    DP("Unexpected ABI version: %u\n", KernelArgs->Version);

  if (KernelArgs->Version == 1) {
    LocalKernelArgs.Version = 2;
    LocalKernelArgs.NumArgs = KernelArgs->NumArgs;
    LocalKernelArgs.ArgBasePtrs = KernelArgs->ArgBasePtrs;
    LocalKernelArgs.ArgPtrs = KernelArgs->ArgPtrs;
    LocalKernelArgs.ArgSizes = KernelArgs->ArgSizes;
    LocalKernelArgs.ArgTypes = KernelArgs->ArgTypes;
    LocalKernelArgs.ArgNames = KernelArgs->ArgNames;
    LocalKernelArgs.ArgMappers = KernelArgs->ArgMappers;
    LocalKernelArgs.Tripcount = KernelArgs->Tripcount;
    LocalKernelArgs.Flags = KernelArgs->Flags;
    LocalKernelArgs.DynCGroupMem = 0;
    LocalKernelArgs.NumTeams[0] = NumTeams;
    LocalKernelArgs.NumTeams[1] = 0;
    LocalKernelArgs.NumTeams[2] = 0;
    LocalKernelArgs.ThreadLimit[0] = ThreadLimit;
    LocalKernelArgs.ThreadLimit[1] = 0;
    LocalKernelArgs.ThreadLimit[2] = 0;
    return &LocalKernelArgs;
  }

  return KernelArgs;
}

template <typename TargetAsyncInfoTy>
static inline int targetKernel(ident_t *Loc, int64_t DeviceId, int32_t NumTeams,
                               int32_t ThreadLimit, void *HostPtr,
                               KernelArgsTy *KernelArgs) {
  static_assert(std::is_convertible_v<TargetAsyncInfoTy, AsyncInfoTy>,
                "Target AsyncInfoTy must be convertible to AsyncInfoTy.");

  TIMESCOPE_WITH_IDENT(Loc);

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
  int64_t EncodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_CUSTOMIZATION

  DP("Entering target region for device %" PRId64 " with entry point " DPxMOD
     "\n",
     DeviceId, DPxPTR(HostPtr));

  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return OMP_TGT_FAIL;
  }

  bool IsTeams = NumTeams != -1;
  if (!IsTeams)
    KernelArgs->NumTeams[0] = NumTeams = 1;

  // Auto-upgrade kernel args version 1 to 2.
  KernelArgsTy LocalKernelArgs;
  KernelArgs =
      upgradeKernelArgs(KernelArgs, LocalKernelArgs, NumTeams, ThreadLimit);

  assert(KernelArgs->NumTeams[0] == static_cast<uint32_t>(NumTeams) &&
         !KernelArgs->NumTeams[1] && !KernelArgs->NumTeams[2] &&
         "OpenMP interface should not use multiple dimensions");
  assert(KernelArgs->ThreadLimit[0] == static_cast<uint32_t>(ThreadLimit) &&
         !KernelArgs->ThreadLimit[1] && !KernelArgs->ThreadLimit[2] &&
         "OpenMP interface should not use multiple dimensions");

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(Loc, DeviceId, KernelArgs->NumArgs,
                         KernelArgs->ArgSizes, KernelArgs->ArgTypes,
                         KernelArgs->ArgNames, "Entering OpenMP kernel");
#ifdef OMPTARGET_DEBUG
  for (uint32_t I = 0; I < KernelArgs->NumArgs; ++I) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       I, DPxPTR(KernelArgs->ArgBasePtrs[I]), DPxPTR(KernelArgs->ArgPtrs[I]),
       KernelArgs->ArgSizes[I], KernelArgs->ArgTypes[I],
       (KernelArgs->ArgNames)
           ? getNameFromMapping(KernelArgs->ArgNames[I]).c_str()
           : "unknown");
  }
#endif

#if INTEL_CUSTOMIZATION
  // Push device encoding
  PM->Devices[DeviceId]->pushSubDevice(EncodedId, DeviceId);
#endif // INTEL_CUSTOMIZATION

  DeviceTy &Device = *PM->Devices[DeviceId];
  TargetAsyncInfoTy TargetAsyncInfo(Device);
  AsyncInfoTy &AsyncInfo = TargetAsyncInfo;
  /// RAII to establish tool anchors before and after target region
  OMPT_IF_BUILT(InterfaceRAII TargetRAII(
                    RegionInterface.getCallbacks<ompt_target>(), DeviceId,
                    /* CodePtr */ OMPT_GET_RETURN_ADDRESS(0));)

  int Rc = OFFLOAD_SUCCESS;
  Rc = target(Loc, Device, HostPtr, *KernelArgs, AsyncInfo);

  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();

  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);

#if INTEL_CUSTOMIZATION
  if (EncodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_CUSTOMIZATION

  assert(Rc == OFFLOAD_SUCCESS && "__tgt_target_kernel unexpected failure!");

  return OMP_TGT_SUCCESS;
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
                               KernelArgsTy *KernelArgs) {
#if INTEL_CUSTOMIZATION
  // TODO: investigate what it costs to enable the new (improved) scheme of
  // invoking asynchronous target region. We are using customization in all
  // places (codegen, host runtime, offload runtime), so we are not able to
  // adopt the new scheme right away.
#else  // INTEL_CUSTOMIZATION
  if (KernelArgs->Flags.NoWait)
    return targetKernel<TaskAsyncInfoWrapperTy>(
        Loc, DeviceId, NumTeams, ThreadLimit, HostPtr, KernelArgs);
  else
#endif // INTEL_CUSTOMIZATION
    return targetKernel<AsyncInfoTy>(Loc, DeviceId, NumTeams, ThreadLimit,
                                     HostPtr, KernelArgs);
}

/// Activates the record replay mechanism.
/// \param DeviceId The device identifier to execute the target region.
/// \param MemorySize The number of bytes to be (pre-)allocated
///                   by the bump allocator
/// /param IsRecord Activates the record replay mechanism in
///                 'record' mode or 'replay' mode.
/// /param SaveOutput Store the device memory after kernel
///                   execution on persistent storage
EXTERN int __tgt_activate_record_replay(int64_t DeviceId, uint64_t MemorySize,
                                        void *VAddr, bool IsRecord,
                                        bool SaveOutput) {
  if (!deviceIsReady(DeviceId)) {
    DP("Device %" PRId64 " is not ready\n", DeviceId);
    return OMP_TGT_FAIL;
  }

  DeviceTy &Device = *PM->Devices[DeviceId];
  [[maybe_unused]] int Rc =
      target_activate_rr(Device, MemorySize, VAddr, IsRecord, SaveOutput);
  assert(Rc == OFFLOAD_SUCCESS &&
         "__tgt_activate_record_replay unexpected failure!");
  return OMP_TGT_SUCCESS;
}

/// Implements a target kernel entry that replays a pre-recorded kernel.
/// \param Loc Source location associated with this target region (unused).
/// \param DeviceId The device identifier to execute the target region.
/// \param HostPtr A pointer to an address that uniquely identifies the kernel.
/// \param DeviceMemory A pointer to an array storing device memory data to move
///                     prior to kernel execution.
/// \param DeviceMemorySize The size of the above device memory data in bytes.
/// \param TgtArgs An array of pointers of the pre-recorded target kernel
///                arguments.
/// \param TgtOffsets An array of pointers of the pre-recorded target kernel
///                   argument offsets.
/// \param NumArgs The number of kernel arguments.
/// \param NumTeams Number of teams to launch the target region with.
/// \param ThreadLimit Limit to the number of threads to use in kernel
///                    execution.
/// \param LoopTripCount The pre-recorded value of the loop tripcount, if any.
/// \return OMP_TGT_SUCCESS on success, OMP_TGT_FAIL on failure.
EXTERN int __tgt_target_kernel_replay(ident_t *Loc, int64_t DeviceId,
                                      void *HostPtr, void *DeviceMemory,
                                      int64_t DeviceMemorySize, void **TgtArgs,
                                      ptrdiff_t *TgtOffsets, int32_t NumArgs,
                                      int32_t NumTeams, int32_t ThreadLimit,
                                      uint64_t LoopTripCount) {

  if (checkDeviceAndCtors(DeviceId, Loc)) {
    DP("Not offloading to device %" PRId64 "\n", DeviceId);
    return OMP_TGT_FAIL;
  }
  DeviceTy &Device = *PM->Devices[DeviceId];
  /// RAII to establish tool anchors before and after target region
  OMPT_IF_BUILT(InterfaceRAII TargetRAII(
                    RegionInterface.getCallbacks<ompt_target>(), DeviceId,
                    /* CodePtr */ OMPT_GET_RETURN_ADDRESS(0));)

  AsyncInfoTy AsyncInfo(Device);
  int Rc = target_replay(Loc, Device, HostPtr, DeviceMemory, DeviceMemorySize,
                         TgtArgs, TgtOffsets, NumArgs, NumTeams, ThreadLimit,
                         LoopTripCount, AsyncInfo);
  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();
  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);
  assert(Rc == OFFLOAD_SUCCESS &&
         "__tgt_target_kernel_replay unexpected failure!");
  return OMP_TGT_SUCCESS;
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

#if INTEL_CUSTOMIZATION
EXTERN int32_t __tgt_is_device_available(int64_t DeviceNum, void *DeviceType) {
  DeviceNum = EXTRACT_BITS(DeviceNum, 31, 0);
  if (checkDeviceAndCtors(DeviceNum, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", DeviceNum);
    handleTargetOutcome(false, nullptr);
    return false;
  }

  return PM->Devices[DeviceNum]->isSupportedDevice(DeviceType);
}

EXTERN char *__tgt_get_device_name(
    int64_t DeviceNum, char *Buffer, size_t BufferMaxSize) {
  DP("Call to __tgt_get_device_name with device number %" PRId64
     " and max buffer size %zu.\n", DeviceNum, BufferMaxSize);

  if (!Buffer || BufferMaxSize == 0 || isOffloadDisabled())
    return NULL;

  if (checkDeviceAndCtors(DeviceNum, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", DeviceNum);
    handleTargetOutcome(false, nullptr);
    return NULL;
  }

  DP("Querying device for its name.\n");

  DeviceTy &Device = *PM->Devices[DeviceNum];
  return Device.getDeviceName(Buffer, BufferMaxSize);
}

EXTERN char *__tgt_get_device_rtl_name(
    int64_t DeviceNum, char *Buffer, size_t BufferMaxSize) {
  DP("Call to __tgt_get_device_rtl_name with device_num %" PRId64
     " and max buffer size %zu.\n", DeviceNum, BufferMaxSize);

  if (!Buffer || BufferMaxSize == 0 || isOffloadDisabled())
    return NULL;

  if (checkDeviceAndCtors(DeviceNum, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", DeviceNum);
    handleTargetOutcome(false, nullptr);
    return NULL;
  }

  const RTLInfoTy *RTL = PM->Devices[DeviceNum]->RTL;
  assert(RTL && "Device with uninitialized RTL.");
  strncpy(Buffer, RTL->RTLConstName, BufferMaxSize - 1);
  Buffer[BufferMaxSize - 1] = '\0';
  return Buffer;
}

// Begin INTEL DISPATCH extension
// The following code upto "End INTEL DISPATCH extension should be deleted
// once we obsolete INTEL DISPATCH extension
EXTERN void __tgt_offload_proxy_task_complete_ooo(void *InteropObj) {
  DP("Call to __tgt_offload_proxy_task_complete_ooo with interop obj " DPxMOD
     "\n", DPxPTR(InteropObj));
  void *AsyncObj = ((__tgt_interop *)InteropObj)->IntelTmpExt->AsyncObj;
  (void)__tgt_release_interop(InteropObj);
  __kmpc_proxy_task_completed_ooo(AsyncObj);
}

// TEMPORARY Remove once Intel interop extension is obsoleted
// Declared in api.cpp
EXTERN omp_intptr_t omp_get_interop_int(const omp_interop_t Interop,
    omp_interop_property_t PropertyId, int *RetCode);
EXTERN void *omp_get_interop_ptr(const omp_interop_t Interop,
    omp_interop_property_t PropertyId, int *RetCode);

// Use single queue for Intel dispatch when is_async is false
static std::map<int32_t, void *> InteropObjQueues;

// END TEMPORARY

EXTERN void *__tgt_create_interop_obj(
    int64_t DeviceCode, bool IsAsync, void *AsyncObj) {
  int64_t DeviceId = EXTRACT_BITS(DeviceCode, 31, 0);
  int PlugInType;
  omp_interop_t Interop;

  bool QueueFound = InteropObjQueues.find(DeviceId) != InteropObjQueues.end();

  if (IsAsync || !QueueFound)
    Interop =
        __tgt_create_interop(DeviceId, OMP_INTEROP_CONTEXT_TARGETSYNC, 0, NULL);
  else
    Interop =
        __tgt_create_interop(DeviceId, OMP_INTEROP_CONTEXT_TARGET, 0, NULL);
  if (!Interop)
    return NULL;

  // Save the queue created for reuse  later
  if (!IsAsync && !QueueFound) {
    // Save the queue for reuse and set TargetSync to NULL so the queue
    // is not destroyed in plugin when interop obj is released.
    int RC = OFFLOAD_FAIL;
    InteropObjQueues[DeviceId] =
        (void *)omp_get_interop_ptr(Interop, omp_ipr_targetsync, &RC);
    ((__tgt_interop *)Interop)->TargetSync= NULL;
  }

  __tgt_interop_obj *ExtObj =
      (__tgt_interop_obj *)malloc(sizeof(__tgt_interop_obj));
  if (!ExtObj)
    return NULL;

  ((__tgt_interop *)Interop)->IntelTmpExt = ExtObj;

  ExtObj->IsAsync = IsAsync;
  ExtObj->AsyncObj = AsyncObj;
  ExtObj->AsyncHandler = &__tgt_offload_proxy_task_complete_ooo;
  int RC = OFFLOAD_FAIL;
  ExtObj->DeviceId = omp_get_interop_int(Interop, omp_ipr_device_num, &RC);
  PlugInType = (int)omp_get_interop_int(Interop, omp_ipr_fr_id, &RC);
  if (PlugInType == 6)
    PlugInType = INTEROP_PLUGIN_LEVEL0;
  else if (PlugInType == 3)
    PlugInType = INTEROP_PLUGIN_OPENCL;
  else
    DP("%d does not support interop plugin type \n", PlugInType);
  ExtObj->PlugInType = PlugInType;
  return Interop;
}

EXTERN int __tgt_release_interop_obj(void *InteropObj) {
  DP("Call to __tgt_release_interop_obj with interop object " DPxMOD "\n",
     DPxPTR(InteropObj));

  free(static_cast<__tgt_interop *>(InteropObj)->IntelTmpExt);
  return __tgt_release_interop((omp_interop_t)InteropObj);
}

EXTERN int __tgt_set_interop_property(
    void *InteropObj, int32_t PropertyId, void *PropertyValue) {
  DP("Call to __tgt_set_interop_property with interop object " DPxMOD
     ", property ID %" PRId32 "\n", DPxPTR(InteropObj), PropertyId);

  if (isOffloadDisabled() || !InteropObj || !PropertyValue) {
    return OFFLOAD_FAIL;
  }

  __tgt_interop_obj *Interop =
      (__tgt_interop_obj *)((__tgt_interop *)InteropObj)->IntelTmpExt;

  // Currently we support setting async object only
  switch (PropertyId) {
  case INTEROP_ASYNC_OBJ:
    if (Interop->AsyncObj) {
      DP("Updating async object is not allowed" PRId32 "\n");
      return OFFLOAD_FAIL;
    }
    Interop->AsyncObj = PropertyValue;
    break;
  default:
    DP("Invalid interop property name " PRId32 "\n");
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int __tgt_get_interop_property(
    void *InteropObj, int32_t PropertyId, void **PropertyValue) {

  DP("Call to __tgt_get_interop_property with interop object " DPxMOD
     ", property ID %" PRId32 "\n", DPxPTR(InteropObj), PropertyId);

  int RC = OFFLOAD_FAIL;
  *PropertyValue = 0;

  __tgt_interop *Interop = (__tgt_interop *)InteropObj;
  __tgt_interop_obj *ExtObj = ((__tgt_interop *)InteropObj)->IntelTmpExt;

  static int interop_plugin_level0_val = INTEROP_PLUGIN_LEVEL0;
  static int interop_plugin_opencl_val = INTEROP_PLUGIN_OPENCL;

  switch (PropertyId) {
  case INTEROP_DEVICE_ID:
    *PropertyValue = (void *)&Interop->DeviceNum;
    RC = OFFLOAD_SUCCESS;
    break;
  case INTEROP_IS_ASYNC:
    if ( ExtObj ) {
      *PropertyValue = (void *)&ExtObj->IsAsync;
      RC = OFFLOAD_SUCCESS;
    } else return OFFLOAD_FAIL;
    break;
  case INTEROP_ASYNC_OBJ:
    if ( ExtObj ) {
      *PropertyValue = (void *)ExtObj->AsyncObj;
      RC = OFFLOAD_SUCCESS;
    } else return OFFLOAD_FAIL;
    break;
  case INTEROP_ASYNC_CALLBACK:
    if ( ExtObj ) {
      *PropertyValue = (void *)ExtObj->AsyncHandler;
    } else return OFFLOAD_FAIL;
    break;
  case INTEROP_PLUGIN_INTERFACE:
    if ( ExtObj ) {
      *PropertyValue = (void *)&ExtObj->PlugInType;
    } else {
      int PlugInType = Interop->FrId;
      if (PlugInType == 6)
         *PropertyValue = (void *)&interop_plugin_level0_val;
      else if (PlugInType == 3)
         *PropertyValue = (void *)&interop_plugin_opencl_val;
      else return OFFLOAD_FAIL;
    }
    RC = OFFLOAD_SUCCESS;
    break;
  case INTEROP_OFFLOAD_QUEUE:
    if (!ExtObj || ExtObj->IsAsync)
      *PropertyValue =
          (void *)omp_get_interop_ptr(InteropObj, omp_ipr_targetsync, &RC);
    else
      *PropertyValue = InteropObjQueues.at(ExtObj->DeviceId);
    break;
  case INTEROP_PLATFORM_HANDLE:
    // For level zero & SYCL return PLATFORM_HANDLE, for OpenCL return CONTEXT_HANDLE
    if ( Interop->FrId == 6 || Interop->FrId == 4 ) {
      *PropertyValue =
          (void *)omp_get_interop_ptr(InteropObj, omp_ipr_platform, &RC);
      break;
    }
    [[fallthrough]];
  case INTEROP_CONTEXT_HANDLE:
    *PropertyValue =
        (void *)omp_get_interop_ptr(InteropObj, omp_ipr_device_context, &RC);
    break;
  case INTEROP_DEVICE_HANDLE:
    *PropertyValue =
        (void *)omp_get_interop_ptr(InteropObj, omp_ipr_device, &RC);
    break;
  default:
    DP("Invalid interop property name " PRId32 "\n");
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

// End INTEL DISPATCH extension

EXTERN omp_interop_t __tgt_create_interop(
    int64_t DeviceNum, int32_t InteropType, int32_t NumPrefers,
    int32_t *PreferIds) {
  DP("Call to %s with device_num %" PRId64 ", interop type %" PRId32
     ", number of preferred IDs %" PRId32 ", preferred IDs " DPxMOD "\n",
     __func__, DeviceNum, InteropType, NumPrefers, DPxPTR(PreferIds));

  if (isOffloadDisabled())
    return omp_interop_none;

  omp_interop_t Interop = omp_interop_none;

  // Now, try to create an interop with DeviceNum.
  if (DeviceNum == OFFLOAD_DEVICE_DEFAULT)
    DeviceNum = omp_get_default_device();

  if (deviceIsReady(DeviceNum)) {
    Interop = PM->Devices[DeviceNum]->createInterop(InteropType, NumPrefers,
                                                    PreferIds);
    DP("Created an interop " DPxMOD " from device number %" PRId64 "\n",
       DPxPTR(Interop), DeviceNum);
  }

  return Interop;
}

EXTERN omp_interop_t __tgt_get_interop_obj(
    ident_t *loc_ref, int32_t interop_type, uint32_t num_prefers,
    int32_t *prefer_ids, int64_t device_num, int gtid, void *current_task) {

  DP("Call to %s with device_num %" PRId64 ", interop_type %" PRId32
     ", num_prefers %" PRId32 ", prefer_ids " DPxMOD ", gtid %" PRId32
     ", current_task " DPxMOD "\n",
     __func__, device_num, interop_type, num_prefers, DPxPTR(prefer_ids), gtid,
     DPxPTR(current_task));

  if (isOffloadDisabled())
    return omp_interop_none;

  omp_interop_t Interop = omp_interop_none;

  // Now, try to create an interop with device_num.
  if (device_num == OFFLOAD_DEVICE_DEFAULT)
    device_num = omp_get_default_device();

  if (deviceIsReady(device_num)) {
    auto first = PM->InteropTbl.begin(gtid, current_task);
    auto last = PM->InteropTbl.end(gtid, current_task);
    __tgt_interop *tiop = NULL;
    for (auto iop = first; iop != last; ++iop) {
      if ((*iop)->isCompatibleWith(interop_type, num_prefers, prefer_ids,
                                   device_num, gtid, current_task)) {
        tiop = *iop;
        tiop->markDirty();
        DP("Reused interop " DPxMOD " from device_num %" PRId64 "\n",
           DPxPTR(tiop), device_num);
        break;
      }
    }
    if (!tiop) {
      tiop = PM->Devices[device_num]->createInterop(interop_type, num_prefers,
                                                    prefer_ids);
      if (tiop) {
        DP("Created an interop " DPxMOD " from device_num %" PRId64 "\n",
           DPxPTR(tiop), device_num);
        tiop->setOwner(gtid, current_task);
        PM->InteropTbl.addInterop(tiop);
      }
    }
    Interop = tiop;
  }

  return Interop;
}

EXTERN void __tgt_target_sync(ident_t *loc_ref, int gtid, void *current_task,
                              void *event) {
  auto first = PM->InteropTbl.begin(gtid, current_task);
  auto last = PM->InteropTbl.end(gtid, current_task);

  if (first == last)
    return;

  DP("Processing target_sync for gtid %" PRId32 ", current_task " DPxMOD
     " event " DPxMOD "\n",
     gtid, DPxPTR(current_task), DPxPTR(event));

  for (auto it = first; it != last; ++it) {
    __tgt_interop *iop = *it;
    if (iop->TargetSync != NULL && iop->isOwnedBy(gtid, current_task) &&
        !iop->isClean()) {

      iop->flush();

      // Implementation option 1
      iop->syncBarrier();
      iop->markClean();

      // Alternate implementation option
      // event = iop->asyncBarrier();
      // ptask = createProxyTask();
      // Events->add(event,ptask);
    }
  }
  // This would be needed for the alternate implementation
  // processEvents();
}

EXTERN int __tgt_interop_use_async(ident_t *loc_ref, int gtid,
                                   omp_interop_t interop, bool nowait,
                                   void *ptask) {
  DP("Call to %s with interop " DPxMOD ", nowait %" PRId32 "\n", __func__,
     DPxPTR(interop), nowait);

  if (isOffloadDisabled() || !interop)
    return OFFLOAD_FAIL;

  __tgt_interop *iop = static_cast<__tgt_interop *>(interop);
  if (iop->TargetSync) {
    if (nowait)
      iop->asyncBarrier();
    else {
      iop->flush();
      iop->syncBarrier();
      iop->markClean();
    }
  }

  return OFFLOAD_SUCCESS;
}

EXTERN int __tgt_release_interop(omp_interop_t Interop) {
  DP("Call to %s with interop " DPxMOD "\n", __func__, DPxPTR(Interop));

  if (isOffloadDisabled() || !Interop)
    return OFFLOAD_FAIL;

  __tgt_interop *TgtInterop = static_cast<__tgt_interop *>(Interop);
  int64_t DeviceNum = TgtInterop->DeviceNum;

  if (!deviceIsReady(DeviceNum)) {
    DP("Device %" PRId64 " is not ready when releasing an interop " DPxMOD "\n",
       DeviceNum, DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  return PM->Devices[DeviceNum]->releaseInterop(TgtInterop);
}

EXTERN int __tgt_use_interop(omp_interop_t Interop) {
  DP("Call to %s with interop " DPxMOD "\n", __func__, DPxPTR(Interop));

  if (isOffloadDisabled() || !Interop)
    return OFFLOAD_FAIL;

  __tgt_interop *TgtInterop = static_cast<__tgt_interop *>(Interop);
  int64_t DeviceNum = TgtInterop->DeviceNum;

  if (!deviceIsReady(DeviceNum)) {
    DP("Device %" PRId64 " is not ready when using an interop " DPxMOD "\n",
       DeviceNum, DPxPTR(Interop));
    return OFFLOAD_FAIL;
  }

  if (!TgtInterop->TargetSync)
    return OFFLOAD_SUCCESS;

  return PM->Devices[DeviceNum]->useInterop(TgtInterop);
}

EXTERN int __tgt_get_target_memory_info(
    void *InteropObj, int32_t NumPtrs, void *TgtPtrs, void *PtrInfo) {
  DP("Call to __tgt_get_target_memory_info with interop object " DPxMOD
     ", num of pointers %" PRId32 ", target pointers " DPxMOD
     ", pointer info " DPxMOD "\n",
     DPxPTR(InteropObj), NumPtrs, DPxPTR(TgtPtrs), DPxPTR(PtrInfo));

  if (isOffloadDisabled() || !InteropObj || !TgtPtrs || !PtrInfo ||
      NumPtrs <= 0) {
    return OFFLOAD_FAIL;
  }

  __tgt_interop_obj *Obj = static_cast<__tgt_interop_obj *>(InteropObj);
  DeviceTy &Device = *PM->Devices[Obj->DeviceId];
  return Device.getDataAllocInfo(NumPtrs, TgtPtrs, PtrInfo);
}

EXTERN void __tgt_push_code_location(const char *Loc, void *CodePtrRA) {
  // Temporary workaround since code location directly passed with __tgt*
  // entries is incorrect.
  XPTIRegistry->pushCodeLocation(Loc);
#ifdef OMPT_SUPPORT
  RegionInterface.CodeLocation = Loc;
#endif // OMPT_SUPPORT
}

EXTERN int __tgt_get_num_devices(void) { return omp_get_num_devices(); }
#endif // INTEL_CUSTOMIZATION

EXTERN void __tgt_set_info_flag(uint32_t NewInfoLevel) {
  std::atomic<uint32_t> &InfoLevel = getInfoLevelInternal();
  InfoLevel.store(NewInfoLevel);
  for (auto &R : PM->RTLs.AllRTLs) {
    if (R.set_info_flag)
      R.set_info_flag(NewInfoLevel);
  }
}

EXTERN int __tgt_print_device_info(int64_t DeviceId) {
  // Make sure the device is ready.
  if (!deviceIsReady(DeviceId)) {
    DP("Device %" PRId64 " is not ready\n", DeviceId);
    return OMP_TGT_FAIL;
  }

  return PM->Devices[DeviceId]->printDeviceInfo(
      PM->Devices[DeviceId]->RTLDeviceID);
}

#if INTEL_CUSTOMIZATION
typedef void* (*omp_create_task_fptr)(int);
typedef void (*omp_complete_task_fptr)(int, void *);

EXTERN void __tgt_register_ptask_services(omp_create_task_fptr createf,
                                          omp_complete_task_fptr completef) {
  DP("Callback to __tgt_register_ptask_services with handlers " DPxMOD
     " " DPxMOD "\n",
     DPxPTR(createf), DPxPTR(completef));
}

EXTERN void __tgt_task_completed(void *task) {
  DP("Callback to _tgt_task_completed task=" DPxMOD "\n", DPxPTR(task));
}
#endif // INTEL_CUSTOMIZATION

EXTERN void __tgt_target_nowait_query(void **AsyncHandle) {
  if (!AsyncHandle || !*AsyncHandle) {
    FATAL_MESSAGE0(
        1, "Receive an invalid async handle from the current OpenMP task. Is "
           "this a target nowait region?\n");
  }

  // Exponential backoff tries to optimally decide if a thread should just query
  // for the device operations (work/spin wait on them) or block until they are
  // completed (use device side blocking mechanism). This allows the runtime to
  // adapt itself when there are a lot of long-running target regions in-flight.
  using namespace llvm::omp::target;
  static thread_local ExponentialBackoff QueryCounter(
      Int64Envar("OMPTARGET_QUERY_COUNT_MAX", 10),
      Int64Envar("OMPTARGET_QUERY_COUNT_THRESHOLD", 5),
      Envar<float>("OMPTARGET_QUERY_COUNT_BACKOFF_FACTOR", 0.5f));

  auto *AsyncInfo = (AsyncInfoTy *)*AsyncHandle;

  // If the thread is actively waiting on too many target nowait regions, we
  // should use the blocking sync type.
  if (QueryCounter.isAboveThreshold())
    AsyncInfo->SyncType = AsyncInfoTy::SyncTy::BLOCKING;

  if (const int Rc = AsyncInfo->synchronize())
    FATAL_MESSAGE0(1, "Error while querying the async queue for completion.\n");
  // If there are device operations still pending, return immediately without
  // deallocating the handle and increase the current thread query count.
  if (!AsyncInfo->isDone()) {
    QueryCounter.increment();
    return;
  }

  // When a thread successfully completes a target nowait region, we
  // exponentially backoff its query counter by the query factor.
  QueryCounter.decrement();

  // Delete the handle and unset it from the OpenMP task data.
  delete AsyncInfo;
  *AsyncHandle = nullptr;
}

#if INTEL_CUSTOMIZATION
EXTERN int __tgt_get_mem_resources(int32_t NumDevices, const int32_t *DeviceIds,
                                   int32_t HostAccess,
                                   omp_memspace_handle_t MemSpace,
                                   int32_t *ResourceIds) {
  // Check if input is correct.
  int32_t RTLNumDevices = __tgt_get_num_devices();
  if (NumDevices > RTLNumDevices) {
    DP("Invalid number of devices requested\n");
    return 0;
  }
  for (int32_t I = 0; I < NumDevices; I++) {
    if (DeviceIds[I] >= RTLNumDevices || !deviceIsReady(DeviceIds[I])) {
      DP("Device %" PRId32 " is invalid or not ready\n", DeviceIds[I]);
      return 0;
    }
  }
  auto *RTL = PM->Devices[DeviceIds[0]]->RTL;
  if (!RTL->get_mem_resources)
    return 0;
  return RTL->get_mem_resources(NumDevices, DeviceIds, HostAccess, MemSpace,
                                ResourceIds);
}

EXTERN void *__tgt_omp_alloc(size_t Size, omp_allocator_handle_t Allocator) {
  if (Allocator <= kmp_max_mem_alloc) {
    DP("Predefined allocator is not allowed here\n");
    return nullptr;
  }
  // Assume the allocator is tied to a memory space with a list of resources.
  auto *MemAlloc = reinterpret_cast<kmp_allocator_t *>(Allocator);
  auto *MemSpace = reinterpret_cast<kmp_memspace_t *>(MemAlloc->memspace);
  if (!MemSpace || MemSpace->num_resources <= 0 || !MemSpace->resources) {
    DP("Invalid memory space\n");
    return nullptr;
  }
  // Check if all devices are ready.
  int32_t NumDevices = __tgt_get_num_devices();
  for (int32_t I = 0; I < NumDevices; I++) {
    if (!deviceIsReady(I)) {
      DP("Device %" PRId32 " is not ready\n", I);
      return nullptr;
    }
  }
  auto *RTL = PM->Devices[0]->RTL;
  if (RTL->omp_alloc)
    return RTL->omp_alloc(Size, Allocator);
  return nullptr;
}

EXTERN void __tgt_omp_free(void *Ptr, omp_allocator_handle_t Allocator) {
  if (Allocator <= kmp_max_mem_alloc) {
    DP("Predefined allocator is not allowed here\n");
    return;
  }
  // Assume the allocator is tied to a memory space with a list of resources.
  auto *MemAlloc = reinterpret_cast<kmp_allocator_t *>(Allocator);
  auto *MemSpace = reinterpret_cast<kmp_memspace_t *>(MemAlloc->memspace);
  if (!MemSpace || MemSpace->num_resources <= 0 || !MemSpace->resources) {
    DP("Invalid memory space\n");
    return;
  }
  // Check if all devices are ready.
  int32_t NumDevices = __tgt_get_num_devices();
  for (int32_t I = 0; I < NumDevices; I++) {
    if (!deviceIsReady(I)) {
      DP("Device %" PRId32 " is not ready\n", I);
      return;
    }
  }
  auto *RTL = PM->Devices[0]->RTL;
  if (RTL->omp_free)
    RTL->omp_free(Ptr, Allocator);
}
#endif // INTEL_CUSTOMIZATION
