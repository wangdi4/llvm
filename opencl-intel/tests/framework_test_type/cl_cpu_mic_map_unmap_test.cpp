#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>
#include "FrameworkTest.h"

// SilentCheck/Check
#define CHECK SilentCheck

#define BUFFER_SIZE 128

#define TITLE( prefix, dev_name ) get_title( title, prefix, sizeof(prefix), dev_name, (4*sizeof(wchar_t)) )
    

inline wchar_t* get_title( wchar_t* buffer, 
                           const wchar_t* prefix, unsigned int psize, 
                           const wchar_t* suffix, unsigned int ssize  )
{
    memcpy( buffer, prefix, psize );    
    memcpy( buffer + (psize-1)/sizeof( wchar_t ), suffix, ssize );
    return buffer;
}

//
// 1. Create buffer on multi-device context
// 2. Map data for WRITE on Device 1 -> Device 2 -> Device 1 -> Device 2
// 3. Write pattern.
// 3. Unmap data on         Device 1 -> Device 2 -> Device 1 -> Device 2
// 4. Map data for READ  on Device 1 
// 5. Compare
//
bool move_data( cl_context context, const wchar_t* queue1_name, cl_command_queue queue1, 
                                    const wchar_t* queue2_name, cl_command_queue queue2 )
{
    wchar_t title[1024];
    
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;

    cl_command_queue dev_queue[] = { queue1,      queue2,      queue1,      queue2 };  
    const wchar_t*   dev_names[] = { queue1_name, queue2_name, queue1_name, queue2_name };    
    void*            map_addrs[] = { 0,           0,           0,           0 };

    unsigned int     num_queues = sizeof(dev_queue)/sizeof(dev_queue[0]);

    size_t stBuffSize = BUFFER_SIZE*sizeof(cl_uint);
    cl_mem clBuff;
    cl_uint*  map_addr;

    clBuff = clCreateBuffer(context, CL_MEM_READ_WRITE, stBuffSize, NULL, &iRet);
    bResult &= CHECK(L"clCreateBuffer", CL_SUCCESS, iRet);
    if (!bResult)
    {
        return false;
    }

    for (unsigned int curr_dev = 0; curr_dev < num_queues; ++curr_dev )
	{
        const wchar_t*   dev_name = dev_names[curr_dev];
        cl_command_queue queue    = dev_queue[curr_dev];

        map_addrs[curr_dev] = clEnqueueMapBuffer( queue, clBuff, 
                                                  CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 
//                                                  curr_dev*sizeof(cl_uint), stBuffSize - curr_dev*sizeof(cl_uint), 0, NULL, NULL, &iRet );
                                                  0, stBuffSize, 0, NULL, NULL, &iRet );
		bResult &= CHECK(TITLE(L"clEnqueueMapBuffer(CL_MAP_WRITE_INVALIDATE_REGION) for ",dev_name), CL_SUCCESS, iRet);    
        if (!bResult) goto release_buf;
    }

    map_addr = (cl_uint*)map_addrs[0];
        
    for( unsigned ui = 0; ui < BUFFER_SIZE; ui++ )
    {
        map_addr[ui] = (cl_uint)(ui + 0x1000);
    }

    for (unsigned int curr_dev = 0; curr_dev < num_queues; ++curr_dev )
	{
        const wchar_t*   dev_name = dev_names[curr_dev];
        cl_command_queue queue    = dev_queue[curr_dev];
        
        iRet = clEnqueueUnmapMemObject( queue, clBuff, map_addrs[curr_dev], 0, NULL, NULL );
		bResult &= CHECK(TITLE(L"clEnqueueUnmapMemObject for ",dev_name), CL_SUCCESS, iRet);    
        if (!bResult) goto release_buf;

        iRet = clFinish(queue);
        bResult &= CHECK(TITLE(L"clFinish for ",dev_name), CL_SUCCESS, iRet);    
        if (!bResult) goto release_buf;
    }
    
    map_addr = (cl_uint*)clEnqueueMapBuffer( dev_queue[0], clBuff, CL_TRUE, CL_MAP_READ,0, stBuffSize, 0, NULL, NULL, &iRet );
    bResult &= CHECK(TITLE(L"clEnqueueMapBuffer(CL_MAP_READ) for ",dev_names[0]), CL_SUCCESS, iRet);    
    if (!bResult) goto release_buf;

    iRet = CL_SUCCESS;
    for( unsigned ui = 0; ui < BUFFER_SIZE; ui++ )
    {
        if (map_addr[ui] != (cl_uint)(ui + 0x1000))
        {
            iRet = CL_INVALID_VALUE;
            break;
        }
    }

    bResult &= CHECK(TITLE(L"Data Validation for ",dev_names[0]), CL_SUCCESS, iRet);    
    if (!bResult) goto release_buf;

    iRet = clEnqueueUnmapMemObject( dev_queue[1], clBuff, map_addr, 0, NULL, NULL );
    bResult &= CHECK(TITLE(L"clEnqueueUnmapMemObject for ",dev_names[1]), CL_SUCCESS, iRet);    
    if (!bResult) goto release_buf;

    iRet = clFinish(dev_queue[0]);
    bResult &= CHECK(TITLE(L"clFinish for ",dev_names[0]), CL_SUCCESS, iRet);    
    if (!bResult) goto release_buf;

    iRet = clFinish(dev_queue[1]);
    bResult &= CHECK(TITLE(L"clFinish for ",dev_names[1]), CL_SUCCESS, iRet);    
    if (!bResult) goto release_buf;

release_buf:
    iRet = clReleaseMemObject( clBuff );
    bResult &= CHECK(L"clReleaseMemObject", CL_SUCCESS, iRet);    
    return bResult;
}


/**************************************************************************************************
 **************************************************************************************************/
bool cl_CPU_MIC_MapUnmapTest_worker(const char* name, bool use_out_of_order)
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;
    cl_device_id clCpuDeviceId;
    cl_device_id clMicDeviceId;
    cl_command_queue clCpuQueue = NULL;
    cl_command_queue clMicQueue = NULL;
    cl_command_queue_properties queue_type = (use_out_of_order) ? CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE : 0;
        
	printf("=============================================================\n");
	printf("%s\n", name);
	printf("=============================================================\n");

	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= CHECK(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &clCpuDeviceId, NULL);
    bResult &= CHECK(L"clGetDeviceIDs(CL_DEVICE_TYPE_CPU)", CL_SUCCESS, iRet);    
    if (!bResult) return bResult;

    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &clMicDeviceId, NULL);
    if (CL_DEVICE_NOT_FOUND == iRet)
    {
        // CL_DEVICE_TYPE_ACCELERATOR not found - skipping the test
        bResult &= CHECK(L"clGetDeviceIDs(CL_DEVICE_TYPE_ACCELERATOR)", CL_DEVICE_NOT_FOUND, iRet);    
        return bResult;
    }
    
    bResult &= CHECK(L"clGetDeviceIDs(CL_DEVICE_TYPE_ACCELERATOR)", CL_SUCCESS, iRet);    
    if (!bResult) return bResult;

    // For CPU-only uncomment
    //clCpuDeviceId = clMicDeviceId;

    // For MIC-only uncomment
    //clMicDeviceId = clCpuDeviceId;
    
    //
    // Initiate test infrastructure:
    // Create context, Queue
    //
    cl_device_id all_devices[] = { clCpuDeviceId, clMicDeviceId };
	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    cl_context context = clCreateContext( prop, sizeof( all_devices ) / sizeof( all_devices[0] ), all_devices, NULL, NULL, &iRet);
    bResult &= CHECK(L"clCreateContext", CL_SUCCESS, iRet);    
    if (!bResult) return bResult;

    

    clCpuQueue = clCreateCommandQueue (context, clCpuDeviceId, queue_type, &iRet);
    bResult &= CHECK(L"clCreateCommandQueue - queue for Cpu", CL_SUCCESS, iRet);
    if (!bResult) goto release_queues;

    clMicQueue = clCreateCommandQueue (context, clMicDeviceId, queue_type, &iRet);
    bResult &= CHECK(L"clCreateCommandQueue - queue for Mic", CL_SUCCESS, iRet);
    if (!bResult) goto release_queues;

    printf("Map Cpu -> Mic.....\n");
    bResult = move_data( context, L"Cpu", clCpuQueue, L"Mic", clMicQueue );
    if (!bResult) goto release_queues;
    printf("Map Cpu -> Mic.....passed\n");
    
    printf("Map Mic -> Cpu.....\n");
    bResult = move_data( context, L"Mic", clMicQueue, L"Cpu", clCpuQueue );
    if (!bResult) goto release_queues;
    printf("Map Mic -> Cpu.....passed\n");

release_queues:
    if (NULL != clCpuQueue)
    {
		clFinish(clCpuQueue);
		clReleaseCommandQueue(clCpuQueue);
    }
    if (NULL != clMicQueue)
    {
		clFinish(clMicQueue);
		clReleaseCommandQueue(clMicQueue);
    }

    clReleaseContext(context);
    return bResult;
}

/**************************************************************************************************
 * cl_CPU_MIC_MapUnmapTest_InOrder
 * -------------------
 * Implement multi-device access test
 **************************************************************************************************/
bool cl_CPU_MIC_MapUnmapTest_InOrder()
{
    return cl_CPU_MIC_MapUnmapTest_worker( __FUNCTION__, false );
}

/**************************************************************************************************
 * cl_CPU_MIC_MapUnmapTest_OutOfOrder
 * -------------------
 * Implement multi-device access test
 **************************************************************************************************/
bool cl_CPU_MIC_MapUnmapTest_OutOfOrder()
{
    return cl_CPU_MIC_MapUnmapTest_worker( __FUNCTION__, true );
}


