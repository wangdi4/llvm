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
    m_pModuleJITHolder(nullptr)
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

ModuleJITHolder* MICProgram::GetModuleJITHolder()
{
    return m_pModuleJITHolder.get();
}

const ICLDevBackendProgramJITCodeProperties* MICProgram::GetProgramJITCodeProperties() const
{
    return this;
}

size_t MICProgram::GetJITCodeSize() const
{
    return (size_t)m_pModuleJITHolder->GetJITCodeSize();
}

void MICProgram::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
    //Serializer::SerialPrimitive<unsigned long long int>(&m_programID, ost);
    // NOTICE: NO Need to serialize the LLVM bit code to the device

    Serializer::SerialPointerHint((const void**)&m_pModuleJITHolder, ost); 
    if(nullptr != m_pModuleJITHolder.get())
    {
        m_pModuleJITHolder->Serialize(ost, stats);
    }

    Program::Serialize(ost, stats);
}

void MICProgram::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    //Serializer::DeserialPrimitive<unsigned long long int>(&m_programID, ist);

    int* ptrHint;
    Serializer::DeserialPointerHint((void**)(&ptrHint), ist); 
    if(nullptr != ptrHint)
    {
        m_pModuleJITHolder.reset(new ModuleJITHolder());
        m_pModuleJITHolder->Deserialize(ist, stats);
    }
    stats->SetPointerMark("pModuleJITHolder", m_pModuleJITHolder.get());

    Program::Deserialize(ist, stats);
}


}}} // namespace
