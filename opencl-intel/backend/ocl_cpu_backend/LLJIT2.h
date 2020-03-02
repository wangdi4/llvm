// Copyright (c) 2020 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#ifndef INTEL_OPENCL_BACKEND_CPU_LLJIT2_H
#define INTEL_OPENCL_BACKEND_CPU_LLJIT2_H

#include "llvm/ExecutionEngine/Orc/LLJIT.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// An extended version of LLJIT that calls recordCtorDtors and runs
/// constructors by name.
class LLJIT2 : public llvm::orc::LLJIT {

public:
  LLJIT2(llvm::orc::LLJITBuilderState &S, llvm::Error &Err)
      : llvm::orc::LLJIT(S, Err) {}

  /// Add a module to be compiled to the main JITDylib.
  llvm::Error addIRModule(llvm::orc::ThreadSafeModule TSM) {
    assert(TSM && "Can not add null module");

    if (auto Err = TSM.withModuleDo([&](llvm::Module &M) -> llvm::Error {
          if (auto Err = applyDataLayout(M))
            return Err;

          recordCtorDtors(M);
          return llvm::Error::success();
        }))
      return Err;

    return LLJIT::addIRModule(*Main, std::move(TSM));
  }

  // Add an object file and it key to the Main JITDylib
  llvm::Error addObjectFile(std::unique_ptr<llvm::MemoryBuffer> Obj,
                            llvm::orc::VModuleKey K) {
    return ObjTransformLayer.add(*Main, std::move(Obj), std::move(K));
  }

  /// Run the initializers
  llvm::Error initialize() { return LLJIT::initialize(*Main); }

  /// Run the initializers, i.e. static constructors, by name.
  llvm::Error initialize(const std::vector<std::string> &Ctors) {
    using CtorTy = void (*)();
    for (const std::string &Name : Ctors) {
      if (auto Sym = lookupLinkerMangled(Name)) {
        auto Ctor =
            reinterpret_cast<CtorTy>(static_cast<uintptr_t>(Sym->getAddress()));
        Ctor();
      } else
        return Sym.takeError();
    }
    return llvm::Error::success();
  }
};

/// Constructs LLJIT2 instances.
class LLJIT2Builder
    : public llvm::orc::LLJITBuilderState,
      public llvm::orc::LLJITBuilderSetters<LLJIT2, LLJIT2Builder,
                                            llvm::orc::LLJITBuilderState> {};

} // End namespace DeviceBackend
} // End namespace OpenCL
} // End namespace Intel

#endif // INTEL_OPENCL_BACKEND_CPU_LLJIT2_H
