// Copyright (c) 2006-2012 Intel Corporation
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

// for debug...???
#include <limits.h>
#include <assert.h>
#include <algorithm>

#include "Context.h"
#include "Device.h"
#include "program_with_source.h"
#include "program_with_binary.h"
#include "program_for_link.h"
#include "program_service.h"
#include "sampler.h"
#include "cl_sys_defines.h"
#include "context_module.h"
#include "MemoryAllocator/MemoryObjectFactory.h"
#include "MemoryAllocator/MemoryObject.h"
#include "ocl_itt.h"
#include <cl_utils.h>
#include <cl_objects_map.h>
#include <cl_local_array.h>
#include <task_executor.h>

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::TaskExecutor;

// Function to compare two image formats.
static bool compareImageFormats(cl_image_format f1, cl_image_format f2);
// Function to get format map key
static int getFormatsKey(int clObjType , int clMemFlags);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::Context(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint uiNumRootDevices, FissionableDevice **ppDevices, logging_fn pfnNotify, void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData)
	: OCLObject<_cl_context_int>("Context"), m_devTypeMask(0), m_programService(this), m_pfnNotify(NULL), m_pUserData(NULL), m_ulMaxMemAllocSize(0),m_MemObjectsHeap(NULL)
{

	INIT_LOGGER_CLIENT(TEXT("Context"), LL_DEBUG);
	LOG_DEBUG(TEXT("%S"), TEXT("Context constructor enter"));

	m_bTEActivated = GetTaskExecutor()->Activate();
	if ( !m_bTEActivated )
	{
		*pclErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}

	m_ppAllDevices = NULL;
    m_ppExplicitRootDevices = NULL;
	m_pDeviceIds = NULL;
    m_pOriginalDeviceIds = NULL;
	m_pGPAData = pGPAData;

	if ((0 != clCreateHeap( 0, 0, &m_MemObjectsHeap )) || (NULL == m_MemObjectsHeap))
	{
		*pclErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}

	assert ((NULL != ppDevices) && (uiNumDevices > 0));
    m_uiNumRootDevices = uiNumRootDevices;

	m_ppAllDevices = new FissionableDevice*[uiNumDevices];
	if (NULL == m_ppAllDevices)
	{
		*pclErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}
    m_ppExplicitRootDevices = new Device*[m_uiNumRootDevices];
    if (NULL == m_ppExplicitRootDevices)
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
        delete[] m_ppExplicitRootDevices;
		return;
	}
    m_pOriginalDeviceIds = new cl_device_id[uiNumDevices];
    if (NULL == m_pDeviceIds)
    {
        *pclErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_pDeviceIds;
        delete[] m_ppAllDevices;
        delete[] m_ppExplicitRootDevices;
        return;
    }

    cl_uint curRoot = 0;
	for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
	{
		ppDevices[ui]->AddedToContext();
		m_mapDevices.AddObject(ppDevices[ui], false);
		m_ppAllDevices[ui] = ppDevices[ui];
		m_pDeviceIds[ui] = ppDevices[ui]->GetHandle();
        m_pOriginalDeviceIds[ui] = ppDevices[ui]->GetHandle();

        // Create a set of all root devices implicitly/explicitly defined in the context.
        m_allRootDevices.insert(ppDevices[ui]->GetRootDevice());

        // Create a list of all explicit root devices in the context.
        if (ppDevices[ui]->IsRootLevelDevice())
        {
            assert(curRoot < m_uiNumRootDevices);
            // GetRootDevice used just for the purpose of casting to Device*.
            m_ppExplicitRootDevices[curRoot++] = ppDevices[ui]->GetRootDevice();
        }

		cl_bitfield devType = ppDevices[ui]->GetRootDevice()->GetDeviceType();
		m_devTypeMask |= devType;
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
	cl_err_code ret = CL_SUCCESS;
	cl_uint idx = 0;
	for(; idx < uiNumDevices && CL_SUCCEEDED(ret); idx++)
    {
		// Need to make insure that instance of DeviceAgent exists
        if (ppDevices[idx]->IsRootLevelDevice())
        {
            ret = ppDevices[idx]->GetRootDevice()->CreateInstance();		    
        }
        else
		{
            ppDevices[idx]->AddPendency(this);
        }
	}
	if ( CL_FAILED(ret) )
	{
		for(cl_uint ui=0; ui<(idx-1); ++ui)
		{
			if (m_ppAllDevices[ui]->IsRootLevelDevice())
			{
				m_ppAllDevices[ui]->GetRootDevice()->CloseDeviceInstance();
			}
			else
			{
				m_ppAllDevices[ui]->RemovePendency(this);
			}

	        m_mapDevices.RemoveObject(ppDevices[ui]->GetHandle());
			ppDevices[ui]->RemovedFromContext();
		}
		*pclErr = ret;
		m_mapDevices.ReleaseAllObjects(false);
		return;
	}
	GetMaxImageDimensions(&m_sz2dWidth, &m_sz2dHeight, &m_sz3dWidth, &m_sz3dHeight, &m_sz3dDepth, &m_szArraySize, &m_sz1dImgBufSize);

	m_handle.object   = this;
    *((ocl_entry_points*)(&m_handle)) = *pOclEntryPoints;

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
    cl_uint uiNumDevices = m_mapDevices.Count();
	for (cl_uint ui = 0; ui < uiNumDevices; ++ui)
	{
		// The pendency to the device implicitly removed by RemoveObject()
        if (m_ppAllDevices[ui]->IsRootLevelDevice())
        {
            m_ppAllDevices[ui]->GetRootDevice()->CloseDeviceInstance();
        }
		else
		{
			m_ppAllDevices[ui]->RemovePendency(this);
        }
        m_mapDevices.RemoveObject(m_ppAllDevices[ui]->GetHandle());
		m_ppAllDevices[ui]->RemovedFromContext();
	}

	if ( m_bTEActivated )
	{
		GetTaskExecutor()->Deactivate();
		m_bTEActivated = false;
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

	clDeleteHeap( m_MemObjectsHeap );

	RELEASE_LOGGER_CLIENT;

	m_mapPrograms.Clear();

    m_mapDevices.Clear();
 
	if (NULL != m_ppAllDevices)
	{
		delete[] m_ppAllDevices;
		m_ppAllDevices = NULL;
	}
    if (NULL != m_ppExplicitRootDevices)
    {
        delete[] m_ppExplicitRootDevices;
        m_ppExplicitRootDevices = NULL;
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
    m_mapMemObjects.Clear();
    m_mapSamplers.Clear();

	if (NULL != m_pclContextProperties)
	{
		delete []m_pclContextProperties;
		m_pclContextProperties = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) const
{
	LOG_DEBUG(TEXT("Context::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), param_name, param_value_size, param_value, param_value_size_ret);
	
	size_t szParamValueSize = 0;
	cl_uint uiVal;
	const void * pValue = NULL;
	
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
		uiVal = (cl_uint)m_mapDevices.Count();
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

	// check input parameters
	if (NULL == ppProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == ppProgram; return CL_INVALID_VALUE"));
		return CL_INVALID_VALUE;
	}
	cl_err_code clErrRet = CL_SUCCESS;
	// create new program object
	Program* pProgram = new ProgramWithSource(this, uiCount,ppcStrings, szLengths, &clErrRet, (ocl_entry_points*)&m_handle);
	if (!pProgram)
	{
        if (CL_SUCCESS != clErrRet)
        {
            return clErrRet;
        }
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
	m_mapPrograms.AddObject((OCLObject<_cl_program_int>*)pProgram);
	*ppProgram = pProgram;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateProgramForLink
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramForLink(cl_uint				IN  uiNumDevices, 
							              const cl_device_id *	IN  pclDeviceList, 
								          Program **			OUT ppProgram)
{
    LOG_DEBUG(TEXT("CreateProgramFromLink enter. uiNumDevices=%d, pclDeviceList=%d, ppProgram=%d"), uiNumDevices, pclDeviceList, ppProgram);
	
    cl_err_code clErrRet = CL_SUCCESS;

    // check input parameters
	if (NULL == ppProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == ppProgram; return CL_INVALID_VALUE"));
		return CL_INVALID_VALUE;
	}

    if (NULL == pclDeviceList || 0 == uiNumDevices)
	{
		// invalid input args
		LOG_ERROR(TEXT("%S"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices"));
		return CL_INVALID_VALUE;
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
	Program* pProgram = new ProgramForLink(this, uiNumDevices, ppDevices, &clErrRet, (ocl_entry_points*)&m_handle);
	delete[] ppDevices;

	if (!pProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("Out of memory for creating program"));
		return CL_OUT_OF_HOST_MEMORY;
	}
	pProgram->SetLoggerClient(GET_LOGGER_CLIENT);
	
	m_mapPrograms.AddObject(pProgram);
	*ppProgram = pProgram;
	return clErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CompileProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CompileProgram(cl_program	IN  clProgram, 
                           cl_uint				IN  uiNumDevices,
						   const cl_device_id*	IN  pclDeviceList, 
                           cl_uint				IN  uiNumHeaders,
                           const cl_program*	IN  pclHeaders, 
                           const char**         IN  pszHeadersNames, 
                           const char*          IN  szOptions, 
                           pfnNotifyBuildDone   IN  pfn_notify,
                           void*                IN  user_data)
{
    Program* pProg;
    cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProg);
    if (CL_FAILED(clErrRet) || NULL == pProg)
	{
		LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}

    Program** ppHeaders = NULL;

    if (0 < uiNumHeaders)
    {
        // This array will be freed by the program service
        ppHeaders = new Program*[uiNumHeaders];
        if (!ppHeaders)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        for (unsigned int i = 0; i < uiNumHeaders; ++i)
        {
            clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)pclHeaders[i], (OCLObject<_cl_program_int>**)&ppHeaders[i]);
            if (CL_FAILED(clErrRet) || NULL == ppHeaders[i])
	        {
                delete[] ppHeaders;
		        LOG_ERROR(TEXT("One of the header programs %d isn't valid program"), clProgram);
		        return CL_INVALID_PROGRAM;
	        }
        }
    }

    cl_int clErr = m_programService.CompileProgram(pProg, uiNumDevices, pclDeviceList, 
                       uiNumHeaders, ppHeaders, pszHeadersNames, szOptions, pfn_notify, user_data);

    return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::LinkProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::LinkProgram(cl_program				IN  clProgram, 
                                cl_uint					IN  uiNumDevices,
								const cl_device_id*	    IN  pclDeviceList, 
                                cl_uint					IN  uiNumBinaries,
                                const cl_program*		IN  pclBinaries, 
                                const char*             IN  szOptions, 
                                pfnNotifyBuildDone      IN  pfn_notify,
                                void*                   IN  user_data)
{
    Program* pProg;
    cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProg);
    if (CL_FAILED(clErrRet) || NULL == pProg)
	{
		LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}

    Program** ppBinaries = NULL;

    if (0 < uiNumBinaries)
    {
        // This array will be freed by the program service
        ppBinaries = new Program*[uiNumBinaries];
        if (!ppBinaries)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        for (unsigned int i = 0; i < uiNumBinaries; ++i)
        {
            clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)pclBinaries[i], (OCLObject<_cl_program_int>**)&ppBinaries[i]);
            if (CL_FAILED(clErrRet) || NULL == ppBinaries[i])
	        {
                delete[] ppBinaries;
		        LOG_ERROR(TEXT("One of the binaries programs %d isn't valid program"), clProgram);
		        return CL_INVALID_PROGRAM;
	        }
        }
    }

    cl_int clErr = m_programService.LinkProgram(pProg, uiNumDevices, pclDeviceList,
                        uiNumBinaries, ppBinaries, szOptions, pfn_notify, user_data);

    return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::BuildProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::BuildProgram(cl_program			IN  clProgram, 
                                  cl_uint				IN  uiNumDevices,
								  const cl_device_id*   IN  pclDeviceList, 
                                  const char*           IN  szOptions, 
                                  pfnNotifyBuildDone    IN  pfn_notify,
                                  void*                 IN  user_data)
{
    Program* pProg;
    cl_err_code clErrRet = m_mapPrograms.GetOCLObject((_cl_program_int*)clProgram, (OCLObject<_cl_program_int>**)&pProg);
    if (CL_FAILED(clErrRet) || NULL == pProg)
	{
		LOG_ERROR(TEXT("program %d isn't valid program"), clProgram);
		return CL_INVALID_PROGRAM;
	}

    cl_int clErr = m_programService.BuildProgram(pProg, uiNumDevices, pclDeviceList, szOptions, pfn_notify, user_data);
    
    return clErr;
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
	
	cl_err_code clErrRet = m_mapDevices.GetObjectByIndex((cl_int)uiDeviceIndex, (OCLObject<_cl_device_id_int>**)ppDevice);
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
		clErrRet = m_mapDevices.GetOCLObject((_cl_device_id_int*)pclDevices[ui], reinterpret_cast<OCLObject<_cl_device_id_int>**>(&pDevice));
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
		clErrRet = m_mapDevices.GetOCLObject((_cl_device_id_int*)pclDevices[ui], reinterpret_cast<OCLObject<_cl_device_id_int>**>(ppDevices + ui));
		if ( CL_FAILED(clErrRet) || NULL == ppDevices[ui])
		{
			LOG_ERROR(TEXT("device %d wasn't found in this context"), pclDevices[ui]);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetGPAData
///////////////////////////////////////////////////////////////////////////////////////////////////
ocl_gpa_data * Context::GetGPAData() const
{
	return m_pGPAData;
}


// Context::CreateProgramWithBinary
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithBinary(cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const size_t * pszLengths, const unsigned char ** ppBinaries, cl_int * piBinaryStatus, Program ** ppProgram)
{
	LOG_DEBUG(TEXT("CreateProgramWithBinary enter. uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d, ppProgram=%d"), 
		uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, ppProgram);
	
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
	Program* pProgram = new ProgramWithBinary(this, uiNumDevices, ppDevices, pszLengths, ppBinaries, piBinaryStatus, &clErrRet, (ocl_entry_points*)&m_handle);
	delete[] ppDevices;

	if (!pProgram)
	{
		LOG_ERROR(TEXT("%S"), TEXT("Out of memory for creating program"));
		return CL_OUT_OF_HOST_MEMORY;
	}
	pProgram->SetLoggerClient(GET_LOGGER_CLIENT);
	
	m_mapPrograms.AddObject(pProgram);
	*ppProgram = pProgram;
	return clErrRet;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveProgram
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveProgram(cl_program clProgramId)
{
	LOG_DEBUG(TEXT("Enter RemoveProgram (clProgramId=%d)"), clProgramId);

	return m_mapPrograms.RemoveObject((_cl_program_int*)clProgramId);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveMemObject(cl_mem clMem)
{
	LOG_DEBUG(TEXT("Enter RemoveMemObject (clMem=%d)"), clMem);

	return m_mapMemObjects.RemoveObject((_cl_mem_int*)clMem);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::RemoveSampler
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::RemoveSampler(cl_sampler clSampler)
{
	LOG_DEBUG(TEXT("Enter RemoveSampler (clSampler=%d)"), clSampler);

	return m_mapSamplers.RemoveObject((_cl_sampler_int*)clSampler);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, MemoryObject ** ppBuffer)
{
	LOG_DEBUG(TEXT("Enter CreateBuffer (cl_mem_flags=%d, szSize=%d, pHostPtr=%d, ppBuffer=%d)"), 
		clFlags, szSize, pHostPtr, ppBuffer);

	assert ( NULL != ppBuffer );

	cl_ulong ulMaxMemAllocSize = GetMaxMemAllocSize();
	LOG_DEBUG(TEXT("GetMaxMemAllocSize() = %d"), ulMaxMemAllocSize);
	
	// check buffer size
	if (szSize == 0 || szSize > ulMaxMemAllocSize)
	{
		LOG_ERROR(TEXT("szSize == %d, ulMaxMemAllocSize =%d"), szSize, ulMaxMemAllocSize);
		return CL_INVALID_BUFFER_SIZE;
	}

	cl_err_code clErr;
	clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, CL_MEM_OBJECT_BUFFER, CL_MEMOBJ_GFX_SHARE_NONE, this, ppBuffer);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new buffer, returned: %S"), ClErrTxt(clErr));
		return clErr;
	}

	clErr = (*ppBuffer)->Initialize(clFlags, NULL, 1, &szSize, NULL, pHostPtr, 0);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error Initialize new buffer, returned: %S"), ClErrTxt(clErr));
		(*ppBuffer)->Release();
		return clErr;
	}
	m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)*ppBuffer);

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CreateSubBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateSubBuffer(MemoryObject* pBuffer, cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
									 const void * buffer_create_info, MemoryObject** ppBuffer)
{
	LOG_DEBUG(TEXT("Enter CreateBuffer (cl_mem_flags=%d, buffer_create_type=%d, ppBuffer=%d)"), 
		clFlags, buffer_create_type, ppBuffer);

	assert ( NULL != ppBuffer );


	// Parameters check
	if ( CL_BUFFER_CREATE_TYPE_REGION != buffer_create_type )
	{
		return CL_INVALID_VALUE;
	}

	if ( NULL == buffer_create_info )
	{
		return CL_INVALID_VALUE;
	}
	const cl_buffer_region* region = reinterpret_cast<const cl_buffer_region*>(buffer_create_info);
	assert(region);

	if (region->size == 0 )
	{
		return CL_INVALID_BUFFER_SIZE;
	}

	if ( (region->origin + region->size) > pBuffer->GetSize()  )
	{
		return CL_INVALID_VALUE;
	}

	if (CL_SUCCESS != pBuffer->ValidateChildFlags(clFlags))
	{
		return CL_INVALID_VALUE;
	}

	// Copy access flags from parent, if none supplied.
	cl_mem_flags pflags = pBuffer->GetFlags();
	if ( !(clFlags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE)) )
	{
		clFlags |= (pflags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE));
	}
	if ( !(clFlags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY)) )
	{
		clFlags |= (pflags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY));
	}
    // These flags cannot be specified in flags but are inherited from the corresponding memory access qualifiers associated with buffer.
    clFlags |= pflags & (CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR);


	cl_err_code clErr;		
	clErr = pBuffer->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error initializing sub buffer, returned: %S"), ClErrTxt(clErr));
		return clErr;
	}


	m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)*ppBuffer);

	return clErr;
}

// Context::clCreateImageArray
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateImageArray(cl_mem_flags clFlags, const cl_image_format* pclImageFormat, void* pHostPtr, const cl_image_desc* pClImageDesc,
                                      MemoryObject** ppImageArr)
{
	assert(NULL != ppImageArr);
    assert(CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type || CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type);
    
    if (pClImageDesc->image_array_size < 1 || pClImageDesc->image_array_size > m_szArraySize || pClImageDesc->image_width < 1 || pClImageDesc->image_width > m_sz2dWidth ||
        (CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type && (pClImageDesc->image_height < 1 || pClImageDesc->image_height > m_sz2dHeight)))
    {
        return CL_INVALID_IMAGE_DESCRIPTOR;
    }

	const size_t pixelBytesCnt = clGetPixelBytesCount(pclImageFormat);
    // handle the mess in the spec, where for 1D image array the slice pitch defines the size in bytes of each 1D image
    const size_t szPitchDim1 =
        CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type ?
        (0 == pClImageDesc->image_slice_pitch ? pClImageDesc->image_width * pixelBytesCnt : pClImageDesc->image_slice_pitch) :
        (0 == pClImageDesc->image_row_pitch ? pClImageDesc->image_width * pixelBytesCnt : pClImageDesc->image_row_pitch);
    const size_t szPitchDim2 =
        CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type ?
        0 :
        0 == pClImageDesc->image_slice_pitch ? szPitchDim1 * pClImageDesc->image_height : pClImageDesc->image_slice_pitch;

    // flags and imageFormat are validated by Image2D and MemoryObject contained inside Image2DArray and hostPtr by MemoryObject::initialize.
	cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, pClImageDesc->image_type, CL_MEMOBJ_GFX_SHARE_NONE, this, ppImageArr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new Image3D, returned: %ws"), ClErrTxt(clErr));
		return clErr;
	}

    size_t dim[3] = {pClImageDesc->image_width};
	const size_t pitch[2] = {szPitchDim1, szPitchDim2};

    if (CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type)
    {
        dim[1] = pClImageDesc->image_array_size;
        clErr = (*ppImageArr)->Initialize(clFlags, pclImageFormat, 2, dim, pitch, pHostPtr, 0);
    }
    else
    {
        dim[1] = pClImageDesc->image_height;
        dim[2] = pClImageDesc->image_array_size;
        clErr = (*ppImageArr)->Initialize(clFlags, pclImageFormat, 3, dim, pitch, pHostPtr, 0);
    }	
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error Initialize new buffer, returned: %S"), ClErrTxt(clErr));
		(*ppImageArr)->Release();
		return clErr;
	}
	m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)*ppImageArr);
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

	if (clType == CL_MEM_OBJECT_BUFFER)
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

	// get supported image types from all devices
	// TODO: prepare the minimum overlapping list from all devices
	// Need to iterate only over root devices
	assert(m_mapDevices.Count() == 1);

	cl_err_code clErr = CL_SUCCESS;
	for (cl_uint ui=0; ui<m_mapDevices.Count(); ++ui)
	{
		clErr = m_ppAllDevices[ui]->GetDeviceAgent()->clDevGetSupportedImageFormats(clFlags, clType, uiNumEntries, pclImageFormats, puiNumImageFormats);
		if (CL_FAILED(clErr))
		{
			return clErr;
		}
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMaxMemAllocSize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_ulong Context::GetMaxMemAllocSize()
{
	if ( 0 != m_ulMaxMemAllocSize )
	{
		return m_ulMaxMemAllocSize;
	}

	LOG_DEBUG(TEXT("%S"), TEXT("Enter GetDeviceMaxMemAllocSize"));

	cl_ulong ulMemAllocSize = 0;
	
	for (cl_uint ui=0; ui<m_mapDevices.Count(); ++ui)
	{
		cl_err_code clErr = m_ppAllDevices[ui]->GetInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulMemAllocSize, NULL);
		if (CL_FAILED(clErr))
		{
			continue;
		}
		m_ulMaxMemAllocSize = (0 == m_ulMaxMemAllocSize) ? ulMemAllocSize :
									(ulMemAllocSize < m_ulMaxMemAllocSize) ? ulMemAllocSize : m_ulMaxMemAllocSize;
	}

	return m_ulMaxMemAllocSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMaxImageDimensions
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetMaxImageDimensions(size_t * psz2dWidth, 
										   size_t * psz2dHeight, 
										   size_t * psz3dWidth, 
										   size_t * psz3dHeight, 
										   size_t * psz3dDepth,
                                           size_t * pszArraySize,
                                           size_t * psz1dImgBufSize)
{
	assert ( "wrong input params" && ((psz2dWidth != NULL) || (psz2dHeight != NULL) || (psz3dWidth != NULL) || (psz3dHeight != NULL) || (psz3dDepth != NULL)) );

	LOG_DEBUG(TEXT("%S"), TEXT("Enter GetMaxAllowedImageWidth"));

	size_t sz2dWith = 0, sz2dHeight = 0, szMax2dWith = 0, szMax2dHeight = 0;
	size_t sz3dWith = 0, sz3dHeight = 0, szMax3dWith = 0, szMax3dHeight = 0, sz3dDepth = 0, szMax3dDepth = 0;
    size_t szArraySize = 0, szMaxArraySize = 0;
    size_t sz1dImgBufSize = 0, szMax1dImgBufSize = 0;
	cl_err_code clErr = CL_SUCCESS;
	Device * pDevice = NULL;
	
	for (cl_uint ui=0; ui<m_mapDevices.Count(); ++ui)
	{
		clErr = m_mapDevices.GetObjectByIndex(ui, (OCLObject<_cl_device_id_int>**)&pDevice);
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
        if (NULL != pszArraySize)
        {
            clErr = pDevice->GetInfo(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(size_t), &szArraySize, NULL);
            if (CL_SUCCEEDED(clErr))
            {
                szMaxArraySize = ((0 == ui) || (szArraySize < szMaxArraySize)) ? szArraySize : szMaxArraySize;
            }
        }
        if (NULL != psz1dImgBufSize)
        {
            clErr = pDevice->GetInfo(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, sizeof(size_t), &sz1dImgBufSize, NULL);
            if (CL_SUCCEEDED(clErr))
            {
                szMax1dImgBufSize = 0 == ui || sz1dImgBufSize < szMax1dImgBufSize ? sz1dImgBufSize : szMax1dImgBufSize;
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
    if (NULL != pszArraySize)
    {
        *pszArraySize = szMaxArraySize;
    }
    if (NULL != psz1dImgBufSize)
    {
        *psz1dImgBufSize = szMax1dImgBufSize;
    }
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetMemObject
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetMemObject(cl_mem clMemId, MemoryObject ** ppMemObj)
{
	return m_mapMemObjects.GetOCLObject((_cl_mem_int*)clMemId, (OCLObject<_cl_mem_int>**)ppMemObj);
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
	assert ( NULL != ppSampler );
	LOG_DEBUG(TEXT("Enter CreateSampler (bNormalizedCoords=%d, clAddressingMode=%d, clFilterMode=%d, ppSampler=%d)"), 
		bNormalizedCoords, clAddressingMode, clFilterMode, ppSampler);

#ifdef _DEBUG
	assert ( NULL != ppSampler );
#endif

	Sampler * pSampler = new Sampler();
	cl_err_code clErr = pSampler->Initialize(this, bNormalizedCoords, clAddressingMode, clFilterMode, (ocl_entry_points*)&m_handle);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(TEXT("Error creating new Sampler, returned: %S"), ClErrTxt(clErr));
        pSampler->Release();
		return clErr;
	}
	
	m_mapSamplers.AddObject((OCLObject<_cl_sampler_int>*)pSampler);

	*ppSampler = pSampler;
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::GetSampler
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetSampler(cl_sampler clSamplerId, Sampler ** ppSampler)
{
	return m_mapSamplers.GetOCLObject((_cl_sampler_int*)clSamplerId, (OCLObject<_cl_sampler_int>**)ppSampler);
}
FissionableDevice ** Context::GetDevices(cl_uint * puiNumDevices)
{
	if (NULL != puiNumDevices)
	{
		*puiNumDevices = m_mapDevices.Count();
	}
	return m_ppAllDevices;
}

Device** Context::GetExplicitlyAssociatedRootDevices(cl_uint* puiNumDevices)
{
    if (NULL != puiNumDevices)
    {
        *puiNumDevices = m_uiNumRootDevices;
    }
    return m_ppExplicitRootDevices;
}

const tSetOfDevices *Context::GetAllRootDevices() const
{
	return &m_allRootDevices;
}

cl_device_id * Context::GetDeviceIds(size_t * puiNumDevices)
{
	if (NULL != puiNumDevices)
	{
		*puiNumDevices = m_mapDevices.Count();
	}
	return m_pDeviceIds;
}

cl_dev_subdevice_id Context::GetSubdeviceId(cl_device_id id)
{
    FissionableDevice* pDevice;
    if (CL_SUCCESS != m_mapDevices.GetOCLObject((_cl_device_id_int*)id, (OCLObject<_cl_device_id_int>**)(&pDevice)))
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

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context::CheckSupportedImageFormat
// Calculate the supported file formats for context.
// UNION of all device capabilities (see clGetSupportedImageFormats).
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CheckSupportedImageFormat( const cl_image_format* pclImageFormat, cl_mem_flags clMemFlags, cl_mem_object_type clObjType)
{
	// Check for invalid format
	if (NULL == pclImageFormat || (0 == clGetPixelBytesCount(pclImageFormat)) )
	{
		return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
	}

	// Calculate supported format key
	int key = getFormatsKey(clObjType, clMemFlags);

	tImageFormatMap::iterator mapIT;
	{	// Critical section
		OclAutoMutex mu(&m_muFormatsMap);
		mapIT = m_mapSupportedFormats.find(key);
		// First access to the key, need to get formats from devices
		if ( m_mapSupportedFormats.end() == mapIT )
		{
			if ( 0 == CalculateSupportedImageFormats(clMemFlags, clObjType) )
			{
				return CL_IMAGE_FORMAT_NOT_SUPPORTED;
			}
			mapIT = m_mapSupportedFormats.find(key);
		}
	}

	// Now look for the format in relevant list
	tImageFormatList::iterator listIT = mapIT->second.begin();
	while (mapIT->second.end() != listIT)
	{
		if ( (pclImageFormat->image_channel_order == listIT->image_channel_order) &&
			(pclImageFormat->image_channel_data_type == listIT->image_channel_data_type) )
		{
			return CL_SUCCESS;
		}
		listIT++;
	}

	return CL_IMAGE_FORMAT_NOT_SUPPORTED;
}


/**
 * Calculate the supported file formats for context.
 * UNION of all device capabilities (see clGetSupportedImageFormats).
 * @param clMemFlags
 * @param clObjType
 * @return size of supported image formats list.
 */
size_t Context::CalculateSupportedImageFormats( const cl_mem_flags clMemFlags, cl_mem_object_type clObjType )
{
	// Calculate supported format key
	int key = getFormatsKey(clObjType, clMemFlags);

	OclAutoMutex mu(&m_muFormatsMap);

	tImageFormatMap::iterator mapIT = m_mapSupportedFormats.find(key);

	// Found the supported formats list, no need to calculate it.
	if ( m_mapSupportedFormats.end() != mapIT )
	{
		return mapIT->second.size();
	}

	cl_err_code          clErr = CL_SUCCESS;
	cl_uint              maxFormatCount = 0;
	cl_image_format*     pFormats = NULL;
	tImageFormatList     imageFormatsList;
	const tSetOfDevices *rDevSet = GetAllRootDevices();
	bool                 exitWithErr = false;

	// Go through the devices and accumulate formats (union)
	tSetOfDevices::const_iterator rDev;
	for ( rDev = rDevSet->begin() ;
			rDev != rDevSet->end() ; ++rDev)
	{
		cl_uint devSpecificFormatsCount(0);

		// find number of formats to expect.
		clErr = (*rDev)->GetDeviceAgent()->clDevGetSupportedImageFormats(clMemFlags, clObjType,
				0, NULL, &devSpecificFormatsCount);
		if (CL_FAILED(clErr))
		{
			exitWithErr = true;
			break;
		}

		if (maxFormatCount < devSpecificFormatsCount)
		{
			if (pFormats) delete[] pFormats;
			maxFormatCount = devSpecificFormatsCount;
			pFormats = new cl_image_format[maxFormatCount];
		}

		// get formats
		clErr = (*rDev)->GetDeviceAgent()->clDevGetSupportedImageFormats(clMemFlags, clObjType,
				devSpecificFormatsCount, pFormats, NULL);
		if (CL_FAILED(clErr))
		{
			exitWithErr = true;
			break;
		}

		std::sort(&pFormats[0], &pFormats[devSpecificFormatsCount], compareImageFormats);

		if (rDev != rDevSet->begin())
		{
			// not first device
			tImageFormatList tempFormatsList;
			std::set_union(&pFormats[0], &pFormats[devSpecificFormatsCount],
					imageFormatsList.begin(), imageFormatsList.end(),
					tempFormatsList.begin(), compareImageFormats);

			imageFormatsList = tempFormatsList;
		} else {
			// only for first device, add all formats
			for (unsigned int ui=0; ui<devSpecificFormatsCount; ++ui)
			{
				imageFormatsList.push_back(pFormats[ui]);
			}
		}
	}

	if (!exitWithErr)
	{
		if (rDev == rDevSet->begin() && rDev == rDevSet->end())
		{
			assert(0 && "CalculateSupportedImageFormats: No root devices for context.");
			exitWithErr = true;
		}
	}

	if (pFormats) delete[] pFormats;

	if (exitWithErr)
	{
		imageFormatsList.clear();
	}
	m_mapSupportedFormats[key] = imageFormatsList;
	return imageFormatsList.size();
}

/**
 * Comparison function for image formats.
 * Sorting order:  image_channel_data_type, image_channel_order.
 * @param f1
 * @param f2
 * @return true if f1 is smaller than f2, false otherwise.
 */
bool compareImageFormats(cl_image_format f1, cl_image_format f2)
{
	if (f1.image_channel_data_type < f2.image_channel_data_type) return true;
	if (f1.image_channel_data_type > f2.image_channel_data_type) return false;

	// here only if image_channel_data_type is equal.
	if (f1.image_channel_order < f2.image_channel_order) return true;

	// in case of equality, or image_channel_order not smaller:
	return false;
}

static int getFormatsKey(int clObjType , int clMemFlags)
{
	int key = clObjType << 16 | clMemFlags;
	return key;
}

