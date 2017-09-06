#include <CL/cl.h>
#include <stdio.h>

// Need to know if ITT/GPA is active, to determine number of iterations - to prevent timeouts.
#include "../../utils/cl_sys_utils/export/cl_config.h"

#include "FrameworkTest.h"
//|
//| TEST: Memoryleak.CreateReleaseOOOQueueTest
//|
//| Purpose 
//| -------
//|
//| Test all memory is released when releasing an out of order command queue
//|
//| Method
//| ------
//|
//| 1. Create a context with a single device
//| 2. Repeatedly create and release an OOO queue
//|
//| Pass criteria
//| -------------
//|
//| No crash due to running out of memory

extern cl_device_type gDeviceType;


#define ITERATION_COUNT 500000
#define REDUCED_ITERATION_COUNT 5000

bool CreateReleaseOOOQueueTest()
{
	printf("CreateReleaseOOOQueueTest\n");
	cl_uint                     uiNumDevices = 0;
	cl_device_id*               pDevices;
	cl_context                  context;
	cl_command_queue            queue;
	cl_command_queue_properties queue_properties;
	cl_device_id                device_id;

	bool bResult = true;
	cl_int iRet = CL_SUCCESS;

	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
	bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}


	pDevices = new cl_device_id[uiNumDevices];
	iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
	bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

	device_id = pDevices[0];

	iRet = clGetDeviceInfo(device_id, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, sizeof(cl_command_queue_properties), &queue_properties, NULL);
	bResult &= SilentCheck("clGetDeviceInfo (CL_DEVICE_QUEUE_ON_HOST_PROPERTIES)", CL_SUCCESS, iRet);

	if (!bResult)
	{
		delete []pDevices;
		return bResult;
	}

	if (!(queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE))
	{
		printf("Device doesn't support OOO queues. Passing vacuously\n");
		delete[] pDevices;
		return true;
	}

	context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
    delete []pDevices;
	bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

    size_t numOfInterations = ITERATION_COUNT;

    /* 
     * Check if GPA/ITT is active
     * see: CSSD100013767 - [CI] FrameworkTestType_gpa: Test_CreateReleaseOOOQueueTest timeout
     */
    std::string strUseGPAVal;
    Intel::OpenCL::Utils::GetEnvVar(strUseGPAVal, "CL_CONFIG_USE_GPA");
    bool bUseGPA = Intel::OpenCL::Utils::ConfigFile::ConvertStringToType<bool>(strUseGPAVal);
    Intel::OpenCL::Utils::GetEnvVar(strUseGPAVal, "CL_CONFIG_USE_ITT");
    bUseGPA |= Intel::OpenCL::Utils::ConfigFile::ConvertStringToType<bool>(strUseGPAVal);
    if (bUseGPA) {
        numOfInterations = REDUCED_ITERATION_COUNT;        
    }

	for (size_t i = 0; i < numOfInterations; ++i)
	{
    	queue = clCreateCommandQueue(context, device_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &iRet);
	    bResult &= SilentCheck("clCreateCommandQueue", CL_SUCCESS, iRet);

	    if (!bResult)
	    {
            clReleaseContext(context);
		    return bResult;
	    }

		iRet = clReleaseCommandQueue(queue);
	    bResult &= SilentCheck("clReleaseCommandQueue", CL_SUCCESS, iRet);

	    if (!bResult)
	    {
		    clReleaseContext(context);
		    return bResult;
	    }
	}

    clReleaseContext(context);

	return bResult;
}
