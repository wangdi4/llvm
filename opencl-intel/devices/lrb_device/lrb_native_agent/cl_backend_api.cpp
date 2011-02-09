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
//  cl_backend_api.cpp
///////////////////////////////////////////////////////////
#include "cl_backend_api.h"
#include "lrb_dynamic_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_FILE_NAME_SIZE (256)

using namespace Intel::OpenCL::LRBAgent;

/************************************************************************
 * Define a fixed argument list for prototype
 * 
 ************************************************************************/
cl_int sLrbNativeKernelFuncArgsSize = 3;
cl_kernel_argument sLrbNativeKernelFuncArgList[] =
{
    { CL_KRNL_ARG_PTR_GLOBAL, 0 },
    { CL_KRNL_ARG_PTR_GLOBAL, 0 },
    { CL_KRNL_ARG_PTR_GLOBAL, 0 }
};

/************************************************************************
 * 
 ************************************************************************/
cl_int CLBackendProgram::CreateProgram(const cl_prog_container* pContainer, CLBackendProgram**  ppProgram )
{
    //
    // Load dll 
    // For the prototype the bin is a path to the library of kernels.
    // TODO: for HW/XenSim execution need to search for *.so lib and use all FreeBSD equivalents,
    //   
    char* libName = (char*)pContainer->container;
    void* hLib =  lrb_dynmic_load_library(libName);
    if (NULL == hLib)
    {
        return CL_DEV_INVALID_VALUE;
    }

    CLBackendProgram* pNewProgram = new CLBackendProgram; 
    pNewProgram->m_hLib = hLib;
    //
    // Trim file name
    //
    const char* pTrimlibName = lrb_dynmic_trim_file_path(libName);
    pNewProgram->m_pLibName = (char*)malloc(strlen(pTrimlibName)+1);
    strcpy(pNewProgram->m_pLibName, pTrimlibName);

    *ppProgram = pNewProgram;
    return 0;
}

/************************************************************************
 * 
 ************************************************************************/
CLBackendProgram::CLBackendProgram():
    m_hLib(NULL),
    m_kernelsTable(NULL)
{
}

/************************************************************************
 * 
 ************************************************************************/
CLBackendProgram::~CLBackendProgram()
{
    if ( NULL != m_kernelsTable)
    {
        for (int i=0; i< m_iNumKernels; i++)
        {
            delete m_kernelsTable[i];
        }
        free(m_kernelsTable);
    }
}

/************************************************************************
 * 
 ************************************************************************/
const cl_prog_container* CLBackendProgram::GetContainer( const cl_prog_binary_desc* pDescriptor  )
{
    assert( 0 && "GetContainer is not implemented yet");
    return NULL;
}

/************************************************************************
 * This object build the program and fill kernels objects
 * TODO: This prototype use loaded binary only with predefined prototype,
 *       for full implementation this should be changed.
 ************************************************************************/
cl_int CLBackendProgram::BuildProgram( const char *pOptions )
{
    // Get number of kernels
    char pVarName[MAX_FILE_NAME_SIZE];
    sprintf(pVarName, "%s_prototypes_count", m_pLibName);
    void* varAddr = lrb_dynmic_get_variable(m_hLib, pVarName);
    if ( NULL == varAddr)
    {
        return CL_DEV_INVALID_PROGRAM;
    }
    m_iNumKernels = *((int*)varAddr);

    // Get list of kernels
    sprintf(pVarName, "%s_prototypes", m_pLibName);
    varAddr = lrb_dynmic_get_variable(m_hLib, pVarName);
    if ( NULL == varAddr)
    {
        return CL_DEV_INVALID_PROGRAM;
    }
    char** kernelsNames = (char**)varAddr;

    //
    // Create kernels
    //
    m_kernelsTable = (CLBackendKernel**) malloc(sizeof(CLBackendKernel*)*m_iNumKernels);
    for( int i=0; i<m_iNumKernels; i++)
    {
        fn_lrbKernelFunction* pFunc = (fn_lrbKernelFunction*)lrb_dynmic_get_function(m_hLib, kernelsNames[i]);
        if( NULL == pFunc ) 
        {
            // error
            return CL_DEV_INVALID_PROGRAM;
        }
        m_kernelsTable[i] = new CLBackendKernel(kernelsNames[i], pFunc);
    }
    return CL_SUCCESS;
}

/************************************************************************
 * This function writes program build data inside pOutputBuffer.
 * If uiBufferLen < kernels data than error is returned
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
cl_int CLBackendProgram::GenerateOutputData(void* pOutputBuffer, uint32_t uiBufferLen)
{
    int8_t* pCurrentWritePtr = (int8_t*)pOutputBuffer;
    int8_t* pBufferEnd = pCurrentWritePtr+uiBufferLen;
    size_t szUint32 = sizeof(uint32_t);

    // Check buffer overrun before starting to write kernels name.
    int8_t* pExpectedPos = pCurrentWritePtr+3*szUint32;
    if(  pExpectedPos >= pBufferEnd )
    {
        return CL_DEV_INVALID_VALUE;
    }

    uint32_t thisAddr = (uint32_t)this;
        
    // 1. Write program address
    memcpy(pCurrentWritePtr, &thisAddr, szUint32);
    pCurrentWritePtr+=szUint32;

    // 2. Write Num of kernels
    memcpy(pCurrentWritePtr, &m_iNumKernels, szUint32);
    pCurrentWritePtr+=szUint32;

    // 3. Write kernels
    for ( int i=0; i<m_iNumKernels; i++)
    {
        // Kernel addr.
        memcpy(pCurrentWritePtr, &(m_kernelsTable[i]), szUint32);
        pCurrentWritePtr+=szUint32;

        uint32_t len = strlen(m_kernelsTable[i]->GetKernelName()) + 1;

        // Check length
        pExpectedPos = pCurrentWritePtr+len+2*szUint32;
        if(  pExpectedPos >= pBufferEnd )
        {
            return CL_DEV_INVALID_VALUE;
        }

        // Kernel name length.
        memcpy(pCurrentWritePtr, &len, szUint32);
        pCurrentWritePtr+=szUint32;

        // Kernel name
        memcpy(pCurrentWritePtr, m_kernelsTable[i]->GetKernelName(), len);
        pCurrentWritePtr+=len;
    }
    return CL_DEV_SUCCESS;
}


/************************************************************************
 * 
 ************************************************************************/
cl_int CLBackendProgram::GetBuildLog(size_t *pSize, char* pLog) const
{
    assert( 0 && "GetBuildLog is not supported yet");
    return CL_SUCCESS;

}

/************************************************************************
 * 
 ************************************************************************/
cl_int CLBackendProgram::GetKernel(const char* pKernelName, const CLBackendKernel** pKernel)
{
    for( int i=0; i<m_iNumKernels; i++)
    {
        if (!(strcmp(pKernelName, m_kernelsTable[i]->GetKernelName())))
        {
            // found a match, return
            *pKernel = m_kernelsTable[i];
            return CL_DEV_SUCCESS;
        }
    }
    return CL_DEV_INVALID_VALUE;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int CLBackendProgram::GetAllKernels(CLBackendKernel*** pKernels, cl_uint*  puiRetCount)
{
    *pKernels = m_kernelsTable;
    *puiRetCount = (cl_uint)m_iNumKernels;
    return CL_SUCCESS;
}


/************************************************************************
 * 
 ************************************************************************/
CLBackendKernel::CLBackendKernel( char* pKernelName, fn_lrbKernelFunction* pfnLrbKernelFunction ):
    m_pfnLrbKernelFunction(pfnLrbKernelFunction)
{
    m_pKernelName = (char*)malloc(sizeof(char)*(strlen(pKernelName)+1));
    sprintf(m_pKernelName, pKernelName);    
}

/************************************************************************
 * 
 ************************************************************************/
CLBackendKernel::~CLBackendKernel()
{
    if ( NULL != m_pfnLrbKernelFunction ) free(m_pfnLrbKernelFunction);
}

/************************************************************************
 * 
 ************************************************************************/
cl_int CLBackendKernel::CreateExecutable(
                        void*                 pArgsBuffer,
                        size_t                szBufferSize,
                        cl_uint               uiWorkDimension,
                        const size_t*         pGlobalWorkSize,
                        const size_t*         pLocalWorkSize,
                        CLBackendExecutable** ppExecutable
                        )
{
    *ppExecutable = new CLBackendExecutable(this, pArgsBuffer, szBufferSize, uiWorkDimension, pGlobalWorkSize, pLocalWorkSize);
    return CL_SUCCESS;
}


/************************************************************************
 * 
 ************************************************************************/
cl_int CLBackendKernel::GetKernelParams( void* pArgsBuffer, size_t* pBufferSize )
{
    // Currently all kernels are with prototype of 3 global buffers: CL_KRNL_ARG_PTR_GLOBAL
    // TODO: adjust this function to use each kernel prototype.
    if( NULL != pArgsBuffer )
    {
        memcpy(pArgsBuffer, sLrbNativeKernelFuncArgList, sizeof(sLrbNativeKernelFuncArgList));
    }
    *pBufferSize = sLrbNativeKernelFuncArgsSize;
    return CL_SUCCESS;
}

/************************************************************************
 * 
 ************************************************************************/
CLBackendExecutable::CLBackendExecutable(
    CLBackendKernel* pKernel, 
    void*            pArgsBuffer, 
    size_t           szBufferSize, 
    cl_uint          uiWorkDimension, 
    const size_t*    pszGlobalWorkSize, 
    const size_t*    pszLocalWorkSize
    ):
    m_pKernel(pKernel),
    m_uiWorkDimension(uiWorkDimension)
{
    // Set work group sizes
    for(int i=0; i<3; i++)
    {
        m_pszGlobalWorkSize[i] = pszGlobalWorkSize[i];
        m_pszLocalWorkSize[i]  = pszLocalWorkSize[i];
    }
    // This executable supports only a fix type of kernel with the prototype:
    // void (fn_lrbKernelFunction)( const float* , const float* , float* c, int tid);
    uint32_t* pBuffersAddr = (uint32_t*)pArgsBuffer;
    m_pSrcBuffer1 = (void*)pBuffersAddr[0];
    m_pSrcBuffer2 = (void*)pBuffersAddr[1];
    m_pDstBuffer  = (void*)pBuffersAddr[2];
}

/************************************************************************
 * 
 ************************************************************************/
cl_uint CLBackendExecutable::Execute(
    void*         pLocalMemoryBuffers, 
    const size_t* pBufferCount, 
    const size_t* pGlobalId, 
    const size_t* pLocalId, 
    const size_t* pItemsToProcess 
    )
{
    // This executable supports only a fix type of kernel with the prototype:
    // void (fn_lrbKernelFunction)( const float*, const float*, float* c, int tid);
    // TODO: Set it to dynamic type of executables.
    m_pKernel->m_pfnLrbKernelFunction( (const float*)m_pSrcBuffer1, (const float*)m_pSrcBuffer2, (float*)m_pDstBuffer, pGlobalId[0]);
    return CL_SUCCESS;
}
