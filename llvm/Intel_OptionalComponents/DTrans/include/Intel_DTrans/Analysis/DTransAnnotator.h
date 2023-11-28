//===----------- DTransAnnotator.h - Interfaces for DTrans annotations-----===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// Interface routines for getting/setting DTrans annotations used to convey
// information from one transformation back to the analysis or to another
// transformation. (metadata or intrinsics)
//
// ===--------------------------------------------------------------------=== //

#if !INTEL_FEATURE_SW_DTRANS
#error DTransAnnot.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif
#ifndef INTEL_DTRANS_ANALYSIS_DTRANSANNOTOR_H
#define INTEL_DTRANS_ANALYSIS_DTRANSANNOTOR_H

#include "llvm/ADT/Twine.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"

namespace llvm {

class Module;

namespace dtrans {

// Annotator for DTrans transformations. All annotations that are to be
// recognized for cleanup by the AnnotCleaner are defined within this pass to
// encapsulate them in a single location to enable recognition.
//
// This class encapsulates the behavior for both metadata annotations and
// annotation intrinsic function calls.
//
class DTransAnnotator {
public:
  // Enumeration of supported DTrans metadata types
  enum DMD_MetadataKind {
    // Marks the type of pointer an instruction should be considered as
    // producing for the DTrans local pointer analyzer.
    DMD_DTransType,

    // Marks a function as having been transformed to use an array of
    // structures in place of a structure of arrays (SOA-to-AOS)
    DMD_DTransSOAToAOS,

    // Indicates that the function is created by SOAToAOSPrepare to
    // do vector functionalities like SetElem/CCtor.
    DMD_DTransSOAToAOSPrepare,

    // End of list marker.
    DMD_Last
  };

  // Enumeration of supported DTrans annotation types
  enum DPA_AnnotKind {
    // Annotation type used to indicate the allocation call result is the memory
    // block for the structure of arrays produced by AOS-to-SOA.
    DPA_AOSToSOAAllocation,

    // Annotation type used to indicate the pointer is a pointer to a
    // peeling index produced by the AOS-to-SOA transformation.
    DPA_AOSToSOAIndex,

    // End of list marker.
    DPA_Last
  };

  /////////////////////////////////////
  // Metadata based annotating routines
  /////////////////////////////////////

  // Annotate an instruction to provide a pointer type that the result of the
  // instruction aliases. This method can be used by a DTrans transformation for
  // cases where the generated IR is unable to be analyzed directly by the
  // DTransAnalysis.
  // 'Ty' cannot be a pointer type. For pointer types, pass 'Ty' as the Type
  // pointed to, and 'PtrLevel' as the number of levels of pointer indirection.
  static void createDTransTypeAnnotation(Instruction &I, llvm::Type *Ty,
                                         unsigned PtrLevel);

  // Get the type and level of pointer indirection that exists in an annotation,
  // if one exists, for the instruction.
  static std::optional<std::pair<llvm::Type *, unsigned>>
  lookupDTransTypeAnnotation(Instruction &I);

  // Remove a type annotation from the instruction. Return 'true' if the
  // instruction is changed.
  static bool removeDTransTypeAnnotation(Instruction &I);

  // Annotate a function as having been transformed by the DTrans SOA-to-AOS
  // transformation with the data type used.
  // 'Ty' cannot be a pointer type. For pointer types, pass 'Ty' as the Type
  // pointed to, and 'PtrLevel' as the number of levels of pointer indirection.
  static void createDTransSOAToAOSTypeAnnotation(Function &F, llvm::Type *Ty,
                                                 unsigned PtrLevel);

  // Get the SOA-to-AOS transformation type and level of pointer indirection for
  // the Function, if one exists. Otherwise, nullptr
  static std::optional<std::pair<llvm::Type *, unsigned>>
  lookupDTransSOAToAOSTypeAnnotation(Function &F);

  // Return 'true' if the SOA-to-AOS transformation annotation is on the
  // Function.
  static bool hasDTransSOAToAOSTypeAnnotation(Function &F);

  // Remove the SOA-to-AOS transformation annotation marker from the Function.
  // Return 'true' if the metadata is changed.
  static bool removeDTransSOAToAOSTypeAnnotation(Function &F);

  // Annotate a function as having been created by the SOAToAOSPrepare
  // transformation with the data type used.
  // 'Ty' cannot be a pointer type. For pointer types, pass 'Ty' as the Type
  // pointed to, and 'PtrLevel' as the number of levels of pointer indirection.
  static void createDTransSOAToAOSPrepareTypeAnnotation(Function &F,
                                                        llvm::Type *Ty,
                                                        unsigned PtrLevel);

  // Get the type and level of pointer indirection for the Function that is
  // created by SOAToAOSPrepare, if one exists. Otherwise, nullptr.
  static std::optional<std::pair<llvm::Type *, unsigned>>
  lookupDTransSOAToAOSPrepareTypeAnnotation(Function &F);

  // Return 'true' if the SOAToAOSPrepare transformation annotation is on the
  // Function.
  static bool hasDTransSOAToAOSPrepareTypeAnnotation(Function &F);

  // Remove the SOAToAOSPrepare transformation annotation marker from the
  // Function. Return 'true' if the metadata is changed.
  static bool removeDTransSOAToAOSPrepareTypeAnnotation(Function &F);

  /////////////////////////////////////////////////
  // Annotation intrinsic based annotating routines
  /////////////////////////////////////////////////

  // Get a GlobalVariable for a specific DTrans annotation type. An optional
  // extension string can be used to enable getting a class of variables with a
  // common base name, but using different annotation string values.
  static GlobalVariable &getAnnotationVariable(Module &M, DPA_AnnotKind Type,
                                               StringRef AnnotContent,
                                               StringRef Extension = "");

  // Create a llvm.ptr.annotation call using the input parameters to populate
  // the call arguments as:
  //    llvm.ptr.annoation(Ptr, AnnotVal, FilenameVal, LineNum)
  //
  // \p NameStr can be provided to name the result of the CallInst.
  // \p InsertBefore can be provided to have the inserted at the
  //    specified location. Otherwise, the instruction is created, but not
  //    inserted.
  static Instruction *createPtrAnnotation(Module &M, Value &Ptr,
                                          Value &AnnotVal, Value &FileNameVal,
                                          unsigned LineNum,
                                          const Twine &NameStr = "",
                                          Instruction *InsertBefore = nullptr);

  // Utility function to create an internal global variable initialized with a
  // string constant.
  static GlobalVariable &createGlobalVariableString(Module &M, StringRef Name,
                                                    StringRef Str);

  // Utility function to create a GEP for a string created with
  // createGlobalVariableString of the form:
  //   getelementptr @GV, i32 0, i32 %ByteOffset
  static Value *createConstantStringGEP(GlobalVariable &GV,
                                        unsigned ByteOffset);

  // Return true if the Instruction is a ptr.annotation intrinsic that passes
  // a DTrans variable for the annotation string argument.
  static bool isDTransPtrAnnotation(Instruction &I);

  // Return true if the Instruction is an dtrans annotation corresponding to the
  // specific annotation type.
  static bool isDTransAnnotationOfType(Instruction &I, DPA_AnnotKind DPAType);

  // Get the type of annotation the Instruction corresponds to. Otherwise,
  // return DPA_Last.
  static DPA_AnnotKind getDTransPtrAnnotationKind(Instruction &I);

  // If the GlobalVariable is one of the types managed by this class, return the
  // type of annotation it corresponds to. Otherwise, return DPA_Last.
  static DPA_AnnotKind lookupDTransAnnotationVariable(GlobalVariable *GV);

private:
  // The following template methods provide implementations for the metadata
  // methods in order to have a common implementation that works for both
  // Instructions and Functions.
  //
  // Annotate an object that can take a metadata annotation (Instruction,
  // Function, ...) to associate a type with that object. We currently limit
  // this to holding a single type per metadata tag.
  //
  // The format of the metadata is to store a null value of the specified type
  // as follows:
  //   { Ty null, i32 PtrLevel }
  //
  // The use of a null value of the type enables the type to be kept up-to-date
  // when DTrans transformations run because when the instruction referencing
  // the metadata is remapped, the type within the metadata will be remapped as
  // well, if the type changes.
  template <typename Annotatable>
  static void createDTransTypeAnnotationImpl(Annotatable &A, StringRef Name,
                                             llvm::Type *Ty,
                                             unsigned PtrLevel) {
    assert(Ty && "Type must be not be null");
    assert(A.getMetadata(Name) == nullptr &&
           "Only a single SOA-to-AOS type metadata attachment allowed.");

    LLVMContext &Ctx = A.getContext();
    auto *Int32Ty = Type::getInt32Ty(Ctx);
    MDTuple *MD = MDTuple::get(
        Ctx, {ConstantAsMetadata::get(Constant::getNullValue(Ty)),
              ConstantAsMetadata::get(ConstantInt::get(Int32Ty, PtrLevel))});
    A.setMetadata(Name, MD);
  }

  // Get the type that exists in an annotation, if one exists, for the
  // Annotatable object.
  template <typename Annotatable>
  static std::optional<std::pair<llvm::Type *, unsigned>>
  lookupDTransTypeAnnotationImpl(Annotatable &A, StringRef Name) {
    auto *MD = A.getMetadata(Name);
    if (!MD)
      return std::nullopt;

    assert(MD->getNumOperands() == 2 && "Unexpected metadata operand count");
    auto &MDOpp0 = MD->getOperand(0);
    auto *TyMD = dyn_cast<ConstantAsMetadata>(MDOpp0);
    if (!TyMD)
      return std::nullopt;

    auto &MDOpp1 = MD->getOperand(1);
    auto *PtrLevelMD = dyn_cast<ConstantAsMetadata>(MDOpp1);
    if (!PtrLevelMD)
      return std::nullopt;

    llvm::Type *BaseTy = TyMD->getType();
    unsigned PtrLevel =
        cast<ConstantInt>(PtrLevelMD->getValue())->getZExtValue();
    return std::make_pair(BaseTy, PtrLevel);
  }

  // Return 'true' if an annotation of type 'Name' exists on the Annotatable
  // object.
  template <typename Annotatable>
  static bool hasDTransTypeAnnotationImpl(Annotatable &A, StringRef Name) {
    auto *MD = A.getMetadata(Name);
    if (!MD)
      return false;

    assert(MD->getNumOperands() == 2 && "Unexpected metadata operand count");
    auto &MDOpp1 = MD->getOperand(0);
    auto *TyMD = dyn_cast<ConstantAsMetadata>(MDOpp1);
    if (!TyMD)
      return false;

    return true;
  }

  // Remove a type annotation. Return 'true' if metadata was removed from the
  // object.
  template <typename Annotatable>
  static bool removeDTransTypeAnnotationImpl(Annotatable &A, StringRef Name) {
    bool HadMD = A.getMetadata(Name) ? true : false;
    A.setMetadata(Name, nullptr);
    return HadMD;
  }
};

} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSANNOTOR_H
