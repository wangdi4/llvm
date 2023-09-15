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

#ifndef __ICONTAINERVISITOR_H__
#define __ICONTAINERVISITOR_H__

#include "IBufferContainer.h"
#include "IBufferContainerList.h"
#include "IMemoryObject.h"

namespace Validation {
/// @brief IContainerVisitor interface
/// Base interface for all the container visitors
class IContainerVisitor {
public:
  virtual ~IContainerVisitor() {}

  virtual void visitImage(const IMemoryObject *pImage) = 0;
  virtual void visitBuffer(const IMemoryObject *pBuffer) = 0;
  virtual void
  visitBufferContainer(const IBufferContainer *pBufferContainer) = 0;
  virtual void visitBufferContainerList(
      const IBufferContainerList *pBufferContainerList) = 0;
};
} // namespace Validation

#endif // __ICONTAINERVISITOR_H__
