// Copyright (c) 2006-2009 Intel Corporation
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
#include "task_executor.h"

#if ! defined( __THREAD_EXECUTOR__) && ! defined( __TBB_EXECUTOR__ ) && ! defined( __XN_EXECUTOR__ )
#define __THREAD_EXECUTOR__
#endif

#ifdef __TBB_EXECUTOR__
#include "tbb_executor.h"
#define PTR_CAST	TBBTaskExecutor
#endif
#ifdef __THREAD_EXECUTOR__
#include "thread_executor.h"
#define PTR_CAST	ThreadTaskExecutor
#endif
#ifdef __XN_EXECUTOR__
#include "xn_executor.h"
#define PTR_CAST	XNTaskExecutor
#endif

#include <stdio.h>

using namespace Intel::OpenCL::TaskExecutor;

namespace Intel { namespace OpenCL { namespace TaskExecutor {
void __attribute__ ((constructor)) dll_init(void);
void __attribute__ ((destructor)) dll_fini(void);
}}}

ITaskExecutor* g_pTaskExecutor = NULL;

void Intel::OpenCL::TaskExecutor::dll_init(void)
{
#ifdef __TBB_EXECUTOR__
	g_pTaskExecutor = new TBBTaskExecutor;
#endif
#ifdef __THREAD_EXECUTOR__
	g_pTaskExecutor = new ThreadTaskExecutor;
#endif
#ifdef __XN_EXECUTOR__
	g_pTaskExecutor = new XNTaskExecutor;
#endif
}

void Intel::OpenCL::TaskExecutor::dll_fini(void)
{
	if (g_pTaskExecutor)
	{
		delete ((PTR_CAST*)g_pTaskExecutor);
		g_pTaskExecutor = NULL;
	}
}

TASK_EXECUTOR_API ITaskExecutor* Intel::OpenCL::TaskExecutor::GetTaskExecutor()
{
	return g_pTaskExecutor;
}

TASK_EXECUTOR_API IThreadPoolPartitioner* Intel::OpenCL::TaskExecutor::CreateThreadPartitioner(size_t numThreads)
{
    //Todo: implement for non-TBB
#ifdef __TBB_EXECUTOR__
    return new TBBThreadPoolPartitioner(numThreads);
#else
    return NULL;
#endif
}
