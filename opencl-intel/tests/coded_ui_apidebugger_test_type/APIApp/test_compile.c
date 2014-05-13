/******************************************************************
//
//  OpenCL Conformance Tests
//
//  Copyright:  (c) 2008-2013 by Apple Inc. All Rights Reserved.
//  Copyright:  (c) 2011 by Intel Inc. All Rights Reserved.
//
******************************************************************/

#include "testBase.h"
#if defined(_WIN32)
#include <time.h>
#elif  defined(__linux__) || defined(__APPLE__)
#include <sys/time.h>
#include <unistd.h>
#endif
#include "harness/conversions.h"

extern cl_uint gRandomSeed;

#define MAX_LINE_SIZE_IN_PROGRAM 1024
#define MAX_LOG_SIZE_IN_PROGRAM  2048

const char *sample_kernel_start =
"__kernel void sample_test(__global float *src, __global int *dst)\n"
"{\n"
"    float temp;\n"
"    int  tid = get_global_id(0);\n";

const char *sample_kernel_end = "}\n";

const char *sample_kernel_lines[] = {
"dst[tid] = src[tid];\n",
"dst[tid] = src[tid] * 3.f;\n",
"temp = src[tid] / 4.f;\n",
"dst[tid] = dot(temp,src[tid]);\n",
"dst[tid] = dst[tid] + temp;\n" };

/* I compile and link therefore I am. Robert Ioffe */
/* The following kernels are used in testing Improved Compilation and Linking feature */

const char *simple_kernel =
"__kernel void\n"
"CopyBuffer(\n"
"    __global float* src,\n"
"    __global float* dst )\n"
"{\n"
"    int id = (int)get_global_id(0);\n"
"    dst[id] = src[id];\n"
"}\n";

const char *simple_kernel_with_defines =
"__kernel void\n"
"CopyBuffer(\n"
"    __global float* src,\n"
"    __global float* dst )\n"
"{\n"
"    int id = (int)get_global_id(0);\n"
"    float temp = src[id] - 42;\n"
"    dst[id] = FIRST + temp + SECOND;\n"
"}\n";

const char *simple_kernel_template =
"__kernel void\n"
"CopyBuffer%d(\n"
"    __global float* src,\n"
"    __global float* dst )\n"
"{\n"
"    int id = (int)get_global_id(0);\n"
"    dst[id] = src[id];\n"
"}\n";

const char *composite_kernel_start =
"__kernel void\n"
"CompositeKernel(\n"
"    __global float* src,\n"
"    __global float* dst )\n"
"{\n";

const char *composite_kernel_end = "}\n";

const char *composite_kernel_template =
"    CopyBuffer%d(src, dst);\n";

const char *composite_kernel_extern_template =
"extern __kernel void\n"
"CopyBuffer%d(\n"
"    __global float* src,\n"
"    __global float* dst );\n";

const char *another_simple_kernel =
"extern __kernel void\n"
"CopyBuffer(\n"
"    __global float* src,\n"
"    __global float* dst );\n"
"__kernel void\n"
"AnotherCopyBuffer(\n"
"    __global float* src,\n"
"    __global float* dst )\n"
"{\n"
"    CopyBuffer(src, dst);\n"
"}\n";

const char* simple_header =
"extern __kernel void\n"
"CopyBuffer(\n"
"    __global float* src,\n"
"    __global float* dst );\n";

const char* simple_header_name = "simple_header.h";

const char* another_simple_kernel_with_header =
"#include \"simple_header.h\"\n"
"__kernel void\n"
"AnotherCopyBuffer(\n"
"    __global float* src,\n"
"    __global float* dst )\n"
"{\n"
"    CopyBuffer(src, dst);\n"
"}\n";

const char* header_name_templates[4]   = { "simple_header%d.h",
                                           "foo/simple_header%d.h",
                       "foo/bar/simple_header%d.h",
                       "foo/bar/baz/simple_header%d.h"};

const char* include_header_name_templates[4]   = { "#include \"simple_header%d.h\"\n",
                                                   "#include \"foo/simple_header%d.h\"\n",
                                                   "#include \"foo/bar/simple_header%d.h\"\n",
                                                   "#include \"foo/bar/baz/simple_header%d.h\"\n"};

const char* compile_extern_var      = "extern constant float foo;\n";
const char* compile_extern_struct   = "extern constant struct bar bart;\n";
const char* compile_extern_function = "extern int baz(int, int);\n";

const char* compile_static_var      = "static constant float foo = 2.78;\n";
const char* compile_static_struct   = "static constant struct bar {float x, y, z, r; int color; } foo = {3.14159};\n";
const char* compile_static_function = "static int foo(int x, int y) { return x*x + y*y; }\n";

const char* compile_regular_var      = "constant float foo = 4.0f;\n";
const char* compile_regular_struct   = "constant struct bar {float x, y, z, r; int color; } foo = {0.f, 0.f, 0.f, 0.f, 0};\n";
const char* compile_regular_function = "int foo(int x, int y) { return x*x + y*y; }\n";

const char* link_static_var_access = // use with compile_static_var
"extern constant float foo;\n"
"float access_foo() { return foo; }\n";

const char* link_static_struct_access = // use with compile_static_struct
"extern constant struct bar{float x, y, z, r; int color; } foo;\n"
"struct bar access_foo() {return foo; }\n";

const char* link_static_function_access = // use with compile_static_function
"extern int foo(int, int);\n"
"int access_foo() { int blah = foo(3, 4); return blah + 5; }\n";

static int verifyCopyBuffer(cl_context context, cl_command_queue queue, cl_kernel kernel);

#if defined(__APPLE__) || defined(__linux)
#define _strdup strdup
#endif

struct simple_user_data {
  const char*   m_message;
  cl_event    m_event;
};

const char* when_i_pondered_weak_and_weary = "When I pondered weak and weary!";

static void CL_CALLBACK simple_link_callback(cl_program program, void* user_data)
{
  simple_user_data* simple_link_user_data = (simple_user_data*)user_data;
  log_info("in the simple_link_callback: program %p just completed linking with '%s'\n", program, (const char*)simple_link_user_data->m_message);
  if (strcmp(when_i_pondered_weak_and_weary, simple_link_user_data->m_message) != 0)
  {
    log_error("ERROR: in the simple_compile_callback: Expected '%s' and got %s! (in %s:%d)\n", when_i_pondered_weak_and_weary, simple_link_user_data->m_message, __FILE__, __LINE__);
  }

  int error;
    log_info("in the simple_link_callback: program %p just completed linking with '%p'\n", program, simple_link_user_data->m_event);

    error = clSetUserEventStatus(simple_link_user_data->m_event, CL_RUNNING);
    error = clSetUserEventStatus(simple_link_user_data->m_event, CL_COMPLETE);
    if (error != CL_SUCCESS)
    {
        log_error( "ERROR: simple_link_callback: Unable to set user event status to CL_COMPLETE! (%s in %s:%d)\n", IGetErrorString( error ), __FILE__, __LINE__ );
        exit(-1);
    }
    log_info("in the simple_link_callback: Successfully signaled link_program_completion_event event!\n");
}

int test_simple_link_with_callback(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
  int error;
  cl_program program;
  cl_event link_program_completion_event;

  log_info("Testing a simple linking with callback...\n");
    program = clCreateProgramWithSource(context, 1, &simple_kernel, NULL, &error);
  if( program == NULL || error != CL_SUCCESS )
  {
    log_error( "ERROR: Unable to create a simple test program! (%s in %s:%d)\n", IGetErrorString( error ), __FILE__, __LINE__ );
    return -1;
  }

    error = clCompileProgram(program, 1, NULL, NULL, 0, NULL, NULL, NULL, NULL);
    error = clCompileProgram(program, 1, &deviceID, NULL, 0, NULL, NULL, NULL, NULL);
  test_error( error, "Unable to compile a simple program" );

  link_program_completion_event = clCreateUserEvent(NULL, &error);
  link_program_completion_event = clCreateUserEvent(context, &error);
    test_error( error, "Unable to create a user event");

  error = clRetainEvent(NULL);
  error = clRetainEvent(link_program_completion_event);

  error = clReleaseEvent(link_program_completion_event);

  simple_user_data simple_link_user_data = {when_i_pondered_weak_and_weary, link_program_completion_event};

  clLinkProgram(NULL, 1, &deviceID, NULL, 1, &program, simple_link_callback, (void*)&simple_link_user_data, &error);
  clLinkProgram(context, 1, &deviceID, NULL, 1, &program, simple_link_callback, (void*)&simple_link_user_data, &error);
    test_error( error, "Unable to link a simple program" );

  error = clWaitForEvents(1, &link_program_completion_event);
    test_error( error, "clWaitForEvents failed when waiting on link_program_completion_event");

  /* All done! */
  error = clReleaseEvent(link_program_completion_event);
  test_error( error, "Unable to release event object" );

  error = clReleaseProgram( program );
  test_error( error, "Unable to release program object" );

  return 0;
}

