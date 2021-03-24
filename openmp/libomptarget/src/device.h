//===----------- device.h - Target independent OpenMP target RTL ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declarations for managing devices that are handled by RTL plugins.
//
//===----------------------------------------------------------------------===//

#ifndef _OMPTARGET_DEVICE_H
#define _OMPTARGET_DEVICE_H

#include <cassert>
#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "omptarget.h"
#include "rtl.h"

// Forward declarations.
struct RTLInfoTy;
struct __tgt_bin_desc;
struct __tgt_target_table;

using map_var_info_t = void *;

// enum for OMP_TARGET_OFFLOAD; keep in sync with kmp.h definition
enum kmp_target_offload_kind {
  tgt_disabled = 0,
  tgt_default = 1,
  tgt_mandatory = 2
};
typedef enum kmp_target_offload_kind kmp_target_offload_kind_t;

/// Map between host data and target data.
struct HostDataToTargetTy {
  uintptr_t HstPtrBase; // host info.
  uintptr_t HstPtrBegin;
  uintptr_t HstPtrEnd;       // non-inclusive.
  map_var_info_t HstPtrName; // Optional source name of mapped variable.

  uintptr_t TgtPtrBegin; // target info.

private:
  /// use mutable to allow modification via std::set iterator which is const.
  mutable uint64_t RefCount;
  static const uint64_t INFRefCount = ~(uint64_t)0;

public:
  HostDataToTargetTy(uintptr_t BP, uintptr_t B, uintptr_t E, uintptr_t TB,
                     map_var_info_t Name = nullptr, bool IsINF = false)
      : HstPtrBase(BP), HstPtrBegin(B), HstPtrEnd(E), HstPtrName(Name),
        TgtPtrBegin(TB), RefCount(IsINF ? INFRefCount : 1) {}

  uint64_t getRefCount() const { return RefCount; }

  uint64_t resetRefCount() const {
    if (RefCount != INFRefCount)
      RefCount = 1;

    return RefCount;
  }

  uint64_t incRefCount() const {
    if (RefCount != INFRefCount) {
      ++RefCount;
      assert(RefCount < INFRefCount && "refcount overflow");
    }

    return RefCount;
  }

  uint64_t decRefCount() const {
    if (RefCount != INFRefCount) {
      assert(RefCount > 0 && "refcount underflow");
      --RefCount;
    }

    return RefCount;
  }

  bool isRefCountInf() const { return RefCount == INFRefCount; }
};

typedef uintptr_t HstPtrBeginTy;
inline bool operator<(const HostDataToTargetTy &lhs, const HstPtrBeginTy &rhs) {
  return lhs.HstPtrBegin < rhs;
}
inline bool operator<(const HstPtrBeginTy &lhs, const HostDataToTargetTy &rhs) {
  return lhs < rhs.HstPtrBegin;
}
inline bool operator<(const HostDataToTargetTy &lhs,
                      const HostDataToTargetTy &rhs) {
  return lhs.HstPtrBegin < rhs.HstPtrBegin;
}

typedef std::set<HostDataToTargetTy, std::less<>> HostDataToTargetListTy;

struct LookupResult {
  struct {
    unsigned IsContained : 1;
    unsigned ExtendsBefore : 1;
    unsigned ExtendsAfter : 1;
  } Flags;

  HostDataToTargetListTy::iterator Entry;

  LookupResult() : Flags({0, 0, 0}), Entry() {}
};

/// Map for shadow pointers
struct ShadowPtrValTy {
  void *HstPtrVal;
  void *TgtPtrAddr;
  void *TgtPtrVal;
};
typedef std::map<void *, ShadowPtrValTy> ShadowPtrListTy;

///
struct PendingCtorDtorListsTy {
  std::list<void *> PendingCtors;
  std::list<void *> PendingDtors;
};
typedef std::map<__tgt_bin_desc *, PendingCtorDtorListsTy>
    PendingCtorsDtorsPerLibrary;
#if INTEL_COLLAB
typedef std::vector<std::set<void *>> UsedPtrsTy;
#endif // INTEL_COLLAB

struct DeviceTy {
  int32_t DeviceID;
  RTLInfoTy *RTL;
  int32_t RTLDeviceID;

  bool IsInit;
  std::once_flag InitFlag;
  bool HasPendingGlobals;

  HostDataToTargetListTy HostDataToTargetMap;
  PendingCtorsDtorsPerLibrary PendingCtorsDtors;

  ShadowPtrListTy ShadowPtrMap;

  std::mutex DataMapMtx, PendingGlobalsMtx, ShadowMtx;
#if INTEL_COLLAB
  std::map<int32_t, UsedPtrsTy> UsedPtrs;
  std::mutex UsedPtrsMtx;
#endif // INTEL_COLLAB

  // NOTE: Once libomp gains full target-task support, this state should be
  // moved into the target task in libomp.
  std::map<int32_t, uint64_t> LoopTripCnt;

  DeviceTy(RTLInfoTy *RTL);

  // The existence of mutexes makes DeviceTy non-copyable. We need to
  // provide a copy constructor and an assignment operator explicitly.
  DeviceTy(const DeviceTy &D);

  DeviceTy &operator=(const DeviceTy &D);

  ~DeviceTy();

  // Return true if data can be copied to DstDevice directly
  bool isDataExchangable(const DeviceTy &DstDevice);

  uint64_t getMapEntryRefCnt(void *HstPtrBegin);
  LookupResult lookupMapping(void *HstPtrBegin, int64_t Size);
  void *getOrAllocTgtPtr(void *HstPtrBegin, void *HstPtrBase, int64_t Size,
                         map_var_info_t HstPtrName, bool &IsNew,
                         bool &IsHostPtr, bool IsImplicit, bool UpdateRefCount,
                         bool HasCloseModifier, bool HasPresentModifier);
  void *getTgtPtrBegin(void *HstPtrBegin, int64_t Size);
  void *getTgtPtrBegin(void *HstPtrBegin, int64_t Size, bool &IsLast,
                       bool UpdateRefCount, bool &IsHostPtr,
                       bool MustContain = false);
  int deallocTgtPtr(void *TgtPtrBegin, int64_t Size, bool ForceDelete,
                    bool HasCloseModifier = false);
  int associatePtr(void *HstPtrBegin, void *TgtPtrBegin, int64_t Size);
  int disassociatePtr(void *HstPtrBegin);

  // calls to RTL
  int32_t initOnce();
  __tgt_target_table *load_binary(void *Img);

  // device memory allocation/deallocation routines
  /// Allocates \p Size bytes on the device, host or shared memory space
  /// (depending on \p Kind) and returns the address/nullptr when
  /// succeeds/fails. \p HstPtr is an address of the host data which the
  /// allocated target data will be associated with. If it is unknown, the
  /// default value of \p HstPtr is nullptr. Note: this function doesn't do
  /// pointer association. Actually, all the __tgt_rtl_data_alloc
  /// implementations ignore \p HstPtr. \p Kind dictates what allocator should
  /// be used (host, shared, device).
  void *allocData(int64_t Size, void *HstPtr = nullptr,
                  int32_t Kind = TARGET_ALLOC_DEFAULT);
  /// Deallocates memory which \p TgtPtrBegin points at and returns
  /// OFFLOAD_SUCCESS/OFFLOAD_FAIL when succeeds/fails.
  int32_t deleteData(void *TgtPtrBegin);

  // Data transfer. When AsyncInfo is nullptr, the transfer will be
  // synchronous.
  // Copy data from host to device
  int32_t submitData(void *TgtPtrBegin, void *HstPtrBegin, int64_t Size,
                     AsyncInfoTy &AsyncInfo);
  // Copy data from device back to host
  int32_t retrieveData(void *HstPtrBegin, void *TgtPtrBegin, int64_t Size,
                       AsyncInfoTy &AsyncInfo);
  // Copy data from current device to destination device directly
  int32_t dataExchange(void *SrcPtr, DeviceTy &DstDev, void *DstPtr,
                       int64_t Size, AsyncInfoTy &AsyncInfo);

  int32_t runRegion(void *TgtEntryPtr, void **TgtVarsPtr, ptrdiff_t *TgtOffsets,
                    int32_t TgtVarsSize, AsyncInfoTy &AsyncInfo);
  int32_t runTeamRegion(void *TgtEntryPtr, void **TgtVarsPtr,
                        ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                        int32_t NumTeams, int32_t ThreadLimit,
                        uint64_t LoopTripCount, AsyncInfoTy &AsyncInfo);

#if INTEL_COLLAB
  int32_t manifest_data_for_region(void *TgtEntryPtr);
  char *get_device_name(char *Buffer, size_t BufferMaxSize);
  void *data_alloc_base(int64_t Size, void *HstPtrBegin, void *HstPtrBase);
  void *data_alloc_user(int64_t Size, void *HstPtrBegin);
  int32_t data_submit_nowait(void *TgtPtrBegin, void *HstPtrBegin,
                             int64_t Size, void *AsyncData);
  int32_t data_retrieve_nowait(void *HstPtrBegin, void *TgtPtrBegin,
                               int64_t Size, void *AsyncData);
  int32_t run_team_nd_region(void *TgtEntryPtr, void **TgtVarsPtr,
                             ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                             int32_t NumTeams, int32_t ThreadLimit,
                             void *TgtNDLoopDesc);
  int32_t run_team_nd_region_nowait(void *TgtEntryPtr, void **TgtVarsPtr,
                                    ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                                    int32_t NumTeams, int32_t ThreadLimit,
                                    void *TgtNDLoopDesc, void *AsyncData);
  int32_t run_region_nowait(void *TgtEntryPtr, void **TgtVarsPtr,
                            ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                            void *AsyncData);
  int32_t run_team_region_nowait(void *TgtEntryPtr, void **TgtVarsPtr,
                                 ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                                 int32_t NumTeams, int32_t ThreadLimit,
                                 uint64_t LoopTripCount, void *AsyncData);
  void create_offload_queue(void *Interop);
  int32_t release_offload_queue(void *);
  void *get_platform_handle();
  void setDeviceHandle(void *Interop);
  void *get_context_handle();
  void *data_alloc_managed(int64_t Size);
  int32_t is_device_accessible_ptr(void *Ptr);
  int32_t managed_memory_supported();
  void *data_alloc_explicit(int64_t Size, int32_t Kind);
  int32_t get_data_alloc_info(int32_t NumPtrs, void *Ptrs, void *Infos);
  int32_t pushSubDevice(int64_t EncodedID, int64_t DeviceID);
  int32_t popSubDevice(void);
  int32_t isSupportedDevice(void *DeviceType);
  __tgt_interop *createInterop(int32_t InteropContext);
  int32_t releaseInterop(__tgt_interop *Interop);
  int32_t getNumInteropProperties(void);
  int32_t getInteropPropertyValue(__tgt_interop *Interop,
                                  omp_interop_property_t Property,
                                  int32_t ValueType, size_t Size, void *Value);
  const char *getInteropPropertyInfo(omp_interop_property_t Property,
                                     int32_t InfoType);
  const char *getInteropRcDesc(int32_t RetCode);
  int32_t setSubDevice(int32_t Level);
  void unsetSubDevice(void);
#endif // INTEL_COLLAB

  /// Synchronize device/queue/event based on \p AsyncInfo and return
  /// OFFLOAD_SUCCESS/OFFLOAD_FAIL when succeeds/fails.
  int32_t synchronize(AsyncInfoTy &AsyncInfo);

private:
  // Call to RTL
  void init(); // To be called only via DeviceTy::initOnce()
};

/// Map between Device ID (i.e. openmp device id) and its DeviceTy.
typedef std::vector<DeviceTy> DevicesTy;

extern bool device_is_ready(int device_num);

/// Struct for the data required to handle plugins
struct PluginManager {
  /// RTLs identified on the host
  RTLsTy RTLs;

  /// Devices associated with RTLs
  DevicesTy Devices;
  std::mutex RTLsMtx; ///< For RTLs and Devices

  /// Translation table retreived from the binary
  HostEntriesBeginToTransTableTy HostEntriesBeginToTransTable;
  std::mutex TrlTblMtx; ///< For Translation Table
  /// Host offload entries in order of image registration
  std::vector<__tgt_offload_entry *> HostEntriesBeginRegistrationOrder;

  /// Map from ptrs on the host to an entry in the Translation Table
  HostPtrToTableMapTy HostPtrToTableMap;
  std::mutex TblMapMtx; ///< For HostPtrToTableMap

  // Store target policy (disabled, mandatory, default)
  kmp_target_offload_kind_t TargetOffloadPolicy = tgt_default;
  std::mutex TargetOffloadMtx; ///< For TargetOffloadPolicy
#if INTEL_COLLAB
  /// Root device ID and sub-device mask set by user
  /// These should be global to all threads.
  int64_t RootDeviceID = -1;
  int64_t SubDeviceMask = 0;
#endif // INTEL_COLLAB
};

extern PluginManager *PM;

#endif
