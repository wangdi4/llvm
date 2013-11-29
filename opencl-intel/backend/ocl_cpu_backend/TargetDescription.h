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

File Name:  TargetDescription.h

\*****************************************************************************/

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

