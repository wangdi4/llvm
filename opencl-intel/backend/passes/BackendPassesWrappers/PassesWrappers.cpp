/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  PassesWrappers.cpp

\*****************************************************************************/

#include "OCLPassSupport.h"
#include "LocalBuffers.h"

#include <llvm/Pass.h>

#include <memory>

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
        llvm::SmallVector<llvm::Function*, 16> m_vectFunctions;
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

