/*****************************************************************************\

Copyright (c) Intel Corporation (2011, 2012).

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
#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include <BE_DynamicLib.h>
#include <string>
#include <stdexcept>
#include <cstdlib>

#include "link_data.h"
#include "compile_data.h"

//we define those just before the llvm inclusion, so if stdint.h was not
//included thus far, we won't break compilation
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif//__STDC_LIMIT_MACROS

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif//__STDC_CONSTANT_MACROS

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Atomic.h"
#include "llvm/Support/Mutex.h"

namespace llvm{
class Function;
}

namespace Intel { namespace OpenCL { 
namespace DeviceBackend {
class ICLDevBackendKernel_;
class ICLDevBackendProgram_;
}//end DeviceBackend

struct IPlugin;

class PluginManagerException : public std::runtime_error{
public:
    virtual ~PluginManagerException() throw();
    PluginManagerException(std::string message);
};

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
    ~PluginManager();
    PluginManager();
    
    /////////////////////////////////////////////////
    //Description:
    //  invoked by the OCL Backend, when the runtime initializes an NDRange.
    //  (could be as a result by call to function such as clEnqueueBuffer
    /////////////////////////////////////////////////
    void OnCreateBinary(const DeviceBackend::ICLDevBackendKernel_* pKernel, 
                        const _cl_work_description_type* pWorkDesc, 
                        size_t bufSize, 
                        void* pArgsBuffer);

    /////////////////////////////////////////////////
    //Description:
    // invoked by the OCL BE, as a result of a call to clCreateKernel by the application
    /////////////////////////////////////////////////
    void OnCreateKernel(const DeviceBackend::ICLDevBackendProgram_* pProgram,
                        const DeviceBackend::ICLDevBackendKernel_* pKernel,
                        const llvm::Function* pFunction);
    
    /////////////////////////////////////////////////
    //Description:
    //  invoked by OCL BE, when a OCL Program object is created.
    //  (Program objects may be created by several function such as
    //  'clCreateProgramWithSource' and clCreateProgramWithBinary).
    /////////////////////////////////////////////////
    void OnCreateProgram(const _cl_prog_container_header* pContainer, 
                         const DeviceBackend::ICLDevBackendProgram_* pProgram);

    /////////////////////////////////////////////////
    //Description:
    //  invoked by OCL BE, when clReleaseProgram is called by the application.
    /////////////////////////////////////////////////
    void OnReleaseProgram(const DeviceBackend::ICLDevBackendProgram_* pProgram);

    /////////////////////////////////////////////////
    //Description:
    //  invoked by clang, (compiler's FE), when a program is linked by
    //  clLinkProgram. This callback is only relevant to OCL 1.2 onwards.
    /////////////////////////////////////////////////
    void OnLink(const Frontend::LinkData* linkData);
    
    /////////////////////////////////////////////////
    //Description:
    //  invoked by clang, when compiling an open CL source file to llvm bytecode.
    /////////////////////////////////////////////////
    void OnCompile(const Frontend::CompileData* compileData);

private:
    //
    //Load the plugins which corresponds the OCLXXX_PLUGINS environment variables
    void LoadPlugins();

private:
    class PluginInfo{
        Intel::OpenCL::DeviceBackend::Utils::BE_DynamicLib m_dll;
        IPlugin* m_pPlugin;
        //lock for the cleanup operation
        llvm::sys::Mutex m_lock;
     public:
        PluginInfo(const std::string& dllName);
        ~PluginInfo();
        IPlugin* plugin();
    };//end class PluginInfo

    typedef llvm::SmallPtrSet<PluginInfo*, 10> PluginsList;
    //list of registered plugins
    PluginsList m_listPlugins;

    // don't allow Copy constructor and assignment operator
    PluginManager(const PluginManager&);
    PluginManager& operator=(const PluginManager&);
};//end class PluginManager

}}


#endif
