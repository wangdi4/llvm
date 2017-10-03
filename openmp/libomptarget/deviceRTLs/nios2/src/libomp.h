// OpenMP library for Nios
// kmp declarations

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern int mini_ffi(void* f, int argc, int* args[]);

#define KMP_DEBUG_ASSERT(x)
#define KE_TRACE(d,x)
#define KMP_ASSERT2(x,y)

#define kmp_int32 int
#define kmp_uint32 unsigned int

#define FALSE 0
#define TRUE  1

typedef kmp_int32  ST;
typedef kmp_uint32 UT;

static const kmp_int32  int_max  = 0x7fffffff;
static const kmp_int32  int_min  = 0x80000000;
static const kmp_uint32 uint_max = 0xffffffff;
static const kmp_uint32 uint_min = 0x00000000;

/*!
 * The ident structure that describes a source location.
 */
typedef struct ident {
    kmp_int32 reserved_1;   /**<  might be used in Fortran; see above  */
    kmp_int32 flags;        /**<  also f.flags; KMP_IDENT_xxx flags; KMP_IDENT_KMPC identifies this union member  */
    kmp_int32 reserved_2;   /**<  not really used in Fortran any more; see above */
    kmp_int32 reserved_3;   /**< source[4] in Fortran, do not use for C++  */
    char const *psource;    /**< String describing the source location.
                            The string is composed of semi-colon separated fields which describe the source file,
                            the function and a pair of line numbers that delimit the construct.
                             */
} ident_t;


/*!
@ingroup PARALLEL
The type for a microtask which gets passed to @ref __kmpc_fork_call().
The arguments to the outlined function are
@param global_tid the global thread identity of the thread executing the function.
@param bound_tid  the local identitiy of the thread executing the function
@param ... pointers to shared variables accessed by the function.
*/
typedef void (*kmpc_micro) ( kmp_int32 * global_tid, kmp_int32 * bound_tid, ... );


void __kmpc_fork_call(ident_t* loc, kmp_int32 argc, kmpc_micro microtask, ... );

/*!
@ingroup WORK_SHARING
@param    loc       Source code location
@param    gtid      Global thread id of this thread
@param    schedtype  Scheduling type
@param    plastiter Pointer to the "last iteration" flag
@param    plower    Pointer to the lower bound
@param    pupper    Pointer to the upper bound
@param    pstride   Pointer to the stride
@param    incr      Loop increment
@param    chunk     The chunk size

Each of the four functions here are identical apart from the argument types.

The functions compute the upper and lower bounds and stride to be used for the set of iterations
to be executed by the current thread from the statically scheduled loop that is described by the
initial values of the bounds, stride, increment and chunk size.

@{
*/
void
__kmpc_for_static_init_4( ident_t *loc, kmp_int32 gtid, kmp_int32 schedtype, kmp_int32 *plastiter,
                      kmp_int32 *plower, kmp_int32 *pupper,
                      kmp_int32 *pstride, kmp_int32 incr, kmp_int32 chunk );

/*!
 See @ref __kmpc_for_static_init_4
 */
void
__kmpc_for_static_init_4u( ident_t *loc, kmp_int32 gtid, kmp_int32 schedtype, kmp_int32 *plastiter,
                      kmp_uint32 *plower, kmp_uint32 *pupper,
                      kmp_int32 *pstride, kmp_int32 incr, kmp_int32 chunk );

/*!
@}
*/

void __kmpc_for_static_fini( ident_t *loc, kmp_int32 global_tid );


/*!
 @ingroup WORK_SHARING
 * Describes the loop schedule to be used for a parallel for loop.
 */
enum sched_type {
    kmp_sch_lower                     = 32,   /**< lower bound for unordered values */
    kmp_sch_static_chunked            = 33,
    kmp_sch_static                    = 34,   /**< static unspecialized */
    kmp_sch_dynamic_chunked           = 35,
    kmp_sch_guided_chunked            = 36,   /**< guided unspecialized */
    kmp_sch_runtime                   = 37,
    kmp_sch_auto                      = 38,   /**< auto */
    kmp_sch_trapezoidal               = 39,

    /* accessible only through KMP_SCHEDULE environment variable */
    kmp_sch_static_greedy             = 40,
    kmp_sch_static_balanced           = 41,
    /* accessible only through KMP_SCHEDULE environment variable */
    kmp_sch_guided_iterative_chunked  = 42,
    kmp_sch_guided_analytical_chunked = 43,

    kmp_sch_static_steal              = 44,   /**< accessible only through KMP_SCHEDULE environment variable */

#if OMP_45_ENABLED
    kmp_sch_static_balanced_chunked   = 45,   /**< static with chunk adjustment (e.g., simd) */
#endif

    /* accessible only through KMP_SCHEDULE environment variable */
    kmp_sch_upper                     = 46,   /**< upper bound for unordered values */

    kmp_ord_lower                     = 64,   /**< lower bound for ordered values, must be power of 2 */
    kmp_ord_static_chunked            = 65,
    kmp_ord_static                    = 66,   /**< ordered static unspecialized */
    kmp_ord_dynamic_chunked           = 67,
    kmp_ord_guided_chunked            = 68,
    kmp_ord_runtime                   = 69,
    kmp_ord_auto                      = 70,   /**< ordered auto */
    kmp_ord_trapezoidal               = 71,
    kmp_ord_upper                     = 72,   /**< upper bound for ordered values */

#if OMP_40_ENABLED
    /* Schedules for Distribute construct */
    kmp_distribute_static_chunked     = 91,   /**< distribute static chunked */
    kmp_distribute_static             = 92,   /**< distribute static unspecialized */
#endif

    /*
     * For the "nomerge" versions, kmp_dispatch_next*() will always return
     * a single iteration/chunk, even if the loop is serialized.  For the
     * schedule types listed above, the entire iteration vector is returned
     * if the loop is serialized.  This doesn't work for gcc/gcomp sections.
     */
    kmp_nm_lower                      = 160,  /**< lower bound for nomerge values */

    kmp_nm_static_chunked             = (kmp_sch_static_chunked - kmp_sch_lower + kmp_nm_lower),
    kmp_nm_static                     = 162,  /**< static unspecialized */
    kmp_nm_dynamic_chunked            = 163,
    kmp_nm_guided_chunked             = 164,  /**< guided unspecialized */
    kmp_nm_runtime                    = 165,
    kmp_nm_auto                       = 166,  /**< auto */
    kmp_nm_trapezoidal                = 167,

    kmp_sch_default = kmp_sch_static  /**< default scheduling algorithm */
};

struct __nios_workitem {
   kmpc_micro  omp_outlined;
   kmp_int32   argc;
   kmp_int32 * args[0];
};

