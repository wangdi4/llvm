// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "cl_sys_info.h"
#include <buildversion.h>

////////////////////////////////////////////////////////////////////////////////
// return the product version:
// Arguments - major, minor, revision, build - output version numbers
////////////////////////////////////////////////////////////////////
const char* Intel::OpenCL::Utils::GetModuleProductVersion()
{
    return VERSIONSTRING;
}
