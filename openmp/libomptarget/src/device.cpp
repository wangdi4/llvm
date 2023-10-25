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
#include "OmptCallback.h"
#include "OmptInterface.h"
#include "omptarget.h"
#include "private.h"
#include "rtl.h"

#include "Utilities.h"

#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>

#ifdef INTEL_CUSTOMIZATION
using llvm::SmallVector;
#endif // INTEL_CUSTOMIZATION
#ifdef OMPT_SUPPORT
using namespace llvm::omp::target::ompt;
#endif

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
      HasPendingGlobals(false), PendingCtorsDtors(), PendingGlobalsMtx() {}

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
    std::lock_guard<HostDataToTargetTy> LG(HDTT);
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
               /*TgtAllocBegin=*/(uintptr_t)TgtPtrBegin,
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

  // Notify the plugin about the new mapping.
  return notifyDataMapped(HstPtrBegin, Size);
}

int DeviceTy::disassociatePtr(void *HstPtrBegin) {
  HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();

  auto It = HDTTMap->find(HstPtrBegin);
  if (It == HDTTMap->end()) {
    REPORT("Association not found\n");
    return OFFLOAD_FAIL;
  }
  // Mapping exists
  HostDataToTargetTy &HDTT = *It->HDTT;
#if INTEL_CUSTOMIZATION
  // Use unique_lock to release ownership before deleting HDTT below.
  std::unique_lock<HostDataToTargetTy> LG(HDTT);
#else  // INTEL_CUSTOMIZATION
  std::lock_guard<HostDataToTargetTy> LG(HDTT);
#endif // INTEL_CUSTOMIZATION

  if (HDTT.getHoldRefCount()) {
    // This is based on OpenACC 3.1, sec 3.2.33 "acc_unmap_data", L3656-3657:
    // "It is an error to call acc_unmap_data if the structured reference
    // count for the pointer is not zero."
    REPORT("Trying to disassociate a pointer with a non-zero hold reference "
           "count\n");
    return OFFLOAD_FAIL;
  }

  if (HDTT.isDynRefCountInf()) {
    DP("Association found, removing it\n");
    void *Event = HDTT.getEvent();
#if INTEL_CUSTOMIZATION
    LG.unlock();
#endif // INTEL_CUSTOMIZATION
    delete &HDTT;
    if (Event)
      destroyEvent(Event);
    HDTTMap->erase(It);
    return notifyDataUnmapped(HstPtrBegin);
  }

  REPORT("Trying to disassociate a pointer which was not mapped via "
         "omp_target_associate_ptr\n");
  return OFFLOAD_FAIL;
}

LookupResult DeviceTy::lookupMapping(HDTTMapAccessorTy &HDTTMap,
                                     void *HstPtrBegin, int64_t Size,
                                     HostDataToTargetTy *OwnedTPR) {

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
      LR.TPR.setEntry(std::prev(Upper)->HDTT, OwnedTPR);
      // the left side of extended address range is satisified.
      // hp >= LR.TPR.getEntry()->HstPtrBegin || hp >=
      // LR.TPR.getEntry()->HstPtrBase
      LR.Flags.IsContained = HP < LR.TPR.getEntry()->HstPtrEnd ||
                             HP < LR.TPR.getEntry()->HstPtrBase;
    }

    if (!LR.Flags.IsContained && Upper != HDTTMap->end()) {
      LR.TPR.setEntry(Upper->HDTT, OwnedTPR);
      // the right side of extended address range is satisified.
      // hp < LR.TPR.getEntry()->HstPtrEnd || hp < LR.TPR.getEntry()->HstPtrBase
      LR.Flags.IsContained = HP >= LR.TPR.getEntry()->HstPtrBase;
    }
  } else {
    // check the left bin
    if (Upper != HDTTMap->begin()) {
      LR.TPR.setEntry(std::prev(Upper)->HDTT, OwnedTPR);
      // Is it contained?
      LR.Flags.IsContained = HP >= LR.TPR.getEntry()->HstPtrBegin &&
                             HP < LR.TPR.getEntry()->HstPtrEnd &&
                             (HP + Size) <= LR.TPR.getEntry()->HstPtrEnd;
      // Does it extend beyond the mapped region?
      LR.Flags.ExtendsAfter = HP < LR.TPR.getEntry()->HstPtrEnd &&
                              (HP + Size) > LR.TPR.getEntry()->HstPtrEnd;
    }

    // check the right bin
    if (!(LR.Flags.IsContained || LR.Flags.ExtendsAfter) &&
        Upper != HDTTMap->end()) {
      LR.TPR.setEntry(Upper->HDTT, OwnedTPR);
      // Does it extend into an already mapped region?
      LR.Flags.ExtendsBefore = HP < LR.TPR.getEntry()->HstPtrBegin &&
                               (HP + Size) > LR.TPR.getEntry()->HstPtrBegin;
      // Does it extend beyond the mapped region?
      LR.Flags.ExtendsAfter = HP < LR.TPR.getEntry()->HstPtrEnd &&
                              (HP + Size) > LR.TPR.getEntry()->HstPtrEnd;
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
    HDTTMapAccessorTy &HDTTMap, void *HstPtrBegin, void *HstPtrBase,
    int64_t TgtPadding, int64_t Size, map_var_info_t HstPtrName, bool HasFlagTo,
    bool HasFlagAlways, bool IsImplicit, bool UpdateRefCount,
    bool HasCloseModifier, bool HasPresentModifier, bool HasHoldModifier,
    AsyncInfoTy &AsyncInfo,
#if INTEL_COLLAB
    HostDataToTargetTy *OwnedTPR, bool ReleaseHDTTMap, bool UseHostMem) {
#else  // INTEL_COLLAB
    HostDataToTargetTy *OwnedTPR, bool ReleaseHDTTMap) {
#endif // INTEL_COLLAB

  LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size, OwnedTPR);
  LR.TPR.Flags.IsPresent = true;

  // Release the mapping table lock only after the entry is locked by
  // attaching it to TPR. Once TPR is destroyed it will release the lock
  // on entry. If it is returned the lock will move to the returned object.
  // If LR.Entry is already owned/locked we avoid trying to lock it again.

  // Check if the pointer is contained.
  // If a variable is mapped to the device manually by the user - which would
  // lead to the IsContained flag to be true - then we must ensure that the
  // device address is returned even under unified memory conditions.
  if (LR.Flags.IsContained ||
      ((LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter) && IsImplicit)) {
    const char *RefCountAction;
    if (UpdateRefCount) {
      // After this, reference count >= 1. If the reference count was 0 but the
      // entry was still there we can reuse the data on the device and avoid a
      // new submission.
      LR.TPR.getEntry()->incRefCount(HasHoldModifier);
      RefCountAction = " (incremented)";
    } else {
      // It might have been allocated with the parent, but it's still new.
      LR.TPR.Flags.IsNewEntry = LR.TPR.getEntry()->getTotalRefCount() == 1;
      RefCountAction = " (update suppressed)";
    }
    const char *DynRefCountAction = HasHoldModifier ? "" : RefCountAction;
    const char *HoldRefCountAction = HasHoldModifier ? RefCountAction : "";
    uintptr_t Ptr = LR.TPR.getEntry()->TgtPtrBegin +
                    ((uintptr_t)HstPtrBegin - LR.TPR.getEntry()->HstPtrBegin);
    INFO(OMP_INFOTYPE_MAPPING_EXISTS, DeviceID,
         "Mapping exists%s with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD
         ", Size=%" PRId64 ", DynRefCount=%s%s, HoldRefCount=%s%s, Name=%s\n",
         (IsImplicit ? " (implicit)" : ""), DPxPTR(HstPtrBegin), DPxPTR(Ptr),
         Size, LR.TPR.getEntry()->dynRefCountToStr().c_str(), DynRefCountAction,
         LR.TPR.getEntry()->holdRefCountToStr().c_str(), HoldRefCountAction,
         (HstPtrName) ? getNameFromMapping(HstPtrName).c_str() : "unknown");
    LR.TPR.TargetPointer = (void *)Ptr;
  } else if ((LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter) && !IsImplicit) {
    // Explicit extension of mapped data - not allowed.
    MESSAGE("explicit extension not allowed: host address specified is " DPxMOD
            " (%" PRId64
            " bytes), but device allocation maps to host at " DPxMOD
            " (%" PRId64 " bytes)",
            DPxPTR(HstPtrBegin), Size, DPxPTR(LR.TPR.getEntry()->HstPtrBegin),
            LR.TPR.getEntry()->HstPtrEnd - LR.TPR.getEntry()->HstPtrBegin);
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
      LR.TPR.Flags.IsHostPointer = true;
    LR.TPR.Flags.IsPresent = false;
    LR.TPR.TargetPointer = HstPtrBegin;
#endif // INTEL_COLLAB
  } else if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY &&
             !HasCloseModifier) {
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
      LR.TPR.Flags.IsPresent = false;
      LR.TPR.Flags.IsHostPointer = true;
      LR.TPR.TargetPointer = HstPtrBegin;
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
    LR.TPR.Flags.IsNewEntry = true;
#if INTEL_COLLAB
    int32_t AllocOpt = UseHostMem ? ALLOC_OPT_HOST_MEM : ALLOC_OPT_NONE;
    uintptr_t TgtAllocBegin = (uintptr_t)dataAllocBase(
        TgtPadding + Size, HstPtrBegin, HstPtrBase, AllocOpt);
#else  // INTEL_COLLAB
    uintptr_t TgtAllocBegin =
        (uintptr_t)allocData(TgtPadding + Size, HstPtrBegin);
#endif // INTEL_COLLAB
    uintptr_t TgtPtrBegin = TgtAllocBegin + TgtPadding;
    // Release the mapping table lock only after the entry is locked by
    // attaching it to TPR.
    LR.TPR.setEntry(HDTTMap
                        ->emplace(new HostDataToTargetTy(
                            (uintptr_t)HstPtrBase, (uintptr_t)HstPtrBegin,
                            (uintptr_t)HstPtrBegin + Size, TgtAllocBegin,
                            TgtPtrBegin, HasHoldModifier, HstPtrName))
                        .first->HDTT);
#if INTEL_CUSTOMIZATION
    XPTIRegistry->traceMemAssociate((uintptr_t)HstPtrBegin, TgtPtrBegin);
#endif // INTEL_CUSTOMIZATION
    INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
         "Creating new map entry with HstPtrBase=" DPxMOD
         ", HstPtrBegin=" DPxMOD ", TgtAllocBegin=" DPxMOD", TgtPtrBegin=" DPxMOD
#if INTEL_COLLAB
         ", Size=%" PRId64 ", "
#else  // INTEL_COLLAB
         ", Size=%ld, "
#endif // INTEL_COLLAB
         "DynRefCount=%s, HoldRefCount=%s, Name=%s\n",
         DPxPTR(HstPtrBase), DPxPTR(HstPtrBegin), DPxPTR(TgtAllocBegin),
         DPxPTR(TgtPtrBegin), Size,
         LR.TPR.getEntry()->dynRefCountToStr().c_str(),
         LR.TPR.getEntry()->holdRefCountToStr().c_str(),
         (HstPtrName) ? getNameFromMapping(HstPtrName).c_str() : "unknown");
    LR.TPR.TargetPointer = (void *)TgtPtrBegin;

    // Notify the plugin about the new mapping.
    if (notifyDataMapped(HstPtrBegin, Size))
      return {{false /* IsNewEntry */, false /* IsHostPointer */},
              nullptr /* Entry */,
              nullptr /* TargetPointer */};
  } else {
    // This entry is not present and we did not create a new entry for it.
    LR.TPR.Flags.IsPresent = false;
  }

  // All mapping table modifications have been made. If the user requested it we
  // give up the lock.
  if (ReleaseHDTTMap)
    HDTTMap.destroy();

  // If the target pointer is valid, and we need to transfer data, issue the
  // data transfer.
  if (LR.TPR.TargetPointer && !LR.TPR.Flags.IsHostPointer && HasFlagTo &&
      (LR.TPR.Flags.IsNewEntry || HasFlagAlways) && Size != 0) {
    DP("Moving %" PRId64 " bytes (hst:" DPxMOD ") -> (tgt:" DPxMOD ")\n", Size,
       DPxPTR(HstPtrBegin), DPxPTR(LR.TPR.TargetPointer));

    int Ret = submitData(LR.TPR.TargetPointer, HstPtrBegin, Size, AsyncInfo,
                         LR.TPR.getEntry());
    if (Ret != OFFLOAD_SUCCESS) {
      REPORT("Copying data to device failed.\n");
      // We will also return nullptr if the data movement fails because that
      // pointer points to a corrupted memory region so it doesn't make any
      // sense to continue to use it.
      LR.TPR.TargetPointer = nullptr;
    } else if (LR.TPR.getEntry()->addEventIfNecessary(*this, AsyncInfo) !=
               OFFLOAD_SUCCESS)
      return {{false /* IsNewEntry */, false /* IsHostPointer */},
              nullptr /* Entry */,
              nullptr /* TargetPointer */};
  } else {
    // If not a host pointer and no present modifier, we need to wait for the
    // event if it exists.
    // Note: Entry might be nullptr because of zero length array section.
    if (LR.TPR.getEntry() && !LR.TPR.Flags.IsHostPointer &&
        !HasPresentModifier) {
      void *Event = LR.TPR.getEntry()->getEvent();
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

  return std::move(LR.TPR);
}

TargetPointerResultTy
DeviceTy::getTgtPtrBegin(void *HstPtrBegin, int64_t Size, bool UpdateRefCount,
                         bool UseHoldRefCount, bool MustContain,
                         bool ForceDelete, bool FromDataEnd) {
  HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor();

  LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size);

  LR.TPR.Flags.IsPresent = true;

  if (LR.Flags.IsContained ||
      (!MustContain && (LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter))) {
    LR.TPR.Flags.IsLast =
        LR.TPR.getEntry()->decShouldRemove(UseHoldRefCount, ForceDelete);

    if (ForceDelete) {
      LR.TPR.getEntry()->resetRefCount(UseHoldRefCount);
      assert(LR.TPR.Flags.IsLast ==
                 LR.TPR.getEntry()->decShouldRemove(UseHoldRefCount) &&
             "expected correct IsLast prediction for reset");
    }

    // Increment the number of threads that is using the entry on a
    // targetDataEnd, tracking the number of possible "deleters". A thread may
    // come to own the entry deletion even if it was not the last one querying
    // for it. Thus, we must track every query on targetDataEnds to ensure only
    // the last thread that holds a reference to an entry actually deletes it.
    if (FromDataEnd)
      LR.TPR.getEntry()->incDataEndThreadCount();

    const char *RefCountAction;
    if (!UpdateRefCount) {
      RefCountAction = " (update suppressed)";
    } else if (LR.TPR.Flags.IsLast) {
      LR.TPR.getEntry()->decRefCount(UseHoldRefCount);
      assert(LR.TPR.getEntry()->getTotalRefCount() == 0 &&
             "Expected zero reference count when deletion is scheduled");
      if (ForceDelete)
        RefCountAction = " (reset, delayed deletion)";
      else
        RefCountAction = " (decremented, delayed deletion)";
    } else {
      LR.TPR.getEntry()->decRefCount(UseHoldRefCount);
      RefCountAction = " (decremented)";
    }
    const char *DynRefCountAction = UseHoldRefCount ? "" : RefCountAction;
    const char *HoldRefCountAction = UseHoldRefCount ? RefCountAction : "";
    uintptr_t TP = LR.TPR.getEntry()->TgtPtrBegin +
                   ((uintptr_t)HstPtrBegin - LR.TPR.getEntry()->HstPtrBegin);
    INFO(OMP_INFOTYPE_MAPPING_EXISTS, DeviceID,
         "Mapping exists with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD ", "
         "Size=%" PRId64 ", DynRefCount=%s%s, HoldRefCount=%s%s\n",
         DPxPTR(HstPtrBegin), DPxPTR(TP), Size,
         LR.TPR.getEntry()->dynRefCountToStr().c_str(), DynRefCountAction,
         LR.TPR.getEntry()->holdRefCountToStr().c_str(), HoldRefCountAction);
    LR.TPR.TargetPointer = (void *)TP;
#if INTEL_COLLAB
  } else if (!requiresMapping(HstPtrBegin, Size)) {
    DP("Get HstPtrBegin " DPxMOD " Size=%" PRId64
       " for device-accessible memory\n", DPxPTR(HstPtrBegin), Size);
    if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY)
      LR.TPR.Flags.IsHostPointer = true;
    LR.TPR.Flags.IsPresent = false;
    LR.TPR.TargetPointer = HstPtrBegin;
#endif // INTEL_COLLAB
  } else if (PM->RTLs.RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) {

    // If the value isn't found in the mapping and unified shared memory
    // is on then it means we have stumbled upon a value which we need to
    // use directly from the host.
    DP("Get HstPtrBegin " DPxMOD " Size=%" PRId64 " for unified shared "
       "memory\n",
       DPxPTR((uintptr_t)HstPtrBegin), Size);
    LR.TPR.Flags.IsPresent = false;
    LR.TPR.Flags.IsHostPointer = true;
    LR.TPR.TargetPointer = HstPtrBegin;
  } else {
    // OpenMP Specification v5.2: if a matching list item is not found, the
    // pointer retains its original value as per firstprivate semantics.
    LR.TPR.Flags.IsPresent = false;
    LR.TPR.Flags.IsHostPointer = false;
    LR.TPR.TargetPointer = HstPtrBegin;
  }

  return std::move(LR.TPR);
}

// Return the target pointer begin (where the data will be moved).
void *DeviceTy::getTgtPtrBegin(HDTTMapAccessorTy &HDTTMap, void *HstPtrBegin,
                               int64_t Size) {
  uintptr_t HP = (uintptr_t)HstPtrBegin;
  LookupResult LR = lookupMapping(HDTTMap, HstPtrBegin, Size);
  if (LR.Flags.IsContained || LR.Flags.ExtendsBefore || LR.Flags.ExtendsAfter) {
    uintptr_t TP =
        LR.TPR.getEntry()->TgtPtrBegin + (HP - LR.TPR.getEntry()->HstPtrBegin);
    return (void *)TP;
  }

  return NULL;
}

int DeviceTy::eraseMapEntry(HDTTMapAccessorTy &HDTTMap,
                            HostDataToTargetTy *Entry, int64_t Size) {
  assert(Entry && "Trying to delete a null entry from the HDTT map.");
  assert(Entry->getTotalRefCount() == 0 && Entry->getDataEndThreadCount() == 0 &&
         "Trying to delete entry that is in use or owned by another thread.");

  INFO(OMP_INFOTYPE_MAPPING_CHANGED, DeviceID,
       "Removing map entry with HstPtrBegin=" DPxMOD ", TgtPtrBegin=" DPxMOD
       ", Size=%" PRId64 ", Name=%s\n",
       DPxPTR(Entry->HstPtrBegin), DPxPTR(Entry->TgtPtrBegin), Size,
       (Entry->HstPtrName) ? getNameFromMapping(Entry->HstPtrName).c_str()
                           : "unknown");

  if (HDTTMap->erase(Entry) == 0) {
    REPORT("Trying to remove a non-existent map entry\n");
    return OFFLOAD_FAIL;
  }

  return OFFLOAD_SUCCESS;
}

int DeviceTy::deallocTgtPtrAndEntry(HostDataToTargetTy *Entry, int64_t Size) {
  assert(Entry && "Trying to deallocate a null entry.");

  DP("Deleting tgt data " DPxMOD " of size %" PRId64 " by freeing allocation "
     "starting at " DPxMOD "\n",
     DPxPTR(Entry->TgtPtrBegin), Size, DPxPTR(Entry->TgtAllocBegin));

  void *Event = Entry->getEvent();
  if (Event && destroyEvent(Event) != OFFLOAD_SUCCESS) {
    REPORT("Failed to destroy event " DPxMOD "\n", DPxPTR(Event));
    return OFFLOAD_FAIL;
  }

  int Ret = deleteData((void *)Entry->TgtAllocBegin);

  // Notify the plugin about the unmapped memory.
  Ret |= notifyDataUnmapped((void *)Entry->HstPtrBegin);

  delete Entry;

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

  // Enables recording kernels if set.
  llvm::omp::target::BoolEnvar OMPX_RecordKernel("LIBOMPTARGET_RECORD", false);
  if (OMPX_RecordKernel) {
    // Enables saving the device memory kernel output post execution if set.
    llvm::omp::target::BoolEnvar OMPX_ReplaySaveOutput(
        "LIBOMPTARGET_RR_SAVE_OUTPUT", false);
    // Sets the maximum to pre-allocate device memory.
    llvm::omp::target::UInt64Envar OMPX_DeviceMemorySize(
        "LIBOMPTARGET_RR_DEVMEM_SIZE", 16);
#if INTEL_CUSTOMIZATION
    DP("Activating Record-Replay for Device %d with %lu GB memory\n",
       RTLDeviceID, (unsigned long) OMPX_DeviceMemorySize.get());
#else // INTEL_CUSTOMIZATION
    DP("Activating Record-Replay for Device %d with %lu GB memory\n",
       RTLDeviceID, OMPX_DeviceMemorySize.get());
#endif // INTEL_CUSTOMIZATION

    RTL->activate_record_replay(RTLDeviceID,
                                OMPX_DeviceMemorySize * 1024 * 1024 * 1024,
                                true, OMPX_ReplaySaveOutput);
  }

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
#else  // INTEL_CUSTOMIZATION
  /// RAII to establish tool anchors before and after data allocation
  void *TargetPtr = nullptr;
  OMPT_IF_BUILT(InterfaceRAII TargetDataAllocRAII(
                    RegionInterface.getCallbacks<ompt_target_data_alloc>(),
                    DeviceID, HstPtr, &TargetPtr, Size,
                    /* CodePtr */ OMPT_GET_RETURN_ADDRESS(0));)

  TargetPtr = RTL->data_alloc(RTLDeviceID, Size, HstPtr, Kind);
  return TargetPtr;
#endif // INTEL_CUSTOMIZATION
}

int32_t DeviceTy::deleteData(void *TgtAllocBegin, int32_t Kind) {
#if INTEL_CUSTOMIZATION
  auto CorrID = XPTIRegistry->traceMemReleaseBegin((uintptr_t)TgtAllocBegin);
  auto Rc = RTL->data_delete(RTLDeviceID, TgtAllocBegin, Kind);
  XPTIRegistry->traceMemReleaseEnd((uintptr_t)TgtAllocBegin, CorrID);
  return Rc;
#else  // INTEL_CUSTOMIZATION
  /// RAII to establish tool anchors before and after data deletion
  OMPT_IF_BUILT(InterfaceRAII TargetDataDeleteRAII(
                    RegionInterface.getCallbacks<ompt_target_data_delete>(),
                    DeviceID, TgtAllocBegin,
                    /* CodePtr */ OMPT_GET_RETURN_ADDRESS(0));)

  return RTL->data_delete(RTLDeviceID, TgtAllocBegin, Kind);
#endif // INTEL_CUSTOMIZATION
}

static void printCopyInfo(int DeviceId, bool H2D, void *SrcPtrBegin,
                          void *DstPtrBegin, int64_t Size,
                          HostDataToTargetTy *HT) {

  INFO(OMP_INFOTYPE_DATA_TRANSFER, DeviceId,
       "Copying data from %s to %s, %sPtr=" DPxMOD ", %sPtr=" DPxMOD
       ", Size=%" PRId64 ", Name=%s\n",
       H2D ? "host" : "device", H2D ? "device" : "host", H2D ? "Hst" : "Tgt",
       DPxPTR(SrcPtrBegin), H2D ? "Tgt" : "Hst", DPxPTR(DstPtrBegin), Size,
       (HT && HT->HstPtrName) ? getNameFromMapping(HT->HstPtrName).c_str()
                              : "unknown");
}

// Submit data to device
int32_t DeviceTy::submitData(void *TgtPtrBegin, void *HstPtrBegin, int64_t Size,
                             AsyncInfoTy &AsyncInfo,
                             HostDataToTargetTy *Entry) {
  if (getInfoLevel() & OMP_INFOTYPE_DATA_TRANSFER) {
    HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor(Entry);
    LookupResult LR;
    if (!Entry) {
      LR = lookupMapping(HDTTMap, HstPtrBegin, Size);
      Entry = LR.TPR.getEntry();
    }
    printCopyInfo(DeviceID, /* H2D */ true, HstPtrBegin, TgtPtrBegin, Size,
                  Entry);
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
#else  // INTEL_CUSTOMIZATION
  /// RAII to establish tool anchors before and after data submit
  OMPT_IF_BUILT(
      InterfaceRAII TargetDataSubmitRAII(
          RegionInterface.getCallbacks<ompt_target_data_transfer_to_device>(),
          DeviceID, TgtPtrBegin, HstPtrBegin, Size,
          /* CodePtr */ OMPT_GET_RETURN_ADDRESS(0));)

  if (!AsyncInfo || !RTL->data_submit_async || !RTL->synchronize)
    return RTL->data_submit(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size);
  return RTL->data_submit_async(RTLDeviceID, TgtPtrBegin, HstPtrBegin, Size,
                                AsyncInfo);
#endif // INTEL_CUSTOMIZATION
}

// Retrieve data from device
int32_t DeviceTy::retrieveData(void *HstPtrBegin, void *TgtPtrBegin,
                               int64_t Size, AsyncInfoTy &AsyncInfo,
                               HostDataToTargetTy *Entry) {
  if (getInfoLevel() & OMP_INFOTYPE_DATA_TRANSFER) {
    HDTTMapAccessorTy HDTTMap = HostDataToTargetMap.getExclusiveAccessor(Entry);
    LookupResult LR;
    if (!Entry) {
      LR = lookupMapping(HDTTMap, HstPtrBegin, Size);
      Entry = LR.TPR.getEntry();
    }
    printCopyInfo(DeviceID, /* H2D */ false, TgtPtrBegin, HstPtrBegin, Size,
                  Entry);
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
#else  // INTEL_CUSTOMIZATION
  /// RAII to establish tool anchors before and after data retrieval
  OMPT_IF_BUILT(
      InterfaceRAII TargetDataRetrieveRAII(
          RegionInterface.getCallbacks<ompt_target_data_transfer_from_device>(),
          DeviceID, HstPtrBegin, TgtPtrBegin, Size,
          /* CodePtr */ OMPT_GET_RETURN_ADDRESS(0));)

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

int32_t DeviceTy::notifyDataMapped(void *HstPtr, int64_t Size) {
  if (!RTL->data_notify_mapped)
    return OFFLOAD_SUCCESS;

  DP("Notifying about new mapping: HstPtr=" DPxMOD ", Size=%" PRId64 "\n",
     DPxPTR(HstPtr), Size);

  if (RTL->data_notify_mapped(RTLDeviceID, HstPtr, Size)) {
    REPORT("Notifiying about data mapping failed.\n");
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t DeviceTy::notifyDataUnmapped(void *HstPtr) {
  if (!RTL->data_notify_unmapped)
    return OFFLOAD_SUCCESS;

  DP("Notifying about an unmapping: HstPtr=" DPxMOD "\n", DPxPTR(HstPtr));

  if (RTL->data_notify_unmapped(RTLDeviceID, HstPtr)) {
    REPORT("Notifiying about data unmapping failed.\n");
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

// Run region on device
int32_t DeviceTy::launchKernel(void *TgtEntryPtr, void **TgtVarsPtr,
                               ptrdiff_t *TgtOffsets,
                               const KernelArgsTy &KernelArgs,
                               AsyncInfoTy &AsyncInfo) {
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, KernelArgs.NumTeams[0]));
  int32_t Ret = RTL->launch_kernel(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                   TgtOffsets, &KernelArgs, AsyncInfo);
  OMPT_TRACE(targetSubmitEnd(RTLDeviceID, KernelArgs.NumTeams[0]));
  return Ret;
#else  // INTEL_CUSTOMIZATION
  return RTL->launch_kernel(RTLDeviceID, TgtEntryPtr, TgtVarsPtr, TgtOffsets,
                            &KernelArgs, AsyncInfo);
#endif // INTEL_CUSTOMIZATION
}

// Run region on device
bool DeviceTy::printDeviceInfo(int32_t RTLDevId) {
  if (!RTL->print_device_info)
    return false;
  RTL->print_device_info(RTLDevId);
  return true;
}

#if INTEL_COLLAB
int32_t DeviceTy::getGroupsShape(void *TgtEntryPtr, int32_t NumTeams,
                                 int32_t ThreadLimit, void *GroupSizes,
                                 void *GroupCounts, void *LoopDesc) {
  return RTL->get_groups_shape(RTLDeviceID, NumTeams, ThreadLimit, TgtEntryPtr,
                               GroupSizes, GroupCounts, LoopDesc);
}

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
    // Check if it has associated shadow pointers
    (void)HDTT.foreachShadowPointerInfo(
        [&](const ShadowPtrInfoTy &SPI) {
          DP("Manifesting shadow target pointers:\n");
          DP("\tHstPtrAddr=" DPxMOD ", HstPtrVal=" DPxMOD
             ", TgtPtrAddr=" DPxMOD ", TgtPtrVal=" DPxMOD "\n",
             DPxPTR(SPI.HstPtrAddr), DPxPTR(SPI.HstPtrVal),
             DPxPTR(SPI.TgtPtrAddr), DPxPTR(SPI.TgtPtrVal));
          ObjectPtrs.push_back(SPI.TgtPtrVal);
          return OFFLOAD_SUCCESS;
        });
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
                              void *HstPtrBase, int32_t AllocOpt) {
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetDataAllocBegin(RTLDeviceID, Size));
  auto CorrID = XPTIRegistry->traceMemAllocBegin(Size, 0 /* GuardZone */);
#endif // INTEL_CUSTOMIZATION
  void *Ret = nullptr;
  if (RTL->data_alloc_base)
    Ret = RTL->data_alloc_base(RTLDeviceID, Size, HstPtrBegin, HstPtrBase,
                               AllocOpt);
  else
    Ret = RTL->data_alloc(RTLDeviceID, Size, HstPtrBegin, TARGET_ALLOC_DEFAULT);
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
                                  void *TgtNDLoopDesc, AsyncInfoTy &AsyncInfo) {
#if INTEL_CUSTOMIZATION
  OMPT_TRACE(targetSubmitBegin(RTLDeviceID, NumTeams));
#endif // INTEL_CUSTOMIZATION
  int32_t Ret =
      RTL->run_team_nd_region
          ? RTL->run_team_nd_region(RTLDeviceID, TgtEntryPtr, TgtVarsPtr,
                                    TgtOffsets, TgtVarsSize, NumTeams,
                                    ThreadLimit, TgtNDLoopDesc, AsyncInfo)
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

int32_t DeviceTy::requiresMapping(void *Ptr, int64_t Size) {
  if (RTL->requires_mapping)
    return RTL->requires_mapping(RTLDeviceID, Ptr, Size);
  else
    return 1;
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
  if (RTL->create_interop) {
    __tgt_interop * ret = RTL->create_interop(RTLDeviceID, InteropContext, NumPrefers,
                               PreferIDs);
    // common fields to all plugin
    ret->OwnerGtid = -1;
    ret->OwnerTask = NULL;
    ret->markDirty();
    return ret;
  } else
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

int32_t DeviceTy::queryAsync(AsyncInfoTy &AsyncInfo) {
  if (RTL->query_async)
    return RTL->query_async(RTLDeviceID, AsyncInfo);

  return synchronize(AsyncInfo);
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

#if INTEL_CUSTOMIZATION
int32_t DeviceTy::memcpyRect3D(void *Dst, const void *Src, size_t ElementSize,
                               int32_t NumDims, const size_t *Volume,
                               const size_t *DstOffsets,
                               const size_t *SrcOffsets, const size_t *DstDims,
                               const size_t *SrcDims) {
  if (RTL->memcpy_rect_3d)
    return RTL->memcpy_rect_3d(RTLDeviceID, Dst, Src, ElementSize, NumDims,
                               Volume, DstOffsets, SrcOffsets, DstDims,
                               SrcDims);
  return OFFLOAD_FAIL;
}
#endif // INTEL_CUSTOMIZATION
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

#if INTEL_CUSTOMIZATION
bool __tgt_interop::isCompatibleWith(
    int32_t InteropType, uint32_t NumPrefers, int32_t *PreferIDs,
    int64_t DeviceNum_, int GTID, void *CurrentTask) {
  if (DeviceNum != DeviceNum_)
    return false;

  if (InteropType != OMP_INTEROP_CONTEXT_TARGET && 
      InteropType != OMP_INTEROP_CONTEXT_TARGETSYNC)
    return false;

  if (InteropType == OMP_INTEROP_CONTEXT_TARGETSYNC && TargetSync == NULL)
    return false;

  if (NumPrefers > 0) {
    int FId;
    for (FId = 0; FId < NumPrefers; FId++)
      if (PreferIDs[FId] == FrId)
        break;
    if (FId == NumPrefers)
      return false;
  }

  if (GTID != OwnerGtid)
    return false;

#if 0
  // Task ownernship mode still not fully implemented
  if (CurrentTask != OwnerTask)
    return false;
#endif

  return true;
}

bool __tgt_interop::isOwnedBy(int GTID, void *CurrentTask) {
  if (GTID == OwnerGtid)
    return true;

#if 0
  // Task ownernship mode still not fully implemented
  if (CurrentTask != OwnerTask)
    return false;
#endif

  return false;
}

int32_t __tgt_interop::flush() {
  DeviceTy &Device = *PM->Devices[DeviceNum];
  if (Device.RTL->flush_queue)
    return Device.RTL->flush_queue(this);

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_interop::syncBarrier() {
  DeviceTy &Device = *PM->Devices[DeviceNum];
  if (Device.RTL->sync_barrier)
    return Device.RTL->sync_barrier(this);

  return OFFLOAD_FAIL;
}

int32_t __tgt_interop ::asyncBarrier() {
  DeviceTy &Device = *PM->Devices[DeviceNum];
  if (Device.RTL->async_barrier)
    return Device.RTL->async_barrier(this);

  return OFFLOAD_FAIL;
}

void InteropTblTy::clear() {
  DP("Clearing Interop Table\n");
  for(__tgt_interop *IOP : Interops) {
    IOP->flush();
    IOP->syncBarrier();
    PM->Devices[IOP->DeviceNum]->releaseInterop(IOP);
  }
  Interops.clear();
}
#endif
