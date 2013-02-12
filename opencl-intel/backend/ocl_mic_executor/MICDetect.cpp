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

File Name:  MICDetect.cpp

\*****************************************************************************/

#include "MICDetect.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

MICDetect::MICDetect(void) : m_CPUId(MIC_KNC, 0, true)
{
    // TODO-KNC: Call CPUID for detection of the MIC Card type
}

MICDetect::~MICDetect(void) 
{
}

MICDetect MICDetect::m_Instance;

}}}} // namespace
