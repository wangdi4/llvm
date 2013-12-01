/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "BuiltInFuncImport.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

#include <string>

using namespace llvm;
namespace intel {

  char BIImport::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(BIImport, "builtin-import", "Built-in function pass", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(BIImport, "builtin-import", "Built-in function pass", false, true)

  bool BIImport::runOnModule(Module &M) {
    BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
    m_pSourceModule = BLI.getBuiltinModule();
    if (m_pSourceModule == NULL) {
      // If there is no source module, then nothing can be imported.
      return false;
    }

    // Initialize members
    m_pModule = &M;
    m_valueMap.clear();
    m_functionsToImport.clear();
    m_globalsToImport.clear();

    CollectRootFunctionsToImport();
    // Global variables might contain pointers to function, thus, need
    // to check if there is new functions to import after collecting globals.
    // Adding new functions, might cause adding new globals, thus, need to loop
    // on collecting globals & functions till there is no more functions to import.
    while(true) {
      CollectGlobalsToImport();
      if(!CollectSourceFunctionsToImport()) {
        // Break when there is no new functions to import
        break;
      }
    }
    ImportGlobalVariablesInitializations();
    ImportFunctionDefinitions();

    // If value map is not empty, then "probably" imported some values, thus module was changed.
    return !m_valueMap.empty();
  }

  void BIImport::CollectRootFunctionsToImport() {
    assert(m_functionsToImport.empty() && "Starting to fill a non-empty list of all functions.");

    TFunctionsVec rootFunctionsToImport;

    // Find all "root" functions.
    for (Module::iterator it = m_pModule->begin(), e = m_pModule->end(); it != e; ++it) {
      Function *pDstFunc = it;
      std::string pFuncName = pDstFunc->getName().str();
      if (pDstFunc->isDeclaration()) {
        Function* pSrcFunc = m_pSourceModule->getFunction(pFuncName);
        if (!pSrcFunc) continue;
        if (MapAndImportFunctionDclIfNeeded(pSrcFunc, pDstFunc)) {
          rootFunctionsToImport.push_back(pSrcFunc);
        }
      }
    }
    // Collect functions recursively used by root functions
    CollectFunctionChainToImport(rootFunctionsToImport);
  }

  void BIImport::CollectGlobalsToImport() {
    Module::GlobalListType &lstGlobals = m_pSourceModule->getGlobalList();
    // Iterate over all globals in source module and check if any needs to be imported
    for (Module::GlobalListType::iterator it = lstGlobals.begin(), e = lstGlobals.end(); it != e; ++it) {
      GlobalVariable* pGlobalVal = it;
      if (m_valueMap.count(pGlobalVal)) {
        // Global variable is already mapped to destination module
        continue;
      }
      if (IsSrcValUsedInModule(pGlobalVal)) {
        // Global value is used in module import it without initialization
        ImportGlobalVariableDeclaration(pGlobalVal);
        if (pGlobalVal->hasInitializer()) {
          // Global value has initializer add it to globals list to import
          m_globalsToImport.push_back(pGlobalVal);
        }
      }
    }
  }

  bool BIImport::CollectSourceFunctionsToImport() {
    TFunctionsVec tmpFunctionsToImport;
    // Iterate over all functions in source module and check if any needs to be imported
    for (Module::iterator it = m_pSourceModule->begin(), e = m_pSourceModule->end(); it != e; ++it) {
      Function *pSrcFunc = it;
      if (MapAndImportFunctionDclIfNeeded(pSrcFunc, NULL)) {
        tmpFunctionsToImport.push_back(pSrcFunc);
      }
    }
    if(tmpFunctionsToImport.empty()) {
      // No new functions to import
      return false;
    }
    // We have new functions to import
    // Collect functions recursively used by these new functions
    CollectFunctionChainToImport(tmpFunctionsToImport);
    return true;
  }

  void BIImport::ImportGlobalVariablesInitializations() {
    // Loop over all of the globals to import and import their initialization
    for (TGlobalsVec::iterator it = m_globalsToImport.begin(),
        e = m_globalsToImport.end(); it != e; ++it) {
      GlobalVariable *pSrcGlobal = *it;
      assert(pSrcGlobal->hasInitializer() && "source global variable already inialized!");

      // Grab destination global variable.
      GlobalVariable *pDstGlobal = dyn_cast<GlobalVariable>(m_valueMap[pSrcGlobal]);
      assert(pDstGlobal != NULL && "global variable must be mapped to global variable");
      assert(!pDstGlobal->hasInitializer() && "global variable already has initialization");
      // Figure out what the initializer looks like in the dest module.
      pDstGlobal->setInitializer(MapValue(pSrcGlobal->getInitializer(), m_valueMap));
    }
  }

  void BIImport::ImportFunctionDefinitions() {
    // Loop over all of the functions to import and import their definition
    for (TFunctionsVec::iterator it = m_functionsToImport.begin(),
        e = m_functionsToImport.end(); it != e; ++it) {
      Function *pSrcFunction = *it;
      assert(pSrcFunction && !pSrcFunction->isDeclaration() && "source function has no definition");

      Function *pDstFunction = dyn_cast<Function>(m_valueMap[pSrcFunction]);
      assert(pDstFunction != NULL && "function must be mapped to function");
      assert(pDstFunction->isDeclaration() && "destination function is not a declaration");

      // Go through and convert function arguments over, remembering the mapping.
      Function::arg_iterator itDst = pDstFunction->arg_begin();
      Function::arg_iterator itSrc = pSrcFunction->arg_begin();
      Function::arg_iterator eSrc = pSrcFunction->arg_end();
      for (; itSrc != eSrc; ++itSrc, ++itDst) {
        // Copy the name over.
        itDst->setName(itSrc->getName());
        // Add a mapping to our mapping.
        m_valueMap[itSrc] = itDst;
      }

      // Clone the body of the function into the dest function.
      SmallVector<ReturnInst*, 8> Returns; // Ignore returns.
      CloneFunctionInto(pDstFunction, pSrcFunction, m_valueMap, false, Returns);

      // There is no need to map the arguments anymore.
      for (Function::arg_iterator it = pSrcFunction->arg_begin(), e = pSrcFunction->arg_end(); it != e; ++it) {
        m_valueMap.erase(it);
      }
    }
  }


  void BIImport::CollectFunctionChainToImport(TFunctionsVec &rootFunctionsToImport) {
    // Find all functions called by "root" functions.
    while (!rootFunctionsToImport.empty()) {
      // Move source function from root list to functions to import list.
      Function* pSrcFunc = rootFunctionsToImport.back();
      rootFunctionsToImport.pop_back();
      m_functionsToImport.push_back(pSrcFunc);

      // Get functions called by current source function
      TFunctionsVec calledFuncs;
      GetCalledFunctions(pSrcFunc, calledFuncs);

      // Loop over all called functions and check if any need to be imported
      TFunctionsVec::iterator calledFuncIt = calledFuncs.begin();
      TFunctionsVec::iterator calledFuncE = calledFuncs.end();
      for (; calledFuncIt != calledFuncE; ++calledFuncIt) {
        Function* pCalledFunc = *calledFuncIt;
        if (MapAndImportFunctionDclIfNeeded(pCalledFunc, NULL)) {
          // In this case we have a new function that need to be import and
          // was not handled yet, added it to the functions-to-imprt list
          rootFunctionsToImport.push_back(pCalledFunc);
        }
      }
    }
  }

  bool BIImport::MapAndImportFunctionDclIfNeeded(Function *pSrcFunc, Function *pDstFunc) {
    assert(pSrcFunc && "Function to import is a NULL");
    if (m_valueMap.count(pSrcFunc)) {
      // Source function declaration is already mapped to destination module
      return false;
    }
    bool isNeededByModule =
      (pDstFunc != NULL && IsDestValUsedInModule(pDstFunc)) ||
      (pDstFunc == NULL && IsSrcValUsedInModule(pSrcFunc));
    if (!isNeededByModule) {
      // No need to import source function, it is not needed by destination module
      return false;
    }
    if (!pDstFunc) {
      // Create declaration inside the destination module.
      pDstFunc = Function::Create(pSrcFunc->getFunctionType(), pSrcFunc->getLinkage(), pSrcFunc->getName(), m_pModule);
      pDstFunc->setAttributes(pSrcFunc->getAttributes());
    }
    if(pSrcFunc->isDeclaration() && pSrcFunc->isMaterializable()) {
      // Materialize function source function
      m_pSourceModule->Materialize(pSrcFunc);
    }
    // Map source function to its declaration in destination module.
    m_valueMap[pSrcFunc] = pDstFunc;
    // Return true only if source function is a definition that need to be imported
    return !pSrcFunc->isDeclaration();
  }

  void BIImport::ImportGlobalVariableDeclaration(GlobalVariable* pSrcGlobal) {
    GlobalVariable *pDstGlobal =
        new GlobalVariable(*m_pModule,
            pSrcGlobal->getType()->getElementType(),
            pSrcGlobal->isConstant(),
            pSrcGlobal->getLinkage(),
            0, // We only create global variable declaration, no init yet!
            pSrcGlobal->getName(),
            0,
            pSrcGlobal->getThreadLocalMode(),
            pSrcGlobal->getType()->getAddressSpace());

    // Propagate alignment, visibility and section info.
    pDstGlobal->copyAttributesFrom(pSrcGlobal);
    pDstGlobal->setAlignment(pSrcGlobal->getAlignment());

    m_valueMap[pSrcGlobal] = pDstGlobal;
  }

  void BIImport::GetCalledFunctions(const Function* pFunc, TFunctionsVec& calledFuncs) {
    TFunctionsSet visitedSet;
    // Iterate over function instructions and look for call instructions
    for (const_inst_iterator it = inst_begin(pFunc), e = inst_end(pFunc); it != e; ++it) {
      const CallInst *pInstCall = dyn_cast<CallInst>(&*it);
      if(!pInstCall) continue;
      Function* pCalledFunc = pInstCall->getCalledFunction();
      if(!pCalledFunc){
        // This case can occur only if CallInst is calling something other than LLVM function.
        // Thus, no need to handle this case - function casting is not allowed (and not expected!)
        continue;
      }
      if(visitedSet.count(pCalledFunc)) continue;

      visitedSet.insert(pCalledFunc);
      calledFuncs.push_back(pCalledFunc);
    }
  }

  bool BIImport::IsDestValUsedInModule(Value *pVal) {
    // Given value is assumed to be part of destination module
    assert(pVal && "Given value pointer is a NULL");
    // Iterate over function usages and check (recursively) if any is an instruction
    for (Value::use_iterator it = pVal->use_begin(), e = pVal->use_end(); it != e; ++it) {
      User* user = *it;
      if(isa<Instruction>(user)) {
        assert(cast<Instruction>(user)->getParent()->getParent()->getParent()
          == m_pModule && "value is assumed to be part of destination module");
        // found a function using this value that is needed to be import to destination module.
        return true;
      }
      if (IsDestValUsedInModule(user)) {
        return true;
      }
    }
    return false;
  }

  bool BIImport::IsSrcValUsedInModule(Value *pVal) {
    // Given value is assumed to be part of source module
    assert(pVal && "Given vlaue pointer is a NULL");
    // Iterate over value usages and check if any is part "needed to import" function
    for (Value::use_iterator it = pVal->use_begin(), e = pVal->use_end(); it != e; ++it) {
      User* user = *it;
      if (isa<Instruction>(user)) {
        Function* pFunc = cast<Instruction>(user)->getParent()->getParent();
        if (m_valueMap.count(pFunc)) {
          assert(cast<Instruction>(user)->getParent()->getParent()->getParent()
            == m_pSourceModule && "value is assumed to be part of source module");
          // found a function using this value that is needed to be import to destination module.
          return true;
        }
        // The usage is an instruction in a function that is not needed by the
        // destenation module, discard this usage and continue to check other usages.
        continue;
      }
      // If we reach here, means the usage is a global value or constant
      if (IsSrcValUsedInModule(user)) {
        return true;
      }
    }
    return false;
  }

} //namespace intel {

extern "C" llvm::ModulePass* createBuiltInImportPass() {
  return new intel::BIImport();
}
