// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "plugin_manager.h"
#include "plugin_interface.h"
#include <BE_DynamicLib.h>
#include "cl_utils.h"
#include "cl_synch_objects.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/Atomic.h"
#include "llvm/Support/Mutex.h"

#include <assert.h>
#include <algorithm>
#include <cstdio>
#include <functional>
#include <exception>

using namespace Intel::OpenCL::DeviceBackend::Utils;

namespace Intel { namespace OpenCL {
static DeviceBackend::ICLDevBackendPlugin* getBackendPlugin(IPlugin* plugin)
{
    assert (plugin && "Null plugin given!");
    DeviceBackend::ICLDevBackendPlugin* ret = plugin->getBackendPlugin();
    assert(ret && "NULL backend plugin is given");
    return ret;
}

static Frontend::ICLFrontendPlugin* getFrontendPlugin(IPlugin* plugin)
{
    assert (plugin && "Null plugin given!");
    Frontend::ICLFrontendPlugin* ret = plugin->getFrontendPlugin();
    assert(ret && "NULL frontend plugin is given");
    return ret;
}

class PluginInfo
{
    Intel::OpenCL::DeviceBackend::Utils::BE_DynamicLib m_dll;
    IPlugin* m_pPlugin;
    //lock for the cleanup operation
    Utils::OclMutex m_lock;
public:
    PluginInfo(const std::string& dllName);
    ~PluginInfo();
    IPlugin* plugin();
};//end class PluginInfo

//
//PluginManager
//
PluginManager::PluginManager():m_bInitialized(false)
{
}

PluginManager::~PluginManager()
{
    for(PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; it++)
        if(*it) delete *it;
}

void PluginManager::LoadPlugins()
{
    if( m_bInitialized)
        return;

    typedef std::vector<std::string> DllNamesVector;
#ifdef WIN32
    char buffer[MAX_PATH];
    const char *dlls = buffer;
    int len = GetEnvironmentVariableA("OCLBACKEND_PLUGINS", buffer, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        dlls = nullptr;
    }
#else
    const char *dlls = getenv("OCLBACKEND_PLUGINS");
#endif
    if (nullptr == dlls || (std::string)dlls == "")
        return;
    DllNamesVector namesVector;
    std::string namesEnv(dlls);
    SplitString(namesEnv, ',', namesVector);
    PluginsList plugins;
    for( DllNamesVector::iterator it = namesVector.begin(); it != namesVector.end(); ++it)
    {
        try
        {
            PluginInfo* pInfo = new PluginInfo(*it);
            plugins.push_back(pInfo);
        }
        catch (DeviceBackend::Exceptions::DynamicLibException& ex)
        {
            throw PluginManagerException(ex.what());
        }
        catch (std::bad_alloc& )
        {
            throw PluginManagerException("Out of memory");
        }
    }
    m_listPlugins.swap(plugins);
    m_bInitialized = true;
}

/**
 * Called by OCL Backend on each call for CreateBinary.
 */
void PluginManager::OnCreateBinary(const DeviceBackend::ICLDevBackendKernel_* pKernel,
                                   const _cl_work_description_type* pWorkDesc,
                                   size_t bufSize,
                                   void* pArgsBuffer)
{
    LoadPlugins();
    for( PluginsList::iterator it= m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnCreateBinary(pKernel,
                                 pWorkDesc,
                                 bufSize,
                                 pArgsBuffer);
}

/**
 * Called by OCL Backend on CreateProgram call
 */
void PluginManager::OnCreateProgram(const void * pBinary,
                                    size_t uiBinarySize,
                                    const DeviceBackend::ICLDevBackendProgram_* pProgram)
{
    LoadPlugins();
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnCreateProgram(pBinary, uiBinarySize, pProgram);
}

/**
 * Called by OCL Backend on CreateKernels call
 */
void PluginManager::OnCreateKernel(const DeviceBackend::ICLDevBackendProgram_* pProgram,
                                   const DeviceBackend::ICLDevBackendKernel_* pKernel,
                                   const void* pFunc)
{
    LoadPlugins();
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnCreateKernel(pProgram, pKernel, pFunc);
}

/*
 * called by OCL Backend on each call the Program.Release method.
 *
 */
void PluginManager::OnReleaseProgram(const DeviceBackend::ICLDevBackendProgram_* pProgram)
{
    LoadPlugins();
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getBackendPlugin((*it)->plugin())->OnReleaseProgram(pProgram);
}

void PluginManager::OnLink(const Frontend::LinkData* linkData)
{
    LoadPlugins();
    for (PluginsList::iterator it = m_listPlugins.begin() ; it != m_listPlugins.end() ; ++it)
        getFrontendPlugin((*it)->plugin())->OnLink(linkData);
}

void PluginManager::OnCompile(const Frontend::CompileData* compileData)
{
    LoadPlugins();
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
PluginInfo::PluginInfo(const std::string& dllName)
{
    m_dll.Load(dllName.c_str());
    PLUGIN_CREATE_FUNCPTR factory = (PLUGIN_CREATE_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("CreatePlugin");
    if (nullptr == factory)
    {
        m_pPlugin = nullptr;
        throw DeviceBackend::Exceptions::DynamicLibException(dllName);
    }
    m_pPlugin = factory();
}

PluginInfo::~PluginInfo()
{
    if (nullptr != m_pPlugin)
    {
        PLUGIN_RELEASE_FUNCPTR cleanup = (PLUGIN_RELEASE_FUNCPTR)(intptr_t)m_dll.GetFuncPtr("ReleasePlugin");
        {
            assert(m_pPlugin && "NULL plugin");
            Utils::OclAutoMutex cleanlock(&m_lock);
            cleanup(m_pPlugin);
        }
        m_pPlugin = nullptr;
        m_dll.Close();
    }
}

IPlugin* PluginInfo::plugin()
{
    return m_pPlugin;
}
}}
