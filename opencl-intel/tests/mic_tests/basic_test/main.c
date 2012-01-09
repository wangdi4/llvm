#include <CL/opencl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

/*
const char *source = {"                                  \n"
        "kernel void hello(global int *in, global int *out)   \n"
        "{                                               \n"
        "  int id = get_global_id(0);                    \n"
        "  out[id] = in[id];                             \n"
        "}                                               \n"
        };
*/

/*
const char *source = {"                                      \n"
         "__kernel void hello(__global const float *memA,    \n"
         "__global const float *memB, __global float *memC,  \n"
         "const float s)                                     \n"
         "{                                                  \n"
         "   int gid = get_global_id(0);                     \n"
         "   memC[gid] = memA[gid] + s*memB[gid];            \n"
         "}                                                  \n"
    };
*/

const char *source = {"                                      \n"
         "__kernel void hello(__global const float *mem_in,  \n"
         "                    __global float *mem_out )      \n"
         "{                                                  \n"
         "   int gid = get_global_id(0);                     \n"
         "   mem_out[gid] = mem_in[gid] + mem_in[gid];       \n"
         "}                                                  \n"
    };

const unsigned char code[] = {
        0x55,                      // push   %rbp
        0x48, 0x89, 0xe5,          // mov    %rsp,%rbp
        0x89, 0x7d, 0xfc,          // mov    %edi,-0x4(%rbp)
        0x48, 0x89, 0x75, 0xf0,    // mov    %rsi,-0x10(%rbp)
        0x48, 0x89, 0x55, 0xe8,    // mov    %rdx,-0x18(%rbp)
        0x8b, 0x45, 0xfc,          // mov    -0x4(%rbp),%eax
        0x48, 0x98,                // cltq
        0x48, 0xc1, 0xe0, 0x02,    // shl    $0x2,%rax
        0x48, 0x03, 0x45, 0xe8,    // add    -0x18(%rbp),%rax
        0x8b, 0x55, 0xfc,          // mov    -0x4(%rbp),%edx
        0x48, 0x63, 0xd2,          // movslq %edx,%rdx
        0x48, 0xc1, 0xe2, 0x02,    // shl    $0x2,%rdx
        0x48, 0x03, 0x55, 0xf0,    // add    -0x10(%rbp),%rdx
        0x8b, 0x12,                // mov    (%rdx),%edx
        0x83, 0xc2, 0x01,          // add    $0x1,%edx
        0x89, 0x10,                // mov    %edx,(%rax)
        0xc9,                      // leaveq
        0xc3                       // retq
};

static void ttttt( void* a)
{
    printf("ttttt called\n");
}

typedef void (*pFunc)(int idx, int* in, int* out);

int main( void )
{
    cl_int         ok;
    cl_uint        num_platforms;
    cl_platform_id platforms[4];
    cl_uint        num_devices;
    cl_device_id   devices[4];
    cl_device_id   mic_device;
    cl_device_type device_type;
    size_t         param_filled;
    cl_context     context;
    cl_command_queue queue;
    cl_program     program;
    cl_kernel      kernel;
    cl_mem         buff_in, buff_out;
    int i;
    char           log[1024];

    pFunc          func;
    int            ii = 5;
    int            oo;

    void*  jj;
/*    oo = posix_memalign(&jj, 4096, sizeof(code));
    if (0 != oo)
    {
        printf("posix_memalign returned %d\n", oo);
        return 0;
    }
*/
    jj = mmap( NULL, 4096, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (MAP_FAILED == jj)
    {
        printf("Cannot mmap jj\n");
        perror(NULL);
        return 0;
    }

    if (NULL == jj)
    {
        printf("Cannot alloc jj\n");
        return 0;
    }


    memcpy( jj, code, sizeof(code) );
    func = (pFunc)jj;
    /*oo = mprotect( jj, sizeof(code), PROT_EXEC|PROT_READ );
    if (0 != oo)
    {
        printf("mprotect returned %d\n", oo);
        perror(NULL);
        return 0;
    }
*/
    func(0, &ii, &oo);

/*    oo = mprotect( jj, sizeof(code), PROT_READ|PROT_WRITE );
    if (0 != oo)
    {
        printf("mprotect returned %d\n", oo);
        perror(NULL);
        return 0;
    }
*/

    ok = clGetPlatformIDs(4, platforms, &num_platforms);

    if (CL_SUCCESS != ok)
    {
        printf( "clGetPlatformIDs returned %d\n", ok );
        goto done;
    }

    printf("clGetPlatformIDs returned platforms:");
    for (i = 0; i < num_platforms; ++i)
    {
        printf(" %p", platforms[i]);
    }
    printf("\n");

    // -------------------------------------------------------------------

    ok = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 4, devices, &num_devices);

    if (CL_SUCCESS != ok)
    {
        printf( "clGetDeviceIDs returned %d\n", ok );
        goto done;
    }

    printf("clGetDeviceIDs returned devices:\n");
    for (i = 0; i < num_devices; ++i)
    {
        printf(" %p", devices[i]);
        ok = clGetDeviceInfo( devices[i], CL_DEVICE_TYPE, sizeof(device_type), &device_type, &param_filled );

        if (CL_SUCCESS != ok)
        {
            printf( "clGetDeviceInfo(CL_DEVICE_TYPE) returned %d\n", ok );
            continue;
        }
        else
        {
#define PR_DEV_TYPE(d, type) if ((d) & type) { printf( " " #type); }
            PR_DEV_TYPE( device_type, CL_DEVICE_TYPE_DEFAULT );
            PR_DEV_TYPE( device_type, CL_DEVICE_TYPE_CPU );
            PR_DEV_TYPE( device_type, CL_DEVICE_TYPE_GPU );
            PR_DEV_TYPE( device_type, CL_DEVICE_TYPE_ACCELERATOR );
        }

        printf("\n");
    }

    // -------------------------------------------------------------------

    mic_device = devices[1];
    context = clCreateContext( NULL, 1, &mic_device, NULL, NULL, &ok );

    if (CL_SUCCESS != ok)
    {
        printf( "clCreateContext returned %d\n", ok );
        goto done;
    }

    if (NULL == context)
    {
        printf( "clCreateContext returned zero context\n" );
        goto done;
    }

    printf("Context=%p\n", context);

    queue = clCreateCommandQueue(context, mic_device, 0, &ok);

    if (CL_SUCCESS != ok)
    {
        printf( "clCreateCommandQueue returned %d\n", ok );
        goto done;
    }

    if (NULL == queue)
    {
        printf( "clCreateCommandQueue returned zero queue\n" );
        goto done;
    }

    printf("Queue=%p\n", queue);

    program = clCreateProgramWithSource(context,
                                        1, &source, NULL,
                                        &ok);

    if (CL_SUCCESS != ok)
    {
        printf( "clCreateProgramWithSource returned %d\n", ok );
        goto done;
    }

    if (NULL == queue)
    {
        printf( "clCreateProgramWithSource returned zero queue\n" );
        goto done;
    }

    printf("program=%p\n", program);

    ok = clBuildProgram( program,
                         1, &mic_device,
                         "",
                         NULL, NULL);

    if (CL_SUCCESS != ok)
    {
        size_t log_used;
        printf( "clBuildProgram returned %d\n", ok );

        ok = clGetProgramBuildInfo(program, mic_device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, &log_used);

        if ((CL_SUCCESS == ok) && (log_used > 0))
        {
            printf("%s\n",log);
        }
        else
        {
            printf("Cannot get build log\n");
        }

        goto done;
    }

    printf("Program built ok\n");

    kernel = clCreateKernel(program, "hello", &ok);

    if (CL_SUCCESS != ok)
    {
        printf( "clCreateKernel returned %d\n", ok );
        goto done;
    }

    printf("Kernel hello: %p\n", kernel);

    // create buffers
    #define BUF_SIZE_IN_INTS 256
    {
        cl_float b_init[BUF_SIZE_IN_INTS];
        int i;

        for (i=0; i < BUF_SIZE_IN_INTS; ++i)
        {
            b_init[i] = (cl_float)i * 10;
        }

        buff_in = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, BUF_SIZE_IN_INTS*sizeof(cl_int), (void*)b_init, &ok);

        if (CL_SUCCESS != ok)
        {
            printf( "clCreateBuffer for buff_in returned %d\n", ok );
            goto done;
        }

        buff_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY, BUF_SIZE_IN_INTS*sizeof(cl_int), NULL, &ok);

        if (CL_SUCCESS != ok)
        {
            printf( "clCreateBuffer for buff_out returned %d\n", ok );
            goto done;
        }
    }

    printf(" Buff_in = %p\n Buff_out= %p\n", buff_in, buff_out );

    ok = clSetKernelArg( kernel, 0, sizeof(cl_mem), &buff_in);

    if (CL_SUCCESS != ok)
    {
        printf( "clSetKernelArg for buff_in returned %d\n", ok );
        goto done;
    }

    ok = clSetKernelArg( kernel, 1, sizeof(cl_mem), &buff_out);

    if (CL_SUCCESS != ok)
    {
        printf( "clSetKernelArg for buff_out returned %d\n", ok );
        goto done;
    }

    size_t global_work_size[] = {BUF_SIZE_IN_INTS, 0, 0};
    size_t local_work_size[] = {1, 0, 0};
    ok = clEnqueueNDRangeKernel( queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL );
    if (CL_SUCCESS != ok)
    {
        printf( "clEnqueueNDRangeKernel returned %d\n", ok );
        goto done;
    }

//    ok = clEnqueueNativeKernel( queue, ttttt, NULL, 0, 0, NULL, NULL, 0, NULL, NULL );
//
//    if (CL_SUCCESS != ok)
//    {
//        printf( "clEnqueueNativeKernel returned %d\n", ok );
//        goto done;
//    }


    void* out_mapped = clEnqueueMapBuffer(queue,
                                          buff_out, 0 /*FALSE*/,
                                          CL_MAP_READ, 0, BUF_SIZE_IN_INTS*sizeof(cl_int),
                                          0, NULL,
                                          NULL,
                                          &ok );

    if (CL_SUCCESS != ok)
    {
        printf( "clEnqueueMapBuffer returned %d\n", ok );
        goto done;
    }

    ok = clFinish(queue);

    if (CL_SUCCESS != ok)
    {
        printf( "clFinish returned %d\n", ok );
        goto done;
    }

    cl_float* out_data = (cl_float*)out_mapped;
    for (i=0; i < BUF_SIZE_IN_INTS; ++i)
    {
        printf("out[%d] = %f\n", i, out_data[i]);
    }


    ok = clReleaseCommandQueue(queue);

    if (CL_SUCCESS != ok)
    {
        printf( "clReleaseCommandQueue returned %d\n", ok );
        goto done;
    }

    ok = clReleaseContext(context);

    if (CL_SUCCESS != ok)
    {
        printf( "clReleaseContext returned %d\n", ok );
        goto done;
    }

done:
    printf("test Done\n");
    return 0;
}
