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

File Name:  ProcessorDetect.cpp

\*****************************************************************************/

#include "ProcessorDetect.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

ProcessorDetect::ProcessorDetect()
{
    m_CPUNames[CPU_PENTIUM]     = "pentium";
	m_CPUNames[CPU_NOCONA]      = "nicona";
	m_CPUNames[CPU_CORE2]       = "core2";
	m_CPUNames[CPU_PENRYN]      = "penryn";
	m_CPUNames[CPU_COREI7]      = "corei7";
	m_CPUNames[CPU_SANDYBRIDGE] = "sandybridge";
	m_CPUNames[CPU_HASWELL]     = "haswell";
    m_CPUNames[MIC_KNIGHTSFERRY] = "knf";

#if !defined(_M_X64) && !defined(__LP64__)
	m_CPUPrefixes[CPU_PENTIUM] = "w7";
	m_CPUPrefixes[CPU_NOCONA] = "t7";
	m_CPUPrefixes[CPU_CORE2] = "v8";
	m_CPUPrefixes[CPU_PENRYN] = "p8";
	m_CPUPrefixes[CPU_COREI7] = "n8";
	m_CPUPrefixes[CPU_SANDYBRIDGE] = "g9";
	m_CPUPrefixes[CPU_HASWELL] = "g9";
    m_CPUPrefixes[MIC_KNIGHTSFERRY] = "b1";
#else
	m_CPUPrefixes[CPU_PENTIUM] = "unknown";
	m_CPUPrefixes[CPU_NOCONA] = "e7";
	m_CPUPrefixes[CPU_CORE2] = "u8";
	m_CPUPrefixes[CPU_PENRYN] = "y8";
	m_CPUPrefixes[CPU_COREI7] = "h8";
	m_CPUPrefixes[CPU_SANDYBRIDGE] = "e9";
	m_CPUPrefixes[CPU_HASWELL] = "e9";
    m_CPUPrefixes[MIC_KNIGHTSFERRY] = "b1";
#endif
}

ProcessorDetect::~ProcessorDetect()
{
}

}}}} // namespace
