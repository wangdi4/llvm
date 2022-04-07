// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
// program_service_test.cpp
///////////////////////////////////////////////////////////

#include "cpu_dev_test.h"
#include "program_service_test.h"
#include "memory_test.h"

#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>

extern RTMemObjService localRTMemService;

bool KernelExecute_Dot_Test(const char* prog_file)
{
    cl_dev_program prog;
    cl_dev_kernel kernel;

    float a[] = {1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 2.0f, 2.0f, 3.0f, 3.0f, 3.0f, 3.0f, 4.0f, 4.0f, 4.0f, 4.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 2.0f, 2.0f, 3.0f, 3.0f, 3.0f, 3.0f, 4.0f, 4.0f, 4.0f, 4.0f};
    float b[] = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    float    res[] = {-1, -1, -1, -1, -1, -1, -1, -1};
    float    tst[] = {8, 16, 24, 32, 8, 16, 24, 32};

    size_t    stSizeRes = sizeof(res);
    size_t    stSizeAB = sizeof(a);
    size_t    size_res = sizeof(res)/sizeof(float);

    if ( !BuildProgram(prog_file, &prog) )
    {
        return false;
    }

    if ( !CreateKernel(prog, "dot_product_2D", &kernel) )
    {
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    IOCLDevMemoryObject *memObjA, *memObjB, *memObjRes;

    //Create Memory Object
    localRTMemService.SetupState( NULL, 1, &stSizeAB, NULL, CL_MEM_OBJECT_BUFFER );
    cl_int iRes = dev_entry->clDevCreateMemoryObject(0, CL_MEM_READ_ONLY, NULL, 1, &stSizeAB, &localRTMemService, &memObjA);
    if (CL_DEV_FAILED(iRes))
    {
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCreateMemoryObject failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    localRTMemService.SetupState( NULL, 1, &stSizeAB, NULL, CL_MEM_OBJECT_BUFFER );
    iRes = dev_entry->clDevCreateMemoryObject(0, CL_MEM_READ_ONLY, NULL, 1, &stSizeAB, &localRTMemService, &memObjB);
    if (CL_DEV_FAILED(iRes))
    {
        memObjA->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCreateMemoryObject failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    localRTMemService.SetupState( NULL, 1, &stSizeRes, NULL, CL_MEM_OBJECT_BUFFER );
    iRes = dev_entry->clDevCreateMemoryObject(0, CL_MEM_WRITE_ONLY, NULL, 1, &stSizeRes, &localRTMemService, &memObjRes);
    if (CL_DEV_FAILED(iRes))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCreateMemoryObject failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    //Enqueue write memory
    if(!writeMemory(false, memObjA, a, sizeof(a)))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }
    if(!writeMemory(false, memObjB, b, sizeof(b)))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    // Query device argument parameters
    cl_dev_dispatch_buffer_prop dispatch_prop;
    iRes = dev_entry->clDevGetKernelInfo(kernel, CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES, 0, NULL, sizeof(dispatch_prop), &dispatch_prop, NULL);
    if (CL_DEV_FAILED(iRes))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("clDevGetKernelInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    // Setup Kernel parameters
    cl_dev_cmd_desc    cmdDesc;
    cl_dev_cmd_param_kernel krnlParam;

    krnlParam.kernel = kernel;
    krnlParam.arg_size = dispatch_prop.size;
    krnlParam.arg_values = ALIGNED_MALLOC(dispatch_prop.size, dispatch_prop.alignment);
    krnlParam.uiNonArgSvmBuffersCount = 0;
    krnlParam.ppNonArgSvmBuffers = nullptr;
    krnlParam.uiNonArgUsmBuffersCount = 0;
    krnlParam.ppNonArgUsmBuffers = nullptr;
    
    ((IOCLDevMemoryObject**)((char*)krnlParam.arg_values+dispatch_prop.argumentOffset))[0] = memObjA;
    ((IOCLDevMemoryObject**)((char*)krnlParam.arg_values+dispatch_prop.argumentOffset))[1] = memObjB;
    ((IOCLDevMemoryObject**)((char*)krnlParam.arg_values+dispatch_prop.argumentOffset))[2] = memObjRes;

//    *((float*)((cl_char*)krnlParam.arg_values+3*sizeof(IOCLDevMemoryObject*))) = 2.f;
    krnlParam.work_dim = 2;
    krnlParam.glb_wrk_offs[0] = 0;
    krnlParam.glb_wrk_offs[1] = 0;
    krnlParam.glb_wrk_size[0] = 4;
    krnlParam.glb_wrk_size[1] = 2;
    krnlParam.lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][0] = krnlParam.lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][0] = 2;
    krnlParam.lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][1] = krnlParam.lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][1] = 1;
//    krnlParam.work_dim = 1;
//    krnlParam.glb_wrk_offs[0] = 0;
//    krnlParam.glb_wrk_size[0] = 8;
//    krnlParam.lcl_wrk_size[0] = 1;

    // Setup command descriptor
    cmdDesc.id = (cl_dev_cmd_id)CL_DEV_CMD_EXEC_KERNEL;
    cmdDesc.params = &krnlParam;
    cmdDesc.param_size = sizeof(cl_dev_cmd_param_kernel);
    cmdDesc.type = CL_DEV_CMD_EXEC_KERNEL;

    // Enqueu kernel
    gExecDone = false;
    //cl_int clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count );
    cl_dev_cmd_desc* cmdsBuff = &cmdDesc;
    iRes = dev_entry->clDevCommandListExecute(0, &cmdsBuff, 1);
    if (CL_DEV_FAILED(iRes))
    {
        ALIGNED_FREE(krnlParam.arg_values);
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCommandListExecute failed: %s\n", clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    while(!gExecDone )
    {
        SLEEP(10);
    }

    if(!readMemory(false, memObjRes, res, sizeof(res)))
    {
        ALIGNED_FREE(krnlParam.arg_values);
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    // test result
    bool bRes = true;
    for(size_t i=0; i< size_res; ++i)
    {
        if ( res[i] != tst[i] )
        {
            bRes = false;
        }
    }

    ALIGNED_FREE(krnlParam.arg_values);
    memObjA->clDevMemObjRelease();
    memObjB->clDevMemObjRelease();
    dev_entry->clDevReleaseProgram(prog);

    return bRes;
}

bool KernelExecute_Lcl_Mem_Test(const char* prog_file)
{
    cl_dev_program prog;
    cl_dev_kernel kernel;

    char    BuffA[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    char    BuffB[10];
    size_t    sizeAB = sizeof(BuffA)/sizeof(char);

    if ( !BuildProgram(prog_file, &prog) )
    {
        return false;
    }

    if ( !CreateKernel(prog, "lcl_mem_test", &kernel) )
    {
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    IOCLDevMemoryObject *memObjA, *memObjB;

    //Create Memory Object
    localRTMemService.SetupState( NULL, 1, &sizeAB, NULL, CL_MEM_OBJECT_BUFFER );
    cl_int iRes = dev_entry->clDevCreateMemoryObject(0, CL_MEM_READ_WRITE, NULL, 1, &sizeAB, &localRTMemService, &memObjA);
    if (CL_DEV_FAILED(iRes))
    {
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCreateMemoryObject failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    localRTMemService.SetupState( NULL, 1, &sizeAB, NULL, CL_MEM_OBJECT_BUFFER );
    iRes = dev_entry->clDevCreateMemoryObject(0, CL_MEM_READ_WRITE, NULL, 1, &sizeAB, &localRTMemService, &memObjB);
    if (CL_DEV_FAILED(iRes))
    {
        memObjA->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCreateMemoryObject failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    //Enqeue write memory
    if(!writeMemory(false, memObjA, BuffA, sizeAB))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    // Setup Kernel parameters
    cl_dev_cmd_desc    cmdDesc;
    cl_dev_cmd_param_kernel krnlParam;
    cl_dev_dispatch_buffer_prop dispatch_prop;

    // Query device argument parameters
    iRes = dev_entry->clDevGetKernelInfo(kernel, CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES, 0, NULL, sizeof(dispatch_prop), &dispatch_prop, NULL);
    if (CL_DEV_FAILED(iRes))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("clDevGetKernelInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    krnlParam.kernel = kernel;
    krnlParam.arg_size = dispatch_prop.size;
    krnlParam.arg_values = ALIGNED_MALLOC(dispatch_prop.size, dispatch_prop.alignment);
    krnlParam.uiNonArgSvmBuffersCount = 0;
    krnlParam.ppNonArgSvmBuffers = nullptr;
    krnlParam.uiNonArgUsmBuffersCount = 0;
    krnlParam.ppNonArgUsmBuffers = nullptr;
    
    ((IOCLDevMemoryObject**)((char*)krnlParam.arg_values+dispatch_prop.argumentOffset))[0] = memObjA;
    ((IOCLDevMemoryObject**)((char*)krnlParam.arg_values+dispatch_prop.argumentOffset))[1] = memObjB;

    krnlParam.work_dim = 1;
    krnlParam.glb_wrk_offs[0] = 0;
    krnlParam.glb_wrk_size[0] = 20;
    krnlParam.lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][0] = krnlParam.lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][0] = 1;

    // Setup command descriptor
    cmdDesc.id = (cl_dev_cmd_id)CL_DEV_CMD_EXEC_KERNEL;
    cmdDesc.params = &krnlParam;
    cmdDesc.param_size = sizeof(cl_dev_cmd_param_kernel);
    cmdDesc.type = CL_DEV_CMD_EXEC_KERNEL;

    // Enqueu kernel
    gExecDone = false;
    //cl_int clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count );
    cl_dev_cmd_desc* cmdsBuff = &cmdDesc;
    iRes = dev_entry->clDevCommandListExecute(0, &cmdsBuff, 1);
    if (CL_DEV_FAILED(iRes))
    {
        ALIGNED_FREE(krnlParam.arg_values);
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCommandListExecute failed: %s\n", clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    while(!gExecDone )
    {
        SLEEP(10);
    }

    if(!readMemory(false, memObjB, BuffB, sizeAB))
    {
        ALIGNED_FREE(krnlParam.arg_values);
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    // test result
    for (size_t i=0; i<sizeAB; ++i)
    {
        if ( BuffA[i] == BuffB[i] )
        {
            ALIGNED_FREE(krnlParam.arg_values);
            memObjA->clDevMemObjRelease();
            memObjB->clDevMemObjRelease();
            dev_entry->clDevReleaseProgram(prog);
            return false;
        }
    }

    ALIGNED_FREE(krnlParam.arg_values);
    memObjA->clDevMemObjRelease();
    memObjB->clDevMemObjRelease();
    dev_entry->clDevReleaseProgram(prog);

    return true;
}

#define TEST_BUFF_SIZE 32

bool KernelExecute_Math_Test(const char* prog_file)
{
    cl_dev_program prog;
    cl_dev_kernel kernel;

    float    BuffA[TEST_BUFF_SIZE];
    float    BuffB[TEST_BUFF_SIZE];
    float    BuffRef[TEST_BUFF_SIZE];;
    size_t    sizeAB = sizeof(BuffA);

    // Fill Source buffer
    //srand(time(NULL));
    srand(0);
    for(int i=0; i<TEST_BUFF_SIZE; ++i)
    {
        BuffA[i] = ((float)(rand() % 100)) / 100.f;
        BuffB[i] = -1.;
        BuffRef[i] = sqrtf(BuffA[i]);
    }

    if ( !BuildProgram(prog_file, &prog) )
    {
        return false;
    }

    if ( !CreateKernel(prog, "math_func_test_f4", &kernel) )
    {
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    IOCLDevMemoryObject *memObjA, *memObjB;

    //Create Memory Object
    localRTMemService.SetupState( NULL, 1, &sizeAB, NULL, CL_MEM_OBJECT_BUFFER );
    cl_int iRes = dev_entry->clDevCreateMemoryObject(0, CL_MEM_READ_WRITE, NULL, 1, &sizeAB, &localRTMemService, &memObjA);
    if (CL_DEV_FAILED(iRes))
    {
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCreateMemoryObject failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    localRTMemService.SetupState( NULL, 1, &sizeAB, NULL, CL_MEM_OBJECT_BUFFER );
    iRes = dev_entry->clDevCreateMemoryObject(0, CL_MEM_READ_WRITE, NULL, 1, &sizeAB, &localRTMemService, &memObjB);
    if (CL_DEV_FAILED(iRes))
    {
        memObjA->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCreateMemoryObject failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    //Enqueue write memory
    if(!writeMemory(false, memObjA, BuffA, sizeAB))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        return false;
    }

    // Setup Kernel parameters
    cl_dev_cmd_desc    cmdDesc;
    cl_dev_cmd_param_kernel krnlParam;
    cl_dev_dispatch_buffer_prop dispatch_prop;

    // Query device argument parameters
    iRes = dev_entry->clDevGetKernelInfo(kernel, CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES, 0, NULL, sizeof(dispatch_prop), &dispatch_prop, NULL);
    if (CL_DEV_FAILED(iRes))
    {
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("clDevGetKernelInfo failed: %s\n",clDevErr2Txt((cl_dev_err_code)iRes));
        return false;
    }

    krnlParam.kernel = kernel;
    krnlParam.arg_size = dispatch_prop.size;
    krnlParam.arg_values = ALIGNED_MALLOC(dispatch_prop.size, dispatch_prop.alignment);
    krnlParam.uiNonArgSvmBuffersCount = 0;
    krnlParam.ppNonArgSvmBuffers = nullptr;
    krnlParam.uiNonArgUsmBuffersCount = 0;
    krnlParam.ppNonArgUsmBuffers = nullptr;
    
    ((IOCLDevMemoryObject**)((char*)krnlParam.arg_values+dispatch_prop.argumentOffset))[0] = memObjA;
    ((IOCLDevMemoryObject**)((char*)krnlParam.arg_values+dispatch_prop.argumentOffset))[1] = memObjB;

    krnlParam.work_dim = 1;
    krnlParam.glb_wrk_offs[0] = 0;
    krnlParam.glb_wrk_size[0] = TEST_BUFF_SIZE/4; // Kernel forks on float4
    krnlParam.lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][0] = krnlParam.lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][0] = 0;

    // Setup command descriptor
    cmdDesc.id = (cl_dev_cmd_id)CL_DEV_CMD_EXEC_KERNEL;
    cmdDesc.params = &krnlParam;
    cmdDesc.param_size = sizeof(cl_dev_cmd_param_kernel);
    cmdDesc.type = CL_DEV_CMD_EXEC_KERNEL;

    // Enqueu kernel
    gExecDone = false;
    //cl_int clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count );
    cl_dev_cmd_desc* cmdsBuff = &cmdDesc;
    iRes = dev_entry->clDevCommandListExecute(0, &cmdsBuff, 1);
    if (CL_DEV_FAILED(iRes))
    {
        delete [] (char*)(krnlParam.arg_values);
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        printf("pclDevCommandListExecute failed: %s\n", clDevErr2Txt((cl_dev_err_code)iRes));
        ALIGNED_FREE(krnlParam.arg_values);
        return false;
    }

    while(!gExecDone )
    {
        SLEEP(10);
    }

    if(!readMemory(false, memObjB, BuffB, sizeAB))
    {
        delete [] (char*)(krnlParam.arg_values);
        memObjA->clDevMemObjRelease();
        memObjB->clDevMemObjRelease();
        dev_entry->clDevReleaseProgram(prog);
        ALIGNED_FREE(krnlParam.arg_values);
        return false;
    }

    // test result
    for (int i=0; i<TEST_BUFF_SIZE; ++i)
    {
        if ( abs(((int*)BuffRef)[i] - ((int*)BuffB)[i]) > 3 )
        {
            delete [] (char*)(krnlParam.arg_values);
            memObjA->clDevMemObjRelease();
            memObjB->clDevMemObjRelease();
            dev_entry->clDevReleaseProgram(prog);
            ALIGNED_FREE(krnlParam.arg_values);
            return false;
        }
    }

    memObjA->clDevMemObjRelease();
    memObjB->clDevMemObjRelease();
    dev_entry->clDevReleaseProgram(prog);
    ALIGNED_FREE(krnlParam.arg_values);

    return true;
}

