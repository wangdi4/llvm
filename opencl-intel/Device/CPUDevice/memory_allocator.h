
// Copyright (c) 2006-2008 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  MemoryAllocator.h
//  Implementation of the Class MemoryAllocator
//  Created on:      16-Dec-2008 4:54:53 PM
//  Original author: efiksman
///////////////////////////////////////////////////////////

#pragma once

#include"cl_device_api.h"

namespace Intel { namespace OpenCL { namespace CPUDevice {

class MemoryAllocator
{

public:
	MemoryAllocator(cl_int devId, cl_dev_log_descriptor *logDesc);
	virtual ~MemoryAllocator();

	// Create/Release functions
	cl_int	CreateObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format, size_t IN width,
								size_t IN height, size_t IN depth, cl_dev_mem* OUT memObj );
	cl_int	ReleaseObject( cl_dev_mem IN memObj );

	// Checks that given object is valid object and belongs to memory allocator
	cl_int	ValidateObject( cl_dev_mem IN memObj );

	// Lock/Unlock functions
	cl_int	LockObject(cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
								void** OUT ptr, size_t* OUT rowPitch, size_t* OUT slicePitch);
	cl_int	UnLockObject(cl_dev_mem IN memObj, void* IN ptr);

	// Mapped region functions
	cl_int	CreateMappedRegion(cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
								void** OUT ptr, size_t* OUT rowPitch, size_t* OUT slicePitch );
	cl_int	ReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr );

protected:
	cl_int					m_iDevId;
	cl_dev_log_descriptor   m_logDescriptor;
	cl_int					m_iLogHandle;

};

}}}