// Copyright 2010 Intel Corporation.
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

// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS
#pragma once

#include "ObjectCodeCache.h"
#include "Program.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include <memory>

namespace llvm {
class Function;
}

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class CPUProgram : public Program {
public:
  CPUProgram() : LLJITLogStream(LLJITLog) {}
  virtual ~CPUProgram();

  CPUProgram(const CPUProgram &) = delete;
  CPUProgram &operator=(const CPUProgram &) = delete;

  void SetBuiltinModule(
      llvm::SmallVector<llvm::Module *, 2> &bltnFuncList) override {
    m_bltnFuncList = bltnFuncList;
  }

  const std::string &getLLJITLog() const {
    LLJITLogStream.flush();
    return LLJITLog;
  }

  llvm::raw_ostream &getLLJITLogStream() { return LLJITLogStream; }

  void SetExecutionEngine(std::unique_ptr<llvm::ExecutionEngine> EE) override {
    m_pExecutionEngine = std::move(EE);
  }

  llvm::ExecutionEngine *GetExecutionEngine() {
    return m_pExecutionEngine.get();
  }

  void SetLLJIT(std::unique_ptr<llvm::orc::LLJIT> LLJIT) override {
    m_LLJIT = std::move(LLJIT);
  }

  // Get LLJIT instance
  llvm::orc::LLJIT *GetLLJIT() override { return m_LLJIT.get(); }

  void LoadProfileLib() override;

  void ReleaseExecutionEngine();

  // Return the address of a function.
  void *GetPointerToFunction(llvm::StringRef Name) const;

  // Return the address of a global value
  void *GetPointerToGlobalValue(llvm::StringRef Name) const;

  /**
   * Serialization methods for the class (used by the serialization service)
   */
  virtual void Deserialize(IInputStream &ist,
                           SerializationStatus *stats) override;

  void SetObjectCache(ObjectCodeCache *);

  cl_dev_err_code Finalize() override;

  cl_ulong GetFunctionPointerFor(const char *) const override;

  // Get a map from global variable name to its property (size/pointer).
  void GetGlobalVariablePointers(const cl_prog_gv **GVs,
                                 size_t *GVCount) const override;

  void CreateAndSetBlockToKernelMapper();

private:
  using funcPtr = void (*)(void);
  funcPtr llvmWriteoutFilesPtr = nullptr;
  std::unique_ptr<llvm::ExecutionEngine> m_pExecutionEngine;
  std::unique_ptr<llvm::orc::LLJIT> m_LLJIT;
  llvm::SmallVector<llvm::Module *, 2> m_bltnFuncList;
  std::unique_ptr<ObjectCodeCache> m_ObjectCodeCache;
  // Store log from LLJIT
  mutable llvm::raw_string_ostream LLJITLogStream;
  mutable std::string LLJITLog;

  uintptr_t LLJITLookUp(llvm::StringRef Name) const;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
