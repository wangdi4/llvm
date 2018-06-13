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
#include "llvm/IR/CallSite.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

namespace llvm {

class BinaryOperator;

class TargetLibraryInfo;
class GetElementPtrInst;

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

  using PtrSubInfoMapType = ValueMap<Value *, llvm::Type *>;

  using ByteFlattenedGEPInfoMapType = ValueMap<Value *,
                                               std::pair<llvm::Type*, size_t>>;

public:
  DTransAnalysisInfo();
  DTransAnalysisInfo(DTransAnalysisInfo&& Other);
  ~DTransAnalysisInfo();

  DTransAnalysisInfo(const DTransAnalysisInfo &) = delete;
  DTransAnalysisInfo &operator=(const DTransAnalysisInfo &) = delete;

  DTransAnalysisInfo &operator=(DTransAnalysisInfo &&);

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

  // If the specified BinaryOperator was identified as a subtraction of
  // pointers to a type of interest, return the type that is pointed to
  // by the pointers being subtracted. Otherwise, return nullptr.
  llvm::Type *getResolvedPtrSubType(BinaryOperator *BinOp);

  // If the specified GEP was identified as a byte flattened access of
  // a structure element, return the type-index pair for the element accessed.
  // Otherwise, return (nullptr, 0).
  std::pair<llvm::Type *, size_t>
  getByteFlattenedGEPElement(GetElementPtrInst *GEP);

  // Retrieve the CallInfo object for the instruction, if information exists.
  // Otherwise, return nullptr.
  dtrans::CallInfo *getCallInfo(Instruction *I);

  // Create an entry in the CallInfoMap about a memory allocation call.
  dtrans::AllocCallInfo *createAllocCallInfo(Instruction *I,
                                             dtrans::AllocKind AK);

  // Create an entry in the CallInfoMap about a memory freeing call
  dtrans::FreeCallInfo *createFreeCallInfo(Instruction *I, dtrans::FreeKind FK);

  // Create an entry in the CallInfoMap about a memory setting/copying/moving
  // call.
  dtrans::MemfuncCallInfo *
  createMemfuncCallInfo(Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
                        dtrans::MemfuncRegion &MR);

  dtrans::MemfuncCallInfo *
  createMemfuncCallInfo(Instruction *I, dtrans::MemfuncCallInfo::MemfuncKind MK,
                        dtrans::MemfuncRegion &MR1, dtrans::MemfuncRegion &MR2);

  // Destroy the CallInfo stored about the specific instruction.
  void deleteCallInfo(Instruction *I);

  // Update the instruction associated with the CallInfo object. This
  // is necessary because when a function is cloned during the DTrans
  // optimizations, the information needs to be transferred to the
  // newly created instruction of the cloned routine.
  void replaceCallInfoInstruction(dtrans::CallInfo *Info, Instruction *NewI);

  // Interface routine to get possible targets of given function pointer 'FP'.
  // It computes all possible targets of 'FP' using field single value analysis
  // and adds valid targets to 'Targets' vector. It skips adding unknown/
  // invalid targets to 'Targets' vector and returns false if there are any
  // unknown/invalid targets.
  //
  bool GetFuncPointerPossibleTargets(llvm::Value *FP,
                                     std::vector<llvm::Value *> &Targets,
                                     llvm::CallSite, bool);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printCallInfo(raw_ostream &OS);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  void addPtrSubMapping(llvm::BinaryOperator *BinOp, llvm::Type *Ty);

  void addByteFlattenedGEPMapping(GetElementPtrInst *GEP,
                                  std::pair<llvm::Type*, size_t> Pointee);
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

  // A mapping from BinaryOperator instructions that have been identified as
  // subtracting two pointers to types of interest to the interesting type
  // aliased by the operands.
  PtrSubInfoMapType PtrSubInfoMap;

  // A mapping from GetElementPtr instructions that have been identified as
  // being structure element accesses in byte-flattened form to a type-index
  // pair for the element being accessed.
  ByteFlattenedGEPInfoMapType ByteFlattenedGEPInfoMap;
};

// Analysis pass providing a data transformation analysis result.
class DTransAnalysis : public AnalysisInfoMixin<DTransAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<DTransAnalysis>;
  static char PassID;

public:
  typedef DTransAnalysisInfo Result;

  Result run(Module &M, ModuleAnalysisManager &AM);
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
