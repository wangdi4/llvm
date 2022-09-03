//===--------- device.cpp - Target independent OpenMP target RTL ----------===//
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
// Functionality for managing devices that are handled by RTL plugins.
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
#include <climits>
#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>

#ifdef INTEL_CUSTOMIZATION
using llvm::SmallVector;
#endif // INTEL_CUSTOMIZATION

int HostDataToTargetTy::addEventIfNecessary(DeviceTy &Device,
                                            AsyncInfoTy &AsyncInfo) const {
  // First, check if the user disabled atomic map transfer/malloc/dealloc.
  if (!PM->UseEventsForAtomicTransfers)
    return OFFLOAD_SUCCESS;

  void *Event = getEvent();
  bool NeedNewEvent = Event == nullptr;
  if (NeedNewEvent && Device.createEvent(&Event) != OFFLOAD_SUCCESS) {
    REPORT("Failed to create event\n");
    return OFFLOAD_FAIL;
  }

  // We cannot assume the event should not be nullptr because we don't
  // know if the target support event. But if a target doesn't,
  // recordEvent should always return success.
  if (Device.recordEvent(Event, AsyncInfo) != OFFLOAD_SUCCESS) {
    REPORT("Failed to set dependence on event " DPxMOD "\n", DPxPTR(Event));
    return OFFLOAD_FAIL;
  }

  if (NeedNewEvent)
    setEvent(Event);

  return OFFLOAD_SUCCESS;
}

DeviceTy::DeviceTy(RTLInfoTy *RTL)
    : DeviceID(-1), RTL(RTL), RTLDeviceID(-1), IsInit(false), InitFlag(),
      HasPendingGlobals(false), PendingCtorsDtors(), ShadowPtrMap(),
      PendingGlobalsMtx(), ShadowMtx() {}

DeviceTy::~DeviceTy() {
  if (DeviceID == -1 || !(getInfoLevel() & OMP_INFOTYPE_DUMP_TABLE))
    return;

  ident_t Loc = {0, 0, 0, 0, ";libomptarget;libomptarget;0;0;;"};
  dumpTargetPointerMappings(&Loc, *this);
}

int DeviceTy::associatePtr(void *HstPtrBegin, void *TgtPtrBegin, int64_t Size) {
  HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();

  // Check if entry exists
  auto It = HDTTMap->find(HstPtrBegin);
  if (It != HDTTMap->end()) {
    HostDataToTargetTy &HDTT = *It->HDTT;
    // Mapping already exists
    bool IsValid = HDTT.HstPtrEnd == (uintptr_t)HstPtrBegin + Size &&
                   HDTT.TgtPtrBegin == (uintptr_t)TgtPtrBegin;
    if (IsValid) {
      DP("Attempt to re-associate the same device ptr+offset with the same "
         "host ptr, nothing to do\n");
      return OFFLOAD_SUCCESS;
    }
    REPORT("Not allowed to re-associate a different device ptr+offset with "
           "the same host ptr\n");
    return OFFLOAD_FAIL;
  }

  // Mapping does not exist, allocate it with refCount=INF
  const HostDataToTargetTy &NewEntry =
      *HDTTMap
           ->emplace(new HostDataToTargetTy(
               /*HstPtrBase=*/(uintptr_t)HstPtrBegin,
               /*HstPtrBegin=*/(uintptr_t)HstPtrBegin,
               /*HstPtrEnd=*/(uintptr_t)HstPtrBegin + Size,
               /*TgtPtrBegin=*/(uintptr_t)TgtPtrBegin,
               /*UseHoldRefCount=*/false, /*Name=*/nullptr,
               /*IsRefCountINF=*/true))
           .first->HDTT;
  DP("Creating new map entry: HstBase=" DPxMOD ", HstBegin=" DPxMOD
     ", HstEnd=" DPxMOD ", TgtBegin=" DPxMOD ", DynRefCount=%s, "
     "HoldRefCount=%s\n",
     DPxPTR(NewEntry.HstPtrBase), DPxPTR(NewEntry.HstPtrBegin),
     DPxPTR(NewEntry.HstPtrEnd), DPxPTR(NewEntry.TgtPtrBegin),
     NewEntry.dynRefCountToStr().c_str(), NewEntry.holdRefCountToStr().c_str());
  (void)NewEntry;

  return OFFLOAD_SUCCESS;
}

int DeviceTy::disassociatePtr(void *HstPtrBegin) {
  HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();

  auto It = HDTTMap->find(HstPtrBegin);
  if (It != HDTTMap->end()) {
    HostDataToTargetTy &HDTT = *It->HDTT;
    // Mapping exists
    if (HDTT.getHoldRefCount()) {
      // This is based on OpenACC 3.1, sec 3.2.33 "acc_unmap_data", L3656-3657:
      // "It is an error to call acc_unmap_data if the structured reference
      // count for the pointer is not zero."
      REPORT("Trying to disassociate a pointer with a non-zero hold reference "
             "count\n");
    } else if (HDTT.isDynRefCountInf()) {
      DP("Association found, removing it\n");
      void *Event = HDTT.getEvent();
      delete &HDTT;
      if (Event)
        destroyEvent(Event);
      HDTTMap->erase(It);
      return OFFLOAD_SUCCESS;
    } else {
      REPORT("Trying to disassociate a pointer which was not mapped via "
             "omp_target_associate_ptr\n");
    }
  } else {
    REPORT("Association not found\n");
  }

  // Mapping not found
  return OFFLOAD_FAIL;
}

LookupResult DeviceTy::lookupMapping(HDTTMapAccessorTy &HDTTMap,
                                     void *HstPtrBegin, int64_t Size) {

  uintptr_t HP = (uintptr_t)HstPtrBegin;
  LookupResult LR;

  DP("Looking up mapping(HstPtrBegin=" DPxMOD ", Size=%" PRId64 ")...\n",
     DPxPTR(HP), Size);

  if (HDTTMap->empty())
    return LR;

  auto Upper = HDTTMap->upper_bound(HP);

  if (Size == 0) {
    // specification v5.1 Pointer Initialization for Device Data Environments
    // upper_bound satisfies
    //   std::prev(upper)->HDTT.HstPtrBegin <= hp < upper->HDTT.HstPtrBegin
    if (Upper != HDTTMap->begin()) {
      LR.Entry = std::prev(Upper)->HDTT;
      auto &HT = *LR.Entry;
      // the left side of extended address range is satisified.
      // hp >= HT.HstPtrBegin || hp >= HT.HstPtrBase
      LR.Flags.IsContained = HP < HT.HstPtrEnd || HP < HT.HstPtrBase;
    }

    if (!LR.Flags.IsContained && Upper != HDTTMap->end()) {
      LR.Entry = Upper->HDTT;
      auto &HT = *LR.Entry;
      // the right side of extended address range is satisified.
      // hp < HT.HstPtrEnd || hp < HT.HstPtrBase
      LR.Flags.IsContained = HP >= HT.HstPtrBase;
    }
  } else {
    // check the left bin
    if (Upper != HDTTMap->begin()) {
      LR.Entry = std::prev(Upper)->HDTT;
      auto &HT = *LR.Entry;
      // Is it contained?
      LR.Flags.IsContained = HP >= HT.HstPtrBegin && HP < HT.HstPtrEnd &&
                             (HP + Size) <= HT.HstPtrEnd;
      // Does it extend beyond the mapped region?
      LR.Flags.ExtendsAfter = HP < HT.HstPtrEnd && (HP + Size) > HT.HstPtrEnd;
    }

    // check the right bin
    if (!(LR.Flags.IsContained || LR.Flags.ExtendsAfter) &&
        Upper != HDTTMap->end()) {
      LR.Entry = Upper->HDTT;
      auto &HT = *LR.Entry;
      // Does it extend into an already mapped region?
      LR.Flags.ExtendsBefore =
          HP < HT.HstPtrBegin && (HP + Size) > HT.HstPtrBegin;
      // Does it extend beyond the mapped region?
      LR.Flags.ExtendsAfter = HP < HT.HstPtrEnd && (HP + Size) > HT.HstPtrEnd;
    }

    if (LR.Flags.ExtendsBefore) {
      DP("WARNING: Pointer is not mapped but section extends into already "
         "mapped data\n");
    }
    if (LR.Flags.ExtendsAfter) {
      DP("WARNING: Pointer is already mapped but section extends beyond mapped "
         "region\n");
    }
  }

  return LR;
}

TargetPointerResultTy DeviceTy::getTargetPointer(
    void *HstPtrBegin, void *HstPtrBase, int64_t Size,
    map_var_info_t HstPtrName, bool HasFlagTo, bool HasFlagAlways,
    bool IsImplicit, bool UpdateRefCount, bool HasCloseModifier,
    bool HasPresentModifier, bool HasHoldModifier, AsyncInfoTy &AsyncInfo) {
  HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();

  void *TargetPointer = nullptr;
  bool IsHostPtr = false;
  bool IsNew = false;

  LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size);
  auto *Entry = LR.Entry;

  // Check if the pointer is contained.
  // If a variable is mapped to the device manually by the user - which would
  // lead to the IsContained flag to be true - then we must ensure that the
  // device address is returned even under unified memory conditions.
  if (LR.Flags.IsContained ||
      ((LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter) && IsImplicit)) {
    auto &HT = *LR.Entry;
    const char *RefCountAction;
    if (UpdateRefCount) {
      // After this, reference count >= 1. If the reference count was 0 but the
      // entry was still there we can reuse the data on the device and avoid a
      // new submission.
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
  } else if (!requiresMapping(HstPtrBegin, Size)) {
    // Let plugin decide if HstPtrBegin and Size requires mapping. Size check is
    // done to differentiated implicit/explicit mapping
    DP("Return HstPtrBegin " DPxMOD " Size=%" PRId64
       " for device-accessible memory\n", DPxPTR(HstPtrBegin), Size);
    if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY)
      IsHostPtr = true;
    TargetPointer = HstPtrBegin;
    // Lookup result becomes irrelevant in this case, and it should be ignored.
    Entry = nullptr;
  } else if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY &&
             !HasCloseModifier && !managedMemorySupported()) {
#else // INTEL_COLLAB
  } else if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY &&
             !HasCloseModifier) {
#endif // INTEL_COLLAB
    // If unified shared memory is active, implicitly mapped variables that are
    // not privatized use host address. Any explicitly mapped variables also use
    // host address where correctness is not impeded. In all other cases maps
    // are respected.
    // In addition to the mapping rules above, the close map modifier forces the
    // mapping of the variable to the device.
    if (Size) {
      DP("Return HstPtrBegin " DPxMOD " Size=%" PRId64 " for unified shared "
         "memory\n",
         DPxPTR((uintptr_t)HstPtrBegin), Size);
      IsHostPtr = true;
      TargetPointer = HstPtrBegin;
    }
  } else if (HasPresentModifier) {
    DP("Mapping required by 'present' map type modifier does not exist for "
       "HstPtrBegin=" DPxMOD ", Size=%" PRId64 "\n",
       DPxPTR(HstPtrBegin), Size);
    MESSAGE("device mapping required by 'present' map type modifier does not "
            "exist for host address " DPxMOD " (%" PRId64 " bytes)",
            DPxPTR(HstPtrBegin), Size);
  } else if (Size) {
    // If it is not contained and Size > 0, we should create a new entry for it.
    IsNew = true;
#if INTEL_COLLAB
    uintptr_t Ptr = (uintptr_t)dataAllocBase(Size, HstPtrBegin, HstPtrBase);
#else // INTEL_COLLAB
    uintptr_t Ptr = (uintptr_t)allocData(Size, HstPtrBegin);
#endif // INTEL_COLLAB
    Entry = HDTTMap
                ->emplace(new HostDataToTargetTy(
                    (uintptr_t)HstPtrBase, (uintptr_t)HstPtrBegin,
                    (uintptr_t)HstPtrBegin + Size, Ptr, HasHoldModifier,
                    HstPtrName))
                .first->HDTT;
#if INTEL_CUSTOMIZATION
    XPTIRegistry->traceMemAssociate((uintptr_t)HstPtrBegin, Ptr);
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
    INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
         "Creating new map entry with "
         "HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD ", Size=%" PRId64 ", "
         "DynRefCount=%s, HoldRefCount=%s, Name=%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(Ptr), Size,
         Entry->dynRefCountToStr().c_str(), Entry->holdRefCountToStr().c_str(),
         (HstPtrName) ? getNameFromMapping(HstPtrName).c_str() : "unknown");
#else // INTEL_COLLAB
    INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
         "Creating new map entry with HstPtrBase=" DPxMOD
         ", HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD ", Size=%ld, "
         "DynRefCount=%s, HoldRefCount=%s, Name=%s\n",
         DPxPTR(HstPtrBase), DPxPTR(HstPtrBegin), DPxPTR(Ptr), Size,
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
    std::lock_guard<decltype(*Entry)> LG(*Entry);
    // Release the mapping table lock right after the entry is locked.
    HDTTMap.destroy();

    DP("Moving %" PRId64 " bytes (hst:" DPxMOD ") -> (tgt:" DPxMOD ")\n", Size,
       DPxPTR(HstPtrBegin), DPxPTR(TargetPointer));

    int Ret = submitData(TargetPointer, HstPtrBegin, Size, AsyncInfo);
    if (Ret != OFFLOAD_SUCCESS) {
      REPORT("Copying data to device failed.\n");
      // We will also return nullptr if the data movement fails because that
      // pointer points to a corrupted memory region so it doesn't make any
      // sense to continue to use it.
      TargetPointer = nullptr;
    } else if (Entry->addEventIfNecessary(*this, AsyncInfo) != OFFLOAD_SUCCESS)
      return {{false /* IsNewEntry */, false /* IsHostPointer */},
              nullptr /* Entry */,
              nullptr /* TargetPointer */};
  } else {
    // Release the mapping table lock directly.
    HDTTMap.destroy();
    // If not a host pointer and no present modifier, we need to wait for the
    // event if it exists.
    // Note: Entry might be nullptr because of zero length array section.
    if (Entry && !IsHostPtr && !HasPresentModifier) {
      std::lock_guard<decltype(*Entry)> LG(*Entry);
      void *Event = Entry->getEvent();
      if (Event) {
        int Ret = waitEvent(Event, AsyncInfo);
        if (Ret != OFFLOAD_SUCCESS) {
          // If it fails to wait for the event, we need to return nullptr in
          // case of any data race.
          REPORT("Failed to wait for event " DPxMOD ".\n", DPxPTR(Event));
          return {{false /* IsNewEntry */, false /* IsHostPointer */},
                  nullptr /* Entry */,
                  nullptr /* TargetPointer */};
        }
      }
    }
  }

  return {{IsNew, IsHostPtr}, Entry, TargetPointer};
}

// Used by targetDataBegin, targetDataEnd, targetDataUpdate and target.
// Return the target pointer begin (where the data will be moved).
// Decrement the reference counter if called from targetDataEnd.
TargetPointerResultTy
DeviceTy::getTgtPtrBegin(void *HstPtrBegin, int64_t Size, bool &IsLast,
                         bool UpdateRefCount, bool UseHoldRefCount,
                         bool &IsHostPtr, bool MustContain, bool ForceDelete) {
  HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();

  void *TargetPointer = NULL;
  bool IsNew = false;
  IsHostPtr = false;
  IsLast = false;
  LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size);

  if (LR.Flags.IsContained ||
      (!MustContain && (LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter))) {
    auto &HT = *LR.Entry;
    IsLast = HT.decShouldRemove(UseHoldRefCount, ForceDelete);

    if (ForceDelete) {
      HT.resetRefCount(UseHoldRefCount);
      assert(IsLast == HT.decShouldRemove(UseHoldRefCount) &&
             "expected correct IsLast prediction for reset");
    }

    const char *RefCountAction;
    if (!UpdateRefCount) {
      RefCountAction = " (update suppressed)";
    } else if (IsLast) {
      // Mark the entry as to be deleted by this thread. Another thread might
      // reuse the entry and take "ownership" for the deletion while this thread
      // is waiting for data transfers. That is fine and the current thread will
      // simply skip the deletion step then.
      HT.setDeleteThreadId();
      HT.decRefCount(UseHoldRefCount);
      assert(HT.getTotalRefCount() == 0 &&
             "Expected zero reference count when deletion is scheduled");
      if (ForceDelete)
        RefCountAction = " (reset, delayed deletion)";
      else
        RefCountAction = " (decremented, delayed deletion)";
    } else {
      HT.decRefCount(UseHoldRefCount);
      RefCountAction = " (decremented)";
    }
    const char *DynRefCountAction = UseHoldRefCount ? "" : RefCountAction;
    const char *HoldRefCountAction = UseHoldRefCount ? RefCountAction : "";
    uintptr_t TP = HT.TgtPtrBegin + ((uintptr_t)HstPtrBegin - HT.HstPtrBegin);
    INFO(OMP_INFOTYPE_MAPPING_EXISTS, DeviceID,
         "Mapping exists with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD ", "
         "Size=%" PRId64 ", DynRefCount=%s%s, HoldRefCount=%s%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(TP), Size, HT.dynRefCountToStr().c_str(),
         DynRefCountAction, HT.holdRefCountToStr().c_str(), HoldRefCountAction);
    TargetPointer = (void *)TP;

#if INTEL_COLLAB
  } else if (!requiresMapping(HstPtrBegin, Size)) {
    DP("Get HstPtrBegin " DPxMOD " Size=%" PRId64
       " for device-accessible memory\n", DPxPTR(HstPtrBegin), Size);
    if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY)
      IsHostPtr = true;
    TargetPointer = HstPtrBegin;
  } else if ((PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) &&
             !managedMemorySupported()) {
#else // INTEL_COLLAB
  } else if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) {
#endif // INTEL_COLLAB

    // If the value isn't found in the mapping and unified shared memory
    // is on then it means we have stumbled upon a value which we need to
    // use directly from the host.
    DP("Get HstPtrBegin " DPxMOD " Size=%" PRId64 " for unified shared "
       "memory\n",
       DPxPTR((uintptr_t)HstPtrBegin), Size);
    IsHostPtr = true;
    TargetPointer = HstPtrBegin;
  }

  return {{IsNew, IsHostPtr}, LR.Entry, TargetPointer};
}

// Return the target pointer begin (where the data will be moved).
void *DeviceTy::getTgtPtrBegin(HDTTMapAccessorTy &HDTTMap, void *HstPtrBegin,
                               int64_t Size) {
  uintptr_t HP = (uintptr_t)HstPtrBegin;
  LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size);
  if (LR.Flags.IsContained || LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter) {
    auto &HT = *LR.Entry;
    uintptr_t TP = HT.TgtPtrBegin + (HP - HT.HstPtrBegin);
    return (void *)TP;
  }

  return NULL;
}

int DeviceTy::deallocTgtPtr(HDTTMapAccessorTy &HDTTMap, LookupResult LR,
                            int64_t Size) {
  // Check if the pointer is contained in any sub-nodes.
  if (!(LR.Flags.IsContained || LR.Flags.ExtendsBefore ||
        LR.Flags.ExtendsAfter)) {
    REPORT("Section to delete (hst addr " DPxMOD ") does not exist in the"
           " allocated memory\n",
           DPxPTR(LR.Entry->HstPtrBegin));
    return OFFLOAD_FAIL;
  }

  auto &HT = *LR.Entry;
  // Verify this thread is still in charge of deleting the entry.
  assert(HT.getTotalRefCount() == 0 &&
         HT.getDeleteThreadId() == std::this_thread::get_id() &&
         "Trying to delete entry that is in use or owned by another thread.");

  DP("Deleting tgt data " DPxMOD " of size %" PRId64 "\n",
     DPxPTR(HT.TgtPtrBegin), Size);
  deleteData((void *)HT.TgtPtrBegin);
  INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
       "Removing map entry with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD
       ", Size=%" PRId64 ", Name=%s\n",
       DPxPTR(HT.HstPtrBegin), DPxPTR(HT.TgtPtrBegin), Size,
       (HT.HstPtrName) ? getNameFromMapping(HT.HstPtrName).c_str() : "unknown");
  void *Event = LR.Entry->getEvent();
  HDTTMap->erase(LR.Entry);
  delete LR.Entry;

  int Ret = OFFLOAD_SUCCESS;
  if (Event && destroyEvent(Event) != OFFLOAD_SUCCESS) {
    REPORT("Failed to destroy event " DPxMOD "\n", DPxPTR(Event));
    Ret = OFFLOAD_FAIL;
  }

  return Ret;
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
  return OFFLOAD_FAIL;
}

void DeviceTy::deinit() {
  if (RTL->deinit_device)
    RTL->deinit_device(RTLDeviceID);
}

// Load binary to device.
__tgt_target_table *DeviceTy::loadBinary(void *Img) {
  std::lock_guard<decltype(RTL->Mtx)> LG(RTL->Mtx);
  return RTL->load_binary(RTLDeviceID, Img);
}

void *DeviceTy::allocData(int64_t Size, void *HstPtr, int32_t Kind) {
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataAllocBegin(RTLDeviceID, Size));
  auto CorrID = XPTIRegistry->traceMemAllocBegin(Size, 0 /* GuardZone */);
  void *Ret = RTL->data_alloc(RTLDeviceID, Size, HstPtr, Kind);
  XPTIRegistry->traceMemAllocEnd((uintptr_t)Ret, Size, 0 /* GuardZone */,
                                 CorrID);
  OMPT_TRACE(targetDataAllocEnd(RTLDeviceID, Size, Ret));
  return Ret;
#else // INTEL_CUSTOMIZATION
  return RTL->data_alloc(RTLDeviceID, Size, HstPtr, Kind);
#endif // INTEL_CUSTOMIZATION
}

int32_t DeviceTy::deleteData(void *TgtPtrBegin) {
#if INTEL_CUSTOMIZATION
  auto CorrID = XPTIRegistry->traceMemReleaseBegin((uintptr_t)TgtPtrBegin);
  auto Rc = RTL->data_delete(RTLDeviceID, TgtPtrBegin);
  XPTIRegistry->traceMemReleaseEnd((uintptr_t)TgtPtrBegin, CorrID);
  return Rc;
#else // INTEL_CUSTOMIZATION
  return RTL->data_delete(RTLDeviceID, TgtPtrBegin);
#endif // INTEL_CUSTOMIZATION
}

// Submit data to device
int32_t DeviceTy::submitData(void *TgtPtrBegin, void *HstPtrBegin, int64_t Size,
                             AsyncInfoTy &AsyncInfo) {
  if (getInfoLevel() & OMP_INFOTYPE_DATA_TRANSFER) {
    HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();
    LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size);
    auto *HT = &*LR.Entry;

    INFO(OMP_INFOTYPE_DATA_TRANSFER, DeviceID,
         "Copying data from host to device, HstPtr=" DPxMOD ", TgtPtr=" DPxMOD
         ", Size=%" PRId64 ", Name=%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(TgtPtrBegin), Size,
         (HT && HT->HstPtrName) ? getNameFromMapping(HT->HstPtrName).c_str()
                                : "unknown");
  }

#if INTEL_CUSTOMIZATION
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
#else // INTEL_CUSTOMIZATION
  if (!AsyncInfo || !RTL->data_submit_async || !RTL->synchronize)
    return RTL->data_submit(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size);
  return RTL->data_submit_async(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size,
                                AsyncInfo);
#endif // INTEL_CUSTOMIZATION
}

// Retrieve data from device
int32_t DeviceTy::retrieveData(void *HstPtrBegin, void *TgtPtrBegin,
                               int64_t Size, AsyncInfoTy &AsyncInfo) {
  if (getInfoLevel() & OMP_INFOTYPE_DATA_TRANSFER) {
    HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();
    LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size);
    auto *HT = &*LR.Entry;
    INFO(OMP_INFOTYPE_DATA_TRANSFER, DeviceID,
         "Copying data from device to host, TgtPtr=" DPxMOD ", HstPtr=" DPxMOD
         ", Size=%" PRId64 ", Name=%s\n",
         DPxPTR(TgtPtrBegin), DPxPTR(HstPtrBegin), Size,
         (HT && HT->HstPtrName) ? getNameFromMapping(HT->HstPtrName).c_str()
                                : "unknown");
  }

#if INTEL_CUSTOMIZATION
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
#else // INTEL_CUSTOMIZATION
  if (!RTL->data_retrieve_async || !RTL->synchronize)
    return RTL->data_retrieve(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size);
  return RTL->data_retrieve_async(RTLDeviceID, HstPtrBegin, TgtPtrBegin, Size,
                                  AsyncInfo);
#endif // INTEL_CUSTOMIZATION
}

// Copy data from current device to destination device directly
int32_t DeviceTy::dataExchange(void *SrcPtr, DeviceTy &DstDev, void *DstPtr,
                               int64_t Size, AsyncInfoTy &AsyncInfo) {
  if (!AsyncInfo || !RTL->data_exchange_async || !RTL->synchronize) {
    assert(RTL->data_exchange && "RTL->data_exchange is nullptr");
    return RTL->data_exchange(RTLDeviceID, SrcPtr, DstDev.RTLDeviceID, DstPtr,
                              Size);
  }
  return RTL->data_exchange_async(RTLDeviceID, SrcPtr, DstDev.RTLDeviceID,
                                  DstPtr, Size, AsyncInfo);
}

// Run region on device
int32_t DeviceTy::runRegion(void *TgtEntryPtr, void **TgtVarsPtr,
                            ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                            AsyncInfoTy &AsyncInfo) {
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, 1));
  int32_t Ret;
  if (!RTL->run_region_async || !RTL->synchronize)
    Ret = RTL->run_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                          TgtVarsSize);
  else
    Ret = RTL->run_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                TgtOffsets, TgtVarsSize, AsyncInfo);
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, 1));
  return Ret;
#else // INTEL_CUSTOMIZATION
  if (!RTL->run_region_async || !RTL->synchronize)
    return RTL->run_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                           TgtVarsSize);
  return RTL->run_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                               TgtVarsSize, AsyncInfo);
#endif // INTEL_CUSTOMIZATION
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
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, NumTeams));
  int32_t Ret;
  if (!RTL->run_team_region_async || !RTL->synchronize)
    Ret = RTL->run_team_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                               TgtVarsSize, NumTeams, ThreadLimit,
                               LoopTripCount);
  else
    Ret = RTL->run_team_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                     TgtOffsets, TgtVarsSize, NumTeams,
                                     ThreadLimit, LoopTripCount, AsyncInfo);
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, NumTeams));
  return Ret;
#else // INTEL_CUSTOMIZATION
  if (!RTL->run_team_region_async || !RTL->synchronize)
    return RTL->run_team_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                TgtOffsets, TgtVarsSize, NumTeams, ThreadLimit,
                                LoopTripCount);
  return RTL->run_team_region_async(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                    TgtOffsets, TgtVarsSize, NumTeams,
                                    ThreadLimit, LoopTripCount, AsyncInfo);
#endif // INTEL_CUSTOMIZATION
}

#if INTEL_COLLAB
int32_t DeviceTy::manifestDataForRegion(void *TgtEntryPtr) {
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
  SmallVector<void *> ObjectPtrs;

  HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();

  for (const auto &It : *HDTTMap) {
    HostDataToTargetTy &HDTT = *It.HDTT;
    if (!HDTT.isDynRefCountInf() ||
        // Function pointers has zero size, and we do not have to manifest
        // them, because program code is always resident on the device.
        HDTT.HstPtrBegin == HDTT.HstPtrEnd)
      continue;

    void *TgtPtrBegin = reinterpret_cast<void *>(HDTT.TgtPtrBegin);

    if (ObjectPtrs.empty())
      DP("Manifesting target pointers for globals:\n");

    DP("\tHstPtrBase=" DPxMOD ", HstPtrBegin=" DPxMOD
       ", HstPtrEnd=" DPxMOD ", TgtPtrBegin=" DPxMOD "\n",
       DPxPTR(HDTT.HstPtrBase), DPxPTR(HDTT.HstPtrBegin),
       DPxPTR(HDTT.HstPtrEnd), DPxPTR(TgtPtrBegin));

    ObjectPtrs.push_back(TgtPtrBegin);
  }

  HDTTMap.destroy();

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

char *DeviceTy::getDeviceName(char *Buffer, size_t BufferMaxSize) {
  assert(Buffer && "Buffer cannot be nullptr.");
  assert(BufferMaxSize > 0 && "BufferMaxSize cannot be zero.");
  if (RTL->get_device_name)
    return RTL->get_device_name(RTLDeviceID, Buffer, BufferMaxSize);
  // Make Buffer an empty string, if RTL does not support
  // name query.
  Buffer[0] = '\0';
  return Buffer;
}

void *DeviceTy::dataAllocBase(int64_t Size, void *HstPtrBegin,
                              void *HstPtrBase) {
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataAllocBegin(RTLDeviceID, Size));
  auto CorrID = XPTIRegistry->traceMemAllocBegin(Size, 0 /* GuardZone */);
#endif // INTEL_CUSTOMIZATION
  void *Ret =
      RTL->data_alloc_base
          ? RTL->data_alloc_base(RTLDeviceID, Size, HstPtrBegin, HstPtrBase)
          : RTL->data_alloc(RTLDeviceID, Size, HstPtrBegin,
                            TARGET_ALLOC_DEFAULT);
#if INTEL_CUSTOMIZATION
  XPTIRegistry->traceMemAllocEnd((uintptr_t)Ret, Size, 0 /* GuardZone */,
                                 CorrID);
  OMPT_TRACE(targetDataAllocEnd(RTLDeviceID, Size, Ret));
#endif // INTEL_CUSTOMIZATION
  return Ret;
}

int32_t DeviceTy::runTeamNDRegion(void *TgtEntryPtr, void **TgtVarsPtr,
                                  ptrdiff_t *TgtOffsets, int32_t TgtVarsSize,
                                  int32_t NumTeams, int32_t ThreadLimit,
                                  void *TgtNDLoopDesc) {
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, NumTeams));
#endif // INTEL_CUSTOMIZATION
  int32_t Ret = RTL->run_team_nd_region
      ? RTL->run_team_nd_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                TgtOffsets, TgtVarsSize, NumTeams, ThreadLimit,
                                TgtNDLoopDesc)
      : OFFLOAD_FAIL;
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, NumTeams));
#endif // INTEL_CUSTOMIZATION
  return Ret;
}

void *DeviceTy::getContextHandle() {
  if (!RTL->get_context_handle)
    return nullptr;
  return RTL->get_context_handle(RTLDeviceID);
}

void *DeviceTy::dataAllocManaged(int64_t Size) {
  if (RTL->data_alloc_managed)
    return RTL->data_alloc_managed(RTLDeviceID, Size);
  else
    return RTL->data_alloc(RTLDeviceID, Size, nullptr, TARGET_ALLOC_DEFAULT);
}

int32_t DeviceTy::requiresMapping(void *Ptr, int64_t Size) {
  if (RTL->requires_mapping)
    return RTL->requires_mapping(RTLDeviceID, Ptr, Size);
  else
    return 1;
}

int32_t DeviceTy::managedMemorySupported() {
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

bool DeviceTy::registerHostPointer(void *Ptr, size_t Size) {
  if (RTL->register_host_pointer)
    return RTL->register_host_pointer(RTLDeviceID, Ptr,Size);
  else
    return false;
}

bool DeviceTy::unregisterHostPointer(void *Ptr) {
  if (RTL->unregister_host_pointer)
    return RTL->unregister_host_pointer(DeviceID, Ptr);
  else
    return false;
}

int32_t DeviceTy::getDataAllocInfo(
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

#if INTEL_CUSTOMIZATION
__tgt_interop *DeviceTy::createInterop(int32_t InteropContext,
                                       int32_t NumPrefers,
                                       int32_t *PreferIDs) {
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
#endif // INTEL_CUSTOMIZATION

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
    LambdaPtrs.emplace(GTID, SmallVector<void *>{});
  LambdaPtrs.at(GTID).push_back(TgtPtr);
}

int32_t DeviceTy::isAccessibleAddrRange(const void *Ptr, size_t Size) {
  if (RTL->is_accessible_addr_range)
    return RTL->is_accessible_addr_range(RTLDeviceID, Ptr, Size);
  else
    return 0;
}

#if INTEL_CUSTOMIZATION
int32_t DeviceTy::notifyIndirectAccess(const void *Ptr, size_t Offset) {
  if (RTL->notify_indirect_access)
    return RTL->notify_indirect_access(RTLDeviceID, Ptr, Offset);
  else
    return OFFLOAD_SUCCESS;
}
#endif // INTEL_CUSTOMIZATION

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

int32_t DeviceTy::setFunctionPtrMap() {
  uint64_t Size = FnPtrMap.size();
  if (Size == 0)
    return OFFLOAD_SUCCESS;
  if (!RTL->set_function_ptr_map)
    return OFFLOAD_FAIL;
  SmallVector<__omp_offloading_fptr_map_t> FnPtrs;
  for (auto &FnPtr : FnPtrMap)
    FnPtrs.push_back({FnPtr.first, FnPtr.second});
  return RTL->set_function_ptr_map(RTLDeviceID, Size, FnPtrs.data());
}

int32_t DeviceTy::supportsPerHWThreadScratch(void) {
  if (RTL->alloc_per_hw_thread_scratch)
    return 1;
  else
    return 0;
}

void *DeviceTy::allocPerHWThreadScratch(size_t ObjSize, int32_t AllocKind) {
  if (RTL->alloc_per_hw_thread_scratch)
    return RTL->alloc_per_hw_thread_scratch(RTLDeviceID, ObjSize, AllocKind);
  else
    return nullptr;
}

void DeviceTy::freePerHWThreadScratch(void *Ptr) {
  if (RTL->free_per_hw_thread_scratch)
    RTL->free_per_hw_thread_scratch(RTLDeviceID, Ptr);
}

int32_t DeviceTy::getDeviceInfo(int32_t InfoID, size_t InfoSize,
                                void *InfoValue, size_t *InfoSizeRet) {
  if (RTL->get_device_info)
    return RTL->get_device_info(RTLDeviceID, InfoID, InfoSize, InfoValue,
                                InfoSizeRet);
  else
    return OFFLOAD_SUCCESS;
}

void *DeviceTy::dataAlignedAllocShared(size_t Align, size_t Size,
                                       int32_t AccessHint) {
  if (RTL->data_aligned_alloc_shared)
    return RTL->data_aligned_alloc_shared(RTLDeviceID, Align, Size, AccessHint);
  else
    return dataAlignedAlloc(Align, Size, TARGET_ALLOC_SHARED);
}

int DeviceTy::prefetchSharedMem(size_t NumPtrs, void **Ptrs, size_t *Sizes) {
  if (RTL->prefetch_shared_mem)
    return RTL->prefetch_shared_mem(RTLDeviceID, NumPtrs, Ptrs, Sizes);
  else
    return OFFLOAD_SUCCESS; // no-op if not supported
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
bool deviceIsReady(int DeviceNum) {
  DP("Checking whether device %d is ready.\n", DeviceNum);
  // Devices.size() can only change while registering a new
  // library, so try to acquire the lock of RTLs' mutex.
  size_t DevicesSize;
  {
    std::lock_guard<decltype(PM->RTLsMtx)> LG(PM->RTLsMtx);
    DevicesSize = PM->Devices.size();
  }
  if (DevicesSize <= (size_t)DeviceNum) {
    DP("Device ID  %d does not have a matching RTL\n", DeviceNum);
    return false;
  }

  // Get device info
  DeviceTy &Device = *PM->Devices[DeviceNum];

  DP("Is the device %d (local ID %d) initialized? %d\n", DeviceNum,
     Device.RTLDeviceID, Device.IsInit);

  // Init the device if not done before
  if (!Device.IsInit && Device.initOnce() != OFFLOAD_SUCCESS) {
    DP("Failed to init device %d\n", DeviceNum);
    return false;
  }

  DP("Device %d is ready to use.\n", DeviceNum);

  return true;
}
