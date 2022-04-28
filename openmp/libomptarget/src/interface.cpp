//===-------- interface.cpp - Target independent OpenMP target RTL --------===//
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
static uint32_t useSingleQueue = 0;

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
EXTERN void __tgt_register_requires(int64_t flags) {
  TIMESCOPE();
  PM->RTLs.RegisterRequires(flags);
}

////////////////////////////////////////////////////////////////////////////////
/// adds a target shared library to the target execution image
EXTERN void __tgt_register_lib(__tgt_bin_desc *desc) {
  TIMESCOPE();
  std::call_once(PM->RTLs.initFlag, &RTLsTy::LoadRTLs, &PM->RTLs);
  for (auto &RTL : PM->RTLs.AllRTLs) {
    if (RTL.register_lib) {
      if ((*RTL.register_lib)(desc) != OFFLOAD_SUCCESS) {
        DP("Could not register library with %s", RTL.RTLName.c_str());
      }
    }
  }
  PM->RTLs.RegisterLib(desc);
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize all available devices without registering any image
EXTERN void __tgt_init_all_rtls() { PM->RTLs.initAllRTLs(); }

////////////////////////////////////////////////////////////////////////////////
/// unloads a target shared library
EXTERN void __tgt_unregister_lib(__tgt_bin_desc *desc) {
  TIMESCOPE();
  PM->RTLs.UnregisterLib(desc);
  for (auto &RTL : PM->RTLs.UsedRTLs) {
    if (RTL->unregister_lib) {
      if ((*RTL->unregister_lib)(desc) != OFFLOAD_SUCCESS) {
        DP("Could not register library with %s", RTL->RTLName.c_str());
      }
    }
  }
}

/// creates host-to-target data mapping, stores it in the
/// libomptarget.so internal structure (an entry in a stack of data maps)
/// and passes the data to the device.
EXTERN void __tgt_target_data_begin(int64_t device_id, int32_t arg_num,
                                    void **args_base, void **args,
                                    int64_t *arg_sizes, int64_t *arg_types) {
  TIMESCOPE();
  __tgt_target_data_begin_mapper(nullptr, device_id, arg_num, args_base, args,
                                 arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN void __tgt_target_data_begin_nowait(int64_t device_id, int32_t arg_num,
                                           void **args_base, void **args,
                                           int64_t *arg_sizes,
                                           int64_t *arg_types, int32_t depNum,
                                           void *depList, int32_t noAliasDepNum,
                                           void *noAliasDepList) {
  TIMESCOPE();

  __tgt_target_data_begin_mapper(nullptr, device_id, arg_num, args_base, args,
                                 arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN void __tgt_target_data_begin_mapper(ident_t *loc, int64_t device_id,
                                           int32_t arg_num, void **args_base,
                                           void **args, int64_t *arg_sizes,
                                           int64_t *arg_types,
                                           map_var_info_t *arg_names,
                                           void **arg_mappers) {
  TIMESCOPE_WITH_IDENT(loc);
#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(loc, __func__);
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(device_id);
#endif // INTEL_COLLAB
  DP("Entering data begin region for device %" PRId64 " with %d mappings\n",
     device_id, arg_num);
  if (checkDeviceAndCtors(device_id, loc)) {
    DP("Not offloading to device %" PRId64 "\n", device_id);
    return;
  }

  DeviceTy &Device = *PM->Devices[device_id];

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(loc, device_id, arg_num, arg_sizes, arg_types,
                         arg_names, "Entering OpenMP data region");
#ifdef OMPTARGET_DEBUG
  for (int i = 0; i < arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       i, DPxPTR(args_base[i]), DPxPTR(args[i]), arg_sizes[i], arg_types[i],
       (arg_names) ? getNameFromMapping(arg_names[i]).c_str() : "unknown");
  }
#endif

#if INTEL_COLLAB
  Device.pushSubDevice(encodedId, device_id);
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataEnterBegin(device_id));
#endif // INTEL_CUSTOMIZATION

  AsyncInfoTy AsyncInfo(Device);
  int rc = targetDataBegin(loc, Device, arg_num, args_base, args, arg_sizes,
                           arg_types, arg_names, arg_mappers, AsyncInfo);
  if (rc == OFFLOAD_SUCCESS)
    rc = AsyncInfo.synchronize();
  handleTargetOutcome(rc == OFFLOAD_SUCCESS, loc);
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataEnterEnd(device_id));
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (encodedId != device_id)
    PM->Devices[device_id]->popSubDevice();
#endif // INTEL_COLLAB
}

EXTERN void __tgt_target_data_begin_nowait_mapper(
    ident_t *loc, int64_t device_id, int32_t arg_num, void **args_base,
    void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList) {
  TIMESCOPE_WITH_IDENT(loc);

  __tgt_target_data_begin_mapper(loc, device_id, arg_num, args_base, args,
                                 arg_sizes, arg_types, arg_names, arg_mappers);
}

/// passes data from the target, releases target memory and destroys
/// the host-target mapping (top entry from the stack of data maps)
/// created by the last __tgt_target_data_begin.
EXTERN void __tgt_target_data_end(int64_t device_id, int32_t arg_num,
                                  void **args_base, void **args,
                                  int64_t *arg_sizes, int64_t *arg_types) {
  TIMESCOPE();
  __tgt_target_data_end_mapper(nullptr, device_id, arg_num, args_base, args,
                               arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN void __tgt_target_data_end_nowait(int64_t device_id, int32_t arg_num,
                                         void **args_base, void **args,
                                         int64_t *arg_sizes, int64_t *arg_types,
                                         int32_t depNum, void *depList,
                                         int32_t noAliasDepNum,
                                         void *noAliasDepList) {
  TIMESCOPE();

  __tgt_target_data_end_mapper(nullptr, device_id, arg_num, args_base, args,
                               arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN void __tgt_target_data_end_mapper(ident_t *loc, int64_t device_id,
                                         int32_t arg_num, void **args_base,
                                         void **args, int64_t *arg_sizes,
                                         int64_t *arg_types,
                                         map_var_info_t *arg_names,
                                         void **arg_mappers) {
  TIMESCOPE_WITH_IDENT(loc);
#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(loc, __func__);
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(device_id);
#endif // INTEL_COLLAB
  DP("Entering data end region with %d mappings\n", arg_num);
  if (checkDeviceAndCtors(device_id, loc)) {
    DP("Not offloading to device %" PRId64 "\n", device_id);
    return;
  }

  DeviceTy &Device = *PM->Devices[device_id];

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(loc, device_id, arg_num, arg_sizes, arg_types,
                         arg_names, "Exiting OpenMP data region");
#ifdef OMPTARGET_DEBUG
  for (int i = 0; i < arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       i, DPxPTR(args_base[i]), DPxPTR(args[i]), arg_sizes[i], arg_types[i],
       (arg_names) ? getNameFromMapping(arg_names[i]).c_str() : "unknown");
  }
#endif

#if INTEL_COLLAB
  Device.pushSubDevice(encodedId, device_id);
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataExitBegin(device_id));
#endif // INTEL_CUSTOMIZATION

  AsyncInfoTy AsyncInfo(Device);
  int rc = targetDataEnd(loc, Device, arg_num, args_base, args, arg_sizes,
                         arg_types, arg_names, arg_mappers, AsyncInfo);
  if (rc == OFFLOAD_SUCCESS)
    rc = AsyncInfo.synchronize();
  handleTargetOutcome(rc == OFFLOAD_SUCCESS, loc);
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataExitEnd(device_id));
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (encodedId != device_id)
    PM->Devices[device_id]->popSubDevice();
#endif // INTEL_COLLAB
}

EXTERN void __tgt_target_data_end_nowait_mapper(
    ident_t *loc, int64_t device_id, int32_t arg_num, void **args_base,
    void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList) {
  TIMESCOPE_WITH_IDENT(loc);

  __tgt_target_data_end_mapper(loc, device_id, arg_num, args_base, args,
                               arg_sizes, arg_types, arg_names, arg_mappers);
}

EXTERN void __tgt_target_data_update(int64_t device_id, int32_t arg_num,
                                     void **args_base, void **args,
                                     int64_t *arg_sizes, int64_t *arg_types) {
  TIMESCOPE();
  __tgt_target_data_update_mapper(nullptr, device_id, arg_num, args_base, args,
                                  arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN void __tgt_target_data_update_nowait(
    int64_t device_id, int32_t arg_num, void **args_base, void **args,
    int64_t *arg_sizes, int64_t *arg_types, int32_t depNum, void *depList,
    int32_t noAliasDepNum, void *noAliasDepList) {
  TIMESCOPE();

  __tgt_target_data_update_mapper(nullptr, device_id, arg_num, args_base, args,
                                  arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN void __tgt_target_data_update_mapper(ident_t *loc, int64_t device_id,
                                            int32_t arg_num, void **args_base,
                                            void **args, int64_t *arg_sizes,
                                            int64_t *arg_types,
                                            map_var_info_t *arg_names,
                                            void **arg_mappers) {
  TIMESCOPE_WITH_IDENT(loc);
#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(loc, __func__);
#endif // INTEL_CUSTOMIZATION
  DP("Entering data update with %d mappings\n", arg_num);
#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(device_id);
#endif // INTEL_COLLAB
  if (checkDeviceAndCtors(device_id, loc)) {
    DP("Not offloading to device %" PRId64 "\n", device_id);
    return;
  }

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(loc, device_id, arg_num, arg_sizes, arg_types,
                         arg_names, "Updating OpenMP data");

  DeviceTy &Device = *PM->Devices[device_id];
  AsyncInfoTy AsyncInfo(Device);
#if INTEL_COLLAB
  Device.pushSubDevice(encodedId, device_id);
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataUpdateBegin(device_id));
#endif // INTEL_CUSTOMIZATION
  int rc = targetDataUpdate(loc, Device, arg_num, args_base, args, arg_sizes,
                            arg_types, arg_names, arg_mappers, AsyncInfo);
  if (rc == OFFLOAD_SUCCESS)
    rc = AsyncInfo.synchronize();
  handleTargetOutcome(rc == OFFLOAD_SUCCESS, loc);
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataUpdateEnd(device_id));
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (encodedId != device_id)
    PM->Devices[device_id]->popSubDevice();
#endif // INTEL_COLLAB
}

EXTERN void __tgt_target_data_update_nowait_mapper(
    ident_t *loc, int64_t device_id, int32_t arg_num, void **args_base,
    void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList) {
  TIMESCOPE_WITH_IDENT(loc);

  __tgt_target_data_update_mapper(loc, device_id, arg_num, args_base, args,
                                  arg_sizes, arg_types, arg_names, arg_mappers);
}

EXTERN int __tgt_target(int64_t device_id, void *host_ptr, int32_t arg_num,
                        void **args_base, void **args, int64_t *arg_sizes,
                        int64_t *arg_types) {
  TIMESCOPE();
  return __tgt_target_mapper(nullptr, device_id, host_ptr, arg_num, args_base,
                             args, arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN int __tgt_target_nowait(int64_t device_id, void *host_ptr,
                               int32_t arg_num, void **args_base, void **args,
                               int64_t *arg_sizes, int64_t *arg_types,
                               int32_t depNum, void *depList,
                               int32_t noAliasDepNum, void *noAliasDepList) {
  TIMESCOPE();

  return __tgt_target_mapper(nullptr, device_id, host_ptr, arg_num, args_base,
                             args, arg_sizes, arg_types, nullptr, nullptr);
}

EXTERN int __tgt_target_mapper(ident_t *loc, int64_t device_id, void *host_ptr,
                               int32_t arg_num, void **args_base, void **args,
                               int64_t *arg_sizes, int64_t *arg_types,
                               map_var_info_t *arg_names, void **arg_mappers) {
  TIMESCOPE_WITH_IDENT(loc);
#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(loc, __func__);
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(device_id);
#endif // INTEL_COLLAB
  DP("Entering target region with entry point " DPxMOD " and device Id %" PRId64
     "\n",
     DPxPTR(host_ptr), device_id);
  if (checkDeviceAndCtors(device_id, loc)) {
    DP("Not offloading to device %" PRId64 "\n", device_id);
    return OMP_TGT_FAIL;
  }

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(loc, device_id, arg_num, arg_sizes, arg_types,
                         arg_names, "Entering OpenMP kernel");
#ifdef OMPTARGET_DEBUG
  for (int i = 0; i < arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       i, DPxPTR(args_base[i]), DPxPTR(args[i]), arg_sizes[i], arg_types[i],
       (arg_names) ? getNameFromMapping(arg_names[i]).c_str() : "unknown");
  }
#endif

#if INTEL_COLLAB
  // Push device encoding
  PM->Devices[device_id]->pushSubDevice(encodedId, device_id);
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetBegin(device_id));
#endif // INTEL_CUSTOMIZATION
  DeviceTy &Device = *PM->Devices[device_id];
  AsyncInfoTy AsyncInfo(Device);
  int rc = target(loc, Device, host_ptr, arg_num, args_base, args, arg_sizes,
                  arg_types, arg_names, arg_mappers, 0, 0, false /*team*/,
                  AsyncInfo);
  if (rc == OFFLOAD_SUCCESS)
    rc = AsyncInfo.synchronize();
  handleTargetOutcome(rc == OFFLOAD_SUCCESS, loc);
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetEnd(device_id));
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (encodedId != device_id)
    PM->Devices[device_id]->popSubDevice();
#endif // INTEL_COLLAB
  assert(rc == OFFLOAD_SUCCESS && "__tgt_target_mapper unexpected failure!");
  return OMP_TGT_SUCCESS;
}

EXTERN int __tgt_target_nowait_mapper(
    ident_t *loc, int64_t device_id, void *host_ptr, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList) {
  TIMESCOPE_WITH_IDENT(loc);

  return __tgt_target_mapper(loc, device_id, host_ptr, arg_num, args_base, args,
                             arg_sizes, arg_types, arg_names, arg_mappers);
}

EXTERN int __tgt_target_teams(int64_t device_id, void *host_ptr,
                              int32_t arg_num, void **args_base, void **args,
                              int64_t *arg_sizes, int64_t *arg_types,
                              int32_t team_num, int32_t thread_limit) {
  TIMESCOPE();
  return __tgt_target_teams_mapper(nullptr, device_id, host_ptr, arg_num,
                                   args_base, args, arg_sizes, arg_types,
                                   nullptr, nullptr, team_num, thread_limit);
}

EXTERN int __tgt_target_teams_nowait(int64_t device_id, void *host_ptr,
                                     int32_t arg_num, void **args_base,
                                     void **args, int64_t *arg_sizes,
                                     int64_t *arg_types, int32_t team_num,
                                     int32_t thread_limit, int32_t depNum,
                                     void *depList, int32_t noAliasDepNum,
                                     void *noAliasDepList) {
  TIMESCOPE();

  return __tgt_target_teams_mapper(nullptr, device_id, host_ptr, arg_num,
                                   args_base, args, arg_sizes, arg_types,
                                   nullptr, nullptr, team_num, thread_limit);
}

EXTERN int __tgt_target_teams_mapper(ident_t *loc, int64_t device_id,
                                     void *host_ptr, int32_t arg_num,
                                     void **args_base, void **args,
                                     int64_t *arg_sizes, int64_t *arg_types,
                                     map_var_info_t *arg_names,
                                     void **arg_mappers, int32_t team_num,
                                     int32_t thread_limit) {
#if INTEL_CUSTOMIZATION
  XPTIEventCacheTy XPTIEvt(loc, __func__);
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  int64_t encodedId = GetEncodedDeviceID(device_id);
#endif // INTEL_COLLAB
  DP("Entering target region with entry point " DPxMOD " and device Id %" PRId64
     "\n",
     DPxPTR(host_ptr), device_id);
  if (checkDeviceAndCtors(device_id, loc)) {
    DP("Not offloading to device %" PRId64 "\n", device_id);
    return OMP_TGT_FAIL;
  }

  if (getInfoLevel() & OMP_INFOTYPE_KERNEL_ARGS)
    printKernelArguments(loc, device_id, arg_num, arg_sizes, arg_types,
                         arg_names, "Entering OpenMP kernel");
#ifdef OMPTARGET_DEBUG
  for (int i = 0; i < arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 ", Name=%s\n",
       i, DPxPTR(args_base[i]), DPxPTR(args[i]), arg_sizes[i], arg_types[i],
       (arg_names) ? getNameFromMapping(arg_names[i]).c_str() : "unknown");
  }
#endif

#if INTEL_COLLAB
  // Push device encoding
  PM->Devices[device_id]->pushSubDevice(encodedId, device_id);
#endif // INTEL_COLLAB
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetBegin(device_id));
#endif // INTEL_CUSTOMIZATION
  DeviceTy &Device = *PM->Devices[device_id];
  AsyncInfoTy AsyncInfo(Device);
  int rc = target(loc, Device, host_ptr, arg_num, args_base, args, arg_sizes,
                  arg_types, arg_names, arg_mappers, team_num, thread_limit,
                  true /*team*/, AsyncInfo);
  if (rc == OFFLOAD_SUCCESS)
    rc = AsyncInfo.synchronize();

  handleTargetOutcome(rc == OFFLOAD_SUCCESS, loc);
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetEnd(device_id));
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
  if (encodedId != device_id)
    PM->Devices[device_id]->popSubDevice();
#endif // INTEL_COLLAB
  assert(rc == OFFLOAD_SUCCESS &&
         "__tgt_target_teams_mapper unexpected failure!");
  return OMP_TGT_SUCCESS;
}

EXTERN int __tgt_target_teams_nowait_mapper(
    ident_t *loc, int64_t device_id, void *host_ptr, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t team_num,
    int32_t thread_limit, int32_t depNum, void *depList, int32_t noAliasDepNum,
    void *noAliasDepList) {
  TIMESCOPE_WITH_IDENT(loc);

  return __tgt_target_teams_mapper(loc, device_id, host_ptr, arg_num, args_base,
                                   args, arg_sizes, arg_types, arg_names,
                                   arg_mappers, team_num, thread_limit);
}

// Get the current number of components for a user-defined mapper.
EXTERN int64_t __tgt_mapper_num_components(void *rt_mapper_handle) {
  TIMESCOPE();
  auto *MapperComponentsPtr = (struct MapperComponentsTy *)rt_mapper_handle;
  int64_t size = MapperComponentsPtr->Components.size();
  DP("__tgt_mapper_num_components(Handle=" DPxMOD ") returns %" PRId64 "\n",
     DPxPTR(rt_mapper_handle), size);
  return size;
}

// Push back one component for a user-defined mapper.
EXTERN void __tgt_push_mapper_component(void *rt_mapper_handle, void *base,
                                        void *begin, int64_t size, int64_t type,
                                        void *name) {
  TIMESCOPE();
  DP("__tgt_push_mapper_component(Handle=" DPxMOD
     ") adds an entry (Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
     ", Type=0x%" PRIx64 ", Name=%s).\n",
     DPxPTR(rt_mapper_handle), DPxPTR(base), DPxPTR(begin), size, type,
     (name) ? getNameFromMapping(name).c_str() : "unknown");
  auto *MapperComponentsPtr = (struct MapperComponentsTy *)rt_mapper_handle;
  MapperComponentsPtr->Components.push_back(
      MapComponentInfoTy(base, begin, size, type, name));
}

EXTERN void __kmpc_push_target_tripcount(int64_t device_id,
                                         uint64_t loop_tripcount) {
  __kmpc_push_target_tripcount_mapper(nullptr, device_id, loop_tripcount);
}

EXTERN void __kmpc_push_target_tripcount_mapper(ident_t *loc, int64_t device_id,
                                                uint64_t loop_tripcount) {
  TIMESCOPE_WITH_IDENT(loc);
  if (checkDeviceAndCtors(device_id, loc)) {
    DP("Not offloading to device %" PRId64 "\n", device_id);
    return;
  }

  DP("__kmpc_push_target_tripcount(%" PRId64 ", %" PRIu64 ")\n", device_id,
     loop_tripcount);
  PM->TblMapMtx.lock();
  PM->Devices[device_id]->LoopTripCnt.emplace(__kmpc_global_thread_num(NULL),
                                              loop_tripcount);
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


EXTERN void __tgt_offload_proxy_task_complete_ooo(void *interop_obj) {

// This structure is same as kmp_task_t defined in omp.h
typedef struct {
  void *shareds;   // not used
  void * routine;  // not used
  int part_id;     // not used
}async_t;

  DP("Call to __tgt_offload_proxy_task_complete_ooo interop obj " DPxMOD "\n",
      DPxPTR(interop_obj));
  __tgt_interop_obj *tgt_interop_obj = (__tgt_interop_obj *) interop_obj;
  async_t *async_obj =  (async_t *) tgt_interop_obj->async_obj;

  __tgt_release_interop_obj(interop_obj);

  __kmpc_proxy_task_completed_ooo(async_obj);
}

EXTERN void *__tgt_create_interop_obj(
    int64_t device_code, bool is_async, void *async_obj) {
  int64_t device_id = EXTRACT_BITS(device_code, 31, 0);

  static std::once_flag Flag{};

  DP("Call to __tgt_create_interop_obj with device_id %" PRId64 ", is_async %s"
     ", async_obj " DPxMOD "\n", device_id, is_async ? "true" : "false",
     DPxPTR(async_obj));

  if (checkDeviceAndCtors(device_id, nullptr) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_id);
    handleTargetOutcome(false, nullptr);
    return NULL;
  }

  DeviceTy &Device = *PM->Devices[device_id];

  auto &rtl_name = Device.RTL->RTLName;
  int32_t plugin;

  if (rtl_name.find("opencl") != std::string::npos) {
    plugin = INTEROP_PLUGIN_OPENCL;
  } else if (rtl_name.find("level0") != std::string::npos) {
    plugin = INTEROP_PLUGIN_LEVEL0;
  } else if (rtl_name.find("x86_64") != std::string::npos) {
    plugin = INTEROP_PLUGIN_X86_64;
  } else {
    DP("%s does not support interop interface\n", rtl_name.c_str());
    return NULL;
  }

  __tgt_interop_obj *obj =
      (__tgt_interop_obj *)malloc(sizeof(__tgt_interop_obj));
  if (!obj) {
    DP("Failed to malloc memory for interop object\n");
    return NULL;
  }
   std::call_once(Flag, []() {
   useSingleQueue = 0;
   if (char *EnvStr = getenv("LIBOMPTARGET_INTEROP_USE_SINGLE_QUEUE"))
         useSingleQueue = std::atoi(EnvStr);
   });

  obj->device_id = device_id;
  obj->device_code = device_code; // Preserve 64-bit device encoding
  obj->is_async = is_async;
  obj->async_obj = async_obj;
  obj->async_handler = &__tgt_offload_proxy_task_complete_ooo;
  obj->queue = nullptr; // Will be created when property is requested.
  obj->platform_handle = Device.get_platform_handle();
  obj->context_handle = Device.get_context_handle();
  Device.setDeviceHandle(obj);
  obj->plugin_interface = plugin;

  return obj;
}

EXTERN int __tgt_release_interop_obj(void *interop_obj) {
  DP("Call to __tgt_release_interop_obj with interop_obj " DPxMOD "\n",
     DPxPTR(interop_obj));

  assert(!isOffloadDisabled() &&
          "Freeing interop object with Offload Disabled.");
  if (!interop_obj)
    return OFFLOAD_FAIL;

  __tgt_interop_obj *obj = static_cast<__tgt_interop_obj *>(interop_obj);
  DeviceTy &Device = *PM->Devices[obj->device_id];
  if (obj->queue && obj->is_async && !useSingleQueue)
    Device.release_offload_queue(obj->queue);
  free(interop_obj);

  return OFFLOAD_SUCCESS;
}

EXTERN int __tgt_set_interop_property(
    void *interop_obj, int32_t property_id, void *property_value) {
  DP("Call to __tgt_set_interop_property with interop_obj " DPxMOD
     ", property_id %" PRId32 "\n", DPxPTR(interop_obj), property_id);

  if (isOffloadDisabled() || !interop_obj || !property_value) {
    return OFFLOAD_FAIL;
  }

  __tgt_interop_obj *interop = (__tgt_interop_obj *)interop_obj;
  // Currently we support setting async object only
  switch (property_id) {
  case INTEROP_ASYNC_OBJ:
    if (interop->async_obj) {
       DP("Updating async obj is not allowed" PRId32 "\n");
       return OFFLOAD_FAIL;
    }
    interop->async_obj = property_value;
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

  if (isOffloadDisabled() || !interop_obj || !property_value) {
    return OFFLOAD_FAIL;
  }

  __tgt_interop_obj *interop = (__tgt_interop_obj *)interop_obj;
  switch (property_id) {
  case INTEROP_DEVICE_ID:
    *property_value = (void *)&interop->device_id;
    break;
  case INTEROP_IS_ASYNC:
    *property_value = (void *)&interop->is_async;
    break;
  case INTEROP_ASYNC_OBJ:
    *property_value = interop->async_obj;
    break;
  case INTEROP_ASYNC_CALLBACK:
    *property_value = (void *)interop->async_handler;
    break;
  case INTEROP_OFFLOAD_QUEUE:
    if (!interop->queue)
      PM->Devices[interop->device_id]->get_offload_queue(
          interop, (!interop->is_async || useSingleQueue)
                        ? false : true /*create_new*/);
    *property_value = interop->queue;
    break;
  case INTEROP_PLATFORM_HANDLE:
    *property_value = interop->platform_handle;
    break;
  case INTEROP_DEVICE_HANDLE:
    *property_value = interop->device_handle;
    break;
  case INTEROP_PLUGIN_INTERFACE:
    *property_value = (void *)&interop->plugin_interface;
    break;
  case INTEROP_CONTEXT_HANDLE:
    *property_value = interop->context_handle;
    break;
  default:
    DP("Invalid interop property name " PRId32 "\n");
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

EXTERN omp_interop_t __tgt_create_interop(
    int64_t device_num, int32_t interop_type, int32_t num_prefers,
    intptr_t *prefer_ids) {
  DP("Call to %s with device_num %" PRId64 ", interop_type %" PRId32
     ", num_prefers %" PRId32 ", prefer_ids " DPxMOD "\n",
     __func__, device_num, interop_type, num_prefers, DPxPTR(prefer_ids));

  if (isOffloadDisabled())
    return omp_interop_none;

  omp_interop_t Interop = omp_interop_none;

  // Now, try to create an interop with device_num.
  if (device_num == OFFLOAD_DEVICE_DEFAULT)
    device_num = omp_get_default_device();

  if (device_is_ready(device_num)) {
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

  if (!device_is_ready(DeviceNum)) {
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

  if (!device_is_ready(DeviceNum)) {
    DP("Device %" PRId64 " is not ready when using an interop " DPxMOD "\n",
       DeviceNum, DPxPTR(interop));
    return OFFLOAD_FAIL;
  }

  if (!TgtInterop->TargetSync)
    return OFFLOAD_SUCCESS;

  return PM->Devices[DeviceNum]->useInterop(TgtInterop);
}

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

  if (!device_is_ready(device_num)) {
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

EXTERN int __tgt_print_device_info(int64_t device_id) {
  return PM->Devices[device_id]->printDeviceInfo(
      PM->Devices[device_id]->RTLDeviceID);
}
