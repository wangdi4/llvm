//===---------------- ReorderFields.cpp - DTransReorderFieldsPass ---------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Field Reorder (DFR) optimization pass.
//
// Will use DFR to refer to DTrans Field Reorder optimization in the upcoming comments.
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ReorderFields.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "dtrans-reorderfields"

STATISTIC(NumReorderFieldTransformed,
          "Number of structs with reorder-field transformation triggered");

// Flag to enable/disable DTrans::ReorderField pass.
// Default is on (enabled).
static cl::opt<bool> EnableReorderField("dtrans-reorderfield-enable",
                                        cl::init(true), cl::ReallyHidden);

// Flag to enable/disable DTrans::ReorderField pass with advanced optimizations.
// Default is off (disabled).
// The current legal test is not entire safe. Work is needed to improve
// DTransAnalysis on detecting legal conditions for DTrans' ReorderFields.
static cl::opt<bool> EnableReorderFieldAdv("dtrans-reorderfield-enable-adv",
                                           cl::init(false),
                                           cl::ReallyHidden);

// Flag to enable/disable DTrans::ReorderField legal test.
// Default is on (enabled).
static cl::opt<bool>
    EnableReorderFieldLegalTest("dtrans-reorderfield-enable-legal-test",
                                cl::init(true), cl::ReallyHidden);

// Flag to enable/disable DTrans::ReorderField applicable test.
// Default is on (enabled).
static cl::opt<bool> EnableReorderFieldApplicableTest(
    "dtrans-reorderfield-enable-applicable-test", cl::init(true),
    cl::ReallyHidden);

// Flag to enable/disable DTrans::ReorderField profitable test.
// Default is on (enabled).
static cl::opt<bool> EnableReorderFieldProfitableTest(
    "dtrans-reorderfield-enable-profitable-test", cl::init(true),
    cl::ReallyHidden);

// Flag to enable/disable all DTrans::ReorderField tests, including legal test,
// profitable test, and applicable test.
// Default is on (enabled).
static cl::opt<bool> EnableReorderFieldTests("dtrans-reorderfield-enable-tests",
                                             cl::init(true), cl::ReallyHidden);

// Enable Padding based heuristic to select candidates for "Reorder Fields".
static cl::opt<bool> DTransReoderFieldsPaddingHeuristic(
    "dtrans-reorder-fields-padding-heuristic", cl::init(true),
    cl::ReallyHidden);

// Minimum unused space (padding) percent threshold to select candidate
// structure for reorder fields.
static cl::opt<unsigned> DTransReorderFieldsUnusedSpacePercentThreshold(
    "dtrans-reorder-fields-unused-space-percent-threshold", cl::init(6),
    cl::ReallyHidden);

// Field reordering may not help to utilize all unused space in the original
// structure layout. This flag is used to avoid applying reordering
// transformation if reordering is not profitable. Minimum saved space
// percent threshold with field reordering to enable the transformation.
static cl::opt<unsigned> DTransReorderFieldsSavedSpacePercentThreshold(
    "dtrans-reorder-fields-saved-space-percent-threshold", cl::init(13),
    cl::ReallyHidden);

// Percent threshold used for non-simple struct.
// Otherise same as DTransReorderFieldsSavedSpacePercentThreshold.
static cl::opt<unsigned> DTransReorderFieldsSavedSpacePercentComplexThreshold(
    "dtrans-reorder-fields-saved-space-percent-complex-threshold", cl::init(7),
    cl::ReallyHidden);

// Max number of fields in a struct that maybe considered SIMPLE.
static cl::opt<unsigned> DTransReorderFieldsNumFieldsThreshold(
    "dtrans-reorder-fields-numfields-threshold", cl::init(15),
    cl::ReallyHidden);

// Number of fields in a struct to be considered as Advanced StructType
static cl::opt<unsigned> DTransReorderFieldsNumFieldsAdvPrecise(
    "dtrans-reorder-fields-numfields-adv-precise", cl::init(20),
    cl::ReallyHidden);

// Begin index for 1st pair of fields to swap, in order to relocate key fields
// to the beginning of the struct.
static cl::opt<unsigned>
    DTransReorderFieldsSwapIndex1Begin("dtrans-reorder-fields-swapidx1-begin",
                                       cl::init(3), cl::ReallyHidden);

// End index for 1st pair of fields to swap, in order to relocate key fields
// to the end of the struct.
static cl::opt<unsigned>
    DTransReorderFieldsSwapIndex1End("dtrans-reorder-fields-swapidx1-end",
                                     cl::init(18), cl::ReallyHidden);

// Begin index for 2nd pair of fields to swap, in order to relocate key fields
// to the beginning of the struct.
static cl::opt<unsigned>
    DTransReorderFieldsSwapIndex2Begin("dtrans-reorder-fields-swapidx2-begin",
                                       cl::init(4), cl::ReallyHidden);

// End index for 2nd pair of fields to swap, in order to relocate key fields
// to the end of the struct.
static cl::opt<unsigned>
    DTransReorderFieldsSwapIndex2End("dtrans-reorder-fields-swapidx2-end",
                                     cl::init(5), cl::ReallyHidden);

// Expected number of integer-type fields within the target struct type
static cl::opt<unsigned> DTransReorderFieldsComplexStructIntTypeCount(
    "dtrans-reorder-fields-complexstruct-int-counts", cl::init(12),
    cl::ReallyHidden);

// Expected number of struct-type fields within the target struct type
static cl::opt<unsigned> DTransReorderFieldsComplexStructStructTypeCount(
    "dtrans-reorder-fields-complexstruct-struct-counts", cl::init(5),
    cl::ReallyHidden);

// Expected number of pointer-type fields within the target struct type
static cl::opt<unsigned> DTransReorderFieldsComplexStructPionterTypeCount(
    "dtrans-reorder-fields-complexstruct-ptr-counts", cl::init(3),
    cl::ReallyHidden);

// Limit size of structs that are selected for reordering fields based
// on padding heuristic. Field affinity info may be needed to apply
// reordering for structs that don’t fit cache Line.
// "TargetTransformInfo::getCacheLineSize" can be used instead MaxStructSize
// after doing more experiments.
static const unsigned MaxStructSize = 160;

// Minimum number of fields to select a struct as candidate.
static const unsigned MinNumElems = 3;

// Maximum number of fields to select a struct as candidate.
static const unsigned MaxNumElems = 20;

// number of key fields:
static const unsigned NumKeyFields = 4;

namespace {

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD StringRef getStName(StructType *StTy) {
  return StTy->hasName() ? StTy->getName() : "<unnamed struct>";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

class DTransReorderFieldsWrapper : public ModulePass {
private:
  dtrans::ReorderFieldsPass Impl;

public:
  static char ID;

  DTransReorderFieldsWrapper() : ModulePass(ID) {
    initializeDTransReorderFieldsWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    DTransAnalysisWrapper &DTAnalysisWrapper =
        getAnalysis<DTransAnalysisWrapper>();
    DTransAnalysisInfo &DTInfo = DTAnalysisWrapper.getDTransInfo(M);
    auto GetTLI = [this](const Function &F) -> TargetLibraryInfo & {
      return this->getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    };

    bool Changed = Impl.runImpl(
        M, DTInfo, GetTLI, getAnalysis<WholeProgramWrapperPass>().getResult());

    if (Changed)
      DTAnalysisWrapper.setInvalidated();

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<DTransAnalysisWrapper>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransReorderFieldsWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransReorderFieldsWrapper, "dtrans-reorderfields",
                      "DTrans reorder fields", false, false)
  INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
  INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
  INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(DTransReorderFieldsWrapper, "dtrans-reorderfields",
                    "DTrans reorder fields", false, false)

ModulePass *llvm::createDTransReorderFieldsWrapperPass() {
  return new DTransReorderFieldsWrapper();
}

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

namespace llvm {

namespace dtrans {

// Map InclusiveStructType(s) on both ways
bool ReorderFieldTransInfo::doInclusiveStructTypeMap(
    DTransTypeRemapper *TypeRemapper, Function *F) {
  // Ensure: map ONCE only for F
  if (InclusiveStructTypeMapped[F])
    return true;

  // Do the mapping
  for (StructType *OrigTy : InclusiveStructTypeV) {
    // Skip the OrigTy if it is already mapped
    if (InclusiveStructTypeMap[OrigTy])
      continue;

    StructType *ReorderedTy =
        dyn_cast<StructType>(TypeRemapper->lookupTypeMapping(OrigTy));

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

#endif //#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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
class ReorderFieldsImpl : public DTransOptBase {
public:
  ReorderFieldsImpl(
      ReorderFieldTransInfo &RTI, DTransAnalysisInfo &DTInfo,
      LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(&DTInfo, Context, DL, GetTLI, DepTypePrefix,
                      TypeRemapper),
        RTI(RTI) {}

  virtual bool prepareTypes(Module &M) override;
  virtual void populateTypes(Module &M) override;
  virtual void processFunction(Function &F) override;
  virtual void postprocessFunction(Function &Func, bool isCloned) override;

private:
  ReorderFieldTransInfo &RTI;
  TypeToTypeMap Orig2NewTypeMap;
  TypeToTypeMap New2OrigTypeMap; // reverse map, for easier lookup

  void processBinaryOperator(BinaryOperator &BO);
  void processCallInst(CallInst &CI);
  void transformDivOp(BinaryOperator &I);
  void processGetElementPtrInst(GetElementPtrInst &GEP);
  void processByteFlattenedGetElementPtrInst(GetElementPtrInst &GEP);
  void transformAllocCall(CallInst &CI, StructType *OrigTy,
                          AllocCallInfo *CInfo);
  void transformMemfunc(CallInst &CI, StructType *Ty);
  bool replaceOldSizeWithNewSize(Value *Val, uint64_t OldSize, uint64_t NewSize,
                                 Instruction *I, uint32_t APos);
  bool replaceOldSizeWithNewSizeForConst(Value *Val, uint64_t OldSize,
                                         uint64_t NewSize, Instruction *I,
                                         uint32_t APos);
  void replaceOldValWithNewVal(Instruction *I, uint32_t APos, Value *NewVal);
  StructType *getAssociatedOrigTypeOfSub(Value *SubV);
  Type *getOrigTyOfTransformedType(Type *TType);
  StructType *getStructTyAssociatedWithCallInfo(CallInfo *CI);
  StructType *unmapInclusiveType(CallInfo *CI);

  bool collectInclusiveStructTypes(const Module &M);
  bool findInclusiveStructType(
      StructType *BaseInclStTy,
      std::vector<StructType *> &DerivedInclusiveStructTypeV);

  bool checkDependentTypeSafety(llvm::Type *Ty);

public:
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dumpRTI() const { RTI.dump(); }

  LLVM_DUMP_METHOD void dumpTypeMap() const {
    unsigned Count = 0;
    for (auto &Pair : Orig2NewTypeMap) {
      Type *OrigTy = Pair.first;
      Type *ReplTy = Pair.second;
      dbgs() << Count++ << " OrigTy: " << *OrigTy
             << "\n->\n   ReplTy: " << *ReplTy << "\n";
    }
  }

  LLVM_DUMP_METHOD void dumpReverseTypeMap() const {
    unsigned Count = 0;
    for (auto &Pair : New2OrigTypeMap) {
      Type *ReplTy = Pair.first;
      Type *OrigTy = Pair.second;
      dbgs() << Count++ << " ReplTy: " << *ReplTy
             << "\n->\n   OrigTy: " << *OrigTy << "\n";
    }
  }

  LLVM_DUMP_METHOD void dump() const {
    dumpRTI();
    dumpTypeMap();
    dumpReverseTypeMap();
  }
#endif // NDEBUG
};

// Helper function to get associated StructType of \p CallInfo.
StructType *ReorderFieldsImpl::getStructTyAssociatedWithCallInfo(CallInfo *CI) {
  for (auto *PTy : CI->getPointerTypeInfoRef().getTypes()) {
    Type *StTy = PTy->getPointerElementType();
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
Type *ReorderFieldsImpl::getOrigTyOfTransformedType(Type *TType) {
  // Search Orig2NewTypeMap:
  for (auto &TypesPair : Orig2NewTypeMap) {
    Type *OrigTy = TypesPair.first;
    Type *NewTy = TypesPair.second;
    LLVM_DEBUG(dbgs() << "OrigTy: " << *OrigTy << "\nNewTy: " << *NewTy
                      << "\n";);
    if (NewTy == TType || OrigTy == TType)
      return OrigTy;
  }

  return nullptr;
}

StructType *ReorderFieldsImpl::unmapInclusiveType(CallInfo *CI) {
  // Build mapping for types in InclusiveStructTypeV only ONCE!
  Function *F = CI->getInstruction()->getParent()->getParent();
  if (!RTI.doInclusiveStructTypeMap(TypeRemapper, F)) {
    assert(0 && "Failed mapInclusiveStructTypes()\n");
    return nullptr;
  }

  // Find the OrigTy of a matching ReorderedTy
  for (auto *Ty : CI->getPointerTypeInfoRef().getTypes()) {
    StructType *ReorderedTy = dyn_cast<StructType>(Ty->getPointerElementType());

    // Search InclusiveStructTypeUnmap
    // map ReorderedTy -> OrigTy
    StructType *OrigTy = RTI.unmapInclusiveStructType(ReorderedTy);
    if (OrigTy)
      return OrigTy;
  }

  return nullptr;
}

// Sets \p NewVal as \p APos th operand of \p I. This routine expects
// only CallInst and SDiv/UDiv instructions.
void ReorderFieldsImpl::replaceOldValWithNewVal(Instruction *I, uint32_t APos,
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
bool ReorderFieldsImpl::replaceOldSizeWithNewSizeForConst(Value *Val,
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
bool ReorderFieldsImpl::replaceOldSizeWithNewSize(Value *Val, uint64_t OldSize,
                                                  uint64_t NewSize,
                                                  Instruction *I,
                                                  uint32_t APos) {
  if (!Val)
    return false;

  // Check if Val is multiple of OldSize.
  if (!isValueMultipleOfSize(Val, OldSize))
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
void ReorderFieldsImpl::transformMemfunc(CallInst &CI, StructType *Ty) {
  Value *SizeVal = CI.getArgOperand(2);
  LLVM_DEBUG(dbgs() << "Memfunc Before:" << CI << "\n");

  bool Replaced =
      replaceOldSizeWithNewSize(SizeVal, DL.getTypeAllocSize(Ty),
                                RTI.getTransformedTypeNewSize(Ty), &CI, 2);
  assert(Replaced == true &&
      "Expecting oldSize should be replaced with NewSize");

  (void) Replaced;
  LLVM_DEBUG(dbgs() << "Memfunc After:" << CI << "\n");
}

static bool getConstantFromValue(Value *V, uint64_t &Val) {
  auto *CInt = dyn_cast<ConstantInt>(V);
  if (!CInt)
    return false;

  Val = CInt->getValue().getZExtValue();

  return true;
}

// Fix size argument of calloc/malloc/realloc
void ReorderFieldsImpl::transformAllocCall(CallInst &CI, StructType *OrigTy,
                                           AllocCallInfo *CInfo) {
  AllocKind Kind = CInfo->getAllocKind();
  LLVM_DEBUG(dbgs() << "Alloc Before:" << CI << "\n");

  unsigned SizeArgPos = 0;
  unsigned CountArgPos = 0;
  const TargetLibraryInfo &TLI = GetTLI(*CI.getFunction());
  getAllocSizeArgs(Kind, &CI, SizeArgPos, CountArgPos, TLI);

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
  // there are corner cases where the size appearing under the malloc instruction
  // is not a direct multiple of the OrigTypeSize.
  //
  // E.g.
  // %1 = tail call i8* @_Znwm(i64 208) #34
  // %1 will be used to store %class.cPacket type, but size of %class.cPacket
  // is actually 192 instead of 208.
  //
  // For not-clear reason(s), the allocated size is 16B larger than its matching type size.
  // Size of Original %class.cPacket is 192, size of reordered %class.cPacket is 200.
  // It becomes 8B larger.
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
void ReorderFieldsImpl::processGetElementPtrInst(GetElementPtrInst &GEP) {
  Type *SourceTy = GEP.getSourceElementType();
  if (!isa<StructType>(SourceTy))
    return;

  // Get original struct type (before mapping)
  // Attempt: is OrigStTy a relevant Inclusive Struct Type?
  StructType *OrigStTy = dyn_cast_or_null<StructType>(
      getOrigTyOfTransformedType(SourceTy));
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

  // If the GEP refers to a field inside a StructType being reordered, replace the proper
  // indices.
  const unsigned TotalIdx = std::distance(GEP.idx_begin(), GEP.idx_end());
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
    if (!New2OrigTypeMap.count(IndexedTy))
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
void ReorderFieldsImpl::processByteFlattenedGetElementPtrInst(
    GetElementPtrInst &GEP) {

  // Only two operands are expected for ByteFlattened GEPs
  if (GEP.getNumOperands() != 2)
    return;

  auto GEPInfo = DTInfo->getByteFlattenedGEPElement(&GEP);
  if (!GEPInfo.first)
    return;

  Type *SourceTy = GEPInfo.first;
  StructType *StructTy = dyn_cast<StructType>(SourceTy);
  if (!StructTy)
    return;

  LLVM_DEBUG({ GEP.dump(); });

  // Byte-Flatten GEP -- BFGEP
  bool BFGEPHandled = false;
  uint64_t NewOffset = 0;
  auto TypesMap = RTI.getTypesMap();

  // OrigTy is a DTrans's Field-Reorder (DFR) transformed type
  if (TypesMap.find(StructTy) != TypesMap.end()) {
    StructType *OrigTy = StructTy;
    StructType *NewTy = cast<StructType>(Orig2NewTypeMap[OrigTy]);
    if (!NewTy)
      return;
    uint32_t OldIdx = GEPInfo.second;
    uint32_t NewIdx = RTI.getTransformedIndex(OrigTy, OldIdx);

    // Get offset of the field in new layout.
    auto *SL = DL.getStructLayout(NewTy);
    NewOffset = SL->getElementOffset(NewIdx);

    BFGEPHandled = true;

  } else if (RTI.hasInclusiveStructType(StructTy)) {
    // StructTy is a valid Inclusive StructType, but not a DTrans's
    // Field-Reorder (DFR) transformed type
    uint64_t OffsetVal = 0;
    if (!getConstantFromValue(GEP.getOperand(1), OffsetVal))
      return;

    StructType *OrigTy = RTI.getDFRCandidateStructType(StructTy);
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
      uint32_t NewIdx = RTI.getTransformedIndex(OrigTy, OldIdx);
      StructType *NewTy = cast<StructType>(Orig2NewTypeMap[OrigTy]);
      if (!NewTy)
        return;

      // Get offset of the field in new layout.
      auto *SL = DL.getStructLayout(NewTy);
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
      StructType *NewTy = cast<StructType>(Orig2NewTypeMap[OrigTy]);
      if (!NewTy)
        return;
      const unsigned NewTySize = DL.getTypeStoreSize(NewTy);
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

// Gets StructType associated with \p SubV if it is subtraction
// of pointers to a structure. Otherwise, it returns nullptr.
StructType *ReorderFieldsImpl::getAssociatedOrigTypeOfSub(Value *SubV) {
  // FIXME: This is just temporary fix to get type info of operands
  // of Instruction::Sub. Once LPI is available in transformations,
  // get type info using LPI instead of this work around.
  if (!isa<Instruction>(SubV) || !SubV->hasOneUse())
    return nullptr;

  auto *I = cast<Instruction>(SubV);

  if (I->getOpcode() != Instruction::Sub)
    return nullptr;

  if (I->getOperand(0)->getType() != I->getOperand(1)->getType())
    return nullptr;

  if (!isa<PtrToIntInst>(I->getOperand(0)))
    return nullptr;

  PtrToIntInst *Src = cast<PtrToIntInst>(I->getOperand(0));
  Type *PtrTy = Src->getPointerOperand()->getType();
  if (!isa<PointerType>(PtrTy))
    return nullptr;

  Type *StTy = PtrTy->getPointerElementType();

  // Get original struct Type from transformed type.
  if (Type *OrigStTy = getOrigTyOfTransformedType(StTy))
    return cast<StructType>(OrigStTy);

  // Search Inclusive Struct Type:
  if (StructType *OrigInclusiveStructTy =
      RTI.unmapInclusiveStructType(cast<StructType>(StTy)))
    return OrigInclusiveStructTy;

  return nullptr;
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
void ReorderFieldsImpl::transformDivOp(BinaryOperator &I) {
  assert((I.getOpcode() == Instruction::SDiv ||
      I.getOpcode() == Instruction::UDiv) &&
      "Unexpected opcode");
  Value *SubI = I.getOperand(0);

  StructType *StructTy = getAssociatedOrigTypeOfSub(SubI);
  if (!StructTy)
    return;

  auto TypesMap = RTI.getTypesMap();

  // StructTy is a DTrans's Field-Reorder (DFR) transformed type
  if (TypesMap.find(StructTy) != TypesMap.end()) {
    LLVM_DEBUG(dbgs() << "SDiv/UDiv Before:" << I << "\n");
    Value *SizeVal = I.getOperand(1);
    bool Replaced =
        replaceOldSizeWithNewSize(SizeVal, DL.getTypeAllocSize(StructTy),
                                  RTI.getTransformedTypeNewSize(StructTy), &I,
                                  1);
    assert(Replaced == true &&
        "Expecting oldSize should be replaced with NewSize");

    (void) Replaced;
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

    (void) Replaced;
    LLVM_DEBUG(dbgs() << "SDiv/UDiv After:" << I << "\n");

  } else {
    // Unknown case
    llvm_unreachable("Unknown BinOp case");
  }
}

// Only Pointer arithmetic that is allowed in Analysis is Instruction::Sub
// that is used by SDiv.
void ReorderFieldsImpl::processBinaryOperator(BinaryOperator &BO) {
  switch (BO.getOpcode()) {
  case Instruction::SDiv:
  case Instruction::UDiv:transformDivOp(BO);
    break;
  default:break;
  }
}

// Handle each relevant CallInst.
// The only CallInst instructions that need to be handled by Reordering are
// Alloc and Memfunc.
//
// There is no need to process any GEP that is passed as an argument to a call
// since reordering is disabled as it is treated as AddressTaken.
//
void ReorderFieldsImpl::processCallInst(CallInst &CI) {
  CallInfo *CInfo = DTInfo->getCallInfo(&CI);
  if (CInfo == nullptr)
    return;

  StructType *ReorderTy = getStructTyAssociatedWithCallInfo(CInfo);
  StructType *OrigTy = unmapInclusiveType(CInfo);
  if (!ReorderTy && !OrigTy)
    return;
  StructType *StructTy = ReorderTy ? ReorderTy : OrigTy;

  switch (CInfo->getCallInfoKind()) {
  case CallInfo::CIK_Alloc: {
    transformAllocCall(CI, StructTy, cast<AllocCallInfo>(CInfo));
    break;
  }
  case CallInfo::CIK_Memfunc: {
    transformMemfunc(CI, StructTy);
    break;
  }
  default: break;
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
void ReorderFieldsImpl::postprocessFunction(Function &Func, bool isCloned) {
  Function *F = isCloned ? OrigFuncToCloneFuncMap[&Func] : &Func;

  // Map relevant Inclusive Struct Types for F only if clone happens
  if (isCloned)
    RTI.doInclusiveStructTypeMap(TypeRemapper, F);

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
void ReorderFieldsImpl::processFunction(Function &F) {
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst))
      processByteFlattenedGetElementPtrInst(*GEP);
  }
}

// Create new types based on reordered field layout.
void ReorderFieldsImpl::populateTypes(Module &M) {
  LLVM_DEBUG(dbgs() << "ReorderFieldsImpl::populateTypes(M)\n");

  for (auto &TyPair : RTI.getTypesMap()) {
    auto OrigTy = TyPair.first;
    std::vector<Type *> EltTys(OrigTy->getNumElements());

    // Create reordered field layout
    for (unsigned I = 0, E = OrigTy->getNumElements(); I < E; ++I) {
      Type *Ty = OrigTy->getElementType(I);
      unsigned NewIdx = RTI.getTransformedIndex(OrigTy, I);
      EltTys[NewIdx] = TypeRemapper->remapType(Ty);
    }

    // Setup replacement type:
    StructType *NewTy = cast<StructType>(Orig2NewTypeMap[OrigTy]);
    NewTy->setBody(EltTys);

    LLVM_DEBUG(dbgs() << "Struct " << getStName(OrigTy)
                      << " after field-reordering: \n"
                      << *NewTy << "\n");
    assert(RTI.getTransformedTypeNewSize(OrigTy) ==
        DL.getTypeAllocSize(NewTy) &&
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
// If class.A is a base IST, then class.B, class.C, and class.D are all derived ISTs.
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
#if 0
class C{...};
class B: public A{
  ....
};

%class.B = {%class.A, ....}
%class.A = {%class.C, ....}
%class.D = {%class.B, ....}
#endif
bool ReorderFieldsImpl::findInclusiveStructType(
    StructType *BaseInclStTy,
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
  auto Ty2DepTySetVector = TypeToDependentTypes[BaseInclStTy];
  if (Ty2DepTySetVector.empty())
    // No mapping available for BaseInclStTy
    return false;

  // Save each qualified mapping into QualifiedStTyV
  std::vector<StructType *> QualifiedStTypeV;
  QualifiedStTypeV.clear();
  for (auto *Ty: Ty2DepTySetVector) {
    StructType *StTy = dyn_cast<StructType>(Ty);
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
                      << "QualifiedStTypeV <: " << QualifiedStTypeV.size()
                      << ">\n";
               unsigned Count = 0;
               for (auto StTy: QualifiedStTypeV) {
                 dbgs() << Count++ << " : " << *StTy << "\n";
               }
             });
  std::copy(QualifiedStTypeV.begin(), QualifiedStTypeV.end(),
            std::back_inserter(DerivedInclusiveStructTypeV));

  for (auto StTy: QualifiedStTypeV) {
    LLVM_DEBUG(dbgs() << *StTy << "\n";);
    findInclusiveStructType(StTy, DerivedInclusiveStructTypeV);
  }

  LLVM_DEBUG({
               dbgs() << "BaseInclStTy: " << *BaseInclStTy << "\n"
                      << "DerivedInclusiveStructTypeV <: "
                      << DerivedInclusiveStructTypeV.size() << ">\n";
               unsigned Count = 0;
               for (auto StTy: DerivedInclusiveStructTypeV) {
                 dbgs() << Count++ << " : " << *StTy << "\n";
               }
             });

  return true;
}

// Collect all inclusive Module-level StructType * into InclusiveStructTypeV.
// Return true if there is any inclusive struct type(s) collected.
//
bool ReorderFieldsImpl::collectInclusiveStructTypes(const Module &M) {
  std::vector<StructType *> DerivedInclusiveStructTypeV;

  // For each valid OrigTy (from RTI), find all Inclusive Struct Types,
  // and save them into DerivedInclusiveStructTypeV.
  for (auto &TyPair : RTI.getTypesMap()) {
    StructType *BaseInclStTy = TyPair.first;
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
bool ReorderFieldsImpl::checkDependentTypeSafety(llvm::Type *Ty) {
  auto *TI = DTInfo->getTypeInfo(Ty);
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
// If any test on a DFR candidate's inclusive struct type fails, this routine
// returns false.
// As a result, DTrans Field Reorder (DFR) on the respective struct type
// candidate will fail.
//
// This is a side exit inside the DTrans Optimization Framework.
//
// It is necessary to locate the 2nd legal test in the prepareTypes() routine
// because the test needs to access TypeToDependenTypes map which is only
// available inside an overwritten virtual function from a sub class of
// DtransOptBase type.
//
bool ReorderFieldsImpl::prepareTypes(Module &M) {
  LLVM_DEBUG(dbgs() << "ReorderFieldsImpl::prepareTypes(M)\n");

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
      // Skip any non StructType DepTy
      if (!isa<llvm::StructType>(InclStTy))
        continue;

      // Skip any dependent type that is subject to DFR transformation.
      if (RTI.hasTransformedTypeNewSize(InclStTy))
        continue;

      // Legal test on InclStTy:
      if (!checkDependentTypeSafety(InclStTy)) {
        LLVM_DEBUG(
            dbgs() << "DTRANS-FieldsReordering: Disqualified based on safety"
                      " conditions of dependent inclusive type: "
                   << getStName(cast<StructType>(InclStTy)) << *InclStTy
                   << "\n");
        return false;
      }
    }
  }

  // Create opaque type place holder for each to-be reordered struct type
  for (auto &TyPair : RTI.getTypesMap()) {
    StructType *OrigTy = TyPair.first;
    StructType *NewTy = StructType::create(OrigTy->getContext(),
                                           "__DFR_" + OrigTy->getName().str());
    TypeRemapper->addTypeMapping(OrigTy, NewTy);
    Orig2NewTypeMap[OrigTy] = NewTy;
    New2OrigTypeMap[NewTy] = OrigTy;

    LLVM_DEBUG(dbgs() << "OrigTy: " << *OrigTy << "\n"
                      << "NewTy: " << *NewTy << "\n";);
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
static bool isSimpleStructType(TypeInfo *TI) {
  auto *StInfo = dyn_cast<StructInfo>(TI);
  if (!StInfo)
    return false;
  StructType *StructT = cast<StructType>(StInfo->getLLVMType());

  // a struct with any aggregate type or vector type is rejected
  if (StructT && std::any_of(StructT->element_begin(), StructT->element_end(),
                             isArrayOrVectTy))
    return false;

  if (StructT
      && StructT->getNumElements() < DTransReorderFieldsNumFieldsThreshold)
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
static bool isAdvancedStructType(TypeInfo *TI) {
  auto *StInfo = dyn_cast<StructInfo>(TI);
  if (!StInfo)
    return false;

  StructType *StructT = cast<StructType>(StInfo->getLLVMType());

  // Reject any struct type with a vector-type or array-type field:
  if (StructT && std::any_of(StructT->element_begin(), StructT->element_end(),
                             isArrayOrVectTy))
    return false;

  // Expect the struct to have DTransReorderFieldsNumFieldsAdvPrecise (20) fields
  if (StructT
      && StructT->getNumElements() != DTransReorderFieldsNumFieldsAdvPrecise)
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
      //early bailout: do not expect any other type
      return false;
  }

  return (IntTypeCount == DTransReorderFieldsComplexStructIntTypeCount) && //12
      (StructTypeCount ==
          DTransReorderFieldsComplexStructStructTypeCount) && // 5
      (PointerTypeCount ==
          DTransReorderFieldsComplexStructPionterTypeCount); // 3
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
// If the Struct-Type is profitable, update RTI to record the reorder-field
// index mapping.
//
// [Note]
// Due to the nature of field reordering within a structure, it is possible that
// the size of the struct may grow (becomes bigger) after reordering. As long as
// this size growth is bounded by a pre-defined threshold, it still can be
// profitable.
//
bool ReorderFieldsPass::isProfitable(TypeInfo *TI, const DataLayout &DL) {

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
                                const unsigned TopAlignToMatch,
                                unsigned TopIdx, unsigned BottomIdx) -> bool {
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
  if (!EnableReorderFieldProfitableTest || !EnableReorderFieldTests)
    return true;

  auto *StInfo = dyn_cast<StructInfo>(TI);
  if (!StInfo)
    return false;
  StructType *StructT = cast<StructType>(TI->getLLVMType());

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
  if (isa<StructType>(StructT->getElementType(0))) {
    I = 1;
    SkipField0 = true;
    Type *Ty0 = StructT->getElementType(0);
    dtrans::FieldInfo &FI = StInfo->getField(0);
    FD0.set(DL.getABITypeAlignment(Ty0), DL.getTypeStoreSize(Ty0), 0,
            FI.getFrequency());
  }

  // Collect all fields that will be sorted
  unsigned const E = StructT->getNumElements();
  for (; I < E; ++I) {
    Type *FieldTy = StructT->getElementType(I);
    dtrans::FieldInfo &FI = StInfo->getField(I);
    FieldData FD(DL.getABITypeAlignment(FieldTy), DL.getTypeStoreSize(FieldTy),
                 I, FI.getFrequency());
    Fields.push_back(FD);
  }

  // Print Fields before sorting
  LLVM_DEBUG({
               dbgs() << "Fields before sort:\t(SkipField0: " << SkipField0
                      << ")\n";
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
    std::swap(Fields[DTransReorderFieldsSwapIndex1Begin - AdjustIdx],
              Fields[DTransReorderFieldsSwapIndex1End - AdjustIdx]);
    std::swap(Fields[DTransReorderFieldsSwapIndex2Begin - AdjustIdx],
              Fields[DTransReorderFieldsSwapIndex2End - AdjustIdx]);

    // Print Fields after placing the key fields at the right positions
    LLVM_DEBUG({
                 dbgs() << "Fields after sort and swap:\t(SkipField0: "
                        << SkipField0 << ")\n";
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
                           "\t(SkipField0: " << SkipField0 << ")\n";
                 if (SkipField0)
                   FD0.dump();
                 for (auto &Field : Fields) {
                   Field.dump(true);
                 }
               });

    // Minimize connection padding by adjusting field(s) at the border
    unsigned TopAlignToMatch = SkipField0 ? FD0.getSize()
                                          : Fields[BeginIdx - 1].getSize();
    AdjustBorderFields(Fields, TopAlignToMatch, BeginIdx, EndIdx - 1);

    // Print Fields after minimizing connection padding
    LLVM_DEBUG({
                 dbgs()
                     << "Fields after minimize connection padding:\t(SkipField0: "
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
               dbgs() << "Fields after sort and swap:\t(SkipField0: "
                      << SkipField0
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

  uint64_t StructAlign = DL.getABITypeAlignment(StructT);
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
  const uint64_t OrigStructSize = DL.getTypeAllocSize(StructT);
  int64_t SpaceDiff = OrigStructSize - Offset;

  const unsigned Threshold =
      IsSimpleStructType ? DTransReorderFieldsSavedSpacePercentThreshold
                         : DTransReorderFieldsSavedSpacePercentComplexThreshold;

  if (IsSimpleStructType) {
    if (SpaceDiff > 0) {
      // Struct size shrink: ensure the savings is big enough
      if (OrigStructSize <= Offset ||
          (SpaceDiff * 100) / OrigStructSize < Threshold) {
        LLVM_DEBUG(dbgs() << "  Not profitable to reorder: "
                          << getStName(StructT)
                          << " ( Size: " << DL.getTypeAllocSize(StructT)
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
                        << getStName(StructT)
                        << " ( Size: " << DL.getTypeAllocSize(StructT)
                        << " SpaceSaved: " << SpaceDiff << " )\n");
      return false;
    }
  }

  LLVM_DEBUG({
               dbgs() << getStName(StructT) << " selected for Field-reorder: "
                      << " ( Size: " << DL.getTypeAllocSize(StructT)
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
  RTI.setTransformedIndexes(StructT, NewIdxVec);

  // Check what we have in RTI:
  LLVM_DEBUG({
               dbgs() << "RTI Dump:\n";
               RTI.dump();
             });

  return true;
} // namespace dtrans

// Returns true if a given StructType * is applicable for DTrans's Field-Reorder
// Optimization.
//
// This checks the property on the said StructType:
// - not packed already
// - each field is NOT an aggregate or vector type (only simple types)
// - check additional filter conditions
//
bool ReorderFieldsPass::isApplicable(TypeInfo *TI, const DataLayout &DL) {
  // Bypass the applicable test?
  if (!EnableReorderFieldApplicableTest || !EnableReorderFieldTests)
    return true;

  auto *StInfo = dyn_cast<StructInfo>(TI);
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

  if (isSimpleStructType(TI) || isAdvancedStructType(TI))
    return true;

  return false;
}

bool ReorderFieldsPass::isLegal(TypeInfo *TI, DTransAnalysisInfo &DTInfo) {
  // Bypass the legal test?
  if (!EnableReorderFieldLegalTest || !EnableReorderFieldTests)
    return true;

  auto *StInfo = dyn_cast<StructInfo>(TI);
  if (!StInfo)
    return false;

  // **Simple Struct Legal Test**
  if (isSimpleStructType(TI)) {
    if (DTInfo.testSafetyData(TI, dtrans::DT_ReorderFields)) {
      LLVM_DEBUG(dbgs() << "  Rejecting "
                        << getStName(cast<StructType>(StInfo->getLLVMType()))
                        << " based on safety data.\n");
      return false;
    }
    return true;
  }

  // Advanced Struct Legal Test:
  // protected under EnableReorderFieldAdv flag, default is disabled.
  if (EnableReorderFieldAdv && isAdvancedStructType(TI)) {
    // Relaxed ReorderFields SafetyDataMask
    static const SafetyData ReorderFieldsSafetyDataScheme =
        dtrans::BadPtrManipulation | dtrans::GlobalInstance |
            dtrans::HasInitializerList | dtrans::UnsafePtrMerge |
            dtrans::MemFuncPartialWrite | dtrans::NoFieldsInStruct |
            dtrans::LocalInstance | dtrans::BadCastingConditional |
            dtrans::UnsafePointerStoreConditional | dtrans::UnhandledUse |
            dtrans::WholeStructureReference | dtrans::VolatileData |
            dtrans::BadMemFuncSize | dtrans::BadMemFuncManipulation |
            dtrans::AmbiguousPointerTarget;

    if (StInfo->testSafetyData(ReorderFieldsSafetyDataScheme)) {
      LLVM_DEBUG({
                   dbgs() << "  Rejecting ";
                   StInfo->getLLVMType()->print(dbgs(), true, true);
                   dbgs() << " based on revised ReorderField safety data.\n";
                 });
      return false;
    }
    return true;
  }

  // Default: illegal
  return false;
}

// Collect all potential StructType * as Reorder-Field candidates
bool ReorderFieldsPass::doCollection(DTransAnalysisInfo &DTInfo,
                                     const DataLayout &DL, const Module &M) {
  LLVM_DEBUG(dbgs() << "Reorder fields: looking for candidate structures.\n");

  for (TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<StructInfo>(TI);
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

bool ReorderFieldsPass::runImpl(
    Module &M, DTransAnalysisInfo &DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {
  if (!EnableReorderField)
    return false;
  if (!WPInfo.isWholeProgramSafe())
    return false;
  if (!DTInfo.useDTransAnalysis())
    return false;
  auto &DL = M.getDataLayout();

  // *Collection*
  // - scan each StructType in module
  // - do legal test, profit test, and applicable test
  // - collect all suitable Candidate struct types into CandidateTypeV
  if (!doCollection(DTInfo, DL, M))
    return false;

  // Check if any type is selected for field-reordering.
  if (!RTI.hasAnyTypeTransformed())
    return false;

  // *ReorderField Transformation*
  DTransTypeRemapper TypeRemapper;
  ReorderFieldsImpl ReorderFieldsImpl(RTI, DTInfo, M.getContext(), DL, GetTLI,
                                      "__DFR_", &TypeRemapper);
  bool RunResult = ReorderFieldsImpl.run(M);

  // Update statistical counter only if run() is successful
  if (RunResult)
    NumReorderFieldTransformed += CandidateTypeV.size();

  return true;
}

PreservedAnalyses ReorderFieldsPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };

  if (!runImpl(M, DTransInfo, GetTLI, WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm
