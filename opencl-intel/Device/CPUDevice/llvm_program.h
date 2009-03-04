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

#include "kernel_api.h"
#include "program_api.h"
#include "cl_dynamic_lib.h"
#include "cl_thread.h"
#include "cl_synch_objects.h"

#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Module.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace Intel::OpenCL::Utils;
using namespace llvm;

namespace Intel { namespace OpenCL { namespace CPUDevice {

	// Defines a class that implements CPU device program that is generated from LLVM IR

	class LLVMProgram : public ICLDevProgram
	{
		friend class LLVMProgramThread;

	public:
		// Contructor/Destructor
		LLVMProgram();
		virtual ~LLVMProgram();


		// IProgram interface
		// Create program from DLL container
		cl_int	CreateProgram(const cl_prog_container*	pContainer);
		// Executes asyncronious program build process
		cl_int	BuildProgram(fn_clDevBuildStatusUpdate* pfnCallBack, cl_dev_program progId, void* pUserData);
		// Quaries program building status
		cl_build_status	GetBuildStatus() const { return m_clBuildStatus;}
		// Quaries program build log
		const char*	GetBuildLog() const;
		// Returns the stored container size
		size_t	GetContainerSize() const;
		// Returns a pointer to internally stored container
		const cl_prog_container* GetContainer() const {return &m_ContainerInfo;}
		// Copies internally stored container into provided buffer
		cl_int CopyContainer(void* pBuffer, size_t stSize) const;
		// Retrieves a pointer to a function descriptor by kernel name
		cl_int	GetKernel(const char* pKernelName, const ICLDevKernel* *pKernel) const;
		// Retrieves a vector of pointers to a function descriptors
		cl_int	GetAllKernels(const ICLDevKernel* *pKernels, unsigned int uiCount, unsigned int *puiRetCount) const;

	protected:
		// Parses libary information and retrieves/builds function descripotrs
		cl_int		LoadProgram();
		// Release kernel map
		void		FreeMap();

		typedef	std::map<std::string, ICLDevKernel*>	TKernelMap;

		TKernelMap			m_mapKernels;	// A map used for translation between Short name and function descriptor
		cl_prog_container	m_ContainerInfo;// Current container information
		MemoryBuffer*		m_pMemBuffer;	// A memory buffer used to store LLVM IR

		LLVMProgramThread*	m_pBuildingThread;
		cl_build_status		m_clBuildStatus;
		std::string			m_strLastError;

		// Static Members
		static ExecutionEngine*	m_spExecEngine;
		static OclMutex			m_muEEMutex;

	};

	class LLVMProgramThread : public OclThread
	{
	public:
		LLVMProgramThread(LLVMProgram* pProgram, fn_clDevBuildStatusUpdate* pfnCallBack,
			cl_dev_program progId, void* pUserData);

    protected:
        virtual int         Run();          // The actual thread running loop.    

		// Owner of the thread
		LLVMProgram*				m_pProgram;
		// Data to be passed to the calling party
		fn_clDevBuildStatusUpdate*	m_pfnCallBack;
		cl_dev_program				m_progId;
		void*						m_pUserData;
	};

}}}