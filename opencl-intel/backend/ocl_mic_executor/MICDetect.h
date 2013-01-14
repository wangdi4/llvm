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

File Name:  MICDetect.h

\*****************************************************************************/

#pragma once
#include "TargetArch.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

// This class used in order to detect the MIC card type with it's features
class MICDetect
{
public:

    static MICDetect *  GetInstance() { return &m_Instance; }
    const CPUId      &  GetCPUId() const { return m_CPUId; }  
private:
    MICDetect(void);
    ~MICDetect(void);

    CPUId m_CPUId;

    static MICDetect m_Instance;
};

}}}} // namespace
