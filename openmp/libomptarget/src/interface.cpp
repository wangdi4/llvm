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

#include <omptarget.h>
#if INTEL_COLLAB
#include "omptarget-tools.h"
#endif // INTEL_COLLAB

#include "device.h"
#include "private.h"
#include "rtl.h"

#include <cassert>
#include <cstdlib>
#include <mutex>
#if INTEL_COLLAB
#include <string.h>
#endif  // INTEL_COLLAB

// Store target policy (disabled, mandatory, default)
kmp_target_offload_kind_t TargetOffloadPolicy = tgt_default;
std::mutex TargetOffloadMtx;

////////////////////////////////////////////////////////////////////////////////
/// manage the success or failure of a target construct

static void HandleDefaultTargetOffload() {
  TargetOffloadMtx.lock();
  if (TargetOffloadPolicy == tgt_default) {
    if (omp_get_num_devices() > 0) {
      DP("Default TARGET OFFLOAD policy is now mandatory "
         "(devices were found)\n");
      TargetOffloadPolicy = tgt_mandatory;
    } else {
      DP("Default TARGET OFFLOAD policy is now disabled "
         "(no devices were found)\n");
      TargetOffloadPolicy = tgt_disabled;
    }
  }
  TargetOffloadMtx.unlock();
}

static int IsOffloadDisabled() {
  if (TargetOffloadPolicy == tgt_default) HandleDefaultTargetOffload();
  return TargetOffloadPolicy == tgt_disabled;
}

static void HandleTargetOutcome(bool success) {
  switch (TargetOffloadPolicy) {
    case tgt_disabled:
      if (success) {
        FATAL_MESSAGE0(1, "expected no offloading while offloading is disabled");
      }
      break;
    case tgt_default:
      FATAL_MESSAGE0(1, "default offloading policy must be switched to "
                        "mandatory or disabled");
      break;
    case tgt_mandatory:
      if (!success) {
        FATAL_MESSAGE0(1, "failure of target construct while offloading is mandatory");
      }
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// adds requires flags
EXTERN void __tgt_register_requires(int64_t flags) {
  RTLs->RegisterRequires(flags);
}

////////////////////////////////////////////////////////////////////////////////
/// adds a target shared library to the target execution image
EXTERN void __tgt_register_lib(__tgt_bin_desc *desc) {
  RTLs->RegisterLib(desc);
}

////////////////////////////////////////////////////////////////////////////////
/// unloads a target shared library
EXTERN void __tgt_unregister_lib(__tgt_bin_desc *desc) {
  RTLs->UnregisterLib(desc);
}

/// creates host-to-target data mapping, stores it in the
/// libomptarget.so internal structure (an entry in a stack of data maps)
/// and passes the data to the device.
EXTERN void __tgt_target_data_begin(int64_t device_id, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types) {
  if (IsOffloadDisabled()) return;

  DP("Entering data begin region for device %" PRId64 " with %d mappings\n",
      device_id, arg_num);

  // No devices available?
  if (device_id == OFFLOAD_DEVICE_DEFAULT) {
    device_id = omp_get_default_device();
    DP("Use default device id %" PRId64 "\n", device_id);
  }

  if (CheckDeviceAndCtors(device_id) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_id);
    HandleTargetOutcome(false);
    return;
  }

#if INTEL_COLLAB
  OMPT_TRACE(targetDataEnterBegin(device_id));
#endif // INTEL_COLLAB
  DeviceTy &Device = Devices[device_id];

#ifdef OMPTARGET_DEBUG
  for (int i = 0; i < arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
       ", Type=0x%" PRIx64 "\n",
       i, DPxPTR(args_base[i]), DPxPTR(args[i]), arg_sizes[i], arg_types[i]);
  }
#endif

  int rc = target_data_begin(Device, arg_num, args_base, args, arg_sizes,
                             arg_types, nullptr);
  HandleTargetOutcome(rc == OFFLOAD_SUCCESS);
#if INTEL_COLLAB
  OMPT_TRACE(targetDataEnterEnd(device_id));
#endif // INTEL_COLLAB
}

EXTERN void __tgt_target_data_begin_nowait(int64_t device_id, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types,
    int32_t depNum, void *depList, int32_t noAliasDepNum,
    void *noAliasDepList) {
  if (depNum + noAliasDepNum > 0)
    __kmpc_omp_taskwait(NULL, __kmpc_global_thread_num(NULL));

  __tgt_target_data_begin(device_id, arg_num, args_base, args, arg_sizes,
                          arg_types);
}

/// passes data from the target, releases target memory and destroys
/// the host-target mapping (top entry from the stack of data maps)
/// created by the last __tgt_target_data_begin.
EXTERN void __tgt_target_data_end(int64_t device_id, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types) {
  if (IsOffloadDisabled()) return;
  DP("Entering data end region with %d mappings\n", arg_num);

  // No devices available?
  if (device_id == OFFLOAD_DEVICE_DEFAULT) {
    device_id = omp_get_default_device();
  }

  RTLsMtx->lock();
  size_t Devices_size = Devices.size();
  RTLsMtx->unlock();
  if (Devices_size <= (size_t)device_id) {
    DP("Device ID  %" PRId64 " does not have a matching RTL.\n", device_id);
    HandleTargetOutcome(false);
    return;
  }

  DeviceTy &Device = Devices[device_id];
  if (!Device.IsInit) {
    DP("Uninit device: ignore");
    HandleTargetOutcome(false);
    return;
  }

#ifdef OMPTARGET_DEBUG
  for (int i=0; i<arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
        ", Type=0x%" PRIx64 "\n", i, DPxPTR(args_base[i]), DPxPTR(args[i]),
        arg_sizes[i], arg_types[i]);
  }
#endif

#if INTEL_COLLAB
  OMPT_TRACE(targetDataExitBegin(device_id));
#endif // INTEL_COLLAB
  int rc = target_data_end(Device, arg_num, args_base, args, arg_sizes,
                           arg_types, nullptr);
  HandleTargetOutcome(rc == OFFLOAD_SUCCESS);
#if INTEL_COLLAB
  OMPT_TRACE(targetDataExitEnd(device_id));
#endif // INTEL_COLLAB
}

EXTERN void __tgt_target_data_end_nowait(int64_t device_id, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types,
    int32_t depNum, void *depList, int32_t noAliasDepNum,
    void *noAliasDepList) {
  if (depNum + noAliasDepNum > 0)
    __kmpc_omp_taskwait(NULL, __kmpc_global_thread_num(NULL));

  __tgt_target_data_end(device_id, arg_num, args_base, args, arg_sizes,
                        arg_types);
}

EXTERN void __tgt_target_data_update(int64_t device_id, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types) {
  if (IsOffloadDisabled()) return;
  DP("Entering data update with %d mappings\n", arg_num);

  // No devices available?
  if (device_id == OFFLOAD_DEVICE_DEFAULT) {
    device_id = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_id) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_id);
    HandleTargetOutcome(false);
    return;
  }

#if INTEL_COLLAB
  OMPT_TRACE(targetDataUpdateBegin(device_id));
#endif // INTEL_COLLAB
  DeviceTy& Device = Devices[device_id];
  int rc = target_data_update(Device, arg_num, args_base,
      args, arg_sizes, arg_types);
  HandleTargetOutcome(rc == OFFLOAD_SUCCESS);
#if INTEL_COLLAB
  OMPT_TRACE(targetDataUpdateEnd(device_id));
#endif // INTEL_COLLAB
}

EXTERN void __tgt_target_data_update_nowait(
    int64_t device_id, int32_t arg_num, void **args_base, void **args,
    int64_t *arg_sizes, int64_t *arg_types, int32_t depNum, void *depList,
    int32_t noAliasDepNum, void *noAliasDepList) {
  if (depNum + noAliasDepNum > 0)
    __kmpc_omp_taskwait(NULL, __kmpc_global_thread_num(NULL));

  __tgt_target_data_update(device_id, arg_num, args_base, args, arg_sizes,
                           arg_types);
}

EXTERN int __tgt_target(int64_t device_id, void *host_ptr, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types) {
  if (IsOffloadDisabled()) return OFFLOAD_FAIL;
  DP("Entering target region with entry point " DPxMOD " and device Id %"
      PRId64 "\n", DPxPTR(host_ptr), device_id);

  if (device_id == OFFLOAD_DEVICE_DEFAULT) {
    device_id = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_id) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_id);
    HandleTargetOutcome(false);
    return OFFLOAD_FAIL;
  }

#ifdef OMPTARGET_DEBUG
  for (int i=0; i<arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
        ", Type=0x%" PRIx64 "\n", i, DPxPTR(args_base[i]), DPxPTR(args[i]),
        arg_sizes[i], arg_types[i]);
  }
#endif

#if INTEL_COLLAB
  OMPT_TRACE(targetBegin(device_id));
#endif // INTEL_COLLAB
  int rc = target(device_id, host_ptr, arg_num, args_base, args, arg_sizes,
      arg_types, 0, 0, false /*team*/);
  HandleTargetOutcome(rc == OFFLOAD_SUCCESS);
#if INTEL_COLLAB
  OMPT_TRACE(targetEnd(device_id));
#endif // INTEL_COLLAB
  return rc;
}

EXTERN int __tgt_target_nowait(int64_t device_id, void *host_ptr,
    int32_t arg_num, void **args_base, void **args, int64_t *arg_sizes,
    int64_t *arg_types, int32_t depNum, void *depList, int32_t noAliasDepNum,
    void *noAliasDepList) {
  if (depNum + noAliasDepNum > 0)
    __kmpc_omp_taskwait(NULL, __kmpc_global_thread_num(NULL));

  return __tgt_target(device_id, host_ptr, arg_num, args_base, args, arg_sizes,
                      arg_types);
}

EXTERN int __tgt_target_teams(int64_t device_id, void *host_ptr,
    int32_t arg_num, void **args_base, void **args, int64_t *arg_sizes,
    int64_t *arg_types, int32_t team_num, int32_t thread_limit) {
  if (IsOffloadDisabled()) return OFFLOAD_FAIL;
  DP("Entering target region with entry point " DPxMOD " and device Id %"
      PRId64 "\n", DPxPTR(host_ptr), device_id);

  if (device_id == OFFLOAD_DEVICE_DEFAULT) {
    device_id = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_id) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_id);
    HandleTargetOutcome(false);
    return OFFLOAD_FAIL;
  }

#ifdef OMPTARGET_DEBUG
  for (int i=0; i<arg_num; ++i) {
    DP("Entry %2d: Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
        ", Type=0x%" PRIx64 "\n", i, DPxPTR(args_base[i]), DPxPTR(args[i]),
        arg_sizes[i], arg_types[i]);
  }
#endif

#if INTEL_COLLAB
  OMPT_TRACE(targetBegin(device_id));
#endif // INTEL_COLLAB
  int rc = target(device_id, host_ptr, arg_num, args_base, args, arg_sizes,
      arg_types, team_num, thread_limit, true /*team*/);
  HandleTargetOutcome(rc == OFFLOAD_SUCCESS);
#if INTEL_COLLAB
  OMPT_TRACE(targetEnd(device_id));
#endif // INTEL_COLLAB

  return rc;
}

EXTERN int __tgt_target_teams_nowait(int64_t device_id, void *host_ptr,
    int32_t arg_num, void **args_base, void **args, int64_t *arg_sizes,
    int64_t *arg_types, int32_t team_num, int32_t thread_limit, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList) {
  if (depNum + noAliasDepNum > 0)
    __kmpc_omp_taskwait(NULL, __kmpc_global_thread_num(NULL));

  return __tgt_target_teams(device_id, host_ptr, arg_num, args_base, args,
                            arg_sizes, arg_types, team_num, thread_limit);
}

// Get the current number of components for a user-defined mapper.
EXTERN int64_t __tgt_mapper_num_components(void *rt_mapper_handle) {
  auto *MapperComponentsPtr = (struct MapperComponentsTy *)rt_mapper_handle;
  int64_t size = MapperComponentsPtr->Components.size();
  DP("__tgt_mapper_num_components(Handle=" DPxMOD ") returns %" PRId64 "\n",
     DPxPTR(rt_mapper_handle), size);
  return size;
}

// Push back one component for a user-defined mapper.
EXTERN void __tgt_push_mapper_component(void *rt_mapper_handle, void *base,
                                        void *begin, int64_t size,
                                        int64_t type) {
  DP("__tgt_push_mapper_component(Handle=" DPxMOD
     ") adds an entry (Base=" DPxMOD ", Begin=" DPxMOD ", Size=%" PRId64
     ", Type=0x%" PRIx64 ").\n",
     DPxPTR(rt_mapper_handle), DPxPTR(base), DPxPTR(begin), size, type);
  auto *MapperComponentsPtr = (struct MapperComponentsTy *)rt_mapper_handle;
  MapperComponentsPtr->Components.push_back(
      MapComponentInfoTy(base, begin, size, type));
}

EXTERN void __kmpc_push_target_tripcount(int64_t device_id,
    uint64_t loop_tripcount) {
  if (IsOffloadDisabled())
    return;

  if (device_id == OFFLOAD_DEVICE_DEFAULT) {
    device_id = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_id) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_id);
    HandleTargetOutcome(false);
    return;
  }

  DP("__kmpc_push_target_tripcount(%" PRId64 ", %" PRIu64 ")\n", device_id,
      loop_tripcount);
  TblMapMtx->lock();
  Devices[device_id].LoopTripCnt.emplace(__kmpc_global_thread_num(NULL),
                                         loop_tripcount);
  TblMapMtx->unlock();
}

#if INTEL_COLLAB
EXTERN bool __tgt_is_device_available(int64_t device_num, void *device_type) {
  if (IsOffloadDisabled())
    return false;

  if (device_num == OFFLOAD_DEVICE_DEFAULT) {
    device_num = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_num) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    HandleTargetOutcome(false);
    return false;
  }

  return (device_num >= 0 && device_num < omp_get_num_devices());
}

// Find device pointer from the given host pointer and create a buffer object
EXTERN void *__tgt_create_buffer(int64_t device_num, void *host_ptr) {
  DP("Call to __tgt_create_buffer with host_ptr " DPxMOD ", "
      "device_num %" PRId64 "\n", DPxPTR(host_ptr), device_num);

  if (IsOffloadDisabled())
    return NULL;

  if (device_num == OFFLOAD_DEVICE_DEFAULT) {
    device_num = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_num) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    HandleTargetOutcome(false);
    return NULL;
  }

  if (!host_ptr) {
    DP("Call to __tgt_create_buffer with invalid host_ptr\n");
    HandleTargetOutcome(false);
    return NULL;
  }

  DeviceTy &Device = Devices[device_num];
  void *ret = Device.create_buffer(host_ptr);
  if (!ret) {
    DP("Call to __tgt_create_buffer with no associated device_ptr\n");
    HandleTargetOutcome(false);
  }
  DP("__tgt_create_buffer returns " DPxMOD "\n", DPxPTR(ret));

  return ret;
}

// Release the device buffer
EXTERN int __tgt_release_buffer(int64_t device_num, void *device_buffer) {
  DP("Call to __tgt_release_buffer with device_buffer " DPxMOD ", "
      "device_num %" PRId64 "\n", DPxPTR(device_buffer), device_num);

  if (IsOffloadDisabled())
    return OFFLOAD_FAIL;

  if (device_num == OFFLOAD_DEVICE_DEFAULT) {
    device_num = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_num) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    HandleTargetOutcome(false);
    return OFFLOAD_FAIL;
  }

  if (!device_buffer) {
    DP("Call to __tgt_release_buffer with invalid device_buffer\n");
    return OFFLOAD_FAIL;
  }

  DeviceTy &Device = Devices[device_num];
  return Device.release_buffer(device_buffer);
}

EXTERN char *__tgt_get_device_name(
    int64_t device_num, char *buffer, size_t buffer_max_size) {
  DP("Call to __tgt_get_device_name with device_num %" PRId64 " and "
     "buffer_max_size %zu.\n",
     device_num, buffer_max_size);

  if (!buffer || buffer_max_size == 0 || IsOffloadDisabled())
    return NULL;

  if (device_num == OFFLOAD_DEVICE_DEFAULT) {
    device_num = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_num) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    HandleTargetOutcome(false);
    return NULL;
  }

  DP("Querying device for its name.\n");

  DeviceTy &Device = Devices[device_num];
  return Device.get_device_name(buffer, buffer_max_size);
}

EXTERN char *__tgt_get_device_rtl_name(
    int64_t device_num, char *buffer, size_t buffer_max_size) {
  DP("Call to __tgt_get_device_rtl_name with device_num %" PRId64 " and "
     "buffer_max_size %zu.\n",
     device_num, buffer_max_size);

  if (!buffer || buffer_max_size == 0 || IsOffloadDisabled())
    return NULL;

  if (device_num == OFFLOAD_DEVICE_DEFAULT) {
    device_num = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_num) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_num);
    HandleTargetOutcome(false);
    return NULL;
  }

  const RTLInfoTy *RTL = Devices[device_num].RTL;
  assert(RTL && "Device with uninitialized RTL.");
  strncpy(buffer, RTL->RTLConstName, buffer_max_size - 1);
  buffer[buffer_max_size - 1] = '\0';
  return buffer;
}


EXTERN void __tgt_offload_proxy_task_complete_ooo(void *interop_obj) {

// The 1st 3 fields of this structure is same as kmp_task_t defined in omp.h
// The last field is private and it is used to represent number
// of buffers for async variant offload.
typedef struct {
  void *shareds;   // pointer to block of pointers to shared vars
  void * routine;  // not used
  int part_id;     // not used
  int num_buffers;
}async_t;

  DP("Call to __tgt_offload_proxy_task_complete_ooo interop obj " DPxMOD "\n",
      DPxPTR(interop_obj));
  __tgt_interop_obj *tgt_interop_obj = (__tgt_interop_obj *) interop_obj;
  async_t *async_obj =  (async_t *) tgt_interop_obj->async_obj;
  int64_t device_num = tgt_interop_obj->device_id;
  int num_buffers = async_obj->num_buffers;
  void **device_buffers = (void **) async_obj->shareds;
  for (int i=0; i<num_buffers; ++i) {
    __tgt_release_buffer(device_num, device_buffers[i]);
  }

  __tgt_release_interop_obj(interop_obj);

  __kmpc_proxy_task_completed_ooo(async_obj);
}

EXTERN void *__tgt_create_interop_obj(
    int64_t device_id, bool is_async, void *async_obj) {
  DP("Call to __tgt_create_interop_obj with device_id %" PRId64 ", is_async %s"
     ", async_obj " DPxMOD "\n", device_id, is_async ? "true" : "false",
     DPxPTR(async_obj));

  if (IsOffloadDisabled()) {
    return NULL;
  }

  if (device_id == OFFLOAD_DEVICE_DEFAULT) {
    device_id = omp_get_default_device();
  }

  if (CheckDeviceAndCtors(device_id) != OFFLOAD_SUCCESS) {
    DP("Failed to get device %" PRId64 " ready\n", device_id);
    HandleTargetOutcome(false);
    return NULL;
  }

  DeviceTy &Device = Devices[device_id];

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

  obj->device_id = device_id;
  obj->is_async = is_async;
  obj->async_obj = async_obj;
  obj->async_handler = &__tgt_offload_proxy_task_complete_ooo;
  obj->queue = Device.create_offload_queue(is_async);
  obj->platform_handle = Device.get_platform_handle();
  if (plugin == INTEROP_PLUGIN_LEVEL0)
     obj->device_handle = Device.get_device_handle();
  else
     obj->device_handle = NULL;
  obj->plugin_interface = plugin;

  return obj;
}

EXTERN int __tgt_release_interop_obj(void *interop_obj) {
  DP("Call to __tgt_release_interop_obj with interop_obj " DPxMOD "\n",
     DPxPTR(interop_obj));

  assert(!IsOffloadDisabled() &&
          "Freeing interop object with Offload Disabled.");
  if (IsOffloadDisabled() || !interop_obj)
    return OFFLOAD_FAIL;

  __tgt_interop_obj *obj = static_cast<__tgt_interop_obj *>(interop_obj);
  DeviceTy &Device = Devices[obj->device_id];
  Device.release_offload_queue(obj->queue);
  free(interop_obj);

  return OFFLOAD_SUCCESS;
}

EXTERN int __tgt_set_interop_property(
    void *interop_obj, int32_t property_id, void *property_value) {
  DP("Call to __tgt_set_interop_property with interop_obj " DPxMOD
     ", property_id %" PRId32 "\n", DPxPTR(interop_obj), property_id);

  if (IsOffloadDisabled() || !interop_obj || !property_value) {
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

  if (IsOffloadDisabled() || !interop_obj || !property_value) {
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
  default:
    DP("Invalid interop property name " PRId32 "\n");
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

EXTERN void __tgt_push_code_location(const char *location, void *codeptr_ra) {
  OmptGlobal->getTrace().pushCodeLocation(location, codeptr_ra);
}

EXTERN int __tgt_get_num_devices(void) {
  RTLsMtx->lock();
  size_t Devices_size = Devices.size();
  RTLsMtx->unlock();
  DP("Call to omp_get_num_devices returning %zd\n", Devices_size);
  return Devices_size;
}
#endif // INTEL_COLLAB
