// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#ifndef __CPU_BLOCK_TO_KERNEL_MAPPER_H__
#define __CPU_BLOCK_TO_KERNEL_MAPPER_H__

#include "IBlockToKernelMapper.h"
#include <map>

namespace llvm {
class Module;
}

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class Program;

/// CPU mapping OCL20 block at runtime to ICLDevBackendKernel object
/// Trick here is that there are block_invoke function and kernel cloned from
/// block_invoke function This separation is needed since kernel's code is
/// modified in optimization Passes However code uses block_literal structure
/// pointer to original block_invoke function This mapper maps original
/// block_invoke function ptr to Kernel object cloned from original block_invoke
/// function
class CPUBlockToKernelMapper : public IBlockToKernelMapper {
public:
  /// @brief constructs from Program
  /// kernel should have JIT
  /// @param pProgram - Program object
  /// @param pModule - LLVM compiled module
  /// return
  CPUBlockToKernelMapper(Program *pProgram);

  /// @brief map key to ICLDevBackendKernel object
  /// @param key - unique block id. For CPU supposed to be block function entry
  /// point
  /// @return pointer to ICLDevBackendKernel_  object
  virtual const ICLDevBackendKernel_ *Map(const void *key) const override;

  /// @brief dtor. No reference counters
  virtual ~CPUBlockToKernelMapper() {}

private:
  /// map
  std::map<const void *, const ICLDevBackendKernel_ *> m_map;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __CPU_BLOCK_TO_KERNEL_MAPPER_H__
