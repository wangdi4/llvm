/////////////////////////////////////////////////////////////////////////
// BuiltInFunctionImport.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2009 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

// This pass is used for import of built-in functions from runtime module

#include "stdafx.h"
#include "llvm_backend.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <list>
#include <map>
#include <string>

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::DeviceBackend;

#pragma comment(lib, "linker.lib")

bool LinkFunctionBody(Function *Dest, Function *Src,
							 std::map<const Value*, Value*> &ValueMap,
							 std::string *Err);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BIImport : public ModulePass
{
public:
	BIImport(Module* pRTModule) : ModulePass(&ID), m_pRTModule(pRTModule) {}

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

	typedef std::list<std::pair<llvm::Function*, llvm::Function*>>			TImportListType;
	typedef std::map<const llvm::Value*, llvm::Value*>						TValueMap;
	bool ImportGlobalsAsExternals(Module* pModule, TImportListType& impLst, TValueMap& ValueMap);
};

char BIImport::ID = 0;

ModulePass *createBuiltInImportPass(Module* pRTModule) {
	return new BIImport(pRTModule);
}

bool BIImport::ImportGlobalsAsExternals(Module* pModule, TImportListType& impLst, TValueMap& ValueMap)
{
	TImportListType::iterator impIt, impE;
	LLVMBackend::TGlobalUsageMap&	glbMap = LLVMBackend::GetInstance()->m_mapFunc2Glb;

	// Iterate through imported functions and export all relevant global values
	for (impIt=impLst.begin(), impE=impLst.end(); impIt!= impE; ++impIt)
	{
		// check all globals used by imported function
		LLVMBackend::TGlobalUsageMap::iterator mapIt = glbMap.find(impIt->second);
		if ( mapIt == glbMap.end() )
		{
			continue;
		}
		// Get list of all used variables
		LLVMBackend::TGlobalsLst& glbLst = mapIt->second;
		LLVMBackend::TGlobalsLst::iterator glbIt, glbE;
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
					new GlobalVariable(SGV->getType()->getElementType(),
					SGV->isConstant(), SGV->getLinkage(), /*init*/0,
					SGV->getName(), pModule, false,
					SGV->getType()->getAddressSpace());
				// Propagate alignment, visibility and section info.
				NewDGV->copyAttributesFrom(SGV);
				NewDGV->setAlignment(SGV->getAlignment());

#if 0
				// If the LLVM runtime renamed the global, but it is an externally visible
				// symbol, DGV must be an existing global with internal linkage.  Rename
				// it.
				if (!NewDGV->hasLocalLinkage() && NewDGV->getName() != SGV->getName())
					ForceRenaming(NewDGV, SGV->getName());
#endif
				// Make sure to remember this mapping.
				ValueMap[SGV] = NewDGV;
				continue;
			}
			const Function* SFn = dyn_cast<Function>(*glbIt);
			if ( SFn )
			{
				if ( ValueMap.find(SGV) != ValueMap.end() )
				{
					// We already mapped this function
					continue;
				}
				Function* DFn = pModule->getFunction(SFn->getNameStart());
				if ( NULL == DFn )
				{
					DFn = Function::Create(SFn->getFunctionType(),
						SFn->getLinkage(),
						SFn->getName(), pModule);
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
	TValueMap ValueMap;

	// Create list of imported functions
	TImportListType lstImported;

	Module::iterator it,e;
	for (it = M.begin(), e=M.end(); it != e; ++it)
	{
		const char* pFuncName = it->getNameStart();
		if ( it->isDeclaration() && strncmp(pFuncName, "get_", 4) )
		{
			Function* pImpFunc = m_pRTModule->getFunction(pFuncName);
			if ( pImpFunc && !pImpFunc->isDeclaration() )
			{
				// Add source and destination functions
				lstImported.push_back(pair<Function*, Function*>(it, pImpFunc));
			}
		}
	}

	if ( lstImported.empty() )
	{
		return false;			// Nothing to import
	}

	// Now we need to import global variable required by the new functions
	// Declare them as external
	ImportGlobalsAsExternals(&M, lstImported, ValueMap);

	// Now we need to copy required declarations
	TImportListType::iterator impIt, impE;

	// Now import functions
	for (impIt = lstImported.begin(), impE=lstImported.end(); impIt != impE; ++impIt)
	{
		// Clone the original function
		Function* pClone = CloneFunction(impIt->second, NULL);

		// Only provide the function body if there isn't one already.
		LinkFunctionBody( impIt->first, pClone, ValueMap, NULL );
		delete pClone;

	}

	return true;
}

}}}
