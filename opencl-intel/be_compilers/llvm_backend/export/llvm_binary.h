/////////////////////////////////////////////////////////////////////////
// llvm_executable.h:
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

#include "cl_device_api.h"
#include "cl_dev_backend_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
	struct sWorkInfo
	{
		unsigned int	uiWorkDim;
		size_t			GlobalOffset[MAX_WORK_DIM];
		size_t			GlobalSize[MAX_WORK_DIM];
		size_t			LocalSize[MAX_WORK_DIM];
		size_t			WGNumber[MAX_WORK_DIM];
	};

	class LLVMKernel;

	class LLVMBinary : public ICLDevBackendBinary
	{
	public:
		LLVMBinary(const LLVMKernel* pKernel,
						void* IN pArgsBuffer,
						size_t IN ArgBuffSize,
						cl_uint IN WorkDimension,
						const size_t* IN pGlobalOffset,
						const size_t* IN pGlobalWorkSize,
						const size_t* IN pLocalWorkSize);

		//ICLDevBackendBinary interface
		cl_uint Execute( void* IN pMemoryBuffers, 
							const size_t* IN pBufferCount, 
							const size_t* IN pGlobalId, 
							const size_t* IN pLocalId, 
							const size_t* IN pItemsToProcess ) const;

		// Returns the required number of memory buffers, their sizes and their types 
		cl_uint GetMemoryBuffersDescriptions(size_t* IN pBuffersSizes, 
												cl_exec_mem_type* IN pBuffersTypes, 
												size_t* INOUT pBufferCount ) const;
		// Returns the actual number of Work Items handled by each executable instance
		const size_t* GetWorkGroupSize() const
							{return m_WorkInfo.LocalSize;}

		const ICLDevBackendKernel* GetKernel() const
				{ return ((ICLDevBackendKernel*)m_pKernel);}

		// Create execution context which will be used across different execution threads
		cl_uint CreateExecutable(void* IN *pMemoryBuffers, 
								size_t IN pBufferCount, ICLDevBackendExecutable* OUT *pContext);

		// Releases executable instance
		void	Release() {delete this;}

	protected:
		friend class LLVMKernel;
		friend class LLVMExecutable;
		friend class LLVMExecSingleWI;
		friend class LLVMExecMultipleWINoBarrier;
		friend class LLVMExecMultipleWIWithBarrier;
		virtual ~LLVMBinary();

		const LLVMKernel*		m_pKernel;
		const void*				m_pEntryPoint;
		void*					m_pArgsBuffer;
		size_t					m_ArgBuffSize;
		sWorkInfo				m_WorkInfo;
		char*					m_pLocalParams;
		unsigned int			m_uiLocalCount;
		size_t*					m_pLocalBufferOffsets;
	};
}}}