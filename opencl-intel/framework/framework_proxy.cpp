
///////////////////////////////////////////////////////////
//  FrameworkFactory.cpp
//  Implementation of the Class FrameworkFactory
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "framework_proxy.h"
#include "logger.h"
#include "cl_sys_info.h"
#include <task_executor.h>

#include <windows.h>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::TaskExecutor;

cl_monitor_init;

char clFRAMEWORK_CFG_PATH[MAX_PATH];

ocl_entry_points FrameworkProxy::OclEntryPoints = {
	clGetPlatformInfo,
	clGetDeviceIDs,
	clGetDeviceInfo,
	clCreateContext,
	clCreateContextFromType,
	clRetainContext,
	clReleaseContext,
	clGetContextInfo,
	clCreateCommandQueue,
	clRetainCommandQueue,
	clReleaseCommandQueue,
	clGetCommandQueueInfo,
	clSetCommandQueueProperty,
	clCreateBuffer,
	clCreateImage2D,
	clCreateImage3D,
	clRetainMemObject,
	clReleaseMemObject,
	clGetSupportedImageFormats,
	clGetMemObjectInfo,
	clGetImageInfo,
	clCreateSampler,
	clRetainSampler,
	clReleaseSampler,
	clGetSamplerInfo,
	clCreateProgramWithSource,
	clCreateProgramWithBinary,
	clRetainProgram,
	clReleaseProgram,
	clBuildProgram,
	clUnloadCompiler,
	clGetProgramInfo,
	clGetProgramBuildInfo,
	clCreateKernel,
	clCreateKernelsInProgram,
	clRetainKernel,
	clReleaseKernel,
	clSetKernelArg,
	clGetKernelInfo,
	clGetKernelWorkGroupInfo,
	clWaitForEvents,
	clGetEventInfo,
	clRetainEvent,
	clReleaseEvent,
	clGetEventProfilingInfo,
	clFlush,
	clFinish,
	clEnqueueReadBuffer,
	clEnqueueWriteBuffer,
	clEnqueueCopyBuffer,
	clEnqueueReadImage,
	clEnqueueWriteImage,
	clEnqueueCopyImage,
	clEnqueueCopyImageToBuffer,
	clEnqueueCopyBufferToImage,
	clEnqueueMapBuffer,
	clEnqueueMapImage,
	clEnqueueUnmapMemObject,
	clEnqueueNDRangeKernel,
	clEnqueueTask,
	clEnqueueNativeKernel,
	clEnqueueMarker,
	clEnqueueWaitForEvents,
	clEnqueueBarrier
};

FrameworkProxy * FrameworkProxy::m_pInstance = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy::FrameworkProxy()
{
	m_pPlatformModule = NULL;
	m_pContextModule = NULL;
	m_pExecutionModule = NULL;
	m_pFileLogHandler = NULL;
	m_pConfig = NULL;
	m_pLoggerClient = NULL;

	Initialize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// ~FrameworkProxy()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy::~FrameworkProxy()
{
	LOG_DEBUG(L"FrameworkProxy destructor enter");
	Release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Initialize()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Initialize()
{
	// initialize configuration file
	m_pConfig = new OCLConfig();
	m_pConfig->Initialize(clFRAMEWORK_CFG_PATH);

	bool bUseLogger = m_pConfig->UseLogger();
	if (bUseLogger)
	{
		string str = m_pConfig->GetLogFile();
		if (str != "")
		{
			// Construct file name with process ID
			// Search for file extension
			size_t ext = str.rfind(".");
			if ( -1 == ext )
			{
				// If "." not found -> no extension
				ext = str.length();
			}
			// Add Process if before the "."
			//Calculate Extension lenght
			size_t extLen = str.length() - ext + 16;
			std::string procId;
			procId.resize(extLen);
			sprintf_s(&procId[0], extLen, "_%d%s", GetProcessId(), &str[ext]);
			str.insert(ext, procId);

			//null-call to get the size
			size_t needed;
			::mbstowcs_s(&needed, NULL, 0, &str[0], str.length());
			// allocate
			std::wstring wstr;
			wstr.resize(needed);
			// real call
			::mbstowcs_s(&needed, &wstr[0], wstr.length(), &str[0], str.length());

			// Prepare log title
			wchar_t		strProcName[MAX_PATH];
			GetProcessName(strProcName, MAX_PATH);
			std::wstring title = L"---------------------------------> ";
			title += strProcName;
			title += L" <-----------------------------------\n";

			// initialise logger
			m_pFileLogHandler = new FileLogHandler(L"cl_framework");
			cl_err_code clErrRet = m_pFileLogHandler->Init(LL_DEBUG, wstr.c_str(), title.c_str());
			if (CL_SUCCEEDED(clErrRet))
			{
				Logger::GetInstance().AddLogHandler(m_pFileLogHandler);
			}
		}
	}
	Logger::GetInstance().SetActive(bUseLogger);

	INIT_LOGGER_CLIENT(L"FrameworkProxy", LL_DEBUG);
	
	LOG_INFO(L"Initialize platform module: m_PlatformModule = new PlatformModule()");
	m_pPlatformModule = new PlatformModule();
	m_pPlatformModule->Initialize(&OclEntryPoints, m_pConfig);

	LOG_INFO(L"Initialize context module: m_pContextModule = new ContextModule(%d)",m_pPlatformModule);
	m_pContextModule = new ContextModule(m_pPlatformModule);
	m_pContextModule->Initialize(&OclEntryPoints);

	LOG_INFO(L"Initialize context module: m_pExecutionModule = new ExecutionModule(%d,%d)", m_pPlatformModule, m_pContextModule);
	m_pExecutionModule = new ExecutionModule(m_pPlatformModule, m_pContextModule);
	m_pExecutionModule->Initialize(&OclEntryPoints);

	// Initialize TaskExecutor
	LOG_INFO(L"Initialize Executor");
	GetTaskExecutor()->Init(0);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Destroy()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Destroy()
{
	if (NULL != m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Release()
{
	LOG_DEBUG(L"FrameworkProxy::Release enter");
	
	m_pExecutionModule->Release();
    delete m_pExecutionModule;

    m_pContextModule->Release(true);
	delete m_pContextModule;
	
	m_pPlatformModule->Release();
	delete m_pPlatformModule;

	// Close TaskExecutor
	GetTaskExecutor()->Close(true);

	if (m_pFileLogHandler)
	{
		m_pFileLogHandler->Flush();
		delete m_pFileLogHandler;
		m_pFileLogHandler = NULL;
	}

	cl_monitor_summary;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Instance()
///////////////////////////////////////////////////////////////////////////////////////////////////
FrameworkProxy* FrameworkProxy::Instance()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new FrameworkProxy();
	}
	return m_pInstance;
}
