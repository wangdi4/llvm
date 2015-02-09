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

#include <map>
#include <list>
#include <iostream>
#include <string>
#include <vector>
#include "llvm/Support/Mutex.h"

// Singleton class for resource management
// Its main purpose is to cache the buffers loaded from the resources
// but it could be easely extended to support file based buffers as well
class ResourceManager
{
public:
    static ResourceManager& instance(){return g_instance;}

    const char* get_resource(const char* id, const char* pszType, bool requireNullTerminate, size_t& out_size, const char* lib);

    const char* get_file(const char* path, bool binary, bool requireNullTerminate, size_t& out_size);

private:
    ResourceManager(){}

    void load_resource(const char* id, const char* pszType, bool requireNullTerminate, const char* lib);

    void load_file(const char* path, bool binary, bool requireNullTerminate);

private:
    static ResourceManager g_instance;
    llvm::sys::Mutex m_lock;
    // map that caches the pointers to the loaded buffers and their sizes
    // those buffers could be either the pointer to the loaded
    // resource or to the cached buffers (stored in the m_allocations var below)
    std::map<std::string, std::pair<const char*, size_t> > m_buffers;
    std::map<std::string, std::vector<char> > m_allocations;
};
