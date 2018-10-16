//===------DTransAnnotator.cpp - Annotation utilities for DTrans ----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
// Utility routines for getting/setting DTrans annotations used to convey
// information from one transformation back to the analysis or to another
// transformation. (metadata or intrinsics)
///
// ===--------------------------------------------------------------------=== //
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace dtrans {

namespace {
constexpr const char *MetadataNames[] = {
    /*DMD_DTransType=*/"dtrans-type"};

static_assert(sizeof(MetadataNames) / sizeof(char *) ==
                  DTransAnnotator::DMD_Last,
              "Missing metadata name");

constexpr const char *AnnotNames[] = {
    /*DPA_AOSToSOAAllocation=*/"__intel_dtrans_aostosoa_alloc",
    /*DPA_AOSToSOAIndex=*/"__intel_dtrans_aostosoa_index"};

static_assert(sizeof(AnnotNames) / sizeof(char *) == DTransAnnotator::DPA_Last,
              "Missing annotation name");
} // end anonymous namespace

// Annotate an instruction to indicate that the value should be treated as a
// specific type for DTrans. Currently, this attaches metadata to the
// instruction, but could be changed to insert an intrinsic in the IR in the
// future. We currently limit this to holding a single type.
//
// The format of the metadata is to store a null value of the specified type
// as follows:
//   { Ty null }
//
// The use of a null value of the type enables the type to be kept up-to-date
// when DTrans transformations run because when the instruction referencing the
// metadata is remapped, the type within the metadata will be remapped as well,
// if the type changes.
void DTransAnnotator::createDTransTypeAnnotation(Instruction *I,
                                                 llvm::Type *Ty) {
  assert(Ty->isPointerTy() && "Annotation type must be pointer type");
  assert(I->getMetadata(MetadataNames[DMD_DTransType]) == nullptr &&
         "Only a single dtrans type metadata attachment allowed.");

  LLVMContext &Ctx = I->getContext();
  MDNode *MD =
      MDNode::get(Ctx, {ConstantAsMetadata::get(Constant::getNullValue(Ty))});
  I->setMetadata(MetadataNames[DMD_DTransType], MD);
}

void DTransAnnotator::removeDTransTypeAnnotation(Instruction *I) {
  I->setMetadata(MetadataNames[DMD_DTransType], nullptr);
}

// Get the type that exists in an annotation, if one exists, for the
// instruction.
llvm::Type *DTransAnnotator::lookupDTransTypeAnnotation(Instruction *I) {
  auto *MD = I->getMetadata(MetadataNames[DMD_DTransType]);
  if (!MD)
    return nullptr;

  assert(MD->getNumOperands() == 1 && "Unexpected metadata operand count");
  auto &MDOpp1 = MD->getOperand(0);
  auto *TyMD = dyn_cast<ConstantAsMetadata>(MDOpp1);
  if (!TyMD)
    return nullptr;

  return TyMD->getType();
}

GlobalVariable &DTransAnnotator::getAnnotationVariable(Module &M,
                                                       DPA_AnnotKind Type,
                                                       StringRef AnnotContent,
                                                       StringRef Extension) {
  std::string Name(AnnotNames[Type]);
  if (!Extension.empty())
    Name += "." + Extension.str();

  GlobalVariable *Var = M.getGlobalVariable(Name, /*AllowInternal=*/true);
  if (!Var)
    Var = &createGlobalVariableString(M, Name, AnnotContent);
  else
    assert(AnnotContent ==
               cast<ConstantDataArray>(Var->getInitializer())->getAsCString() &&
           "Annotation string does not match existing value");

  return *Var;
}

Instruction *DTransAnnotator::createPtrAnnotation(
    Module &M, Value &Ptr, Value &AnnotVal, Value &FileNameVal,
    unsigned LineNum, const Twine &NameStr, Instruction *InsertBefore) {
  auto *Intrin =
      Intrinsic::getDeclaration(&M, Intrinsic::ptr_annotation, Ptr.getType());
  Value *Args[] = {&Ptr, &AnnotVal, &FileNameVal,
                   ConstantInt::get(Type::getInt32Ty(M.getContext()), LineNum)};
  Instruction *Call = CallInst::Create(Intrin, Args, NameStr, InsertBefore);
  return Call;
}

bool DTransAnnotator::isDTransPtrAnnotation(Instruction &I) {
  return getDTransPtrAnnotationKind(I) != DPA_Last;
}

bool DTransAnnotator::isDTransAnnotationOfType(
    Instruction &I, DTransAnnotator::DPA_AnnotKind DPAType) {
  return getDTransPtrAnnotationKind(I) == DPAType;
}

DTransAnnotator::DPA_AnnotKind
DTransAnnotator::getDTransPtrAnnotationKind(Instruction &I) {
  auto *II = dyn_cast<IntrinsicInst>(&I);
  if (!II)
    return DPA_Last;

  if (II->getIntrinsicID() != Intrinsic::ptr_annotation)
    return DPA_Last;

  if (auto *O = dyn_cast<ConstantExpr>(II->getArgOperand(1)))
    if (auto *G = dyn_cast<GlobalVariable>(O->getOperand(0)))
      return lookupDTransAnnotationVariable(G);

  return DPA_Last;
}

DTransAnnotator::DPA_AnnotKind
DTransAnnotator::lookupDTransAnnotationVariable(GlobalVariable *GV) {
  for (unsigned i = 0; i < DPA_Last; ++i) {
    StringRef VarName = GV->getName();
    auto Tmp = VarName.rsplit(".");
    if (Tmp.first == AnnotNames[i])
      return static_cast<DPA_AnnotKind>(i);
  }
  return DPA_Last;
}

GlobalVariable &DTransAnnotator::createGlobalVariableString(Module &M,
                                                            StringRef Name,
                                                            StringRef Str) {
  Constant *StrConstant = ConstantDataArray::getString(M.getContext(), Str);
  auto *GV = new GlobalVariable(M, StrConstant->getType(), /*Constant=*/true,
                                GlobalValue::PrivateLinkage, StrConstant, Name);
  return *GV;
}

Value *DTransAnnotator::createConstantStringGEP(GlobalVariable &GV,
                                                unsigned ByteOffset) {
  Constant *Zero = ConstantInt::get(Type::getInt32Ty(GV.getContext()), 0);
  Constant *Offset =
      ConstantInt::get(Type::getInt32Ty(GV.getContext()), ByteOffset);
  Constant *Indices[] = {Zero, Offset};
  Constant *GEP =
      ConstantExpr::getInBoundsGetElementPtr(GV.getValueType(), &GV, Indices);
  return GEP;
}

} // namespace dtrans
} // namespace llvm
