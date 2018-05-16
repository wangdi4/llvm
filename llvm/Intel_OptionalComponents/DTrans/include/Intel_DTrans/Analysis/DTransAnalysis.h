//===----------------- DTransAnalysis.h - DTrans Analysis -----------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Data Transformation Analysis
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTrans.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSANALYSIS_H
#define INTEL_DTRANS_ANALYSIS_DTRANSANALYSIS_H

#include "Intel_DTrans/Analysis/DTrans.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

class TargetLibraryInfo;

namespace dtrans {
// This structure is used to describe the affected portion of an aggregate type
// passed as an argument of the memfunc call. This will be used to communicate
// information collected during the analysis to the transforms about how
// a memfunc call is impacting a structure.
struct MemfuncRegion {
  MemfuncRegion() : IsCompleteAggregate(true), FirstField(0), LastField(0) {}

  // If this is 'false', the FirstField and LastField members must be set
  // to indicate an inclusive set of fields within the structure that are
  // affected. If this is 'true', the FieldField and LastField member values
  // are undefined.
  bool IsCompleteAggregate;

  // If the region is a description of a partial structure modification, these
  // members specify the first and last fields touched.
  unsigned int FirstField;
  unsigned int LastField;
};
} // end namespace dtrans

class DTransAnalysisInfo {
public:
  /// Adaptor for directly iterating over the dtrans::TypeInfo pointers.
  struct type_info_iterator
      : public iterator_adaptor_base<
            type_info_iterator, DenseMap<Type *, dtrans::TypeInfo *>::iterator,
            std::forward_iterator_tag, dtrans::TypeInfo *> {
    explicit type_info_iterator(
        DenseMap<Type *, dtrans::TypeInfo *>::iterator X)
        : iterator_adaptor_base(X) {}

    dtrans::TypeInfo *&operator*() const { return I->second; }
    dtrans::TypeInfo *&operator->() const { return operator*(); }
  };

  using CallInfoMapType = DenseMap<llvm::Instruction *, dtrans::CallInfo *>;

  /// Adaptor for directly iterating over the dtrans::CallInfo pointers.
  struct call_info_iterator
      : public iterator_adaptor_base<
            call_info_iterator, CallInfoMapType::iterator,
            std::forward_iterator_tag, CallInfoMapType::value_type> {
    explicit call_info_iterator(CallInfoMapType::iterator X)
        : iterator_adaptor_base(X) {}

    dtrans::CallInfo *&operator*() const { return I->second; }
    dtrans::CallInfo *&operator->() const { return operator*(); }
  };

public:
  DTransAnalysisInfo();
  DTransAnalysisInfo(DTransAnalysisInfo&&) = default;
  ~DTransAnalysisInfo();

  DTransAnalysisInfo(const DTransAnalysisInfo&) = delete;
  DTransAnalysisInfo& operator=(const DTransAnalysisInfo&) = delete;

  DTransAnalysisInfo& operator=(DTransAnalysisInfo&&);

  bool analyzeModule(Module &M, TargetLibraryInfo &TLI);
  void reset();

  /// Return true if we are interested in tracking values of the specified type.
  bool isTypeOfInterest(llvm::Type *Ty);

  /// Retrieve the DTrans type information entry for an array of element of
  /// the specified LLVM type.  An LLVM array type is generated if necessary.
  /// If there is no entry for the specified type, create one.
  dtrans::TypeInfo *getOrCreateTypeInfoForArray(llvm::Type *Ty, uint64_t Num);

  /// Retrieve the DTrans type information entry for the specified LLVM type.
  /// If there is no entry for the specified type, create one.
  dtrans::TypeInfo *getOrCreateTypeInfo(llvm::Type *Ty);

  /// Retrieve the DTrans type information entry for the specified LLVM type.
  /// If there is no entry for the specified type, return nullptr.
  dtrans::TypeInfo *getTypeInfo(llvm::Type *Ty) const;

  iterator_range<type_info_iterator> type_info_entries() {
    return make_range(type_info_iterator(TypeInfoMap.begin()),
                      type_info_iterator(TypeInfoMap.end()));
  }

  iterator_range<call_info_iterator> call_info_entries() {
    return make_range(call_info_iterator(CallInfoMap.begin()),
                      call_info_iterator(CallInfoMap.end()));
  }

  // Retrieve the CallInfo object for the instruction, if information exists.
  // Otherwise, return nullptr.
  dtrans::CallInfo *getCallInfo(Instruction *I);

  // Create an entry in the CallInfoMap about a memory allocation call.
  dtrans::AllocCallInfo *createAllocCallInfo(Instruction *I,
                                             dtrans::AllocKind AK);

  dtrans::FreeCallInfo *createFreeCallInfo(Instruction *I, dtrans::FreeKind FK);

#if 0
  // These will be enabled in a subsequent changeset when
  // MemfuncCallInfo is added.

  dtrans::MemfuncCallInfo *createMemfuncCallInfo(Instruction *I,
    dtrans::MemfuncCallInfo::MemfuncKind MK,
    dtrans::MemfuncRegion &MR);
  dtrans::MemfuncCallInfo *createMemfuncCallInfo(Instruction *I,
    dtrans::MemfuncCallInfo::MemfuncKind MK,
    dtrans::MemfuncRegion &MR1,
    dtrans::MemfuncRegion &MR2);
#endif

  // Destroy the CallInfo stored about the specific instruction.
  void deleteCallInfo(Instruction *I);

  // Update the instruction associated with the CallInfo object. This
  // is necessary because when a function is cloned during the DTrans
  // optimizations, the information needs to be transferred to the
  // newly created instruction of the cloned routine.
  void replaceCallInfoInstruction(dtrans::CallInfo *Info, Instruction *NewI);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printCallInfo();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  void printStructInfo(dtrans::StructInfo *AI);
  void printArrayInfo(dtrans::ArrayInfo *AI);
  void printFieldInfo(dtrans::FieldInfo &FI);

  void addCallInfo(llvm::Instruction *I, dtrans::CallInfo *Info);
  void destructCallInfo(dtrans::CallInfo *Info);

  DenseMap<llvm::Type *, dtrans::TypeInfo *> TypeInfoMap;

  // A mapping from function calls that special information is collected for
  // (malloc, free, memset, etc) to the information stored about those calls.
  CallInfoMapType CallInfoMap;
};

// Analysis pass providing a data transformation analysis result.
class DTransAnalysis : public AnalysisInfoMixin<DTransAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<DTransAnalysis>;
  static char PassID;

public:
  typedef DTransAnalysisInfo Result;

  Result run(Module &M, AnalysisManager<Module> &AM);
};

// Legacy wrapper pass to provide DTrans analysis.
class DTransAnalysisWrapper : public ModulePass {
private:
  DTransAnalysisInfo Result;

public:
  static char ID;

  DTransAnalysisWrapper();

  DTransAnalysisInfo &getDTransInfo() { return Result; }

  bool runOnModule(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

ModulePass *createDTransAnalysisWrapperPass();

} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSANALYSIS_H
