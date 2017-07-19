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

#if ! defined( __THREAD_EXECUTOR__) && ! defined( __TBB_EXECUTOR__ ) && ! defined( __OMP_EXECUTOR__ )
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
#ifdef __OMP_EXECUTOR__
#include "omp_executor.h"
#define PTR_CAST	OMPTaskExecutor
#endif

#include "cl_shared_ptr.h"
#include "cl_shared_ptr.hpp"
#include <stdio.h>
#include <pthread.h>

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

#ifdef __TBB_EXECUTOR__
template class Intel::OpenCL::Utils::SharedPtrBase<Intel::OpenCL::TaskExecutor::SyncTask>;
#endif
template class Intel::OpenCL::Utils::SharedPtrBase<Intel::OpenCL::TaskExecutor::ITaskBase>;

namespace Intel { namespace OpenCL { namespace TaskExecutor {
void __attribute__ ((constructor)) dll_init(void);
void __attribute__ ((destructor)) dll_fini(void);

ITaskExecutor* g_pTaskExecutor = nullptr;
pthread_key_t thkShedMaster;

static void thread_cleanup_callback(void* _NULL)
{
}

void dll_init(void)
{
	thkShedMaster = 0;
	pthread_key_create(&thkShedMaster, thread_cleanup_callback);

#ifdef _DEBUG  // this is needed to initialize allocated objects DB, which is maintained in only in debug
     InitSharedPtrs();
#endif

#ifdef __TBB_EXECUTOR__
	g_pTaskExecutor = new TBBTaskExecutor;
#endif
#ifdef __THREAD_EXECUTOR__
	g_pTaskExecutor = new ThreadTaskExecutor;
#endif
#ifdef __OMP_EXECUTOR__
	g_pTaskExecutor = new OMPTaskExecutor;
#endif
}

void dll_fini(void)
{
	if ( nullptr != g_pTaskExecutor)
	{
		delete ((PTR_CAST*)g_pTaskExecutor);
		g_pTaskExecutor = nullptr;
	}
	if ( thkShedMaster )
	{
		pthread_key_delete(thkShedMaster);
        thkShedMaster = 0;  
	}

#ifdef _DEBUG
    FiniSharedPts();
#endif
}

TASK_EXECUTOR_API ITaskExecutor* GetTaskExecutor()
{
	return g_pTaskExecutor;
}

void RegisterReleaseSchedulerForMasterThread()
{
	pthread_setspecific(thkShedMaster, nullptr);
}

}}}
