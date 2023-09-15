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

#ifndef __IBLOCK_TO_KERNEL_MAPPER_H__
#define __IBLOCK_TO_KERNEL_MAPPER_H__

/*
  Interface for mapping enqueued Block to ICLDevBackendKernel object
*/
namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class ICLDevBackendKernel_;

/// interface for mapping OCL20 block at runtime to ICLDevBackendKernel object
class IBlockToKernelMapper {
public:
  /// @brief map key to ICLDevBackendKernel object
  /// @param key - unique block id. For CPU supposed to be block function entry
  /// point
  /// @return pointer to constant ICLDevBackendKernel_  object.
  virtual const ICLDevBackendKernel_ *Map(const void *key) const = 0;

  /// @brief dtor
  virtual ~IBlockToKernelMapper() {}
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __IBLOCK_TO_KERNEL_MAPPER_H__
