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

#if !INTEL_INCLUDE_DTRANS
#error DTransAnnot.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif
#ifndef INTEL_DTRANS_ANALYSIS_DTRANSANNOTOR_H
#define INTEL_DTRANS_ANALYSIS_DTRANSANNOTOR_H

#include "llvm/IR/Instruction.h"

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
  static void createDTransTypeAnnotation(Instruction &I, llvm::Type *Ty);

  // Get the type that exists in an annotation, if one exists, for the
  // instruction.
  static llvm::Type *lookupDTransTypeAnnotation(Instruction &I);

  // Remove a type annotation from the instruction. Return 'true' if the
  // instruction is changed.
  static bool removeDTransTypeAnnotation(Instruction &I);

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
};

} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSANNOTOR_H
