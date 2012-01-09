// Copyright (c) 1997-2004 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTO_S_/_"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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
#include "DiscreteClass.h"
#include "Arrangement.h"
#include <string.h>
#include <Windows.h>

static const char*     ssProgram[] =
{
    "__kernel void test_kernel(int x)",
    "{",
    "    return;",
    "}"
};
//    "int main(int argv, char* argv[])",
//    "{",
//    "    return 0;",
//    "}"
//};
static const cl_int    count = 4;
static unsigned char   binary[1024 * 1024];

unsigned char* CreateBinary(cl_context* cntxt, cl_program* prog)
{
    int             r;
    // avoid errors on the cleanup
    *prog = (cl_program)INVALID_VALUE;
    *cntxt = (cl_context)INVALID_VALUE;
    r = context(cntxt);
    if (PROCESSED_FAIL == r)
        return NULL;
    *prog = clCreateProgramWithSource(*cntxt, count, ssProgram, NULL, NULL);
    if (NULL == prog)
        return NULL;
    else
    {
        cl_int          rc;
        cl_device_id    dev;
        rc = clGetContextInfo(*cntxt, CL_CONTEXT_DEVICES, sizeof(dev), &dev, NULL);
        if (CL_SUCCESS != rc)
        {
            prog_cleanup(*cntxt, *prog);
            return NULL;
        }
        rc = clBuildProgram(*prog, 1, &dev, "-w", NULL, NULL);
        if (CL_SUCCESS != rc)
        {
            prog_cleanup(*cntxt, *prog);
            return NULL;
        }
        size_t          bin_size;
        rc = clGetProgramInfo(*prog, CL_PROGRAM_BINARY_SIZES, sizeof(bin_size), &bin_size, NULL);
        if (CL_SUCCESS != rc)
        {
            prog_cleanup(*cntxt, *prog);
            return NULL;
        }
        rc = clGetProgramInfo(*prog, CL_PROGRAM_BINARIES, sizeof(binary), &binary, NULL);
        if (CL_SUCCESS != rc)
        {
            prog_cleanup(*cntxt, *prog);
            return NULL;
        }
    }

    return binary;
}


void context_cleanup(const cl_context cntxt)
{
    if ((cl_context)INVALID_VALUE != cntxt && (cl_context)0 != cntxt)
        clReleaseContext(cntxt);
}

void cntxt_mem_cleanup(const cl_context cntxt, const cl_mem mem)
{
    if ((cl_mem)INVALID_VALUE != mem && (cl_mem)0 != mem)
        clReleaseMemObject(mem);
    context_cleanup(cntxt);
}

void queue_cleanup(const cl_context cntxt, const cl_command_queue queue)
{
    if ((cl_context)INVALID_VALUE != cntxt)
        clReleaseContext(cntxt);
    if ((cl_command_queue)INVALID_VALUE != queue)
        clReleaseCommandQueue(queue);
}

int queue(cl_context* cntxt, cl_device_id* dev, cl_command_queue* queue)
{
    cl_int          r;
    cl_uint         ndev;

    // avoid errors on the cleanup
    *queue = (cl_command_queue)INVALID_VALUE;
    *cntxt = (cl_context)INVALID_VALUE;
    *dev = (cl_device_id)INVALID_VALUE;

    r = context(cntxt);
    if (PROCESSED_FAIL == r)
        return PROCESSED_FAIL;
    r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_DEFAULT, 1, dev, &ndev);
    if (CL_SUCCESS != r)
    {
        queue_cleanup(*cntxt, *queue);
        return PROCESSED_FAIL;
    }
    *queue = clCreateCommandQueue(*cntxt, *dev, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &r);
    if (CL_SUCCESS != r)
    {
        queue_cleanup(*cntxt, *queue);
        return PROCESSED_FAIL;
    }
    return PROCESSED_OK;
}

void prog_cleanup(const cl_context cntxt, const cl_program prog)
{
    if ((cl_context)INVALID_VALUE != cntxt)
        clReleaseContext(cntxt);
    if ((cl_program)INVALID_VALUE != prog)
        clReleaseProgram(prog);
}

int prog_create(cl_context* cntxt, cl_program* prog)
{
    return PROCESSED_OK;
}

int context(cl_context* cntxt)
{
    cl_int          r;
    *cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
    if (NULL == *cntxt || CL_SUCCESS != r)
        return PROCESSED_FAIL;
    return PROCESSED_OK;
}

int cntxt_dev(cl_context* cntxt, cl_device_id* dev)
{
    int             rc;
    // avoid errors on the cleanup
    *cntxt = (cl_context)INVALID_VALUE;
    *dev = (cl_device_id)INVALID_VALUE;

    rc = context(cntxt);
    if (PROCESSED_OK != rc)
    {
        context_cleanup(*cntxt);
        return PROCESSED_FAIL;
    }
    cl_int          r;
    cl_uint         ndev;
    r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_DEFAULT, 1, dev, &ndev);
    if (CL_SUCCESS != r)
    {
        context_cleanup(*cntxt);
        return PROCESSED_FAIL;
    }

    return PROCESSED_OK;
}

void kernel_cleanup(const cl_context cntxt, const cl_program prog, const cl_kernel kernel)
{
    if ((cl_kernel)INVALID_VALUE != kernel)
        clReleaseKernel(kernel);
    prog_cleanup(cntxt, prog);
}

int kernel(cl_context* cntxt, cl_program* prog, cl_kernel* kernel)
{
    return PROCESSED_OK;
}

Arrangement::Arrangement()
{
    m_bBuild = false;
    m_bArgs = false;
    m_cntxt = NULL;
    m_Program = NULL;
    m_dev = NULL;
    m_queue = NULL;
    m_kernel = NULL;
    m_sampler = NULL;
    m_buffer = NULL;
	m_image = NULL;
	m_event = NULL;
	m_ProgramBinary = NULL;
}

Arrangement::~Arrangement()
{
	if (m_queue)
		finish();
	if (m_kernel)
		clReleaseKernel(m_kernel);
	if (m_event)
		clReleaseEvent(m_event);
    if (m_Program)
        clReleaseProgram(m_Program);
    if (m_cntxt)
        clReleaseContext(m_cntxt);
    if (m_queue)
        clReleaseCommandQueue(m_queue);
    if (m_sampler)
        clReleaseSampler(m_sampler);
    if (m_buffer)
        clReleaseMemObject(m_buffer);
    if (m_image)
        clReleaseMemObject(m_image);
    if (m_ProgramBinary)
        delete m_ProgramBinary;
	Sleep(10);
}

int Arrangement::context()
{
    if (NULL == m_cntxt)
    {
        cl_int          r;
        m_cntxt = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &r);
        if (NULL == m_cntxt || CL_SUCCESS != r)
            return PROCESSED_FAIL;
    }
    return PROCESSED_OK;
}

int Arrangement::cntxt_dev()
{
    if (PROCESSED_FAIL == context())
        return PROCESSED_FAIL;
    else
    {
        cl_int          rc;
		rc = clGetContextInfo(m_cntxt, CL_CONTEXT_DEVICES, sizeof(m_dev), &m_dev, NULL);
        if (CL_SUCCESS == rc)
		{
            return PROCESSED_OK;
		}
        else
            return PROCESSED_FAIL;
    }
}

size_t Arrangement::GetBinarySize()
{
    return true == m_bBuild ? m_ProgramSize : 0;
}

void* Arrangement::GetBinary()
{
    return true == m_bBuild ? m_ProgramBinary : NULL;
}

int Arrangement::CreateProgramWithSource()
{
    if (PROCESSED_FAIL == context())
        return PROCESSED_FAIL;
    if (!m_bBuild)
    {
        m_Program = clCreateProgramWithSource(m_cntxt, count, ssProgram, NULL, NULL);
        if (NULL == m_Program)
            return PROCESSED_FAIL;
    }
    return PROCESSED_OK;
}

int Arrangement::BuildProgram()
{
    if (true == m_bBuild)
        return PROCESSED_OK;
    if (PROCESSED_OK == CreateProgramWithSource())
    {
        if (PROCESSED_OK != cntxt_dev())
            return PROCESSED_FAIL;
        cl_int          rc;
        rc = clBuildProgram(m_Program, 1, &m_dev, NULL, NULL, NULL);
        if (CL_SUCCESS != rc)
            return PROCESSED_FAIL;

		cl_build_status stat;
		rc = clGetProgramBuildInfo (m_Program, m_dev, CL_PROGRAM_BUILD_STATUS, sizeof cl_build_status, &stat, NULL);
		if (CL_SUCCESS != rc)
            return PROCESSED_FAIL;
		if (CL_BUILD_SUCCESS != stat)
			return PROCESSED_FAIL;

        rc = clGetProgramInfo(m_Program, CL_PROGRAM_BINARY_SIZES, sizeof(m_ProgramSize), &m_ProgramSize, NULL);
        if (CL_SUCCESS != rc)
            return PROCESSED_FAIL;
        m_ProgramBinary = new char[m_ProgramSize];
        rc = clGetProgramInfo(m_Program, CL_PROGRAM_BINARIES, m_ProgramSize, &m_ProgramBinary, NULL);
        if (CL_SUCCESS != rc)
            return PROCESSED_FAIL;
        m_bBuild = true;
        return PROCESSED_OK;
    }

    return PROCESSED_FAIL;
}

cl_program Arrangement::GetProgram()
{
    return m_Program;
}

cl_context Arrangement::GetContext()
{
    return m_cntxt;
}

cl_device_id Arrangement::GetDevice()
{
    return m_dev;
}

cl_command_queue Arrangement::GetQueue()
{
    return m_queue;
}

cl_kernel Arrangement::GetKernel()
{
	return m_kernel;
}

int Arrangement::Kernel()
{
	if (m_kernel)
		return PROCESSED_OK;

	if (PROCESSED_OK != BuildProgram())
		return PROCESSED_FAIL;
	m_kernel = clCreateKernel(GetProgram(), "test_kernel", NULL);
	return m_kernel ? PROCESSED_OK : PROCESSED_FAIL;
}

cl_event Arrangement::GetEvent()
{
	return m_event;
}

int Arrangement::Event()
{
	if (m_event)
		return PROCESSED_OK;

	if (PROCESSED_OK != Queue())
		return PROCESSED_FAIL;
	if (PROCESSED_OK != KernelReady())
		return PROCESSED_FAIL;
	if (CL_SUCCESS != clEnqueueTask(GetQueue(), GetKernel(), 0, NULL, &m_event))
		return PROCESSED_FAIL;
	return m_event ? PROCESSED_OK : PROCESSED_FAIL;
}

int Arrangement::KernelReady()
{
    if (m_bArgs)
        return PROCESSED_OK;

    if (NULL == m_kernel)
        if (PROCESSED_FAIL == Kernel())
            return PROCESSED_FAIL;

    cl_int          rc;
    int             arg = 0;
    rc = clSetKernelArg(m_kernel, 0, sizeof(int), &arg);

    if (CL_SUCCESS != rc)
        return PROCESSED_FAIL;
    else
    {
        m_bArgs = true;
        return PROCESSED_OK;
    }
}

cl_sampler Arrangement::GetSampler()
{
    return m_sampler;
}

int Arrangement::Sampler()
{
    if (m_sampler)
        return PROCESSED_OK;

    if (PROCESSED_OK != context())
        return PROCESSED_FAIL;
    m_sampler = clCreateSampler(m_cntxt, CL_TRUE, CL_ADDRESS_REPEAT, CL_FILTER_NEAREST, NULL);
    return m_sampler ? PROCESSED_OK : PROCESSED_FAIL;
}

cl_mem Arrangement::GetBuffer()
{
    return m_buffer;
}

int Arrangement::Buffer(size_t size)
{
    if (m_buffer)
        return PROCESSED_OK;

    if (PROCESSED_OK != context())
        return PROCESSED_FAIL;
    m_buffer = clCreateBuffer(m_cntxt, CL_MEM_ALLOC_HOST_PTR, size, NULL, NULL);
    return m_buffer ? PROCESSED_OK : PROCESSED_FAIL;
}

cl_mem Arrangement::GetImage()
{
    return m_image;
}

int Arrangement::Image2D()
{
    if (m_image)
        return PROCESSED_OK;

    if (PROCESSED_OK != context())
        return PROCESSED_FAIL;
    cl_image_format     fmt = {CL_BGRA, CL_UNORM_INT8};
    char                picture[32];
    m_image = clCreateImage2D(m_cntxt, CL_MEM_USE_HOST_PTR, &fmt, 2, 2, 0, picture, NULL);

    return m_image ? PROCESSED_OK : PROCESSED_FAIL;
}

int Arrangement::Image3D()
{
    if (m_image)
        return PROCESSED_OK;

    if (PROCESSED_OK != context())
        return PROCESSED_FAIL;
    cl_image_format     fmt = {CL_BGRA, CL_UNORM_INT8};
    char                picture[32];
    m_image = clCreateImage3D(m_cntxt, CL_MEM_USE_HOST_PTR, &fmt, 2, 2, 2, 0, 0, picture, NULL);

    return m_image ? PROCESSED_OK : PROCESSED_FAIL;
}

int Arrangement::Queue(cl_command_queue_properties properties)
{
    if (m_queue)
        return PROCESSED_OK;

    if (PROCESSED_OK != cntxt_dev())
        return PROCESSED_FAIL;
    m_queue = clCreateCommandQueue(m_cntxt, m_dev, properties, NULL);
    return m_queue ? PROCESSED_OK : PROCESSED_FAIL;
}
int Arrangement::flush()
{
	return clFlush( m_queue );
}

int Arrangement::finish()
{
	cl_int err = clFlush( m_queue );
	if ( CL_SUCCESS != err )
	{
		return err;
	}
	return clFinish( m_queue );
}