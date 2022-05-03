//===-------- DTransOPOptBase.h - Base class for DTrans Transforms ---------==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
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

#if !INTEL_FEATURE_SW_DTRANS
#error DTransOPOptBase.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASE_H
#define INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASE_H

#include "llvm/ADT/SetVector.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

namespace llvm {
class Module;

namespace dtransOP {
class DTransSafetyInfo;
class DTransStructType;
class DTransType;
class DTransTypeManager;

// This class handles the remapping of types during the transformation and
// cloning of functions for DTrans.
//
// Clients should first populate the old type to new type mapping for types the
// transformation needs to replace with the addTypeMapping() method. New
// types that derive from the type mapping can be then be computed using the
// remapType() method.
//
// For example: if %struct.t1 is to be replaced with %struct.xyz_trans.t1,
// then a call should be made to addTypeMapping() to indicate the llvm::Type
// and DTransOp:DTransType mapping from old type to new type.
//
// After all the mappings are done by the base class and derived class, lookups
// of replacements for types can then be performed using the remapType()
// method. For example, when the transformation needs to know what the
// replacement for the array type "[5 x %struct.t1]" or the function type "void
// (%struct.t1)" should be, a call to remapType() can be made.
//
//
// After all type mappings for the structures being modified are added, the
// setAllTypeMappingsAdded() method must be called, which will allow the
// remapType() routine to be used to compute and cache results.
//
class DTransOPTypeRemapper : public ValueMapTypeRemapper {
public:
  DTransOPTypeRemapper(DTransTypeManager &TM, bool UsingOpaquePtrs)
      : TM(TM), UsingOpaquePtrs(UsingOpaquePtrs), AllTypeMappingsAdded(false) {}

  DTransOPTypeRemapper(const DTransOPTypeRemapper &) = delete;
  DTransOPTypeRemapper &operator=(const DTransOPTypeRemapper &) = delete;
  DTransOPTypeRemapper(DTransOPTypeRemapper &&) = delete;
  DTransOPTypeRemapper &operator=(DTransOPTypeRemapper &&) = delete;

  // Return the type to use for 'SrcTy'.
  //
  // If the type is not being changed, then 'SrcTy' will be returned. Otherwise
  // the replacement type will be returned. This is the method that will be used
  // by the ValueMapper class when rewriting IR.
  //
  // This method caches the results for subsequent lookups, and may only be
  // used after all the base types being replaced have been populated via
  // the addTypeMapping() method.
  llvm::Type *remapType(llvm::Type *SrcTy) override;

  // This method can be used by the base class and derived class to get the
  // DTransType that should be used for 'SrcTy'.
  //
  // If the type is not being changed, then 'SrcTy' will be returned. Otherwise
  // the replacement type will be returned.
  DTransType *remapType(DTransType *SrcTy);

  // Add a type 'SrcTy' that needs to be remapped to 'DestTy'. Also, the
  // corresponding DTransTypes objects are required parameters.
  void addTypeMapping(llvm::Type *SrcTy, llvm::Type *DestTy,
                      DTransType *DTSrcTy, DTransType *DTDestTy);

  // Indicate that all structure types that DTrans needs to rewrite have
  // been added.
  void setAllTypeMappingsAdded() { AllTypeMappingsAdded = true; }

  // Check if the 'SrcTy' type has a mapping in the type list.
  bool hasRemappedType(llvm::Type *SrcTy) const;
  bool hasRemappedType(DTransType *SrcTy) const;

  // Return the type mapping for 'SrcTy' type, if there is one. If there is
  // not one, return nullptr. This differs from remapType(), in that it will
  // not create and cache a new type mapping for 'SrcTy'.
  llvm::Type *lookupTypeMapping(llvm::Type *SrcTy) const;
  DTransType *lookupTypeMapping(DTransType *SrcTy) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

private: // methods
  // Compute the replacement type for 'SrcTy' based on the 'SrcTypeToNewType'
  // mapping. If the type needs to be replaced, return the type to be used. If
  // the type should not be replaced, return nullptr.
  llvm::Type *computeReplacementType(llvm::Type *SrcTy) const;
  DTransType *computeReplacementType(DTransType *SrcTy) const;

  // Return the cached result for a type mapping for 'SrcTy' type, if the type
  // has been evaluated previously. Otherwise, return nullptr.
  llvm::Type *lookupCachedTypeMapping(llvm::Type *SrcTy) const;
  DTransType *lookupCachedTypeMapping(DTransType *SrcTy) const;

private: // data
  // Reference to the DTransTypeManager object being used.
  // NOTE: The same object must be used by the safety analyzer and the
  // transformation.
  DTransTypeManager &TM;

  // Indicates whether opaque pointers are being used. This affects whether or
  // not to try to get a pointer element type from a llvm::PointerType object
  // when computing replacement types.
  bool UsingOpaquePtrs;

  // Mapping from original type to the replacement type.
  DenseMap<llvm::Type *, llvm::Type *> SrcTypeToNewType;
  DenseMap<DTransType *, DTransType *> DTransSrcTypeToNewType;

  // During the remapping process, a cache is built up to avoid repeated
  // computations on complex types that have been determine to need or not need
  // to be replaced.
  DenseMap<llvm::Type *, llvm::Type *> RemapSrcToDestTypeCache;
  DenseMap<DTransType *, DTransType *> DTransRemapSrcToDestTypeCache;

  // This indicates the client has added all the structure types the
  // transformation needs to replace.
  bool AllTypeMappingsAdded;
};

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
  // Data structure to use for mapping one type to another type.
  using LLVMTypeToTypeMap = DenseMap<llvm::Type *, llvm::Type *>;

  // Data structure for storing the set of types that are dependent types for
  // another type.
  using DTransTypeToTypeSetMap =
      DenseMap<DTransType *, SetVector<DTransType *>>;

  DTransOPOptBase(LLVMContext &Ctx, DTransSafetyInfo *DTInfo,
                  StringRef DepTypePrefix);

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

protected:
  //===-------------------------------------------------------------------===//
  // Methods that must be implemented by the derived classes that perform some
  // transformation.
  //===-------------------------------------------------------------------===//

  // Derived classes need to implement this method to construct 'llvm::Type' and
  // 'DTransType' objects for any structures they are directly converting. When
  // new types are created they must be added to the TypeRemapper. Generally,
  // the derived class will create an opaque type within this routine because
  // the structure being converted may contain pointers to other structures that
  // need to be remapped. The body elements of the type will be populated after
  // all types have been created.
  virtual bool prepareTypes(Module &M) = 0;

  // Derived classes need to implement this method to populate the body for any
  // types they are directly converting to contain the body elements of the new
  // type based on the remapped types returned by calls to the TypeRemapper.
  virtual void populateTypes(Module &M) = 0;

  //===-------------------------------------------------------------------===//
  // Methods that may be implemented by the derived classes that perform some
  // transformation.
  //===-------------------------------------------------------------------===//

  // The ValueMapper takes a materializer object that will be called with
  // certain Value objects (functions, basic blocks, and constants) during the
  // type remapping process. This function allows the derived classes to supply
  // a Materializer to use during the type remapping process.
  virtual ValueMaterializer* getMaterializer() { return nullptr; }

  // Derived classes may implement this to perform module level work that needs
  // to be performed on global variables prior to beginning any function
  // transformation work. For example, creating new global variables needed
  // for the optimization.
  virtual void prepareModule(Module &M) {};

  // Derived classes may implement this method to create the replacement
  // variable for an existing global variable. If a replacement is made, then
  // the new variable must be returned, and the derived class will be
  // responsible for initializing the variable when a call to
  // initializeGlobalVariable is made. If the child class does not need to do
  // something specific for replacing the variable, it should return nullptr. An
  // example of the use would be if a global variable is instantiated for a type
  // that is having some field deleted, the base class would not know how to
  // initialize the value of a newly created variable, but the derived class
  // would.
  virtual GlobalVariable *createGlobalVariableReplacement(GlobalVariable *GV,
                                                          ValueMapper &Mapper) {
    return nullptr;
  }

  // Derived classes may implement this method to update uses of global
  // variables after the global variables have been updated.
  virtual void postprocessGlobalVariable(GlobalVariable *OrigGV,
                                         GlobalVariable *NewGV) {}

  // Derived classes may implement this to perform the transformation on a
  // function.
  virtual void processFunction(Function &F) {}

  // Derived class may implement this to perform any work that is needed on
  // the function following all the types being remapped to new types.
  virtual void postprocessFunction(Function &OrigFunc, bool isCloned) {}

  //===-------------------------------------------------------------------===//
  // Methods that are exposed to the derived classes
  //===-------------------------------------------------------------------===//

  DTransOPTypeRemapper *getTypeRemapper() { return &TypeRemapper; }

  // Sets the body for the all the types in the \p DependentTypeMapping based
  // on types computed by the TypeRemapper.
  void populateDependentTypes(Module &M,
                              const LLVMTypeToTypeMap &DependentTypeMapping);

  // Derived class that override createReplacementGlobalVariable must implement
  // this method to handle the initialization of any GlobalVariable objects the
  // derived class returned during calls to that method so that fields of the
  // new structure body are properly initialized. However, the base class
  // version is exposed to the derived classes because there are some cases
  // where they can simply rely on the base class implementation, such as if the
  // initialization value was a zeroinitializer.
  virtual void initializeGlobalVariableReplacement(GlobalVariable *OrigGV,
                                                   GlobalVariable *NewGV,
                                                   ValueMapper &Mapper) {
    initializeGlobalVariableReplacementBaseImpl(OrigGV, NewGV, Mapper);
  }

  void remapDTransTypeMetadata(Value *V, ValueMapper &Mapper);

private:
  //===-------------------------------------------------------------------===//
  // These methods should not be made available to the derived classes.
  //===-------------------------------------------------------------------===//

  bool prepareTypesBaseImpl(Module &M);
  void buildTypeDependencyMapping();
  void collectDependenciesForType(DTransStructType *StructTy);
  void prepareDependentTypes(Module &M,
                             LLVMTypeToTypeMap &DependentTypeMapping);
  void updateDTransTypesMetadata(Module &M, ValueMapper &Mapper);
  void createCloneFunctionDeclarations(Module &M);
  void convertGlobalVariables(Module &M, ValueMapper &Mapper);
  void initializeGlobalVariableReplacementBaseImpl(GlobalVariable *OrigGV,
                                                   GlobalVariable *NewGV,
                                                   ValueMapper &Mapper);
  void transformIR(Module &M, ValueMapper &Mapper);
  void updateAttributeTypes(Function *CloneFunc);
  void removeDeadValues();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpTypeToTypeSetMapping(StringRef Header,
                                DTransTypeToTypeSetMap &TypeToDependentTypes);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

protected:
  //===-------------------------------------------------------------------===//
  // Data that may be shared with the derived class.
  // TODO: Some of these could be made private, and just have accessors for
  // use by the derived classes.
  //===-------------------------------------------------------------------===//

  // The DTransSafetyInfo object that analyzed the types and calls of the
  // module.
  DTransSafetyInfo *DTInfo;

  // Reference to the DTransTypeManager object being used.
  // NOTE: The same object must be used by the safety analyzer and the
  // transformation.
  DTransTypeManager &TM;

  // Optional string to precede names of dependent types that get renamed.
  std::string DepTypePrefix;

  // 'true' if pointers are opaque.
  bool UsingOpaquePtrs;

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

  DTransOPTypeRemapper TypeRemapper;

  // Mapping of original Value* to the replacement Value*. This mapping serves
  // two purposes.
  // 1: It is used by the ValueMapper to lookup whether a replacement for
  //    a value has been defined. Therefore, transformations can put items into
  //    this map prior to running the remapping to get those replacements to
  //    occur. This will be done for things like changing a function call to
  //    instead go to a cloned function.
  //  2: This mapping also gets populated as the replacements are created during
  //     the remapping process. This allows finding what value was used as
  //     the replacement.
  // Initially, it will be primed with the global variables and functions that
  // need cloning. As the ValueMapper replaces values those will get inserted.
  ValueToValueMapTy VMap;

  // A mapping from the original function to the clone function that will
  // replace the original function.
  DenseMap<Function *, Function *> OrigFuncToCloneFuncMap;

  // A mapping from the clone function to the original function to enable
  // lookups of the original function based on a clone function pointer.
  DenseMap<Function *, Function *> CloneFuncToOrigFuncMap;

  // List of global variables that are being replaced with variables of the new
  // types due to type remapping. The variables in this list need to be
  // destroyed once the entire module has been remapped.
  SmallVector<GlobalValue *, 16> GlobalsForRemoval;
};

} // namespace dtransOP
} // namespace llvm

#endif // INTEL_DTRANS_TRANSFORMS_DTRANSOPOPTBASE_H
