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
	// initialize logger client
	INIT_LOGGER_CLIENT(L"Device", LL_DEBUG);
	m_mapDeviceLoggerClinets[0] = GET_LOGGER_CLIENT;
	m_pFECompiler = NULL;
	
	LOG_DEBUG(L"Device constructor enter");

	m_handle.dispatch = NULL;
	m_handle.object   = this;
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
	
	m_handle.dispatch = pOclEntryPoints;

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

	LOG_DEBUG(L"Call Device::fn_clDevCreateDeviceInstance");
	int clDevErr = devCreateInstance(m_iId, this, this, &m_pDevice);
	if (clDevErr != (int)CL_DEV_SUCCESS)
	{
		LOG_ERROR(L"Device::devCreateInstance returned %d", clDevErr);
		return CL_ERR_DEVICE_INIT_FAIL;
	}
	LOG_DEBUG(L"Device::fn_clDevCreateDeviceInstance exit. (CL_SUCCESS)");    
    return CL_SUCCESS;
}

cl_err_code Device::CloseDeviceInstance()
{
    LOG_DEBUG(L"CloseDeviceInstance enter");

    m_pDevice->clDevCloseDevice();

    return CL_SUCCESS;
}
cl_int Device::clLogCreateClient(cl_int device_id, wchar_t* client_name, cl_int * client_id)
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

cl_int Device::clLogReleaseClient(cl_int client_id)
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

cl_int Device::clLogAddLine(cl_int client_id, cl_int log_level, 
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

void Device::clDevBuildStatusUpdate(cl_dev_program clDevProg, void * pData, cl_build_status clBuildStatus)
{
	IBuildDoneObserver * pBuildDoneObserver = (IBuildDoneObserver*)pData;

	assert(pBuildDoneObserver);
	pBuildDoneObserver->NotifyBuildDone((cl_device_id)m_iId, clBuildStatus);
	return;
}

void Device::clDevCmdStatusChanged(cl_dev_cmd_id cmd_id, void * pData, cl_int cmd_status, cl_int status_result, cl_ulong timer)
{
	assert(pData);
	ICmdStatusChangedObserver *pObserver = (ICmdStatusChangedObserver *)pData;

	pObserver->NotifyCmdStatusChanged(cmd_id, cmd_status, status_result, timer);
	return;
}
