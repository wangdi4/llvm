// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

///////////////////////////////////////////////////////////
//  ProgramService.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "program_service.h"
#include "program_config.h"
#include "cpu_logger.h"
#include "backend_wrapper.h"

#include <builtin_kernels.h>
#include <cpu_dev_limits.h>
#include <cl_synch_objects.h>

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::BuiltInKernels;

// Static members
static cl_prog_binary_desc gSupportedBinTypes[] =
{
    {CL_PROG_DLL_X86, 0, 0},
    {CL_PROG_BIN_EXECUTABLE_LLVM, 0, 0},
};
static  unsigned int    UNUSED(gSupportedBinTypesCount) = sizeof(gSupportedBinTypes)/sizeof(cl_prog_binary_desc);

ProgramService::ProgramService(cl_int devId,
                               IOCLFrameworkCallbacks *devCallbacks,
                               IOCLDevLogDescriptor *logDesc,
                               CPUDeviceConfig *config,
                               ICLDevBackendServiceFactory* pBackendFactory) :
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0),
    m_pCallBacks(devCallbacks), m_pBackendFactory(pBackendFactory), m_pBackendCompiler(nullptr),
    m_pBackendExecutor(nullptr), m_pBackendImageService(nullptr), m_pCPUConfig(config)
{
    assert(m_pBackendFactory && "getting backend factory assumed to allways succeed if initialization has succeeded");

    if ( nullptr != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, "CPU Device: Program Service", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CPUDevice: Program Service - Created"));
}

ProgramService::~ProgramService()
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CPUDevice: Program Service - Destructed"));

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }

    if( m_pBackendCompiler )
    {
        m_pBackendCompiler->Release();
    }

    if( m_pBackendExecutor )
    {
        m_pBackendExecutor->Release();
    }

    if( m_pBackendImageService )
    {
        m_pBackendImageService->Release();
    }
}

/****************************************************************************************************************
 Init
    Description
        Initializes the program service. Initialize all the components that could fail - this is an only way to
        return the initialization error information when the exceptions are not used
    Input
        NONE
    Output
        NONE
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_FAILED           The function failed
********************************************************************************************************************/
cl_dev_err_code ProgramService::Init()
{
    ProgramConfig programConfig;
    programConfig.InitFromCpuConfig(*m_pCPUConfig);

    ICLDevBackendCompilationService* pCompiler = nullptr;
    cl_dev_err_code ret = m_pBackendFactory->GetCompilationService(&programConfig, &pCompiler);
    if( CL_DEV_FAILED(ret) )
    {
        return ret;
    }

    ICLDevBackendImageService* pImageService = nullptr;
    ret = m_pBackendFactory->GetImageService(&programConfig, &pImageService);
    if( CL_DEV_FAILED(ret) )
    {
        pCompiler->Release();
        return ret;
    }

    ICLDevBackendExecutionService* pExecutor = nullptr;
    ret = m_pBackendFactory->GetExecutionService(&programConfig, &pExecutor);
    if( CL_DEV_FAILED(ret) )
    {
        //Oh, where is my auto_ptr_ex :-( ?
        pImageService->Release();
        pCompiler->Release();
        return ret;
    }

    m_pBackendCompiler = pCompiler;
    m_pBackendExecutor = pExecutor;
    m_pBackendImageService = pImageService;
    return CL_DEV_SUCCESS;
}



/****************************************************************************************************************
 CheckProgramBinary
    Description
        Performs syntax validation of the intermediate or binary to be built by the device during later stages.
        Call backend compiler to do the check
    Input
        bin_size                Size of the binary buffer
        bin                     A pointer to binary buffer that holds program container defined by cl_prog_container.
    Output
        NONE
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_VALUE    If bin_size is 0 or bin is NULL.
        CL_DEV_INVALID_BINARY   If the binary is not supported by the device or program container content is invalid.
********************************************************************************************************************/
cl_dev_err_code ProgramService::CheckProgramBinary (size_t IN binSize, const void* IN bin)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CheckProgramBinary enter"));
    // Need to check if binary is supported by the device
    return m_pBackendCompiler->CheckProgramBinary(bin, binSize);
}

/*******************************************************************************************************************
clDevCreateProgram
    Description
        Creates a device specific program entity (no build is performed).
    Input
        bin_size                        Size of the binary buffer
        bin                             A pointer to binary buffer that holds program container defined by cl_prog_container.
        prop                            Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_BINARY           If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/
cl_dev_err_code ProgramService::CreateProgram( size_t IN binSize,
                                   const void* IN bin,
                                   cl_dev_binary_prop IN prop,
                                   cl_dev_program* OUT prog
                                   )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CreateProgram enter"));

    // Input parameters validation
    if(0 == binSize || nullptr == bin)
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid binSize or bin parameters"));
        return CL_DEV_INVALID_VALUE;
    }

    if ( nullptr == prog )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid prog parameter"));
        return CL_DEV_INVALID_VALUE;
    }

    // If the origin of binary is user loaded
    // check for rightness
    if(prop == CL_DEV_BINARY_USER)
    {
        cl_dev_err_code rc = CheckProgramBinary(binSize, bin);
        if ( CL_DEV_FAILED(rc) )
        {
            CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Check program binary failed"));
            return rc;
        }
    }

    // Create new program
    TProgramEntry*  pEntry      = new TProgramEntry;
    if ( nullptr == pEntry )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Cann't allocate program entry"));
        return CL_DEV_OUT_OF_MEMORY;
    }

	pEntry->programType = PTCompiledProgram;
    pEntry->pProgram = nullptr;
    pEntry->clBuildStatus = CL_BUILD_NONE;

    cl_dev_err_code ret;
    assert(m_pBackendCompiler);
    ret = m_pBackendCompiler->CreateProgram(bin, binSize, &pEntry->pProgram);

    if ( CL_DEV_FAILED(ret) )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to create program from given container<%0X>"), ret);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

    *prog = (cl_dev_program)pEntry;

    return CL_DEV_SUCCESS;
}


 cl_dev_err_code ProgramService::CreateBuiltInKernelProgram( const char* IN szBuiltInNames,
										cl_dev_program* OUT prog
                                       )
 {
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateBuiltInKernelProgram enter"));

	ICLDevBackendProgram_* pProg;
	cl_dev_err_code err = BuiltInKernelRegistry::GetInstance()->CreateBuiltInProgram(szBuiltInNames, &pProg);
	if ( CL_DEV_FAILED(err) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("CreateBuiltInProgram failed with %x"), err);
		return err;
	}

    TProgramEntry*  pEntry = new TProgramEntry;
    if ( nullptr == pEntry )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to allocate new handle"));
        return CL_DEV_OUT_OF_MEMORY;
    }

	pEntry->programType = PTBuiltInProgram;
    pEntry->pProgram = pProg;
	pEntry->clBuildStatus = CL_BUILD_SUCCESS;
	assert(nullptr!=prog&&"prog expected to be valid pointer");
	*prog = (cl_dev_program)pEntry;
	return CL_DEV_SUCCESS;
}

/*******************************************************************************************************************
clDevBuildProgram
    Description
        Builds (compiles & links) a program executable from the program intermediate or binary.
    Input
        bin_size                        Size of the binary buffer
        bin                             A pointer to binary buffer that holds program container defined by cl_prog_container.
        options                         A pointer to a string that describes the build options to be used for building the program executable.
                                        The list of supported options is described in section 5.4.3 in OCL spec. document.
        user_data                       This value will be passed as an argument when clDevBuildFinished is called. Can be NULL.
        prop                            Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_BUILD_OPTIONS    If build options for back-end compiler specified by options are invalid.
        CL_DEV_INVALID_BINARY           If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/

cl_dev_err_code ProgramService::BuildProgram( cl_dev_program OUT prog,
                                    const char* IN options,
                                    cl_build_status* OUT buildStatus
                                   )
{
    const char *p = nullptr;

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("BuildProgram enter"));

    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);

    // Program already built?
    if (CL_BUILD_SUCCESS == pEntry->clBuildStatus)
    {
        return CL_DEV_BUILD_ALREADY_COMPLETE;
    }

    // Trying two simultaneous builds of the same program?
    if ( CL_BUILD_IN_PROGRESS == pEntry->clBuildStatus )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid build status(%d), should be CL_BUILD_NONE(%d)"),
            pEntry->clBuildStatus, CL_BUILD_NONE);
        //Asserting because framework should have stopped it
        assert(0);
        return CL_DEV_BUILD_IN_PROGRESS;
    }

    if ( CL_BUILD_NONE != pEntry->clBuildStatus )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid build status(%d), should be CL_BUILD_NONE(%d)"),
            pEntry->clBuildStatus, CL_BUILD_NONE);
        return CL_DEV_INVALID_OPERATION;
    }

    pEntry->clBuildStatus = CL_BUILD_IN_PROGRESS;

    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Starting build"));

    cl_dev_err_code ret = m_pBackendCompiler->BuildProgram(pEntry->pProgram, nullptr, options);

    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Build Done (%d)"), ret);

    cl_build_status status = CL_DEV_SUCCEEDED(ret) ? CL_BUILD_SUCCESS : CL_BUILD_ERROR;
    pEntry->clBuildStatus = status;

    // if the user requested -dump-opt-asm, emit the asm of this module into a file
    if( CL_DEV_SUCCEEDED(ret) && (nullptr != options) && ('\0' != *options) &&
        (nullptr != (p = strstr(options, "-dump-opt-asm="))))
    {
        assert( pEntry->pProgram && "Program must be created already");
        ProgramDumpConfig dumpOptions(p);
        m_pBackendCompiler->DumpJITCodeContainer( pEntry->pProgram->GetProgramIRCodeContainer(),
            dumpOptions.GetStringValue(CL_DEV_BACKEND_OPTION_DUMPFILE,""));
    }

    // if the user requested -dump-opt-llvm, print the IR of this module
    if( CL_DEV_SUCCEEDED(ret) && (nullptr != options) && ('\0' != *options) &&
        (nullptr != (p = strstr(options, "-dump-opt-llvm="))))
    {
        assert( pEntry->pProgram && "Program must be created already");
        ProgramDumpConfig dumpOptions(p);
        m_pBackendCompiler->DumpCodeContainer( pEntry->pProgram->GetProgramIRCodeContainer(), &dumpOptions);
    }

    if ( nullptr != buildStatus )
    {
        *buildStatus = status;
    }

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Exit"));
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetFunctionPointerFor(cl_dev_program IN prog,
    const char* IN func_name, cl_ulong* OUT func_pointer_ret) const
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"),
        TEXT("GetFunctionPointerFor enter"));

    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);

    // Program already built?
    if (CL_BUILD_SUCCESS != pEntry->clBuildStatus)
    {
        return CL_DEV_INVALID_PROGRAM_EXECUTABLE;
    }

    assert(func_pointer_ret && "func_pointer ret is nullptr");
    *func_pointer_ret = pEntry->pProgram->GetFunctionPointerFor(func_name);

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Exit"));
    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
clDevReleaseProgram
    Description
        Deletes previously created program object and releases all related resources.
    Input
        prog                            A handle to program object to be deleted
    Output
        NONE
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_PROGRAM          Invalid program object was specified.
********************************************************************************************************************/
cl_dev_err_code ProgramService::ReleaseProgram( cl_dev_program IN prog )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ReleaseProgram enter"));

    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);

    DeleteProgramEntry(pEntry);

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
clDevUnloadCompiler
    Description
        Allows the framework to release the resources allocated by the back-end compiler.
        This is a hint from the framework and does not guarantee that the compiler will not be used in the future
        or that the compiler will actually be unloaded by the device.
    Input
        NONE
    Output
        NONE
    Returns
        CL_DEV_SUCCESS  The function is executed successfully.
********************************************************************************************************************/
cl_dev_err_code ProgramService::UnloadCompiler()
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("UnloadCompiler enter"));
    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
clDevGetProgramBinary
    Description
        Returns the compiled program binary. The output buffer contains program container as defined cl_prog_binary_desc.
    Input
        prog                    A handle to created program object.
        size                    Size in bytes of the buffer passed to the function.
    Output
        binary                  A pointer to buffer wherein program binary will be stored.
        size_ret                The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL, returns size in bytes of a program binary. If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_PROGRAM  If program is not valid program object.
        CL_DEV_INVALID_VALUE    If size is not enough to store the binary or binary is NULL and size is not 0.
********************************************************************************************************************/
cl_dev_err_code ProgramService::GetProgramBinary( cl_dev_program IN prog,
                                        size_t IN size,
                                        void* OUT binary,
                                        size_t* OUT sizeRet
                                        )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetProgramBinary enter"));

    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);
    ICLDevBackendProgram_ *pProg = pEntry->pProgram;

    const ICLDevBackendCodeContainer* pCodeContainer = pProg->GetProgramCodeContainer();
    if ( nullptr == pCodeContainer )
    {
        return CL_DEV_INVALID_VALUE;
    }

    size_t stSize = pCodeContainer->GetCodeSize();
    if ( nullptr != sizeRet )
    {
        *sizeRet = stSize;
    }

    if ( (0 == size) && (nullptr == binary) )
    {
        return CL_DEV_SUCCESS;
    }

    if ( (nullptr == binary) || (size < stSize) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    MEMCPY_S( (char*)binary, size, pCodeContainer->GetCode(), stSize);
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetBuildLog( cl_dev_program IN prog,
                                  size_t IN size,
                                  char* OUT log,
                                  size_t* OUT sizeRet
                                  )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetBuildLog enter"));

    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);
    ICLDevBackendProgram_ *pProg = pEntry->pProgram;

    const char* pLog = pProg->GetBuildLog();

    size_t  stLogSize = strlen(pLog) + 1;

    if ( (0 == size) && (nullptr == log) )
    {
        if ( nullptr == sizeRet )
        {
            return CL_DEV_INVALID_VALUE;
        }
        *sizeRet = stLogSize;
        return CL_DEV_SUCCESS;
    }

    if ( (nullptr == log) || (size < stLogSize) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    MEMCPY_S( log, size, pLog, stLogSize);

    if ( nullptr != sizeRet )
    {
        *sizeRet = stLogSize;
    }
    return CL_DEV_SUCCESS;
}
/************************************************************************************************************
    clDevGetSupportedBinaries (optional)
    Description
        Returns the list of supported binaries.
    Input
        count                   Size of the buffer passed to the function in terms of cl_prog_binary_desc.
    Output
        types                   A pointer to buffer wherein binary types will be stored.
        count_ret               The actual size ofthe buffer returned by the function in terms of cl_prog_binary_desc.
                                When count is equal to 0 and types is NULL, function returns a size of the list.
                                If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_PROGRAM  If program is not valid program object.
        CL_DEV_INVALID_VALUE    If count is not enough to store the binary or types is NULL and count is not 0.
***************************************************************************************************************/
cl_dev_err_code ProgramService::GetSupportedBinaries( size_t IN size,
                                       cl_prog_binary_desc* OUT types,
                                       size_t* OUT sizeRet
                                       )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetSupportedBinaries enter"));
    if ( nullptr != sizeRet )
    {
        // TODO: Create supported list
        *sizeRet = sizeof(gSupportedBinTypes);
    }

    if ( (0 == size) && (nullptr == types) )
    {
        return CL_DEV_SUCCESS;
    }

    if( (nullptr == types) || (size < sizeof(gSupportedBinTypes)))
    {
        return CL_DEV_INVALID_VALUE;
    }

    //currently support only user binaries
    MEMCPY_S(types, size, gSupportedBinTypes, sizeof(gSupportedBinTypes));

    return CL_DEV_SUCCESS;
}


cl_dev_err_code ProgramService::GetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetKernelId enter"));

    if ( (nullptr == name) || (nullptr == kernelId) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    // Find if ID is already allocated
    TName2IdMap::const_iterator nameIt;
    pEntry->muMap.Lock();
    nameIt = pEntry->mapKernels.find(name);
    if ( pEntry->mapKernels.end() != nameIt )
    {
        *kernelId = (cl_dev_kernel)&nameIt->second;
        pEntry->muMap.Unlock();
        return CL_DEV_SUCCESS;
    }

    // Retrieve kernel from program
    cl_dev_err_code iRet;
    const ICLDevBackendKernel_* pKernel;

    iRet = pEntry->pProgram->GetKernelByName(name, &pKernel);
    if ( CL_DEV_FAILED(iRet) )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Requested kernel not found<%0X>"), iRet);
        pEntry->muMap.Unlock();
        return iRet;
    }

    // Add to program entry map
    KernelMapEntry mapEntry;
    mapEntry.pBEKernel = pKernel;
#ifdef USE_ITT
    mapEntry.ittTaskNameHandle = __itt_string_handle_create(name);
#endif
    pEntry->mapKernels[name] = mapEntry;
    pEntry->muMap.Unlock();

    *kernelId = (cl_dev_kernel)&(pEntry->mapKernels[name]);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetProgramKernels enter"));

    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    unsigned int const uiAllProgKernels = pEntry->pProgram->GetKernelsCount();
    unsigned int const uiNonBlockProgKernels = pEntry->pProgram->GetNonBlockKernelsCount();
    cl_dev_err_code         iRet;

    // Check input parameters
    if ( (0==num_kernels) && (nullptr==kernels) )
    {
        if ( nullptr == numKernelsRet )
        {
            return CL_DEV_INVALID_VALUE;
        }

        *numKernelsRet = uiNonBlockProgKernels;
        return CL_DEV_SUCCESS;
    }

    if ( (nullptr==kernels) || (num_kernels < uiNonBlockProgKernels) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    OclAutoMutex mu(&pEntry->muMap);
    // Retrieve kernels from program and store internally
    for(unsigned int i=0, ret_kernel_idx=0; i<uiAllProgKernels; ++i)
    {
        const ICLDevBackendKernel_* pKernel;

        iRet = pEntry->pProgram->GetKernel(i, &pKernel);
        if ( CL_DEV_FAILED(iRet) )
        {
            CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to retrieve kernels<%X>"), iRet);
            return iRet;
        }
        // Check if kernel handle exists
        const char* szKernelName = pKernel->GetKernelName();
        TName2IdMap::const_iterator nameIt;
        nameIt = pEntry->mapKernels.find(szKernelName);

        // New Kernel ID is required?
        if ( pEntry->mapKernels.end() == nameIt )
        {
            // Add to program entry map
            KernelMapEntry mapEntry;
            mapEntry.pBEKernel = pKernel;
#ifdef USE_ITT
            mapEntry.ittTaskNameHandle = __itt_string_handle_create(szKernelName);
#endif
            // Add new ID to program's Name2ID map
            pEntry->mapKernels[szKernelName] = mapEntry;
        }
        // Skip block kernels
        if( pKernel->GetKernelProporties()->IsBlock() )
        {
          continue;
        }

        kernels[ret_kernel_idx++] = (cl_dev_kernel)&(pEntry->mapKernels[szKernelName]);
    }

    if ( nullptr != numKernelsRet )
    {
        *numKernelsRet = uiNonBlockProgKernels;
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetKernelInfo(cl_dev_kernel      IN  kernel,
                                              cl_dev_kernel_info IN  param,
                                              size_t             IN  input_value_size,
                                              const void*        IN  input_value,
                                              size_t             IN  value_size,
                                              void*              OUT value,
                                              size_t*            OUT valueSizeRet) const
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetKernelInfo enter"));
    const KernelMapEntry* pKernelEntry = (const KernelMapEntry*)kernel;
    const ICLDevBackendKernel_* pKernel = pKernelEntry->pBEKernel;
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

    // Set value parameters
    cl_uint stValSize = 0;
    std::vector<size_t> vValues(1,0);
    unsigned long long ullValue = 0;
    const void*        pValue   = &ullValue;
    cl_dev_dispatch_buffer_prop dispatchProperties;

    switch (param)
    {
    case CL_DEV_KERNEL_NAME:
        pValue = pKernel->GetKernelName();
        assert(strlen((const char*)pValue) + 1 <= UINT_MAX);//MAXUINT32);
        stValSize = (cl_uint)strlen((const char*)pValue)+1;
        break;

    case CL_DEV_KERNEL_PROTOTYPE:
        stValSize = pKernel->GetKernelParamsCount();
        pValue = (void*)pKernel->GetKernelParams();
        stValSize *= sizeof(cl_kernel_argument);
        break;

    case CL_DEV_KERNEL_WG_SIZE_REQUIRED:
        pValue = pKernelProps->GetRequiredWorkGroupSize();
        stValSize = sizeof(size_t) * MAX_WORK_DIM;
        break;

    case CL_DEV_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT:
        *(cl_bool*)pValue = (cl_bool)pKernelProps->IsNonUniformWGSizeSupported();
        stValSize = sizeof(cl_bool);
        break;

    case CL_DEV_KERNEL_MAX_WG_SIZE:
        // TODO: Current implementation uses constants and it's OK with allocated on the stack dynamic local buffers.
        //       But take it into account if the available stack frame size is known at RT. I.e.:
        //          GetMaxWorkGroupSize(CPU_MAX_WORK_GROUP_SIZE, stackFrameSize - CPU_DEV_LCL_MEM_SIZE);
        {
            size_t maxPrivateMemSize =
              (m_pCPUConfig->GetForcedPrivateMemSize() > 0)
              ? m_pCPUConfig->GetForcedPrivateMemSize()
              : CPU_DEV_MAX_WG_PRIVATE_SIZE;
            if (FPGA_EMU_DEVICE == m_pCPUConfig->GetDeviceMode())
            {
                ullValue = pKernelProps->GetMaxWorkGroupSize(
                    FPGA_MAX_WORK_GROUP_SIZE, maxPrivateMemSize);
            }
            else
            {
                ullValue = pKernelProps->GetMaxWorkGroupSize(
                    CPU_MAX_WORK_GROUP_SIZE, maxPrivateMemSize);
            }
            stValSize = sizeof(size_t);
        }
        break;

    case CL_DEV_KERNEL_WG_SIZE:
        ullValue = pKernelProps->GetKernelPackCount();
        stValSize = sizeof(size_t);
        break;

    case CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE:
        ullValue = pKernelProps->GetImplicitLocalMemoryBufferSize();
        stValSize = sizeof(cl_ulong);
        break;

    case CL_DEV_KERNEL_PRIVATE_SIZE:
        ullValue = pKernelProps->GetPrivateMemorySize();
        stValSize = sizeof(cl_ulong);
        break;

    case CL_DEV_KERNEL_ARG_INFO:
        stValSize = pKernel->GetKernelParamsCount() * sizeof(cl_kernel_argument_info);
        pValue = (void*)pKernel->GetKernelArgInfo();
        break;

    case CL_DEV_KERNEL_MEMORY_OBJECT_INDEXES:
        stValSize = pKernel->GetMemoryObjectArgumentCount() * sizeof(unsigned int);
        pValue = (void*)pKernel->GetMemoryObjectArgumentIndexes();
        break;

    case CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES:
        stValSize = sizeof(cl_dev_dispatch_buffer_prop);
        dispatchProperties.size = pKernel->GetExplicitArgumentBufferSize()+sizeof(cl_uniform_kernel_args);
        dispatchProperties.argumentOffset = 0;
        dispatchProperties.alignment = 64; // Currently ask for 64 byte aligment. Next BE will provide maximum aligment for the buffer.
        pValue = &dispatchProperties;
        break;

    case CL_DEV_KERNEL_ATTRIBUTES:
        pValue = pKernel->GetKernelProporties()->GetKernelAttributes();
        stValSize = (cl_uint)strlen((const char*)pValue)+1;
        break;
    case CL_DEV_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE:
    {
        const size_t* pInVal = (const size_t*)input_value;
        const size_t  dim    = input_value_size/sizeof(size_t);
        ullValue = pKernelProps->GetMaxSubGroupSize(dim, pInVal);
        stValSize = sizeof(size_t);
        break;
    }
    case CL_DEV_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE:
    {
        const size_t* pInVal = (const size_t*)input_value;
        const size_t dim = input_value_size/sizeof(size_t);
        ullValue = pKernelProps->GetNumberOfSubGroups(dim, pInVal);
        stValSize = sizeof(size_t);
        break;
    }
    case CL_DEV_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT:
    {
        const size_t dim = value_size/sizeof(size_t);
        if(nullptr != input_value &&
           nullptr != value &&
           dim > 0)
        {
            const size_t desiredSGCount = *(const size_t*)input_value;
            vValues.resize(dim, 0);
            pValue = &vValues[0];
            size_t maxPrivateMemSize =
              (m_pCPUConfig->GetForcedPrivateMemSize() > 0)
              ? m_pCPUConfig->GetForcedPrivateMemSize()
              : CPU_DEV_MAX_WG_PRIVATE_SIZE;
            if (FPGA_EMU_DEVICE == m_pCPUConfig->GetDeviceMode())
            {
                pKernelProps->GetLocalSizeForSubGroupCount(
                    desiredSGCount,
                    FPGA_MAX_WORK_GROUP_SIZE,
                    maxPrivateMemSize,
                    &vValues[0],
                    dim);
            }
            else
            {
                pKernelProps->GetLocalSizeForSubGroupCount(
                    desiredSGCount,
                    CPU_MAX_WORK_GROUP_SIZE,
                    maxPrivateMemSize,
                    &vValues[0],
                    dim);
            }
        }
        stValSize = (nullptr != value && nullptr != input_value)? sizeof(size_t) * dim: 0;
        break;
    }
    case CL_DEV_KERNEL_MAX_NUM_SUB_GROUPS:
    {
        if (FPGA_EMU_DEVICE == m_pCPUConfig->GetDeviceMode())
        {
            ullValue = pKernelProps->GetMaxNumSubGroups(FPGA_MAX_WORK_GROUP_SIZE);
        }
        else
        {
            ullValue = pKernelProps->GetMaxNumSubGroups(CPU_MAX_WORK_GROUP_SIZE);
        }
        stValSize = sizeof(size_t);
        break;
    }
    case CL_DEV_KERNEL_COMPILE_NUM_SUB_GROUPS:
    {
        ullValue = pKernelProps->GetRequiredNumSubGroups();
        stValSize = sizeof(size_t);
        break;
    }
    case CL_DEV_KERNEL_IS_AUTORUN:
    {
        *(cl_bool*)pValue = (cl_bool)pKernelProps->IsAutorun();
        stValSize = sizeof(cl_bool);
        break;
    }
    case CL_DEV_KERNEL_IS_TASK:
    {
        *(cl_bool*)pValue = (cl_bool)pKernelProps->IsTask();
        stValSize = sizeof(cl_bool);
        break;
    }
    case CL_DEV_KERNEL_CAN_USE_GLOBAL_WORK_OFFSET:
    {
        *(cl_bool*)pValue = (cl_bool)pKernelProps->CanUseGlobalWorkOffset();
        stValSize = sizeof(cl_bool);
        break;
    }
    case CL_DEV_KERNEL_SPILL_MEM_SIZE_INTEL:
    {
        // Despite we can obtain real value, we always return 0 for now
        *(cl_ulong*)pValue = 0UL;
        stValSize = sizeof(cl_ulong);
        break;
    }
    case CL_DEV_KERNEL_COMPILE_SUB_GROUP_SIZE_INTEL:
    {
        *(size_t*)pValue = pKernelProps->GetRequiredSubGroupSize();
        stValSize = sizeof(size_t);
        break;
    }

    default:
        return CL_DEV_INVALID_VALUE;
    }

    if (nullptr != value && value_size < stValSize)
    {
        return CL_DEV_INVALID_VALUE;
    }

    if ( valueSizeRet )
    {
        *valueSizeRet = stValSize;
    }

    if ( nullptr != value )
    {
        if ( nullptr != pValue )
        {
            MEMCPY_S(value, value_size, pValue, stValSize);
        } else {
            memset(value, 0, stValSize);
        }
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetGlobalVariableTotalSize( cl_dev_program IN prog, size_t* OUT size) const
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetGlobalVariableTotalSize enter"));

    // Return error if program was not built yet.
    TProgramEntry* pEntry = reinterpret_cast<TProgramEntry*>(prog);
    if( nullptr == pEntry )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, "Requested program not found (%0X)", (size_t)prog);
        return CL_DEV_INVALID_PROGRAM;
    }

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    // Just return what back-end gives us.
    ICLDevBackendProgram_ *pProgram = pEntry->pProgram;
    *size = pProgram->GetGlobalVariableTotalSize();
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
    //image_type describes the image type and must be either an image object
	if(imageType == CL_MEM_OBJECT_BUFFER || imageType == CL_MEM_OBJECT_PIPE)
    {
        return CL_DEV_INVALID_VALUE;
    }

    if(0 == numEntries && nullptr != formats)
    {
        return CL_DEV_INVALID_VALUE;
    }

    unsigned int uiNumEntries;
    const cl_image_format* supportedImageFormats = m_pBackendImageService->GetSupportedImageFormats(&uiNumEntries, imageType, flags);

	if(nullptr != formats)
    {
		uiNumEntries = min(uiNumEntries, numEntries);
        MEMCPY_S(formats, numEntries * sizeof(cl_image_format), supportedImageFormats, uiNumEntries * sizeof(cl_image_format));
    }
    if(nullptr != numEntriesRet)
    {
        *numEntriesRet = uiNumEntries;
    }

    return CL_DEV_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////
//  Private methods
void ProgramService::DeleteProgramEntry(TProgramEntry* pEntry)
{
    // Finally release the object
    assert(m_pBackendCompiler);

#ifdef __INCLUDE_MKL__
	if ( PTBuiltInProgram == pEntry->programType )
	{
		BuiltInProgram* pBIProgram = static_cast<BuiltInProgram*>(pEntry->pProgram);
		delete pBIProgram;
	}
	else
#endif
	{
		m_pBackendCompiler->ReleaseProgram(pEntry->pProgram);
	}
    pEntry->mapKernels.clear();
    delete pEntry;
}
