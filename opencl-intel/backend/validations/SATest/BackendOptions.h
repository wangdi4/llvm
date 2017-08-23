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
#include "Exception.h"

#if defined(SATEST_INCLUDE_MIC_DEVICE)
#include "MICNative/common.h"
#endif

#include "cl_dev_backend_api.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

namespace Validation
{

class GlobalBackendOptions: public ICLDevBackendOptions
{
public:
    void InitFromRunConfiguration(const BERunOptions& runConfig)
    {
        m_TimePasses = runConfig.GetValue<std::string>(RC_BR_TIME_PASSES, "");
        m_DisableStackDump = runConfig.GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false);
    }


    const char* GetStringValue(int optionId, const char* defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_TIME_PASSES:
            return m_TimePasses.c_str();
        default:
            return defaultValue;
        }
    }

    bool GetBooleanValue(int optionId, bool defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_DISABLE_STACKDUMP:
            return m_DisableStackDump;
        default:
            return defaultValue;
        }
        return defaultValue;
    }

    virtual int GetIntValue( int optionId, int defaultValue) const
    {
        return defaultValue;
    }

    virtual bool GetValue(int optionId, void* Value, size_t* pSize) const
    {
        return false;
    }

private:
    std::string m_TimePasses;
    bool        m_DisableStackDump;
};



class CPUBackendOptions: public ICLDevBackendOptions
{
public:
    CPUBackendOptions() {}

    virtual ~CPUBackendOptions() {}

    void InitFromRunConfiguration(const BERunOptions& runConfig)
    {
        m_transposeSize = runConfig.GetValue<Intel::OpenCL::DeviceBackend::ETransposeSize>(RC_BR_TRANSPOSE_SIZE, TRANSPOSE_SIZE_NOT_SET);
        m_cpu           = runConfig.GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "auto");
        m_cpuFeatures   = runConfig.GetValue<std::string>(RC_BR_CPU_FEATURES, "");
        m_useVTune      = runConfig.GetValue<bool>(RC_BR_USE_VTUNE, false);

        m_DumpIROptionAfter = runConfig.GetValue<const std::vector<IRDumpOptions> * >
                                (RC_BR_DUMP_IR_AFTER, 0);
        m_DumpIROptionBefore = runConfig.GetValue<const std::vector<IRDumpOptions> * >
                                (RC_BR_DUMP_IR_BEFORE, 0);

        m_DumpIRDir = runConfig.GetValue<std::string>(RC_BR_DUMP_IR_DIR, "");
        m_dumpHeuristcIR = runConfig.GetValue<bool>(RC_BR_DUMP_HEURISTIC_IR, false);

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
        case CL_DEV_BACKEND_OPTION_DUMP_HEURISTIC_IR :
            return m_dumpHeuristcIR;
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
        case CL_DEV_BACKEND_OPTION_DEVICE :
            return "cpu";
        case CL_DEV_BACKEND_OPTION_SUBDEVICE :
            return m_cpu.c_str();
        case CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES:
            return m_cpuFeatures.c_str();
        case CL_DEV_BACKEND_OPTION_DUMP_IR_DIR:
            return m_DumpIRDir.c_str();
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
    bool m_dumpHeuristcIR;
};

#if defined(SATEST_INCLUDE_MIC_DEVICE)
class MICBackendOptions: public ICLDevBackendOptions
{
public:
    void InitFromRunConfiguration(const BERunOptions& runConfig)
    {
        m_transposeSize = runConfig.GetValue<Intel::OpenCL::DeviceBackend::ETransposeSize>(RC_BR_TRANSPOSE_SIZE, TRANSPOSE_SIZE_NOT_SET);
        m_cpu           = runConfig.GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "knc");
        m_cpuFeatures   = runConfig.GetValue<std::string>(RC_BR_CPU_FEATURES, "");
        m_useVTune      = runConfig.GetValue<bool>(RC_BR_USE_VTUNE, false);
        m_fileName      = runConfig.GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "-");

        m_DumpIROptionAfter = runConfig.GetValue<const std::vector<IRDumpOptions> * >
                                (RC_BR_DUMP_IR_AFTER, 0);
        m_DumpIROptionBefore = runConfig.GetValue<const std::vector<IRDumpOptions> * >
                                (RC_BR_DUMP_IR_BEFORE, 0);

        m_DumpIRDir = runConfig.GetValue<std::string>(RC_BR_DUMP_IR_DIR, "");
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
        case CL_DEV_BACKEND_OPTION_DEVICE :
            return "mic";
        case CL_DEV_BACKEND_OPTION_SUBDEVICE :
            return m_cpu.c_str();
        case CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES:
            return m_cpuFeatures.c_str();
        //case CL_DEV_BACKEND_OPTION_DUMPFILE :
        //    return m_fileName.c_str();
        case CL_DEV_BACKEND_OPTION_DUMP_IR_DIR:
            return m_DumpIRDir.c_str();
        default:
            return defaultValue;
        }
    }

    virtual void SetStringValue(int optionId, const char* value)
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_SUBDEVICE :
            m_cpu = std::string(value);
        case CL_DEV_BACKEND_OPTION_SUBDEVICE_FEATURES:
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
        case OPTION_IR_DUMPTYPE_AFTER :
            *(static_cast<const std::vector<IRDumpOptions>* * >(Value)) = m_DumpIROptionAfter;
            return true;
        case OPTION_IR_DUMPTYPE_BEFORE :
            *(static_cast<const std::vector<IRDumpOptions>* * >(Value)) = m_DumpIROptionBefore;
            return true;
        case CL_DEV_BACKEND_OPTION_TARGET_DESC_BLOB:
            if(*pSize < m_targetDescSize) return false;
            memcpy(Value, &m_pTargetDesc[0], m_targetDescSize);
            return true;
        default:
            return false;
        }
    }

    // These two methods intended to serialize command line options only.
    // Target data is not serialized because:
    // 1. It will not be available at the moment of serialization.
    // 2. It's not needed to pass to the device. Device already has this information.
    // m_fileName member is not serialized because it's host-specific.
    uint64_t GetSerializedSize() const
    {
        return 4 + // will pass m_transposeSize as uint32_t
            m_cpu.size() + 1 + 4 + // 1 for termination symbol '\0', 4 for storing string size
            m_cpuFeatures.size() + 1 + 4 +
            1; // m_useVTune
    }

    char* GetSerializedData() const
    {
        uint64_t size = GetSerializedSize();
        uint32_t offset = 0;
        char* serialized = new char[size];
        // transpose size
        uint32_t transposeSize = (uint32_t)m_transposeSize;
        memcpy(serialized+offset, &transposeSize, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        // m_cpu
        uint32_t cpuSize = (uint32_t)m_cpu.size();
        memcpy(serialized+offset, &cpuSize, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(serialized+offset, m_cpu.c_str(), cpuSize);
        offset += cpuSize;
        // m_cpuFeatures
        uint32_t cpuFeaturesSize = (uint32_t)m_cpuFeatures.size();
        memcpy(serialized+offset, &cpuFeaturesSize, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(serialized+offset, m_cpuFeatures.c_str(), cpuFeaturesSize);
        offset += cpuFeaturesSize;
        // m_useVTune
        uint8_t useVTune = (uint8_t)m_useVTune;
        memcpy(serialized+offset, &useVTune, sizeof(uint8_t));
        return serialized;
    }

private:
    bool isMIC()
    {
        return m_cpu == "knc";
    }

    size_t              m_targetDescSize;
    std::vector<char>   m_pTargetDesc;

    const std::vector<IRDumpOptions>* m_DumpIROptionAfter;
    const std::vector<IRDumpOptions>* m_DumpIROptionBefore;
    std::string m_DumpIRDir;
    Intel::OpenCL::DeviceBackend::ETransposeSize m_transposeSize;
    std::string    m_cpu;
    std::string    m_cpuFeatures;
    bool           m_useVTune;
    std::string    m_fileName;
};
#endif // SATEST_INCLUDE_MIC_DEVICE

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

    virtual const char* GetStringValue(int optionId, const char* defaultValue)const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_DEVICE :
            return isMIC() ? "mic" : "cpu";
        default:
            return CPUBackendOptions::GetStringValue(optionId, defaultValue);
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
    bool isMIC() const
    {
        return m_cpu == "knc";
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

/**
 * Options used during program building
 */
class BuildProgramOptions: public ICLDevBackendOptions
{
public:
    BuildProgramOptions() : m_pInjectedObjectStart(NULL), m_bStopBeforeJIT(false) {}

    void SetInjectedObject(const char* pInjectedObjectStart, size_t injectedObjectSize)
    {
        m_pInjectedObjectStart = pInjectedObjectStart;
        m_injectedObjectSize   = injectedObjectSize;
    }

    const char* GetStringValue(int optionId, const char* defaultValue) const
    {
        return defaultValue;
    }

    bool GetBooleanValue(int optionId, bool defaultValue) const
    {
        switch(optionId)
        {
        case CL_DEV_BACKEND_OPTION_STOP_BEFORE_JIT:
            return m_bStopBeforeJIT;
        default:
            return defaultValue;
        }
    }

    int GetIntValue( int optionId, int defaultValue) const
    {
        return defaultValue;
    }

    bool GetValue(int optionId, void* Value, size_t* pSize) const
    {
        if (Value == NULL)
        {
            throw Exception::InvalidArgument("Value is not initialized");
        }

        switch(optionId)
        {
            case CL_DEV_BACKEND_OPTION_INJECTED_OBJECT:
                *(static_cast<const char* *>(Value)) = m_pInjectedObjectStart;
                *pSize = m_injectedObjectSize;
                return true;
            default:
                return false;
        }
    }
    void SetStopBeforeJIT() { m_bStopBeforeJIT = true; }

private:
    const char*  m_pInjectedObjectStart;
    size_t       m_injectedObjectSize;
    bool         m_bStopBeforeJIT;
};

} // namespace Validation

#endif // BACKEND_OPTIONS_H

