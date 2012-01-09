//|
//| TEST: DeviceFissionTest.fissionContextTest
//|
//| Purpose 
//| -------
//|
//| Test device fission after context.
//|
//| Method
//| ------
//|
//| 1. Create context on the root device.
//| 2. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS_EXT property from root device.
//| 3. Release the context and the sub devices.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"

bool fission_context_test(){
	printf("---------------------------------------\n");
	printf("fission context test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_context context=NULL;
	cl_device_id device=NULL;
	cl_int err;
	cl_platform_id platform=NULL;

	//init platform
	err = clGetPlatformIDs(1,&platform,NULL);
	bResult = SilentCheck(L"clGetPlatformIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...)
	err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_DEFAULT,1,&device,NULL);
	bResult = SilentCheck(L"clGetDeviceIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	cl_uint numComputeUnits;
	err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &numComputeUnits, NULL);
	bResult = SilentCheck(L"clGetDeviceInfo(CL_DEVICE_MAX_COMPUTE_UNITS)",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	if (numComputeUnits < 2)
	{
    	printf("Not enough compute units, tast passing vacuously\n");
		return true;
	}

	//init context
	context = clCreateContext(NULL,1,&device,NULL,NULL,&err);
	bResult = SilentCheck(L"clCreateContext",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	cl_uint num_entries = 100;
	cl_device_id out_devices[100];
	cl_uint num_devices = 0;

	cl_device_partition_property_ext properties[] = {CL_DEVICE_PARTITION_BY_COUNTS_EXT, numComputeUnits - 1,CL_PARTITION_BY_COUNTS_LIST_END_EXT, CL_PROPERTIES_LIST_END_EXT};
	err = clCreateSubDevicesEXT(device, properties, num_entries, out_devices, &num_devices);
	bResult = SilentCheck(L"clCreateSubDevicesEXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	fflush(stdout);
	err = clReleaseContext(context);
	bResult = SilentCheck(L"clReleaseContext",CL_SUCCESS,err);
	if (!bResult)	return bResult;
	for (size_t i = 0; i < num_devices; i++)
	{
		err = clReleaseDeviceEXT(out_devices[i]);
		bResult = SilentCheck(L"clReleaseDeviceEXT",CL_SUCCESS,err);
	}
	printf("\n---------------------------------------\n");
	printf("fission context test succeeded!\n");
	printf("---------------------------------------\n");
	return bResult;
}