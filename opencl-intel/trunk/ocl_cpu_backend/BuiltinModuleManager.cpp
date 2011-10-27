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
#include "BuiltinModule.h"
#include "MICBuiltinLibrary.h"
#include "CPUBuiltinLibrary.h"

void RegisterBIFunctions(void);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

BuiltinModuleManager* BuiltinModuleManager::s_pInstance = NULL;

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
    RegisterBIFunctions();
}

void BuiltinModuleManager::Terminate()
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

BuiltinModuleManager* BuiltinModuleManager::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}

// TODO: Make this method re-entrable
BuiltinLibrary* BuiltinModuleManager::GetOrLoadCPULibrary(Intel::ECPU cpuId, unsigned int cpuFeatures)
{
    const CPUArchFeatures key(std::make_pair(cpuId, cpuFeatures));
    BuiltinsMap::iterator it = m_BuiltinLibs.find(key);
    if( it != m_BuiltinLibs.end() )
    {
        return it->second;
    }

    std::auto_ptr<BuiltinLibrary> pLibrary( new CPUBuiltinLibrary(cpuId, cpuFeatures) );
    pLibrary->Load();
    
    m_BuiltinLibs[key] = pLibrary.get();
    return pLibrary.release();
}

// TODO: Make this method re-entrable
BuiltinLibrary* BuiltinModuleManager::GetOrLoadMICLibrary(Intel::ECPU micId, unsigned int micFeatures)
{
    const CPUArchFeatures key(std::make_pair(micId, micFeatures));
    BuiltinsMap::iterator it = m_BuiltinLibs.find(key);
    if( it != m_BuiltinLibs.end() )
    {
        return it->second;
    }

    std::auto_ptr<BuiltinLibrary> pLibrary( new MICBuiltinLibrary(micId, micFeatures) );
    pLibrary->Load();
    
    m_BuiltinLibs[key] = pLibrary.get();
    return pLibrary.release();
}


}}}
