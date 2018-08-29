// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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
    //TODO: This class should not know how to serialize CPUId
    unsigned Is64BitOS = m_cpuID.Is64BitOS() ? 1 : 0;
    unsigned CPU = m_cpuID.GetCPU();
    unsigned CPUFeatures = m_cpuID.GetCPUFeatureSupport();
    Serializer::SerialPrimitive(&CPU, ost);
    Serializer::SerialPrimitive(&CPUFeatures, ost);
    Serializer::SerialPrimitive(&Is64BitOS, ost);

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
    //TODO: This class should not know how to deserialize CPUId
    unsigned Is64BitOS;
    unsigned CPU;
    unsigned CPUFeatures;
    Serializer::DeserialPrimitive(&CPU, ist);
    Serializer::DeserialPrimitive(&CPUFeatures, ist);
    Serializer::DeserialPrimitive(&Is64BitOS, ist);
    m_cpuID = Intel::CPUId(Intel::ECPU(CPU),
            Intel::ECPUFeatureSupport(CPUFeatures),
            Is64BitOS==0 ? false : true);

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

