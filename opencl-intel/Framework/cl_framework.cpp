// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cl_framework.h"
#include "FrameworkFactory.h"

using namespace Intel::OpenCL::Framework;

wchar_t* ClErrTxt(cl_err_code error_code)
{
	switch(error_code)
	{
	case (CL_INT_NOT_IMPLEMENTED): return L"CL_INT_NOT_IMPLEMENTED";
	case (CL_INT_INITILIZATION_FAILED): return L"CL_INT_INITILIZATION_FAILED";
	case (CL_INT_PLATFORM_FAILED): return L"CL_INT_PLATFORM_FAILED";
	case (CL_INT_CONTEXT_FAILED): return L"CL_INT_CONTEXT_FAILED";
	case (CL_INT_EXECUTION_FAILED): return L"CL_INT_EXECUTION_FAILED";
	default: return L"Unknown Error Code";
	}
}

//////////////////////////////////////////////////////////////////////////
// Platform APIs
//////////////////////////////////////////////////////////////////////////

cl_int clGetPlatformInfo(cl_platform_info param_name, 
						 size_t param_value_size, 
						 void* param_value, 
						 size_t* param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkFactory* pFrameworkFactory = Intel::OpenCL::Framework::FrameworkFactory::Instance();
	if (NULL == pFrameworkFactory)
	{
		// can't initialize framework factory
		CL_INT_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFrameworkFactory->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_INT_PLATFORM_FAILED;
	}
	cl_err_code clRet = pPlatformModule->GetPlatformInfo(param_name, param_value_size, param_value, param_value_size_ret);
	return (CL_ERR_OUT(clRet));
}

//////////////////////////////////////////////////////////////////////////
// Device APIs
//////////////////////////////////////////////////////////////////////////

cl_int clGetDeviceIDs(cl_device_type device_type, 
					  cl_uint num_entries, 
					  cl_device_id* devices, 
			          cl_uint* num_devices)
{
	// get instance of the framework factory class
	FrameworkFactory* pFrameworkFactory = Intel::OpenCL::Framework::FrameworkFactory::Instance();
	if (NULL == pFrameworkFactory)
	{
		// can't initialize framework factory
		CL_INT_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFrameworkFactory->GetPlatformModule();
	if (NULL == pFrameworkFactory)
	{
		CL_INT_PLATFORM_FAILED;
	}
	return CL_INT_NOT_IMPLEMENTED;
}

cl_int clGetDeviceInfo(cl_device_id device,
					   cl_device_info param_name, 
					   size_t param_value_size, 
					   void* param_value,
					   size_t* param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkFactory* pFrameworkFactory = FrameworkFactory::Instance();
	if (NULL == pFrameworkFactory)
	{
		// can't initialize framework factory
		CL_INT_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFrameworkFactory->GetPlatformModule();
	if (NULL == pFrameworkFactory)
	{
		CL_INT_PLATFORM_FAILED;
	}
	return CL_INT_NOT_IMPLEMENTED;
}
