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

File Name:  plugin_manager.cpp

\*****************************************************************************////////////////////////////////////////////////////////////

#include "llvm/ADT/StringRef.h"
#include <assert.h>
#include <algorithm>
#include "plugin_manager.h"
#include "plugin_interface.h"



namespace Intel { namespace OpenCL { namespace DeviceBackend {


PluginManager* PluginManager::s_instance = NULL;

/**
 * Will be called on OCL Backend initialization.
 * 
 * Responsible for creating and initializing the static instance of the
 * PluginManager and loading all the plugin dlls
 */
void PluginManager::Init()
{
    assert( NULL == s_instance);
    try
    {
        s_instance = new PluginManager();
        s_instance->LoadPlugins();
    }
    catch(Exceptions::PluginManagerException& )
    {
        //LLVMBackend::GetInstance()->m_logger->Log( Logger::ERROR_LEVEL, ex.what());
        throw;
    }
}

void PluginManager::Terminate()
{
    if( NULL != s_instance)
    {
        delete s_instance;
        s_instance = NULL;
    }
}

PluginManager::~PluginManager()
{
    for( PluginsList::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
    {
        (*it)->m_pReleaseFunc((*it)->m_pPlugin);
        (*it)->m_dll.Close();
    }
}

void PluginManager::LoadPlugins()
{
    char* env = getenv("OCLBACKEND_PLUGINS");
    if( NULL == env)
    {
        return;
    }

    typedef llvm::SmallVector<llvm::StringRef, 10> DllNamesVector;
    DllNamesVector namesVector;
    llvm::StringRef namesEnv(env);

    namesEnv.split(namesVector, ",", -1, false);

    for( DllNamesVector::iterator it = namesVector.begin(); it != namesVector.end(); ++it)
    {
        LoadPlugin(*it);
    }
}

void PluginManager::LoadPlugin( const std::string& filename)
{
    std::auto_ptr<PluginInfo> pInfo(new PluginInfo());

    if( !pInfo->m_dll.Load(filename.c_str()) )
    {
        throw Exceptions::PluginManagerException("Plugin Load failed");
    }

    PLUGIN_CREATE_FUNCPTR pCreatePluginFunc = (PLUGIN_CREATE_FUNCPTR)(intptr_t)pInfo->m_dll.GetFuncPtr("CreatePlugin");

    if( NULL == pCreatePluginFunc )
    {
        throw Exceptions::PluginManagerException("Failed to get the plugin's factory function");
    }

    pInfo->m_pReleaseFunc = (PLUGIN_RELEASE_FUNCPTR)(intptr_t)pInfo->m_dll.GetFuncPtr("ReleasePlugin");

    if( NULL == pInfo->m_pReleaseFunc )
    {
        throw Exceptions::PluginManagerException("Failed to get the plugin's release function");
    }

    pInfo->m_pPlugin = pCreatePluginFunc();

    m_plugins.insert( pInfo.release() );
}


/**
 * Returns the static instance of the PluginManager.
 */
PluginManager& PluginManager::Instance()
{
    assert( NULL != s_instance );
    return *s_instance;
}


/**
 * Called by OCL Backend on each call for CreateBinary.
 */
void PluginManager::OnCreateBinary(const ICLDevBackendKernel_* pKernel, 
                                   const _cl_work_description_type* pWorkDesc, 
                                   size_t bufSize, 
                                   void* pArgsBuffer)
{
    for( PluginsList::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
    {
        (*it)->m_pPlugin->OnCreateBinary(pKernel, 
                                         pWorkDesc, 
                                         bufSize, 
                                         pArgsBuffer);
    }
}


/**
 * Called by OCL Backend on CreateProgram call
 */
void PluginManager::OnCreateProgram(const _cl_prog_container_header* pContainer, 
                                    const ICLDevBackendProgram_* pProgram)
{
    for( PluginsList::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
    {
        (*it)->m_pPlugin->OnCreateProgram(pContainer, pProgram);
    }
}

/**
 * Called by OCL Backend on CreateKernels call
 */
void PluginManager::OnCreateKernel(const ICLDevBackendProgram_* pProgram,
                                   const ICLDevBackendKernel_* pKernel,
                                   const llvm::Function* pFunc)
{
    for( PluginsList::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
    {
        (*it)->m_pPlugin->OnCreateKernel(pProgram, pKernel, pFunc);
    }
}



/**
 * called by OCL Backend on each call the Program.Release method.
 * 
 */
void PluginManager::OnReleaseProgram(const ICLDevBackendProgram_* pProgram)
{
    for( PluginsList::iterator it = m_plugins.begin(); it != m_plugins.end(); ++it)
    {
        (*it)->m_pPlugin->OnReleaseProgram(pProgram);
    }
}

}}}