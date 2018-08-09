// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "task_executor.h"

#if ! defined( __THREAD_EXECUTOR__) && ! defined( __TBB_EXECUTOR__ )
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
