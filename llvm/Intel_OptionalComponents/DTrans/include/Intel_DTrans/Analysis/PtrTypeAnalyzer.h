//===------------------------PtrTypeAnalyzer.h----------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

// This file defines the pointer type analyzer classes and interfaces that are
// used by the DTrans safety checks and transformations to perform type recovery
// from opaque pointers in order to identify the types of object that a pointer
// may be referring to.

#if !INTEL_INCLUDE_DTRANS
#error PtrTypeAnalyzer.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_PTRYPEANALYZER_H
#define INTEL_DTRANS_ANALYSIS_PTRYPEANALYZER_H

#include <functional>
#include <memory>

namespace llvm {
class DataLayout;
class Function;
class Module;
class TargetLibraryInfo;

namespace dtrans {

class DTransTypeManager;
class PtrTypeAnalyzerImpl;
class TypeMetadataReader;

// This class is for keeping track of the source object type that a Value refers
// to, and what type of object it is used as. If the Value is the address of an
// element within a structure or array, that will be tracked as well.
class ValueTypeInfo {
  // TODO: Add the ValueTypeInfo implementation used to store a map from a
  // Value object to a set of DTransPointerType objects
};

// This class provides the interface to the information collected by the
// pointer type analyzer for use by the DTrans safety analysis and DTrans
// transformation passes.
class PtrTypeAnalyzer {
public:
  PtrTypeAnalyzer(
      DTransTypeManager &TM, TypeMetadataReader &MDReader, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI);

  ~PtrTypeAnalyzer();

  // Copying is not allowed, but moving should be.
  PtrTypeAnalyzer(const PtrTypeAnalyzer &) = delete;
  PtrTypeAnalyzer(PtrTypeAnalyzer &&);
  PtrTypeAnalyzer &operator=(const PtrTypeAnalyzer &) = delete;
  PtrTypeAnalyzer &operator=(PtrTypeAnalyzer &&);

  // This method is the entry point to perform analysis of the module to
  // identify pointer types.
  void run(Module &M);

private:
  std::unique_ptr<PtrTypeAnalyzerImpl> Impl;
};

} // end namespace dtrans
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_PTRYPEANALYZER_H
