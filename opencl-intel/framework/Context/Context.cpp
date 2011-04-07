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
#include "program_with_source.h"
#include "program_with_binary.h"
#include "cl_buffer.h"
#include "image.h"
#include "sampler.h"
#include "cl_sys_defines.h"
#include <cl_utils.h>
#include <Device.h>
#include <cl_objects_map.h>
#include "context_module.h"
#include <cl_local_array.h>


#if defined(USE_GPA)    
	//#include "tal\tal.h"
	#include <ittnotify.h>
#endif
// for debug...???
#include <limits.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::Context(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint uiNumRootDevices, FissionableDevice **ppDevices, logging_fn pfnNotify, void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints, bool bUseTaskalyzer, char cStageMarkerFlags)
{
	//cl_start;

	m_pfnNotify = NULL;
	m_pUserData = NULL;

	INIT_LOGGER_CLIENT(L"Context", LL_DEBUG);
	LOG_DEBUG(TEXT("%S"), TEXT("Context constructor enter"));

	m_pPrograms = new OCLObjectsMap<_cl_program_int>();
	m_pDevices = new OCLObjectsMap<_cl_device_id_int>();
	m_pMemObjects = new OCLObjectsMap<_cl_mem_int>();
	m_pSamplers = new OCLObjectsMap<_cl_sampler_int>();

	m_ppAllDevices = NULL;
    m_ppRootDevices = NULL;
	m_pDeviceIds = NULL;
    m_pOriginalDeviceIds = NULL;
	m_bUseTaskalyzer = bUseTaskalyzer;
	m_cStageMarkerFlags = cStageMarkerFlags;

#ifdef _DEBUG
assert ((NULL != ppDevices) && (uiNumDevices > 0));
#endif
    m_uiNumRootDevices = uiNumRootDevices;

	m_ppAllDevices = new FissionableDevice*[uiNumDevices];
	if (NULL == m_ppAllDevices)
	{
		*pclErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}
    m_ppRootDevices = new Device*[m_uiNumRootDevices];
    if (NULL == m_ppRootDevices)
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_ppAllDevices;
        return;
    }

	m_pDeviceIds = new cl_device_id[uiNumDevices];
	if (NULL == m_pDeviceIds)
	{
		*pclErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_ppAllDevices;
        delete[] m_ppRootDevices;
		return;
	}
    m_pOriginalDeviceIds = new cl_device_id[uiNumDevices];
    if (NULL == m_pDeviceIds)
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_pDeviceIds;
        delete[] m_ppAllDevices;
        delete[] m_ppRootDevices;
        return;
    }

    cl_uint curRoot = 0;
	for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
	{
		m_pDevices->AddObject(ppDevices[ui], false);
		m_ppAllDevices[ui] = ppDevices[ui];
		m_pDeviceIds[ui] = ppDevices[ui]->GetHandle();
        m_pOriginalDeviceIds[ui] = ppDevices[ui]->GetHandle();
        if (ppDevices[ui]->IsRootLevelDevice())
        {
            assert(curRoot < m_uiNumRootDevices);
            m_ppRootDevices[curRoot++] = ppDevices[ui]->GetRootDevice();
        }
	}
    m_pOriginalNumDevices = uiNumDevices;

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
//			m_mapPropertyMap[clProperties[m_uiContextPropCount]] = clProperties[m_uiContextPropCount+1];
			m_uiContextPropCount+=2;
		}
		m_uiContextPropCount++; // last property = NULL;
		// allocate new buffer for context's properties
		m_pclContextProperties = new cl_context_properties[m_uiContextPropCount];
		if (NULL != m_pclContextProperties)
		{
			MEMCPY_S(m_pclContextProperties, m_uiContextPropCount * sizeof(cl_context_properties), clProperties, m_uiContextPropCount * sizeof(cl_context_properties));
		}
	}
	m_pfnNotify = pfnNotify;
	m_pUserData = pUserData;

    //
    // For each device in this context, "create" it (either really create or increment its ref count)
    //
    for(cl_uint idx = 0; idx < uiNumDevices; idx++)
    {
        if (ppDevices[idx]->IsRootLevelDevice())
        {
            ppDevices[idx]->GetRootDevice()->CreateInstance();		    
        }
        else
        {
            ppDevices[idx]->AddPendency();
        }
        ppDevices[idx]->RegisterDeviceFissionObserver(this);
    }

	m_handle.object   = this;
	m_handle.dispatch = pOclEntryPoints;

	*pclErr = CL_SUCCESS;
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

	// Close all device instances. Device will decide whether to close or just decrease ref count
    cl_uint uiNumDevices = m_pDevices->Count();
	for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
	{
#if defined(USE_GPA)
		// Before the thread is closed, we need to flush the
		// trace data into file in order not to lose trace data
		
		// This code was removed for the initial porting of TAL
		// to GPA 4.0 and might be used in later stages
//      TAL_TRACE* trace;
//		if (m_bUseTaskalyzer)
//		{
//			trace = TAL_GetThreadTrace();
//			assert(NULL != trace);
//			
//			TAL_Flush(trace);
//		}
#endif
        m_ppAllDevices[ui]->UnregisterDeviceFissionObserver(this);
        if (m_ppAllDevices[ui]->IsRootLevelDevice())
        {
            m_ppAllDevices[ui]->GetRootDevice()->CloseDeviceInstance();
        }
        m_pDevices->RemoveObject(m_ppAllDevices[ui]->GetHandle());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::~Context()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Context destructor enter"));
    LOG_DEBUG(TEXT("CONTEXT_TEST: Context destructor enter. (id = %d)"), m_iId);

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
	if (NULL != m_ppAllDevices)
	{
		delete[] m_ppAllDevices;
		m_ppAllDevices = NULL;
	}
    if (NULL != m_ppRootDevices)
    {
        delete[] m_ppRootDevices;
        m_ppRootDevices = NULL;
    }
	if (NULL != m_pDeviceIds)
	{
		delete[] m_pDeviceIds;
		m_pDeviceIds = NULL;
	}
    if (NULL != m_pOriginalDeviceIds)
    {
        delete[] m_pOriginalDeviceIds;
        m_pOriginalDeviceIds = NULL;
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
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	LOG_DEBUG(TEXT("Context::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), param_name, param_value_size, param_value, param_value_size_ret);
	
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;
	cl_uint uiVal;
	void * pValue = NULL;
	
	cl_err_code clErrRet = CL_SUCCESS;
	switch ( (cl_context_info)param_name )
	{

	case CL_CONTEXT_REFERENCE_COUNT:
		szParamValueSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_CONTEXT_DEVICES:
		szParamValueSize = sizeof(cl_device_id) * m_pOriginalNumDevices;
		pValue = m_pOriginalDeviceIds;
		break;
	case CL_CONTEXT_PROPERTIES:
		szParamValueSize = sizeof(cl_context_properties) * (m_uiContextPropCount);
		pValue = m_pclContextProperties;
		break;
	case CL_CONTEXT_NUM_DEVICES:
		szParamValueSize = sizeof(cl_uint);
		uiVal = m_pDevices->Count();
		pValue = &uiVal;
		break;

	default:
		LOG_ERROR(TEXT("param_name (=%d) isn't valid"), param_name);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != param_value && param_value_size < szParamValueSize)
	{
		LOG_ERROR(TEXT("param_value_size (=%d) < szParamValueSize (=%d)"), param_value_size, szParamValueSize);
		return CL_INVALID_VALUE;
	}

	// return param value size
	if (NULL != param_value_size_ret)
	{
		*param_value_size_ret = szParamValueSize;
	}

	if (NULL != param_value && szParamValueSize > 0)
	{
		MEMCPY_S(param_value, param_value_size, pValue, szParamValueSize);
	}

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramWithSource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithSource(cl_uint uiCount, const char ** ppcStrings, const size_t * szLengths, Program ** ppProgram)
{
	LOG_DEBUG(TEXT("CreateProgramWithSource enter. uiCount=%d, ppcStrings=%d, szLengths=%d, ppProgram=%d"), uiCount, ppcStrings, szLengths, ppProgram);

    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);
	// check input parameters
	if (NULL == ppProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == ppProgram; return CL_INVALID_VALUE"));
		return CL_INVALID_VALUE;
	}
	if (NULL == m_pPrograms)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED"));
		return CL_ERR_INITILIZATION_FAILED;
	}
	cl_err_code clErrRet;
	// create new program object
	Program* pProgram = new ProgramWithSource(this, uiCount,ppcStrings, szLengths, &clErrRet, (ocl_entry_points*)m_handle.dispatch);
	if (!pProgram)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	pProgram->SetLoggerClient(GET_LOGGER_CLIENT);

	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("Create Program With Source(%d, %d, %d) = %d"),uiCount, ppcStrings, szLengths, clErrRet);
		pProgram->Release();
		*ppProgram = NULL;
		return clErrRet;
	}

	// add program object to programs map list
	m_pPrograms->AddObject((OCLObject<_cl_program_int>*)pProgram);
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
	
	cl_err_code clErrRet = m_pDevices->GetObjectByIndex((cl_int)uiDeviceIndex, (OCLObject<_cl_device_id_int>**)ppDevice);
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
	LOG_DEBUG(TEXT("CheckDevices enter. uiNumDevices=%d, pclDevices=%d"), uiNumDevices, pclDevices);
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		// invalid inputs
		LOG_ERROR(TEXT("%S"), TEXT("0 == uiNumDevices || NULL == pclDevices"));
		return false;
	}
	Device* pDevice;
	cl_err_code clErrRet = CL_SUCCESS;
	for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
	{
		clErrRet = m_pDevices->GetOCLObject((_cl_device_id_int*)pclDevices[ui], reinterpret_cast<OCLObject<_cl_device_id_int>**>(&pDevice));
		if ( CL_FAILED(clErrRet) || NULL == pDevice)
		{
			LOG_ERROR(TEXT("device %d wasn't found in this context"), pclDevices[ui]);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetDevicesFromList
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Context::GetDevicesFromList(cl_uint uiNumDevices, const cl_device_id * pclDevices, FissionableDevice** ppDevices)
{
	LOG_DEBUG(TEXT("GetDeviceFromList enter. uiNumDevices=%d, pclDevices=%d"), uiNumDevices, pclDevices);
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		// invalid inputs
		LOG_ERROR(TEXT("%S"), TEXT("0 == uiNumDevices || NULL == pclDevices"));
		return false;
	}
	cl_err_code clErrRet = CL_SUCCESS;
	for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
	{
		clErrRet = m_pDevices->GetOCLObject((_cl_device_id_int*)pclDevices[ui], reinterpret_cast<OCLObject<_cl_device_id_int>**>(ppDevices + ui));
		if ( CL_FAILED(clErrRet) || NULL == ppDevices[ui])
		{
			LOG_ERROR(TEXT("device %d wasn't found in this context"), pclDevices[ui]);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetUseTaskalyzer
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Context::GetUseTaskalyzer() const
{
	return m_bUseTaskalyzer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetStatusMarkerFlags
///////////////////////////////////////////////////////////////////////////////////////////////////
char Context::GetStatusMarkerFlags() const
{
	return m_cStageMarkerFlags;
}

// Context::CreateProgramWithBinary
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithBinary(cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const size_t * pszLengths, const unsigned char ** ppBinaries, cl_int * piBinaryStatus, Program ** ppProgram)
{
	LOG_DEBUG(TEXT("CreateProgramWithBinary enter. uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d, ppProgram=%d"), 
		uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, ppProgram);
    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);
	
	cl_err_code clErrRet = CL_SUCCESS;
	
	if (NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries)
	{
		// invalid input args
		LOG_ERROR(TEXT("%S"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries"));
		return CL_INVALID_VALUE;
	}
	// check items in pszLengths and in ppBinaries
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		if (0 == pszLengths[ui] || NULL == ppBinaries[ui])
		{
			LOG_ERROR(TEXT("0 == pszLengths[%d] || NULL == ppBinaries[%d]"), ui, ui);
			if (NULL != piBinaryStatus)
			{
				piBinaryStatus[ui] = CL_INVALID_VALUE;
			}
			return CL_INVALID_VALUE;
		}
	}

	// get devices
	FissionableDevice ** ppDevices = new FissionableDevice *[uiNumDevices];
	if (NULL == ppDevices)
	{
		// can't allocate memory for devices
		LOG_ERROR(TEXT("%S"), TEXT("Can't allocated memory for devices"));
		return CL_OUT_OF_HOST_MEMORY;
	}

	// check devices
	bool bRes = GetDevicesFromList(uiNumDevices, pclDeviceList, ppDevices);
	if (false == bRes)
	{
		LOG_ERROR(TEXT("%S"), TEXT("GetDevicesFromList(uiNumDevices, pclDeviceList) = false"));
		delete[] ppDevices;
		return CL_INVALID_DEVICE;
	}

	// create program object
	Program* pProgram = new ProgramWithBinary(this, uiNumDevices, ppDevices, pszLengths, ppBinaries, piBinaryStatus, &clErrRet, (ocl_entry_points *)m_handle.dispatch);
	delete[] ppDevices;

	if (!pProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("Out of memory for creating program"));
		return CL_OUT_OF_HOST_MEMORY;
	}
	pProgram->SetLoggerClient(GET_LOGGER_CLIENT);
	
	m_pPrograms->AddObject(pProgram);
	*ppProgram = pProgram;
	return clErrRet;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveProgram(cl_program clProgramId)
{
	LOG_DEBUG(TEXT("Enter RemoveProgram (clProgramId=%d)"), clProgramId);

#ifdef _DEBUG
	assert ( NULL != m_pPrograms );
#endif

	return m_pPrograms->RemoveObject((_cl_program_int*)clProgramId);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveMemObject(cl_mem clMem)
{
	LOG_DEBUG(TEXT("Enter RemoveMemObject (clMem=%d)"), clMem);

#ifdef _DEBUG
	assert ( NULL != m_pMemObjects );
#endif

	return m_pMemObjects->RemoveObject((_cl_mem_int*)clMem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveSampler
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveSampler(cl_sampler clSampler)
{
	LOG_DEBUG(TEXT("Enter RemoveSampler (clSampler=%d)"), clSampler);

#ifdef _DEBUG
	assert ( NULL != m_pSamplers );
#endif

	return m_pSamplers->RemoveObject((_cl_sampler_int*)clSampler);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, Buffer ** ppBuffer)
{
	LOG_DEBUG(TEXT("Enter CreateBuffer (cl_mem_flags=%d, szSize=%d, pHostPtr=%d, ppBuffer=%d)"), 
		clFlags, szSize, pHostPtr, ppBuffer);

    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);

#ifdef _DEBUG
	assert ( NULL != ppBuffer );
	assert ( NULL != m_pMemObjects );
#endif

	cl_ulong ulMaxMemAllocSize = GetMaxMemAllocSize();
	LOG_DEBUG(TEXT("GetMaxMemAllocSize() = %d"), ulMaxMemAllocSize);
	
	// check buffer size
	if (szSize == 0 || szSize > ulMaxMemAllocSize)
	{
		LOG_ERROR(TEXT("szSize == %d, ulMaxMemAllocSize =%d"), szSize, ulMaxMemAllocSize);
		return CL_INVALID_BUFFER_SIZE;
	}

	cl_err_code clErr = CL_SUCCESS;
	Buffer * pBuffer = new Buffer(this, clFlags, szSize, (ocl_entry_points*)m_handle.dispatch, &clErr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new buffer, returned: %S"), ClErrTxt(clErr));
        pBuffer->Release();
		return clErr;
	}

	clErr = pBuffer->Initialize(pHostPtr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Failed to initialize data, pBuffer->Initialize(pHostPtr = %S"), ClErrTxt(clErr));
        pBuffer->Release();
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject<_cl_mem_int>*)pBuffer);

	*ppBuffer = pBuffer;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateSubBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateSubBuffer(Buffer* clBuffer, cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type, const void * buffer_create_info, Buffer ** ppBuffer)
{
	LOG_DEBUG(TEXT("Enter CreateBuffer (cl_mem_flags=%d, buffer_create_type=%d, ppBuffer=%d)"), 
		clFlags, buffer_create_type, ppBuffer);

#ifdef _DEBUG
	assert ( NULL != ppBuffer );
	assert ( NULL != m_pMemObjects );
#endif

    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);

	cl_err_code clErr = CL_SUCCESS;		

	SubBuffer * pBuffer = new SubBuffer(this, clFlags, (ocl_entry_points*)m_handle.dispatch, &clErr);
	if (pBuffer == NULL)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new buffer, returned: %S"), ClErrTxt(clErr));
        pBuffer->Release();
		return clErr;
	}		
		
	clErr = pBuffer->initialize(clBuffer, buffer_create_type, buffer_create_info);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error initializing sub buffer, returned: %S"), ClErrTxt(clErr));
        pBuffer->Release();
		return clErr;
	}
	m_pMemObjects->AddObject(pBuffer);

	*ppBuffer = pBuffer;
	return CL_SUCCESS;
}
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
	LOG_DEBUG(TEXT("Enter CreateImage2D (clFlags=%d, pclImageFormat=%d, pHostPtr=%d, szImageWidth=%d, szImageHeight=%d, szImageRowPitch=%d, ppImage2d=%d)"), 
		clFlags, pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, ppImage2d);

    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);
#ifdef _DEBUG
	assert ( NULL != ppImage2d );
	assert ( NULL != m_pMemObjects );
#endif

	size_t szMaxAllowedImageWidth = 0, szMaxAllowedImageHeight = 0;
	cl_err_code clErr = GetMaxImageDimensions(&szMaxAllowedImageWidth, &szMaxAllowedImageHeight, NULL, NULL, NULL);
	
	LOG_DEBUG(TEXT("szMaxAllowedImageWidth = %d, szMaxAllowedImageHeight = %d"), szMaxAllowedImageWidth, szMaxAllowedImageHeight);
	
	//check image sizes
	if ((szImageWidth < 1)	||
		(szImageHeight < 1)	||
		(szImageWidth > szMaxAllowedImageWidth)	||
		(szImageHeight > szMaxAllowedImageHeight))
	{
		LOG_ERROR(TEXT("szImageWidth == %d, szImageHeight =%d"), szImageWidth, szImageHeight);
		return CL_INVALID_IMAGE_SIZE;
	}

	Image2D * pImage2D = new Image2D(this, clFlags, (cl_image_format*)pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageRowPitch, (ocl_entry_points*)m_handle.dispatch, &clErr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new Image2D, returned: %S"), ClErrTxt(clErr));
        pImage2D->Release();
		return clErr;
	}

	clErr = pImage2D->Initialize(pHostPtr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Failed to initialize data, pImage2D->Initialize(pHostPtr = %S"), ClErrTxt(clErr));
        pImage2D->Release();
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject<_cl_mem_int>*)pImage2D);

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
	LOG_DEBUG(TEXT("Enter CreateImage3D (clFlags=%d, pclImageFormat=%d, pHostPtr=%d, szImageWidth=%d, szImageHeight=%d, szImageDepth=%d, szImageRowPitch=%d, szImageSlicePitch=%d, ppImage2d=%d)"), 
		clFlags, pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, ppImage3d);

    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);
#ifdef _DEBUG
	assert ( NULL != ppImage3d );
	assert ( NULL != m_pMemObjects );
#endif

	size_t szMaxAllowedImageWidth = 0, szMaxAllowedImageHeight = 0, szMaxAllowedImageDepth = 0;
	cl_err_code clErr = GetMaxImageDimensions(NULL, NULL, &szMaxAllowedImageWidth, &szMaxAllowedImageHeight, &szMaxAllowedImageDepth);
	
	LOG_DEBUG(TEXT("szMaxAllowedImageWidth = %d, szMaxAllowedImageHeight = %d, szMaxAllowedImageDepth=%d"), 
		szMaxAllowedImageWidth, szMaxAllowedImageHeight, szMaxAllowedImageDepth);
	
	//check image sizes
	if ((szImageWidth < 1)	||
		(szImageHeight < 1)	||
		(szImageDepth <= 1)	||
		(szImageWidth > szMaxAllowedImageWidth)	||
		(szImageHeight > szMaxAllowedImageHeight) ||
		(szImageDepth > szMaxAllowedImageDepth))
	{
		LOG_ERROR(TEXT("szImageWidth == %d, szImageHeight =%d, szImageDepth = %d"), szImageWidth, szImageHeight, szImageDepth);
		return CL_INVALID_IMAGE_SIZE;
	}

	Image3D * pImage3D = new Image3D(this, clFlags, (cl_image_format*)pclImageFormat, pHostPtr, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch, (ocl_entry_points*)m_handle.dispatch, &clErr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new Image3D, returned: %S"), ClErrTxt(clErr));
        pImage3D->Release();
		return clErr;
	}

	clErr = pImage3D->Initialize(pHostPtr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Failed to initialize data, pImage3D->Initialize(pHostPtr = %S"), ClErrTxt(clErr));
        pImage3D->Release();
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject<_cl_mem_int>*)pImage3D);

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
	LOG_DEBUG(TEXT("Enter GetSupportedImageFormats(clFlags=%d, clType=%d, uiNumEntries=%d, pclImageFormats=%d, puiNumImageFormats=%d"),
		clFlags, clType, uiNumEntries, pclImageFormats, puiNumImageFormats);

	if ( (uiNumEntries == 0 && pclImageFormats != NULL) )
	{
		LOG_ERROR(TEXT("%S"), TEXT("uiNumEntries == 0 && pclImageFormats != NULL"));
		return CL_INVALID_VALUE;
	}

	if ( clType != CL_MEM_OBJECT_IMAGE2D && clType != CL_MEM_OBJECT_IMAGE3D )
	{
		LOG_ERROR(TEXT("%S"), TEXT("clType != CL_MEM_OBJECT_IMAGE2D && clType != CL_MEM_OBJECT_IMAGE3D"));
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
		clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject<_cl_device_id_int>**)&pDevice);
		if (CL_SUCCEEDED(clErr))
		{
			clErr = pDevice->GetDeviceAgent()->clDevGetSupportedImageFormats(clDevMemFlags, clDevMemObjType, uiNumEntries, pclImageFormats, puiNumImageFormats);
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

	LOG_DEBUG(TEXT("%S"), TEXT("Enter GetDeviceMaxMemAllocSize"));

	cl_ulong ulMemAllocSize = 0, ulMaxMemAllocSize = 0;
	cl_err_code clErr = CL_SUCCESS;
	Device * pDevice = NULL;
	
	for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
	{
		clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject<_cl_device_id_int>**)&pDevice);
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

	LOG_DEBUG(TEXT("%S"), TEXT("Enter GetMaxAllowedImageWidth"));

	size_t sz2dWith = 0, sz2dHeight = 0, szMax2dWith = 0, szMax2dHeight = 0;
	size_t sz3dWith = 0, sz3dHeight = 0, szMax3dWith = 0, szMax3dHeight = 0, sz3dDepth = 0, szMax3dDepth = 0;
	cl_err_code clErr = CL_SUCCESS;
	Device * pDevice = NULL;
	
	for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
	{
		clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject<_cl_device_id_int>**)&pDevice);
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
	return m_pMemObjects->GetOCLObject((_cl_mem_int*)clMemId, (OCLObject<_cl_mem_int>**)ppMemObj);
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
	LOG_DEBUG(TEXT("Enter CreateSampler (bNormalizedCoords=%d, clAddressingMode=%d, clFilterMode=%d, ppSampler=%d)"), 
		bNormalizedCoords, clAddressingMode, clFilterMode, ppSampler);

#ifdef _DEBUG
	assert ( NULL != ppSampler );
	assert ( NULL != m_pSamplers );
#endif

	Sampler * pSampler = new Sampler();
	cl_err_code clErr = pSampler->Initialize(this, bNormalizedCoords, clAddressingMode, clFilterMode, (ocl_entry_points*)m_handle.dispatch);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new Sampler, returned: %S"), ClErrTxt(clErr));
        pSampler->Release();
		return clErr;
	}
	
	m_pSamplers->AddObject((OCLObject<_cl_sampler_int>*)pSampler);

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
	return m_pSamplers->GetOCLObject((_cl_sampler_int*)clSamplerId, (OCLObject<_cl_sampler_int>**)ppSampler);
}
FissionableDevice ** Context::GetDevices(cl_uint * puiNumDevices)
{
    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);

	if (NULL != puiNumDevices)
	{
		*puiNumDevices = m_pDevices->Count();
	}
	return m_ppAllDevices;
}

Device** Context::GetRootDevices(cl_uint* puiNumDevices)
{
    if (NULL != puiNumDevices)
    {
        *puiNumDevices = m_uiNumRootDevices;
    }
    return m_ppRootDevices;
}

cl_device_id * Context::GetDeviceIds(cl_uint * puiNumDevices)
{
    //Acquire a reader lock on the device map to prevent race with device fission
    OclAutoReader CS(&m_deviceDependentObjectsLock);

	if (NULL != puiNumDevices)
	{
		*puiNumDevices = m_pDevices->Count();
	}
	return m_pDeviceIds;
}

cl_dev_subdevice_id Context::GetSubdeviceId(cl_device_id id)
{
    OclAutoReader CS(&m_deviceDependentObjectsLock);
    FissionableDevice* pDevice;
    if (CL_SUCCESS != m_pDevices->GetOCLObject((_cl_device_id_int*)id, (OCLObject<_cl_device_id_int>**)(&pDevice)))
    {
        return 0;
    }
    SubDevice* pSubdevice = dynamic_cast<SubDevice*>(pDevice);
    if (NULL == pSubdevice)
    {
        return 0;
    }
    return pSubdevice->GetSubdeviceId();
}

cl_err_code Context::NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children)
{
    cl_err_code ret = CL_SUCCESS;

    // Prevent further accesses to device-specific objects
    // This will block further calls to clCreate<MemObj> and clCreateProgramWithXXX
    OclAutoWriter CS(&m_deviceDependentObjectsLock);

    size_t prevNumDevices = m_pDevices->Count();
    cl_device_id* pNewDeviceIds = new cl_device_id[prevNumDevices + count];
    if (NULL == pNewDeviceIds)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    FissionableDevice** pNewDevices = new FissionableDevice* [prevNumDevices + count];
    if (NULL == pNewDeviceIds)
    {
        delete[] pNewDeviceIds;
        return CL_OUT_OF_HOST_MEMORY;
    }
    MEMCPY_S(pNewDeviceIds, sizeof(cl_device_id) * prevNumDevices, m_pDeviceIds, sizeof(cl_device_id) * prevNumDevices);
    MEMCPY_S(pNewDevices, sizeof(FissionableDevice*) * prevNumDevices, m_ppAllDevices, sizeof(FissionableDevice*) * prevNumDevices);

    for (size_t i = 0; i < count; ++i)
    {
        pNewDeviceIds[prevNumDevices + i] = children[i]->GetHandle();
        pNewDevices[prevNumDevices + i]   = children[i];
        m_pDevices->AddObject(children[i], false);
    }

    delete m_pDeviceIds;
    m_pDeviceIds = pNewDeviceIds;
    delete[] m_ppAllDevices;
    m_ppAllDevices = pNewDevices;

    //Now, notify the dependents
    cl_uint numMemObjects, numPrograms;
    OCLObject<_cl_mem_int>** memObjs   = NULL;
    OCLObject<_cl_program_int>** progs = NULL;

    do //I have to iterate as it's possible another object will be added between querying for count and querying for objects
    {
        if (NULL != memObjs)
        {
            delete[] memObjs;
        }
        if (NULL != progs)
        {
            delete[] progs;
        }

        numMemObjects = m_pMemObjects->Count();
        numPrograms   = m_pPrograms->Count();

        //Todo: use local array here
        memObjs       = new OCLObject<_cl_mem_int>*[numMemObjects];
        progs         = new OCLObject<_cl_program_int>*[numPrograms];

        assert(memObjs);
        assert(progs);

        ret  = m_pMemObjects->GetObjects(numMemObjects, memObjs, NULL);
        ret |= m_pPrograms->GetObjects(numPrograms, progs, NULL);
    } while (CL_SUCCESS != ret);

    //Iterate over all programs and all memory objects and notify them of the new device
    for (cl_uint i = 0; i < numMemObjects; ++i)
    {
        MemoryObject* pMemObj = dynamic_cast<MemoryObject*>(memObjs[i]);
        assert(pMemObj);
        pMemObj->NotifyDeviceFissioned(parent, count, children);
    }
    for (cl_uint i = 0; i < numPrograms; ++i)
    {
        Program* pProgram = dynamic_cast<Program*>(progs[i]);
        assert(pProgram);
        pProgram->NotifyDeviceFissioned(parent, count, children);
    }
    delete[] memObjs;
    delete[] progs;
    return CL_SUCCESS;
}
