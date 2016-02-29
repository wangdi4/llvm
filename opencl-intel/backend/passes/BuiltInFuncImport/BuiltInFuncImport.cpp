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
#include "llvm/IR/InstIterator.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/Debug.h>

#include <string>

using namespace llvm;
namespace intel {

  char BIImport::ID = 0;

  OCL_INITIALIZE_PASS_BEGIN(BIImport, "builtin-import", "Built-in function pass", false, true)
  OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
  OCL_INITIALIZE_PASS_END(BIImport, "builtin-import", "Built-in function pass", false, true)

  bool BIImport::runOnModule(Module &M) {
    BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
    m_runtimeModuleList = BLI.getBuiltinModules();
    if (m_runtimeModuleList.empty()) {
      // If there are no source modules, then nothing can be imported.
      return false;
    }

    // Initialize members
    m_pModule = &M;
    m_valueMap.clear();
    m_functionsToImport.clear();
    m_globalsToImport.clear();
    m_structTypeRemapper.clear();

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

  Function* BIImport::FindFunctionBodyInModules(const std::string& funcName) {
    SmallVector<Module*, 2>::iterator it = m_runtimeModuleList.begin(),
                                 itEnd = m_runtimeModuleList.end();
    for (; it != itEnd; ++it) {
      assert(*it != NULL && "NULL pointer detected in BIImport::FindFunctionInModules");

      Function* pRetFunction = (*it)->getFunction(funcName);

      // Test if the function body is contained in this module. Note that due to the lazy
      // parsing of built-in modules the function body might not be materialized yet and is reported
      // as declaration in that case. (This behaviour was fixed in LLVM 3.6)
      if (pRetFunction != NULL &&
        (pRetFunction->isMaterializable() || !pRetFunction->isDeclaration()) )
        return pRetFunction;
    }
    return NULL;
  }

  void BIImport::CollectRootFunctionsToImport() {
    assert(m_functionsToImport.empty() && "Starting to fill a non-empty list of all functions.");

    TFunctionsVec rootFunctionsToImport;

    // Find all "root" functions.
    for (Module::iterator it = m_pModule->begin(), e = m_pModule->end(); it != e; ++it) {
      Function *pDstFunc = it;
      std::string funcName = pDstFunc->getName().str();
      if (pDstFunc->isDeclaration()) {
        Function* pSrcFunc = FindFunctionBodyInModules(funcName);
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
    for (SmallVector<Module*, 2>::iterator pSourceModuleIter = m_runtimeModuleList.begin();
         pSourceModuleIter != m_runtimeModuleList.end();
         pSourceModuleIter++) {
      Module::GlobalListType &lstGlobals = (*pSourceModuleIter)->getGlobalList();
      // Iterate over all globals in source module and check if any needs to be imported
      for (Module::GlobalListType::iterator it = lstGlobals.begin(), e = lstGlobals.end(); it != e; ++it) {
        GlobalVariable* pGlobalVal = it;
        if (m_valueMap.count(pGlobalVal)) {
          // Global variable is already mapped to destination module
          continue;
        }
        if (IsSrcValUsedInModule(pGlobalVal)) {
          // Global value is used in module import it without initialization
          GlobalVariable* pGlobalValDecl = ImportGlobalVariableDeclaration(pGlobalVal);
          // Iterate over builtin modules and map Global Values with the name of pGlobalVal
          for (SmallVector<Module*, 2>::iterator pSourceModuleIter = m_runtimeModuleList.begin();
            pSourceModuleIter != m_runtimeModuleList.end();
            pSourceModuleIter++) {
              GlobalVariable* bltnGlobalVariable = (*pSourceModuleIter)->getGlobalVariable(pGlobalVal->getName());
              if (bltnGlobalVariable && bltnGlobalVariable->hasInitializer()) {
                  m_globalsToImport.push_back(bltnGlobalVariable);
              }
              m_valueMap[bltnGlobalVariable] = pGlobalValDecl;
          }
        }
      }
    }
  }

  bool BIImport::CollectSourceFunctionsToImport() {
    TFunctionsVec tmpFunctionsToImport;
    for (SmallVector<Module*, 2>::iterator pSourceModuleIter = m_runtimeModuleList.begin();
         pSourceModuleIter != m_runtimeModuleList.end();
         pSourceModuleIter++) {
      // Iterate over all functions in source module and check if any needs to be imported
      for (Module::iterator it = (*pSourceModuleIter)->begin(), e = (*pSourceModuleIter)->end(); it != e; ++it) {
        Function *pSrcFunc = it;
        // iterating only by function definitions or materialazible functions
        if ((!pSrcFunc->isDeclaration() || pSrcFunc->isMaterializable())) {
          Function* pDstFunction = m_pModule->getFunction(pSrcFunc->getName());
          if((!pDstFunction || pDstFunction->isDeclaration()) &&
             MapAndImportFunctionDclIfNeeded(pSrcFunc, pDstFunction))
            tmpFunctionsToImport.push_back(pSrcFunc);
        }
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
      // Propagate alignment, visibility and section info.
      pDstGlobal->copyAttributesFrom(pSrcGlobal);
      pDstGlobal->setAlignment(pSrcGlobal->getAlignment());

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
        // Add mapping of argument types.
      }

      // Clone the body of the function into the dest function.
      SmallVector<ReturnInst*, 8> Returns; // Ignore returns.
      // At this point m_structTypeRemapper must be correctly filled.
      CloneFunctionInto(pDstFunction, pSrcFunction, m_valueMap, false, Returns, "", 0, &m_structTypeRemapper);
      // Allow removal of function from module after it is inlined
      pDstFunction->setLinkage(GlobalVariable::LinkOnceODRLinkage);

      // Workaround for current LLVM issue: LLVM merges attributes of pDstFunction
      // with pSrcFunction, possibly causing a collision between readonly
      // and readnone attributes
      if (pDstFunction->getAttributes().hasAttribute(AttributeSet::FunctionIndex, Attribute::ReadNone) &&
       pDstFunction->getAttributes().hasAttribute(AttributeSet::FunctionIndex, Attribute::ReadOnly)) {
        AttrBuilder B;
        B.addAttribute(Attribute::ReadOnly);
        pDstFunction->removeAttributes(AttributeSet::FunctionIndex,
                                       AttributeSet::get(pDstFunction->getContext(),
                                       AttributeSet::FunctionIndex, B));
      }

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
      // assert that a function body was materialized (and thus is not a declaration)
      // or can be materialized.
      assert((!pSrcFunc->isDeclaration() || pSrcFunc->isMaterializable())
        && "A function without a body was detected");
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

        // Check if a function with the same name is defined in the destination
        // module (for example if a user has defined a function w\ the same name as
        // a helper function in built-in modules).
        // In this case a new function must be created and added to the mapping.
        // NOTE: this may create redundant declarations of the same function
        //       but only the latest one will be used in ImportFunctionDefinitions
        Function* pDstFunction = m_pModule->getFunction(pCalledFunc->getName());
        if(pDstFunction && !pDstFunction->isDeclaration()) {
          pDstFunction = NULL;
        }
        if (MapAndImportFunctionDclIfNeeded(pCalledFunc, pDstFunction)) {
          // In this case we have a new function that need to be import and
          // was not handled yet, added it to the functions-to-imprt list
          rootFunctionsToImport.push_back(pCalledFunc);
        }
      }
    }
  }

  bool BIImport::MapAndImportFunctionDclIfNeeded(Function* &pSrcFunc, Function *pDstFunc) {
    assert(pSrcFunc && "Function to import is a NULL");
    if (m_valueMap.count(pSrcFunc)) {
      // Source function declaration is already mapped to destination module
      return false;
    }
    // It is crucial to check first if the source function is needed because the destination
    // function declaration could be created on a previous iteration and not yet used by a
    // destination module.
    bool isNeededByModule =
      IsSrcValUsedInModule(pSrcFunc) || (pDstFunc != NULL && IsDestValUsedInModule(pDstFunc));
    if (!isNeededByModule) {
      // No need to import source function, it is not needed by destination module
      return false;
    }
    // Find function with body for pSrcFunc.
    if (pDstFunc) {
      // Remember the mapping of struct types.
      m_structTypeRemapper.insert(pSrcFunc, pDstFunc);
    } else {
      // Create a new function decalaration in the destination module
      // only if there is no declaration for that name already created.
      // At this point m_structTypeRemapper must contain correct mapping
      // based on the set of the root functions.
      pDstFunc = Function::Create(m_structTypeRemapper.remapFunctionType(pSrcFunc),
                                  pSrcFunc->getLinkage(), pSrcFunc->getName(), m_pModule);
      pDstFunc->setAttributes(pSrcFunc->getAttributes());
    }

    // Map source function and its declaration in the builtin modules
    // to its declaration in destination (user) module.
    for (SmallVector<Module*, 2>::iterator pSourceModuleIter = m_runtimeModuleList.begin();
       pSourceModuleIter != m_runtimeModuleList.end();
       pSourceModuleIter++) {
      Function* bltFunction = (*pSourceModuleIter)->getFunction(pSrcFunc->getName());
      if (bltFunction) {
        m_valueMap[bltFunction] = pDstFunc;
      }
    }

    // Return true only if source function is a definition that need to be imported
    pSrcFunc = FindFunctionBodyInModules(pSrcFunc->getName());
    if(pSrcFunc && pDstFunc->isDeclaration()) {
      // Materialize source function
      (void)pSrcFunc->materialize();
      return true;
    }
    return false;
  }

  GlobalVariable* BIImport::ImportGlobalVariableDeclaration(GlobalVariable* pSrcGlobal) {
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

    return pDstGlobal;
  }


  // this function replaces keyword "shared" in the builtin name by current CPU prefix, for example:
  // if CPU is l9, __ocl_svml_shared_acos1f to be changed to __ocl_svml_l9_acos1f
  static void UpdateSvmlBuiltinName(Function* fn, const char* pCPUPrefix){

    llvm::StringRef fName = fn->getName();
    if (fName.startswith("__ocl_svml_shared")) {
        std::string s = fName.str();
        s.replace(11,6,pCPUPrefix);
        fn->setName(s);
    }
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

      assert(!m_cpuPrefix.empty() && "m_cpuPrefix is empty");
      UpdateSvmlBuiltinName(pCalledFunc,m_cpuPrefix.c_str());

      visitedSet.insert(pCalledFunc);

      calledFuncs.push_back(pCalledFunc);
    }
  }

  bool BIImport::IsDestValUsedInModule(Value *pVal) {
    // Given value is assumed to be part of destination module
    assert(pVal && "Given value pointer is a NULL");
    // Iterate over function usages and check (recursively) if any is an instruction
    for (Value::user_iterator it = pVal->user_begin(), e = pVal->user_end(); it != e; ++it) {
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
    for (Value::user_iterator it = pVal->user_begin(), e = pVal->user_end(); it != e; ++it) {
      User* user = *it;
      if (isa<Instruction>(user)) {
        Function* pFunc = cast<Instruction>(user)->getParent()->getParent();
        if (m_valueMap.count(pFunc)) {
            assert(std::find( m_runtimeModuleList.begin(), m_runtimeModuleList.end(),
                cast<Instruction>(user)->getParent()->getParent()->getParent() )
                != m_runtimeModuleList.end() && "value is assumed to be part of source modules list");
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

  ////////////////////////////////
  // StructTypeRemapper methods //
  ////////////////////////////////

  // Convert a pointer type to a structure type it points to.
  StructType * StructTypeRemapper::ptrToStruct(Type *T) {
    if (PointerType *PT = dyn_cast<PointerType>(T)) {
      // Handle also pointer to pointer to ...
      while (PointerType *PT2 = dyn_cast<PointerType>(PT->getElementType()))
        PT = PT2;
      Type *ET = PT->getElementType();
      return dyn_cast<StructType>(ET);
    }
    return NULL;
  }

  // Recursively construct a pointer of arbitrary dimension to DstST type
  // using the SrcTy as a pattern.
  Type* StructTypeRemapper::constructPtr(Type * SrcTy, StructType * DstST) {
    PointerType * SrcPT = cast<PointerType>(SrcTy);

    if(isa<StructType>(SrcPT->getElementType())) {
      return PointerType::get(DstST, SrcPT->getAddressSpace());

    } else {
      Type * RetTy = constructPtr(SrcPT->getElementType(), DstST);
      return PointerType::get(RetTy, SrcPT->getAddressSpace());
    }
  }

  // Insert struct types mapping into m_TypeMap.
  void StructTypeRemapper::insert(Type * SrcTy, Type * DstTy) {
    if(SrcTy == DstTy) return; // No need to map equal types.

    StructType *SrcST = ptrToStruct(SrcTy);
    if (!SrcST) return; // Not a struct type.
    if(m_TypeMap.count(SrcST) != 0) return; // Type is already in the map.

    StructType *DstST = ptrToStruct(DstTy);
    assert(DstST && "Destination type must be a struct too.");

    // It is not guaranteed that we have here an original type name without
    // a number appended. Take it into account.
    StringRef TypeName = SrcST->getName().count('.') < DstST->getName().count('.') ?
                         SrcST->getName() :
                         SrcST->getName().rsplit('.').first;
    assert(SrcST->getName().startswith("struct.ocl_pipe") || // CSSD100017148 workaround for the pipe built-ins
           (SrcST->getName().startswith(TypeName) && DstST->getName().startswith(TypeName) &&
           "It is illegal to map different struct types."));
    (void)TypeName; // Avoid warnings about unused variable in the Release build.

    m_TypeMap[SrcST] = DstST;
  }

  // Remap any pointer to a struct type found in the m_TypeMap.
  // Or return the same type if no mapping defined.
  Type *  StructTypeRemapper::remapType (Type *SrcTy) {
    StructType * SrcST = ptrToStruct(SrcTy);
    if(!SrcST) return SrcTy;

    TypeMap::iterator it = m_TypeMap.find(SrcST);
    if(it == m_TypeMap.end()) return SrcTy;

    return constructPtr(SrcTy, cast<StructType>(it->second));
  }

  // Insert type mapping based on the difference of the source and destination
  // function types.
  void StructTypeRemapper::insert(Function *pSrcFunc, Function *pDstFunc) {
    // Add mapping of return types
    this->insert(pSrcFunc->getReturnType(), pDstFunc->getReturnType());
    // Go through and remember mapping of function arguments.
    Function::arg_iterator itDst = pDstFunc->arg_begin();
    Function::arg_iterator itSrc = pSrcFunc->arg_begin();
    Function::arg_iterator eSrc  = pSrcFunc->arg_end();
    for (; itSrc != eSrc; ++itSrc, ++itDst) {
      this->insert(itSrc->getType(), itDst->getType());
    }
  }

  // Create a new function type by replacing struct types.
  FunctionType * StructTypeRemapper::remapFunctionType(Function *pSrcFunc) {
    FunctionType* srcFuncType = pSrcFunc->getFunctionType();

    // Remap return type.
    Type * result = this->remapType(pSrcFunc->getReturnType());
    // Remap argument types.
    llvm::SmallVector<Type*, 16> args;
    for(unsigned parIdx = 0; parIdx < srcFuncType->getNumParams(); ++parIdx) {
      args.push_back(this->remapType(srcFuncType->getParamType(parIdx)));
    }

    return FunctionType::get(result, args, srcFuncType->isVarArg());
  }
} //namespace intel {

extern "C" llvm::ModulePass* createBuiltInImportPass(const char* CPUPrefix) {
  return new intel::BIImport(CPUPrefix);
}
