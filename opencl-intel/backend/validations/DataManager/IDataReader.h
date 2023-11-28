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

#ifndef __I_DATA_READER_H__
#define __I_DATA_READER_H__

#include "IContainer.h"

namespace Validation {
/// @brief Interface to a data file reader. It reads object IContainer from
/// storage
class IDataReader {
public:
  /// @brief Loads from data file.
  /// @param [OUT] pContainer pointer to object with IContainer interface
  virtual void Read(IContainer *pContainer) = 0;
  virtual ~IDataReader() {}
};
} // namespace Validation
#endif // __I_DATA_READER_H__
