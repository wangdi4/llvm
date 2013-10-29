// Copyright (c) 2006-2008 Intel Corporation
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

///////////////////////////////////////////////////////////
//  ProgramService.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "program_service.h"
#include "program_config.h"
#include "cpu_logger.h"
#include "backend_wrapper.h"
#include "builtin_kernels.h"

#include <cpu_dev_limits.h>
#include <cl_synch_objects.h>

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

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
    m_pCallBacks(devCallbacks), m_pBackendFactory(pBackendFactory), m_pBackendCompiler(NULL),
    m_pBackendExecutor(NULL), m_pBackendImageService(NULL), m_pCPUConfig(config)
{
    assert(m_pBackendFactory && "getting backend factory assumed to allways succeed if initialization has succeeded");

    if ( NULL != logDesc )
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
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CPUDevice: Program Service - Distructed"));

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
cl_dev_err_code ProgramService::Init(IDeviceCommandManager* pDeviceCommandManager)
{
	ProgramConfig programConfig(pDeviceCommandManager);
    programConfig.InitFromCpuConfig(*m_pCPUConfig);   
    
    ICLDevBackendCompilationService* pCompiler = NULL;
    cl_dev_err_code ret = m_pBackendFactory->GetCompilationService(&programConfig, &pCompiler);
    if( CL_DEV_FAILED(ret) )
    {
        return ret;
    }

    ICLDevBackendImageService* pImageService = NULL;
    ret = m_pBackendFactory->GetImageService(&programConfig, &pImageService);
    if( CL_DEV_FAILED(ret) )
    {
        pCompiler->Release();
        return ret;
    }

    ICLDevBackendExecutionService* pExecutor = NULL;
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
    const cl_prog_container_header* pProgCont = (cl_prog_container_header*)bin;

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CheckProgramBinary enter"));

    // Check container size
    if ( sizeof(cl_prog_container_header) > binSize )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid Binary Size was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check container mask
    if ( memcmp(_CL_CONTAINER_MASK_, pProgCont->mask, sizeof(pProgCont->mask)) )
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid Container Mask was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported container type
    switch ( pProgCont->container_type )
    {
    // Supported containers
    case CL_PROG_CNT_PRIVATE:
        break;

    default:
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid Container Type was provided"));
        return CL_DEV_INVALID_BINARY;
    }

    // Check supported binary types
    switch ( pProgCont->description.bin_type )
    {
    // Supported program binaries
    case CL_PROG_BIN_EXECUTABLE_LLVM:          // The container should contain valid LLVM-IR
        break;

    case CL_PROG_OBJ_X86:           // The container should contain binary buffer of object file
    case CL_PROG_DLL_X86:           // The container should contain a full path name to DLL file to load
    default:
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid Container Type was provided<%0X>"), pProgCont->description.bin_type);
        return CL_DEV_INVALID_BINARY;
    }

    return CL_DEV_SUCCESS;
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
    if(0 == binSize || NULL == bin)
    {
        CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid binSize or bin parameters"));
        return CL_DEV_INVALID_VALUE;
    }

    if ( NULL == prog )
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
    const cl_prog_container_header* pProgCont = (cl_prog_container_header*)bin;
    TProgramEntry*  pEntry      = new TProgramEntry;
    if ( NULL == pEntry )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Cann't allocate program entry"));
        return CL_DEV_OUT_OF_MEMORY;
    }

	pEntry->programType = PTCompiledProgram;
    pEntry->pProgram = NULL;
    pEntry->clBuildStatus = CL_BUILD_NONE;

    cl_dev_err_code ret;
    switch(pProgCont->description.bin_type)
    {
    case CL_PROG_BIN_EXECUTABLE_LLVM:
        assert(m_pBackendCompiler);
        ret = m_pBackendCompiler->CreateProgram(pProgCont, &pEntry->pProgram);
        break;
    default:
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to find approproiate program for type<%d>"), pProgCont->description.bin_type);
        delete pEntry;
        return CL_DEV_INVALID_BINARY;
    }

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
    if ( NULL == pEntry )
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to allocate new handle"));
        return CL_DEV_OUT_OF_MEMORY;
    }

	pEntry->programType = PTBuiltInProgram;
    pEntry->pProgram = pProg;
	pEntry->clBuildStatus = CL_BUILD_SUCCESS;
	assert(NULL!=prog&&"prog expected to be valid pointer");
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
    const char *p = NULL;

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("BuildProgram enter"));

    TProgramEntry* pEntry = (TProgramEntry*)prog;

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

    cl_dev_err_code ret = m_pBackendCompiler->BuildProgram(pEntry->pProgram, NULL);

    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Build Done (%d)"), ret);

    cl_build_status status = CL_DEV_SUCCEEDED(ret) ? CL_BUILD_SUCCESS : CL_BUILD_ERROR;
    pEntry->clBuildStatus = status;
    
    // if the user requested -dump-opt-asm, emit the asm of this module into a file
    if( CL_DEV_SUCCEEDED(ret) && (NULL != options) && ('\0' != *options) &&
        (NULL != (p = strstr(options, "-dump-opt-asm="))))
    {
        assert( pEntry->pProgram && "Program must be created already");
        ProgramDumpConfig dumpOptions(p);
        m_pBackendCompiler->DumpJITCodeContainer( pEntry->pProgram->GetProgramCodeContainer(),
            dumpOptions.GetStringValue(CL_DEV_BACKEND_OPTION_DUMPFILE,""));
    }

    // if the user requested -dump-opt-llvm, print the IR of this module
    if( CL_DEV_SUCCEEDED(ret) && (NULL != options) && ('\0' != *options) &&
        (NULL != (p = strstr(options, "-dump-opt-llvm="))))
    {
        assert( pEntry->pProgram && "Program must be created already");
        ProgramDumpConfig dumpOptions(p);
        m_pBackendCompiler->DumpCodeContainer( pEntry->pProgram->GetProgramCodeContainer(), &dumpOptions);
    }

    if ( NULL != buildStatus )
    {
        *buildStatus = status;
    }

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

    TProgramEntry* pEntry = (TProgramEntry*)prog;

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

    TProgramEntry* pEntry = (TProgramEntry*)prog;
    ICLDevBackendProgram_ *pProg = pEntry->pProgram;

    const ICLDevBackendCodeContainer* pCodeContainer = pProg->GetProgramCodeContainer();
    if ( NULL == pCodeContainer )
    {
        return CL_DEV_INVALID_VALUE;
    }

    size_t stSize = pCodeContainer->GetCodeSize();
    if ( NULL != sizeRet )
    {
        *sizeRet = stSize;
    }

    if ( (0 == size) && (NULL == binary) )
    {
        return CL_DEV_SUCCESS;
    }

    if ( (NULL == binary) || (size < stSize) )
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

    TProgramEntry* pEntry = (TProgramEntry*)prog;
    ICLDevBackendProgram_ *pProg = pEntry->pProgram;

    const char* pLog = pProg->GetBuildLog();

    size_t  stLogSize = strlen(pLog) + 1;

    if ( (0 == size) && (NULL == log) )
    {
        if ( NULL == sizeRet )
        {
            return CL_DEV_INVALID_VALUE;
        }
        *sizeRet = stLogSize;
        return CL_DEV_SUCCESS;
    }

    if ( (NULL == log) || (size < stLogSize) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    MEMCPY_S( log, size, pLog, stLogSize);    
    
    if ( NULL != sizeRet )
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
    if ( NULL != sizeRet )
    {
        // TODO: Create supported list
        *sizeRet = sizeof(gSupportedBinTypes);
    }

    if ( (0 == size) && (NULL == types) )
    {
        return CL_DEV_SUCCESS;
    }

    if( (NULL == types) || (size < sizeof(gSupportedBinTypes)))
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

    if ( (NULL == name) || (NULL == kernelId) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    TProgramEntry* pEntry = (TProgramEntry*)prog;

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

    TProgramEntry* pEntry = (TProgramEntry*)prog;

    if ( pEntry->clBuildStatus != CL_BUILD_SUCCESS )
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    unsigned int    uiNumProgKernels = pEntry->pProgram->GetKernelsCount();
    cl_dev_err_code         iRet;

    // Check input parameters
    if ( (0==num_kernels) && (NULL==kernels) )
    {
        if ( NULL == numKernelsRet )
        {
            return CL_DEV_INVALID_VALUE;
        }

        *numKernelsRet = uiNumProgKernels;
        return CL_DEV_SUCCESS;
    }

    if ( (NULL==kernels) || (num_kernels < uiNumProgKernels) )
    {
        return CL_DEV_INVALID_VALUE;
    }

    OclAutoMutex mu(&pEntry->muMap);
    // Retrieve kernels from program and store internally
    for(unsigned int i=0; i<uiNumProgKernels; ++i)
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

        kernels[i] = (cl_dev_kernel)&(pEntry->mapKernels[szKernelName]);
    }

    if ( NULL != numKernelsRet )
    {
        *numKernelsRet = uiNumProgKernels;
    }


    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::GetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
                    void* OUT value, size_t* OUT valueSizeRet ) const
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetKernelInfo enter"));
    const KernelMapEntry* pKernelEntry = (const KernelMapEntry*)kernel;
    const ICLDevBackendKernel_* pKernel = pKernelEntry->pBEKernel;
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();

    // Set value parameters
    cl_uint stValSize = 0;
    unsigned long long ullValue = 0;
    const void* pValue;
    pValue = &ullValue;

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
        stValSize = sizeof(size_t)*MAX_WORK_DIM;
        break;

    case CL_DEV_KERNEL_MAX_WG_SIZE:
        {
            size_t private_mem_size = pKernelProps->GetPrivateMemorySize();
            ullValue = MIN(CPU_MAX_WORK_GROUP_SIZE, (CPU_DEV_MAX_WG_PRIVATE_SIZE /( (private_mem_size > 0) ? private_mem_size : 1)));
            size_t packSize = pKernelProps->GetMinGroupSizeFactorial();
            if (ullValue > packSize)
                ullValue = ( ullValue ) & ~(packSize-1);
        }
        stValSize = sizeof(size_t);
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

    case CL_DEV_KENREL_ARGUMENT_BUFFER_SIZE:
        stValSize = sizeof(size_t);
        ullValue = pKernel->GetArgumentBufferSize();
        pValue = &ullValue;
        break;

    default:
        return CL_DEV_INVALID_VALUE;
    }

    if (NULL != value && value_size < stValSize)
    {
        return CL_DEV_INVALID_VALUE;
    }

    if ( valueSizeRet )
    {
        *valueSizeRet = stValSize;
    }

    if ( NULL != value )
    {
        if ( NULL != pValue )
        {
            MEMCPY_S(value, value_size, pValue, stValSize);
        } else {
           memset(value, 0, stValSize);
    	  }
    }

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

    if(0 == numEntries && NULL != formats)
    {
        return CL_DEV_INVALID_VALUE;
    }

    unsigned int uiNumEntries;
    const cl_image_format* supportedImageFormats = m_pBackendImageService->GetSupportedImageFormats(&uiNumEntries, imageType, flags);

	if(NULL != formats)
    {
		uiNumEntries = min(uiNumEntries, numEntries);
        MEMCPY_S(formats, numEntries * sizeof(cl_image_format), supportedImageFormats, uiNumEntries * sizeof(cl_image_format));
    }
    if(NULL != numEntriesRet)
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

