/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Program.cpp

\*****************************************************************************/
#include "Program.h"
#include "Kernel.h"

#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif

#include "exceptions.h"
#include "cl_device_api.h"
#include "BitCodeContainer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

Program::Program():
    m_pCodeContainer(NULL),
    m_kernels(NULL)
{
}

Program::~Program()
{
    m_kernels.reset(NULL);
    delete m_pCodeContainer;
}


unsigned long long int Program::GetProgramID() const
{
    assert(false && "NotImplemented");
    return 0; 
}

const char* Program::GetBuildLog() const
{
    return m_buildLog.empty() ? "" : m_buildLog.c_str(); 
}

const ICLDevBackendCodeContainer* Program::GetProgramCodeContainer() const
{
    return m_pCodeContainer;
}

const ICLDevBackendProgramJITCodeProperties* Program::GetProgramJITCodeProperties() const
{
    assert(false && "NotImplemented");
    return NULL;
}

cl_dev_err_code Program::GetKernelByName(const char* IN pKernelName, 
                                         const ICLDevBackendKernel_** OUT pKernel) const
{
    if( !m_kernels.get() || m_kernels->Empty())
    {
        return CL_DEV_INVALID_OPERATION;
    }
    
        *pKernel = (ICLDevBackendKernel_*)m_kernels->GetKernel(pKernelName);
        return CL_DEV_SUCCESS; 
    }

int Program::GetKernelsCount() const
{
    if(!m_kernels.get())
    {
        return 0;
    }

    return m_kernels->GetCount();
}

cl_dev_err_code Program::GetKernel(int kernelIndex, 
                                   const ICLDevBackendKernel_** OUT ppKernel) const
{
    if( !m_kernels.get() || m_kernels->Empty())
    {
        return CL_DEV_INVALID_OPERATION;
    }
    //TODO: exception handling

        *ppKernel = m_kernels->GetKernel(kernelIndex);
        return CL_DEV_SUCCESS; 
    }


void Program::SetBitCodeContainer(BitCodeContainer* bitCodeContainer)
{
    m_pCodeContainer = bitCodeContainer;
}

void Program::SetBuildLog( const std::string& buildLog )
{
    m_buildLog = buildLog;
}

void Program::SetKernelSet( KernelSet* pKernels) 
{ 
    m_kernels.reset(pKernels); 
}

void Program::SetModule( void* pModule)
{ 
    assert(m_pCodeContainer && "code container should be initialized by now");
    m_pCodeContainer->SetModule(pModule); 
}

bool Program::GetDisableOpt() const
{
    assert(m_pCodeContainer && "code container should be initialized by now");
    return m_pCodeContainer->GetProgramHeader()->bDisableOpt;
}

bool Program::GetDebugInfoFlag() const
{
    assert(m_pCodeContainer && "code container should be initialized by now");
    return m_pCodeContainer->GetProgramHeader()->bDebugInfo;
}

bool Program::GetFastRelaxedMath() const
{
    assert(m_pCodeContainer && "code container should be initialized by now");
    return m_pCodeContainer->GetProgramHeader()->bFastRelaxedMath;
}

bool Program::GetDAZ() const
{
    assert(m_pCodeContainer && "code container should be initialized by now");
    return m_pCodeContainer->GetProgramHeader()->bDemorsAreZero;
}


void Program::Release()
{
#ifdef OCL_DEV_BACKEND_PLUGINS  
    PluginManager::Instance().OnReleaseProgram(this);
#endif  
    delete this;
}


}}}