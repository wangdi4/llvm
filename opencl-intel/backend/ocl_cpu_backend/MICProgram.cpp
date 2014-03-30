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

File Name:  MICProgram.cpp

\*****************************************************************************/
#include "MICProgram.h"
#include "MICKernel.h"
#include "MICSerializationService.h"
#include "IAbstractBackendFactory.h"
#include "MICSerializationService.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICProgram::MICProgram():
    Program(),
    m_pModuleJITHolder(NULL)
{
}

MICProgram::~MICProgram()
{
}

void MICProgram::SetModuleJITHolder( ModuleJITHolder* pModuleJITHolder)
{
    assert(pModuleJITHolder && "pModuleJITHolder is null");
    m_pModuleJITHolder.reset(pModuleJITHolder);
}

const ModuleJITHolder* MICProgram::GetModuleJITHolder() const
{
    return m_pModuleJITHolder.get();
}

const ICLDevBackendProgramJITCodeProperties* MICProgram::GetProgramJITCodeProperties() const
{
    return this;
}

size_t MICProgram::GetCodeSize() const
{
    return (size_t)m_pModuleJITHolder->GetJITCodeSize();
}

void MICProgram::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
    //Serializer::SerialPrimitive<unsigned long long int>(&m_programID, ost);
    // NOTICE: NO Need to serialize the LLVM bit code to the device

    Serializer::SerialString(m_buildLog, ost);

    Serializer::SerialPointerHint((const void**)&m_pModuleJITHolder, ost); 
    if(NULL != m_pModuleJITHolder.get())
    {
        m_pModuleJITHolder->Serialize(ost, stats);
    }

    unsigned int kernelsCount = m_kernels->GetCount();
    Serializer::SerialPrimitive<unsigned int>(&kernelsCount, ost);
    for(unsigned int i = 0; i < m_kernels->GetCount(); ++i)
    {
        MICKernel* currentKernel = static_cast<MICKernel*>(m_kernels->GetKernel(i));
        Serializer::SerialPointerHint((const void**)&currentKernel, ost); 
        if(NULL != currentKernel)
        {
            currentKernel->Serialize(ost, stats);
        }
    }
}

void MICProgram::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    //Serializer::DeserialPrimitive<unsigned long long int>(&m_programID, ist);
    m_pCodeContainer = NULL;
    Serializer::DeserialString(m_buildLog, ist);
   
    int* ptrHint;
    Serializer::DeserialPointerHint((void**)(&ptrHint), ist); 
    if(NULL != ptrHint)
    {
        m_pModuleJITHolder.reset(new ModuleJITHolder());
        m_pModuleJITHolder->Deserialize(ist, stats);
    }
    stats->SetPointerMark("pModuleJITHolder", m_pModuleJITHolder.get());

    unsigned int kernelsCount = 0;
    Serializer::DeserialPrimitive<unsigned int>(&kernelsCount, ist);
    m_kernels.reset(new KernelSet());
    for(unsigned int i = 0; i < kernelsCount; ++i)
    {
        MICKernel* currentKernel = NULL;
        Serializer::DeserialPointerHint((void**)(&currentKernel), ist); 
        if(NULL != currentKernel)
        {
            currentKernel = static_cast<MICKernel*>(stats->GetBackendFactory()->CreateKernel());
            currentKernel->Deserialize(ist, stats);
        }
        m_kernels->AddKernel(currentKernel);
    }
}


}}} // namespace
