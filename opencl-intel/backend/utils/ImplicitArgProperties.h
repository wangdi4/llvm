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

#ifndef __IMPLICIT_ARG_PROPERTIES_H__
#define __IMPLICIT_ARG_PROPERTIES_H__

#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  ImplicitArgProperties struct used to describe each implicit argument
  struct ImplicitArgProperties {
    /// @brief Implicit argument's name
    const char* m_name;
      
    /// @brief Implicit argument's size
    size_t m_size; 
      
    /// @brief Implicit argument's alignment
    size_t m_alignment; 

    /// @brief Indicates if implicit argument is initialized by the wrapper
    bool m_bInitializedByWrapper; 
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARG_PROPERTIES_H__
