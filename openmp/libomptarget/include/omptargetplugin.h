//===-- omptargetplugin.h - Target dependent OpenMP Plugin API --*- C++ -*-===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
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
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines an interface between target independent OpenMP offload
// runtime library libomptarget and target dependent plugin.
//
//===----------------------------------------------------------------------===//

#ifndef _OMPTARGETPLUGIN_H_
#define _OMPTARGETPLUGIN_H_

#include <omptarget.h>

#ifdef __cplusplus
extern "C" {
#endif

// First method called on the plugin
int32_t __tgt_rtl_init_plugin();

// Last method called on the plugin
int32_t __tgt_rtl_deinit_plugin();

// Return the number of available devices of the type supported by the
// target RTL.
int32_t __tgt_rtl_number_of_devices(void);

// Return an integer different from zero if the provided device image can be
// supported by the runtime. The functionality is similar to comparing the
// result of __tgt__rtl__load__binary to NULL. However, this is meant to be a
// lightweight query to determine if the RTL is suitable for an image without
// having to load the library, which can be expensive.
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image);

// This provides the same functionality as __tgt_rtl_is_valid_binary except we
// also use additional information to determine if the image is valid. This
// allows us to determine if an image has a compatible architecture.
int32_t __tgt_rtl_is_valid_binary_info(__tgt_device_image *Image,
                                       __tgt_image_info *Info);

// Return an integer other than zero if the data can be exchaned from SrcDevId
// to DstDevId. If it is data exchangable, the device plugin should provide
// function to move data from source device to destination device directly.
int32_t __tgt_rtl_is_data_exchangable(int32_t SrcDevId, int32_t DstDevId);

// Return an integer other than zero if the plugin can handle images which do
// not contain target regions and global variables (but can contain other
// functions)
int32_t __tgt_rtl_supports_empty_images();

// Initialize the requires flags for the device.
int64_t __tgt_rtl_init_requires(int64_t RequiresFlags);

// Initialize the specified device. In case of success return 0; otherwise
// return an error code.
int32_t __tgt_rtl_init_device(int32_t ID);

// Deinitialize the specified device. In case of success return 0; otherwise
// return an error code.
int32_t __tgt_rtl_deinit_device(int32_t ID);

// Pass an executable image section described by image to the specified
// device and prepare an address table of target entities. In case of error,
// return NULL. Otherwise, return a pointer to the built address table.
// Individual entries in the table may also be NULL, when the corresponding
// offload region is not supported on the target device.
__tgt_target_table *__tgt_rtl_load_binary(int32_t ID,
                                          __tgt_device_image *Image);

// Allocate data on the particular target device, of the specified size.
// HostPtr is a address of the host data the allocated target data
// will be associated with (HostPtr may be NULL if it is not known at
// allocation time, like for example it would be for target data that
// is allocated by omp_target_alloc() API). Return address of the
// allocated data on the target that will be used by libomptarget.so to
// initialize the target data mapping structures. These addresses are
// used to generate a table of target variables to pass to
// __tgt_rtl_run_region(). The __tgt_rtl_data_alloc() returns NULL in
// case an error occurred on the target device. Kind dictates what allocator
// to use (e.g. shared, host, device).
void *__tgt_rtl_data_alloc(int32_t ID, int64_t Size, void *HostPtr,
                           int32_t Kind);

// Pass the data content to the target device using the target address. In case
// of success, return zero. Otherwise, return an error code.
int32_t __tgt_rtl_data_submit(int32_t ID, void *TargetPtr, void *HostPtr,
                              int64_t Size);

int32_t __tgt_rtl_data_submit_async(int32_t ID, void *TargetPtr, void *HostPtr,
                                    int64_t Size, __tgt_async_info *AsyncInfo);

// Retrieve the data content from the target device using its address. In case
// of success, return zero. Otherwise, return an error code.
int32_t __tgt_rtl_data_retrieve(int32_t ID, void *HostPtr, void *TargetPtr,
                                int64_t Size);

// Asynchronous version of __tgt_rtl_data_retrieve
int32_t __tgt_rtl_data_retrieve_async(int32_t ID, void *HostPtr,
                                      void *TargetPtr, int64_t Size,
                                      __tgt_async_info *AsyncInfo);

// Copy the data content from one target device to another target device using
// its address. This operation does not need to copy data back to host and then
// from host to another device. In case of success, return zero. Otherwise,
// return an error code.
int32_t __tgt_rtl_data_exchange(int32_t SrcID, void *SrcPtr, int32_t DstID,
                                void *DstPtr, int64_t Size);

// Asynchronous version of __tgt_rtl_data_exchange
int32_t __tgt_rtl_data_exchange_async(int32_t SrcID, void *SrcPtr,
                                      int32_t DesID, void *DstPtr, int64_t Size,
                                      __tgt_async_info *AsyncInfo);

// De-allocate the data referenced by target ptr on the device. In case of
// success, return zero. Otherwise, return an error code. Kind dictates what
// allocator to use (e.g. shared, host, device).
int32_t __tgt_rtl_data_delete(int32_t ID, void *TargetPtr, int32_t Kind);

// Transfer control to the offloaded entry Entry on the target device.
// Args and Offsets are arrays of NumArgs size of target addresses and
// offsets. An offset should be added to the target address before passing it
// to the outlined function on device side. If AsyncInfo is nullptr, it is
// synchronous; otherwise it is asynchronous. However, AsyncInfo may be
// ignored on some platforms, like x86_64. In that case, it is synchronous. In
// case of success, return zero. Otherwise, return an error code.
int32_t __tgt_rtl_run_target_region(int32_t ID, void *Entry, void **Args,
                                    ptrdiff_t *Offsets, int32_t NumArgs);

// Asynchronous version of __tgt_rtl_run_target_region
int32_t __tgt_rtl_run_target_region_async(int32_t ID, void *Entry, void **Args,
                                          ptrdiff_t *Offsets, int32_t NumArgs,
                                          __tgt_async_info *AsyncInfo);

// Similar to __tgt_rtl_run_target_region, but additionally specify the
// number of teams to be created and a number of threads in each team. If
// AsyncInfo is nullptr, it is synchronous; otherwise it is asynchronous.
// However, AsyncInfo may be ignored on some platforms, like x86_64. In that
// case, it is synchronous.
int32_t __tgt_rtl_run_target_team_region(int32_t ID, void *Entry, void **Args,
                                         ptrdiff_t *Offsets, int32_t NumArgs,
                                         int32_t NumTeams, int32_t ThreadLimit,
                                         uint64_t LoopTripcount);

// Asynchronous version of __tgt_rtl_run_target_team_region
int32_t __tgt_rtl_run_target_team_region_async(
    int32_t ID, void *Entry, void **Args, ptrdiff_t *Offsets, int32_t NumArgs,
    int32_t NumTeams, int32_t ThreadLimit, uint64_t LoopTripcount,
    __tgt_async_info *AsyncInfo);

// Device synchronization. In case of success, return zero. Otherwise, return an
// error code.
int32_t __tgt_rtl_synchronize(int32_t ID, __tgt_async_info *AsyncInfo);

// Queries for the completion of asynchronous operations. Instead of blocking
// the calling thread as __tgt_rtl_synchronize, the progress of the operations
// stored in AsyncInfo->Queue is queried in a non-blocking manner, partially
// advancing their execution. If all operations are completed, AsyncInfo->Queue
// is set to nullptr. If there are still pending operations, AsyncInfo->Queue is
// kept as a valid queue. In any case of success (i.e., successful query
// with/without completing all operations), return zero. Otherwise, return an
// error code.
int32_t __tgt_rtl_query_async(int32_t ID, __tgt_async_info *AsyncInfo);

// Set plugin's internal information flag externally.
void __tgt_rtl_set_info_flag(uint32_t);

// Print the device information
void __tgt_rtl_print_device_info(int32_t ID);

// Event related interfaces. It is expected to use the interfaces in the
// following way:
// 1) Create an event on the target device (__tgt_rtl_create_event).
// 2) Record the event based on the status of \p AsyncInfo->Queue at the moment
// of function call to __tgt_rtl_record_event. An event becomes "meaningful"
// once it is recorded, such that others can depend on it.
// 3) Call __tgt_rtl_wait_event to set dependence on the event. Whether the
// operation is blocking or non-blocking depends on the target. It is expected
// to be non-blocking, just set dependence and return.
// 4) Call __tgt_rtl_sync_event to sync the event. It is expected to block the
// thread calling the function.
// 5) Destroy the event (__tgt_rtl_destroy_event).
// {
int32_t __tgt_rtl_create_event(int32_t ID, void **Event);

int32_t __tgt_rtl_record_event(int32_t ID, void *Event,
                               __tgt_async_info *AsyncInfo);

int32_t __tgt_rtl_wait_event(int32_t ID, void *Event,
                             __tgt_async_info *AsyncInfo);

int32_t __tgt_rtl_sync_event(int32_t ID, void *Event);

int32_t __tgt_rtl_destroy_event(int32_t ID, void *Event);
// }

int32_t __tgt_rtl_init_async_info(int32_t ID, __tgt_async_info **AsyncInfoPtr);
int32_t __tgt_rtl_init_device_info(int32_t ID, __tgt_device_info *DeviceInfoPtr,
                                   const char **ErrStr);

// lock/pin host memory
int32_t __tgt_rtl_data_lock(int32_t ID, void *HstPtr, int64_t Size,
                            void **LockedPtr);

// unlock/unpin host memory
int32_t __tgt_rtl_data_unlock(int32_t ID, void *HstPtr);

// Notify the plugin about a new mapping starting at the host address \p HstPtr
// and \p Size bytes. The plugin may lock/pin that buffer to achieve optimal
// memory transfers involving that buffer.
int32_t __tgt_rtl_data_notify_mapped(int32_t ID, void *HstPtr, int64_t Size);

// Notify the plugin about an existing mapping being unmapped, starting at the
// host address \p HstPtr and \p Size bytes.
int32_t __tgt_rtl_data_notify_unmapped(int32_t ID, void *HstPtr);

// Set the global device identifier offset, such that the plugin may determine a
// unique device number.
int32_t __tgt_rtl_set_device_offset(int32_t DeviceIdOffset);

#if INTEL_CUSTOMIZATION
int32_t __tgt_rtl_launch_kernel(int32_t ID, void *Entry, void **Args,
                                ptrdiff_t *Offsets, KernelArgsTy *KernelArgs,
                                __tgt_async_info *AsyncInfo);

// Manifest target pointers, which are not passed as arguments,
// to the offloaded entry represented by TgtEntryPtr. The target pointers
// are passed in TgtPtrs array consisting of NumPtrs pointers.
int32_t __tgt_rtl_manifest_data_for_region(int32_t ID, void *TgtEntryPtr,
                                           void **TgtPtrs, size_t NumPtrs);

// Similar to __tgt_rtl_data_alloc, but additionally specify the base host ptr
// in case the plugin needs this information.
// AllocOpt is used to indicate where the memory should be allocated.
void *__tgt_rtl_data_alloc_base(int32_t ID, int64_t Size, void *HostPtr,
                                void *HostBase, int32_t AllocOpt);

// Entry for supporting realloc
void *__tgt_rtl_data_realloc(int32_t ID, void *Ptr, size_t Size, int32_t Kind);

// Entry for supporting aligned_alloc
void *__tgt_rtl_data_aligned_alloc(int32_t ID, size_t Align, size_t Size,
                                   int32_t Kind);

// Entry for supporting registering of host_pointer
bool __tgt_rtl_register_host_pointer(int32_t ID, void *Ptr, size_t Size);

// Entry for supporting unregistering of host pointer
bool __tgt_rtl_unregister_host_pointer(int32_t ID, void *Ptr);

// Returns implementation defined device name for the given device number,
// using provided Buffer. Buffer must be able to hold at least BufferMaxSize
// characters. Returns nullptr, if device name cannot be acquired, otherwise,
// returns a '\0' terminated C string (pointer to Buffer).
char *__tgt_rtl_get_device_name(int32_t ID, char *Buffer, size_t BufferMaxSize);

// Unlike __tgt_rtl_run_target_team_region, a loop descriptor for
// multi-dimensional loop is passed to this function.
int32_t __tgt_rtl_run_target_team_nd_region(int32_t ID, void *Entry,
                                            void **Args, ptrdiff_t *Offsets,
                                            int32_t NumArgs, int32_t NumTeams,
                                            int32_t ThreadLimit, void *LoopDesc,
                                            __tgt_async_info *AsyncInfo);

int32_t __tgt_rtl_get_groups_shape(int32_t DeviceId, int32_t NumTeams,
                                   int32_t ThreadLimit, void *TgtEntryPtr,
                                   void *GroupSizes, void *GroupCounts,
                                   void *LoopDesc);

// Creates an opaque handle to the  context handle.
void *__tgt_rtl_get_context_handle(int32_t ID);

// Check if the specified pointer and size requires mapping.
int32_t __tgt_rtl_requires_mapping(int32_t ID, void *Ptr, int64_t Size);

// Get target memory allocation information
int32_t __tgt_rtl_get_data_alloc_info(int32_t ID, int32_t NumPtrs, void *Ptrs,
                                      void *Info);

// Push subdevice encoding
int32_t __tgt_rtl_push_subdevice(int64_t ID);

// Pop subdevice encoding
int32_t __tgt_rtl_pop_subdevice(void);

// Check if the specified device type is supported
int32_t __tgt_rtl_is_supported_device(int32_t ID, void *DeviceType);

// Create OpenMP interop with the given interop context
__tgt_interop *__tgt_rtl_create_interop(int32_t ID, int32_t InteropContext,
                                        int32_t NumPrefers, int32_t *PreferIDs);

// Release OpenMP interop
int32_t __tgt_rtl_release_interop(int32_t ID, __tgt_interop *Interop);

// Change OpenMP interop to usable state
int32_t __tgt_rtl_use_interop(int32_t ID, __tgt_interop *Interop);

// Get number of implementation-defined interop properties
int32_t __tgt_rtl_get_num_interop_properties(int32_t ID);

// Get interop property value from plugin
int32_t __tgt_rtl_get_interop_property_value(int32_t ID, __tgt_interop *Interop,
                                             int32_t Property,
                                             int32_t ValueType, size_t Size,
                                             void *Value);

// Get interop property info from plugin
const char *__tgt_rtl_get_interop_property_info(int32_t ID, int32_t Property,
                                                int32_t InfoType);

// Get interop return code description from plugin
const char *__tgt_rtl_get_interop_rc_desc(int32_t ID, int32_t Rc);

int32_t __tgt_rtl_flush_queue(__tgt_interop *Interop);
int32_t __tgt_rtl_sync_barrier(__tgt_interop *Interop);
int32_t __tgt_rtl_async_barrier(__tgt_interop *Interop);

// Return number of available sub-devices at the given level
int32_t __tgt_rtl_get_num_sub_devices(int32_t ID, int32_t Level);

// Check if given host pointer and size is accessible by device
int32_t __tgt_rtl_is_accessible_addr_range(int32_t ID, const void *Ptr,
                                           size_t Size);

// Notify indirectly accessed target pointer
int32_t __tgt_rtl_notify_indirect_access(int32_t ID, const void *Ptr,
                                         size_t Offset);

// Check if the RTL expects that the outlined function's argument
// specified by Idx will be passed from libomptarget to the RTL
// in the host memory.
int32_t __tgt_rtl_is_private_arg_on_host(int32_t ID, const void *TgtEntryPtr,
                                         uint32_t Idx);

// Begin data batch commands
int32_t __tgt_rtl_command_batch_begin(int32_t ID, int32_t BatchLevel);

// End data batch commands
int32_t __tgt_rtl_command_batch_end(int32_t ID, int32_t BatchLevel);

void __tgt_rtl_kernel_batch_begin(int32_t ID, uint32_t MaxKernels);
void __tgt_rtl_kernel_batch_end(int32_t ID);

// Access device information
int __tgt_rtl_get_device_info(int32_t ID, int32_t InfoID, size_t InfoSize,
                              void *InfoVal, size_t *InfoSizeRet);

int32_t
__tgt_rtl_set_function_ptr_map(int32_t ID, uint64_t Size,
                               const __omp_offloading_fptr_map_t *FnPtrs);

// Allocate shared memory with access hint
void *__tgt_rtl_data_aligned_alloc_shared(int32_t ID, size_t Align, size_t Size,
                                          int32_t AccessHint);

// Prefetch shared memory
int __tgt_rtl_prefetch_shared_mem(int32_t ID, size_t NumPtrs, void **Ptrs,
                                  size_t *Sizes);

// Return the device ID that owns the specified memory location
int __tgt_rtl_get_device_from_ptr(const void *Ptr);

int __tgt_rtl_memcpy_rect_3d(int32_t ID, void *Dst, const void *Src,
                             size_t ElementSize, int32_t NumDims,
                             const size_t *Volume, const size_t *DstOffsets,
                             const size_t *SrcOffsets, const size_t *DstDims,
                             const size_t *SrcDims);

/// Returns number of resources and list of unique resource IDs for the given
/// device information and base memory space. If "ResourceIds" is null, it
/// only returns number of available resources.
int32_t __tgt_rtl_get_mem_resources(int32_t NumDevices,
                                    const int32_t *DeviceIds,
                                    int32_t HostAccess,
                                    omp_memspace_handle_t MemSpace,
                                    int32_t *ResourceIds);

/// Allocates memory with the specified OMP allocator
void *__tgt_rtl_omp_alloc(size_t Size, omp_allocator_handle_t Allocator);

/// Releases memory with the specified OMP allocator
void __tgt_rtl_omp_free(void *Ptr, omp_allocator_handle_t Allocator);
#endif // INTEL_CUSTOMIZATION

#ifdef __cplusplus
}
#endif

#endif // _OMPTARGETPLUGIN_H_
