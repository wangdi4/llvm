/*
    Copyright 2005-2013 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#ifndef __TBB_parallel_for_opencl_H
#define __TBB_parallel_for_opencl_H

#ifndef __TBB_STATIC_THRESHOLD
#define __TBB_STATIC_THRESHOLD 40000
#endif

#ifndef __TBB_OCL_INIT_DEPTH
#define __TBB_OCL_INIT_DEPTH 6
#endif

#include <algorithm>    // std::max
#include <new>

#include <uneven/parallel_for.h>

#include <stdio.h>
#include <tbb/task_arena.h>

#include <stdio.h>

namespace tbb {

namespace interfaceOCL {
using namespace tbb::uneven::interface6;

namespace internal {
    class opencl_partition_type;
} // namespace internal

//! An affinity partitioner
class opencl_partitioner {
public:
    opencl_partitioner(int master_slot = 0, int concurrency = 0, bool use_zero_slot = true, int num_cores=0, int thread_per_core=0) :
      my_master_slot(master_slot), my_concurrency(concurrency), my_use_zero_slot(use_zero_slot),
      my_num_cores(num_cores), my_thread_per_core(thread_per_core) {
          my_active_slots = tbb::internal::get_initial_auto_partitioner_divisor() / 4; // let exactly P tasks to be distributed across workers
          if ( 0 == my_concurrency ) {
              my_concurrency = my_active_slots;
          }
#if 0
          printf("MasterSlot=%d,my_concurrency=%d,zero_slot=%d, active_slots=%d\n", master_slot, concurrency, use_zero_slot,my_active_slots);fflush(0);
#endif
      }

private:
    int my_master_slot;
    int my_concurrency;
    int my_active_slots;
    bool my_use_zero_slot;
    int my_num_cores;
    int my_thread_per_core;

    friend class internal::opencl_partition_type;
//    template<typename Range, typename Body, typename Partitioner> friend class serial::interface6::start_for;
    template<typename Range, typename Body, typename Partitioner> friend class uneven::interface6::internal::start_for;
    template<typename Range, typename Body, typename Partitioner> friend class uneven::interface6::internal::start_reduce;
    template<typename Range, typename Body, typename Partitioner> friend class tbb::uneven::internal::start_scan;
    // backward compatibility - for parallel_scan only
    typedef uneven::interface6::internal::old_auto_partition_type partition_type;
    // new implementation just extends existing interface
    typedef interfaceOCL::internal::opencl_partition_type task_partition_type;
};


//! @cond INTERNAL
namespace internal {
using namespace tbb::uneven::interface6::internal;

#ifdef __MIC__
static inline unsigned long long get_tsc_mark() {
    unsigned int a, d;
    __asm__ __volatile__("rdtsc" : "=a" (a), "=d" (d));
    return (((unsigned long long)d)<<32) + a;
}
#else
#ifdef WIN32
static inline unsigned long long get_tsc_mark() {
	return __rdtsc();
}
#else
static inline unsigned long long  get_tsc_mark() {
    unsigned int a, d;
    __asm__ __volatile__("lfence\n"
                         "rdtsc" : "=a" (a), "=d" (d));
    return (((unsigned long long)d)<<32) + a;
}
#endif
#endif


static inline bool is_big_enough(unsigned long long start) {
    return get_tsc_mark() - start > __TBB_STATIC_THRESHOLD;
}

//! Provides default methods for affinity (adaptive) partition objects.
class opencl_partition_type : public tbb::uneven::interface6::internal::partition_type_base<opencl_partition_type> {
    const interfaceOCL::opencl_partitioner& my_part;
    unsigned my_begin, my_end;
    depth_t my_max_depth;
    char my_delay;
    unsigned long long my_tsc;
public:
    opencl_partition_type( const interfaceOCL::opencl_partitioner& part) : my_part(part){
        my_begin = 0;
        my_end = my_part.my_concurrency;
        my_max_depth = __TBB_OCL_INIT_DEPTH;
        my_delay = 0;
        my_tsc = 0;
        __TBB_ASSERT(my_end, "initial value of get_initial_auto_partitioner_divisor() is not valid");
        __TBB_ASSERT( my_max_depth < range_pool_size, 0 );
    }
    opencl_partition_type( opencl_partition_type& p, uneven::split ) : my_part(p.my_part){
        using namespace std;
        my_max_depth = p.my_max_depth;
        my_delay = p.my_delay;
        my_tsc = 0;
        const int right_part = (p.my_begin+1 < p.my_end)? max(1U, (p.my_end-p.my_begin+2)/3) : 0;
        my_end = p.my_end;
        my_begin = p.my_end = p.my_end - right_part;
    }
    int portion_size() { return my_end-my_begin; } // uneven splits are not used for portion size = 0
    bool divisions_left() { // part of old should_execute_range()
        return my_begin+1 < my_end;
    }
    bool is_divisible() { return divisions_left(); }
    void set_affinity( task &t ) {
        if( my_begin < my_end ) {
            __TBB_ASSERT(my_begin > 0, "Task with my_begin==0 is not expected");
            int affinity = my_begin;
            if ( !my_part.my_use_zero_slot ) {
                // if slot 0, master is not in use, we should calculate affinity relatively to current master slot
                affinity = my_part.my_master_slot + my_begin;
                if ( affinity > my_part.my_concurrency ) {
                    affinity -= my_part.my_concurrency;
                    __TBB_ASSERT( affinity > 0,  "Task affinity expected to be  greater than 0");
                }
            }
#if __ENABLE_SCATTER_DISTRIBUTION__
            // if number of cores was specified and concurrency is less than twice number of cores,
            // need to scatter threads over cores
            if ( (my_part.my_num_cores > 0) && (my_part.my_concurrency < (2*my_part.my_num_cores)) )
            {
                int hw_tid = (long)affinity / my_part.my_num_cores;
                int core_id = (long)affinity % my_part.my_num_cores;
                affinity = core_id * my_part.my_thread_per_core + hw_tid;
            }
#endif
#if 0
            printf("master slot %d slot %d setting affinity to %d concurrency=%d, my_begin=%d, my_end=%d\n", my_part.my_master_slot, tbb::task_arena::current_slot(), affinity, my_part.my_concurrency, my_begin, my_end);fflush(0);
#endif
            t.set_affinity( affinity + 1);
        }
    }
    bool check_being_stolen( task &t) { // part of old should_execute_range()
        if( my_begin == my_end ) { // if not from the top P tasks of binary tree
            my_end++; // TODO: replace by on-stack flag (partition_state's member)?
            if( t.is_stolen_task() ) {
#if TBB_USE_EXCEPTIONS
                // RTTI is available, check whether the cast is valid
                __TBB_ASSERT(dynamic_cast<flag_task*>(t.parent()), 0);
                // correctness of the cast relies on avoiding the root task for which:
                // - initial value of my_divisor != 0 (protected by separate assertion)
                // - is_stolen_task() always returns false for the root task.
#endif
                flag_task::mark_task_stolen(t);
                if(!my_max_depth) my_max_depth++; // should be able to divide at least to quarters
                my_max_depth+=2;
                return true;
            }
        }
        return false;
    }
    bool check_for_demand( task &t ) {
        if( my_delay == 0 ) {
            my_delay = 1;
            my_tsc = get_tsc_mark();
        } else {
            if( my_delay == 1 ) {
                if( !is_big_enough(my_tsc) ) {
                    __TBB_ASSERT(my_max_depth, "unbelievable");
                    my_max_depth--;
                    return false;
                }
                my_delay++;
            }
            if( my_begin < my_end && my_max_depth) {
                my_end = my_begin; // once per task (recovered in check_being_stolen)
                return true;// do not do my_max_depth++ here, but be sure my_max_depth is big enough
            }
            if( flag_task::is_peer_stolen(t) ) {
                my_max_depth++;
                return true;
            }
        }
        return false;
    }
    bool should_create_trap() {
        return false;
    }
    void align_depth(depth_t base) {
        __TBB_ASSERT(base <= my_max_depth, 0);
        my_max_depth -= base;
    }
    depth_t max_depth() { return my_max_depth; }
    static const unsigned range_pool_size = 8;
};

} // namespace internal
//! @endcond

} // namespace interfaceX
using interfaceOCL::opencl_partitioner;

// Requirements on Range concept are documented in blocked_range.h

//! Parallel iteration over range with simple partitioner.
/** @ingroup algorithms **/
namespace uneven {
template<typename Range, typename Body>
void parallel_for( const Range& range, const Body& body, const opencl_partitioner& partitioner ) {
    internal::start_for<Range,Body,const opencl_partitioner>::run(range,body,partitioner);
}

#if __TBB_TASK_GROUP_CONTEXT
//! Parallel iteration over range with simple partitioner and user-supplied context.
/** @ingroup algorithms **/
template<typename Range, typename Body>
void parallel_for( const Range& range, const Body& body, const opencl_partitioner& partitioner, task_group_context& context ) {
    uneven::internal::start_for<Range,Body,const opencl_partitioner>::run(range, body, partitioner, context);
}
#endif /* __TBB_TASK_GROUP_CONTEXT */
//@}
}

namespace uneven {

namespace strict_ppl {

//@{
#if 0
//! Implementation of parallel iteration over stepped range of integers with explicit step and partitioner
template <typename Index, typename Function>
void parallel_for_oclimpl(Index first, Index last, Index step, const Function& f, const opencl_partitioner& partitioner) {
    if (step <= 0 )
        internal::throw_exception(internal::eid_nonpositive_step); // throws std::invalid_argument
    else if (last > first) {
        // Above "else" avoids "potential divide by zero" warning on some platforms
        Index end = (last - first - Index(1)) / step + Index(1);
        tbb::ocl_blocked_range<Index> range(static_cast<Index>(0), end);
        internal::parallel_for_body<Function, Index> body(f, first, step);
        tbb::parallel_for(range, body, partitioner);
    }
}
#endif
//! Parallel iteration over a range of integers with a step provided and simple partitioner
template <typename Index, typename Function>
void parallel_for(Index first, Index last, Index step, const Function& f, const opencl_partitioner& partitioner) {
    parallel_for_impl<Index,Function,const opencl_partitioner>(first, last, step, f, partitioner);
}
//! Parallel iteration over a range of integers with a default step value and simple partitioner
template <typename Index, typename Function>
void parallel_for(Index first, Index last, const Function& f, const opencl_partitioner& partitioner) {
    parallel_for_impl<Index,Function,const opencl_partitioner>(first, last, static_cast<Index>(1), f, partitioner);
}

#if __TBB_TASK_GROUP_CONTEXT
#if 0
//! Implementation of parallel iteration over stepped range of integers with explicit step, task group context, and partitioner
template <typename Index, typename Function>
void parallel_for_oclimpl(Index first, Index last, Index step, const Function& f, const opencl_partitioner& partitioner, tbb::task_group_context &context) {
    if (step <= 0 )
        internal::throw_exception(internal::eid_nonpositive_step); // throws std::invalid_argument
    else if (last > first) {
        // Above "else" avoids "potential divide by zero" warning on some platforms
        Index end = (last - first - Index(1)) / step + Index(1);
        tbb::ocl_blocked_range<Index> range(static_cast<Index>(0), end);
        internal::parallel_for_body<Function, Index> body(f, first, step);
        tbb::parallel_for(range, body, partitioner, context);
    }
}
#endif
//! Parallel iteration over a range of integers with explicit step, task group context, and simple partitioner
 template <typename Index, typename Function>
void parallel_for(Index first, Index last, Index step, const Function& f, const opencl_partitioner& partitioner, tbb::task_group_context &context) {
    parallel_for_impl<Index,Function,const opencl_partitioner>(first, last, step, f, partitioner, context);
}
//! Parallel iteration over a range of integers with a default step value, explicit task group context, and simple partitioner
 template <typename Index, typename Function, typename Partitioner>
void parallel_for(Index first, Index last, const Function& f, const opencl_partitioner& partitioner, tbb::task_group_context &context) {
    parallel_for_impl<Index,Function,const opencl_partitioner>(first, last, static_cast<Index>(1), f, partitioner, context);
}
#endif /* __TBB_TASK_GROUP_CONTEXT */
//@}
} // namespace strict_ppl
} // namespace uneven

} // namespace tbb

#endif /* __TBB_parallel_for_opencl_H */

