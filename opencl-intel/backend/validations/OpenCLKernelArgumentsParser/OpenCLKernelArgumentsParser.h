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

#ifndef __OPENCL_KERNEL_ARGUMENTS_PARSER__
#define __OPENCL_KERNEL_ARGUMENTS_PARSER__

#include "Exception.h"
#include "IMemoryObjectDesc.h"
#include "IOpenCLKernelArgumentsParser.h"
#include "TypeDesc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include <list>
#include <stack>
#include <string>

namespace Validation {
class OpenCLKernelArgumentsParser : public IOpenCLKernelArgumentsParser {
public:
  /// @brief parse kernel argument descriptions
  /// @param [IN] programObject LLVM program object
  /// @param [IN] kernelName Name of kernel
  /// @return list of kernel argument descriptions
  OCLKernelArgumentsList
  KernelArgumentsParser(const std::string &kernelName,
                        const llvm::Module *programObject) override;
  /// @brief static function that tries to guess
  /// number of pointed elements by pointer
  /// @param [IN] Args - reference to output of arg parser
  /// @param [IN] globalworksize is the pointer to array that
  /// contains number of work items for dimention i
  /// @param [IN] dim the number of dimentions
  /// @return new list of arguments
  static OCLKernelArgumentsList
  KernelArgHeuristics(const OCLKernelArgumentsList &Args,
                      const size_t *globalworksize, const uint64_t dim);

private:
  /// @brief parse of struct
  /// @param [IN] struct of LLVM program object
  /// @return struct descriptions
  TypeDesc forParserStruct(llvm::StructType *structTy);
};

/// @brief recurively iterates elements of the TypeDesc tree
/// @param [in] head is a refence to head of current sub-tree
/// @param [in] def_size is the number of elemnts in pointer
/// @return new sub-tree
static TypeDesc RecursiveDFS(const TypeDesc &head, const uint64_t def_size) {
  uint64_t i;
  TypeDesc node;
  TypeDesc tmp;

  node = head;
  if (head.GetType() == TPOINTER) {
    node.SetNumberOfElements(def_size);
  }
  for (i = 0; i < head.GetNumOfSubTypes(); ++i) {
    tmp = head.GetSubTypeDesc(i);
    tmp = RecursiveDFS(tmp, def_size);
    node.SetSubTypeDesc(i, tmp);
  }
  return node;
}

} // namespace Validation
#endif // __OPENCL_KERNEL_ARGUMENTS_PARSER__
