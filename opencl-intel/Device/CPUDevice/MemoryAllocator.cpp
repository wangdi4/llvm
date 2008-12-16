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
//  MemoryAllocator.cpp
//  Implementation of the Class MemoryAllocator
//  Created on:      16-Dec-2008 4:54:53 PM
//  Original author: efiksman
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MemoryAllocator.h"

#include<stdlib.h>

using namespace Intel::OpenCL::CPUDevice;

MemoryAllocator::MemoryAllocator(cl_int devId, cl_dev_log_descriptor *logDesc) :
	m_devId(devId), m_logDesc(logDesc)
{

}



MemoryAllocator::~MemoryAllocator()
{

}

cl_int MemoryAllocator::CreateObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format, size_t IN width,
							size_t IN height, size_t IN depth, cl_dev_mem* OUT memObj )
{
	// TODO : Add log
	if ( NULL == memObj )
	{
		return CL_DEV_INVALID_VALUE;
	}

	if ( (NULL != format) || (1 != height) || (1 != depth) )
	{
		// Only 1D buffers are supported for now
		return CL_DEV_INVALID_VALUE;
	}

	// Allocate memory
	memObj->allocId = m_devId;
	memObj->objHandle = malloc(width);

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::ReleaseObject( cl_dev_mem IN memObj )
{
	// TODO : Add log
	if ( (m_devId != memObj.allocId) || (NULL == memObj.objHandle) )
	{
		return CL_INVALID_MEM_OBJECT;
	}

	free(memObj.objHandle);
	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::LockObject(cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
							void** OUT ptr, size_t* OUT rowPitch, size_t* OUT slicePitch)
{
	cl_char* lockedPtr = (cl_char*)memObj.objHandle;

	lockedPtr += origin[0];
	if ( NULL != rowPitch )
	{
		*rowPitch = 0;
	}
	if ( NULL != slicePitch )
	{
		*slicePitch = 0;
	}
	*ptr = lockedPtr;

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::UnLockObject(cl_dev_mem IN memObj, void* IN ptr)
{
	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::CreateMappedRegion(cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
							void** OUT ptr, size_t* OUT rowPitch, size_t* OUT slicePitch )
{
	cl_char* mappedPtr = (cl_char*)memObj.objHandle;

	mappedPtr += origin[0];
	if ( NULL != rowPitch )
	{
		*rowPitch = 0;
	}
	if ( NULL != slicePitch )
	{
		*slicePitch = 0;
	}
	*ptr = mappedPtr;

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::ReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr )
{
	return CL_DEV_SUCCESS;
}

