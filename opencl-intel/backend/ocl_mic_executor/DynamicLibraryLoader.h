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

File Name:  DynamicLibraryLoader.h

\*****************************************************************************/
#pragma once

#include "TargetArch.h"
#include <map>
#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class DynamicLibraryLoader
{
public:
    DynamicLibraryLoader();
    ~DynamicLibraryLoader();

    void SetTargetArch(Intel::ECPU cpuId, unsigned int cpuFeatures);

    void Load();
    void GetLibraryFunctions(std::map<std::string, unsigned long long int>& functionsTable);

private:
    Intel::ECPU m_cpuId;
    unsigned int m_cpuFeatures;

    void* m_pLibHandle;
};

}}} // namespace
