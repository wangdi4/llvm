// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#include <assert.h>
#include <string>
#include <memory>
#include "BuiltinModuleManager.h"
#include "BuiltinModules.h"
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

}}}
