/////////////////////////////////////////////////////////////////////////
// RelaxedPass.cpp:
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

// This pass is used for relaxed functions substitution 

#include "stdafx.h"
#include "llvm_backend.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Constants.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <list>
#include <map>
#include <string>

using namespace llvm;
using namespace std;
using namespace Intel::OpenCL::DeviceBackend;

#pragma comment(lib, "linker.lib")

#define RELAXED_P1_vX(map, func, length, native_length)											\
	map##["_Z" #length #func "f"] = "_Z" #native_length "native_" #func "f";						\
	map##["_Z" #length #func "U8__vector2f"] = "_Z" #native_length "native_" #func "U8__vector2f";	\
	map##["_Z" #length #func "U8__vector3f"] = "_Z" #native_length "native_" #func "U8__vector3f";	\
	map##["_Z" #length #func "U8__vector4f"] = "_Z" #native_length "native_" #func "U8__vector4f";	\
	map##["_Z" #length #func "U8__vector8f"] = "_Z" #native_length "native_" #func "U8__vector8f";	\
	map##["_Z" #length #func "U8__vector16f"] = "_Z" #native_length "native_" #func "U8__vector16f";\
	//																								\
	map##["_Z" #length #func "d"] = "_Z" #native_length "native_" #func "f";						\
	map##["_Z" #length #func "U8__vector2d"] = "_Z" #native_length "native_" #func "U8__vector2d";	\
	map##["_Z" #length #func "U8__vector3d"] = "_Z" #native_length "native_" #func "U8__vector3d";	\
	map##["_Z" #length #func "U8__vector4d"] = "_Z" #native_length "native_" #func "U8__vector4d";	\
	map##["_Z" #length #func "U8__vector8d"] = "_Z" #native_length "native_" #func "U8__vector8d";	\
	map##["_Z" #length #func "U8__vector16d"] = "_Z" #native_length "native_" #func "U8__vector16d";

#define RELAXED_P2_vX_vY(map, func, length, native_length)													\
	map##["_Z" #length #func "ff"] = "_Z" #native_length "native_" #func "ff";								\
	map##["_Z" #length #func "U8__vector2fS_"] = "_Z" #native_length "native_" #func "U8__vector2fS_";		\
	map##["_Z" #length #func "U8__vector3fS_"] = "_Z" #native_length "native_" #func "U8__vector3fS_";		\
	map##["_Z" #length #func "U8__vector4fS_"] = "_Z" #native_length "native_" #func "U8__vector4fS_";		\
	map##["_Z" #length #func "U8__vector8fS_"] = "_Z" #native_length "native_" #func "U8__vector8fS_";		\
	map##["_Z" #length #func "U8__vector16fS_"] = "_Z" #native_length "native_" #func "U8__vector16fS_";	\
	//																										\
	map##["_Z" #length #func "dd"] = "_Z" #native_length "native_" #func "dd";								\
	map##["_Z" #length #func "U8__vector2dS_"] = "_Z" #native_length "native_" #func "U8__vector2dS_";		\
	map##["_Z" #length #func "U8__vector3dS_"] = "_Z" #native_length "native_" #func "U8__vector3dS_";		\
	map##["_Z" #length #func "U8__vector4dS_"] = "_Z" #native_length "native_" #func "U8__vector4dS_";		\
	map##["_Z" #length #func "U8__vector8dS_"] = "_Z" #native_length "native_" #func "U8__vector8dS_";		\
	map##["_Z" #length #func "U8__vector16dS_"] = "_Z" #native_length "native_" #func "U8__vector16dS_";

#define RELAXED_P2_vX_sY(map, func, length, native_length)												\
	map##["_Z" #length #func "ff"] = "_Z" #native_length "native_" #func "ff";							\
	map##["_Z" #length #func "U8__vector2ff"] = "_Z" #native_length "native_" #func "U8__vector2ff";	\
	map##["_Z" #length #func "U8__vector3ff"] = "_Z" #native_length "native_" #func "U8__vector3ff";	\
	map##["_Z" #length #func "U8__vector4ff"] = "_Z" #native_length "native_" #func "U8__vector4ff";	\
	map##["_Z" #length #func "U8__vector8ff"] = "_Z" #native_length "native_" #func "U8__vector8ff";	\
	map##["_Z" #length #func "U8__vector16ff"] = "_Z" #native_length "native_" #func "U8__vector16ff";	\
	//																									\
	map##["_Z" #length #func "dd"] = "_Z" #native_length "native_" #func "dd";							\
	map##["_Z" #length #func "U8__vector2dd"] = "_Z" #native_length "native_" #func "U8__vector2dd";	\
	map##["_Z" #length #func "U8__vector3dd"] = "_Z" #native_length "native_" #func "U8__vector3dd";	\
	map##["_Z" #length #func "U8__vector4dd"] = "_Z" #native_length "native_" #func "U8__vector4dd";	\
	map##["_Z" #length #func "U8__vector8dd"] = "_Z" #native_length "native_" #func "U8__vector8dd";	\
	map##["_Z" #length #func "U8__vector16dd"] = "_Z" #native_length "native_" #func "U8__vector16dd";

#define RELAXED_P2_vX_pY(map, func, length, native_length)													\
	map##["_Z" #length #func "fPf"] = "_Z" #native_length "native_" #func "fPf";							\
	map##["_Z" #length #func "U8__vector2fPS_"] = "_Z" #native_length "native_" #func "U8__vector2fPS_";	\
	map##["_Z" #length #func "U8__vector3fPS_"] = "_Z" #native_length "native_" #func "U8__vector3fPS_";	\
	map##["_Z" #length #func "U8__vector4fPS_"] = "_Z" #native_length "native_" #func "U8__vector4fPS_";	\
	map##["_Z" #length #func "U8__vector8fPS_"] = "_Z" #native_length "native_" #func "U8__vector8fPS_";	\
	map##["_Z" #length #func "U8__vector16fPS_"] = "_Z" #native_length "native_" #func "U8__vector16fPS_";	\
	//																										\
	map##["_Z" #length #func "dPd"] = "_Z" #native_length "native_" #func "dPd";							\
	map##["_Z" #length #func "U8__vector2dPS_"] = "_Z" #native_length "native_" #func "U8__vector2dPS_";	\
	map##["_Z" #length #func "U8__vector3dPS_"] = "_Z" #native_length "native_" #func "U8__vector3dPS_";	\
	map##["_Z" #length #func "U8__vector4dPS_"] = "_Z" #native_length "native_" #func "U8__vector4dPS_";	\
	map##["_Z" #length #func "U8__vector8dPS_"] = "_Z" #native_length "native_" #func "U8__vector8dPS_";	\
	map##["_Z" #length #func "U8__vector16dPS_"] = "_Z" #native_length "native_" #func "U8__vector16dPS_";


namespace Intel { namespace OpenCL { namespace DeviceBackend {

class RelaxedPass : public ModulePass
{
public:
	RelaxedPass() : ModulePass(&ID)
	{  
		RELAXED_P1_vX(m_relaxedFunctions, tan, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, sin, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, log, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, exp, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, cos, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, logb, 4, 11);
		RELAXED_P1_vX(m_relaxedFunctions, exp2, 4, 11);
		RELAXED_P1_vX(m_relaxedFunctions, log2, 4, 11);
		RELAXED_P1_vX(m_relaxedFunctions, sqrt, 4, 11);
		RELAXED_P1_vX(m_relaxedFunctions, recip, 5, 12);
		RELAXED_P1_vX(m_relaxedFunctions, rsqrt, 5, 12);
		RELAXED_P1_vX(m_relaxedFunctions, log10, 5, 12);
		RELAXED_P1_vX(m_relaxedFunctions, exp10, 5, 12);
		RELAXED_P1_vX(m_relaxedFunctions, ilogb, 5, 12);
		//RELAXED_P1_ALL(m_relaxedFunctions, divide, 6, 13);

		RELAXED_P2_vX_vY(m_relaxedFunctions, fdim, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, fmod, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, powr, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, hypot, 5, 12);

		RELAXED_P2_vX_vY(m_relaxedFunctions, fmax, 4, 11);
		RELAXED_P2_vX_sY(m_relaxedFunctions, fmax, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, fmin, 4, 11);
		RELAXED_P2_vX_sY(m_relaxedFunctions, fmin, 4, 11);

		RELAXED_P2_vX_pY(m_relaxedFunctions, fract, 5, 12);
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

		Module*			m_pModule;
		
		std::map<std::string, std::string> m_relaxedFunctions;
		typedef std::map<std::string, std::string> Mymap;
	};

char RelaxedPass::ID = 0;

ModulePass *createRelaxedPass() 
{
	return new RelaxedPass();
}


bool RelaxedPass::runOnModule(Module &M)
{
	m_pModule = &M;
	bool changed = false;

	Module::iterator it,e;
	for (it = M.begin(), e=M.end(); it != e; ++it)
	{
		std::string pFuncName = it->getNameStr();
		if ( ( it->isDeclaration() ) && ( m_relaxedFunctions.count( pFuncName ) != 0 ) )
		{
			Function* pFunction = &*it;
			std::string stRelaxedName = m_relaxedFunctions[pFuncName];
			const FunctionType* pType = pFunction->getFunctionType();

			Function* pRelaxedFunction = dyn_cast<Function>(M.getOrInsertFunction( stRelaxedName, pType, pFunction->getAttributes() ));
			assert(NULL != pRelaxedFunction);
			pFunction->replaceAllUsesWith(pRelaxedFunction);

			changed = true;
		}
	}

	return changed;
}


}}}