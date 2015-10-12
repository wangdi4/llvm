#include <iostream>
#include "CL/cl.h"
#include "test_utils.h"
#include "CL/cl_platform.h"

extern cl_device_type gDeviceType;

bool Timers()
{
    std::cout << "=============================================================" << std::endl;
    std::cout << "Timers" << std::endl;
    std::cout << "=============================================================" << std::endl;

    cl_int iRet = CL_SUCCESS;

    //Get platform.
    cl_platform_id platform = 0;
    iRet = clGetPlatformIDs(1, &platform, NULL);
    CheckException(L"clGetPlatformIDs", CL_SUCCESS, iRet);

    //Get device.
    cl_device_id device = NULL;
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    CheckException(L"clGetDeviceIDs", CL_SUCCESS, iRet);

    //Get host time.
    cl_ulong host_timer = 0;
    {
        //Wrong prameters for call.
        iRet = clGetHostTimer(cl_device_id(-1), &host_timer);
        CheckException(L"clGetHostTimer with invalid device", CL_INVALID_DEVICE, iRet);

        iRet = clGetHostTimer(device, NULL);
        CheckException(L"clGetHostTimer with invalid ptr to host_timer", CL_INVALID_VALUE, iRet);

        host_timer = 0;
        //Proper prameters for call.
        iRet = clGetHostTimer(device, &host_timer);
        CheckException(L"clGetHostTimer", CL_SUCCESS, iRet);
    }
    if(!host_timer)
    {
        std::cout << "Looks like clGetHostTimer has not set host time." << std::endl;
        return false;
    }

    cl_ulong device_timer = 0;
    {
        //Wrong prameters for call.
        iRet = clGetDeviceAndHostTimer(cl_device_id(-1), &device_timer, &host_timer);
        CheckException(L"clGetDeviceAndHostTimer with invalid device", CL_INVALID_DEVICE, iRet);

        iRet = clGetDeviceAndHostTimer(device, NULL, &host_timer);
        CheckException(L"clGetDeviceAndHostTimer with invalid ptr to device_timer", CL_INVALID_VALUE, iRet);

        iRet = clGetDeviceAndHostTimer(device, &device_timer, NULL);
        CheckException(L"clGetDeviceAndHostTimer with invalid ptr to host_timer", CL_INVALID_VALUE, iRet);

        device_timer = 0;
        host_timer = 0;
        //Proper prameters for call.
        iRet = clGetDeviceAndHostTimer(device, &device_timer, &host_timer);
        CheckException(L"clGetDeviceAndHostTimer", CL_SUCCESS, iRet);
    }
    if(!host_timer)
    {
        std::cout << "Looks like clGetDeviceAndHostTimer has not set host time." << std::endl;
        return false;
    }
    if(!device_timer)
    {
        std::cout << "Looks like clGetDeviceAndHostTimer has not set device time." << std::endl;
        return false;
    }
    if((host_timer/10000) != (device_timer/10000))
    {
        std::cout << "Looks like clGetDeviceAndHostTimer returns async timestamps. Very slow system?" << std::endl;
        return false;
    }

    return true;
}
