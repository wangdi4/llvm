// OpenCL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "OpenCLFramework.h"

// Platform APIs
cl_int clGetPlatformInfo(cl_platform_info param_name, 
						 size_t param_value_size, 
						 void* param_value, 
						 size_t* param_value_size_ret)
{
	return CL_NOT_IMPLEMENTED;
}

// Device APIs
cl_int clGetDeviceIDs(cl_device_type device_type, 
					  cl_uint num_entries, 
					  cl_device_id* devices, 
			          cl_uint* num_devices)
{
	return CL_NOT_IMPLEMENTED;
}

cl_int clGetDeviceInfo(cl_device_id device,
					   cl_device_info param_name, 
					   size_t param_value_size, 
					   void* param_value,
					   size_t* param_value_size_ret)
{
	return CL_NOT_IMPLEMENTED;
}
