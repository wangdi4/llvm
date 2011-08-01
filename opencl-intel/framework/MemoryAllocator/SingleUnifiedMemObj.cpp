// Copyright (c) 2006-2010 Intel Corporation
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  cl_memory_object.cpp
//  Implementation of the MemoryObject Class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SingleUnifiedMemObj.h"

#include <Device.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Framework;


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
SingleUnifiedMemObject::SingleUnifiedMemObject(Context * pContext, ocl_entry_points * pOclEntryPoints) : 
	MemoryObject(pContext, pOclEntryPoints),
	m_pDeviceObject(NULL)
{
	INIT_LOGGER_CLIENT(TEXT("SingleUnifiedMemObject"), LL_DEBUG);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
SingleUnifiedMemObject::~SingleUnifiedMemObject()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter SingleUnifiedMemObject D'tor"));

	NotifyDestruction();

	if ( NULL != m_pDeviceObject )
	{
		m_pDeviceObject->clDevMemObjRelease();
	}

	RELEASE_LOGGER_CLIENT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////

// initialize the memory object
cl_err_code SingleUnifiedMemObject::Initialize(
							   cl_mem_flags		clMemFlags,
							   const cl_image_format*	pclImageFormat,
							   unsigned int		dim_count,
							   const size_t*		dimension,
							   const size_t*       pitches,
							   void*			pHostPtr
							   )
{
	m_clFlags = clMemFlags;

	// assign default value
	if ( !(m_clFlags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE)) )
	{
		m_clFlags |= CL_MEM_READ_WRITE;
	}

	// save host ptr only if CL_MEM_USE_HOST_PTR is set
	m_pHostPtr = NULL;

	int devMemFlags = CL_DEV_HOST_PTR_NONE;

	if (m_clFlags & CL_MEM_USE_HOST_PTR)
	{
		// in case that we're using host ptr we don't need to allocated memory for the buffer
		// just use the host ptr instead
		m_pHostPtr = pHostPtr;
		devMemFlags = CL_DEV_HOST_PTR_USER_MAPPED_REGION;
		if ( 0 == (clMemFlags & CL_MEM_WRITE_ONLY))
			devMemFlags |= CL_DEV_HOST_PTR_DATA_AVAIL;
	}
	else if (m_clFlags & CL_MEM_COPY_HOST_PTR)
	{
		devMemFlags = CL_DEV_HOST_PTR_DATA_AVAIL;
	}

#if 0 // Create always the object, no need to wait for first command
	// If not created with USE_HOST or COPY_HOST don't create the resource
	if ( !((CL_DEV_HOST_PTR_DATA_AVAIL & devMemFlags) || (CL_DEV_HOST_PTR_MAPPED_REGION & devMemFlags)) )
	{
		return CL_SUCCESS;
	}
#endif
	// If we have user pointed we need to create the resource now
	// Pass only R/W values
	clMemFlags &= (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY);

	cl_uint numDev;
	// Need too allocate object now
	FissionableDevice* *pDevices = m_pContext->GetDevices(&numDev);
	
	assert(numDev > 0 && "Context should have atleast one device");
	// We assume we have only single device, for fission it's first
	cl_dev_err_code devErr = pDevices[0]->GetDeviceAgent()->clDevCreateMemoryObject(pDevices[0]->GetSubdeviceId(),
			clMemFlags, pclImageFormat, dim_count, dimension, pHostPtr, pitches, (cl_dev_host_ptr_flags)devMemFlags, &m_pDeviceObject);

	if ( CL_DEV_FAILED(devErr) )
	{
		m_pDeviceObject = NULL;
		return CL_OUT_OF_RESOURCES;
	}

	m_pLocation = pDevices[0];

	// Now we should set backing store
	// Get access to internal pointer
	cl_dev_cmd_param_map mapRegion;
	mapRegion.dim_count = GetNumDimensions();
	memset(&mapRegion.origin, 0, sizeof(mapRegion.origin));
	mapRegion.flags = CL_MAP_READ | CL_MAP_WRITE;
	cl_err_code res = m_pDeviceObject->clDevMemObjCreateMappedRegion(&mapRegion);
	if (CL_FAILED(res))
	{
		return CL_OUT_OF_RESOURCES;
	}

	// We assume that the pointer is valid for whole object lifecycle
	m_pMemObjData = mapRegion.ptr;
	m_pDeviceObject->clDevMemObjCreateMappedRegion(&mapRegion);

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::SetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SingleUnifiedMemObject::UpdateLocation(FissionableDevice* pDevice)
{
	LOG_DEBUG(TEXT("Enter SetDataLocation (clDevice=%x)"), pDevice);

	m_pLocation = pDevice;
	return CL_SUCCESS;
}

bool SingleUnifiedMemObject::IsSharedWith(FissionableDevice* pDevice)
{
	return (pDevice == m_pLocation);
}

cl_err_code SingleUnifiedMemObject::CreateDeviceResource(FissionableDevice* pDevice)
{
	if ( NULL != m_pDeviceObject )
	{
		// The device resource already exists
		return CL_SUCCESS;
	}

	cl_dev_subdevice_id subDevId = pDevice->GetSubdeviceId();
	size_t dim[MAX_WORK_DIM], pitch[MAX_WORK_DIM-1];
	GetLayout( (size_t*)&dim, &pitch[0], &pitch[1] );

	// Pass only R/W values
	int clMemFlags = m_clFlags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY);

	cl_uint numDim = GetNumDimensions();
	cl_image_format* pImgFormat = numDim > 1 ? &m_clImageFormat : NULL;

	cl_dev_err_code devErr;
	devErr = pDevice->GetDeviceAgent()->clDevCreateMemoryObject(
		subDevId, clMemFlags, pImgFormat, numDim, dim, NULL, pitch, CL_DEV_HOST_PTR_NONE, &m_pDeviceObject);

	return CL_DEV_SUCCEEDED(devErr) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

cl_err_code SingleUnifiedMemObject::GetDeviceDescriptor(FissionableDevice* pDevice, IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent)
{
	assert(NULL != m_pDeviceObject);
	assert(NULL != ppDevObject);

	*ppDevObject = m_pDeviceObject;

	return CL_SUCCESS;
}

cl_err_code	SingleUnifiedMemObject::MemObjCreateDevMappedRegion(const FissionableDevice*,
										cl_dev_cmd_param_map*	cmd_param_map)
{
	cl_dev_err_code err =  m_pDeviceObject->clDevMemObjCreateMappedRegion(cmd_param_map);

	return CL_DEV_SUCCEEDED(err) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

cl_err_code	SingleUnifiedMemObject::MemObjReleaseDevMappedRegion(const FissionableDevice*,
																cl_dev_cmd_param_map*	cmd_param_map)
{
	cl_dev_err_code err =  m_pDeviceObject->clDevMemObjReleaseMappedRegion(cmd_param_map);

	return CL_DEV_SUCCEEDED(err) ? CL_SUCCESS : CL_INVALID_VALUE;
}

cl_err_code SingleUnifiedMemObject::NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children)
{
    return CL_SUCCESS;
}