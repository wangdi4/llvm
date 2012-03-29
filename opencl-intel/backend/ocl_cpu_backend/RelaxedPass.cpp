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

#define INSERT_MAP_TO_NATIVE(map,func,type,length,native_length) \
    map.insert ( pair<std::string, std::string>("_Z" #length #func type, "_Z" #native_length "native_" #func type) );

// native built-ins, one argument, float version
#define RELAXED_P1_vX(map, func, length, native_length)											\
    INSERT_MAP_TO_NATIVE(map,func,"f",length,native_length)               \
    INSERT_MAP_TO_NATIVE(map,func,"Dv2_f",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv3_f",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv4_f",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv8_f",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv16_f",length,native_length)

// native built-ins, one argument, double version
#define RELAXED_P1_vX_D(map, func, length, native_length)											\
    INSERT_MAP_TO_NATIVE(map,func,"d",length,native_length)               \
    INSERT_MAP_TO_NATIVE(map,func,"Dv2_d",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv3_d",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv4_d",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv8_d",length,native_length)    \
    INSERT_MAP_TO_NATIVE(map,func,"Dv16_d",length,native_length)

// native built-ins, two argument, float version
#define RELAXED_P2_vX_vY(map, func, length, native_length)													\
    INSERT_MAP_TO_NATIVE(map,func,"ff",length,native_length)              \
    INSERT_MAP_TO_NATIVE(map,func,"Dv2_fS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv3_fS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv4_fS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv8_fS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv16_fS_",length,native_length)

// native built-ins, two argument, double version
#define RELAXED_P2_vX_vY_D(map, func, length, native_length)													\
    INSERT_MAP_TO_NATIVE(map,func,"dd",length,native_length)              \
    INSERT_MAP_TO_NATIVE(map,func,"Dv2_dS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv3_dS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv4_dS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv8_dS_",length,native_length)  \
    INSERT_MAP_TO_NATIVE(map,func,"Dv16_dS_",length,native_length)

// native built-ins, two argument(vector, scalar), double version
#define RELAXED_P2_vX_sY(map, func, length, native_length)												\
    INSERT_MAP_TO_NATIVE(map,func,"ff",length,native_length)              \
    INSERT_MAP_TO_NATIVE(map,func,"Dv2_ff",length,native_length)   \
    INSERT_MAP_TO_NATIVE(map,func,"Dv3_ff",length,native_length)   \
    INSERT_MAP_TO_NATIVE(map,func,"Dv4_ff",length,native_length)   \
    INSERT_MAP_TO_NATIVE(map,func,"Dv8_ff",length,native_length)   \
    INSERT_MAP_TO_NATIVE(map,func,"Dv16_ff",length,native_length)

#define RELAXED_P2_vX_pY(map, func, length, native_length)													\
    INSERT_MAP_TO_NATIVE(map,func,"fPf",length,native_length)             \
    INSERT_MAP_TO_NATIVE(map,func,"Dv2_fPS_",length,native_length) \
    INSERT_MAP_TO_NATIVE(map,func,"Dv3_fPS_",length,native_length) \
    INSERT_MAP_TO_NATIVE(map,func,"Dv4_fPS_",length,native_length) \
    INSERT_MAP_TO_NATIVE(map,func,"Dv8_fPS_",length,native_length) \
    INSERT_MAP_TO_NATIVE(map,func,"Dv16_fPS_",length,native_length)

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class RelaxedPass : public ModulePass
{
public:
	RelaxedPass() : ModulePass(ID)
	{  
    // float version of native built-ins
		RELAXED_P1_vX(m_relaxedFunctions, tan, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, sin, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, log, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, exp, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, cos, 3, 10);
		RELAXED_P1_vX(m_relaxedFunctions, logb, 4, 11);
		RELAXED_P1_vX(m_relaxedFunctions, exp2, 4, 11);
		RELAXED_P1_vX(m_relaxedFunctions, log2, 4, 11);
		RELAXED_P1_vX(m_relaxedFunctions, sqrt, 4, 11);
	// RELAXED_P1_vX(m_relaxedFunctions, recip, 5, 12);// there is no recip BI in the spec, there for we can't replace it with native_recip
		RELAXED_P1_vX(m_relaxedFunctions, rsqrt, 5, 12);
		RELAXED_P1_vX(m_relaxedFunctions, log10, 5, 12);
		RELAXED_P1_vX(m_relaxedFunctions, exp10, 5, 12);
		RELAXED_P1_vX(m_relaxedFunctions, ilogb, 5, 12);
		//RELAXED_P1_ALL(m_relaxedFunctions, divide, 6, 13);

    // double version of native built-ins
    RELAXED_P1_vX_D(m_relaxedFunctions, tan, 3, 10);
		RELAXED_P1_vX_D(m_relaxedFunctions, sin, 3, 10);
    RELAXED_P1_vX_D(m_relaxedFunctions, log, 3, 10);
		RELAXED_P1_vX_D(m_relaxedFunctions, exp, 3, 10);
		RELAXED_P1_vX_D(m_relaxedFunctions, cos, 3, 10);
		RELAXED_P1_vX_D(m_relaxedFunctions, exp2, 4, 11);
		RELAXED_P1_vX_D(m_relaxedFunctions, log2, 4, 11);
		RELAXED_P1_vX_D(m_relaxedFunctions, sqrt, 4, 11);
		RELAXED_P1_vX_D(m_relaxedFunctions, rsqrt, 5, 12);
		RELAXED_P1_vX_D(m_relaxedFunctions, log10, 5, 12);
		RELAXED_P1_vX_D(m_relaxedFunctions, exp10, 5, 12);
		
    RELAXED_P2_vX_vY_D(m_relaxedFunctions, powr, 4, 11);
    RELAXED_P2_vX_vY_D(m_relaxedFunctions, hypot, 5, 12);

		RELAXED_P2_vX_vY(m_relaxedFunctions, fdim, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, fmod, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, powr, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, hypot, 5, 12);

		RELAXED_P2_vX_vY(m_relaxedFunctions, fmax, 4, 11);
		RELAXED_P2_vX_sY(m_relaxedFunctions, fmax, 4, 11);
		RELAXED_P2_vX_vY(m_relaxedFunctions, fmin, 4, 11);
		RELAXED_P2_vX_sY(m_relaxedFunctions, fmin, 4, 11);

		RELAXED_P2_vX_pY(m_relaxedFunctions, fract, 5, 12);

    // non-spec native-BI
    RELAXED_P1_vX(m_relaxedFunctions, acos, 4, 11);
    RELAXED_P1_vX_D(m_relaxedFunctions, acos, 4, 11);
    
    RELAXED_P1_vX(m_relaxedFunctions, acosh, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, acosh, 5, 12);

    RELAXED_P1_vX(m_relaxedFunctions, acospi, 6, 13);
    RELAXED_P1_vX_D(m_relaxedFunctions, acospi, 6, 13);

    RELAXED_P1_vX(m_relaxedFunctions, asin, 4, 11);
    RELAXED_P1_vX_D(m_relaxedFunctions, asin, 4, 11);

    RELAXED_P1_vX(m_relaxedFunctions, asinh, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, asinh, 5, 12);

    RELAXED_P1_vX(m_relaxedFunctions, asinpi, 6, 13);
    RELAXED_P1_vX_D(m_relaxedFunctions, asinpi, 6, 13);

    RELAXED_P1_vX(m_relaxedFunctions, atan, 4, 11);
    RELAXED_P1_vX_D(m_relaxedFunctions, atan, 4, 11);

    RELAXED_P1_vX(m_relaxedFunctions, atanh, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, atanh, 5, 12);

    RELAXED_P1_vX(m_relaxedFunctions, atanpi, 6, 13);
    RELAXED_P1_vX_D(m_relaxedFunctions, atanpi, 6, 13);

    RELAXED_P2_vX_vY(m_relaxedFunctions, atan2, 5, 12);
    RELAXED_P2_vX_vY_D(m_relaxedFunctions, atan2, 5, 12);

    RELAXED_P2_vX_vY(m_relaxedFunctions, atan2pi, 7, 14);
    RELAXED_P2_vX_vY_D(m_relaxedFunctions, atan2pi, 7, 14);

    RELAXED_P1_vX(m_relaxedFunctions, cbrt, 4, 11);
    RELAXED_P1_vX_D(m_relaxedFunctions, cbrt, 4, 11);

    RELAXED_P1_vX(m_relaxedFunctions, cospi, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, cospi, 5, 12);

    RELAXED_P1_vX(m_relaxedFunctions, erfc, 4, 11);
    RELAXED_P1_vX_D(m_relaxedFunctions, erfc, 4, 11);

    RELAXED_P1_vX(m_relaxedFunctions, erf, 3, 10);
    RELAXED_P1_vX_D(m_relaxedFunctions, erf, 3, 10);

    RELAXED_P1_vX(m_relaxedFunctions, expm1, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, expm1, 5, 12);

    RELAXED_P1_vX(m_relaxedFunctions, log1p, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, log1p, 5, 12);

    RELAXED_P2_vX_vY(m_relaxedFunctions, pow, 3, 10);
    RELAXED_P2_vX_vY_D(m_relaxedFunctions, pow, 3, 10);

    RELAXED_P1_vX(m_relaxedFunctions, sinpi, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, sinpi, 5, 12);

    RELAXED_P1_vX(m_relaxedFunctions, tanpi, 5, 12);
    RELAXED_P1_vX_D(m_relaxedFunctions, tanpi, 5, 12);


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
			FunctionType* pType = pFunction->getFunctionType();

			Function* pRelaxedFunction = dyn_cast<Function>(M.getOrInsertFunction( stRelaxedName, pType, pFunction->getAttributes() ));
			assert(NULL != pRelaxedFunction);
			pFunction->replaceAllUsesWith(pRelaxedFunction);

			changed = true;
		}
	}

	return changed;
}


}}}