//===---------------- ReorderFields.cpp - DTransReorderFieldsPass ---------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans fields reorder optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ReorderFields.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;

#define DEBUG_TYPE "dtrans-reorderfields"

// Enable Padding based heuristic to select candidates for "Reorder Fields".
static cl::opt<bool> DTransReoderFieldsPaddingHeuristic(
    "dtrans-reorder-fields-padding-heuristic", cl::init(true),
    cl::ReallyHidden);

// Minimum unused space (padding) percent threshold to select candidate
// structure for reorder fields.
static cl::opt<unsigned> DTransReorderFieldsUnusedSpacePercentThreshold(
    "dtrans-reorder-fields-unused-space-percent-threshold", cl::init(16),
    cl::ReallyHidden);

// Field reordering may not help to utilize all unused space in the original
// structure layout. This flag is used to avoid applying reordering
// transformation if reordering is not profitable. Minimum saved space
// percent threshold with field reordering to enable the transformation.
static cl::opt<unsigned> DTransReorderFieldsSavedSpacePercentThreshold(
    "dtrans-reorder-fields-saved-space-percent-threshold", cl::init(13),
    cl::ReallyHidden);

// Limit size of structs that are selected for reordering fields based
// on padding heuristic. Field affinity info may be needed to apply
// reordering for structs that don’t fit cache Line.
// "TargetTransformInfo::getCacheLineSize" can be used instead MaxStructSize
// after doing more experiments.
static const unsigned MaxStructSize = 64;

// Minimum number of fields to select a struct as candidate.
static const unsigned MinNumElems = 3;

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
    DTransAnalysisInfo &DTInfo =
        getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    return Impl.runImpl(M, DTInfo,
                        getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(),
                        getAnalysis<WholeProgramWrapperPass>().getResult());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<WholeProgramWrapperPass>();
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

namespace llvm {

namespace dtrans {

// This class, which is derived from DTransOptBase, does all necessary
// changes for field-reordering.
// postprocessFunction:
//   1. GEP: Replace old index with new index of fields
//   2. memset/memcpy/memmov/alloc/calloc/realloc/sdiv: Replace old size
//      with new size.
// prepareTypes & populateTypes:
//    Create new types for structs that are field-reordered and let
//    DTransOptBase do the actual type replacement.
class ReorderFieldsImpl : public DTransOptBase {
public:
  ReorderFieldsImpl(ReorderTransInfo &RTI, DTransAnalysisInfo &DTInfo,
                    LLVMContext &Context, const DataLayout &DL,
                    const TargetLibraryInfo &TLI, StringRef DepTypePrefix,
                    DTransTypeRemapper *TypeRemapper)
      : DTransOptBase(DTInfo, Context, DL, TLI, DepTypePrefix, TypeRemapper),
        RTI(RTI) {}

  virtual bool prepareTypes(Module &M) override;
  virtual void populateTypes(Module &M) override;
  virtual void processFunction(Function &F) override;
  virtual void postprocessFunction(Function &Func, bool isCloned) override;

private:
  ReorderTransInfo &RTI;
  TypeToTypeMap OrigToNewTypeMapping;

  void processBinaryOperator(BinaryOperator &BO);
  void processCallInst(CallInst &CI);
  void transformDivOp(BinaryOperator &I);
  void processGetElementPtrInst(GetElementPtrInst &GEP);
  void processByteFlattenedGetElementPtrInst(GetElementPtrInst &GEP);
  void transformAllocCall(CallInst &CI, StructType *Ty);
  void transformMemfunc(CallInst &CI, StructType *Ty);
  bool replaceOldSizeWithNewSize(Value *Val, uint64_t OldSize, uint64_t NewSize,
                                 Instruction *I, uint32_t APos);
  bool replaceOldSizeWithNewSizeForConst(Value *Val, uint64_t OldSize,
                                         uint64_t NewSize, Instruction *I,
                                         uint32_t APos);
  void replaceOldValWithNewVal(Instruction *I, uint32_t APos, Value *NewVal);
  StructType *getAssociatedOrigTypeOfSub(Value *SubV);
  Type *getOrigTyOfTransformedType(Type *TType);
  StructType *getStructTyAssociatedWithCallInfo(CallInfo *CallInfo);
};

// Helper function to get associated StructType of \p CallInfo.
StructType *
ReorderFieldsImpl::getStructTyAssociatedWithCallInfo(CallInfo *CallInfo) {
  for (auto *PTy : CallInfo->getPointerTypeInfoRef().getTypes()) {
    Type *StrTy = PTy->getPointerElementType();
    // StrTy is orignal type but call getOrigTyOfTransformedType to
    // make sure the type is transformed.
    Type *OrigStrTy = getOrigTyOfTransformedType(StrTy);
    if (OrigStrTy)
      return cast<StructType>(OrigStrTy);
  }
  return nullptr;
}

// Helper function to get original StructType of given \p TType
// if it is either Orig or New type in OrigToNewTypeMapping.
// Otherwise, returns nullptr.
Type *ReorderFieldsImpl::getOrigTyOfTransformedType(Type *TType) {
  for (auto &TypesPair : OrigToNewTypeMapping) {
    Type *OrigTy = TypesPair.first;
    Type *NewTy = TypesPair.second;
    if (NewTy == TType || OrigTy == TType)
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
//        (Val / OldSize ) * NewSize
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

// This routine converts \p Val from multiple of OldSize to multiple
// of NewSize. If \p Val is not constant, it generates Mul & SDiv to
// do the same.
bool ReorderFieldsImpl::replaceOldSizeWithNewSize(Value *Val, uint64_t OldSize,
                                                  uint64_t NewSize,
                                                  Instruction *I,
                                                  uint32_t APos) {
  if (!Val)
    return false;
  // Check if Val is multiple of OldSize.
  if (!isValueMultipleOfSize(Val, OldSize, true))
    return false;
  if (replaceOldSizeWithNewSizeForConst(Val, OldSize, NewSize, I, APos))
    return true;
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
// MemFuncPartialWrite. So, just need to handle MemFunc with
// Complete aggregates. Only Size argument needs to be fixed.
void ReorderFieldsImpl::transformMemfunc(CallInst &CI, StructType *Ty) {
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
void ReorderFieldsImpl::transformAllocCall(CallInst &CI, StructType *Ty) {
  AllocKind Kind = getAllocFnKind(&CI, TLI);
  assert((Kind == AK_Calloc || Kind == AK_Malloc || Kind == AK_Realloc) &&
         "Unexpected alloc call");
  LLVM_DEBUG(dbgs() << "Alloc Before:" << CI << "\n");

  unsigned SizeArgPos = 0;
  unsigned CountArgPos = 0;
  getAllocSizeArgs(Kind, &CI, SizeArgPos, CountArgPos, TLI);

  uint64_t OldSize = DL.getTypeAllocSize(Ty);
  uint64_t NewSize = RTI.getTransformedTypeNewSize(Ty);
  auto *AllocSizeVal = CI.getArgOperand(SizeArgPos);
  bool Replaced = replaceOldSizeWithNewSize(AllocSizeVal, OldSize, NewSize, &CI,
                                            SizeArgPos);
  // If AllocSizeVal is not multiple of size of struct, try to fix
  // count argument.
  if (CountArgPos != -1U && !Replaced)
    Replaced = replaceOldSizeWithNewSize(CI.getArgOperand(CountArgPos), OldSize,
                                         NewSize, &CI, CountArgPos);

  assert(Replaced == true &&
         "Expecting oldSize should be replaced with NewSize");
  LLVM_DEBUG(dbgs() << "Alloc After:" << CI << "\n");
}

// Fix field index value in GEP if GEP is computing address of a field
// in any reordered struct.
void ReorderFieldsImpl::processGetElementPtrInst(GetElementPtrInst &GEP) {
  Type *SourceTy = GEP.getSourceElementType();
  uint32_t NewIdx;

  if (!isa<StructType>(SourceTy))
    return;
  // Get original struct type.
  Type *OrigStrTy = getOrigTyOfTransformedType(SourceTy);
  if (!OrigStrTy)
    return;

  StructType *StructTy = cast<StructType>(OrigStrTy);
  // Nothing to do if it is not computing address of any field.
  if (GEP.getNumOperands() != 3)
    return;
  // Get struct field index.
  Value *Op = GEP.getOperand(2);
  auto *LastArg = dyn_cast<ConstantInt>(Op);

  LLVM_DEBUG(dbgs() << "GEP Before:" << GEP << "\n");
  NewIdx = RTI.getTransformedIndex(StructTy, LastArg->getLimitedValue());
  GEP.setOperand(2, ConstantInt::get(Op->getType(), NewIdx));
  LLVM_DEBUG(dbgs() << "GEP After:" << GEP << "\n");
}

// Fix offset value in ByteFlattened GEP if it is computing address of
// a field in any reordered struct.
void ReorderFieldsImpl::processByteFlattenedGetElementPtrInst(
    GetElementPtrInst &GEP) {

  // Only two operands are expected for ByteFlattened GEPs
  if (GEP.getNumOperands() != 2)
    return;
  auto GEPInfo = DTInfo.getByteFlattenedGEPElement(&GEP);
  if (!GEPInfo.first)
    return;

  Type *SourceTy = GEPInfo.first;
  // Check if SourceTy is transformed.
  Type *OrigTy = getOrigTyOfTransformedType(SourceTy);
  if (!OrigTy)
    return;

  StructType *StructTy = cast<StructType>(OrigTy);
  uint32_t OldIdx = GEPInfo.second;
  uint32_t NewIdx = RTI.getTransformedIndex(StructTy, OldIdx);

  // Get offset of the field in new layout.
  StructType *NewSt = cast<StructType>(OrigToNewTypeMapping[StructTy]);
  auto *SL = DL.getStructLayout(NewSt);
  uint64_t NewOffset = SL->getElementOffset(NewIdx);

  LLVM_DEBUG(dbgs() << "ByteFGEP Before:" << GEP << "\n");
  GEP.setOperand(1, ConstantInt::get(GEP.getOperand(1)->getType(), NewOffset));
  LLVM_DEBUG(dbgs() << "ByteFGEP After:" << GEP << "\n");
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
  Type *StrTy = PtrTy->getPointerElementType();
  // Get original struct Type from transformed type.
  Type *OrigStrTy = getOrigTyOfTransformedType(StrTy);
  if (!OrigStrTy)
    return nullptr;
  return cast<StructType>(OrigStrTy);
}

// DTransAnalysis allows below pointer arithmetic pattern:
//    %2 = sub Ptr1_str1, Ptr2_str1
//    %3 = sdiv %2, size_of(Str1)
//
//    size_of(str1) needs to be changed if reordering is applied to str1
//
void ReorderFieldsImpl::transformDivOp(BinaryOperator &I) {
  assert((I.getOpcode() == Instruction::SDiv ||
          I.getOpcode() == Instruction::UDiv) &&
         "Unexpected opcode");
  Value *SubI = I.getOperand(0);

  StructType *STy = getAssociatedOrigTypeOfSub(SubI);
  if (!STy)
    return;

  LLVM_DEBUG(dbgs() << "SDiv/UDiv  Before:" << I << "\n");
  Value *SizeVal = I.getOperand(1);
  bool Replaced =
      replaceOldSizeWithNewSize(SizeVal, DL.getTypeAllocSize(STy),
                                RTI.getTransformedTypeNewSize(STy), &I, 1);
  assert(Replaced == true &&
         "Expecting oldSize should be replaced with NewSize");

  (void)Replaced;
  LLVM_DEBUG(dbgs() << "SDiv/UDiv  After:" << I << "\n");
}

// Only Pointer arithmetic that is allowed in Analysis is Instruction::Sub
// that is used by SDiv.
void ReorderFieldsImpl::processBinaryOperator(BinaryOperator &BO) {
  switch (BO.getOpcode()) {
  case Instruction::SDiv:
  case Instruction::UDiv:
    transformDivOp(BO);
    break;
  default:
    break;
  }
}

// Handle all CallInsts here. The only CallInst instructions that need
// to be handled by Reordering are Alloc and Memfunc. There is no need
// to process GEPs that are passed as arguments to any calls since
// Reordering is disabled as it is treated as AddressTaken.
void ReorderFieldsImpl::processCallInst(CallInst &CI) {
  CallInfo *CInfo = DTInfo.getCallInfo(&CI);
  if (CInfo == nullptr)
    return;
  StructType *StrTy = getStructTyAssociatedWithCallInfo(CInfo);
  if (!StrTy)
    return;
  switch (CInfo->getCallInfoKind()) {
  case CallInfo::CIK_Alloc: {
    transformAllocCall(CI, StrTy);
    break;
  }

  case CallInfo::CIK_Memfunc: {
    transformMemfunc(CI, StrTy);
    break;
  }
  default:
    break;
  }
}

// Fix all necessary IR changes related to field-reordering transform except
// for ByteFlattened GEPs, which are processed in processFunction.
//   GEP Inst: Replace old index of a field with new index
//   CallInst: Replace old size of struct with new size in malloc/calloc/
//             realloc/memset/memcpy etc.
//   BinaryOp: Fix size that is used in SDiv/UDiv.
void ReorderFieldsImpl::postprocessFunction(Function &Func, bool isCloned) {
  Function *F = isCloned ? OrigFuncToCloneFuncMap[&Func] : &Func;
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
//   ByteFlattened GEP: Old offset of a field is replaced with new offset.
void ReorderFieldsImpl::processFunction(Function &F) {
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst))
      processByteFlattenedGetElementPtrInst(*GEP);
  }
}

// Create new types based on reordered field layout.
void ReorderFieldsImpl::populateTypes(Module &M) {
  for (auto &TySetTyPair : RTI.getTypesMap()) {
    auto StTy = TySetTyPair.first;
    std::vector<Type *> EltTys(StTy->getNumElements());

    // Create reordered field layout
    for (unsigned I = 0; I < StTy->getNumElements(); ++I) {
      Type *Ty = StTy->getElementType(I);
      unsigned NewIdx = RTI.getTransformedIndex(StTy, I);
      EltTys[NewIdx] = TypeRemapper->remapType(Ty);
    }
    StructType *NewSt = cast<StructType>(OrigToNewTypeMapping[StTy]);
    NewSt->setBody(EltTys);
    LLVM_DEBUG(dbgs() << "Struct " << getStName(StTy)
                      << "after field-reordering: \n"
                      << *NewSt << "\n");
    assert(RTI.getTransformedTypeNewSize(StTy) == DL.getTypeAllocSize(NewSt) &&
           "Size computation is incorrect");
  }
}

bool ReorderFieldsImpl::prepareTypes(Module &M) {
  for (auto &TySetTyPair : RTI.getTypesMap()) {
    StructType *NewSt =
        StructType::create(TySetTyPair.first->getContext(),
                           "__DFR_" + TySetTyPair.first->getName().str());
    TypeRemapper->addTypeMapping(TySetTyPair.first, NewSt);
    OrigToNewTypeMapping[TySetTyPair.first] = NewSt;
  }
  return true;
}

// Returns true if StructT is a candidate for field reordering based
// on unused space in the structure due to alignment
bool ReorderFieldsPass::isCandidateTypeHasEnoughPadding(StructType *StructT,
                                                        const DataLayout &DL) {
  if (!DTransReoderFieldsPaddingHeuristic)
    return false;

  size_t StructSize = DL.getTypeAllocSize(StructT);
  uint32_t NumElems = StructT->getNumElements();
  if (StructSize > MaxStructSize || NumElems < MinNumElems)
    return false;

  // Compute unused space due to alignment
  uint32_t TotalFieldSize = 0;
  int32_t UnusedSpace = 0;
  for (unsigned i = 0; i < NumElems; ++i) {
    Type *Ty = StructT->getElementType(i);
    TotalFieldSize += DL.getTypeAllocSize(Ty);
  }
  UnusedSpace = StructSize - TotalFieldSize;
  if ((UnusedSpace * 100) / StructSize <
      DTransReorderFieldsUnusedSpacePercentThreshold)
    return false;

  LLVM_DEBUG(dbgs() << "  Selected based on padding heuristic: "
                    << getStName(StructT) << " ( Size: " << StructSize
                    << " UnusedSpace: " << UnusedSpace << " )\n");
  return true;
}

// Returns true if StructT is a candidate for field reordering based
// on heuristics.
bool ReorderFieldsPass::isCandidateType(StructType *StructT,
                                        const DataLayout &DL) {

  if (!doesTypeMeetReorderRestrictions(StructT)) {
    LLVM_DEBUG(dbgs() << "  Rejecting " << getStName(StructT)
                      << " based on reordering restrictions.\n");
    return false;
  }

  // Check Padding heuristic
  if (isCandidateTypeHasEnoughPadding(StructT, DL))
    return true;

  // Check more heuristics here if needed.

  LLVM_DEBUG(dbgs() << "  Rejecting " << getStName(StructT)
                    << " based on heuristics.\n");

  return false;
}

// Returns true if okay to apply field-reordering
bool ReorderFieldsPass::doesTypeMeetReorderRestrictions(StructType *StructT) {
  // check if it is packed.
  if (StructT->isPacked())
    return false;

  auto NotSimpleField = [](Type *Ty) -> bool {
    // Check if it has aggregate types.
    // No attributes are marked to indicate bit-fields, force alignment
    // on fields but adding char array fields to fill gaps between source
    // level fields. isAggregateType() check is good enough to avoid those
    // cases.
    if (Ty->isAggregateType() || Ty->isVectorTy())
      return true;
    return false;
  };

  if (std::any_of(StructT->element_begin(), StructT->element_end(),
                  NotSimpleField))
    return false;
  return true;
}

// This is used to sort fields based on alignment, size and index of
// fields.
class FieldData {
  uint64_t Align;
  uint64_t Size;
  uint32_t Index;

public:
  FieldData(uint64_t Align, uint64_t Size, uint64_t Index)
      : Align(Align), Size(Size), Index(Index) {}

  uint64_t getFieldAlign() const { return Align; }
  uint64_t getFieldSize() const { return Size; }
  uint64_t getFieldIndex() const { return Index; }

  bool operator<(const FieldData &RHS) const {
    // Sort fields based on below compare function to avoid gaps between
    // fields.
    //    Ascending order based on Alignment, then
    //    Ascending order based on Size, then
    //    Descending order based on Index.
    //
    // The purpose of using Index in sorting fields is to maintain source
    // order of fields if alignment and size of the fields are same.
    // "Index + 1" is used instead of "Index" to avoid incorrect comparison
    // with zero (i.e for 1st field) since negative index values are used
    // for sorting.
    // TODO: Revisit this to have even better layout.
    return std::make_tuple(RHS.getFieldAlign(), RHS.getFieldSize(),
                           -(RHS.getFieldIndex() + 1)) <
           std::make_tuple(Align, Size, -(Index + 1));
  }
};

// Finds new layout of \p TI based on alignment, size and index of fields
// and collects required info to apply reorder transformation if it
// is profitable.
void ReorderFieldsPass::collectReorderTransInfoIfProfitable(
    TypeInfo *TI, const DataLayout &DL) {
  SmallVector<FieldData, 8> Fields;
  auto *StInfo = dyn_cast<StructInfo>(TI);
  StructType *StructT = cast<StructType>(StInfo->getLLVMType());

  // Collect info Fields
  for (unsigned I = 0; I < StructT->getNumElements(); ++I) {
    Type *Ty = StructT->getElementType(I);
    FieldData FD(DL.getABITypeAlignment(Ty), DL.getTypeStoreSize(Ty), I);
    Fields.push_back(FD);
  }
  // Sort Fields
  llvm::sort(Fields.begin(), Fields.end());

  // Compute size of reordered layout
  uint64_t Offset = 0;
  for (const FieldData &FD : Fields) {
    uint64_t TyAlign = FD.getFieldAlign();
    if ((Offset & (TyAlign - 1)) != 0)
      Offset = alignTo(Offset, TyAlign);
    Offset += FD.getFieldSize();
  }
  uint64_t StructAlign = DL.getABITypeAlignment(StructT);
  if ((Offset & (StructAlign - 1)) != 0)
    Offset = alignTo(Offset, StructAlign);

  // Check if it really saves space by doing field-reordering
  uint64_t SpaceSaved = DL.getTypeAllocSize(StructT) - Offset;
  if (DL.getTypeAllocSize(StructT) <= Offset ||
      (SpaceSaved * 100) / DL.getTypeAllocSize(StructT) <
          DTransReorderFieldsSavedSpacePercentThreshold) {
    LLVM_DEBUG(dbgs() << "  Not profitable to reorder: " << getStName(StructT)
                      << " ( Size: " << DL.getTypeAllocSize(StructT)
                      << " SpaceSaved: " << SpaceSaved << " )\n");
    return;
  }
  LLVM_DEBUG(dbgs() << "  Field-reorder will be applied: " << getStName(StructT)
                    << " ( Size: " << DL.getTypeAllocSize(StructT)
                    << " SpaceSaved: " << SpaceSaved << " )\n");

  // Update field reorder info that is required to do IR transform.
  RTI.setTransformedTypeNewSize(StructT, Offset);
  std::vector<uint32_t> NewIdxVec(Fields.size());
  uint32_t NewIndex = 0;
  for (const FieldData &FD : Fields)
    NewIdxVec[FD.getFieldIndex()] = NewIndex++;
  RTI.setTransformedIndexes(StructT, NewIdxVec);
}

bool ReorderFieldsPass::gatherCandidateTypes(DTransAnalysisInfo &DTInfo,
                                             const DataLayout &DL) {
  LLVM_DEBUG(dbgs() << "Reorder fields: looking for candidate structures.\n");

  for (TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<StructInfo>(TI);
    if (!StInfo)
      continue;

    if (DTInfo.testSafetyData(TI, dtrans::DT_ReorderFields)) {
      LLVM_DEBUG(dbgs() << "  Rejecting "
                        << getStName(cast<StructType>(StInfo->getLLVMType()))
                        << " based on safety data.\n");
      continue;
    }
    StructType *StructT = cast<StructType>(StInfo->getLLVMType());

    if (!isCandidateType(StructT, DL))
      continue;

    CandidateTypes.push_back(StInfo);
  }

  LLVM_DEBUG(if (CandidateTypes.empty()) dbgs() << "  No candidates found.\n");

  return !CandidateTypes.empty();
}

bool ReorderFieldsPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                                const TargetLibraryInfo &TLI,
                                WholeProgramInfo &WPInfo) {

  if (!WPInfo.isWholeProgramSafe())
    return false;

  if (!DTInfo.useDTransAnalysis())
    return false;

  auto &DL = M.getDataLayout();

  if (!gatherCandidateTypes(DTInfo, DL))
    return false;

  for (auto *TI : CandidateTypes)
    collectReorderTransInfoIfProfitable(TI, DL);

  // Check if any type is selected for field-reordering.
  if (!RTI.hasAnyTypeTransformed())
    return false;

  // Apply IR and type transformations using ReorderFieldsImpl.
  DTransTypeRemapper TypeRemapper;
  ReorderFieldsImpl ReorderFieldsImpl(RTI, DTInfo, M.getContext(), DL, TLI,
                                      "__BDFR_", &TypeRemapper);
  ReorderFieldsImpl.run(M);
  return true;
}

PreservedAnalyses ReorderFieldsPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, AM.getResult<TargetLibraryAnalysis>(M), WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

} // namespace dtrans

} // namespace llvm
