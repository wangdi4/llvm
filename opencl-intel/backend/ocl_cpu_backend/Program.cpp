// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#include "BitCodeContainer.h"
#include "Kernel.h"
#include "Program.h"
#include "cl_device_api.h"
#include "exceptions.h"
#include "ObjectCodeContainer.h"
#include "Serializer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
Program::Program():
    m_pObjectCodeContainer(nullptr),
    m_pIRCodeContainer(nullptr),
    m_kernels(nullptr),
    m_globalVariableTotalSize(0)
{}

Program::~Program()
{
    m_kernels.reset(nullptr);

    delete m_pObjectCodeContainer;
    delete m_pIRCodeContainer;
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

const ICLDevBackendCodeContainer* Program::GetProgramIRCodeContainer() const
{
    return m_pIRCodeContainer;
}

const ICLDevBackendCodeContainer* Program::GetProgramCodeContainer() const
{
    return m_pObjectCodeContainer ? m_pObjectCodeContainer: GetProgramIRCodeContainer();
}

const ICLDevBackendProgramJITCodeProperties* Program::GetProgramJITCodeProperties() const
{
    assert(false && "NotImplemented");
    return nullptr;
}

cl_dev_err_code Program::GetKernelByName(const char* IN pKernelName,
                                         const ICLDevBackendKernel_** OUT pKernel) const
{
    if( !m_kernels.get() || m_kernels->Empty())
    {
        return CL_DEV_INVALID_KERNEL_NAME;
    }

    try
    {
        *pKernel = (ICLDevBackendKernel_*)m_kernels->GetKernel(pKernelName);
        return CL_DEV_SUCCESS;
    }
    catch(Exceptions::DeviceBackendExceptionBase& )
    {
        return CL_DEV_INVALID_KERNEL_NAME;
    }
}

int Program::GetKernelsCount() const
{
    if(!m_kernels.get())
    {
        return 0;
    }

    return m_kernels->GetCount();
}

int Program::GetNonBlockKernelsCount() const
{
    if(!m_kernels.get())
    {
        return 0;
    }

    return m_kernels->GetCount() - m_kernels->GetBlockCount();
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

size_t Program::GetGlobalVariableTotalSize() const
{
    return m_globalVariableTotalSize;
}

void Program::SetGlobalVariableTotalSize(size_t size)
{
    m_globalVariableTotalSize = size;
}

void Program::SetObjectCodeContainer(ObjectCodeContainer* pObjCodeContainer)
{
    delete m_pObjectCodeContainer;
    m_pObjectCodeContainer = pObjCodeContainer;
}

ObjectCodeContainer* Program::GetObjectCodeContainer()
{
    return m_pObjectCodeContainer;
}

void Program::SetBitCodeContainer(BitCodeContainer* bitCodeContainer)
{
    delete m_pIRCodeContainer;
    m_pIRCodeContainer = bitCodeContainer;
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
    assert(m_pIRCodeContainer && "code container should be initialized by now");
    m_pIRCodeContainer->SetModule(pModule);
}

void* Program::GetModule()
{
    assert(m_pIRCodeContainer && "code container should be initialized by now");
    return m_pIRCodeContainer->GetModule();
}

void Program::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
    Serializer::SerialString(m_buildLog, ost);

    unsigned int kernelsCount = m_kernels->GetCount();
    Serializer::SerialPrimitive<unsigned int>(&kernelsCount, ost);
    for(unsigned int i = 0; i < m_kernels->GetCount(); ++i)
    {
        Kernel* currentKernel = m_kernels->GetKernel(i);
        Serializer::SerialPointerHint((const void**)&currentKernel, ost);
        if(nullptr != currentKernel)
        {
            currentKernel->Serialize(ost, stats);
        }
    }
    unsigned long long int tmp = (unsigned long long int)m_globalVariableTotalSize;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
}

void Program::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    Serializer::DeserialString(m_buildLog, ist);

    unsigned int kernelsCount = 0;
    Serializer::DeserialPrimitive<unsigned int>(&kernelsCount, ist);
    m_kernels.reset(new KernelSet());
    for(unsigned int i = 0; i < kernelsCount; ++i)
    {
        Kernel* currentKernel = nullptr;
        Serializer::DeserialPointerHint((void**)(&currentKernel), ist);
        if(nullptr != currentKernel)
        {
            currentKernel = stats->GetBackendFactory()->CreateKernel();
            currentKernel->Deserialize(ist, stats);
            m_kernels->AddKernel(currentKernel);
        }
    }
    unsigned long long int tmp;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_globalVariableTotalSize = (size_t)tmp;
}
}}}
