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

File Name:  BuiltinModuleManager.cpp

\*****************************************************************************/

#include <assert.h>
#include <string>
#include <memory>
#include "BuiltinModuleManager.h"
#include "BuiltinModules.h"
#if defined(INCLUDE_MIC_DEVICE)
#include "MICBuiltinLibrary.h"
#endif
#include "CPUBuiltinLibrary.h"

void RegisterCPUBIFunctions(void);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

BuiltinModuleManager* BuiltinModuleManager::s_pInstance = nullptr;

BuiltinModuleManager::BuiltinModuleManager()
{}

BuiltinModuleManager::~BuiltinModuleManager()
{
    for( BuiltinsMap::iterator i = m_BuiltinLibs.begin(), e = m_BuiltinLibs.end(); i != e; ++i )
    {
        delete i->second;
    }
}

void BuiltinModuleManager::Init()
{
    assert(!s_pInstance);
    s_pInstance = new BuiltinModuleManager();
    // TODO: need to move this function from the Manager Initialization
    RegisterCPUBIFunctions();
}

void BuiltinModuleManager::Terminate()
{
    if( nullptr != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

BuiltinModuleManager* BuiltinModuleManager::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}

// TODO: Make this method re-entrable
BuiltinLibrary* BuiltinModuleManager::GetOrLoadCPULibrary(Intel::CPUId cpuId)
{
    DevIdCpuId key = std::make_pair(0, cpuId);
    BuiltinsMap::iterator it = m_BuiltinLibs.find(key);
    if( it != m_BuiltinLibs.end() )
    {
        return it->second;
    }

    std::auto_ptr<BuiltinLibrary> pLibrary( new CPUBuiltinLibrary(cpuId) );
    pLibrary->Load();

    m_BuiltinLibs[key] = pLibrary.get();
    return pLibrary.release();
}

#if defined(INCLUDE_MIC_DEVICE)
// TODO: Make this method re-entrable
BuiltinLibrary* BuiltinModuleManager::GetOrLoadMICLibrary(unsigned int targetID, Intel::CPUId cpuId,
     const void* targetContext)
{
    std::auto_ptr<BuiltinLibrary> pLibrary( new MICBuiltinLibrary(cpuId) );
    pLibrary->SetContext(targetContext);
    pLibrary->Load();

    return pLibrary.release();
}
#endif

}}}
