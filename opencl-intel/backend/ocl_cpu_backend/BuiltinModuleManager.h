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

File Name:  BuiltinModuleManager.h

\*****************************************************************************/
#pragma once
#include <map>
#include "CPUDetect.h"

namespace llvm
{
class LLVMContext;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinModules;
class BuiltinLibrary;

//*****************************************************************************************
// Responsible for loading builtin modules in a lazy fashion
//
class BuiltinModuleManager
{
private:
    BuiltinModuleManager();
    ~BuiltinModuleManager();

public:
    /**
     * Static singleton intialization
     */
    static void Init();
    /**
     * Static singleton termination
     */
    static void Terminate();
    /**
     * Singleton instance
     */
    static BuiltinModuleManager* GetInstance();

    /**
     * Returns the \see BuiltinsLibrary for the given cpu. Loads it if necessary
     */
    BuiltinLibrary* GetOrLoadCPULibrary(Intel::CPUId cpuId);
    /**
     * Creates the builtins module for the given cpu using the given LLVMContext
     */
    BuiltinModules*  CreateBuiltinModule(int cpuId, llvm::LLVMContext* pContext);

private:
    typedef std::pair<int, CPUId> DevIdCpuId;
    typedef std::map<DevIdCpuId, BuiltinLibrary*> BuiltinsMap;
    static BuiltinModuleManager* s_pInstance;
    BuiltinsMap m_BuiltinLibs;
};

}}}
