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

File Name:  CPUBuiltinLibrary.h

\*****************************************************************************/
#pragma once

#include "BuiltinModule.h"
#include "cl_dev_backend_api.h"
#include "CPUDetect.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUBuiltinLibrary : public BuiltinLibrary
{
public:
    CPUBuiltinLibrary(Intel::ECPU cpuId, unsigned int cpuFeatures) :
    BuiltinLibrary(cpuId, cpuFeatures) { }
    ~CPUBuiltinLibrary() { }

    void Load();
};

}}} // namespace
