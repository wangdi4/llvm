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

#ifndef __IBUFFER_CONTAINER_LIST_H__
#define __IBUFFER_CONTAINER_LIST_H__

#include "IBufferContainer.h"
#include "IContainer.h"
#include <cstddef> // for std::size_t
#include <vector>

namespace Validation {

/// @brief Interface to a BufferContainerList object
/// this object stores collection of BufferContainer objects
class IBufferContainerList : public IContainer {
public:
  /// @brief Factory method, which creates empty BufferContainer object and puts
  /// it into BufferContainerList container.
  /// @return Pointer to created BufferContainer object interface.
  virtual IBufferContainer *CreateBufferContainer() = 0;
  // Methods to iterate over buffers
  // TODO: implement C++-style iterator for BufferContainer
  /// @brief Method to get the number of BufferContainers
  /// @return number of BufferContainer objects
  virtual std::size_t GetBufferContainerCount() const = 0;
  /// @brief Method to get particular buffer by index.
  /// @param [IN] id Buffer index. Id have to lay inside range [0,
  /// GetMemoryObjectCount() - 1]
  /// @return Pointer to the buffer's interface.
  virtual IBufferContainer *GetBufferContainer(std::size_t id) const = 0;
  /// @brief Method to get data version read from data file.
  /// @return read data version unsigned int.
  virtual uint32_t GetDataVersion() = 0;
  /// @brief Method to set data version read from data file.
  /// @param [IN] inDataVersion, unsigned int.
  virtual void SetDataVersion(uint32_t const inDataVersion) = 0;
};

} // namespace Validation
#endif // __IBUFFER_CONTAINER_LIST_H__
