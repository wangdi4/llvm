/////////////////////////////////////////////////////////////////////////
// llvm_kernel.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
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

#include "cl_device_api.h"
#include "cl_dev_backend_api.h"

#include <vector>
#include <map>
#include <string>

#ifndef LLVM_BACKEND_API
#ifdef LLVM_BACKEND_EXPORTS
#define LLVM_BACKEND_API __declspec(dllexport)
#else
#define LLVM_BACKEND_API __declspec(dllimport)
#endif
#endif

#define DEFAULT_STACK_SIZE	(1024*4)

// Declare external classes
namespace llvm {
	class Function;
	class CallInst;
	class Argument;
	class Value;
	class LoadInst;
	class Instruction;
	class Module;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

	// Defines a class that implements CPU device program that is loaded from DLL
	class LLVMProgram;
	class LLVMKernel : public ICLDevBackendKernel
	{
	public:
		LLVMKernel(LLVMProgram* pProgram);

		// ICLDevBackendKernel interface
		virtual cl_int CreateBinary(void* IN pArgsBuffer, 
			size_t IN BufferSize,
			cl_uint IN WorkDimension,
			const size_t* IN pGlobalWorkOffeset,
			const size_t* IN pGlobalWorkSize,
			const size_t* IN pLocalWorkSize,
			ICLDevBackendBinary** OUT pBinary) const;

		// return a pointer to the Kernel Arguments
		cl_int GetKernelParams( const cl_kernel_argument* OUT *pArgsBuffer, cl_uint* OUT ArgCount ) const;
		// Returns a pointer to the kernel name
		const char*	GetKernelName() const {return m_szName;}
		// Returns the size in bytes of the local memory buffer required by this kernel
		size_t GetImplicitLocalMemoryBufferSize() const { return m_uiTotalImplSize;}
		// Returns the number of Work Items handled by each kernel instance
		size_t GetKernelPackSize() const { return m_uiOptWGSize;}
		// Returns the required work-group size that was declared during kernel compilation.
		// NULL when is not this attribute is not present
		const size_t* GetRequiredWorkgroupSize() const { return m_pReqdWGSize;}
		// Returns the required stack size for single Work Item execution
		// 0 when is not available
		unsigned int  GetWIStackSize() const { return m_uiStackSize;}
		// Releases kernel instance
		void	Release() {delete this;}

		// Local interface
		cl_int ParseLLVM(llvm::Function *pFunc);

	protected:
		friend class LLVMBinary;
		virtual ~LLVMKernel();

		LLVMProgram*		m_pProgram;
		const void*			m_pFuncPtr;
		const char*			m_szName;
		unsigned int		m_uiArgCount;
		cl_kernel_argument*	m_pArguments;

		unsigned int		m_uiExplLocalMemCount;		// A number of explicit local buffers passed with kernel arguments
		const size_t*		m_pReqdWGSize;				// A pointer required work-group size that was declared during kernel compilation
		const size_t*		m_pHintWGSize;				// A hint to work-group size that was declared during kernel compilation
		size_t				m_uiOptWGSize;				// An optimal work-group size, that can be used for kernel execution
		bool				m_bBarrier;					// Is true if barrier() is present in kernel

		llvm::Module*		m_pModule;					// Pointer to the module this kernel belongs to

		cl_dev_err_code	ParseArguments(llvm::Function *pFunc);
		llvm::Value*	SubstituteWIcall(llvm::CallInst *pCall,
			llvm::Argument* pWorkInfo, llvm::Argument* pWGid,
			llvm::Argument* pBaseGlbId, llvm::Argument* pLocalId);
		// Executable creation
		void CheckAndUpdateWGSize(unsigned int uiDimCount, const size_t* pLocal);

		// Calculate and return Global ID value for given dimension
		llvm::Value*	CalcGlobalId(llvm::CallInst *pCall, llvm::Argument* pWGinfo, llvm::Argument* pLocalId);
		llvm::Value*	m_GlbIds[MAX_WORK_DIM];

		// Handling of implicit "local" buffers
		std::map<std::string, llvm::Value*>	m_mapImplLocalPtr;			// A map of implicit local buffers that are used by the kernel
		unsigned int				m_uiTotalImplSize;			// Total size of implicit local buffers
		
		// Substitute a pointer to local buffer with a relative pointer into a buffer passed in parameters
		llvm::Value* SubstituteImplLocalPtr(llvm::Instruction* pInst, llvm::Argument* pLocalMem, llvm::Value* pArgVal);

		// Barrier/AsyncCopy/Wait
		void			AddCtxAddress(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		void			UpdateBarrier(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		llvm::Value*	UpdateAsyncCopy(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		void			UpdateWaitGroup(llvm::CallInst* pCall, llvm::Argument* pLocalId);
		llvm::Value*	m_pCtxPtr;
		// Stack size required by the kernel
		unsigned int	m_uiStackSize;
	};
}}}