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
#include "ProcessorDetect.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

// This class used in order to detect the MIC card type with it's features
class MICDetect : public ProcessorDetect
{
public:

    static MICDetect *  GetInstance() { return &m_Instance; }
    unsigned            GetMICFeatureSupport() const { return m_uiCPUFeatures; }
    const char*         GetMICPrefix() const { return m_CPUPrefixes[m_CPU]; }
    const char*         GetMICPrefix(ECPU CPU) const { return m_CPUPrefixes[CPU]; }
    ECPU                GetMICId() const { return m_CPU; }  
  
private:
    MICDetect(void);
    ~MICDetect(void);

    unsigned int m_uiCPUFeatures;
    ECPU m_CPU;

    static MICDetect m_Instance;
};

}}}} // namespace
