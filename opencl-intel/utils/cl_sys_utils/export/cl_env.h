// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#pragma once

#include "cl_utils.h"

#include <string>

namespace Intel {
namespace OpenCL {
namespace Utils {
// Returns false i.f.f the target environment variable is not defined.
// Note: if the target environment variable is set to an empty string, this
// function will return true.
bool getEnvVar(std::string &EnvValue, const std::string &EnvName);
} // namespace Utils
} // namespace OpenCL
} // namespace Intel
