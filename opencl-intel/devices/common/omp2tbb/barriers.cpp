#include "stdafx.h"

#include <tbb/tbb_thread.h>

#include "libomp2tbb.h"

extern "C" int omp_get_thread_num();

extern "C" LIBOMP2TBB_API void __kmpc_critical (ident_t* loc, kmp_int32 global_tid, kmp_critical_name* crit)
{
	omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );

	entry->my_critical->lock();
}

extern "C" LIBOMP2TBB_API void __kmpc_end_critical (ident_t* loc, kmp_int32 global_tid, kmp_critical_name* crit)
{
	omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );

	entry->my_critical->unlock();
}

extern "C" LIBOMP2TBB_API kmp_int32 __kmpc_single (ident_t* loc, kmp_int32 global_tid)
{
	omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );
	ident_t* old_loc = NULL;
	long prev = -1;

	// We assume implicit barrier after "omp single". This means that all threads will go through this point before reaching the barrier
	old_loc = entry->my_last_single_location.fetch_and_store(loc);
	prev = entry->my_single_count.fetch_and_increment();
	
	// When all threads reached this reageon we can reset appearance
	if ( prev == entry->my_concurency ) {
		entry->my_last_single_location = NULL;
	}

	// Return true only if previous location is difference then current
	return old_loc != loc;
}

extern "C" LIBOMP2TBB_API void __kmpc_end_single (ident_t* loc, kmp_int32 global_tid)
{
	// nothing to do here
	return;
}

extern "C" LIBOMP2TBB_API void __kmpc_barrier (ident_t* loc, kmp_int32 global_tid)
{
	omp2tbb::global_thread_table::thread_info_entry* entry = omp2tbb::g_thread_info->get_master_entry( global_tid );

	// Add efficient barrier implementation, currently we just do pooling

	const long epoch = entry->my_barrier_epoch;

	// First thread that reaches this point should set the value
	entry->my_barrier_count.compare_and_swap(entry->my_concurency, 0);
	long prev = entry->my_barrier_count.fetch_and_decrement();
	if ( 1 == prev ) {
		// last worker reach arena, need to change epoch
		// Single thread does this operation, no need in atomic
		++(entry->my_barrier_epoch);
	}

	while ( (0 != entry->my_barrier_count) && ( epoch == entry->my_barrier_epoch) )	{
		tbb::this_tbb_thread::yield();
	}
}


extern "C" LIBOMP2TBB_API kmp_int32 __kmpc_master (ident_t* loc, kmp_int32 global_tid)
{
	return 0 == omp_get_thread_num();
}

extern "C" LIBOMP2TBB_API void __kmpc_end_master (ident_t* loc, kmp_int32 global_tid)
{
	// nothing to do here
	return;
}
