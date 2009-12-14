// Copyright (c) 2006-2007 Intel Corporation
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
//  Context.cpp
//  Implementation of the Class Context
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Context.h"
#include "program.h"
#include "cl_buffer.h"
#include "image.h"
#include "sampler.h"
#include <cl_utils.h>
#include <device.h>
#include <cl_objects_map.h>

// for debug...???
#include <limits.h>
#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

map<cl_device_id, cl_uint> Context::m_sContextsPerDevices;   // Used to determine whether to create/close a device inst.

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::Context(const cl_context_properties * clProperties, cl_uint uiNumDevices, Device **ppDevices, logging_fn pfnNotify, void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints)
{
	//cl_start;

	::OCLObject();
	m_pfnNotify = NULL;
	m_pUserData = NULL;

	INIT_LOGGER_CLIENT(L"Context", LL_DEBUG);
	LOG_DEBUG(L"Context constructor enter");

	m_pPrograms = new OCLObjectsMap();
	m_pDevices = new OCLObjectsMap();
	m_pMemObjects = new OCLObjectsMap();
	m_pSamplers = new OCLObjectsMap();

	m_ppDevices = NULL;
	m_pDeviceIds = NULL;

#ifdef _DEBUG
assert ((NULL != ppDevices) && (uiNumDevices > 0));
#endif

	m_ppDevices = new Device*[uiNumDevices];
	if (NULL == m_ppDevices)
	{
		*pclErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}
	m_pDeviceIds = new cl_device_id[uiNumDevices];
	if (NULL == m_ppDevices)
	{
		*pclErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		m_pDevices->AddObject(ppDevices[ui], ppDevices[ui]->GetId(), false);
		m_ppDevices[ui] = ppDevices[ui];
		m_pDeviceIds[ui] = (cl_device_id)ppDevices[ui]->GetId();
	}

	m_uiContextPropCount = 0;
	m_pclContextProperties = NULL;
	if (NULL == clProperties)
	{
		m_pclContextProperties = new cl_context_properties[1];
		if (NULL != m_pclContextProperties)
		{
			m_pclContextProperties[0] = 0;
		}
	}
	else
	{
		// count the number of properties;
		while (0 != clProperties[m_uiContextPropCount])
		{
			m_uiContextPropCount++;
		}
		m_uiContextPropCount++; // last property = NULL;
		// allocate new buffer for context's properties
		m_pclContextProperties = new cl_context_properties[m_uiContextPropCount];
		if (NULL != m_pclContextProperties)
		{
			memcpy_s(m_pclContextProperties, m_uiContextPropCount * sizeof(cl_context_properties), clProperties, m_uiContextPropCount * sizeof(cl_context_properties));
		}
	}
	m_pfnNotify = pfnNotify;
	m_pUserData = pUserData;

    //
    // For each device in this context, check if there is a need to create device instance.
    //
    for(cl_uint idx = 0; idx < uiNumDevices; idx++)
    {
        cl_device_id devId = (cl_device_id)ppDevices[idx]->GetId();
	    map<cl_device_id, cl_uint>::iterator it = m_sContextsPerDevices.find(devId);
	    if (it == m_sContextsPerDevices.end())
	    {
            // First time for this device, create it.
            m_sContextsPerDevices[devId] = 1;
            ppDevices[idx]->CreateInstance();		    
	    }
        else
        {
            // Find device, increment its value;
            m_sContextsPerDevices[devId] = (it->second)+1;
        }
    }

	m_pHandle = new _cl_context;
	m_pHandle->object = this;
	m_pHandle->dispatch = pOclEntryPoints;
	return;
	//cl_return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////////////////////////
void Context::Cleanup( bool bTerminate )
{
    if (bTerminate)
    {
        // If terminate, do nothing since devices are off already
        return;
    }
    //
    // First, remove device if it required,
    //
    cl_uint uiNumDevices = m_pDevices->Count();
    OCLObject** ppDevices = new OCLObject* [uiNumDevices];
    m_pDevices->GetObjects(uiNumDevices, ppDevices, NULL);

    for(cl_uint idx = 0; idx < uiNumDevices; idx++)
    {
        Device* pDevice = dynamic_cast<Device*>(ppDevices[idx]);
        cl_device_id clDevId = (cl_device_id)pDevice->GetId();
        map<cl_device_id, cl_uint>::iterator it = m_sContextsPerDevices.find(clDevId);
        if (it != m_sContextsPerDevices.end())
        {
            // Find device, decrement its value;
            cl_int numInstances = it->second;
            numInstances--;
            m_sContextsPerDevices.erase(it);
            if ( 0 != numInstances)
            {
                m_sContextsPerDevices[clDevId] = numInstances;
            }
            else
            {
                // Done - close device	                
                pDevice->CloseDeviceInstance();
            }
        }
    }
    delete[] ppDevices;


}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::~Context()
{
	LOG_DEBUG(L"Context destructor enter");
    LOG_DEBUG(L"CONTEXT_TEST: Context destructor enter. (id = %d)", m_iId);

    //
    // Free private resources
    //

	RELEASE_LOGGER_CLIENT;

    if ( NULL != m_pPrograms )
    {
        delete m_pPrograms;
        m_pPrograms = NULL;
    }
    if ( NULL != m_pDevices )
    {
        delete m_pDevices;
        m_pDevices = NULL;
    }
	if (NULL != m_ppDevices)
	{
		delete[] m_ppDevices;
		m_ppDevices = NULL;
	}
	if (NULL != m_pDeviceIds)
	{
		delete[] m_pDeviceIds;
		m_pDeviceIds = NULL;
	}
    if ( NULL != m_pMemObjects )
    {
        delete m_pMemObjects;
        m_pMemObjects = NULL;
    }
    if ( NULL != m_pSamplers )
    {
        delete m_pSamplers;
        m_pSamplers = NULL;
    }

	if (NULL != m_pclContextProperties)
	{
		delete []m_pclContextProperties;
		m_pclContextProperties = NULL;
	}

	if (NULL != m_pHandle)
	{
		delete m_pHandle;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	LOG_DEBUG(L"Context::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", param_name, param_value_size, param_value, param_value_size_ret);
	
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;
	void * pValue = NULL;
	
	cl_err_code clErrRet = CL_SUCCESS;
	switch ( (cl_context_info)param_name )
	{

	case CL_CONTEXT_REFERENCE_COUNT:
		szParamValueSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_CONTEXT_DEVICES:
		szParamValueSize = sizeof(cl_device_id) * m_pDevices->Count();
		pValue = m_pDeviceIds;
		break;
	case CL_CONTEXT_PROPERTIES:
		szParamValueSize = sizeof(cl_context_properties) * (m_uiContextPropCount);
		pValue = m_pclContextProperties;
		break;
	default:
		LOG_ERROR(L"param_name (=%d) isn't valid", param_name);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != param_value && param_value_size < szParamValueSize)
	{
		LOG_ERROR(L"param_value_size (=%d) < szParamValueSize (=%d)", param_value_size, szParamValueSize);
		return CL_INVALID_VALUE;
	}

	// return param value size
	if (NULL != param_value_size_ret)
	{
		*param_value_size_ret = szParamValueSize;
	}

	if (NULL != param_value && szParamValueSize > 0)
	{
		memcpy_s(param_value, param_value_size, pValue, szParamValueSize);
	}

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramWithSource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithSource(cl_uint uiCount, const char ** ppcStrings, const size_t * szLengths, Program ** ppProgram)
{
	LOG_DEBUG(L"CreateProgramWithSource enter. uiCount=%d, ppcStrings=%d, szLengths=%d, ppProgram=%d", uiCount, ppcStrings, szLengths, ppProgram);
	// check input parameters
	if (NULL == ppProgram)
	{
		LOG_ERROR(L"NULL == ppProgram; return CL_INVALID_VALUE");
		return CL_INVALID_VALUE;
	}
	if (NULL == m_pPrograms)
	{
		LOG_ERROR(L"NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	// create new program object
	Program * pProgram = new Program(this);
	pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

	// set source code in program object
	cl_err_code clErrRet = pProgram->AddSource(uiCount, ppcStrings, szLengths);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(L"pProgram->AddSource(%d, %d, %d) = %d",uiCount, ppcStrings, szLengths, clErrRet);
		delete pProgram;
		*ppProgram = NULL;
		return clErrRet;
	}
	// add program object to programs map list
	m_pPrograms->AddObject((OCLObject*)pProgram);
	*ppProgram = pProgram;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetDeviceByIndex
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetDeviceByIndex(cl_uint uiDeviceIndex, Device** ppDevice)
{
	if ( NULL == ppDevice )
    {
        return CL_INVALID_VALUE;
    }
	
	cl_err_code clErrRet = m_pDevices->GetObjectByIndex((cl_int)uiDeviceIndex, (OCLObject**)ppDevice);
    if ( CL_FAILED(clErrRet) || NULL == ppDevice)
    {
        return CL_ERR_KEY_NOT_FOUND;
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CheckDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Context::CheckDevices(cl_uint uiNumDevices, const cl_device_id * pclDevices)
{
	LOG_DEBUG(L"CheckDevices enter. uiNumDevices=%d, pclDevices=%d", uiNumDevices, pclDevices);
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		// invalid inputs
		LOG_ERROR(L"0 == uiNumDevices || NULL == pclDevices");
		return false;
	}
	Device * pDevice = NULL;
	cl_err_code clErrRet = CL_SUCCESS;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		clErrRet = m_pDevices->GetOCLObject((cl_int)pclDevices[ui], (OCLObject**)&pDevice);
		if ( CL_FAILED(clErrRet) || NULL == pDevice)
		{
			LOG_ERROR(L"device %d was't found in this context", pclDevices[ui]);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramWithBinary
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithBinary(cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const size_t * pszLengths, const unsigned char ** ppBinaries, cl_int * piBinaryStatus, Program ** ppProgram)
{
	LOG_DEBUG(L"CreateProgramWithBinary enter. uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d, ppProgram=%d", 
		uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, ppProgram);
	
	cl_err_code clErrRet = CL_SUCCESS;
	
	if (NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries)
	{
		// invalid input args
		LOG_ERROR(L"NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries");
		return CL_INVALID_VALUE;
	}
	// check items in pszLengths and in ppBinaries
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		if (0 == pszLengths[ui] || NULL == ppBinaries[ui])
		{
			LOG_ERROR(L"0 == pszLengths[%d] || NULL == ppBinaries[%d]", ui, ui);
			if (NULL != piBinaryStatus)
			{
				piBinaryStatus[ui] = CL_INVALID_VALUE;
			}
			return CL_INVALID_VALUE;
		}
	}
	
	// check devices
	bool bRes = CheckDevices(uiNumDevices, pclDeviceList);
	if (false == bRes)
	{
		LOG_ERROR(L"CheckDevices(uiNumDevices, pclDeviceList) = false");
		return CL_INVALID_DEVICE;
	}

	// create program object
	Program * pProgram = new Program(this);
	pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

	// get devices and assign binaries to program object
	Device ** ppDevices = new Device * [uiNumDevices];
	if (NULL == ppDevices)
	{
		// can't allocate momery for devices
		LOG_ERROR(L"Can't allocated memory for devices");
		delete pProgram;
		return CL_ERR_INITILIZATION_FAILED;
	}
	
	map<cl_device_id,Device*>::iterator it;
	Device * pDevice = NULL;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		clErrRet = m_pDevices->GetOCLObject((cl_int)pclDeviceList[ui], (OCLObject**)&pDevice);
		if (CL_SUCCEEDED(clErrRet))
		{
			ppDevices[ui] = pDevice;
		}
	}

	clErrRet = pProgram->AddBinaries(uiNumDevices, ppDevices, pszLengths, ppBinaries, piBinaryStatus);

	delete[] ppDevices;
	m_pPrograms->AddObject((OCLObject*)pProgram);
	*ppProgram = pProgram;
	return clErrRet;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveProgram(cl_program clProgramId)
{
	LOG_DEBUG(L"Enter RemoveProgram (clProgramId=%d)", clProgramId);

#ifdef _DEBUG
	assert ( NULL != m_pPrograms );
#endif

	Program * pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgramId, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	return m_pPrograms->RemoveObject((cl_int)clProgramId, NULL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveMemObject(cl_mem clMem)
{
	LOG_DEBUG(L"Enter RemoveMemObject (clMem=%d)", clMem);

#ifdef _DEBUG
	assert ( NULL != m_pMemObjects );
#endif

	MemoryObject * pMemObj = NULL;
	cl_err_code clErrRet = m_pMemObjects->GetOCLObject((cl_int)clMem, (OCLObject**)&pMemObj);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	return m_pMemObjects->RemoveObject((cl_int)clMem, NULL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveSampler
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveSampler(cl_sampler clSampler)
{
	LOG_DEBUG(L"Enter RemoveSampler (clSampler=%d)", clSampler);

#ifdef _DEBUG
	assert ( NULL != m_pSamplers );
#endif

	Sampler * pSmapler = NULL;
	cl_err_code clErrRet = m_pSamplers->GetOCLObject((cl_int)clSampler, (OCLObject**)&pSmapler);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	return m_pSamplers->RemoveObject((cl_int)clSampler, NULL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, Buffer ** ppBuffer)
{
	LOG_DEBUG(L"Enter CreateBuffer (cl_mem_flags=%d, szSize=%d, pHostPtr=%d, ppBuffer=%d)", 
		clFlags, szSize, pHostPtr, ppBuffer);

#ifdef _DEBUG
	assert ( NULL != ppBuffer );
	assert ( NULL != m_pMemObjects );
#endif

	cl_ulong ulMaxMemAllocSize = GetMaxMemAllocSize();
	LOG_DEBUG(L"GetMaxMemAllocSize() = %d", ulMaxMemAllocSize);
	
	// check buffer size
	if (szSize == 0 || szSize > ulMaxMemAllocSize)
	{
		LOG_ERROR(L"szSize == %d, ulMaxMemAllocSize =%d", szSize, ulMaxMemAllocSize);
		return CL_INVALID_BUFFER_SIZE;
	}

	cl_err_code clErr = CL_SUCCESS;
	Buffer * pBuffer = new Buffer(this, clFlags, pHostPtr, szSize, &clErr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new buffer, returned: %ws", ClErrTxt(clErr));
        delete pBuffer;
		return clErr;
	}

	clErr = pBuffer->Initialize(pHostPtr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initizialized data, pBuffer->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
        delete pBuffer;
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject*)pBuffer);

	*ppBuffer = pBuffer;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateImage2D
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateImage2D(cl_mem_flags clFlags, 
								   const cl_image_format * pclImageFormat, 
								   void * pHostPtr,
								   size_t szImageWidth, 
								   size_t szImageHeight, 
								   size_t szImageRowPitch,
								   Image2D ** ppImage2d)
{
	LOG_DEBUG(L"Enter CreateImage2D (clFlags=%d, pclImageFormat=%d, pHostPtr=%d, szImageWidth=%d, szImageHeight=%d, szImageRowPitch=%d, ppImage2d=%d)", 
		clFlags, pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, ppImage2d);

#ifdef _DEBUG
	assert ( NULL != ppImage2d );
	assert ( NULL != m_pMemObjects );
#endif

	size_t szMaxAllowedImageWidth = 0, szMaxAllowedImageHeight = 0;
	cl_err_code clErr = GetMaxImageDimensions(&szMaxAllowedImageWidth, &szMaxAllowedImageHeight, NULL, NULL, NULL);
	
	LOG_DEBUG(L"szMaxAllowedImageWidth = %d, szMaxAllowedImageHeight = %d", szMaxAllowedImageWidth, szMaxAllowedImageHeight);
	
	//check image sizes
	if ((szImageWidth < 1)	||
		(szImageHeight < 1)	||
		(szImageWidth > szMaxAllowedImageWidth)	||
		(szImageHeight > szMaxAllowedImageHeight))
	{
		LOG_ERROR(L"szImageWidth == %d, szImageHeight =%d", szImageWidth, szImageHeight);
		return CL_INVALID_IMAGE_SIZE;
	}

	Image2D * pImage2D = new Image2D(this, clFlags, (cl_image_format*)pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, &clErr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new Image2D, returned: %ws", ClErrTxt(clErr));
        delete pImage2D;
		return clErr;
	}

	clErr = pImage2D->Initialize(pHostPtr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initizialized data, pImage2D->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
        delete pImage2D;
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject*)pImage2D);

	*ppImage2d = pImage2D;
	return CL_SUCCESS;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateImage3D
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateImage3D(cl_mem_flags clFlags, 
								   const cl_image_format * pclImageFormat, 
								   void * pHostPtr,
								   size_t szImageWidth, 
								   size_t szImageHeight, 
								   size_t szImageDepth, 
								   size_t szImageRowPitch,
								   size_t szImageSlicePitch,
								   Image3D ** ppImage3d)
{
	LOG_DEBUG(L"Enter CreateImage3D (clFlags=%d, pclImageFormat=%d, pHostPtr=%d, szImageWidth=%d, szImageHeight=%d, szImageDepth=%d, szImageRowPitch=%d, szImageSlicePitch=%d, ppImage2d=%d)", 
		clFlags, pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, ppImage3d);

#ifdef _DEBUG
	assert ( NULL != ppImage3d );
	assert ( NULL != m_pMemObjects );
#endif

	size_t szMaxAllowedImageWidth = 0, szMaxAllowedImageHeight = 0, szMaxAllowedImageDepth = 0;
	cl_err_code clErr = GetMaxImageDimensions(NULL, NULL, &szMaxAllowedImageWidth, &szMaxAllowedImageHeight, &szMaxAllowedImageDepth);
	
	LOG_DEBUG(L"szMaxAllowedImageWidth = %d, szMaxAllowedImageHeight = %d, szMaxAllowedImageDepth=%d", 
		szMaxAllowedImageWidth, szMaxAllowedImageHeight, szMaxAllowedImageDepth);
	
	//check image sizes
	if ((szImageWidth < 1)	||
		(szImageHeight < 1)	||
		(szImageDepth <= 1)	||
		(szImageWidth > szMaxAllowedImageWidth)	||
		(szImageHeight > szMaxAllowedImageHeight) ||
		(szImageDepth > szMaxAllowedImageDepth))
	{
		LOG_ERROR(L"szImageWidth == %d, szImageHeight =%d, szImageDepth = %d", szImageWidth, szImageHeight, szImageDepth);
		return CL_INVALID_IMAGE_SIZE;
	}

	Image3D * pImage3D = new Image3D(this, clFlags, (cl_image_format*)pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, &clErr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new Image3D, returned: %ws", ClErrTxt(clErr));
        delete pImage3D;
		return clErr;
	}

	clErr = pImage3D->Initialize(pHostPtr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Failed to initizialized data, pImage3D->Initialize(pHostPtr = %ws", ClErrTxt(clErr));
        delete pImage3D;
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject*)pImage3D);

	*ppImage3d = pImage3D;
	return CL_SUCCESS;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetSupportedImageFormats
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetSupportedImageFormats(cl_mem_flags clFlags,
											  cl_mem_object_type clType,
											  cl_uint uiNumEntries,
											  cl_image_format * pclImageFormats,
											  cl_uint * puiNumImageFormats)
{
	LOG_DEBUG(L"Enter GetSupportedImageFormats(clFlags=%d, clType=%d, uiNumEntries=%d, pclImageFormats=%d, puiNumImageFormats=%d",
		clFlags, clType, uiNumEntries, pclImageFormats, puiNumImageFormats);

	if ( (uiNumEntries == 0 && pclImageFormats != NULL) )
	{
		LOG_ERROR(L"uiNumEntries == 0 && pclImageFormats != NULL");
		return CL_INVALID_VALUE;
	}

	if ( clType != CL_MEM_OBJECT_IMAGE2D && clType != CL_MEM_OBJECT_IMAGE3D )
	{
		LOG_ERROR(L"clType != CL_MEM_OBJECT_IMAGE2D && clType != CL_MEM_OBJECT_IMAGE3D");
		return CL_INVALID_VALUE;
	}

	if ( ((clFlags & CL_MEM_READ_ONLY) && (clFlags & CL_MEM_WRITE_ONLY)) ||
		 ((clFlags & CL_MEM_READ_ONLY) && (clFlags & CL_MEM_READ_WRITE)) ||
		 ((clFlags & CL_MEM_WRITE_ONLY) && (clFlags & CL_MEM_READ_WRITE)))
	{
		return CL_INVALID_VALUE;
	}

	if (  !(clFlags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)) )
	{
		if ( !(clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)) )
		{
			return CL_INVALID_VALUE;
		}
		// default value;
		clFlags = clFlags | CL_MEM_READ_WRITE;
	}

	// get device flags
	cl_dev_mem_flags clDevMemFlags = DeviceMemoryObject::GetDevMemFlags(clFlags);
	cl_dev_mem_object_type clDevMemObjType =  DeviceMemoryObject::GetDevMemObjType(clType);


	// get supported image types from all devices
	// TODO: prepare the minimum overlapping list from all devices

	Device * pDevice = NULL;
	cl_err_code clErr = CL_SUCCESS;
	for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
	{
		clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject**)&pDevice);
		if (CL_SUCCEEDED(clErr))
		{
			clErr = pDevice->GetSupportedImageFormats(clDevMemFlags, clDevMemObjType, uiNumEntries, pclImageFormats, puiNumImageFormats);
			if (CL_FAILED(clErr))
			{
				return clErr;
			}
		}
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMaxMemAllocSize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_ulong Context::GetMaxMemAllocSize()
{
#ifdef _DEBUG
	assert ( m_pDevices != NULL );
#endif

	LOG_DEBUG(L"Enter GetDeviceMaxMemAllocSize");

	cl_ulong ulMemAllocSize = 0, ulMaxMemAllocSize = 0;
	cl_err_code clErr = CL_SUCCESS;
	Device * pDevice = NULL;
	
	for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
	{
		clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject**)&pDevice);
		if (CL_FAILED(clErr) || NULL == pDevice)
		{
			continue;
		}
		clErr = pDevice->GetInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulMemAllocSize, NULL);
		if (CL_FAILED(clErr))
		{
			continue;
		}
		// get minimum of all maximum
		if (0 == ui) // first iteration
		{
			ulMaxMemAllocSize = ulMemAllocSize;
		}
		else
		{
			ulMaxMemAllocSize = (ulMemAllocSize < ulMaxMemAllocSize) ? ulMemAllocSize : ulMaxMemAllocSize;
		}
	}
    if ( 0 == ulMaxMemAllocSize)
    {
        // No one declared size, probably ignore this value, set default.
        ulMaxMemAllocSize = SHRT_MAX;
    }
	return ulMaxMemAllocSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMaxImageDimensions
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetMaxImageDimensions(size_t * psz2dWidth, 
										   size_t * psz2dHeight, 
										   size_t * psz3dWidth, 
										   size_t * psz3dHeight, 
										   size_t * psz3dDepth)
{
#ifdef _DEBUG
	assert ( "There are no devices associated to the context!!!" && (m_pDevices != NULL) );
	assert ( "wrong input params" && ((psz2dWidth != NULL) || (psz2dHeight != NULL) || (psz3dWidth != NULL) || (psz3dHeight != NULL) || (psz3dDepth != NULL)) );
#endif

	LOG_DEBUG(L"Enter GetMaxAllowedImageWidth");

	size_t sz2dWith = 0, sz2dHeight = 0, szMax2dWith = 0, szMax2dHeight = 0;
	size_t sz3dWith = 0, sz3dHeight = 0, szMax3dWith = 0, szMax3dHeight = 0, sz3dDepth = 0, szMax3dDepth = 0;
	cl_err_code clErr = CL_SUCCESS;
	Device * pDevice = NULL;
	
	for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
	{
		clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject**)&pDevice);
		if (CL_FAILED(clErr) || NULL == pDevice)
		{
			continue;
		}
		if (NULL != psz2dWidth)
		{
			clErr = pDevice->GetInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sz2dWith, NULL);
			if (CL_SUCCEEDED(clErr))
			{
				szMax2dWith = ((0 == ui) || (sz2dWith < szMax2dWith)) ? sz2dWith : szMax2dWith;
			}
		}
		if (NULL != psz2dHeight)
		{
			clErr = pDevice->GetInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sz2dHeight, NULL);
			if (CL_SUCCEEDED(clErr))
			{
				szMax2dHeight = ((0 == ui) || (sz2dHeight < szMax2dHeight)) ? sz2dHeight : szMax2dHeight;
			}
		}
		if (NULL != psz3dWidth)
		{
			clErr = pDevice->GetInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &sz3dWith, NULL);
			if (CL_SUCCEEDED(clErr))
			{
				szMax3dWith = ((0 == ui) || (sz3dWith < szMax3dWith)) ? sz3dWith : szMax3dWith;
			}
		}
		if (NULL != psz3dHeight)
		{
			clErr = pDevice->GetInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &sz3dHeight, NULL);
			if (CL_SUCCEEDED(clErr))
			{
				szMax3dHeight = ((0 == ui) || (sz3dHeight < szMax3dHeight)) ? sz3dHeight : szMax3dHeight;
			}
		}
		if (NULL != psz3dDepth)
		{
			clErr = pDevice->GetInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &sz3dDepth, NULL);
			if (CL_SUCCEEDED(clErr))
			{
				szMax3dDepth = ((0 == ui) || (sz3dDepth < szMax3dDepth)) ? sz3dDepth : szMax3dDepth;
			}
		}
	}

	if (NULL != psz2dWidth)
	{
		*psz2dWidth = szMax2dWith;
	}
	if (NULL != psz2dHeight)
	{
		*psz2dHeight = szMax2dHeight;
	}
	if (NULL != psz3dWidth)
	{
		*psz3dWidth = szMax3dWith;
	}
	if (NULL != psz3dHeight)
	{
		*psz3dHeight = szMax3dHeight;
	}
	if (NULL != psz3dDepth)
	{
		*psz3dDepth = szMax3dDepth;
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetMemObject(cl_mem clMemId, MemoryObject ** ppMemObj)
{
#ifdef _DEBUG
	assert ( NULL != m_pMemObjects );
#endif
	return m_pMemObjects->GetOCLObject((cl_int)clMemId, (OCLObject**)ppMemObj);
}
void Context::NotifyError(const char * pcErrInfo, const void * pPrivateInfo, size_t szCb)
{
	if (NULL != m_pfnNotify)
	{
		m_pfnNotify(pcErrInfo, pPrivateInfo, szCb, m_pUserData);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateSampler(cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode, Sampler ** ppSampler)
{
	LOG_DEBUG(L"Enter CreateSampler (bNormalizedCoords=%d, clAddressingMode=%d, clFilterMode=%d, ppSampler=%d)", 
		bNormalizedCoords, clAddressingMode, clFilterMode, ppSampler);

#ifdef _DEBUG
	assert ( NULL != ppSampler );
	assert ( NULL != m_pSamplers );
#endif

	Sampler * pSampler = new Sampler();
	cl_err_code clErr = pSampler->Initialize(this, bNormalizedCoords, clAddressingMode, clFilterMode);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Error creating new Sampler, returned: %ws", ClErrTxt(clErr));
        delete pSampler;
		return clErr;
	}
	
	m_pSamplers->AddObject((OCLObject*)pSampler);

	*ppSampler = pSampler;
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetSampler
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetSampler(cl_sampler clSamplerId, Sampler ** ppSampler)
{
#ifdef _DEBUG
	assert ( NULL != m_pSamplers );
#endif
	return m_pSamplers->GetOCLObject((cl_int)clSamplerId, (OCLObject**)ppSampler);
}
Device ** Context::GetDevices(cl_uint * puiNumDevices)
{
	if (NULL != puiNumDevices)
	{
		*puiNumDevices = m_pDevices->Count();
	}
	return m_ppDevices;
}

cl_device_id * Context::GetDeviceIds(cl_uint * puiNumDevices)
{
	if (NULL != puiNumDevices)
	{
		*puiNumDevices = m_pDevices->Count();
	}
	return m_pDeviceIds;
}