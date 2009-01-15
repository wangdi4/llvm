/////////////////////////////////////////////////////////////////////////
// cpu_kernel.h:
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

#include "kernel_api.h"

namespace Intel { namespace OpenCL { namespace CPUDevice {

	// Defines a class that implements CPU device program that is loaded from DLL
	class CPUKernel : public ICLDevKernel
	{
	public:
		// Contructor/Destructor
		CPUKernel();
		virtual ~CPUKernel();

		// ICLDevKernel inerface;
		// Retrives a handle to the kernel object.
		const void*					GetHandle() const {return m_pFuncPtr;}
		// Returns a pointer to kernek name
		const char*					GetKernelName() const {return m_szName;}
		// Returns a number of kernel arguments
		cl_uint						GetNumArgs() const {return m_uiArgCount;}
		// Returns an array of kernel arguments
		const cl_kernel_arg_type*	GetKernelArgs() const {return m_pArguments;}

		// Set functions
		void	SetFuncPtr(const void* pfnFunction) { m_pFuncPtr = pfnFunction;}
		void	SetName(const char* szName, size_t stLen);
		void	SetArgumentList(const cl_kernel_arg_type* pArgs, unsigned int uiArgCount);
	protected:
		const void*			m_pFuncPtr;
		char*				m_szName;
		unsigned int		m_uiArgCount;
		cl_kernel_arg_type*	m_pArguments;
	};
}}}