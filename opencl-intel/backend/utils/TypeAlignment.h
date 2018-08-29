// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __TYPE_ALIGNMRENT_H__
#define __TYPE_ALIGNMRENT_H__

#include "cl_kernel_arg_type.h"
#include "common_dev_limits.h"
#include <cstring>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  TypeAlignment class used to provide alignment and size information
  ///         for cl_kernel_argument and general alignment utilities 
  ///         (aligning offsets and pointers)
  class TypeAlignment
  {
  public:
    
    /// @brief Returns the size of the given argument
    /// @param arg         The argument for which to return its size
    /// @returns The size of the given argument
    static size_t getSize(const cl_kernel_argument& arg);
    
    /// @brief Returns the alignment of the given argument
    /// @param arg         The argument for which to return its alignment
    /// @returns The alignment of the given argument
    static size_t getAlignment(const cl_kernel_argument& arg);
    
    /// @brief Returns offest aligned based on the given alignment
    /// @param alignment    The alignment
    /// @param offset       The offset to align
    /// @returns The offest aligned based on the given alignment
    static size_t align(size_t alignment, size_t offset);
    
    /// @brief Returns pointer aligned based on the given alignment
    /// @param alignment    The alignment
    /// @param pointer       The pointer to align
    /// @returns The pointer aligned based on the given alignment
    static char* align(size_t alignment, const char* ptr);
    
  public:
    // Represents the maximum alignment
    static const size_t MAX_ALIGNMENT = DEV_MAXIMUM_ALIGN;
  };
  
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {


#endif // __TYPE_ALIGNMRENT_H__
