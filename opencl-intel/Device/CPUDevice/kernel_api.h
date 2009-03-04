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

namespace Intel { namespace OpenCL {

	// An interface class that defined interface for OCL kernel object
	class ICLDevKernel
	{
	public:
		// Retrives a handle to the kernel object.
		// This handle is used for kernel execution. When kernel is reference to CPU function,
		// the returned handle is a pointer to calling function.
		virtual const void*					GetHandle() const = 0;
		// Returns a pointer to kernek name
		virtual const char*					GetKernelName() const = 0;
		// Returns a number of kernel arguments
		virtual cl_uint						GetArgCount() const = 0;
		// Returns an array of kernel arguments
		virtual const cl_kernel_argument*	GetKernelArgs() const = 0;
		// Returns a pointer to metadata object used by kernel executor
		virtual void*						GetMetaData() const = 0;
		// Returns a size of metadata object
		virtual size_t						GetMetaDataSize() const = 0;
		// Returns a size of implicitly defined local memory buffers
		virtual size_t						GetImplicitLocalSize() const = 0;
	};
}}