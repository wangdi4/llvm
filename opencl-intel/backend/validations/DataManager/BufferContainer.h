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

#ifndef __BUFFER_CONTAINER_H__
#define __BUFFER_CONTAINER_H__

#include "BufferDesc.h"
#include "IBufferContainer.h"
#include "IMemoryObject.h"
#include "ImageDesc.h"
#include <cstddef> // for std::size_t
#include <vector>

namespace Validation {
class BufferContainerList;
class IContainerVisitor;

class BufferContainer : public IBufferContainer {
public:
  /// ctor
  BufferContainer() {}
  /// dtor. Should be called only by BufferContainerList object
  virtual ~BufferContainer();
  BufferContainer(const BufferContainer &) = delete;
  BufferContainer &operator=(const BufferContainer &) = delete;

  std::size_t GetMemoryObjectCount() const override;
  IMemoryObject *GetMemoryObject(std::size_t buffId) const override;
  IMemoryObject *CreateBuffer(const BufferDesc &buffDesc) override;
  IMemoryObject *CreateImage(const ImageDesc &imDesc) override;

  void Accept(IContainerVisitor &visitor) const override;

private:
  /// typedef for MemoryObjectList
  typedef std::vector<IMemoryObject *> MemoryObjectList;
  /// List of buffers
  MemoryObjectList m_buffs;
  /// declare friend for access private members
  friend class BufferContainerList;
};
} // namespace Validation

#endif // __BUFFERS_CONTEINER_H__
