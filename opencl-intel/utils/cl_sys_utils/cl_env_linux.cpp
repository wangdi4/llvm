// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#include "cl_env.h"

using namespace Intel::OpenCL::Utils;

#include<cstdlib>

cl_err_code Intel::OpenCL::Utils::GetEnvVar(std::string & strVarValue, const std::string strVarName)
{
	char * pBuffer;
	pBuffer = getenv(strVarName.c_str());
	if (pBuffer == nullptr)
	{
        strVarValue = std::string("");
		return CL_ERR_FAILURE;
	}
    strVarValue = std::string(pBuffer);
	return CL_SUCCESS;
}
