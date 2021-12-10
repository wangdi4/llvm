//===--------- device.cpp - Target independent OpenMP target RTL ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Functionality for managing devices that are handled by RTL plugins.
//
//===----------------------------------------------------------------------===//

#if INTEL_COLLAB
#include "omptarget-tools.h"
#endif // INTEL_COLLAB
#include "device.h"
#include "private.h"
#include "rtl.h"

#include <cassert>
#include <climits>
#include <cstdio>
#include <string>

DeviceTy::DeviceTy(RTLInfoTy *RTL)
    : DeviceID(-1), RTL(RTL), RTLDeviceID(-1), IsInit(false), InitFlag(),
      HasPendingGlobals(false), HostDataToTargetMap(), PendingCtorsDtors(),
      ShadowPtrMap(), DataMapMtx(), PendingGlobalsMtx(), ShadowMtx() {}

DeviceTy::~DeviceTy() {
  if (DeviceID == -1 || !(getInfoLevel() & OMP_INFOTYPE_DUMP_TABLE))
    return;

  ident_t loc = {0, 0, 0, 0, ";libomptarget;libomptarget;0;0;;"};
  dumpTargetPointerMappings(&loc, *this);
}

int DeviceTy::associatePtr(void *HstPtrBegin, void *TgtPtrBegin, int64_t Size) {
  DataMapMtx.lock();

  // Check if entry exists
  auto search = HostDataToTargetMap.find(HstPtrBeginTy{(uintptr_t)HstPtrBegin});
  if (search != HostDataToTargetMap.end()) {
    // Mapping already exists
    bool isValid = search->HstPtrEnd == (uintptr_t)HstPtrBegin + Size &&
                   search->TgtPtrBegin == (uintptr_t)TgtPtrBegin;
    DataMapMtx.unlock();
    if (isValid) {
      DP("Attempt to re-associate the same device ptr+offset with the same "
         "host ptr, nothing to do\n");
      return OFFLOAD_SUCCESS;
    } else {
      REPORT("Not allowed to re-associate a different device ptr+offset with "
             "the same host ptr\n");
      return OFFLOAD_FAIL;
    }
  }

  // Mapping does not exist, allocate it with refCount=INF
  const HostDataToTargetTy &newEntry =
      *HostDataToTargetMap
           .emplace(
               /*HstPtrBase=*/(uintptr_t)HstPtrBegin,
               /*HstPtrBegin=*/(uintptr_t)HstPtrBegin,
               /*HstPtrEnd=*/(uintptr_t)HstPtrBegin + Size,
               /*TgtPtrBegin=*/(uintptr_t)TgtPtrBegin,
               /*UseHoldRefCount=*/false, /*Name=*/nullptr,
               /*IsRefCountINF=*/true)
           .first;
  DP("Creating new map entry: HstBase=" DPxMOD ", HstBegin=" DPxMOD
     ", HstEnd=" DPxMOD ", TgtBegin=" DPxMOD ", DynRefCount=%s, "
     "HoldRefCount=%s\n",
     DPxPTR(newEntry.HstPtrBase), DPxPTR(newEntry.HstPtrBegin),
     DPxPTR(newEntry.HstPtrEnd), DPxPTR(newEntry.TgtPtrBegin),
     newEntry.dynRefCountToStr().c_str(), newEntry.holdRefCountToStr().c_str());
  (void)newEntry;

  DataMapMtx.unlock();

  return OFFLOAD_SUCCESS;
}

int DeviceTy::disassociatePtr(void *HstPtrBegin) {
  DataMapMtx.lock();

  auto search = HostDataToTargetMap.find(HstPtrBeginTy{(uintptr_t)HstPtrBegin});
  if (search != HostDataToTargetMap.end()) {
    // Mapping exists
    if (search->getHoldRefCount()) {
      // This is based on OpenACC 3.1, sec 3.2.33 "acc_unmap_data", L3656-3657:
      // "It is an error to call acc_unmap_data if the structured reference
      // count for the pointer is not zero."
      REPORT("Trying to disassociate a pointer with a non-zero hold reference "
             "count\n");
    } else if (search->isDynRefCountInf()) {
      DP("Association found, removing it\n");
      HostDataToTargetMap.erase(search);
      DataMapMtx.unlock();
      return OFFLOAD_SUCCESS;
    } else {
      REPORT("Trying to disassociate a pointer which was not mapped via "
             "omp_target_associate_ptr\n");
    }
  } else {
    REPORT("Association not found\n");
  }

  // Mapping not found
  DataMapMtx.unlock();
  return OFFLOAD_FAIL;
}

LookupResult DeviceTy::lookupMapping(void *HstPtrBegin, int64_t Size) {
  uintptr_t hp = (uintptr_t)HstPtrBegin;
  LookupResult lr;

  DP("Looking up mapping(HstPtrBegin=" DPxMOD ", Size=%" PRId64 ")...\n",
     DPxPTR(hp), Size);

  if (HostDataToTargetMap.empty())
    return lr;

  auto upper = HostDataToTargetMap.upper_bound(hp);
  // check the left bin
  if (upper != HostDataToTargetMap.begin()) {
    lr.Entry = std::prev(upper);
    auto &HT = *lr.Entry;
    // Is it contained?
    lr.Flags.IsContained = hp >= HT.HstPtrBegin && hp < HT.HstPtrEnd &&
                           (hp + Size) <= HT.HstPtrEnd;
    // Does it extend beyond the mapped region?
    lr.Flags.ExtendsAfter = hp < HT.HstPtrEnd && (hp + Size) > HT.HstPtrEnd;
  }

  // check the right bin
  if (!(lr.Flags.IsContained || lr.Flags.ExtendsAfter) &&
      upper != HostDataToTargetMap.end()) {
    lr.Entry = upper;
    auto &HT = *lr.Entry;
    // Does it extend into an already mapped region?
    lr.Flags.ExtendsBefore =
        hp < HT.HstPtrBegin && (hp + Size) > HT.HstPtrBegin;
    // Does it extend beyond the mapped region?
    lr.Flags.ExtendsAfter = hp < HT.HstPtrEnd && (hp + Size) > HT.HstPtrEnd;
  }

  if (lr.Flags.ExtendsBefore) {
    DP("WARNING: Pointer is not mapped but section extends into already "
       "mapped data\n");
  }
  if (lr.Flags.ExtendsAfter) {
    DP("WARNING: Pointer is already mapped but section extends beyond mapped "
       "region\n");
  }

  return lr;
}

TargetPointerResultTy
DeviceTy::getTargetPointer(void *HstPtrBegin, void *HstPtrBase, int64_t Size,
                           map_var_info_t HstPtrName, bool HasFlagTo,
                           bool HasFlagAlways, bool IsImplicit,
                           bool UpdateRefCount, bool HasCloseModifier,
                           bool HasPresentModifier, bool HasHoldModifier,
                           AsyncInfoTy &AsyncInfo) {
  void *TargetPointer = nullptr;
  bool IsHostPtr = false;
  bool IsNew = false;

  DataMapMtx.lock();

  LookupResult LR = lookupMapping(HstPtrBegin, Size);
  auto Entry = LR.Entry;

  // Check if the pointer is contained.
  // If a variable is mapped to the device manually by the user - which would
  // lead to the IsContained flag to be true - then we must ensure that the
  // device address is returned even under unified memory conditions.
  if (LR.Flags.IsContained ||
      ((LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter) && IsImplicit)) {
    auto &HT = *LR.Entry;
    const char *RefCountAction;
    assert(HT.getTotalRefCount() > 0 && "expected existing RefCount > 0");
    if (UpdateRefCount) {
      // After this, RefCount > 1.
      HT.incRefCount(HasHoldModifier);
      RefCountAction = " (incremented)";
    } else {
      // It might have been allocated with the parent, but it's still new.
      IsNew = HT.getTotalRefCount() == 1;
      RefCountAction = " (update suppressed)";
    }
    const char *DynRefCountAction = HasHoldModifier ? "" : RefCountAction;
    const char *HoldRefCountAction = HasHoldModifier ? RefCountAction : "";
    uintptr_t Ptr = HT.TgtPtrBegin + ((uintptr_t)HstPtrBegin - HT.HstPtrBegin);
    INFO(OMP_INFOTYPE_MAPPING_EXISTS, DeviceID,
         "Mapping exists%s with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD
         ", Size=%" PRId64 ", DynRefCount=%s%s, HoldRefCount=%s%s, Name=%s\n",
         (IsImplicit ? " (implicit)" : ""), DPxPTR(HstPtrBegin), DPxPTR(Ptr),
         Size, HT.dynRefCountToStr().c_str(), DynRefCountAction,
         HT.holdRefCountToStr().c_str(), HoldRefCountAction,
         (HstPtrName) ? getNameFromMapping(HstPtrName).c_str() : "unknown");
    TargetPointer = (void *)Ptr;
  } else if ((LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter) && !IsImplicit) {
    // Explicit extension of mapped data - not allowed.
    MESSAGE("explicit extension not allowed: host address specified is " DPxMOD
            " (%" PRId64
            " bytes), but device allocation maps to host at " DPxMOD
            " (%" PRId64 " bytes)",
            DPxPTR(HstPtrBegin), Size, DPxPTR(Entry->HstPtrBegin),
            Entry->HstPtrEnd - Entry->HstPtrBegin);
    if (HasPresentModifier)
      MESSAGE("device mapping required by 'present' map type modifier does not "
              "exist for host address " DPxMOD " (%" PRId64 " bytes)",
              DPxPTR(HstPtrBegin), Size);
#if INTEL_COLLAB
  } else if (((PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY &&
               !managed_memory_supported()) ||
              is_device_accessible_ptr(HstPtrBegin)) &&
#else // INTEL_COLLAB
  } else if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY &&
#endif // INTEL_COLLAB
             !HasCloseModifier) {
    // If unified shared memory is active, implicitly mapped variables that are
    // not privatized use host address. Any explicitly mapped variables also use
    // host address where correctness is not impeded. In all other cases maps
    // are respected.
    // In addition to the mapping rules above, the close map modifier forces the
    // mapping of the variable to the device.
#if !INTEL_COLLAB
    if (Size) {
#endif // !INTEL_COLLAB
      DP("Return HstPtrBegin " DPxMOD " Size=%" PRId64 " for unified shared "
         "memory\n",
         DPxPTR((uintptr_t)HstPtrBegin), Size);
#if INTEL_COLLAB
      if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY)
#endif // INTEL_COLLAB
        IsHostPtr = true;
      TargetPointer = HstPtrBegin;
#if !INTEL_COLLAB
    }
#endif // !INTEL_COLLAB
  } else if (HasPresentModifier) {
#if INTEL_COLLAB
    DP("Mapping required by 'present' map type modifier does not exist for "
       "HstPtrBegin=" DPxMOD ", Size=%" PRId64 "\n",
       DPxPTR(HstPtrBegin), Size);
#else // INTEL_COLLAB
    DP("Mapping required by 'present' map type modifier does not exist for "
       "HstPtrBegin=" DPxMOD ", Size=%" PRId64 "\n",
       DPxPTR(HstPtrBegin), Size);
#endif // INTEL_COLLAB
#if INTEL_COLLAB
    MESSAGE("device mapping required by 'present' map type modifier does not "
            "exist for host address " DPxMOD " (%" PRId64 " bytes)",
            DPxPTR(HstPtrBegin), Size);
#else // INTEL_COLLAB
    MESSAGE("device mapping required by 'present' map type modifier does not "
            "exist for host address " DPxMOD " (%" PRId64 " bytes)",
            DPxPTR(HstPtrBegin), Size);
#endif // INTEL_COLLAB
  } else if (Size) {
    // If it is not contained and Size > 0, we should create a new entry for it.
    IsNew = true;
#if INTEL_COLLAB
    uintptr_t Ptr = (uintptr_t)data_alloc_base(Size, HstPtrBegin, HstPtrBase);
    Entry = HostDataToTargetMap
                .emplace((uintptr_t)HstPtrBase, (uintptr_t)HstPtrBegin,
                         (uintptr_t)HstPtrBegin + Size, Ptr, HasHoldModifier,
                         HstPtrName)
                .first;
    INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
         "Creating new map entry with "
         "HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD ", Size=%" PRId64 ", "
         "DynRefCount=%s, HoldRefCount=%s, Name=%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(Ptr), Size,
         Entry->dynRefCountToStr().c_str(), Entry->holdRefCountToStr().c_str(),
         (HstPtrName) ? getNameFromMapping(HstPtrName).c_str() : "unknown");
#else // INTEL_COLLAB
    uintptr_t Ptr = (uintptr_t)allocData(Size, HstPtrBegin);
    Entry = HostDataToTargetMap
                .emplace((uintptr_t)HstPtrBase, (uintptr_t)HstPtrBegin,
                         (uintptr_t)HstPtrBegin + Size, Ptr, HasHoldModifier,
                         HstPtrName)
                .first;
    INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
         "Creating new map entry with "
         "HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD ", Size=%ld, "
         "DynRefCount=%s, HoldRefCount=%s, Name=%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(Ptr), Size,
         Entry->dynRefCountToStr().c_str(), Entry->holdRefCountToStr().c_str(),
         (HstPtrName) ? getNameFromMapping(HstPtrName).c_str() : "unknown");
#endif // INTEL_COLLAB
    TargetPointer = (void *)Ptr;
  }

  // If the target pointer is valid, and we need to transfer data, issue the
  // data transfer.
  if (TargetPointer && !IsHostPtr && HasFlagTo && (IsNew || HasFlagAlways)) {
    // Lock the entry before releasing the mapping table lock such that another
    // thread that could issue data movement will get the right result.
    Entry->lock();
    // Release the mapping table lock right after the entry is locked.
    DataMapMtx.unlock();

    DP("Moving %" PRId64 " bytes (hst:" DPxMOD ") -> (tgt:" DPxMOD ")\n", Size,
       DPxPTR(HstPtrBegin), DPxPTR(TargetPointer));

    int Ret = submitData(TargetPointer, HstPtrBegin, Size, AsyncInfo);

    // Unlock the entry immediately after the data movement is issued.
    Entry->unlock();

    if (Ret != OFFLOAD_SUCCESS) {
      REPORT("Copying data to device failed.\n");
      // We will also return nullptr if the data movement fails because that
      // pointer points to a corrupted memory region so it doesn't make any
      // sense to continue to use it.
      TargetPointer = nullptr;
    }
  } else {
    // Release the mapping table lock directly.
    DataMapMtx.unlock();
  }

  return {{IsNew, IsHostPtr}, Entry, TargetPointer};
}

// Used by targetDataBegin, targetDataEnd, targetDataUpdate and target.
// Return the target pointer begin (where the data will be moved).
// Decrement the reference counter if called from targetDataEnd. The data is
// excepted to be mapped already, so the result will never be new.
TargetPointerResultTy
DeviceTy::getTgtPtrBegin(void *HstPtrBegin, int64_t Size, bool &IsLast,
                         bool UpdateRefCount, bool UseHoldRefCount,
                         bool &IsHostPtr, bool MustContain, bool ForceDelete) {
  void *TargetPointer = NULL;
  IsHostPtr = false;
  IsLast = false;
  DataMapMtx.lock();
  LookupResult lr = lookupMapping(HstPtrBegin, Size);

  if (lr.Flags.IsContained ||
      (!MustContain && (lr.Flags.ExtendsBefore || lr.Flags.ExtendsAfter))) {
    auto &HT = *lr.Entry;
    // We do not zero the total reference count here.  deallocTgtPtr does that
    // atomically with removing the mapping.  Otherwise, before this thread
    // removed the mapping in deallocTgtPtr, another thread could retrieve the
    // mapping, increment and decrement back to zero, and then both threads
    // would try to remove the mapping, resulting in a double free.
    IsLast = HT.decShouldRemove(UseHoldRefCount, ForceDelete);
    const char *RefCountAction;
    if (!UpdateRefCount) {
      RefCountAction = " (update suppressed)";
    } else if (ForceDelete) {
      HT.resetRefCount(UseHoldRefCount);
      assert(IsLast == HT.decShouldRemove(UseHoldRefCount) &&
             "expected correct IsLast prediction for reset");
      if (IsLast)
        RefCountAction = " (reset, deferred final decrement)";
      else {
        HT.decRefCount(UseHoldRefCount);
        RefCountAction = " (reset)";
      }
    } else if (IsLast) {
      RefCountAction = " (deferred final decrement)";
    } else {
      HT.decRefCount(UseHoldRefCount);
      RefCountAction = " (decremented)";
    }
    const char *DynRefCountAction = UseHoldRefCount ? "" : RefCountAction;
    const char *HoldRefCountAction = UseHoldRefCount ? RefCountAction : "";
    uintptr_t tp = HT.TgtPtrBegin + ((uintptr_t)HstPtrBegin - HT.HstPtrBegin);
    INFO(OMP_INFOTYPE_MAPPING_EXISTS, DeviceID,
         "Mapping exists with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD ", "
         "Size=%" PRId64 ", DynRefCount=%s%s, HoldRefCount=%s%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(tp), Size, HT.dynRefCountToStr().c_str(),
         DynRefCountAction, HT.holdRefCountToStr().c_str(), HoldRefCountAction);
<<<<<<< HEAD
    rc = (void *)tp;
#if INTEL_COLLAB
  } else if ((PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY &&
                  !managed_memory_supported()) ||
             is_device_accessible_ptr(HstPtrBegin)) {
#else  // INTEL_COLLAB
=======
    TargetPointer = (void *)tp;
>>>>>>> 7c8f4e7b85ed98497f37571d72609f39a8eed447
  } else if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) {
#endif // INTEL_COLLAB
    // If the value isn't found in the mapping and unified shared memory
    // is on then it means we have stumbled upon a value which we need to
    // use directly from the host.
    DP("Get HstPtrBegin " DPxMOD " Size=%" PRId64 " for unified shared "
       "memory\n",
       DPxPTR((uintptr_t)HstPtrBegin), Size);
<<<<<<< HEAD
#if INTEL_COLLAB
    if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY)
#endif // INTEL_COLLAB
       IsHostPtr = true;
    rc = HstPtrBegin;
=======
    IsHostPtr = true;
    TargetPointer = HstPtrBegin;
>>>>>>> 7c8f4e7b85ed98497f37571d72609f39a8eed447
  }

  DataMapMtx.unlock();
  return {{false, IsHostPtr}, lr.Entry, TargetPointer};
}

// Return the target pointer begin (where the data will be moved).
// Lock-free version called when loading global symbols from the fat binary.
void *DeviceTy::getTgtPtrBegin(void *HstPtrBegin, int64_t Size) {
  uintptr_t hp = (uintptr_t)HstPtrBegin;
  LookupResult lr = lookupMapping(HstPtrBegin, Size);
  if (lr.Flags.IsContained || lr.Flags.ExtendsBefore || lr.Flags.ExtendsAfter) {
    auto &HT = *lr.Entry;
    uintptr_t tp = HT.TgtPtrBegin + (hp - HT.HstPtrBegin);
    return (void *)tp;
  }

  return NULL;
}

int DeviceTy::deallocTgtPtr(void *HstPtrBegin, int64_t Size,
                            bool HasHoldModifier) {
  // Check if the pointer is contained in any sub-nodes.
  int rc;
  DataMapMtx.lock();
  LookupResult lr = lookupMapping(HstPtrBegin, Size);
  if (lr.Flags.IsContained || lr.Flags.ExtendsBefore || lr.Flags.ExtendsAfter) {
    auto &HT = *lr.Entry;
    if (HT.decRefCount(HasHoldModifier) == 0) {
      DP("Deleting tgt data " DPxMOD " of size %" PRId64 "\n",
         DPxPTR(HT.TgtPtrBegin), Size);
#if INTEL_COLLAB
      OMPT_TRACE(targetDataDeleteBegin(RTLDeviceID, (void *)HT.TgtPtrBegin));
#endif // INTEL_COLLAB
      deleteData((void *)HT.TgtPtrBegin);
#if INTEL_COLLAB
      OMPT_TRACE(targetDataDeleteEnd(RTLDeviceID, (void *)HT.TgtPtrBegin));
#endif // INTEL_COLLAB
      INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
           "Removing map entry with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD
           ", Size=%" PRId64 ", Name=%s\n",
           DPxPTR(HT.HstPtrBegin), DPxPTR(HT.TgtPtrBegin), Size,
           (HT.HstPtrName) ? getNameFromMapping(HT.HstPtrName).c_str()
                           : "unknown");
      HostDataToTargetMap.erase(lr.Entry);
    }
    rc = OFFLOAD_SUCCESS;
  } else {
    REPORT("Section to delete (hst addr " DPxMOD ") does not exist in the"
           " allocated memory\n",
           DPxPTR(HstPtrBegin));
    rc = OFFLOAD_FAIL;
  }

  DataMapMtx.unlock();
  return rc;
}

/// Init device, should not be called directly.
void DeviceTy::init() {
  // Make call to init_requires if it exists for this plugin.
  if (RTL->init_requires)
    RTL->init_requires(PM->RTLs.RequiresFlags);
  int32_t Ret = RTL->init_device(RTLDeviceID);
  if (Ret != OFFLOAD_SUCCESS)
    return;

  IsInit = true;
}

/// Thread-safe method to initialize the device only once.
int32_t DeviceTy::initOnce() {
  std::call_once(InitFlag, &DeviceTy::init, this);

  // At this point, if IsInit is true, then either this thread or some other
  // thread in the past successfully initialized the device, so we can return
  // OFFLOAD_SUCCESS. If this thread executed init() via call_once() and it
  // failed, return OFFLOAD_FAIL. If call_once did not invoke init(), it means
  // that some other thread already attempted to execute init() and if IsInit
  // is still false, return OFFLOAD_FAIL.
  if (IsInit)
    return OFFLOAD_SUCCESS;
  else
    return OFFLOAD_FAIL;
}

// Load binary to device.
__tgt_target_table *DeviceTy::load_binary(void *Img) {
  RTL->Mtx.lock();
  __tgt_target_table *rc = RTL->load_binary(RTLDeviceID, Img);
  RTL->Mtx.unlock();
  return rc;
}

void *DeviceTy::allocData(int64_t Size, void *HstPtr, int32_t Kind) {
#if INTEL_COLLAB
  OMPT_TRACE(targetDataAllocBegin(RTLDeviceID, Size));
  void *Ret = RTL->data_alloc(RTLDeviceID, Size, HstPtr, Kind);
  OMPT_TRACE(targetDataAllocEnd(RTLDeviceID, Size, Ret));
  return Ret;
#else // INTEL_COLLAB
  return RTL->data_alloc(RTLDeviceID, Size, HstPtr, Kind);
#endif // INTEL_COLLAB
}

int32_t DeviceTy::deleteData(void *TgtPtrBegin) {
  return RTL->data_delete(RTLDeviceID, TgtPtrBegin);
}

// Submit data to device
int32_t DeviceTy::submitData(void *TgtPtrBegin, void *HstPtrBegin, int64_t Size,
                             AsyncInfoTy &AsyncInfo) {
  if (getInfoLevel() & OMP_INFOTYPE_DATA_TRANSFER) {
    LookupResult LR = lookupMapping(HstPtrBegin, Size);
    auto *HT = &*LR.Entry;

    INFO(OMP_INFOTYPE_DATA_TRANSFER, DeviceID,
         "Copying data from host to device, HstPtr=" DPxMOD ", TgtPtr=" DPxMOD
         ", Size=%" PRId64 ", Name=%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(TgtPtrBegin), Size,
         (HT && HT->HstPtrName) ? getNameFromMapping(HT->HstPtrName).c_str()
                                : "unknown");
  }

#if INTEL_COLLAB
  OMPT_TRACE(
      targetDataSubmitBegin(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size));
  int32_t ret;
  if (!AsyncInfo || !RTL->data_submit_async || !RTL->synchronize)
    ret = RTL->data_submit(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size);
  else
    ret = RTL->data_submit_async(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size,
                                 AsyncInfo);
  OMPT_TRACE(targetDataSubmitEnd(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size));
  return ret;
#else // INTEL_COLLAB
  if (!AsyncInfo || !RTL->data_submit_async || !RTL->synchronize)
    return RTL->data_submit(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size);
  else
    return RTL->data_submit_async(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size,
                                  AsyncInfo);
#endif // INTEL_COLLAB
}

// Retrieve data from device
int32_t DeviceTy::retrieveData(void *HstPtrBegin, void *TgtPtrBegin,
                               int64_t Size, AsyncInfoTy &AsyncInfo) {
  if (getInfoLevel() & OMP_INFOTYPE_DATA_TRANSFER) {
    LookupResult LR = lookupMapping(HstPtrBegin, Size);
    auto *HT = &*LR.Entry;
    INFO(OMP_INFOTYPE_DATA_TRANSFER, DeviceID,
         "Copying data from device to host, TgtPtr=" DPxMOD ", HstPtr=" DPxMOD
         ", Size=%" PRId64 ", Name=%s\n",
         DPxPTR(TgtPtrBegin), DPxPTR(HstPtrBegin), Size,
         (HT && HT->HstPtrName) ? getNameFromMapping(HT->HstPtrName).c_str()
                                : "unknown");
  }

#if INTEL_COLLAB
  OMPT_TRACE(
      targetDataRetrieveBegin(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size));
  int32_t ret;
  if (!RTL->data_retrieve_async || !RTL->synchronize)
    ret = RTL->data_retrieve(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size);
  else
    ret = RTL->data_retrieve_async(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size,
                                   AsyncInfo);
  OMPT_TRACE(
      targetDataRetrieveEnd(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size));
  return ret;
#else // INTEL_COLLAB
  if (!RTL->data_retrieve_async || !RTL->synchronize)
    return RTL->data_retrieve(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size);
  else
    return RTL->data_retrieve_async(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size,
                                    AsyncInfo);
#endif // INTEL_COLLAB
}

// Copy data from current device to destination device directly
int32_t DeviceTy::dataExchange(void *SrcPtr, DeviceTy &DstDev, void *DstPtr,
                               int64_t Size, AsyncInfoTy &AsyncInfo) {
  if (!AsyncInfo || !RTL->data_exchange_async || !RTL->synchronize) {
    assert(RTL->data_exchange && "RTL->data_exchange is nullptr");
    return RTL->data_exchange(RTLDeviceID, SrcPtr, DstDev.RTLDeviceID, DstPtr,
                              Size);
  } else
    return RTL->data_exchange_async(RTLDeviceID, SrcPtr, DstDev.RTLDeviceID,
                                    DstPtr, Size, AsyncInfo);
}

// Run region on device
int32_t DeviceTy::runRegion(void *TgtEntryPtr, void **TgtVarsPtr,
                            ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                            AsyncInfoTy &AsyncInfo) {
#if INTEL_COLLAB
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, 1));
  int32_t ret;
  if (!RTL->run_region || !RTL->synchronize)
    ret = RTL->run_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                          TgtVarsSize);
  else
    ret = RTL->run_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                TgtOffsets, TgtVarsSize, AsyncInfo);
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, 1));
  return ret;
#else // INTEL_COLLAB
  if (!RTL->run_region || !RTL->synchronize)
    return RTL->run_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                           TgtVarsSize);
  else
    return RTL->run_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                 TgtOffsets, TgtVarsSize, AsyncInfo);
#endif // INTEL_COLLAB
}

// Run region on device
bool DeviceTy::printDeviceInfo(int32_t RTLDevId) {
  if (!RTL->print_device_info)
    return false;
  RTL->print_device_info(RTLDevId);
  return true;
}

// Run team region on device.
int32_t DeviceTy::runTeamRegion(void *TgtEntryPtr, void **TgtVarsPtr,
                                ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                                int32_t NumTeams, int32_t ThreadLimit,
                                uint64_t LoopTripCount,
                                AsyncInfoTy &AsyncInfo) {
#if INTEL_COLLAB
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, NumTeams));
  int32_t ret;
  if (!RTL->run_team_region_async || !RTL->synchronize)
    ret = RTL->run_team_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                TgtOffsets, TgtVarsSize, NumTeams, ThreadLimit,
                                LoopTripCount);
  else
    ret = RTL->run_team_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                      TgtOffsets, TgtVarsSize, NumTeams,
                                      ThreadLimit, LoopTripCount, AsyncInfo);
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, NumTeams));
  return ret;
#else // INTEL_COLLAB
  if (!RTL->run_team_region_async || !RTL->synchronize)
    return RTL->run_team_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                TgtOffsets, TgtVarsSize, NumTeams, ThreadLimit,
                                LoopTripCount);
  else
    return RTL->run_team_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                      TgtOffsets, TgtVarsSize, NumTeams,
                                      ThreadLimit, LoopTripCount, AsyncInfo);
#endif // INTEL_COLLAB
}

#if INTEL_COLLAB
int32_t DeviceTy::manifest_data_for_region(void *TgtEntryPtr) {
  if (!RTL->manifest_data_for_region)
    return OFFLOAD_SUCCESS;

  // Targets that require explicit manifestation for pointers
  // that are not passed as arguments to the target entry
  // provide an optional manifest_data_for_region interface.
  //
  // Pointers that may be dereferenced inside the target entry
  // and are not necessarily passed as arguments are the following:
  //   1. Pointers to global variables.
  //   2. Shadow pointers mapped as PTR_AND_OBJ.
  std::vector<void *> ObjectPtrs;

  DataMapMtx.lock();

  for (auto &HT : HostDataToTargetMap) {
    if (!HT.isDynRefCountInf())
      continue;

    void *TgtPtrBegin = reinterpret_cast<void *>(HT.TgtPtrBegin);

    if (ObjectPtrs.empty())
      DP("Manifesting target pointers for globals:\n");

    DP("\tHstPtrBase=" DPxMOD ", HstPtrBegin=" DPxMOD
       ", HstPtrEnd=" DPxMOD ", TgtPtrBegin=" DPxMOD "\n",
       DPxPTR(HT.HstPtrBase), DPxPTR(HT.HstPtrBegin),
       DPxPTR(HT.HstPtrEnd), DPxPTR(TgtPtrBegin));

    ObjectPtrs.push_back(TgtPtrBegin);
  }

  DataMapMtx.unlock();

  ShadowMtx.lock();
  if (!ShadowPtrMap.empty()) {
    DP("Manifesting shadow target pointers:\n");
    for (auto &SPE : ShadowPtrMap) {
      DP("\tHstPtrAddr=" DPxMOD ", HstPtrVal=" DPxMOD
         ", TgtPtrAddr=" DPxMOD ", TgtPtrVal=" DPxMOD "\n",
         DPxPTR(SPE.first), DPxPTR(SPE.second.HstPtrVal),
         DPxPTR(SPE.second.TgtPtrAddr), DPxPTR(SPE.second.TgtPtrVal));

      ObjectPtrs.push_back(SPE.second.TgtPtrVal);
    }
  }
  ShadowMtx.unlock();

  UsedPtrsMtx.lock();
  DP("Manifesting used target pointers:\n");
  for (auto &PtrSetList : UsedPtrs) {
    for (auto &PtrSet : PtrSetList.second) {
      for (auto Ptr : PtrSet) {
        DP("\tUsedTargetPtr=" DPxMOD "\n", DPxPTR(Ptr));
        ObjectPtrs.push_back(Ptr);
      }
    }
  }
  UsedPtrsMtx.unlock();

  int32_t GTID = __kmpc_global_thread_num(nullptr);
  LambdaPtrsMtx.lock();
  if (LambdaPtrs.count(GTID) > 0 && LambdaPtrs.at(GTID).size() > 0) {
    DP("Manifesting lambda mapped target pointers:\n");
    for (auto Ptr : LambdaPtrs.at(GTID)) {
      DP("\tTgtPtr=" DPxMOD "\n", DPxPTR(Ptr));
      ObjectPtrs.push_back(Ptr);
    }
    LambdaPtrs.at(GTID).clear();
  }
  LambdaPtrsMtx.unlock();

  if (ObjectPtrs.empty())
    return OFFLOAD_SUCCESS;

  int32_t RC =
      RTL->manifest_data_for_region(RTLDeviceID, TgtEntryPtr,
                                    ObjectPtrs.data(), ObjectPtrs.size());

  return RC;
}

char *DeviceTy::get_device_name(char *Buffer, size_t BufferMaxSize) {
  assert(Buffer && "Buffer cannot be nullptr.");
  assert(BufferMaxSize > 0 && "BufferMaxSize cannot be zero.");
  if (RTL->get_device_name)
    return RTL->get_device_name(RTLDeviceID, Buffer, BufferMaxSize);
  // Make Buffer an empty string, if RTL does not support
  // name query.
  Buffer[0] = '\0';
  return Buffer;
}

void *DeviceTy::data_alloc_base(int64_t Size, void *HstPtrBegin,
                                void *HstPtrBase) {
  OMPT_TRACE(targetDataAllocBegin(RTLDeviceID, Size));
  void *ret =
      RTL->data_alloc_base
          ? RTL->data_alloc_base(RTLDeviceID, Size, HstPtrBegin, HstPtrBase)
          : RTL->data_alloc(RTLDeviceID, Size, HstPtrBegin,
                            TARGET_ALLOC_DEFAULT);
  OMPT_TRACE(targetDataAllocEnd(RTLDeviceID, Size, ret));
  return ret;
}

int32_t DeviceTy::data_submit_nowait(void *TgtPtrBegin, void *HstPtrBegin,
                                     int64_t Size, void *AsyncData) {
  OMPT_TRACE(
      targetDataSubmitBegin(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size));
  int32_t ret = RTL->data_submit_nowait
      ? RTL->data_submit_nowait(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size,
                                AsyncData)
      : OFFLOAD_FAIL;
  OMPT_TRACE(targetDataSubmitEnd(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size));
  return ret;
}

int32_t DeviceTy::data_retrieve_nowait(void *HstPtrBegin, void *TgtPtrBegin,
                                       int64_t Size, void *AsyncData) {
  OMPT_TRACE(
      targetDataRetrieveBegin(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size));
  int32_t ret = RTL->data_retrieve_nowait
      ? RTL->data_retrieve_nowait(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size,
                                  AsyncData)
      : OFFLOAD_FAIL;
  OMPT_TRACE(
      targetDataRetrieveEnd(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size));
  return ret;
}

int32_t DeviceTy::run_team_nd_region(void *TgtEntryPtr, void **TgtVarsPtr,
                                     ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                                     int32_t NumTeams, int32_t ThreadLimit,
                                     void *TgtNDLoopDesc) {
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, NumTeams));
  int32_t ret = RTL->run_team_nd_region
      ? RTL->run_team_nd_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                TgtOffsets, TgtVarsSize, NumTeams, ThreadLimit,
                                TgtNDLoopDesc)
      : OFFLOAD_FAIL;
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, NumTeams));
  return ret;
}

int32_t
DeviceTy::run_team_nd_region_nowait(void *TgtEntryPtr, void **TgtVarsPtr,
                                    ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                                    int32_t NumTeams, int32_t ThreadLimit,
                                    void *TgtNDLoopDesc, void *AsyncData) {
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, NumTeams));
  int32_t ret = RTL->run_team_nd_region_nowait
      ? RTL->run_team_nd_region_nowait(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                       TgtOffsets, TgtVarsSize, NumTeams,
                                       ThreadLimit, TgtNDLoopDesc, AsyncData)
      : OFFLOAD_FAIL;
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, NumTeams));
  return ret;
}

int32_t DeviceTy::run_region_nowait(void *TgtEntryPtr, void **TgtVarsPtr,
                                    ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                                    void *AsyncData) {
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, 1));
  int32_t ret = RTL->run_region_nowait
      ? RTL->run_region_nowait(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                               TgtVarsSize, AsyncData)
      : OFFLOAD_FAIL;
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, 1));
  return ret;
}

int32_t DeviceTy::run_team_region_nowait(void *TgtEntryPtr, void **TgtVarsPtr,
                                         ptrdiff_t *TgtOffsets,
                                         int32_t TgtVarsSize, int32_t NumTeams,
                                         int32_t ThreadLimit,
                                         uint64_t LoopTripCount,
                                         void *AsyncData) {
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, NumTeams));
  int32_t ret = RTL->run_team_region_nowait
      ? RTL->run_team_region_nowait(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                    TgtOffsets, TgtVarsSize, NumTeams,
                                    ThreadLimit, LoopTripCount, AsyncData)
      : OFFLOAD_FAIL;
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, NumTeams));
  return ret;
}

void DeviceTy::get_offload_queue(void *Interop, bool CreateNew) {
  if (RTL->get_offload_queue)
    RTL->get_offload_queue(RTLDeviceID, Interop, CreateNew);
}

int32_t DeviceTy::release_offload_queue(void *Queue) {
  if (RTL->release_offload_queue)
    return RTL->release_offload_queue(RTLDeviceID, Queue);
  else
    return OFFLOAD_SUCCESS;
}

void *DeviceTy::get_platform_handle() {
  if (!RTL->get_platform_handle)
    return nullptr;
  return RTL->get_platform_handle(RTLDeviceID);
}

void DeviceTy::setDeviceHandle(void *Interop) {
  if (RTL->set_device_handle)
    RTL->set_device_handle(RTLDeviceID, Interop);
}

void *DeviceTy::get_context_handle() {
  if (!RTL->get_context_handle)
    return nullptr;
  return RTL->get_context_handle(RTLDeviceID);
}

void *DeviceTy::data_alloc_managed(int64_t Size) {
  if (RTL->data_alloc_managed)
    return RTL->data_alloc_managed(RTLDeviceID, Size);
  else
    return RTL->data_alloc(RTLDeviceID, Size, nullptr, TARGET_ALLOC_DEFAULT);
}

int32_t DeviceTy::is_device_accessible_ptr(void *Ptr) {
  if (RTL->is_device_accessible_ptr)
    return RTL->is_device_accessible_ptr(RTLDeviceID, Ptr);
  else
    return 0;
}

int32_t DeviceTy::managed_memory_supported() {
  return RTL->data_alloc_managed != nullptr;
}

void *DeviceTy::dataRealloc(void *Ptr, size_t Size, int32_t Kind) {
  if (RTL->data_realloc)
    return RTL->data_realloc(RTLDeviceID, Ptr, Size, Kind);
  else
    return allocData(Size, nullptr, Kind);
}

void *DeviceTy::dataAlignedAlloc(size_t Align, size_t Size, int32_t Kind) {
  if (RTL->data_aligned_alloc)
    return RTL->data_aligned_alloc(RTLDeviceID, Align, Size, Kind);
  else
    return allocData(Size, nullptr, Kind);
}

int32_t DeviceTy::get_data_alloc_info(
    int32_t NumPtrs, void *TgtPtrs, void *Infos) {
  if (RTL->get_data_alloc_info)
    return RTL->get_data_alloc_info(RTLDeviceID, NumPtrs, TgtPtrs, Infos);
  else
    return OFFLOAD_FAIL;
}

int32_t DeviceTy::pushSubDevice(int64_t EncodedID, int64_t DeviceID) {
  if (RTL->push_subdevice == nullptr)
    return OFFLOAD_SUCCESS;

  if (EncodedID != DeviceID)
    return RTL->push_subdevice(EncodedID);
  else
    return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::popSubDevice(void) {
  if (RTL->pop_subdevice)
    return RTL->pop_subdevice();
  else
    return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::getNumSubDevices(int32_t Level) {
  if (RTL->get_num_sub_devices)
    return RTL->get_num_sub_devices(RTLDeviceID, Level);
  else
    return 0;
}

int32_t DeviceTy::isSupportedDevice(void *DeviceType) {
  if (RTL->is_supported_device)
    return RTL->is_supported_device(RTLDeviceID, DeviceType);
  else
    return false;
}

__tgt_interop *DeviceTy::createInterop(int32_t InteropContext,
                                       int32_t NumPrefers,
                                       intptr_t *PreferIDs) {
  if (RTL->create_interop)
    return RTL->create_interop(RTLDeviceID, InteropContext, NumPrefers,
                               PreferIDs);
  else
    return NULL;
}

int32_t DeviceTy::releaseInterop(__tgt_interop *Interop) {
  if (RTL->release_interop)
    return RTL->release_interop(RTLDeviceID, Interop);
  else
    return OFFLOAD_FAIL;
}

int32_t DeviceTy::useInterop(__tgt_interop *Interop) {
  if (RTL->use_interop)
    return RTL->use_interop(RTLDeviceID, Interop);
  else
    return OFFLOAD_FAIL;
}

int32_t DeviceTy::getNumInteropProperties(void) {
  if (RTL->get_num_interop_properties)
    return RTL->get_num_interop_properties(RTLDeviceID);
  else
    return 0;
}

int32_t DeviceTy::getInteropPropertyValue(__tgt_interop *Interop,
                                          int32_t Property, int32_t ValueType,
                                          size_t Size, void *Value) {
  if (RTL->get_interop_property_value)
    return RTL->get_interop_property_value(RTLDeviceID, Interop, Property,
                                           ValueType, Size, Value);
  else
    return OFFLOAD_FAIL;
}

const char *DeviceTy::getInteropPropertyInfo(int32_t Property,
                                             int32_t InfoType) {
  if (RTL->get_interop_property_info)
    return RTL->get_interop_property_info(RTLDeviceID, Property, InfoType);
  else
    return NULL;
}

const char *DeviceTy::getInteropRcDesc(int32_t RetCode) {
  if (RTL->get_interop_rc_desc)
    return RTL->get_interop_rc_desc(RTLDeviceID, RetCode);
  else
    return NULL;
}

int32_t DeviceTy::setSubDevice(int32_t Level) {
  if (RTL->get_num_sub_devices) {
    if (PM->RootDeviceID >= 0 || PM->SubDeviceMask != 0) {
      DP("WARNING: unexpected sub-device region detected -- "
         "sub-device environment is not configured.\n");
      return 0;
    }
    int32_t NumSubDevices = RTL->get_num_sub_devices(RTLDeviceID, Level);
    if (NumSubDevices > 0) {
      int64_t Mask = 1ULL << 63; // Sub-device is on
      Mask |= static_cast<int64_t>(Level) << 56; // Level
      // "start" bits will be written by the ID passed to __tgt* entries.
      Mask |= 1ULL << 40; // Count
      Mask |= 1ULL << 32; // Stride
      PM->RootDeviceID = RTLDeviceID;
      PM->SubDeviceMask = Mask;
    }
    return NumSubDevices;
  } else {
    return 0;
  }
}

void DeviceTy::unsetSubDevice(void) {
  PM->RootDeviceID = -1;
  PM->SubDeviceMask = 0;
}

void DeviceTy::addLambdaPtr(void *TgtPtr) {
  int32_t GTID = __kmpc_global_thread_num(nullptr);
  std::lock_guard<std::mutex> Lock(LambdaPtrsMtx);
  if (LambdaPtrs.count(GTID) == 0)
    LambdaPtrs.emplace(GTID, std::vector<void *>{});
  LambdaPtrs.at(GTID).push_back(TgtPtr);
}

int32_t DeviceTy::isAccessibleAddrRange(const void *Ptr, size_t Size) {
  if (RTL->is_accessible_addr_range)
    return RTL->is_accessible_addr_range(RTLDeviceID, Ptr, Size);
  else
    return 0;
}

int32_t DeviceTy::notifyIndirectAccess(const void *Ptr, size_t Offset) {
  if (RTL->notify_indirect_access)
    return RTL->notify_indirect_access(RTLDeviceID, Ptr, Offset);
  else
    return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::isPrivateArgOnHost(const void *TgtEntryPtr, uint32_t Idx) {
  if (RTL->is_private_arg_on_host)
    return RTL->is_private_arg_on_host(RTLDeviceID, TgtEntryPtr, Idx);
  else
    return 0;
}

int32_t DeviceTy::commandBatchBegin(int32_t BatchLevel) {
  if (RTL->command_batch_begin)
    return RTL->command_batch_begin(RTLDeviceID, BatchLevel);
  else
    return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::commandBatchEnd(int32_t BatchLevel) {
  if (RTL->command_batch_end)
    return RTL->command_batch_end(RTLDeviceID, BatchLevel);
  else
    return OFFLOAD_SUCCESS;
}

void DeviceTy::kernelBatchBegin(uint32_t MaxKernels) {
  if (RTL->kernel_batch_begin)
    RTL->kernel_batch_begin(RTLDeviceID, MaxKernels);
}

void DeviceTy::kernelBatchEnd(void) {
  if (RTL->kernel_batch_end)
    RTL->kernel_batch_end(RTLDeviceID);
}
#endif // INTEL_COLLAB

// Whether data can be copied to DstDevice directly
bool DeviceTy::isDataExchangable(const DeviceTy &DstDevice) {
  if (RTL != DstDevice.RTL || !RTL->is_data_exchangable)
    return false;

  if (RTL->is_data_exchangable(RTLDeviceID, DstDevice.RTLDeviceID))
    return (RTL->data_exchange != nullptr) ||
           (RTL->data_exchange_async != nullptr);

  return false;
}

int32_t DeviceTy::synchronize(AsyncInfoTy &AsyncInfo) {
  if (RTL->synchronize)
    return RTL->synchronize(RTLDeviceID, AsyncInfo);
  return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::createEvent(void **Event) {
  if (RTL->create_event)
    return RTL->create_event(RTLDeviceID, Event);

  return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::recordEvent(void *Event, AsyncInfoTy &AsyncInfo) {
  if (RTL->record_event)
    return RTL->record_event(RTLDeviceID, Event, AsyncInfo);

  return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::waitEvent(void *Event, AsyncInfoTy &AsyncInfo) {
  if (RTL->wait_event)
    return RTL->wait_event(RTLDeviceID, Event, AsyncInfo);

  return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::syncEvent(void *Event) {
  if (RTL->sync_event)
    return RTL->sync_event(RTLDeviceID, Event);

  return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::destroyEvent(void *Event) {
  if (RTL->create_event)
    return RTL->destroy_event(RTLDeviceID, Event);

  return OFFLOAD_SUCCESS;
}

/// Check whether a device has an associated RTL and initialize it if it's not
/// already initialized.
bool device_is_ready(int device_num) {
  DP("Checking whether device %d is ready.\n", device_num);
  // Devices.size() can only change while registering a new
  // library, so try to acquire the lock of RTLs' mutex.
  PM->RTLsMtx.lock();
  size_t DevicesSize = PM->Devices.size();
  PM->RTLsMtx.unlock();
  if (DevicesSize <= (size_t)device_num) {
    DP("Device ID  %d does not have a matching RTL\n", device_num);
    return false;
  }

  // Get device info
  DeviceTy &Device = *PM->Devices[device_num];

  DP("Is the device %d (local ID %d) initialized? %d\n", device_num,
     Device.RTLDeviceID, Device.IsInit);

  // Init the device if not done before
  if (!Device.IsInit && Device.initOnce() != OFFLOAD_SUCCESS) {
    DP("Failed to init device %d\n", device_num);
    return false;
  }

  DP("Device %d is ready to use.\n", device_num);

  return true;
}
