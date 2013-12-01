/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CPUBlockToKernelMapper.h

\*****************************************************************************/
#ifndef __CPU_BLOCK_TO_KERNEL_MAPPER_H__
#define __CPU_BLOCK_TO_KERNEL_MAPPER_H__

#include "IBlockToKernelMapper.h"
#include <map>

namespace llvm {class Module;}

namespace Intel { namespace OpenCL { namespace DeviceBackend {
class Program;

  /// CPU mapping OCL20 block at runtime to ICLDevBackendKernel object
  /// Trick here is that there are block_invoke function and kernel cloned from block_invoke function
  /// This separation is needed since kernel's code is modified in optimization Passes 
  /// However code uses block_literal structure pointer to original block_invoke function
  /// This mapper maps original block_invoke function ptr 
  /// to Kernel object cloned from original block_invoke function
  class CPUBlockToKernelMapper : public IBlockToKernelMapper {
  public:
    /// @brief constructs from Program
    /// kernel should have JIT
    /// @param pProgram - Program object
    /// @param pModule - LLVM compiled module
    /// return 
    CPUBlockToKernelMapper(Program* pProgram, const llvm::Module* pModule);

    /// @brief map key to ICLDevBackendKernel object
    /// @param key - unique block id. For CPU supposed to be block function entry point
    ///                               For MIC need to define. Offset, Block number
    /// @return pointer to ICLDevBackendKernel_  object
    virtual const ICLDevBackendKernel_ * Map(const void * key) const;

    /// @brief dtor. No reference counters
    virtual ~CPUBlockToKernelMapper() {}

  private:
    /// map
    std::map<const void *, const ICLDevBackendKernel_ *> m_map;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __CPU_BLOCK_TO_KERNEL_MAPPER_H__
