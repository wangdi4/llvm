// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef __IOPENCL_KERNEL_ARGUMENTS_PARSER__
#define __IOPENCL_KERNEL_ARGUMENTS_PARSER__

#include "IMemoryObjectDesc.h"
#include "llvm/IR/Module.h"
#include <string>
#include <vector>

namespace Validation {
typedef std::vector<IMemoryObjectDescPtr> OCLKernelArgumentsList;
/// @brief Interface to a Kernel Arguments Parser.
class IOpenCLKernelArgumentsParser {
public:
  /// @brief parse of kernel argument descriptions
  /// @param [IN] programObject LLVM program object
  /// @param [IN] kernelName Name of kernel
  /// @return list of kernel argument descriptions
  virtual OCLKernelArgumentsList
  KernelArgumentsParser(const std::string &kernelName,
                        const llvm::Module *programObject) = 0;
  virtual ~IOpenCLKernelArgumentsParser() {}
};

} // namespace Validation
#endif // __OPENCL_KERNEL_ARGUMENTS_PARSER__
