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

#ifndef __ICONTAINER_H__
#define __ICONTAINER_H__

namespace Validation {
class IContainerVisitor;
/// @brief IContainer marker interface
/// Intended to mark all data container objects managed by DataManager
class IContainer {
public:
  /// @brief virtual destructor stub
  virtual ~IContainer() {}
  /// @brief visitor accept method
  virtual void Accept(IContainerVisitor &visitor) const = 0;
};
} // namespace Validation

#endif // __ICONTAINER_H__
