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
//  Device.cpp
//  Implementation of the Class Device
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "device.h"
#include "observer.h"
#include <assert.h>
#include <stdarg.h>

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

cl_int						Device::m_iNextClientId = 1;
map<cl_int, LoggerClient*>	Device::m_mapDeviceLoggerClinets;

Device::Device()
{
	::OCLObject();
	// initialize logger client
	INIT_LOGGER_CLIENT(L"Device", LL_DEBUG);
	m_mapDeviceLoggerClinets[0] = GET_LOGGER_CLIENT;
	m_pFECompiler = NULL;
    m_bIsDeviceOpened = false;
	
	LOG_DEBUG(L"Device constructor enter");

	m_mapBuildDoneObservers.clear();

	m_pHandle = new _cl_device_id;
	m_pHandle->object = this;
}

Device::~Device()
{
	LOG_DEBUG(L"Device destructor enter");

	// release logger clients
	map<cl_int,LoggerClient*>::iterator it = m_mapDeviceLoggerClinets.begin();
	while (it != m_mapDeviceLoggerClinets.end())
	{
		LoggerClient * pLoggerClient = it->second;
		delete pLoggerClient;
		it++;
	}
	m_mapDeviceLoggerClinets.clear();
	m_dlModule.Close();

	if (NULL != m_pHandle)
	{
		delete m_pHandle;
	}
}

cl_err_code	Device::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret)
{
	LOG_DEBUG(L"Enter Device::GetInfo (param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", 
		param_name, param_value_size, param_value, param_value_size_ret);

	int clDevErr = CL_SUCCESS;
	cl_err_code clErrRet = CL_SUCCESS;
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;
	cl_bool bValue = false;
	void * pValue = NULL;

	switch (param_name)
	{
	case CL_DEVICE_COMPILER_AVAILABLE:
		szParamValueSize = sizeof(cl_bool);
		bValue = (NULL == m_pFECompiler) ? false : true;
		pValue = &bValue;
		break;
	
	default:
		clDevErr = m_pFnClDevGetDeviceInfo(param_name, param_value_size, param_value, param_value_size_ret);
		if (clDevErr != (int)CL_DEV_SUCCESS)
		{
			return CL_INVALID_VALUE;
		}
		return CL_SUCCESS;
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

cl_err_code Device::InitDevice(const char * psDeviceAgentDllPath, ocl_entry_points * pOclEntryPoints)
{
	LogDebugA("Device::InitDevice enter. pwcDllPath=%s", psDeviceAgentDllPath);
	
	m_pHandle->dispatch = pOclEntryPoints;

	LogDebugA("LoadLibrary(%s)", psDeviceAgentDllPath);
	if (!m_dlModule.Load(psDeviceAgentDllPath))
	{
		LogErrorA("LoadLibrary(%s) failed", psDeviceAgentDllPath);
		return CL_ERR_DEVICE_INIT_FAIL;
	}

    // Get pointer to the GetInfo function
	LOG_DEBUG(L"GetProcAddress(clDevGetDeviceInfo)");
	m_pFnClDevGetDeviceInfo = (fn_clDevGetDeviceInfo*)m_dlModule.GetFunctionPtrByName("clDevGetDeviceInfo");
	if (NULL == m_pFnClDevGetDeviceInfo)
	{
		LOG_ERROR(L"GetProcAddress(clDevGetDeviceInfo) failed (m_pFnClDevGetDeviceInfo==NULL)");
		return CL_ERR_DEVICE_INIT_FAIL;
	}

	return CL_SUCCESS;
}

cl_err_code Device::CreateInstance()
{
	LOG_DEBUG(L"GetProcAddress(clDevCreateDeviceInstance)");
	fn_clDevCreateDeviceInstance *devCreateInstance;
	devCreateInstance = (fn_clDevCreateDeviceInstance*)m_dlModule.GetFunctionPtrByName("clDevCreateDeviceInstance");
	if (NULL == devCreateInstance)
	{
		LOG_ERROR(L"GetProcAddress(clDevCreateDeviceInstance) failed (devCreateInstance==NULL)");
		return CL_ERR_DEVICE_INIT_FAIL;
	}

	// initialize cl_dev_call_backs
	LOG_DEBUG(L"Initialize cl_dev_call_backs");
	m_clDevCallBacks.pclDevBuildStatusUpdate = Device::BuildStatusUpdate;
	m_clDevCallBacks.pclDevCmdStatusChanged = Device::CmdStatusChanged;

	// initialize cl_dev_log_descriptor
	LOG_DEBUG(L"Initialize cl_dev_log_descriptor");
	m_clDevLogDescriptor.pfnclLogCreateClient = Device::CreateDeviceLogClient;
	m_clDevLogDescriptor.pfnclLogReleaseClient = Device::ReleaseDeviceLogClient;
	m_clDevLogDescriptor.pfnclLogAddLine = Device::DeviceAddLogLine;

	LOG_DEBUG(L"Call Device::fn_clDevCreateDeviceInstance");
	int clDevErr = devCreateInstance(m_iId, &m_clDevEntryPoints, &m_clDevCallBacks, &m_clDevLogDescriptor);
	if (clDevErr != (int)CL_DEV_SUCCESS)
	{
		LOG_ERROR(L"Device::devCreateInstance returned %d", clDevErr);
		return CL_ERR_DEVICE_INIT_FAIL;
	}
    m_bIsDeviceOpened = true;
	LOG_DEBUG(L"Device::fn_clDevCreateDeviceInstance exit. (CL_SUCCESS)");    
    return CL_SUCCESS;
}

cl_err_code Device::CloseDeviceInstance()
{
    LOG_DEBUG(L"CloseDeviceInstance enter");

    // Close device
    { OclAutoMutex CS(&m_muDeviceCloseLock);
    m_bIsDeviceOpened = false;
    } // end CS
    m_clDevEntryPoints.pclDevCloseDevice();

    // Clear objects
	m_mapBuildDoneObservers.clear();
    return CL_SUCCESS;
}



cl_err_code Device::CheckProgramBinary(size_t szBinSize, const void* pBinData)
{
	LOG_DEBUG(L"CheckProgramBinary enter. szBinSize=%d, pBinData=%d", szBinSize, pBinData);
	cl_int iRes = m_clDevEntryPoints.pclDevCheckProgramBinary(szBinSize, pBinData);
	return (cl_err_code)iRes;
}
cl_err_code Device::CreateProgram(size_t szBinSize, const void* pBinData, cl_dev_binary_prop clBinProp, cl_dev_program * pclProg)
{
	LOG_DEBUG(L"CreateProgram enter. szBinSize=%d, pBinData=%d, clBinProp=%d, pclProg=%d", szBinSize, pBinData, clBinProp, pclProg);
	cl_int iRes = m_clDevEntryPoints.pclDevCreateProgram(szBinSize, pBinData, clBinProp, pclProg);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}

cl_err_code Device::BuildProgram(cl_dev_program clProg, const char * pcOptions, IBuildDoneObserver *	pBuildDoneObserver)
{
	LOG_DEBUG(L"BuildProgram enter. clProg=%d, pcOptions=%d, pBuildDoneObserver=%d", clProg, pcOptions, pBuildDoneObserver);

	// check if the program exits
	map<cl_dev_program, IBuildDoneObserver*>::iterator it = m_mapBuildDoneObservers.find(clProg);
	if (it == m_mapBuildDoneObservers.end())
	{
		// register program notification function
		m_mapBuildDoneObservers[clProg] = pBuildDoneObserver;
	}

	cl_int iRes = m_clDevEntryPoints.pclDevBuildProgram(clProg, (const cl_char *)pcOptions, this);
	return (cl_err_code)(iRes);
}
cl_err_code Device::GetProgramBinary(cl_dev_program clDevProg, 
									 size_t szBinSize, 
									 void * pBin,
									 size_t * pszBinSizeRet )
{
	LOG_DEBUG(L"GetProgramBinary enter. clDevProg=%d, szBinSize=%d, pBin=%d, pszBinSizeRet=%d", clDevProg, szBinSize, pBin, pszBinSizeRet);
	cl_int iRes = m_clDevEntryPoints.pclDevGetProgramBinary(clDevProg, szBinSize, pBin, pszBinSizeRet);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}

cl_int Device::CreateDeviceLogClient(cl_int device_id, wchar_t* client_name, cl_int * client_id)
{
	if (NULL == client_id)
	{
		return CL_INVALID_VALUE;
	}

	if (!Logger::GetInstance().IsActive())
	{
		*client_id = 0;
		return CL_SUCCESS;
	}
	
	LoggerClient *pLoggerClient = new LoggerClient(client_name,LL_DEBUG);
	if (NULL == pLoggerClient)
	{
		return CL_ERR_LOGGER_FAILED;
	}
	*client_id = m_iNextClientId++;
	m_mapDeviceLoggerClinets[*client_id] = pLoggerClient;
	return CL_SUCCESS;
}

cl_int Device::ReleaseDeviceLogClient(cl_int client_id)
{
	map<cl_int,LoggerClient*>::iterator it =  m_mapDeviceLoggerClinets.find(client_id);
	if (it == m_mapDeviceLoggerClinets.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	LoggerClient *pLoggerClient = it->second;
	delete pLoggerClient;
	m_mapDeviceLoggerClinets.erase(it);

	return CL_SUCCESS;
}

cl_int Device::DeviceAddLogLine(cl_int client_id, cl_int log_level, 
								const wchar_t* IN source_file, 
								const wchar_t* IN function_name, 
								cl_int line_num, 
								const wchar_t* IN message, ...)
{
	map<cl_int,LoggerClient*>::iterator it =  m_mapDeviceLoggerClinets.find(client_id);
	if (it == m_mapDeviceLoggerClinets.end())
	{
		return CL_ERR_KEY_NOT_FOUND;
	}
	LoggerClient *pLoggerClient = it->second;
	if (NULL != pLoggerClient)
	{
		va_list va;
		va_start(va, message);
		
		pLoggerClient->LogArgListW((ELogLevel)log_level, (wchar_t*)source_file, (wchar_t*)function_name, line_num, (wchar_t*)message, va);

		va_end(va);

	}
	return CL_SUCCESS;
}

void Device::BuildStatusUpdate(cl_dev_program clDevProg, void * pData, cl_build_status clBuildStatus)
{
	Device * pDevice = (Device*)pData;

	map<cl_dev_program, IBuildDoneObserver*>::iterator it = pDevice->m_mapBuildDoneObservers.find(clDevProg);
	if (it != pDevice->m_mapBuildDoneObservers.end())
	{
		IBuildDoneObserver * pBuildDoneObserver = (IBuildDoneObserver*)it->second;
		if (NULL != pBuildDoneObserver)
		{
			pBuildDoneObserver->NotifyBuildDone((cl_device_id)pDevice->m_iId, clBuildStatus);
		}
	}
	return;
}

void Device::CmdStatusChanged(cl_dev_cmd_id cmd_id, void * pData, cl_int cmd_status, cl_int status_result, cl_ulong timer)
{
	assert(pData);
	ICmdStatusChangedObserver *pObserver = (ICmdStatusChangedObserver *)pData;

	pObserver->NotifyCmdStatusChanged(cmd_id, cmd_status, status_result, timer);
	return;
}

cl_err_code Device::GetKernelId(cl_dev_program	clDevProg,
								const char *	psKernelName,
								cl_dev_kernel *	pclKernel)
{
	cl_int iRes = m_clDevEntryPoints.pclDevGetKernelId(clDevProg, psKernelName, pclKernel);
	return (cl_err_code)iRes;
}

cl_err_code Device::GetProgramKernels(cl_dev_program clDevProg,
									  cl_uint uiNumKernels,
									  cl_dev_kernel * pclKernels,
									  cl_uint * puiNumKernelsRet)
{
	LOG_DEBUG(L"GetProgramKernels enter. clDevProg=%d, uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d", 
		clDevProg, uiNumKernels, pclKernels, puiNumKernelsRet);
	cl_int iRes = m_clDevEntryPoints.pclDevGetProgramKernels(clDevProg, uiNumKernels, pclKernels, puiNumKernelsRet);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}
cl_err_code Device::GetKernelInfo(cl_dev_kernel clKernel,
								  cl_dev_kernel_info clParam,
								  size_t szValueSize,
								  void * pValue,
								  size_t * pValueSizeRet)
{
	LOG_DEBUG(L"GetKernelInfo enter. clKernel=%d, clParam=%d, szValueSize=%d, pValue=%d, pValueSizeRet=%d", 
		clKernel, clParam, szValueSize, pValue, pValueSizeRet);
	cl_int iRes = m_clDevEntryPoints.pclDevGetKernelInfo(clKernel, clParam, szValueSize, pValue, pValueSizeRet);
	return (cl_err_code)iRes;
}

cl_err_code Device::CreateMemoryObject(	cl_dev_mem_flags		clFlags,
										const cl_image_format*	pclFormat,
										cl_uint					uiDimCount,
										const size_t *			pszDim,
										void *					pBufferPtr,
										const size_t *			pszPitch,
										cl_dev_host_ptr_flags	hstFlags,
										cl_dev_mem *			pMemObj)
{
	LOG_DEBUG(L"Enter CreateMemoryObject (clFlags=%d, pclFormat=%d, uiDimCount=%d, pszDim=%d, pBufferPtr=%d, pszPitch=%d, pMemObj=%d)", 
		clFlags, pclFormat, uiDimCount, pszDim, pMemObj);
	cl_int iRes = m_clDevEntryPoints.pclDevCreateMemoryObject(clFlags, pclFormat, uiDimCount, pszDim, pBufferPtr, pszPitch, hstFlags, pMemObj);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}
cl_err_code Device::DeleteMemoryObject(cl_dev_mem clMemObj)
{
	LOG_DEBUG(L"Enter DeleteMemoryObject (clMemObj=%d)", clMemObj);
	
	cl_int iRes = m_clDevEntryPoints.pclDevDeleteMemoryObject(clMemObj);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::GetBuildLog(cl_dev_program	clDevProg,
								size_t			szSize,
								char*			psLog,
								size_t*			pszSizeRet)
{
	LOG_DEBUG(L"Enter GetBuildLog (clDevProg=%d, szSize-%d, psLog=%d, pszSizeRet=%d)", 
		clDevProg, szSize, psLog, pszSizeRet);
	
	cl_int iRes = m_clDevEntryPoints.pclDevGetBuildLog(clDevProg, szSize, psLog, pszSizeRet);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::CreateCommandList(cl_dev_cmd_list_props clDevCmdListProps, cl_dev_cmd_list * pclDevCmdList)
{
	LOG_DEBUG(L"CreateCommandList GetBuildLog (cl_dev_cmd_list_props=%d, pclDevCmdList-%d)", 
		clDevCmdListProps, pclDevCmdList);
    if(!m_bIsDeviceOpened)
    {
        return CL_SUCCESS;
    }	
	cl_int iRes = m_clDevEntryPoints.pclDevCreateCommandList(clDevCmdListProps, pclDevCmdList);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::RetainCommandList(cl_dev_cmd_list clDevCmdList)
{
	LOG_DEBUG(L"RetainCommandList GetBuildLog (clDevCmdList=%d)", clDevCmdList);

    if(!m_bIsDeviceOpened)
    {
        return CL_SUCCESS;
    }
	cl_int iRes = m_clDevEntryPoints.pclDevRetainCommandList(clDevCmdList);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::ReleaseCommandList(cl_dev_cmd_list clDevCmdList)
{
	LOG_DEBUG(L"ReleaseCommandList GetBuildLog (clDevCmdList=%d)", clDevCmdList);
    
    { OclAutoMutex CS(&m_muDeviceCloseLock);
    if(!m_bIsDeviceOpened)
    {
        return CL_SUCCESS;
    }
    cl_int iRes = m_clDevEntryPoints.pclDevReleaseCommandList(clDevCmdList);
	return (cl_err_code)iRes;
    }
}
cl_err_code Device::CommandListExecute(cl_dev_cmd_list clDevCmdList,
									   cl_dev_cmd_desc * clDevCmdDesc,
									   cl_uint uiCount,
									   ICmdStatusChangedObserver ** ppCmdStatusChangedObserver)
{
	LOG_INFO(L"Enter (clDevCmdList=%X, clDevCmdDesc-%d, uiCount=%d, ppCmdStatusChangedObserver=%d)", 
		clDevCmdList, clDevCmdDesc, uiCount, ppCmdStatusChangedObserver);

    // TODO: Move this code to upper level
	cl_dev_cmd_desc * *pDescList = new cl_dev_cmd_desc *[uiCount];

	// set observers for each command id
	for (cl_uint ui=0; ui<uiCount; ++ui)
	{
		pDescList[ui] = &clDevCmdDesc[ui];
		cl_dev_cmd_id clDevCmdId = clDevCmdDesc[ui].id;
		clDevCmdDesc[ui].data = ppCmdStatusChangedObserver[ui];
	}
	
	cl_int iRes = m_clDevEntryPoints.pclDevCommandListExecute(clDevCmdList, pDescList, uiCount);
	delete []pDescList;
	LOG_INFO(L"Exit - List:%X, Res=%d", clDevCmdList, iRes);
	return (cl_err_code)iRes;
}


cl_err_code Device::FlushCommandList(cl_dev_cmd_list clDevCmdList)
{
    LOG_INFO(L"Enter (clDevCmdList=%X)",  clDevCmdList);    
    if(!m_bIsDeviceOpened)
    {
        return CL_SUCCESS;
    }
    cl_int iRes = m_clDevEntryPoints.pclDevFlushCommandList(clDevCmdList);
    LOG_INFO(L"Exit (clDevCmdList=%X), Res=%d",  clDevCmdList, iRes);
    return (cl_err_code)iRes;
}


cl_err_code Device::CreateMappedRegion(cl_dev_cmd_param_map * pMapParams)
{
	LOG_DEBUG(L"Enter CreateCommandList (pMapParams=%X)", pMapParams);
	
	cl_int iRes = m_clDevEntryPoints.pclDevCreateMappedRegion(pMapParams);
	
	return (cl_err_code)iRes;
}

cl_err_code Device::ReleaseMappedRegion(cl_dev_cmd_param_map * pMapParams)
{
	LOG_DEBUG(L"Enter ReleaseMappedRegion (pMapParams=%X)", pMapParams);
	
	cl_int iRes = m_clDevEntryPoints.pclDevReleaseMappedRegion(pMapParams);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::UnloadCompiler(void)
{
	LOG_INFO(L"Enter UnloadCompiler");
	
	cl_int iRes = m_clDevEntryPoints.pclDevUnloadCompiler();
	
	return (cl_err_code)iRes;
}
cl_err_code Device::GetSupportedImageFormats(	cl_dev_mem_flags       clDevFlags,
												cl_dev_mem_object_type clDevImageType,
												cl_uint                uiNumEntries,
												cl_image_format *      pclFormats,
												cl_uint *              puiNumEntriesRet)
{
	LOG_DEBUG(L"Enter GetSupportedImageFormats (clDevFlags=%d, clDevImageType=%d, uiNumEntries=%d, pclFormats=%d, puiNumEntriesRet=%d",
		clDevFlags, clDevImageType, uiNumEntries, pclFormats, puiNumEntriesRet);
	
	cl_int iRes = m_clDevEntryPoints.pclDevGetSupportedImageFormats(clDevFlags, clDevImageType, uiNumEntries, pclFormats, puiNumEntriesRet);
	
	return (cl_err_code)iRes;
}