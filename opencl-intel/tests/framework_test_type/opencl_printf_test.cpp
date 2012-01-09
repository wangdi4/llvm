// Tests for the opencl_printf builtin (accessible as a 'printf' call inside
// kernels).
//
// Note: the opencl_printf is comprehensively tested in the backend. Here the
// goal is only to verify that a printf call from the kernel is correctly
// linked to the opencl_printf builtin, and allows to print vectors.
//
#include "CL/cl.h"

// Use the OpenCL C++ bindings, with exceptions enabled. For MSVC, disable
// warning 4290 (C++ exception specifications ignored) that's emitted from
// CL/cl.hpp
//
#define __CL_ENABLE_EXCEPTIONS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4290)
#endif  // _MSC_VER
#include "CL/cl.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

#include "cl_utils.h"
#include "test_utils.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <string>
using namespace std;


const bool DEBUG = false;


// Since kernel output may be in arbitrary order (we can't ensure which work 
// item runs first), to compare it to an expected output we split the output to
// lines and sort them.
// Note: this assumes each work-item outputs one or more lines ending with '\n'
//
bool compare_kernel_output(const string& expected, const string& actual)
{
    vector<string> expected_vec = tokenize(expected, "\n\r");
    vector<string> actual_vec = tokenize(actual, "\n\r");
    
    sort(expected_vec.begin(), expected_vec.end());
    sort(actual_vec.begin(), actual_vec.end());

    return expected_vec == actual_vec;
}


const char* KERNEL_CODE_STR = ""
    "__kernel void hello(__global uchar* buf_in, __global uchar* buf_out)"
    "{ buf_out[get_global_id(0)] = buf_in[get_global_id(0)]; "
    " float4 fl4 = (float4)(1.1f, 2.2f, 3.3f, 4.4f);"
    " fl4.w += (float)get_global_id(0);"
	" int2 ii2 = (int2)(get_global_id(0), 9);"
    " printf(\"%d %6.2v4f - %v2d - a char %c and an int %d\\n\", ii2.x, fl4, ii2, 'k', 112233); "
    "}";


const char* EXPECTED_OUTPUT = ""
	"0   1.10,  2.20,  3.30,  4.40 - 0,9 - a char k and an int 112233\n"
	"1   1.10,  2.20,  3.30,  5.40 - 1,9 - a char k and an int 112233\n"
	"2   1.10,  2.20,  3.30,  6.40 - 2,9 - a char k and an int 112233\n"
	"3   1.10,  2.20,  3.30,  7.40 - 3,9 - a char k and an int 112233\n";


bool opencl_printf_test()
{
    cl_int err = CL_SUCCESS;
    string kernel_code = KERNEL_CODE_STR;

	cout << "---------------------------------------\n";
	cout << "opencl_printf_test\n";
	cout << "---------------------------------------\n";

    if (DEBUG) {
        cout 
            << "Running kernel:\n----------------------------------------------\n" 
            << kernel_code << "\n----------------------------------------------\n";
    }
    
    try 
    {
        vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.size() == 0) {
            cout << "FAIL: 0 platforms found\n";
            return false;
        }

        cl_context_properties properties[] = 
        { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
        cl::Context context(CL_DEVICE_TYPE_CPU, properties); 

        vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        cl::CommandQueue queue(context, devices[0], 0, &err);

        cl::Program::Sources source(1,
            make_pair(kernel_code.c_str(), strlen(kernel_code.c_str())));
        cl::Program program_ = cl::Program(context, source);

        // In case of a build error, we can provide more information on the
        // failure from the build log.
        //
        try 
        {
            program_.build(devices);
        }
        catch (cl::Error err)
        {
            string buildlog;
            program_.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &buildlog);
            cout << "FAIL: Build log:\n" << buildlog << endl;
            throw;
        }

        cl::Kernel kernel(program_, "hello", &err);

        size_t datalen = 4;

        cl::Buffer buf_in(context, 
            CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
            sizeof(cl_uchar) * datalen,
            0,
            &err);

        cl::Buffer buf_out(context, 
            CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, 
            sizeof(cl_uchar) * datalen,
            0,
            &err);

        kernel.setArg(0, buf_in);
        kernel.setArg(1, buf_out);

        CaptureStdout();
        queue.enqueueNDRangeKernel(
            kernel, 
            cl::NullRange, 
            cl::NDRange(datalen),
            cl::NullRange,
            NULL,
            NULL); 
        queue.finish();

        string out = GetCapturedStdout();
		if (compare_kernel_output(EXPECTED_OUTPUT, out))
			return true;
		else {
			cout << "FAIL: kernel output verification failed" << endl;
			cout << "Expected:\n" << EXPECTED_OUTPUT << "------------\n";
			cout << "Got:\n" << out << "------------\n";
			return false;
		}
        cout << "Captured stdout:\n" << out << endl;
    }
    catch (cl::Error err) 
    {
        cout 
            << "FAIL: "
            << err.what()
            << "("
            << err.err()
            << ")"
            << endl;

        fprintf(stderr, "ClErrTxt error: %ws\n", ClErrTxt(err.err()));
        return false;
    }

	if (DEBUG) {
		printf("-----------------------------------------\n");
		printf("And now back to the console once again\n");
	}

    return true;
}

