/* bindings.h contains Intel OpenMP runtime data type definitions, structures and functions.
   Almost all comments are borrowed from Intel OpenMP runtime implementation and can be slightly
   irrelevant wth the current implementation.

   TODO: pay attention to the work-around with '#pragma pack(1)' for the 'kmp_task_red_input'
   structure.
*/

#ifndef __OMPTBB_BINDINGS_H
#define __OMPTBB_BINDINGS_H

#include "tbb/tbb_stddef.h"

typedef tbb::internal::int32_t kmp_int32;
typedef tbb::internal::int64_t kmp_int64;
typedef tbb::internal::uint64_t kmp_uint64;


// Original design of this OMPTBB subset library was to link it IN PLACE OF the
// OMP runtime (libiomp5), so it provided stub routines such as kmpc_begin/end.
// The new usage model is to link the subset library IN ADDITION TO libiomp5,
// so we rename the __kmpc_task_* routines to __tbb_omp_task_*, and remove the
// stub routines. The USE_OMPTBB_EXCLUSIVELY macro below is defined to 0 for
// this purpose.
#define USE_OMPTBB_EXCLUSIVELY 0


// OMPTBB: unused
typedef void ident_t;

typedef kmp_int32(*kmp_routine_entry_t)(kmp_int32, void *);

// The structure is created by the compiler (its size is passed to __kmpc_omp_task_alloc).
// Never instantiate it directly, and do not take its size or use it in address arithmetic
struct kmp_task_t {
    void *              shareds;            /**< pointer to block of pointers to shared vars   */
    kmp_routine_entry_t routine;            /**< pointer to routine to call for executing task */
                                            /*  compiler data and private vars  */
};

typedef struct kmp_task_red_flags {
    unsigned  lazy_priv : 1;  // hint: (1) use lazy allocation (big objects)
    unsigned  reserved31 : 31;
} kmp_task_red_flags_t;

// Some versions of the compiler incorrectly calculate the size of the structure.
#if __INTEL_COMPILER == 1800 && __INTEL_COMPILER_BUILD_DATE <= 20170320
#pragma pack(1)
#endif
// Compiler provides an instance of this structure for each reduction item
typedef struct kmp_task_red_input {
    void       *reduce_shar; // shared reduction item
    size_t      reduce_size; // size of reduction item
    void       *reduce_init; // reduction data initialization routine
    void       *reduce_fini; // reduction data finalization routine
    void       *reduce_comb; // reduction data combiner routine
    kmp_task_red_flags_t flags; // flags for additional info from compiler
} kmp_task_red_input_t;

extern "C" {
    /* ------------------------------------------------------------------------ */

    /*  flags will be used in future, e.g., to implement */
    /*  openmp_strict library restrictions               */

    /*!
    * @ingroup STARTUP_SHUTDOWN
    * @param loc   in   source location information
    * @param flags in   for future use (currently ignored)
    *
    * Initialize the runtime library. This call is optional; if it is not made then
    * it will be implicitly called by attempts to use other library functions.
    *
    */
    // OMPTBB: stub (does nothing).
#if USE_OMPTBB_EXCLUSIVELY
    void __kmpc_begin(ident_t *loc, kmp_int32 flags);
#endif

    /*!
    * @ingroup STARTUP_SHUTDOWN
    * @param loc source location information
    *
    * Shutdown the runtime library. This is also optional, and even if called will not
    * do anything unless the `KMP_IGNORE_MPPEND` environment variable is set to zero.
    */
    // OMPTBB: stub (does nothing).
#if USE_OMPTBB_EXCLUSIVELY
    void __kmpc_end(ident_t *loc);
#endif

    /*!
    @ingroup THREAD_STATES
    @param loc Source location information.
    @return The global thread index of the active thread.

    This function can be called in any context.

    If the runtime has ony been entered at the outermost level from a
    single (necessarily non-OpenMP<sup>*</sup>) thread, then the thread number is that
    which would be returned by omp_get_thread_num() in the outermost
    active parallel construct. (Or zero if there is no active parallel
    construct, since the master thread is necessarily thread zero).

    If multiple non-OpenMP threads all enter an OpenMP construct then this
    will be a unique thread identifier among all the threads created by
    the OpenMP runtime (but the value cannote be defined in terms of
    OpenMP thread ids returned by omp_get_thread_num()).

    */
    // OMPTBB: stub (does nothing).
#if USE_OMPTBB_EXCLUSIVELY
    kmp_int32  __kmpc_global_thread_num(ident_t *);
#endif


    // OMPTBB: allocates a "kmp task" (similar to Intel OpenMP runtime). It is not a TBB task.
    // It contains data that is used to call user code.
#if USE_OMPTBB_EXCLUSIVELY
    kmp_task_t* __kmpc_omp_task_alloc
#else
    kmp_task_t* __tbb_omp_task_alloc
#endif
       (ident_t *loc_ref, kmp_int32 gtid, kmp_int32 flags,
        size_t sizeof_kmp_task_t, size_t sizeof_shareds,
        kmp_routine_entry_t task_entry);

    /*!
    @ingroup TASKING
    @param loc       Source location information
    @param gtid      Global thread ID
    @param task      Task structure
    @param if_val    Value of the if clause
    @param lb        Pointer to loop lower bound
    @param ub        Pointer to loop upper bound
    @param st        Loop stride
    @param nogroup   Flag, 1 if nogroup clause specified, 0 otherwise
    @param sched     Schedule specified 0/1/2 for none/grainsize/num_tasks
    @param grainsize Schedule value if specified
    @param task_dup  Tasks duplication routine

    Execute the taskloop construct.
    */
    // OMPTBB: the main routine that performs taskloop and reduction.
#if USE_OMPTBB_EXCLUSIVELY
    void __kmpc_taskloop
#else
    void __tbb_omp_taskloop
#endif
       (ident_t *loc, kmp_int32 gtid, kmp_task_t *task, kmp_int32 if_val,
        kmp_uint64 *lb, kmp_uint64 *ub, kmp_int64 st, kmp_int32 nogroup,
        kmp_int32 sched, kmp_uint64 grainsize, void * task_dup);

    //-------------------------------------------------------------------------------------
    // __kmpc_taskgroup: Start a new taskgroup
    // OMPTBB: stub (does nothing).
#if USE_OMPTBB_EXCLUSIVELY
    void __kmpc_taskgroup(ident_t * loc, int gtid);
#endif

    //-------------------------------------------------------------------------------------
    // __kmpc_end_taskgroup: Wait until all tasks generated by the current task
    //                       and its descendants are complete
    // OMPTBB: stub (does nothing).
#if USE_OMPTBB_EXCLUSIVELY
    void __kmpc_end_taskgroup(ident_t * loc, int gtid);
#endif

    /*!
    @ingroup TASKING
    @param gtid      Global thread ID
    @param num       Number of data items to reduce
    @param data      Array of data for reduction
    @return The taskgroup identifier

    Initialize task reduction for the taskgroup.
    */
    // OMPTBB: Stores reduction parameters to be used by taskloop.
#if USE_OMPTBB_EXCLUSIVELY
    void* __kmpc_task_reduction_init(int gtid, int num_data, void *data);
#else
    void* __tbb_omp_task_reduction_init(int gtid, int num_data, void *data);
#endif

    /*!
    @ingroup TASKING
    @param gtid    Global thread ID
    @param tskgrp  The taskgroup ID (optional)
    @param data    Shared location of the item
    @return The pointer to per-thread data

    Get thread-specific location of data item
    */
    // OMPTBB: Resolves the shared variable address used for reduction into a thread private
    // address.
#if USE_OMPTBB_EXCLUSIVELY
    void* __kmpc_task_reduction_get_th_data(int gtid, void *tskgrp, void *data);
#else
    void* __tbb_omp_task_reduction_get_th_data(int gtid, void *tskgrp, void *data);
#endif
}

#endif /* __OMPTBB_BINDINGS_H */
