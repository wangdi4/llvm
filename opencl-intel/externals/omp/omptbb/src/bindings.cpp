#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <numeric>

#include "tbb/cache_aligned_allocator.h"
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"

#include "bindings.h"

namespace internal {
    // Historical reason to have 128 byte cache line (it is current behavior of the Intel OpenMP rutime).
    static const size_t CACHE_LINE = 128;

    // Aligns the 'size' in accordance with the 'alignment'.
    size_t align_up(size_t size, size_t alignment) {
        __TBB_ASSERT(tbb::internal::is_power_of_two(alignment), "The aligment is not a power of 2");
        return (size + alignment - 1) & (~(alignment - 1));
    }

    // The prefix used to store the kmp task size. The compiler provides only the task pointer and
    // to duplicate the task we need to know its size.
    struct kmp_task_prefix {
        size_t kmp_task_size;
        static size_t size;
    };
    size_t kmp_task_prefix::size = align_up(sizeof(internal::kmp_task_prefix), sizeof(void*));

    // Forward declaration of taskloop body.
    class loop_body;

    // Per-thread data structure 
    struct reduce_data_t {
        typedef std::vector<kmp_task_red_input, tbb::cache_aligned_allocator<kmp_task_red_input>> reductions_t;
        // Reductions are used to propagate the data from __kmpc_task_reduction_init to __kmpc_taskloop.
        reductions_t reductions;
        // The current body that is executed by the thread. It used by __kmpc_task_reduction_get_th_data
        // to resolve global variable address into private one.
        const loop_body* innermost_loop_body = nullptr;
    };
    static thread_local reduce_data_t reduce_data;

    // The taskloop body that implements almost all logic.
    class loop_body : tbb::internal::no_copy {
        // The list of reductions that should be processed.
        const reduce_data_t::reductions_t& reductions;
        // The same as kmp_task_prefix::kmp_task_size.
        size_t kmp_task_size;
        // The total size of additional memory.
        size_t total_size;
        // The additional memory used to store the kmp task and private variables used for reduction.
        // The layout is [| kmp task | reducing variables |]
        void* data;

        // The offset of lower bound in kmp task.
        size_t lb_off;
        // The offset of upper bound in kmp task.
        size_t ub_off;

        // Calculates the offset of the next private variable.
        // The idea here is that Intel OpenMP runtimes aligns private data by its size but it is
        // limited by the size of a cache line.
        static size_t next_reduction(size_t offset, size_t size) {
            return align_up(offset, std::min(size, CACHE_LINE));
        }

        // Combines two variables by calling compiler provided function.
        static void combine(const kmp_task_red_input& red, void* l, void* r) {
            void(*reduce_comb)(void*, void*) = (void(*)(void*, void*))(red.reduce_comb);
            __TBB_ASSERT_RELEASE(reduce_comb, "The combine routine is not provided");
            reduce_comb(l, r);
        }

        // Helper that applies a functor to each reducing variable.
        // The functor is provided with a reduction and offset inside 'data'.
        template <typename F>
        void for_each_reduction(F f) const {
            size_t offset = kmp_task_size;
            for (const kmp_task_red_input& red : reductions) {
                offset = next_reduction(offset, red.reduce_size);
                __TBB_ASSERT(offset < total_size, NULL);
                f(red, offset);
                offset += red.reduce_size;
            }
        }

        // Copies the additional data that includes kmp task and reducing variables.
        void* copy_data(const void* src) const {
            void* d = tbb::internal::NFS_Allocate(1, total_size, nullptr);
            __TBB_ASSERT_RELEASE(d, "Failed to allocate data");
            // Copy task data
            std::memcpy(d, src, kmp_task_size);
            // Initialize reductions
            for_each_reduction([this,d](const kmp_task_red_input& red, size_t offset) {
                if (void(*reduce_init)(void*) = (void(*)(void*))(red.reduce_init))
                    reduce_init((char*)d + offset);
                else
                    // Compiler expects zero initialization.
                    std::memset((char*)d + offset, 0, red.reduce_size);
            });
            return d;
        }
    public:
        loop_body(const reduce_data_t::reductions_t& reds, const kmp_task_t* t, size_t ts, size_t lb, size_t ub)
            : reductions(reds)
            , kmp_task_size(ts)
            // Total size is sum of kmp task size of the size of memory required for reducing variables.
            , total_size(std::accumulate(reds.cbegin(), reds.cend(), kmp_task_size,
                [](size_t sum, const kmp_task_red_input& in) { return next_reduction(sum, in.reduce_size) + in.reduce_size; }))
            , data(copy_data(t))
            , lb_off(lb)
            , ub_off(ub)
        {}
        loop_body(const loop_body &lb, tbb::split)
            : reductions(lb.reductions)
            , kmp_task_size(lb.kmp_task_size)
            , total_size(lb.total_size)
            , data(copy_data(lb.data))
            , lb_off(lb.lb_off)
            , ub_off(lb.ub_off)
        {}
        ~loop_body() {
            // Destroy reductions
            for_each_reduction([this](const kmp_task_red_input& red, size_t offset) {
                if (void(*reduce_fini)(void*) = (void(*)(void*))(red.reduce_fini))
                    reduce_fini((char*)data + offset);
            });
            tbb::internal::NFS_Free(data);
        }
        // The main function called by tbb::parallel_reduce to process a particular range.
        void operator()(tbb::blocked_range<size_t> r) const {
            kmp_task_t* task = (kmp_task_t*)data;
            // Specify the task range. Currently, the same task is used; however, it can be
            // incorrect if task_dup is not NULL.
            *(size_t*)((char*)task + lb_off) = r.begin();
            *(size_t*)((char*)task + ub_off) = r.end() - 1;
            // Specify the current reduction body if we are in reduction taskloop.
            const loop_body* dispatch_loop_body = nullptr;
            if (!reductions.empty()) {
                dispatch_loop_body = reduce_data.innermost_loop_body;
                reduce_data.innermost_loop_body = this;
            }
            // The first argument is a thread number. Currently, it is not supported and always set to 0.
            task->routine(0, task);
            // Restore the current reduction body if we are in reduction taskloop.
            if (!reductions.empty())
                reduce_data.innermost_loop_body = dispatch_loop_body;
        }
        // The join function called by tbb::parallel_reduce to join two bodies.
        void join(const loop_body& lb) const {
            // Combine two sets of reductions
            for_each_reduction([this, &lb](const kmp_task_red_input& red, size_t offset) {
                combine(red, (char*)data + offset, (char*)lb.data + offset);
            });
        }
        // Similar to the join function but the result is combined into shared variables provied by compiler.
        void join_to_shared() {
            for_each_reduction([this](const kmp_task_red_input& red, size_t offset) {
                combine(red, red.reduce_shar, (char*)data + offset);
            });
        }
        // Implements the logic of __kmpc_task_reduction_get_th_data. It finds the required address in
        // the list of reductions and returns the private representative.
        void* resolve_addr(void* shared) const {
            // Resolve a shared address into a local one.
            size_t offset = kmp_task_size;
            for (const kmp_task_red_input& red : reductions) {
                offset = next_reduction(offset, red.reduce_size);
                __TBB_ASSERT(offset < total_size, NULL);
                if (red.reduce_shar == shared)
                    return (char*)data + offset;
                offset += red.reduce_size;
            }
            return nullptr;
        }
    };
}

extern "C" {
#if USE_OMPTBB_EXCLUSIVELY
    // Optional. It can be safely ignored.
    void __kmpc_begin(ident_t *, kmp_int32) {}
    // Optional. It can be safely ignored.
    void __kmpc_end(ident_t *) {}

    // In OpenMP, the task group functionality is synchronization construct; however, in the
    // current implementation, taskloop is used for synchronization (it is blocked until work is
    // done). Therefore, task group functionality can be safely ignored.
    void __kmpc_taskgroup(ident_t *, int gtid) {
        //JJJ __TBB_ASSERT_RELEASE(gtid == 0, "Unknown gtid");
    }
    void __kmpc_end_taskgroup(ident_t *, int gtid) {
        //JJJ __TBB_ASSERT_RELEASE(gtid == 0, "Unknown gtid");
    }

    // The thread number functionality is not supported and 0 is always returned.
    kmp_int32  __kmpc_global_thread_num(ident_t *) {
        return 0;
    }
#endif

    // Allocates a kmp task. The behavior is similar to Intel OpenMP runtime.
#if USE_OMPTBB_EXCLUSIVELY
    kmp_task_t* __kmpc_omp_task_alloc
#else
    kmp_task_t* __tbb_omp_task_alloc
#endif
    (   ident_t *,
        kmp_int32 gtid,
        kmp_int32 flags,
        size_t sizeof_kmp_task_t,
        size_t sizeof_shareds,
        kmp_routine_entry_t task_entry)
    {
        //JJJ __TBB_ASSERT_RELEASE(gtid == 0, "Unknown gtid");
        __TBB_ASSERT_RELEASE(flags == 1, "Unsupported construction");

        // Allocate memory for the task, shared variable and prefix.
        // The memory layout is [| kmp_task_prefix | kmp_task_t | shared variables |]
        internal::kmp_task_prefix* prefix = (internal::kmp_task_prefix*)tbb::internal::NFS_Allocate(1, internal::kmp_task_prefix::size + sizeof_kmp_task_t + sizeof_shareds, NULL);
        __TBB_ASSERT_RELEASE(prefix, "Failed to allocate a task");
        prefix->kmp_task_size = sizeof_kmp_task_t;
        // Calculate the task address.
        kmp_task_t* task = (kmp_task_t*)((char*)prefix + internal::kmp_task_prefix::size);
        // Calculate the address of shared variables.
        task->shareds = ((char*)task + sizeof_kmp_task_t);
        task->routine = task_entry;
        return task;
    }

#if USE_OMPTBB_EXCLUSIVELY
    void __kmpc_taskloop
#else
    void __tbb_omp_taskloop
#endif
    (
        ident_t *,
        kmp_int32 gtid,
        kmp_task_t * task,
        kmp_int32 if_val,
        kmp_uint64 * lb,
        kmp_uint64 * ub,
        kmp_int64 st,
        kmp_int32 nogroup,
        kmp_int32 sched,      // 1 for Grainsize, 2 for num_tasks, 0 for none.
        kmp_uint64 grainsize,
        void * task_dup)
    {
        //JJJ __TBB_ASSERT_RELEASE(gtid == 0, "Unknown gtid");
        __TBB_ASSERT_RELEASE(st == 1, "Unsupported construction");
        __TBB_ASSERT_RELEASE(nogroup == 0, "Unsupported construction");
        //JJJ __TBB_ASSERT_RELEASE(grainsize == 0, "Unsupported construction");
        __TBB_ASSERT_RELEASE(task_dup == nullptr, "Unsupported construction");

        using namespace internal;

        // Copy reductions because they can be overriden by nested taskloop with a reduction.
        reduce_data_t::reductions_t reductions = std::move(reduce_data.reductions);
        // Clear the reductions not to be reused by a nested taskloop occasionally.
        reduce_data.reductions.resize(0);

        // Calculate prefix address.
        kmp_task_prefix* prefix = (kmp_task_prefix*)((char*)task - kmp_task_prefix::size);
        // Calculate lower and upper bound offsets and initialize the taskloop body.
        internal::loop_body loop_body(reductions, task, prefix->kmp_task_size, (char*)lb - (char*)task, (char*)ub - (char*)task);

        // The blocked_range does not process the last index so we need to
        // increment upper bound by 1.
        kmp_uint64 lower = *lb;
        kmp_uint64 upper = *ub + 1;

        // JJJ ESU 20170605: Process grainsize:
        //  If sched == 0: don't use grainsize
        //  If sched == 1: use grainsize directly
        //  If sched == 2: 'grainsize' is actually num_tasks; compute grainsize
        __TBB_ASSERT_RELEASE(0<=sched && sched<=2, "Unexpected taskloop sched");
        if (sched == 2) {
          __TBB_ASSERT_RELEASE(grainsize!=0, "Unexpected: sched==2 && num_tasks==0");
          __TBB_ASSERT_RELEASE(st!=0, "Unexpected: stride==0");
          kmp_uint64 num_tasks = grainsize;
          kmp_int64 tripcount = (upper - lower) / st;
          if (tripcount < 0) { // st is signed, so it can be negative
            tripcount = -tripcount;
          }
          grainsize = tripcount / num_tasks;
        }
        if (sched > 0 && grainsize > 0) {
          // use grainsize
          tbb::parallel_reduce(tbb::blocked_range<size_t>
                                    (lower, upper, grainsize), loop_body);
        }
        else {
          // don't use grainsize
          tbb::parallel_reduce(tbb::blocked_range<size_t>
                                    (lower, upper), loop_body);
        }

        // Update the shared variables used in reduction.
        loop_body.join_to_shared();

        tbb::internal::NFS_Free(prefix);
    }

#if USE_OMPTBB_EXCLUSIVELY
    void* __kmpc_task_reduction_init
#else
    void* __tbb_omp_task_reduction_init
#endif
    (int gtid, int num_data, void *data) {
        //JJJ __TBB_ASSERT_RELEASE(gtid == 0, "Unknown gtid");
        kmp_task_red_input_t* input = (kmp_task_red_input_t*)data;
        // Copy the reductions to per-thread data structure to be processed by taskloop.
        internal::reduce_data.reductions.assign(input, input + num_data);
        return nullptr;
    }

#if USE_OMPTBB_EXCLUSIVELY
    void* __kmpc_task_reduction_get_th_data
#else
    void* __tbb_omp_task_reduction_get_th_data
#endif
    (int gtid, void *tskgrp, void *data) {
        //JJJ __TBB_ASSERT_RELEASE(gtid == 0, "Unknown gtid");
        __TBB_ASSERT_RELEASE(tskgrp == nullptr, "Unsupported construction");
        // Let body to resolve the address.
        return internal::reduce_data.innermost_loop_body->resolve_addr(data);
    }
}
