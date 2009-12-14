// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING ANY WAY OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  lrb_program_service.cpp
///////////////////////////////////////////////////////////
#include "lrb_program_service.h"
#include "lrb_xn_wrapper.h"
#include "lrb_agent_common.h"
#include <cl_table.h>
#include <cl_device_api.h>

using namespace Intel::OpenCL::LRBAgent;
using namespace Intel::OpenCL::Utils;
using namespace std;

/************************************************************************
 * Define a fixed argument list for prototype
 * 
 ************************************************************************/
cl_int sLrbKernelFuncArgsSize = 3;
cl_kernel_argument sLrbKernelFuncArgList[] =
{
    { CL_KRNL_ARG_PTR_GLOBAL, 0 },
    { CL_KRNL_ARG_PTR_GLOBAL, 0 },
    { CL_KRNL_ARG_PTR_GLOBAL, 0 }
};


/************************************************************************
 * 
 ************************************************************************/
LrbProgramService::LrbProgramService( XNWrapper* pXnWrapper, fn_clDevBuildStatusUpdate* pclDevBuildStatusUpdate ):
m_pXnWrapper(pXnWrapper),
m_pclDevBuildStatusUpdate(pclDevBuildStatusUpdate)
{
    m_pPrograms = new Intel::OpenCL::Utils::ClTable;
}

/************************************************************************
 * 
 ************************************************************************/
LrbProgramService::~LrbProgramService()
{
    delete m_pPrograms;
    m_pPrograms = NULL;
}

/************************************************************************
 * This function creates a program entry for the new program and load the program
 * source to the device.
 ************************************************************************/
cl_int LrbProgramService::CreateProgram(size_t binSize, const void* bin, cl_dev_binary_prop prop, cl_dev_program* prog)
{
    cl_int result = CL_SUCCESS;
    // TODO: Handle different cl_dev_binary_prop. Currently bin is a string with the full library name
    sProgramEntry* pNewProg = new sProgramEntry;
    memset(pNewProg, 0, sizeof(sProgramEntry));
    result = m_pXnWrapper->LoadProgram(binSize, bin, &(pNewProg->ulProgramBinHndl), &(pNewProg->ulBuildOutputHndl));
    if( CL_SUCCESS == result)
    {
        unsigned long ulProgId = m_pPrograms->Insert(pNewProg);        
        *prog = (cl_dev_program)ulProgId;
    }
    return result;
}
/************************************************************************
 * 
 ************************************************************************/
cl_int LrbProgramService::GetBuildLog(cl_dev_program prog, size_t size, char* log, size_t* size_ret)
{
    assert(0 && "GetBuildLog is not implemented yet");
    return CL_DEV_ERROR_FAIL;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbProgramService::GetSupportedBinaries ( cl_uint count, cl_prog_binary_desc* types, size_t* sizeRet )
{
    assert(0 && "GetSupportedBinaries is not implemented yet");
    return CL_DEV_ERROR_FAIL;
}

/************************************************************************
 * Returns an internal identifier of the kernel according to its name
 * If the program wasn't build nothing will e returned
 ************************************************************************/
cl_int LrbProgramService::GetKernelId(cl_dev_program prog, const char* name, cl_dev_kernel* kernelId)
{
    
    sProgramEntry* pProgEntry = (sProgramEntry*)m_pPrograms->Get((unsigned long)prog);
    if( NULL == pProgEntry)
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    cl_int result = CL_INVALID_KERNEL_NAME;

    //
    // Find the kernelId
    //
    size_t szStrSize = strlen(name);
    cl_uint uiNumKernels = pProgEntry->uiNumberOfKernels;
    for (cl_uint ui=0; ui < uiNumKernels; ui++ )
    {
        if( 0 == strncmp(name,pProgEntry->kernels[ui].kernelName, szStrSize))
        {
            *kernelId = (cl_dev_kernel)(pProgEntry->kernels[ui].uiKernelHndl);
            result = CL_SUCCESS;
            break;
        }
    }
    return result;
}

/************************************************************************
 * Returns list of all kernels in program object
 ************************************************************************/
cl_int LrbProgramService::GetProgramKernels(cl_dev_program clProg, cl_uint uiNumKernels, cl_dev_kernel* pKernels, cl_uint* puiNumKernelsRet)
{
    sProgramEntry* pProgEntry = (sProgramEntry*)m_pPrograms->Get((unsigned long)clProg);
    if( NULL == pProgEntry)
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    //
    // Define how many kernels to return
    //
    cl_uint uiKernelsCount = (( uiNumKernels < pProgEntry->uiNumberOfKernels) ? uiNumKernels :  pProgEntry->uiNumberOfKernels);

    //
    // Copy all kernels
    //
    for (cl_uint ui=0; ui < uiKernelsCount; ui++ )
    {
        pKernels[ui] = (cl_dev_kernel)(pProgEntry->kernels[ui].uiKernelHndl);
    }
    return CL_SUCCESS;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbProgramService::GetKernelInfo(cl_dev_kernel clKernel, cl_dev_kernel_info clParam, size_t szValueSize, void* pValue, size_t* szValueSizeRet )
{
    size_t  szInternalReturnedValueSize = szValueSize;
    size_t  *pszInternalRetunedValueSize;
    sKernelInfo* pKernelInfo = m_pKernels[clKernel];

    //if both pParamVal and paramValSize is NULL return error
    if(NULL == pValue && NULL == szValueSizeRet)
    {
        return CL_DEV_INVALID_VALUE;
    }
    //if szValueSizeRet is NULL it should be ignored
    if(szValueSizeRet)
    {
        pszInternalRetunedValueSize = szValueSizeRet;
    }
    else
    {
        pszInternalRetunedValueSize = &szInternalReturnedValueSize;
    }

    switch(clParam)
    {
    case CL_DEV_KERNEL_NAME:
        *pszInternalRetunedValueSize = strlen((const char*)pKernelInfo->kernelName)+1;
        if(pValue)
        {
            if (*pszInternalRetunedValueSize > szValueSize) return CL_DEV_INVALID_VALUE;
            memcpy(pValue, pKernelInfo->kernelName, *pszInternalRetunedValueSize);            
        }
        break;
    case CL_DEV_KERNEL_PROTOTYPE:
        {
        size_t szNumParameters = 0;
        cl_kernel_argument* pArgs;
        GetKernelParams(clKernel, &pArgs, &szNumParameters);
        *pszInternalRetunedValueSize =  szNumParameters*sizeof(cl_kernel_argument);
        if(pValue)
        {
            if (*pszInternalRetunedValueSize > szValueSize) return CL_DEV_INVALID_VALUE;
            memcpy(pValue, pArgs, *pszInternalRetunedValueSize);            
        }
        break;
        }
    case CL_DEV_KERNEL_WG_SIZE:
        // Currently set to 1
        *pszInternalRetunedValueSize = sizeof(cl_uint);
        if(pValue)
        {
            if (*pszInternalRetunedValueSize > szValueSize) return CL_DEV_INVALID_VALUE;
            *(cl_uint*)pValue = 1;            
        }
        break;
    case CL_DEV_KERNEL_WG_SIZE_REQUIRED:
        *pszInternalRetunedValueSize = sizeof(size_t)*3; // dim size 3
        if(pValue)
        {
            if (*pszInternalRetunedValueSize > szValueSize) return CL_DEV_INVALID_VALUE;
            // Currently this feature is not supported, therefore 0,0,0 is returned
            ((size_t*)pValue)[0] = 0;
            ((size_t*)pValue)[1] = 0;
            ((size_t*)pValue)[2] = 0;
        }
        break;
    case CL_DEV_KERNEL_WG_SIZE_HINT:
    case CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE:
    default:
        assert( 0 && "This kernel info is not supported");
        return CL_DEV_INVALID_VALUE;
    }

    return CL_DEV_SUCCESS;
}


/************************************************************************
 * Returns the arguments of the Kernel as a list of cl_kernel_arg_type 
 * according to the order of appearance in the prototype of the kernel
 * ppArgsList can be NULL
 ************************************************************************/
cl_int LrbProgramService::GetKernelParams( cl_dev_kernel kernel, cl_kernel_argument** ppArgsList, size_t* pArgsListSize )
{
    // Currently all kernels are with prototype of 3 global buffers: CL_KRNL_ARG_PTR_GLOBAL
    // TODO: adjust this function to use each kernel prototype.
    if( NULL != ppArgsList )
    {
        *ppArgsList = sLrbKernelFuncArgList;
    }
    *pArgsListSize = sLrbKernelFuncArgsSize;
    return CL_SUCCESS;
}

/************************************************************************
 * Build program is an async
 *
 ************************************************************************/
cl_int LrbProgramService::BuildProgram(cl_dev_program clProg, const cl_char* pOptions, void* pUserData)
{
    sProgramEntry* pProgEntry = (sProgramEntry*)m_pPrograms->Get((unsigned long)clProg);
    if( NULL == pProgEntry)
    {
        return CL_DEV_INVALID_PROGRAM;
    }
    pProgEntry->pUserData = pUserData;
    cl_int result = m_pXnWrapper->BuildProgram(pProgEntry->ulProgramBinHndl, pProgEntry->ulBuildOutputHndl, (uint32_t)clProg, pOptions);
    return result;
}

/************************************************************************
 * Called when build is done
 * The service upload the build data to the host.
 * And notify the framework on done
 *
 * TODO: Make different path for build from source/Front End binaries
 *       in that case, the build output will be available on host and there is
 *       no need for upload
 *
 *
 ************************************************************************/
cl_int LrbProgramService::NotifyBuildDone( unsigned long progId)
{
    sProgramEntry* pProgEntry = (sProgramEntry*)m_pPrograms->Get(progId);
    if( NULL == pProgEntry)
    {
        return CL_DEV_INVALID_PROGRAM;
    }

    void* pBuildOutput = NULL;    
    // 1. Get buffer data
    m_pXnWrapper->MapBuffer((void*)pProgEntry->ulBuildOutputHndl, CL_DEV_MEM_READ, &pBuildOutput);
    // 2. Parse buffer and set program entry
    ParseBuildOutputBuffer(pProgEntry, pBuildOutput);
    // 3. Release buffers
    m_pXnWrapper->UnmapBuffer((void*)pProgEntry->ulBuildOutputHndl);
    m_pXnWrapper->DeleteBuffer((void*)pProgEntry->ulBuildOutputHndl);
    pProgEntry->ulBuildOutputHndl = 0;
    m_pXnWrapper->DeleteBuffer((void*)pProgEntry->ulProgramBinHndl);
    pProgEntry->ulProgramBinHndl = 0;
    // 4. Notify framework on build done
    m_pclDevBuildStatusUpdate((cl_dev_program)progId, pProgEntry->pUserData, CL_BUILD_SUCCESS);

    return CL_SUCCESS;    
}

/************************************************************************
 * This function fills the program entry with the data available in 
 * the pBuildOutputData
 * The syntax of the Build output data is:
 *  ----------------------------------------------------------------------------------------
 *  | uint32 BackendProgAddr | uint32 numOfKernels | sKernelInfo K1 | ... | sKernelInfo Kn |
 *  ----------------------------------------------------------------------------------------
 *
 *  Where sKernelInfo: 
 * The kernelNameLen and the kernelName include the NULL terminated char
 *  ----------------------------------------------------------------------
 *  | uint32 BackendkernelAddr | uint32 kernelNameLen | char* kernelName |
 *  ----------------------------------------------------------------------
 * 
 ************************************************************************/
void LrbProgramService::ParseBuildOutputBuffer( sProgramEntry* pProgEntry, void* pBuildOutputData)
{
    int8_t* parserPos = (int8_t*)pBuildOutputData;
    size_t szSizeOfUint32 = sizeof(uint32_t);
    uint32_t uiKernelNameLen = 0;

    // Get program address
    memcpy(&(pProgEntry->uiBackendProgAddr), parserPos, szSizeOfUint32);
    parserPos += szSizeOfUint32;
    // Get number of kernels
    memcpy(&(pProgEntry->uiNumberOfKernels), parserPos, szSizeOfUint32);
    parserPos += szSizeOfUint32;

    // Create kernels
    pProgEntry->kernels = new sKernelInfo[pProgEntry->uiNumberOfKernels];
    for( cl_uint ui=0; ui<pProgEntry->uiNumberOfKernels; ui++)
    {  
        sKernelInfo* pCurrKernel = &(pProgEntry->kernels[ui]);
        memcpy (&(pCurrKernel->uiKernelHndl), parserPos, szSizeOfUint32);
        parserPos += szSizeOfUint32;
        memcpy (&uiKernelNameLen, parserPos, szSizeOfUint32);
        parserPos += szSizeOfUint32;
        pCurrKernel->kernelName = new char[uiKernelNameLen];
        memcpy (pCurrKernel->kernelName, parserPos, uiKernelNameLen);
        parserPos += uiKernelNameLen;
        
        // Add new kernel object to list
        m_pKernels[(cl_dev_kernel)(pCurrKernel->uiKernelHndl)] = pCurrKernel;
    }
}

/************************************************************************
 * On release program both the native and the program entry should be freed.
 ************************************************************************/
cl_int LrbProgramService::ReleaseProgram(cl_dev_program prog)
{
    sProgramEntry* pProgEntry = (sProgramEntry*)m_pPrograms->Get((unsigned long)prog);
    

    assert(0 && "ReleaseProgram is not implemented yet");
    return CL_DEV_ERROR_FAIL;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbProgramService::UnloadCompiler()
{
    assert(0 && "UnloadCompiler is not implemented yet");
    return CL_DEV_ERROR_FAIL;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbProgramService::GetProgramBinary(cl_dev_program prog, size_t size, void* binary, size_t* sizeRet)
{
    assert(0 && "GetProgramBinary is not implemented yet");
    return CL_DEV_ERROR_FAIL;
}
