/*
 * include/45/omp.h.var
 * $Revision: 2474 $
 * $Date: 2017-03-21 17:42:25 -0700 (Tue, 21 Mar 2017) $
 */

/* <copyright>

    Copyright 1985-2017 Intel Corporation.  All Rights Reserved.

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

    Portions of this software are protected under the following patents:
        U.S. Patent 5,812,852
        U.S. Patent 6,792,599
        U.S. Patent 7,069,556
        U.S. Patent 7,328,433
        U.S. Patent 7,500,242

</copyright> */

#ifndef __OMP_H
#   define __OMP_H

#   define KMP_VERSION_MAJOR    5
#   define KMP_VERSION_MINOR    0
#   define KMP_VERSION_BUILD    20170210
#   define KMP_BUILD_DATE       "2017-02-11 20:08:43 UTC"

#   ifdef __cplusplus
    extern "C" {
#   endif

#   if defined(_WIN32)
#       define __KAI_KMPC_CONVENTION __cdecl
#   else
#       define __KAI_KMPC_CONVENTION
#   endif

    /* schedule kind constants */
    typedef enum omp_sched_t {
	omp_sched_static  = 1,
	omp_sched_dynamic = 2,
	omp_sched_guided  = 3,
	omp_sched_auto    = 4
    } omp_sched_t;

    /* set API functions */
    extern void   __KAI_KMPC_CONVENTION  omp_set_num_threads (int);
    extern void   __KAI_KMPC_CONVENTION  omp_set_dynamic     (int);
    extern void   __KAI_KMPC_CONVENTION  omp_set_nested      (int);
    extern void   __KAI_KMPC_CONVENTION  omp_set_max_active_levels (int);
    extern void   __KAI_KMPC_CONVENTION  omp_set_schedule          (omp_sched_t, int);

    /* query API functions */
    extern int    __KAI_KMPC_CONVENTION  omp_get_num_threads  (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_dynamic      (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_nested       (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_max_threads  (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_thread_num   (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_num_procs    (void);
    extern int    __KAI_KMPC_CONVENTION  omp_in_parallel      (void);
    extern int    __KAI_KMPC_CONVENTION  omp_in_final         (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_active_level        (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_level               (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_ancestor_thread_num (int);
    extern int    __KAI_KMPC_CONVENTION  omp_get_team_size           (int);
    extern int    __KAI_KMPC_CONVENTION  omp_get_thread_limit        (void);
    extern int    __KAI_KMPC_CONVENTION  omp_get_max_active_levels   (void);
    extern void   __KAI_KMPC_CONVENTION  omp_get_schedule            (omp_sched_t *, int *);
    extern int    __KAI_KMPC_CONVENTION  omp_get_max_task_priority   (void);

    /* lock API functions */
    typedef struct omp_lock_t {
        void * _lk;
    } omp_lock_t;

    extern void   __KAI_KMPC_CONVENTION  omp_init_lock    (omp_lock_t *);
    extern void   __KAI_KMPC_CONVENTION  omp_set_lock     (omp_lock_t *);
    extern void   __KAI_KMPC_CONVENTION  omp_unset_lock   (omp_lock_t *);
    extern void   __KAI_KMPC_CONVENTION  omp_destroy_lock (omp_lock_t *);
    extern int    __KAI_KMPC_CONVENTION  omp_test_lock    (omp_lock_t *);

    /* nested lock API functions */
    typedef struct omp_nest_lock_t {
        void * _lk;
    } omp_nest_lock_t;

    extern void   __KAI_KMPC_CONVENTION  omp_init_nest_lock    (omp_nest_lock_t *);
    extern void   __KAI_KMPC_CONVENTION  omp_set_nest_lock     (omp_nest_lock_t *);
    extern void   __KAI_KMPC_CONVENTION  omp_unset_nest_lock   (omp_nest_lock_t *);
    extern void   __KAI_KMPC_CONVENTION  omp_destroy_nest_lock (omp_nest_lock_t *);
    extern int    __KAI_KMPC_CONVENTION  omp_test_nest_lock    (omp_nest_lock_t *);

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
    extern void __KAI_KMPC_CONVENTION omp_init_lock_with_hint(omp_lock_t *, omp_lock_hint_t);
    extern void __KAI_KMPC_CONVENTION omp_init_nest_lock_with_hint(omp_nest_lock_t *, omp_lock_hint_t);

    /* time API functions */
    extern double __KAI_KMPC_CONVENTION  omp_get_wtime (void);
    extern double __KAI_KMPC_CONVENTION  omp_get_wtick (void);

    /* OpenMP 4.0 */
    extern int  __KAI_KMPC_CONVENTION  omp_get_default_device (void);
    extern void __KAI_KMPC_CONVENTION  omp_set_default_device (int);
    extern int  __KAI_KMPC_CONVENTION  omp_is_initial_device (void);
    extern int  __KAI_KMPC_CONVENTION  omp_get_num_devices (void);
    extern int  __KAI_KMPC_CONVENTION  omp_get_num_teams (void);
    extern int  __KAI_KMPC_CONVENTION  omp_get_team_num (void);
    extern int  __KAI_KMPC_CONVENTION  omp_get_cancellation (void);

#   include <stdlib.h>
    /* OpenMP 4.5 */
    extern int   __KAI_KMPC_CONVENTION  omp_get_initial_device (void);
    extern void* __KAI_KMPC_CONVENTION  omp_target_alloc(size_t, int);
    extern void  __KAI_KMPC_CONVENTION  omp_target_free(void *, int);
    extern int   __KAI_KMPC_CONVENTION  omp_target_is_present(void *, int);
    extern int   __KAI_KMPC_CONVENTION  omp_target_memcpy(void *, void *, size_t, size_t, size_t, int, int);
    extern int   __KAI_KMPC_CONVENTION  omp_target_memcpy_rect(void *, void *, size_t, int, const size_t *,
                                            const size_t *, const size_t *, const size_t *, const size_t *, int, int);
    extern int   __KAI_KMPC_CONVENTION  omp_target_associate_ptr(void *, void *, size_t, size_t, int);
    extern int   __KAI_KMPC_CONVENTION  omp_target_disassociate_ptr(void *, int);

    /* kmp API functions */
    extern int    __KAI_KMPC_CONVENTION  kmp_get_stacksize          (void);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_stacksize          (int);
    extern size_t __KAI_KMPC_CONVENTION  kmp_get_stacksize_s        (void);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_stacksize_s        (size_t);
    extern int    __KAI_KMPC_CONVENTION  kmp_get_blocktime          (void);
    extern int    __KAI_KMPC_CONVENTION  kmp_get_library            (void);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_blocktime          (int);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_library            (int);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_library_serial     (void);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_library_turnaround (void);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_library_throughput (void);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_defaults           (char const *);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_disp_num_buffers   (int);

    /* Intel affinity API */
    typedef void * kmp_affinity_mask_t;

    extern int    __KAI_KMPC_CONVENTION  kmp_set_affinity             (kmp_affinity_mask_t *);
    extern int    __KAI_KMPC_CONVENTION  kmp_get_affinity             (kmp_affinity_mask_t *);
    extern int    __KAI_KMPC_CONVENTION  kmp_get_affinity_max_proc    (void);
    extern void   __KAI_KMPC_CONVENTION  kmp_create_affinity_mask     (kmp_affinity_mask_t *);
    extern void   __KAI_KMPC_CONVENTION  kmp_destroy_affinity_mask    (kmp_affinity_mask_t *);
    extern int    __KAI_KMPC_CONVENTION  kmp_set_affinity_mask_proc   (int, kmp_affinity_mask_t *);
    extern int    __KAI_KMPC_CONVENTION  kmp_unset_affinity_mask_proc (int, kmp_affinity_mask_t *);
    extern int    __KAI_KMPC_CONVENTION  kmp_get_affinity_mask_proc   (int, kmp_affinity_mask_t *);

    /* OpenMP 4.0 affinity API */
    typedef enum omp_proc_bind_t {
        omp_proc_bind_false = 0,
        omp_proc_bind_true = 1,
        omp_proc_bind_master = 2,
        omp_proc_bind_close = 3,
        omp_proc_bind_spread = 4
    } omp_proc_bind_t;

    extern omp_proc_bind_t __KAI_KMPC_CONVENTION omp_get_proc_bind (void);

    /* OpenMP 4.5 affinity API */
    extern int  __KAI_KMPC_CONVENTION omp_get_num_places (void);
    extern int  __KAI_KMPC_CONVENTION omp_get_place_num_procs (int);
    extern void __KAI_KMPC_CONVENTION omp_get_place_proc_ids (int, int *);
    extern int  __KAI_KMPC_CONVENTION omp_get_place_num (void);
    extern int  __KAI_KMPC_CONVENTION omp_get_partition_num_places (void);
    extern void __KAI_KMPC_CONVENTION omp_get_partition_place_nums (int *);

    extern void * __KAI_KMPC_CONVENTION  kmp_malloc  (size_t);
    extern void * __KAI_KMPC_CONVENTION  kmp_aligned_malloc  (size_t, size_t);
    extern void * __KAI_KMPC_CONVENTION  kmp_calloc  (size_t, size_t);
    extern void * __KAI_KMPC_CONVENTION  kmp_realloc (void *, size_t);
    extern void   __KAI_KMPC_CONVENTION  kmp_free    (void *);

    extern void   __KAI_KMPC_CONVENTION  kmp_set_warnings_on(void);
    extern void   __KAI_KMPC_CONVENTION  kmp_set_warnings_off(void);

#   undef __KAI_KMPC_CONVENTION

    /* Warning:
       The following typedefs are not standard, deprecated and will be removed in a future release.
    */
    typedef int     omp_int_t;
    typedef double  omp_wtime_t;

#   ifdef __cplusplus
    }
#   endif

#endif /* __OMP_H */

