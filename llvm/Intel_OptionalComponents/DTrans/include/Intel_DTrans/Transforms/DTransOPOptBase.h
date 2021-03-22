//===-------- DTransOPOptBase.h - Base class for DTrans Transforms ---------==//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides the base classes for DTrans Transformations that provide
// the common functionality needed for rewriting dependent data types and
// functions that change as the result of DTrans modifying a structure
// definition. This is to work with an opaque pointer representation.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransOPOptBase.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASE_H
#define INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASE_H

namespace llvm {
class Module;

namespace dtransOP {

// This is a base class that specific transformations derive from to
// implement transformations. This class provides the basic framework
// for driving the transformation and handling the common functionality needed
// by most of the transformations for transforming dependent data types.
//
// This class handles:
// - The identification of dependent data types.
// - The construction of new data types for the dependent types.
// - The replacement of global variables with types being changed.
// - The cloning of functions that have arguments or return values with types
//   that are being modified.
class DTransOPOptBase {
public:
  DTransOPOptBase() {}

  DTransOPOptBase(const DTransOPOptBase &) = delete;
  DTransOPOptBase &operator=(const DTransOPOptBase &) = delete;

  virtual ~DTransOPOptBase() {}

  // The main routine the drives the entire process. Returns 'true' if changes
  // are made to the module.
  //
  // The flow and interaction with the derived classes is:
  //  1. Child class prepares opaque types for new types: (prepareTypes)
  //  2. Base class identifies types dependent on step 1.
  //  3. Base class populates new types for dependent types of step 2.
  //  4. Child class populates types of step 1. (populateTypes)
  //  5. Child class performs any module level transform to create new
  //  variables. (prepareModule)
  //  6. Base class creates new function prototypes for dependent functions.
  //  7. Base class creates new global variables for dependent variables.
  //  8. For each function:
  //    8a. Child class performs transformation (processFunction)
  //    8b. Base class clones or remaps types for function
  //    8c. Child class perform post-processing of transformed functions
  //    (postProcessFunction)
  bool run(Module &M);

  // TODO: Add the functions needed by the run() method.
};

} // namespace dtransOP
} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASE_H
