/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  TargetDescription.cpp

\*****************************************************************************/

#include "TargetDescription.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

unsigned long long int TargetDescription::GetFunctionAddress(const std::string& functionName) const
{
    std::map<std::string, unsigned long long int>::const_iterator it = m_functionsTable.find(functionName);
    if(m_functionsTable.end() == it)
    {
        return 0;
    }
    return it->second;
}

void TargetDescription::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
    Serializer::SerialPrimitive<unsigned int>(&m_cpuArch, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_cpuFeatures, ost);

    unsigned int mapCount = m_functionsTable.size();
    Serializer::SerialPrimitive<unsigned int>(&mapCount, ost);

    std::map<std::string, unsigned long long int>::const_iterator it;
    for (it = m_functionsTable.begin(); it != m_functionsTable.end(); it++)
    {
        Serializer::SerialString(it->first, ost);
        Serializer::SerialPrimitive<unsigned long long int>(&(it->second), ost);
    }
}

void TargetDescription::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    Serializer::DeserialPrimitive<unsigned int>(&m_cpuArch, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_cpuFeatures, ist);

    unsigned int mapCount = 0;
    Serializer::DeserialPrimitive<unsigned int>(&mapCount, ist);

    m_functionsTable.clear();
    for(unsigned int i = 0; i < mapCount; ++i)
    {
        std::string functionName;
        unsigned long long int functionAddress = 0;

        Serializer::DeserialString(functionName, ist);
        Serializer::DeserialPrimitive<unsigned long long int>(&functionAddress, ist);

        m_functionsTable[functionName] = functionAddress;
    }
}

}}} // namespace

