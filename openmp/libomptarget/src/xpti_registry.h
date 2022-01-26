#if INTEL_CUSTOMIZATION
//===----------- xpti_registry.h - XPTI instrumentation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <mutex>
#include <string>
#include <unordered_set>
#include <SourceInfo.h>

#ifdef XPTI_ENABLE_INSTRUMENTATION
#include "xpti/xpti_trace_framework.h"
#endif

/// For now, use this stream name for all
constexpr const char *OPENMP_OFFLOAD_STREAM_NAME = "openmp.offload";

class XPTIRegistryTy;
extern uint8_t OMPStreamID;
extern XPTIRegistryTy *XPTIRegistry;

class XPTIRegistryTy {
  std::unordered_set<std::string> ActiveStreams;
  std::once_flag Initialized;
  std::mutex Mtx;
#ifdef XPTI_ENABLE_INSTRUMENTATION
  std::unordered_map<int32_t, xpti::trace_event_data_t *> TraceEvents;
  std::unordered_map<int32_t, const char *> CodeLocations;
#endif

public:
  void initializeFrameworkOnce() {
#ifdef XPTI_ENABLE_INSTRUMENTATION
    std::call_once(Initialized, [this] {
      xptiFrameworkInitialize();
      OMPStreamID = xptiRegisterStream(OPENMP_OFFLOAD_STREAM_NAME);
      this->initializeStream(OPENMP_OFFLOAD_STREAM_NAME, 0, 1, "0.1");
    });
#endif
  }

  void initializeStream(const std::string &StreamName, uint32_t MajVer,
                        uint32_t MinVer, const std::string &VerStr) {
#ifdef XPTI_ENABLE_INSTRUMENTATION
    ActiveStreams.insert(StreamName);
    xptiInitialize(StreamName.c_str(), MajVer, MinVer, VerStr.c_str());
#endif
  }

  ~XPTIRegistryTy() {
#ifdef XPTI_ENABLE_INSTRUMENTATION
    for (const auto &StreamName : ActiveStreams) {
      xptiFinalize(StreamName.c_str());
    }
    xptiFrameworkFinalize();
#endif
  }

  void pushEvent(ident_t *Loc, const char *EntryName);
  void popEvent(void);
  void pushCodeLocation(const char *Loc);
#ifdef XPTI_ENABLE_INSTRUMENTATION
  xpti::trace_event_data_t *getEvent(void);
#endif

  static uint64_t traceMemAllocBegin(size_t AllocSize, size_t GuardZone);
  static void traceMemAllocEnd(uintptr_t AllocPtr, size_t AllocSize,
                               size_t GuardZone, uint64_t CorrelationID);
  static uint64_t traceMemReleaseBegin(uintptr_t AllocPtr);
  static void traceMemReleaseEnd(uintptr_t AllocPtr, uint64_t CorrelationID);
  static void traceMemAssociate(uintptr_t UserPtr, uintptr_t AllocPtr);
};

class XPTIEventCacheTy {
public:
  XPTIEventCacheTy(ident_t *Loc, const char *EntryName) {
    XPTIRegistry->pushEvent(Loc, EntryName);
  }

  ~XPTIEventCacheTy() {
    XPTIRegistry->popEvent();
  }
};

#endif // INTEL_CUSTOMIZATION
