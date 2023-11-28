// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
namespace Utils {

/**
 * Splits the given string using the supplied delimiter
 * populates the given vector of strings
 */
void SplitString(const std::string &s, const char *d,
                 std::vector<std::string> &v);

/**
 * Joins the given strings (as a vector of strings) using
 * the supplied delimiter.
 * Returns: joined string
 */
std::string JoinStrings(const std::vector<std::string> &vs, const char *d);

} // namespace Utils
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif
