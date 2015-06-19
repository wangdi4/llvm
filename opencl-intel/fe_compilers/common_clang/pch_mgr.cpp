/* Copyright 2000 - 2013 Intel Corporation All Rights Reserved.
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and licensors. The
 * Material is protected by worldwide copyright and trade secret laws and treaty
 * provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or disclosed
 * in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by Intel in writing.
 */

#include "pch_mgr.h"
#include "exceptions.h"
#include <cstdlib>
#include <stdio.h>
#include <assert.h>
#include "llvm/Support/ELF.h"
#include <fstream>

#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#include <stdio.h>

struct auto_dlclose
{
    auto_dlclose(void* module):
        m_pModule(module)
    {}

    ~auto_dlclose()
    {
#ifndef __ANDROID__
        if(m_pModule)
            dlclose(m_pModule);
#endif
    }

    void* get()
    {
        return m_pModule;
    }

    bool operator!()
    {
        return !m_pModule;
    }

    operator bool()
    {
        return m_pModule != NULL;
    }

    void*  release()
    {
        void * pTemp = m_pModule;
        m_pModule = NULL;
        return pTemp;
    }

private:
    auto_dlclose( const auto_dlclose& );

    void *m_pModule;
};
#endif

ResourceManager ResourceManager::g_instance;

void dummy(){}

// returns the pointer to the buffer loaded from the resource with the given id
const char* ResourceManager::get_resource( const char* id, const char* pszType, bool requireNullTerminate, size_t& out_size, const char* lib)
{
    //OclAutoMutex mutexGuard(&m_lock);
    llvm::sys::ScopedLock mutexGuard(m_lock);

    if( m_buffers.find(id) == m_buffers.end() )
    {
        // lazy load the resource if not found in the cache
        load_resource(id, pszType, requireNullTerminate, lib);
    }

    assert(m_buffers.find(id) != m_buffers.end());
    out_size = m_buffers[id].second;
    return m_buffers[id].first;
}

const char* ResourceManager::get_file(const char* path, bool binary, bool requireNullTerminate, size_t& out_size)
{
    //OclAutoMutex mutexGuard(&m_lock);
    llvm::sys::ScopedLock mutexGuard(m_lock);

    std::string key(path);

    if( m_buffers.find(key) == m_buffers.end() )
    {
        // lazy load the resource if not found in the cache
        load_file(path, binary, requireNullTerminate);
    }

    assert(m_buffers.find(key) != m_buffers.end());
    out_size = m_buffers[key].second;
    return m_buffers[key].first;
}

void ResourceManager::load_resource(const char* id, const char* pszType, bool requireNullTerminate, const char* lib)
{
    // this function is called under lock
    assert( m_buffers.find(id) == m_buffers.end());

#ifdef WIN32
    HMODULE hMod = NULL;

    char ResName[20] = { '-' };
    _snprintf_c(ResName, sizeof(ResName), "\"%s\"", id);

    // Get the handle to the current module
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    (LPCSTR)dummy,
                    &hMod);
    if (hMod == NULL)
    {
        throw internal_error("can't find module handle");
    }

    // Locate the resource
    HRSRC hRes = FindResource(hMod, ResName, pszType);
    if (hRes == NULL)
    {
        throw internal_error("can't find resource");
    }

    // Load the resource
    HGLOBAL hBytes = LoadResource(hMod, hRes);
    if (hBytes == NULL)
    {
        throw internal_error("can't load resource");
    }

    // Get the base address to the resource. This call doesn't really lock it
    char *pData = (char *)LockResource(hBytes);
    if (pData == NULL)
    {
        throw internal_error("can't lock the resource");
    }

    // Get the buffer size
    DWORD dResSize = SizeofResource(hMod, hRes);
    if (dResSize == 0)
    {
        throw internal_error("can't get the resource size");
    }

    if( requireNullTerminate && pData[dResSize-1] != '\0')
    {
        //reallocate the buffer to ensure the null termination
        std::vector<char>& buffer = m_allocations[id];
        buffer.resize(dResSize+1);
        buffer.assign(pData, pData+dResSize);
        buffer.push_back('\0');
        m_buffers[id] = std::pair<const char*, size_t>(buffer.data(), buffer.size());
    }
    else
    {
        m_buffers[id] = std::pair<const char*, size_t>(pData, size_t(dResSize));
    }
#else
    // Symbol Name is <type>_<number>
    char name[64];
    char size_name[69];
    void *symbol;
    uint32_t size;
    void *handle;

    snprintf(name, 64, "%s_%s", pszType, id);
    snprintf(size_name, 64, "%s_%s_size", pszType, id);
    if (!lib)
    {
#ifdef __ANDROID__
        handle = RTLD_DEFAULT;
#else
        handle = dlopen(NULL, RTLD_NOW);
#endif
    }
    else
    {
        handle = dlopen(lib, RTLD_NOW);
    }
    auto_dlclose module(handle);
    if (!module)
    {
        throw internal_error("can't open the module");
    }

    symbol = dlsym(module.get(), size_name);
    if (!symbol)
    {
        throw internal_error("can't find the symbol");
    }

    size = *(uint32_t *)symbol;
    symbol = dlsym(module.get(), name);
    if (!symbol)
    {
        throw internal_error("can't find the symbol");
    }

    // Cache the buffers
    std::vector<char>& buffer = m_allocations[id];
    if( requireNullTerminate && ((char*)symbol)[size-1] != '\0' )
    {
        buffer.resize(size+1);
        buffer.assign((char*)symbol, (char*)symbol+size);
        buffer.push_back('\0');
    }
    else
    {
        buffer.resize(size);
        buffer.assign((char*)symbol, (char*)symbol+size);
    }
    m_buffers[id] = std::pair<const char*, size_t>(buffer.data(), buffer.size());
#endif // WIN32
}

// cache the content of the file to the internal buffers
void ResourceManager::load_file(const char* path, bool binary, bool requireNullTerminate)
{
    std::string key(path);
    std::ifstream fs(path, binary? std::ios::binary : std::ios::in);
    std::vector<char>& buffer = m_allocations[key];

    buffer.assign(std::istreambuf_iterator<char>(fs),
                  std::istreambuf_iterator<char>());

    if( requireNullTerminate && buffer.size() > 0 && buffer.back() != '\0' )
    {
        buffer.push_back('\0');
    }
    m_buffers[key] = std::pair<const char*, size_t>(buffer.data(), buffer.size());
}
