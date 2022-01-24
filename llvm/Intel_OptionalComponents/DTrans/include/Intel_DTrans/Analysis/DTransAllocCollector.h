//===-----DTransAllocCollector.h - Allocation/Free function analyzer-------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides support for identifying user functions that wrap memory
// allocation and free calls.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/MemoryBuiltinsExtras.h"
#include "llvm/IR/Instructions.h"
#include <map>
#include <set>

#if !INTEL_FEATURE_SW_DTRANS
#error DTransAllocCollector.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSALLOCCOLLECTOR_H
#define INTEL_DTRANS_ANALYSIS_DTRANSALLOCCOLLECTOR_H

namespace llvm {

class TargetLibraryInfo;

namespace dtransOP {

class TypeMetadataReader;

class DTransAllocCollector {
public:
  DTransAllocCollector(
      TypeMetadataReader &MDReader,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : MDReader(MDReader), GetTLI(GetTLI) {}

  DTransAllocCollector(const DTransAllocCollector &) = delete;
  DTransAllocCollector(DTransAllocCollector &&) = delete;
  DTransAllocCollector &operator=(const DTransAllocCollector &) = delete;
  DTransAllocCollector &operator=(DTransAllocCollector &&) = delete;

  void populateAllocDeallocTable(const Module &M);

  dtrans::AllocKind getAllocFnKind(const CallBase *Call,
                                   const TargetLibraryInfo &TLI);
  dtrans::FreeKind getFreeFnKind(const CallBase *Call,
                                 const TargetLibraryInfo &TLI);

  bool isUserAllocOrDummyFunc(const CallBase *Call);
  bool isUserFreeOrDummyFunc(const CallBase *Call);

  static bool isDummyFuncWithThisAndIntArgs(const CallBase *Call,
                                            const TargetLibraryInfo &TLI,
                                            TypeMetadataReader &MDReader);

  static bool isDummyFuncWithThisAndInt8PtrArgs(const CallBase *Call,
                                                const TargetLibraryInfo &TLI,
                                                TypeMetadataReader &MDReader);

private:
  // An enum recording the status of a function. The status is
  // computed in populateAllocDeallocTable.
  // Note: The specific 'malloc'/'free' types in this enumeration have a 1-1
  // correspondence to the DTrans AllocKind/FreeKind enumerations.
  enum AllocStatus {
    AKS_Unknown,
    AKS_Malloc,
    AKS_Malloc0,
    AKS_MallocThis,
    AKS_Free,
    AKS_Free0,
    AKS_FreeThis
  };
  bool allocStatusIsMalloc(AllocStatus AS) {
    return AS >= AKS_Malloc && AS <= AKS_MallocThis;
  }

  bool allocStatusIsFree(AllocStatus AS) {
    return AS >= AKS_Free && AS <= AKS_FreeThis;
  }

  dtrans::AllocKind getOrAnalyzeAllocFnKind(const CallBase *Call,
                                            const TargetLibraryInfo &TLI);

  AllocStatus analyzeForMallocStatus(const Function *F);
  AllocStatus analyzeForFreeStatus(const Function *F);

  bool isMallocWithStoredMMPtr(const Function *F);
  bool isFreeWithStoredMMPtr(const Function *F);

  TypeMetadataReader &MDReader;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // Mapping for the AllocStatus of each Function we have identified as being a
  // user allocation or free function.
  std::map<const Function *, AllocStatus> AllocStatusMap;
};

} // namespace dtransOP
} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSALLOCCOLLECTOR_H
