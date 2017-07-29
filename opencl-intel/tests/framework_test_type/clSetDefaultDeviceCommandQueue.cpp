#include "CL21.h"

static void BuildProgramAndExecWithQueue(cl_command_queue target_queue, cl_device_id m_device, cl_context m_context, cl_command_queue m_queue)
{
    const char* kernel = "\
        __kernel void check_default_queue( queue_t queue_from_host, global int* success)\
        {\
            *success = 0;\
            queue_t default_queue = get_default_queue();\
            uint* p_uint_default_queue = (uint*)&default_queue;\
            uint* p_uint_queue_from_host = (uint*)&queue_from_host;\
            if( *p_uint_default_queue == *p_uint_queue_from_host )\
            {\
                enqueue_kernel(default_queue, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange_1D(1), ^{ *success = 1; });\
            }\
        }\
        ";
    const size_t kernel_size = strlen(kernel);

    cl_int iRet = CL_SUCCESS;

    iRet = clSetDefaultDeviceCommandQueue(m_context, m_device, target_queue);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clSetDefaultDeviceCommandQueue failed. ";

    cl_program program = clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

    iRet = clBuildProgram(program, 0, NULL, "-cl-std=CL2.0", NULL, NULL);
    if( CL_SUCCESS != iRet )
    {
        std::string log("", 1000);
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, log.size(), &log[0], NULL);
        std::cout << log << std::endl;
    }
    ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed. ";

    cl_kernel kern = clCreateKernel(program, "check_default_queue", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed. ";

    iRet = clSetKernelArg(kern, 0, sizeof(target_queue), &target_queue);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clSetKernelArg failed. ";

    size_t arg_size = 4;
    cl_mem arg = clCreateBuffer(m_context, 0, arg_size, NULL, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed. ";

    int pattern = 2;
    iRet = clEnqueueFillBuffer(m_queue, arg, &pattern, sizeof(pattern), 0, arg_size, 0, NULL, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueFillBuffer failed. ";

    iRet = clSetKernelArg(kern, 1, sizeof(arg), &arg);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clSetKernelArg failed. ";

    iRet = clFinish(m_queue);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish failed. ";

    size_t gws = 1;
    iRet = clEnqueueNDRangeKernel(m_queue, kern, 1, NULL, &gws, NULL, 0, NULL, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueNDRangeKernel failed. ";

    iRet = clFinish(m_queue);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clFinish failed. ";

    int* success = (int*)clEnqueueMapBuffer(m_queue, arg, CL_TRUE, CL_MAP_READ, 0, arg_size, 0, NULL, NULL, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueMapBuffer failed. ";

    ASSERT_EQ(1, *success) <<
        "Queue setted from host and queue returned by get_default_queue() are different";
}

void CL21::SetDefaultDeviceCommandQueueOOO() const
{
    cl_int iRet = CL_SUCCESS;

    cl_queue_properties prop_for_default[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    clCreateCommandQueueWithProperties(m_context, m_device, prop_for_default, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create default device queue) failed. ";

    cl_queue_properties prop_for_not_default[] = {CL_QUEUE_PROPERTIES,
        CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    cl_command_queue queue_created_not_as_default = clCreateCommandQueueWithProperties(m_context, m_device, prop_for_not_default, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create simple OOO queue) failed. ";

    BuildProgramAndExecWithQueue(queue_created_not_as_default, m_device, m_context, m_queue);
}

void CL21::SetDefaultDeviceCommandQueueOOO_Profiling() const
{

    cl_int iRet = CL_SUCCESS;

    cl_queue_properties prop_for_default[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    clCreateCommandQueueWithProperties(m_context, m_device, prop_for_default, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create default device queue) failed. ";

    cl_queue_properties prop_for_not_default[] = {CL_QUEUE_PROPERTIES,
        CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, 0};
    cl_command_queue queue_created_not_as_default = clCreateCommandQueueWithProperties(m_context, m_device, prop_for_not_default, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create profiling queue) failed. ";

    BuildProgramAndExecWithQueue(queue_created_not_as_default, m_device, m_context, m_queue);

}

void CL21::SetDefaultDeviceCommandQueueOOO_SubDevice() const
{

    cl_int iRet = CL_SUCCESS;

    cl_device_partition_property prop_sub_device[] = {CL_DEVICE_PARTITION_EQUALLY, 1 ,0};
    unsigned int num_sub_devices = 0;
    iRet = clCreateSubDevices(m_device, prop_sub_device, 0, NULL, &num_sub_devices);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateSubDevice failed. ";

    std::vector<cl_device_id> sub_devices(num_sub_devices, 0);
    iRet = clCreateSubDevices(m_device, prop_sub_device, sub_devices.size(), &sub_devices[0], NULL);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateSubDevice failed. ";

    const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)m_platform, 0 };
    cl_context context_ = clCreateContext(prop, 1, &sub_devices[0], NULL, NULL, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed. ";

    cl_queue_properties prop_for_default[] = {CL_QUEUE_PROPERTIES,
        CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    clCreateCommandQueueWithProperties(context_, sub_devices[0], prop_for_default, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create default device queue) failed. ";

    cl_queue_properties prop_for_not_default[] = {CL_QUEUE_PROPERTIES,
        CL_QUEUE_ON_DEVICE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, 0};
    cl_command_queue queue_created_not_as_default = clCreateCommandQueueWithProperties(context_, sub_devices[0], prop_for_not_default, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create simple OOO queue) failed. ";

    cl_queue_properties host_queue_prop[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    cl_command_queue host_queue = clCreateCommandQueueWithProperties(context_, sub_devices[0], host_queue_prop, &iRet);

    BuildProgramAndExecWithQueue(queue_created_not_as_default, sub_devices[0], context_, host_queue);

}

void CL21::SetDefaultDeviceCommandQueue_Get_Default_Queue_Query() const
{
    cl_int iRet = CL_SUCCESS;

    cl_command_queue default_queue = 0;
    size_t data_being_queried = 0;

    cl_queue_properties prop_for_not_default[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE , 0};
    cl_command_queue queue_created_not_as_default = clCreateCommandQueueWithProperties(m_context, m_device, prop_for_not_default, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties(create default device queue) failed. ";

    iRet = clSetDefaultDeviceCommandQueue(m_context, m_device, queue_created_not_as_default);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clSetDefaultDeviceCommandQueue failed. ";

    // Try to confuse runtime
    iRet = clRetainCommandQueue(queue_created_not_as_default);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clRetainCommandQueue failed. ";

    iRet = clReleaseCommandQueue(queue_created_not_as_default);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clRetainCommandQueue failed. ";

    iRet = clGetCommandQueueInfo(m_queue, CL_QUEUE_DEVICE_DEFAULT, sizeof(cl_command_queue), &default_queue, &data_being_queried);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetCommandQueueInfo during quering CL_QUEUE_DEVICE_DEFAULT failed. ";

    ASSERT_EQ(queue_created_not_as_default, default_queue) << " Queue setted as default and returned by CL_QUEUE_DEVICE_DEFAULT query don't match. ";
}

void CL21::SetDefaultDeviceCommandQueue_Negative() const
{
    cl_int iRet = CL_SUCCESS;

    iRet = clSetDefaultDeviceCommandQueue((cl_context)m_device, m_device, m_queue);
    ASSERT_EQ(CL_INVALID_CONTEXT, iRet) << " clSetDefaultDeviceCommandQueue with invalid context failed. ";

    iRet = clSetDefaultDeviceCommandQueue(m_context, (cl_device_id)m_context, m_queue);
    ASSERT_EQ(CL_INVALID_DEVICE, iRet) << " clSetDefaultDeviceCommandQueue with invalid device failed. ";

    iRet = clSetDefaultDeviceCommandQueue(m_context, m_device, (cl_command_queue)m_context);
    ASSERT_EQ(CL_INVALID_COMMAND_QUEUE, iRet) << " clSetDefaultDeviceCommandQueue with invalid device failed. ";
}
