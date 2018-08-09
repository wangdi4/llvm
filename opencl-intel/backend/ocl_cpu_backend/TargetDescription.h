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

#pragma once
#include "TargetArch.h"
#include "IDynamicFunctionsResolver.h"
#include "Serializer.h"

#include <map>
#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class SerializationStatus;

class TargetDescription : public IDynamicFunctionsResolver
{
public:
    TargetDescription() {}
    virtual ~TargetDescription() {}

    void SetCPUId(const Intel::CPUId &cpuID)
    {
        m_cpuID = cpuID;
    }

    const Intel::CPUId &GetCPUId() { return m_cpuID; }

    void SetFunctionsTable(const std::map<std::string, unsigned long long int>& functionsTable)
    {
        m_functionsTable = functionsTable;
    }

    void SetFunctionAddress(const std::string& functionName, unsigned long long int address)
    {
        m_functionsTable[functionName] = address;
    }

    unsigned long long int GetFunctionAddress(const std::string& functionName) const;

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    void Deserialize(IInputStream& ist, SerializationStatus* stats);

private:
    // cpu details
    Intel::CPUId m_cpuID;

    // maps between function name to it's address
    std::map<std::string, unsigned long long int> m_functionsTable;
};

}}} // namespace

