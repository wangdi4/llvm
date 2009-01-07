/////////////////////////////////////////////////////////////////////////
// program_api.h:
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

namespace Intel { namespace OpenCL {

	// An interface class that defined interface for OCL program object
	class ICLDevProgram
	{
	public:
		// Creates a program from provided container
		// Input
		//		pContainer		- A pointer to valid conteiner that is supported by this program implementation
		// Returns
		//		CL_DEV_SUCCESS			- command was created succefully
		//		CL_DEV_INVALID_BINARY	- provided binary within the container is not supported or
		//									binary data is broken
		//		CL_DEV_OUT_OF_MEMORY	- there is not enough memory to create the program
		virtual cl_int	CreateProgram(const cl_prog_container*	pContainer) = 0;

		// Executes asyncronious program build process
		// Input
		//		
		// Returns
		//		true	-	Program is built and ready for execution
		//		false	-	Program must be built before execution
		virtual cl_int	BuildProgram(fn_clDevBuildStatusUpdate* pfnCallBack, cl_dev_program progId, void* pUserData) = 0;

		// Quaries program building status
		// Returns
		//		CL_BUILD_NONE		- no build has been performed
		//		CL_BUILD_ERROR		- the last call to clBuildProgram generated an error
		//		CL_BUILD_SUCCESS	- the last call to clBuildProgram was successful
		//		CL_BUILD_IN_PROGRESS- the last call to clBuildProgram was successful is not finished
		virtual cl_build_status	GetBuildStatus() const = 0;

		// Quaries program build log
		// Returns
		//		A pointer to null terminated string that describes build log
		virtual const char*	GetBuildLog() const = 0;

		// Returns the stored container size
		virtual size_t	GetContainerSize() const = 0;

		// Returns a pointer to internally stored container
		virtual const cl_prog_container* GetContainer() const = 0;

		// Copies internally stored container into provided buffer
		// Input
		//		pBuffer	- A pointer to buffer where to store the data
		//		szSize	- Size in bytes of the provided buffer
		// Returns
		//		CL_DEV_SUCCESS			- if buffer was succesfully copied to buffer
		//		CL_DEV_INVALID_VALUE	- if buffer size is insufficient
		virtual cl_int CopyContainer(void* pBuffer, size_t stSize) const = 0;

		// Retrieves a pointer to a kernel descriptor by kernel name
		// Input
		//		pKernelName	- A pointer to null terminated string that specified kernel name
		// Output
		//		pKernel		- A pointer to returned kernel descripor
		// Returns
		//		CL_DEV_SUCCESS			- if kernel descriptor was successefully retrived
		//		CL_DEV_INVALID_KERNEL	- if kernel name was not found
		virtual cl_int	GetKernel(const char* pKernelName, const ICLDevKernel* *pKernel) const = 0;

		// Retrieves a vector of pointers to a function descriptors
		// User shoud provide a buffer for vector storage
		// Input
		//		pKernels	- A pointer to buffer that will hold pointers to kernel descriptors
		//		uiCount		- A number of items in pKernel buffers
		//		puiRetCount	- A pointer to a buffer where the actual number of kernels will be returned (optional)
		// Returns
		//		CL_DEV_SUCCESS			- if vector sucssefully was retrived
		//		CL_DEV_INVALID_VALUE	- if provided buffer is not enought or one of the parameters is invalid
		virtual cl_int	GetAllKernels(const ICLDevKernel* *pKernels, unsigned int uiCount, unsigned int *puiRetCount) const = 0;
	};
}}