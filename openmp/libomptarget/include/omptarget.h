//===-------- omptarget.h - Target independent OpenMP target RTL -- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Interface to be used by Clang during the codegen of a
// target region.
//
//===----------------------------------------------------------------------===//

#ifndef _OMPTARGET_H_
#define _OMPTARGET_H_

#include <deque>
#include <stddef.h>
#include <stdint.h>

#include <SourceInfo.h>

#define OFFLOAD_SUCCESS (0)
#define OFFLOAD_FAIL (~0)

#define OFFLOAD_DEVICE_DEFAULT -1

// Don't format out enums and structs.
// clang-format off

/// return flags of __tgt_target_XXX public APIs
enum __tgt_target_return_t : int {
  /// successful offload executed on a target device
  OMP_TGT_SUCCESS = 0,
  /// offload may not execute on the requested target device
  /// this scenario can be caused by the device not available or unsupported
  /// as described in the Execution Model in the specifcation
  /// this status may not be used for target device execution failure
  /// which should be handled internally in libomptarget
  OMP_TGT_FAIL = ~0
};

/// Data attributes for each data reference used in an OpenMP target region.
#if INTEL_COLLAB
enum tgt_map_type : uint64_t {
#else  // INTEL_COLLAB
enum tgt_map_type {
#endif // INTEL_COLLAB
  // No flags
  OMP_TGT_MAPTYPE_NONE            = 0x000,
  // copy data from host to device
  OMP_TGT_MAPTYPE_TO              = 0x001,
  // copy data from device to host
  OMP_TGT_MAPTYPE_FROM            = 0x002,
  // copy regardless of the reference count
  OMP_TGT_MAPTYPE_ALWAYS          = 0x004,
  // force unmapping of data
  OMP_TGT_MAPTYPE_DELETE          = 0x008,
  // map the pointer as well as the pointee
  OMP_TGT_MAPTYPE_PTR_AND_OBJ     = 0x010,
  // pass device base address to kernel
  OMP_TGT_MAPTYPE_TARGET_PARAM    = 0x020,
  // return base device address of mapped data
  OMP_TGT_MAPTYPE_RETURN_PARAM    = 0x040,
  // private variable - not mapped
  OMP_TGT_MAPTYPE_PRIVATE         = 0x080,
  // copy by value - not mapped
  OMP_TGT_MAPTYPE_LITERAL         = 0x100,
  // mapping is implicit
  OMP_TGT_MAPTYPE_IMPLICIT        = 0x200,
  // copy data to device
  OMP_TGT_MAPTYPE_CLOSE           = 0x400,
#if INTEL_COLLAB
  OMP_TGT_MAPTYPE_ND_DESC         = 0x800,
#endif // INTEL_COLLAB
  // runtime error if not already allocated
  OMP_TGT_MAPTYPE_PRESENT         = 0x1000,
  // use a separate reference counter so that the data cannot be unmapped within
  // the structured region
  // This is an OpenMP extension for the sake of OpenACC support.
  OMP_TGT_MAPTYPE_OMPX_HOLD       = 0x2000,
  // descriptor for non-contiguous target-update
  OMP_TGT_MAPTYPE_NON_CONTIG      = 0x100000000000,
  // member of struct, member given by [16 MSBs] - 1
  OMP_TGT_MAPTYPE_MEMBER_OF       = 0xffff000000000000
};

enum OpenMPOffloadingDeclareTargetFlags {
  /// Mark the entry as having a 'link' attribute.
  OMP_DECLARE_TARGET_LINK = 0x01,
  /// Mark the entry as being a global constructor.
  OMP_DECLARE_TARGET_CTOR = 0x02,
  /// Mark the entry as being a global destructor.
  OMP_DECLARE_TARGET_DTOR = 0x04
#if INTEL_COLLAB
  ,
  /// Mark the entry as being a function pointer.
  OMP_DECLARE_TARGET_FPTR = 0x8
#endif // INTEL_COLLAB
};

enum OpenMPOffloadingRequiresDirFlags {
  /// flag undefined.
  OMP_REQ_UNDEFINED               = 0x000,
  /// no requires directive present.
  OMP_REQ_NONE                    = 0x001,
  /// reverse_offload clause.
  OMP_REQ_REVERSE_OFFLOAD         = 0x002,
  /// unified_address clause.
  OMP_REQ_UNIFIED_ADDRESS         = 0x004,
  /// unified_shared_memory clause.
  OMP_REQ_UNIFIED_SHARED_MEMORY   = 0x008,
  /// dynamic_allocators clause.
  OMP_REQ_DYNAMIC_ALLOCATORS      = 0x010
};

enum TargetAllocTy : int32_t {
  TARGET_ALLOC_DEVICE = 0,
  TARGET_ALLOC_HOST,
  TARGET_ALLOC_SHARED,
  TARGET_ALLOC_DEFAULT
};

#if INTEL_COLLAB
///
/// OpenMP 5.1 interop support types
///
typedef intptr_t omp_intptr_t;
typedef void * omp_interop_t;
#define omp_interop_none 0

// 0..omp_get_num_interop_properties()-1 are reserved for implementation-defined
// properties
typedef enum omp_interop_property {
    omp_ipr_fr_id = -1,
    omp_ipr_fr_name = -2,
    omp_ipr_vendor = -3,
    omp_ipr_vendor_name = -4,
    omp_ipr_device_num = -5,
    omp_ipr_platform = -6,
    omp_ipr_device = -7,
    omp_ipr_device_context = -8,
    omp_ipr_targetsync = -9,
    omp_ipr_first = -9
} omp_interop_property_t;

typedef enum omp_interop_rc {
    omp_irc_no_value = 1,
    omp_irc_success = 0,
    omp_irc_empty = -1,
    omp_irc_out_of_range = -2,
    omp_irc_type_int = -3,
    omp_irc_type_ptr = -4,
    omp_irc_type_str = -5,
    omp_irc_other = -6
} omp_interop_rc_t;

enum OmpIprValueTy : int32_t {
  OMP_IPR_VALUE_INT = 0,
  OMP_IPR_VALUE_PTR,
  OMP_IPR_VALUE_STR
};

enum OmpIprInfoTy : int32_t {
  OMP_IPR_INFO_NAME = 0,
  OMP_IPR_INFO_TYPE_DESC
};

enum OmpInteropContextTy: int32_t {
  OMP_INTEROP_CONTEXT_TARGET = 0,
  OMP_INTEROP_CONTEXT_TARGETSYNC
};

/// Common interop properties defined in OpenMP 5.1
struct __tgt_interop {
  intptr_t FrId;
  const char *FrName;
  intptr_t Vendor;
  const char *VendorName;
  intptr_t DeviceNum;
  void *Platform;
  void *Device;
  void *DeviceContext;
  void *TargetSync;
  void *RTLProperty; // implementation-defined interop property
};


///
/// Custom interop support types
///
enum InteropPropertyTy : int32_t {
  INTEROP_DEVICE_ID = 1,
  INTEROP_IS_ASYNC,
  INTEROP_ASYNC_OBJ,
  INTEROP_ASYNC_CALLBACK,
  INTEROP_OFFLOAD_QUEUE,
  INTEROP_PLATFORM_HANDLE,
  INTEROP_CONTEXT =  INTEROP_PLATFORM_HANDLE,
  INTEROP_DRIVER_HANDLE =  INTEROP_PLATFORM_HANDLE,
  INTEROP_DEVICE_HANDLE,
  INTEROP_PLUGIN_INTERFACE,
  INTEROP_CONTEXT_HANDLE
};

enum InteropPluginInterfaceTy : int32_t {
  INTEROP_PLUGIN_OPENCL = 1,
  INTEROP_PLUGIN_LEVEL0,
  INTEROP_PLUGIN_X86_64
};

struct __tgt_interop_obj {
  int64_t device_id; // OpenMP device id
  int64_t device_code; // Encoded device id
  bool is_async; // Whether it is for asynchronous operation
  void *async_obj; // Pointer to the asynchronous object
  void (*async_handler)(void *); // Callback function for asynchronous operation
  void *queue; // Opaque handle to device-dependent offload queue
  void *platform_handle; // Opaque handle:  For opencl  cl_context,
                         //for level0  ze_driver_handle_t
  void *device_handle; // Opaque handle:  For level0 ze_device_handle_t.
                       // Not valid for opencl
  void *context_handle; // Opaque handle:  For level0 ze_context_handle_t.
                       // Not valid for opencl
  int32_t plugin_interface; // Plugin selector
};

struct __tgt_memory_info {
  void *Base;       // Base address
  uintptr_t Offset; // Offset from base address
  size_t Size;      // Allocation Size from Base + Offset
};

// MSB=63, LSB=0
#define EXTRACT_BITS(I64, HIGH, LOW)                                           \
  (((uint64_t)I64) >> (LOW)) & (((uint64_t)1 << ((HIGH) - (LOW) + 1)) - 1)
#endif // INTEL_COLLAB

/// This struct is a record of an entry point or global. For a function
/// entry point the size is expected to be zero
struct __tgt_offload_entry {
  void *addr;   // Pointer to the offload entry info (function or global)
  char *name;   // Name of the function or global
  size_t size;  // Size of the entry info (0 if it is a function)
  int32_t flags; // Flags associated with the entry, e.g. 'link'.
  int32_t reserved; // Reserved, to be used by the runtime library.
};

/// This struct is a record of the device image information
struct __tgt_device_image {
  void *ImageStart;                  // Pointer to the target code start
  void *ImageEnd;                    // Pointer to the target code end
  __tgt_offload_entry *EntriesBegin; // Begin of table with all target entries
  __tgt_offload_entry *EntriesEnd;   // End of table (non inclusive)
};

/// This struct is a record of all the host code that may be offloaded to a
/// target.
struct __tgt_bin_desc {
  int32_t NumDeviceImages;           // Number of device types supported
  __tgt_device_image *DeviceImages;  // Array of device images (1 per dev. type)
  __tgt_offload_entry *HostEntriesBegin; // Begin of table with all host entries
  __tgt_offload_entry *HostEntriesEnd;   // End of table (non inclusive)
};

/// This struct contains the offload entries identified by the target runtime
struct __tgt_target_table {
  __tgt_offload_entry *EntriesBegin; // Begin of the table with all the entries
  __tgt_offload_entry
      *EntriesEnd; // End of the table with all the entries (non inclusive)
};

#if INTEL_COLLAB
typedef struct __omp_offloading_fptr_map_t {
  uint64_t host_ptr; // key
  uint64_t tgt_ptr;  // value
} __omp_offloading_fptr_map_t;

#ifdef __cplusplus

#if _WIN32
#define EXTERN extern "C" __declspec(dllexport)
#else   // !_WIN32
#define EXTERN extern "C"
#endif  // !_WIN32

#else   // !__cplusplus

#if _WIN32
#define EXTERN extern __declspec(dllexport)
#else   // !_WIN32
#define EXTERN extern
#endif  // !_WIN32

#endif  // !__cplusplus
#endif  // INTEL_COLLAB
// clang-format on

/// This struct contains information exchanged between different asynchronous
/// operations for device-dependent optimization and potential synchronization
struct __tgt_async_info {
  // A pointer to a queue-like structure where offloading operations are issued.
  // We assume to use this structure to do synchronization. In CUDA backend, it
  // is CUstream.
  void *Queue = nullptr;
};

struct DeviceTy;

/// The libomptarget wrapper around a __tgt_async_info object directly
/// associated with a libomptarget layer device. RAII semantics to avoid
/// mistakes.
class AsyncInfoTy {
  /// Locations we used in (potentially) asynchronous calls which should live
  /// as long as this AsyncInfoTy object.
  std::deque<void *> BufferLocations;

  __tgt_async_info AsyncInfo;
  DeviceTy &Device;

public:
  AsyncInfoTy(DeviceTy &Device) : Device(Device) {}
  ~AsyncInfoTy() { synchronize(); }

  /// Implicit conversion to the __tgt_async_info which is used in the
  /// plugin interface.
  operator __tgt_async_info *() { return &AsyncInfo; }

  /// Synchronize all pending actions.
  ///
  /// \returns OFFLOAD_FAIL or OFFLOAD_SUCCESS appropriately.
  int synchronize();

  /// Return a void* reference with a lifetime that is at least as long as this
  /// AsyncInfoTy object. The location can be used as intermediate buffer.
  void *&getVoidPtrLocation();
};

/// This struct is a record of non-contiguous information
struct __tgt_target_non_contig {
  uint64_t Offset;
  uint64_t Count;
  uint64_t Stride;
};

struct __tgt_device_info {
  void *Context = nullptr;
  void *Device = nullptr;
};

#ifdef __cplusplus
extern "C" {
#endif

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int omp_get_num_devices(void);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int omp_get_initial_device(void);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void *omp_target_alloc(size_t size, int device_num);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void omp_target_free(void *device_ptr, int device_num);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int omp_target_is_present(const void *ptr, int device_num);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int omp_target_memcpy(void *dst, const void *src, size_t length,
                      size_t dst_offset, size_t src_offset, int dst_device,
                      int src_device);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int omp_target_memcpy_rect(void *dst, const void *src, size_t element_size,
                           int num_dims, const size_t *volume,
                           const size_t *dst_offsets, const size_t *src_offsets,
                           const size_t *dst_dimensions,
                           const size_t *src_dimensions, int dst_device,
                           int src_device);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int omp_target_associate_ptr(const void *host_ptr, const void *device_ptr,
                             size_t size, size_t device_offset, int device_num);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int omp_target_disassociate_ptr(const void *host_ptr, int device_num);

#if INTEL_COLLAB
EXTERN
void * omp_get_mapped_ptr(void *host_ptr, int device_num);

EXTERN
int omp_target_is_accessible(const void *ptr, size_t size, int device_num);

/// Explicit target memory allocators
/// Are we OK with omp_ prefix?
EXTERN void *omp_target_alloc_device(size_t size, int device_num);
EXTERN void *omp_target_alloc_host(size_t size, int device_num);
EXTERN void *omp_target_alloc_shared(size_t size, int device_num);

/// Get target device context
EXTERN void *omp_target_get_context(int device_num);

/// Set sub-device mode to map OpenMP device ID to sub-device ID at the
/// specified level. Returns number of sub-devices if the requested mode is
/// supported and the operation is successful, 0 otherwise.
/// Calling this routine not from "sequential part" of the OpenMP program
/// results in undefined behavior.
EXTERN int omp_set_sub_device(int device_num, int level);

/// Unset sub-device mode.
EXTERN void omp_unset_sub_device(int device_num);

/// Target memory realloc extension
EXTERN void *ompx_target_realloc(void *ptr, size_t size, int device_num);
EXTERN void *ompx_target_realloc_device(void *ptr, size_t size, int device_num);
EXTERN void *ompx_target_realloc_host(void *ptr, size_t size, int device_num);
EXTERN void *ompx_target_realloc_shared(void *ptr, size_t size, int device_num);

/// Target memory aligned alloc extension
EXTERN void *ompx_target_aligned_alloc(
    size_t align, size_t size, int device_num);
EXTERN void *ompx_target_aligned_alloc_device(
    size_t align, size_t size, int device_num);
EXTERN void *ompx_target_aligned_alloc_host(
    size_t align, size_t size, int device_num);
EXTERN void *ompx_target_aligned_alloc_shared(
    size_t align, size_t size, int device_num);

/// Get number of subdevices supported by the given device ID at the specified
/// level
EXTERN int ompx_get_num_subdevices(int device_num, int level);

EXTERN void ompx_kernel_batch_begin(int device_num, uint32_t max_kernels);
EXTERN void ompx_kernel_batch_end(int device_num);
#endif // INTEL_COLLAB

/// Explicit target memory allocators
/// Using the llvm_ prefix until they become part of the OpenMP standard.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void *llvm_omp_target_alloc_device(size_t size, int device_num);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void *llvm_omp_target_alloc_host(size_t size, int device_num);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void *llvm_omp_target_alloc_shared(size_t size, int device_num);

/// Dummy target so we have a symbol for generating host fallback.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void *llvm_omp_target_dynamic_shared_alloc();

/// add the clauses of the requires directives in a given file
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_register_requires(int64_t flags);

/// adds a target shared library to the target execution image
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_register_lib(__tgt_bin_desc *desc);

/// Initialize all RTLs at once
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_init_all_rtls();

/// removes a target shared library from the target execution image
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_unregister_lib(__tgt_bin_desc *desc);

// creates the host to target data mapping, stores it in the
// libomptarget.so internal structure (an entry in a stack of data maps) and
// passes the data to the device;
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_begin(int64_t device_id, int32_t arg_num,
                             void **args_base, void **args, int64_t *arg_sizes,
                             int64_t *arg_types);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_begin_nowait(int64_t device_id, int32_t arg_num,
                                    void **args_base, void **args,
                                    int64_t *arg_sizes, int64_t *arg_types,
                                    int32_t depNum, void *depList,
                                    int32_t noAliasDepNum,
                                    void *noAliasDepList);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_begin_mapper(ident_t *loc, int64_t device_id,
                                    int32_t arg_num, void **args_base,
                                    void **args, int64_t *arg_sizes,
                                    int64_t *arg_types,
                                    map_var_info_t *arg_names,
                                    void **arg_mappers);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_begin_nowait_mapper(
    ident_t *loc, int64_t device_id, int32_t arg_num, void **args_base,
    void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList);

// passes data from the target, release target memory and destroys the
// host-target mapping (top entry from the stack of data maps) created by
// the last __tgt_target_data_begin
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_end(int64_t device_id, int32_t arg_num, void **args_base,
                           void **args, int64_t *arg_sizes, int64_t *arg_types);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_end_nowait(int64_t device_id, int32_t arg_num,
                                  void **args_base, void **args,
                                  int64_t *arg_sizes, int64_t *arg_types,
                                  int32_t depNum, void *depList,
                                  int32_t noAliasDepNum, void *noAliasDepList);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_end_mapper(ident_t *loc, int64_t device_id,
                                  int32_t arg_num, void **args_base,
                                  void **args, int64_t *arg_sizes,
                                  int64_t *arg_types, map_var_info_t *arg_names,
                                  void **arg_mappers);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_end_nowait_mapper(
    ident_t *loc, int64_t device_id, int32_t arg_num, void **args_base,
    void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList);

/// passes data to/from the target
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_update(int64_t device_id, int32_t arg_num,
                              void **args_base, void **args, int64_t *arg_sizes,
                              int64_t *arg_types);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_update_nowait(int64_t device_id, int32_t arg_num,
                                     void **args_base, void **args,
                                     int64_t *arg_sizes, int64_t *arg_types,
                                     int32_t depNum, void *depList,
                                     int32_t noAliasDepNum,
                                     void *noAliasDepList);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_update_mapper(ident_t *loc, int64_t device_id,
                                     int32_t arg_num, void **args_base,
                                     void **args, int64_t *arg_sizes,
                                     int64_t *arg_types,
                                     map_var_info_t *arg_names,
                                     void **arg_mappers);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_target_data_update_nowait_mapper(
    ident_t *loc, int64_t device_id, int32_t arg_num, void **args_base,
    void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t depNum,
    void *depList, int32_t noAliasDepNum, void *noAliasDepList);

// Performs the same actions as data_begin in case arg_num is non-zero
// and initiates run of offloaded region on target platform; if arg_num
// is non-zero after the region execution is done it also performs the
// same action as data_end above. The following types are used; this
// function returns 0 if it was able to transfer the execution to a
// target and an int different from zero otherwise.
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target(int64_t device_id, void *host_ptr, int32_t arg_num,
                 void **args_base, void **args, int64_t *arg_sizes,
                 int64_t *arg_types);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target_nowait(int64_t device_id, void *host_ptr, int32_t arg_num,
                        void **args_base, void **args, int64_t *arg_sizes,
                        int64_t *arg_types, int32_t depNum, void *depList,
                        int32_t noAliasDepNum, void *noAliasDepList);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target_mapper(ident_t *loc, int64_t device_id, void *host_ptr,
                        int32_t arg_num, void **args_base, void **args,
                        int64_t *arg_sizes, int64_t *arg_types,
                        map_var_info_t *arg_names, void **arg_mappers);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target_nowait_mapper(ident_t *loc, int64_t device_id, void *host_ptr,
                               int32_t arg_num, void **args_base, void **args,
                               int64_t *arg_sizes, int64_t *arg_types,
                               map_var_info_t *arg_names, void **arg_mappers,
                               int32_t depNum, void *depList,
                               int32_t noAliasDepNum, void *noAliasDepList);

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target_teams(int64_t device_id, void *host_ptr, int32_t arg_num,
                       void **args_base, void **args, int64_t *arg_sizes,
                       int64_t *arg_types, int32_t num_teams,
                       int32_t thread_limit);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target_teams_nowait(int64_t device_id, void *host_ptr,
                              int32_t arg_num, void **args_base, void **args,
                              int64_t *arg_sizes, int64_t *arg_types,
                              int32_t num_teams, int32_t thread_limit,
                              int32_t depNum, void *depList,
                              int32_t noAliasDepNum, void *noAliasDepList);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target_teams_mapper(ident_t *loc, int64_t device_id, void *host_ptr,
                              int32_t arg_num, void **args_base, void **args,
                              int64_t *arg_sizes, int64_t *arg_types,
                              map_var_info_t *arg_names, void **arg_mappers,
                              int32_t num_teams, int32_t thread_limit);
#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_target_teams_nowait_mapper(
    ident_t *loc, int64_t device_id, void *host_ptr, int32_t arg_num,
    void **args_base, void **args, int64_t *arg_sizes, int64_t *arg_types,
    map_var_info_t *arg_names, void **arg_mappers, int32_t num_teams,
    int32_t thread_limit, int32_t depNum, void *depList, int32_t noAliasDepNum,
    void *noAliasDepList);

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __kmpc_push_target_tripcount(int64_t device_id, uint64_t loop_tripcount);

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __kmpc_push_target_tripcount_mapper(ident_t *loc, int64_t device_id,
                                         uint64_t loop_tripcount);

#if INTEL_COLLAB
EXTERN
int32_t __tgt_is_device_available(int64_t device_num, void *device_type);

// Returns implementation defined device name for the given device number,
// using provided Buffer. Buffer must be able to hold at least BufferMaxSize
// characters. Returns nullptr, if device name cannot be acquired, otherwise,
// returns a '\0' terminated C string (pointer to Buffer).
EXTERN char *__tgt_get_device_name(
    int64_t device_num, char *buffer, size_t buffer_max_size);

// Returns implementation defined RTL name corresponding to the given
// device number, using provided Buffer. Buffer must be able to hold
// at least BufferMaxSize characters.
// Returns nullptr, if RTL name cannot be acquired, otherwise,
// returns a '\0' terminated C string (pointer to Buffer).
EXTERN char *__tgt_get_device_rtl_name(
    int64_t device_num, char *buffer, size_t buffer_max_size);

// Callback function for asynchronous offloading
EXTERN void __tgt_offload_proxy_task_complete_ooo(void *);

// Creates an interop object.
EXTERN void * __tgt_create_interop_obj(
    int64_t device_id, bool is_async, void *async_obj);

// Releases an interop object.
EXTERN int __tgt_release_interop_obj(void *interop_obj);

// Create an OpenMP 5.1 interop object.
EXTERN omp_interop_t __tgt_create_interop(
    int64_t device_id, int32_t interop_type, int32_t num_prefers,
    intptr_t *prefer_ids);

// Release an OpenMP 5.1 interop object.
EXTERN int __tgt_release_interop(omp_interop_t interop);

// Change OpenMP 5.1 interop object to usable state ("use" clause)
EXTERN int __tgt_use_interop(omp_interop_t interop);

// Returns an interop property from the given interop object.
EXTERN int __tgt_get_interop_property(
    void *interop_obj, int32_t property_id, void **property_value);

// Update the interop object's property with given property_value.
EXTERN int __tgt_set_interop_property(
    void *interop_obj, int32_t property_id, void *property_value);

#if INTEL_CUSTOMIZATION
// Set code location information
EXTERN void __tgt_push_code_location(const char *location, void *codeptr_ra);
#endif // INTEL_CUSTOMIZATION

// Return number of devices
EXTERN int __tgt_get_num_devices(void);

// Return target memory information
EXTERN int __tgt_get_target_memory_info(
    void *interop_obj, int32_t num_ptrs, void *ptrs, void *ptr_info);

// Pass target code build options to plugins.
// This should be called after __tgt_register_lib().
// TODO: remove this if we choose to modify device image description.
EXTERN void __tgt_add_build_options(
    const char *compile_options, const char *link_options);

// Check if reduction scratch is supported
EXTERN int __tgt_target_supports_per_hw_thread_scratch(int64_t device_id);

// Allocate per-hw-thread reducion scratch
EXTERN void *__tgt_target_alloc_per_hw_thread_scratch(
    int64_t device_id, size_t obj_size, int32_t alloc_kind);

// Free per-hw-thread reduction scratch
EXTERN void __tgt_target_free_per_hw_thread_scratch(
    int64_t device_id, void *ptr);
#endif // INTEL_COLLAB

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
void __tgt_set_info_flag(uint32_t);

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int __tgt_print_device_info(int64_t device_id);
#ifdef __cplusplus
}
#endif


#if INTEL_COLLAB
#else  // INTEL_COLLAB
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif
#endif  // INTEL_COLLAB

#endif // _OMPTARGET_H_
