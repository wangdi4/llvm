// icd_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <CL/cl.h>

int main(void)
{
	cl_int err = CL_SUCCESS;
	cl_uint uiNumPlatforms = 0;
	cl_uint uiNumDevices = 0;
	cl_platform_id * pPlatforms = NULL;
	cl_device_id * pDevices = NULL;

	err = clGetPlatformIDs(0, NULL, &uiNumPlatforms);
	pPlatforms = new cl_platform_id[uiNumPlatforms];

	err = clGetPlatformIDs(uiNumPlatforms, pPlatforms, NULL);

	if (CL_SUCCESS != err)
	{
		return -1;
	}

	err = clGetDeviceIDs(pPlatforms[0], CL_DEVICE_TYPE_CPU, 0, NULL, &uiNumDevices);
	pDevices = new cl_device_id[uiNumDevices];

	err = clGetDeviceIDs(pPlatforms[0], CL_DEVICE_TYPE_CPU, uiNumDevices, pDevices, NULL);

	if (CL_SUCCESS != err)
	{
		return -1;
	}

	cl_uint uiVendorId = -1;
	err = clGetDeviceInfo(pDevices[0], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uiVendorId, NULL);
	printf ("Vendor id = %d\n", uiVendorId);

	err = clGetDeviceInfo((cl_device_id)12345, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uiVendorId, NULL);

	cl_context context = clCreateContext(NULL, 1, pDevices, NULL, NULL, NULL);

	return 0;
}

