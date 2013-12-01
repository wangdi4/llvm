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

File Name:  BlockUtils.h

\*****************************************************************************/

#ifndef __BLOCK_UTILS_H__
#define __BLOCK_UTILS_H__

#include "llvm/IR/Function.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
  /// @brief  BlockUtils class used to provide helper utilies handling block_invoke functions
  ///         used in several modules related to OCL2.0 Extended Execution 
  class BlockUtils {

  public:
    /// @brief Is llvm function a block invoke functions
    /// @return - true if function is block invoke function
    static bool isBlockInvokeFunction(const llvm::Function& F);

    /// @brief Create name for kernel created from block_invoke function
    /// @return - string with name
    static std::string CreateBlockInvokeKernelName(const std::string& F);

    /// @brief Is kernel created from block_invoke function
    /// @param F - llvm function with kernel
    /// @return - true if kernel was created from block invoke function
    static bool IsBlockInvocationKernel(const llvm::Function& F);
    
    /// @brief Obtain name of original block_invoke function from kernel function
    /// @param str - kernel function name
    /// @return - string with block_invoke function name
    static std::string ObtainBlockInvokeFuncNameFromKernel(const std::string& str);
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __BLOCK_UTILS_H__
