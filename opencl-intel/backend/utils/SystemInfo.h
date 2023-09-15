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

#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

#include <cstddef>
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
namespace Utils {

class SystemInfo {
public:
  SystemInfo(void);
  ~SystemInfo(void);

  static unsigned long long HostTime();

  /// Get executable filename.
  static std::string GetExecutableFilename();
};

} // namespace Utils
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __SYSTEM_INFO_H__
