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

#include "llvm/ADT/SetVector.h"

namespace llvm {
class Module;

namespace dtransOP {

class DTransStructType;
class DTransType;
class DTransTypeManager;

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
  // Data structure for storing the set of types that are dependent types for
  // another type.
  using DTransTypeToTypeSetMap =
      DenseMap<DTransType *, SetVector<DTransType *>>;

  DTransOPOptBase(DTransTypeManager &TM) : TM(TM) {}

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

private:
  //===-------------------------------------------------------------------===//
  // These methods should not be made available to the derived classes.
  //===-------------------------------------------------------------------===//

  bool prepareTypesBaseImpl(Module &M);
  void buildTypeDependencyMapping();
  void collectDependenciesForType(DTransStructType *StructTy);
  void dumpTypeToTypeSetMapping(StringRef Header,
                                DTransTypeToTypeSetMap &TypeToDependentTypes);

protected:
  //===-------------------------------------------------------------------===//
  // Data that may be shared with the derived class.
  // TODO: Some of these could be made private, and just have accessors for
  // use by the derived classes.
  //===-------------------------------------------------------------------===//

  // Reference to the DTransTypeManager object being used.
  // NOTE: The same object must be used by the safety analyzer and the
  // transformation.
  DTransTypeManager &TM;

  // Collection of all the structure types.
  std::vector<DTransStructType *> KnownStructTypes;

  // These will be populated with a list of dependent types for each
  // structure type prior to the call to the prepareTypes() method of derived
  // classes. This enables derived classes to examine those types, which may
  // impact how transformed types are constructed.
  //
  // One set is maintained for direct dependencies due to nesting of types. This
  // set is needed because changing the type of a structure nested within
  // another type requires generating a new type within the outer type. For
  // example:
  //   %struct.A = type { %struct.B, [ 4 x %struct.C] }
  //
  // A second set is maintained for cases where there is a dependency due to
  // pointer references. This set is used when opaque pointers are not in use
  // because in that case changing a referenced type will require changing the
  // type that references it. For example:
  //   %struct.D = type { %struct.E*, void (%struct.F*)* }
  DTransTypeToTypeSetMap TypeToDirectDependentTypes;
  DTransTypeToTypeSetMap TypeToPtrDependentTypes;
};

} // namespace dtransOP
} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASE_H
