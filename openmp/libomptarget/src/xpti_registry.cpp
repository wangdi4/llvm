#if INTEL_CUSTOMIZATION
//===----------- xpti_registry.cpp - XPTI instrumentation -----------------===//
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "xpti_registry.h"
#include "private.h"

#ifdef XPTI_ENABLE_INSTRUMENTATION
#include "xpti/xpti_trace_framework.h"
#endif
uint8_t OMPStreamID;
XPTIRegistryTy *XPTIRegistry;

void XPTIRegistryTy::pushCodeLocation(const char *Loc) {
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (!xptiTraceEnabled())
    return;

  int32_t GTID = __kmpc_global_thread_num(nullptr);
  std::lock_guard<std::mutex> Lock(Mtx);
  CodeLocations[GTID] = Loc;
#endif
}

void XPTIRegistryTy::pushEvent(ident_t *Loc, const char *EntryName) {
  (void)Loc;
  (void)EntryName;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (!xptiTraceEnabled())
    return;

  // Compose event key with entry name and source location
  std::string Name(EntryName);
#if 0
  // TODO: use this when Loc contains correct information
  SourceInfo CodeLoc(Loc);
#else
  int32_t GTID = __kmpc_global_thread_num(nullptr);
  Mtx.lock();
  const char *CodeLocStr = CodeLocations[GTID];
  Mtx.unlock();
  ident_t Idnt = {0, 0, 0, 0, CodeLocStr};
  SourceInfo CodeLoc(&Idnt);
#endif
  Name += std::string(":") + CodeLoc.getName();
  Name += std::string(":") + std::to_string(CodeLoc.getLine());
  Name += std::string(":") + std::to_string(CodeLoc.getColumn());

  xpti::payload_t PayLoad(Name.c_str(), CodeLoc.getFilename(),
                          CodeLoc.getLine(), CodeLoc.getColumn(), nullptr);

  uint64_t IId;
  xpti::trace_event_data_t *TraceEvent =
      xptiMakeEvent(Name.c_str(), &PayLoad, xpti::trace_algorithm_event,
                    xpti_at::active, &IId);

  std::lock_guard<std::mutex> Lock(Mtx);
  TraceEvents[GTID] = TraceEvent;
#endif
}

#ifdef XPTI_ENABLE_INSTRUMENTATION
xpti::trace_event_data_t *XPTIRegistryTy::getEvent(void) {
  if (!xptiTraceEnabled())
    return nullptr;

  int32_t GTID = __kmpc_global_thread_num(nullptr);
  std::lock_guard<std::mutex> Lock(Mtx);
  if (TraceEvents.count(GTID) > 0)
    return TraceEvents.at(GTID);
  else
    return nullptr;
}
#endif

void XPTIRegistryTy::popEvent(void) {
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (!xptiTraceEnabled())
    return;

  int32_t GTID = __kmpc_global_thread_num(nullptr);
  std::lock_guard<std::mutex> Lock(Mtx);
  TraceEvents.erase(GTID);
#endif
}

uint64_t XPTIRegistryTy::traceMemAllocBegin(
    size_t AllocSize, size_t GuardZone) {
  (void)AllocSize;
  (void)GuardZone;
  uint64_t CorrID = 0;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (xptiTraceEnabled()) {
    xpti::mem_alloc_data_t MemAlloc{0 /* ObjHandle */, 0 /* AllocPtr */,
                                    AllocSize, GuardZone};
    // XPTIRegistry->getEvent() returns the event created for the latest active
    // target region, and it is notified as a parent event of this memory
    // allocation. Parent event can be null for memory allocation by API.
    CorrID = xptiGetUniqueId();
    xptiNotifySubscribers(
        OMPStreamID,
        static_cast<uint16_t>(xpti::trace_point_type_t::mem_alloc_begin),
        XPTIRegistry->getEvent(), nullptr, CorrID, &MemAlloc);
  }
#endif
  return CorrID;
}

void XPTIRegistryTy::traceMemAllocEnd(
    uintptr_t AllocPtr, size_t AllocSize, size_t GuardZone,
    uint64_t CorrelationID) {
  (void)AllocPtr;
  (void)AllocSize;
  (void)GuardZone;
  (void)CorrelationID;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (xptiTraceEnabled()) {
    xpti::mem_alloc_data_t MemAlloc{0 /* ObjHandle */, AllocPtr, AllocSize,
                                    GuardZone};
    xptiNotifySubscribers(
        OMPStreamID,
        static_cast<uint16_t>(xpti::trace_point_type_t::mem_alloc_end),
        XPTIRegistry->getEvent(), nullptr, CorrelationID, &MemAlloc);
  }
#endif
}

uint64_t XPTIRegistryTy::traceMemReleaseBegin(uintptr_t AllocPtr) {
  (void)AllocPtr;
  uint64_t CorrID = 0;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (xptiTraceEnabled()) {
    xpti::mem_alloc_data_t MemAlloc{0 /* ObjHandle */, AllocPtr, 0, 0};
    CorrID = xptiGetUniqueId();
    xptiNotifySubscribers(
        OMPStreamID,
        static_cast<uint16_t>(xpti::trace_point_type_t::mem_release_begin),
        XPTIRegistry->getEvent(), nullptr, CorrID, &MemAlloc);
  }
#endif
  return CorrID;
}

void XPTIRegistryTy::traceMemReleaseEnd(
    uintptr_t AllocPtr, uint64_t CorrelationID) {
  (void)AllocPtr;
  (void)CorrelationID;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (xptiTraceEnabled()) {
    xpti::mem_alloc_data_t MemAlloc{0, AllocPtr, 0, 0};
    xptiNotifySubscribers(
        OMPStreamID,
        static_cast<uint16_t>(xpti::trace_point_type_t::mem_release_end),
        XPTIRegistry->getEvent(), nullptr, CorrelationID, &MemAlloc);
  }
#endif
}

void XPTIRegistryTy::traceMemAssociate(uintptr_t HostPtr, uintptr_t TargetPtr) {
  (void)HostPtr;
  (void)TargetPtr;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (xptiTraceEnabled()) {
    uint64_t CorrID = xptiGetUniqueId();
    xpti::offload_association_data_t Map{HostPtr, TargetPtr};
    xptiNotifySubscribers(
        OMPStreamID,
        xpti::trace_offload_alloc_memory_object_associate,
        XPTIRegistry->getEvent(), nullptr, CorrID, &Map);
  }
#endif
}

#endif // INTEL_CUSTOMIZATION
