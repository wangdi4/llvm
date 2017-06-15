//======== deviceRTLs/nios2/libomp.cpp - OMP runtime for NiosII -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implements an OpenMP runtime library for the NiosII target.
///
//===----------------------------------------------------------------------===//

#include "libomp.h"
#include <fpga_mc_dispatcher.h>
#include "mini_ffi.h"

extern "C" {

static int omp_initialized = 0;
static int omp_max_num_threads;
static int omp_num_threads;

#define OMP_INIT if (!omp_initialized) omp_init();
static void omp_init()
{
    // Currently this sets #cores in current column only
    omp_max_num_threads = omp_num_threads = fpga_mc_get_num_cores();
}

void __kmp_nios_omp_outlined_dispatch(
    struct __nios_workitem * wi
)
{
    // Call the compiler-outlined function
    mini_ffi((void*)wi->omp_outlined, wi->argc, &wi->args[0]);

#if 0
    switch (wi->argc)
    {
    case 0:
        (wi->omp_outlined)(wi->tid, wi->bid);
        break;
    case 1:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0]);
        break;
    case 2:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1]);
        break;
    case 3:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2]);
        break;
    case 4:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2], wi->args[3]);
        break;
    case 5:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2], wi->args[3], wi->args[4]);
        break;
    case 6:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2], wi->args[3], wi->args[4], wi->args[5]);
        break;
    case 7:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2], wi->args[3], wi->args[4], wi->args[5],
            wi->args[6]);
        break;
    case 8:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2], wi->args[3], wi->args[4], wi->args[5],
            wi->args[6], wi->args[7]);
        break;
    case 9:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2], wi->args[3], wi->args[4], wi->args[5],
            wi->args[6], wi->args[7], wi->args[8]);
        break;
    case 10:
        (wi->omp_outlined)(wi->tid, wi->bid,
            wi->args[0], wi->args[1], wi->args[2], wi->args[3], wi->args[4], wi->args[5],
            wi->args[6], wi->args[7], wi->args[8], wi->args[9]);
        break;
    default:
        break;

    }
#endif
}

void __kmp_nios_omp_outlined_wrapper()
{
    struct __nios_workitem * wi =
        (struct __nios_workitem *)(fpga_mc_cpux_read_message());
    // Flush the base workitem struct
    //alt_dcache_flush_no_writeback(wi, sizeof(struct __nios_workitem));
    fpga_mc_flush_cache_extent(wi, sizeof(struct __nios_workitem));
    // Flush the arguments array in workitem struct
    //alt_dcache_flush_no_writeback(&wi->args, wi->argc * sizeof(int*));
    fpga_mc_flush_cache_extent(&wi->args, wi->argc * sizeof(int*));

    //AFU_STATUS[myTask->id] = 1;
    // Call the compiler-outlined function
    __kmp_nios_omp_outlined_dispatch(wi);

    //mmult(myTask);
    //myTask->inuse = 0;
    //alt_dcache_flush_all();
    fpga_mc_flush_cache();
    //AFU_STATUS[myTask->id] = 0;
    fpga_mc_cpux_done();
}

void __kmpc_fork_call(ident_t* loc, kmp_int32 argc, kmpc_micro omp_outlined, ... )
{
    struct __nios_workitem * wi;
    int i;
    int cores_to_use;
    va_list vl;
    va_start(vl, omp_outlined);

    // Allocate work item
    wi = (struct __nios_workitem *)malloc(sizeof(struct __nios_workitem) + argc*sizeof(int));

    // Fill in work item contents
    wi->omp_outlined = omp_outlined;
    wi->tid = 0;
    wi->bid = 0;
    wi->argc = argc;
    for (i=0; i<argc; i++)
    {
        wi->args[i] = va_arg(vl, int*);
    }

    // Schedule task on all non-master cores
    cores_to_use = fpga_mc_get_num_cores();
    for (i=0; i<cores_to_use; i++)
    {
        //nios_multi_core_scheduler_fork_func_with_msg_on_cpux(
        //    &__kmp_nios_omp_outlined_wrapper,
        //    reinterpret_cast<void*>(wi),
        //    i+1
        //);
        fpga_mc_scheduler_fork_all_worker_cores(
            &__kmp_nios_omp_outlined_wrapper, (void*)wi);
    }

    // Run task on master core
    __kmp_nios_omp_outlined_dispatch(wi);

    // Wait for other cores to complete
    fpga_mc_scheduler_wait_all_core_reset();

    va_end(vl);
}

void
__kmpc_for_static_init_4( ident_t *loc, kmp_int32 gtid, kmp_int32 schedtype, kmp_int32 *plastiter,
                      kmp_int32 *plower, kmp_int32 *pupper,
                      kmp_int32 *pstride, kmp_int32 incr, kmp_int32 chunk )
{
    /*  this all has to be changed back to TID and such.. */
    //register kmp_int32   gtid = global_tid;
    register kmp_uint32  tid = fpga_mc_get_core_id();
    register kmp_uint32  nth;
    register UT          trip_count;

    KMP_DEBUG_ASSERT( plastiter && plower && pupper && pstride );
    KE_TRACE( 10, ("__kmpc_for_static_init called (%d)\n", global_tid));
    #ifdef KMP_DEBUG
    {
        const char * buff;
    }
    #endif

    /* special handling for zero-trip loops */
    if ( incr > 0 ? (*pupper < *plower) : (*plower < *pupper) ) {
        if( plastiter != NULL )
            *plastiter = FALSE;
        /* leave pupper and plower set to entire iteration space */
        *pstride = incr;   /* value should never be used */
    //        *plower = *pupper - incr;   // let compiler bypass the illegal loop (like for(i=1;i<10;i--))  THIS LINE CAUSED shape2F/h_tests_1.f TO HAVE A FAILURE ON A ZERO-TRIP LOOP (lower=1,\
      upper=0,stride=1) - JPH June 23, 2009.
        #ifdef KMP_DEBUG
        {
            const char * buff;
        }
        #endif
        KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

        return;
    }

    nth = fpga_mc_get_num_cores();
    if ( nth == 1 ) {
        if( plastiter != NULL )
            *plastiter = TRUE;
        *pstride = (incr > 0) ? (*pupper - *plower + 1) : (-(*plower - *pupper + 1));
        #ifdef KMP_DEBUG
        {
            const char * buff;
            // create format specifiers before the debug output
            buff = __kmp_str_format(
                "__kmpc_for_static_init: (serial) liter=%%d lower=%%%s upper=%%%s stride = %%%s\n",
                traits_t< T >::spec, traits_t< T >::spec, traits_t< ST >::spec );
            KD_TRACE(100, ( buff, *plastiter, *plower, *pupper, *pstride ) );
            __kmp_str_free( &buff );
        }
        #endif
        KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

        return;
    }

    /* compute trip count */
    if ( incr == 1 ) {
        trip_count = *pupper - *plower + 1;
    } else if (incr == -1) {
        trip_count = *plower - *pupper + 1;
    } else if ( incr > 0 ) {
        // upper-lower can exceed the limit of signed type
        trip_count = (UT)(*pupper - *plower) / incr + 1;
    } else {
        trip_count = (UT)(*plower - *pupper) / (-incr) + 1;
    }

    /* compute remaining parameters */
    switch ( schedtype ) {
    case kmp_sch_static:
        {
            if ( trip_count < nth ) {
                KMP_DEBUG_ASSERT(
                    __kmp_static == kmp_sch_static_greedy || \
                    __kmp_static == kmp_sch_static_balanced
                ); // Unknown static scheduling type.
                if ( tid < trip_count ) {
                    *pupper = *plower = *plower + tid * incr;
                } else {
                    *plower = *pupper + incr;
                }
                if( plastiter != NULL )
                    *plastiter = ( tid == trip_count - 1 );
            } else {
                register kmp_int32 big_chunk_inc_count =
                    ( trip_count/nth + ( ( trip_count % nth ) ? 1 : 0) ) * incr;
                register kmp_int32 old_upper = *pupper;

                *plower += tid * big_chunk_inc_count;
                *pupper = *plower + big_chunk_inc_count - incr;
                if ( incr > 0 ) {
                    if( *pupper < *plower )
                        *pupper = int_max;
                    if( plastiter != NULL )
                        *plastiter = *plower <= old_upper && *pupper > old_upper - incr;
                    if ( *pupper > old_upper ) *pupper = old_upper; // tracker C73258
                } else {
                    if( *pupper > *plower )
                        *pupper = int_min;
                    if( plastiter != NULL )
                        *plastiter = *plower >= old_upper && *pupper < old_upper - incr;
                    if ( *pupper < old_upper ) *pupper = old_upper; // tracker C73258
                }
            }
            break;
        }
    default:
        KMP_ASSERT2( 0, "__kmpc_for_static_init: unknown scheduling type" );
        break;
    }

    #ifdef KMP_DEBUG
    {
        const char * buff;
    }
    #endif
    KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

    return;
}

void
__kmpc_for_static_init_4u( ident_t *loc, kmp_int32 gtid, kmp_int32 schedtype, kmp_int32 *plastiter,
                      kmp_uint32 *plower, kmp_uint32 *pupper,
                      kmp_int32 *pstride, kmp_int32 incr, kmp_int32 chunk )
{
    /*  this all has to be changed back to TID and such.. */
    //register kmp_int32   gtid = global_tid;
    register kmp_uint32  tid = fpga_mc_get_core_id();
    register kmp_uint32  nth;
    register UT          trip_count;

    KMP_DEBUG_ASSERT( plastiter && plower && pupper && pstride );
    KE_TRACE( 10, ("__kmpc_for_static_init called (%d)\n", global_tid));
    #ifdef KMP_DEBUG
    {
        const char * buff;
    }
    #endif

    /* special handling for zero-trip loops */
    if ( incr > 0 ? (*pupper < *plower) : (*plower < *pupper) ) {
        if( plastiter != NULL )
            *plastiter = FALSE;
        /* leave pupper and plower set to entire iteration space */
        *pstride = incr;   /* value should never be used */
    //        *plower = *pupper - incr;   // let compiler bypass the illegal loop (like for(i=1;i<10;i--))  THIS LINE CAUSED shape2F/h_tests_1.f TO HAVE A FAILURE ON A ZERO-TRIP LOOP (lower=1,\
      upper=0,stride=1) - JPH June 23, 2009.
        #ifdef KMP_DEBUG
        {
            const char * buff;
        }
        #endif
        KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

        return;
    }

    nth = fpga_mc_get_num_cores();
    if ( nth == 1 ) {
        if( plastiter != NULL )
            *plastiter = TRUE;
        *pstride = (incr > 0) ? (*pupper - *plower + 1) : (-(*plower - *pupper + 1));
        #ifdef KMP_DEBUG
        {
            const char * buff;
            // create format specifiers before the debug output
            buff = __kmp_str_format(
                "__kmpc_for_static_init: (serial) liter=%%d lower=%%%s upper=%%%s stride = %%%s\n",
                traits_t< T >::spec, traits_t< T >::spec, traits_t< ST >::spec );
            KD_TRACE(100, ( buff, *plastiter, *plower, *pupper, *pstride ) );
            __kmp_str_free( &buff );
        }
        #endif
        KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

        return;
    }

    /* compute trip count */
    if ( incr == 1 ) {
        trip_count = *pupper - *plower + 1;
    } else if (incr == -1) {
        trip_count = *plower - *pupper + 1;
    } else if ( incr > 0 ) {
        // upper-lower can exceed the limit of signed type
        trip_count = (UT)(*pupper - *plower) / incr + 1;
    } else {
        trip_count = (UT)(*plower - *pupper) / (-incr) + 1;
    }

    /* compute remaining parameters */
    switch ( schedtype ) {
    case kmp_sch_static:
        {
            if ( trip_count < nth ) {
                KMP_DEBUG_ASSERT(
                    __kmp_static == kmp_sch_static_greedy || \
                    __kmp_static == kmp_sch_static_balanced
                ); // Unknown static scheduling type.
                if ( tid < trip_count ) {
                    *pupper = *plower = *plower + tid * incr;
                } else {
                    *plower = *pupper + incr;
                }
                if( plastiter != NULL )
                    *plastiter = ( tid == trip_count - 1 );
            } else {
                register kmp_uint32 big_chunk_inc_count =
                    ( trip_count/nth + ( ( trip_count % nth ) ? 1 : 0) ) * incr;
                register kmp_uint32 old_upper = *pupper;

                *plower += tid * big_chunk_inc_count;
                *pupper = *plower + big_chunk_inc_count - incr;
                if ( incr > 0 ) {
                    if( *pupper < *plower )
                        *pupper = uint_max;
                    if( plastiter != NULL )
                        *plastiter = *plower <= old_upper && *pupper > old_upper - incr;
                    if ( *pupper > old_upper ) *pupper = old_upper; // tracker C73258
                } else {
                    if( *pupper > *plower )
                        *pupper = uint_min;
                    if( plastiter != NULL )
                        *plastiter = *plower >= old_upper && *pupper < old_upper - incr;
                    if ( *pupper < old_upper ) *pupper = old_upper; // tracker C73258
                }
            }
            break;
        }
    default:
        KMP_ASSERT2( 0, "__kmpc_for_static_init: unknown scheduling type" );
        break;
    }

    #ifdef KMP_DEBUG
    {
        const char * buff;
    }
    #endif
    KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

    return;
}

void __kmpc_for_static_fini(
    ident_t *loc,
    kmp_int32 global_tid )
{
}


// OpenMP standard API definitions 

static void omp_not_implemented_for_nios(const char* api)
{
    printf("\"%s\" is not implemented on Nios\n", api);
}

void omp_set_num_threads(int num_threads)
{
    omp_num_threads = num_threads;
}

void omp_set_dynamic(int b)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_set_nested(int l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_set_max_active_levels(int l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_set_schedule(enum sched_type s, int l)
{
    omp_not_implemented_for_nios(__func__);
}

/* query API functions */
int omp_get_num_threads(void)
{
    return omp_num_threads;
}

int omp_get_dynamic(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_nested(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_max_threads(void)
{
    return omp_max_num_threads;
}

int omp_get_thread_num(void)
{
    // Currently this is within current column only
    return fpga_mc_get_core_id();
}

int omp_get_num_procs(void)
{
    return omp_max_num_threads;
}

int omp_in_parallel(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_in_final(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_active_level(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_level(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_ancestor_thread_num(int n)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_team_size(int s)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_thread_limit(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_max_active_levels(void)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_get_schedule(enum sched_type *s, int *l)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_max_task_priority(void)
{
    omp_not_implemented_for_nios(__func__);
}


/* lock API functions */
typedef struct omp_lock_t {
    void * _lk;
} omp_lock_t;

void omp_init_lock(omp_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_set_lock(omp_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_unset_lock(omp_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_destroy_lock(omp_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_test_lock(omp_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}


/* nested lock API functions */
typedef struct omp_nest_lock_t {
    void * _lk;
} omp_nest_lock_t;

void omp_init_nest_lock(omp_nest_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_set_nest_lock(omp_nest_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_unset_nest_lock(omp_nest_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_destroy_nest_lock(omp_nest_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_test_nest_lock(omp_nest_lock_t *l)
{
    omp_not_implemented_for_nios(__func__);
}


/* lock hint type for dynamic user lock */
typedef enum omp_lock_hint_t {
    omp_lock_hint_none           = 0,
    omp_lock_hint_uncontended    = 1,
    omp_lock_hint_contended      = (1<<1 ),
    omp_lock_hint_nonspeculative = (1<<2 ),
    omp_lock_hint_speculative    = (1<<3 ),
    kmp_lock_hint_hle            = (1<<16),
    kmp_lock_hint_rtm            = (1<<17),
    kmp_lock_hint_adaptive       = (1<<18)
} omp_lock_hint_t;

/* hinted lock initializers */
void omp_init_lock_with_hint(omp_lock_t *l, omp_lock_hint_t h)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_init_nest_lock_with_hint(omp_nest_lock_t *l, omp_lock_hint_t h)
{
    omp_not_implemented_for_nios(__func__);
}


/* time API functions */
double omp_get_wtime(void)
{
    omp_not_implemented_for_nios(__func__);
}

double omp_get_wtick(void)
{
    omp_not_implemented_for_nios(__func__);
}


/* OpenMP 4.0 */
int omp_get_default_device(void)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_set_default_device(int d)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_is_initial_device(void)
{
    return 0;
}

int omp_get_num_devices(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_num_teams(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_team_num(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_cancellation(void)
{
    omp_not_implemented_for_nios(__func__);
}


#include <stdlib.h>
/* OpenMP 4.5 */
int omp_get_initial_device(void)
{
    omp_not_implemented_for_nios(__func__);
}

/* OpenMP 4.0 affinity API */
typedef enum omp_proc_bind_t {
    omp_proc_bind_false = 0,
    omp_proc_bind_true = 1,
    omp_proc_bind_master = 2,
    omp_proc_bind_close = 3,
    omp_proc_bind_spread = 4
} omp_proc_bind_t;

omp_proc_bind_t  omp_get_proc_bind (void)
{
    omp_not_implemented_for_nios(__func__);
}

/* OpenMP 4.5 affinity API */
int omp_get_num_places(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_place_num_procs(int a)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_get_place_proc_ids(int a, int *b)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_place_num(void)
{
    omp_not_implemented_for_nios(__func__);
}

int omp_get_partition_num_places(void)
{
    omp_not_implemented_for_nios(__func__);
}

void omp_get_partition_place_nums(int *a)
{
    omp_not_implemented_for_nios(__func__);
}

}
