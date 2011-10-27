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

File Name:  CPUExecutable.h

\*****************************************************************************/

#pragma once

#include "Executable.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUExecutable : public Executable
{
public:
    CPUExecutable(const Binary* pBin);
    
    // Prepares current thread for the executable execution
    virtual cl_dev_err_code PrepareThread();

    // Restores Thread state as it was before the execution
    virtual cl_dev_err_code RestoreThreadState();

private:
    bool            m_hasAVX1;         // Are we running on a CPU supporting AVX?
    bool            m_hasAVX2;        // Running on Haswell
};

}}} // namespace
