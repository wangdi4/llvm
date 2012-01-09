//|
//| TEST: DeviceFissionTest.fissionDeviceInfoSelectorsTest
//|
//| Purpose 
//| -------
//|
//| Test clGetDeviceInfo selectors correctness.
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_EQUALLY_EXT property from root device.
//| 2. Check for support and correctness of all relevant "clGetDeviceInfo" selectors, for sub devices.
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"

bool fission_deviceInfoSelectors_test(){
	printf("---------------------------------------\n");
	printf("fission device info selectors test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
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

	if (numComputeUnits < 5)
	{
    	printf("Not enough compute units, tast passing vacuously\n");
		return true;
	}

	cl_uint num_entries = 100;
	cl_device_id out_devices[100];
	cl_uint num_devices = 0;
	cl_device_partition_property_ext properties[] = {CL_DEVICE_PARTITION_EQUALLY_EXT, 4, CL_PROPERTIES_LIST_END_EXT};
	err = clCreateSubDevicesEXT(device, properties, num_entries, out_devices, &num_devices);
	bResult = SilentCheck(L"clCreateSubDevicesEXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	cl_device_id param;
	size_t actual_size;
	cl_uint ref;

	err = clGetDeviceInfo(out_devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &ref, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo",CL_SUCCESS,err);
	if (!bResult)	return bResult;
	if (4 != ref)
	{
		printf("FAIL: clGetDeviceInfo\n");
		printf("\t\texpected = %d, result = %d\n", 4, ref);
		return false;
	}

	//CL_DEVICE_PARENT_DEVICE_EXT for sub device
	err = clGetDeviceInfo(out_devices[0], CL_DEVICE_PARENT_DEVICE_EXT, sizeof(cl_device_id), &param, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;
	if (device != param)
	{
		printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE_EXT\n");
		printf("\t\texpected = 0x%x, result = 0x%x\n", device, param);
		return false;
	}

	//CL_DEVICE_PARENT_DEVICE_EXT for root device
	err = clGetDeviceInfo(device, CL_DEVICE_PARENT_DEVICE_EXT, sizeof(cl_device_id), &param, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;
	if (NULL != param)
	{
		printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARENT_DEVICE_EXT\n");
		printf("\t\texpected = 0x0, result = 0x%x\n", param);
		return false;
	}

	//CL_DEVICE_PARTITION_TYPES_EXT
	cl_device_partition_property_ext prop[20];
	err = clGetDeviceInfo(out_devices[num_devices-1], CL_DEVICE_PARTITION_TYPES_EXT, 20*sizeof(cl_device_partition_property_ext), &prop, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_PARTITION_TYPES_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;
	if (1 > actual_size)
	{
		printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARTITION_TYPES_EXT\n");
		printf("\t\texpected at least one supported property\n");
		return false;
	}
	for (size_t i = 0; i < actual_size / sizeof(cl_device_partition_property_ext); i++)
	{
		switch (prop[i])
		{
		case CL_DEVICE_PARTITION_EQUALLY_EXT:
		case CL_DEVICE_PARTITION_BY_COUNTS_EXT:
		case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT:
			break;
		default:
			printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARTITION_TYPES_EXT\n");
			printf("\t\tinvalid property\n");
			return false;
		}
	}

	//CL_DEVICE_AFFINITY_DOMAINS_EXT
	err = clGetDeviceInfo(out_devices[num_devices-1], CL_DEVICE_AFFINITY_DOMAINS_EXT, 20*sizeof(cl_device_partition_property_ext), &prop, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_AFFINITY_DOMAINS_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;
	for (size_t i = 0; i < actual_size; i++)
	{
		switch (prop[i])
		{
		case CL_AFFINITY_DOMAIN_NUMA_EXT:
			break;
		default:
			printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_AFFINITY_DOMAINS_EXT\n");
			printf("\t\tinvalid property\n");
			return false;
		}
	}

	//CL_DEVICE_REFERENCE_COUNT_EXT for sub device
	err = clRetainDeviceEXT(out_devices[0]);
	bResult = SilentCheck(L"clRetainDeviceEXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	err = clGetDeviceInfo(out_devices[0], CL_DEVICE_REFERENCE_COUNT_EXT, sizeof(cl_uint), &ref, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	clReleaseDeviceEXT(out_devices[0]);

	if (2 != ref)
	{
		printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT_EXT\n");
		printf("\t\texpected = 2, result = %d\n", ref);
		return false;
	}

	//CL_DEVICE_REFERENCE_COUNT_EXT for root device
	err = clRetainDeviceEXT(device);
	bResult = SilentCheck(L"clRetainDeviceEXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	err = clGetDeviceInfo(device, CL_DEVICE_REFERENCE_COUNT_EXT, sizeof(cl_uint), &ref, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	if (1 != ref)
	{
		printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT_EXT\n");
		printf("\t\texpected = 1, result = %d\n", ref);
		return false;
	}

	//CL_DEVICE_PARTITION_STYLE_EXT for root device
	err = clGetDeviceInfo(device, CL_DEVICE_PARTITION_STYLE_EXT, 20*sizeof(cl_device_partition_property_ext), prop, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	if ((CL_PROPERTIES_LIST_END_EXT != prop[0]) || (1 != actual_size))
	{
		printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARTITION_STYLE_EXT\n");
		printf("\t\texpected only one property: %d, result: %d properties - %d\n", properties[0], actual_size, prop[0]);
		return false;
	}

	//CL_DEVICE_PARTITION_STYLE_EXT for sub device
	err = clGetDeviceInfo(out_devices[0], CL_DEVICE_PARTITION_STYLE_EXT, 20*sizeof(cl_device_partition_property_ext), prop, &actual_size);
	bResult = SilentCheck(L"clGetDeviceInfo for selector CL_DEVICE_REFERENCE_COUNT_EXT",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	for (size_t i = 0; i < actual_size; i++)
	{
		if (prop[i] != properties[i])
		{
			printf("FAIL: clGetDeviceInfo for selector CL_DEVICE_PARTITION_STYLE_EXT\n");
			printf("\t\texpected = %d, result = %d\n", properties[i], prop[i]);
			return false;
		}
	}
	printf("\n---------------------------------------\n");
	printf("fission device info selectors test succeeded!\n");
	printf("---------------------------------------\n");

	for (size_t i = 0; i < num_devices; i++)
	{
		clReleaseDeviceEXT(out_devices[i]);
	}

	return bResult;
}