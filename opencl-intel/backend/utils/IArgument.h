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

#ifndef __IARGUMENT_H__
#define __IARGUMENT_H__

#include <cstddef>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  IArgument interface class descibes the functionality needed
  ///         to be implemented by arguments classes
  class IArgument {
  
  public:
    
    /// @brief Returns the size of this argument
    /// @returns  The size of this argument
    virtual size_t getSize() const = 0;
    
    /// @brief Returns the alignment of this argument
    /// @returns  The alignment of this argument
    virtual size_t getAlignment() const = 0;
    
    /// @brief  Returns the size of this argument with needed alignment considurations 
    ///         (i.e. considarating alignment of the destination pointer of this argument)
    /// @returns  The aligned size of this argument
    virtual size_t getAlignedSize() const = 0;
    
    /// @brief Sets the value of this argument
    /// @param pValue       The src from which to copy the value
    virtual void setValue(const char* pValue) = 0;
  
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IARGUMENT_H__
