// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#ifndef __DATAVERSION_H__
#define __DATAVERSION_H__

#include "IBufferContainerList.h"
#include "llvm/IR/Metadata.h"
#include <iomanip>

namespace Validation {
struct DataVersion {
public:
  static void ConvertData(IBufferContainerList *pContainerList,
                          llvm::Function *pKernel);

  static std::string GetDataVersionSignature() {
    return std::string("DataVersion ");
  }

  static uint32_t GetCurrentDataVersion() { return currentVersion; }

  // data version can be in the range from 00001 to 99999
  static uint32_t GetNumOfDigits() {
    static const uint32_t len = 5;
    return len;
  }

  static std::string GetCurrentDataVersionString() {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(GetNumOfDigits());
    ss << currentVersion;
    return ss.str();
  }

private:
  /// current data version
  static const uint32_t currentVersion = 1;
};

} // namespace Validation

#endif // __DATAVERSION_H__
