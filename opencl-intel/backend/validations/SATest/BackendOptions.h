/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BackendOptions.h

\*****************************************************************************/
#ifndef BACKEND_OPTIONS_H
#define BACKEND_OPTIONS_H

#include "OpenCLRunConfiguration.h"
#if defined(INCLUDE_MIC_TARGET)
#include "DeviceCommunicationService.h"
#endif // INCLUDE_MIC_TARGET
#include "Exception.h"

#include "cl_dev_backend_api.h"
#include <string.h>
#include <assert.h>

namespace Validation
{

class CPUBackendOptions: public ICLDevBackendOptions
{
public:
    CPUBackendOptions() {}

    virtual ~CPUBackendOptions() {}

    void InitFromRunConfiguration(const BERunOptions& runConfig)
    {
        m_transposeSize = runConfig.GetValue<Intel::OpenCL::DeviceBackend::ETransposeSize>(RC_BR_TRANSPOSE_SIZE, TRANSPOSE_SIZE_AUTO);
        m_cpu           = runConfig.GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "auto");
        m_cpuFeatures   = runConfig.GetValue<std::string>(RC_BR_CPU_FEATURES, "");
        m_useVTune      = runConfig.GetValue<bool>(RC_BR_USE_VTUNE, false);

        m_DumpIROptionAfter = runConfig.GetValue<const std::vector<IRDumpOptions> * >
                                (RC_BR_DUMP_IR_AFTER, 0);
        m_DumpIROptionBefore = runConfig.GetValue<const std::vector<IRDumpOptions> * >
                                (RC_BR_DUMP_IR_BEFORE, 0);

        m_DumpIRDir = runConfig.GetValue<std::string>(RC_BR_DUMP_IR_DIR, "");
        m_TimePasses = runConfig.GetValue<std::string>(RC_BR_TIME_PASSES, "");
    }

    virtual void InitTargetDescriptionSession(ICLDevBackendExecutionService* pExecutionService)
    {
    }

    bool GetBooleanValue(int optionId, bool defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_USE_VTUNE :
            return m_useVTune;
        default:
            return defaultValue;
        }
    }

    virtual int GetIntValue( int optionId, int defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
            return m_transposeSize;
        default:
             return defaultValue;
        }
    }

    virtual const char* GetStringValue(int optionId, const char* defaultValue)const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_CPU_ARCH :
            return m_cpu.c_str();
        case CL_DEV_BACKEND_OPTION_CPU_FEATURES:
            return m_cpuFeatures.c_str();
        case CL_DEV_BACKEND_OPTION_DUMP_IR_DIR:
            return m_DumpIRDir.c_str();
        case CL_DEV_BACKEND_OPTION_TIME_PASSES:
            return m_TimePasses.c_str();
        default:
            return defaultValue;
        }
    }

    virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
    {
        if (Value == NULL)
        {
            throw Exception::InvalidArgument("Value is not initialized");
        }
        switch(optionId)
        {
        case OPTION_IR_DUMPTYPE_AFTER :
            *(static_cast<const std::vector<IRDumpOptions>* * >(Value)) = m_DumpIROptionAfter;
            return true;
        case OPTION_IR_DUMPTYPE_BEFORE :
            *(static_cast<const std::vector<IRDumpOptions>* * >(Value)) = m_DumpIROptionBefore;
            return true;
        default:
            assert(false && "Unknown option");
            return false;
        }
    }

protected:

    Intel::OpenCL::DeviceBackend::ETransposeSize m_transposeSize;
    std::string    m_cpu;
    std::string    m_cpuFeatures;
    bool           m_useVTune;
    const std::vector<IRDumpOptions>* m_DumpIROptionAfter;
    const std::vector<IRDumpOptions>* m_DumpIROptionBefore;
    std::string m_DumpIRDir;
    std::string m_TimePasses;
};

#if defined(INCLUDE_MIC_TARGET)
class MICBackendOptions: public ICLDevBackendOptions
{
public:
    void InitFromRunConfiguration(const BERunOptions& runConfig)
    {
        m_transposeSize = runConfig.GetValue<Intel::OpenCL::DeviceBackend::ETransposeSize>(RC_BR_TRANSPOSE_SIZE, TRANSPOSE_SIZE_AUTO);
        m_cpu           = runConfig.GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "auto-remote");
        m_cpuFeatures   = runConfig.GetValue<std::string>(RC_BR_CPU_FEATURES, "");
        m_useVTune      = runConfig.GetValue<bool>(RC_BR_USE_VTUNE, false);
        m_fileName      = runConfig.GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "-");
    }

    bool GetBooleanValue(int optionId, bool defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_USE_VTUNE :
            return m_useVTune;
        default:
            return defaultValue;
        }
    }

    void InitTargetDescriptionSession(size_t targetDescriptionSize, const char* targetDescription)
    {
        if(!isMIC()) return;
        m_targetDescSize = targetDescriptionSize;

        if(0 != m_targetDescSize)
        {
            m_pTargetDesc.resize(m_targetDescSize);
            memcpy(&(m_pTargetDesc[0]), targetDescription, m_targetDescSize);
        }
    }

    virtual int GetIntValue( int optionId, int defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_TARGET_DESC_SIZE:
            return (int)m_targetDescSize;
        case CL_DEV_BACKEND_OPTION_TRANSPOSE_SIZE:
            return (int)m_transposeSize;
        default:
            return defaultValue;
        }
    }

    virtual const char* GetStringValue(int optionId, const char* defaultValue)const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_CPU_ARCH :
            return m_cpu.c_str();
        case CL_DEV_BACKEND_OPTION_CPU_FEATURES:
            return m_cpuFeatures.c_str();
        case CL_DEV_BACKEND_OPTION_DUMPFILE :
            return m_fileName.c_str();
        default:
            return defaultValue;
        }
    }

    virtual void SetStringValue(int optionId, const char* value)
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_CPU_ARCH :
            m_cpu = std::string(value);
        case CL_DEV_BACKEND_OPTION_CPU_FEATURES:
            m_cpuFeatures = std::string(value);
        default:
            return;
        }
    }

    virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
    {
        if (Value == NULL)
        {
            throw Exception::InvalidArgument("Value is not initialized");
        }
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_TARGET_DESC_BLOB:
            if(*pSize < m_targetDescSize) return false;
            memcpy(Value, &m_pTargetDesc[0], m_targetDescSize);
            return true;
        default:
            return false;
        }
    }

private:
    bool isMIC()
    {
        return (0 == m_cpu.compare(0, 3, "knc")) || (0 == m_cpu.compare(0, 3, "knf")) || (0 == m_cpu.compare(0, 11, "auto-remote"));
    }

    size_t              m_targetDescSize;
    std::vector<char>   m_pTargetDesc;

    Intel::OpenCL::DeviceBackend::ETransposeSize m_transposeSize;
    std::string    m_cpu;
    std::string    m_cpuFeatures;
    bool           m_useVTune;
    std::string    m_fileName;
};
#endif // INCLUDE_MIC_TARGET

/**
 * Description of ENABLE_SDE mode for MIC:
 * In the initialization flow for MIC, we need from the device (Target 
 * Description), this is a buffer of target data which will contain some 
 * info about the device capabilities and "execution context" (for now 
 * execution context can be SVML function addresses), this needed by the 
 * Compiler in order to generate the JIT and link it with the addresses 
 * given from the device.
 */

class SDEBackendOptions: public CPUBackendOptions
{
public:
    SDEBackendOptions() : m_targetDescSize(0), m_pTargetDesc(NULL) {}
    
    virtual ~SDEBackendOptions()
    {
        delete[] m_pTargetDesc;
    }

    SDEBackendOptions(const SDEBackendOptions& options)
    {
        copy(options);
    }

    SDEBackendOptions& operator=(const SDEBackendOptions& options)
    {
        delete[] m_pTargetDesc;
        copy(options);
        return *this;
    }

    virtual void InitTargetDescriptionSession(ICLDevBackendExecutionService* pExecutionService)
    {
        if(!isMIC()) return ;
        m_targetDescSize = pExecutionService->GetTargetMachineDescriptionSize();

        if(0 != m_targetDescSize)
        {
            m_pTargetDesc = new char[m_targetDescSize];
            pExecutionService->GetTargetMachineDescription(m_pTargetDesc, m_targetDescSize);
        }
    }

    virtual int GetIntValue( int optionId, int defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_TARGET_DESC_SIZE:
            return m_targetDescSize;
        default:
            return CPUBackendOptions::GetIntValue(optionId, defaultValue);
        }
    }

    virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
    {
        if (Value == NULL)
        {
            throw Exception::InvalidArgument("Value is not initialized");
        }
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_TARGET_DESC_BLOB:
            if(*pSize < m_targetDescSize) return false;
            memcpy(Value, m_pTargetDesc, m_targetDescSize);
            return true;
        default:
            return CPUBackendOptions::GetValue(optionId, Value, pSize);
        }
    }


private:
    bool isMIC()
    {
        return (0 == m_cpu.compare(0, 3, "knf"));
    }

    void copy(const SDEBackendOptions& options)
    {
        m_targetDescSize = options.m_targetDescSize;
        m_pTargetDesc = NULL;

        if(0 != m_targetDescSize)
        {
            m_pTargetDesc = new char[m_targetDescSize];
            memcpy(m_pTargetDesc, options.m_pTargetDesc, m_targetDescSize);
        }
    }

private:
    size_t m_targetDescSize;
    char*  m_pTargetDesc;
};

/**
 * Options used during program code container dump
 */
class ProgramDumpConfig: public ICLDevBackendOptions
{
public:

    ProgramDumpConfig(const BERunOptions* runConfig)
    {
        m_fileName = runConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "-");
    }

    bool GetBooleanValue(int optionId, bool defaultValue) const
    {
        return defaultValue;
    }

    virtual int GetIntValue( int optionId, int defaultValue) const
    {
        return defaultValue;
    }

    virtual const char* GetStringValue(int optionId, const char* defaultValue)const
    {
        if( CL_DEV_BACKEND_OPTION_DUMPFILE != optionId )
        {
            return defaultValue;
        }

        return m_fileName.c_str();
    }

    virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
    {
        return false;
    }

private:
    std::string m_fileName;
};

}

#endif // BACKEND_OPTIONS_H

