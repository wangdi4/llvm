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

File Name:  MICJITContainer.cpp

\*****************************************************************************/

#include "MICJITContainer.h"
#include "MICKernelProperties.h"
#include "Serializer.h"
#include "MICSerializationService.h"
#include "IAbstractBackendFactory.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICJITContainer::MICJITContainer(const ModuleJITHolder* pModuleHolder,
                                 unsigned long long int funcID,
                                 KernelJITProperties* pProps):
    m_pModuleJITHolder(pModuleHolder),
    m_funcID(funcID),
    m_pProps(pProps) // get ownership of the pProps pointer
{
    assert(m_pModuleJITHolder && "Module JIT Holder Cannot be NULL");
}

const void* MICJITContainer::GetJITCode() const 
{
    const char* pModuleJIT = (const char*)m_pModuleJITHolder->GetJITCodeStartPoint();
    // TODO: Check what the raised Exception when the ID is not valid
    int kernelEntryPoint = m_pModuleJITHolder->GetKernelEntryPoint(m_funcID);
    return pModuleJIT + kernelEntryPoint;
}

int MICJITContainer::GetLineNumber(void* pointer) const
{
    // Pointers are guaranteed to fit in 'unsigned long long'.
    unsigned long long address = (unsigned long long)pointer;
    unsigned long long kernelAddress = (unsigned long long)GetJITCode();
    if (address < kernelAddress || address > kernelAddress + GetJITCodeSize())
    {
        // This doesn't point to an instruction in this kernel.
        return -1;
    }

    int offset = address - kernelAddress;
    const LineNumberTable& table =
        *(m_pModuleJITHolder->GetKernelLineNumberTable(m_funcID));

    // The table is arranged in the same form as VTune expects; so the following:
    //
    //    Offset LineNumber
    //     1       2
    //     12      4
    //     15      2
    //     18      1
    //     21      30
    //
    // Is understood as:
    //
    //    Code subrange  Line number
    //     0-1             2
    //     1-12            4
    //     12-15           2
    //     15-18           1
    //     18-21           30
    //
    // That means that we always pair the *last* offset to the line number,
    // hence the '<=' condition below.

    for (LineNumberTable::const_iterator iter = table.begin(), end = table.end();
        iter != end; iter++)
    {
        if (offset <= iter->offset) return iter->offset;
    }
    return -1;
}

MICJITContainer::~MICJITContainer()
{
    delete m_pProps;
}

void MICJITContainer::Serialize(IOutputStream& ost, SerializationStatus* stats)
{
    Serializer::SerialPrimitive<unsigned long long int>(&m_funcID, ost);

    Serializer::SerialPointerHint((const void**)&m_pProps, ost);
    if(NULL != m_pProps)
    {
        static_cast<MICKernelJITProperties*>(m_pProps)->Serialize(ost, stats);
    }
}

void MICJITContainer::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    Serializer::DeserialPrimitive<unsigned long long int>(&m_funcID, ist);

    Serializer::DeserialPointerHint((void**)&m_pProps, ist);
    if(NULL != m_pProps)
    {
        m_pProps =  static_cast<MICKernelJITProperties*>(stats->GetBackendFactory()->CreateKernelJITProperties());
        static_cast<MICKernelJITProperties*>(m_pProps)->Deserialize(ist, stats);
    }

    m_pModuleJITHolder = (const ModuleJITHolder*)stats->GetPointerMark("pModuleJITHolder");
}

}}} // namespace
