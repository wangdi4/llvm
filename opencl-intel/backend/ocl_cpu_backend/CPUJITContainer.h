// INTEL CONFIDENTIAL
//
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

#pragma once

#include "CPUCompiler.h"
#include "Kernel.h"
#include "Serializer.h"
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"

namespace llvm {
class Module;
class Function;
} // namespace llvm

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class CPUJITContainer : public IKernelJITContainer {
public:
  CPUJITContainer();

  CPUJITContainer(const void *pFuncCode, llvm::Function *pFunction,
                  llvm::Module *pModule, KernelJITProperties *pProps);
  virtual ~CPUJITContainer();

  CPUJITContainer(const CPUJITContainer &) = delete;
  CPUJITContainer &operator=(const CPUJITContainer &) = delete;

  /*
   * ICLDevBackendJITContainer methods
   */
  virtual const void *GetJITCode() const override { return m_pFuncCode; }
  virtual size_t GetJITCodeSize() const override {
    return 0;
  } // TODO: Check this later
  virtual int GetLineNumber(void * /*pointer*/) const override { return -1; }

  /*
   * IJITContainer methods
   */
  const std::string &GetFunctionName() const override { return m_FuncName; }
  void SetJITCode(const void *addr) override { m_pFuncCode = addr; }
  KernelJITProperties *GetProps() const override { return m_pProps; }

  /**
   * Serialization methods for the class (used by the serialization service)
   */
  void Serialize(IOutputStream &ost, SerializationStatus *stats) const override;
  void Deserialize(IInputStream &ist, SerializationStatus *stats) override;

private:
  const void *m_pFuncCode;
  llvm::Function *m_pFunction;
  std::string m_FuncName;
  llvm::Module *m_pModule; // not owned by the class

  KernelJITProperties *m_pProps;
};
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
