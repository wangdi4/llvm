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

File Name:  plugin_manager.h

\*****************************************************************************/
#pragma once
#include "plugin_interface.h"
#include "exceptions.h"
#include "DynamicLib.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include <string>
#include <stdexcept>


namespace Intel { namespace OpenCL { namespace DeviceBackend {

DEFINE_EXCEPTION(PluginManagerException)

class ICLDevBackendKernel_;
class ICLDevBackendProgram_;
const int INIT_PLUGIN_COUNT = 10;

/**
 * PluginManager presents the facade infterface through which the OCL Backend
 * could notify the plugins about specific events.
 * 
 * PluginManager is responsible for loading the plugins from OS upon
 * initialization. It will pass each notification to each loaded plugin in
 * sequence.
 * 
 * Each plugin will be deployed as a separate DLL. There are several proposed
 * method for loading such plugins:
 * 
 * 1. Each plugin's dll will be named with special prefix such that PluginManager
 * could scan the current working directory and load all such dlls
 * 2. There will be a special environment variable that will specify the list of
 * plugins to load.
 * 3. OCL Backend will use the configuration file ( currently not implemented) to
 * specify the needed plugins.
 */
class PluginManager
{
public:
    static void Init();
    static void Terminate();
    static PluginManager& Instance();

    void OnCreateBinary(const ICLDevBackendKernel_* pKernel, 
                        const _cl_work_description_type* pWorkDesc, 
                        size_t bufSize, 
                        void* pArgsBuffer);

    void OnCreateKernel(const ICLDevBackendProgram_* pProgram,
                        const ICLDevBackendKernel_* pKernel,
                        const llvm::Function* pFunction);

    void OnCreateProgram(const _cl_prog_container_header* pContainer, 
                         const ICLDevBackendProgram_* pProgram);



    void OnReleaseProgram(const ICLDevBackendProgram_* pProgram);

private:
    PluginManager(){}
    ~PluginManager();

    void LoadPlugins();
    void LoadPlugin(const std::string& filename);

private:

    struct PluginInfo
    {
        ICLDevBackendPlugin*   m_pPlugin;
        Utils::DynamicLib      m_dll;
        PLUGIN_RELEASE_FUNCPTR m_pReleaseFunc;
    };

    typedef llvm::SmallPtrSet<PluginInfo*, INIT_PLUGIN_COUNT> PluginsList;

    PluginsList m_plugins;
    static PluginManager* s_instance;
};

}}}