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
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "cl_utils.h"

//using namespace Intel::OpenCL::Utils;

wchar_t* ClErrTxt(cl_err_code error_code)
{
	switch(error_code)
	{
	// OpenCL error codes
	case (CL_SUCCESS): return L"CL_SUCCESS";
	case (CL_DEVICE_NOT_FOUND): return L"CL_DEVICE_NOT_FOUND";
	case (CL_DEVICE_NOT_AVAILABLE): return L"CL_DEVICE_NOT_AVAILABLE";
	case (CL_COMPILER_NOT_AVAILABLE): return L"CL_COMPILER_NOT_AVAILABLE";
	case (CL_MEM_OBJECT_ALLOCATION_FAILURE): return L"CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case (CL_OUT_OF_RESOURCES): return L"CL_OUT_OF_RESOURCES";
	case (CL_OUT_OF_HOST_MEMORY): return L"CL_OUT_OF_HOST_MEMORY";
	case (CL_PROFILING_INFO_NOT_AVAILABLE): return L"CL_PROFILING_INFO_NOT_AVAILABLE";
	case (CL_MEM_COPY_OVERLAP): return L"CL_MEM_COPY_OVERLAP";
	case (CL_IMAGE_FORMAT_MISMATCH): return L"CL_IMAGE_FORMAT_MISMATCH";
	case (CL_IMAGE_FORMAT_NOT_SUPPORTED): return L"CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case (CL_BUILD_PROGRAM_FAILURE): return L"CL_BUILD_PROGRAM_FAILURE";
	case (CL_MAP_FAILURE): return L"CL_MAP_FAILURE";
	case (CL_INVALID_VALUE): return L"CL_INVALID_VALUE";
	case (CL_INVALID_DEVICE_TYPE): return L"CL_INVALID_DEVICE_TYPE";
	case (CL_INVALID_PLATFORM): return L"CL_INVALID_PLATFORM";
	case (CL_INVALID_DEVICE): return L"CL_INVALID_DEVICE";
	case (CL_INVALID_CONTEXT): return L"CL_INVALID_CONTEXT";
	case (CL_INVALID_QUEUE_PROPERTIES): return L"CL_INVALID_QUEUE_PROPERTIES";
	case (CL_INVALID_COMMAND_QUEUE): return L"CL_INVALID_COMMAND_QUEUE";
	case (CL_INVALID_HOST_PTR): return L"CL_INVALID_HOST_PTR";
	case (CL_INVALID_MEM_OBJECT): return L"CL_INVALID_MEM_OBJECT";
	case (CL_INVALID_IMAGE_FORMAT_DESCRIPTOR): return L"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case (CL_INVALID_IMAGE_SIZE): return L"CL_INVALID_IMAGE_SIZE";
	case (CL_INVALID_SAMPLER): return L"CL_INVALID_SAMPLER";
	case (CL_INVALID_BINARY): return L"CL_INVALID_BINARY";
	case (CL_INVALID_BUILD_OPTIONS): return L"CL_INVALID_BUILD_OPTIONS";
	case (CL_INVALID_PROGRAM): return L"CL_INVALID_PROGRAM";
	case (CL_INVALID_PROGRAM_EXECUTABLE): return L"CL_INVALID_PROGRAM_EXECUTABLE";
	case (CL_INVALID_KERNEL_NAME): return L"CL_INVALID_KERNEL_NAME";
	case (CL_INVALID_KERNEL_DEFINITION): return L"CL_INVALID_KERNEL_DEFINITION";
	case (CL_INVALID_KERNEL): return L"CL_INVALID_KERNEL";
	case (CL_INVALID_ARG_INDEX): return L"CL_INVALID_ARG_INDEX";
	case (CL_INVALID_ARG_VALUE): return L"CL_INVALID_ARG_VALUE";
	case (CL_INVALID_ARG_SIZE): return L"CL_INVALID_ARG_SIZE";
	case (CL_INVALID_KERNEL_ARGS): return L"CL_INVALID_KERNEL_ARGS";
	case (CL_INVALID_WORK_DIMENSION): return L"CL_INVALID_WORK_DIMENSION";
	case (CL_INVALID_WORK_GROUP_SIZE): return L"CL_INVALID_WORK_GROUP_SIZE";
	case (CL_INVALID_WORK_ITEM_SIZE): return L"CL_INVALID_WORK_ITEM_SIZE";
	case (CL_INVALID_GLOBAL_OFFSET): return L"CL_INVALID_GLOBAL_OFFSET";
	case (CL_INVALID_EVENT_WAIT_LIST): return L"CL_INVALID_EVENT_WAIT_LIST";
	case (CL_INVALID_EVENT): return L"CL_INVALID_EVENT";
	case (CL_INVALID_OPERATION): return L"CL_INVALID_GL_OBJECT";
	case (CL_INVALID_GL_OBJECT): return L"CL_INVALID_GL_OBJECT";
	case (CL_INVALID_BUFFER_SIZE): return L"CL_INVALID_BUFFER_SIZE";
	case (CL_INVALID_MIP_LEVEL): return L"CL_INVALID_MIP_LEVEL";

		// OpenCL framework erro codes

	case (CL_ERR_LOGGER_FAILED): return L"CL_ERR_LOGGER_FAILED";
	case (CL_ERR_NOT_IMPLEMENTED): return L"CL_ERR_NOT_IMPLEMENTED";
	case (CL_ERR_NOT_SUPPORTED): return L"CL_ERR_NOT_SUPPORTED";
	case (CL_ERR_INITILIZATION_FAILED): return L"CL_ERR_INITILIZATION_FAILED";
	case (CL_ERR_PLATFORM_FAILED): return L"CL_ERR_PLATFORM_FAILED";
	case (CL_ERR_CONTEXT_FAILED): return L"CL_ERR_CONTEXT_FAILED";
	case (CL_ERR_EXECUTION_FAILED): return L"CL_ERR_EXECUTION_FAILED";
	case (CL_ERR_FILE_NOT_EXISTS): return L"CL_ERR_FILE_NOT_EXISTS";
	case (CL_ERR_KEY_NOT_FOUND): return L"CL_ERR_KEY_NOT_FOUND";
	case (CL_ERR_KEY_ALLREADY_EXISTS): return L"CL_ERR_KEY_ALLREADY_EXISTS";
	case (CL_ERR_LIST_EMPTY): return L"CL_ERR_LIST_EMPTY";
	case (CL_ERR_DEVICE_INIT_FAIL): return L"CL_ERR_DEVICE_INIT_FAIL";
	case (CL_ERR_FE_COMPILER_INIT_FAIL): return L"CL_ERR_FE_COMPILER_INIT_FAIL";
	default: 
		return L"Unknown Error Code";
	}
}
