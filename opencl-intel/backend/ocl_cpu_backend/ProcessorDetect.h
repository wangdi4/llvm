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

File Name:  ProcessorDetect.h

\*****************************************************************************/

#pragma once
#include "TargetArch.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {


/**
 * This class serves as database for all the supported processors with some basic
 * properties
 */
class ProcessorDetect
{
public:
    ProcessorDetect();
    virtual ~ProcessorDetect();
protected:
    const char* m_CPUNames[CPU_LAST];
    const char* m_CPUPrefixes[CPU_LAST];
};

}}}} // namespace
