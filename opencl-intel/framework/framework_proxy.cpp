
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

ocl_entry_points FrameworkProxy::OclEntryPoints;


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

void FrameworkProxy::InitOCLEntryPoints()
{
    /// ICD functions
    OclEntryPoints.clGetPlatformIDs = (KHRpfn_clGetPlatformIDs)GET_ALIAS(clGetPlatformIDs);
    OclEntryPoints.clGetPlatformInfo = (KHRpfn_clGetPlatformInfo)GET_ALIAS(clGetPlatformInfo);
    OclEntryPoints.clGetDeviceIDs = (KHRpfn_clGetDeviceIDs)GET_ALIAS(clGetDeviceIDs);
    OclEntryPoints.clGetDeviceInfo = (KHRpfn_clGetDeviceInfo)GET_ALIAS(clGetDeviceInfo);
    OclEntryPoints.clCreateContext = (KHRpfn_clCreateContext)GET_ALIAS(clCreateContext);
    OclEntryPoints.clCreateContextFromType = (KHRpfn_clCreateContextFromType)GET_ALIAS(clCreateContextFromType);
    OclEntryPoints.clRetainContext = (KHRpfn_clRetainContext)GET_ALIAS(clRetainContext);
    OclEntryPoints.clReleaseContext = (KHRpfn_clReleaseContext)GET_ALIAS(clReleaseContext);
    OclEntryPoints.clGetContextInfo = (KHRpfn_clGetContextInfo)GET_ALIAS(clGetContextInfo);
    OclEntryPoints.clCreateCommandQueue = (KHRpfn_clCreateCommandQueue)GET_ALIAS(clCreateCommandQueue);
    OclEntryPoints.clRetainCommandQueue = (KHRpfn_clRetainCommandQueue)GET_ALIAS(clRetainCommandQueue);
    OclEntryPoints.clReleaseCommandQueue = (KHRpfn_clReleaseCommandQueue)GET_ALIAS(clReleaseCommandQueue);
    OclEntryPoints.clGetCommandQueueInfo = (KHRpfn_clGetCommandQueueInfo)GET_ALIAS(clGetCommandQueueInfo);
    OclEntryPoints.clSetCommandQueueProperty = NULL;
    OclEntryPoints.clCreateBuffer = (KHRpfn_clCreateBuffer)GET_ALIAS(clCreateBuffer);
    OclEntryPoints.clCreateImage2D = (KHRpfn_clCreateImage2D)GET_ALIAS(clCreateImage2D);
    OclEntryPoints.clCreateImage3D = (KHRpfn_clCreateImage3D)GET_ALIAS(clCreateImage3D);
    OclEntryPoints.clRetainMemObject = (KHRpfn_clRetainMemObject)GET_ALIAS(clRetainMemObject);
    OclEntryPoints.clReleaseMemObject = (KHRpfn_clReleaseMemObject)GET_ALIAS(clReleaseMemObject);
    OclEntryPoints.clGetSupportedImageFormats = (KHRpfn_clGetSupportedImageFormats)GET_ALIAS(clGetSupportedImageFormats);
    OclEntryPoints.clGetMemObjectInfo = (KHRpfn_clGetMemObjectInfo)GET_ALIAS(clGetMemObjectInfo);
    OclEntryPoints.clGetImageInfo = (KHRpfn_clGetImageInfo)GET_ALIAS(clGetImageInfo);
    OclEntryPoints.clCreateSampler = (KHRpfn_clCreateSampler)GET_ALIAS(clCreateSampler);
    OclEntryPoints.clRetainSampler = (KHRpfn_clRetainSampler)GET_ALIAS(clRetainSampler);
    OclEntryPoints.clReleaseSampler = (KHRpfn_clReleaseSampler)GET_ALIAS(clReleaseSampler);
    OclEntryPoints.clGetSamplerInfo = (KHRpfn_clGetSamplerInfo)GET_ALIAS(clGetSamplerInfo);
    OclEntryPoints.clCreateProgramWithSource = (KHRpfn_clCreateProgramWithSource)GET_ALIAS(clCreateProgramWithSource);
    OclEntryPoints.clCreateProgramWithBinary = (KHRpfn_clCreateProgramWithBinary)GET_ALIAS(clCreateProgramWithBinary);
    OclEntryPoints.clRetainProgram = (KHRpfn_clRetainProgram)GET_ALIAS(clRetainProgram);
    OclEntryPoints.clReleaseProgram = (KHRpfn_clReleaseProgram)GET_ALIAS(clReleaseProgram);
    OclEntryPoints.clBuildProgram = (KHRpfn_clBuildProgram)GET_ALIAS(clBuildProgram);
    OclEntryPoints.clUnloadCompiler = (KHRpfn_clUnloadCompiler)GET_ALIAS(clUnloadCompiler);
    OclEntryPoints.clGetProgramInfo = (KHRpfn_clGetProgramInfo)GET_ALIAS(clGetProgramInfo);
    OclEntryPoints.clGetProgramBuildInfo = (KHRpfn_clGetProgramBuildInfo)GET_ALIAS(clGetProgramBuildInfo);
    OclEntryPoints.clCreateKernel = (KHRpfn_clCreateKernel)GET_ALIAS(clCreateKernel);
    OclEntryPoints.clCreateKernelsInProgram = (KHRpfn_clCreateKernelsInProgram)GET_ALIAS(clCreateKernelsInProgram);
    OclEntryPoints.clRetainKernel = (KHRpfn_clRetainKernel)GET_ALIAS(clRetainKernel);
    OclEntryPoints.clReleaseKernel = (KHRpfn_clReleaseKernel)GET_ALIAS(clReleaseKernel);
    OclEntryPoints.clSetKernelArg = (KHRpfn_clSetKernelArg)GET_ALIAS(clSetKernelArg);
    OclEntryPoints.clGetKernelInfo = (KHRpfn_clGetKernelInfo)GET_ALIAS(clGetKernelInfo);
    OclEntryPoints.clGetKernelWorkGroupInfo = (KHRpfn_clGetKernelWorkGroupInfo)GET_ALIAS(clGetKernelWorkGroupInfo);
    OclEntryPoints.clWaitForEvents = (KHRpfn_clWaitForEvents)GET_ALIAS(clWaitForEvents);
    OclEntryPoints.clGetEventInfo = (KHRpfn_clGetEventInfo)GET_ALIAS(clGetEventInfo);
    OclEntryPoints.clRetainEvent = (KHRpfn_clRetainEvent)GET_ALIAS(clRetainEvent);
    OclEntryPoints.clReleaseEvent = (KHRpfn_clReleaseEvent)GET_ALIAS(clReleaseEvent);
    OclEntryPoints.clGetEventProfilingInfo = (KHRpfn_clGetEventProfilingInfo)GET_ALIAS(clGetEventProfilingInfo);
    OclEntryPoints.clFlush = (KHRpfn_clFlush)GET_ALIAS(clFlush);
    OclEntryPoints.clFinish = (KHRpfn_clFinish)GET_ALIAS(clFinish);
    OclEntryPoints.clEnqueueReadBuffer = (KHRpfn_clEnqueueReadBuffer)GET_ALIAS(clEnqueueReadBuffer);
    OclEntryPoints.clEnqueueWriteBuffer = (KHRpfn_clEnqueueWriteBuffer)GET_ALIAS(clEnqueueWriteBuffer);
    OclEntryPoints.clEnqueueCopyBuffer = (KHRpfn_clEnqueueCopyBuffer)GET_ALIAS(clEnqueueCopyBuffer);
    OclEntryPoints.clEnqueueReadImage = (KHRpfn_clEnqueueReadImage)GET_ALIAS(clEnqueueReadImage);
    OclEntryPoints.clEnqueueWriteImage = (KHRpfn_clEnqueueWriteImage)GET_ALIAS(clEnqueueWriteImage);
    OclEntryPoints.clEnqueueCopyImage = (KHRpfn_clEnqueueCopyImage)GET_ALIAS(clEnqueueCopyImage);
    OclEntryPoints.clEnqueueCopyImageToBuffer = (KHRpfn_clEnqueueCopyImageToBuffer)GET_ALIAS(clEnqueueCopyImageToBuffer);
    OclEntryPoints.clEnqueueCopyBufferToImage = (KHRpfn_clEnqueueCopyBufferToImage)GET_ALIAS(clEnqueueCopyBufferToImage);
    OclEntryPoints.clEnqueueMapBuffer = (KHRpfn_clEnqueueMapBuffer)GET_ALIAS(clEnqueueMapBuffer);
    OclEntryPoints.clEnqueueMapImage = (KHRpfn_clEnqueueMapImage)GET_ALIAS(clEnqueueMapImage);
    OclEntryPoints.clEnqueueUnmapMemObject = (KHRpfn_clEnqueueUnmapMemObject)GET_ALIAS(clEnqueueUnmapMemObject);
    OclEntryPoints.clEnqueueNDRangeKernel = (KHRpfn_clEnqueueNDRangeKernel)GET_ALIAS(clEnqueueNDRangeKernel);
    OclEntryPoints.clEnqueueTask = (KHRpfn_clEnqueueTask)GET_ALIAS(clEnqueueTask);
    OclEntryPoints.clEnqueueNativeKernel = (KHRpfn_clEnqueueNativeKernel)GET_ALIAS(clEnqueueNativeKernel);
    OclEntryPoints.clEnqueueMarker = (KHRpfn_clEnqueueMarker)GET_ALIAS(clEnqueueMarker);
    OclEntryPoints.clEnqueueWaitForEvents = (KHRpfn_clEnqueueWaitForEvents)GET_ALIAS(clEnqueueWaitForEvents);
    OclEntryPoints.clEnqueueBarrier = (KHRpfn_clEnqueueBarrier)GET_ALIAS(clEnqueueBarrier);
    OclEntryPoints.clGetExtensionFunctionAddress = (KHRpfn_clGetExtensionFunctionAddress)GET_ALIAS(clGetExtensionFunctionAddress);
    OclEntryPoints.clCreateFromGLBuffer = (KHRpfn_clCreateFromGLBuffer)GET_ALIAS(clCreateFromGLBuffer);
    OclEntryPoints.clCreateFromGLTexture2D = (KHRpfn_clCreateFromGLTexture2D)GET_ALIAS(clCreateFromGLTexture2D);
    OclEntryPoints.clCreateFromGLTexture3D = (KHRpfn_clCreateFromGLTexture3D)GET_ALIAS(clCreateFromGLTexture3D);
    OclEntryPoints.clCreateFromGLRenderbuffer = (KHRpfn_clCreateFromGLRenderbuffer)GET_ALIAS(clCreateFromGLRenderbuffer);
    OclEntryPoints.clGetGLObjectInfo = (KHRpfn_clGetGLObjectInfo)GET_ALIAS(clGetGLObjectInfo);
    OclEntryPoints.clGetGLTextureInfo = (KHRpfn_clGetGLTextureInfo)GET_ALIAS(clGetGLTextureInfo);
    OclEntryPoints.clEnqueueAcquireGLObjects = (KHRpfn_clEnqueueAcquireGLObjects)GET_ALIAS(clEnqueueAcquireGLObjects);
    OclEntryPoints.clEnqueueReleaseGLObjects = (KHRpfn_clEnqueueReleaseGLObjects)GET_ALIAS(clEnqueueReleaseGLObjects);
    OclEntryPoints.clGetGLContextInfoKHR = (KHRpfn_clGetGLContextInfoKHR)GET_ALIAS(clGetGLContextInfoKHR);
    OclEntryPoints.clGetDeviceIDsFromD3D10KHR = NULL;
    OclEntryPoints.clCreateFromD3D10BufferKHR = NULL;
    OclEntryPoints.clCreateFromD3D10Texture2DKHR = NULL;
    OclEntryPoints.clCreateFromD3D10Texture3DKHR = NULL;
    OclEntryPoints.clEnqueueAcquireD3D10ObjectsKHR = NULL;
    OclEntryPoints.clEnqueueReleaseD3D10ObjectsKHR = NULL;
    OclEntryPoints.clSetEventCallback = (KHRpfn_clSetEventCallback)GET_ALIAS(clSetEventCallback);
    OclEntryPoints.clCreateSubBuffer = (KHRpfn_clCreateSubBuffer)GET_ALIAS(clCreateSubBuffer);
    OclEntryPoints.clSetMemObjectDestructorCallback = (KHRpfn_clSetMemObjectDestructorCallback)GET_ALIAS(clSetMemObjectDestructorCallback);
    OclEntryPoints.clCreateUserEvent = (KHRpfn_clCreateUserEvent)GET_ALIAS(clCreateUserEvent);
    OclEntryPoints.clSetUserEventStatus = (KHRpfn_clSetUserEventStatus)GET_ALIAS(clSetUserEventStatus);
    OclEntryPoints.clEnqueueReadBufferRect = (KHRpfn_clEnqueueReadBufferRect)GET_ALIAS(clEnqueueReadBufferRect);
    OclEntryPoints.clEnqueueWriteBufferRect = (KHRpfn_clEnqueueWriteBufferRect)GET_ALIAS(clEnqueueWriteBufferRect);
    OclEntryPoints.clEnqueueCopyBufferRect = (KHRpfn_clEnqueueCopyBufferRect)GET_ALIAS(clEnqueueCopyBufferRect);
    OclEntryPoints.clCreateSubDevicesEXT = (KHRpfn_clCreateSubDevicesEXT)GET_ALIAS(clCreateSubDevicesEXT);
    OclEntryPoints.clRetainDeviceEXT = (KHRpfn_clRetainDeviceEXT)GET_ALIAS(clRetainDeviceEXT);
    OclEntryPoints.clReleaseDeviceEXT = (KHRpfn_clReleaseDeviceEXT)GET_ALIAS(clReleaseDeviceEXT);

    /// Extra functions for Common Runtime
    OclEntryPoints.clGetKernelArgInfo = (KHRpfn_clGetKernelArgInfo)GET_ALIAS(clGetKernelArgInfo);
#if defined DX9_MEDIA_SHARING
    OclEntryPoints.clGetDeviceIDsFromDX9INTEL = (KHRpfn_clGetDeviceIDsFromDX9INTEL)GET_ALIAS(clGetDeviceIDsFromDX9INTEL);
    OclEntryPoints.clCreateFromDX9MediaSurfaceINTEL = (KHRpfn_clCreateFromDX9MediaSurfaceINTEL)GET_ALIAS(clCreateFromDX9MediaSurfaceINTEL);
    OclEntryPoints.clEnqueueAcquireDX9ObjectsINTEL = (KHRpfn_clEnqueueAcquireDX9ObjectsINTEL)GET_ALIAS(clEnqueueAcquireDX9ObjectsINTEL);
    OclEntryPoints.clEnqueueReleaseDX9ObjectsINTEL = (KHRpfn_clEnqueueReleaseDX9ObjectsINTEL)GET_ALIAS(clEnqueueReleaseDX9ObjectsINTEL);
#endif
    /// Extra CPU specific functions
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Initialize()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Initialize()
{
   
    // Initialize entry points table
    InitOCLEntryPoints();

#ifdef WIN32
	// The loading on tbb.dll was delayed,
	// Need to load manually before defualt dll is loaded
	char tBuff[MAX_PATH], *ptCutBuff;
	char oldDLLDir[MAX_PATH];
	DWORD oldDllRet;
	int iCh = '\\';
	int iPathLength;

	HMODULE hModule = NULL;
	
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)FrameworkProxy::Instance, &hModule);
	assert(NULL!=hModule);
	if ( NULL != hModule )
	{
		oldDllRet = GetDllDirectoryA(MAX_PATH, oldDLLDir);
		GetModuleFileNameA(hModule, tBuff, MAX_PATH-1);
		ptCutBuff = strrchr ( tBuff, iCh );
		iPathLength = (int)(ptCutBuff - tBuff + 1);
		tBuff[iPathLength] = 0;
		// Add SDK installation path to DLL search directory
		SetDllDirectoryA(tBuff);

		strcpy_s(clFRAMEWORK_CFG_PATH, MAX_PATH-1, tBuff);
		strcat_s(clFRAMEWORK_CFG_PATH, MAX_PATH-1, "cl.cfg");
	}
#endif

	// initialize configuration file
	m_pConfig = new OCLConfig();
    if (NULL == m_pConfig)
    {
        //Todo: terrible crash imminent
        return;
    }
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
	m_GPAData.bUseGPA = m_pConfig->UseGPA();
	m_GPAData.bEnableAPITracing = m_pConfig->EnableAPITracing();
	m_GPAData.bEnableContextTracing = m_pConfig->EnableContextTracing();
	m_GPAData.cStatusMarkerFlags = 0;
	if (m_GPAData.bUseGPA)
	{
		if (m_pConfig->ShowQueuedMarker())
			m_GPAData.cStatusMarkerFlags += GPA_SHOW_QUEUED_MARKER;
		if (m_pConfig->ShowSubmittedMarker())
			m_GPAData.cStatusMarkerFlags += GPA_SHOW_SUBMITTED_MARKER;
		if (m_pConfig->ShowRunningMarker())
			m_GPAData.cStatusMarkerFlags += GPA_SHOW_RUNNING_MARKER;
		if (m_pConfig->ShowCompletedMarker())
			m_GPAData.cStatusMarkerFlags += GPA_SHOW_COMPLETED_MARKER;

		// Create domains
		m_GPAData.pDeviceDomain = __itt_domain_createA("com.intel.open_cl.device");
		m_GPAData.pAPIDomain = __itt_domain_createA("com.intel.open_cl.api");
		if (m_GPAData.bEnableContextTracing)
		{
			m_GPAData.pContextDomain = __itt_domain_createA("com.intel.open_cl.context");
			
			// Create Context task group
			__itt_string_handle* pContextTrackGroupHandle = __itt_string_handle_createA("Context Track Group");
			m_GPAData.pContextTrackGroup = __itt_track_group_create(pContextTrackGroupHandle, __itt_track_group_type_normal);

			// Create task states
			m_GPAData.pWaitingTaskState = __ittx_task_state_create(m_GPAData.pContextDomain, "OpenCL Waiting");
			m_GPAData.pRunningTaskState = __ittx_task_state_create(m_GPAData.pContextDomain, "OpenCL Running");
		}

		m_GPAData.pReadHandle = __itt_string_handle_createA("Read");
		m_GPAData.pWriteHandle = __itt_string_handle_createA("Write");
		m_GPAData.pCopyHandle = __itt_string_handle_createA("Copy");
		m_GPAData.pMapHandle = __itt_string_handle_createA("Map");
		m_GPAData.pUnmapHandle = __itt_string_handle_createA("Unmap");
		m_GPAData.pSizeHandle = __itt_string_handle_createA("Size");
		m_GPAData.pWidthHandle = __itt_string_handle_createA("Width");
		m_GPAData.pHeightHandle = __itt_string_handle_createA("Height");
		m_GPAData.pDepthHandle = __itt_string_handle_createA("Depth");
		m_GPAData.pWorkGroupSizeHandle = __itt_string_handle_createA("Work Group Size");
		m_GPAData.pNumberOfWorkGroupsHandle = __itt_string_handle_createA("Number of Work Groups");
		m_GPAData.pWorkGroupRangeHandle = __itt_string_handle_createA("Work Group Range");
		m_GPAData.pMarkerHandle = __itt_string_handle_createA("Marker");
		m_GPAData.pWorkDimensionHandle = __itt_string_handle_createA("Work Dimension");
		m_GPAData.pGlobalWorkSizeHandle = __itt_string_handle_createA("Global Work Size");
		m_GPAData.pLocalWorkSizeHandle = __itt_string_handle_createA("Local Work Size");
		m_GPAData.pGlobalWorkOffsetHandle = __itt_string_handle_createA("Global Work Offset");
	}
#endif
	
	LOG_INFO(TEXT("%S"), TEXT("Initialize platform module: m_PlatformModule = new PlatformModule()"));
	m_pPlatformModule = new PlatformModule();
	m_pPlatformModule->Initialize(&OclEntryPoints, m_pConfig, &m_GPAData);

	LOG_INFO(TEXT("Initialize context module: m_pContextModule = new ContextModule(%d)"),m_pPlatformModule);
	m_pContextModule = new ContextModule(m_pPlatformModule);
	m_pContextModule->Initialize(&OclEntryPoints, &m_GPAData);

	LOG_INFO(TEXT("Initialize context module: m_pExecutionModule = new ExecutionModule(%d,%d)"), m_pPlatformModule, m_pContextModule);
	m_pExecutionModule = new ExecutionModule(m_pPlatformModule, m_pContextModule);
	m_pExecutionModule->Initialize(&OclEntryPoints, m_pConfig, &m_GPAData);

	// Initialize TaskExecutor
	LOG_INFO(TEXT("%S"), TEXT("Initialize Executor"));
	GetTaskExecutor()->Init(0, &m_GPAData);

#ifdef WIN32
	if ( 0 != oldDllRet )
	{
		SetDllDirectoryA(oldDLLDir);
	}
#endif

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
    ITaskExecutor* pTaskExecutor = GetTaskExecutor();
	if (NULL != pTaskExecutor)
    {
        pTaskExecutor->Close(true);
    }

    if (NULL != m_pExecutionModule)
    {
        m_pExecutionModule->Release();
        delete m_pExecutionModule;
    }

    if (NULL != m_pContextModule)
    {
        m_pContextModule->Release(true);
        delete m_pContextModule;
    }
	
    if (NULL != m_pPlatformModule)
    {
        m_pPlatformModule->Release();
        delete m_pPlatformModule;
    }

	
	if (NULL != m_pFileLogHandler)
	{
		m_pFileLogHandler->Flush();
		delete m_pFileLogHandler;
		m_pFileLogHandler = NULL;
	}
    if (NULL != m_pConfig)
    {
        m_pConfig->Release();
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
