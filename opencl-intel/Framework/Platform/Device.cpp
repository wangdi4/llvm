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
using namespace std;
using namespace Intel::OpenCL::Framework;

cl_int						Device::m_iNextClientId = 0;
map<cl_int, LoggerClient*>	Device::m_mapDeviceLoggerClinets;

Device::Device()
{
	// initialize logger client
	m_pLoggerClient = new LoggerClient(L"Device",LL_DEBUG);
	m_mapDeviceLoggerClinets[0] = m_pLoggerClient;
	
	InfoLog(m_pLoggerClient, L"Device contsructor enter");
	::OCLObject();
}

Device::~Device()
{
	InfoLog(m_pLoggerClient, L"Device destructor enter");

	// release logger clients
	map<cl_int,LoggerClient*>::iterator it = m_mapDeviceLoggerClinets.begin();
	while (it != m_mapDeviceLoggerClinets.end())
	{
		LoggerClient * pLoggerClient = it->second;
		delete pLoggerClient;
	}
	m_mapDeviceLoggerClinets.clear();
}

cl_err_code	Device::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret)
{
	InfoLog(m_pLoggerClient, L"Device::GetInfo enter. param_name=%d, param_value_size=%d", param_name, param_value_size);
	int clDevErr = m_clDevEntryPoints.pclDevGetDeviceInfo(param_name, param_value_size, param_value, param_value_size_ret);
	if (clDevErr != (int)CL_DEV_SUCCESS)
	{
		return CL_INVALID_VALUE;
	}
	return CL_SUCCESS;
}

cl_err_code Device::InitDevice(const wchar_t * pwcDllPath)
{
	InfoLog(m_pLoggerClient, L"Device::InitDevice enter. pwcDllPath=%ws", pwcDllPath);
	
	InfoLog(m_pLoggerClient, L"LoadLibrary(%ws)", pwcDllPath);
	HINSTANCE devHndl;
	devHndl = LoadLibrary(pwcDllPath);
	if (NULL == devHndl)
	{
		ErrLog(m_pLoggerClient, L"LoadLibrary(%ws) failed", pwcDllPath);
		return CL_ERR_DEVICE_INIT_FAIL;
	}
	
	InfoLog(m_pLoggerClient, L"GetProcAddress(clDevInitDevice)");
	fn_clDevInitDevice *cpuDevInitDevice;
	cpuDevInitDevice = (fn_clDevInitDevice*)GetProcAddress(devHndl,"clDevInitDevice");
	if (NULL == cpuDevInitDevice)
	{
		ErrLog(m_pLoggerClient, L"GetProcAddress(clDevInitDevice) failed (fn_clDevInitDevice==NULL)");
		return CL_ERR_DEVICE_INIT_FAIL;
	}

	InfoLog(m_pLoggerClient, L"Initialize cl_dev_log_descriptor");
	m_clDevLogDescriptor.pfnclLogCreateClient = Device::CreateDeviceLogClient;
	m_clDevLogDescriptor.pfnclLogReleaseClient = Device::ReleaseDeviceLogClient;
	m_clDevLogDescriptor.pfnclLogAddLine = Device::DeviceAddLogLine;

	InfoLog(m_pLoggerClient, L"Call CPUDevice::fn_clDevInitDevice");
	int clDevErr = cpuDevInitDevice(m_iId, &m_clDevEntryPoints, &m_clDevCallBacks, &m_clDevLogDescriptor);
	if (clDevErr != (int)CL_DEV_SUCCESS)
	{
		ErrLog(m_pLoggerClient, L"CPUDevice::fn_clDevInitDevice returned %d", clDevErr);
		return CL_ERR_DEVICE_INIT_FAIL;
	}
	InfoLog(m_pLoggerClient, L"Device::InitDevice exit. (CL_SUCCESS)");
	return CL_SUCCESS;
}

cl_int Device::CreateDeviceLogClient(cl_int device_id, wchar_t* client_name, cl_int * client_id)
{
	InfoLog(m_mapDeviceLoggerClinets[0],L"Device::CreateDeviceLogClient enter. device_id=%d, client_name=%ws", device_id, client_name);
	if (NULL == client_id)
	{
		ErrLog(m_mapDeviceLoggerClinets[0],L"client_id == NULL");
		return CL_INVALID_VALUE;
	}
	
	InfoLog(m_mapDeviceLoggerClinets[0],L"Create new logger client: (LoggerClient *pLoggerClient = new LoggerClient(client_name,LL_DEBUG))");	
	LoggerClient *pLoggerClient = new LoggerClient(client_name,LL_DEBUG);
	if (NULL == pLoggerClient)
	{
		ErrLog(m_mapDeviceLoggerClinets[0],L"NULL == pLoggerClient");
		return CL_ERR_LOGGER_FAILED;
	}
	m_mapDeviceLoggerClinets[*client_id] = pLoggerClient;
	*client_id = m_iNextClientId++;
	InfoLog(m_mapDeviceLoggerClinets[0],L"Device::CreateDeviceLogClient exit. (CL_SUCCESS)");	
	return CL_SUCCESS;
}

cl_int Device::ReleaseDeviceLogClient(cl_int client_id)
{
	InfoLog(m_mapDeviceLoggerClinets[0],L"Device::ReleaseDeviceLogClient enter. client_id=%d", client_id);
	map<cl_int,LoggerClient*>::iterator it =  m_mapDeviceLoggerClinets.find(client_id);
	if (it == m_mapDeviceLoggerClinets.end())
	{
		ErrLog(m_mapDeviceLoggerClinets[0],L"CL_ERR_KEY_NOT_FOUND: client id (%d) doesn't exists in device's logger clients pool", client_id);
		return CL_ERR_KEY_NOT_FOUND;
	}
	LoggerClient *pLoggerClient = it->second;
	InfoLog(m_mapDeviceLoggerClinets[0],L"Delete logger client (delete pLoggerClient; m_mapDeviceLoggerClinets.erase(it);)");
	delete pLoggerClient;
	m_mapDeviceLoggerClinets.erase(it);

	InfoLog(m_mapDeviceLoggerClinets[0], L"Device::ReleaseDeviceLogClient exit. (CL_SUCCESS)");
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
		
		pLoggerClient->LogArgList((ELogLevel)log_level, (wchar_t*)source_file, (wchar_t*)function_name, line_num, (wchar_t*)message, va);

		va_end(va);

	}
	return CL_SUCCESS;
}