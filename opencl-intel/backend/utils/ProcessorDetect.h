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

#ifndef __PROCESSOR_DETECT_H__
#define __PROCESSOR_DETECT_H__

#include "TargetArch.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {


/**
 * This class serves as database for all the supported processors with some basic
 * properties
 */
class ProcessorDetect
{
public:
    ProcessorDetect() {};
    virtual ~ProcessorDetect() {};
};

}}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

#endif // __PROCESSOR_DETECT_H__