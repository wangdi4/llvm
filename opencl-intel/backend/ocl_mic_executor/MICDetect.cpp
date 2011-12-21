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
#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

MICDetect::MICDetect(void) : m_uiCPUFeatures(0)
{
    // TODO: Call CPUID for detection of the MIC Card type
    m_CPU = MIC_KNIGHTSFERRY;
    assert(m_CPU!=CPU_LAST && "Unknown CPU");
}

MICDetect::~MICDetect(void) 
{
}

MICDetect MICDetect::m_Instance;

}}}} // namespace
