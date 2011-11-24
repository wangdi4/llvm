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

File Name:  CompilerConfig.h

\*****************************************************************************/
#pragma once

#include <assert.h>
#include <string>
#include "cl_dev_backend_api.h"
#include <vector>
#include <algorithm>

namespace Intel { namespace OpenCL { namespace DeviceBackend {


//*****************************************************************************************
// Abstract compiler configuration interface
// 
class CompilerConfig
{
public:
    CompilerConfig():m_transposeSize(TRANSPOSE_SIZE_AUTO),m_useVTune(false),
                    m_DumpIROptionAfter(NULL), m_DumpIROptionBefore(NULL) {}
    ~CompilerConfig(){}

    // CompilerConfig interface methods
	const std::string& GetBackendMode() const { return m_backendMode; }
    const std::string& GetCpuArch() const     { return m_cpuArch; }
    const std::string& GetCpuFeatures() const { return m_cpuFeatures; }
    ETransposeSize GetTransposeSize() const   { return m_transposeSize; }
    bool  GetUseVTune() const                 { return m_useVTune; }

    std::vector<int> GetIRDumpOptionsAfter() const
    {
        if(!m_DumpIROptionAfter){
            std::vector<int> tempVecotr;
            return tempVecotr;
        }
        std::vector<int> optionsVector(m_DumpIROptionAfter->begin(), m_DumpIROptionAfter->end());
        //sort the vector for later use (binary_search)
        std::sort(optionsVector.begin(), optionsVector.end());
        return optionsVector;
    }
    std::vector<int> GetIRDumpOptionsBefore() const
    {
       if(!m_DumpIROptionBefore){
            std::vector<int> tempVecotr;
            return tempVecotr;
        }
        std::vector<int> optionsVector(m_DumpIROptionBefore->begin(), m_DumpIROptionBefore->end());
        //sort the vector for later use (binary_search)
        std::sort(optionsVector.begin(), optionsVector.end());
        return optionsVector;
    }
    const std::string& GetDumpIRDir() const { return m_dumpIRDir; }
    const std::string& GetTimePasses() const { return m_TimePasses; }

protected:
	std::string m_backendMode;
    std::string m_cpuArch;
    std::string m_cpuFeatures;
    ETransposeSize m_transposeSize;
    bool        m_useVTune;
    const std::vector<IRDumpOptions>* m_DumpIROptionAfter;
    const std::vector<IRDumpOptions>* m_DumpIROptionBefore;
    std::string m_dumpIRDir;
    std::string m_TimePasses;
};

}}}