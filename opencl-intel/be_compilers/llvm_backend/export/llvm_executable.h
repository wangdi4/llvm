/////////////////////////////////////////////////////////////////////////
// llvm_context.h:
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
// or disclosed in any way without Intel�s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "cl_dev_backend_api.h"

#include <setjmp.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {
	class LLVMBinary;

	// Base executable object which knows how to create an internal context
	// The Execute() method is still not implemented
	class LLVMExecutable : public ICLDevBackendExecutable
	{
	public:
		LLVMExecutable(const LLVMBinary* pExec);
		virtual ~LLVMExecutable();

		// Initialize context to with specific number of WorkItems 
		virtual cl_uint	Init(void* *pLocalMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount);

		// Prepares current thread for the executable execution
		cl_uint PrepareThread();
		// Restores Thread state as it was before the execution
		cl_uint RestoreThreadState();

		// Releases the context object
		void	Release();
		// Returns the executable object which generated this context
		const ICLDevBackendBinary* GetBinary() const
						{return (const ICLDevBackendBinary*)m_pBinary;}

		virtual	bool	IsMultipleWIs() {return false;}

		// Returns true if copy procedure to be done, false if not
		virtual	bool	SetAndCheckAsyncCopy(unsigned int uiKey) = 0;
		// Returns true if barrier() should be applied to make WI switch
		virtual	bool	ResetAsyncCopy(unsigned int uiKey) = 0;

	protected:
		const LLVMBinary*	m_pBinary;
		char*				m_pContext;
		size_t*				m_pBaseGlobalId;
		unsigned int		m_uiWICount;
		
		unsigned int		m_uiMXCSRstate; // Stores thread CSR state
		unsigned int		m_uiCSRMask;	// Mask to be applied to set the execution flags	
		unsigned int		m_uiCSRFlags;	// Flags to be set during execution
	};

	// Specialization of executor which executes single WI in iteration
	class LLVMExecSingleWI : public LLVMExecutable
	{
	public:
		LLVMExecSingleWI(const LLVMBinary* pExec) : LLVMExecutable(pExec){}
		cl_uint Execute(const size_t* IN pGroupId,
			const size_t* IN pLocalOffset, 
			const size_t* IN pItemsToProcess);

		bool	SetAndCheckAsyncCopy(unsigned int uiKey) {return true;}
		bool	ResetAsyncCopy(unsigned int uiKey) {return false;}
	};

	// Specialization of executor which executes multiple WIs, no barrier()
	class LLVMExecMultipleWINoBarrier : public LLVMExecutable
	{
	public:
		LLVMExecMultipleWINoBarrier(const LLVMBinary* pExec) : LLVMExecutable(pExec){}
		cl_uint Execute(const size_t* IN pGroupId,
			const size_t* IN pLocalOffset, 
			const size_t* IN pItemsToProcess);
		bool	SetAndCheckAsyncCopy(unsigned int uiKey)
				{bool bFirst = m_bIsFirst; m_bIsFirst =false; return bFirst;}
		bool	ResetAsyncCopy(unsigned int uiKey) {return false;}
	private:
		bool	m_bIsFirst;
	};

	// Specialization of executor which executes multiple WIs, with barrier()
	class LLVMExecMultipleWIWithBarrier : public LLVMExecutable
	{
	public:
		LLVMExecMultipleWIWithBarrier(const LLVMBinary* pExec) :
		  LLVMExecutable(pExec){}
		virtual ~LLVMExecMultipleWIWithBarrier();

		// Override init function
		cl_uint	Init(void* *pLocalMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount);
		cl_uint Execute(const size_t* IN pGroupId,
			const size_t* IN pLocalOffset, 
			const size_t* IN pItemsToProcess);

		virtual void SwitchToMain();

		virtual	bool IsMultipleWIs() {return true;}
		bool	SetAndCheckAsyncCopy(unsigned int uiKey);
		bool	ResetAsyncCopy(unsigned int uiKey);

	protected:
		unsigned int	m_iCurrWI;
		size_t			m_pLclGroupId[MAX_WORK_DIM];
		_JUMP_BUFFER	m_pJmpBuf[CPU_MAX_WORK_GROUP_SIZE];	// Buffer for setjmp info storage
		_JUMP_BUFFER	m_mainJmpBuf;
		unsigned int	m_uiWIStackSize;

#if 0
		typedef	stdext::hash_map<unsigned int, unsigned int> AsyncCopyMap;
		AsyncCopyMap			m_setAsyncCopyCmds;
#endif
	};

	// Specialization of executor which executes vectorized multiple WIs, no barrier()
	class LLVMExecVectorizedNoBarrier : public LLVMExecutable
	{
	public:
		LLVMExecVectorizedNoBarrier(const LLVMBinary* pExec) : LLVMExecutable(pExec) {}
		cl_uint Execute(const size_t* IN pGroupId,
			const size_t* IN pLocalOffset, 
			const size_t* IN pItemsToProcess);

		bool	SetAndCheckAsyncCopy(unsigned int uiKey)
				{bool bFirst = m_bIsFirst; m_bIsFirst =false; return bFirst;}
		bool	ResetAsyncCopy(unsigned int uiKey) {return false;}
	private:
		bool	m_bIsFirst;
	};

	// Specialization of executor which executes vectorized multiple WIs, with barrier()
	class LLVMExecVectorizedWithBarrier : public LLVMExecMultipleWIWithBarrier
	{
	public:
		LLVMExecVectorizedWithBarrier(const LLVMBinary* pExec) :
		  LLVMExecMultipleWIWithBarrier(pExec) {}
		virtual ~LLVMExecVectorizedWithBarrier();

		// Override init function
		cl_uint	Init(void* *pLocalMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount);
		cl_uint Execute(const size_t* IN pGroupId,
			const size_t* IN pLocalOffset, 
			const size_t* IN pItemsToProcess);

		virtual void SwitchToMain();
	};

}}}