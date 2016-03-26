//==---- MapIntrinToIml.cpp - Replace intrinsics with IML call -*- C++ -*---==//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
/// This pass consumes llvm intrinsics and maps them to libm calls or svml.
/// For vector calls, this pass also performs type legalization for when
/// partial registers are used or when the logical vector length exceeds the
/// true vector length of the target architecture. The type legalization
/// adjusts vector lengths of the svml calls and those instructions that are
/// users of them. The rest of the legalization is left to ISD-level type
/// legalization.
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
    cl::desc("Scalarize vector intrinsic instead of calling svml."));

MapIntrinToIml::MapIntrinToIml() : FunctionPass(ID) {}

// The idea of this pass is simple and involves these steps:
// 1) Find vectorized math library related intrinsic calls, or scalar
//    intrinsics.
// 2) Get the IMF attributes in the form of LLVM Attributes from the call.
// 3) Map each call Attribute to an ImfAttr struct, building a list of
//    attribute value pairs <char* attr_name, char* attr_value> that can be
//    used by the get_library_function_name() function found in libiml_attr.
//    This is the external interface used to do the actual mapping of
//    intrinsics to SVML/libm functions. The ImfAttr struct is populated using
//    the strings below for attribute names. The value of the attribute is
//    just a string also. libiml_attr will then select the appropriate SVML/libm
//    function based on the function name and attributes.
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

bool MapIntrinToIml::parseIntrinsicName(StringRef IntrinName,
                                        StringRef &FuncName,
                                        TargetLibraryInfo *TLI) {
  // This function extracts the math function name from the intrinsic call.
  //
  // Example:
  //
  // llvm.sin.v4f32
  //
  // FuncName = sin
  //
  // The caller then uses this information to query the IML interface.

  StringRef IntrinPrefix = "llvm.";

  size_t Idx = IntrinName.find(IntrinPrefix, 0);

  // Not an intrinsic call. By definition, all LLVM intrinsics start with
  // "llvm.".
  if (Idx == StringRef::npos)
    return false;

  // Skip past the "llvm." prefix.
  StringRef RemainFuncName =
      IntrinName.substr(Idx + IntrinPrefix.size(), StringRef::npos);

  // We should find another '.' delimiter for math calls that serves as the
  // separator for the type information. If not, return unsuccessfully, and
  // find the next intrinsic call.
  Idx = RemainFuncName.find(".", 0);
  if (Idx == StringRef::npos)
    return false;

  // Checking the LibFunc entries serves as an additional filter so that we
  // do not process intrinsics such as "llvm.lifetime.start", etc.
  LibFunc::Func LF;
  FuncName = RemainFuncName.substr(0, Idx);
  bool IsLibFunc = TLI->getLibFunc(FuncName, LF);
  if (!IsLibFunc)
    return false;

  return true;
}

void MapIntrinToIml::addAttributeToList(ImfAttr **List, ImfAttr **Tail,
                                        ImfAttr *Attr) {
  if (*Tail)
    (*Tail)->next = Attr;
  else
    *List = Attr;

  *Tail = Attr;
}

void MapIntrinToIml::deleteAttributeList(ImfAttr **List) {
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
bool MapIntrinToIml::isValidIMFAttribute(std::string AttrName) {
  if (AttrName == "absolute-error" || AttrName == "accuracy-bits" ||
      AttrName == "accuracy-bits-128" || AttrName == "accuracy-bits-32" ||
      AttrName == "accuracy-bits-64" || AttrName == "accuracy-bits-80" ||
      AttrName == "arch-consistency" || AttrName == "configuration" ||
      AttrName == "domain-exclusion" || AttrName == "max-error" ||
      AttrName == "precision" || AttrName == "valid-status-bits")
    return true;

  return false;
}

unsigned MapIntrinToIml::calculateNumReturns(TargetTransformInfo *TTI,
                                             unsigned TypeBitWidth,
                                             unsigned LogicalVL,
                                             unsigned *TargetVL) {
  unsigned VectorBitWidth = TTI->getRegisterBitWidth(true);
  *TargetVL = VectorBitWidth / TypeBitWidth;
  unsigned NumRet = LogicalVL / *TargetVL;

  // If the logical vector width is smaller than the target vector width,
  // then we have less than full vector. Thus, just set NumRet = 1.
  NumRet = NumRet == 0 ? 1 : NumRet;

  DEBUG(dbgs() << "Type Bit Width: " << TypeBitWidth << "\n");
  DEBUG(dbgs() << "Legalizing VL: " << LogicalVL << "\n");
  DEBUG(dbgs() << "Vector Bit Width: " << VectorBitWidth << "\n");
  DEBUG(dbgs() << "Legal Target VL: " << *TargetVL << "\n");
  DEBUG(dbgs() << "Num Regs: " << NumRet << "\n");

  return NumRet;
}

void MapIntrinToIml::splitArgs(
    SmallVectorImpl<Value *> &Args,
    SmallVectorImpl<SmallVector<Value *, 8>> &NewArgs, unsigned NumRet,
    unsigned TargetVL) {
  DEBUG(dbgs() << "Splitting Args to match legal VL:\n");
  NewArgs.resize(NumRet);

  for (unsigned I = 0; I < Args.size(); I++) {

    DEBUG(dbgs() << "Arg Name: ");
    Args[I]->dump();
    DEBUG(dbgs() << "\n");
    DEBUG(dbgs() << "Arg Type: " << *Args[I]->getType() << "\n");

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

void MapIntrinToIml::createImfAttributeList(IntrinsicInst *II,
                                            ImfAttr **List) {
  // Tail of the linked list of IMF attributes. The head of the list is
  // passed in from the caller via the List parameter.
  ImfAttr *Tail = nullptr;

  // Build the linked list of IMF attributes that will be used to query
  // the IML interface.

  const StringRef ImfPrefix = "imf-";
  const AttributeSet Attrs = II->getAttributes();
  unsigned Slot = ~0U;
  for (unsigned I = 0, E = Attrs.getNumSlots(); I != E; ++I) {
    if (Attrs.getSlotIndex(I) == AttributeSet::FunctionIndex) {
      Slot = I;
      break;
    }
  }

  AttributeSet::iterator FAIt = Attrs.begin(Slot);
  AttributeSet::iterator FAEnd = Attrs.end(Slot);
  for (; FAIt != FAEnd; ++FAIt) {
    // Attributes will be of the form:
    //
    // "imf-max-error"="0.6"
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

  // If no IMF attributes were found for the function call, then default to
  // high accuracy.
  if (!*List) {
    ImfAttr *MaxError = new ImfAttr();
    MaxError->name = "max-error";
    MaxError->value = "0.5";
    ImfAttr *Precision = new ImfAttr();
    Precision->name = "precision";
    Precision->value = "high";
    MaxError->next = Precision;
    Precision->next = nullptr;
    addAttributeToList(List, &Tail, MaxError);
    addAttributeToList(List, &Tail, Precision);
  }

  // TODO: only debug mode
  ImfAttr *CurrAttr = *List;
  DEBUG(dbgs() << "Attribute List for function:\n");
  while (CurrAttr) {
    DEBUG(dbgs() << CurrAttr->name << " = " << CurrAttr->value << "\n");
    CurrAttr = CurrAttr->next;
  }
  // end debug
}

void MapIntrinToIml::addAlwaysInlineAttribute(CallInst *CI) {
  AttrBuilder AttrList;
  AttrList.addAttribute(Attribute::AlwaysInline);
  CI->setAttributes(CI->getAttributes().addAttributes(
      CI->getContext(), AttributeSet::FunctionIndex,
      AttributeSet::get(CI->getContext(), AttributeSet::FunctionIndex,
                        AttrList)));
}

FunctionType *
MapIntrinToIml::legalizeFunctionTypes(FunctionType *FT,
                                      SmallVectorImpl<Value *> &Args,
                                      unsigned TargetVL, LibFunc::Func &LF) {
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
    unsigned TypeVL = LF == LibFunc::sincos ? TargetVL * 2 : TargetVL;
    ReturnType = VectorType::get(VectorReturn->getElementType(), TypeVL);
  }

  FunctionType *LegalFT = FunctionType::get(ReturnType, NewArgTypes, false);
  return LegalFT;
}

void MapIntrinToIml::generateSvmlCalls(
    unsigned NumRet, Constant *Func,
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
    Twine TempName = "vcall";
    CallInst *NewCI = CallInst::Create(Func, Args[I], TempName);
    NewCI->insertAfter(*InsertPt);
    Calls.push_back(NewCI);
    *InsertPt = NewCI;
  }
}

Instruction *
MapIntrinToIml::combineSvmlCallResults(unsigned NumRet,
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
  // First, %1 (original intrinsic arg) is split via splitArgs() call above
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
  // %final then replaces the users of the original intrinsic. The remainder of
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

LoadStoreMode MapIntrinToIml::getLoadStoreModeForArg(AttributeSet &AS,
                                                     unsigned ArgNo,
                                                     StringRef &AttrValStr) {
  assert(AS.hasAttribute(ArgNo, "stride") &&
         "Expected argument to have a specified stride");
  Attribute Attr = AS.getAttribute(ArgNo, "stride");
  AttrValStr = Attr.getValueAsString();

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

// TODO: For now, we assume LoadStoreMode is the same for each arg.
// This will need to change because it's possible that the memory
// reference pattern is different for each one.
void MapIntrinToIml::generateSinCosStore(IntrinsicInst *VectorIntrin,
                                         Instruction *ResultVector,
                                         unsigned NumElemsToStore,
                                         unsigned TargetVL,
                                         unsigned StorePtrIdx,
                                         Instruction **InsertPt) {
  // For __svml_sincos calls, the result vector is always 2x that of the input
  // vector.
  unsigned NumResultVectors = 2;

  AttributeSet AttrList = VectorIntrin->getAttributes();
  // For Arg 2, I = 0. Attributes are at position 2
  // For Arg 3, I = 1. Attributes are at position 3

  for (unsigned I = 0; I < NumResultVectors; ++I) {

    // Name used to distinguish what is going into registers.
    // Iteration 0 is sin
    // Iteration 1 is cos
    StringRef ResultName = I == 0 ? "sin" : "cos";

    // This represents the address to where we will store the sin/cos results.
    // Either arg 2 or 3 from the vector sincos call.
    Value *SvmlArg = VectorIntrin->getArgOperand(I + 1);

    // Shuffle out the sin/cos results from the return vector of the svml call.
    // This is necessary since we have a 2 x VL wide return vector.
    SmallVector<Constant *, 8> Splat;
    unsigned StartElemIdx = I * TargetVL;
    unsigned ElemIdx = StartElemIdx;

    Instruction *ShuffleInst =
        extractElemsFromVector(ResultVector, ElemIdx, NumElemsToStore);
    ShuffleInst->insertAfter(*InsertPt);
    *InsertPt = ShuffleInst;

    VectorType *ShuffleType = dyn_cast<VectorType>(ShuffleInst->getType());
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
        Twine ExtractName = ResultName + ".elem";
        ExtractElementInst *ElemToStore =
            ExtractElementInst::Create(ShuffleInst, ElmIdx, ExtractName);
        ElemToStore->insertAfter(*InsertPt);
        *InsertPt = ElemToStore;

        // Extract the pointer from the vector of pointers argument from the
        // intrinsic call.
        Twine ExtractNamePtr = ResultName + ".elem.ptr";
        ExtractElementInst *ElemToStorePtr =
            ExtractElementInst::Create(SvmlArg, ElmIdxPtr, ExtractNamePtr);
        ElemToStorePtr->insertAfter(*InsertPt);
        *InsertPt = ElemToStorePtr;

        // Store the sin/cos element to the pointer.
        StoreInst *Store = new StoreInst(ElemToStore, ElemToStorePtr);
        Store->insertAfter(*InsertPt);
        *InsertPt = Store;
      }
    } else if (Mode == UNIT_STRIDE) {
      // Generate unit stride store. Here, we just need to extract the 1st
      // address of the argument vector from the intrinsic because this will
      // represent the base address for the vector store. If there are multiple
      // returns, the index into the pointer of vectors on the intrinsic arg
      // needs to advance by the number of elements being stored. E.g.,
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

bool MapIntrinToIml::isLessThanFullVector(Type *ValType, Type *LegalType) {

  VectorType *ValVecType = dyn_cast<VectorType>(ValType);
  VectorType *LegalVecType = dyn_cast<VectorType>(LegalType);

  if (ValVecType && LegalVecType &&
      ValVecType->getBitWidth() < LegalVecType->getBitWidth()) {
    return true;
  }

  return false;
}

void MapIntrinToIml::generateNewArgsFromPartialVectors(
    IntrinsicInst *II, FunctionType *FT, unsigned TargetVL,
    SmallVectorImpl<Value *> &NewArgs, Instruction **InsertPt) {
  // This function builds a new argument list for the svml function call by
  // finding any arguments that are less than full vector and duplicating the
  // low order elements into the upper part of a new vector register. If the
  // argument type is already legal, then just insert it as is in the arg list.

  for (unsigned I = 0; I < FT->getNumParams(); ++I) {

    // Type of the parameter on the intrinsic, which can be driven by the user
    // specifying an explicit vectorlength.
    Value *NewArg = II->getArgOperand(I);
    VectorType *VecArgType = dyn_cast<VectorType>(NewArg->getType());

    // The type of the parameter if using the full register specified through
    // legalization.
    VectorType *LegalVecArgType = dyn_cast<VectorType>(FT->getParamType(I));

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
      ShuffleVectorInst *ShuffleInst = new ShuffleVectorInst(
          II->getArgOperand(I), Undef, Mask, "shuffle.dup");
      ShuffleInst->insertBefore(*InsertPt);
      *InsertPt = ShuffleInst;
      NewArg = ShuffleInst;
    }

    NewArgs.push_back(NewArg);
  }
}

Instruction *MapIntrinToIml::extractElemsFromVector(Value *Reg,
                                                    unsigned StartPos,
                                                    unsigned NumElems) {
  Type *RegType = Reg->getType();
  assert(RegType->isVectorTy() && "Expected vector register type for extract");

  VectorType *VecRegType = dyn_cast<VectorType>(RegType);

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
// correct intrinsic type information. sincos is void, but the incoming vector
// intrinsic will be transformed to an svml call that returns a 2x wide vector
// based on the type of the 1st argument of the intrinsic signature. For ilogb,
// the number of elements returned is the number of elements indicated by the
// 1st argument since it is 2x the size of the return. Case 2 will work as is.

VectorType *MapIntrinToIml::getIntrinsicType(IntrinsicInst *Intrin) {

  Intrinsic::ID ID = Intrin->getIntrinsicID();

DEBUG(dbgs() << "Intrin: " << *Intrin << "\n");
  if (ID == Intrinsic::sincos)
    return dyn_cast<VectorType>(Intrin->getArgOperand(0)->getType());

  FunctionType *IntrinFT = Intrin->getFunctionType();
  Type *IntrinRetType = IntrinFT->getReturnType();
  return dyn_cast<VectorType>(IntrinRetType);
}

void MapIntrinToIml::scalarizeVectorIntrinsic(IntrinsicInst *II,
                                              StringRef LibFuncName,
                                              unsigned VL, Type *ElemType) {
  SmallVector<Type *, 4> FTArgTypes;
  StringRef LibFuncVariantName;

  FunctionType *IntrinFT = II->getFunctionType();
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

  if (ElemType->isFloatTy()) {
    Twine LibFuncStr = LibFuncName + "f";
    LibFuncVariantName = LibFuncStr.str();
  } else if (ElemType->isDoubleTy()) {
    LibFuncVariantName = LibFuncName;
  }

  FunctionType *FT = FunctionType::get(RetType, FTArgTypes, false);
  Constant *FCache = M->getOrInsertFunction(LibFuncVariantName, FT);
  SmallVector<Value *, 4> CallResults;

  for (unsigned I = 0; I < VL; ++I) {
    SmallVector<Value *, 4> ScalarCallArgs;

    for (unsigned J = 0; J < II->getNumArgOperands(); ++J) {
      Value *Parm = II->getArgOperand(J);
      VectorType *VecParmType = dyn_cast<VectorType>(Parm->getType());

      if (VecParmType) {
        Constant *Index =
            ConstantInt::get(Type::getInt32Ty(Func->getContext()), I);

        ExtractElementInst *Extract =
            ExtractElementInst::Create(II->getOperand(J), Index, "arg", II);

        Parm = Extract;
      }

      ScalarCallArgs.push_back(Parm);
    }

    CallInst *ScalarCall = CallInst::Create(FCache, ScalarCallArgs);
    ScalarCall->insertBefore(II);
    CallResults.push_back(ScalarCall);
  }

  if (!RetType->isVoidTy()) {
    // If this is not a void intrinsic then there will be users of the call
    // results. Insert each scalar call instruction (this corresponds to an
    // SSA temp) into a vector and replace the original vector intrinsic call's
    // users.

    Value *InsertVector =
        UndefValue::get(II->getFunctionType()->getReturnType());

    for (unsigned I = 0; I < CallResults.size(); ++I) {
      Constant *Index =
          ConstantInt::get(Type::getInt32Ty(Func->getContext()), I);
      InsertVector = InsertElementInst::Create(InsertVector, CallResults[I],
                                               Index, "ins", II);
    }

    // Find the instructions that are using the intrinsic results and replace
    // with the vector that has all of the scalar call results.
    II->replaceAllUsesWith(InsertVector);
  }
}

bool MapIntrinToIml::runOnFunction(Function &F) {
  DEBUG(dbgs() << "\nExecuting MapIntrinToIml ...\n\n");
  if (RunSvmlStressMode) {
    DEBUG(dbgs() << "Stress Testing Mode Invoked - intrinsic calls will be "
                    "scalarized");
  }

  Func = &F;
  M = F.getParent();

  // Use TTI to provide information on the legal vector register size for the
  // target.
  TargetTransformInfo *TTI =
      &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

  TargetLibraryInfo *TLI =
      &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

  const DataLayout DL = M->getDataLayout();

  // Will be populated with the intrinsics that will be replaced with svml
  // calls.
  SmallVector<Instruction *, 4> InstToRemove;

  // Dirty becomes true (LLVM IR is modified) under two circumstances:
  // 1) A candidate vector math intrinsic is found and is replaced with an
  //    svml/libm variant.
  // 2) A candidate vector math intrinsic is found, is not replaced by an
  //    svml/libm variant, but is scalarized.
  bool Dirty = false; // LLVM IR not yet modified

  // Begin searching for vector intrinsics that are candidates for translation
  // to svml/libm.
  for (inst_iterator Inst = inst_begin(F), InstEnd = inst_end(F);
       Inst != InstEnd; ++Inst) {

    // Attempt to find an SVML/libm variant for all intrinsic calls and let
    // libiml_attr worry about those functions that do not have any SVML/libm
    // variants. It will simply return a NULL pointer if a variant is not found.
    // This way we don't need to check any specific intrinsic IDs. If a vector
    // math intrinsic is found but not replaced by an svml/libm variant, the
    // intrinsic is scalarized. Dirty stays false if this check fails since
    // a vector intrinsic is not found.
    if (IntrinsicInst *II = dyn_cast<IntrinsicInst>(&*Inst)) {

      StringRef FuncName;
      if (!parseIntrinsicName(II->getCalledFunction()->getName(), FuncName,
                              TLI))
        continue;

      unsigned ScalarBitWidth = 0;
      StringRef DataType;
      LibFunc::Func LF;
      TLI->getLibFunc(FuncName, LF);

      VectorType *IntrinType = getIntrinsicType(II);

      // Translation of scalar intrinsics is not yet supported. This check will
      // be removed when libm support is added, and the code will be refactored
      // so that there are separate code paths for vector(svml) and scalar(libm)
      // functionality. As of now, the code below assumes svml translation only.
      if (!IntrinType)
        continue;

      unsigned LogicalVL = IntrinType->getNumElements();
      Type *ElemType = IntrinType->getElementType();

      // Need to go through DataLayout in cases where we have pointer types to
      // get the correct pointer bit size.
      ScalarBitWidth = DL.getTypeSizeInBits(ElemType);

      // Translation for float versions of svml variants require an 'f' to
      // be part of the name. e.g., sinf vs. sin for double.
      if (ElemType->isFloatTy())
        DataType = "f";

      // Get the number of svml calls that will be required, indicated by
      // NumRet. NumRet is determined by getting the vector type info from the
      // intrinsic signature. See notes in getIntrinsicType() for more
      // information.
      unsigned TargetVL = 0;
      unsigned NumRet =
          calculateNumReturns(TTI, ScalarBitWidth, LogicalVL, &TargetVL);

      std::string TargetVLString = std::to_string(TargetVL);

      // Construct the function name that will be used to look up the proper
      // SVML/libm variant (the parent function).
      Twine TempFuncName = "__svml_" + FuncName + DataType + TargetVLString;
      std::string TempFuncNameRef = TempFuncName.str();
      char *ParentFuncName = new char[TempFuncNameRef.length() + 1];
      std::strcpy(ParentFuncName, TempFuncNameRef.c_str());

      DEBUG(dbgs() << "Parent Function: " << ParentFuncName << "\n");

      ImfAttr *AttrList = nullptr;
      createImfAttributeList(II, &AttrList);

      // External libiml_attr interface that returns the SVML/libm variant if
      // the parent function and IMF attributes match. Return NULL otherwise.
      const char *VariantFuncName =
          get_library_function_name(ParentFuncName, AttrList);

      // No longer need the IMF attribute list at this point, so free up the
      // memory. Note: this does not remove the attributes from the instruction,
      // only the internal data structure used to query the iml interface.
      deleteAttributeList(&AttrList);

      // An svml/libm variant was found for the vector intrinsic, so replace
      // the intrinsic with the new svml/libm calls. Set Dirty to true since
      // the LLVM IR is modified.
      if (VariantFuncName && !RunSvmlStressMode) {
        StringRef VariantFuncNameRef = StringRef(VariantFuncName);
        DEBUG(dbgs() << "IML Variant: " << VariantFuncNameRef << "\n\n");

        // Original arguments to the vector intrinsic.
        SmallVector<Value *, 8> Args;

        FunctionType *FT;

        if (LF == LibFunc::sincos) {
          // We have to do some special handling for sincos calls because we
          // must transform the widened intrinsic form of
          // 'void sincos(<vl x type>, <vl x type*>, <vl x type*>)' to
          // '<vl x 2 x type> __svml_sincos(<vl x type>)'
          Args.push_back(II->getArgOperand(0));
          VectorType *RetType =
              VectorType::getDoubleElementsVectorType(IntrinType);
          FT = FunctionType::get(RetType, IntrinType, false);
        } else {
          unsigned NumArgs = II->getNumArgOperands();
          for (unsigned I = 0; I < NumArgs; I++) {
            Value *Arg = II->getArgOperand(I);
            Args.push_back(Arg);
          }
          FT = II->getFunctionType();
        }

        // FT will point to the legal FunctionType based on target register
        // size requirements. This FunctionType is used to create the call
        // to the svml function.
        FT = legalizeFunctionTypes(FT, Args, TargetVL, LF);
        Constant *FCache = M->getOrInsertFunction(VariantFuncNameRef, FT);
        Instruction *InsertPt = II;

        // WorkList contains the instructions that are the results of svml
        // calls. This could be the call instructions themselves, or the
        // results of shuffles for calls to sincos or calls to functions
        // where the result is less than full vector.
        SmallVector<Instruction *, 8> WorkList;

        if (NumRet > 1) {
          // NewArgs contains the type legalized parameters that are split in
          // splitArgs(). These are the results of shuffle instructions.
          SmallVector<SmallVector<Value *, 8>, 8> NewArgs;
          splitArgs(Args, NewArgs, NumRet, TargetVL);

          generateSvmlCalls(NumRet, FCache, NewArgs, WorkList, &InsertPt);

          if (LF != LibFunc::sincos) {
            // There will be no users of sincos because it's a void function.
            // Because of this, we have to generate the store instructions
            // explicitly. See generateSinCosStore().
            Instruction *FinalResult =
                combineSvmlCallResults(NumRet, WorkList, &InsertPt);
            II->replaceAllUsesWith(FinalResult);
          }
        } else {
          // NumRet = 1

          // Type legalization still needs to happen for NumRet = 1 when
          // dealing with less than full vector cases.
          // generateNewArgsFromPartialVectors() duplicates the low elements
          // into the upper part of the vector so the svml call operates on
          // safe values. But, the duplicate results are not needed, so shuffle
          // out the ones we want and then replace the users of the function
          // call with the shuffle. This work is done via
          // extractElemsFromVector().
          //
          // Assuming target register width of xmm (128-bit) and user
          // specifying explicit vl=2:
          //
          // Before Transformation:
          //
          // %7 = sitofp <2 x i32> %induction3.1 to <2 x float>
          // %8 = call <2 x float> @llvm.cos.v2f32(<2 x float> %7)
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
          generateNewArgsFromPartialVectors(II, FT, TargetVL, NewArgs,
                                            &InsertPt);

          CallInst *NewCI = CallInst::Create(FCache, NewArgs, "vcall");
          Instruction *CallResult = NewCI;
          NewCI->insertAfter(InsertPt);

          if (LF != LibFunc::sincos) {
            bool LessThanFullVector = isLessThanFullVector(
                II->getFunctionType()->getReturnType(), FT->getReturnType());

            if (LessThanFullVector) {
              // sincos requires a special extraction because it has a double
              // wide return register and returns void. This is dealt with in
              // generateSinCosStore().
              //
              // Extract the number of elements specified by the partial vector
              // return type of the intrinsic (indicated by LogicalVL). The
              // NewCI return type will indicate the size of the vector
              // extracted from. Start extracting from position 0.
              CallResult = extractElemsFromVector(NewCI, 0, LogicalVL);
              CallResult->insertAfter(NewCI);
            }

            II->replaceAllUsesWith(CallResult);
          } else {
            WorkList.push_back(NewCI);
          }
        }

        if (LF == LibFunc::sincos) {
          unsigned StorePtrIdx = 0;
          // For partial register cases, we only want to extract the partial
          // number of elements from the results. Otherwise, extract the full
          // legal register width number of elements.
          unsigned NumElemsToStore =
              LogicalVL < TargetVL ? LogicalVL : TargetVL;
          for (unsigned I = 0; I < NumRet; ++I) {
            InsertPt = WorkList[I];
            generateSinCosStore(II, WorkList[I], NumElemsToStore, TargetVL,
                                StorePtrIdx, &InsertPt);
            StorePtrIdx += NumElemsToStore;
          }
        }

        InstToRemove.push_back(II);
        Dirty = true; // LLVM-IR has been changed because we found an svml/libm
                      // variant.
      } else {
        // TODO: If for some reason svml call translation failed, it is possible
        // that the vector intrinsic left in the LLVM IR will not be recognized
        // by the back-end. To prevent this, scalarize the vector intrinsic
        // call. Calls to vector intrinsics that already exist in LLVM do not
        // have to go through this interface since they can be scalarized at the
        // ISD level, but any new intrinsics added will not have back-end
        // support. Thus, for consistency sake, just handle all scalarization of
        // svml translation failures here. Also scalarize when running in
        // stress test mode. Since the LLVM IR is modified, set Dirty to true.

        scalarizeVectorIntrinsic(II, TLI->getName(LF), LogicalVL, ElemType);
        InstToRemove.push_back(II);
        Dirty = true; // LLVM-IR has been changed because the vector intrinsic
                      // was replaced with scalar calls.

        DEBUG(dbgs() << "\n\n");
      }
    }
  }

  // Remove the old intrinsic calls since they have been replaced with the
  // svml/libm calls or the intrinsic has been scalarized.
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
INITIALIZE_PASS_END(MapIntrinToIml, SV_NAME, lv_name,
                    false /* modififies CFG */, false /* transform pass */)
