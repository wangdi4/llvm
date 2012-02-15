/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  BuiltInFunctionImport.cpp

\*****************************************************************************/

// This pass imports built-in functions from builtins module to destination module.

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Transforms/Utils/ValueMapper.h>
#include <llvm/Instruction.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <list>
#include <map>
#include <set>
#include <string>

using namespace llvm;
using namespace std;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BIImport : public ModulePass
{
protected:

  // Type used to hold a set of Functions and augment it during traversal.
  typedef std::vector<const llvm::Function*>             TFunctionsVec;

  // Types used to map all callee Functions to a caller Function.
  typedef std::list<const llvm::Function*>               TFunctionsLst;
  typedef std::map<const llvm::Function*, TFunctionsLst> TFunctionUsageMap;

  // Types used to map all Globals used by a Function.
  typedef std::list<const llvm::GlobalVariable*>         TGlobalsLst;
  typedef std::map<const llvm::Function*, TGlobalsLst>   TGlobalUsageMap;

public:
  BIImport(Module* pRTModule) : ModulePass(ID), m_pRTModule(pRTModule) {
    BuildRTModuleFunc2Funcs();
    BuildRTModuleFunc2Globals();
  }

  bool runOnModule(Module &M);

protected:

  void BuildRTModuleFunc2Funcs();

  void BuildRTModuleFunc2Globals();
  
  void CollectFuncs2Import(Module*, ValueToValueMapTy&, TFunctionsVec&);

  void ImportGlobals(Module*, TFunctionsVec&, ValueToValueMapTy&);

  void ImportFunctions(Module*, TFunctionsVec&, ValueToValueMapTy&);

  void ImportFunctionArrays(Module*, TFunctionsVec&, ValueToValueMapTy&);

protected:

  static char ID; // Pass identification, replacement for typeid.

  Module* m_pRTModule;

  TFunctionUsageMap  m_mapFunc2Fnc;

  TGlobalUsageMap    m_mapFunc2Glb;
};

char BIImport::ID = 0;

ModulePass *createBuiltInImportPass(Module* pRTModule) {
  return new BIImport(pRTModule);
}

/// \brief First find all "root" functions that are only declared in destination module and defined in builtins module,
///        excluding those whose name starts with "get_". Then scan all the functions which they call, iteratively.
///        Make sure each such called function has a declaration in the destination module.
///        Add it to list of all functions to be imported, unless it's an intrinsic.
///        Add the builtin-module-definition -to- destination-module-declation to valueMap.
///
/// \param pModule The destination module.
///
/// \param valueMap In/Out: maps Functions (and GlobalVariables) in builtins module
///        to their counterpart in the destination module. Augmented with the functions encountered.
/// \param allFuncs2Import Out: contains all functions to be imported, i.e., all root functions
///        plus any (non intrinsic) functions they call, iteratively.
void BIImport::CollectFuncs2Import(Module* pModule, ValueToValueMapTy& valueMap, TFunctionsVec& allFuncs2Import)
{
  assert(allFuncs2Import.empty() && "Starting to fill a non-empty list of all functions.");

  TFunctionsVec tmpFuncs2Import;

  // Find all "root" functions.
  for (Module::iterator it = pModule->begin(), e = pModule->end(); it != e; ++it)
  {
    std::string pFuncName = it->getNameStr();
    if (it->isDeclaration() && (pFuncName.substr(0,4) != "get_"))
    {
      Function* pImpFunc = m_pRTModule->getFunction(pFuncName);
      if (pImpFunc && !pImpFunc->isDeclaration())
      {
        // Map built-in function to its declaration in destination module.
        valueMap[pImpFunc] = it;
        tmpFuncs2Import.push_back(pImpFunc);
      }
    }
  }

  // Find all functions called by "root" functions.
  while (!tmpFuncs2Import.empty())
  {
    // Move first function from tmp to all functions lists.
    const Function* impF = tmpFuncs2Import.back();
    tmpFuncs2Import.pop_back();
    allFuncs2Import.push_back(impF);

    // Check all functions called by this first function.
    TFunctionUsageMap::iterator calledFunc2FncIt = m_mapFunc2Fnc.find(impF);
    if (calledFunc2FncIt == m_mapFunc2Fnc.end()) continue;

    TFunctionsLst& calledFuncs = calledFunc2FncIt->second;

    TFunctionsLst::iterator calledFuncIt, calledFuncE;
    for (calledFuncIt = calledFuncs.begin(), calledFuncE = calledFuncs.end();
         calledFuncIt != calledFuncE; ++calledFuncIt)
    {
      const Function* calledFunc = *calledFuncIt;
      
      if (valueMap.count(calledFunc)) continue;  // We already mapped this function.

      Function* calledFuncDest = pModule->getFunction(calledFunc->getName());
      // Check if function declaration already exists in the destination module.
      if (calledFuncDest == NULL)
      {
        // Create declaration inside the destination module.
        calledFuncDest = Function::Create(calledFunc->getFunctionType(),
                    calledFunc->getLinkage(),
                    calledFunc->getName(), pModule);
                    calledFuncDest->setAttributes(calledFunc->getAttributes());
      }
      if (calledFuncDest->isDeclaration() && !calledFunc->isDeclaration())
      {
        // We need to import the called function too.
        tmpFuncs2Import.push_back(calledFunc);
      }

      valueMap[calledFunc] = calledFuncDest;
    }
  }
}


/// \brief Scan all global variables used by functions to be imported.
///        Import them into the destination module.
///        Add the builtin-module-global -to- destination-module-global mapping into valueMap.
///
/// \param pModule The destination module.
///
/// \param allFuncs2Import In: contains all functions to be imported.
///
/// \param valueMap In/Out: maps elements (Functions and GlobalVariables) in builtins module
///                 to their counterpart in the destination module.
void BIImport::ImportGlobals(Module* pModule, TFunctionsVec& allFuncs2Import, ValueToValueMapTy& valueMap)
{
  // Iterate through all functions to import, importing the globals they use and marking them in valueMap.
  TFunctionsVec::iterator func2ImportIt, func2ImportE;
  for (func2ImportIt = allFuncs2Import.begin(), func2ImportE = allFuncs2Import.end();
       func2ImportIt != func2ImportE; ++func2ImportIt)
  {
    // Check all globals used by function to import, if any.
    TGlobalUsageMap::iterator func2GlbsIt = m_mapFunc2Glb.find(*func2ImportIt);
    if (func2GlbsIt == m_mapFunc2Glb.end()) continue;

    // Get list of all used global variables.
    TGlobalsLst& globalsLst = func2GlbsIt->second;
    TGlobalsLst::iterator globalsIt, globalsE;
    for (globalsIt = globalsLst.begin(), globalsE = globalsLst.end(); globalsIt != globalsE; ++globalsIt)
    {
      GlobalVariable* usedGlobal = const_cast<GlobalVariable*>(*globalsIt);

	  if (valueMap.count(usedGlobal)) continue; // We already mapped this global.

      GlobalVariable *usedGlobalDest =
                     new GlobalVariable(*pModule, usedGlobal->getType()->getElementType(),
                     usedGlobal->isConstant(), GlobalValue::PrivateLinkage, usedGlobal->getInitializer(),
                     usedGlobal->getName(), 0, false,
                     usedGlobal->getType()->getAddressSpace());
 
      // Propagate alignment, visibility and section info.
      usedGlobalDest->copyAttributesFrom(usedGlobal);
      usedGlobalDest->setAlignment(usedGlobal->getAlignment());

      valueMap[usedGlobal] = usedGlobalDest;
	}
  }
}

/// \brief Iterate over all functions to import, and perform the actual importation.
///
/// \param pModule The destination module to import to.
///
/// \param allFuncs2Import Set of functions of builtin module to import.
///
/// \param valueMap Maps each Function (definition) in builtin module to the Function (declaration) in the
///        destination module (among other mappings). Assumed to contain a map of each function to be imported,
///        over to a declaration in the destination module.
void BIImport::ImportFunctions(Module* pModule, TFunctionsVec& allFuncs2Import, ValueToValueMapTy& valueMap)
{
  TFunctionsVec::iterator impIt, impE;
  for (impIt = allFuncs2Import.begin(), impE = allFuncs2Import.end(); impIt != impE; ++impIt)
  {
    const Function* origF = *impIt;
    ValueToValueMapTy::iterator newFs = valueMap.find(origF);
    assert (newFs != valueMap.end() && "Missing clone of function.");
    assert (isa<Function>(newFs->second) && "Function in list for importing not mapped to destination function");
    Function* Dst = cast<Function>(newFs->second);

	// Clone the original function.
    Function* Src = CloneFunction(origF, NULL);
    Dst->setAttributes(origF->getAttributes());

    // Following is taken from ModuleLinker::linkFunctionBody(Function *Dst, Function *Src)
    // in order to feed an external ValueMap.

    assert(Src && Dst && Dst->isDeclaration() && !Src->isDeclaration());

    // Go through and convert function arguments over, remembering the mapping.
    Function::arg_iterator DI = Dst->arg_begin();
    for (Function::arg_iterator I = Src->arg_begin(), E = Src->arg_end();
         I != E; ++I, ++DI) {
      DI->setName(I->getName());  // Copy the name over.

      // Add a mapping to our mapping.
      valueMap[I] = DI;
    }

    assert(DI == Dst->arg_end() && 
           "Src and Dst have a different number of args");

    // Clone the body of the function into the dest function.
    SmallVector<ReturnInst*, 8> Returns; // Ignore returns.
    CloneFunctionInto(Dst, Src, valueMap, false, Returns);
  
    // There is no need to map the arguments anymore.
    for (Function::arg_iterator I = Src->arg_begin(), E = Src->arg_end();
         I != E; ++I)
      valueMap.erase(I);

	delete Src;
  }
}

/// \brief Scan all arrays that point to imported functions, and import them.
///        For every array A that holds pointers to functions F that have been cloned,
///        create a new array A' holding pointers to the clones F', and modify every
///        cloned global variable G' to use A' instead of A.
///        Note the both A and F belong to builtins module, whereas A', F' and G' belong
///        to the destination module.
///
/// \param pModule The destination module.
///
/// \param allFuncs2Import In: contains all functions to be imported.
///
/// \param valueMap In: maps elements (Functions and GlobalVariables) in builtins module
///                 to their counterpart in the destination module.
void BIImport::ImportFunctionArrays(Module* pModule, TFunctionsVec& allFuncs2Import, ValueToValueMapTy& valueMap)
{
  typedef std::set<const ConstantArray *> TConstantArrays;

  TConstantArrays origArrays;

  // Check if any function F that we import is pointed to by an array A (both of the builtins module),
  // and collect all such arrays.
  for (TFunctionsVec::iterator impIt =  allFuncs2Import.begin(), impE =  allFuncs2Import.end(); impIt != impE; ++impIt)
  {
    for (Value::const_use_iterator I = (*impIt)->use_begin(), E = (*impIt)->use_end(); I != E; ++I)
    {
      if (isa<ConstantArray>(*I)) origArrays.insert(cast<ConstantArray>(*I));
      else assert (isa<Instruction>(*I) &&
                   "The only non-Instruction allowed to use an imported function is a ConstantArray");
    }
  }

  // Clone every array A pointing to functions F that we import, such that A' point to the corresponding functions F',
  // and update every GlobalVariable G' using A to use A' instead (where A', F' and G' belong to destination module).
  for (TConstantArrays::iterator conIt = origArrays.begin(), conE = origArrays.end(); conIt != conE; ++conIt)
  {
    const ConstantArray * origArray = *conIt;
    std::vector<Constant*> elements;
    for (User::const_op_iterator i = origArray->op_begin(), e = origArray->op_end(); i != e; ++i)
    {
      assert (isa<Function>(*i) && "Constant Array of functions contains non-Function operand");
      ValueToValueMapTy::iterator newFs = valueMap.find(cast<Function>(*i));
      assert (newFs != valueMap.end() && "Missing clone of function.");
      assert (isa<Function>(newFs->second) && "Function mapped to non-Function.");
      elements.push_back(cast<Function>(newFs->second));
    }

    Constant * newArray = ConstantArray::get(origArray->getType(), elements);
    for (Value::const_use_iterator I1 = origArray->use_begin(), E1 = origArray->use_end(); I1 != E1; ++I1)
    {
      assert (isa<GlobalVariable>(*I1) && "ConstantArray holding imported function must be used by GlobalVariables only");
      const GlobalVariable * GV = cast<GlobalVariable>(*I1);
      if (GV->getParent() == pModule) ((GlobalVariable*)GV)->replaceUsesOfWith ((ConstantArray*)origArray, newArray);
    }
  }
}

/// \brief Main entry point. Find all builtins to import, and import them along with callees and globals.
///
/// \param M The destination module.
bool BIImport::runOnModule(Module &M)
{
  if (m_pRTModule == NULL) return false;

  ValueToValueMapTy valueMap;

  TFunctionsVec funcs2Import;

  // Collect functions to import.
  CollectFuncs2Import(&M, valueMap, funcs2Import);

  if (funcs2Import.empty()) return false;  // Nothing to import.

  // Import global variables required by all functions to be imported.
  ImportGlobals(&M, funcs2Import, valueMap);

  // Import functions.
  ImportFunctions(&M, funcs2Import, valueMap);

  // Import arrays holding pointers to functions that have been imported. 
  ImportFunctionArrays(&M, funcs2Import, valueMap);
  return true;
}

/// \brief Enumerate globals and the functions that use them, building the function-to-globals map.
///        Later we will export the needed globals to a destination module.
void BIImport::BuildRTModuleFunc2Globals()
{
  Module::GlobalListType &lstGlobals = m_pRTModule->getGlobalList();
  for (Module::GlobalListType::iterator it = lstGlobals.begin(), e = lstGlobals.end(); it != e; ++it)
  {
    GlobalVariable* pVal = it;
    GlobalValue::use_iterator use_it, use_e;
    for (use_it = pVal->use_begin(), use_e = pVal->use_end(); use_it != use_e; ++use_it)
    {
      User* user = *use_it;
      if (isa<Instruction>(user))
      {
        Function* pFunc = cast<Instruction>(user)->getParent()->getParent();
        m_mapFunc2Glb[pFunc].push_back(pVal);
        continue;
      }
      assert (isa<ConstantExpr>(user) &&
              "Global Variable may be used only in Instruction or Constant Expression");
      for (Value::use_iterator it = user->use_begin(), e = user->use_end(); it != e; ++it)
      {
        assert (isa<Instruction>(*it) &&
                "Global variable used by constant expression can in turn be used by instruction only");
        Function* pFunc = cast<Instruction>(*it)->getParent()->getParent();
        m_mapFunc2Glb[pFunc].push_back(pVal);
      }
    }
  }
}

/// \brief Enumerate functions and the functions that call them, building the function-to-functions map.
///        Later we will export the called functions along with their callers to a destination module.
void BIImport::BuildRTModuleFunc2Funcs()
{
  for (Module::iterator extIt = m_pRTModule->begin(), extE = m_pRTModule->end(); extIt != extE; ++extIt)
  {
    Function* pVal = extIt;
    Function::use_iterator use_it, use_e;
    for (use_it = pVal->use_begin(), use_e = pVal->use_end(); use_it != use_e; ++use_it)
    {
      if (isa<Instruction>(*use_it))        // Direct call
      {
        Function* pFunc = cast<Instruction>(*use_it)->getParent()->getParent();
        m_mapFunc2Fnc[pFunc].push_back(pVal);
      }
      else       // Indirect call; check and allow only a specific pattern:
      {
        assert (isa<ConstantArray>(*use_it) && "Pointers to functions allowed only in ConstantArrays.");

        for (Value::use_iterator I = use_it->use_begin(), E = use_it->use_end(); I != E; ++I)
        {
          assert (isa<GlobalVariable>(*I) && "Only GlobalVariables can use ConstantArray of functions.");

          // Global is added to m_mapFunc2Glb by BuildRTModuleFunc2Globals.
          for (Value::use_iterator I1 = I->use_begin(), E1 = I->use_end(); I1 != E1; ++I1)
          {
            assert (isa<Instruction>(*I1) && "Only Instruction can use a GlobalVariable using a function array.");
            Function* pFunc = cast<Instruction>(*I1)->getParent()->getParent();
            m_mapFunc2Fnc[pFunc].push_back(pVal);
          }
        }
      }
    }
  }
}
}}}
