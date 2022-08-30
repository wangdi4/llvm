//==--- MapIntrinToIml.cpp - Legalize svml calls and apply IMF -*- C++ -*---==//
//                           attributes.
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Transforms/Utils/Intel_IMLUtils.h"
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
// "use-svml"
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
      AttrName == "domain-exclusion" || AttrName == "force-dynamic" ||
      AttrName == "max-error" || AttrName == "precision" ||
      AttrName == "use-svml" || AttrName == "valid-status-bits")
    return true;

  return false;
}

unsigned MapIntrinToImlImpl::calculateNumReturns(TargetTransformInfo *TTI,
                                                 unsigned ComponentBitWidth,
                                                 unsigned LogicalVL,
                                                 unsigned *TargetVL) {
  unsigned VectorBitWidth =
      TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector);
  // Under x86 architecture, getRegisterBitWidth() may return 0 for vectors
  // if no vector ISA is specified. In this case, there should not be any SVML
  // call in the input IR.
  assert(VectorBitWidth != 0 && "SVML call is not expected when compiling to a "
                                "target without vector ISA support.");
  *TargetVL = VectorBitWidth / ComponentBitWidth;
  unsigned NumRet = LogicalVL / *TargetVL;

  // If the logical vector width is smaller than the target vector width,
  // then we have less than full vector. Thus, just set NumRet = 1.
  NumRet = NumRet == 0 ? 1 : NumRet;

  LLVM_DEBUG(dbgs() << "Component Bit Width: " << ComponentBitWidth << "\n");
  LLVM_DEBUG(dbgs() << "Legalizing VL: " << LogicalVL << "\n");
  LLVM_DEBUG(dbgs() << "Vector Bit Width: " << VectorBitWidth << "\n");
  LLVM_DEBUG(dbgs() << "Legal Target VL: " << *TargetVL << "\n");
  LLVM_DEBUG(dbgs() << "Num Regs: " << NumRet << "\n");

  return NumRet;
}

void MapIntrinToImlImpl::createImfAttributeList(Instruction *I,
                                                ImfAttr **List) {

  // Tail of the linked list of IMF attributes. The head of the list is
  // passed in from the caller via the List parameter.
  ImfAttr *Tail = nullptr;

  // Set default precision to high accuracy. For bitwise reproducible svml
  // functions, the iml accuracy inferface expects these attributes to appear
  // before imf-arch-consistency.
  ImfAttr *Precision = new ImfAttr();
  Precision->name = "precision";
  // If fast math is enabled, use medium accuracy (as ICC does), otherwise
  // defaults to high accuracy.
  Precision->value = isa<FPMathOperator>(I) &&
    I->getFastMathFlags().approxFunc() ? "medium" : "high";
  Precision->next = nullptr;
  addAttributeToList(List, &Tail, Precision);

  // Populate the list using attributes attached to this instruction if it's a
  // call, otherwise just return the default list.
  CallInst *CI = dyn_cast<CallInst>(I);
  if (!CI)
    return;

  // Try to select an SVML function variant for the ISAs enabled in the current
  // function statically
  ImfAttr *ISASetAttr = new ImfAttr();
  ISASetAttr->name = "isa-set";
  ISASetAttr->value = TTI->getISASetForIMLFunctions();
  addAttributeToList(List, &Tail, ISASetAttr);

  // Build the linked list of IMF attributes that will be used to query
  // the IML interface.

  const StringRef ImfPrefix = "imf-";
  const AttributeList AttrList = CI->getAttributes();

  if (AttrList.hasFnAttrs()) {

    AttributeSet Attrs = AttrList.getFnAttrs();

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
      if (AttrName.find(std::string(ImfPrefix)) != 0)
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

// Legalize a single type T to TargetVL. There are two cases to handle:
// 1) T is a vector type: Returns a new vector type with TargetVL components.
// 2) T is a structure type with vector fields (appeared in arguments and return
//    value of TwoRets functions): Returns a new structure type with each field
//    legalized.
static Type *legalizeArgumentOrReturnType(Type *T, unsigned LogicalVL,
                                          unsigned TargetVL) {
  if (VectorType *VecTy = dyn_cast<VectorType>(T)) {
    return VectorType::get(
        VecTy->getElementType(),
        (VecTy->getElementCount().divideCoefficientBy(LogicalVL)) * TargetVL);
  }

  assert(T->isStructTy() &&
         "Expect vector or struct type in SVML function type legalization");
  SmallVector<Type *, 2> NewStructElementTypes;
  for (unsigned I = 0; I < T->getStructNumElements(); I++) {
    Type *StructElementTy = T->getStructElementType(I);
    assert(StructElementTy->isVectorTy() &&
           "Expect all elements in struct to be vectors");
    VectorType *VecTy = cast<VectorType>(StructElementTy);
    NewStructElementTypes.push_back(VectorType::get(
        VecTy->getElementType(),
        (VecTy->getElementCount().divideCoefficientBy(LogicalVL)) * TargetVL));
  }

  return StructType::get(T->getContext(), NewStructElementTypes);
}

FunctionType *MapIntrinToImlImpl::legalizeFunctionTypes(FunctionType *FT,
                                                        ArrayRef<Value *> Args,
                                                        unsigned LogicalVL,
                                                        unsigned TargetVL,
                                                        StringRef FuncName) {
  // Perform type legalization for arguments and return type respectively.

  // New type legalized argument types.
  SmallVector<Type *, 8> NewArgTypes;
  for (unsigned I = 0; I < Args.size(); I++)
    NewArgTypes.push_back(
        legalizeArgumentOrReturnType(Args[I]->getType(), LogicalVL, TargetVL));

  Type *ReturnType =
      legalizeArgumentOrReturnType(FT->getReturnType(), LogicalVL, TargetVL);

  FunctionType *LegalFT = FunctionType::get(ReturnType, NewArgTypes, false);
  return LegalFT;
}

void MapIntrinToImlImpl::splitMathLibCalls(
    unsigned NumRet, unsigned TargetVL, FunctionCallee Func,
    ArrayRef<Value *> Args, SmallVectorImpl<Value *> &SplitCalls) {
  LLVM_DEBUG(dbgs() << "Splitting Args to match legal VL:\n";
             for (const Value *A : Args)
               dbgs() << "Arg Value: " << *A << "\n"
                      << "Arg Type: " << *A->getType() << "\n");

  // Use shuffle instructions to split the parameters into pseudo-registers
  // of size TargetVL. For example, if the logical VL = 8, and TargetVL = 4,
  // then the vector parameter can be split into two parts, where the
  // shuffle masks correspond to the following:
  //
  // Shuffle Mask for Part 1:
  // <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  //
  // Shuffle Mask for Part 2:
  // <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  //
  // The resulting registers will hold the elements corresponding to the
  // masks.
  //
  //
  // If an parameter is a struct of vectors, each field needs to be extracted
  // and shuffled individually, these split fields then form the new struct
  // parameters.
  // The example below demonstrates a hypothetical case splitting VF=8 to VF=4,
  // with both struct and vector parameters present (struct parameter only
  // occurs for VF=16 variant of masked sincosf, a short VF is used here just
  // for the sake of brevity):
  //
  // %src.sin = extractvalue { <8 x float>, <8 x float> } %src, 0
  // %src.cos = extractvalue { <8 x float>, <8 x float> } %src, 1
  // %src.part1.sin = shufflevector <8 x float> %src.sin, <8 x float> undef,
  //                                <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %src.part1.tmp = insertvalue { <4 x float>, <4 x float> } undef,
  //                              <4 x float> %src.part1.sin, 0
  // %src.part1.cos = shufflevector <8 x float> %src.cos, <8 x float> undef,
  //                                <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %src.part1 = insertvalue { <4 x float>, <4 x float> } %src.part1.tmp,
  //                          <4 x float> %src.part1.cos, 1
  // %mask.part1 = shufflevector <8 x i1> %mask, <8 x i1> undef,
  //                             <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %arg.part1 = shufflevector <8 x float> %arg, <8 x float> undef,
  //                            <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %result.part1 = call svml_cc { <4 x float>, <4 x float> }
  //                      @__svml_sincosf4_ha_mask(
  //                        { <4 x float>, <4 x float> } %src.part1,
  //                        <4 x i1> %mask.part1, <4 x float> %arg.part1)
  // %src.part2.sin = shufflevector <8 x float> %src.sin, <8 x float> undef,
  //                                <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  // %src.part2.tmp = insertvalue { <4 x float>, <4 x float> } undef,
  //                              <4 x float> %src.part2.sin, 0
  // %src.part2.cos = shufflevector <8 x float> %src.cos, <8 x float> undef,
  //                                <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  // %src.part2 = insertvalue { <4 x float>, <4 x float> } %src.part2.tmp,
  //                          <4 x float> %src2.cos, 1
  // %mask.part2 = shufflevector <8 x i1> %mask, <8 x i1> undef,
  //                             <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  // %arg.part2 = shufflevector <8 x float> %arg, <8 x float> undef,
  //                            <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  // %result.part2 = call svml_cc { <4 x float>, <4 x float> }
  //                      @__svml_sincosf4_ha_mask(
  //                        { <4 x float>, <4 x float> } %src.part2,
  //                        <4 x i1> %mask.part2, <4 x float> %arg.part2)

  // Extract all fields in struct parameters (if any) and cache them to ensure
  // these extractvalue instructions are emitted only once.
  SmallVector<SmallVector<Value *, 2>, 4> StructFields;
  StructFields.resize(Args.size());
  for (unsigned I = 0; I < Args.size(); I++) {
    Value *Arg = Args[I];
    StructType *ArgStructType = dyn_cast<StructType>(Args[I]->getType());
    if (!ArgStructType)
      continue;
    for (unsigned J = 0; J < ArgStructType->getNumElements(); J++)
      StructFields[I].push_back(
          Builder.CreateExtractValue(Arg, J, "extract.split"));
  }

  for (unsigned Part = 0; Part < NumRet; Part++) {
    SmallVector<Value *, 8> NewArgs;

    for (unsigned ArgIdx = 0; ArgIdx < Args.size(); ArgIdx++) {
      Value *Arg = Args[ArgIdx];
      Type *ArgType = Args[ArgIdx]->getType();

      if (ArgType->isVectorTy()) {
        NewArgs.push_back(generateExtractSubVector(Arg, Part, NumRet, Builder));
        continue;
      }

      assert(ArgType->isStructTy() &&
             "SVML functions only accept vector or struct of vector arguments");

      Value *LegalArg =
          UndefValue::get(Func.getFunctionType()->getParamType(ArgIdx));
      for (unsigned FieldIdx = 0; FieldIdx < ArgType->getStructNumElements();
           ++FieldIdx) {
        Value *ExtractInst = StructFields[ArgIdx][FieldIdx];
        assert(
            ExtractInst &&
            "An extractfield instruction should be pre-created but not found");
        assert(ExtractInst->getType()->isVectorTy() &&
               "Expect all elements in struct argument to be vectors");
        Value *ShuffleInst =
            generateExtractSubVector(ExtractInst, Part, NumRet, Builder);
        LegalArg = Builder.CreateInsertValue(LegalArg, ShuffleInst, FieldIdx,
                                             "insert.arg");
      }
      NewArgs.push_back(LegalArg);
    }

    CallInst *NewCI = createSVMLCall(Func, NewArgs, "vcall");
    SplitCalls.push_back(NewCI);
  }
}

/// Join a list of vectors into a single vector, and optionally merge it with
/// \p SourceValue using \p Mask, if both of them are present. If \p SourceValue
/// is nullptr, \p Mask is ignored.
static Value *joinVectorsWithMask(ArrayRef<Value *> VectorsToJoin,
                                  Value *SourceValue, Value *Mask,
                                  IRBuilder<> &Builder, const Twine &Name) {
  Value *Result = joinVectors(VectorsToJoin, Builder, Name);
  if (SourceValue && Mask) {
    assert(cast<FixedVectorType>(SourceValue->getType())->getNumElements() ==
               cast<FixedVectorType>(Result->getType())->getNumElements() &&
           cast<FixedVectorType>(Mask->getType())->getNumElements() ==
               cast<FixedVectorType>(Result->getType())->getNumElements() &&
           "Inconsistent vector length");
    assert(cast<FixedVectorType>(SourceValue->getType())->getElementType() ==
               cast<FixedVectorType>(Result->getType())->getElementType() &&
           "Vector element type mismatch");
    Result = Builder.CreateSelect(Mask, Result, SourceValue, "select.merge");
  }
  return Result;
}

Value *MapIntrinToImlImpl::joinSplitCallResults(unsigned NumRet,
                                                ArrayRef<Value *> SplitCalls,
                                                FunctionType *FT,
                                                Value *SourceValue,
                                                Value *Mask) {

  Type *CallType = SplitCalls[0]->getType();
  assert((CallType->isVectorTy() ||
          (CallType->isStructTy() &&
           cast<StructType>(CallType)->getElementType(0)->isVectorTy())) &&
         "Invalid type of result of SVML call");
  assert(
      (!(SourceValue && Mask) ||
       (Mask->getType()->isVectorTy() &&
        cast<VectorType>(Mask->getType())->getElementType()->isIntegerTy(1))) &&
      "Invalid mask type");

  // Again, we need to handle 2 cases: the return value is a vector,
  // or a structure of vectors.
  // If it's a vector, just join them together with shufflevector instruction.
  if (CallType->isVectorTy())
    return joinVectorsWithMask(SplitCalls, SourceValue, Mask, Builder,
                               "shuffle.comb");

  assert(CallType->isStructTy() &&
         "SVML functions only returns vector or struct");

  // For the structure case, individual fields are collected from structures
  // returned by the split calls with extractvalue instructions, then several
  // shufflevector instructions are generated to combine them into one field,
  // which are then populated into the result structure with insertvalue.
  //
  // Take sincos as an example:
  //
  // ; extract and combine sin field of the return values
  // %sin.part1 = extractvalue { <2 x float>, <2 x float> } %call.1, 0
  // %sin.part2 = extractvalue { <2 x float>, <2 x float> } %call.2, 0
  // %sin.combined = shufflevector <2 x float> %sin.part1,
  //                               <2 x float> %sin.part2,
  //                               <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %result.tmp = insertvalue { <4 x float>, <4 x float> } undef,
  //                           <4 x float> %sin.combined, 0
  //
  // ; extract and combine cos field of the return values
  // %cos.part1 = extractvalue { <2 x float>, <2 x float> } %call.1, 1
  // %cos.part2 = extractvalue { <2 x float>, <2 x float> } %call.2, 1
  // %cos.combined = shufflevector <2 x float> %cos.part1,
  //                               <2 x float> %cos.part2,
  //                               <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %result = insertvalue { <4 x float>, <4 x float> } %result.tmp,
  //                       <4 x float> %cos.combined, 1
  StructType *CallStructType = cast<StructType>(CallType);
  Value *CombinedInsert = UndefValue::get(FT->getReturnType());
  for (unsigned I = 0; I < CallStructType->getNumElements(); I++) {
    SmallVector<Value *, 4> Parts;
    for (unsigned J = 0; J < NumRet; J++)
      Parts.push_back(
          Builder.CreateExtractValue(SplitCalls[J], I, "extract.result"));

    Value *SourceField = nullptr;
    if (SourceValue)
      SourceField =
          Builder.CreateExtractValue(SourceValue, I, "extract.source");

    Value *Combined =
        joinVectorsWithMask(Parts, SourceField, Mask, Builder, "shuffle.comb");
    // TODO: In most cases, the fields inserted to the returned structure will
    // soon be extracted again, and the structure is almost never used as a
    // whole. Consider optimizing out the insertvalues used for reconstructing
    // the structure and feed vectors to users directly.
    CombinedInsert =
        Builder.CreateInsertValue(CombinedInsert, Combined, I, "insert.result");
  }
  return CombinedInsert;
}

bool MapIntrinToImlImpl::isLessThanFullVector(Type *ValType, Type *LegalType) {
  if (ValType->isStructTy()) {
    assert(LegalType->isStructTy() &&
           ValType->getStructNumElements() ==
               LegalType->getStructNumElements() &&
           ValType->getStructNumElements() > 0 &&
           "Expect ValType and LegalType to be both structures of vectors");

    StructType *ValStructType = cast<StructType>(ValType);
    StructType *LegalStructType = cast<StructType>(LegalType);

    assert(std::all_of(
               ValStructType->element_begin(), ValStructType->element_end(),
               [ValStructType](Type *ElementTy) {
                 auto *ElementVecTy = dyn_cast<FixedVectorType>(ElementTy);
                 return ElementVecTy &&
                        ElementVecTy->getNumElements() ==
                            cast<FixedVectorType>(
                                ValStructType->getStructElementType(0))
                                ->getNumElements();
               }) &&
           "Expect all struct fields to be vectors of the same length");
    assert(std::all_of(
               LegalStructType->element_begin(), LegalStructType->element_end(),
               [LegalStructType](Type *ElementTy) {
                 auto *ElementVecTy = dyn_cast<FixedVectorType>(ElementTy);
                 return ElementVecTy &&
                        ElementVecTy->getNumElements() ==
                            cast<FixedVectorType>(
                                LegalStructType->getStructElementType(0))
                                ->getNumElements();
               }) &&
           "Expect all struct fields to be vectors of the same length");

    return isLessThanFullVector(ValStructType->getElementType(0),
                                LegalStructType->getElementType(0));
  }

  VectorType *ValVecType = cast<VectorType>(ValType);
  VectorType *LegalVecType = cast<VectorType>(LegalType);

  if (ValVecType->getPrimitiveSizeInBits().getFixedSize() <
      LegalVecType->getPrimitiveSizeInBits().getFixedSize())
    return true;

  return false;
}

void MapIntrinToImlImpl::generateNewArgsFromPartialVectors(
    ArrayRef<Value *> Args, ArrayRef<Type *> NewArgTypes, unsigned TargetVL,
    SmallVectorImpl<Value *> &NewArgs) {

  // This function builds a new argument list for the svml function call by
  // finding any arguments that are less than full vector and duplicating the
  // low order elements into the upper part of a new vector register. If the
  // argument type is already legal, then just insert it as is in the arg list.

  for (unsigned I = 0; I < NewArgTypes.size(); ++I) {

    // Type of the parameter on the math lib call, which can be driven by the
    // user specifying an explicit vectorlength.
    Value *NewArg = Args[I];
    Type *ArgType = NewArg->getType();
    // The type of the parameter if using the full register specified through
    // legalization.
    Type *LegalArgType = NewArgTypes[I];

    // SVML functions accept vector arguments, the new args are created with
    // widenPartialVector().
    // Although masked 512-bit SVML sincos/divrem functions need struct of
    // vector as source, it'll never be widened as we don't have wider functions
    // yet. Masked functions with lower vector width (but has no source
    // argument) may also be widened into 512-bit, in which case the source
    // argument will just be set to undef.
    if (!isLessThanFullVector(ArgType, LegalArgType)) {
      NewArgs.push_back(NewArg);
    } else if (isa<UndefValue>(Args[I])) {
      NewArgs.push_back(UndefValue::get(LegalArgType));
    } else if (auto *VecTy = dyn_cast<FixedVectorType>(ArgType)) {
      unsigned NumElems = VecTy->getNumElements();
      unsigned LegalNumElems =
          cast<FixedVectorType>(LegalArgType)->getNumElements();
      NewArg = replicateVector(NewArg, LegalNumElems / NumElems, Builder,
                               "shuffle.dup");
      NewArgs.push_back(NewArg);
    } else {
      llvm_unreachable("Invalid argument of SVML function call");
    }
  }
}

Value *MapIntrinToImlImpl::extractLowerPart(Value *V, unsigned ExtractingVL,
                                            unsigned SourceVL) {
  assert(SourceVL >= ExtractingVL && (!(SourceVL % ExtractingVL)) &&
         "SourceVL must be multiple of ExtractingVL");
  Type *T = V->getType();
  unsigned NumParts = SourceVL / ExtractingVL;

  if (T->isVectorTy())
    return generateExtractSubVector(V, 0, NumParts, Builder);

  assert(T->isStructTy() &&
         "SVML functions only returns vector or struct");
  Type *ExtractedType = legalizeArgumentOrReturnType(T, SourceVL, ExtractingVL);

  // For functions that return structures, individual fields are extracted
  // respectively:
  //
  // %sin = extractvalue { <8 x float>, <8 x float> } %call, 0
  // %sin.extract = shufflevector <8 x float> %sin, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %result.tmp = insertvalue { <4 x float>, <4 x float> } undef, <4 x float> %sin.extract, 0
  //
  // %cos = extractvalue { <8 x float>, <8 x float> } %call, 1
  // %cos.extract = shufflevector <8 x float> %cos, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  // %result = insertvalue { <4 x float>, <4 x float> } %result.tmp, <4 x float> %cos.extract, 1
  Value *Extracted = UndefValue::get(ExtractedType);
  for (unsigned I = 0; I < ExtractedType->getStructNumElements(); ++I) {
    Value *ExtractInst = Builder.CreateExtractValue(V, {I}, "extract.result");
    Value *ExtractedField =
        generateExtractSubVector(ExtractInst, 0, NumParts, Builder);
    Extracted = Builder.CreateInsertValue(Extracted, ExtractedField, {I},
                                          "insert.result");
  }
  return Extracted;
}

void MapIntrinToImlImpl::scalarizeVectorCall(CallInst *CI,
                                             StringRef ScalarFuncName,
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

  LLVM_DEBUG(dbgs() << "Scalarizing call to '"
                    << CI->getCalledFunction()->getName() << "' with '"
                    << ScalarFuncName << "'");

  FunctionType *FT = FunctionType::get(RetType, FTArgTypes, false);
  FunctionCallee FCache = M->getOrInsertFunction(ScalarFuncName, FT);
  SmallVector<Value *, 4> CallResults;

  for (unsigned I = 0; I < LogicalVL; ++I) {
    SmallVector<Value *, 4> ScalarCallArgs;

    for (unsigned J = 0; J < CI->arg_size(); ++J) {
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

StringRef MapIntrinToImlImpl::findX86SVMLVariantForScalarFunction(
    StringRef ScalarFuncName, unsigned TargetVL, bool Masked, Instruction *I) {

  std::string TargetVLString = std::to_string(TargetVL);

  // Use the generic parent function name emitted by the vectorizer to lookup
  // the proper variant. E.g., "__svml_sinf4" -> "__svml_sinf4_ha",
  // "__svml_sinf4" -> "__svml_sinf4_ep". The selection process uses the IMF
  // attributes to find the appropriate variant. The function name used for
  // the lookup is based on the legal target vector length. This is important
  // to remember since the input function (FuncName) could be a logical vector
  // that is larger.
  std::string TargetVLStr = toString(APInt(32, TargetVL), 10, false);

  std::string VectorFuncStem = ScalarFuncName.str();
  // Use "s" suffix for FP16 functions
  if (ScalarFuncName.endswith("f16"))
    VectorFuncStem.replace(VectorFuncStem.end() - 3, VectorFuncStem.end(), "s");
  std::string TempFuncName = "__svml_" + VectorFuncStem + TargetVLStr;
  if (Masked)
    TempFuncName += "_mask";
  char *ParentFuncName = new char[TempFuncName.size() + 1];
  std::strcpy(ParentFuncName, TempFuncName.c_str());

  LLVM_DEBUG(dbgs() << "Legal Function: " << TempFuncName << "\n");

  ImfAttr *AttrList = nullptr;
  createImfAttributeList(I, &AttrList);

  // External libiml_attr interface that returns the SVML/libm variant if the
  // parent function and IMF attributes match. Return NULL otherwise.
  Triple T(M->getTargetTriple());
  const char *VariantFuncName = get_library_function_name(
      ParentFuncName, AttrList, T.getArch(), T.getOS());

  // No longer need the IMF attribute list at this point, so free up the memory.
  // Note: this does not remove the attributes from the instruction, only the
  // internal data structure used to query the iml interface.
  deleteAttributeList(&AttrList);
  delete[] ParentFuncName;

  return VariantFuncName;
}

std::string MapIntrinToImlImpl::getSVMLFunctionProperties(
    StringRef FuncName, VectorType *VecCallType, unsigned &LogicalVL,
    bool &Masked) {

  assert(!Masked && "Expect Masked to be false");
  // Incoming FuncName is something like '__svml_sinf4'. Removing the '__svml_'
  // prefix and logical vl from the end yields the scalar lib name.
  StringRef Prefix = "__svml_";
  StringRef ScalarFuncName = FuncName.substr(Prefix.size());
  if (ScalarFuncName.endswith("_mask")) {
    Masked = true;
    ScalarFuncName = ScalarFuncName.rtrim("_mask");
  }

  unsigned ReturnVL = cast<FixedVectorType>(VecCallType)->getNumElements();
  std::string ReturnVLStr = toString(APInt(32, ReturnVL), 10, false);
  LogicalVL = ReturnVL;
  StringRef LogicalVLStr = ReturnVLStr;

  // If the SVML function name ends with half of VL of it's return type, then
  // it's a complex function.
  if (ReturnVL >= 2) {
    std::string HalfReturnVLStr = std::to_string(ReturnVL / 2);
    assert(ScalarFuncName.endswith(ReturnVLStr) ^
               ScalarFuncName.endswith(HalfReturnVLStr) &&
           "Can't identify whether an SVML function is a complex function");
    if (ScalarFuncName.endswith(HalfReturnVLStr)) {
      assert(ScalarFuncName[0] == 'c' &&
             "SVML functions operating on complex numbers must have names "
             "starting with 'c'");
      LogicalVL = ReturnVL / 2;
      LogicalVLStr = HalfReturnVLStr;
    }
  }
  ScalarFuncName = ScalarFuncName.drop_back(LogicalVLStr.size());

  // Use "f16" suffix for FP16 scalar functions
  if (VecCallType->getElementType() ==
      Type::getHalfTy(VecCallType->getContext())) {
    assert(ScalarFuncName.back() == 's' &&
           "Name of FP16 vector function should ends with 's'.");
    ScalarFuncName = ScalarFuncName.drop_back(1);
    return ScalarFuncName.str() + "f16";
  }

  return ScalarFuncName.str();
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

  std::string Result =
      (Opcode == Instruction::UDiv || Opcode == Instruction::URem) ? "u" : "i";

  if (ScalarSize != 32)
    Result += std::to_string(ScalarSize);

  Result += (Opcode == Instruction::UDiv || Opcode == Instruction::SDiv)
                ? "div"
                : "rem";

  return Result;
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
      auto *VecTy = dyn_cast<FixedVectorType>(I.getType());
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

      VectorType *LegalVecTy =
          FixedVectorType::get(VecTy->getScalarType(), TargetVL);
      // Find an appropriate SVML function to call. If there is none, this
      // instruction will not be optimized to SVML.
      std::string FuncName = getSVMLIDivOrRemFuncName(
          BinOp->getOpcode(), cast<VectorType>(LegalVecTy));
      StringRef VariantFuncName =
          findX86SVMLVariantForScalarFunction(FuncName, TargetVL, false, &I);
      if (VariantFuncName.empty())
        continue;

      FunctionCallee Func = M->getOrInsertFunction(VariantFuncName, LegalVecTy,
                                                   LegalVecTy, LegalVecTy);

      Value *Result = BinOp;
      SmallVector<Value *, 2> Args(BinOp->operands());
      Builder.SetInsertPoint(BinOp);

      if (NumRet > 1) {
        // NumRet > 1 means that multiple library calls are required to
        // support the vector length of the integer division instruction.

        // Generate SVML call for each part of the split operands
        SmallVector<Value *, 8> SplitCalls;
        splitMathLibCalls(NumRet, TargetVL, Func, Args, SplitCalls);

        Result = joinVectors(SplitCalls, Builder, "shuffle.comb");
      } else {
        // generateNewArgsFromPartialVectors() duplicates the low elements
        // into the upper part of the vector so the math library call operates
        // on safe values. But, the duplicate results are not needed, so
        // shuffle out the ones we want and then replace the users of the
        // function call with the shuffle. This work is done via
        // extractElemsFromVector().
        SmallVector<Type *, 2> NewArgTypes{LegalVecTy, LegalVecTy};
        SmallVector<Value *, 2> NewArgs;
        generateNewArgsFromPartialVectors(Args, NewArgTypes, TargetVL, NewArgs);

        CallInst *NewCI = createSVMLCall(Func, NewArgs, "vcall");
        Result = NewCI;

        bool LessThanFullVector = isLessThanFullVector(VecTy, LegalVecTy);
        if (LessThanFullVector) {
          // Extract the number of elements specified by the partial vector
          // return type of the call (indicated by LogicalVL). The NewCI
          // return type will indicate the size of the vector extracted from.
          // Start extracting from position 0.
          Result = extractLowerPart(NewCI, LogicalVL, TargetVL);
        }
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

void MapIntrinToImlImpl::legalizeAVX512MaskArgs(
    CallInst *CI, SmallVectorImpl<Value *> &Args, Value *MaskValue,
    unsigned LogicalVL, unsigned TargetVL, unsigned ComponentBitWidth) {
  if (TargetVL < LogicalVL) {
    assert(LogicalVL * ComponentBitWidth >= 512 &&
           "Expecting source to be more than 512-bit wide");
    // If we are splitting a call to masked 512-bit SVML function, create
    // a new mask parameter and get rid of the source parameter which is
    // absent in non-512bit SVML functions.
    IntegerType *NewMaskElementType =
        IntegerType::getIntNTy(CI->getContext(), ComponentBitWidth);
    VectorType *NewMaskType = FixedVectorType::get(NewMaskElementType, LogicalVL);

    Constant *Zeros = ConstantAggregateZero::get(NewMaskType);
    Constant *Ones =
        ConstantVector::getSplat(ElementCount::getFixed(LogicalVL),
                                 ConstantInt::get(NewMaskElementType, -1));
    Value *NewMask =
        Builder.CreateSelect(MaskValue, Ones, Zeros, "select.maskcvt");

    assert(Args.size() > 2 && "Too few arguments in SVML call");
    assert(Args[0]->getType() == CI->getType() && "Invalid source argument");
    assert(Args[1]->getType() ==
               FixedVectorType::get(IntegerType::getInt1Ty(CI->getContext()),
                               LogicalVL) &&
           "Invalid mask argument");

    Args.erase(Args.begin(), Args.begin() + 2);
    Args.push_back(NewMask);
  } else if (TargetVL > LogicalVL) {
    assert(TargetVL * ComponentBitWidth == 512 &&
           "Expecting target to be 512-bit wide");
    // On the other hand if we are widening a masked non-512-bit SVML
    // function call to 512-bit, convert and reposition the mask
    // parameter and create a new source parameter.
    VectorType *OldMaskType = cast<VectorType>(MaskValue->getType());
    Constant *Splat = ConstantVector::getSplat(
        ElementCount::getFixed(LogicalVL),
        ConstantInt::get(OldMaskType->getElementType(), -1));
    Value *NewMask = Builder.CreateICmpEQ(MaskValue, Splat, "icmp.maskcvt");

    Type *CallType = CI->getType();
    Value *Source = UndefValue::get(CallType);

    assert(Args.size() >= 2 && "Too few arguments in SVML call");
    assert(Args.back()->getType() ==
               FixedVectorType::get(
                   IntegerType::getIntNTy(CI->getContext(), ComponentBitWidth),
                   LogicalVL) &&
           "Invalid mask argument");

    Args.pop_back();
    Args.insert(Args.begin(), NewMask);
    Args.insert(Args.begin(), Source);
  }
}

CallInst *MapIntrinToImlImpl::createSVMLCall(FunctionCallee Callee,
                                             ArrayRef<Value *> Args,
                                             const Twine &Name) {
  CallInst *NewCI = Builder.CreateCall(Callee, Args, Name);
  StringRef FunctionName = cast<Function>(Callee.getCallee())->getName();
  Optional<CallingConv::ID> UnifiedCC =
      getSVMLCallingConvByNameAndType(FunctionName, Callee.getFunctionType());
  CallingConv::ID CC =
      getLegacyCSVMLCallingConvFromUnified(UnifiedCC.value());
  // Release YMM16-31 as callee-saved registers for 256-bit SVML function calls
  // statically dispatched to AVX2 implementation.
  if (CC == CallingConv::SVML_AVX &&
      (FunctionName.endswith("_l9") || FunctionName.endswith("_e9")))
    CC = CallingConv::SVML_AVX_AVX_Impl;
  NewCI->setCallingConv(CC);
  return NewCI;
}

// Get the corresponding base C function name of an LLVM math function
// intrinsic. Return empty string if it's not an eligible intrinsic.
static StringRef getIntrinsicBaseName(Intrinsic::ID IID) {
  switch (IID) {
  case Intrinsic::sin:
    return "sin";
  case Intrinsic::cos:
    return "cos";
  case Intrinsic::sqrt:
    return "sqrt";
  case Intrinsic::exp:
    return "exp";
  case Intrinsic::exp2:
    return "exp2";
  case Intrinsic::log:
    return "log";
  case Intrinsic::log2:
    return "log2";
  case Intrinsic::log10:
    return "log10";
  case Intrinsic::pow:
    return "pow";
  case Intrinsic::powi:
    return "powi";
  case Intrinsic::floor:
    return "floor";
  case Intrinsic::ceil:
    return "ceil";
  case Intrinsic::trunc:
    return "trunc";
  case Intrinsic::rint:
    return "rint";
  case Intrinsic::nearbyint:
    return "nearbyint";
  case Intrinsic::round:
    return "round";
  case Intrinsic::lround:
    return "lround";
  case Intrinsic::llround:
    return "llround";
  case Intrinsic::lrint:
    return "lrint";
  case Intrinsic::llrint:
    return "llrint";
  case Intrinsic::fabs:
    return "fabs";
  case Intrinsic::copysign:
    return "copysign";
  case Intrinsic::ldexp:
    return "ldexp";
  default:
    return StringRef();
  }
}

// Determine whether there is a corresponding function in libimf for the LLVM
// intrinsic specified by \p IID and \p T. Only floating-point intrinsics are
// supported.
static bool isIMFScalarFPIntrinsic(Intrinsic::ID IID, Type *T) {
  if (!T->isFloatTy() && !T->isDoubleTy() && !T->isHalfTy())
    return false;
  if (getIntrinsicBaseName(IID).empty())
    return false;
  return true;
}

// Get the standard C function name for a scalar LLVM floating-point intrinsic.
// The intrinsic must satisfy isIMFScalarFPIntrinsic(IID, T) == true.
static std::string scalarFPIntrinsicToFuncName(Intrinsic::ID IID, Type *T) {
  assert(isIMFScalarFPIntrinsic(IID, T) && "Unsupported intrinsic");
  std::string Result = getIntrinsicBaseName(IID).str();
  if (T->isFloatTy())
    Result += "f";
  else if (T->isHalfTy())
    Result += "f16";
  return Result;
}

bool MapIntrinToIml::runOnFunction(Function &F) {
  auto *TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
  auto *TLI = &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  return MapIntrinToImlImpl(F, TTI, TLI).runImpl();
}

PreservedAnalyses MapIntrinToImlPass::run(Function &F,
                                          FunctionAnalysisManager &AM) {
  auto *TTI = &AM.getResult<TargetIRAnalysis>(F);
  auto *TLI = &AM.getResult<TargetLibraryAnalysis>(F);
  if (!MapIntrinToImlImpl(F, TTI, TLI).runImpl())
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

MapIntrinToImlImpl::MapIntrinToImlImpl(Function &F, TargetTransformInfo *TTI,
                                       TargetLibraryInfo *TLI)
    : M(F.getParent()), Func(&F), TTI(TTI), TLI(TLI), Builder(F.getContext()) {}

bool MapIntrinToImlImpl::runImpl() {
  LLVM_DEBUG(dbgs() << "\nExecuting MapIntrinToIml ...\n\n");
  if (RunSvmlStressMode) {
    LLVM_DEBUG(dbgs() << "Stress Testing Mode Invoked - svml calls will be "
                         "scalarized\n");
  }

  const DataLayout DL = M->getDataLayout();

  Triple T(M->getTargetTriple());
  llvm::Triple::ArchType Arch = T.getArch();
  bool X86Target = (Arch == Triple::x86 || Arch == Triple::x86_64);

  // Will be populated with the call instructions that will be replaced with
  // legalized/refined svml calls.
  SmallVector<Instruction *, 4> InstToRemove;

  // Keep track of calls that have already been translated so we don't consider
  // them again.
  SmallVector<CallInst *, 4> InstToTranslate;

  // Keep track of scalar math function calls to translate
  SmallVector<CallInst *, 4> ScalarCallsToTranslate;

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
  bool isOCL = M->getNamedMetadata("opencl.ocl.version") != nullptr;
  if (!isOCL && TLI->isSVMLEnabled()) // INTEL_CUSTOMIZATION
    Dirty |= replaceVectorIDivAndRemWithSVMLCall(TTI, *Func);

  // Begin searching for calls that are candidates for legalization and/or
  // refinement based on IMF attributes attached to the call.
  for (inst_iterator Inst = inst_begin(*Func), InstEnd = inst_end(*Func);
       Inst != InstEnd; ++Inst) {

    // Find incoming svml calls and insert them into a map. These are candidates
    // for legalization and application of IMF attributes. This check can later
    // be extended to include scalar (libm) candidates.
    CallInst *CI = dyn_cast<CallInst>(&*Inst);
    if (CI && CI->getCalledFunction()) {
      StringRef FuncName = CI->getCalledFunction()->getName();
      // Only transform functions with SVML naming scheme if SVML is enabled.
      if (FuncName.startswith("__svml")) {
        if (TLI->isSVMLEnabled() &&
            getVectorTypeForCSVMLFunction(CI->getFunctionType()))
          InstToTranslate.push_back(CI);
      } else if (is_libm_function(FuncName.str().c_str())) {
        ScalarCallsToTranslate.push_back(CI);
      } else if (CI->getCalledFunction()->isIntrinsic()) {
        Intrinsic::ID IID = CI->getIntrinsicID();
        // Some FP functions like lround and llround have integer return types.
        // We must use type of operand to determine whether it's an FP
        // intrinsics.
        if (CI->getNumOperands() >= 1 &&
            isIMFScalarFPIntrinsic(IID, CI->getOperand(0)->getType()))
          ScalarCallsToTranslate.push_back(CI);
      }
    }
  }

  for (auto *CI : InstToTranslate) {
    LLVM_DEBUG(dbgs() << "Call Inst: " << *CI << "\n");
    Builder.SetInsertPoint(CI);

    StringRef FuncName = CI->getCalledFunction()->getName();

    // The function call is only inserted into the candidates map
    // (InstToTranslate) when it is known the return/arguments are explicitly
    // vector typed. Thus, here it is known that
    // getVectorTypeForSVMLFunction(FunctionType) will return a vector type.
    VectorType *VecCallType =
        getVectorTypeForCSVMLFunction(CI->getFunctionType());

    Type *ElemType = VecCallType->getElementType();
    unsigned LogicalVL = 0;
    bool Masked = false;
    std::string ScalarFuncName =
        getSVMLFunctionProperties(FuncName, VecCallType, LogicalVL, Masked);

    // Need to go through DataLayout in cases where we have pointer types to
    // get the correct pointer bit size.
    unsigned ScalarBitWidth = DL.getTypeSizeInBits(ElemType);
    unsigned ComponentBitWidth =
        (cast<FixedVectorType>(VecCallType)->getNumElements() / LogicalVL) *
        ScalarBitWidth;

    // Get the number of library calls that will be required, indicated by
    // NumRet. NumRet is determined by getting the vector type info from the
    // call signature. See notes in getVectorTypeForSVMLFunction() for more
    // information.
    unsigned TargetVL = 0;
    unsigned NumRet =
        calculateNumReturns(TTI, ComponentBitWidth, LogicalVL, &TargetVL);

    StringRef VariantFuncName;

    if (X86Target) {
      VariantFuncName = findX86SVMLVariantForScalarFunction(
          ScalarFuncName, TargetVL, Masked, CI);
    }

    // Preserve fast math flag of the original call (if any)
    IRBuilder<>::FastMathFlagGuard FMFGuard(Builder);
    if (isa<FPMathOperator>(CI))
      Builder.setFastMathFlags(CI->getFastMathFlags());

    // An alternate math library function was found for the original call, so
    // replace the original call with the new call. Set Dirty to true since
    // the LLVM IR is modified. Scalarize the call when:
    // 1) an appropriate math library function is not found through querying
    //    the function selection interface.
    // 2) the pass is running in stress testing mode.
    if (!VariantFuncName.empty() && !RunSvmlStressMode) {
      LLVM_DEBUG(dbgs() << "Function Variant: " << VariantFuncName << "\n\n");

      // Original arguments to the vector call.
      SmallVector<Value *, 8> Args(CI->arg_begin(), CI->arg_end());

      // Masked SVML functions with 512-bit VL differs from other functions
      // in position and type of mask argument, they also have a source
      // argument. Such calls require special treatment when splitting.
      bool CallIsAVX512 = (LogicalVL * ComponentBitWidth) >= 512;
      bool NewCallIsAVX512 = (TargetVL * ComponentBitWidth) == 512;
      Value *SourceArg = nullptr;
      Value *MaskArg = nullptr;
      if (Masked) {
        SourceArg = CallIsAVX512 ? Args[0] : nullptr;
        MaskArg = CallIsAVX512 ? Args[1] : Args.back();
        if (CallIsAVX512 ^ NewCallIsAVX512)
          legalizeAVX512MaskArgs(CI, Args, MaskArg, LogicalVL, TargetVL,
                                 ComponentBitWidth);
      }

      // FT will point to the legal FunctionType based on target register
      // size requirements. This FunctionType is used to create the call
      // to the svml function.
      FunctionType *FT = legalizeFunctionTypes(CI->getFunctionType(), Args,
                                               LogicalVL, TargetVL, FuncName);
      FunctionCallee FCache = M->getOrInsertFunction(VariantFuncName, FT);

      // SplitCalls contains the instructions that are the results of math
      // library calls. This could be the call instructions themselves, or
      // the results of shuffles for cases like calls to sincos, or calls to
      // functions that are less than full vector.
      SmallVector<Value *, 8> SplitCalls;

      if (NumRet > 1) {

        // NumRet > 1 means that multiple library calls are required to
        // support the vector length of the call.

        splitMathLibCalls(NumRet, TargetVL, FCache, Args, SplitCalls);

        // If the split calls are 512-bit (i.e. it has source argument), then
        // the merge in final join is redundant.
        Value *SourceArgForJoin = NewCallIsAVX512 ? nullptr : SourceArg;
        Value *FinalResult =
            joinSplitCallResults(NumRet, SplitCalls, CI->getFunctionType(),
                                 SourceArgForJoin, MaskArg);

        CI->replaceAllUsesWith(FinalResult);
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
        generateNewArgsFromPartialVectors(Args, FT->params(), TargetVL, NewArgs);

        CallInst *NewCI = createSVMLCall(FCache, NewArgs, "vcall");
        Value *CallResult = NewCI;

        bool LessThanFullVector = isLessThanFullVector(
            CI->getFunctionType()->getReturnType(), FT->getReturnType());

        if (LessThanFullVector) {
          // Extract the number of elements specified by the partial vector
          // return type of the call (indicated by LogicalVL). The NewCI
          // return type will indicate the size of the vector extracted from.
          // Start extracting from position 0.
          CallResult = extractLowerPart(NewCI, LogicalVL, TargetVL);
        }

        CI->replaceAllUsesWith(CallResult);
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

      scalarizeVectorCall(CI, ScalarFuncName, LogicalVL, ElemType);
      InstToRemove.push_back(CI);
      Dirty = true; // LLVM-IR has been changed because the original vector call
                    // was replaced with scalar calls.

      LLVM_DEBUG(dbgs() << "\n\n");
    }
  }

  // Legalize scalar math function calls
  for (auto *ScalarCI : ScalarCallsToTranslate) {
    LLVM_DEBUG(dbgs() << "ScalarCI: "; ScalarCI->dump());
    if (X86Target) {
      // TODO: Can scalar math functions have any prefixes/suffixes?
      Function *F = ScalarCI->getCalledFunction();
      std::string ScalarFuncName;
      assert(ScalarCI->getNumOperands() >= 1 &&
             "Math function should have at least 1 argument");
      if (F->isIntrinsic())
        ScalarFuncName = scalarFPIntrinsicToFuncName(
            F->getIntrinsicID(), ScalarCI->getOperand(0)->getType());
      else
        ScalarFuncName = F->getName().str();

      ImfAttr *AttrList = nullptr;
      createImfAttributeList(ScalarCI, &AttrList);

      StringRef VariantFuncName = ScalarFuncName;

      // If iml accuracy interface returns a non-null variant then replace
      // current scalar function with variant function name.
      if (const char *VariantFuncStr = get_library_function_name(
              ScalarFuncName.c_str(), AttrList, Arch, T.getOS())) {
        VariantFuncName = StringRef(VariantFuncStr);
      }

      deleteAttributeList(&AttrList);

      // Don't perform replacement if variant is same as scalar function
      if (VariantFuncName.equals(ScalarFuncName))
        continue;
      if (VariantFuncName.startswith("__svml"))
        ScalarCI->setCallingConv(CallingConv::SVML);

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
