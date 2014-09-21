/*****************************************************************************
 * Copyright (c) 2013-2014 Intel Corporation
 * All rights reserved.
 *
 * WARRANTY DISCLAIMER
 *
 * THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
 * MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Intel Corporation is the author of the Materials, and requests that all
 * problem reports or change requests be submitted to it directly
 *****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include <vector>

#include "CL\cl.h"
#include "utils.h"
#include "CLApiWrapper.h"
#include "CLApiWrapper.h"
#include "cl_utils.h"

//for perf. counters
#include <Windows.h>

// Macros for OpenCL versions
#define OPENCL_VERSION_1_2  1.2f
#define OPENCL_VERSION_2_0  2.0f

struct ocl_args_d_t
{
    ocl_args_d_t();
    ~ocl_args_d_t();

    // Regular OpenCL objects:
    cl_context       context;           // hold the context handler
    cl_device_id     device;            // hold the selected device handler
    cl_int           minAlign;          // size (in bytes) of largest OpenCL built-in data type supported by the device
    cl_command_queue commandQueue;      // hold the commands-queue handler
    cl_command_queue commandQueue2;      // hold the commands-queue handler
    cl_program       program;           // hold the program handler
    cl_kernel        kernel;            // hold the kernel handler
    float            platformVersion;   // hold the OpenCL platform version (default 1.2)
    float            deviceVersion;     // hold the OpenCL device version (default. 1.2)
    float            compilerVersion;   // hold the device OpenCL C version (default. 1.2)

    // Objects that are specific for algorithm implemented in this sample
    cl_mem           srcA;              // hold first source buffer
    cl_mem           srcB;              // hold second source buffer
    cl_mem           dstMem;            // hold destination buffer
    ocl_wrap_data* wrap_data;        // hold data needed for the OCL Wrappers
};


/* Convenient container for all OpenCL specific objects used in the sample
 *
 * It consists of two parts:
 *   - regular OpenCL objects which are used in almost each normal OpenCL applications
 *   - several OpenCL objects that are specific for this particular sample
 *
 * You collect all these objects in one structure for utility purposes
 * only, there is no OpenCL specific here: just to avoid global variables
 * and make passing all these arguments in functions easier.
 */
ocl_args_d_t::ocl_args_d_t():
        context(NULL),
        device(NULL),
        minAlign(0),
        commandQueue(NULL),
        commandQueue2(NULL),
        program(NULL),
        kernel(NULL),
        platformVersion(OPENCL_VERSION_1_2),
        deviceVersion(OPENCL_VERSION_1_2),
        compilerVersion(OPENCL_VERSION_1_2),
        srcA(NULL),
        srcB(NULL),
        dstMem(NULL)
{
    wrap_data = new ocl_wrap_data();
}

/*
 * destructor - called only once
 * Release all OpenCL objects
 * This is a regular sequence of calls to deallocate all created OpenCL resources in bootstrapOpenCL.
 *
 * You may want to call these deallocation procedures in the middle of your application execution
 * (not at the end) if you don't further need OpenCL runtime.
 * You may want to do that in order to free some memory, for example,
 * or recreate OpenCL objects with different parameters.
 *
 */
ocl_args_d_t::~ocl_args_d_t()
{
    // all this code moved to destroyOclArgsObject function

    // code here moved to destroyOclArgsObject.

    delete wrap_data;
    wrap_data = NULL;

    /*
     * Note there is no procedure to deallocate platform
     * because it was not created at the startup,
     * but just queried from OpenCL runtime.
     */
}

cl_int destroyOclArgsObject(ocl_args_d_t* ocl)
{
    cl_int err= CL_SUCCESS;

    if (ocl->kernel)
    {
        err = ReleaseKernel(ocl->kernel, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseKernel returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->program)
    {
        err = ReleaseProgram(ocl->program, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseProgram returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->srcA)
    {
        err = ReleaseMemObject(ocl->srcA, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseMemObject returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->srcB)
    {
        err = ReleaseMemObject(ocl->srcB, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseMemObject returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->dstMem)
    {
        err = ReleaseMemObject(ocl->dstMem, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseMemObject returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->commandQueue)
    {
        err = ReleaseCommandQueue(ocl->commandQueue, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseCommandQueue returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->commandQueue2)
    {
        err = ReleaseCommandQueue(ocl->commandQueue2, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseCommandQueue returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->device)
    {
        err = ReleaseDevice(ocl->device, ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseDevice returned '%s'.\n", ClErrTxt(err));
        }
    }
    if (ocl->context)
    {
        err = ReleaseContext(ocl->context,ocl->wrap_data);
        if (CL_SUCCESS != err)
        {
            LogError("Error: clReleaseContext returned '%s'.\n", ClErrTxt(err));
        }
    }

    return err;
}

/*
 * Check whether an OpenCL platform is the required platform
 * (based on the platform's name)
 */
bool CheckPreferredPlatformMatch(ocl_args_d_t* ocl, cl_platform_id platform, const char* preferredPlatform)
{
    size_t stringLength = 0;
    cl_int err = CL_SUCCESS;
    bool match = false;

    // In order to read the platform's name, we first read the platform's name string length (param_value is NULL).
    // The value returned in stringLength
    err = GetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &stringLength, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetPlatformInfo() to get CL_PLATFORM_NAME length returned '%s'.\n", ClErrTxt(err));
        return false;
    }

    // Now, that we know the platform's name string length, we can allocate enough space before read it
    std::vector<char> platformName(stringLength);

    // Read the platform's name string
    // The read value returned in platformName
    err = GetPlatformInfo(platform, CL_PLATFORM_NAME, stringLength, &platformName[0], NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get CL_PLATFORM_NAME returned %s.\n", ClErrTxt(err));
        return false;
    }

    // Now check if the platform's name is the required one
    if (strstr(&platformName[0], preferredPlatform) != 0)
    {
        // The checked platform is the one we're looking for
        match = true;
    }

    return match;
}

/*
 * Find and return the preferred OpenCL platform
 * In case that preferredPlatform is NULL, the ID of the first discovered platform will be returned
 */
cl_platform_id FindOpenCLPlatform(ocl_args_d_t* ocl, const char* preferredPlatform, cl_device_type deviceType)
{
    cl_uint numPlatforms = 0;
    cl_int err = CL_SUCCESS;

    // Get (in numPlatforms) the number of OpenCL platforms available
    // No platform ID will be return, since platforms is NULL
    err = GetPlatformIDs(0, NULL, &numPlatforms, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get num platforms returned %s.\n", ClErrTxt(err));
        return NULL;
    }
    LogInfo("Number of available platforms: %u\n", numPlatforms);

    if (0 == numPlatforms)
    {
        LogError("Error: No platforms found!\n");
        return NULL;
    }

    std::vector<cl_platform_id> platforms(numPlatforms);

    // Now, obtains a list of numPlatforms OpenCL platforms available
    // The list of platforms available will be returned in platforms
    err = GetPlatformIDs(numPlatforms, &platforms[0], NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get platforms returned %s.\n", ClErrTxt(err));
        return NULL;
    }

    // Check if one of the available platform matches the preferred requirements
    for (cl_uint i = 0; i < numPlatforms; i++)
    {
        bool match = true;
        cl_uint numDevices = 0;

        // If the preferredPlatform is not NULL then check if platforms[i] is the required one
        // Otherwise, continue the check with platforms[i]
        if ((NULL != preferredPlatform) && (strlen(preferredPlatform) > 0))
        {
            // In case we're looking for a specific platform
            match = CheckPreferredPlatformMatch(ocl, platforms[i], preferredPlatform);
        }

        // match is true if the platform's name is the required one or don't care (NULL)
        if (match)
        {
            // Obtains the number of deviceType devices available on platform
            // When the function failed we expect numDevices to be zero.
            // We ignore the function return value since a non-zero error code
            // could happen if this platform doesn't support the specified device type.
            err = GetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices, ocl->wrap_data);
            if (CL_SUCCESS != err)
            {
                LogError("clGetDeviceIDs() returned %s.\n", ClErrTxt(err));
            }

            if (0 != numDevices)
            {
                // There is at list one device that answer the requirements
                return platforms[i];
            }
            UnloadPlatformCompiler(platforms[i], ocl->wrap_data);
            exit(1);
        }

    }

    return NULL;
}


/*
 * This function read the OpenCL platdorm and device versions
 * (using clGetxxxInfo API) and stores it in the ocl structure.
 * Later it will enable us to support both OpenCL 1.2 and 2.0 platforms and devices
 * in the same program.
 */
int GetPlatformAndDeviceVersion (cl_platform_id platformId, ocl_args_d_t *ocl, cl_device_id deviceId)
{
    cl_int err = CL_SUCCESS;

    // Read the platform's version string length (param_value is NULL).
    // The value returned in stringLength
    size_t stringLength = 0;
    err = GetPlatformInfo(platformId, CL_PLATFORM_VERSION, 0, NULL, &stringLength, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: GetPlatformInfo() to get CL_PLATFORM_VERSION length returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    // Now, that we know the platform's version string length, we can allocate enough space before read it
    std::vector<char> platformVersion(stringLength);

    // Read the platform's version string
    // The read value returned in platformVersion
    err = GetPlatformInfo(platformId, CL_PLATFORM_VERSION, stringLength, &platformVersion[0], NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetplatform_ids() to get CL_PLATFORM_VERSION returned %s.\n", ClErrTxt(err));
        return err;
    }

    if (strstr(&platformVersion[0], "OpenCL 2.0") != NULL)
    {
        ocl->platformVersion = OPENCL_VERSION_2_0;
    }
    // Read the device's version string length (param_value is NULL).

    err = GetDeviceInfo(deviceId, CL_DEVICE_VERSION, 0, NULL, &stringLength, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("clGetDeviceInfo() to get CL_DEVICE_VERSION length returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    // Now, that we know the device's version string length, we can allocate enough space before read it
    std::vector<char> deviceVersion(stringLength);

    // Read the device's version string
    // The read value returned in deviceVersion
    err = GetDeviceInfo(deviceId, CL_DEVICE_VERSION, stringLength, &deviceVersion[0], NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetDeviceInfo() to get CL_DEVICE_VERSION returned %s.\n", ClErrTxt(err));
        return err;
    }

    if (strstr(&deviceVersion[0], "OpenCL 2.0") != NULL)
    {
        ocl->deviceVersion = OPENCL_VERSION_2_0;
    }
    printf("deviceVersion = %s\n", &deviceVersion[0]);

    // Read the device's OpenCL C version string length (param_value is NULL).
    err = GetDeviceInfo(deviceId, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &stringLength, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetDeviceInfo() to get CL_DEVICE_OPENCL_C_VERSION length returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    // Now, that we know the device's OpenCL C version string length, we can allocate enough space before read it
    std::vector<char> compilerVersion(stringLength);

    // Read the device's OpenCL C version string
    // The read value returned in compilerVersion
    err = GetDeviceInfo(deviceId, CL_DEVICE_OPENCL_C_VERSION, stringLength, &compilerVersion[0], NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetDeviceInfo() to get CL_DEVICE_OPENCL_C_VERSION returned %s.\n", ClErrTxt(err));
        return err;
    }

    else if (strstr(&compilerVersion[0], "OpenCL C 2.0") != NULL)
    {
        ocl->compilerVersion = OPENCL_VERSION_2_0;
    }

    return err;
}


/*
 * Generate random value for input buffers
 */
void generateInput(cl_int* inputArray, cl_uint arrayWidth, cl_uint arrayHeight)
{
    srand(12345);

    // random initialization of input
    cl_uint array_size = arrayWidth * arrayHeight;
    for (cl_uint i = 0; i < array_size; ++i)
    {
        inputArray[i] = rand();
    }
}


/*
 * This function picks/creates necessary OpenCL objects which are needed.
 * The objects are:
 * OpenCL platform, device, context, and command queue.
 *
 * All these steps are needed to be performed once in a regular OpenCL application.
 * This happens before actual compute kernels calls are performed.
 *
 * For convenience, in this application you store all those basic OpenCL objects in structure ocl_args_d_t,
 * so this function populates fields of this structure, which is passed as parameter ocl.
 * Please, consider reviewing the fields before going further.
 * The structure definition is right in the beginning of this file.
 */
int SetupOpenCL(ocl_args_d_t *ocl, cl_device_type deviceType)
{
    // The following variable stores return codes for all OpenCL calls.
    cl_int err = CL_SUCCESS;

    // Query for all available OpenCL platforms on the system
    // Here you enumerate all platforms and pick one which name has preferredPlatform as a sub-string
    cl_platform_id platformId = FindOpenCLPlatform(ocl, "Intel", deviceType);
    if (NULL == platformId)
    {
        LogError("Error: Failed to find OpenCL platform.\n");
        return CL_INVALID_VALUE;
    }

	cl_device_id device_id[2];
    cl_uint numDevices;
    err = GetDeviceIDs(platformId, deviceType, 1, &device_id[0], &numDevices, ocl->wrap_data);
    if( err != CL_SUCCESS )
    {
        LogError("Couldn't get device Id, clGetDeviceIDs() returned '%s'.\n", ClErrTxt(err));
        return err;
    }
    err = GetDeviceIDs(platformId, CL_DEVICE_TYPE_CPU, 1, &device_id[1], &numDevices, ocl->wrap_data);
    if( err != CL_SUCCESS )
    {
        LogError("Couldn't get device Id, clGetDeviceIDs() returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    err = RetainDevice(device_id[0], ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clRetainDevice returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    cl_uint num_devices=0;
    cl_device_partition_property deviceProperties[] = {CL_DEVICE_PARTITION_EQUALLY, 8, 0};
    err = CreateSubDevices(device_id[0],deviceProperties,num_devices,NULL,NULL, ocl->wrap_data);
    if( err != CL_SUCCESS )
    {
        LogError("Couldn't create sub devices, clCreateSubDevices() returned '%s'.\n", ClErrTxt(err));
        //return err;
    }

    cl_context m_context;
    cl_context_properties contextProperties2[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0};
    m_context = CreateContext(contextProperties2, 2, &device_id[0], NULL, NULL, &err, ocl->wrap_data);
    if( err != CL_SUCCESS )
    {
        LogError("Couldn't create a context, clCreateContext() returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    // make two command queues with two different devices in the same context.
    // this context is different from the context that we use in other places in the app.

    // Read the OpenCL platform's version and the device OpenCL and OpenCL C versions
    err = GetPlatformAndDeviceVersion(platformId, ocl, device_id[0]);
    if (CL_SUCCESS != err)
    {
        LogError("Error: GetPlatformAndDeviceVersion() returned %s.\n", ClErrTxt(err));
        return err;
    }

    cl_command_queue cmmdqueue3, cmmdqueue4;

#if 0

    const cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};

    cmmdqueue3 = CreateCommandQueueWithProperties(m_context, device_id[0], properties, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue()3 returned %s.\n", ClErrTxt(err));
        return err;
    }

    cmmdqueue4 = CreateCommandQueueWithProperties(m_context, device_id[1], properties, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue()4 returned %s.\n", ClErrTxt(err));
        return err;
    }

#else
    // For OpenCl 1.2 Only!!!
    cmmdqueue3 = CreateCommandQueue(m_context, device_id[0], CL_QUEUE_PROFILING_ENABLE, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        printf("device_id[0] = %d\n",device_id[0]);
        LogError("Error: clCreateCommandQueue()3 returned %s.\n", ClErrTxt(err));
        return err;
    }

    // For OpenCl 1.2 Only!!!
    cmmdqueue4 = CreateCommandQueue(m_context, device_id[1], CL_QUEUE_PROFILING_ENABLE, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue()4 returned %s.\n", ClErrTxt(err));
        return err;
    }

#endif

    cl_int stamData[128];
    cl_mem stamMem;
    stamMem = CreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_uint) * 128, stamData, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateBuffer for stamMem returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueReadBuffer(cmmdqueue3, stamMem, true, 0, 8, stamData, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueReadBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueReadBuffer(cmmdqueue4, stamMem, true, 0, 8, stamData, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueReadBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    err |= ReleaseMemObject(stamMem, ocl->wrap_data);
    err |= ReleaseDevice(device_id[0], ocl->wrap_data);
    err |= ReleaseDevice(device_id[1], ocl->wrap_data);
    err |= ReleaseCommandQueue(cmmdqueue3, ocl->wrap_data);
    err |= ReleaseCommandQueue(cmmdqueue4, ocl->wrap_data);
    if( err != CL_SUCCESS )
    {
        LogError("clReleaseDevice returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    err = RetainContext(m_context, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clRetainContext returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    err |= ReleaseContext(m_context, ocl->wrap_data);
    err |= ReleaseContext(m_context, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clReleaseContext returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    // Create context with device of specified type.
    // Required device type is passed as function argument deviceType.
    // So you may use this function to create context for any CPU or GPU OpenCL device.
    // The creation is synchronized (pfn_notify is NULL) and NULL user_data
    cl_context_properties contextProperties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platformId, 0};
    ocl->context = CreateContextFromType(contextProperties, deviceType, NULL, NULL, &err, ocl->wrap_data);
    if ((CL_SUCCESS != err) || (NULL == ocl->context))
    {
        LogError("Couldn't create a context, clCreateContextFromType() returned '%s'.\n", ClErrTxt(err));
        return err;
    }

    // Query for OpenCL device which was used for context creation
    err = GetContextInfo(ocl->context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &ocl->device, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetContextInfo() to get list of devices returned %s.\n", ClErrTxt(err));
        return err;
    }

    // Create command queue.
    // OpenCL kernels are enqueued for execution to a particular device through special objects called command queues.
    // Command queue guarantees some ordering between calls and other OpenCL commands.
    // Here you create a simple in-order OpenCL command queue that doesn't allow execution of two kernels in parallel on a target device.
#if 0

    ocl->commandQueue = CreateCommandQueueWithProperties(ocl->context, ocl->device, properties, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue() returned %s.\n", ClErrTxt(err));
        return err;
    }

    ocl->commandQueue2 = CreateCommandQueueWithProperties(ocl->context, ocl->device, properties, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue()2 returned %s.\n", ClErrTxt(err));
        return err;
    }

#else
    // default behavior: OpenCL 1.2
    cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
    ocl->commandQueue = CreateCommandQueue(ocl->context, ocl->device, properties, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue() returned %s.\n", ClErrTxt(err));
        return err;
    }

    // For OpenCl 1.2 Only!!!
    ocl->commandQueue2 = CreateCommandQueue(ocl->context, ocl->device, properties, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateCommandQueue()2 returned %s.\n", ClErrTxt(err));
        return err;
    }

#endif

    err = Flush(ocl->commandQueue, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clFlush returned %s.\n", ClErrTxt(err));
        return err;
    }

    err = RetainCommandQueue(ocl->commandQueue2, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clRetainCommandQueue returned %s.\n", ClErrTxt(err));
        return err;
    }
    err = ReleaseCommandQueue(ocl->commandQueue2, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clReleaseCommandQueue returned %s.\n", ClErrTxt(err));
        return err;
    }

    size_t initialDeviceId;
    GetCommandQueueInfo(ocl->commandQueue, CL_QUEUE_DEVICE, sizeof(size_t), &initialDeviceId, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetCommandQueueInfo returned %s\n", ClErrTxt(err));
        return err;
    }
    else
    {
        LogInfo("device id which specified when command-queue is created: %d.\n", initialDeviceId);
    }

    // Obtain the size (in bits) of the largest OpenCL built-in data type supported by the device
    // this value is used when allocating memory to avoid alignment issues
    err = GetDeviceInfo(ocl->device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(cl_uint), &ocl->minAlign, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: Failed to get device information (maximum memory base address align size), returned %s\n", ClErrTxt(err));
        return err;
    }
    ocl->minAlign /= 8; //in bytes
    LogInfo("Expected minimum alignment for buffers is %d bytes...\n", ocl->minAlign);

    return CL_SUCCESS;
}

/*
 * Create and build OpenCL program from its source code
 */
int CreateAndBuildProgram(ocl_args_d_t *ocl)
{
    cl_int err = CL_SUCCESS;

    // Upload the OpenCL C source code from the input file to source
    // The size of the C program is returned in sourceSize
    char* source = NULL;
    size_t src_size = 0;
    err = ReadSourceFromFile("addKernel.cl", &source, &src_size);
    if (CL_SUCCESS != err)
    {
        LogError("Error: ReadSourceFromFile returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    // And now after you obtained a regular C string call clCreateProgramWithSource to create OpenCL program object.
    ocl->program = CreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateProgramWithSource returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    cl_program program2 = CreateProgramWithSource(ocl->context, 1, (const char**)&source, &src_size, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateProgramWithSource returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    err = RetainProgram(program2, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateProgramWithSource returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    err = CompileProgram(program2, 1, &ocl->device,"", 0, NULL, NULL, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCompileProgram returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    LinkProgram(ocl->context, 1, &ocl->device,"", 1, &program2, NULL, NULL, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clLinkProgram returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    err |= ReleaseProgram(program2, ocl->wrap_data);
    err |= ReleaseProgram(program2, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clReleaseProgram returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    size_t prog3Size = 1;
    const unsigned char bin = ' ';
    const unsigned char* bin2 = &bin;
    const unsigned char** binaries = &bin2;

    cl_program program3 = CreateProgramWithBinary(ocl->context,1,&ocl->device,&prog3Size,binaries,NULL,NULL, ocl->wrap_data);
    {
        LogError("clCreateProgramWithBinary returned %s.\n", ClErrTxt(err));
        // we want to continue executing the app so we ignore this error code
        err = CL_SUCCESS;
    }

    // these releases should return CL_INVALID_PROGRMA
    ReleaseProgram(program3, ocl->wrap_data);
    ReleaseProgram(program3, ocl->wrap_data);

    cl_program program4 = CreateProgramWithBuiltInKernels(ocl->context,1,&ocl->device, "block_motion_estimate_intel", &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateProgramWithBuiltInKernels returned %s.\n", ClErrTxt(err));
        goto Finish;
    }

    ReleaseProgram(program4, ocl->wrap_data);

    cl_int numDevices;
    err = GetProgramInfo(ocl->program,CL_PROGRAM_NUM_DEVICES,sizeof(cl_int),&numDevices,NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetProgramInfo returned %s.\n", ClErrTxt(err));
        goto Finish;
    }
    LogInfo("Number of devices associated with program: %u\n", numDevices);

    // Build the program
    // During creation a program is not built. You need to explicitly call build function.
    // Here you just use create-build sequence,
    // but there are also other possibilities when program consist of several parts,
    // some of which are libraries, and you may want to consider using clCompileProgram and clLinkProgram as
    // alternatives.
    err = BuildProgram(ocl->program, 1, &ocl->device, "", NULL, NULL, ocl->wrap_data );
    if (CL_SUCCESS != err)
    {
        LogError("Error: clBuildProgram() for source program returned %s.\n", ClErrTxt(err));

        // In case of error print the build log to the standard output
        // First check the size of the log
        // Then allocate the memory and obtain the log from the program
        if (err == CL_BUILD_PROGRAM_FAILURE)
        {
            size_t log_size = 0;
            GetProgramBuildInfo(ocl->program, ocl->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size, ocl->wrap_data);

            std::vector<char> build_log(log_size);
            GetProgramBuildInfo(ocl->program, ocl->device, CL_PROGRAM_BUILD_LOG, log_size, &build_log[0], NULL, ocl->wrap_data);

            LogError("Error happened during the build of OpenCL program.\nBuild log:%s", &build_log[0]);
        }
    }


Finish:
    if (source)
    {
        delete[] source;
        source = NULL;
    }

    return err;
}


void CL_CALLBACK memObjectDestructorHandler(cl_mem memobj, void* user_data)
{
    LogInfo("Memory object freed successfully!.\n");
}

/*
 * Create OpenCL buffers from host memory
 * These buffers will be used later by the OpenCL kernel
 */
int CreateBufferArguments(ocl_args_d_t *ocl, cl_int* inputA, cl_int* inputB, cl_int* outputC, cl_uint arrayWidth, cl_uint arrayHeight)
{
    cl_int err = CL_SUCCESS;

    // Create new OpenCL buffer objects
    // As these buffer are used only for read by the kernel, you are recommended to create it with flag CL_MEM_READ_ONLY.
    // Always set minimal read/write flags for buffers, it may lead to better performance because it allows runtime
    // to better organize data copying.
    // You use CL_MEM_COPY_HOST_PTR here, because the buffers should be populated with bytes at inputA and inputB.

    ocl->srcA = CreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_uint) * arrayWidth * arrayHeight, inputA, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateBuffer for srcA returned %s\n", ClErrTxt(err));
        return err;
    }

    ocl->srcB = CreateBuffer(ocl->context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_uint) * arrayWidth * arrayHeight, inputB, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateBuffer for srcB returned %s\n", ClErrTxt(err));
        return err;
    }

    // If the output buffer is created directly on top of output buffer using CL_MEM_USE_HOST_PTR,
    // then, depending on the OpenCL runtime implementation and hardware capabilities,
    // it may save you not necessary data copying.
    // As it is known that output buffer will be write only, you explicitly declare it using CL_MEM_WRITE_ONLY.
    ocl->dstMem = CreateBuffer(ocl->context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_uint) * arrayWidth * arrayHeight, outputC, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateBuffer for dstMem returned %s\n", ClErrTxt(err));
        return err;
    }


    return CL_SUCCESS;
}


/*
 * Set kernel arguments
 */
cl_uint SetKernelArguments(ocl_args_d_t *ocl)
{
    cl_int err = CL_SUCCESS;

    err  =  SetKernelArg(ocl->kernel, 0, sizeof(cl_mem), (void *)&ocl->srcA, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("error: Failed to set argument srcA, returned %s\n", ClErrTxt(err));
        return err;
    }

    err  = SetKernelArg(ocl->kernel, 1, sizeof(cl_mem), (void *)&ocl->srcB, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: Failed to set argument srcB, returned %s\n", ClErrTxt(err));
        return err;
    }

    err  = SetKernelArg(ocl->kernel, 2, sizeof(cl_mem), (void *)&ocl->dstMem, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: Failed to set argument dstMem, returned %s\n", ClErrTxt(err));
        return err;
    }

    return err;
}


/*
 * Execute the kernel
 */
cl_uint ExecuteAddKernel(ocl_args_d_t *ocl, cl_uint width, cl_uint height)
{
    cl_int err = CL_SUCCESS;

    // Define global iteration space for clEnqueueNDRangeKernel.
    size_t globalWorkSize[2] = {width, height};

    // execute kernel
    err = EnqueueNDRangeKernel(ocl->commandQueue, ocl->kernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: Failed to run kernel, return %s\n", ClErrTxt(err));
        return err;
    }

    // Wait until the queued kernel is completed by the device
    err = Finish(ocl->commandQueue, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clFinish return %s\n", ClErrTxt(err));
        return err;
    }

    return CL_SUCCESS;
}


/*
 * "Read" the result buffer (mapping the buffer to the host memory address)
 */
bool ReadAndVerify(ocl_args_d_t *ocl, cl_uint width, cl_uint height, cl_int *inputA, cl_int *inputB)
{
    cl_int err = CL_SUCCESS;
    bool result = true;

    // Enqueue a command to map the buffer object (ocl->dstMem) into the host address space and returns a pointer to it
    // The map operation is blocking
    cl_int *resultPtr = (cl_int *)EnqueueMapBuffer(ocl->commandQueue, ocl->dstMem, true, CL_MAP_READ, 0, sizeof(cl_uint) * width * height, 0, NULL, NULL, &err, ocl->wrap_data);

    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueMapBuffer returned %s\n", ClErrTxt(err));
        return false;
    }

    // Call clFinish to guarantee that output region is updated
    err = Finish(ocl->commandQueue, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clFinish returned %s\n", ClErrTxt(err));
    }

    // We mapped dstMem to resultPtr, so resultPtr is ready and includes the kernel output !!!
    // Verify the results
    unsigned int size = width * height;
    for (unsigned int k = 0; k < size; ++k)
    {
        if (resultPtr[k] != inputA[k] + inputB[k])
        {
            LogError("Verification failed at %d: (%d + %d = %d)\n", k, inputA[k], inputB[k], resultPtr[k]);
            result = false;
        }
    }

     // Unmapped the output buffer before releasing it
    err = EnqueueUnmapMemObject(ocl->commandQueue, ocl->dstMem, resultPtr, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueUnmapMemObject returned %s\n", ClErrTxt(err));
    }

    return result;
}

void CL_CALLBACK myCallback(cl_event event, cl_int event_command_exec_status, void* user_data)
{
    LogInfo("MyCallback event notify function called.\n");
}

int DoBufferAndImageOperations(ocl_args_d_t *ocl, cl_int* inputA, cl_int* inputB)
{
    cl_int err;

    cl_buffer_region buff_reg;
    buff_reg.origin = 8;
    buff_reg.size = 64;

    //----------------------------- Buffer operations --------------------------
    cl_int hostPtrA[128];
    cl_int hostPtrB[128];
    cl_mem helpBufferA = NULL;
    cl_mem helpBufferB = NULL;
    cl_mem subBuffer1 = NULL;
    cl_mem subBuffer2 = NULL;


    helpBufferA = CreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 128, hostPtrA, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    helpBufferB = CreateBuffer(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int) * 128, hostPtrB, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    subBuffer1 = CreateSubBuffer(helpBufferA, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &buff_reg, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateSubBuffer returned %s\n", ClErrTxt(err));
        return err;
    }
    subBuffer2 = CreateSubBuffer(helpBufferB, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &buff_reg, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateSubBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueCopyBuffer(ocl->commandQueue,  helpBufferA, helpBufferB, 0, 0, 8, 0 ,NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueCopyBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    size_t zeroOrigin[3] = {0, 0, 0};
    size_t region[3] = {8, 1, 1};
    err = EnqueueCopyBufferRect(ocl->commandQueue, subBuffer1, helpBufferB, zeroOrigin, zeroOrigin, region, 256, 256, 256, 256, 0, NULL,NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueCopyBufferRect returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueReadBuffer(ocl->commandQueue, helpBufferA, true, 0, 8, hostPtrA, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueReadBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueReadBufferRect(ocl->commandQueue, helpBufferA, true, zeroOrigin, zeroOrigin, region, 256, 256, 256, 256, hostPtrA, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueReadBufferRect returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueWriteBuffer(ocl->commandQueue, helpBufferA, true, 0, 8, hostPtrA, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueWriteBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueWriteBufferRect(ocl->commandQueue, helpBufferA, true, zeroOrigin, zeroOrigin, region, 256, 256, 256, 256, hostPtrA, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueWriteBufferRect returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueFillBuffer(ocl->commandQueue, helpBufferA, hostPtrA, sizeof(cl_int), 0, 8, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueFillBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    err = SetMemObjectDestructorCallback(helpBufferB, &memObjectDestructorHandler, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clSetMemObjectDestructorCallback returned %s\n", ClErrTxt(err));
        return err;
    }

    err = RetainMemObject(helpBufferB, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clRetainMemObject returned %s\n", ClErrTxt(err));
        return err;
    }

    size_t objMemSize;
    err = GetMemObjectInfo(helpBufferA, CL_MEM_SIZE, sizeof(size_t), &objMemSize, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetMemObjectInfo returned %s\n", ClErrTxt(err));
        return err;
    }
    else
    {
        LogInfo("subBuffer1 memory object size is: %d bytes.\n", objMemSize);
    }

    err = EnqueueBarrierWithWaitList(ocl->commandQueue, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueBarrierWithWaitList returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueMarkerWithWaitList (ocl->commandQueue, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueMarkerWithWaitList  returned %s\n", ClErrTxt(err));
        return err;
    }

    //----------------------------- Image operations --------------------------

    cl_mem img1 = NULL;
    cl_mem img2 = NULL;
    cl_image_format imgFormat;
    imgFormat.image_channel_order = CL_R;
    imgFormat.image_channel_data_type = CL_FLOAT;
    cl_image_desc imgDesc1;
    imgDesc1.image_type = CL_MEM_OBJECT_IMAGE2D;
    imgDesc1.image_width = 8;
    imgDesc1.image_height = 1;
    imgDesc1.image_row_pitch = 0;
    imgDesc1.num_mip_levels = 0;
    imgDesc1.num_samples = 0;
    imgDesc1.buffer = NULL;

    img1 = CreateImage(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR , &imgFormat, &imgDesc1, hostPtrA, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateImage returned %s\n", ClErrTxt(err));
        return err;
    }

    img2 = CreateImage(ocl->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR , &imgFormat, &imgDesc1, hostPtrB, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateImage returned %s\n", ClErrTxt(err));
        return err;
    }

    size_t elementSize;
    err = GetImageInfo(img2, CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &elementSize, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetImageInfo returned %s\n", ClErrTxt(err));
        return err;
    }
    else
    {
        LogInfo("img2 Element size is: %d bytes.\n", elementSize);
    }

    err = EnqueueCopyImage(ocl->commandQueue2,img1, img2, zeroOrigin, zeroOrigin, region, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueCopyImage returned %s\n", ClErrTxt(err));
        return err;
    }

    cl_int hostImgPtr[128];
    err = EnqueueReadImage(ocl->commandQueue2, img1, true, zeroOrigin, region, 0, 0, hostImgPtr, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueReadImage returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueWriteImage(ocl->commandQueue2, img1, true, zeroOrigin, region, 0, 0, hostImgPtr, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueWriteImage returned %s\n", ClErrTxt(err));
        return err;
    }

    cl_float fill_color = 1;
    err = EnqueueFillImage(ocl->commandQueue2,img1, &fill_color, zeroOrigin, region,0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueFillImage returned %s\n", ClErrTxt(err));
        return err;
    }

    size_t row_pitch = 0;
    size_t slice_pitch = 0;

    cl_int *resultPtr = (cl_int *)EnqueueMapImage(ocl->commandQueue2, img2, true, CL_MEM_READ_WRITE, zeroOrigin, region, &row_pitch, &slice_pitch, 0, NULL, NULL, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueMapImage returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueUnmapMemObject(ocl->commandQueue2, img2, resultPtr, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueUnmapMemObject returned %s\n", ClErrTxt(err));
    }

    cl_uint num_images_formats;
    err = GetSupportedImageFormats(ocl->context, CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, 100, NULL, &num_images_formats, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetSupportedImageFormats returned %s\n", ClErrTxt(err));
        return err;
    }
    else
    {
        LogInfo("num of images formats supported: %u\n",num_images_formats);
    }

    cl_mem buffImg[2] = {img1, helpBufferA};
    err = EnqueueMigrateMemObjects(ocl->commandQueue2, 2, buffImg, CL_MIGRATE_MEM_OBJECT_HOST, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueMigrateMemObjects returned %s\n", ClErrTxt(err));
        return err;
    }

    Finish(ocl->commandQueue, ocl->wrap_data);
    Finish(ocl->commandQueue2, ocl->wrap_data);

    //----------------------------- Buffer&Image copy operations--------------------------

    err = EnqueueCopyBufferToImage(ocl->commandQueue, helpBufferA, img1, 0, zeroOrigin, region, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueCopyBufferToImage returned %s\n", ClErrTxt(err));
        return err;
    }

    err = EnqueueCopyImageToBuffer(ocl->commandQueue, img2, subBuffer2, zeroOrigin, region, 0, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueCopyImageToBuffer returned %s\n", ClErrTxt(err));
        return err;
    }

    //----------------------------- Release Memory Objects --------------------------
    err |= ReleaseMemObject(subBuffer1, ocl->wrap_data);
    err |= ReleaseMemObject(subBuffer2, ocl->wrap_data);
    err |= ReleaseMemObject(helpBufferA, ocl->wrap_data);
    err |= ReleaseMemObject(helpBufferB, ocl->wrap_data);
    err |= ReleaseMemObject(helpBufferB, ocl->wrap_data);
    err |= ReleaseMemObject(img1, ocl->wrap_data);
    err |= ReleaseMemObject(img2, ocl->wrap_data);

    if (CL_SUCCESS != err)
    {
        LogError("Error: clReleaseMemObject returned '%s'.\n", ClErrTxt(err));
    }

    return CL_SUCCESS;
}



int DoEventOperations(ocl_args_d_t *ocl)
{
    int err = CL_SUCCESS;
    cl_event myEvent = CreateUserEvent(ocl->context, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateUserEvent returned %s\n", ClErrTxt(err));
    }

    cl_int hostPtrA[128];
    EnqueueReadBuffer(ocl->commandQueue,ocl->srcA, true, 0, 8, hostPtrA, 0, NULL, &myEvent, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueReadBuffer returned %s\n", ClErrTxt(err));
    }

    cl_command_type commandType ;
    err = GetEventInfo(myEvent, CL_EVENT_COMMAND_TYPE, sizeof(cl_command_type), &commandType, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetEventInfo returned %s\n", ClErrTxt(err));
    }
    else
    {
        LogInfo("Command Type of myEvent is: %u\n", commandType);
    }

    cl_ulong startTime=0, endTime=0;
    err |= GetEventProfilingInfo(myEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL, ocl->wrap_data);
    err |= GetEventProfilingInfo(myEvent, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetEventProfilingInfo returned %s\n", ClErrTxt(err));
    }
    else
    {
        LogInfo("Time Execution of myEvent Command: %lu\n", endTime-startTime);
    }

    cl_event myEvent2 = CreateUserEvent(ocl->context, &err, ocl->wrap_data);
    err = SetUserEventStatus(myEvent2, CL_COMPLETE, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clSetUserEventStatus returned %s\n", ClErrTxt(err));
    }

    err = WaitForEvents(1,&myEvent, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clWaitForEvents returned %s\n", ClErrTxt(err));
    }



    err = SetEventCallback (myEvent2, CL_COMPLETE, &myCallback, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clSetEventCallback returned %s\n", ClErrTxt(err));
    }
    err = RetainEvent(myEvent2, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clRetainEvent returned %s\n", ClErrTxt(err));
    }

    err |= ReleaseEvent(myEvent, ocl->wrap_data);
    err |= ReleaseEvent(myEvent2, ocl->wrap_data);
    err |= ReleaseEvent(myEvent2, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clReleaseEvent returned %s\n", ClErrTxt(err));
    }

    return CL_SUCCESS;
}

int DoSamplerOperations(ocl_args_d_t *ocl)
{
    cl_int err = CL_SUCCESS;
    cl_sampler mySampler;

#if 0
    mySampler = CreateSamplerWithProperties(ocl->context, NULL, &err);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateSamplerWithProperties returned %s\n", ClErrTxt(err));
    }
#else
    // For OpenCl 1.2 Only!!!
    mySampler = CreateSampler(ocl->context, CL_FALSE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR, &err, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateSampler returned %s\n", ClErrTxt(err));
    }

#endif

    cl_uint refCount;
    err = GetSamplerInfo(mySampler, CL_SAMPLER_REFERENCE_COUNT, sizeof(cl_uint), &refCount, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetSamplerInfo returned %s\n", ClErrTxt(err));
    }
    else
    {
        LogInfo("Sampler refurence count: %u\n", refCount);
    }

    err = RetainSampler(mySampler, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clRetainSampler returned %s\n", ClErrTxt(err));
    }

    err |= ReleaseSampler(mySampler, ocl->wrap_data);
    err |= ReleaseSampler(mySampler, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clReleaseSampler returned %s\n", ClErrTxt(err));
    }

    return CL_SUCCESS;
}


void CL_CALLBACK user_func(void* user_data)
{
    printf("user_func is called using clEnqueueNativeKernel with param %d.\n", user_data);
}

cl_int DoKernelOperations(ocl_args_d_t* ocl)
{
    cl_int err;

    cl_kernel newKernel;
    err = CreateKernelsInProgram(ocl->program,1,&newKernel,NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateKernelsInProgram returned %s\n", ClErrTxt(err));
        return -1;
    }
    err = RetainKernel(newKernel, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clRetainKernel returned '%s'.\n", ClErrTxt(err));
        return err;
    }
    err |= ReleaseKernel(newKernel, ocl->wrap_data);
    err |= ReleaseKernel(newKernel, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clReleaseKernel returned '%s'.\n", ClErrTxt(err));
    }

    size_t argsNum;
    err = GetKernelInfo(ocl->kernel,CL_KERNEL_NUM_ARGS,sizeof(size_t),&argsNum,NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetKernelInfo returned %s\n", ClErrTxt(err));
        return -1;
    }
    LogInfo("Number of kernel arguments: %u\n", argsNum);

    char argType[32] = {'\0'};
    err = GetKernelArgInfo(ocl->kernel,1,CL_KERNEL_ARG_NAME,sizeof(argType),&argType,NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetKernelArgInfo (1) returned %s\n", ClErrTxt(err));
    }
    else
    {
        LogInfo("First Argument Type: %s\n", argType);
    }

    err = GetKernelArgInfo(ocl->kernel,20,CL_KERNEL_ARG_NAME,sizeof(argType),&argType,NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetKernelArgInfo (20) returned %s\n", ClErrTxt(err));
    }
    else
    {
        LogInfo("realy?! you have 20 arguments for this kernel!?");
        return -1;
    }

    cl_ulong localMemUsed;
    err = GetKernelWorkGroupInfo (ocl->kernel,ocl->device,CL_KERNEL_LOCAL_MEM_SIZE,sizeof(cl_ulong),&localMemUsed,NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clGetKernelWorkGroupInfo  returned %s\n", ClErrTxt(err));
        return -1;
    }
    LogInfo("Local memory used by this kernel: %u\n", localMemUsed);

    // expect to return CL_INVALID_COMMAND_QUEUE because the device capabilities (read in spec.)
    cl_int param;
    err = EnqueueNativeKernel(ocl->commandQueue2, &user_func, &param,sizeof(cl_int), 0, NULL, NULL, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueNativeKernel returned %s\n", ClErrTxt(err));
    }

#if 0

#else
    // For OpenCl 1.2 Only!!!
    err = EnqueueTask(ocl->commandQueue2, ocl->kernel, 0, NULL, NULL, ocl->wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clEnqueueTask returned %s\n", ClErrTxt(err));
    }

#endif

    return CL_SUCCESS;
}


void printApiCallsData(ocl_args_d_t* ocl)
{
    map<string, list<int>, mapComparer >::iterator it;
    for(it = ocl->wrap_data->apiCallsMap->begin(); it != ocl->wrap_data->apiCallsMap->end(); it++)
    {
        // remove CL_SUCCESS error codes
        list<int> actualErrList(it->second);
        actualErrList.remove(0);

        ocl->wrap_data->apiCallsOut << it->first << ", " << it->second.size() << ", " << actualErrList.size();

        // print all error codes which is not CL_SUCCESS.

        actualErrList.sort();
        list<int>::iterator errIt;
        for(errIt = actualErrList.begin(); errIt != actualErrList.end(); errIt++)
        {
            ocl->wrap_data->apiCallsOut << ", " << *errIt;
        }

        //ocl->wrap_data->apiCallsOut << ";";
        ocl->wrap_data->apiCallsOut << endl;
    }

}

void printKernelLaunchData(ocl_args_d_t* ocl)
{
    list<string>::iterator it;
    list<string>* myList = ocl->wrap_data->kernelLaunchList;

    for(it = myList->begin(); it != myList->end(); it++)
    {
        ocl->wrap_data->kernelLaunchOut << *it;
    }
}

void printMemCommandsData(ocl_args_d_t* ocl)
{
    map<int, list<string>, mapComparer >::iterator it;
    for(it = ocl->wrap_data->memCommandsMap->begin(); it != ocl->wrap_data->memCommandsMap->end(); it++)
    {
        list<string> linesPerQueue(it->second);

        // print all command lines per this queue
        list<string>::iterator it2;
        for(it2 = linesPerQueue.begin(); it2 != linesPerQueue.end(); it2++)
        {
            ocl->wrap_data->memCommandsOut << *it2;
        }
    }

}
cl_int printDataToFile(ocl_args_d_t* ocl)
{
    cl_int err= CL_SUCCESS;

    printApiCallsData(ocl);
    printKernelLaunchData(ocl);
    printMemCommandsData(ocl);

    return err;
}


/*
 * main execution routine
 * Basically it consists of three parts:
 *   - generating the inputs
 *   - running OpenCL kernel
 *   - reading results of processing
 */
int _tmain(int argc, TCHAR* argv[])
{
    cl_int err;
    ocl_args_d_t ocl;
    cl_device_type deviceType = CL_DEVICE_TYPE_GPU;

    LARGE_INTEGER perfFrequency;
    LARGE_INTEGER performanceCountNDRangeStart;
    LARGE_INTEGER performanceCountNDRangeStop;

    cl_uint arrayWidth  = 1024;
    cl_uint arrayHeight = 1024;


    //initialize Open CL objects (context, queue, etc.)
    if (CL_SUCCESS != SetupOpenCL(&ocl, deviceType))
    {
        return -1;
    }
    // allocate working buffers
    cl_int* inputA  = (cl_int*)_aligned_malloc(sizeof(cl_int) * arrayWidth * arrayHeight, ocl.minAlign);
    cl_int* inputB  = (cl_int*)_aligned_malloc(sizeof(cl_int) * arrayWidth * arrayHeight, ocl.minAlign);
    cl_int* outputC = (cl_int*)_aligned_malloc(sizeof(cl_int) * arrayWidth * arrayHeight, ocl.minAlign);
    if (NULL == inputA || NULL == inputB || NULL == outputC)
    {
        LogError("Error: _aligned_malloc failed to allocate buffers.\n");
        return -1;
    }

    //random input
    generateInput(inputA, arrayWidth, arrayHeight);
    generateInput(inputB, arrayWidth, arrayHeight);

    // Create OpenCL buffers from host memory
    // These buffers will be used later by the OpenCL kernel
    if (CL_SUCCESS != CreateBufferArguments(&ocl, inputA, inputB, outputC, arrayWidth, arrayHeight))
    {
        return -1;
    }

    // BUFFER & IMAGE
    if (CL_SUCCESS != DoBufferAndImageOperations(&ocl, inputA, inputB))
    {
        return -1;
    }

    // Create and build the OpenCL program
    if (CL_SUCCESS != CreateAndBuildProgram(&ocl))
    {
        return -1;
    }

    // EVENT , SAMPLER

    if (CL_SUCCESS != DoEventOperations(&ocl) )
    {
        return -1;
    }

    // Sampler
    if (CL_SUCCESS != DoSamplerOperations(&ocl) )
    {
        return -1;
    }

	// Program consists of kernels.
    // Each kernel can be called (enqueued) from the host part of OpenCL application.
    // To call the kernel, you need to create it from existing program.
    ocl.kernel = CreateKernel(ocl.program, "Add", &err, ocl.wrap_data);
    if (CL_SUCCESS != err)
    {
        LogError("Error: clCreateKernel returned %s\n", ClErrTxt(err));
        return -1;
    }


    // Passing arguments into OpenCL kernel.
    if (CL_SUCCESS != SetKernelArguments(&ocl))
    {
        return -1;
    }

    // Kernel
    if (CL_SUCCESS != DoKernelOperations(&ocl) )
    {
        return -1;
    }

    // Regularly you wish to use OpenCL in your application to achieve greater performance results
    // that are hard to achieve in other ways.
    // To understand those performance benefits you may want to measure time your application spent in OpenCL kernel execution.
    // The recommended way to obtain this time is to measure interval between two moments:
    //   - just before clEnqueueNDRangeKernel is called, and
    //   - just after clFinish is called
    // clFinish is necessary to measure entire time spending in the kernel, measuring just clEnqueueNDRangeKernel is not enough,
    // because this call doesn't guarantees that kernel is finished.
    // clEnqueueNDRangeKernel is just enqueue new command in OpenCL command queue and doesn't wait until it ends.
    // clFinish waits until all commands in command queue are finished, that suits your need to measure time.
    bool queueProfilingEnable = true;
    if (queueProfilingEnable)
        QueryPerformanceCounter(&performanceCountNDRangeStart);
    // Execute (enqueue) the kernel
    if (CL_SUCCESS != ExecuteAddKernel(&ocl, arrayWidth, arrayHeight))
    {
        return -1;
    }
    if (queueProfilingEnable)
        QueryPerformanceCounter(&performanceCountNDRangeStop);

    // The last part of this function: getting processed results back.
    // use map-unmap sequence to update original memory area with output buffer.
    ReadAndVerify(&ocl, arrayWidth, arrayHeight, inputA, inputB);

    // retrieve performance counter frequency
    if (queueProfilingEnable)
    {
        QueryPerformanceFrequency(&perfFrequency);
        LogInfo("NDRange performance counter time %f ms.\n",
            1000.0f*(float)(performanceCountNDRangeStop.QuadPart - performanceCountNDRangeStart.QuadPart) / (float)perfFrequency.QuadPart);
    }

    destroyOclArgsObject(&ocl);

    if( CL_SUCCESS != printDataToFile(&ocl))
    {
        LogInfo("Error: can't print the data to file");
    }

    _aligned_free(inputA);
    _aligned_free(inputB);
    _aligned_free(outputC);

    return 0;
}

