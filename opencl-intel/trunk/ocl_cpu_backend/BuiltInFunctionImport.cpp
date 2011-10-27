/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

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

// This pass is used for import of built-in functions from runtime module

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
#include <string>

using namespace llvm;
using namespace std;

bool LinkFunctionBody(Function *Dest, Function *Src,
                             ValueToValueMapTy &ValueMap,
                             std::string *Err);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BIImport : public ModulePass
{
public:
  BIImport(Module* pRTModule) : ModulePass(ID), m_pRTModule(pRTModule) {
    ParseRTModule();
  }

  // doPassInitialization - For this pass, it removes global symbol table
  // entries for primitive types.  These are never used for linking in GCC and
  // they make the output uglier to look at, so we nuke them.
  //
  // Also, initialize instance variables.
  //
  bool runOnModule(Module &M);

protected:
	static char ID; // Pass identification, replacement for typeid

	Module*	m_pRTModule;

	typedef std::list<std::pair<llvm::Function*,const llvm::Function*> >			TImportListType;
	typedef std::map<const llvm::Value*, llvm::Value*>						TValueMap;
	// This function goes through all functions/global variable than must be imported
	// The global values are added as external symbols, and mapped to point out to original variable
	bool Preporcessor(Module* pModule, TImportListType& impLst, ValueToValueMapTy& ValueMap);
    void ParseRTModule();

    // Updates a map that maps functions to the global values they use
    // pVal - a global value
    // user - a direct user of pVal
    void UpdateGlobalsMap(GlobalValue* pVal, User * user);

    typedef	std::list<const llvm::GlobalValue*>				TGlobalsLst;
    typedef std::map<const llvm::Function*, TGlobalsLst>	TGlobalUsageMap;
    TGlobalUsageMap											m_mapFunc2Glb;
};

char BIImport::ID = 0;

ModulePass *createBuiltInImportPass(Module* pRTModule) {
	return new BIImport(pRTModule);
}

bool BIImport::Preporcessor(Module* pModule, TImportListType& impLst, ValueToValueMapTy& ValueMap)
{
	TImportListType::iterator impIt, impE;
	TGlobalUsageMap&	glbMap = m_mapFunc2Glb;

	// Iterate through imported functions and export all relevant global values
	for (impIt=impLst.begin(); impIt!= impLst.end(); ++impIt)
	{
		// check all globals used by imported function
		TGlobalUsageMap::iterator mapIt = glbMap.find(impIt->second);
		if ( mapIt == glbMap.end() )
		{
			continue;
		}
		// Get list of all used variables
		TGlobalsLst& glbLst = mapIt->second;
		TGlobalsLst::iterator glbIt, glbE;
		for (glbIt=glbLst.begin(), glbE = glbLst.end(); glbIt!=glbE; ++glbIt)
		{
			const GlobalVariable* SGV = dyn_cast<GlobalVariable>(*glbIt);
			if ( SGV )
			{
				if ( ValueMap.find(SGV) != ValueMap.end() )
				{
					// We already mapped this value
					continue;
				}
				GlobalVariable *NewDGV =
					new GlobalVariable(*pModule, SGV->getType()->getElementType(),
                    SGV->isConstant(), GlobalValue::PrivateLinkage, SGV->getInitializer(),
					SGV->getName(), 0, false,
					SGV->getType()->getAddressSpace());

				// Propagate alignment, visibility and section info.
				NewDGV->copyAttributesFrom(SGV);
				NewDGV->setAlignment(SGV->getAlignment());

				// Make sure to remember this mapping.
				ValueMap[SGV] = NewDGV;
				continue;
			}
			const Function* SFn = dyn_cast<Function>(*glbIt);
			if ( SFn )
			{
				if ( ValueMap.find(SFn) != ValueMap.end() )
				{
					// We already mapped this function
					continue;
				}
				Function* DFn = pModule->getFunction(SFn->getName());
				// Check if function declaration already exists in the module
				if ( NULL == DFn )
				{
					// Create declaration in side the module
					DFn = Function::Create(SFn->getFunctionType(),
						SFn->getLinkage(),
						SFn->getName(), pModule);
					DFn->setAttributes(SFn->getAttributes());
				}
				if ( DFn->isDeclaration() && !SFn->isDeclaration() )
				{
					// We need import the called function too
					impLst.push_back(make_pair<Function*, const Function*>(DFn, SFn));
				}

				ValueMap[SFn] = DFn;
			}
		}

	}

	return true;
}

bool BIImport::runOnModule(Module &M)
{
	// Check if RT library module presents
	if ( NULL == m_pRTModule )
		return false;

	// Mapped values
	ValueToValueMapTy ValueMap;

	// Create list of imported functions
	TImportListType lstImported;

	Module::iterator it,e;
	for (it = M.begin(), e=M.end(); it != e; ++it)
	{
		std::string pFuncName = it->getNameStr();
		if ( it->isDeclaration() && (pFuncName.substr(0,4) != "get_") )
		{
			Function* pImpFunc = m_pRTModule->getFunction(pFuncName);
			// Import function only if exists, not a declaration and could be inlined
			if ( pImpFunc && !pImpFunc->isDeclaration() && !pImpFunc->hasFnAttr(Attribute::NoInline) )
			{
				// Add current declaration(first) and built-in functions to be exported (second)
				lstImported.push_back(pair<Function*, Function*>(it, pImpFunc));
			}
		}
	}

	if ( lstImported.empty() )
	{
		return false;			// Nothing to import
	}

	// Now we need to import global variable required by the new functions
	// Also need to add other functions, which are not in-lined
	// Declare them as external
	Preporcessor(&M, lstImported, ValueMap);

	// Now we need to copy required declarations
	TImportListType::iterator impIt, impE;

	// Now import functions
	for (impIt = lstImported.begin(), impE=lstImported.end(); impIt != impE; ++impIt)
	{
		if ( !impIt->first->isDeclaration() )
			continue;
		// Clone the original function
		Function* pClone = CloneFunction(impIt->second, NULL);
		// Copy attributes
		impIt->first->setAttributes(impIt->second->getAttributes());

		// Only provide the function body if there isn't one already.
		LinkFunctionBody( impIt->first, pClone, ValueMap, NULL );
		delete pClone;

	}

	return true;
}

void BIImport::UpdateGlobalsMap(GlobalValue* pVal, User * user)
{
    // llvm::Instruction, llvm::Operator and llvm::ConstantExpr are the only possible subtypes of llvm::user
    if ( isa<Instruction>(user) )
    {
        Function* pFunc = cast<Instruction>(user)->getParent()->getParent();
        m_mapFunc2Glb[pFunc].push_back(pVal);
    }
    else // llvm::Operator or llvm::ConstantExpr
    {
        Value::use_iterator it;

        for ( it = user->use_begin(); it != user->use_end(); ++it )
        {
            UpdateGlobalsMap(pVal, *it);
        }
    }
}


void BIImport::ParseRTModule()
{
  // Built usage table for globals,
  // Later we need to exported the globals to a kernel module

  Module* pModule = m_pRTModule;

  // Enumerate globals and build function to global map
  Module::GlobalListType &lstGlobals = pModule->getGlobalList();
  for ( Module::GlobalListType::iterator it = lstGlobals.begin(), e = lstGlobals.end(); it != e; ++it)
  {
    GlobalVariable* pVal = it;

    GlobalValue::use_iterator use_it, use_e;

    for (use_it= pVal->use_begin(), use_e=pVal->use_end(); use_it != use_e; ++use_it)
    {

        UpdateGlobalsMap(pVal, *use_it);
    }
  } // Enumeration done

  // Enumerate functions and add to global map, those that are called from other built-ins
  for (Module::iterator extIt = pModule->begin(), extE = pModule->end(); extIt != extE; ++extIt)
  {
    // We need only external functions
    GlobalValue* pVal = extIt;

    GlobalValue::use_iterator use_it, use_e;

    for (use_it= pVal->use_begin(), use_e=pVal->use_end(); use_it != use_e; ++use_it)
    {
      if ( isa<Instruction>(*use_it) )
      {
        Function* pFunc = cast<Instruction>(*use_it)->getParent()->getParent();
        m_mapFunc2Glb[pFunc].push_back(pVal);
      }
    }
  } // Enumeration done

}
}}}
