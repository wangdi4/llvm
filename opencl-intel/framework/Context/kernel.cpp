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
#include "cl_sys_defines.h"
#include "fe_compiler.h"
#include <cl_objects_map.h>
#include <Device.h>
#include <assert.h>
#include <cl_utils.h>
#include "sampler.h"
#include "cl_shared_ptr.hpp"
#include "svm_buffer.h"
#include "Context.h"
#include "context_module.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::DeviceKernel(Kernel*                             pKernel, 
                           const SharedPtr<FissionableDevice>& pDevice,
                           cl_dev_program  devProgramId,
                           const char *    psKernelName, 
                           LoggerClient *  pLoggerClient,
                           cl_err_code *   pErr) 
: OCLObjectBase("DeviceKernel"), m_clDevKernel(CL_INVALID_HANDLE), m_pKernel(pKernel), m_pDevice(pDevice)
{
    assert ( pErr != NULL );

    SET_LOGGER_CLIENT(pLoggerClient);
    LOG_DEBUG(TEXT("%s"), TEXT("DeviceKernel C'tor enter"));
    m_sKernelPrototype.m_psKernelName = NULL;
    m_sKernelPrototype.m_uiArgsCount  = 0;
    m_sKernelPrototype.m_pArgs        = NULL;
    m_sKernelPrototype.m_pArgsInfo    = NULL;

    if (NULL == m_pKernel || NULL == m_pDevice || NULL == psKernelName || CL_INVALID_HANDLE == devProgramId)
    {
        LOG_ERROR(TEXT("%s"), TEXT("NULL == m_pKernel || NULL == m_pDevice || NULL == psKernelName || CL_INVALID_HANDLE == devProgramId"));
        *pErr = CL_INVALID_VALUE;
        return;
    }

    // update kernel prototype
    size_t szNameLength = strlen(psKernelName) + 1;
    m_sKernelPrototype.m_psKernelName = new char[szNameLength];
    if (NULL == m_sKernelPrototype.m_psKernelName)
    {
        LOG_ERROR(TEXT("new char[%d] == NULL"), strlen(psKernelName) + 1);
        *pErr = CL_OUT_OF_HOST_MEMORY;
        return;
    }

    // copy kernel name;
    STRCPY_S(m_sKernelPrototype.m_psKernelName, szNameLength, psKernelName);

    // get kernel id
    cl_dev_err_code clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelId(devProgramId, m_sKernelPrototype.m_psKernelName, &m_clDevKernel);
    if (CL_DEV_FAILED(clErrRet))
    {
        LOG_ERROR(TEXT("%s"), TEXT("Device->GetKernelId failed"));
        delete[] m_sKernelPrototype.m_psKernelName;
        m_sKernelPrototype.m_psKernelName = NULL;
        *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        return;
    }

    // get kernel prototype
    size_t szArgsCount = 0;
    clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, 0, NULL, &szArgsCount);
    if (CL_DEV_FAILED(clErrRet))
    {
        *pErr = clErrRet;
        return;
    }
    assert(szArgsCount / sizeof(cl_kernel_argument) <= CL_MAX_UINT32);
    m_sKernelPrototype.m_uiArgsCount = (cl_uint)(szArgsCount / sizeof(cl_kernel_argument));
    m_sKernelPrototype.m_pArgs = new cl_kernel_argument[m_sKernelPrototype.m_uiArgsCount];
    if ( NULL == m_sKernelPrototype.m_pArgs )
    {
        *pErr = CL_OUT_OF_HOST_MEMORY;
        delete[] m_sKernelPrototype.m_psKernelName;
        m_sKernelPrototype.m_psKernelName = NULL;
        return;
    }

    clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE,
        m_sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument), m_sKernelPrototype.m_pArgs, NULL);
    if (CL_DEV_FAILED(clErrRet))
    {
        delete[] m_sKernelPrototype.m_psKernelName;
        m_sKernelPrototype.m_psKernelName = NULL;
        delete[] m_sKernelPrototype.m_pArgs;
        m_sKernelPrototype.m_pArgs = NULL;
        *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
        return;
    }
    
    const ConstSharedPtr<FrontEndCompiler>& pFECompiler = m_pDevice->GetRootDevice()->GetFrontEndCompiler();
    cl_device_id devID = (cl_device_id)m_pDevice->GetHandle();
    const char* pBin = m_pKernel->m_pProgram->GetBinaryInternal(devID);

    if ( (NULL != pFECompiler) && (NULL != pBin) )
    {
        cl_err_code clErrCode = pFECompiler->GetKernelArgInfo(pBin, psKernelName, &m_sKernelPrototype.m_pArgsInfo, NULL);
        if ( CL_FAILED(clErrCode) )
        {
            m_sKernelPrototype.m_pArgsInfo = NULL;
            // If no kernel arg info, so just ignore.
            // Otherwise, free internal data
            if ( CL_KERNEL_ARG_INFO_NOT_AVAILABLE != clErrCode )
            {
                delete[] m_sKernelPrototype.m_psKernelName;
                m_sKernelPrototype.m_psKernelName = NULL;
                delete[] m_sKernelPrototype.m_pArgs;
                m_sKernelPrototype.m_pArgs = NULL;
                *pErr = clErrCode;
            }
            return;
        }

        assert((CL_SUCCESS == clErrCode) && "other codes indicates logical errors and should never occure");
    }
    else
    {

        m_sKernelPrototype.m_pArgsInfo = new cl_kernel_argument_info[m_sKernelPrototype.m_uiArgsCount];
        if ( NULL == m_sKernelPrototype.m_pArgsInfo )
        {
            delete[] m_sKernelPrototype.m_psKernelName;
            m_sKernelPrototype.m_psKernelName = NULL;
            delete[] m_sKernelPrototype.m_pArgs;
            m_sKernelPrototype.m_pArgs = NULL;
            *pErr = CL_OUT_OF_HOST_MEMORY;
            return;
        }
        
        clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_ARG_INFO,
            m_sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument_info), m_sKernelPrototype.m_pArgsInfo, NULL);
        if (CL_DEV_FAILED(clErrRet))
        {
            delete[] m_sKernelPrototype.m_psKernelName;
            m_sKernelPrototype.m_psKernelName = NULL;
            delete[] m_sKernelPrototype.m_pArgs;
            m_sKernelPrototype.m_pArgs = NULL;
            delete[] m_sKernelPrototype.m_pArgsInfo;
            m_sKernelPrototype.m_pArgsInfo = NULL;
            *pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
            return;
        }
    }

    // we are here - all passed ok    
    if (!CacheRequiredInfo())
    {
        delete[] m_sKernelPrototype.m_psKernelName;
        m_sKernelPrototype.m_psKernelName = NULL;
        delete[] m_sKernelPrototype.m_pArgs;
        m_sKernelPrototype.m_pArgs = NULL;
        delete[] m_sKernelPrototype.m_pArgsInfo;
        m_sKernelPrototype.m_pArgsInfo = NULL;
        *pErr =  CL_INVALID_DEVICE;
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::~DeviceKernel()
{
    LOG_DEBUG(TEXT("%s"), TEXT("DeviceKernel D'tor enter"));
    if (NULL != m_sKernelPrototype.m_psKernelName)
    {
        delete[] m_sKernelPrototype.m_psKernelName;
        m_sKernelPrototype.m_psKernelName = NULL;
    }
    if (NULL != m_sKernelPrototype.m_pArgs)
    {
        delete[] m_sKernelPrototype.m_pArgs;
        m_sKernelPrototype.m_pArgs = NULL;
    }
    if (NULL != m_sKernelPrototype.m_pArgsInfo)
    {
        delete[] m_sKernelPrototype.m_pArgsInfo;
        m_sKernelPrototype.m_pArgsInfo = NULL;
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeviceKernel::CacheRequiredInfo()
{
    cl_dev_err_code err;
    IOCLDeviceAgent* pDevAgent = m_pDevice->GetDeviceAgent();
    assert( NULL != pDevAgent );

    err = pDevAgent->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_MAX_WG_SIZE, sizeof(m_CL_KERNEL_WORK_GROUP_SIZE), &m_CL_KERNEL_WORK_GROUP_SIZE, NULL);
    assert( CL_DEV_SUCCEEDED(err) && "caching of GetKernelWorkGroupSize failed" );

    if (CL_DEV_SUCCEEDED(err))
    {
        err = pDevAgent->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_WG_SIZE_REQUIRED, sizeof(m_CL_KERNEL_COMPILE_WORK_GROUP_SIZE), &m_CL_KERNEL_COMPILE_WORK_GROUP_SIZE, NULL);
        assert( CL_DEV_SUCCEEDED(err) && "caching of GetKernelCompileWorkGroupSize failed" );
    }
    
    if (CL_DEV_SUCCEEDED(err))
    {
        err = pDevAgent->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE, sizeof(m_CL_KERNEL_LOCAL_MEM_SIZE), &m_CL_KERNEL_LOCAL_MEM_SIZE, NULL);
        assert( CL_DEV_SUCCEEDED(err) && "caching of GetKernelLocalMemSize failed" );
    }

    return CL_DEV_SUCCEEDED(err);        
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeviceKernel::CheckKernelDefinition(DeviceKernel * pKernel) const
{
    //cl_start;

    if (NULL == pKernel)
    {
        return false;
    }
    SKernelPrototype sKernelPrototype = pKernel->GetPrototype();
    if (strcmp(sKernelPrototype.m_psKernelName, m_sKernelPrototype.m_psKernelName) != 0)
    {
        return false;
    }
    if (sKernelPrototype.m_uiArgsCount != m_sKernelPrototype.m_uiArgsCount)
    {
        return false;
    }
    for (cl_uint ui=0; ui<m_sKernelPrototype.m_uiArgsCount; ++ui)
    {
        if ((sKernelPrototype.m_pArgs[ui].type != m_sKernelPrototype.m_pArgs[ui].type) ||
            (sKernelPrototype.m_pArgs[ui].size_in_bytes != m_sKernelPrototype.m_pArgs[ui].size_in_bytes))
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
void KernelArg::Init( cl_uint uiIndex, const cl_kernel_argument& clKernelArgType )
{
    m_pValue                = NULL;
    m_szOffset              = 0;
    m_pSvmPtrArg            = NULL;
    m_bValid                = false;
    m_uiIndex               = uiIndex;
    m_clKernelArgType       = clKernelArgType;

    // BUGBUG: In BE - wrong size for sampler
    if (CL_KRNL_ARG_SAMPLER == m_clKernelArgType.type)
    {
        m_clKernelArgType.size_in_bytes = sizeof(cl_uint);
    }
    else if (CL_KRNL_ARG_VECTOR == m_clKernelArgType.type)
    {
        m_clKernelArgType.size_in_bytes = (m_clKernelArgType.size_in_bytes & 0xFFFF) * ((m_clKernelArgType.size_in_bytes >> 16) & 0xFFFF);
    }
}

void KernelArg::SetValuePlaceHolder( void * pValuePlaceHolder, size_t offset )
{
    assert( NULL != pValuePlaceHolder );
    m_pValue    = pValuePlaceHolder;
    m_szOffset  = offset;

    if (CL_KRNL_ARG_PTR_LOCAL == m_clKernelArgType.type)
    {
        *(size_t*)m_pValue = 0;
    }    
}

void KernelArg::SetValue( size_t szSize, void * pValue )
{
    assert( m_pValue && "SetValuePlaceHolder was not called before");

    if (IsLocalPtr())
    {
        *(size_t*)m_pValue = szSize;
    }
    else
    {
        assert( szSize == GetSize() && "Wrong param provided" );
        if (NULL != pValue)
        {
            switch (szSize)
            {
                case sizeof(cl_uint):
                    *((cl_uint*)m_pValue) = *((cl_uint*)pValue);
                    break;

                case sizeof(cl_long):
                    *((cl_long*)m_pValue) = *((cl_long*)pValue);
                    break;

                default:
                    MEMCPY_S( m_pValue, GetSize(), pValue, szSize );
            }
        }
        else
        {
            switch (szSize)
            {
                case sizeof(cl_uint):
                    *((cl_uint*)m_pValue) = 0;
                    break;

                case sizeof(cl_long):
                    *((cl_long*)m_pValue) = 0;
                    break;

                default:
                    memset( m_pValue, 0, GetSize() );
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
    m_deviceArgsSize(0), m_totalLocalSize(0), m_bSvmFineGrainSystem(false)
{
    size_t szNameLength = strlen(psKernelName) + 1;
    m_sKernelPrototype.m_psKernelName = new char[szNameLength];
    //Todo: what if allocation fails here?
    if (NULL != m_sKernelPrototype.m_psKernelName)
    {
        STRCPY_S(m_sKernelPrototype.m_psKernelName, szNameLength, psKernelName);
    }
        
    m_sKernelPrototype.m_pArgs = NULL;
    m_sKernelPrototype.m_uiArgsCount = 0;

    m_ppDeviceKernels = new DeviceKernel* [m_szAssociatedDevices];
    memset(m_ppDeviceKernels, 0, sizeof(DeviceKernel *) * m_szAssociatedDevices);

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

    // release kernel prototype
    if (m_sKernelPrototype.m_psKernelName)
    {
        delete[] m_sKernelPrototype.m_psKernelName;
    }
    if (m_sKernelPrototype.m_pArgs)
    {
        delete[] m_sKernelPrototype.m_pArgs;
    }

    if (m_ppDeviceKernels)
    {
        // clear device kernels
        for (size_t i = 0; i < m_szAssociatedDevices; ++i)
        {
            delete m_ppDeviceKernels[i];
        }
        delete[] m_ppDeviceKernels;
    }

    m_vecArgs.clear();

    if (m_pArgsBlob)
    {
        delete[] m_pArgsBlob;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code    Kernel::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
    LOG_DEBUG(TEXT("Enter Kernel::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
        iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    size_t szParamSize = 0;
    const void * pValue = NULL;
    cl_ulong iParam = 0;
    switch (iParamName)
    {
    case CL_KERNEL_FUNCTION_NAME:
        if (NULL != m_sKernelPrototype.m_psKernelName)
        {
            szParamSize = strlen(m_sKernelPrototype.m_psKernelName) + 1;
            pValue = m_sKernelPrototype.m_psKernelName;
        }
        break;
    case CL_KERNEL_NUM_ARGS:
        szParamSize = sizeof(cl_uint);
        pValue = &(m_sKernelPrototype.m_uiArgsCount);
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
    //get device
    assert (NULL != m_ppDeviceKernels);

    if ( NULL == pDevice )
    {
        pDevice = m_ppDeviceKernels[0]->GetDevice().GetPtr();
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
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_MAX_WG_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE_REQUIRED, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_LOCAL_MEM_SIZE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
        break;
    case CL_KERNEL_PRIVATE_MEM_SIZE:
        clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_PRIVATE_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
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
cl_err_code Kernel::CreateDeviceKernels(DeviceProgram** ppDevicePrograms)
{
    if (NULL == ppDevicePrograms)
    {
        return CL_INVALID_VALUE;
    }
    
    cl_err_code clErrRet = CL_SUCCESS;
    DeviceKernel * pDeviceKernel = NULL, * pPrevDeviceKernel = NULL;
    bool bResult = false;
    size_t i;
    
    for(i = 0; i < m_szAssociatedDevices; ++i)
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
        pDeviceKernel = new DeviceKernel(this, ppDevicePrograms[i]->GetDevice(), ppDevicePrograms[i]->GetDeviceProgramHandle(), m_sKernelPrototype.m_psKernelName, GET_LOGGER_CLIENT, &clErrRet);
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
                LOG_ERROR(TEXT("%s"), TEXT("CheckKernelDefinition failed (returned false)"));
                delete pDeviceKernel;
                clErrRet = CL_INVALID_KERNEL_DEFINITION;
                break;
            }
        }

        // update previous device kernel pointer
        pPrevDeviceKernel = pDeviceKernel;
        
        // add new device kernel to the objects map list
        m_ppDeviceKernels[i] = pDeviceKernel;
    }
    
    if (CL_FAILED(clErrRet))
    {
        // Delete already-created device kernels
        for (size_t j = 0; j < i; ++j)
        {
            delete m_ppDeviceKernels[j];
        }
        delete [] m_ppDeviceKernels;
        m_ppDeviceKernels = NULL;
        return clErrRet;
    }

    // set the kernel prototype for the current kernel
    if (NULL != pDeviceKernel)
    {
        SKernelPrototype sKernelPrototype = (SKernelPrototype)pDeviceKernel->GetPrototype();
        SetKernelPrototype(sKernelPrototype);
    }

    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::SetKernelPrototype
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::SetKernelPrototype(SKernelPrototype sKernelPrototype)
{
    //cl_start;

    //We initialized our name at kernel creation, no need to do it again
    assert(sKernelPrototype.m_psKernelName);
    assert(m_sKernelPrototype.m_psKernelName);
    assert(0 == strcmp(sKernelPrototype.m_psKernelName, m_sKernelPrototype.m_psKernelName));

    m_sKernelPrototype.m_uiArgsCount = sKernelPrototype.m_uiArgsCount;

    if (NULL == m_sKernelPrototype.m_pArgs)
    {
        m_sKernelPrototype.m_pArgs = new cl_kernel_argument[sKernelPrototype.m_uiArgsCount];
        if (NULL == m_sKernelPrototype.m_pArgs)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        MEMCPY_S(m_sKernelPrototype.m_pArgs, sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument), sKernelPrototype.m_pArgs, sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument));
    }

    // allocate and init arguments
    size_t argsSize = 0;
    
    m_vecArgs.resize( m_sKernelPrototype.m_uiArgsCount );
    for (unsigned int i = 0; i < m_sKernelPrototype.m_uiArgsCount; ++i)
    {
        KernelArg& arg = m_vecArgs[i];
        arg.Init( i, m_sKernelPrototype.m_pArgs[i] );
        argsSize += arg.GetSize();
    }

    if ((NULL != m_pArgsBlob) && (argsSize != m_deviceArgsSize))
    {
        delete [] m_pArgsBlob;
    }

    m_pArgsBlob = new char[argsSize];
    m_deviceArgsSize = argsSize;

    // allocate placeholder for each argument
    argsSize = 0;
    for (unsigned int i = 0; i < m_sKernelPrototype.m_uiArgsCount; ++i)
    {
        KernelArg& arg = m_vecArgs[i];
        arg.SetValuePlaceHolder( m_pArgsBlob + argsSize, argsSize );
        argsSize += arg.GetSize();
    }
    
    return CL_SUCCESS;
}

cl_err_code Kernel::SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue, bool bIsSvmPtr)
{
    //cl_start;

    LOG_DEBUG(TEXT("Enter SetKernelArg (uiIndex=%d, szSize=%d, pValue=%d"), uiIndex, szSize, pValue);

    assert ( m_pProgram != NULL );
    assert ( m_pContext != NULL );

    // check argument's index
    if (uiIndex > m_sKernelPrototype.m_uiArgsCount - 1)
    {
        return CL_INVALID_ARG_INDEX;
    }

    // TODO: check for NULL and __local / __global / ... qualifier mismatches

    // check for invalid arg sizes
    KernelArg&         clArg         = m_vecArgs[uiIndex];
    cl_kernel_arg_type clArgType     = clArg.GetType(); 
    size_t             clArgSize     = clArg.GetSize();

    Context*           pContext      = m_pContext.GetPtr();

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

        const SharedPtr<MemoryObject>& pMemObj = pContext->GetMemObject(*(cl_mem*)pValue);
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

        bool depth_required = (clArgType == CL_KRNL_ARG_PTR_IMG_2D_DEPTH || clArgType == CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH);
        // either both depth_required and depth provided or not
        if (depth_required ^ (imgFormat.image_channel_order == CL_DEPTH))
        {
            return CL_INVALID_ARG_VALUE;   
        }
        
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
    }

    else if (clArg.IsSampler())
    {
        if (sizeof(cl_sampler) != szSize)
        {
            return CL_INVALID_ARG_SIZE;
        }
    }
    
    else
    {    // other type = check size
        if (szSize != clArgSize)
        {
            return CL_INVALID_ARG_SIZE;
        }
    }

    // set arguments        
    if (clArg.IsBuffer() || clArg.IsImage())
    {
        // memory object
        
        if (NULL != pValue)
        {
            if (bIsSvmPtr)
            {
                const SharedPtr<SVMBuffer>& pSvmBuf = pContext->GetSVMBufferContainingAddr(const_cast<void*>(pValue));
                SharedPtr<SVMPointerArg> pSvmPtrArg;
                if (NULL != pSvmBuf)
                {
                    pSvmPtrArg = SVMBufferPointerArg::Allocate(pSvmBuf, pValue);
                }
                else
                {
                    pSvmPtrArg = SVMSystemPointerArg::Allocate(pValue);
                }
                SVMPointerArg* value = pSvmPtrArg.GetPtr();
                clArg.SetValue(sizeof(SVMPointerArg*), &value );
                clArg.SetSvmObject( pSvmPtrArg );  
            }
            else
            {
                // value is not NULL - get memory object from context
                cl_mem clMemId = *((cl_mem*)(pValue));
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
        else
        {
            clArg.SetValue(sizeof(cl_mem), NULL);
        }

    }
    
    else if (clArg.IsSampler())
    {
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
        cl_uint  value       = pSampler->GetValue();
        assert( sizeof(value) == clArg.GetSize());
        clArg.SetValue(clArg.GetSize(), &value);
    }
    
    else if (clArg.IsLocalPtr())
    {
        // local memory pointer 
        
        m_totalLocalSize -= clArg.GetLocalBufferSize();
        clArg.SetValue(szSize, (void*)pValue);
        m_totalLocalSize += clArg.GetLocalBufferSize();
    }
    
    else
    {
        // any other 
        
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
    assert( (NULL!=m_ppDeviceKernels) && "Device kernel array is not available");
    assert( (NULL!=pDevice) && "Invalid device");

    if ( (m_ppDeviceKernels==NULL) || (NULL==pDevice) )
    {
        return NULL;
    }

    // First look in list of kernel built device
    for (size_t i = 0; i < m_szAssociatedDevices; ++i)
    {
        if ( (NULL!=m_ppDeviceKernels[i]) && (pDevice == m_ppDeviceKernels[i]->GetDevice().GetPtr()) )
        {
            return m_ppDeviceKernels[i];
        }
    }

    // If not found, need to look into the program, maybe was built on "parent" device
    cl_int relatedDeviceObjId = 0;
    const cl_device_id devId = (const cl_device_id)pDevice->GetHandle();
    // Get the object id of the device that I'm inherit its binary
    bool isFound = m_pProgram->GetMyRelatedProgramDeviceIDInternal(devId, &relatedDeviceObjId);
    if (isFound)
    {
        for (size_t i = 0; i < m_szAssociatedDevices; ++i)
        {
            if (NULL != m_ppDeviceKernels[i] && relatedDeviceObjId == m_ppDeviceKernels[i]->GetDeviceId())
            {
                return m_ppDeviceKernels[i];
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

    cl_kernel_argument_info* pKernelArgInfo = NULL;

    // find a valid device kernel
    for (cl_uint i = 0; i < m_szAssociatedDevices; ++i)
    {
        if (NULL == m_ppDeviceKernels[i])
        {
            continue;
        }

        if (NULL == m_ppDeviceKernels[i]->GetPrototype().m_pArgsInfo)
        {
            continue;
        }

        if (argIndx >= m_ppDeviceKernels[i]->GetPrototype().m_uiArgsCount)
        {
            return CL_INVALID_ARG_INDEX;
        }

        pKernelArgInfo = m_ppDeviceKernels[i]->GetPrototype().m_pArgsInfo;
        break;
    }

    if (!pKernelArgInfo)
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
        pValue = pKernelArgInfo[argIndx].name;
        stParamSize = strlen((const char*)pValue) + 1;      
        break;
    case CL_KERNEL_ARG_TYPE_NAME:
        pValue = pKernelArgInfo[argIndx].typeName;
        stParamSize = strlen((const char*)pValue) + 1;      
        break;
    case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
        pValue = &(pKernelArgInfo[argIndx].adressQualifier);
        stParamSize = sizeof(cl_kernel_arg_address_qualifier);      
        break;
    case CL_KERNEL_ARG_ACCESS_QUALIFIER:
        pValue = &(pKernelArgInfo[argIndx].accessQualifier);
        stParamSize = sizeof(cl_kernel_arg_access_qualifier);       
        break;
    case CL_KERNEL_ARG_TYPE_QUALIFIER:
        pValue = &(pKernelArgInfo[argIndx].typeQualifier);
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


