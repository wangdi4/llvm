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

#ifndef _OCL_KERNEL_BUFFER_CONTAINER_LIST_ALLOCATOR_
#define _OCL_KERNEL_BUFFER_CONTAINER_LIST_ALLOCATOR_

#include "IContainer.h"
#include "IDataReader.h"
#include "OpenCLKernelArgumentsParser.h"

namespace Validation {
/// class - memory allocator
class OCLKernelBufferContainerListAllocator : public IDataReader {
public:
  ///@brief ctor
  ///@param [in] list is a const reference to list of kernel
  /// arguments(OCLKernelArgumentsList)
  OCLKernelBufferContainerListAllocator(const OCLKernelArgumentsList &list)
      : m_list(list) {}
  ///@brief allocate memory for arguments
  ///@param [in out] p is a pointer to instance of BufferContainerList
  /// creates buffer in BufferContainerList for each argument from
  /// OCLKernelArgumentsList
  virtual void Read(IContainer *p) override;

private:
  OCLKernelArgumentsList m_list;
};

} // namespace Validation
#endif
