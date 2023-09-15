// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#ifndef __I_MEMORY_OBJECT_H__
#define __I_MEMORY_OBJECT_H__

#include "BufferDesc.h" // Data Manager library data types
#include "IContainer.h"
#include <string>

namespace Validation {
///////////////////////////////////////////////////
/// Interface to data container objects.
class IMemoryObject : public IContainer {
public:
  /// @brief Provide access to buffer's data.
  /// @return Pointer to memory region with buffer's data.
  virtual void *GetDataPtr() const = 0;
  /// @brief Provide access to buffer's description.
  /// @return buffer's description.
  virtual const IMemoryObjectDesc *GetMemoryObjectDesc() const = 0;
  /// @brief Name of object class.
  /// @return String with the name of the class.
  virtual std::string GetName() const = 0;
};
} // namespace Validation

#endif // __I_MEMORY_OBJECT_H__
