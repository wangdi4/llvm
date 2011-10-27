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

File Name:  MICKernel.cpp

\*****************************************************************************/

#include "MICKernel.h"
#include "MICJITContainer.h"
#include <stdio.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {


void MICKernel::SetKernelID(unsigned long long int kernelID)
{
    m_kernelID = kernelID;
}

unsigned long long int MICKernel::GetKernelID() const
{
    return m_kernelID;
}

void MICKernel::Serialize(IOutputStream& ost, SerializationStatus* stats)
{
    Serializer::SerialPrimitive<unsigned long long int>(&m_kernelID, ost);
    Serializer::SerialString(m_name, ost);

    // Serial the kernel arguments (one by one)
    unsigned int vectorSize = m_args.size();
    Serializer::SerialPrimitive<unsigned int>(&vectorSize, ost);
    for(std::vector<cl_kernel_argument>::const_iterator it = m_args.begin(); it != m_args.end(); ++it)
    {
        cl_kernel_argument currentArgument = *it;
        Serializer::SerialPrimitive<cl_kernel_arg_type>(&currentArgument.type, ost);
        Serializer::SerialPrimitive<unsigned int>(&currentArgument.size_in_bytes, ost);
    }

    Serializer::SerialPointerHint((const void**)&m_pProps, ost); 
    if(NULL != m_pProps)
    {
        static_cast<MICKernelProperties*>(m_pProps)->Serialize(ost, stats);
    }

    // Serial the kernel JIT's (one by one)
    vectorSize = m_JITs.size();
    Serializer::SerialPrimitive<unsigned int>(&vectorSize, ost);
    for(std::vector<IKernelJITContainer*>::const_iterator it = m_JITs.begin(); it != m_JITs.end(); ++it)
    {
        MICJITContainer* currentArgument = (MICJITContainer*)(*it);
        Serializer::SerialPointerHint((const void**)&currentArgument, ost); 
        if(NULL != currentArgument)
        {
            currentArgument->Serialize(ost, stats);
        }
    }
}

void MICKernel::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    Serializer::DeserialPrimitive<unsigned long long int>(&m_kernelID, ist);
    Serializer::DeserialString(m_name, ist);

    // Deserial the kernel arguments (one by one)
    unsigned int vectorSize = 0;
    Serializer::DeserialPrimitive<unsigned int>(&vectorSize, ist);
    for(unsigned int i = 0; i < vectorSize; ++i)
    {
        cl_kernel_argument currentArgument;
        Serializer::DeserialPrimitive<cl_kernel_arg_type>(&currentArgument.type, ist);
        Serializer::DeserialPrimitive<unsigned int>(&currentArgument.size_in_bytes, ist);
        m_args.push_back(currentArgument);
    }

    Serializer::DeserialPointerHint((void**)&m_pProps, ist);
    if(NULL != m_pProps)
    {
        m_pProps = new MICKernelProperties();
        static_cast<MICKernelProperties*>(m_pProps)->Deserialize(ist, stats);
    }

    Serializer::DeserialPrimitive<unsigned int>(&vectorSize, ist);
    for(unsigned int i = 0; i < vectorSize; ++i)
    {
        MICJITContainer* currentArgument = NULL;
        Serializer::DeserialPointerHint((void**)&currentArgument, ist); 
        if(NULL != currentArgument)
        {
            currentArgument = new MICJITContainer();
            currentArgument->Deserialize(ist, stats);
        }
        m_JITs.push_back(currentArgument);
    }
}


}}} // namespace
