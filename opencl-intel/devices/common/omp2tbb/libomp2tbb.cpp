// libomp2tbb.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "libomp2tbb.h"
#include <stdio.h>
#include <assert.h>

#include <tbb/task_scheduler_init.h>

omp2tbb::global_thread_table* omp2tbb::g_thread_info = NULL;

THREAD_LOCAL int t_max_concurency = -1;

kmp_int32 omp2tbb::global_thread_table::allocate_master_thread()
{
	tbb::critical_section::scoped_lock lock(masters_guard);

	// Critical scode
	size_t master_count = masters.size();
	masters.resize(master_count + 1);


	return (kmp_int32)master_count;
}

omp2tbb::global_thread_table::thread_info_entry::thread_info_entry() :
	 my_concurency(0), my_barrier_epoch(0)
{
	my_last_single_location = NULL;
	my_single_count = 0;
	my_critical = new tbb::critical_section();
	my_barrier_count = 0;
}

omp2tbb::global_thread_table::thread_info_entry::thread_info_entry(const omp2tbb::global_thread_table::thread_info_entry& o)
{
	my_last_single_location = NULL;
	my_single_count = 0;
	my_critical = new tbb::critical_section();
	my_barrier_count = 0;
}

omp2tbb::global_thread_table::thread_info_entry::~thread_info_entry()
{
	delete my_critical;
}

extern "C" LIBOMP2TBB_API void __omp2tbb_set_thread_max_concurency(int max_concurency)
{
	t_max_concurency = max_concurency;
}

// External functions
extern "C" LIBOMP2TBB_API void __kmpc_begin(ident_t* loc, kmp_int32 flags)
{
	omp2tbb::g_thread_info = new omp2tbb::global_thread_table();
}

extern "C" LIBOMP2TBB_API void __kmpc_end(ident_t* loc)
{
	delete omp2tbb::g_thread_info;
}

extern "C" LIBOMP2TBB_API kmp_int32 __kmpc_ok_to_fork(ident_t* loc)
{
	return 1;
}

// NOT described in reference doc
extern "C" LIBOMP2TBB_API int omp_get_num_procs()
{
	return tbb::task_scheduler_init::default_num_threads();
}


extern "C" LIBOMP2TBB_API int omp_get_num_threads()
{
	return omp2tbb::get_current_team_concurency();
}

extern "C" LIBOMP2TBB_API int omp_get_max_threads()
{
	if ( -1 == t_max_concurency ) {
		t_max_concurency = tbb::task_scheduler_init::default_num_threads();
	}

	return t_max_concurency;
}

extern "C" LIBOMP2TBB_API int omp_in_parallel()
{
	return 0 == omp2tbb::get_current_team_concurency();
}
//////////////////////////////////////

// Function that currently are not used, define just symbols in Linux
#ifndef WIN32
#define DECLARE_UNUSED_FUNCTION(func_name) extern "C" LIBOMP2TBB_API int func_name(void) { assert(0&&"Undefined function used"); return 0;}

DECLARE_UNUSED_FUNCTION(__kmpc_ordered)
DECLARE_UNUSED_FUNCTION(__kmpc_dispatch_next_4)
DECLARE_UNUSED_FUNCTION(__kmpc_end_reduce_nowait)
DECLARE_UNUSED_FUNCTION(__kmpc_dispatch_fini_8)
DECLARE_UNUSED_FUNCTION(__kmpc_atomic_cmplx8_add)
DECLARE_UNUSED_FUNCTION(__kmpc_atomic_float4_add)
DECLARE_UNUSED_FUNCTION(__kmpc_dispatch_init_8)
DECLARE_UNUSED_FUNCTION(ompc_set_nested)
DECLARE_UNUSED_FUNCTION(__kmpc_for_static_init_8u)
DECLARE_UNUSED_FUNCTION(omp_get_nested)
DECLARE_UNUSED_FUNCTION(__kmpc_dispatch_fini_4)
DECLARE_UNUSED_FUNCTION(__kmpc_atomic_float8_max)
DECLARE_UNUSED_FUNCTION(__kmpc_reduce_nowait)
DECLARE_UNUSED_FUNCTION(__kmpc_for_static_init_4)
DECLARE_UNUSED_FUNCTION(__kmpc_flush)
DECLARE_UNUSED_FUNCTION(__kmpc_dispatch_next_8)
DECLARE_UNUSED_FUNCTION(__kmpc_atomic_float8_add)
DECLARE_UNUSED_FUNCTION(__kmpc_atomic_float4_max)
DECLARE_UNUSED_FUNCTION(__kmpc_dispatch_init_4)
DECLARE_UNUSED_FUNCTION(__kmpc_end_ordered)
DECLARE_UNUSED_FUNCTION(__kmpc_atomic_fixed8_add)
DECLARE_UNUSED_FUNCTION(__kmpc_atomic_cmplx4_add)

#endif
