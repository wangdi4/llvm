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

File Name:  KernelProperties.cpp

\*****************************************************************************/
#include "MICKernelProperties.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICKernelProperties::MICKernelProperties(KernelProperties* pKernelProps)
{
    SetOptWGSize(pKernelProps->GetOptWGSize());
    SetReqdWGSize(pKernelProps->GetReqdWGSize());
    SetHintWGSize(pKernelProps->GetHintWGSize());
    SetTotalImplSize(pKernelProps->GetImplicitLocalMemoryBufferSize());
    SetDAZ( pKernelProps->GetDAZ());
    SetCpuId( pKernelProps->GetCpuId() );
    SetCpuFeatures( pKernelProps->GetCpuFeatures() );
    SetPrivateMemorySize(pKernelProps->GetPrivateMemorySize());
}

void MICKernelJITProperties::Serialize(IOutputStream& ost, SerializationStatus* stats)
{
    Serializer::SerialPrimitive<bool>(&m_useVTune, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_VTuneId, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_stackSize, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_vectorSize, ost);
}

void MICKernelJITProperties::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    Serializer::DeserialPrimitive<bool>(&m_useVTune, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_VTuneId, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_stackSize, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_vectorSize, ist);
}

void MICKernelProperties::Serialize(IOutputStream& ost, SerializationStatus* stats)
{
    // Need to revert dbgPrint flag
    //Serializer::SerialPrimitive<bool>(&m_dbgPrint, ost);
    Serializer::SerialPrimitive<bool>(&m_DAZ, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_optWGSize, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_cpuId, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_cpuFeatures, ost);

    // using unsigned long long int instead of size_t is because that size_t
    // varies in it's size relating to the platform (32/64 bit)
    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        unsigned long long int tmp = (unsigned long long int)m_reqdWGSize[i];
        Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    }
    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        unsigned long long int tmp = (unsigned long long int)m_hintWGSize[i];
        Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    }

    unsigned long long int tmp = (unsigned long long int)m_totalImplSize;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    tmp = (unsigned long long int)m_privateMemorySize;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
}

void MICKernelProperties::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    // Need to revert dbgPrint flag
    //Serializer::DeserialPrimitive<bool>(&m_dbgPrint, ist);
    Serializer::DeserialPrimitive<bool>(&m_DAZ, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_optWGSize, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_cpuId, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_cpuFeatures, ist);

    // using unsigned long long int instead of size_t is because that size_t
    // varies in it's size relating to the platform (32/64 bit)
    unsigned long long int tmp;
    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
        m_reqdWGSize[i] = (size_t)tmp;
    }
    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
        m_hintWGSize[i] = (size_t)tmp;
    }

    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_totalImplSize = (size_t)tmp;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_privateMemorySize = (size_t)tmp;
}

}}} // namespace
