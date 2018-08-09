// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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
#include "crt_types.h"

#include <iostream>
#include <vector>
#include <string>

class CrtConfig
{
public:
    CrtConfig();
    crt_err_code Init();
    cl_uint getNumPlatforms();
    std::string& getPlatformLibName(cl_uint index);
    ~CrtConfig();
private:
	bool emulatorEnabled();

    std::vector<std::string>    m_libraryNames;
};
