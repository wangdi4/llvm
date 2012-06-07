/*****************************************************************************\

Copyright (c) Intel Corporation (2011,2012).

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
#include "llvm/Support/MutexGuard.h"
#include <assert.h>
#include <algorithm>
#include <cstdio>
#include <functional>
#include "plugin_manager.h"
#include "plugin_interface.h"

using namespace Intel::OpenCL::DeviceBackend::Utils;

namespace Intel { namespace OpenCL {

DeviceBackend::ICLDevBackendPlugin* getBackendPlugin(IPlugin* plugin)
{
    assert (plugin && "Null plugin given!");
    DeviceBackend::ICLDevBackendPlugin* ret = plugin->getBackendPlugin();
    assert(ret && "NULL backend plugin is given");
    return ret;
}

Frontend::ICLFrontendPlugin* getFrontendPlugin(IPlugin* plugin)
{
    assert (plugin && "Null plugin given!");
    Frontend::ICLFrontendPlugin* ret = plugin->getFrontendPlugin();
    assert(ret && "NULL frontend plugin is given");
    return ret;
}

PluginManager* PluginManager::s_instance = NULL;

/**
 * Will be called on OCL Backend initialization.
 * 
 * Responsible for creating and initializing the static instance of the
 * PluginManager and loading all the plugin dlls
 * Note! this method should be called exactly one time before any other
 * usage of the PluginManager methods.
 */
void PluginManager::Init()
{
    assert (NULL == s_instance);
    try
    {
        s_instance = new PluginManager();
        s_instance->LoadPlugins();
    }
    catch(PluginManagerException& )
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
    for(PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; it++)
        if(*it) delete *it;
}

void PluginManager::LoadPlugins()
{
    typedef llvm::SmallVector<llvm::StringRef, 10> DllNamesVector;
    const char *dlls = getenv("OCLBACKEND_PLUGINS");
    if (NULL == dlls || (std::string)dlls == "")
        return;
    DllNamesVector namesVector;
    llvm::StringRef namesEnv(dlls);
    namesEnv.split(namesVector, ",", -1, false);
    for( DllNamesVector::iterator it = namesVector.begin(); it != namesVector.end(); ++it)
    {
        try
        {
            PluginInfo* pInfo = new PluginInfo(*it);
            m_listPlugins.insert(pInfo);
        }
        catch (DeviceBackend::Exceptions::DynamicLibException ex)
        {
            throw PluginManagerException(ex.what());
        }
    }
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
void PluginManager::OnCreateBinary(const DeviceBackend::ICLDevBackendKernel_* pKernel,
                                   const _cl_work_description_type* pWorkDesc, 
                                   size_t bufSize, 
                                   void* pArgsBuffer)
{
    for( PluginsList::iterator it= m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnCreateBinary(pKernel, 
                                 pWorkDesc, 
                                 bufSize, 
                                 pArgsBuffer);
}

/**
 * Called by OCL Backend on CreateProgram call
 */
void PluginManager::OnCreateProgram(const _cl_prog_container_header* pContainer, 
                                    const DeviceBackend::ICLDevBackendProgram_* pProgram)
{
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnCreateProgram(pContainer, pProgram);
}

/**
 * Called by OCL Backend on CreateKernels call
 */
void PluginManager::OnCreateKernel(const DeviceBackend::ICLDevBackendProgram_* pProgram,
                                   const DeviceBackend::ICLDevBackendKernel_* pKernel,
                                   const llvm::Function* pFunc)
{
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnCreateKernel(pProgram, pKernel, pFunc);
}

/*
 * called by OCL Backend on each call the Program.Release method.
 * 
 */
void PluginManager::OnReleaseProgram(const DeviceBackend::ICLDevBackendProgram_* pProgram)
{
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnReleaseProgram(pProgram);
}

void PluginManager::OnLink(const Frontend::LinkData* linkData)
{
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getFrontendPlugin((*it)->plugin())->OnLink(linkData);
}

void PluginManager::OnCompile(const Frontend::CompileData* compileData)
{
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getFrontendPlugin((*it)->plugin())->OnCompile(compileData);
}

PluginManagerException::~PluginManagerException() throw()
{
}

PluginManagerException::PluginManagerException(std::string message) : std::runtime_error(message)
{
}

///PluginInfo
//
PluginManager::PluginInfo::PluginInfo(const std::string& dllName)
{
    m_dll.Load(dllName.c_str());
    PLUGIN_CREATE_FUNCPTR factory 
        = (PLUGIN_CREATE_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("CreatePlugin");
    m_pPlugin = factory();
}

PluginManager::PluginInfo::~PluginInfo()
{
    PLUGIN_RELEASE_FUNCPTR cleanup
        = (PLUGIN_RELEASE_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("ReleasePlugin");
    {
        assert(m_pPlugin && "NULL plugin");
        llvm::MutexGuard cleanlock(m_lock);
        cleanup(m_pPlugin);
    }
    m_pPlugin = NULL;
    m_dll.Close();
}

IPlugin* PluginManager::PluginInfo::plugin()
{
    return m_pPlugin;
}

}}
