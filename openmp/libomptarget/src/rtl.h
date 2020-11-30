//===------------ rtl.h - Target independent OpenMP target RTL ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declarations for handling RTL plugins.
//
//===----------------------------------------------------------------------===//

#ifndef _OMPTARGET_RTL_H
#define _OMPTARGET_RTL_H

#include "omptarget.h"
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Forward declarations.
struct DeviceTy;
struct __tgt_bin_desc;

struct RTLInfoTy {
  typedef int32_t(is_valid_binary_ty)(void *);
  typedef int32_t(is_data_exchangable_ty)(int32_t, int32_t);
  typedef int32_t(number_of_devices_ty)();
  typedef int32_t(init_device_ty)(int32_t);
  typedef __tgt_target_table *(load_binary_ty)(int32_t, void *);
  typedef void *(data_alloc_ty)(int32_t, int64_t, void *);
  typedef int32_t(data_submit_ty)(int32_t, void *, void *, int64_t);
  typedef int32_t(data_submit_async_ty)(int32_t, void *, void *, int64_t,
                                        __tgt_async_info *);
  typedef int32_t(data_retrieve_ty)(int32_t, void *, void *, int64_t);
  typedef int32_t(data_retrieve_async_ty)(int32_t, void *, void *, int64_t,
                                          __tgt_async_info *);
  typedef int32_t(data_exchange_ty)(int32_t, void *, int32_t, void *, int64_t);
  typedef int32_t(data_exchange_async_ty)(int32_t, void *, int32_t, void *,
                                          int64_t, __tgt_async_info *);
  typedef int32_t(data_delete_ty)(int32_t, void *);
  typedef int32_t(run_region_ty)(int32_t, void *, void **, ptrdiff_t *,
                                 int32_t);
  typedef int32_t(run_region_async_ty)(int32_t, void *, void **, ptrdiff_t *,
                                       int32_t, __tgt_async_info *);
  typedef int32_t(run_team_region_ty)(int32_t, void *, void **, ptrdiff_t *,
                                      int32_t, int32_t, int32_t, uint64_t);
  typedef int32_t(run_team_region_async_ty)(int32_t, void *, void **,
                                            ptrdiff_t *, int32_t, int32_t,
                                            int32_t, uint64_t,
                                            __tgt_async_info *);
  typedef int64_t(init_requires_ty)(int64_t);
  typedef int64_t(synchronize_ty)(int32_t, __tgt_async_info *);
#if INTEL_COLLAB
  typedef int32_t(data_submit_nowait_ty)(int32_t, void *, void *, int64_t,
                                         void *);
  typedef int32_t(data_retrieve_nowait_ty)(int32_t, void *, void *, int64_t,
                                           void *);
  typedef int32_t(manifest_data_for_region_ty)(int32_t, void *,
                                               void **, size_t);
  typedef void *(data_alloc_base_ty)(int32_t, int64_t, void *, void *);
  typedef void *(data_alloc_user_ty)(int32_t, int64_t, void *);
  typedef char *(get_device_name_ty)(int32_t, char *, size_t);
  typedef int32_t(run_team_nd_region_ty)(int32_t, void *, void **, ptrdiff_t *,
                                         int32_t, int32_t, int32_t, void *);
  typedef int32_t(run_team_nd_region_nowait_ty)(int32_t, void *, void **,
                                                ptrdiff_t *, int32_t, int32_t,
                                                int32_t, void *, void *);
  typedef int32_t(run_region_nowait_ty)(int32_t, void *, void **, ptrdiff_t *,
                                        int32_t, void *);
  typedef int32_t(run_team_region_nowait_ty)(int32_t, void *, void **,
                                             ptrdiff_t *, int32_t, int32_t,
                                             int32_t, uint64_t, void *);
  typedef void *(create_offload_queue_ty)(int32_t, bool);
  typedef void *(get_platform_handle_ty)(int32_t);
  typedef void *(get_device_handle_ty)(int32_t);
  typedef void *(get_context_handle_ty)(int32_t);
  typedef int32_t(release_offload_queue_ty)(int32_t, void *);
  typedef void *(data_alloc_managed_ty)(int32_t, int64_t);
  typedef int32_t(is_device_accessible_ptr_ty)(int32_t, void *);
  typedef void *(data_alloc_explicit_ty)(int32_t, int64_t, int32_t);
  typedef void (init_ompt_ty)(void *);
  typedef int32_t(get_data_alloc_info_ty)(int32_t, int32_t, void *, void *);
  typedef int32_t(push_subdevice_ty)(int64_t);
  typedef int32_t(pop_subdevice_ty)(void);
  typedef void (add_build_options_ty)(const char *, const char *);
#endif // INTEL_COLLAB

  int32_t Idx = -1;             // RTL index, index is the number of devices
                                // of other RTLs that were registered before,
                                // i.e. the OpenMP index of the first device
                                // to be registered with this RTL.
  int32_t NumberOfDevices = -1; // Number of devices this RTL deals with.

  void *LibraryHandler = nullptr;

#ifdef OMPTARGET_DEBUG
  std::string RTLName;
#endif
#if INTEL_COLLAB
  // FIXME: can we combine this with RTLName, when open-sourcing?
  const char *RTLConstName = "";
#endif  // INTEL_COLLAB

  // Functions implemented in the RTL.
  is_valid_binary_ty *is_valid_binary = nullptr;
  is_data_exchangable_ty *is_data_exchangable = nullptr;
  number_of_devices_ty *number_of_devices = nullptr;
  init_device_ty *init_device = nullptr;
  load_binary_ty *load_binary = nullptr;
  data_alloc_ty *data_alloc = nullptr;
  data_submit_ty *data_submit = nullptr;
  data_submit_async_ty *data_submit_async = nullptr;
  data_retrieve_ty *data_retrieve = nullptr;
  data_retrieve_async_ty *data_retrieve_async = nullptr;
  data_exchange_ty *data_exchange = nullptr;
  data_exchange_async_ty *data_exchange_async = nullptr;
  data_delete_ty *data_delete = nullptr;
  run_region_ty *run_region = nullptr;
  run_region_async_ty *run_region_async = nullptr;
  run_team_region_ty *run_team_region = nullptr;
  run_team_region_async_ty *run_team_region_async = nullptr;
  init_requires_ty *init_requires = nullptr;
  synchronize_ty *synchronize = nullptr;
#if INTEL_COLLAB
  data_submit_nowait_ty *data_submit_nowait = nullptr;
  data_retrieve_nowait_ty *data_retrieve_nowait = nullptr;
  manifest_data_for_region_ty *manifest_data_for_region = nullptr;
  data_alloc_base_ty *data_alloc_base = nullptr;
  data_alloc_user_ty *data_alloc_user = nullptr;
  get_device_name_ty *get_device_name = nullptr;
  run_team_nd_region_ty *run_team_nd_region = nullptr;
  run_team_nd_region_nowait_ty *run_team_nd_region_nowait = nullptr;
  run_region_nowait_ty *run_region_nowait = nullptr;
  run_team_region_nowait_ty *run_team_region_nowait = nullptr;
  create_offload_queue_ty *create_offload_queue = nullptr;
  get_platform_handle_ty *get_platform_handle = nullptr;
  get_device_handle_ty *get_device_handle = nullptr;
  get_context_handle_ty *get_context_handle = nullptr;
  release_offload_queue_ty *release_offload_queue = nullptr;
  data_alloc_managed_ty *data_alloc_managed = nullptr;
  is_device_accessible_ptr_ty *is_device_accessible_ptr = nullptr;
  data_alloc_explicit_ty *data_alloc_explicit = nullptr;
  init_ompt_ty *init_ompt = nullptr;
  get_data_alloc_info_ty *get_data_alloc_info = nullptr;
  push_subdevice_ty *push_subdevice = nullptr;
  pop_subdevice_ty *pop_subdevice = nullptr;
  add_build_options_ty *add_build_options = nullptr;
#endif // INTEL_COLLAB

  // Are there images associated with this RTL.
  bool isUsed = false;

  // Mutex for thread-safety when calling RTL interface functions.
  // It is easier to enforce thread-safety at the libomptarget level,
  // so that developers of new RTLs do not have to worry about it.
  std::mutex Mtx;

  // The existence of the mutex above makes RTLInfoTy non-copyable.
  // We need to provide a copy constructor explicitly.
  RTLInfoTy() = default;

  RTLInfoTy(const RTLInfoTy &r) {
    Idx = r.Idx;
    NumberOfDevices = r.NumberOfDevices;
    LibraryHandler = r.LibraryHandler;
#ifdef OMPTARGET_DEBUG
    RTLName = r.RTLName;
#endif
#if INTEL_COLLAB
    RTLConstName = r.RTLConstName;
#endif  // INTEL_COLLAB
    is_valid_binary = r.is_valid_binary;
    is_data_exchangable = r.is_data_exchangable;
    number_of_devices = r.number_of_devices;
    init_device = r.init_device;
    load_binary = r.load_binary;
    data_alloc = r.data_alloc;
    data_submit = r.data_submit;
    data_submit_async = r.data_submit_async;
    data_retrieve = r.data_retrieve;
    data_retrieve_async = r.data_retrieve_async;
    data_exchange = r.data_exchange;
    data_exchange_async = r.data_exchange_async;
    data_delete = r.data_delete;
    run_region = r.run_region;
    run_region_async = r.run_region_async;
    run_team_region = r.run_team_region;
    run_team_region_async = r.run_team_region_async;
    init_requires = r.init_requires;
#if INTEL_COLLAB
    data_submit_nowait = r.data_submit_nowait;
    data_retrieve_nowait = r.data_retrieve_nowait;
    manifest_data_for_region = r.manifest_data_for_region;
    data_alloc_base = r.data_alloc_base;
    data_alloc_user = r.data_alloc_user;
    get_device_name = r.get_device_name;
    run_team_nd_region = r.run_team_nd_region;
    run_team_nd_region_nowait = r.run_team_nd_region_nowait;
    run_region_nowait = r.run_region_nowait;
    run_team_region_nowait = r.run_team_region_nowait;
    create_offload_queue = r.create_offload_queue;
    get_platform_handle = r.get_platform_handle;
    get_device_handle = r.get_device_handle;
    get_context_handle = r.get_context_handle;
    release_offload_queue = r.release_offload_queue;
    data_alloc_managed = r.data_alloc_managed;
    is_device_accessible_ptr = r.is_device_accessible_ptr;
    data_alloc_explicit = r.data_alloc_explicit;
    init_ompt = r.init_ompt;
    get_data_alloc_info = r.get_data_alloc_info;
    push_subdevice = r.push_subdevice;
    pop_subdevice = r.pop_subdevice;
    add_build_options = r.add_build_options;
#endif // INTEL_COLLAB
    isUsed = r.isUsed;
    synchronize = r.synchronize;
  }
};

/// RTLs identified in the system.
class RTLsTy {
private:
  // Mutex-like object to guarantee thread-safety and unique initialization
  // (i.e. the library attempts to load the RTLs (plugins) only once).
  std::once_flag initFlag;
  void LoadRTLs(); // not thread-safe

public:
  // List of the detected runtime libraries.
  std::list<RTLInfoTy> AllRTLs;

  // Array of pointers to the detected runtime libraries that have compatible
  // binaries.
  std::vector<RTLInfoTy *> UsedRTLs;

#if INTEL_COLLAB
  int64_t RequiresFlags = OMP_REQ_UNDEFINED;
#else  // INTEL_COLLAB
  int64_t RequiresFlags;
#endif // INTEL_COLLAB

  explicit RTLsTy() = default;

  // Register the clauses of the requires directive.
  void RegisterRequires(int64_t flags);

  // Register a shared library with all (compatible) RTLs.
  void RegisterLib(__tgt_bin_desc *desc);

  // Unregister a shared library from all RTLs.
  void UnregisterLib(__tgt_bin_desc *desc);
};


/// Map between the host entry begin and the translation table. Each
/// registered library gets one TranslationTable. Use the map from
/// __tgt_offload_entry so that we may quickly determine whether we
/// are trying to (re)register an existing lib or really have a new one.
struct TranslationTable {
  __tgt_target_table HostTable;

  // Image assigned to a given device.
  std::vector<__tgt_device_image *> TargetsImages; // One image per device ID.

  // Table of entry points or NULL if it was not already computed.
  std::vector<__tgt_target_table *> TargetsTable; // One table per device ID.
};
typedef std::map<__tgt_offload_entry *, TranslationTable>
    HostEntriesBeginToTransTableTy;

/// Map between the host ptr and a table index
struct TableMap {
  TranslationTable *Table = nullptr; // table associated with the host ptr.
  uint32_t Index = 0; // index in which the host ptr translated entry is found.
  TableMap() = default;
  TableMap(TranslationTable *table, uint32_t index)
      : Table(table), Index(index) {}
};
typedef std::map<void *, TableMap> HostPtrToTableMapTy;

#endif
