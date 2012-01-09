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

#pragma once
#include "CL\cl.h"
#include <cl_device_api.h>

class Arrangement
{
private:
    bool                m_bBuild;
    bool                m_bArgs;
    cl_program          m_Program;
    size_t              m_ProgramSize;
    void*               m_ProgramBinary;
    cl_mem              m_buffer;
    cl_mem              m_image;
    cl_context          m_cntxt;
    cl_device_id        m_dev;
    cl_command_queue    m_queue;
	cl_kernel           m_kernel;
	cl_event			m_event;
    cl_sampler          m_sampler;

public:
                        Arrangement();
                        ~Arrangement();

    cl_program          GetProgram();
    void*               GetBinary();
    size_t              GetBinarySize();
    int                 CreateProgramWithSource();
//    int                CreateProgramWithBinary();
    int                 BuildProgram();

    cl_context          GetContext();
    int                 context();

    cl_device_id        GetDevice();
    int                 cntxt_dev();

    cl_command_queue    GetQueue();
    int                 Queue(cl_command_queue_properties properties = 0);

    cl_kernel           GetKernel();
    int                 Kernel();
    int                 KernelReady();

    cl_sampler          GetSampler();
    int                 Sampler();

    cl_mem              GetImage();
    int                 Image2D();
    int                 Image3D();

	cl_mem              GetBuffer();
	int                 Buffer(size_t size);

	cl_event            GetEvent();
	int                 Event();

	int					flush();
	int					finish();
};

// The following list of stand-alone functions
int     context(cl_context* cntxt);
void    context_cleanup(const cl_context cntxt);
void    cntxt_mem_cleanup(const cl_context cntxt, const cl_mem mem);
int     cntxt_dev(cl_context* cntxt, cl_device_id* dev);
int     queue(cl_context* cntxt, cl_device_id* dev, cl_command_queue* queue);
void    queue_cleanup(const cl_context cntxt, const cl_command_queue queue);
void    prog_cleanup(const cl_context cntxt, const cl_program prog);
int     prog_create(cl_context* cntxt, cl_program* prog);
int     prog_build(cl_context* cntxt, cl_program* prog);
void    kernel_cleanup(const cl_context cntxt, const cl_program prog, const cl_kernel kernel);
int     kernel(cl_context* cntxt, cl_program* prog, cl_kernel* kernel);
