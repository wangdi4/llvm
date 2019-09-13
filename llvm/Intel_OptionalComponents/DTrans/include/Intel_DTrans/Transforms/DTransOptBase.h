//===---- DTransOptBase.h - Common base classes for DTrans Transforms --==//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares the base classes for DTrans transformations that provide
// the common functionality needed for rewriting dependent data types and
// functions which change as the result of DTrans modifying a structure
// definition. Transformations should derive from the DTransOptBase class to
// get the needed common functionality.
//
//===----------------------------------------------------------------------===//

#if !INTEL_INCLUDE_DTRANS
#error DTransOptBase.h include in an non-INTEL_INCLUDE_DTRANS build.
#endif

#ifndef INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTBASE_H
#define INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTBASE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

namespace llvm {

class TargetLibraryInfo;

namespace dtrans {
class CallInfo;
}

class DTransAnalysisInfo;

// This class handles the remapping of structure types from old to new types
// during the transformation and cloning of functions for DTrans.
//
// Clients should first populate the old type to new type mapping for types the
// transformation needs to replace with the \b addTypeMapping() method. New
// types that derive from the type mapping can be then be computed using the
// \b computeReplacementType() method.
//
// For example: if %struct.t1 is to be replaced with %struct.xyz_trans.t1,
// then a call should be made to:
//   addTypeMapping(%struct.t1, %struct.xyx_trans.t1);
// If the transformation then needs to know what the replacement for the array
// type "[5 x %struct.t1**]" or the function type "void (%struct.t1*)*" should
// be, a call to \b computeReplacementType() can be made.
//
//
// After all type mappings for the structures being modified are added, the
// \b setAllTypeMappingsAdded() method must be called, which will allow the
// \b remapType() routine to be used to compute and cache results.
//
class DTransTypeRemapper : public ValueMapTypeRemapper {
public:
  DTransTypeRemapper() : AllTypeMappingsAdded(false) {}

  DTransTypeRemapper(const DTransTypeRemapper &) = delete;
  DTransTypeRemapper &operator=(const DTransTypeRemapper &) = delete;

  // Return the type to use for \p SrcTy.
  //
  // If the type is not being changed, then \p SrcTy will be returned. Otherwise
  // the replacement type will be returned.
  //
  // This method caches the results for subsequent lookups, and may only be
  // used after all the base types being replaced have been populated via
  // the \b addTypeMapping() method.
  virtual llvm::Type *remapType(llvm::Type *SrcTy) override;

  // Add a type \p SrcTy that needs to be remapped to \p DestTy.
  void addTypeMapping(llvm::Type *SrcTy, llvm::Type *DestTy);

  // Indicate that all structure types that DTrans needs to rewrite have
  // been added.
  void setAllTypeMappingsAdded() { AllTypeMappingsAdded = true; }

  // Check if the \p SrcTy type has a mapping in the type list
  bool hasRemappedType(llvm::Type *SrcTy) const;

  // Return the type mapping for \p SrcTy type, if there is one. If there is
  // not one, return nullptr. This differs from \b remapType, in that it will
  // not create and cache a new type mapping for \p SrcTy.
  llvm::Type *lookupTypeMapping(llvm::Type *SrcTy) const;

  // Return the cached result for a type mapping for \p SrcTy type, if the type
  // has been evaluated previously. Otherwise, return nullptr.
  llvm::Type *lookupCachedTypeMapping(llvm::Type *SrcTy) const;

  // Compute the replacement type for \p SrcTy based on the SrcTypeToNewType
  // mapping. If the type needs to be replaced, return the type to be used. If
  // the type should not be replaced, return nullptr.
  llvm::Type *computeReplacementType(llvm::Type *SrcTy) const;

private:
  // Mapping from original type to the replacement type.
  DenseMap<llvm::Type *, llvm::Type *> SrcTypeToNewType;

  // During the remapping process, a cache is built up to avoid repeated
  // computations on complex types that have been determine to need or not need
  // to be replaced.
  DenseMap<llvm::Type *, llvm::Type *> RemapSrcToDestTypeCache;

  // This indicates the client has added all the structure types the
  // transformation needs to replace.
  bool AllTypeMappingsAdded;
};

// This is a base class the specific transformations derive from to
// implement specific transformations. This class provides the basic framework
// for driving the transformation and handling the common functionality for
// transforming dependent data types.
//
// This class handles:
// - The identification of dependent data types
// - The construction of new data types for the dependent types
// - The replacement of global variables with types being changed
// - The cloning of functions that have arguments or return values with types
//   that are being modified.
class DTransOptBase {
public:
  // Data structure to use for mapping one type to another type.
  using TypeToTypeMap = DenseMap<llvm::Type *, llvm::Type *>;

  // Data structure to use for mapping one type to a set of types.
  using TypeToTypeSetMap = DenseMap<llvm::Type *, SetVector<Type *>>;

  // \param DTInfo        DTrans Analysis Result. This is an optional parameter
  //                      for the base class. However, if a transform needs to
  //                      have the DTrans CallInfo data structures kept
  //                      up-to-date when functions are transformed, this
  //                      parameter must be provided.
  // \param Context       llvm context for the module
  // \param DL            Module's data layout
  // \param GetTLI        Routine to get TargetLibraryInfo for a function.
  // \param DepTypePrefix Optional string to prefix structure names of rewritten
  //                      dependent types
  // \param TypeRemapper  Class that will perform type mapping from old types
  //                      to new types
  // \param Materializer  Optional class that works with ValueMapper to create
  //                      new Values during type remapping
  DTransOptBase(
      DTransAnalysisInfo *DTInfo, LLVMContext &Context, const DataLayout &DL,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
      StringRef DepTypePrefix, DTransTypeRemapper *TypeRemapper,
      ValueMaterializer *Materializer = nullptr)
      : DTInfo(DTInfo), Context(Context), DL(DL), GetTLI(GetTLI),
        DepTypePrefix(DepTypePrefix), TypeRemapper(TypeRemapper),
        Materializer(Materializer) {}

  DTransOptBase(const DTransOptBase &) = delete;
  DTransOptBase &operator=(const DTransOptBase &) = delete;

  virtual ~DTransOptBase() {}

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
  // Derived classes need to implement this method to construct 'llvm::Type'
  // objects for any structures they are directly converting. When new types are
  // created they must be added to the TypeRemapper. Generally, the derived
  // class will create an opaque type within this routine because the structure
  // being converted may contain pointers to other structures that need to be
  // remapped. The body elements of the type will be populated after all types
  // have been created.
  virtual bool prepareTypes(Module &M) = 0;

  // Derived classes need to implement this method to populate the body for any
  // types they are directly converting to contain the body elements of the new
  // type, based on the remapped types returned by calls to the TypeRemapper.
  virtual void populateTypes(Module &M) = 0;

  // Derived classes may implement this to perform module level work that needs
  // to be performed on global variables prior to beginning any function
  // transformation work. For example, creating any new global variables needed
  // for the optimization.
  virtual void prepareModule(Module &M){};

  // Derived classes may implement this method to create the replacement
  // variable for an existing global variable. If a replacement is made, then
  // the new variable must be returned, and the derived class will be
  // responsible for initializing the variable when a call to
  // initializeGlobalVarialbe is made. If the child class does not need to do
  // something specific for replacing the variable, it should return nullptr. An
  // example of the use would be if a global variable is instantiated for a type
  // that is having some field deleted, the base class would not know how to
  // initialize the value of a newly created variable, but the derived class
  // would. In effect, this method is to declaring that the replacement and
  // initialization of some global variable that needs transforming is going to
  // be delegated to the derived class.
  virtual GlobalVariable *createGlobalVariableReplacement(GlobalVariable *GV) {
    return nullptr;
  }

  // Derived class that implement createReplacementGlobalVariable must implement
  // this method to handle the initialization of any GlobalVariable objects the
  // derived class returned within that method
  virtual void initializeGlobalVariableReplacement(GlobalVariable *OrigGV,
                                                   GlobalVariable *NewGV,
                                                   ValueMapper &Mapper) {
    llvm_unreachable("Global variable replacement must be done by derived "
                     "class implementing createGlobalVariableReplacement");
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

  // Sets the body for the all the types in the \p DependentTypeMapping based
  // on types computed by the TypeRemapper.
  void populateDependentTypes(Module &M, TypeToTypeMap &DependentTypeMapping);

private:
  // Identify and create new types for any types the child class is going
  // to replace.
  bool prepareTypesBaseImpl(Module &M);

  // Identify and create types that need to be remapped because due to an
  // existing type that contains a reference to a type being changed by
  // the transformation.
  void buildTypeDependencyMapping();

  void collectDependenciesForType(Type *Ty);
  void collectDependenciesForTypeRecurse(Type *Depender, Type *Ty);

  void buildTypeEnclosingMapping();
  void collectEnclosingForType(Type *Ty);
  void collectEnclosingForTypeRecurse(SmallVectorImpl<Type *> &EnclosingTypes,
                                      Type *Ty);

#if !defined(NDEBUG)
  // Debug method to print the type dependency mapping table.
  void dumpTypeToTypeSetMapping(StringRef Header,
                                TypeToTypeSetMap &TypeToDependentTypes);
#endif // !defined(NDEBUG)

  void prepareDependentTypes(Module &M, TypeToTypeSetMap &TypeToDependentTypes,
                             TypeToTypeMap &DependentTypeMapping);

  // Identify and clone any function prototypes for functions that will need
  // to be cloned.
  void createCloneFunctionDeclarations(Module &M);

  // Remap global variables that have type changes to their new types
  void convertGlobalVariables(Module &M, ValueMapper &Mapper);

  // Perform all the module and function processing to transform the IR.
  void transformIR(Module &M, ValueMapper &Mapper);

  // Remove functions and global variables that have been completely
  // replaced due to the remapping.
  void removeDeadValues();


protected:

  void deleteCallInfo(dtrans::CallInfo *CInfo);

  // Returns a set of types enclosing \p Ty.
  const typename TypeToTypeSetMap::mapped_type &getEnclosingTypes(Type *Ty);

protected:
  DTransAnalysisInfo *const DTInfo;
  LLVMContext &Context;
  const DataLayout &DL;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  // Optional string to precede names of dependent types that get renamed.
  std::string DepTypePrefix;

  // Collection of all the structure types in the IR.
  SetVector<llvm::StructType *> KnownStructTypes;

  // This will be populated with the a list of dependent types for each
  // structure type prior to call to the prepareTypes method of derived
  // classes. This enables derived classes to examine those types, which may
  // impact how transformed types are constructed.
  TypeToTypeSetMap TypeToDependentTypes;

  DTransTypeRemapper *TypeRemapper;
  ValueMaterializer *Materializer;

  // Mapping of original Value* to the replacement Value*. This mapping serves
  // two purposes.
  // 1: It is used by the ValueMapper to lookup whether a replacement for
  //    a value has been defined. Therefore, transformations can set items into
  //    this map prior to running the remapping to get those replacements to
  //    occur. This will be done for things like changing a function call to
  //    instead go to a cloned function.
  //  2: This mapping also gets populated as the replacements are created during
  //     the remapping process. This allows finding what value was used as
  //     the replacement.
  // Initially it will be primed with the global variables and functions that
  // need cloning. As the ValueMapper replaces values those will get inserted.
  ValueToValueMapTy VMap;

  // A mapping from the original function to the clone function that will
  // replace the original function.
  DenseMap<Function *, Function *> OrigFuncToCloneFuncMap;

  // A mapping from the clone function to the original function to enable
  // lookups of the original function based on a clone function pointer.
  MapVector<Function *, Function *> CloneFuncToOrigFuncMap;

  // List of global variables that are being replaced with variables of the new
  // types due to the type remapping. The variables in this list need to be
  // destroyed once the entire module has been remapped.
  SmallVector<GlobalValue *, 16> GlobalsForRemoval;

private:
  // A mapping of "Function -> { call info set }" objects.
  // The CallInfo objects will need to have their types updated
  // following cloning/remapping of the function. This map will
  // be used to find which CallInfo objects need to be updated after
  // processing each function. This map is kept private to prevent
  // the derived classes from accessing it directly. Any updates
  // needed must be done through this class's interface methods.
  DenseMap<Function *, SmallVector<dtrans::CallInfo *, 4>>
      FunctionToCallInfoVec;

  // A mapping of "Type -> set of enclosing types". Computed on-demand by
  // getEnclosingTypes().
  TypeToTypeSetMap TypeToEnclosingTypes;

  // Set up the mapping of Functions to a set of CallInfo objects that need to
  // be processed as each function is transformed.
  void initializeFunctionCallInfoMapping();

  // Updates the CallInfo objects associated with a specific function.
  void updateCallInfoForFunction(Function *F, bool isCloned);

  // Update types used in the parameter attributes of a function that is
  // cloned.
  void updateAttributeTypes(Function *CloneFunc);

  // Clear the Function to CallInfo mapping.
  void resetFunctionCallInfoMapping();
};

} // namespace llvm

#endif // INTEL_OPTIONALCOMPONENTS_INTEL_DTRANS_TRANSFORMS_DTRANSOPTBASE_H
