// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef __IBUFFER_CONTAINER_H__
#define __IBUFFER_CONTAINER_H__

#include "BufferDesc.h"
#include "IContainer.h"
#include "IMemoryObject.h"
#include "ImageDesc.h"
#include <cstddef> // for std::size_t

namespace Validation {
/// @brief Interface to a container of buffers
class IBufferContainer : public IContainer {
public:
  /// @brief Factory method, which creates buffer and puts it into buffer
  /// container.
  /// @param [IN] buffDesc Description of buffer to create.
  /// @return Pointer to created buffer.
  virtual IMemoryObject *CreateBuffer(const BufferDesc &buffDesc) = 0;

  /// @brief Factory method, which creates buffer and puts it into buffer
  /// container.
  /// @param [IN] buffDesc Description of buffer to create.
  /// @return Pointer to created buffer.
  virtual IMemoryObject *CreateImage(const ImageDesc &imDesc) = 0;

  // Methods to iterate over buffers
  // TODO: implement C++-style iterator for BufferContainer

  /// @brief Method to get the number of buffers in buffer container.
  /// @return number of buffers
  virtual std::size_t GetMemoryObjectCount() const = 0;
  /// @brief Method to get particular buffer by index.
  /// @param [IN] id Buffer index. Id have to lay inside range [0,
  /// GetMemoryObjectCount() - 1]
  /// @return Pointer to the buffer's interface.
  virtual IMemoryObject *GetMemoryObject(std::size_t id) const = 0;
};

} // namespace Validation

#endif // __IBUFFERS_CONTAINER_H__
