/////////////////////////////////////////////////////////////////////////
// cl_utils.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel�s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "cl_env.h"

using namespace Intel::OpenCL::Utils;

#include<windows.h>

cl_err_code Intel::OpenCL::Utils::GetEnvVar(std::string & strVarValue, const std::string strVarName)
{
	char * pBuffer;
	size_t szBufferSizeRet = 0;
	errno_t err = _dupenv_s(&pBuffer, &szBufferSizeRet, strVarName.c_str());
	if ( err || (0 == szBufferSizeRet) || (NULL == pBuffer))
	{
        strVarValue = std::string("");
		return CL_ERR_FAILURE;
	}
    strVarValue = std::string(pBuffer);
    free(pBuffer);
	return CL_SUCCESS;
}
