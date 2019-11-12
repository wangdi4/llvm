//===-- omptargetplugin.h - Target dependent OpenMP Plugin API --*- C++ -*-===//
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

// Return the number of available devices of the type supported by the
// target RTL.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_number_of_devices(void);

// Return an integer different from zero if the provided device image can be
// supported by the runtime. The functionality is similar to comparing the
// result of __tgt__rtl__load__binary to NULL. However, this is meant to be a
// lightweight query to determine if the RTL is suitable for an image without
// having to load the library, which can be expensive.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image);

// Initialize the requires flags for the device.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int64_t __tgt_rtl_init_requires(int64_t RequiresFlags);

// Initialize the specified device. In case of success return 0; otherwise
// return an error code.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_init_device(int32_t ID);

// Pass an executable image section described by image to the specified
// device and prepare an address table of target entities. In case of error,
// return NULL. Otherwise, return a pointer to the built address table.
// Individual entries in the table may also be NULL, when the corresponding
// offload region is not supported on the target device.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
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
// case an error occurred on the target device.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void *__tgt_rtl_data_alloc(int32_t ID, int64_t Size, void *HostPtr);

// Pass the data content to the target device using the target address.
// In case of success, return zero. Otherwise, return an error code.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_data_submit(int32_t ID, void *TargetPtr, void *HostPtr,
                              int64_t Size);

// Retrieve the data content from the target device using its address.
// In case of success, return zero. Otherwise, return an error code.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_data_retrieve(int32_t ID, void *HostPtr, void *TargetPtr,
                                int64_t Size);

// De-allocate the data referenced by target ptr on the device. In case of
// success, return zero. Otherwise, return an error code.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_data_delete(int32_t ID, void *TargetPtr);

// Transfer control to the offloaded entry Entry on the target device.
// Args and Offsets are arrays of NumArgs size of target addresses and
// offsets. An offset should be added to the target address before passing it
// to the outlined function on device side. In case of success, return zero.
// Otherwise, return an error code.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_run_target_region(int32_t ID, void *Entry, void **Args,
                                    ptrdiff_t *Offsets, int32_t NumArgs);

// Similar to __tgt_rtl_run_target_region, but additionally specify the
// number of teams to be created and a number of threads in each team.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_run_target_team_region(int32_t ID, void *Entry, void **Args,
                                         ptrdiff_t *Offsets, int32_t NumArgs,
                                         int32_t NumTeams, int32_t ThreadLimit,
                                         uint64_t loop_tripcount);

#if INTEL_COLLAB
// Manifest target pointers, which are not passed as arguments,
// to the offloaded entry represented by TgtEntryPtr. The target pointers
// are passed in TgtPtrs array consisting of NumPtrs pointers.
EXTERN
int32_t __tgt_rtl_manifest_data_for_region(int32_t ID, void *TgtEntryPtr,
                                           void **TgtPtrs, size_t NumPtrs);

// Similar to __tgt_rtl_data_alloc, but additionally specify the base host ptr
// in case the plugin needs this information.
EXTERN
void *__tgt_rtl_data_alloc_base(int32_t ID, int64_t Size, void *HostPtr,
                                void *HostBase);

// Similar to __tgt_rtl_data_alloc, but additionally specify that user initiated
// the allocation
EXTERN
void *__tgt_rtl_data_alloc_user(int32_t ID, int64_t Size, void *HostPtr);

// Create a device-specific buffer object from the given device pointer
EXTERN
void *__tgt_rtl_create_buffer(int32_t ID, void *TgtPtr);

// Release a device-specific buffer
EXTERN
int32_t __tgt_rtl_release_buffer(void *TgtBuffer);

// Returns implementation defined device name for the given device number,
// using provided Buffer. Buffer must be able to hold at least BufferMaxSize
// characters. Returns nullptr, if device name cannot be acquired, otherwise,
// returns a '\0' terminated C string (pointer to Buffer).
EXTERN char *__tgt_rtl_get_device_name(
    int32_t ID, char *Buffer, size_t BufferMaxSize);

// Unlike __tgt_rtl_run_target_team_region, a loop descriptor for
// multi-dimensional loop is passed to this function.
EXTERN
int32_t __tgt_rtl_run_target_team_nd_region(int32_t ID, void *Entry,
                                            void **Args, ptrdiff_t *Offsets,
                                            int32_t NumArgs, int32_t NumTeams,
                                            int32_t ThreadLimit,
                                            void *LoopDesc);

// Asynchronous version of __tgt_rtl_run_target_region.
EXTERN
int32_t __tgt_rtl_run_target_region_nowait(int32_t ID, void *Entry, void **Args,
                                           ptrdiff_t *Offsets, int32_t NumArgs,
                                           void *AsyncData);

// Asynchronous version of __tgt_rtl_run_target_team_region.
EXTERN
int32_t __tgt_rtl_run_target_team_region_nowait(
    int32_t ID, void *Entry, void **Args, ptrdiff_t *Offsets, int32_t NumArgs,
    int32_t NumTeams, int32_t ThreadLimit, uint64_t LoopTripCount,
    void *AsyncData);

// Asynchronous version of __tgt_rtl_run_target_team_nd_region.
EXTERN
int32_t __tgt_rtl_run_target_team_nd_region_nowait(
    int32_t ID, void *Entry, void **Args, ptrdiff_t *Offsets, int32_t NumArgs,
    int32_t NumTeams, int32_t ThreadLimit, void *LoopDesc, void *AsyncData);

// Returns opaque handle to a device-dependent offload pipe.
EXTERN
void *__tgt_rtl_get_offload_pipe(int32_t ID);
#endif // INTEL_COLLAB
#ifdef __cplusplus
}
#endif

#endif // _OMPTARGETPLUGIN_H_
