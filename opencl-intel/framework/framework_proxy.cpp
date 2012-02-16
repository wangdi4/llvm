
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

KHRicdVendorDispatch	    FrameworkProxy::ICDDispatchTable;
COCLCRTDispatchTable        FrameworkProxy::CRTDispatchTable;
ocl_entry_points            FrameworkProxy::OclEntryPoints;


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
    OclEntryPoints.icdDispatch = &ICDDispatchTable;
    OclEntryPoints.crtDispatch = &CRTDispatchTable;
    
    /// ICD functions
    ICDDispatchTable.clGetPlatformIDs = (KHRpfn_clGetPlatformIDs)GET_ALIAS(clGetPlatformIDs);
    ICDDispatchTable.clGetPlatformInfo = (KHRpfn_clGetPlatformInfo)GET_ALIAS(clGetPlatformInfo);
    ICDDispatchTable.clGetDeviceIDs = (KHRpfn_clGetDeviceIDs)GET_ALIAS(clGetDeviceIDs);
    ICDDispatchTable.clGetDeviceInfo = (KHRpfn_clGetDeviceInfo)GET_ALIAS(clGetDeviceInfo);
    ICDDispatchTable.clCreateContext = (KHRpfn_clCreateContext)GET_ALIAS(clCreateContext);
    ICDDispatchTable.clCreateContextFromType = (KHRpfn_clCreateContextFromType)GET_ALIAS(clCreateContextFromType);
    ICDDispatchTable.clRetainContext = (KHRpfn_clRetainContext)GET_ALIAS(clRetainContext);
    ICDDispatchTable.clReleaseContext = (KHRpfn_clReleaseContext)GET_ALIAS(clReleaseContext);
    ICDDispatchTable.clGetContextInfo = (KHRpfn_clGetContextInfo)GET_ALIAS(clGetContextInfo);
    ICDDispatchTable.clCreateCommandQueue = (KHRpfn_clCreateCommandQueue)GET_ALIAS(clCreateCommandQueue);
    ICDDispatchTable.clRetainCommandQueue = (KHRpfn_clRetainCommandQueue)GET_ALIAS(clRetainCommandQueue);
    ICDDispatchTable.clReleaseCommandQueue = (KHRpfn_clReleaseCommandQueue)GET_ALIAS(clReleaseCommandQueue);
    ICDDispatchTable.clGetCommandQueueInfo = (KHRpfn_clGetCommandQueueInfo)GET_ALIAS(clGetCommandQueueInfo);
    ICDDispatchTable.clSetCommandQueueProperty = NULL;
    ICDDispatchTable.clCreateBuffer = (KHRpfn_clCreateBuffer)GET_ALIAS(clCreateBuffer);
    ICDDispatchTable.clCreateImage = (KHRpfn_clCreateImage)GET_ALIAS(clCreateImage);
    ICDDispatchTable.clCreateImage2D = (KHRpfn_clCreateImage2D)GET_ALIAS(clCreateImage2D);
    ICDDispatchTable.clCreateImage3D = (KHRpfn_clCreateImage3D)GET_ALIAS(clCreateImage3D);
    ICDDispatchTable.clRetainMemObject = (KHRpfn_clRetainMemObject)GET_ALIAS(clRetainMemObject);
    ICDDispatchTable.clReleaseMemObject = (KHRpfn_clReleaseMemObject)GET_ALIAS(clReleaseMemObject);
    ICDDispatchTable.clGetSupportedImageFormats = (KHRpfn_clGetSupportedImageFormats)GET_ALIAS(clGetSupportedImageFormats);
    ICDDispatchTable.clGetMemObjectInfo = (KHRpfn_clGetMemObjectInfo)GET_ALIAS(clGetMemObjectInfo);
    ICDDispatchTable.clGetImageInfo = (KHRpfn_clGetImageInfo)GET_ALIAS(clGetImageInfo);
    ICDDispatchTable.clCreateSampler = (KHRpfn_clCreateSampler)GET_ALIAS(clCreateSampler);
    ICDDispatchTable.clRetainSampler = (KHRpfn_clRetainSampler)GET_ALIAS(clRetainSampler);
    ICDDispatchTable.clReleaseSampler = (KHRpfn_clReleaseSampler)GET_ALIAS(clReleaseSampler);
    ICDDispatchTable.clGetSamplerInfo = (KHRpfn_clGetSamplerInfo)GET_ALIAS(clGetSamplerInfo);
    ICDDispatchTable.clCreateProgramWithSource = (KHRpfn_clCreateProgramWithSource)GET_ALIAS(clCreateProgramWithSource);
    ICDDispatchTable.clCreateProgramWithBinary = (KHRpfn_clCreateProgramWithBinary)GET_ALIAS(clCreateProgramWithBinary);
    ICDDispatchTable.clRetainProgram = (KHRpfn_clRetainProgram)GET_ALIAS(clRetainProgram);
    ICDDispatchTable.clReleaseProgram = (KHRpfn_clReleaseProgram)GET_ALIAS(clReleaseProgram);
    ICDDispatchTable.clBuildProgram = (KHRpfn_clBuildProgram)GET_ALIAS(clBuildProgram);
    ICDDispatchTable.clUnloadCompiler = (KHRpfn_clUnloadCompiler)GET_ALIAS(clUnloadCompiler);
    ICDDispatchTable.clGetProgramInfo = (KHRpfn_clGetProgramInfo)GET_ALIAS(clGetProgramInfo);
    ICDDispatchTable.clGetProgramBuildInfo = (KHRpfn_clGetProgramBuildInfo)GET_ALIAS(clGetProgramBuildInfo);
    ICDDispatchTable.clCreateKernel = (KHRpfn_clCreateKernel)GET_ALIAS(clCreateKernel);
    ICDDispatchTable.clCreateKernelsInProgram = (KHRpfn_clCreateKernelsInProgram)GET_ALIAS(clCreateKernelsInProgram);
    ICDDispatchTable.clRetainKernel = (KHRpfn_clRetainKernel)GET_ALIAS(clRetainKernel);
    ICDDispatchTable.clReleaseKernel = (KHRpfn_clReleaseKernel)GET_ALIAS(clReleaseKernel);
    ICDDispatchTable.clSetKernelArg = (KHRpfn_clSetKernelArg)GET_ALIAS(clSetKernelArg);
    ICDDispatchTable.clGetKernelInfo = (KHRpfn_clGetKernelInfo)GET_ALIAS(clGetKernelInfo);
    ICDDispatchTable.clGetKernelWorkGroupInfo = (KHRpfn_clGetKernelWorkGroupInfo)GET_ALIAS(clGetKernelWorkGroupInfo);
    ICDDispatchTable.clWaitForEvents = (KHRpfn_clWaitForEvents)GET_ALIAS(clWaitForEvents);
    ICDDispatchTable.clGetEventInfo = (KHRpfn_clGetEventInfo)GET_ALIAS(clGetEventInfo);
    ICDDispatchTable.clRetainEvent = (KHRpfn_clRetainEvent)GET_ALIAS(clRetainEvent);
    ICDDispatchTable.clReleaseEvent = (KHRpfn_clReleaseEvent)GET_ALIAS(clReleaseEvent);
    ICDDispatchTable.clGetEventProfilingInfo = (KHRpfn_clGetEventProfilingInfo)GET_ALIAS(clGetEventProfilingInfo);
    ICDDispatchTable.clFlush = (KHRpfn_clFlush)GET_ALIAS(clFlush);
    ICDDispatchTable.clFinish = (KHRpfn_clFinish)GET_ALIAS(clFinish);
    ICDDispatchTable.clEnqueueReadBuffer = (KHRpfn_clEnqueueReadBuffer)GET_ALIAS(clEnqueueReadBuffer);
    ICDDispatchTable.clEnqueueWriteBuffer = (KHRpfn_clEnqueueWriteBuffer)GET_ALIAS(clEnqueueWriteBuffer);
    ICDDispatchTable.clEnqueueCopyBuffer = (KHRpfn_clEnqueueCopyBuffer)GET_ALIAS(clEnqueueCopyBuffer);
    ICDDispatchTable.clEnqueueFillBuffer = (KHRpfn_clEnqueueFillBuffer)GET_ALIAS(clEnqueueFillBuffer);
    ICDDispatchTable.clEnqueueReadImage = (KHRpfn_clEnqueueReadImage)GET_ALIAS(clEnqueueReadImage);
    ICDDispatchTable.clEnqueueWriteImage = (KHRpfn_clEnqueueWriteImage)GET_ALIAS(clEnqueueWriteImage);
    ICDDispatchTable.clEnqueueCopyImage = (KHRpfn_clEnqueueCopyImage)GET_ALIAS(clEnqueueCopyImage);
    ICDDispatchTable.clEnqueueCopyImageToBuffer = (KHRpfn_clEnqueueCopyImageToBuffer)GET_ALIAS(clEnqueueCopyImageToBuffer);
    ICDDispatchTable.clEnqueueCopyBufferToImage = (KHRpfn_clEnqueueCopyBufferToImage)GET_ALIAS(clEnqueueCopyBufferToImage);
    ICDDispatchTable.clEnqueueMapBuffer = (KHRpfn_clEnqueueMapBuffer)GET_ALIAS(clEnqueueMapBuffer);
    ICDDispatchTable.clEnqueueMapImage = (KHRpfn_clEnqueueMapImage)GET_ALIAS(clEnqueueMapImage);
    ICDDispatchTable.clEnqueueUnmapMemObject = (KHRpfn_clEnqueueUnmapMemObject)GET_ALIAS(clEnqueueUnmapMemObject);
    ICDDispatchTable.clEnqueueNDRangeKernel = (KHRpfn_clEnqueueNDRangeKernel)GET_ALIAS(clEnqueueNDRangeKernel);
    ICDDispatchTable.clEnqueueTask = (KHRpfn_clEnqueueTask)GET_ALIAS(clEnqueueTask);
    ICDDispatchTable.clEnqueueNativeKernel = (KHRpfn_clEnqueueNativeKernel)GET_ALIAS(clEnqueueNativeKernel);
    ICDDispatchTable.clEnqueueMarker = (KHRpfn_clEnqueueMarker)GET_ALIAS(clEnqueueMarker);
    ICDDispatchTable.clEnqueueMarkerWithWaitList = (KHRpfn_clEnqueueMarkerWithWaitList)GET_ALIAS(clEnqueueMarkerWithWaitList);
    ICDDispatchTable.clEnqueueBarrierWithWaitList = (KHRpfn_clEnqueueBarrierWithWaitList)GET_ALIAS(clEnqueueBarrierWithWaitList);
    ICDDispatchTable.clEnqueueWaitForEvents = (KHRpfn_clEnqueueWaitForEvents)GET_ALIAS(clEnqueueWaitForEvents);
    ICDDispatchTable.clEnqueueBarrier = (KHRpfn_clEnqueueBarrier)GET_ALIAS(clEnqueueBarrier);
    ICDDispatchTable.clGetExtensionFunctionAddress = (KHRpfn_clGetExtensionFunctionAddress)GET_ALIAS(clGetExtensionFunctionAddress);
    ICDDispatchTable.clCreateFromGLBuffer = (KHRpfn_clCreateFromGLBuffer)GET_ALIAS(clCreateFromGLBuffer);
    ICDDispatchTable.clCreateFromGLTexture2D = (KHRpfn_clCreateFromGLTexture2D)GET_ALIAS(clCreateFromGLTexture2D);
    ICDDispatchTable.clCreateFromGLTexture3D = (KHRpfn_clCreateFromGLTexture3D)GET_ALIAS(clCreateFromGLTexture3D);
    ICDDispatchTable.clCreateFromGLRenderbuffer = (KHRpfn_clCreateFromGLRenderbuffer)GET_ALIAS(clCreateFromGLRenderbuffer);
    ICDDispatchTable.clGetGLObjectInfo = (KHRpfn_clGetGLObjectInfo)GET_ALIAS(clGetGLObjectInfo);
    ICDDispatchTable.clGetGLTextureInfo = (KHRpfn_clGetGLTextureInfo)GET_ALIAS(clGetGLTextureInfo);
    ICDDispatchTable.clEnqueueAcquireGLObjects = (KHRpfn_clEnqueueAcquireGLObjects)GET_ALIAS(clEnqueueAcquireGLObjects);
    ICDDispatchTable.clEnqueueReleaseGLObjects = (KHRpfn_clEnqueueReleaseGLObjects)GET_ALIAS(clEnqueueReleaseGLObjects);
    ICDDispatchTable.clGetGLContextInfoKHR = (KHRpfn_clGetGLContextInfoKHR)GET_ALIAS(clGetGLContextInfoKHR);
    ICDDispatchTable.clGetDeviceIDsFromD3D10KHR = NULL;
    ICDDispatchTable.clCreateFromD3D10BufferKHR = NULL;
    ICDDispatchTable.clCreateFromD3D10Texture2DKHR = NULL;
    ICDDispatchTable.clCreateFromD3D10Texture3DKHR = NULL;
    ICDDispatchTable.clEnqueueAcquireD3D10ObjectsKHR = NULL;
    ICDDispatchTable.clEnqueueReleaseD3D10ObjectsKHR = NULL;
    ICDDispatchTable.clSetEventCallback = (KHRpfn_clSetEventCallback)GET_ALIAS(clSetEventCallback);
    ICDDispatchTable.clCreateSubBuffer = (KHRpfn_clCreateSubBuffer)GET_ALIAS(clCreateSubBuffer);
    ICDDispatchTable.clSetMemObjectDestructorCallback = (KHRpfn_clSetMemObjectDestructorCallback)GET_ALIAS(clSetMemObjectDestructorCallback);
    ICDDispatchTable.clCreateUserEvent = (KHRpfn_clCreateUserEvent)GET_ALIAS(clCreateUserEvent);
    ICDDispatchTable.clSetUserEventStatus = (KHRpfn_clSetUserEventStatus)GET_ALIAS(clSetUserEventStatus);
    ICDDispatchTable.clEnqueueReadBufferRect = (KHRpfn_clEnqueueReadBufferRect)GET_ALIAS(clEnqueueReadBufferRect);
    ICDDispatchTable.clEnqueueWriteBufferRect = (KHRpfn_clEnqueueWriteBufferRect)GET_ALIAS(clEnqueueWriteBufferRect);
    ICDDispatchTable.clEnqueueCopyBufferRect = (KHRpfn_clEnqueueCopyBufferRect)GET_ALIAS(clEnqueueCopyBufferRect);
    ICDDispatchTable.clCreateSubDevicesEXT = (KHRpfn_clCreateSubDevicesEXT)GET_ALIAS(clCreateSubDevicesEXT);
    ICDDispatchTable.clRetainDeviceEXT = (KHRpfn_clRetainDeviceEXT)GET_ALIAS(clRetainDeviceEXT);
    ICDDispatchTable.clReleaseDeviceEXT = (KHRpfn_clReleaseDeviceEXT)GET_ALIAS(clReleaseDeviceEXT);
    ICDDispatchTable.clEnqueueMigrateMemObjects = (KHRpfn_clEnqueueMigrateMemObjects)GET_ALIAS(clEnqueueMigrateMemObjects);
    ICDDispatchTable.clCreateSubDevices = (KHRpfn_clCreateSubDevices)GET_ALIAS(clCreateSubDevices);
    ICDDispatchTable.clRetainDevice = (KHRpfn_clRetainDevice)GET_ALIAS(clRetainDevice);
    ICDDispatchTable.clReleaseDevice = (KHRpfn_clReleaseDevice)GET_ALIAS(clReleaseDevice);       
    ICDDispatchTable.clGetKernelArgInfo = (KHRpfn_clGetKernelArgInfo)GET_ALIAS(clGetKernelArgInfo);    

    ICDDispatchTable.clEnqueueBarrierWithWaitList = (KHRpfn_clEnqueueBarrierWithWaitList)GET_ALIAS(clEnqueueBarrierWithWaitList);
    ICDDispatchTable.clCompileProgram = (KHRpfn_clCompileProgram)GET_ALIAS(clCompileProgram);
    ICDDispatchTable.clLinkProgram = (KHRpfn_clLinkProgram)GET_ALIAS(clLinkProgram);
    ICDDispatchTable.clEnqueueMarkerWithWaitList = (KHRpfn_clEnqueueMarkerWithWaitList)GET_ALIAS(clEnqueueMarkerWithWaitList);
    
#if defined DX9_MEDIA_SHARING
    CRTDispatchTable.clGetDeviceIDsFromDX9INTEL = (KHRpfn_clGetDeviceIDsFromDX9INTEL)GET_ALIAS(clGetDeviceIDsFromDX9INTEL);
    CRTDispatchTable.clCreateFromDX9MediaSurfaceINTEL = (KHRpfn_clCreateFromDX9MediaSurfaceINTEL)GET_ALIAS(clCreateFromDX9MediaSurfaceINTEL);
    CRTDispatchTable.clEnqueueAcquireDX9ObjectsINTEL = (KHRpfn_clEnqueueAcquireDX9ObjectsINTEL)GET_ALIAS(clEnqueueAcquireDX9ObjectsINTEL);
    CRTDispatchTable.clEnqueueReleaseDX9ObjectsINTEL = (KHRpfn_clEnqueueReleaseDX9ObjectsINTEL)GET_ALIAS(clEnqueueReleaseDX9ObjectsINTEL);
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
	DWORD oldDllRet = 0 ;
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
		m_GPAData.pSyncDataHandle = __itt_string_handle_createA("Sync Data");
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
		m_pInstance->Release(true);
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FrameworkProxy::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
void FrameworkProxy::Release(bool bTerminate)
{
	LOG_DEBUG(TEXT("%S"), TEXT("FrameworkProxy::Release enter"));

    if (NULL != m_pExecutionModule)
    {
        m_pExecutionModule->Release(bTerminate);
        delete m_pExecutionModule;
    }

    if (NULL != m_pContextModule)
    {
        m_pContextModule->Release(bTerminate);
        delete m_pContextModule;
    }
	
    if (NULL != m_pPlatformModule)
    {
        m_pPlatformModule->Release(bTerminate);
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
