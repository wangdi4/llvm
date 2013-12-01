/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __BUILT_IN_FUNCTION_IMPORT_H__
#define __BUILT_IN_FUNCTION_IMPORT_H__

#include "BuiltinLibInfo.h"
#include <llvm/Pass.h>
#include <llvm/IR/Constants.h>
#include <llvm/Transforms/Utils/ValueMapper.h>

#include <vector>
#include <set>

using namespace llvm;

namespace intel {

  /// This pass imports built-in functions from source module to destination module.
  class BIImport : public ModulePass {
  protected:
    // Type used to hold a vector of Functions and augment it during traversal.
    typedef std::vector<llvm::Function*>       TFunctionsVec;

    // Type used to hold a vector of Globals and augment it during traversal.
    typedef std::vector<llvm::GlobalVariable*> TGlobalsVec;

    // Type used to hold a set of Functions and augment it during traversal.
    typedef std::set<llvm::Function*>          TFunctionsSet;

  public:
    // Pass identification, replacement for typeid.
    static char ID;

    /// @brief Constructor
    BIImport() : ModulePass(ID) {}

    /// @brief Provides name of pass
    virtual const char *getPassName() const {
      return "BIImport";
    }

    /// @brief Main entry point.
    ///        Find all builtins to import, and import them along with callees and globals.
    /// @param M The destination module.
    bool runOnModule(Module &M);

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BuiltinLibInfo>();
    }

  protected:
    /// @brief First find all "root" functions that are only declared in
    ///        destination module and defined in builtins module.
    ///        Then scan all the functions which they call, iteratively.
    ///        Make sure each such called function has a declaration in the destination module.
    ///        Add it to list of all functions to be imported, unless it's an intrinsic.
    ///        Add the builtin-module-definition -to- destination-module-declation to valueMap.
    ///
    /// @brief Scan all destination module function declarations used by defined
    ///        functions in destination module and which have implemnetation in source module.
    ///        Materialize the source function if needed.
    ///        Add the source-module-functions to destination-module-functions mapping.
    void CollectRootFunctionsToImport();

    /// @brief Scan all global variables used by functions to be imported.
    ///        Import them without initialization into the destination module.
    ///        Add the source-module-global to destination-module-global mapping.
    void CollectGlobalsToImport();

    /// @brief Scan all source module functions used by functions to be imported.
    ///        Import their declarations into the destination module.
    ///        Materialize the source function if needed.
    ///        Add the source-module-functions to destination-module-functions mapping.
    bool CollectSourceFunctionsToImport();

    /// @brief Iterate over all global variables to import,
    ///        and perform the actual importation (import initialization).
    void ImportGlobalVariablesInitializations();

    /// @brief Iterate over all functions to import,
    ///        and perform the actual importation (import definition).
    void ImportFunctionDefinitions();

    /// @brief Collect (recursively) functions used by given root functions.
    /// @param [IN/OUT] rootFunctionsToImport The giving list of root functions
    ///        To start searching from, this list will be destroyed.
    void CollectFunctionChainToImport(TFunctionsVec &rootFunctionsToImport);

    /// @brief Import declaration of given source function from source module
    ///        into destination module if it is needed by destination module,
    ///        materialize the source function if needed,
    ///        and map source function declaration to destination function.
    /// @param [IN] pSrcFunc The given source function.
    /// @param [IN] pDstFunc The given destination function declaration (can be NULL).
    /// @return true if given source function needed to be mapped, false otherwise.
    bool MapAndImportFunctionDclIfNeeded(Function *pSrcFunc, Function *pDstFunc);

    /// @brief Import given source global variable from source module into
    ///        destination module, without its initialization.
    /// @param [IN] pSrcGlobal The given source global variable.
    void ImportGlobalVariableDeclaration(GlobalVariable* pSrcGlobal);

    /// @brief Get all the functions called by given function.
    /// @param [IN] pFunc The given function.
    /// @param [OUT] calledFuncs The list of all functions called by pFunc.
    void GetCalledFunctions(const Function* pFunc, TFunctionsVec& calledFuncs);

    /// @brief Check if given value is used by any function that need to be imported.
    ///        pVal is assumed to belong to Destination module.
    /// @param [IN] pVal The given value.
    /// @return true if given value is used by any function that need to be imported, false otherwise.
    bool IsDestValUsedInModule(Value *pVal);

    /// @brief Check if given value is used by any function that need to be imported.
    ///        pVal is assumed to belong to source module.
    /// @param [IN] pVal The given value.
    /// @return true if given value is used by any function that need to be imported, false otherwise.
    bool IsSrcValUsedInModule(Value *pVal);

  protected:

    /// Source module - conatians the source function definition to import
    Module* m_pSourceModule;
    /// Destination module - contains function declarations to resolve from RT module
    Module* m_pModule;

    /// Map between values (functions/globals) in source module
    //  to their counterpart values in destination module
    ValueToValueMapTy  m_valueMap;
    /// List of source functions to import
    TFunctionsVec      m_functionsToImport;
    /// List of source global values to import
    TGlobalsVec        m_globalsToImport;
  };

} //namespace Intel {

#endif // __BUILT_IN_FUNCTION_IMPORT_H__