//|
//| TEST: DeviceFissionTest.BackwardsCompatabilityTest
//|
//| Purpose 
//| -------
//|
//| Test backwards compatability of the device fission feature
//|
//| Method
//| ------
//|
//| 1. Create sub devices with CL_DEVICE_PARTITION_BY_COUNTS_EXT property from root device.
//| 2. Create two buffers
//| 3. Write to the first buffer on the sub-device and read from the second
//| 4. Verify the second buffer was updated
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include <CL/cl.h>
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

static bool isLegalPartitionEXT(cl_device_partition_property_ext prop)
{
    const cl_device_partition_property_ext legalProperties[] = {CL_DEVICE_PARTITION_EQUALLY_EXT, CL_DEVICE_PARTITION_BY_COUNTS_EXT, CL_DEVICE_PARTITION_BY_NAMES_INTEL, CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT};
    const size_t numProperties = sizeof(legalProperties) / sizeof(cl_device_partition_property_ext);
    for (size_t i = 0; i < numProperties; ++i)
    {
        if (legalProperties[i] == prop) return true;
    }
    return false;
}

static bool isLegalDomainEXT(cl_device_partition_property_ext domain)
{
    return false;
}

bool fission_backwards_compatability_test()
{
	printf("---------------------------------------\n");
	printf("fission backwards compatability test\n");
	printf("---------------------------------------\n");
	bool bResult = true;
	cl_context context=NULL;
	cl_command_queue cmd_queue=NULL;
	cl_device_id devices[2];
	cl_mem buf;
	cl_int err;
	cl_platform_id platform=NULL;

	//init platform
	err = clGetPlatformIDs(1,&platform,NULL);
	bResult &= SilentCheck(L"clGetPlatformIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	// init Devices (only one CPU...)
	err = clGetDeviceIDs(platform, gDeviceType, 1, devices, NULL);
	bResult &= SilentCheck(L"clGetDeviceIDs",CL_SUCCESS,err);
	if (!bResult)	return bResult;

    size_t szRet;
    cl_device_partition_property_ext supportedTypes[10];
    err = clGetDeviceInfo(devices[0], CL_DEVICE_PARTITION_TYPES_EXT, sizeof(supportedTypes), supportedTypes, &szRet);
	bResult &= SilentCheck(L"clGetDeviceInfo(CL_DEVICE_PARTITION_TYPES_EXT)", CL_SUCCESS, err);
	if (!bResult)	return bResult;
    for (size_t i = 0; i < szRet / sizeof(cl_device_partition_property_ext); ++i)
    {
        if (!isLegalPartitionEXT(supportedTypes[i]))
        {
            printf("Illegal property %p detected at index %lu\n", supportedTypes[i], i);
            return false;
        }
    }

    cl_device_partition_property_ext supportedDomains[10];
    err = clGetDeviceInfo(devices[0], CL_DEVICE_AFFINITY_DOMAINS_EXT, sizeof(supportedDomains), supportedDomains, &szRet);
	bResult &= SilentCheck(L"clGetDeviceInfo(CL_DEVICE_AFFINITY_DOMAINS_EXT)", CL_SUCCESS, err);
	if (!bResult)	return bResult;
    for (size_t i = 0; i < szRet / sizeof(cl_device_partition_property_ext); ++i)
    {
        if (!isLegalDomainEXT(supportedDomains[i]))
        {
            printf("Illegal domain %p detected at index %lu\n", supportedDomains[i], i);
            return false;
        }
    }

	cl_uint num_entries = 1;
	cl_uint num_devices = 1;
	cl_device_partition_property_ext properties[] = {CL_DEVICE_PARTITION_BY_COUNTS_EXT, 1,CL_PARTITION_BY_COUNTS_LIST_END_EXT, CL_PROPERTIES_LIST_END_EXT};
    const size_t num_props = sizeof(properties)/sizeof(properties[0]);
	err = clCreateSubDevicesEXT(devices[0], properties, num_entries, devices + 1, &num_devices);
	bResult &= SilentCheck(L"clCreateSubDevices",CL_SUCCESS,err);
	if (!bResult)	return bResult;

	//init context
	context = clCreateContext(NULL, 2, devices, NULL, NULL, &err);
	bResult &= SilentCheck(L"clCreateContext",CL_SUCCESS,err);
	if (!bResult)	
	{
		clReleaseDeviceEXT(devices[1]);
		return bResult;
	}

	//init Command Queue
	cmd_queue = clCreateCommandQueue(context, devices[1], 0, &err);
	bResult &= SilentCheck(L"clCreateCommandQueue",CL_SUCCESS,err);
	if (!bResult)
    {
		clReleaseContext(context);
		clReleaseDeviceEXT(devices[1]);
		return bResult;
	}	
	
	//Create buffers
	cl_uint data[2] = {0xABCD, 0x1234};
	//buf = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_uint), data, &err); 
    buf = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_uint), NULL, &err); 
	bResult &= SilentCheck(L"clCreateBuffer #1",CL_SUCCESS,err);
	if (!bResult) 
	{
		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
		clReleaseDeviceEXT(devices[1]);
		return bResult;
	}

    err = clEnqueueWriteBuffer(cmd_queue, buf, CL_FALSE, 0, sizeof(cl_uint), data, 0, NULL, NULL);
    bResult &= SilentCheck(L"clEnqueueWriteBuffer", CL_SUCCESS, err);
	err = clEnqueueReadBuffer(cmd_queue, buf, CL_TRUE, 0, sizeof(cl_uint), data + 1, 0, NULL, NULL);
	bResult &= SilentCheck(L"clEnqueueReadBuffer", CL_SUCCESS, err);

    clReleaseMemObject(buf);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
	clReleaseDeviceEXT(devices[1]);
	if (!bResult) return bResult;
	
	return data[0] == data[1];
}
