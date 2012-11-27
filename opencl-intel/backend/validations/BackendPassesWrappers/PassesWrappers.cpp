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

#include "ImplicitArgsUtils.h"

#include "ModuleCleanup.h"
#include "AddImplicitArgs.h"
#include "LocalBuffers.h"

#include <llvm/Pass.h>

#include <memory>

/// @brief Backend Passes Wrappers,
///        We use these wrappers instead of the actual passes classes because when
///        registering the passes in opt, the passes constructors must have no arguments,
///        so the wrappers hide the original pass constructor and provide a constructor
///        with no arguments for opt.

namespace Intel { namespace OpenCL { namespace DeviceBackend {

    /// @brief PassesWrappersSupporter, initiate the variables that are needed by
    ///        the passes classes
    class PassesWrappersSupporter
    {
    public:
        /// @brief Get the singleton instance of PassesWrappersSupporter
        static PassesWrappersSupporter* getInstance()
        {
            if (NULL == m_instance) {
                m_instance = new PassesWrappersSupporter();
            }
            return m_instance;
        }

        /// @brief Get a dummy function set to be used instead of the actual
        ///        vectorized function set
        SmallVectorImpl<Function*> &getDummyFuncSet()
        {
            m_vectFunctions.clear();
            return m_vectFunctions;
        }

        /// @brief Get a dummy map from function to TLLVMKernelInfo to be used instead of the actual map
        std::map<const llvm::Function*, TLLVMKernelInfo> &getDummyLocalBufferMap()
        {
            m_kernelsLocalBufferMap.clear();
            return m_kernelsLocalBufferMap;
        }
    private:
        PassesWrappersSupporter() {}

        static PassesWrappersSupporter * m_instance;
        llvm::SmallVector<llvm::Function*, 16> m_vectFunctions;
        std::map<const llvm::Function*, TLLVMKernelInfo> m_kernelsLocalBufferMap;
    };
    PassesWrappersSupporter * PassesWrappersSupporter::m_instance = NULL;


    class ModuleCleanupWrapper : public ModuleCleanup
    {
    public:
        ModuleCleanupWrapper() :
          ModuleCleanup(PassesWrappersSupporter::getInstance()->getDummyFuncSet())
          {}
        static char ID;
    };

    class AddImplicitArgsWrapper : public AddImplicitArgs
    {
    public:
        AddImplicitArgsWrapper() :
          AddImplicitArgs(PassesWrappersSupporter::getInstance()->getDummyFuncSet())
          {}
        static char ID;
    };

    class LocalBuffersWrapper : public LocalBuffers
    {
    public:
        LocalBuffersWrapper() :
          LocalBuffers(PassesWrappersSupporter::getInstance()->getDummyLocalBufferMap(), false)
          {}
        static char ID;
    };

    class LocalBuffersWithDebugWrapper : public LocalBuffers
    {
    public:
        LocalBuffersWithDebugWrapper() :
          LocalBuffers(PassesWrappersSupporter::getInstance()->getDummyLocalBufferMap(), true)
          {}
        static char ID;
    };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

char Intel::OpenCL::DeviceBackend::ModuleCleanupWrapper::ID = 0;
char Intel::OpenCL::DeviceBackend::AddImplicitArgsWrapper::ID = 0;
char Intel::OpenCL::DeviceBackend::LocalBuffersWrapper::ID = 0;
char Intel::OpenCL::DeviceBackend::LocalBuffersWithDebugWrapper::ID = 0;


// Create functions for use in opt
extern "C" {
    void* createModuleCleanupWrapper()
    {
        return new Intel::OpenCL::DeviceBackend::ModuleCleanupWrapper();
    }

    void* createAddImplicitArgsWrapper()
    {
        return new Intel::OpenCL::DeviceBackend::AddImplicitArgsWrapper();
    }

    void* createLocalBuffersWrapper()
    {
        return new Intel::OpenCL::DeviceBackend::LocalBuffersWrapper();
    }

    void* createLocalBuffersWithDebugWrapper()
    {
        return new Intel::OpenCL::DeviceBackend::LocalBuffersWithDebugWrapper();
    }
}

// Register passes for opt
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::ModuleCleanupWrapper>         ModuleCleanupPass("module-cleanup", "Cleans OpenCL module: removes functions which are not kernels (or called by kernels).");
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::AddImplicitArgsWrapper>       AddImplicitArgsPass("add-implicit-args", "Adds the implicit arguments to signature of all functions of the module (that are defined inside the module)");
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::LocalBuffersWrapper>          LocalBuffersPass("local-buffers", "Resolves the internal local variables and map them to local buffer");
static llvm::RegisterPass<Intel::OpenCL::DeviceBackend::LocalBuffersWithDebugWrapper> LocalBuffersWithDebugPass("local-buffers-debug", "Resolves the internal local variables and map them to local buffer, in debugger mode");
