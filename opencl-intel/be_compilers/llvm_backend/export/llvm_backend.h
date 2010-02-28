/////////////////////////////////////////////////////////////////////////
// llvm_backend.h:
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

#pragma once

// LLVM backend class, singleton
// Contains RunTime library module and JITer

#include <list>
#include <map>
#include "cl_dynamic_lib.h"

namespace llvm {
	class ExecutionEngine;
	class ModuleProvider;
	class Function;
	class GlobalValue;
	class Module;
	class ModulePass;
	class Pass;
	template <typename T> class SmallVectorImpl;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

struct TLLVMKernelInfo
{
	bool	bDbgPrint;
	bool	bAsynCopy;
	bool	bBarrier;
	size_t	stTotalImplSize;
};

llvm::ModulePass *createBuiltInImportPass(llvm::Module* pRTModule);
llvm::ModulePass *createKernelUpdatePass(llvm::Pass *, llvm::SmallVectorImpl<llvm::Function*> &vectFunctions);

void getKernelInfoMap(llvm::ModulePass *pKUPath, std::map<const llvm::Function*, TLLVMKernelInfo>& infoMap);

class LLVMBackend
{
public:
	static LLVMBackend* GetInstance();
	void Release();

	llvm::ExecutionEngine*	GetExecEngine()
								{return m_pExecEngine;}
	llvm::Module*			GetRTModule();
	
protected:
	friend class LLVMProgram;
	friend class BIImport;

	LLVMBackend();
	~LLVMBackend();
	bool	Init();
	void    InitVTune();

	void	ParseRTModule();

	// Precompiled BI module
	Intel::OpenCL::Utils::OclDynamicLib	m_dllBuiltIns;

	// Global LLVM JIT
	llvm::ExecutionEngine*	m_pExecEngine;
	llvm::ModuleProvider*	m_pModuleProvider;
	bool					m_bRTLoaded;

	bool                    m_bVTuneInitialized;

	// Globals map
	typedef	std::list<const llvm::GlobalValue*>				TGlobalsLst;
	typedef std::map<const llvm::Function*, TGlobalsLst>	TGlobalUsageMap;
	TGlobalUsageMap											m_mapFunc2Glb;

	static LLVMBackend*		s_pLLVMInstance;
};

}}}