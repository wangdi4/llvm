/////////////////////////////////////////////////////////////////////////
// llvm_program.h:
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

#include "cl_dev_backend_api.h"

#ifndef LLVM_BACKEND_API
#ifdef LLVM_BACKEND_EXPORTS
#define LLVM_BACKEND_API __declspec(dllexport)
#else
#define LLVM_BACKEND_API __declspec(dllimport)
#endif
#endif

namespace llvm
{
class MemoryBuffer;
class ExecutionEngine;
class ModuleProvider;
class Module;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

	// Defines a class that implements CPU device program that is generated from LLVM IR
	class LLVMProgram : public ICLDevBackendProgram
	{
		friend class LLVMProgramBuildTask;
		friend class LLVMKernel;

	public:
		// Creates a program object from a provided container. 
		static cl_int LLVM_BACKEND_API CreateProgram( const cl_prog_container* IN pContainer, ICLDevBackendProgram** OUT pProgram );

		// ICLDevBackendProgram interface
		// Builds the program, the generated program byte code is compiler specific and is not necessarily the final target machine code. 
		cl_int	BuildProgram( const char IN *pOptions );
		// Get the program build log
		cl_int GetBuildLog(size_t INOUT *pSize, char* OUT pLog) const;
		// get a container of the program 
		cl_int GetContainer( size_t INOUT *pSize, cl_prog_container* OUT pContainer  ) const;
		// Retrieves a pointer to a kernel object by kernel name
		cl_int	GetKernel(const char* IN pKernelName, const ICLDevBackendKernel** OUT pKernel) const;
		// Retrieves a vector of pointers to a function descriptors
		cl_int	GetAllKernels(const ICLDevBackendKernel** IN pKernels, cl_uint* INOUT puiRetCount) const;
		// Releases program instance
		void	Release();

	protected:
		// Can't allocate program directly
		LLVMProgram();

		// Parses library information and retrieves/builds function descriptors
		cl_dev_err_code		LoadProgram();
		// Release kernel map
		void				FreeMap();

		typedef	std::map<std::string, ICLDevBackendKernel*>	TKernelMap;

		TKernelMap				m_mapKernels;	// A map used for translation between Short name and function descriptor
		cl_prog_container		m_ContainerInfo;// Current container information
		llvm::MemoryBuffer*		m_pMemBuffer;	// A memory buffer used to store LLVM IR
		llvm::ModuleProvider*	m_pModuleProvider;	// Module provider to store the IR

		std::string				m_strLastError;

		void	AddWIInfoDeclarations(llvm::Module* pModule);
		void	AddBarrierDeclaration(llvm::Module* pModule);
		void	AddAsyncCopyDeclaration(llvm::Module* pModule);

		bool	m_bBarrierDecl;
		bool	m_bAsyncCopy;
	};

}}}