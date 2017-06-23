// Copyright (c) 2006-2012 Intel Corporation
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
//  kernel.cpp
//  Implementation of the Kernel class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "kernel.h"
#include "Context.h"
#include "program.h"
#include "fe_compiler.h"
#include "common_clang.h"
#include "sampler.h"
#include "cl_shared_ptr.hpp"
#include "svm_buffer.h"
#include "Context.h"
#include "context_module.h"
#include "framework_proxy.h"

#include <cl_sys_defines.h>
#include <cl_objects_map.h>
#include <cl_local_array.h>
#include <Device.h>
#include <assert.h>
#include <cl_utils.h>


using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::DeviceKernel(Kernel*                             pKernel, 
                           const SharedPtr<FissionableDevice>& pDevice,
                           cl_dev_program  devProgramId,
                           LoggerClient *  pLoggerClient,
                           cl_err_code *   pErr) :
                OCLObjectBase("DeviceKernel"), m_clDevKernel(CL_INVALID_HANDLE), m_pKernel(pKernel), m_pDevice(pDevice),
                m_CL_KERNEL_WORK_GROUP_SIZE(0), m_CL_KERNEL_LOCAL_MEM_SIZE(0),
                m_CL_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT(false)
{
    assert ( pErr != NULL && "Error argument always must be provided");

    *pErr = CL_SUCCESS;
    SET_LOGGER_CLIENT(pLoggerClient);
    LOG_DEBUG(TEXT("%s"), TEXT("DeviceKernel C'tor enter"));

    if (NULL == m_pKernel || NULL == m_pDevice || CL_INVALID_HANDLE == devProgramId)
    {
        LOG_ERROR(TEXT("%s"), TEXT("NULL == m_pKernel || NULL == m_pDevice || CL_INVALID_HANDLE == devProgramId"));
        *pErr = CL_INVALID_VALUE;
        return;
    }

    const char* pKernelName = pKernel->GetName();
    // get kernel id
    cl_dev_err_code clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelId(devProgramId, pKernelName, &m_clDevKernel);
    if (CL_DEV_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("Device->GetKernelId failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
        *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        return;
    }

    m_sKernelPrototype.m_szKernelName = pKernelName;

    // get kernel prototype
    size_t argsSize = 0;
    clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, 0, NULL, 0, NULL, &argsSize);
    if (CL_DEV_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("Device->clDevGetKernelInfo failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
        *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        return;
    }

    // Get kernel attributes
    size_t attrSize;
    clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_ATTRIBUTES, 0, NULL, 0, NULL, &attrSize);
    if (CL_DEV_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("Device->clDevGetKernelInfo failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
        *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        return;
    }

    char *kernelAttrs = new char[attrSize];
    if (NULL == kernelAttrs)
    {
        LOG_ERROR(TEXT("Device->clDevGetKernelInfo failed kernel<%s>, ERR=%d"), pKernelName, CL_DEV_OUT_OF_MEMORY);
        *pErr = CL_OUT_OF_HOST_MEMORY;
        return;
    }

    clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_ATTRIBUTES, 0, NULL, attrSize, kernelAttrs, NULL);
    if (CL_DEV_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("Device->clDevGetKernelInfo failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
        *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        delete [] kernelAttrs;
        return;
    }

    m_sKernelPrototype.m_szKernelAttributes = kernelAttrs;
    delete [] kernelAttrs;

    // Get argument buffer size
    clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES, 0, NULL,
                                                                sizeof(m_sKernelPrototype.m_dispatchBufferProperties), &m_sKernelPrototype.m_dispatchBufferProperties, NULL);
    if (CL_DEV_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("Device->clDevGetKernelInfo(CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES) failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
        *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        return;
    }

    size_t argsCount = argsSize / sizeof(cl_kernel_argument);
    assert(argsCount  <= CL_MAX_UINT32 && "Number or arguments is to high");
    if ( argsCount > 0)
    {
        m_sKernelPrototype.m_vArguments.resize(argsCount);
        clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, 0, NULL,
                                                                    argsCount*sizeof(cl_kernel_argument), &(m_sKernelPrototype.m_vArguments[0]), NULL);
        if (CL_DEV_FAILED(clErrRet))
        {
            LOG_ERROR(TEXT("Device->clDevGetKernelInfo failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
            *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
            return;
        }

        // Get memory object arguments
        clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_MEMORY_OBJECT_INDEXES, 0, NULL, 0, NULL, &argsSize);
        if (CL_DEV_FAILED(clErrRet))
        {
            LOG_ERROR(TEXT("Device->clDevGetKernelInfo failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
            *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
            return;
        }

        if ( argsSize > 0 )
        {
            argsCount = argsSize / sizeof(unsigned int);
            assert(argsCount  <= CL_MAX_UINT32 && "Number or arguments is to high");
            m_sKernelPrototype.m_MemArgumentsIndx.resize(argsCount);
            clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_MEMORY_OBJECT_INDEXES, 0, NULL,
                                                                argsCount*sizeof(unsigned int), &(m_sKernelPrototype.m_MemArgumentsIndx[0]), NULL);
            if (CL_DEV_FAILED(clErrRet))
            {
                LOG_ERROR(TEXT("Device->clDevGetKernelInfo failed kernel<%s>, ERR=%d"), pKernelName, clErrRet);
                *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
                return;
            }
        }
    }

    // we are here - all passed ok    
    if (!CacheRequiredInfo())
    {
        *pErr =  CL_INVALID_DEVICE;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::~DeviceKernel()
{
    LOG_DEBUG(TEXT("%s"), TEXT("DeviceKernel D'tor enter"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeviceKernel::CacheRequiredInfo()
{
    cl_dev_err_code err;
    IOCLDeviceAgent* pDevAgent = m_pDevice->GetDeviceAgent();
    assert( NULL != pDevAgent );

    err = pDevAgent->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_MAX_WG_SIZE, 0, NULL, sizeof(m_CL_KERNEL_WORK_GROUP_SIZE), &m_CL_KERNEL_WORK_GROUP_SIZE, NULL);
    assert( CL_DEV_SUCCEEDED(err) && "caching of GetKernelWorkGroupSize failed" );

    if (CL_DEV_SUCCEEDED(err))
    {
        err = pDevAgent->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_WG_SIZE_REQUIRED, 0, NULL, sizeof(m_CL_KERNEL_COMPILE_WORK_GROUP_SIZE), &m_CL_KERNEL_COMPILE_WORK_GROUP_SIZE, NULL);
        assert( CL_DEV_SUCCEEDED(err) && "caching of GetKernelCompileWorkGroupSize failed" );
    }
    
    if (CL_DEV_SUCCEEDED(err))
    {
        err = pDevAgent->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE, 0, NULL, sizeof(m_CL_KERNEL_LOCAL_MEM_SIZE), &m_CL_KERNEL_LOCAL_MEM_SIZE, NULL);
        assert( CL_DEV_SUCCEEDED(err) && "caching of GetKernelLocalMemSize failed" );
    }

    if (CL_DEV_SUCCEEDED(err))
    {
        err = pDevAgent->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT, 0, NULL, sizeof(m_CL_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT), &m_CL_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT, NULL);
        assert( CL_DEV_SUCCEEDED(err) && "caching of GetKernelNonUniformWGSizeSupport failed" );
    }

    return CL_DEV_SUCCEEDED(err);        
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeviceKernel::CheckKernelDefinition(const DeviceKernel * pKernel) const
{
    //cl_start;

    if (NULL == pKernel)
    {
        return false;
    }
    const SKernelPrototype& sKernelPrototype = pKernel->GetPrototype();
    if ( m_sKernelPrototype.m_szKernelName.compare(sKernelPrototype.m_szKernelName) )
    {
        return false;
    }
    if (sKernelPrototype.m_vArguments.size() != m_sKernelPrototype.m_vArguments.size())
    {
        return false;
    }
    for (size_t ui=0; ui<m_sKernelPrototype.m_vArguments.size(); ++ui)
    {
        if ((sKernelPrototype.m_vArguments[ui].type != m_sKernelPrototype.m_vArguments[ui].type) ||
            (sKernelPrototype.m_vArguments[ui].size_in_bytes != m_sKernelPrototype.m_vArguments[ui].size_in_bytes))
        {
            return false;
        }
    }
    // kernel prototypes are identical

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  KernelArg class
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void KernelArg::Init( char* baseAddress, const cl_kernel_argument& clKernelArgType )
{
    m_pValueLocation        = baseAddress + clKernelArgType.offset_in_bytes;
    m_pSvmPtrArg            = NULL;
    m_bValid                = false;
    m_clKernelArgType       = clKernelArgType;

    // Correct complex sizes
    const cl_uint size = m_clKernelArgType.size_in_bytes;
    assert( 0 != size && "argument size can't be 0");
    const cl_uint mswSize = (size >> 16) & 0xFFFF;
    if ( 0 != mswSize )
    {
        m_clKernelArgType.size_in_bytes = mswSize * (size & 0xFFFF);
    }
}

void KernelArg::SetValuePlaceHolder( void * pValuePlaceHolder, size_t offset )
{
    assert( NULL != pValuePlaceHolder && "Invalid placeholder was provided" );
    m_pValueLocation    = pValuePlaceHolder;

    if (CL_KRNL_ARG_PTR_LOCAL == m_clKernelArgType.type)
    {
        // Initial local pointer sizes with 0
        *(size_t*)pValuePlaceHolder = 0;
    }    
}

void KernelArg::GetValue( size_t size, void* pValue ) const
{
    assert( NULL!=m_pValueLocation && NULL!=pValue && "Value location is not set or invalid return adderss");
    assert( size == GetSize() && "Wrong param size provided" );

    switch (size)
    {
        case sizeof(cl_uint):
            *((cl_uint*)pValue) = *((cl_uint*)m_pValueLocation);
            break;

        case sizeof(cl_long):
            *((cl_long*)pValue) = *((cl_long*)m_pValueLocation);
            break;

        default:
            MEMCPY_S( pValue, size, m_pValueLocation, GetSize() );
    }
}

void KernelArg::SetValue( size_t size, const void * pValue )
{
    assert( m_pValueLocation && "Value location is not set");

    if (IsLocalPtr())
    {
        *(size_t*)m_pValueLocation = size;
    }
    else
    {
        assert( size == GetSize() && "Wrong param size provided" );
        if (NULL != pValue)
        {
            switch (size)
            {
                case sizeof(cl_uint):
                    *((cl_uint*)m_pValueLocation) = *((cl_uint*)pValue);
                    break;

                case sizeof(cl_long):
                    *((cl_long*)m_pValueLocation) = *((cl_long*)pValue);
                    break;

                default:
                    MEMCPY_S( m_pValueLocation, GetSize(), pValue, size );
            }
        }
        else
        {
            switch (size)
            {
                case sizeof(cl_uint):
                    *((cl_uint*)m_pValueLocation) = 0;
                    break;

                case sizeof(cl_long):
                    *((cl_long*)m_pValueLocation) = 0;
                    break;

                default:
                    memset( m_pValueLocation, 0, GetSize() );
            }
        }
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::Kernel(const SharedPtr<Program>& pProgram, const char * psKernelName, size_t szNumDevices) :
OCLObject<_cl_kernel_int>(pProgram->GetParentHandle(), "Kernel"),
    m_pProgram(pProgram), m_szAssociatedDevices(szNumDevices), m_pArgsBlob(NULL), m_numValidArgs(0), 
    m_totalLocalSize(0), m_bSvmFineGrainSystem(false)
{
    m_sKernelPrototype.m_szKernelName = psKernelName;

    if (NULL != pProgram)
    {
        m_pContext = pProgram->GetContext();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::~Kernel()
{
    LOG_DEBUG(TEXT("%s"), TEXT("Kernel D'tor enter"));

    m_sKernelPrototype.m_vArguments.clear();

    // clear device kernels
    for (size_t i = 0; i < m_vpDeviceKernels.size(); ++i)
    {
        delete m_vpDeviceKernels[i];
    }
    m_vpDeviceKernels.clear();

    m_vecArgs.clear();

    if (m_pArgsBlob)
    {
        delete[] m_pArgsBlob;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
    LOG_DEBUG(TEXT("Enter Kernel::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
        iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    size_t szParamSize = 0;
    const void * pValue = NULL;
    cl_ulong iParam = 0;
    switch (iParamName)
    {
    case CL_KERNEL_FUNCTION_NAME:
        szParamSize = m_sKernelPrototype.m_szKernelName.length() + 1;
        pValue = m_sKernelPrototype.m_szKernelName.c_str();
        break;
    case CL_KERNEL_NUM_ARGS:
        szParamSize = sizeof(cl_uint);
        iParam = m_sKernelPrototype.m_vArguments.size();
        pValue = &(iParam);
        break;
    case CL_KERNEL_REFERENCE_COUNT:
        szParamSize = sizeof(cl_uint);
        pValue = &m_uiRefCount;
        break;
    case CL_KERNEL_CONTEXT:
        if (NULL != m_pProgram && NULL != m_pContext)
        {
            szParamSize = sizeof(cl_context);
            iParam = (cl_long)m_pContext->GetHandle();
            pValue = &iParam;
        }
        break;
    case CL_KERNEL_PROGRAM:
        if (NULL != m_pProgram)
        {
            szParamSize = sizeof(cl_program);
            iParam = (cl_ulong)m_pProgram->GetHandle();
            pValue = &iParam;
        }
        break;
    case CL_KERNEL_ATTRIBUTES:
        szParamSize = m_sKernelPrototype.m_szKernelAttributes.length() + 1;
        pValue = m_sKernelPrototype.m_szKernelAttributes.c_str();
        break;
    default:
        return CL_INVALID_VALUE;
        break;
    }

    if (NULL != pParamValue && szParamValueSize < szParamSize)
    {
        return CL_INVALID_VALUE;
    }

    if (NULL != pszParamValueSizeRet)
    {
        *pszParamValueSizeRet = szParamSize;
    }

    if (NULL != pParamValue && szParamSize > 0)
    {
        MEMCPY_S(pParamValue, szParamValueSize, pValue, szParamSize);
    }

    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetSubGroupInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::GetSubGroupInfo(const       SharedPtr<FissionableDevice>& device,
                                    cl_int      iParamName,
                                    size_t      szParamValueSize,
                                    size_t      input_value_size,
                                    const void* input_value,
                                    void *      pParamValue,
                                    size_t*     pszParamValueSizeRet)
{
    LOG_DEBUG(TEXT("Enter Kernel::GetSubGroupInfo (pDevice=%p, iParamName=%d,\
                    szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
                    device.GetPtr(), iParamName, szParamValueSize,
                    pParamValue, pszParamValueSizeRet);
    assert("No context assigned to the kernel" && (NULL != m_pProgram) && (NULL != m_pContext));

    FissionableDevice* pDevice = device.GetPtr();
    // check input parameters
    if ( (NULL == pDevice) && (m_szAssociatedDevices > 1) )
    {
        return CL_INVALID_DEVICE;
    }

    if ( NULL == pDevice )
    {
        pDevice = m_vpDeviceKernels[0]->GetDevice().GetPtr();
    }
    assert(NULL!=pDevice && "Device can't be detected");

    cl_dev_kernel clDevKernel = GetDeviceKernelId(pDevice);
    if ( CL_INVALID_HANDLE == clDevKernel )
    {
        return CL_INVALID_KERNEL;
    }

    cl_err_code clErr = CL_SUCCESS;
    switch (iParamName)
    {
    case CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel,
                                                              CL_DEV_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                                              input_value_size,
                                                              input_value,
                                                              szParamValueSize,
                                                              pParamValue,
                                                              pszParamValueSizeRet);
        break;
    case CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE:
        if(0 == input_value_size || nullptr == input_value)
        {
            return CL_INVALID_VALUE;
        }
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel,
                                                              CL_DEV_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                                              input_value_size,
                                                              input_value,
                                                              szParamValueSize,
                                                              pParamValue,
                                                              pszParamValueSizeRet);
        break;
    case CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel,
                                                              CL_DEV_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
                                                              input_value_size,
                                                              input_value,
                                                              szParamValueSize,
                                                              pParamValue,
                                                              pszParamValueSizeRet);
        break;
    case CL_KERNEL_MAX_NUM_SUB_GROUPS:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel,
                                                              CL_DEV_KERNEL_MAX_NUM_SUB_GROUPS,
                                                              0,
                                                              NULL,
                                                              szParamValueSize,
                                                              pParamValue,
                                                              pszParamValueSizeRet);
        break;

    case CL_KERNEL_COMPILE_NUM_SUB_GROUPS:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel,
                                                              CL_DEV_KERNEL_COMPILE_NUM_SUB_GROUPS,
                                                              0,
                                                              NULL,
                                                              szParamValueSize,
                                                              pParamValue,
                                                              pszParamValueSizeRet);
        break;
    default:
        clErr = CL_INVALID_VALUE;
    }

    if( (signed)CL_DEV_INVALID_KERNEL == clErr )
    {
        clErr = CL_INVALID_KERNEL;
    }
    if( (signed)CL_DEV_INVALID_VALUE == clErr )
    {
        clErr = CL_INVALID_VALUE;
    }

    return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetWorkGroupInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code    Kernel::GetWorkGroupInfo(const SharedPtr<FissionableDevice>& device, cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
    LOG_DEBUG(TEXT("Enter Kernel::GetWorkGroupInfo (pDevice=%p, iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
        device.GetPtr(), iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

#ifdef _DEBUG
    assert ( "No context assigned to the kernel" && (NULL != m_pProgram) && (NULL != m_pContext) );
#endif

    FissionableDevice* pDevice = device.GetPtr();
    // check input parameters
    if ( (NULL == pDevice) && (m_szAssociatedDevices > 1) )
    {
        return CL_INVALID_DEVICE;
    }

    if ( NULL == pDevice )
    {
        pDevice = m_vpDeviceKernels[0]->GetDevice().GetPtr();
    }
    assert(NULL!=pDevice && "Device can't be detected");

    cl_dev_kernel clDevKernel = GetDeviceKernelId(pDevice);
    if ( CL_INVALID_HANDLE == clDevKernel )
    {
        return CL_INVALID_KERNEL;
    }

    cl_err_code clErr = CL_SUCCESS;
    switch (iParamName)
    {
    case CL_KERNEL_WORK_GROUP_SIZE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_MAX_WG_SIZE, 0, NULL, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE_REQUIRED, 0, NULL, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_LOCAL_MEM_SIZE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE, 0, NULL, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE, 0, NULL, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_PRIVATE_MEM_SIZE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_PRIVATE_SIZE, 0, NULL, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;

    default:
        clErr = CL_INVALID_VALUE;
    }

    if( (signed)CL_DEV_INVALID_KERNEL == clErr )
    {
        clErr = CL_INVALID_KERNEL;
    }
    if( (signed)CL_DEV_INVALID_VALUE == clErr )
    {
        clErr = CL_INVALID_VALUE;
    }

    return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::CreateDeviceKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::CreateDeviceKernels(std::vector<unique_ptr<DeviceProgram>>& ppDevicePrograms)
{
    if (ppDevicePrograms.empty())
    {
        return CL_INVALID_VALUE;
    }
    
    cl_err_code     clErrRet = CL_SUCCESS;
    const DeviceKernel* pDeviceKernel = NULL;
    const DeviceKernel* pPrevDeviceKernel = NULL;
    bool            bResult = false;
    size_t          maxArgBufferSize = 0;
    size_t          maxArgumentBufferAlignment = sizeof(size_t); // Minimum aligment is as size_t

    for(size_t i = 0; i < m_szAssociatedDevices; ++i)
    {
        // get build status and check that there is a valid binary;
        cl_build_status clBuildStatus = ppDevicePrograms[i]->GetBuildStatus();
        EDeviceProgramState program_state= ppDevicePrograms[i]->GetStateInternal();
        if ( (CL_BUILD_SUCCESS!=clBuildStatus)  && (DEVICE_PROGRAM_BUILTIN_KERNELS!=program_state) )
        {
            continue;
        }
        const FissionableDevice* pDevice = ppDevicePrograms[i]->GetDevice().GetPtr();
        if (NULL != GetDeviceKernel(pDevice))
        {
            LOG_ERROR(TEXT("Already have a kernel for device ID(%d)"), pDevice->GetId());
            continue;
        }
        
        // create the device kernel object
        pDeviceKernel = new DeviceKernel(this, ppDevicePrograms[i]->GetDevice(),
                                                ppDevicePrograms[i]->GetDeviceProgramHandle(),
                                                GET_LOGGER_CLIENT, &clErrRet);
        if (NULL == pDeviceKernel)
        {
            clErrRet = CL_OUT_OF_HOST_MEMORY;
        }
        if (CL_FAILED(clErrRet))
        {
            LOG_ERROR(TEXT("new DeviceKernel(...) failed (returned %s)"), ClErrTxt(clErrRet));
            delete pDeviceKernel;
            break;
        }
        
        // check kernel definition - compare previous kernel to the next one
        if (NULL != pPrevDeviceKernel)
        {
            bResult = pDeviceKernel->CheckKernelDefinition(pPrevDeviceKernel);
            if (false == bResult)
            {
                LOG_ERROR(TEXT("%s"), TEXT("Device kernel prototypes don't match"));
                delete pDeviceKernel;
                clErrRet = CL_INVALID_KERNEL_DEFINITION;
                break;
            }
        }

        // update previous device kernel pointer
        pPrevDeviceKernel = pDeviceKernel;
        
        size_t argBufferSize = pDeviceKernel->GetKernelArgBufferSize();
        if ( argBufferSize > maxArgBufferSize )
            maxArgBufferSize = argBufferSize;

        size_t argBufferAlignment = pDeviceKernel->GetKernelArgBufferAlignment();
        if ( argBufferAlignment > maxArgumentBufferAlignment )
            maxArgumentBufferAlignment = argBufferAlignment;

        // add new device kernel to the objects map list
        m_vpDeviceKernels.push_back(pDeviceKernel);

        // assume kernel attributes are the same for all devices
        // get the information from the first device
        if (i == 0)
        {
          m_sKernelPrototype.m_szKernelAttributes = pDeviceKernel->GetPrototype().m_szKernelAttributes;
        }
    }
    
    if (CL_FAILED(clErrRet))
    {
        // Delete already-created device kernels
        for (size_t i = 0; i<m_vpDeviceKernels.size(); ++i)
        {
            delete m_vpDeviceKernels[i];
        }
        m_vpDeviceKernels.clear();
        return clErrRet;
    }

    // At lease one device kernel was crated
    // set the kernel prototype for the current kernel based on its information
    if (NULL != pDeviceKernel)
    {
        SetKernelPrototype(pDeviceKernel->GetPrototype(), maxArgBufferSize, maxArgumentBufferAlignment);
        SetKernelArgumentInfo(pDeviceKernel);
    }

    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::SetKernelArgumentInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::SetKernelArgumentInfo(const DeviceKernel* pDeviceKernel)
{
    // Query kernel argument information
    const SharedPtr<FissionableDevice>& pDevice = pDeviceKernel->GetDevice();
    const ConstSharedPtr<FrontEndCompiler>& pFECompiler = pDevice->GetRootDevice()->GetFrontEndCompiler();
    cl_device_id devID = (cl_device_id)pDevice->GetHandle();
    const char* pBin = m_pProgram->GetBinaryInternal(devID);
    size_t uiBinSize = m_pProgram->GetBinarySizeInternal(devID);

    cl_err_code clErrCode = CL_SUCCESS;

    if ( (NULL != pFECompiler) || (NULL != pBin) )
    {
        // First use Front-end compiler for the information
        ClangFE::IOCLFEKernelArgInfo* pArgsInfo;
        clErrCode = pFECompiler->GetKernelArgInfo(pBin, uiBinSize, m_sKernelPrototype.m_szKernelName.c_str(), &pArgsInfo );
        if ( CL_FAILED(clErrCode) )
        {
            // If no kernel arg info, so just ignore.
            // Otherwise, free internal data
            // [OpenCL2.1 Development] Temporary disable assert below due to in case of SPIRV FE compiler returns CL_OUT_OF_HOST_MEMORY instead of CL_KERNEL_ARG_INFO_NOT_AVAILABLE.
            //assert(  CL_KERNEL_ARG_INFO_NOT_AVAILABLE == clErrCode && "other codes indicates logical errors and should never occure");
            return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
        }

        // Fill information
        size_t numArgs =pArgsInfo->getNumArgs();
        m_vArgumentsInfo.resize(numArgs);
        for(size_t i=0; i<m_vArgumentsInfo.size(); ++i)
        {
            SKernelArgumentInfo& argInfo = m_vArgumentsInfo[i];
             
            argInfo.accessQualifier = pArgsInfo->getArgAccessQualifier(i);
            argInfo.adressQualifier = pArgsInfo->getArgAdressQualifier(i);
            argInfo.name            = pArgsInfo->getArgName(i);
            argInfo.typeName        = pArgsInfo->getArgTypeName(i);
            argInfo.typeQualifier   = pArgsInfo->getArgTypeQualifier(i);
        }

        pArgsInfo->Release();
    }
    else
    {
        size_t numArgs = m_sKernelPrototype.m_vArguments.size();
        m_vArgumentsInfo.resize(numArgs);

        clLocalArray<cl_kernel_argument_info>  argInfoArray(numArgs);

        cl_dev_err_code clDevErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(
            pDeviceKernel->GetId(), CL_DEV_KERNEL_ARG_INFO, 0, NULL, numArgs*sizeof(cl_kernel_argument_info), &argInfoArray[0], NULL);
        if (CL_DEV_FAILED(clDevErr))
        {
            m_vArgumentsInfo.clear();
            clErrCode = (clDevErr == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        }

        // Now assign the values
        for(size_t i=0; i<m_vArgumentsInfo.size(); ++i)
        {
            SKernelArgumentInfo& argInfo = m_vArgumentsInfo[i];
             
            argInfo.accessQualifier = argInfoArray[i].accessQualifier;
            argInfo.adressQualifier = argInfoArray[i].adressQualifier;
            argInfo.name            = argInfoArray[i].name;
            argInfo.typeName        = argInfoArray[i].typeName;
            argInfo.typeQualifier   = argInfoArray[i].typeQualifier;
        }
    }

    return clErrCode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::SetKernelPrototype
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::SetKernelPrototype(const SKernelPrototype& sKernelPrototype, size_t maxArgumentBufferSize, size_t maxArgumentBufferAlignment)
{
    m_sKernelPrototype = sKernelPrototype;

    if ( NULL != m_pArgsBlob )
    {
        delete [] m_pArgsBlob;
    }

    m_pArgsBlob = new char[maxArgumentBufferSize];
    if ( NULL == m_pArgsBlob )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memset(m_pArgsBlob, 0, maxArgumentBufferSize);

    m_sKernelPrototype.m_dispatchBufferProperties.size = maxArgumentBufferSize;
    m_sKernelPrototype.m_dispatchBufferProperties.argumentOffset = 0;
    m_sKernelPrototype.m_dispatchBufferProperties.alignment = maxArgumentBufferAlignment;

    // allocate and init arguments
    size_t argsCount = m_sKernelPrototype.m_vArguments.size();;
    
    m_vecArgs.resize( argsCount );
    for (unsigned int i = 0; i < argsCount; ++i)
    {
        KernelArg& arg = m_vecArgs[i];
        arg.Init( m_pArgsBlob, m_sKernelPrototype.m_vArguments[i] );
    }

    return CL_SUCCESS;
}

cl_err_code Kernel::SetKernelArgInternal(cl_uint uiIndex, const KernelArg* arg)
{
    KernelArg& clArg = m_vecArgs[uiIndex];
    cl_err_code ret = CL_SUCCESS;

    size_t valueSize = arg->GetSize();
    std::vector<char> value(valueSize);
    arg->GetValue(valueSize, &value[0]);
    if(arg->IsLocalPtr())
    {
        m_totalLocalSize -= clArg.GetLocalBufferSize();
        clArg.SetValue(valueSize, (void*)&value[0]);
        m_totalLocalSize += clArg.GetLocalBufferSize();
    }
    else if(arg->IsSampler() || clArg.IsQueueId())
        clArg.SetValue(clArg.GetSize(), &value[0]);
    else if(clArg.IsImage())
        clArg.SetValue(sizeof(cl_mem), &value[0]);
    else if(arg->IsSvmPtr())
    {
        char* backingStore = (char*)(*(SVMPointerArg**)&value[0])->GetBackingStoreData();
        return SetKernelArg(uiIndex, valueSize, backingStore, true/*IsSvmPtr*/);
    }
    else
        return SetKernelArg(uiIndex, valueSize, &value[0], false/*IsSvmPtr*/);

    ++m_numValidArgs;
    clArg.SetValid();
    return ret;
}

cl_err_code Kernel::SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue, bool bIsSvmPtr)
{
    LOG_DEBUG(TEXT("Enter SetKernelArg (uiIndex=%d, szSize=%d, pValue=%d"), uiIndex, szSize, pValue);

    size_t argCount = m_sKernelPrototype.m_vArguments.size();
    // check argument's index
    if (uiIndex + 1 > argCount)
    {
        return CL_INVALID_ARG_INDEX;
    }

    // TODO: check for NULL and __local / __global / ... qualifier mismatches

    // check for invalid arg sizes
    KernelArg&         clArg         = m_vecArgs[uiIndex];
    size_t             clArgSize     = clArg.GetSize();

    Context*           pContext      = m_pContext.GetPtr();

    if ( clArg.IsBuffer() )
    {
        // memory object
        if ( NULL == pValue )
        {
            clArg.SetValue(sizeof(cl_mem), NULL);
        } else 
        {
            if (bIsSvmPtr)
            {
                const SharedPtr<SVMBuffer>& pSvmBuf = pContext->GetSVMBufferContainingAddr(const_cast<void*>(pValue));
                // !!!!!!
                // TODO: Why we need this wrapper, why just not put SVMBuffer inside or just vritual address
                // !!!!!!
                SharedPtr<SVMPointerArg> pSvmPtrArg;
                if (NULL != pSvmBuf)
                {
                    pSvmPtrArg = SVMBufferPointerArg::Allocate(pSvmBuf.GetPtr(), pValue);
                }
                else
                {
                    pSvmPtrArg = SVMSystemPointerArg::Allocate(pValue);
                }
                SVMPointerArg* argVal = pSvmPtrArg.GetPtr();
                clArg.SetValue(sizeof(SVMPointerArg*), &argVal );
                clArg.SetSvmObject( pSvmPtrArg );  
            }
            else
            {
                // value is not NULL - get memory object from context
                cl_mem clMemId = *((cl_mem*)(pValue));
                LOG_DEBUG(TEXT("SetKernelArg buffer (cl_mem=%d)"), clMemId);
                clArg.SetSvmObject( NULL );
                if (NULL == clMemId)
                {
                    clArg.SetValue(sizeof(cl_mem), NULL);
                }
                else
                {
                    MemoryObject* pMemObj = pContext->GetMemObjectPtr(clMemId);
                    if (NULL == pMemObj)
                    {
                        return CL_INVALID_MEM_OBJECT;
                    }
                    // TODO: check Memory properties
                    clArg.SetValue(sizeof(cl_mem), &clMemId); 
                }
            }            
        }
    } else
    // Images
    if (clArg.IsImage())
    {
        if (clArgSize != szSize)
        {
            return CL_INVALID_ARG_SIZE;
        }

        if (NULL == pValue)
        {
            return CL_INVALID_ARG_VALUE;
        }

        cl_mem clMemId = *((cl_mem*)(pValue));

        LOG_DEBUG(TEXT("SetKernelArg image (cl_mem=%d)"), clMemId);

        const SharedPtr<MemoryObject>& pMemObj = pContext->GetMemObject(clMemId);
        if (NULL == pMemObj)
        {
            return CL_INVALID_ARG_VALUE;
        }     

        cl_image_format imgFormat;
        cl_err_code err = pMemObj->GetImageInfo(CL_IMAGE_FORMAT, sizeof(imgFormat), &imgFormat, NULL);
        if (CL_FAILED(err))
        {
            return CL_INVALID_ARG_VALUE;
        }

        cl_kernel_arg_type clArgType     = clArg.GetType(); 
        bool depth_required = (clArgType == CL_KRNL_ARG_PTR_IMG_2D_DEPTH || clArgType == CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH);
        // either both depth_required and depth provided or not
        if (depth_required ^ (imgFormat.image_channel_order == CL_DEPTH))
        {
            return CL_INVALID_ARG_VALUE;   
        }

        clArg.SetValue(sizeof(cl_mem), &clMemId); 
    }
    // Local buffer
    else if (clArg.IsLocalPtr())
    {
        if (0 == szSize)
        {
            return CL_INVALID_ARG_SIZE;
        }
        
        if (NULL != pValue)
        {
            return CL_INVALID_ARG_VALUE;
        }

        // local memory pointer 
        m_totalLocalSize -= clArg.GetLocalBufferSize();
        clArg.SetValue(szSize, (void*)pValue);
        m_totalLocalSize += clArg.GetLocalBufferSize();
    }
    else if (clArg.IsSampler())
    {
        if (sizeof(cl_sampler) != szSize)
        {
            return CL_INVALID_ARG_SIZE;
        }

        // sampler
        if (NULL == pValue)
        {
            return CL_INVALID_SAMPLER;
        }
        
        cl_sampler clSamplerId = *((cl_sampler*)(pValue));
        SharedPtr<Sampler> pSampler = pContext->GetSampler(clSamplerId); 
        if (NULL == pSampler)
        {
            return CL_INVALID_SAMPLER;
        }
        //Support for new(opaque ptr) and old(int32) sampler.
        if (clArg.IsInt32Sampler())
        {
            assert(clArg.GetSize() == sizeof(cl_uint) &&
                "Mismatch between sampler representation on host and device!");
            cl_uint  value = pSampler->GetValue();
            clArg.SetValue(clArg.GetSize(), &value);
        }
        else if (clArg.IsOpaqueSampler())
        {
            assert(clArg.GetSize() == sizeof(size_t) &&
                "Mismatch between sampler representation on host and device!");
            size_t value = (size_t)pSampler->GetValue();
            clArg.SetValue(clArg.GetSize(), &value);
        }
        else
            assert(0 && "Unknown type of sampler.");
    }
    else if (clArg.IsQueueId())
    {
        if (sizeof(cl_command_queue) != szSize)
        {
            return CL_INVALID_ARG_SIZE;
        }

        const cl_command_queue queueId = *(cl_command_queue*)pValue;
        ExecutionModule& pExecutionModule = *FrameworkProxy::Instance()->GetExecutionModule();
        const SharedPtr<OclCommandQueue> pQueue = pExecutionModule.GetCommandQueue(queueId);
        if (NULL == pQueue)
        {
            return CL_INVALID_QUEUE;
        }

        void* const pCmdListPtr = pQueue->GetDeviceCommandListPtr();
        clArg.SetValue(clArg.GetSize(), &pCmdListPtr);
    }
    else
    {    // other type = check size
        if (szSize != clArgSize)
        {
            return CL_INVALID_ARG_SIZE;
        }

        if ( NULL == pValue) 
        {
            return CL_INVALID_ARG_VALUE;
        }

        clArg.SetValue(szSize, (void*)pValue);
    }

    if (!clArg.IsValid())
    {
        ++m_numValidArgs;
        clArg.SetValid();
    }
    
    return CL_SUCCESS;
}


const DeviceKernel* Kernel::GetDeviceKernel(const FissionableDevice* pDevice) const
{
    assert( (NULL!=pDevice) && "Invalid device");

    if ( NULL==pDevice )
    {
        return NULL;
    }

    size_t numDevKernels = m_vpDeviceKernels.size();
    // First look in list of kernel built device
    for (size_t i = 0; i < numDevKernels; ++i)
    {
        if ( pDevice == m_vpDeviceKernels[i]->GetDevice().GetPtr() )
        {
            return m_vpDeviceKernels[i];
        }
    }

    // If not found, need to look into the program, maybe was built on "parent" device
    cl_int relatedDeviceObjId = 0;
    const cl_device_id devId = (const cl_device_id)pDevice->GetHandle();
    // Get the object id of the device that I'm inherit its binary
    bool isFound = m_pProgram->GetMyRelatedProgramDeviceIDInternal(devId, &relatedDeviceObjId);
    if (isFound)
    {
        for (size_t i = 0; i < numDevKernels; ++i)
        {
            if (NULL != m_vpDeviceKernels[i] && relatedDeviceObjId == m_vpDeviceKernels[i]->GetDeviceId())
            {
                return m_vpDeviceKernels[i];
            }
        }
    }
    return NULL;
}

cl_dev_kernel Kernel::GetDeviceKernelId(const FissionableDevice* pDevice) const
{
    const DeviceKernel* pDeviceKernel = GetDeviceKernel(pDevice);
    if (pDeviceKernel)
    {
        return pDeviceKernel->GetId();
    }
    return CL_INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////
// OpenCL 1.2 functions
/////////////////////////////////////////////////////////////////////

cl_err_code Kernel::GetKernelArgInfo (    cl_uint argIndx,
                                cl_kernel_arg_info paramName,
                                size_t      szParamValueSize,
                                void *      pParamValue,
                                size_t *    pszParamValueSizeRet)
{
    size_t stParamSize;
    const void* pValue;

    if ( m_vArgumentsInfo.empty() )
    {
        return CL_KERNEL_ARG_INFO_NOT_AVAILABLE;
    }

    if  (!pszParamValueSizeRet && !pParamValue)
    {
        return CL_INVALID_VALUE;
    }
    if ( argIndx > (GetKernelArgsCount()-1) )
    {
        return CL_INVALID_VALUE;
    }

    // Initial implementation requried by the common runtime
    switch ( paramName )
    {
    case CL_KERNEL_ARG_NAME:
        pValue = m_vArgumentsInfo[argIndx].name.c_str();
        stParamSize = m_vArgumentsInfo[argIndx].name.length() + 1;      
        break;
    case CL_KERNEL_ARG_TYPE_NAME:
        pValue = m_vArgumentsInfo[argIndx].typeName.c_str();
        stParamSize = m_vArgumentsInfo[argIndx].typeName.length() + 1;      
        break;
    case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
        pValue = &(m_vArgumentsInfo[argIndx].adressQualifier);
        stParamSize = sizeof(cl_kernel_arg_address_qualifier);      
        break;
    case CL_KERNEL_ARG_ACCESS_QUALIFIER:
        pValue = &(m_vArgumentsInfo[argIndx].accessQualifier);
        stParamSize = sizeof(cl_kernel_arg_access_qualifier);       
        break;
    case CL_KERNEL_ARG_TYPE_QUALIFIER:
        pValue = &(m_vArgumentsInfo[argIndx].typeQualifier);
        stParamSize = sizeof(cl_kernel_arg_type_qualifier);      
        break;
    default:
        return CL_INVALID_VALUE;        
    }
    
    if (NULL != pParamValue)
    {
        if (szParamValueSize >= stParamSize)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, stParamSize);
        }
        else
        {
            return CL_INVALID_VALUE;
        }
    }
    
    if ( NULL != pszParamValueSizeRet )
    {
        *pszParamValueSizeRet = stParamSize;
    }

    return CL_SUCCESS;
}

void Kernel::SetNonArgSvmBuffers(const std::vector<SharedPtr<SVMBuffer> >& svmBufs)
{
    OclAutoWriter mutex(&m_rwlock);
    m_nonArgSvmBufs.resize(svmBufs.size());
    std::copy(svmBufs.begin(), svmBufs.end(), m_nonArgSvmBufs.begin());
}

void Kernel::GetNonArgSvmBuffers(std::vector<SharedPtr<SVMBuffer> >& svmBufs) const
{    
    OclAutoReader mutex(&m_rwlock);
    svmBufs.resize(m_nonArgSvmBufs.size());
    std::copy(m_nonArgSvmBufs.begin(), m_nonArgSvmBufs.end(), svmBufs.begin());
}

size_t Kernel::GetNonArgSvmBuffersCount() const 
{ 
    return m_nonArgSvmBufs.size(); 
}


