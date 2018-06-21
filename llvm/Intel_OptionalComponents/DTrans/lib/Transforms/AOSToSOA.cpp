//===---------------- AOSToSOA.cpp - DTransAOStoSOAPass -------------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans Array of Structures to Structure of Arrays
// data layout optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/AOSToSOA.h"
#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
using namespace llvm;
using namespace dtrans;

#define DEBUG_TYPE "dtrans-aostosoa"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// This option is used during testing to allow qualifying specific structure
// types be converted via the AOS-to-SOA transform without running the
// profitability heuristics. (The type must pass all other qualification tests,
// just the profitability test is skipped).
//
// This is a comma separated list of structure type names that will not be
// disqualified by the profitability heuristic.
static cl::opt<std::string>
    DTransAOSToSOAHeurOverride("dtrans-aostosoa-heur-override",
                               cl::ReallyHidden);

// This is a temporary flag to allow testing of the selection/qualification of
// candidates without the transformation code being available to completely
// transform the IR contained within those tests. Once the transformation code
// is complete, this flag will be removed.
static cl::opt<bool>
    DTransAOSToSOAQualificationOnly("dtrans-aostosoa-qualification-only",
                                    cl::ReallyHidden);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

namespace {
// This class is used during the type remapping process to perform the
// translation of a null value pointer to an integer index.
//
// The AOS to SOA conversion modifies pointers to the structure to be
// integer indices. During the type remapping, pointers to the constant
// nullptr need to be converted to be represented with an integer 0 index.
class AOSToSOAMaterializer : public ValueMaterializer {
public:
  AOSToSOAMaterializer(ValueMapTypeRemapper &TypeRemapper)
      : TypeRemapper(TypeRemapper) {}

  virtual ~AOSToSOAMaterializer() {}

  virtual Value *materialize(Value *V) override {
    // Check if a null value of a different type needs to be generated.
    auto *C = dyn_cast<Constant>(V);
    if (!C)
      return nullptr;

    if (!C->isNullValue())
      return nullptr;

    Type *Ty = V->getType();
    Type *ReTy = TypeRemapper.remapType(Ty);
    if (Ty == ReTy)
      return nullptr;

    return Constant::getNullValue(ReTy);
  }

private:
  ValueMapTypeRemapper &TypeRemapper;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Helper method for getting a name to print for structures in debug traces.
StringRef getStructName(llvm::Type *Ty) {
  auto *StructTy = dyn_cast<llvm::StructType>(Ty);
  assert(StructTy && "Expected structure type");
  return StructTy->hasName() ? StructTy->getStructName() : "<unnamed struct>";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This class is responsible for all the transformation work for the AOS to SOA
// with Indexing conversion.
class AOSToSOATransformImpl : public DTransOptBase {
public:
  // Constructor that takes parameters needed for the base class, plus a list of
  // types that have been qualified for the transformation.
  AOSToSOATransformImpl(DTransAnalysisInfo &DTInfo, LLVMContext &Context,
                        const DataLayout &DL, StringRef DepTypePrefix,
                        DTransTypeRemapper *TypeRemapper,
                        AOSToSOAMaterializer *Materializer,
                        SmallVectorImpl<dtrans::StructInfo *> &Types)
      : DTransOptBase(DTInfo, Context, DL, DepTypePrefix, TypeRemapper,
                      Materializer) {
    std::copy(Types.begin(), Types.end(), std::back_inserter(TypesToTransform));

    PeelIndexWidth = DL.getPointerSizeInBits();
    PeelIndexType = Type::getIntNTy(Context, PeelIndexWidth);
    PtrSizedIntType = Type::getIntNTy(Context, DL.getPointerSizeInBits());
    Int8PtrType = llvm::Type::getInt8PtrTy(Context);

    IncompatiblePeelTypeAttrs = AttributeFuncs::typeIncompatible(PeelIndexType);
  }

  ~AOSToSOATransformImpl() {}

  // Create new data types for each of the types being converted.
  virtual bool prepareTypes(Module &M) override {
    for (auto *StInfo : TypesToTransform) {
      StructType *OrigTy = cast<StructType>(StInfo->getLLVMType());
      StructType *NewTy = StructType::create(
          Context, (Twine("__SOA_" + OrigTy->getName()).str()));
      TypeRemapper->addTypeMapping(OrigTy, NewTy);
      TypeRemapper->addTypeMapping(OrigTy->getPointerTo(),
                                   getPeeledIndexType());
      OrigToNewTypeMapping[OrigTy] = NewTy;
    }

    return !OrigToNewTypeMapping.empty();
  }

  // Set the structure body of all the types this transformation created.
  virtual void populateTypes(Module &M) override {
    for (auto &ONPair : OrigToNewTypeMapping) {
      Type *OrigTy = ONPair.first;
      Type *NewTy = ONPair.second;

      SmallVector<Type *, 8> DataTypes;
      StructType *OrigStructTy = cast<StructType>(OrigTy);
      for (auto *MemberTy : OrigStructTy->elements()) {
        DataTypes.push_back(TypeRemapper->remapType(MemberTy)->getPointerTo());
      }

      StructType *NewStructTy = cast<StructType>(NewTy);
      NewStructTy->setBody(DataTypes);
      LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Type replacement:\n  Old: "
                        << *OrigTy << "\n  New: " << *NewStructTy << "\n");
    }
  }

  // Create a new global variable for each type peeled that will serve as the
  // base pointer to the peeled variable.
  virtual void prepareModule(Module &M) override {
    for (auto &ONPair : OrigToNewTypeMapping) {
      StructType *StType = cast<StructType>(ONPair.first);
      StructType *PeelTy = cast<StructType>(ONPair.second);

      auto *PeelVar = new GlobalVariable(
          M, PeelTy, false, GlobalValue::InternalLinkage,
          /*init=*/ConstantAggregateZero::get(PeelTy),
          "__soa_" + StType->getName(),
          /*insertbefore=*/nullptr, GlobalValue::NotThreadLocal,
          /*AddressSpace=*/0, /*isExternallyInitialized=*/false);
      PeeledTypeToVariable.insert(std::make_pair(PeelTy, PeelVar));
      LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: PeelVar: " << *PeelVar << "\n");
    }
  }

  virtual void processFunction(Function &F) override {
    SmallVector<GetElementPtrInst *, 16> GEPsToConvert;
    SmallVector<GetElementPtrInst *, 16> ByteGEPsToConvert;
    SmallVector<BitCastInst *, 16> BCsToConvert;
    SmallVector<BinaryOperator *, 16> BinOpsToConvert;

    SmallVector<std::pair<AllocCallInfo *, StructInfo *>, 4> AllocsToConvert;
    SmallVector<std::pair<FreeCallInfo *, StructInfo *>, 4> FreesToConvert;

    for (auto It = inst_begin(&F), E = inst_end(F); It != E; ++It) {
      Instruction *I = &*It;
      if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
        CallSite CS(I);
        updateCallAttributes(CS);

        // Check if the call needs to be transformed based on the CallInfo.
        if (auto *CInfo = DTInfo.getCallInfo(I))
          if (auto *CInfoElemTy = getCallInfoTypeToTransform(CInfo)) {
            auto *TI = DTInfo.getTypeInfo(CInfoElemTy);
            assert(TI && "Expected TypeInfo for structure type");

            switch (CInfo->getCallInfoKind()) {
            case dtrans::CallInfo::CIK_Alloc: {
              auto *AInfo = cast<AllocCallInfo>(CInfo);
              // The candidate qualification should have only allowed calloc or
              // malloc calls.
              assert((AInfo->getAllocKind() == AK_Calloc ||
                      AInfo->getAllocKind() == AK_Malloc) &&
                     "Only calloc or malloc expected for transformed types");

              AllocsToConvert.push_back(
                  std::make_pair(AInfo, cast<StructInfo>(TI)));
              break;
            }
            case dtrans::CallInfo::CIK_Free: {
              FreesToConvert.push_back(std::make_pair(cast<FreeCallInfo>(CInfo),
                                                      cast<StructInfo>(TI)));
              break;
            }
            case dtrans::CallInfo::CIK_Memfunc:
              // TODO: Need to add support for converted memfunc
              // calls.
              break;
            }
          }
      } else if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
        if (checkByteGEPConversionNeeded(GEP))
          ByteGEPsToConvert.push_back(GEP);
        else if (checkConversionNeeded(GEP))
          GEPsToConvert.push_back(GEP);
      } else if (auto *PTI = dyn_cast<PtrToIntInst>(I)) {
        // A pointer may be cast to an integer type, and after the
        // type remapping of the pointer to the structure this would be
        // a PtrToInt instruction with an integer type as the source type.
        // Collect this cast into the list of casts that need to be removed
        // during post processing.
        //
        // The DTransAnalysis guarantees that the size of the target integer
        // will be the same size as the pointer type.
        // The DTransAnalysis guarantees that IntToPtr instructions do not
        // need to be considered currently.
        if (checkConversionNeeded(PTI))
          PtrConverts.push_back(PTI);
      } else if (auto *BC = dyn_cast<BitCastInst>(I)) {
        if (checkConversionNeeded(BC))
          BCsToConvert.push_back(BC);
      } else if (auto *BinOp = dyn_cast<BinaryOperator>(I)) {
        if (checkConversionNeeded(BinOp)) {
          BinOpsToConvert.push_back(BinOp);
        }
      }
    }

    for (auto &Alloc : AllocsToConvert)
      processAllocCall(Alloc.first, Alloc.second);

    for (auto &Free : FreesToConvert)
      processFreeCall(Free.first, Free.second);

    for (auto *GEP : ByteGEPsToConvert)
      processByteFlattendGEP(GEP);

    for (auto *GEP : GEPsToConvert)
      processGEP(GEP);

    for (auto *BinOp : BinOpsToConvert)
      processBinOp(BinOp);

    // The bitcasts should be processed after the 'free' calls and
    // byte-flattened GEPs, because those conversions are expecting
    // instructions with the bitcasts as an input.
    for (auto *BC : BCsToConvert)
      processBitcast(BC);

    for (auto *I : InstructionsToDelete)
      I->eraseFromParent();

    PeelIndexCache.clear();
    InstructionsToDelete.clear();
  }

  bool checkConversionNeeded(GetElementPtrInst *GEP) {
    // The only cases that need to be converted are GEPs
    // with 1 or 2 indices because nested structures are not supported.
    if (GEP->getNumIndices() > 2)
      return false;

    Type *ElementTy = GEP->getSourceElementType();
    return isTypeToTransform(ElementTy);
  }

  bool checkByteGEPConversionNeeded(GetElementPtrInst *GEP) {
    if (GEP->getNumIndices() != 1)
      return false;

    auto InfoPair = DTInfo.getByteFlattenedGEPElement(GEP);
    if (!InfoPair.first)
      return false;

    return isTypeToTransform(InfoPair.first);
  }

  bool checkConversionNeeded(PtrToIntInst *PTI) {
    Type *Ty = PTI->getOperand(0)->getType()->getPointerElementType();
    return isTypeToTransform(Ty);
  }

  bool checkConversionNeeded(BitCastInst *BC) {
    // Conversions to or from pointers to the struct type being converted need
    // to be processed, because after rewriting the types, these would become
    // a bitcast from or to an i64 type.
    if (isTypeToTransform(BC->getDestTy()->getPointerElementType())) {
      // We only expect bitcasts to be with i8* due to the safety checks, assert
      // this to be sure.
      assert(BC->getSrcTy() == getInt8PtrType() &&
             "Only cast from i8* expected for transformed type");
      return true;
    }

    Type *Ty = BC->getSrcTy()->getPointerElementType();
    bool ShouldTransform = isTypeToTransform(Ty);
    if (!ShouldTransform)
      return false;

    // We only expect bitcasts to be to i8* due to the safety checks, assert
    // this to be sure.
    assert(BC->getDestTy() == getInt8PtrType() &&
           "Only cast to i8* expected for transformed type");
    return true;
  }

  bool checkConversionNeeded(BinaryOperator *BinOp) {
    if (BinOp->getOpcode() != Instruction::Sub)
      return false;

    llvm::Type *PtrSubTy = DTInfo.getResolvedPtrSubType(BinOp);
    if (!PtrSubTy)
      return false;

    return isTypeToTransform(PtrSubTy);
  }

  void processGEP(GetElementPtrInst *GEP) {
    // There are 2 cases that need to be converted.
    // 1: Getting the address of a structure within the array of structures.
    //    %addr1 = getelementptr %struct.t, %struct.t* %base, i64 %n
    //
    // 2: Getting the address of a structure field within the array of
    // structures.
    //    %addr2 = getelementptr %struct.t, %struct.t* %base, i64 %n, i32 1
    //
    // These need to be processed prior to the type remapping process because
    // the type remapping will change %struct.t* to be an integer type, which
    // would prevent identification of the instructions that need to be
    // rewritten.
    //
    // There will not be higher numbers of indices because nested structures are
    // disqualified during the safety checks.
    unsigned NumIndices = GEP->getNumIndices();
    assert(NumIndices <= 2 && "Unexpected index count for GEP");

    if (NumIndices == 1) {
      // This will convert the GEP of case 1 from:
      //    %arrayidx = getelementptr %struct.t, %struct.t* %base, i64 %idx_in
      // To:
      //    %peelIdxAsInt = ptrtoint %struct.t* %base to i64
      //    %add = add i64 %peelIdxAsInt, %idx_in
      //    %arrayidx = inttoptr i64 %add to %struct.t*
      //
      // Note: We do not use a named variable in the IR for this, the names are
      // just shown here to make the following code transformation clearer.
      //
      // The ptrtoint and inttoptr instructions are temporary in order
      // to support the type remapping process. During the type remapping
      // process, the pointer type in those instructions will be converted
      // into an integer type, resulting in an i64 to i64 cast, which must
      // be removed during the function post processing. The use of the
      // conversion instruction here allows for values into and out of affected
      // instructions to be replaced without violating the type matching.
      Value *Src = GEP->getPointerOperand();
      CastInst *PeelIdxAsInt = createCastToPeelIndexType(Src, GEP);

      // Indexing into an array allows the GEP index value to be an integer of
      // any width, sign-extend the smaller of the GEP index and our peeling
      // index to make the types compatible for addition.
      Value *GEPBaseIdx = GEP->getOperand(1);
      uint64_t PeelIdxWidth = getPeeledIndexWidth();
      uint64_t BaseIdxWidth = DL.getTypeSizeInBits(GEPBaseIdx->getType());
      if (BaseIdxWidth < PeelIdxWidth) {
        CastInst *SE = CastInst::Create(CastInst::SExt, GEPBaseIdx,
                                        PeelIdxAsInt->getType());
        SE->insertBefore(GEP);
        GEPBaseIdx = SE;
      } else if (PeelIdxWidth < BaseIdxWidth) {
        // We don't expect a GEP index parameter to ever be larger than a
        // pointer, assert to be sure.
        assert(BaseIdxWidth <= DL.getTypeSizeInBits(getPtrSizedIntType()) &&
               "Unsupported GEP index type");
        CastInst *SE = CastInst::Create(CastInst::SExt, PeelIdxAsInt,
                                        GEPBaseIdx->getType());
        SE->insertBefore(GEP);
        PeelIdxAsInt = SE;
      }

      BinaryOperator *Add = BinaryOperator::CreateAdd(PeelIdxAsInt, GEPBaseIdx);

      // We will steal the name of the GEP and put it on this instruction
      // because even if the replacement is going to be done via a cast
      // statement, the cast is going to eventually be eliminated anyway.
      Add->takeName(GEP);
      Add->insertBefore(GEP);

      // Cast the computed peeled index back to the original pointer type, and
      // substitute this into the users. When the type remapping occurs, the
      // users will be converted to an integer type, and the cast instruction
      // will be removed during post-processing.
      //
      // Note: Currently this requires the peeling index type to be the same
      // bit width as the pointer type, when updates are made for using a
      // smaller size, we will likely need to add some truncation here.
      CastInst *ArrayIdx =
          CastInst::CreateBitOrPointerCast(Add, Src->getType());
      ArrayIdx->insertBefore(GEP);
      PtrConverts.push_back(ArrayIdx);

      GEP->replaceAllUsesWith(ArrayIdx);
      InstructionsToDelete.insert(GEP);
    } else {
      // This will convert the GEP of case 2 from:
      //    %elem_addr =
      //         getelementptr %struct.t, %struct.t* %base, i64 %idx_n, i32 1
      //
      // To:
      //    %peelIdxAsInt = ptrtoint %struct.test01* %base to i64
      //    %peel_base = getelementptr % __soa_struct.t,
      //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
      //    %soa_addr = load i32*, i32** %peel_base
      //    %adjusted_idx = add i64 %peelIdxAsInt, %idx_n
      //    %elem_addr = getelementptr i32, i32* %soa_addr, i64 %adjusted_idx
      //
      // In this case, the 2nd GEP index will always be the field number which
      // is a constant integer.

      Type *ElementTy = GEP->getSourceElementType();
      StructType *PeelType = getTransformedType(ElementTy);
      Value *PeelVar = PeeledTypeToVariable[PeelType];
      assert(PeelVar && "Peeling variable should have already been created");

      CastInst *PeelIdxAsInt =
          createCastToPeelIndexType(GEP->getPointerOperand(), GEP);
      Value *FieldNum = GEP->getOperand(2);

      // Create and insert the instructions that get the address of the field in
      // the transformed structure.
      Instruction *FieldGEP = createGEPFieldAddressReplacement(
          PeelType, PeelVar, PeelIdxAsInt, GEP->getOperand(1), FieldNum, GEP);
      // We will steal the name of the GEP and put it on this instruction
      // because even if the replacement is going to be done via a cast
      // statement, the cast is going to eventually be eliminated anyway.
      FieldGEP->takeName(GEP);

      // If the field type is pointer to a structure that is being transformed,
      // generate a temporary cast to the original type so that the user
      // instruction will have expected types. These casts will become casts
      // from and to the same type after the remapping happens, and will be
      // removed during post-processing.
      Type *OrigFieldTy = GEP->getType();
      Value *ReplVal = FieldGEP;

      // Identify the type in the new structure for the field being accessed.
      unsigned FieldIdx = cast<ConstantInt>(FieldNum)->getLimitedValue();
      llvm::Type *PeelFieldTy = PeelType->getElementType(FieldIdx);

      if (PeelFieldTy != OrigFieldTy) {
        CastInst *CastToPtr =
            CastInst::CreateBitOrPointerCast(FieldGEP, OrigFieldTy);
        CastToPtr->insertBefore(GEP);
        PtrConverts.push_back(CastToPtr);
        ReplVal = CastToPtr;
      }

      GEP->replaceAllUsesWith(ReplVal);
      InstructionsToDelete.insert(GEP);
    }
  }

  // Create an intermediate cast instruction to cast a pointer to a
  // structure type being converted into the peeling index type. If
  // \p InsertBefore is non-null the instruction will be inserted into the IR
  // before it. This cast is added to the list of casts that are to be removed
  // during post processing because it will end up being i64 to i64 after the
  // type remapping.
  CastInst *createCastToPeelIndexType(Value *V,
                                      Instruction *InsertBefore = nullptr) {
    CastInst *ToInt = CastInst::CreateBitOrPointerCast(V, getPeeledIndexType(),
                                                       "", InsertBefore);
    PtrConverts.push_back(ToInt);

    return ToInt;
  }

  // Create and insert the instruction sequence that gets the address of a
  // specific element in the peeled structure.
  // \p PeelType is the peeled structure type.
  // \p PeelVar is the global variable for the peeled structure.
  // \p FieldNumVal is a constant integer for the field number.
  // \p GEPBaseIdx and \p GEPFieldNum are the two GEP indices.
  // The set of instructions will be inserted immediately before \p
  // InsertBefore.
  // Returns the getelementptr instruction that represents the address.
  //
  // Generates:
  //    %peel_base = getelementptr % __soa_struct.t,
  //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
  //    %soa_addr = load i32*, i32** %peel_base
  //    %adjusted_idx = add i64 %peelIdxAsInt, %idx_n
  //    %elem_addr = getelementptr i32, i32* %soa_addr, i64 %adjusted_idx
  Instruction *createGEPFieldAddressReplacement(
      llvm::StructType *PeelType, Value *PeelVar, Value *PeelIdxAsInt,
      Value *GEPBaseIdx, Value *GEPFieldNum, Instruction *InsertBefore) {

    Instruction *SOAAddr =
        createPeelFieldLoad(PeelType, PeelVar, GEPFieldNum, InsertBefore);
    uint64_t PeelIdxWidth = getPeeledIndexWidth();
    uint64_t BaseIdxWidth = DL.getTypeSizeInBits(GEPBaseIdx->getType());

    if (PeelIdxWidth < BaseIdxWidth) {
      // We don't expect a GEP index parameter to ever be larger than a
      // pointer, assert to be sure.
      assert(BaseIdxWidth <= DL.getTypeSizeInBits(getPtrSizedIntType()) &&
             "Unsupported GEP index type");
      CastInst *SE = CastInst::Create(CastInst::SExt, PeelIdxAsInt,
                                      GEPBaseIdx->getType(), "", InsertBefore);
      PeelIdxAsInt = SE;
    }

    Value *AdjustedPeelIdxAsInt = PeelIdxAsInt;
    // If first index is not constant 0, then we need to index by that amount
    if (!dtrans::isValueEqualToSize(GEPBaseIdx, 0)) {
      if (BaseIdxWidth < PeelIdxWidth) {
        CastInst *SE =
            CastInst::Create(CastInst::SExt, GEPBaseIdx,
                             PeelIdxAsInt->getType(), "", InsertBefore);
        GEPBaseIdx = SE;
      }

      BinaryOperator *Add =
          BinaryOperator::CreateAdd(PeelIdxAsInt, GEPBaseIdx, "", InsertBefore);
      AdjustedPeelIdxAsInt = Add;
    }

    // Identify the type in the new structure for the field being accessed.
    unsigned FieldIdx = cast<ConstantInt>(GEPFieldNum)->getLimitedValue();
    llvm::Type *PeelFieldTy = PeelType->getElementType(FieldIdx);

    // We know all elements of the peeled structure are pointer types.
    // Get the type that it points to.
    Type *FieldElementTy = cast<PointerType>(PeelFieldTy)->getElementType();
    GetElementPtrInst *FieldGEP = GetElementPtrInst::Create(
        FieldElementTy, SOAAddr, AdjustedPeelIdxAsInt, "", InsertBefore);

    return FieldGEP;
  }

  // Create a load of the array address for a specific field of the peeled
  // structure.
  // \p PeelType is the peeled structure type
  // \p PeelVar is the global variable for the peeled structure
  // \p FieldNumVal is a constant integer for the field number.
  // Instructions are inserted before \p InsertBefore
  //
  // Generates:
  //    %peel_base = getelementptr % __soa_struct.t, %__soa_struct.t*
  //                     @__soa_struct.t, i64 0, i32 1
  //    %soa_addr = load i32*, i32** %peel_base
  Instruction *createPeelFieldLoad(llvm::Type *PeelType, Value *PeelVar,
                                   Value *FieldNumVal,
                                   Instruction *InsertBefore) {
    GetElementPtrInst *PeelBase = GetElementPtrInst::Create(
        PeelType, PeelVar,
        {Constant::getNullValue(getPtrSizedIntType()), FieldNumVal}, "",
        InsertBefore);
    LoadInst *SOAAddr = new LoadInst(PeelBase);
    SOAAddr->insertBefore(InsertBefore);
    return SOAAddr;
  }

  // The analysis phase has resolved that the use of this subtract instruction
  // is used to divide by the structure size or some multiple of it.
  // Because the pointer to the structure has been converted to be an
  // integer index, the result of the subtract is the distance between
  // the pointers that the divide was going to compute when the divisor
  // equaled the structure size. Update the divide instruction to replace the
  // divisor.
  void processBinOp(BinaryOperator *BinOp) {
    llvm::Type *PtrSubTy = DTInfo.getResolvedPtrSubType(BinOp);
    uint64_t OrigSize = DL.getTypeAllocSize(PtrSubTy);
    updatePtrSubDivUserSizeOperand(BinOp, OrigSize, 1);
  }

  // The byte-flattened GEP needs to be transformed from getting the address as:
  //    %p8_B = getelementptr i8, i8* %p, i64 8
  // (In this case field 1 is offset 8 bytes from the start of the structure)
  //
  // To:
  //    %peelIdxAsInt = ptrtoint %struct.test01* %p to i64
  //    %peel_base = getelementptr % __soa_struct.t,
  //                        %__soa_struct.t* @__soa_struct.t, i64 0, i32 1
  //    %soa_addr = load i32*, i32** %peel_base
  //    %field_gep = getelementptr i32, i32* %soa_addr, i64 %peelIdxAsInt
  //    %p8_B = bitcast i32* %elem_addr to i8*
  void processByteFlattendGEP(GetElementPtrInst *GEP) {
    auto InfoPair = DTInfo.getByteFlattenedGEPElement(GEP);
    auto *OrigStructTy = cast<llvm::StructType>(InfoPair.first);

    // Trace the pointer operand to find the base value that is needed
    // for indexing into the field's array.
    Value *IndexAsInt =
        getPeelIndexFromValue(GEP->getPointerOperand(), OrigStructTy,
                              /*ForMemfunc=*/false);

    StructType *PeelType = getTransformedType(OrigStructTy);
    Value *PeelVar = PeeledTypeToVariable[PeelType];
    Value *FieldNum =
        ConstantInt::get(Type::getInt32Ty(GEP->getContext()), InfoPair.second);
    Instruction *FieldGEP = createGEPFieldAddressReplacement(
        PeelType, PeelVar, IndexAsInt,
        ConstantInt::get(getPtrSizedIntType(), 0), FieldNum, GEP);

    llvm::Type *PeelFieldTy = PeelType->getElementType(InfoPair.second);
    if (PeelFieldTy != GEP->getType())
      FieldGEP =
          CastInst::CreateBitOrPointerCast(FieldGEP, GEP->getType(), "", GEP);

    FieldGEP->takeName(GEP);
    GEP->replaceAllUsesWith(FieldGEP);
    InstructionsToDelete.insert(GEP);
  }

  // This is a helper function for identifying the peeling index to use
  // for byte-flattened GEPs and for memfunc transformations. For memfunc
  // transformations there is an additional pattern allowed which looks at
  // GEPs that get the address of a field within the original structure when
  // \p ForMemfunc is true.
  Value *getPeelIndexFromValue(Value *Op, StructType *OrigStructTy,
                               bool ForMemfunc) {
    if (!PeelIndexCache[Op])
      PeelIndexCache[Op] =
          createPeelIndexFromValue(Op, OrigStructTy, ForMemfunc);

    return PeelIndexCache[Op];
  }

  // This walks the definitions through bitcasts, selects and PHI nodes for
  // \p Op to find the index value to use for peeling index.
  Value *createPeelIndexFromValue(Value *Op, StructType *OrigStructTy,
                                  bool ForMemfunc) {
    // If \p Op is a bitcast from a pointer to the type being transformed to an
    // i8*, then we can directly use the source operand of bitcast, since we
    // know this is going to be turned into the peeling index type.
    if (auto *BC = dyn_cast<BitCastInst>(Op)) {
      if (BC->getOperand(0)->getType() == OrigStructTy->getPointerTo())
        return createCastToPeelIndexType(BC->getOperand(0), BC);
      else
        return getPeelIndexFromValue(BC->getOperand(0), OrigStructTy,
                                     ForMemfunc);
    }

    // For select and PHI nodes, create new instructions that will act on the
    // peeling index type, instead of the pointer type.
    if (auto *Sel = dyn_cast<SelectInst>(Op)) {
      Value *NewTrue =
          getPeelIndexFromValue(Sel->getTrueValue(), OrigStructTy, ForMemfunc);
      Value *NewFalse =
          getPeelIndexFromValue(Sel->getFalseValue(), OrigStructTy, ForMemfunc);
      Instruction *NewSel =
          SelectInst::Create(Sel->getCondition(), NewTrue, NewFalse, "", Sel);
      return NewSel;
    }

    if (auto *PHI = dyn_cast<PHINode>(Op)) {
      PHINode *NewPhi = PHINode::Create(getPeeledIndexType(), 0, "", PHI);

      // Save the new PHI so that if another reference to is found while walking
      // the definitions this one will be used.
      PeelIndexCache[PHI] = NewPhi;

      SmallVector<Value *, 4> NewPhiVals;
      for (Value *Val : PHI->incoming_values())
        NewPhiVals.push_back(
            getPeelIndexFromValue(Val, OrigStructTy, ForMemfunc));

      unsigned NumIncoming = PHI->getNumIncomingValues();
      for (unsigned Num = 0; Num < NumIncoming; ++Num)
        NewPhi->addIncoming(NewPhiVals[Num], PHI->getIncomingBlock(Num));

      return NewPhi;
    }

    // When back tracing for the index used in a memfunc, we need to also
    // consider the case where the address of the field is used to form
    // the i8* pointer passed to the call.
    // For example:
    //    %p = getelementptr %struct.ty, %struct.ty* %S, i64 %n, i32 0
    //
    // If %p were passed to a memfunc, we cannot simply convert the pointer to
    // an integer to get the peeling index. %S will be the peeling index, and we
    // need to offset this by the value of %n to get &S.field0[n]. In this case,
    // we return the expression (ptrtoint %S) + %n so that the conversion of the
    // memfunc call will start with that index value in all the fields being
    // touched. If %n equals 0, we can just return (ptrtoint %S)
    if (ForMemfunc)
      if (auto *GEP = dyn_cast<GetElementPtrInst>(Op))
        if (GEP->getSourceElementType() == OrigStructTy &&
            GEP->getNumIndices() == 2) {
          Instruction *PeelIdxAsInt =
              createCastToPeelIndexType(GEP->getPointerOperand(), GEP);

          Value *GEPBaseIdx = GEP->getOperand(1);
          if (!dtrans::isValueEqualToSize(GEPBaseIdx, 0)) {
            uint64_t PeelIdxWidth = getPeeledIndexWidth();
            uint64_t BaseIdxWidth = DL.getTypeSizeInBits(GEPBaseIdx->getType());
            if (BaseIdxWidth < PeelIdxWidth)
              GEPBaseIdx = CastInst::Create(CastInst::SExt, GEPBaseIdx,
                                            PeelIdxAsInt->getType(), "", GEP);

            PeelIdxAsInt =
                BinaryOperator::CreateAdd(PeelIdxAsInt, GEPBaseIdx, "", GEP);
          }
          return PeelIdxAsInt;
        }

    // We should not get here, but assert if we do in case there are additional
    // cases that require a conversion of the pointer type to the peeling index.
    llvm_unreachable("Not handled");
  }

  // Update the signature of the function's clone and any instructions that need
  // to be modified after the type remapping of data types has taken place.
  virtual void postprocessFunction(Function &OrigFunc, bool isCloned) override {
    Function *Func = &OrigFunc;
    if (isCloned) {
      Func = cast<Function>(VMap[&OrigFunc]);

      LLVM_DEBUG({
        dbgs() << "DTRANS-AOSTOSOA: Updating function attributes for: "
               << Func->getName() << " Type: " << *Func->getType() << "\n";
        Func->getAttributes().dump();
      });

      // For cloned functions, update any attributes on the return type or
      // arguments that are not compatible with types that have been converted
      // from pointers to integers. We do not change attributes on types
      // that were pointers-to-pointers of the transformed types because those
      // are now pointers-to-integer types, but the attributes they contained
      // should still be valid.
      llvm::Type *CloneRetTy = Func->getReturnType();
      llvm::Type *OrigRetTy = OrigFunc.getReturnType();
      if (!CloneRetTy->isPointerTy() && OrigRetTy->isPointerTy()) {

        // The only time we expect processing a function to change a pointer
        // type to an integer type is when the original argument was pointer to
        // the type being transformed, and integer type is the peeled type.
        assert(OrigToNewTypeMapping.count(OrigRetTy->getPointerElementType()) &&
               "Expected original return type to be a type being transformed");
        assert(CloneRetTy == getPeeledIndexType() &&
               "Expected clone return type to be peeling index type");

        Func->removeAttributes(0, IncompatiblePeelTypeAttrs);
      }

      assert(OrigFunc.arg_size() == Func->arg_size() &&
             "Expected clone arg for each original arg");
      Function::arg_iterator OrigArgIt = OrigFunc.arg_begin();
      Function::arg_iterator OrigArgEnd = OrigFunc.arg_end();
      Function::arg_iterator CloneArgIt = Func->arg_begin();
      for (uint64_t Idx = 0; OrigArgIt != OrigArgEnd;
           ++OrigArgIt, ++CloneArgIt, ++Idx) {
        llvm::Type *CloneArgType = CloneArgIt->getType();
        llvm::Type *OrigArgType = OrigArgIt->getType();
        if (!CloneArgType->isPointerTy() && OrigArgType->isPointerTy()) {
          assert(
              OrigToNewTypeMapping.count(
                  OrigArgType->getPointerElementType()) &&
              "Expected original argument type to be a type being transformed");
          assert(CloneArgType == getPeeledIndexType() &&
                 "Expected clone argument type to be peeling index type");

          Func->removeParamAttrs(Idx, IncompatiblePeelTypeAttrs);
        }
      }

      LLVM_DEBUG({
        dbgs() << "DTRANS-AOSTOSOA: After function attribute update\n";
        Func->getAttributes().dump();
      });
    }

    for (auto *Conv : PtrConverts) {
      if (isCloned)
        Conv = cast<CastInst>(VMap[Conv]);

      assert(Conv->getType() == Conv->getOperand(0)->getType() &&
             "Expected self-type in cast after remap");
      Conv->replaceAllUsesWith(Conv->getOperand(0));
      Conv->eraseFromParent();
    }

    PtrConverts.clear();
  }

  // The allocation call for a type being transformed into a structure of arrays
  // needs to be converted to initialize the peeling variable to store the
  // addresses that start each array in the new data structure.
  //
  // For example: struct t1 { int a, b, c};
  // will have the new global structure: struct __AOS_t1 { int *a, *b, *c };
  // This routine initializes the pointers of the global variable for a, b, c to
  // point to the appropriate location of the allocated block of memory.
  //
  // The size of the allocation, and uses of the result of the allocation call
  // will also be updated within the function.
  //
  // This function only operates on the pre-cloned version of a function.
  void processAllocCall(dtrans::AllocCallInfo *AInfo, StructInfo *StInfo) {
    auto *AllocCallInst = cast<CallInst>(AInfo->getInstruction());

    StructType *StructTy = cast<StructType>(StInfo->getLLVMType());
    uint64_t StructSize = DL.getTypeAllocSize(StructTy);
    StructType *PeelTy = getTransformedType(StructTy);
    Value *PeelVar = PeeledTypeToVariable[PeelTy];

    // Set up an IR builder to insert instructions starting before the
    // allocation statement. The IR builder will perform constant
    // folding for us when the allocation count is a constant when
    // computing new allocation sizes or offsets into the allocated memory
    // block.
    IRBuilder<> IRB(AllocCallInst);

    // This will store the adjusted number of elements allocated by the call.
    Value *NewAllocCountVal = nullptr;

    AllocKind Kind = AInfo->getAllocKind();
    Value *OrigAllocSizeVal;
    Value *OrigAllocCountVal;
    getAllocSizeArgs(Kind, AllocCallInst, OrigAllocSizeVal, OrigAllocCountVal);

    if (Kind == dtrans::AK_Malloc) {
      assert(OrigAllocSizeVal && "getAllocSizeArgs should return size value");

      // Compute the number of elements being allocated. There is no need to
      // special case this for an allocation size that exactly matches the size
      // of a structure or check for constant values because the IR builder will
      // constant fold the calculations for us, if possible.
      llvm::Type *SizeType = OrigAllocSizeVal->getType();
      Value *StructSizeVal = ConstantInt::get(SizeType, StructSize);
      NewAllocCountVal = IRB.CreateSDiv(OrigAllocSizeVal, StructSizeVal);

      // Update the size allocated to hold one additional structure element.
      // This is necessary because a peeling index value of 0 will represent
      // the nullptr, and the array accesses will be in the range of 1..N.
      // Note: Rather than just adding 12 to the parameter size, we maintain the
      // invariant that the allocation is a multiple of the structure size.
      NewAllocCountVal =
          IRB.CreateAdd(NewAllocCountVal, ConstantInt::get(SizeType, 1));
      Value *NewAllocationSize = IRB.CreateMul(NewAllocCountVal, StructSizeVal);
      AllocCallInst->setOperand(0, NewAllocationSize);
    } else {
      assert(Kind == dtrans::AK_Calloc && "Expected calloc");
      assert(OrigAllocSizeVal && OrigAllocCountVal &&
             "getAllocSizeArgs should return size and count value");

      // Determine the values to use for the number of allocated objects, and
      // size being allocated.
      Value *AllocCountVal = nullptr;
      Value *AllocSizeVal = nullptr;
      if (isValueEqualToSize(OrigAllocSizeVal, StructSize)) {
        AllocCountVal = OrigAllocCountVal;
        AllocSizeVal = OrigAllocSizeVal;
      } else if (isValueEqualToSize(OrigAllocCountVal, StructSize)) {
        // Reverse the size and count parameters.
        AllocCountVal = OrigAllocSizeVal;
        AllocSizeVal = OrigAllocCountVal;
      } else {
        // At this point, we know that either the number allocated or the
        // allocation size is a multiple of the structure size, but not
        // how many elements are being allocated.
        Value *TotalSize = IRB.CreateMul(OrigAllocCountVal, OrigAllocSizeVal);
        AllocSizeVal = ConstantInt::get(TotalSize->getType(), StructSize);
        AllocCountVal = IRB.CreateSDiv(TotalSize, AllocSizeVal);
      }

      // Compute new values for the allocation count and size parameters.
      // We want the count to be 1 larger than the original number that
      // were being allocated, and the size parameter to equal the structure
      // size for the calloc parameters.
      NewAllocCountVal = IRB.CreateAdd(
          AllocCountVal, ConstantInt::get(AllocCountVal->getType(), 1));
      AllocCallInst->setOperand(0, NewAllocCountVal);
      AllocCallInst->setOperand(1, AllocSizeVal);
    }

    assert(NewAllocCountVal &&
           "Expected alloc call processing to set NewAllocCountVal");

    // Pointers in the peeled structure that corresponded to the base allocation
    // address will be accessed via index 1 in the peeled structure.\Update the
    // users of the allocation to refer to index 1. This loop collects the
    // values to be updated to avoid changing the users while walking them.
    SmallVector<StoreInst *, 4> StoresToFix;
    SmallVector<BitCastInst *, 4> BitCastsToFix;

    for (auto *User : AllocCallInst->users()) {
      if (auto *U = dyn_cast<Instruction>(&*User)) {
        if (auto *CmpI = dyn_cast<CmpInst>(U)) {
          // There is nothing to be changed on a comparison of the allocated
          // pointer against a null value. Both are i8* types, and will not
          // be transformed.
          (void)CmpI;
          assert((CmpI->getOperand(0) ==
                      Constant::getNullValue(AllocCallInst->getType()) ||
                  CmpI->getOperand(1) ==
                      Constant::getNullValue(AllocCallInst->getType())) &&
                 "Allocation result used in non-null comparison");
        } else if (auto *SI = dyn_cast<StoreInst>(U)) {
          // We expect the value operand to have been the use, because writing
          // to the allocated memory pointer should have set the safety flags
          // that inhibit the transformation.
          assert(SI->getValueOperand() == AllocCallInst &&
                 "Expected allocation result to be stored value");
          StoresToFix.push_back(SI);
        } else if (auto *BC = dyn_cast<BitCastInst>(U)) {
          // We expect the cast to only be to the type being transformed based
          // on the safety flags.
          assert(BC->getType() == StructTy->getPointerTo() &&
                 "Expected allocation result cast to be to type being "
                 "transformed");
          BitCastsToFix.push_back(BC);
        } else {
          llvm_unreachable("Unexpected instruction using allocation result");
        }
      } else {
        llvm_unreachable("Unexpected use of allocation result");
      }
    }

    // Update the stored value to be an index value of 1.
    for (auto *SI : StoresToFix) {
      CastInst *IndexAsPtr = CastInst::CreateBitOrPointerCast(
          ConstantInt::get(getPeeledIndexType(), 1),
          SI->getOperand(0)->getType());
      IndexAsPtr->insertBefore(SI);
      SI->setOperand(0, IndexAsPtr);
    }

    // Replace the BitCast instructions with an IntToPtr instruction that casts
    // index 1 to the original pointer type. After the type remapping
    // completes, the destination type of the cast will be the peeled index
    // type, and post processing will remove the instruction.
    for (auto *BC : BitCastsToFix) {
      CastInst *IndexAsPtr = CastInst::CreateBitOrPointerCast(
          ConstantInt::get(getPeeledIndexType(), 1), BC->getType());
      IndexAsPtr->insertBefore(BC);
      BC->replaceAllUsesWith(IndexAsPtr);
      InstructionsToDelete.insert(BC);
      PtrConverts.push_back(IndexAsPtr);
    }

    // Initialize the pointer fields of the peeled structure to store an
    // address of the allocated memory block. Padding may be needed
    // to align the start of the array for some elements. When padding
    // is required, we can start the array at the next available aligned
    // address, or we can leave a gap to simulate peeling of the original array
    // padding to get to the aligned address of the field. As an example
    // consider the structure {i32, i64, i32 }, padding may be required between
    // the 1st and 2nd arrays depending on the number of elements being
    // allocated. If we are allocating arrays of length 5 (after the adjustment
    // for the null element), then the array of i64 elements can begin at offset
    // 24 (minimal padding) or at offset 40 (original padding of i32 element is
    // effectively peeled)
    //
    // This version minimizes the padding, but we may revisit this.
    //
    // The memory writes generated below are on the global variable, so will be
    // safe even if the allocation call returns NULL.

    // Use the same type as the allocation parameter for the offset
    // calculations.
    Type *ArithType = NewAllocCountVal->getType();

    unsigned AccumElemSize = 0;
    Value *AddrOffset = ConstantInt::get(ArithType, 0);
    Type *PrevArrayElemType = nullptr;

    // Insert the initialization of the field members to be right after the
    // allocation call.
    IRB.SetInsertPoint(AllocCallInst->getNextNode());
    unsigned int NumElements = PeelTy->getNumElements();
    for (unsigned FieldNum = 0; FieldNum < NumElements; ++FieldNum) {
      Type *PeelFieldType = PeelTy->getElementType(FieldNum);
      Type *ArrayElemType = PeelFieldType->getPointerElementType();

      if (FieldNum != 0) {
        // Compute the offset for the next array based on the size
        // of the previous element's array.
        unsigned PrevElemSize = DL.getTypeAllocSize(PrevArrayElemType);
        AccumElemSize += PrevElemSize;
        Value *Mul = IRB.CreateMul(NewAllocCountVal,
                                   ConstantInt::get(ArithType, PrevElemSize));

        if (AddrOffset == ConstantInt::get(ArithType, 0))
          AddrOffset = Mul;
        else
          AddrOffset = IRB.CreateAdd(AddrOffset, Mul);

        // Add padding, if needed.
        uint64_t FieldAlign = DL.getABITypeAlignment(ArrayElemType);
        uint64_t OldAccumElemSize = AccumElemSize;
        if ((AccumElemSize & (FieldAlign - 1)) != 0)
          AccumElemSize = alignTo(AccumElemSize, FieldAlign);

        unsigned Padding = AccumElemSize - OldAccumElemSize;
        if (Padding)
          AddrOffset =
              IRB.CreateAdd(AddrOffset, ConstantInt::get(ArithType, Padding));
      }

      // Compute the address in the memory block where the array for this field
      // will begin:
      //   %BlockAddr = getelementptr i8, i8* %AllocCallInst, i64/i32 %Offset
      Value *BlockAddr = IRB.CreateGEP(AllocCallInst, AddrOffset);

      // Cast to the pointer type that will be stored:
      //   %CastToMemberTy = bitcast i8* %BlockAddr to %PeelFieldType
      Value *CastToMemberTy = IRB.CreateBitCast(BlockAddr, PeelFieldType);

      // Get the address in the global structure for the field to be stored:
      //   %FieldAddr = getelementptr %PeelVarType, %PeelVarType* %PeelVar i32
      //                %FieldNum
      Value *Idx[2];
      Idx[0] = Constant::getNullValue(Type::getInt64Ty(Context));
      Idx[1] = ConstantInt::get(Type::getInt32Ty(Context), FieldNum);
      Value *FieldAddr = IRB.CreateGEP(PeelTy, PeelVar, Idx);

      // Save the pointer to the global structure field.
      //   store %CastToMemberTy, %FieldAddr
      IRB.CreateStore(CastToMemberTy, FieldAddr);

      PrevArrayElemType = ArrayElemType;
    }
  }

  // The transformation of the call to free needs to change the parameter
  // passed to 'free' to be the address of our peeled global variable.
  void processFreeCall(FreeCallInfo *CInfo, StructInfo *StInfo) {
    Instruction *FreeCall = CInfo->getInstruction();
    assert(FreeCall && isa<CallInst>(FreeCall) &&
           "Instruction should be function call");

    // To free the transformed data structure, we need to get the address
    // of the first field stored within the global variable, and pass that
    // to free.
    StructType *StructTy = cast<StructType>(StInfo->getLLVMType());
    StructType *PeelTy = getTransformedType(StructTy);
    Value *PeelVar = PeeledTypeToVariable[PeelTy];
    Instruction *SOAAddr = createPeelFieldLoad(
        PeelTy, PeelVar, ConstantInt::get(Type::getInt32Ty(Context), 0),
        FreeCall);

    // The peeled structure only contains pointer types, and the value of the
    // first field is always the address that the allocation call returned. This
    // will be safe even if the memory allocation failed, because the nullptr
    // returned by the allocation call will be the value stored. Cast the value
    // loaded to an i8*, and update the parameter to free.
    CastInst *SOAAddrAsI8Ptr =
        CastInst::CreateBitOrPointerCast(SOAAddr, getInt8PtrType());
    SOAAddrAsI8Ptr->insertBefore(FreeCall);

    LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Before modifying free call: "
                      << *FreeCall << "\n");
    FreeCall->setOperand(0, SOAAddrAsI8Ptr);
    LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: After modifying free call: "
                      << *FreeCall << "\n");
  }

  // For bitcasts from pointers to the type being transformed, we
  // need to update them because the pointer is going to be converted
  // to an integer type during type remapping. We also need to do this
  // if the cast is to the type being converted.
  void processBitcast(BitCastInst *BC) {

    // The bitcast may no longer be needed after processing the byte flattened
    // GEPs.
    if (BC->user_empty()) {
      InstructionsToDelete.insert(BC);
      return;
    }

    // Convert a bitcast from or to a pointer of the type being transformed:
    //
    // From: %y = bitcast %struct.ty* %x to i8*
    // To  : %cast1 = ptrtoint %struct.ty* %x to i64
    //        %y = inttoptr i64 %cast1 to i8*
    //
    // From: %y = bitcast i8* %x to %struct.ty*
    // To  : %cast1 = ptrtoint i8* %x to i64
    //       %y = inttoptr i64 %cast1 to struct.ty*
    //
    // This will makes the types consistent. During post-processing
    // the one of the int-ptr conversions will just be a conversion
    // from i64 to i64, and will be removed.
    CastInst *ToInt = CastInst::CreateBitOrPointerCast(
        BC->getOperand(0), getPeeledIndexType(), "", BC);
    CastInst *ToPtr =
        CastInst::CreateBitOrPointerCast(ToInt, BC->getType(), "", BC);

    // Mark one of the casts for remove, and steal the name for the other
    if (isTypeToTransform(BC->getType()->getPointerElementType())) {
      PtrConverts.push_back(ToPtr);
      ToInt->takeName(BC);
    } else {
      PtrConverts.push_back(ToInt);
      ToPtr->takeName(BC);
    }

    BC->replaceAllUsesWith(ToPtr);
    InstructionsToDelete.insert(BC);
  }

private:
  // Return an integer type that will be used as a replacement type for pointers
  // to the types being peeled.
  llvm::Type *getPeeledIndexType() const { return PeelIndexType; }
  uint64_t getPeeledIndexWidth() const { return PeelIndexWidth; }
  llvm::Type *getPtrSizedIntType() const { return PtrSizedIntType; }
  llvm::Type *getInt8PtrType() const { return Int8PtrType; }

  bool isTypeToTransform(llvm::Type *Ty) {
    if (!Ty->isStructTy())
      return false;

    // Since we expect to have very few types to check, just loop over them.
    // This could be converted to OrigToNewTypeMapping.count(Ty) in the
    // future, if it is found that is faster.
    for (auto &ONPair : OrigToNewTypeMapping)
      if (ONPair.first == Ty)
        return true;

    return false;
  }

  // Helper function to get the Type for \p CallInfo cases
  // that can be transformed. If the CallInfo is for a case that is not
  // being transformed, return nullptr.
  llvm::Type *getCallInfoTypeToTransform(dtrans::CallInfo *CInfo) {
    auto &TypeList = CInfo->getPointerTypeInfoRef().getTypes();

    // Only cases with a single type will be allowed during the transformation.
    // If there's more than one, we don't need the type, because we won't be
    // transforming it.
    if (TypeList.size() != 1)
      return nullptr;

    llvm::Type *Ty = *TypeList.begin();
    if (!Ty->isPointerTy())
      return nullptr;

    llvm::Type *ElemTy = Ty->getPointerElementType();
    if (!isTypeToTransform(ElemTy))
      return nullptr;

    return ElemTy;
  }

  // Get the new type for a structure type being transformed. This function
  // can only be used for types that are known to be transformed to avoid
  // the need for nullptr checks on the return value, and to allow casting
  // the result from the map to be a StructType.
  llvm::StructType *getTransformedType(llvm::Type *OrigTy) {
    assert(isTypeToTransform(OrigTy) &&
           "Expected input type to be type being transformed");

    // Since we expect to have very few types to check, just loop over them.
    // This could be converted to OrigToNewTypeMapping.count(Ty) in the
    // future, if it is found that is faster.
    for (auto &ONPair : OrigToNewTypeMapping)
      if (ONPair.first == OrigTy)
        return cast<StructType>(ONPair.second);

    llvm_unreachable("AOS-to-SOA transformation type not found");
  }

  // When rewriting pointers to the transformed structure type to integer
  // types, attributes on the return type and function parameters may need to
  // be updated because some attributes are only allowed on pointer
  // parameters. This routine updates callsites in the original function prior
  // to the clone function creation.
  void updateCallAttributes(CallSite &CS) {
    bool Changed = false;
    AttributeList Attrs = CS.getAttributes();

    // Only calls to functions to be cloned (or indirect calls) need to be
    // checked because the parameter types will not be changing for any
    // other calls.
    Function *Callee = CS.getCalledFunction();
    if (Callee && !OrigFuncToCloneFuncMap.count(Callee))
      return;

    LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Updating callsite attributes for: "
                      << *CS.getInstruction() << "\n");

    Type *OrigRetTy = CS.getType();
    if (OrigRetTy->isPointerTy() &&
        OrigToNewTypeMapping.count(OrigRetTy->getPointerElementType())) {
      // Argument index 0 is used for return type attributes
      Attrs = Attrs.removeAttributes(Context, 0, IncompatiblePeelTypeAttrs);
      Changed = true;
    }

    // Argument index numbers start with 1 for calls to removeAttributes.
    unsigned Idx = 1;
    for (auto &Arg : CS.args()) {
      Type *ArgTy = Arg->getType();
      if (ArgTy->isPointerTy() &&
          OrigToNewTypeMapping.count(ArgTy->getPointerElementType())) {
        Attrs = Attrs.removeAttributes(Context, Idx, IncompatiblePeelTypeAttrs);
        Changed = true;
      }
      ++Idx;
    }

    if (Changed)
      CS.setAttributes(Attrs);

    LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: After callsite update: "
                      << *CS.getInstruction() << "\n");
  }

  // The list of types to be transformed.
  SmallVector<dtrans::StructInfo *, 4> TypesToTransform;

  // A mapping from the original structure type to the new structure type
  TypeToTypeMap OrigToNewTypeMapping;

  // A mapping from the peeled structure type to the global variable used to
  // access it.
  DenseMap<StructType *, GlobalVariable *> PeeledTypeToVariable;

  // Pointers to the peeled structure will be converted to an index
  // value for the array element of the structure of arrays. These
  // variables hold the integer width and type that will used for the index.
  uint64_t PeelIndexWidth;
  llvm::Type *PeelIndexType;

  // Integer type that has the same size as a pointer type. This may be
  // different than the peel index type.
  llvm::Type *PtrSizedIntType;

  // i8* Type
  llvm::Type *Int8PtrType;

  // When the pointer to the peeled structure is converted, there are several
  // attributes that may have been used on function parameters or return
  // types that are not valid on the index type being used. This variable
  // holds the list of incompatible attributes that need to be removed
  // from function signatures and call sites for the cloned routines.
  AttrBuilder IncompatiblePeelTypeAttrs;

  // A list of pointer conversion instructions (contains instructions that are
  // pointer to int, int to pointer, or pointer of original type to pointer of
  // remapped type) that after the remapping process should have the same source
  // and destination types, and are to be removed during function post
  // processing. This list is reset for each function processed.
  SmallVector<CastInst *, 16> PtrConverts;

  // When processing byte-flattened GEPs and memfuncs some instructions may be
  // processed to get the peeling index. This cache is used to get the
  // processed values so they do not need to be recomputed for multiple
  // instructions being processed. This is also necessary to avoid cycles when
  // walking PHI nodes. This map needs be cleared after processing each
  // function.
  DenseMap<Value *, Value *> PeelIndexCache;

  // List of instructions that are to be removed at the end of
  // processFunction().
  SmallPtrSet<Instruction *, 16> InstructionsToDelete;
};

class DTransAOSToSOAWrapper : public ModulePass {
private:
  dtrans::AOSToSOAPass Impl;

public:
  static char ID;

  DTransAOSToSOAWrapper() : ModulePass(ID) {
    initializeDTransAOSToSOAWrapperPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto &DTInfo = getAnalysis<DTransAnalysisWrapper>().getDTransInfo();
    auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

    // This lambda function is to allow getting the DominatorTree analysis for a
    // specific function to allow analysis of loops for the dynamic allocation
    // of the structure.
    dtrans::AOSToSOAPass::DominatorTreeFuncType GetDT =
        [this](Function &F) -> DominatorTree & {
      return this->getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    return Impl.runImpl(M, DTInfo, TLI, GetDT);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    // TODO: Mark the actual required and preserved analyses.
    AU.addRequired<DTransAnalysisWrapper>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
  }
};

} // end anonymous namespace

char DTransAOSToSOAWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(DTransAOSToSOAWrapper, "dtrans-aostosoa",
                      "DTrans array of structs to struct of arrays", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(DTransAnalysisWrapper)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(DTransAOSToSOAWrapper, "dtrans-aostosoa",
                    "DTrans array of structs to struct of arrays", false, false)

ModulePass *llvm::createDTransAOSToSOAWrapperPass() {
  return new DTransAOSToSOAWrapper();
}

namespace llvm {
namespace dtrans {

bool AOSToSOAPass::runImpl(Module &M, DTransAnalysisInfo &DTInfo,
                           const TargetLibraryInfo &TLI,
                           AOSToSOAPass::DominatorTreeFuncType &GetDT) {
  // Check whether there are any candidate structures that can be transformed.
  StructInfoVec CandidateTypes;
  gatherCandidateTypes(DTInfo, CandidateTypes);
  if (CandidateTypes.empty())
    return false;

  qualifyCandidates(CandidateTypes, M, DTInfo, GetDT);

  if (CandidateTypes.empty())
    return false;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Temporary code to allow testing of qualification criteria without having
  // implemented transformation code. Currently, there is no profitability
  // heuristic so only cases specified with the dtrans-aostosoa-heur-override
  // option will reach this point.
  if (DTransAOSToSOAQualificationOnly)
    return false;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Perform the actual transformation.
  DTransTypeRemapper TypeRemapper;
  AOSToSOAMaterializer Materializer(TypeRemapper);
  AOSToSOATransformImpl Transformer(DTInfo, M.getContext(), M.getDataLayout(),
                                    "__SOADT_", &TypeRemapper, &Materializer,
                                    CandidateTypes);
  return Transformer.run(M);
}

// Populate the \p CandidateTypes vector with all the structure types
// that meet the minimum safety conditions to be considered for transformation.
void AOSToSOAPass::gatherCandidateTypes(DTransAnalysisInfo &DTInfo,
                                        StructInfoVecImpl &CandidateTypes) {
  const dtrans::SafetyData AOSToSOASafetyConditions =
      dtrans::BadCasting | dtrans::BadAllocSizeArg |
      dtrans::BadPtrManipulation | dtrans::AmbiguousGEP | dtrans::VolatileData |
      dtrans::MismatchedElementAccess | dtrans::WholeStructureReference |
      dtrans::UnsafePointerStore | dtrans::FieldAddressTaken |
      dtrans::GlobalInstance | dtrans::HasInitializerList |
      dtrans::UnsafePtrMerge | dtrans::BadMemFuncSize |
      dtrans::BadMemFuncManipulation | dtrans::AmbiguousPointerTarget |
      dtrans::AddressTaken | dtrans::NoFieldsInStruct | dtrans::NestedStruct |
      dtrans::ContainsNestedStruct | dtrans::SystemObject |
      dtrans::LocalInstance;

  for (dtrans::TypeInfo *TI : DTInfo.type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    if (TI->testSafetyData(AOSToSOASafetyConditions)) {
      LLVM_DEBUG(
          dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported safety data: "
                 << getStructName(TI->getLLVMType()) << "\n");
      continue;
    }

    CandidateTypes.push_back(cast<StructInfo>(TI));
  }
}

// This routine examines all the candidates and performs additional safety
// checks on the type and usage to determine whether a type is supported for
// being transformed. The \p CandidateTypes list will be updated to only contain
// the elements that pass all the safety checks.
void AOSToSOAPass::qualifyCandidates(
    StructInfoVecImpl &CandidateTypes, Module &M, DTransAnalysisInfo &DTInfo,
    AOSToSOAPass::DominatorTreeFuncType &GetDT) {
  if (!qualifyCandidatesTypes(CandidateTypes, DTInfo))
    return;

  if (!qualifyAllocations(CandidateTypes, DTInfo, GetDT))
    return;

  if (!qualifyHeuristics(CandidateTypes, M, DTInfo))
    return;

  LLVM_DEBUG({
    for (auto *Candidate : CandidateTypes)
      dbgs() << "DTRANS-AOSTOSOA: Passed qualification tests: "
             << getStructName(Candidate->getLLVMType()) << "\n";
  });
}

// Check for any types that are not supported for the transformation.
// 1. Types that are used as arrays are not supported, for example
//     [4 x struct.test], because we would need to handle all the allocation
//     checks and transformation code for these arrays, as well.
// 2. Types that contain arrays are not supported. This restriction could be
//     relaxed in a future version.
// 3. Types that contain vectors are not supported.
//
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyCandidatesTypes(StructInfoVecImpl &CandidateTypes,
                                          DTransAnalysisInfo &DTInfo) {
  // Collect a set of structure types that are arrays composed of structure
  // types so that we can check if any of these match the candidate types.
  SmallPtrSet<dtrans::StructInfo *, 4> ArrayElemTypes;
  for (auto *TI : DTInfo.type_info_entries()) {
    if (!isa<dtrans::ArrayInfo>(TI))
      continue;

    Type *ElemTy = TI->getLLVMType()->getArrayElementType();
    while (isa<ArrayType>(ElemTy))
      ElemTy = ElemTy->getArrayElementType();

    if (!isa<StructType>(ElemTy))
      continue;

    auto *StInfo = cast<dtrans::StructInfo>(DTInfo.getTypeInfo(ElemTy));
    ArrayElemTypes.insert(StInfo);
  }

  StructInfoVec Qualified;
  for (auto *Candidate : CandidateTypes) {
    if (ArrayElemTypes.find(Candidate) != ArrayElemTypes.end()) {
      LLVM_DEBUG(dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Array of type seen: "
                        << getStructName(Candidate->getLLVMType()) << "\n");
      continue;
    }

    // No arrays of the structure type were found, now check the structure
    // field types to verify all members are supported for the transformation.
    // Reject any that contain arrays or vectors. We don't need to check for
    // structures because those were rejected by the safety checks.
    bool Supported = true;
    for (auto &FI : Candidate->getFields()) {
      Type *Ty = FI.getLLVMType();
      if (Ty->isArrayTy() || Ty->isVectorTy()) {
        Supported = false;
        break;
      }
    }

    if (Supported)
      Qualified.push_back(Candidate);
    else
      LLVM_DEBUG(
          dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported structure "
                    "element type: "
                 << getStructName(Candidate->getLLVMType()) << "\n");
  }

  std::swap(CandidateTypes, Qualified);
  return !CandidateTypes.empty();
}

// Check that the type is only allocated once by malloc or calloc.
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyAllocations(StructInfoVecImpl &CandidateTypes,
                                      DTransAnalysisInfo &DTInfo,
                                      DominatorTreeFuncType &GetDT) {
  // Build a mapping from each allocated type to a single allocating
  // instruction, if one exists. If there are multiple allocations or an
  // unsupported allocation, map the type to 'nullptr'.
  DenseMap<dtrans::StructInfo *, Instruction *> TypeToAllocInstr;
  for (auto *Call : DTInfo.call_info_entries()) {
    auto *ACI = dyn_cast<dtrans::AllocCallInfo>(Call);
    if (!ACI || !ACI->getAliasesToAggregatePointer())
      continue;

    // We do not support transforming any allocations that are not
    // calloc/malloc. Invalidate the information for all types.
    if (ACI->getAllocKind() != dtrans::AK_Calloc &&
        ACI->getAllocKind() != dtrans::AK_Malloc) {
      for (auto *AllocatedTy : ACI->getPointerTypeInfoRef().getTypes()) {
        auto *Ty = AllocatedTy->getPointerElementType();
        auto *TI = DTInfo.getTypeInfo(Ty);
        if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
          LLVM_DEBUG({
            if (std::find(CandidateTypes.begin(), CandidateTypes.end(),
                          StInfo) != CandidateTypes.end() &&
                (!TypeToAllocInstr.count(StInfo) ||
                 TypeToAllocInstr[StInfo] != nullptr))
              dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Unsupported "
                        "allocation function: "
                     << getStructName(Ty) << "\n"
                     << "  " << *ACI->getInstruction() << "\n";
          });

          TypeToAllocInstr[StInfo] = nullptr;
        }
      }

      continue;
    }

    // For supported allocations, update the association between the type and
    // allocating instruction.
    for (auto *AllocatedTy : ACI->getPointerTypeInfoRef().getTypes()) {
      auto *Ty = AllocatedTy->getPointerElementType();
      auto *TI = DTInfo.getTypeInfo(Ty);
      if (auto *StInfo = dyn_cast<dtrans::StructInfo>(TI)) {
        if (TypeToAllocInstr.count(StInfo)) {
          LLVM_DEBUG({
            if (std::find(CandidateTypes.begin(), CandidateTypes.end(),
                          StInfo) != CandidateTypes.end() &&
                TypeToAllocInstr[StInfo] != nullptr)
              dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Too many allocations: "
                     << getStructName(Ty) << "\n";
          });
          TypeToAllocInstr[StInfo] = nullptr;
          continue;
        }

        TypeToAllocInstr[StInfo] = ACI->getInstruction();
      }
    }
  }

  // Select the types that passed the single allocation location test. Also,
  // populate a set of instructions  by function that need to be checked to
  // verify the allocation is not within a loop. We group these by function so
  // that the LoopInfo for a function only needs to be calculated one time.
  //
  // Note: Currently this does not reject a type if there is no dynamic
  // allocation of the  type. This may need to be revisited when implementing
  // the transformation.
  StructInfoVec Qualified;
  DenseMap<Function *, DenseSet<std::pair<Instruction *, dtrans::StructInfo *>>>
      AllocPathMap;
  SmallVector<std::pair<Function *, Instruction *>, 4> CallChain;
  for (auto *TyInfo : CandidateTypes) {
    if (TypeToAllocInstr.count(TyInfo)) {
      if (TypeToAllocInstr[TyInfo] == nullptr)
        continue;

      Instruction *I = TypeToAllocInstr[TyInfo];
      Value *Unsupported;
      if (!supportedAllocationUsers(I, TyInfo->getLLVMType(), &Unsupported)) {
        LLVM_DEBUG(
            dbgs()
            << "DTRANS-AOSTOSOA: Rejecting -- Unsupported allocation usage: "
            << getStructName(TyInfo->getLLVMType()) << "\n"
            << "  " << *Unsupported << "\n");
        continue;
      }

      // Verify the call chain to the instruction consists of a single path
      CallChain.clear();
      if (!collectCallChain(I, CallChain)) {
        LLVM_DEBUG(
            dbgs() << "DTRANS-AOSTOSOA: Rejecting -- Multiple call paths: "
                   << TyInfo->getLLVMType()->getStructName() << "\n");
        continue;
      }

      // Save the instruction and all it's caller to the list of locations that
      // will need to be checked for being within loops.
      AllocPathMap[I->getParent()->getParent()].insert(
          std::make_pair(I, TyInfo));
      for (auto &FuncInstrPair : CallChain)
        AllocPathMap[FuncInstrPair.first].insert(
            std::make_pair(FuncInstrPair.second, TyInfo));
    }

    Qualified.push_back(TyInfo);
  }

  std::swap(CandidateTypes, Qualified);
  if (CandidateTypes.empty())
    return false;

  // check the function's loops to see if the allocation (or call to the
  // allocation) instruction is within a loop
  for (auto &FuncToAllocPath : AllocPathMap) {
    Function *F = FuncToAllocPath.first;
    DominatorTree &DT = (GetDT)(*F);
    LoopInfo LI(DT);

    if (LI.size())
      for (auto &InstTypePair : FuncToAllocPath.second)
        if (LI.getLoopFor(InstTypePair.first->getParent())) {
          StructInfo *StInfo = InstTypePair.second;
          LLVM_DEBUG(dbgs()
                     << "DTRANS-AOSTOSOA: Rejecting -- Allocation in loop: "
                     << StInfo->getLLVMType()->getStructName()
                     << "\n  Function: " << F->getName() << "\n");
          auto *It =
              std::find(CandidateTypes.begin(), CandidateTypes.end(), StInfo);
          if (It != CandidateTypes.end())
            CandidateTypes.erase(It);
        }
  }

  return !CandidateTypes.empty();
}

// The result of the allocation call is an i8* type. Because we are going to be
// replacing the ultimate pointer assignment during the transformation with an
// integer type, and storing the result of the allocation into a compiler
// generated structure, we need to perform some additional checks to make sure
// the allocation can be replaced. Specifically, the following uses will be
// allowed, and all others rejected:
//   icmp eq i8* %call, null   [also allow inequality, and reversed operands]
//   bitcast i8* %call to %struct.type* [ only allow cast to candidate type ptr]
//   store i8* %call, bitcast (@global to i8**)
//
// In the future, this may be extended to allow memset, memcpy, or memmove
// using the i8* result of the allocation directly.
// Using the result in a PHI/Select instructions could be safe, but will require
// additional analysis, and handling when processing the allocation call, so is
// not supported at this time.
bool AOSToSOAPass::supportedAllocationUsers(Instruction *AllocCall,
                                            llvm::Type *StructTy,
                                            Value **Unsupported) {
  assert(isa<CallInst>(AllocCall) &&
         "Instruction expected to be allocation call");

  *Unsupported = nullptr;
  for (auto *User : AllocCall->users()) {
    auto *I = dyn_cast<Instruction>(&*User);
    if (!I) {
      *Unsupported = User;
      return false;
    }

    switch (I->getOpcode()) {
    default:
      *Unsupported = I;
      return false;
    case Instruction::ICmp:
      if ((cast<ICmpInst>(I))->isRelational()) {
        *Unsupported = I;
        return false;
      }
      if (!(I->getOperand(0) ==
                Constant::getNullValue(I->getOperand(0)->getType()) ||
            I->getOperand(1) ==
                Constant::getNullValue(I->getOperand(1)->getType()))) {
        *Unsupported = I;
        return false;
      }
      break;
    case Instruction::Store:
      if ((cast<StoreInst>(I))->getPointerOperand() == AllocCall) {
        *Unsupported = I;
        return false;
      }
      break;
    case Instruction::BitCast:
      if (I->getType() != StructTy->getPointerTo()) {
        *Unsupported = I;
        return false;
      }
      break;
    }
  }

  return true;
}

// If there is a single call chain that reaches the instruction, \p I,
// add the function to the \p CallChain, and return 'true'. Otherwise, return
// 'false'.
bool AOSToSOAPass::collectCallChain(
    Instruction *I,
    SmallVectorImpl<std::pair<Function *, Instruction *>> &CallChain) {
  Function *F = I->getParent()->getParent();
  Instruction *Callsite = nullptr;

  for (auto *U : F->users()) {
    if (auto *Call = dyn_cast<CallInst>(*&U)) {
      // Check that we only have a single call path to the routine.
      if (Callsite != nullptr)
        return false;

      Callsite = Call;
    } else {
      return false;
    }
  }

  // Verify the top of the callchain is the 'main' routine
  if (!Callsite)
    return F->getName() == "main";

  CallChain.push_back(
      std::make_pair(Callsite->getParent()->getParent(), Callsite));
  return collectCallChain(Callsite, CallChain);
}

// Filter the \p CandidateTypes list based on whether the type meets
// the criteria of the profitability heuristics.
// Return 'true' if candidates remain after this filtering.
bool AOSToSOAPass::qualifyHeuristics(StructInfoVecImpl &CandidateTypes,
                                     Module &M, DTransAnalysisInfo &DTInfo) {
  StructInfoVec Qualified;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Check for any command line structures that do not need to meet the
  // profitability heuristics, and add them to the Qualified list.
  SmallVector<StringRef, 4> SubStrings;
  if (!DTransAOSToSOAHeurOverride.empty()) {
    SplitString(DTransAOSToSOAHeurOverride, SubStrings, ",");
    for (auto &Name : SubStrings) {
      Type *Ty = M.getTypeByName(Name);
      if (auto *StructTy = dyn_cast_or_null<StructType>(Ty)) {
        LLVM_DEBUG(
            dbgs()
            << "DTRANS-AOSTOSOA: Skipped profitability heuristics for type: "
            << Name << "\n");
        dtrans::TypeInfo *Info = DTInfo.getTypeInfo(StructTy);
        assert(Info &&
               "DTransAnalysisInfo does not contain info for structure");

        // Only allow the heuristic override to enable cases that actually
        // met the required safety conditions.
        dtrans::StructInfo *StInfo = cast<dtrans::StructInfo>(Info);
        if (std::find(CandidateTypes.begin(), CandidateTypes.end(), StInfo) ==
            CandidateTypes.end()) {
          LLVM_DEBUG(dbgs()
                     << "DTRANS-AOSTOSOA: Cannot force transformation on type "
                        "that fails safety checks: "
                     << Name << "\n");
          continue;
        }
        Qualified.push_back(StInfo);
      }
    }
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // TODO: Add the type to the qualified list if it passes the heuristic
  // check. For now, we reject everything not explicitly added above, by
  // not placing them in the Qualified list.

  std::swap(CandidateTypes, Qualified);
  return !CandidateTypes.empty();
}

PreservedAnalyses AOSToSOAPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransAnalysis>(M);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  DominatorTreeFuncType GetDT = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  bool Changed = runImpl(M, DTransInfo, TLI, GetDT);

  if (!Changed)
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DTransAnalysis>();
  return PA;
}

} // end namespace dtrans
} // end namespace llvm
