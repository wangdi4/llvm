
///////////////////////////////////////////////////////////
//  FrameworkFactory.cpp
//  Implementation of the Class FrameworkFactory
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "framework_proxy.h"
#include "Logger.h"
#include "cl_sys_info.h"
#include "cl_sys_defines.h"
#include <task_executor.h>

#if defined (_WIN32)
#include <windows.h>
#else
#include "cl_secure_string_linux.h"
#include "cl_framework_alias_linux.h"
#endif
#if defined(USE_GPA)
	// This code was removed for the initial porting of TAL
	// to GPA 4.0 and might be used in later stages
//	#include "tal\tal.h"
#endif
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::TaskExecutor;

cl_monitor_init

char clFRAMEWORK_CFG_PATH[MAX_PATH];

ocl_entry_points FrameworkProxy::OclEntryPoints = {
    (KHRpfn_clGetPlatformIDs)GET_ALIAS(clGetPlatformIDs),
    (KHRpfn_clGetPlatformInfo)GET_ALIAS(clGetPlatformInfo),
    (KHRpfn_clGetDeviceIDs)GET_ALIAS(clGetDeviceIDs),
    (KHRpfn_clGetDeviceInfo)GET_ALIAS(clGetDeviceInfo),
    (KHRpfn_clCreateContext)GET_ALIAS(clCreateContext),
    (KHRpfn_clCreateContextFromType)GET_ALIAS(clCreateContextFromType),
    (KHRpfn_clRetainContext)GET_ALIAS(clRetainContext),
    (KHRpfn_clReleaseContext)GET_ALIAS(clReleaseContext),
    (KHRpfn_clGetContextInfo)GET_ALIAS(clGetContextInfo),
    (KHRpfn_clCreateCommandQueue)GET_ALIAS(clCreateCommandQueue),
    (KHRpfn_clRetainCommandQueue)GET_ALIAS(clRetainCommandQueue),
    (KHRpfn_clReleaseCommandQueue)GET_ALIAS(clReleaseCommandQueue),
    (KHRpfn_clGetCommandQueueInfo)GET_ALIAS(clGetCommandQueueInfo),
    NULL/*(KHRpfn_clSetCommandQueueProperty)GET_ALIAS(clSetCommandQueueProperty)*/,
    (KHRpfn_clCreateBuffer)GET_ALIAS(clCreateBuffer),
    (KHRpfn_clCreateImage2D)GET_ALIAS(clCreateImage2D),
    (KHRpfn_clCreateImage3D)GET_ALIAS(clCreateImage3D),
    (KHRpfn_clRetainMemObject)GET_ALIAS(clRetainMemObject),
    (KHRpfn_clReleaseMemObject)GET_ALIAS(clReleaseMemObject),
    (KHRpfn_clGetSupportedImageFormats)GET_ALIAS(clGetSupportedImageFormats),
    (KHRpfn_clGetMemObjectInfo)GET_ALIAS(clGetMemObjectInfo),
    (KHRpfn_clGetImageInfo)GET_ALIAS(clGetImageInfo),
    (KHRpfn_clCreateSampler)GET_ALIAS(clCreateSampler),
    (KHRpfn_clRetainSampler)GET_ALIAS(clRetainSampler),
    (KHRpfn_clReleaseSampler)GET_ALIAS(clReleaseSampler),
    (KHRpfn_clGetSamplerInfo)GET_ALIAS(clGetSamplerInfo),
    (KHRpfn_clCreateProgramWithSource)GET_ALIAS(clCreateProgramWithSource),
    (KHRpfn_clCreateProgramWithBinary)GET_ALIAS(clCreateProgramWithBinary),
    (KHRpfn_clRetainProgram)GET_ALIAS(clRetainProgram),
    (KHRpfn_clReleaseProgram)GET_ALIAS(clReleaseProgram),
    (KHRpfn_clBuildProgram)GET_ALIAS(clBuildProgram),
    (KHRpfn_clUnloadCompiler)GET_ALIAS(clUnloadCompiler),
    (KHRpfn_clGetProgramInfo)GET_ALIAS(clGetProgramInfo),
    (KHRpfn_clGetProgramBuildInfo)GET_ALIAS(clGetProgramBuildInfo),
    (KHRpfn_clCreateKernel)GET_ALIAS(clCreateKernel),
    (KHRpfn_clCreateKernelsInProgram)GET_ALIAS(clCreateKernelsInProgram),
    (KHRpfn_clRetainKernel)GET_ALIAS(clRetainKernel),
    (KHRpfn_clReleaseKernel)GET_ALIAS(clReleaseKernel),
    (KHRpfn_clSetKernelArg)GET_ALIAS(clSetKernelArg),
    (KHRpfn_clGetKernelInfo)GET_ALIAS(clGetKernelInfo),
    (KHRpfn_clGetKernelWorkGroupInfo)GET_ALIAS(clGetKernelWorkGroupInfo),
    (KHRpfn_clWaitForEvents)GET_ALIAS(clWaitForEvents),
    (KHRpfn_clGetEventInfo)GET_ALIAS(clGetEventInfo),
    (KHRpfn_clRetainEvent)GET_ALIAS(clRetainEvent),
    (KHRpfn_clReleaseEvent)GET_ALIAS(clReleaseEvent),
    (KHRpfn_clGetEventProfilingInfo)GET_ALIAS(clGetEventProfilingInfo),
    (KHRpfn_clFlush)GET_ALIAS(clFlush),
    (KHRpfn_clFinish)GET_ALIAS(clFinish),
    (KHRpfn_clEnqueueReadBuffer)GET_ALIAS(clEnqueueReadBuffer),
    (KHRpfn_clEnqueueWriteBuffer)GET_ALIAS(clEnqueueWriteBuffer),
    (KHRpfn_clEnqueueCopyBuffer)GET_ALIAS(clEnqueueCopyBuffer),
    (KHRpfn_clEnqueueReadImage)GET_ALIAS(clEnqueueReadImage),
    (KHRpfn_clEnqueueWriteImage)GET_ALIAS(clEnqueueWriteImage),
    (KHRpfn_clEnqueueCopyImage)GET_ALIAS(clEnqueueCopyImage),
    (KHRpfn_clEnqueueCopyImageToBuffer)GET_ALIAS(clEnqueueCopyImageToBuffer),
    (KHRpfn_clEnqueueCopyBufferToImage)GET_ALIAS(clEnqueueCopyBufferToImage),
    (KHRpfn_clEnqueueMapBuffer)GET_ALIAS(clEnqueueMapBuffer),
    (KHRpfn_clEnqueueMapImage)GET_ALIAS(clEnqueueMapImage),
    (KHRpfn_clEnqueueUnmapMemObject)GET_ALIAS(clEnqueueUnmapMemObject),
    (KHRpfn_clEnqueueNDRangeKernel)GET_ALIAS(clEnqueueNDRangeKernel),
    (KHRpfn_clEnqueueTask)GET_ALIAS(clEnqueueTask),
    (KHRpfn_clEnqueueNativeKernel)GET_ALIAS(clEnqueueNativeKernel),
    (KHRpfn_clEnqueueMarker)GET_ALIAS(clEnqueueMarker),
    (KHRpfn_clEnqueueWaitForEvents)GET_ALIAS(clEnqueueWaitForEvents),
    (KHRpfn_clEnqueueBarrier)GET_ALIAS(clEnqueueBarrier),
    (KHRpfn_clGetExtensionFunctionAddress)GET_ALIAS(clGetExtensionFunctionAddress),
	(KHRpfn_clCreateFromGLBuffer)GET_ALIAS(clCreateFromGLBuffer),
	(KHRpfn_clCreateFromGLTexture2D)GET_ALIAS(clCreateFromGLTexture2D),
	(KHRpfn_clCreateFromGLTexture3D)GET_ALIAS(clCreateFromGLTexture3D),
	(KHRpfn_clCreateFromGLRenderbuffer)GET_ALIAS(clCreateFromGLRenderbuffer),
	(KHRpfn_clGetGLObjectInfo)GET_ALIAS(clGetGLObjectInfo),
	(KHRpfn_clGetGLTextureInfo)GET_ALIAS(clGetGLTextureInfo),
	(KHRpfn_clEnqueueAcquireGLObjects)GET_ALIAS(clEnqueueAcquireGLObjects),
	(KHRpfn_clEnqueueReleaseGLObjects)GET_ALIAS(clEnqueueReleaseGLObjects),
	(KHRpfn_clGetGLContextInfoKHR)GET_ALIAS(clGetGLContextInfoKHR),
	NULL, // clGetDeviceIDsFromD3D10KHR
	NULL, // clCreateFromD3D10BufferKHR
	NULL, // clCreateFromD3D10Texture2DKHR
	NULL, // clCreateFromD3D10Texture3DKHR
	NULL, // clEnqueueAcquireD3D10ObjectsKHR
	NULL, // clEnqueueReleaseD3D10ObjectsKHR
	(KHRpfn_clSetEventCallback)GET_ALIAS(clSetEventCallback),
	(KHRpfn_clCreateSubBuffer)GET_ALIAS(clCreateSubBuffer),
	(KHRpfn_clSetMemObjectDestructorCallback)GET_ALIAS(clSetMemObjectDestructorCallback),
	(KHRpfn_clCreateUserEvent)GET_ALIAS(clCreateUserEvent),
	(KHRpfn_clSetUserEventStatus)GET_ALIAS(clSetUserEventStatus),
	(KHRpfn_clEnqueueReadBufferRect)GET_ALIAS(clEnqueueReadBufferRect),
	(KHRpfn_clEnqueueWriteBufferRect)GET_ALIAS(clEnqueueWriteBufferRect),
	(KHRpfn_clEnqueueCopyBufferRect)GET_ALIAS(clEnqueueCopyBufferRect),
	NULL, // clCreateSubDevicesEXT,
	NULL, // clRetainDeviceEXT,
	NULL, // clReleaseDeviceEXT,
};

FrameworkProxy * FrameworkProxy::m_pInstance = NULL;
OclSpinMutex FrameworkProxy::m_initializationMutex;

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
			if ( string::npos == ext )
			{
				// If "." not found -> no extension
				ext = str.length();
			}
			// Add Process if before the "."
			//Calculate Extension lenght
			size_t extLen = str.length() - ext + 16;
			std::string procId;
			procId.resize(extLen);
			SPRINTF_S(&procId[0], extLen, "_%d%s", GetProcessId(), &str[ext]);
			str.insert(ext, procId);

			//null-call to get the size
			size_t needed;
			MULTIBYTE_TO_WIDE_CHARACTER_S(&needed, NULL, 0, &str[0], str.length());
			// allocate
			std::wstring wstr;
			wstr.resize(needed);
			// real call
			MULTIBYTE_TO_WIDE_CHARACTER_S(&needed, &wstr[0], wstr.length(), &str[0], str.length());

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
#if defined(USE_GPA)
	// This code was removed for the initial porting of TAL
	// to GPA 4.0 and might be used in later stages
	
	// Open the trace file before any task started in order 
	// to prevent it as showing as part of the task
	//if(m_pConfig->UseTaskalyzer())
	//{
	//	TAL_GetThreadTrace();
	//}
#endif
	
	LOG_INFO(TEXT("%S"), TEXT("Initialize platform module: m_PlatformModule = new PlatformModule()"));
	m_pPlatformModule = new PlatformModule();
	m_pPlatformModule->Initialize(&OclEntryPoints, m_pConfig);

	LOG_INFO(TEXT("Initialize context module: m_pContextModule = new ContextModule(%d)"),m_pPlatformModule);
	m_pContextModule = new ContextModule(m_pPlatformModule);
	m_pContextModule->Initialize(&OclEntryPoints, m_pConfig);

	LOG_INFO(TEXT("Initialize context module: m_pExecutionModule = new ExecutionModule(%d,%d)"), m_pPlatformModule, m_pContextModule);
	m_pExecutionModule = new ExecutionModule(m_pPlatformModule, m_pContextModule);
	m_pExecutionModule->Initialize(&OclEntryPoints, m_pConfig);

	// Initialize TaskExecutor
	LOG_INFO(TEXT("%S"), TEXT("Initialize Executor"));
	GetTaskExecutor()->Init(0, m_pConfig->UseTaskalyzer());

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Destroy()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Destroy()
{
	if (NULL != m_pInstance)
	{
		m_pInstance->Release();
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Release()
{
	LOG_DEBUG(TEXT("%S"), TEXT("FrameworkProxy::Release enter"));

	// Close TaskExecutor
	GetTaskExecutor()->Close(true);	

	m_pExecutionModule->Release();
    delete m_pExecutionModule;

    m_pContextModule->Release(true);
	delete m_pContextModule;
	
	m_pPlatformModule->Release();
	delete m_pPlatformModule;

	
	if (m_pFileLogHandler)
	{
		m_pFileLogHandler->Flush();
		delete m_pFileLogHandler;
		m_pFileLogHandler = NULL;
	}
	m_pConfig->Release();
	if (m_pConfig)
	{
		delete m_pConfig;
		m_pConfig = NULL;
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
		OclAutoMutex cs(&m_initializationMutex);
		if (NULL == m_pInstance)
		{
			m_pInstance = new FrameworkProxy();
		}
	}
	return m_pInstance;
}
