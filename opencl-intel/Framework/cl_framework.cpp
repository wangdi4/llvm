// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cl_framework.h"
#include "framework_proxy.h"

using namespace Intel::OpenCL::Framework;

wchar_t* ClErrTxt(cl_err_code error_code)
{
	switch(error_code)
	{
	case (CL_ERR_NOT_IMPLEMENTED): return L"CL_ERR_NOT_IMPLEMENTED";
	case (CL_ERR_INITILIZATION_FAILED): return L"CL_ERR_INITILIZATION_FAILED";
	case (CL_ERR_PLATFORM_FAILED): return L"CL_ERR_PLATFORM_FAILED";
	case (CL_ERR_CONTEXT_FAILED): return L"CL_ERR_CONTEXT_FAILED";
	case (CL_ERR_EXECUTION_FAILED): return L"CL_ERR_EXECUTION_FAILED";
	default: return L"Unknown Error Code";
	}
}

cl_int TEST(cl_int test_number)
{
	OCLObjectsMap *pObjMap = new OCLObjectsMap();
	OCLObject *pObj1 = new OCLObject();
	OCLObject *pObj2 = new OCLObject();
	OCLObject *pObj3 = new OCLObject();

	cl_err_code clErrRet = CL_SUCCESS;

	cl_int id = pObjMap->AddObject(pObj1);
	id = pObjMap->AddObject(pObj2);
	id = pObjMap->AddObject(pObj3);
	clErrRet = pObjMap->RemoveObject(pObj1->GetId());
	clErrRet = pObjMap->RemoveObject(pObj2->GetId());
	clErrRet = pObjMap->RemoveObject(pObj3->GetId());
	clErrRet = pObjMap->RemoveObject(pObj1->GetId());
	return CL_SUCCESS;
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
	FrameworkProxy* pFramework = Intel::OpenCL::Framework::FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_ERR_PLATFORM_FAILED;
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
	FrameworkProxy* pFramework = Intel::OpenCL::Framework::FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_ERR_PLATFORM_FAILED;
	}
	cl_err_code clRet = pPlatformModule->GetDeviceIDs(device_type, num_entries, devices, num_devices);
	return (CL_ERR_OUT(clRet));
}

cl_int clGetDeviceInfo(cl_device_id device,
					   cl_device_info param_name, 
					   size_t param_value_size, 
					   void* param_value,
					   size_t* param_value_size_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the platform module
	PlatformModule *pPlatformModule = pFramework->GetPlatformModule();
	if (NULL == pPlatformModule)
	{
		CL_ERR_PLATFORM_FAILED;
	}
	cl_err_code clRet = pPlatformModule->GetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
	return (CL_ERR_OUT(clRet));
}

cl_context clCreateContext(cl_context_properties properties,
					   cl_uint num_devices,
					   const cl_device_id * devices,
					   logging_fn pfn_notify,
					   void * user_data,
					   cl_int * errcode_ret)
{
	// get instance of the framework factory class
	FrameworkProxy* pFramework = FrameworkProxy::Instance();
	if (NULL == pFramework)
	{
		// can't initialize framework factory
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the platform module
	ContextModule *pContextModule = pFramework->GetContextModule();
	if (NULL == pContextModule)
	{
		CL_ERR_PLATFORM_FAILED;
	}
	cl_err_code clRet = CL_SUCCESS;
	cl_context iContextId = pContextModule->CreateContext(properties, num_devices, devices, pfn_notify, user_data, &clRet);
	*errcode_ret = CL_ERR_OUT(clRet);
	return iContextId;
}