//==--- MapIntrinToIml.cpp - Legalize svml calls and apply IMF -*- C++ -*---==//
//                           attributes.
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// Main author:
// ------------
// Matt Masten (C) 2015 [matt.masten@intel.com]
//
// Major revisions:
// ----------------
// July 2015, initial development -- Matt Masten
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This pass does two things:
///   1) Consumes svml calls emitted by the vectorizer and legalizes them.
///   2) Refines the svml calls based in the imf attributes.
///
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_MapIntrinToIml/MapIntrinToIml.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

#define SV_NAME "iml-trans"
#define DEBUG_TYPE "MapIntrinToIml"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool> RunSvmlStressMode(
    "svml-trans-stress-mode", cl::init(false), cl::Hidden,
    cl::desc("Scalarize svml calls."));

MapIntrinToIml::MapIntrinToIml() : FunctionPass(ID) {
  initializeMapIntrinToImlPass(*PassRegistry::getPassRegistry());
}

// The idea of this pass is simple and involves these steps:
// 1) Find svml calls.
// 2) Get the IMF attributes in the form of LLVM Attributes from the call
//    arguments.
// 3) Map each call Attribute to an ImfAttr struct, building a list of
//    attribute value pairs <char* attr_name, char* attr_value> that can be
//    used by the get_library_function_name() function found in libiml_attr.
//    This is the external interface used to do the actual mapping of math
//    library calls to legal SVML/libm functions. The ImfAttr struct is
//    populated using the strings below for attribute names. The value of the
//    attribute is just a string also. libiml_attr will then select the
//    appropriate SVML/libm function based on the function name and attributes.
//
// valid_attributes_names defined in iml_accuracy_interface.c in libiml_attr.a.
//
// "absolute-error"
// "accuracy-bits"
// "accuracy-bits-128"
// "accuracy-bits-32"
// "accuracy-bits-64"
// "accuracy-bits-80"
// "arch-consistency"
// "configuration"
// "domain-exclusion"
// "max-error"
// "precision"
// "valid-status-bits"

void MapIntrinToImlImpl::addAttributeToList(ImfAttr **List, ImfAttr **Tail,
                                            ImfAttr *Attr) {
  if (*Tail)
    (*Tail)->next = Attr;
  else
    *List = Attr;

  *Tail = Attr;
}

void MapIntrinToImlImpl::deleteAttributeList(ImfAttr **List) {
  ImfAttr *Attr = *List;

  while (Attr) {
    ImfAttr *Next = Attr->next;
    delete Attr;
    Attr = Next;
  }
}

// Unfortunately, this function exists because valid_attribute_names in
// iml_accuracy_interface.c is declared static. TODO: find out if this
// can be changed so that we can reference it directly to avoid having
// to maintain two pieces of code.
bool MapIntrinToImlImpl::isValidIMFAttribute(std::string AttrName) {
  if (AttrName == "absolute-error" || AttrName == "accuracy-bits" ||
      AttrName == "accuracy-bits-128" || AttrName == "accuracy-bits-32" ||
      AttrName == "accuracy-bits-64" || AttrName == "accuracy-bits-80" ||
      AttrName == "arch-consistency" || AttrName == "configuration" ||
      AttrName == "domain-exclusion" || AttrName == "max-error" ||
      AttrName == "precision" || AttrName == "valid-status-bits")
    return true;

  return false;
}

unsigned MapIntrinToImlImpl::calculateNumReturns(TargetTransformInfo *TTI,
                                                 unsigned TypeBitWidth,
                                                 unsigned LogicalVL,
                                                 unsigned *TargetVL) {
  unsigned VectorBitWidth = TTI->getRegisterBitWidth(true);
  *TargetVL = VectorBitWidth / TypeBitWidth;
  unsigned NumRet = LogicalVL / *TargetVL;

  // If the logical vector width is smaller than the target vector width,
  // then we have less than full vector. Thus, just set NumRet = 1.
  NumRet = NumRet == 0 ? 1 : NumRet;

  LLVM_DEBUG(dbgs() << "Type Bit Width: " << TypeBitWidth << "\n");
  LLVM_DEBUG(dbgs() << "Legalizing VL: " << LogicalVL << "\n");
  LLVM_DEBUG(dbgs() << "Vector Bit Width: " << VectorBitWidth << "\n");
  LLVM_DEBUG(dbgs() << "Legal Target VL: " << *TargetVL << "\n");
  LLVM_DEBUG(dbgs() << "Num Regs: " << NumRet << "\n");

  return NumRet;
}

void MapIntrinToImlImpl::splitArgs(
    SmallVectorImpl<Value *> &Args,
    SmallVectorImpl<SmallVector<Value *, 8>> &NewArgs, unsigned NumRet,
    unsigned TargetVL) {

  LLVM_DEBUG(dbgs() << "Splitting Args to match legal VL:\n");
  NewArgs.resize(NumRet);

  for (unsigned I = 0; I < Args.size(); I++) {

    LLVM_DEBUG(dbgs() << "Arg Name: ");
    LLVM_DEBUG(Args[I]->dump());
    LLVM_DEBUG(dbgs() << "\n");
    LLVM_DEBUG(dbgs() << "Arg Type: " << *Args[I]->getType() << "\n");

    for (unsigned J = 0; J < NumRet; J++) {

      // Use shuffle instructions to split the parameters into pseudo-registers
      // of size TargetVL. For example, if the logical VL = 8, and TargetVL = 4,
      // then the vector parameter will be split into two registers, where the
      // shuffle masks correspond to the following:
      //
      // Shuffle Mask 1:
      // <4 x i32> <i32 0, i32 1, i32 2, i32 3>
      //
      // Shuffle Mask 2:
      // <4 x i32> <i32 4, i32 5, i32 6, i32 7>
      //
      // The resulting registers will hold the elements corresponding to the
      // masks.
      //

      SmallVector<Constant *, 8> Splat;
      unsigned StartElemIdx = J * TargetVL;
      unsigned ElemIdx = StartElemIdx;

      for (unsigned K = 0; K < TargetVL; K++) {
        Constant *ConstVal =
            ConstantInt::get(Type::getInt32Ty(Func->getContext()), ElemIdx);
        Splat.push_back(ConstVal);
        ++ElemIdx;
      }

      Value *Undef = UndefValue::get(Args[I]->getType());
      Value *Mask = ConstantVector::get(Splat);

      ShuffleVectorInst *ShuffleInst =
          new ShuffleVectorInst(Args[I], Undef, Mask, "shuffle");

      NewArgs[J].push_back(ShuffleInst);
    }
  }
}

void MapIntrinToImlImpl::createImfAttributeList(CallInst *CI, ImfAttr **List) {

  // Tail of the linked list of IMF attributes. The head of the list is
  // passed in from the caller via the List parameter.
  ImfAttr *Tail = nullptr;

  // Set default precision to high accuracy. For bitwise reproducible svml
  // functions, the iml accuracy inferface expects these attributes to appear
  // before imf-arch-consistency.
  ImfAttr *MaxError = new ImfAttr();
  MaxError->name = "max-error";
  MaxError->value = "0.6";
  ImfAttr *Precision = new ImfAttr();
  Precision->name = "precision";
  Precision->value = "high";
  MaxError->next = Precision;
  Precision->next = nullptr;
  addAttributeToList(List, &Tail, MaxError);
  addAttributeToList(List, &Tail, Precision);

  // Build the linked list of IMF attributes that will be used to query
  // the IML interface.

  const StringRef ImfPrefix = "imf-";
  const AttributeList AttrList = CI->getAttributes();

  if (AttrList.hasAttributes(AttributeList::FunctionIndex)) {

    AttributeSet Attrs = AttrList.getFnAttributes();

    AttributeSet::iterator FAIt = Attrs.begin();
    AttributeSet::iterator FAEnd = Attrs.end();
    for (; FAIt != FAEnd; ++FAIt) {
      // Attributes will be of the form:
      //
      // "imf-max-error"="0.5"
      //
      std::string AttrStr = FAIt->getAsString();
      size_t EqualIdx = AttrStr.find("=");
      if (EqualIdx == std::string::npos)
        continue;

      // Ignore leading and trailing quotes, and '='
      std::string AttrName = AttrStr.substr(1, EqualIdx - 2);
      size_t QuoteIdx = AttrStr.rfind('"');
      if (QuoteIdx == std::string::npos)
        continue;

      std::string AttrValue =
          AttrStr.substr(EqualIdx + 2, QuoteIdx - (EqualIdx + 2));

      // Make sure this is an IMF attribute by looking for the prefix of
      // "imf-".
      if (AttrName.find(ImfPrefix) != 0)
        continue;
      AttrName = AttrName.substr(ImfPrefix.size(), std::string::npos);

      if (isValidIMFAttribute(AttrName)) {
        ImfAttr *Attribute = new ImfAttr();
        char *Name = new char[AttrName.length() + 1];
        std::strcpy(Name, AttrName.c_str());
        char *Value = new char[AttrValue.length() + 1];
        std::strcpy(Value, AttrValue.c_str());
        Attribute->name = Name;
        Attribute->value = Value;
        Attribute->next = nullptr;
        addAttributeToList(List, &Tail, Attribute);
      }
    }
  }

  // TODO: only debug mode
  ImfAttr *CurrAttr = *List;
  LLVM_DEBUG(dbgs() << "Attribute List for function:\n");
  while (CurrAttr) {
    LLVM_DEBUG(dbgs() << CurrAttr->name << " = " << CurrAttr->value << "\n");
    CurrAttr = CurrAttr->next;
  }
  // end debug
}

FunctionType *MapIntrinToImlImpl::legalizeFunctionTypes(
    FunctionType *FT, SmallVectorImpl<Value *> &Args, unsigned TargetVL,
    StringRef FuncName) {
  // New type legalized argument types.
  SmallVector<Type *, 8> NewArgTypes;

  // Perform type legalization for the function declaration.
  for (unsigned I = 0; I < Args.size(); I++) {
    Type *ParmType = Args[I]->getType();

    // Adjust vector parameter types to a legal vector width. These will
    // be used to build the svml variant declaration.
    VectorType *VecType = dyn_cast<VectorType>(ParmType);
    if (VecType) {
      ParmType = VectorType::get(VecType->getElementType(), TargetVL);
    }
    NewArgTypes.push_back(ParmType);
  }

  Type *ReturnType = FT->getReturnType();
  VectorType *VectorReturn = dyn_cast<VectorType>(FT->getReturnType());

  // Adjust original vector return type to the legal vector width.
  // Insert the svml function declaration.
  if (VectorReturn) {
    bool hasSinCosName = FuncName.startswith("__svml_sincos");
    // sincos with a vector return type, created internally by this pass.
    // Double the vector length.
    unsigned TypeVL = hasSinCosName ? TargetVL * 2 : TargetVL;
    ReturnType = VectorType::get(VectorReturn->getElementType(), TypeVL);
  } else if (auto *StructReturn = dyn_cast<StructType>(FT->getReturnType())) {
    // Structure return, the preferred SVML sincos format.
    // { 4xfloat, 4xfloat } = __svml_sincosf4( 4xfloat )
    unsigned OrigVL = StructReturn->elements().front()->getVectorNumElements();

    // We shouldn't need to widen the call, as we rejected this earlier.
    assert(OrigVL >= TargetVL && "Widening SVML structure calls unsupported.");

    if (OrigVL > TargetVL) {
      // Narrow the return structure.
      // Create a new struct type with narrowed vector fields.
      SmallVector<Type *, 4> ReturnTyFields;
      for (auto *FieldType : StructReturn->elements()) {
        auto *FieldVType = cast<VectorType>(FieldType);
        auto *NarrowType =
            VectorType::get(FieldVType->getElementType(), TargetVL);
        ReturnTyFields.push_back(NarrowType);
      }
      ReturnType = StructType::create(ReturnTyFields, "svml.ret.agg");
    }
    // If we didn't need to narrow, use the original type, already assigned to
    // ReturnType.
  }

  FunctionType *LegalFT = FunctionType::get(ReturnType, NewArgTypes, false);
  return LegalFT;
}

void MapIntrinToImlImpl::generateMathLibCalls(
    unsigned NumRet, FunctionCallee Func,
    SmallVectorImpl<SmallVector<Value *, 8>> &Args,
    SmallVectorImpl<Instruction *> &Calls, Instruction **InsertPt) {

  // Insert the new shuffle instructions that split the original vector
  // parameter and generate the call to the svml function.
  for (unsigned I = 0; I < NumRet; I++) {
    for (unsigned J = 0; J < Args[I].size(); J++) {
      Instruction *ArgInst = dyn_cast<Instruction>(Args[I][J]);
      assert(ArgInst && "Expected arg to be associated with an instruction");
      ArgInst->insertAfter(*InsertPt);
      *InsertPt = ArgInst;
    }
    CallInst *NewCI = CallInst::Create(Func, Args[I], "vcall");
    NewCI->setCallingConv(CallingConv::SVML);
    NewCI->insertAfter(*InsertPt);
    Calls.push_back(NewCI);
    *InsertPt = NewCI;
  }
}

Instruction *
MapIntrinToImlImpl::combineCallResults(unsigned NumRet,
                                       SmallVectorImpl<Instruction *> &WorkList,
                                       Instruction **InsertPt) {

  // The initial number of elements for the first shuffle is NumElems. As we
  // go, this number is adjusted up for the increasing size of vectors used in
  // the shuffles. See notes in loop below.
  VectorType *CallType = dyn_cast<VectorType>(WorkList[0]->getType());
  assert(CallType && "Expected result of call to be vector");
  ShuffleVectorInst *CombinedShuffle;
  unsigned NumElems = CallType->getNumElements() * 2;
  unsigned NumRegs = NumRet;

  // This code works by combining pairs of instructions that are the results of
  // the svml calls. This is necessary for type legalization where NumRet > 1.
  // For example, for logical VL=16, target VL=4:
  //
  // First, %1 (original call arg) is split via splitArgs() call above
  // using shuffles. After the svml calls, the results are combined to form a
  // vector of the logical vector length.
  //
  // %1 = %0 <16 x float>
  //
  // %shuffle.1 = shufflevector <16 x float> %1, <4 x i32> <0, 1, 2, 3>
  // %call.1 = __svml_sinf4(%shuffle.1);
  //
  // %shuffle.2 = shufflevector <16 x float> %1, <4 x i32> <4, 5, 6, 7>
  // %call.2 = __svml_sinf4(%shuffle.2);
  //
  // %shuffle.3 = shufflevector <16 x float> %1, <4 x i32> <8, 9, 10, 11>
  // %call.3 = __svml_sinf4(%shuffle.3);
  //
  // %shuffle.4 = shufflevector <16 x float> %1, <4 x i32> <12, 13, 14, 15>
  // %call.4 = __svml_sinf4(%shuffle.4);
  //
  // %comb.1 = shufflevector <4 x float> %call.1, <4 x float> %call.2,
  //           <8 x i32> <0, 1, 2, 3, 4, 5, 6, 7>
  //
  // %comb.2 = shufflevector <4 x float> %call.3, <4 x float> %call.4,
  //           <8 x i32> <0, 1, 2, 3, 4, 5, 6, 7>
  //
  // %final = shufflevector <8 x float> %comb.1, <8 x float> %comb.2,
  //          <16 x i32> <0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15>
  //
  // %final then replaces the users of the original call. The remainder of
  // instructions are left to CodeGen to type legalize.

  while (NumRegs > 1) {

    for (unsigned I = 0; I < NumRegs; I += 2) {
      // NumRet is a power of two and we must apply shuffles in pairs. Thus, we
      // need to apply shuffles of increasing vector width. E.g., when
      // NumRet = 4, LogicalVL = 16, we need to combine 2 <4 x type> vectors,
      // and then combine 2 <8 x type> vectors. Create a mask the size of
      // NumElems that will reflect an in order sequence of elements from the
      // two input vectors, starting at 0.

      // Create the shuffle mask.
      SmallVector<Constant *, 8> Splat;
      for (unsigned J = 0; J < NumElems; J++) {
        Constant *ConstVal =
            ConstantInt::get(Type::getInt32Ty(Func->getContext()), J);
        Splat.push_back(ConstVal);
      }
      Value *Mask = ConstantVector::get(Splat);

      // Combine the results of the svml calls.
      CombinedShuffle = new ShuffleVectorInst(WorkList[I], WorkList[I + 1],
                                              Mask, "shuffle.comb");
      CombinedShuffle->insertAfter(*InsertPt);

      // Add the new result vector to the WorkList. Subsequent
      // iterations will combine them into a larger vector until we
      // get to the logical vector length.
      WorkList.push_back(CombinedShuffle);

      // Set the Instruction insertion point in the basic block.
      *InsertPt = CombinedShuffle;
    }

    // Remove the Instructions that have been combined from the
    // WorkList.
    SmallVector<Instruction *, 8>::iterator Start = WorkList.begin();
    SmallVector<Instruction *, 8>::iterator End = Start + NumRegs;
    WorkList.erase(Start, End);

    // The next iteration will require a vector 2x the size of this one.
    NumElems *= 2;

    // The number of result vectors to be combined will be reduced by
    // half.
    NumRegs /= 2;
  }

  return CombinedShuffle;
}

LoadStoreMode
MapIntrinToImlImpl::getLoadStoreModeForArg(AttributeList &AL, unsigned ArgNo,
                                           StringRef &AttrValStr) {

  AttributeSet ParamAttrs = AL.getParamAttributes(ArgNo);
  if (ParamAttrs.hasAttribute("stride")) {
    Attribute Attr = ParamAttrs.getAttribute("stride");
    AttrValStr = Attr.getValueAsString();
  } else {
    AttrValStr = "indirect";
  }

  if (AttrValStr == "indirect") {
    return INDIRECT;
  } else if (AttrValStr == "1") {
    return UNIT_STRIDE;
  } else {
    // No strided load/store support right now, so revert to scalar mode.
    // return NON_UNIT_STRIDE;
    return SCALAR;
  }
}

void MapIntrinToImlImpl::generateSinCosStore(
    CallInst *VectorCall, Instruction *ResultVector, unsigned NumElemsToStore,
    unsigned TargetVL, unsigned StorePtrIdx, Instruction **InsertPt) {

  // For __svml_sincos calls, the result vector is always 2x that of the input
  // vector.
  unsigned NumResultVectors = 2;

  AttributeList AttrList = VectorCall->getAttributes();
  // For Arg 2, I = 0. Attributes are at position 2
  // For Arg 3, I = 1. Attributes are at position 3

  for (unsigned I = 0; I < NumResultVectors; ++I) {

    // Name used to distinguish what is going into registers.
    // Iteration 0 is sin
    // Iteration 1 is cos
    StringRef ResultName = I == 0 ? "sin" : "cos";

    // This represents the address to where we will store the sin/cos results.
    // Either arg 2 or 3 from the vector sincos call.
    Value *SvmlArg = VectorCall->getArgOperand(I + 1);

    // Shuffle out the sin/cos results from the return vector of the svml call.
    // This is necessary since we have a 2 x VL wide return vector.
    SmallVector<Constant *, 8> Splat;
    unsigned StartElemIdx = I * TargetVL;
    unsigned ElemIdx = StartElemIdx;

    Instruction *ShuffleInst =
        extractElemsFromVector(ResultVector, ElemIdx, NumElemsToStore);
    ShuffleInst->insertAfter(*InsertPt);
    *InsertPt = ShuffleInst;

    VectorType *ShuffleType = cast<VectorType>(ShuffleInst->getType());
    unsigned NumElems = ShuffleType->getNumElements();

    StringRef StrideValStr;
    LoadStoreMode Mode = getLoadStoreModeForArg(AttrList, I + 2, StrideValStr);

    if (Mode == SCALAR) {
      // Store a single element at a time.
      unsigned PtrIdx = StorePtrIdx;
      for (unsigned Idx = 0; Idx < NumElems; ++Idx, ++PtrIdx) {
        Constant *ElmIdx =
            ConstantInt::get(Type::getInt32Ty(Func->getContext()), Idx);

        Constant *ElmIdxPtr =
            ConstantInt::get(Type::getInt32Ty(Func->getContext()), PtrIdx);

        // Extract the sin/cos result from the vector register.
        ExtractElementInst *ElemToStore =
            ExtractElementInst::Create(ShuffleInst, ElmIdx,
                                       ResultName + ".elem");
        ElemToStore->insertAfter(*InsertPt);
        *InsertPt = ElemToStore;

        // Extract the pointer from the vector of pointers argument from the
        // original call.
        ExtractElementInst *ElemToStorePtr =
            ExtractElementInst::Create(SvmlArg, ElmIdxPtr,
                                       ResultName + ".elem.ptr");
        ElemToStorePtr->insertAfter(*InsertPt);
        *InsertPt = ElemToStorePtr;

        // Store the sin/cos element to the pointer.
        StoreInst *Store = new StoreInst(ElemToStore, ElemToStorePtr);
        Store->insertAfter(*InsertPt);
        *InsertPt = Store;
      }
    } else if (Mode == UNIT_STRIDE) {
      // Generate unit stride store. Here, we just need to extract the 1st
      // address of the argument vector from the original call because this will
      // represent the base address for the vector store. If there are multiple
      // returns, the index into the pointer of vectors on the call arg needs
      // to advance by the number of elements being stored. E.g.,
      //
      // call void @llvm.sincos.v8f32.p0v8f32.p0v8f32(
      //   <8 x float> %wide.load,
      //   <8 x float*> "stride"="1" %32,
      //   <8 x float*> "stride"="1" %48)
      //
      // where NumRet = 2 and VL = 4, so for the first call the based pointers
      // are %32[0] and %48[0]. For the second call, the base pointers are
      // %32[4] and %48[4]. These addresses are then used to store the results
      // of the __svml_sincos calls.
      Constant *Index =
          ConstantInt::get(Type::getInt32Ty(Func->getContext()), StorePtrIdx);

      ExtractElementInst *StorePtr =
          ExtractElementInst::Create(SvmlArg, Index, ResultName + ".ptr");
      StorePtr->insertAfter(*InsertPt);

      // Cast the scalar pointer type to a vector one. The vector pointer is
      // what is used for the store.
      PointerType *PtrElemType = dyn_cast<PointerType>(StorePtr->getType());
      assert(PtrElemType &&
             "Expected element type of svml arg to be a pointer");
      *InsertPt = StorePtr;

      PointerType *VecPtrType = PointerType::get(
          ShuffleInst->getType(), PtrElemType->getAddressSpace());
      BitCastInst *BitCast =
          new BitCastInst(StorePtr, VecPtrType, ResultName + ".ptr.cast");
      BitCast->insertAfter(*InsertPt);
      *InsertPt = BitCast;

      // Store the vector of sin/cos elements to the vector pointer.
      StoreInst *Store = new StoreInst(ShuffleInst, BitCast);
      Store->insertAfter(*InsertPt);
      *InsertPt = Store;
    }
  }
}

bool MapIntrinToImlImpl::isLessThanFullVector(Type *ValType, Type *LegalType) {

  VectorType *ValVecType = dyn_cast<VectorType>(ValType);
  VectorType *LegalVecType = dyn_cast<VectorType>(LegalType);

  if (ValVecType && LegalVecType &&
      ValVecType->getBitWidth() < LegalVecType->getBitWidth()) {
    return true;
  }

  return false;
}

void MapIntrinToImlImpl::generateNewArgsFromPartialVectors(
    ArrayRef<Value *> Args, ArrayRef<Type *> NewArgTypes, unsigned TargetVL,
    SmallVectorImpl<Value *> &NewArgs, Instruction *InsertPt) {

  // This function builds a new argument list for the svml function call by
  // finding any arguments that are less than full vector and duplicating the
  // low order elements into the upper part of a new vector register. If the
  // argument type is already legal, then just insert it as is in the arg list.

  for (unsigned I = 0; I < NewArgTypes.size(); ++I) {

    // Type of the parameter on the math lib call, which can be driven by the
    // user specifying an explicit vectorlength.
    Value *NewArg = Args[I];
    VectorType *VecArgType = cast<VectorType>(NewArg->getType());

    // The type of the parameter if using the full register specified through
    // legalization.
    VectorType *LegalVecArgType = cast<VectorType>(NewArgTypes[I]);

    unsigned NumElems = VecArgType->getNumElements();
    unsigned LegalNumElems = LegalVecArgType->getNumElements();

    bool LessThanFullVector = isLessThanFullVector(VecArgType, LegalVecArgType);

    // Check to see if we have a partial register.
    if (LessThanFullVector) {

      SmallVector<Constant *, 4> Splat;

      // Build a mask vector that repeats the number of elements difference
      // between the partial vector and the legal full vector. e.g., partial
      // vl=2, legal vl=4, build a mask vector of <0, 1, 0, 1>. The svml call
      // will operate on all 4 elements, but only the lower two will be used.

      for (unsigned J = 0; J < LegalNumElems / NumElems; ++J) {
        for (unsigned K = 0; K < NumElems; ++K) {
          Constant *ConstVal =
              ConstantInt::get(Type::getInt32Ty(Func->getContext()), K);
          Splat.push_back(ConstVal);
        }
      }

      // Insert the shuffle that duplicates the elements and use this
      // instruction as the new svml call argument.
      Value *Undef = UndefValue::get(VecArgType);
      Value *Mask = ConstantVector::get(Splat);
      ShuffleVectorInst *ShuffleInst =
          new ShuffleVectorInst(NewArg, Undef, Mask, "shuffle.dup");
      ShuffleInst->insertBefore(InsertPt);
      NewArg = ShuffleInst;
    }

    NewArgs.push_back(NewArg);
  }
}

Instruction *MapIntrinToImlImpl::extractElemsFromVector(Value *Reg,
                                                        unsigned StartPos,
                                                        unsigned NumElems) {
  Type *RegType = Reg->getType();
  assert(RegType->isVectorTy() && "Expected vector register type for extract");

  VectorType *VecRegType = cast<VectorType>(RegType);

  SmallVector<Constant *, 4> Splat;

  for (unsigned I = StartPos; I < NumElems + StartPos; ++I) {
    Constant *ConstVal =
        ConstantInt::get(Type::getInt32Ty(Func->getContext()), I);
    Splat.push_back(ConstVal);
  }

  Value *Undef = UndefValue::get(VecRegType);
  Value *Mask = ConstantVector::get(Splat);

  ShuffleVectorInst *ShuffleResult =
      new ShuffleVectorInst(Reg, Undef, Mask, "shuffle.part");
  return ShuffleResult;
}

// Returns the return vector type in the function signature. NumRet is based
// on this type. Most math functions' argument types match the return type and
// these are the simple cases. However, this function should be flexible enough
// to deal with odd cases like the following (assume xmm target for explanatory
// purposes).
//
// Case 1) int ilogb(double) -> <4 x i32> ilogb(<4 x double>)
// - argument must be split into two <2 x double> vectors
// - return is reduced to <2 x i32>
// - this function should return <4 x double>, so that we know NumRet = 2
//   (i.e., generate two svml calls)
// - this is ok because the function should return only two i32 values in the
//   lower half of the xmm register.
//
// TODO: support for Case 1 still needs to be added. Calls to ilogb (and any
//       other functions with parameter/return types that will result in
//       different vector type widths) will not be vectorized at the moment, so
//       stability is not an issue.
//
// Case 2) double drand48() -> <4 x double> drand(void)
// - return must be split into two <2 x double> vectors
// - this function returns <4 x double>, so NumRet = 2
//
// Case 3) void sincos(double, double*, double*) ->
//         void sincos(<4 x double>, <4 x double*>, <4 x double*>)
// - return is void, so the largest argument is <4 x double> (keep in mind
//   that double* is 32-bit or 64-bit depending on target pointer size).
// - all arguments are reduced to 2 elements
// - NumRet = 2
// - this is ok because __svml_sincos expects two pointers for each sin/cos
//   result.
//
// For cases 1 and 3, we unfortunately need special logic to determine the
// correct call type information. sincos is void, but the incoming vector call
// will be transformed to an svml call that returns a 2x wide vector based on
// the type of the 1st argument of the call signature. For ilogb, the number
// of elements returned is the number of elements indicated by the 1st argument
// since it is 2x the size of the return. Case 2 will work as is.

VectorType *MapIntrinToImlImpl::getCallType(CallInst *CI) {

  Function *CalledFunc = CI->getCalledFunction();
  StringRef FuncName = CalledFunc->getName();
  if (isSincosRefArg(FuncName, CI->getFunctionType())) {
    return dyn_cast<VectorType>(CI->getArgOperand(0)->getType());
  }

  FunctionType *CallFT = CI->getFunctionType();
  Type *CallRetType = CallFT->getReturnType();
  auto *RetStructTy = dyn_cast_or_null<StructType>(CallRetType);
  // For structure return types, return the first structure element as
  // a vector type.
  if (RetStructTy && RetStructTy->getNumElements())
    return dyn_cast<VectorType>(RetStructTy->getElementType(0));
  else
    return dyn_cast<VectorType>(CallRetType);
}

void MapIntrinToImlImpl::scalarizeVectorCall(CallInst *CI,
                                             StringRef LibFuncName,
                                             unsigned LogicalVL,
                                             Type *ElemType) {
  SmallVector<Type *, 4> FTArgTypes;

  FunctionType *IntrinFT = CI->getFunctionType();
  Type *RetType = IntrinFT->getReturnType();
  if (VectorType *VecRetType = dyn_cast<VectorType>(RetType)) {
    RetType = VecRetType->getElementType();
  }

  for (auto *ParmType : IntrinFT->params()) {
    if (VectorType *VecParmType = dyn_cast<VectorType>(ParmType)) {
      Type *ElemType = VecParmType->getElementType();
      FTArgTypes.push_back(ElemType);
    } else {
      FTArgTypes.push_back(ParmType);
    }
  }

  StringRef ScalarLibFuncName = getScalarFunctionName(LibFuncName, LogicalVL);

  LLVM_DEBUG(dbgs() << "Scalarizing call to '" << LibFuncName << "' with '"
                    << ScalarLibFuncName << "'");

  FunctionType *FT = FunctionType::get(RetType, FTArgTypes, false);
  FunctionCallee FCache = M->getOrInsertFunction(ScalarLibFuncName, FT);
  SmallVector<Value *, 4> CallResults;

  for (unsigned I = 0; I < LogicalVL; ++I) {
    SmallVector<Value *, 4> ScalarCallArgs;

    for (unsigned J = 0; J < CI->getNumArgOperands(); ++J) {
      Value *Parm = CI->getArgOperand(J);
      VectorType *VecParmType = dyn_cast<VectorType>(Parm->getType());

      if (VecParmType) {
        Constant *Index =
            ConstantInt::get(Type::getInt32Ty(Func->getContext()), I);

        ExtractElementInst *Extract =
            ExtractElementInst::Create(CI->getOperand(J), Index, "arg", CI);

        Parm = Extract;
      }

      ScalarCallArgs.push_back(Parm);
    }

    CallInst *ScalarCall = CallInst::Create(FCache, ScalarCallArgs);
    ScalarCall->insertBefore(CI);
    CallResults.push_back(ScalarCall);
  }

  if (!RetType->isVoidTy()) {
    // If this is not a void function call then there will be users of the call
    // results. Insert each scalar call instruction (this corresponds to an
    // SSA temp) into a vector and replace the original vector call's users.

    Value *InsertVector =
        UndefValue::get(CI->getFunctionType()->getReturnType());

    for (unsigned I = 0; I < CallResults.size(); ++I) {
      Constant *Index =
          ConstantInt::get(Type::getInt32Ty(Func->getContext()), I);
      InsertVector = InsertElementInst::Create(InsertVector, CallResults[I],
                                               Index, "ins", CI);
    }

    // Find the instructions that are using the call results and replace with
    // the vector that has all of the scalar call results.
    CI->replaceAllUsesWith(InsertVector);
  }
}

// The vectorizer may generate an intermediate-form SVML sincos with
// a void return type and reference args (number depending on mask, etc.):
//  call void @__svml_sincosf4( <4 x float>, <4 x float*>, <4 x float*>)
// This pass must convert to the actual SVML call with a wide-vector return.
bool MapIntrinToImlImpl::isSincosRefArg(StringRef FuncName, FunctionType *FT) {
  if (!FuncName.startswith("__svml_sincos"))
    return false;
  return FT->getReturnType()->isVoidTy() && (FT->getNumParams() > 1);
}

const char *MapIntrinToImlImpl::findX86Variant(CallInst *CI, StringRef FuncName,
                                               unsigned LogicalVL,
                                               unsigned TargetVL) {

  StringRef DataType;
  std::string TargetVLString = std::to_string(TargetVL);

  // Use the generic parent function name emitted by the vectorizer to lookup
  // the proper variant. E.g., "__svml_sinf4" -> "__svml_sinf4_ha",
  // "__svml_sinf4" -> "__svml_sinf4_ep". The selection process uses the IMF
  // attributes to find the appropriate variant. The function name used for
  // the lookup is based on the legal target vector length. This is important
  // to remember since the input function (FuncName) could be a logical vector
  // that is larger.
  StringRef ScalarFuncName = getScalarFunctionName(FuncName, LogicalVL);
  std::string TargetVLStr = APInt(32, TargetVL).toString(10, false);
  std::string TempFuncName = "__svml_" + ScalarFuncName.str() + TargetVLStr;
  if (FuncName.find("mask") != StringRef::npos) {
    TempFuncName += "_mask";
  }
  char *ParentFuncName = new char[TempFuncName.size() + 1];
  std::strcpy(ParentFuncName, TempFuncName.c_str());

  LLVM_DEBUG(dbgs() << "Input Function: " << FuncName << "\n");
  LLVM_DEBUG(dbgs() << "Legal Function: " << TempFuncName << "\n");

  ImfAttr *AttrList = nullptr;
  createImfAttributeList(CI, &AttrList);

  // External libiml_attr interface that returns the SVML/libm variant if the
  // parent function and IMF attributes match. Return NULL otherwise.
  const char *VariantFuncName =
    get_library_function_name(ParentFuncName, AttrList);

  // No longer need the IMF attribute list at this point, so free up the memory.
  // Note: this does not remove the attributes from the instruction, only the
  // internal data structure used to query the iml interface.
  deleteAttributeList(&AttrList);
  delete ParentFuncName;

  return VariantFuncName;
}

StringRef MapIntrinToImlImpl::getScalarFunctionName(StringRef FuncName,
                                                    unsigned LogicalVL) {

  // Incoming FuncName is something like '__svml_sinf4'. Removing the '__svml_'
  // prefix and logical vl from the end yields the scalar lib name.
  StringRef Prefix = "__svml_";
  StringRef LogicalVLStr = APInt(32, LogicalVL).toString(10, false);
  StringRef TempName = FuncName.rtrim("_mask");
  StringRef ScalarFuncName = TempName.substr(Prefix.size());
  ScalarFuncName = ScalarFuncName.drop_back(LogicalVLStr.size());

  return ScalarFuncName;
}

// For a given Instruction with struct type, find an extractvalue
// of that Instruction with the given index. It is an error if
// there is no extractvalue found with that index.
static Instruction *findExtract(Instruction *I, unsigned Idx) {
#ifndef NDEBUG
  auto *StrTy = dyn_cast<StructType>(I->getType());
  assert(StrTy && StrTy->getNumElements() > Idx);
#endif // NDEBUG
  for (User *U : I->users())
    if (auto *Extract = dyn_cast<ExtractValueInst>(U))
      if (Extract->getIndices().front() == Idx)
        return Extract;
  llvm_unreachable("Can't find extract matching index");
  return nullptr;
}

/// Build function name for SVML integer div/rem from opcode, scalar type and
/// vector length information. Requires input to be legal.
static std::string getSVMLIDivOrRemFuncName(Instruction::BinaryOps Opcode,
                                            VectorType *Ty) {
  unsigned ScalarSize = Ty->getScalarSizeInBits();

  assert((ScalarSize == 8 || ScalarSize == 16 || ScalarSize == 32 ||
          ScalarSize == 64) &&
         "integer must be 8/16/32/64 wide");
  assert((Opcode == Instruction::UDiv || Opcode == Instruction::URem ||
          Opcode == Instruction::SDiv || Opcode == Instruction::SRem) &&
         "Opcode must be udiv/urem/sdiv/srem");

  std::string Result = "__svml_";

  Result +=
      (Opcode == Instruction::UDiv || Opcode == Instruction::URem) ? 'u' : 'i';

  if (ScalarSize != 32)
    Result += std::to_string(ScalarSize);

  Result += (Opcode == Instruction::UDiv || Opcode == Instruction::SDiv)
                ? "div"
                : "rem";

  Result += std::to_string(Ty->getNumElements());
  return Result;
}

CallInst *
MapIntrinToImlImpl::generateSVMLIDivOrRemCall(Instruction::BinaryOps Opcode,
                                          Value *V0, Value *V1) {
  Type *Ty = V0->getType();

  assert(Ty->isVectorTy() && Ty->getVectorElementType()->isIntegerTy() &&
         "SVML integer div/rem only works for integer vectors");
  assert(V0->getType() == V1->getType() &&
         "operands of div/rem must have the same type");

  std::string FuncName = getSVMLIDivOrRemFuncName(Opcode, cast<VectorType>(Ty));
  FunctionCallee F = M->getOrInsertFunction(FuncName, Ty, Ty, Ty);
  CallInst *CI = CallInst::Create(F, {V0, V1});
  CI->setCallingConv(CallingConv::SVML);

  return CI;
}

bool MapIntrinToImlImpl::replaceVectorIDivAndRemWithSVMLCall(
    TargetTransformInfo *TTI, Function &F) {
  bool Dirty = false; // LLVM IR not yet modified
  // Will be populated with the integer div/rem instructions that will be
  // replaced with legalized/refined svml calls.
  SmallVector<Instruction *, 4> InstToRemove;

  for (auto &I : instructions(F)) {
    auto Opcode = I.getOpcode();
    // Only int8/16/32/64 vector udiv/sdiv/urem/srem instructions are converted
    // to SVML call
    if (Opcode == Instruction::UDiv || Opcode == Instruction::SDiv ||
        Opcode == Instruction::URem || Opcode == Instruction::SRem) {
      VectorType *VecTy = dyn_cast<VectorType>(I.getType());
      unsigned ScalarBitWidth = I.getType()->getScalarSizeInBits();
      if (!(VecTy && (ScalarBitWidth == 8 || ScalarBitWidth == 16 ||
                      ScalarBitWidth == 32 || ScalarBitWidth == 64)))
        continue;

      unsigned LogicalVL = VecTy->getNumElements();
      // Currently we cannot handle non-power-of-2 vectors
      if (!isPowerOf2_32(LogicalVL) || LogicalVL <= 2)
        continue;

      BinaryOperator *const BinOp = cast<BinaryOperator>(&I);
      if (isa<Constant>(BinOp->getOperand(1)))
        continue;

      // Get the number of library calls that will be required, indicated by
      // NumRet.
      unsigned TargetVL = 0;
      unsigned NumRet =
          calculateNumReturns(TTI, ScalarBitWidth, LogicalVL, &TargetVL);

      Instruction *Result = BinOp;
      SmallVector<Value *, 2> Args(BinOp->operands());

      if (NumRet > 1) {
        // NumRet > 1 means that multiple library calls are required to
        // support the vector length of the integer division instruction.

        // NewArgs contains the type legalized operands that are split in
        // splitArgs(). These are the results of shuffle instructions.
        SmallVector<SmallVector<Value *, 8>, 8> NewArgs;
        splitArgs(Args, NewArgs, NumRet, TargetVL);

        // Generate SVML call for each part of the split operands
        SmallVector<Instruction *, 8> WorkList;
        for (unsigned I = 0; I < NumRet; I++) {
          for (unsigned J = 0; J < NewArgs[I].size(); J++) {
            Instruction *ArgInst = dyn_cast<Instruction>(NewArgs[I][J]);
            assert(ArgInst &&
                   "Expected arg to be associated with an instruction");
            ArgInst->insertBefore(BinOp);
          }
          CallInst *NewInst = generateSVMLIDivOrRemCall(
              BinOp->getOpcode(), NewArgs[I][0], NewArgs[I][1]);
          NewInst->insertBefore(BinOp);
          WorkList.push_back(NewInst);
        }

        Instruction *InsertPt = BinOp;
        Result = combineCallResults(NumRet, WorkList, &InsertPt);
      } else {
        VectorType *LegalVecType =
            VectorType::get(VecTy->getScalarType(), TargetVL);
        // Type legalization needs to happen for NumRet = 1 when dealing with
        // less than full vector cases.
        if (isLessThanFullVector(VecTy, LegalVecType)) {
          // generateNewArgsFromPartialVectors() duplicates the low elements
          // into the upper part of the vector so the math library call operates
          // on safe values. But, the duplicate results are not needed, so
          // shuffle out the ones we want and then replace the users of the
          // function call with the shuffle. This work is done via
          // extractElemsFromVector().
          SmallVector<Type *, 2> NewArgTypes{LegalVecType, LegalVecType};
          SmallVector<Value *, 2> NewArgs;
          generateNewArgsFromPartialVectors(Args, NewArgTypes, TargetVL,
                                            NewArgs, BinOp);
          Result = generateSVMLIDivOrRemCall(BinOp->getOpcode(), NewArgs[0],
                                             NewArgs[1]);
          Result->insertBefore(BinOp);
          // Extract the number of elements specified by the partial vector
          // return type of the call (indicated by LogicalVL). The type of
          // Result will indicate the size of the vector extracted from.
          // Start extracting from position 0.
          Result = extractElemsFromVector(Result, 0, LogicalVL);
        } else {
          // The disvision is operating on full vector, so just create a call
          // directly from its operands
          Result =
              generateSVMLIDivOrRemCall(BinOp->getOpcode(), Args[0], Args[1]);
        }
        Result->insertBefore(BinOp);
      }

      BinOp->replaceAllUsesWith(Result);
      InstToRemove.push_back(BinOp);
      Dirty = true;
    }
  }

  // Remove the old integer divisions since they have been replaced with the
  // legalized library calls.
  for (auto *Inst : InstToRemove)
    Inst->eraseFromParent();

  return Dirty;
}

bool MapIntrinToIml::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;
  auto *TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  return Impl.runImpl(F, TTI);
}

PreservedAnalyses MapIntrinToImlPass::run(Function &F,
                                          FunctionAnalysisManager &AM) {
  auto *TTI = &AM.getResult<TargetIRAnalysis>(F);
  if (!Impl.runImpl(F, TTI))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool MapIntrinToImlImpl::runImpl(Function &F, TargetTransformInfo *TTI) {
  LLVM_DEBUG(dbgs() << "\nExecuting MapIntrinToIml ...\n\n");
  if (RunSvmlStressMode) {
    LLVM_DEBUG(dbgs() << "Stress Testing Mode Invoked - svml calls will be "
                         "scalarized\n");
  }

  Func = &F;
  M = F.getParent();

  const DataLayout DL = M->getDataLayout();

  Triple *T = new Triple(M->getTargetTriple());
  bool X86Target = (T->getArch() == Triple::x86 ||
                    T->getArch() == Triple::x86_64);
  delete T;

  // Will be populated with the call instructions that will be replaced with
  // legalized/refined svml calls.
  SmallVector<Instruction *, 4> InstToRemove;

  // Keep track of calls that have already been translated so we don't consider
  // them again.
  SmallPtrSet<CallInst*, 4> InstToTranslate;

  // Keep track of scalar math function calls to translate
  SmallPtrSet<CallInst *, 4> ScalarCallsToTranslate;

  // Dirty becomes true (LLVM IR is modified) under two circumstances:
  // 1) A candidate vector math call is found and is replaced with an svml
  //    variant.
  // 2) A candidate vector math call is found, is not replaced by an svml
  //    variant, but is scalarized.
  bool Dirty = false; // LLVM IR not yet modified

  // First convert some of vector integer divisions to SVML calls (otherwise
  // they'll likely be serialized and it hurts performance)
  // OpenCL CPU RT uses an alternative version of SVML and is incompatible
  // with the interface we're assuming, so vector idiv transformation is
  // disabled when generating code for OpenCL.
  // FIXME: Currently only SVML is supported for vector idiv transformation.
  // It should only be done when using SVML.
  bool isOCL = M->getNamedMetadata("opencl.ocl.version") != nullptr;
  if (!isOCL)
    Dirty |= replaceVectorIDivAndRemWithSVMLCall(TTI, F);

  // Begin searching for calls that are candidates for legalization and/or
  // refinement based on IMF attributes attached to the call.
  for (inst_iterator Inst = inst_begin(F), InstEnd = inst_end(F);
       Inst != InstEnd; ++Inst) {

    // Find incoming svml calls and insert them into a map. These are candidates
    // for legalization and application of IMF attributes. This check can later
    // be extended to include scalar (libm) candidates.
    CallInst *CI = dyn_cast<CallInst>(&*Inst);
    // Handle only those svml calls that have explicitly defined vector types as
    // returns/arguments. E.g., cases where the user writes intrinsics directly
    // have been known to use pointers to structs of __m128 types. These are not
    // supported as of now.
    if (CI && CI->getCalledFunction() && getCallType(CI)) {
      StringRef FuncName = CI->getCalledFunction()->getName();
      if (FuncName.startswith("__svml"))
        InstToTranslate.insert(CI);
    } else if (CI && CI->getCalledFunction()) {
      std::string FuncName = CI->getCalledFunction()->getName().str();
      if (is_libm_function(FuncName.c_str()))
        ScalarCallsToTranslate.insert(CI);
    }
  }

  SmallPtrSet<CallInst*, 4>::iterator CallInstIt = InstToTranslate.begin();
  SmallPtrSet<CallInst*, 4>::iterator CallInstEnd = InstToTranslate.end();
  for (; CallInstIt != CallInstEnd; ++CallInstIt) {

    CallInst *CI = cast<CallInst>(*CallInstIt);
    LLVM_DEBUG(dbgs() << "Call Inst: " << *CI << "\n");

    StringRef FuncName = CI->getCalledFunction()->getName();
    unsigned ScalarBitWidth = 0;

    // The function call is only inserted into the candidates map
    // (InstToTranslate) when it is known the return/arguments are explicitly
    // vector typed. Thus, here it is known that getCallType(CI) will return
    // a vector type.
    VectorType *VecCallType = getCallType(CI);

    unsigned LogicalVL = VecCallType->getNumElements();
    Type *ElemType = VecCallType->getElementType();

    // Need to go through DataLayout in cases where we have pointer types to
    // get the correct pointer bit size.
    ScalarBitWidth = DL.getTypeSizeInBits(ElemType);

    // Get the number of library calls that will be required, indicated by
    // NumRet. NumRet is determined by getting the vector type info from the
    // call signature. See notes in getCallType() for more information.
    unsigned TargetVL = 0;
    unsigned NumRet =
        calculateNumReturns(TTI, ScalarBitWidth, LogicalVL, &TargetVL);

    if (auto *StructRetTy = dyn_cast<StructType>(CI->getType())) {
      // We don't support widening of struct-return SVML calls. Reduce the
      // target VL to the original call VL.
      if (StructRetTy->getNumElements()) {
        unsigned OrigVL =
            StructRetTy->elements().front()->getVectorNumElements();
        if (TargetVL > OrigVL)
          TargetVL = OrigVL;
      }
    }

    const char *VariantFuncName = nullptr;

    if (X86Target) {
      VariantFuncName = findX86Variant(CI, FuncName, LogicalVL, TargetVL);
    }

    // An alternate math library function was found for the original call, so
    // replace the original call with the new call. Set Dirty to true since
    // the LLVM IR is modified. Scalarize the call when:
    // 1) an appropriate math library function is not found through querying
    //    the function selection interface.
    // 2) the pass is running in stress testing mode.
    if (VariantFuncName && !RunSvmlStressMode) {
      StringRef VariantFuncNameRef = StringRef(VariantFuncName);
      LLVM_DEBUG(dbgs() << "Function Variant: " << VariantFuncNameRef
                        << "\n\n");

      // Original arguments to the vector call.
      SmallVector<Value *, 8> Args;

      FunctionType *FT;

      if (X86Target && isSincosRefArg(FuncName, CI->getFunctionType())) {
        // We have to do some special handling for sincos calls on X86
        // because 'void sincos(<vl x type>, <vl x type*>, <vl x type*>)'
        // must be transformed to:
        // '<vl x 2 x type> __svml_sincos(<vl x type>)'
        Args.push_back(CI->getArgOperand(0));
        VectorType *RetType =
            VectorType::getDoubleElementsVectorType(VecCallType);
        FT = FunctionType::get(RetType, VecCallType, false);
      } else {
        unsigned NumArgs = CI->getNumArgOperands();
        for (unsigned I = 0; I < NumArgs; I++) {
          Value *Arg = CI->getArgOperand(I);
          Args.push_back(Arg);
        }
        FT = CI->getFunctionType();
      }

      // FT will point to the legal FunctionType based on target register
      // size requirements. This FunctionType is used to create the call
      // to the svml function.
      FT = legalizeFunctionTypes(FT, Args, TargetVL, FuncName);
      FunctionCallee FCache = M->getOrInsertFunction(VariantFuncNameRef, FT);
      Instruction *InsertPt = CI;

      // WorkList contains the instructions that are the results of math
      // library calls. This could be the call instructions themselves, or
      // the results of shuffles for cases like calls to sincos, or calls to
      // functions that are less than full vector.
      SmallVector<Instruction *, 8> WorkList;

      if (NumRet > 1) {

        // NumRet > 1 means that multiple library calls are required to
        // support the vector length of the call.

        // NewArgs contains the type legalized parameters that are split in
        // splitArgs(). These are the results of shuffle instructions.
        SmallVector<SmallVector<Value *, 8>, 8> NewArgs;
        splitArgs(Args, NewArgs, NumRet, TargetVL);

        generateMathLibCalls(NumRet, FCache, NewArgs, WorkList, &InsertPt);

        if (auto *StrType = dyn_cast<StructType>(CI->getType())) {
          // The original call was:
          // %struct_2el = svml_big(); // 2-element struct
          // %a = extractvalue %struct_2el , 0
          // %b = extractvalue %struct_2el , 1
          //
          // After splitting, the worklist is:
          // { a0, b0 } = svml0();
          // { a1, b1 } = svml1();
          // { a2, b2 } = svml2();
          // { a3, b3 } = svml3();
          // Generate extracts of each field. Then generate shufflevector
          // combines, to combine the fields like this:
          // [a0,a1,a2,a3]
          //  => combine back to %a
          // then
          // [b0,b1,b2,b3]
          //  => combine back to %b
          // Replace the original %a and %b extractvalues with the
          // combined results.
          unsigned numFields = StrType->getNumElements();
          // For each field position ("a", "b", etc.)
          for (unsigned FieldIdx = 0; FieldIdx < numFields; ++FieldIdx) {
            // Extract this field from each of the calls in the worklist, and
            // combine these fields.
            SmallVector<Instruction *, 8> WorkListStripe;
            for (auto *NewCall : WorkList) {
#ifndef NDEBUG
              auto *NewCallTy = dyn_cast<StructType>(NewCall->getType());
              assert(NewCallTy && NewCallTy->getNumElements() == numFields);
#endif // NDEBUG
              auto *Extract = ExtractValueInst::Create(NewCall, {FieldIdx});
              Extract->insertAfter(NewCall);
              WorkListStripe.push_back(Extract);
            }
            // WorkListStripe contains [a0,a1,a2,a3] (for example). Call
            // combineCallResults to generate shufflevectors which combine
            // these vectors into a single big vector.
            Instruction *StripeInsertPt = WorkListStripe.back();
            Instruction *CombineResult =
                combineCallResults(NumRet, WorkListStripe, &StripeInsertPt);

            // Replace the original extract with the combined result.
            Instruction *OrigExtract = findExtract(CI, FieldIdx);
            OrigExtract->replaceAllUsesWith(CombineResult);
            OrigExtract->eraseFromParent();
            WorkListStripe.clear();
            // Process the next set of fields b0,b1,etc.
          }
          // Original CI gets removed later
        } else if (!isSincosRefArg(FuncName, CI->getFunctionType())) {
          // There will be no users of sincos because it's a void function.
          // Because of this, we have to generate the store instructions
          // explicitly. See generateSinCosStore().
          Instruction *FinalResult =
              combineCallResults(NumRet, WorkList, &InsertPt);
          CI->replaceAllUsesWith(FinalResult);
        }
      } else {
        // NumRet = 1

        // Type legalization still needs to happen for NumRet = 1 when
        // dealing with less than full vector cases.
        // generateNewArgsFromPartialVectors() duplicates the low elements
        // into the upper part of the vector so the math library call operates
        // on safe values. But, the duplicate results are not needed, so
        // shuffle out the ones we want and then replace the users of the
        // function call with the shuffle. This work is done via
        // extractElemsFromVector().
        //
        // Assuming target register width of 128-bit and user specifying
        // explicit vl=2:
        //
        // Before Transformation:
        //
        // %7 = sitofp <2 x i32> %induction3.1 to <2 x float>
        // %8 = call <2 x float> @__svml_cosf2(<2 x float> %7)
        //
        // %9 = getelementptr inbounds [128 x float], [128 x float]* %array,
        //                             i64 0, i64 %index.next
        //
        // %10 = bitcast float* %9 to <2 x float>*
        // store <2 x float> %8, <2 x float>* %10, align 8
        //
        // After Transformation:
        //
        // %1 = sitofp <2 x i32> %induction27 to <2 x float>
        //
        // duplicate low order elements via
        // generateNewArgsFromPartialVectors()
        // %dup = shufflevector <2 x float> %1, <2 x float> undef,
        //                      <4 x i32> <i32 0, i32 1, i32 0, i32 1>
        //
        // Example call here is to X86 svml.
        // %vcall = call <4 x float> @__svml_cosf4_ha(<4 x float> %dup)
        //
        // shuffle out the results that are needed via
        // extractElemsFromVector()
        // %part = shufflevector <4 x float> %vcall, <4 x float> undef,
        //                       <2 x i32> <i32 0, i32 1>
        //
        // %2 = getelementptr inbounds float, float* %array, i64 %index
        // %3 = bitcast float* %2 to <2 x float>*
        // store <2 x float> %part, <2 x float>* %3, align 4
        SmallVector<Value *, 8> NewArgs;
        SmallVector<Value *, 8> Args(CI->args());
        generateNewArgsFromPartialVectors(Args, FT->params(), TargetVL, NewArgs,
                                          InsertPt);

        CallInst *NewCI = CallInst::Create(FCache, NewArgs, "vcall");
        NewCI->setCallingConv(CallingConv::SVML);
        Instruction *CallResult = NewCI;
        NewCI->insertBefore(InsertPt);

        if (!isSincosRefArg(FuncName, CI->getFunctionType())) {
          bool LessThanFullVector = isLessThanFullVector(
              CI->getFunctionType()->getReturnType(), FT->getReturnType());

          if (LessThanFullVector) {
            // sincos requires a special extraction because it has a double
            // wide return register. This is dealt with in
            // generateSinCosStore().
            //
            // Extract the number of elements specified by the partial vector
            // return type of the call (indicated by LogicalVL). The NewCI
            // return type will indicate the size of the vector extracted from.
            // Start extracting from position 0.
            assert(!NewCI->getType()->isStructTy());
            CallResult = extractElemsFromVector(NewCI, 0, LogicalVL);
            CallResult->insertBefore(InsertPt);
          }

          CI->replaceAllUsesWith(CallResult);
        } else {
          WorkList.push_back(NewCI);
        }
      }

      if (X86Target && isSincosRefArg(FuncName, CI->getFunctionType())) {
        unsigned StorePtrIdx = 0;
        // For partial register cases, we only want to extract the partial
        // number of elements from the results. Otherwise, extract the full
        // legal register width number of elements.
        unsigned NumElemsToStore =
            LogicalVL < TargetVL ? LogicalVL : TargetVL;
        for (unsigned I = 0; I < NumRet; ++I) {
          InsertPt = WorkList[I];
          generateSinCosStore(CI, WorkList[I], NumElemsToStore, TargetVL,
                              StorePtrIdx, &InsertPt);
          StorePtrIdx += NumElemsToStore;
        }
      }

      InstToRemove.push_back(CI);
      Dirty = true; // LLVM-IR has been changed because we replaced the
                    // original call with a legalized/refined one.
    } else {
      // TODO: If for some reason the call translation failed, it is possible
      // that the lib call left in the LLVM IR is not legal and will not be
      // found when linking against the math library. To prevent a link error,
      // scalarize the vector math call. Also scalarize when running in stress
      // test mode. Since the LLVM IR is modified, set Dirty to true.

      scalarizeVectorCall(CI, FuncName, LogicalVL, ElemType);
      InstToRemove.push_back(CI);
      Dirty = true; // LLVM-IR has been changed because the original vector call
                    // was replaced with scalar calls.

      LLVM_DEBUG(dbgs() << "\n\n");
    }
  }

  // Legalize scalar math function calls
  for (auto ScalarCallsIt = ScalarCallsToTranslate.begin();
       ScalarCallsIt != ScalarCallsToTranslate.end(); ++ScalarCallsIt) {
    CallInst *ScalarCI = cast<CallInst>(*ScalarCallsIt);
    LLVM_DEBUG(dbgs() << "ScalarCI: "; ScalarCI->dump());
    if (X86Target) {
      // TODO: Can scalar math functions have any prefixes/suffixes?
      std::string ScalarFuncName =
          ScalarCI->getCalledFunction()->getName().str();

      ImfAttr *AttrList = nullptr;
      createImfAttributeList(ScalarCI, &AttrList);

      StringRef VariantFuncName = ScalarFuncName;

      // If iml accuracy interface returns a non-null variant then replace
      // current scalar function with variant function name.
      if (const char *VariantFuncStr =
              get_library_function_name(ScalarFuncName.c_str(), AttrList)) {
        VariantFuncName = StringRef(VariantFuncStr);
      }

      deleteAttributeList(&AttrList);

      // Don't perform replacement if variant is same as scalar function
      if (VariantFuncName.equals(ScalarFuncName))
        continue;

      LLVM_DEBUG(dbgs() << "Input Scalar Math Function: " << ScalarFuncName
                        << "\n");
      LLVM_DEBUG(dbgs() << "Legalized Scalar Math Function: " << VariantFuncName
                        << "\n");

      FunctionCallee FCache =
          M->getOrInsertFunction(VariantFuncName, ScalarCI->getFunctionType());
      ScalarCI->setCalledFunction(FCache);
      Dirty = true; // LLVM-IR is changed since function call is updated
    }
    LLVM_DEBUG(dbgs() << "ScalarCI after legalization: "; ScalarCI->dump());
  }

  // Remove the old math library calls since they have been replaced with the
  // legalized library calls or they have been scalarized.
  for (auto *Inst : InstToRemove) {
    Inst->eraseFromParent();
  }

  return Dirty; // Has the LLVM IR been modified?
}

FunctionPass *llvm::createMapIntrinToImlPass() {
  return new llvm::vpo::MapIntrinToIml();
}

using namespace llvm::vpo;

char MapIntrinToIml::ID = 0;

static const char lv_name[] = "MapIntrinToIml";
INITIALIZE_PASS_BEGIN(MapIntrinToIml, SV_NAME, lv_name,
                      false /* modifies CFG */, false /* transform pass */)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(MapIntrinToIml, SV_NAME, lv_name,
                    false /* modififies CFG */, false /* transform pass */)
