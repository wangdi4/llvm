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

namespace llvm {

class Instruction;
class Type;

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
  // Enumeration of supported DTrans annotation types
  enum DPA_AnnotKind {

    DPA_DTransType,

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
  static void createDTransTypeAnnotation(Instruction *I, llvm::Type *Ty);

  // Get the type that exists in an annotation, if one exists, for the
  // instruction.
  static llvm::Type *lookupDTransTypeAnnotation(Instruction *I);

  // Remove a type annotation from the instruction.
  static void removeDTransTypeAnnotation(Instruction *I);

private:
  // Names to use for metadata or global variables created to hold the
  // annotation information. Having all the names here avoids a tight coupling
  // of transformations, and enables a cleanup pass that can find and remove all
  // them and the end of the DTrans phase.
  static const char *AnnotNames[DPA_Last];
};

} // namespace dtrans
} // namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSANNOTOR_H
