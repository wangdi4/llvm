//===-------------- ReorderFieldsOP.cpp - DTransReorderFieldsPass ---------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Field Reorder (DFR) optimization pass for
// opaque pointers.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ReorderFieldsOP.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOPOptBase.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#include <map>

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-reorderfieldsop"

// Flag to enable/disable DTransOP::ReorderField legality tests.
// Default is on (enabled).
static cl::opt<bool>
    EnableReorderFieldOPLegalTest("dtrans-reorderfieldop-enable-legal-test",
                                  cl::init(true), cl::ReallyHidden);

// Flag to enable/disable DTrans::ReorderField applicablility tests.
// Default is on (enabled).
static cl::opt<bool> EnableReorderFieldOPApplicableTest(
    "dtrans-reorderfieldop-enable-applicable-test", cl::init(true),
    cl::ReallyHidden);

// Flag to enable/disable DTrans::ReorderField profitability tests.
// Default is on (enabled).
static cl::opt<bool> EnableReorderFieldOPProfitableTest(
    "dtrans-reorderfieldop-enable-profitable-test", cl::init(true),
    cl::ReallyHidden);

// Flag to enable/disable all DTransOP::ReorderField tests, including legality,
// applicablility, and profitability tests. Default is on (enabled).
static cl::opt<bool>
    EnableReorderFieldOPTests("dtrans-reorderfieldop-enable-tests",
                              cl::init(true), cl::ReallyHidden);

// Field reordering may not help to utilize all unused space in the original
// structure layout. This flag is used to avoid applying reordering
// transformation if reordering is not profitable. Controls minimum saved space
// percent threshold from field reordering to enable the transformation.
static cl::opt<unsigned> DTransReorderFieldsOPSavedSpacePercentThreshold(
    "dtrans-reorder-fieldsop-saved-space-percent-threshold", cl::init(13),
    cl::ReallyHidden);

// Percent threshold used for non-simple struct.
// Otherise same as DTransReorderFieldsSavedSpacePercentThreshold.
static cl::opt<unsigned> DTransReorderFieldsOPSavedSpacePercentComplexThreshold(
    "dtrans-reorder-fieldsop-saved-space-percent-complex-threshold",
    cl::init(7), cl::ReallyHidden);

// Max number of fields in a struct that may be considered SIMPLE.
static cl::opt<unsigned> DTransReorderFieldsOPNumFieldsThreshold(
    "dtrans-reorder-fieldsop-numfields-threshold", cl::init(15),
    cl::ReallyHidden);

// Number of fields in a struct to be considered as Advanced StructType
static cl::opt<unsigned> DTransReorderFieldsOPNumFieldsAdvPrecise(
    "dtrans-reorder-fieldsop-numfields-adv-precise", cl::init(20),
    cl::ReallyHidden);

// Begin index for 1st pair of fields to swap, in order to relocate key fields
// to the beginning of the struct.
static cl::opt<unsigned> DTransReorderFieldsOPSwapIndex1Begin(
    "dtrans-reorder-fieldsop-swapidx1-begin", cl::init(3), cl::ReallyHidden);

// End index for 1st pair of fields to swap, in order to relocate key fields
// to the end of the struct.
static cl::opt<unsigned>
    DTransReorderFieldsOPSwapIndex1End("dtrans-reorder-fieldsop-swapidx1-end",
                                       cl::init(18), cl::ReallyHidden);

// Begin index for 2nd pair of fields to swap, in order to relocate key fields
// to the beginning of the struct.
static cl::opt<unsigned> DTransReorderFieldsOPSwapIndex2Begin(
    "dtrans-reorder-fieldsop-swapidx2-begin", cl::init(4), cl::ReallyHidden);

// End index for 2nd pair of fields to swap, in order to relocate key fields
// to the end of the struct.
static cl::opt<unsigned>
    DTransReorderFieldsOPSwapIndex2End("dtrans-reorder-fieldsop-swapidx2-end",
                                       cl::init(5), cl::ReallyHidden);

// Expected number of integer-type fields within the target struct type
static cl::opt<unsigned> DTransReorderFieldsOPComplexStructIntTypeCount(
    "dtrans-reorder-fieldsop-complexstruct-int-counts", cl::init(12),
    cl::ReallyHidden);

// Expected number of struct-type fields within the target struct type
static cl::opt<unsigned> DTransReorderFieldsOPComplexStructStructTypeCount(
    "dtrans-reorder-fieldsop-complexstruct-struct-counts", cl::init(5),
    cl::ReallyHidden);

// Expected number of pointer-type fields within the target struct type
static cl::opt<unsigned> DTransReorderFieldsOPComplexStructPionterTypeCount(
    "dtrans-reorder-fieldsop-complexstruct-ptr-counts", cl::init(3),
    cl::ReallyHidden);

namespace {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD StringRef getStName(StructType *StTy) {
  return StTy->hasName() ? StTy->getName() : "<unnamed struct>";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

static bool getConstantFromValue(Value *V, uint64_t &Val) {
  auto *CInt = dyn_cast<ConstantInt>(V);
  if (!CInt)
    return false;

  Val = CInt->getValue().getZExtValue();

  return true;
}

class DTransReorderFieldsOPWrapper : public ModulePass {
public:
  static char ID;

  DTransReorderFieldsOPWrapper() : ModulePass(ID) {
    initializeDTransReorderFieldsOPWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    DTransSafetyAnalyzerWrapper &DTAnalysisWrapper =
        getAnalysis<DTransSafetyAnalyzerWrapper>();
    DTransSafetyInfo &DTInfo = DTAnalysisWrapper.getDTransSafetyInfo(M);
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    bool Changed = Impl.runImpl(
        M, &DTInfo, GetTLI, getAnalysis<WholeProgramWrapperPass>().getResult());
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransSafetyAnalyzerWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }

private:
  ReorderFieldsOPPass Impl;
};

// FieldData records a field's alignment, size, index, and access frequency.
class FieldData {
  uint64_t Align = 0;
  uint64_t Size = 0;
  uint64_t Index = 0;
  uint64_t Frequency = 0;

public:
  FieldData(uint64_t Align, uint64_t Size, uint64_t Index, uint64_t Frequency)
      : Align(Align), Size(Size), Index(Index), Frequency(Frequency) {}

  uint64_t getAlign() const { return Align; }
  uint64_t getSize() const { return Size; }
  uint64_t getIndex() const { return Index; }
  uint64_t getFrequency() const { return Frequency; }
  void set(uint64_t NewAlign, uint64_t NewSize, uint64_t NewIndex,
           uint64_t NewFrequency) {
    Align = NewAlign;
    Size = NewSize;
    Index = NewIndex;
    Frequency = NewFrequency;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump(bool SimpleMode = true) {
    if (SimpleMode)
      dbgs() << Align << ",\t" << Size << ",\t" << Index << ",\t" << Frequency
             << "\n";
    else
      dbgs() << "Align: " << Align << ",\t Size: " << Size
             << ",\tIndex: " << Index << ",\tFrequency: " << Frequency << "\n";
  }
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

// The ReorderFieldTransInfo class has all the required information to do
// DTrans::ReorderField transformation for candidate structs.
class ReorderFieldTransInfo {
private:
  // Map of <Reordered_Struct, Vector of new indexes>
  DenseMap<llvm::StructType *, SmallVector<uint32_t, 8>> TransformedIndexes;

  // Map of <Reordered_Struct, Size of Reordered Struct>
  DenseMap<llvm::StructType *, uint64_t> TransformedTypeNewSizeMap;

  SmallPtrSet<dtransOP::DTransStructType *, 4> TransformedTypes;

  // Collection of module-level Inclusive Struct Types
  //
  // [Inclusive Struct Type]
  // Consider an inheritance case: class B: public A { ... }
  //
  // This is reflected on a type system as:
  // struct A {...};
  // struct B { struct A a; ...}
  //
  // Since struct B type includes a complete copy of struct A type, B is called
  // an inclusive struct type of A.
  // Any type that is directly or indirectly derived from struct A type is also
  // an inclusive struct type of A.
  //
  std::vector<llvm::StructType *> InclusiveStructTypeV;

  // Type map for InclusiveStructType
  // E.g.
  // Src Type        |   Dst Type
  // %class.cPacket  ->  %__DFR_class.cPacket
  // ...
  std::map<llvm::StructType *, llvm::StructType *> InclusiveStructTypeMap;

  // Type (un)map for InclusiveStructType
  // E.g.
  // Dst Type              |  Src Type
  // %__DFR_class.cPacket  -> %class.cPacket
  // ...
  std::map<llvm::StructType *, llvm::StructType *> InclusiveStructTypeUnmap;
  std::map<Function *, bool> InclusiveStructTypeMapped;

public:
  // Set new size of \p StructT after reordering fields.
  void setTransformedTypeNewSize(DTransStructType *StructT, uint64_t NewSize) {
    TransformedTypes.insert(StructT);
    TransformedTypeNewSizeMap[cast<llvm::StructType>(StructT->getLLVMType())] =
        NewSize;
  }

  // Set \p NewIdx as new index for a field of \p StructT at \p OldIdx.
  void setTransformedIndexes(llvm::StructType *StructT,
                             std::vector<uint32_t> &IdxVec) {
    for (auto Idx : IdxVec)
      TransformedIndexes[StructT].push_back(Idx);
  }

  // Return true if there is any type that has been mapped for its
  // reordered-type's size
  bool hasAnyTypeTransformed(void) { return (!TransformedTypes.empty()); }

  // Return new size of \p StructT after reordering fields.
  uint64_t getTransformedTypeNewSize(llvm::StructType *StructT) {
    assert(TransformedTypeNewSizeMap.count(StructT) == 1 &&
           "Struct is not in the transformed list");
    return TransformedTypeNewSizeMap[StructT];
  }

  // Return true if StructT is a struct type with field reordering.
  //
  // TransformedTypeNewSizeMap is a map: StructType -> uint64_t.
  // Since a struct type can only have 1 size,
  // TransformedTypeNewSizeMap.count(Ty) == 1 means that Type ty has a valid
  // entry in the map.
  //
  bool hasTransformedTypeNewSize(llvm::StructType *StructT) {
    return (TransformedTypeNewSizeMap.count(StructT) == 1);
  }

  // Return new index of a field of \p StructT at \p OldIdx index
  // after reordering fields.
  uint32_t getTransformedIndex(llvm::StructType *StructT, uint32_t OldIdx) {
    auto Itr = TransformedIndexes.find(StructT);
    assert(Itr != TransformedIndexes.end() &&
           "Struct type is not in the transformed list");
    return Itr->second[OldIdx];
  }

  SmallPtrSetImpl<DTransStructType *> &getTypesMap() {
    return TransformedTypes;
  }

  std::vector<llvm::StructType *> &getInclusiveStructTypes() {
    return InclusiveStructTypeV;
  }

  // Return true if InclusiveStructType vector is not empty
  bool hasInclusiveStructType() { return !InclusiveStructTypeV.empty(); }

  // Return true if a given StructType* is an Inclusive Struct Type. See comment
  // on the routine findInclusiveStructType() for a description of how Inclusive
  // Struct Types are found.
  bool hasInclusiveStructType(llvm::StructType *StTy) {
    return std::find(InclusiveStructTypeV.begin(), InclusiveStructTypeV.end(),
                     StTy) != InclusiveStructTypeV.end();
  }

  unsigned getInclusiveStructTypeSize() { return InclusiveStructTypeV.size(); }

  // Return true if InclusiveStructTypeMap has a valid entry for OrigTy*
  bool hasInclusiveStructOrigType(llvm::StructType *OrigTy) {
    return (InclusiveStructTypeMap.count(OrigTy) == 1);
  }

  // Return true if InclusiveStructTypeUnmap has a valid entry for ReorderedTy*
  bool hasInclusiveStructReorderedType(llvm::StructType *ReorderedTy) {
    return (InclusiveStructTypeUnmap.count(ReorderedTy) == 1);
  }

  bool isInclusiveStructTypeMapped(Function *F) {
    return InclusiveStructTypeMapped[F];
  }

  bool doInclusiveStructTypeMap(DTransOPTypeRemapper *TypeRemapper,
                                Function *F);

  // Map: OrigTy -> ReorderTy
  llvm::StructType *mapInclusiveStructType(llvm::StructType *OrigTy) {
    return InclusiveStructTypeMap[OrigTy];
  }

  // (Un)Map: ReorderTy -> OrigTy
  llvm::StructType *unmapInclusiveStructType(llvm::StructType *ReorderTy) {
    return InclusiveStructTypeUnmap[ReorderTy];
  }

  // Given a valid InclusiveStructType, obtain its base StructType that is
  // a valid DTrans's Field-Reorder (DFR) candidate.
  llvm::StructType *
  getDFRCandidateStructType(llvm::StructType *OrigInclusiveStructTy) {
    // Expect OrigInclusiveStructTy to be a valid Inclusive Struct Type
    if (!hasInclusiveStructType(OrigInclusiveStructTy))
      return nullptr;

    llvm::StructType *StructTy = OrigInclusiveStructTy;

    while (true) {
      StructType *CandidateStTy =
          dyn_cast<StructType>(StructTy->getElementType(0));
      if (!CandidateStTy)
        break;

      if (TransformedTypeNewSizeMap.find(CandidateStTy) !=
          TransformedTypeNewSizeMap.end())
        return CandidateStTy;

      StructTy = CandidateStTy;
    }

    return nullptr;
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpMappedIndex() const;
  void dumpMappedSize() const;
  void dumpInclusiveStructTypes() const;
  void dump() const;
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

class ReorderFieldsAnalyzer {
public:
  // Limit size of structs that are selected for reordering fields based
  // on padding heuristic. Field affinity info may be needed to apply
  // reordering for structs that don't fit cache Line.
  // "TargetTransformInfo::getCacheLineSize" can be used instead MaxStructSize
  // after doing more experiments.
  static const unsigned MaxStructSize = 160;

  // Minimum number of fields to select a struct as candidate.
  static const unsigned MinNumElems = 3;

  // Maximum number of fields to select a struct as candidate.
  static const unsigned MaxNumElems = 20;

  // number of key fields:
  static const unsigned NumKeyFields = 4;

  bool doCollection(Module &M, DTransSafetyInfo *DTInfo);
  ReorderFieldTransInfo &getRTI() { return RTI; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const {
    dumpCandidateTypes();
    dumpRTI();
  }

  void dumpCandidateTypes() const {
    dbgs() << "Total Candidates: " << CandidateTypeV.size() << "\n";
    unsigned Count = 0;
    for (auto *StInfo : CandidateTypeV) {
      dbgs() << Count++ << " : ";
      StInfo->getLLVMType()->dump();
    }
  }

  void dumpRTI() const { RTI.dump(); }
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private:
  // Collection of suitable StructInfo* types for field reordering
  SmallVector<dtrans::StructInfo *, 4> CandidateTypeV;

  // Reorder-field transformation Information
  ReorderFieldTransInfo RTI;

  bool isLegal(dtrans::TypeInfo *TI, DTransSafetyInfo *DTInfo);
  bool isApplicable(dtrans::TypeInfo *TI, const DataLayout &DL);
  bool isProfitable(dtrans::TypeInfo *TI, const DataLayout &DL);
};

// map Inclusive Struct Type(s) both ways
bool ReorderFieldTransInfo::doInclusiveStructTypeMap(
    DTransOPTypeRemapper *TypeRemapper, Function *F) {
  // Ensure: map ONCE only for F
  if (InclusiveStructTypeMapped[F])
    return true;

  // Do the mapping
  for (llvm::StructType *OrigTy : InclusiveStructTypeV) {
    // Skip the OrigTy if it is already mapped
    if (InclusiveStructTypeMap[OrigTy])
      continue;

    llvm::StructType *ReorderedTy =
        dyn_cast<llvm::StructType>(TypeRemapper->lookupTypeMapping(OrigTy));

    if (ReorderedTy) {
      LLVM_DEBUG(dbgs() << "OrigTy: " << *OrigTy
                        << "\nReorderTy: " << *ReorderedTy << "\n";);
      InclusiveStructTypeMap[OrigTy] = ReorderedTy;
      InclusiveStructTypeUnmap[ReorderedTy] = OrigTy;
    }

    // [Note]
    // Failed to find a mapping is not an error.
    // It can simply be because the said inclusive struct type is not yet
    // mapped, since mapping happens on a per-function basis.
    //
    // E.g.
    // A relevant function that uses the inclusive struct type is not yet
    // cloned.
    //
  }

  // Mark that the InclusiveStructType is now mapped for F
  InclusiveStructTypeMapped[F] = true;
  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void ReorderFieldTransInfo::dumpMappedIndex() const {
  for (auto &Pair : TransformedIndexes) {
    StructType *ST = Pair.first;
    auto &IndexV = Pair.second;
    dbgs() << getStName(ST) << " : <" << IndexV.size() << "> ";
    for (auto V : IndexV)
      dbgs() << V << ", ";
    dbgs() << "\n";
  }
}

LLVM_DUMP_METHOD void ReorderFieldTransInfo::dumpMappedSize() const {
  for (auto &Pair : TransformedTypeNewSizeMap) {
    StructType *ST = Pair.first;
    uint64_t NewSize = Pair.second;
    dbgs() << getStName(ST) << " : new size: " << NewSize << "\n";
  }
}

void ReorderFieldTransInfo::dumpInclusiveStructTypes() const {
  dbgs() << "Total InclusiveStructType(s): " << InclusiveStructTypeV.size()
         << "\n";
  unsigned Count = 0;
  for (auto *Ty : InclusiveStructTypeV) {
    dbgs() << Count++ << " : ";
    Ty->dump();
  }
}

void ReorderFieldTransInfo::dump() const {
  // print TransformedIndexes
  dumpMappedIndex();

  // print TransformedTypeNewSizeMap
  dumpMappedSize();

  // print InclusiveStructTypes
  dumpInclusiveStructTypes();
}
#endif // #if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This class, which is derived from DTransOptBase, does all necessary
// work for field-reorder transformation.
//
// prepareTypes(), populateTypes():
//   Create new types for structs that are field-reordered and let
//   DTransOptBase do the actual type replacement.
//
// processFunction():
//   Handle any Byte-Flatten GEP.
//
// postprocessFunction():
//   1. GEP: Replace old index with new index of fields
//   2. memset/memcpy/memmov/alloc/calloc/realloc/sdiv: Replace old size
//      with new size.
//
class ReorderFieldsOPImpl : public DTransOPOptBase {
public:
  ReorderFieldsOPImpl(
      const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      ReorderFieldTransInfo &RTI, DTransSafetyInfo *DTInfo, LLVMContext &Ctx,
      StringRef DepTypePrefix)
      : DTransOPOptBase(Ctx, DTInfo, DepTypePrefix), DL(DL), GetTLI(GetTLI),
        RTI(RTI) {}

  virtual bool prepareTypes(Module &M) override;
  virtual void populateTypes(Module &M) override;
  virtual void processFunction(Function &F) override;
  virtual void postprocessFunction(Function &Func, bool isCloned) override;

private:
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;
  ReorderFieldTransInfo &RTI;

  using LLVMStructTypeToStructTypeMap =
      DenseMap<llvm::StructType *, llvm::StructType *>;
  LLVMStructTypeToStructTypeMap Orig2NewTypeMap;
  LLVMStructTypeToStructTypeMap
      New2OrigTypeMap; // reverse map, for easier lookup

  // A mapping from the llvm::Type to the DTransType for the original & new
  // structure types processed by this transformation.
  DenseMap<llvm::StructType *, DTransStructType *>
      LLVMStructTypeToDTransStructType;

  DTransStructType *lookupDTransStructTypeForStruct(llvm::StructType *);

  void processBinaryOperator(BinaryOperator &BO);
  void processCallInst(CallInst &CI);
  void transformDivOp(BinaryOperator &I);
  void processGetElementPtrInst(GetElementPtrInst &GEP);
  void processByteFlattenedGetElementPtrInst(GetElementPtrInst &GEP);
  void transformAllocCall(CallInst &CI, StructType *OrigTy,
                          dtrans::AllocCallInfo *CInfo);
  void transformMemfunc(CallInst &CI, StructType *Ty);
  bool replaceOldSizeWithNewSize(Value *Val, uint64_t OldSize, uint64_t NewSize,
                                 Instruction *I, uint32_t APos);
  bool replaceOldSizeWithNewSizeForConst(Value *Val, uint64_t OldSize,
                                         uint64_t NewSize, Instruction *I,
                                         uint32_t APos);
  void replaceOldValWithNewVal(Instruction *I, uint32_t APos, Value *NewVal);
  Type *getOrigTyOfTransformedType(Type *TType);
  StructType *getStructTyAssociatedWithCallInfo(dtrans::CallInfo *CI);
  StructType *unmapInclusiveType(dtrans::CallInfo *CI);

  bool collectInclusiveStructTypes(const Module &M);
  bool findInclusiveStructType(
      llvm::StructType *BaseInclStTy,
      std::vector<StructType *> &DerivedInclusiveStructTypeV);

  bool checkDependentTypeSafety(llvm::StructType *Ty);
};

// Helper function to get associated StructType of \p CallInfo.
StructType *
ReorderFieldsOPImpl::getStructTyAssociatedWithCallInfo(dtrans::CallInfo *CI) {
  for (auto *StTy : CI->getElementTypesRef().element_llvm_types()) {
    // StTy is original type, call getOrigTyOfTransformedType to
    // make sure the type is transformed.
    Type *OrigStTy = getOrigTyOfTransformedType(StTy);
    if (OrigStTy)
      return cast<StructType>(OrigStTy);
  }
  return nullptr;
}

// Helper function to get original StructType of given \p TType
// if it is either Orig or New type in Orig2NewTypeMap.
// Otherwise, returns nullptr.
Type *ReorderFieldsOPImpl::getOrigTyOfTransformedType(Type *TType) {
  // Search Orig2NewTypeMap:
  for (auto &TypesPair : Orig2NewTypeMap) {
    Type *OrigTy = TypesPair.first;
    Type *NewTy = TypesPair.second;
    if (NewTy == TType || OrigTy == TType) {
      LLVM_DEBUG(dbgs() << "OrigTy: " << *OrigTy << "\nNewTy: " << *NewTy
                        << "\n";);
      return OrigTy;
    }
  }

  return nullptr;
}

StructType *ReorderFieldsOPImpl::unmapInclusiveType(dtrans::CallInfo *CI) {
  // Build mapping for types in InclusiveStructTypeV only ONCE!
  Function *F = CI->getInstruction()->getParent()->getParent();
  if (!RTI.doInclusiveStructTypeMap(&TypeRemapper, F)) {
    assert(0 && "Failed mapInclusiveStructTypes()\n");
    return nullptr;
  }

  // Find the OrigTy of a matching ReorderedTy
  for (auto *ElemTy : CI->getElementTypesRef().element_llvm_types()) {
    StructType *ReorderedTy = dyn_cast<StructType>(ElemTy);

    // Search InclusiveStructTypeUnmap
    // map ReorderedTy -> OrigTy
    StructType *OrigTy = RTI.unmapInclusiveStructType(ReorderedTy);
    if (OrigTy)
      return OrigTy;
  }

  return nullptr;
}

DTransStructType *
ReorderFieldsOPImpl::lookupDTransStructTypeForStruct(llvm::StructType *Ty) {
  auto It = LLVMStructTypeToDTransStructType.find(Ty);
  if (It != LLVMStructTypeToDTransStructType.end())
    return It->second;
  return nullptr;
}

// Sets \p NewVal as \p APos th operand of \p I. This routine expects
// only CallInst and SDiv/UDiv instructions.
void ReorderFieldsOPImpl::replaceOldValWithNewVal(Instruction *I, uint32_t APos,
                                                  Value *NewVal) {
  if (auto *CI = dyn_cast<CallInst>(I))
    CI->setArgOperand(APos, NewVal);
  else if (I->getOpcode() == Instruction::SDiv ||
           I->getOpcode() == Instruction::UDiv)
    I->setOperand(APos, NewVal);
  else
    llvm_unreachable("Invalid Inst");
}

// If \p Val is ConstantInt, routine expects it is multiple of
// \p OldSize and it is replaced with
//        (Val / OldSize ) * NewSize.
// Otherwise, it returns false.
bool ReorderFieldsOPImpl::replaceOldSizeWithNewSizeForConst(Value *Val,
                                                            uint64_t OldSize,
                                                            uint64_t NewSize,
                                                            Instruction *I,
                                                            uint32_t APos) {
  auto *ConstVal = dyn_cast<ConstantInt>(Val);
  if (!ConstVal)
    return false;
  uint64_t ConstSize = ConstVal->getLimitedValue();
  assert((ConstSize % OldSize) == 0 && "Expects multiple of OldSize");
  uint64_t NewConst = (ConstSize / OldSize) * NewSize;
  replaceOldValWithNewVal(I, APos, ConstantInt::get(Val->getType(), NewConst));
  return true;
}

// This routine converts \p Val from multiple of OldSize to multiple of NewSize.
// If \p Val is not constant, it generates Mul & SDiv to do the same.
bool ReorderFieldsOPImpl::replaceOldSizeWithNewSize(Value *Val,
                                                    uint64_t OldSize,
                                                    uint64_t NewSize,
                                                    Instruction *I,
                                                    uint32_t APos) {
  if (!Val)
    return false;

  // Check if Val is multiple of OldSize.
  if (!dtrans::isValueMultipleOfSize(Val, OldSize))
    return false;

  if (replaceOldSizeWithNewSizeForConst(Val, OldSize, NewSize, I, APos))
    return true;

  // Code generation to adjust size:
  Value *OldSizeVal = ConstantInt::get(Val->getType(), OldSize);
  Value *NewSizeVal = ConstantInt::get(Val->getType(), NewSize);
  Instruction *Div = BinaryOperator::CreateExactSDiv(Val, OldSizeVal);
  Instruction *Mul = BinaryOperator::CreateMul(Div, NewSizeVal);
  Mul->insertBefore(I);
  Div->insertBefore(Mul);
  LLVM_DEBUG(dbgs() << "Replacing " << *Val << "\nwith\n"
                    << *Div << "\n"
                    << *Mul << "\nin\n"
                    << *I << "\n");

  replaceOldValWithNewVal(I, APos, Mul);

  return true;
}

// Field-reordering is not enabled for types that are marked with
// MemFuncPartialWrite. So, just need to handle MemFunc with Complete
// aggregates.
// Thus only Size argument needs to be fixed.
void ReorderFieldsOPImpl::transformMemfunc(CallInst &CI, StructType *Ty) {
  Value *SizeVal = CI.getArgOperand(2);
  LLVM_DEBUG(dbgs() << "Memfunc Before:" << CI << "\n");

  bool Replaced =
      replaceOldSizeWithNewSize(SizeVal, DL.getTypeAllocSize(Ty),
                                RTI.getTransformedTypeNewSize(Ty), &CI, 2);
  assert(Replaced == true &&
         "Expecting oldSize should be replaced with NewSize");

  (void)Replaced;
  LLVM_DEBUG(dbgs() << "Memfunc After:" << CI << "\n");
}

// Fix size argument of calloc/malloc/realloc
void ReorderFieldsOPImpl::transformAllocCall(CallInst &CI, StructType *OrigTy,
                                             dtrans::AllocCallInfo *CInfo) {
  dtrans::AllocKind Kind = CInfo->getAllocKind();
  LLVM_DEBUG(dbgs() << "Alloc Before:" << CI << "\n");

  unsigned SizeArgPos = 0;
  unsigned CountArgPos = 0;
  const TargetLibraryInfo &TLI = GetTLI(*CI.getFunction());
  dtrans::getAllocSizeArgs(Kind, &CI, SizeArgPos, CountArgPos, TLI);

  // Obtain OrigTypeSize and ReorderTypeSize:
  const uint64_t OrigTypeSize = DL.getTypeAllocSize(OrigTy);
  uint64_t ReorderTypeSize = 0;
  if (RTI.hasTransformedTypeNewSize(OrigTy))
    ReorderTypeSize = RTI.getTransformedTypeNewSize(OrigTy);
  else if (RTI.hasInclusiveStructOrigType(OrigTy)) {
    StructType *ReorderedTy = RTI.mapInclusiveStructType(OrigTy);
    assert(ReorderedTy && "EXPECT ReorderedTy exist\n");
    ReorderTypeSize = DL.getTypeAllocSize(ReorderedTy);
  } else
    llvm_unreachable("Unexpected flow\n");

  // Do size replacement:
  auto *AllocSizeVal = CI.getArgOperand(SizeArgPos);
  bool Replaced = replaceOldSizeWithNewSize(AllocSizeVal, OrigTypeSize,
                                            ReorderTypeSize, &CI, SizeArgPos);

  // If AllocSizeVal is not a multiple of OrigTypeSize, try to fix the
  // count argument.
  if (CountArgPos != -1U && !Replaced)
    Replaced =
        replaceOldSizeWithNewSize(CI.getArgOperand(CountArgPos), OrigTypeSize,
                                  ReorderTypeSize, &CI, CountArgPos);

  // If AllocSizeVal is a constant but not a multiple of the original struct
  // size, try to statically adjust its value.
  //
  // Note:
  // there are corner cases where the size appearing under the malloc
  // instruction is not a direct multiple of the OrigTypeSize.
  //
  // E.g.
  // %1 = tail call i8* @_Znwm(i64 208) #34
  // %1 will be used to store %class.cPacket type, but size of %class.cPacket
  // is actually 192 instead of 208.
  //
  // For not-clear reason(s), the allocated size is 16B larger than its matching
  // type size. Size of Original %class.cPacket is 192, size of reordered
  // %class.cPacket is 200. It becomes 8B larger.
  //
  // For each of such case, take the following actions:
  // - ArgSize = size from the memory allocation call  (208)
  //   DIV = ArgSize / OrigTypeSize                    (  1 = 208 / 192)
  //   REM = ArgSize % OrigTypeSize                    ( 16 = 208 % 192)
  // - NewArgSize = DIV * ReorderedTypeSize + REM      (216 =   1 * 200 + 16)
  // - do size replacement
  //   Produces: %1 = tail call i8* @_Znwm(i64 216) #34
  //
  if (!Replaced) {
    uint64_t AllocatedSize = 0;
    bool HasIntConst = getConstantFromValue(AllocSizeVal, AllocatedSize);
    if (HasIntConst && (AllocatedSize % OrigTypeSize != 0)) {
      unsigned Div = AllocatedSize / OrigTypeSize;
      unsigned Rem = AllocatedSize % OrigTypeSize;
      uint64_t NewArgSize = Div * ReorderTypeSize + Rem;
      replaceOldValWithNewVal(
          &CI, SizeArgPos,
          ConstantInt::get(AllocSizeVal->getType(), NewArgSize));
      Replaced = true;
    }
  }

  assert(Replaced == true &&
         "Expect OrigTypeSize should have been replaced with ReorderTypeSize");
  LLVM_DEBUG(dbgs() << "Alloc After:" << CI << "\n");
}

// Fix field index value in GEP if GEP is computing an address of a field
// inside a struct that is being reordered.
void ReorderFieldsOPImpl::processGetElementPtrInst(GetElementPtrInst &GEP) {
  Type *SourceTy = GEP.getSourceElementType();
  if (!isa<StructType>(SourceTy))
    return;

  // Get original struct type (before mapping)
  // Attempt: is OrigStTy a relevant Inclusive Struct Type
  StructType *OrigStTy =
      dyn_cast_or_null<StructType>(getOrigTyOfTransformedType(SourceTy));
  if (!OrigStTy) {
    OrigStTy = RTI.unmapInclusiveStructType(dyn_cast<StructType>(SourceTy));
    if (OrigStTy)
      LLVM_DEBUG(dbgs() << "SourceTy: " << *SourceTy << "\n"
                        << "OrigStTy: " << *OrigStTy << "\n";);
    if (!OrigStTy)
      return;
  }

  // Nothing to do if the GEP is not computing an address of any field.
  if (GEP.getNumOperands() < 3)
    return;

  // If the GEP refers to a field inside a StructType being reordered, replace
  // the proper indices.
  const unsigned TotalIdx = GEP.getNumOperands() - 1;
  unsigned LastIdx = TotalIdx;
  LLVM_DEBUG(dbgs() << "GEP: " << GEP << "\n"
                    << "Total Idx: " << TotalIdx << "\n";);
  std::vector<unsigned> OpsSize;
  unsigned DstIdx = 0;

  SmallVector<Value *, 8> Ops(GEP.idx_begin(), GEP.idx_end());
  while (Ops.size() > 1) {
    Value *IdxArg = Ops.back();
    Ops.pop_back();
    --LastIdx;

    // If the GEP isn't indexing a structure, continue.
    Type *IndexedTy =
        GetElementPtrInst::getIndexedType(GEP.getSourceElementType(), Ops);
    if (!IndexedTy)
      continue;
    StructType *ReplTy = dyn_cast<StructType>(IndexedTy);
    if (!ReplTy)
      continue;

    // If the StructType * is not one to reorder, continue.
    if (!New2OrigTypeMap.count(ReplTy))
      continue;

    // If the last element isn't a constant int, continue.
    auto *Idx = dyn_cast<ConstantInt>(IdxArg);
    if (!Idx)
      continue;

    // If we can't get a value for this constant or it exceeds the capacity
    // of an unsigned value, continue
    uint64_t SrcIdx = Idx->getLimitedValue();
    if (SrcIdx > std::numeric_limits<unsigned>::max())
      continue;

    LLVM_DEBUG(dbgs() << "GEP (before): " << GEP << "\n"
                      << "SrcIdx: " << SrcIdx << "\t";);

    llvm::Type *OrigTy = New2OrigTypeMap[ReplTy];
    if (!OrigTy || !isa<StructType>(OrigTy))
      assert(0 && "Valid OrigTy must exist\n");

    // Do Field-Reorder index replacement ONLY IF the index changes.
    DstIdx = RTI.getTransformedIndex(cast<StructType>(OrigTy), SrcIdx);
    if (DstIdx != SrcIdx) {
      GEP.setOperand(LastIdx + 1, ConstantInt::get(IdxArg->getType(), DstIdx));
      OpsSize.push_back(Ops.size());
    }

    LLVM_DEBUG(dbgs() << ", DstIdx: " << DstIdx << "\n"
                      << "GEP (after): " << GEP << "\n";);

    // Continue after the current index replacement.
    // May have additional valid replacement(s).
  }
}

// Fix offset value in ByteFlattened GEP if it is computing address of
// a field in any reordered struct.
void ReorderFieldsOPImpl::processByteFlattenedGetElementPtrInst(
    GetElementPtrInst &GEP) {

  // Only two operands are expected for ByteFlattened GEPs
  if (GEP.getNumOperands() != 2)
    return;

  auto GEPInfo = DTInfo->getByteFlattenedGEPElement(&GEP);
  if (!GEPInfo.first)
    return;

  DTransType *SourceTy = GEPInfo.first;
  DTransStructType *StructTy = dyn_cast<DTransStructType>(SourceTy);
  if (!StructTy)
    return;

  // Byte-Flatten GEP -- BFGEP
  bool BFGEPHandled = false;
  uint64_t NewOffset = 0;
  auto &TypesMap = RTI.getTypesMap();

  // OrigTy is a DTrans's Field-Reorder (DFR) transformed type
  StructType *OrigLLVMTy = cast<StructType>(StructTy->getLLVMType());
  if (TypesMap.find(StructTy) != TypesMap.end()) {
    StructType *NewLLVMTy = cast<StructType>(Orig2NewTypeMap[OrigLLVMTy]);
    if (!NewLLVMTy)
      return;
    uint32_t OldIdx = GEPInfo.second;
    uint32_t NewIdx = RTI.getTransformedIndex(OrigLLVMTy, OldIdx);

    // Get offset of the field in new layout.
    auto *SL = DL.getStructLayout(NewLLVMTy);
    NewOffset = SL->getElementOffset(NewIdx);

    BFGEPHandled = true;
  } else if (RTI.hasInclusiveStructType(OrigLLVMTy)) {
    // StructTy is a valid Inclusive StructType, but not a DTrans's
    // Field-Reorder (DFR) transformed type
    uint64_t OffsetVal = 0;
    if (!getConstantFromValue(GEP.getOperand(1), OffsetVal))
      return;

    llvm::StructType *OrigTy = RTI.getDFRCandidateStructType(OrigLLVMTy);
    if (!OrigTy)
      return;

    // Compute the size of the OrigTy, decide where the offset is located
    unsigned OrigTySize = DL.getTypeStoreSize(OrigTy);
    if (OffsetVal < OrigTySize) {
      // Offset is inside OrigTy: NewOffset is inside ReorderedTy

      // Expected Layout:
      // -------------            ------------
      // |           |            |           |
      // | OrigTy    |            | NewTy     |
      // |   *       |            |   *       |
      // -------------  -- DFR -> ------------
      // |   ...     |            |   ...     |
      // | additional|            | additional|
      // |   ...     |            |   ...     |
      // -------------            ------------
      //
      // [Note]
      // *: where the Offset is located
      //
      uint32_t OldIdx = GEPInfo.second;
      uint32_t NewIdx = RTI.getTransformedIndex(OrigLLVMTy, OldIdx);
      StructType *NewLLVMTy = cast<StructType>(Orig2NewTypeMap[OrigLLVMTy]);
      if (!NewLLVMTy)
        return;

      // Get offset of the field in new layout.
      auto *SL = DL.getStructLayout(NewLLVMTy);
      NewOffset = SL->getElementOffset(NewIdx);

      BFGEPHandled = true;
    } else {
      // Offset is outside/after OrigTy: NewOffset is outside ReorderedTy

      // Expected Layout:
      // -------------            ------------
      // |           |            |           |
      // | OrigTy    |            | NewTy     |
      // |           |            |           |
      // -------------  -- DFR -> ------------
      // |   ...     |            |   ...     |
      // | additional|            | additional|
      // |    *      |            |    *      |
      // |   ...     |            |   ...     |
      // -------------            ------------
      //
      // [Note]
      // *: where the Offset is located
      //
      StructType *NewLLVMTy = cast<StructType>(Orig2NewTypeMap[OrigTy]);
      if (!NewLLVMTy)
        return;
      const unsigned NewTySize = DL.getTypeStoreSize(NewLLVMTy);
      NewOffset = OffsetVal - OrigTySize + NewTySize;

      BFGEPHandled = true;
    }
  }

  // Debug print the GEP only if offset is to be updated
  if (BFGEPHandled) {
    LLVM_DEBUG(dbgs() << "ByteFGEP Before:" << GEP << "\n");
    GEP.setOperand(1,
                   ConstantInt::get(GEP.getOperand(1)->getType(), NewOffset));
    LLVM_DEBUG(dbgs() << "ByteFGEP After:" << GEP << "\n");
  }
}

// DTransAnalysis allows below pointer arithmetic pattern:
//    %2 = sub Ptr1_str1, Ptr2_str1
//    %3 = sdiv %2, size_of(Str1)
//
//    size_of(str1) needs to be changed if reordering is applied to str1
//
// [Note]
// The transformation can also support DIV Operations on any Inclusive Struct
// Type.
//
void ReorderFieldsOPImpl::transformDivOp(BinaryOperator &I) {
  assert((I.getOpcode() == Instruction::SDiv ||
          I.getOpcode() == Instruction::UDiv) &&
         "Unexpected opcode");
  auto *SubOp = dyn_cast<BinaryOperator>(I.getOperand(0));
  if (!SubOp)
    return;
  DTransType *PtrSubTy = DTInfo->getResolvedPtrSubType(SubOp);
  if (!PtrSubTy)
    return;

  llvm::StructType *StructTy =
      dyn_cast<llvm::StructType>(PtrSubTy->getLLVMType());
  if (!StructTy)
    return;

  // Search Inclusive Struct Type:
  if (StructType *OrigInclusiveStructTy =
          RTI.unmapInclusiveStructType(cast<StructType>(StructTy)))
    StructTy = OrigInclusiveStructTy;

  // StructTy is a DTrans's Field-Reorder (DFR) transformed type
  if (RTI.hasTransformedTypeNewSize(StructTy)) {
    LLVM_DEBUG(dbgs() << "SDiv/UDiv Before:" << I << "\n");
    Value *SizeVal = I.getOperand(1);
    bool Replaced = replaceOldSizeWithNewSize(
        SizeVal, DL.getTypeAllocSize(StructTy),
        RTI.getTransformedTypeNewSize(StructTy), &I, 1);
    assert(Replaced == true &&
           "Expecting oldSize should be replaced with NewSize");

    (void)Replaced;
    LLVM_DEBUG(dbgs() << "SDiv/UDiv After:" << I << "\n");

  } else if (RTI.hasInclusiveStructType(StructTy)) {
    // StructTy is an Inclusive Struct Type, but not a DTrans's Field-Reorder
    // (DFR) transformed type
    LLVM_DEBUG(dbgs() << "SDiv/UDiv Before:" << I << "\n");
    Value *SizeVal = I.getOperand(1);
    StructType *MappedIncStTy = RTI.mapInclusiveStructType(StructTy);
    bool Replaced =
        replaceOldSizeWithNewSize(SizeVal, DL.getTypeAllocSize(StructTy),
                                  DL.getTypeAllocSize(MappedIncStTy), &I, 1);
    assert(Replaced == true &&
           "Expecting oldSize should be replaced with NewSize");

    (void)Replaced;
    LLVM_DEBUG(dbgs() << "SDiv/UDiv After:" << I << "\n");
  }
}

// Only Pointer arithmetic that is allowed in Analysis is Instruction::Sub
// that is used by SDiv.
void ReorderFieldsOPImpl::processBinaryOperator(BinaryOperator &BO) {
  switch (BO.getOpcode()) {
  case Instruction::SDiv:
  case Instruction::UDiv:
    transformDivOp(BO);
    break;
  default:
    break;
  }
}

// Handle each relevant CallInst.
// The only CallInst instructions that need to be handled by Reordering are
// Alloc and Memfunc.
//
// There is no need to process any GEP that is passed as an argument to a call
// since reordering is disabled as it is treated as AddressTaken.
//
void ReorderFieldsOPImpl::processCallInst(CallInst &CI) {
  dtrans::CallInfo *CInfo = DTInfo->getCallInfo(&CI);
  if (CInfo == nullptr)
    return;

  StructType *ReorderTy = getStructTyAssociatedWithCallInfo(CInfo);
  StructType *OrigTy = unmapInclusiveType(CInfo);
  if (!ReorderTy && !OrigTy)
    return;
  StructType *StructTy = ReorderTy ? ReorderTy : OrigTy;

  switch (CInfo->getCallInfoKind()) {
  case dtrans::CallInfo::CIK_Alloc: {
    transformAllocCall(CI, StructTy, cast<dtrans::AllocCallInfo>(CInfo));
    break;
  }
  case dtrans::CallInfo::CIK_Memfunc: {
    transformMemfunc(CI, StructTy);
    break;
  }
  default:
    break;
  }
}

// Fix all necessary IR changes related to field-reordering transformation
// except for ByteFlattened GEPs, which are processed in processFunction().
//
// - GEP Inst: Replace old index of a field with new index
// - CallInst: Replace old size of struct with new size in malloc/calloc/
//             realloc/memset/memcpy etc.
// - BinaryOp: Fix size that is used in SDiv/UDiv.
//
void ReorderFieldsOPImpl::postprocessFunction(Function &Func, bool isCloned) {
  Function *F = isCloned ? OrigFuncToCloneFuncMap[&Func] : &Func;
  RTI.doInclusiveStructTypeMap(&TypeRemapper, F);

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst))
      processGetElementPtrInst(*GEP);
    else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(Inst))
      processBinaryOperator(*BO);
    else if (CallInst *CI = dyn_cast<CallInst>(Inst))
      processCallInst(*CI);
  }
}

// Only ByteFlattened GEPs related to field-reordering structs are processed
// here.
//
// -ByteFlattened GEP: Old offset of a field is replaced with new offset.
//
void ReorderFieldsOPImpl::processFunction(Function &F) {
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst))
      processByteFlattenedGetElementPtrInst(*GEP);
  }
}

void ReorderFieldsOPImpl::populateTypes(Module &M) {
  LLVM_DEBUG(dbgs() << "ReorderFieldsOPImpl::populateTypes(M)\n");
  for (DTransStructType *OrigDTransTy : RTI.getTypesMap()) {
    llvm::StructType *OrigLLVMTy =
        cast<llvm::StructType>(OrigDTransTy->getLLVMType());
    size_t NumFields = OrigLLVMTy->getNumElements();
    std::vector<Type *> LLVMDataTypes(NumFields);
    std::vector<DTransType *> DTransDataTypes(NumFields);

    // Create reordered field layout
    for (unsigned I = 0, E = NumFields; I < E; ++I) {
      DTransType *DTransFieldTy = OrigDTransTy->getFieldType(I);
      unsigned NewIdx = RTI.getTransformedIndex(OrigLLVMTy, I);
      DTransType *NewFieldTy = TypeRemapper.remapType(DTransFieldTy);
      DTransDataTypes[NewIdx] = NewFieldTy;
      LLVMDataTypes[NewIdx] = NewFieldTy->getLLVMType();
    }

    // Setup replacement type:
    llvm::StructType *NewLLVMTy = Orig2NewTypeMap[OrigLLVMTy];
    assert(NewLLVMTy && "prepareTypes() should set type");
    NewLLVMTy->setBody(LLVMDataTypes, OrigLLVMTy->isPacked());

    DTransStructType *NewDTransTy = LLVMStructTypeToDTransStructType[NewLLVMTy];
    assert(NewDTransTy && "prepareTypes() should set type");
    NewDTransTy->setBody(DTransDataTypes);

    LLVM_DEBUG(dbgs() << "Struct " << getStName(OrigLLVMTy)
                      << " after field-reordering: \n"
                      << *NewDTransTy << "\n");
    assert(RTI.getTransformedTypeNewSize(OrigLLVMTy) ==
               M.getDataLayout().getTypeAllocSize(NewLLVMTy) &&
           "Size computation is incorrect");
  }
}

// Given an Inclusive Struct Type (IST) as root, identify any/all derived
// Inclusive Struct Type(s).
//
// E.g.
// %class.A = {...}
// %class.B = {%class.A, ...}
// %class.C = {%class.A ... }
// %class.D = {%class.B ... }
//
// If class.A is a base IST, then class.B, class.C, and class.D are all derived
// ISTs.
//
// [Algorithm]
// -Step1
//  Given a root/base IST, Obtain all struct types using DTrans's
//  TypeToDependentTypes mapping.
//
// -Step2
//  for each such struct type:
//  . check if there is at least 1 field whose type matches the Root IST
//  . if so, collect current struct type;
//
// -Step3
//  For each collected Derived IST from Step2:
//  recurse into Step1, until no new available type(s) are found
//
bool ReorderFieldsOPImpl::findInclusiveStructType(
    llvm::StructType *BaseInclStTy,
    std::vector<StructType *> &DerivedInclusiveStructTypeV) {

  // Check:
  // Does a given MappedStTy contain at least 1 field whose type matches
  // BaseInclStTy?
  auto contains = [](StructType *MappedStTy,
                     const StructType *BaseInclStTy) -> bool {
    for (unsigned I = 0, E = MappedStTy->getNumElements(); I != E; ++I) {
      llvm::StructType *FieldStTy =
          dyn_cast<StructType>(MappedStTy->getElementType(I));

      // Skip any non-struct type field:
      if (!FieldStTy)
        continue;

      if (FieldStTy == BaseInclStTy)
        return true;
    }

    return false;
  };

  if (!BaseInclStTy)
    llvm_unreachable("BaseInclStTy* can't be empty");

  // Obtain StructType* from TypeToDependenTypes mapping
  DTransType *BaseDTransTy = TM.getStructType(BaseInclStTy->getName());
  auto Ty2DepTySetVector = TypeToDirectDependentTypes[BaseDTransTy];
  if (Ty2DepTySetVector.empty())
    // No mapping available for BaseInclStTy
    return false;

  // Save each qualified mapping into QualifiedStTyV
  std::vector<StructType *> QualifiedStTypeV;
  QualifiedStTypeV.clear();
  for (auto *DTransTy : Ty2DepTySetVector) {
    StructType *StTy = dyn_cast<StructType>(DTransTy->getLLVMType());
    if (!StTy)
      continue;

    // Check: if StTy has at least 1 field that matches the BaseInclStTy type
    if (contains(StTy, BaseInclStTy)) {
      LLVM_DEBUG(dbgs() << "StTy: " << *StTy << "\n";);
      QualifiedStTypeV.push_back(StTy);
    }
  }

  // If nothing in QualifiedStTypeV, return
  if (QualifiedStTypeV.empty())
    return false;

  // Copy everything from QualifiedStTypeV into DerivedInclusiveStructType,
  // and recurse.
  LLVM_DEBUG({
    dbgs() << "BaseInclStTy: " << *BaseInclStTy << "\n"
           << "QualifiedStTypeV <: " << QualifiedStTypeV.size() << ">\n";
    unsigned Count = 0;
    for (auto *StTy : QualifiedStTypeV) {
      dbgs() << Count++ << " : " << *StTy << "\n";
    }
  });
  std::copy(QualifiedStTypeV.begin(), QualifiedStTypeV.end(),
            std::back_inserter(DerivedInclusiveStructTypeV));

  for (auto *StTy : QualifiedStTypeV) {
    LLVM_DEBUG(dbgs() << *StTy << "\n";);
    findInclusiveStructType(StTy, DerivedInclusiveStructTypeV);
  }

  LLVM_DEBUG({
    dbgs() << "BaseInclStTy: " << *BaseInclStTy << "\n"
           << "DerivedInclusiveStructTypeV <: "
           << DerivedInclusiveStructTypeV.size() << ">\n";
    unsigned Count = 0;
    for (auto *StTy : DerivedInclusiveStructTypeV) {
      dbgs() << Count++ << " : " << *StTy << "\n";
    }
  });

  return true;
}

// Collect all inclusive Module-level StructType * into InclusiveStructTypeV.
// Return true if there is any inclusive struct type(s) collected.
//
bool ReorderFieldsOPImpl::collectInclusiveStructTypes(const Module &M) {
  std::vector<StructType *> DerivedInclusiveStructTypeV;

  // For each valid OrigTy (from RTI), find all Inclusive Struct Types,
  // and save them into DerivedInclusiveStructTypeV.
  for (auto *DTransTy : RTI.getTypesMap()) {
    StructType *BaseInclStTy = cast<llvm::StructType>(DTransTy->getLLVMType());
    assert(BaseInclStTy && "Expect BaseInclStTy be valid");
    LLVM_DEBUG(dbgs() << "BaseInclStTy: " << *BaseInclStTy << "\n";);
    std::vector<StructType *> TempInclStructTypeV;

    // Current BaseIncStTy is a valid Inclusive StructType
    DerivedInclusiveStructTypeV.push_back(BaseInclStTy);

    // Search all Inclusive Struct Type(s) rooted from BaseInclStTy
    if (!findInclusiveStructType(BaseInclStTy, TempInclStructTypeV))
      continue;

    // Save any Derived Inclusive Struct Type into DerivedInclusiveStructTypeV:
    std::copy(TempInclStructTypeV.begin(), TempInclStructTypeV.end(),
              std::back_inserter(DerivedInclusiveStructTypeV));
  }

  // Result container:
  std::vector<StructType *> &InclusiveStructTypeV =
      RTI.getInclusiveStructTypes();

  // Copy all Derived Candidate Types into InclusiveStructTypeV
  std::copy(DerivedInclusiveStructTypeV.begin(),
            DerivedInclusiveStructTypeV.end(),
            std::back_inserter(InclusiveStructTypeV));

  LLVM_DEBUG(RTI.dumpInclusiveStructTypes(););

  return InclusiveStructTypeV.size();
}

// Return 'true' if there are no safety issues with a dependent type \p Ty
// that prevent DTrans' ReorderFields (DFR) transformation.
bool ReorderFieldsOPImpl::checkDependentTypeSafety(llvm::StructType *Ty) {
  if (!Ty->hasName())
    return false;

  DTransType *DTy = TM.getStructType(Ty->getName());
  auto *TI = DTInfo->getTypeInfo(DTy);
  assert(TI && "Expected DTrans to have analyzed dependent type");

  if (DTInfo->testSafetyData(TI, dtrans::DT_ReorderFieldsDependent))
    return false;

  // No issues were found on this dependent type that prevent DTrans'
  // Reorder-Fields (DFR) transformation.
  return true;
}

// [Check]
// - Is there any Dependent Types from any type collected in CandidateTypesV?
// - If any DependentType is illegal, will bail out.
//
// [Action]
// Create opaque type place holder for each to-be reordered struct type.
//
// [Notes]
// Work in the following order in this function:
// 1. do collection
// 2. do secondary legal test for any available Inclusive Struct Types
// 3. do Type mapping
//
// If any test on a  DTrans Field Reorder (DFR) candidate's inclusive struct
// type fails, this routine returns false.
// As a result, DFR on the respective struct type candidate will fail.
//
// This is a side exit inside the DTrans Optimization Framework.
//
// It is necessary to locate the 2nd legal test in the prepareTypes() routine
// because the test needs to access TypeToDependenTypes map which is only
// available inside an overwritten virtual function from a sub class of
// DtransOptOPBase type.
//

bool ReorderFieldsOPImpl::prepareTypes(Module &M) {
  LLVM_DEBUG(dbgs() << "ReorderFieldsOPImpl::prepareTypes(M)\n");

  // If there is any candidate, collect the Inclusive Struct Type(s):
  bool HasInclStTy = false;
  if (!RTI.getTypesMap().empty()) {
    HasInclStTy = collectInclusiveStructTypes(M);
    LLVM_DEBUG({
      std::string Msg = HasInclStTy ? "Has" : "No";
      dbgs() << Msg + " InclusiveStructType found.\n";
      RTI.dumpInclusiveStructTypes();
    });
  }

  // If there is any Inclusive Type(s), do legal test on them:
  if (HasInclStTy) {
    for (auto &InclStTy : RTI.getInclusiveStructTypes()) {
      // Skip any dependent type that is subject to DFR transformation.
      if (RTI.hasTransformedTypeNewSize(InclStTy))
        continue;

      // Legal test on InclStTy:
      if (!checkDependentTypeSafety(InclStTy)) {
        LLVM_DEBUG(
            dbgs() << "DTRANS-FieldsReorderingOP: Disqualified based on safety"
                      " conditions of dependent inclusive type: "
                   << getStName(cast<StructType>(InclStTy)) << *InclStTy
                   << "\n");
        return false;
      }
    }
  }

  // Create opaque type place holder for each to-be reordered struct type
  for (DTransStructType *OrigDTransTy : RTI.getTypesMap()) {
    StructType *OrigLLVMTy = cast<StructType>(OrigDTransTy->getLLVMType());
    StructType *NewLLVMTy = StructType::create(
        OrigLLVMTy->getContext(), "__DFR_" + OrigLLVMTy->getName().str());
    DTransStructType *NewDTransTy = TM.getOrCreateStructType(NewLLVMTy);
    TypeRemapper.addTypeMapping(OrigLLVMTy, NewLLVMTy, OrigDTransTy,
                                NewDTransTy);

    Orig2NewTypeMap[OrigLLVMTy] = NewLLVMTy;
    New2OrigTypeMap[NewLLVMTy] = OrigLLVMTy;
    LLVMStructTypeToDTransStructType[OrigLLVMTy] = OrigDTransTy;
    LLVMStructTypeToDTransStructType[NewLLVMTy] = NewDTransTy;

    LLVM_DEBUG(dbgs() << "OrigTy: " << *OrigDTransTy << "\n"
                      << "NewTy: " << *NewDTransTy << "\n";);
  }

  return true;
}

static bool isArrayOrVectTy(Type *Ty) {
  return (Ty->isArrayTy() || Ty->isVectorTy());
}

// Return true if the struct type is considered "Simple":
// - has neither ArrayType nor VectorType
// - not more than DTransReorderFieldsNumFieldsThreshold (default to 15) items
//   within a struct
//
static bool isSimpleStructType(dtrans::TypeInfo *TI) {
  auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
  if (!StInfo)
    return false;
  StructType *StructT = cast<StructType>(StInfo->getLLVMType());

  // a struct with any aggregate type or vector type is rejected
  if (StructT && std::any_of(StructT->element_begin(), StructT->element_end(),
                             isArrayOrVectTy))
    return false;

  // Only field address obtained via GEP are currently supported.
  if (std::any_of(StInfo->getFields().begin(), StInfo->getFields().end(),
                  [](dtrans::FieldInfo &FI) { return FI.hasNonGEPAccess(); }))
    return false;

  if (StructT &&
      StructT->getNumElements() < DTransReorderFieldsOPNumFieldsThreshold)
    return true;

  return false;
}

// Return false if the struct type has any array-type or vector-type field.
//
// Return true if a struct type has precisely:
// - DTransReorderFieldsNumFieldsAdvPrecise fields
// and
// - DTransReorderFieldsComplexStructIntTypeCount integer types
// and
// - DTransReorderFieldsComplexStructStructTypeCount struct types
// and
// - DTransReorderFieldsComplexStructPionterTypeCount pointer types
//
// All these variables are exposed as cmdline options to allow customization.
//
static bool isAdvancedStructType(dtrans::TypeInfo *TI) {
  auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
  if (!StInfo)
    return false;

  StructType *StructT = cast<StructType>(StInfo->getLLVMType());

  // Reject any struct type with a vector-type or array-type field:
  if (std::any_of(StructT->element_begin(), StructT->element_end(),
                  isArrayOrVectTy))
    return false;

  // Expect the struct to have DTransReorderFieldsNumFieldsAdvPrecise (20)
  // fields
  if (StructT->getNumElements() != DTransReorderFieldsOPNumFieldsAdvPrecise)
    return false;

  // Expect the struct to have 12 integer types, 5 struct types and
  // 3 pointer types.
  unsigned IntTypeCount = 0, StructTypeCount = 0, PointerTypeCount = 0;
  for (unsigned I = 0, E = StructT->getNumElements(); I < E; ++I) {
    Type *ElemTy = StructT->getElementType(I);

    if (isa<IntegerType>(ElemTy))
      ++IntTypeCount;
    else if (isa<StructType>(ElemTy))
      ++StructTypeCount;
    else if (isa<PointerType>(ElemTy))
      ++PointerTypeCount;
    else
      // early bailout: do not expect any other type
      return false;
  }

  return (IntTypeCount ==
          DTransReorderFieldsOPComplexStructIntTypeCount) && // 12
         (StructTypeCount ==
          DTransReorderFieldsOPComplexStructStructTypeCount) && // 5
         (PointerTypeCount ==
          DTransReorderFieldsOPComplexStructPionterTypeCount); // 3
}

// Returns true if a StructType is profitable for DFR based on space savings
// from unused space in the field-reorder structure due to field access
// frequency.
//
// Profitable model:
// - Obtain the size of a given Struct Type;
// - Sort the fields in the struct based on top-N access frequency, and obtain
//   accumulated size of each field in the field-reordered struct type;
// - If the difference (space savings) is bigger than a pre-defined threshold
//   (currently, set to 6.25%), the said struct type is profitable for
//   field-reordering transformation.
//
// If the Struct-Type is profitable, update the ReorderFieldTransInfo, RTI,
// class member to record the reorder-field index mapping.
//
// [Note]
// Due to the nature of field reordering within a structure, it is possible that
// the size of the struct may grow (becomes bigger) after reordering. As long as
// this size growth is bounded by a pre-defined threshold, it still can be
// profitable.
//
bool ReorderFieldsAnalyzer::isProfitable(dtrans::TypeInfo *TI,
                                         const DataLayout &DL) {

  // * Simple Sort *
  // Sort fields based on the provided compare function to minimize padding
  // between fields:
  //   - Ascending order based on Alignment, then
  //   - Ascending order based on Size, then
  //   - Descending order based on Index.
  //
  // The purpose of using Index in sorting fields is to maintain source
  // order of fields if alignment and size of the fields are the same.
  // "Index + 1" is used instead of "Index" to avoid incorrect comparison
  // with zero (i.e for 1st field) since negative index values are used
  // for sorting.
  auto SortSimple = [&](const FieldData &FD0, const FieldData &FD1) -> bool {
    return std::make_tuple(FD1.getAlign(), FD1.getSize(),
                           -(FD1.getIndex() + 1)) <
           std::make_tuple(FD0.getAlign(), FD0.getSize(),
                           -(FD0.getIndex() + 1));
  };

  // * Advanced Sort *
  // Sort fields based on access frequency
  auto SortAdvanced = [&](const FieldData &FD0, const FieldData &FD1) -> bool {
    return std::make_tuple(FD1.getFrequency(), FD1.getAlign(), FD1.getSize(),
                           -(FD1.getIndex() + 1)) <
           std::make_tuple(FD0.getFrequency(), FD0.getAlign(), FD0.getSize(),
                           -(FD0.getIndex() + 1));
  };

  // Sort within [BeginIdx, EndIdx] range using SortSimple comparison
  auto SortInRange = [&](std::vector<FieldData> &Fields, unsigned BeginIdx,
                         unsigned EndIdx) -> bool {
    assert((BeginIdx < EndIdx) && "Nothing to sort");
    assert((BeginIdx < Fields.size()) && " BeginIdx overflow");
    assert((EndIdx < Fields.size()) && " EndIdx overflow");

    // copy in
    std::vector<FieldData> Data;
    std::copy(Fields.begin() + BeginIdx, Fields.begin() + EndIdx,
              std::back_inserter(Data));

    // sort
    llvm::sort(Data.begin(), Data.end(), SortSimple);

    // explicit copy out
    for (unsigned I = 0, Size = EndIdx - BeginIdx; I < Size; ++I)
      Fields[BeginIdx + I] = Data[I];

    return true;
  };

  // Adjust border field: match sizes at the border.
  // E.g.
  // [before border field adjust]
  // 2,	2,	2,	21  <- last field of the 4-key field group
  // 4,	4,	9,	18  <- 1st field of non-key fields below
  // 2,	2,	1,	15
  //
  // [after border field adjust]
  // 2,	2,	2,	21  <- last field of the 4-key field group
  // 2,	2,	1,	15  <- 1st field of non-key fields below
  // 4,	4,	9,	18
  //
  auto AdjustBorderFields = [&](std::vector<FieldData> &Fields,
                                const unsigned TopAlignToMatch, unsigned TopIdx,
                                unsigned BottomIdx) -> bool {
    assert((TopIdx < Fields.size()) && "TopIdx overflow");
    assert((BottomIdx < Fields.size()) && "BottomIdxIdx overflow");
    assert((TopIdx < BottomIdx) && "Expect TomPdx < BottomIdxIdx");

    // * Adjust connection on TOP *
    // E.g.
    //
    //[BEFORE]         ->    [AFTER]: swap (1) and (2)
    //
    // ---                   ---
    // 36B                   36B
    // ---                   --- (TOP Position)
    // 8B (1)                4B <- this removes any padding need
    // 8B                    8B
    // 4B (2)                8B
    // 2B                    2B
    // ---                   ---
    //
    //[Action]
    // find the largest-suitable field within the key (4) fields, swap it
    // to the top, and minimize padding at top position.
    //
    const unsigned Alignments[] = {8, 4, 2, 1};
    // Set TopAlign to 1 (its default value).
    // Even if the search loop (below) fails, the default value is valid.
    unsigned TopAlign = Alignments[3];
    for (unsigned I = 0, E = sizeof(Alignments) / sizeof(unsigned); I < E;
         ++I) {
      if (TopAlignToMatch % Alignments[I] == 0) {
        TopAlign = Alignments[I];
        break;
      }
    }

    unsigned Idx = 0;
    bool FoundTopMatch = false;
    for (unsigned I = TopIdx; I < BottomIdx; ++I) {
      if (Fields[I].getSize() == TopAlign) {
        FoundTopMatch = true;
        Idx = I;
        break;
      }
    }

    if (FoundTopMatch)
      std::swap(Fields[Idx], Fields[TopIdx]);

    // * Adjust connection on BOTTOM *

    // E.g.
    //
    //[BEFORE]         ->    [AFTER]
    // .                     .
    // ---                   ---
    // 4B                    4B
    // 8B                    8B
    // 8B                    8B
    // 2B                    2B
    // ---                   ---  (Bottom Position)
    // ?                     2B *
    // .                     .
    // .                     .

    // [Action]
    // Find a field from the fields below Bottom Position, swap it to next to
    // bottom position, so that padding is minimized at bottom position.
    //
    const unsigned BottomPadSize = Fields[BottomIdx].getSize();

    // Find a field in the remaining fields that has the same Size as
    // BottomPadSize
    bool FoundBottomMatch = false;
    unsigned DestPadIdx = -1;
    for (unsigned I = BottomIdx + 1, E = Fields.size(); I < E; ++I) {
      if (Fields[I].getSize() == BottomPadSize) {
        FoundBottomMatch = true;
        DestPadIdx = I;
        break;
      }
    }

    // Swap only if a suitable destination field is available
    if (FoundBottomMatch)
      std::swap(Fields[BottomIdx + 1], Fields[DestPadIdx]);

    return FoundTopMatch || FoundBottomMatch;
  };

  // Bypass profit test?
  if (!EnableReorderFieldOPProfitableTest || !EnableReorderFieldOPTests)
    return true;

  auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
  if (!StInfo)
    return false;
  DTransStructType *StructT = cast<DTransStructType>(TI->getDTransType());
  llvm::StructType *LLVMStructT = cast<StructType>(TI->getLLVMType());

  // Collect Fields' info, including access frequency:
  std::vector<FieldData> Fields; // SmallVector causes memory corruption,
  // Use std::vector instead.
  Fields.clear();

  // Decide whether to include field0 into the sorting (and reordering).
  //
  // In C++, inheritance is made through the field0 of a structType.
  // E.g.
  // class A { int a;...}
  // class B: public A{char b; ...}
  //
  // Their respective types are:
  // %class.A = type {i32, ...}
  // %class.B = type {%class.A, i8, ...}
  //
  // If allowing %class.B's field0 (%class.A) to be sorted, the Dynamic Casting
  // between A and B will fail.
  //
  // To maintain correctness, if Field0 is a StructType, the transformation will
  // NOT include Field0 in sorting, thus won't reorder Field0.
  //
  // In case field0 won't participate in sorting and offset counting, we will
  // create a FD0 object to keep tracking (but not sorting).
  //
  FieldData FD0(0, 0, 0, 0);
  unsigned I = 0;
  bool SkipField0 = false;
  if (isa<StructType>(LLVMStructT->getElementType(0))) {
    I = 1;
    SkipField0 = true;
    Type *Ty0 = LLVMStructT->getElementType(0);
    dtrans::FieldInfo &FI = StInfo->getField(0);
    FD0.set(DL.getABITypeAlignment(Ty0), DL.getTypeStoreSize(Ty0), 0,
            FI.getFrequency());
  }

  // Collect all fields that will be sorted
  unsigned const E = LLVMStructT->getNumElements();
  for (; I < E; ++I) {
    Type *FieldTy = LLVMStructT->getElementType(I);
    dtrans::FieldInfo &FI = StInfo->getField(I);
    FieldData FD(DL.getABITypeAlignment(FieldTy), DL.getTypeStoreSize(FieldTy),
                 I, FI.getFrequency());
    Fields.push_back(FD);
  }

  // Print Fields before sorting
  LLVM_DEBUG({
    dbgs() << "Fields before sort:\t(SkipField0: " << SkipField0 << ")\n";
    if (SkipField0)
      FD0.dump();
    for (auto &Field : Fields) {
      Field.dump(true);
    }
  });

  const bool IsSimpleStructType = isSimpleStructType(TI);
  if (IsSimpleStructType)
    llvm::sort(Fields.begin(), Fields.end(), SortSimple);
  else if (isAdvancedStructType(TI)) {
    // Sort Fields based on descending order of access frequency:
    llvm::sort(Fields.begin(), Fields.end(), SortAdvanced);

    // Print Fields after sorting
    LLVM_DEBUG({
      dbgs() << "Fields after sort and before swap:\t(SkipField0: "
             << SkipField0 << ")\n";
      if (SkipField0)
        FD0.dump();
      for (auto &Field : Fields) {
        Field.dump(true);
      }
    });

    // Make the 4 key fields available at the begin of struct type (skip field0)
    // by swapping desired pairs of fields.
    unsigned AdjustIdx = SkipField0 ? 1 : 0;
    std::swap(Fields[DTransReorderFieldsOPSwapIndex1Begin - AdjustIdx],
              Fields[DTransReorderFieldsOPSwapIndex1End - AdjustIdx]);
    std::swap(Fields[DTransReorderFieldsOPSwapIndex2Begin - AdjustIdx],
              Fields[DTransReorderFieldsOPSwapIndex2End - AdjustIdx]);

    // Print Fields after placing the key fields at the right positions
    LLVM_DEBUG({
      dbgs() << "Fields after sort and swap:\t(SkipField0: " << SkipField0
             << ")\n";
      if (SkipField0)
        FD0.dump();
      for (auto &Field : Fields) {
        Field.dump(true);
      }
    });

    // Adjust order within the NumKeyFields key fields to minimize padding
    // (use SortSimple compare function)
    unsigned BeginIdx = AdjustIdx ? AdjustIdx - 1 : 0;
    unsigned EndIdx = BeginIdx + NumKeyFields;
    SortInRange(Fields, BeginIdx, EndIdx);

    // Print Fields after sort in range of the key fields
    LLVM_DEBUG({
      dbgs() << "Fields after 2nd sort within key-field range:"
                "\t(SkipField0: "
             << SkipField0 << ")\n";
      if (SkipField0)
        FD0.dump();
      for (auto &Field : Fields) {
        Field.dump(true);
      }
    });

    // Minimize connection padding by adjusting field(s) at the border
    unsigned TopAlignToMatch =
        SkipField0 ? FD0.getSize() : Fields[BeginIdx - 1].getSize();
    AdjustBorderFields(Fields, TopAlignToMatch, BeginIdx, EndIdx - 1);

    // Print Fields after minimizing connection padding
    LLVM_DEBUG({
      dbgs() << "Fields after minimize connection padding:\t(SkipField0: "
             << SkipField0 << ")\n";
      if (SkipField0)
        FD0.dump();
      for (auto &Field : Fields) {
        Field.dump(true);
      }
    });

  } else {
    // All unhandled cases
    llvm_unreachable("unsupported case");
    return false;
  }

  // Print Fields after sorting
  LLVM_DEBUG({
    dbgs() << "Fields after sort and swap:\t(SkipField0: " << SkipField0
           << ")\n";
    if (SkipField0)
      FD0.dump();
    for (auto &Field : Fields)
      Field.dump(true);
  });

  // Compute size of reordered layout
#ifndef NDEBUG
  std::vector<unsigned> OffsetTrackV;
#endif
  uint64_t Offset = 0;
  if (SkipField0) {
    Offset += FD0.getSize();
    LLVM_DEBUG({ OffsetTrackV.push_back(Offset); });
  }

  for (const FieldData &FD : Fields) {
    uint64_t TyAlign = FD.getAlign();
    if ((Offset & (TyAlign - 1)) != 0)
      Offset = alignTo(Offset, TyAlign);

    Offset += FD.getSize();
    LLVM_DEBUG({ OffsetTrackV.push_back(Offset); });
  }

  uint64_t StructAlign = DL.getABITypeAlignment(LLVMStructT);
  if ((Offset & (StructAlign - 1)) != 0) {
    Offset = alignTo(Offset, StructAlign);
    LLVM_DEBUG({ OffsetTrackV.push_back(Offset); });
  }

  // See the contents of OffsetTrackV
  LLVM_DEBUG({
    dbgs() << "Offset track:\n";
    for (unsigned I = 0, E = OffsetTrackV.size(); I < E; ++I) {
      dbgs() << I << " : " << OffsetTrackV[I] << "\n";
    }
  });

  // Check if profitable after field reordering:
  const uint64_t OrigStructSize = DL.getTypeAllocSize(LLVMStructT);
  int64_t SpaceDiff = OrigStructSize - Offset;

  const unsigned Threshold =
      IsSimpleStructType
          ? DTransReorderFieldsOPSavedSpacePercentThreshold
          : DTransReorderFieldsOPSavedSpacePercentComplexThreshold;

  if (IsSimpleStructType) {
    if (SpaceDiff > 0) {
      // Struct size shrink: ensure the savings is big enough
      if (OrigStructSize <= Offset ||
          (SpaceDiff * 100) / OrigStructSize < Threshold) {
        LLVM_DEBUG(dbgs() << "  Not profitable to reorder: "
                          << getStName(LLVMStructT)
                          << " ( Size: " << DL.getTypeAllocSize(LLVMStructT)
                          << " SpaceSaved: " << SpaceDiff << " )\n");
        return false;
      }
    }
    // for Simple profit model:
    // if there is no space savings, it is NOT profitable.
    else
      return false;
  }
  // For Advanced Profit model: allow limited size growth
  else {
    // If struct size grows: ensure the growth is within a pre-defined bound
    if ((SpaceDiff < 0) &&
        (std::abs(SpaceDiff) * 100) / OrigStructSize > Threshold) {
      LLVM_DEBUG(dbgs() << "  Not profitable to reorder: "
                        << getStName(LLVMStructT)
                        << " ( Size: " << DL.getTypeAllocSize(LLVMStructT)
                        << " SpaceSaved: " << SpaceDiff << " )\n");
      return false;
    }
  }

  LLVM_DEBUG({
    dbgs() << getStName(LLVMStructT) << " selected for Field-reorder: "
           << " ( Size: " << DL.getTypeAllocSize(LLVMStructT)
           << ", SpaceSaved: " << SpaceDiff << " )\n";

    for (auto &FD : Fields) {
      FD.dump(false);
    }
  });

  // StructType is profitable after reordering,
  // record field reorder index mapping for later transformation.
  RTI.setTransformedTypeNewSize(StructT, Offset);
  const unsigned Size = SkipField0 ? Fields.size() + 1 : Fields.size();
  std::vector<uint32_t> NewIdxVec(Size);
  uint32_t NewIndex = 0;
  if (SkipField0) {
    NewIdxVec[0] = 0;
    NewIndex = 1;
  }

  for (const FieldData &FD : Fields)
    NewIdxVec[FD.getIndex()] = NewIndex++;
  RTI.setTransformedIndexes(LLVMStructT, NewIdxVec);

  // Check what we have in RTI:
  LLVM_DEBUG({
    dbgs() << "RTI Dump:\n";
    RTI.dump();
  });

  return true;
}

// Returns true if a given StructType * is applicable for DTrans's Field-Reorder
// Optimization.
//
// This checks the property on the said StructType:
// - not packed already
// - each field is NOT an aggregate or vector type (only simple types)
// - check additional filter conditions
//
bool ReorderFieldsAnalyzer::isApplicable(dtrans::TypeInfo *TI,
                                         const DataLayout &DL) {
  // Bypass the applicable test?
  if (!EnableReorderFieldOPApplicableTest || !EnableReorderFieldOPTests)
    return true;

  auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
  if (!StInfo)
    return false;
  StructType *StructT = cast<StructType>(StInfo->getLLVMType());

  // Check if the type is packed
  if (StructT->isPacked())
    return false;

  const size_t StructSize = DL.getTypeAllocSize(StructT);
  const uint32_t NumElems = StructT->getNumElements();
  if (StructSize > MaxStructSize || NumElems < MinNumElems ||
      NumElems > MaxNumElems)
    return false;

  // Skip if no fields of struct are accessed.
  if (StInfo->getTotalFrequency() <= 0)
    return false;

  if (isSimpleStructType(TI) || isAdvancedStructType(TI))
    return true;

  return false;
}

bool ReorderFieldsAnalyzer::isLegal(dtrans::TypeInfo *TI,
                                    DTransSafetyInfo *DTInfo) {
  // Bypass the legal test?
  if (!EnableReorderFieldOPLegalTest || !EnableReorderFieldOPTests)
    return true;

  auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
  if (!StInfo)
    return false;

  // **Simple Struct Legal Test**
  if (isSimpleStructType(TI)) {
    if (DTInfo->testSafetyData(TI, dtrans::DT_ReorderFields)) {
      LLVM_DEBUG(dbgs() << "  Rejecting "
                        << getStName(cast<StructType>(StInfo->getLLVMType()))
                        << " based on safety data.\n");
      return false;
    }
    return true;
  }

  // Default: illegal
  return false;
}

// Collect all potential StructType * as Reorder-Field candidates
bool ReorderFieldsAnalyzer::doCollection(Module &M, DTransSafetyInfo *DTInfo) {
  LLVM_DEBUG(
      dbgs() << "Reorder fields OP: looking for candidate structures.\n");

  const DataLayout &DL = M.getDataLayout();
  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    // Legal Test
    if (!isLegal(TI, DTInfo)) {
      LLVM_DEBUG(dbgs() << "Reject "
                        << getStName(cast<StructType>(TI->getLLVMType()))
                        << " type due to failed legal test. \n";);
      continue;
    }

    // Applicable Test
    if (!isApplicable(TI, DL)) {
      LLVM_DEBUG(dbgs() << "Reject "
                        << getStName(cast<StructType>(TI->getLLVMType()))
                        << " type due to failed applicable test. \n";);
      continue;
    }

    // Profitable Test
    if (!isProfitable(TI, DL)) {
      LLVM_DEBUG(dbgs() << "Reject "
                        << getStName(cast<StructType>(TI->getLLVMType()))
                        << " type due to failed profit test. \n";);
      continue;
    }

    // Save Candidate type if all tests pass
    CandidateTypeV.push_back(StInfo);
  }

  LLVM_DEBUG({
    if (CandidateTypeV.empty())
      dbgs() << "  No candidates found.\n";
    else
      dbgs() << CandidateTypeV.size() << " candidates found.\n";

    dumpCandidateTypes();
  });

  return !CandidateTypeV.empty();
}

} // end anonymous namespace

char DTransReorderFieldsOPWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransReorderFieldsOPWrapper, "dtrans-reorderfieldsop",
                      "DTrans reorder fields with opaque pointer support",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(DTransSafetyAnalyzerWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransReorderFieldsOPWrapper, "dtrans-reorderfieldsop",
                    "DTrans reorder fields with opaque pointer support", false,
                    false)

ModulePass *llvm::createDTransReorderFieldsOPWrapperPass() {
  return new DTransReorderFieldsOPWrapper();
}

PreservedAnalyses ReorderFieldsOPPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  DTransSafetyInfo *DTInfo = &AM.getResult<DTransSafetyAnalyzer>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  bool Changed = runImpl(M, DTInfo, GetTLI, WPInfo);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool ReorderFieldsOPPass::runImpl(
    Module &M, DTransSafetyInfo *DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;
  if (!DTInfo->useDTransSafetyAnalysis())
    return false;

  // *Collection*
  // - scan each StructType in module
  // - do legal test, profit test, and applicable test
  // - collect all suitable Candidate struct types into CandidateTypeV
  auto &DL = M.getDataLayout();
  ReorderFieldsAnalyzer Analyzer;
  if (!Analyzer.doCollection(M, DTInfo))
    return false;

  // Check if any type is selected for field-reordering.
  ReorderFieldTransInfo &RTI = Analyzer.getRTI();
  if (!RTI.hasAnyTypeTransformed())
    return false;

  ReorderFieldsOPImpl ReorderFieldsImpl(DL, GetTLI, RTI, DTInfo, M.getContext(),
                                        "__DFR_");
  bool RunResult = ReorderFieldsImpl.run(M);
  return RunResult;
}
