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
  typedef int32_t(deinit_device_ty)(int32_t);
  typedef __tgt_target_table *(load_binary_ty)(int32_t, void *);
  typedef void *(data_alloc_ty)(int32_t, int64_t, void *, int32_t);
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
  typedef int32_t(synchronize_ty)(int32_t, __tgt_async_info *);
#if INTEL_COLLAB
  typedef int32_t(manifest_data_for_region_ty)(int32_t, void *,
                                               void **, size_t);
  typedef void *(data_alloc_base_ty)(int32_t, int64_t, void *, void *);
  typedef char *(get_device_name_ty)(int32_t, char *, size_t);
  typedef int32_t(run_team_nd_region_ty)(int32_t, void *, void **, ptrdiff_t *,
                                         int32_t, int32_t, int32_t, void *);
  typedef void (get_offload_queue_ty)(int32_t, void *, bool);
  typedef void *(get_platform_handle_ty)(int32_t);
  typedef void (set_device_handle_ty)(int32_t, void *);
  typedef void *(get_context_handle_ty)(int32_t);
  typedef int32_t(release_offload_queue_ty)(int32_t, void *);
  typedef void *(data_alloc_managed_ty)(int32_t, int64_t);
  typedef void *(data_realloc_ty)(int32_t, void *, size_t, int32_t);
  typedef void *(data_aligned_alloc_ty)(int32_t, size_t, size_t, int32_t);
  typedef int32_t(requires_mapping_ty)(int32_t, void *);
  typedef void (init_ompt_ty)(void *);
  typedef int32_t(get_data_alloc_info_ty)(int32_t, int32_t, void *, void *);
  typedef int32_t(push_subdevice_ty)(int64_t);
  typedef int32_t(pop_subdevice_ty)(void);
  typedef void (add_build_options_ty)(const char *, const char *);
  typedef int32_t(is_supported_device_ty)(int32_t, void *);
  typedef void (deinit_ty)(void);
  typedef __tgt_interop *(create_interop_ty)(int32_t, int32_t, int32_t,
                                             intptr_t *);
  typedef int32_t(release_interop_ty)(int32_t, __tgt_interop *);
  typedef int32_t(use_interop_ty)(int32_t, __tgt_interop *);
  typedef int32_t(get_num_interop_properties_ty)(int32_t);
  typedef int32_t(get_interop_property_value_ty)(int32_t, __tgt_interop *,
                                                 int32_t, int32_t, size_t,
                                                 void *);
  typedef const char *(get_interop_property_info_ty)(int32_t, int32_t, int32_t);
  typedef const char *(get_interop_rc_desc_ty)(int32_t, int32_t);
  typedef int32_t(get_num_sub_devices_ty)(int32_t, int32_t);
  typedef int32_t(is_accessible_addr_range_ty)(int32_t, const void *, size_t);
  typedef int32_t(notify_indirect_access_ty)(int32_t, const void *, size_t);
  typedef int32_t(is_private_arg_on_host_ty)(int32_t, const void *, uint32_t);
  typedef int32_t(command_batch_begin_ty)(int32_t, int32_t);
  typedef int32_t(command_batch_end_ty)(int32_t, int32_t);
  typedef void(kernel_batch_begin_ty)(int32_t, uint32_t);
  typedef void(kernel_batch_end_ty)(int32_t);
  typedef int32_t(set_function_ptr_map_ty)(int32_t, uint64_t,
                                           const __omp_offloading_fptr_map_t *);
  typedef void *(alloc_per_hw_thread_scratch_ty)(int32_t, size_t, int32_t);
  typedef void(free_per_hw_thread_scratch_ty)(int32_t, void *);
#endif // INTEL_COLLAB
  typedef int32_t (*register_lib_ty)(__tgt_bin_desc *);
  typedef int32_t(supports_empty_images_ty)();
  typedef void(print_device_info_ty)(int32_t);
  typedef void(set_info_flag_ty)(uint32_t);
  typedef int32_t(create_event_ty)(int32_t, void **);
  typedef int32_t(record_event_ty)(int32_t, void *, __tgt_async_info *);
  typedef int32_t(wait_event_ty)(int32_t, void *, __tgt_async_info *);
  typedef int32_t(sync_event_ty)(int32_t, void *);
  typedef int32_t(destroy_event_ty)(int32_t, void *);
  typedef int32_t(release_async_info_ty)(int32_t, __tgt_async_info *);
  typedef int32_t(init_async_info_ty)(int32_t, __tgt_async_info **);
  typedef int64_t(init_device_into_ty)(int64_t, __tgt_device_info *,
                                       const char **);

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
  deinit_device_ty *deinit_device = nullptr;
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
  manifest_data_for_region_ty *manifest_data_for_region = nullptr;
  data_alloc_base_ty *data_alloc_base = nullptr;
  get_device_name_ty *get_device_name = nullptr;
  run_team_nd_region_ty *run_team_nd_region = nullptr;
  get_offload_queue_ty *get_offload_queue = nullptr;
  get_platform_handle_ty *get_platform_handle = nullptr;
  set_device_handle_ty *set_device_handle = nullptr;
  get_context_handle_ty *get_context_handle = nullptr;
  release_offload_queue_ty *release_offload_queue = nullptr;
  data_alloc_managed_ty *data_alloc_managed = nullptr;
  data_realloc_ty *data_realloc = nullptr;
  data_aligned_alloc_ty *data_aligned_alloc = nullptr;
  requires_mapping_ty *requires_mapping = nullptr;
  init_ompt_ty *init_ompt = nullptr;
  get_data_alloc_info_ty *get_data_alloc_info = nullptr;
  push_subdevice_ty *push_subdevice = nullptr;
  pop_subdevice_ty *pop_subdevice = nullptr;
  add_build_options_ty *add_build_options = nullptr;
  is_supported_device_ty *is_supported_device = nullptr;
  deinit_ty *deinit = nullptr;
  create_interop_ty *create_interop = nullptr;
  release_interop_ty *release_interop = nullptr;
  use_interop_ty *use_interop = nullptr;
  get_num_interop_properties_ty *get_num_interop_properties = nullptr;
  get_interop_property_value_ty *get_interop_property_value = nullptr;
  get_interop_property_info_ty *get_interop_property_info = nullptr;
  get_interop_rc_desc_ty *get_interop_rc_desc = nullptr;
  get_num_sub_devices_ty *get_num_sub_devices = nullptr;
  is_accessible_addr_range_ty *is_accessible_addr_range = nullptr;
  notify_indirect_access_ty *notify_indirect_access = nullptr;
  is_private_arg_on_host_ty *is_private_arg_on_host = nullptr;
  command_batch_begin_ty *command_batch_begin = nullptr;
  command_batch_end_ty *command_batch_end = nullptr;
  kernel_batch_begin_ty *kernel_batch_begin = nullptr;
  kernel_batch_end_ty *kernel_batch_end = nullptr;
  set_function_ptr_map_ty *set_function_ptr_map = nullptr;
  alloc_per_hw_thread_scratch_ty *alloc_per_hw_thread_scratch = nullptr;
  free_per_hw_thread_scratch_ty *free_per_hw_thread_scratch = nullptr;
#endif // INTEL_COLLAB
  register_lib_ty register_lib = nullptr;
  register_lib_ty unregister_lib = nullptr;
  supports_empty_images_ty *supports_empty_images = nullptr;
  set_info_flag_ty *set_info_flag = nullptr;
  print_device_info_ty *print_device_info = nullptr;
  create_event_ty *create_event = nullptr;
  record_event_ty *record_event = nullptr;
  wait_event_ty *wait_event = nullptr;
  sync_event_ty *sync_event = nullptr;
  destroy_event_ty *destroy_event = nullptr;
  init_async_info_ty *init_async_info = nullptr;
  init_device_into_ty *init_device_info = nullptr;
  release_async_info_ty *release_async_info = nullptr;

  // Are there images associated with this RTL.
  bool isUsed = false;

  // Mutex for thread-safety when calling RTL interface functions.
  // It is easier to enforce thread-safety at the libomptarget level,
  // so that developers of new RTLs do not have to worry about it.
  std::mutex Mtx;
};

/// RTLs identified in the system.
struct RTLsTy {
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

  // Initialize RTL if it has not been initialized
  void initRTLonce(RTLInfoTy &RTL);

  // Initialize all RTLs
  void initAllRTLs();

  // Register a shared library with all (compatible) RTLs.
  void RegisterLib(__tgt_bin_desc *desc);

  // Unregister a shared library from all RTLs.
  void UnregisterLib(__tgt_bin_desc *desc);

  // Mutex-like object to guarantee thread-safety and unique initialization
  // (i.e. the library attempts to load the RTLs (plugins) only once).
  std::once_flag initFlag;
  void LoadRTLs(); // not thread-safe
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
