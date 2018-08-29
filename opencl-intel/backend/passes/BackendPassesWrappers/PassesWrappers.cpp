// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "OCLPassSupport.h"
#include "LocalBuffers.h"

#include <llvm/Pass.h>

/// @brief Backend Passes Wrappers,
///        We use these wrappers instead of the actual passes classes because when
///        registering the passes in opt, the passes constructors must have no arguments,
///        so the wrappers hide the original pass constructor and provide a constructor
///        with no arguments for opt.

namespace intel{

    /// @brief PassesWrappersSupporter, initiate the variables that are needed by
    ///        the passes classes
    class PassesWrappersSupporter
    {
    public:
        /// @brief Get the singleton instance of PassesWrappersSupporter
        static PassesWrappersSupporter* getInstance()
        {
            if (nullptr == m_instance) {
                m_instance = new PassesWrappersSupporter();
            }
            return m_instance;
        }

    private:
        PassesWrappersSupporter() {}

        static PassesWrappersSupporter * m_instance;
    };
    PassesWrappersSupporter * PassesWrappersSupporter::m_instance = nullptr;


    class LocalBuffersWrapper : public LocalBuffers
    {
    public:
        LocalBuffersWrapper() :
          LocalBuffers(false)
          {}
        static char ID;
    };

    class LocalBuffersWithDebugWrapper : public LocalBuffers
    {
    public:
        LocalBuffersWithDebugWrapper() :
          LocalBuffers(true)
          {}
        static char ID;
    };

char intel::LocalBuffersWrapper::ID = 0;
char intel::LocalBuffersWithDebugWrapper::ID = 0;

OCL_INITIALIZE_PASS(LocalBuffersWrapper, "local-buffers", "Resolves the internal local variables and map them to local buffer", false, false)
OCL_INITIALIZE_PASS(LocalBuffersWithDebugWrapper, "local-buffers-debug", "Resolves the internal local variables and map them to local buffer, in debugger mode", false, false)


} // namespace intel



// Create functions for use in opt
extern "C" {
    void* createLocalBuffersWrapper()
    {
        return new intel::LocalBuffersWrapper();
    }

    void* createLocalBuffersWithDebugWrapper()
    {
        return new intel::LocalBuffersWithDebugWrapper();
    }
}

