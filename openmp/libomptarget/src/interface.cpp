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
  int64_t EncodedId = GetEncodedDeviceID(DeviceId);
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
  Device.pushSubDevice(EncodedId, DeviceId);
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
  if (EncodedId != DeviceId)
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
  int64_t EncodedId = GetEncodedDeviceID(DeviceId);
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
  Device.pushSubDevice(EncodedId, DeviceId);
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
  if (EncodedId != DeviceId)
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
  int64_t EncodedId = GetEncodedDeviceID(DeviceId);
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
  Device.pushSubDevice(EncodedId, DeviceId);
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
  if (EncodedId != DeviceId)
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

#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(Loc, __func__);
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  int64_t EncodedId = GetEncodedDeviceID(DeviceId);
#endif // INTEL_COLLAB

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

#if INTEL_COLLAB
  // Push device encoding
  PM->Devices[DeviceId]->pushSubDevice(EncodedId, DeviceId);
#endif // INTEL_COLLAB

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetBegin(DeviceId));
#endif // INTEL_CUSTOMIZATION

  bool IsTeams = NumTeams != -1;
  if (!IsTeams)
    NumTeams = 0;

  DeviceTy &Device = *PM->Devices[DeviceId];
  AsyncInfoTy AsyncInfo(Device);
  int Rc = target(Loc, Device, HostPtr, Args->NumArgs, Args->ArgBasePtrs,
                  Args->ArgPtrs, Args->ArgSizes, Args->ArgTypes, Args->ArgNames,
                  Args->ArgMappers, NumTeams, ThreadLimit, Args->Tripcount,
                  IsTeams, AsyncInfo);
  if (Rc == OFFLOAD_SUCCESS)
    Rc = AsyncInfo.synchronize();
  handleTargetOutcome(Rc == OFFLOAD_SUCCESS, Loc);

#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetEnd(DeviceId));
#endif // INTEL_CUSTOMIZATION

#if INTEL_COLLAB
  if (EncodedId != DeviceId)
    PM->Devices[DeviceId]->popSubDevice();
#endif // INTEL_COLLAB

  assert(Rc == OFFLOAD_SUCCESS && "__tgt_target_kernel unexpected failure!");
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

#if INTEL_COLLAB
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
    // For level zero return PLATFORM_HANDLE, for OpenCL return CONTEXT_HANDLE
    if ( Interop->FrId == 6 ) {
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

#if INTEL_CUSTOMIZATION
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

EXTERN omp_interop_t __tgt_get_interop_obj (
    ident_t *loc_ref, int32_t interop_type, uint32_t num_prefers,
    int32_t *prefer_ids, int64_t device_num, int gtid, void *current_task ) {

   DP("Call to %s with device_num %" PRId64 ", interop_type %" PRId32
      ", num_prefers %" PRId32 ", prefer_ids " DPxMOD ", gtid %" PRId32
      ", current_task " DPxMOD "\n",
      __func__, device_num, interop_type, num_prefers, DPxPTR(prefer_ids),gtid,DPxPTR(current_task));

   if (isOffloadDisabled())
     return omp_interop_none;
 
   omp_interop_t Interop = omp_interop_none;

   // Now, try to create an interop with device_num.
   if (device_num == OFFLOAD_DEVICE_DEFAULT)
     device_num = omp_get_default_device();

   if (deviceIsReady(device_num)) {
     auto first = PM->InteropTbl.begin(gtid,current_task);
     auto last = PM->InteropTbl.end(gtid,current_task);
     __tgt_interop * tiop = NULL;
     for ( auto iop = first ; iop != last ; ++iop ) {
       if ( (*iop)->isCompatibleWith (interop_type, num_prefers, prefer_ids, device_num, gtid, current_task) ) {
         tiop = *iop;
         DP("Reused interop " DPxMOD " from device_num %" PRId64 "\n",
            DPxPTR(tiop), device_num);
         break;
       }
     }
     if ( !tiop ) {
       tiop = PM->Devices[device_num]->createInterop(interop_type, num_prefers, prefer_ids);
       if ( tiop ) {
          DP("Created an interop " DPxMOD " from device_num %" PRId64 "\n",
             DPxPTR(tiop), device_num);
          tiop->setOwner(gtid,current_task);
          PM->InteropTbl.addInterop(tiop);
       }
     }
     Interop = tiop;
   }

   return Interop;
}

EXTERN void __tgt_target_sync ( ident_t *loc_ref, int gtid, void * current_task, void *event ) {
   DP("Call to %s with gtid %" PRId32 ", current_task " DPxMOD "event " DPxMOD
      "\n", __func__, gtid, DPxPTR(current_task), DPxPTR(event));

   auto first = PM->InteropTbl.begin(gtid,current_task);
   auto last = PM->InteropTbl.end(gtid,current_task);
   for ( auto it = first ; it != last ; ++it ) {
      __tgt_interop *iop = *it;
      if ( iop->TargetSync != NULL &&
           iop->isOwnedBy ( gtid, current_task ) &&
           !iop->isClean() ) {

        iop->flush();

        // Implementation option 1
        iop->syncBarrier();
        iop->markClean();

        // Alternate implementation option
        //event = iop->asyncBarrier();
        // ptask = createProxyTask();
        //Events->add(event,ptask);
      }
   }
   // This would be needed for the alternate implementation
   // processEvents();
}

EXTERN int __tgt_interop_use_async ( ident_t *loc_ref, int gtid, omp_interop_t interop, bool nowait, void *ptask ) {
   DP("Call to %s with interop " DPxMOD ", nowait %" PRId32 "\n", __func__, DPxPTR(interop), nowait);

   __tgt_interop * iop = static_cast<__tgt_interop *>(interop);
   if ( iop->TargetSync ) {
     // async still not supported
     //if (nowait) iop->asyncBarrier();
     //else {
     iop->flush();
     iop->syncBarrier();
     //}
     iop->markClean();
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

#endif // INTEL_CUSTOMIZATION

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

#if INTEL_CUSTOMIZATION
EXTERN void __tgt_push_code_location(const char *Loc, void *CodePtrRA) {
  OmptGlobal->getTrace().pushCodeLocation(Loc, CodePtrRA);
  // Temporary workaround since code location directly passed with __tgt*
  // entries is incorrect.
  XPTIRegistry->pushCodeLocation(Loc);
}
#endif // INTEL_CUSTOMIZATION

EXTERN int __tgt_get_num_devices(void) {
  return omp_get_num_devices();
}

EXTERN void __tgt_add_build_options(
    const char *CompileOptions, const char *LinkOptions) {

  int64_t DeviceNum = omp_get_default_device();

  if (!deviceIsReady(DeviceNum)) {
    REPORT("Device %" PRId64 " is not ready.\n", DeviceNum);
    return;
  }

  auto RTLInfo = PM->Devices[DeviceNum]->RTL;
  if (RTLInfo->add_build_options)
    RTLInfo->add_build_options(CompileOptions, LinkOptions);
}

EXTERN int __tgt_target_supports_per_hw_thread_scratch(int64_t DeviceNum) {
  if (checkDeviceAndCtors(DeviceNum, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", DeviceNum);
    handleTargetOutcome(false, nullptr);
    return 0;
  }

  return PM->Devices[DeviceNum]->supportsPerHWThreadScratch();
}

EXTERN void *__tgt_target_alloc_per_hw_thread_scratch(
    int64_t DeviceNum, size_t ObjSize, int32_t AllocKind) {
  if (ObjSize == 0)
    return nullptr;

  if (checkDeviceAndCtors(DeviceNum, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", DeviceNum);
    handleTargetOutcome(false, nullptr);
    return nullptr;
  }

  return PM->Devices[DeviceNum]->allocPerHWThreadScratch(ObjSize, AllocKind);
}

EXTERN void __tgt_target_free_per_hw_thread_scratch(
    int64_t DeviceNum, void *Ptr) {
  if (!Ptr)
    return;

  if (checkDeviceAndCtors(DeviceNum, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", DeviceNum);
    handleTargetOutcome(false, nullptr);
    return;
  }

  return PM->Devices[DeviceNum]->freePerHWThreadScratch(Ptr);
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

#if INTEL_CUSTOMIZATION
typedef void* (*omp_create_task_fptr)(int);
typedef void (*omp_complete_task_fptr)(int, void *);

EXTERN void __tgt_register_ptask_services ( omp_create_task_fptr createf, 
                                            omp_complete_task_fptr completef)
{
    DP("Callback to __tgt_register_ptask_services with handlers " DPxMOD " " DPxMOD "\n",DPxPTR(createf),DPxPTR(completef));
}

EXTERN void __tgt_task_completed ( void * task )
{
    DP("Callback to _tgt_task_completed task=" DPxMOD "\n",DPxPTR(task));
}
#endif
