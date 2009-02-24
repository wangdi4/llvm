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
#include "windows.h"
#include "observer.h"
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

cl_int						Device::m_iNextClientId = 1;
map<cl_int, LoggerClient*>	Device::m_mapDeviceLoggerClinets;

Device::Device()
{
	// initialize logger client
	m_pLoggerClient = new LoggerClient(L"Device",LL_DEBUG);
	m_mapDeviceLoggerClinets[0] = m_pLoggerClient;
	
	InfoLog(m_pLoggerClient, L"Device contsructor enter");
	m_mapBuildDoneObservers.clear();
	m_mapCmdStatuschangedObservers.clear();
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
		it++;
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

	// initialize cl_dev_call_backs
	InfoLog(m_pLoggerClient, L"Initialize cl_dev_call_backs");
	m_clDevCallBacks.pclDevBuildStatusUpdate = Device::BuildStatusUpdate;
	m_clDevCallBacks.pclDevCmdStatusChanged = Device::CmdStatusChanged;

	// initialize cl_dev_log_descriptor
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

cl_err_code Device::CheckProgramBinary(size_t szBinSize, const void* pBinData)
{
	InfoLog(m_pLoggerClient, L"CheckProgramBinary enter. szBinSize=%d, pBinData=%d", szBinSize, pBinData);
	cl_int iRes = m_clDevEntryPoints.pclDevCheckProgramBinary(szBinSize, pBinData);
	return (cl_err_code)iRes;
}
cl_err_code Device::CreateProgram(size_t szBinSize, const void* pBinData, cl_dev_binary_prop clBinProp, cl_dev_program * pclProg)
{
	InfoLog(m_pLoggerClient, L"CreateProgram enter. szBinSize=%d, pBinData=%d, clBinProp=%d, pclProg=%d", szBinSize, pBinData, clBinProp, pclProg);
	cl_int iRes = m_clDevEntryPoints.pclDevCreateProgram(szBinSize, pBinData, clBinProp, pclProg);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}

cl_err_code Device::BuildProgram(cl_dev_program clProg, const cl_char * pcOptions, IBuildDoneObserver *	pBuildDoneObserver)
{
	InfoLog(m_pLoggerClient, L"BuildProgram enter. clProg=%d, pcOptions=%d, pBuildDoneObserver=%d", clProg, pcOptions, pBuildDoneObserver);

	// check if the program exits
	map<cl_dev_program, IBuildDoneObserver*>::iterator it = m_mapBuildDoneObservers.find(clProg);
	if (it == m_mapBuildDoneObservers.end())
	{
		// register program notification function
		m_mapBuildDoneObservers[clProg] = pBuildDoneObserver;
	}

	cl_int iRes = m_clDevEntryPoints.pclDevBuildProgram(clProg, pcOptions, this);
	return (cl_err_code)(iRes);
}
cl_err_code Device::GetProgramBinary(cl_dev_program clDevProg, 
									 size_t szBinSize, 
									 void * pBin,
									 size_t * pszBinSizeRet )
{
	InfoLog(m_pLoggerClient, L"GetProgramBinary enter. clDevProg=%d, szBinSize=%d, pBin=%d, pszBinSizeRet=%d", clDevProg, szBinSize, pBin, pszBinSizeRet);
	cl_int iRes = m_clDevEntryPoints.pclDevGetProgramBinary(clDevProg, szBinSize, pBin, pszBinSizeRet);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}

cl_int Device::CreateDeviceLogClient(cl_int device_id, wchar_t* client_name, cl_int * client_id)
{
	InfoLog(m_mapDeviceLoggerClinets[0], L"Device::CreateDeviceLogClient enter. device_id=%d, client_name=%ws", device_id, client_name);
	if (NULL == client_id)
	{
		ErrLog(m_mapDeviceLoggerClinets[0], L"client_id == NULL");
		return CL_INVALID_VALUE;
	}
	InfoLog(m_mapDeviceLoggerClinets[0], L"Create new logger client: (LoggerClient *pLoggerClient = new LoggerClient(client_name,LL_DEBUG))");	
	LoggerClient *pLoggerClient = new LoggerClient(client_name,LL_DEBUG);
	if (NULL == pLoggerClient)
	{
		ErrLog(m_mapDeviceLoggerClinets[0], L"NULL == pLoggerClient");
		return CL_ERR_LOGGER_FAILED;
	}
	*client_id = m_iNextClientId++;
	m_mapDeviceLoggerClinets[*client_id] = pLoggerClient;
	InfoLog(m_mapDeviceLoggerClinets[0], L"Device::CreateDeviceLogClient exit. (CL_SUCCESS)");	
	return CL_SUCCESS;
}

cl_int Device::ReleaseDeviceLogClient(cl_int client_id)
{
	InfoLog(m_mapDeviceLoggerClinets[0], L"Device::ReleaseDeviceLogClient enter. client_id=%d", client_id);
	map<cl_int,LoggerClient*>::iterator it =  m_mapDeviceLoggerClinets.find(client_id);
	if (it == m_mapDeviceLoggerClinets.end())
	{
		ErrLog(m_mapDeviceLoggerClinets[0], L"CL_ERR_KEY_NOT_FOUND: client id (%d) doesn't exists in device's logger clients pool", client_id);
		return CL_ERR_KEY_NOT_FOUND;
	}
	LoggerClient *pLoggerClient = it->second;
	InfoLog(m_mapDeviceLoggerClinets[0], L"Delete logger client (delete pLoggerClient; m_mapDeviceLoggerClinets.erase(it);)");
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
void Device::CmdStatusChanged(cl_dev_cmd_id cmd_id, void * pData, cl_int cmd_status, cl_int status_result)
{
	Device * pDevice = (Device*)pData;

	map<cl_dev_cmd_id, ICmdStatusChangedObserver *>::iterator it = pDevice->m_mapCmdStatuschangedObservers.find(cmd_id);
	if (it != pDevice->m_mapCmdStatuschangedObservers.end())
	{
		ICmdStatusChangedObserver * pCmdstatusChangedObserver = (ICmdStatusChangedObserver*)it->second;
		if (NULL != pCmdstatusChangedObserver)
		{
			pCmdstatusChangedObserver->NotifyCmdStatusChanged(cmd_id, cmd_status, status_result);
		}
	}
	return;
}
cl_err_code Device::GetKernelId(cl_dev_program	clDevProg,
								const char *	psKernelName,
								cl_dev_kernel *	pclKernel)
{
	InfoLog(m_pLoggerClient, L"GetKernelId enter. clDevProg=%d, psKernelName=%s, pclKernel=%d", clDevProg, psKernelName, pclKernel);
	cl_int iRes = m_clDevEntryPoints.pclDevGetKernelId(clDevProg, psKernelName, pclKernel);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}

cl_err_code Device::GetProgramKernels(cl_dev_program clDevProg,
									  cl_uint uiNumKernels,
									  cl_dev_kernel * pclKernels,
									  cl_uint * puiNumKernelsRet)
{
	InfoLog(m_pLoggerClient, L"GetProgramKernels enter. clDevProg=%d, uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d", 
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
	InfoLog(m_pLoggerClient, L"GetKernelInfo enter. clKernel=%d, clParam=%d, szValueSize=%d, pValue=%d, pValueSizeRet=%d", 
		clKernel, clParam, szValueSize, pValue, pValueSizeRet);
	cl_int iRes = m_clDevEntryPoints.pclDevGetKernelInfo(clKernel, clParam, szValueSize, pValue, pValueSizeRet);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}

cl_err_code Device::CreateMemoryObject(	cl_dev_mem_flags		clFlags,
										const cl_image_format*	pclFormat,
										cl_uint					uiDimCount,
										const size_t *			pszDim,
										void *					pBufferPtr,
										const size_t *			pszPitch,
										cl_dev_mem *			pMemObj)
{
	InfoLog(m_pLoggerClient, L"Enter CreateMemoryObject (clFlags=%d, pclFormat=%d, uiDimCount=%d, pszDim=%d, pBufferPtr=%d, pszPitch=%d, pMemObj=%d)", 
		clFlags, pclFormat, uiDimCount, pszDim, pMemObj);
	cl_int iRes = m_clDevEntryPoints.pclDevCreateMemoryObject(clFlags, pclFormat, uiDimCount, pszDim, pBufferPtr, pszPitch, pMemObj);
	if (0 != iRes)
	{
		return (cl_err_code)iRes;
	}
	return CL_SUCCESS;
}
cl_err_code Device::DeleteMemoryObject(cl_dev_mem clMemObj)
{
	InfoLog(m_pLoggerClient, L"Enter DeleteMemoryObject (clMemObj=%d)", clMemObj);
	
	cl_int iRes = m_clDevEntryPoints.pclDevDeleteMemoryObject(clMemObj);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::GetBuildLog(cl_dev_program	clDevProg,
								size_t			szSize,
								char*			psLog,
								size_t*			pszSizeRet)
{
	InfoLog(m_pLoggerClient, L"Enter GetBuildLog (clDevProg=%d, szSize-%d, psLog=%d, pszSizeRet=%d)", 
		clDevProg, szSize, psLog, pszSizeRet);
	
	cl_int iRes = m_clDevEntryPoints.pclDevGetBuildLog(clDevProg, szSize, psLog, pszSizeRet);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::CreateCommandList(cl_dev_cmd_list_props clDevCmdListProps, cl_dev_cmd_list * pclDevCmdList)
{
	InfoLog(m_pLoggerClient, L"CreateCommandList GetBuildLog (cl_dev_cmd_list_props=%d, pclDevCmdList-%d)", 
		clDevCmdListProps, pclDevCmdList);
	
	cl_int iRes = m_clDevEntryPoints.pclDevCreateCommandList(clDevCmdListProps, pclDevCmdList);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::RetainCommandList(cl_dev_cmd_list clDevCmdList)
{
	InfoLog(m_pLoggerClient, L"RetainCommandList GetBuildLog (clDevCmdList=%d)", clDevCmdList);
	
	cl_int iRes = m_clDevEntryPoints.pclDevRetainCommandList(clDevCmdList);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::ReleaseCommandList(cl_dev_cmd_list clDevCmdList)
{
	InfoLog(m_pLoggerClient, L"ReleaseCommandList GetBuildLog (clDevCmdList=%d)", clDevCmdList);
	
	cl_int iRes = m_clDevEntryPoints.pclDevReleaseCommandList(clDevCmdList);
	
	return (cl_err_code)iRes;
}
cl_err_code Device::CommandListExecute(cl_dev_cmd_list clDevCmdList,
									   cl_dev_cmd_desc * clDevCmdDesc,
									   cl_uint uiCount,
									   ICmdStatusChangedObserver ** ppCmdStatusChangedObserver)
{
	InfoLog(m_pLoggerClient, L"CommandListExecute GetBuildLog (clDevCmdList=%d, clDevCmdDesc-%d, uiCount=%d, ppCmdStatusChangedObserver=%d)", 
		clDevCmdList, clDevCmdDesc, uiCount, ppCmdStatusChangedObserver);

	// set observers for each command id
	for (cl_uint ui=0; ui<uiCount; ++ui)
	{
		cl_dev_cmd_id clDevCmdId = clDevCmdDesc[ui].id;
		clDevCmdDesc[ui].data = this;
		m_mapCmdStatuschangedObservers[clDevCmdId] = ppCmdStatusChangedObserver[ui];
	}
	
	cl_int iRes = m_clDevEntryPoints.pclDevCommandListExecute(clDevCmdList, clDevCmdDesc, uiCount);
	
	return (cl_err_code)iRes;
}