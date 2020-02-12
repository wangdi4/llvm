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
#include "CompilationUtils.h"
#include "Kernel.h"
#include "ObjectCodeContainer.h"
#include "Program.h"
#include "Serializer.h"
#include "cache_binary_handler.h"
#include "cl_device_api.h"
#include "exceptions.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
Program::Program():
    m_pObjectCodeContainer(nullptr),
    m_pIRCodeContainer(nullptr),
    m_kernels(nullptr)
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

const llvm::StringMap<size_t>& Program::GetGlobalVariableSizes() const
{
    return m_globalVariableSizes;
}

void Program::SetGlobalVariableSizes(const llvm::StringMap<size_t>& sizes)
{
    m_globalVariableSizes = sizes;
}

void Program::RecordCtorDtors(llvm::Module &M) {
    CompilationUtils::recordGlobalCtorDtors(M, m_globalCtors, m_globalDtors);
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

    // Global variables
    unsigned long long int tmp =
        (unsigned long long int)m_globalVariableTotalSize;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    unsigned int gvCount = (unsigned int)m_globalVariableSizes.size();
    Serializer::SerialPrimitive<unsigned int>(&gvCount, ost);
    for (auto &gv : m_globalVariableSizes)
    {
        std::string name = gv.first().str();
        Serializer::SerialString(name, ost);
        tmp = (unsigned long long int)gv.second;
        Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    }

    // Global Ctor Names
    unsigned int ctorCount = (unsigned int)m_globalCtors.size();
    Serializer::SerialPrimitive<unsigned int>(&ctorCount, ost);
    for (const std::string &ctor : m_globalCtors)
        Serializer::SerialString(ctor, ost);
    // Global Dtor Names
    unsigned int dtorCount = (unsigned int)m_globalDtors.size();
    Serializer::SerialPrimitive<unsigned int>(&dtorCount, ost);
    for (const std::string &dtor : m_globalDtors)
        Serializer::SerialString(dtor, ost);
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

    // Global variables
    unsigned long long int tmp;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_globalVariableTotalSize = (size_t)tmp;
    unsigned int gvCount;
    Serializer::DeserialPrimitive<unsigned int>(&gvCount, ist);
    for (unsigned int i = 0; i < gvCount; ++i)
    {
        std::string name;
        Serializer::DeserialString(name, ist);
        Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
        m_globalVariableSizes[name] = (size_t)tmp;
    }

    // Global Ctor Names
    unsigned int ctorCount;
    Serializer::DeserialPrimitive<unsigned int>(&ctorCount, ist);
    for (unsigned int i = 0; i < ctorCount; ++i)
    {
        std::string name;
        Serializer::DeserialString(name, ist);
        m_globalCtors.push_back(name);
    }
    // Global Dtor Names
    unsigned int dtorCount;
    Serializer::DeserialPrimitive<unsigned int>(&dtorCount, ist);
    for (unsigned int i = 0; i < dtorCount; ++i)
    {
        std::string name;
        Serializer::DeserialString(name, ist);
        m_globalDtors.push_back(name);
    }
}

bool Program::HasCachedExecutable() const
{
    if (!m_pObjectCodeContainer)
        return false;
    Intel::OpenCL::ELFUtils::CacheBinaryReader reader(
        m_pObjectCodeContainer->GetCode(),
        m_pObjectCodeContainer->GetCodeSize());
    return reader.IsCachedObject();
}
}}}
