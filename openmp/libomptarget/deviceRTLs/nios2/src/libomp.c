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


void* nios2_unsafe_malloc_l2(size_t s, int l)
{
	return fpga_mc_unsafe_malloc_l2(s,l);
}
void* nios_unsafe_malloc_l2(size_t s, int l)
{
	return fpga_mc_unsafe_malloc_l2(s,l);
}
void nios_memcpy_device_to_l4(l4_virtual_ptr_t dst, void* src, size_t s)
{
	fpga_mc_memcpy_device_to_l4(dst, src, s);
}
void nios_memcpy_l4_to_device(void* dst, l4_virtual_ptr_t src, size_t s)
{
	fpga_mc_memcpy_l4_to_device(dst, src, s);
}

static int omp_initialized = 0;
static int omp_max_num_threads;
static int omp_num_threads;

//#define TRACE if (fpga_mc_get_verbosity() > 0) printf
#define TRACE printf

#define OMP_INIT \
    fpga_mc_load_cache_extent(&omp_initialized, sizeof(omp_initialized)); \
    if (!omp_initialized) omp_init()

static void omp_init()
{

    // Currently this sets #cores in current column only
    omp_max_num_threads = omp_num_threads = fpga_mc_get_num_cores();
    omp_initialized = 1;
    fpga_mc_flush_cache_extent(&omp_max_num_threads, sizeof(omp_max_num_threads));
    fpga_mc_flush_cache_extent(&omp_num_threads, sizeof(omp_num_threads));
    fpga_mc_flush_cache_extent(&omp_initialized, sizeof(omp_initialized));
    TRACE("omp_init()\n");
}

void trace_wi(
    struct __nios_workitem * wi
)
{
    int i;

    TRACE("wi->func    = %x\n", wi->omp_outlined);
    TRACE("wi->argc    = %x\n", wi->argc);
    for (i=0; i<wi->argc; i++)
    {
        TRACE("wi->args[%d] = %x\n", i, wi->args[i]);
    }
}

void __kmp_nios_omp_outlined_dispatch(
    struct __nios_workitem * wi
)
{
    // Call the compiler-outlined function
    mini_ffi((void*)wi->omp_outlined, wi->argc, &wi->args[0]);
}

void __kmp_nios_omp_outlined_wrapper()
{
    struct __nios_workitem * wi;

    //if (fpga_mc_get_core_id() == 1) TRACE("__kmp_nios_omp_outlined_wrapper\n");
    //TRACE("__kmp_nios_omp_outlined_wrapper\n");
    wi = (struct __nios_workitem *)(fpga_mc_cpux_read_message());
    //if (fpga_mc_get_core_id() == 1) TRACE("wi=%x\n", wi);
    TRACE("wi=%x\n", wi);

    // Flush the base workitem struct
    fpga_mc_load_cache_extent(wi, sizeof(struct __nios_workitem));
    // Flush the arguments array in workitem struct
    fpga_mc_load_cache_extent(&wi->args[0], wi->argc * sizeof(int*));
    //if (fpga_mc_get_core_id() == 1) trace_wi(wi);
    //trace_wi(wi);

    // Call the compiler-outlined function
    __kmp_nios_omp_outlined_dispatch(wi);

    fpga_mc_cpux_done();
}

void __kmpc_fork_call(ident_t* loc, kmp_int32 argc, kmpc_micro omp_outlined, ... )
{
    struct __nios_workitem * wi;
    int wil;
    int i;
    int workers_to_use;
    va_list vl;
    va_start(vl, omp_outlined);

    OMP_INIT;
    TRACE("__kmpc_fork_call(loc,argc=%d,omp_outlined=%x)\n", argc, omp_outlined);
    // Allocate work item
    // tid and bid are always the first two arguments
    wil = sizeof(struct __nios_workitem) + (argc+2)*sizeof(int*);
    wi  = (struct __nios_workitem *)fpga_mc_unsafe_malloc_l2(wil, 32);
    TRACE("wi=%x allocated with size=%d\n", wi, wil);

    // Fill in work item contents
    wi->omp_outlined = omp_outlined;
    wi->argc = argc + 2;
    wi->args[0] = 0; // tid is 0
    wi->args[1] = 0; // bid is 0
    for (i=2; i<wi->argc; i++)
    {
        wi->args[i] = va_arg(vl, int*);
    }
    trace_wi(wi);

    // Schedule task on all non-master cores
    // if num_threads > 1
    workers_to_use = omp_num_threads - 1;
    TRACE("workers_to_use=%d\n", workers_to_use);
    if (workers_to_use > 0)
    {
        fpga_mc_flush_cache_extent(
            wi,
            sizeof(struct __nios_workitem) + wi->argc*sizeof(int));
        if (omp_num_threads == omp_max_num_threads)
        {
            TRACE("fpga_mc_scheduler_fork_all_worker_cores(func=%x, wi=%x)\n",
                &__kmp_nios_omp_outlined_wrapper, (void*)wi);
            fpga_mc_scheduler_fork_all_worker_cores(
                &__kmp_nios_omp_outlined_wrapper, (void*)wi);
        } else {
            int worker;

            for (worker=1; worker<=workers_to_use; worker++)
            {
                TRACE("fpga_mc_scheduler_fork_func_with_msg_on_cpux(func=%x, wi=%x, worker=%d)\n",
                    &__kmp_nios_omp_outlined_wrapper, (void*)wi, worker);
		fpga_mc_scheduler_fork_func_with_msg_on_cpux(
                    &__kmp_nios_omp_outlined_wrapper, (void*)wi, worker);
            }
        }
    }

    // Run task on master core
    TRACE("__kmp_nios_omp_outlined_dispatch(wi=%x)\n", wi);
    __kmp_nios_omp_outlined_dispatch(wi);

    // Wait for other cores to complete
    if (workers_to_use > 0)
    {
        if (omp_num_threads == omp_max_num_threads)
        {
            fpga_mc_scheduler_wait_all_core_reset();
            TRACE("fpga_mc_scheduler_wait_all_core_reset()\n");
        } else {
            int worker;

            for (worker=1; worker<=workers_to_use; worker++)
            {
                fpga_mc_scheduler_wait_core_reset(worker);
                TRACE("fpga_mc_scheduler_wait_core_reset(worker=%d)\n", worker);
            }
        }
        TRACE("workers are done\n");
    }

    va_end(vl);
    TRACE("over\n");
}

void
__kmpc_for_static_init_4( ident_t *loc, kmp_int32 gtid, kmp_int32 schedtype, kmp_int32 *plastiter,
                      kmp_int32 *plower, kmp_int32 *pupper,
                      kmp_int32 *pstride, kmp_int32 incr, kmp_int32 chunk )
{
    /*  this all has to be changed back to TID and such.. */
    register kmp_uint32  tid = fpga_mc_get_core_id();
    register kmp_uint32  nth;
    register UT          trip_count;

    OMP_INIT;
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
        #ifdef KMP_DEBUG
        {
            const char * buff;
        }
        #endif
        KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

        return;
    }

    fpga_mc_load_cache_extent(&omp_num_threads, sizeof(omp_num_threads));
    nth = omp_num_threads;
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
        #ifdef KMP_DEBUG
        {
            const char * buff;
        }
        #endif
        KE_TRACE( 10, ("__kmpc_for_static_init: T#%d return\n", global_tid ) );

        return;
    }

    fpga_mc_load_cache_extent(&omp_num_threads, sizeof(omp_num_threads));
    nth = omp_num_threads;
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
    OMP_INIT;
    if (num_threads < 0)
    {
        return;
    }
    if (num_threads > omp_max_num_threads)
    {
        omp_num_threads = omp_max_num_threads;
    } else {
        omp_num_threads = num_threads;
    }
    fpga_mc_flush_cache_extent(&omp_num_threads, sizeof(omp_num_threads));
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
    OMP_INIT;
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
    OMP_INIT;
    return omp_max_num_threads;
}

int omp_get_thread_num(void)
{
    int tid;
    OMP_INIT;
    // Currently this is within current column only
    tid = fpga_mc_get_core_id();
    return tid;
}

int omp_get_num_procs(void)
{
    OMP_INIT;
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
    OMP_INIT;
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

