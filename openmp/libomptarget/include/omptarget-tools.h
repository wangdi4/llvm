#if INTEL_CUSTOMIZATION
//===--- omptarget-tools.h -- OMPT support --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Type/constant definitions for OMPT target device tracing.
//
//===----------------------------------------------------------------------===//
#ifndef _OMPTARGET_TOOLS_H_
#define _OMPTARGET_TOOLS_H_
#ifdef USE_LIBOMP_OMP_TOOLS_H
#include <omp-tools.h>
#else
// Use a copy of the header instead

/*****************************************************************************
 * system include files
 *****************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <Debug.h>

/*****************************************************************************
 * iteration macros
 *****************************************************************************/

#define FOREACH_OMPT_INQUIRY_FN(macro)      \
    macro (ompt_enumerate_states)           \
    macro (ompt_enumerate_mutex_impls)      \
                                            \
    macro (ompt_set_callback)               \
    macro (ompt_get_callback)               \
                                            \
    macro (ompt_get_state)                  \
                                            \
    macro (ompt_get_parallel_info)          \
    macro (ompt_get_task_info)              \
    macro (ompt_get_task_memory)            \
    macro (ompt_get_thread_data)            \
    macro (ompt_get_unique_id)              \
    macro (ompt_finalize_tool)              \
                                            \
    macro(ompt_get_num_procs)               \
    macro(ompt_get_num_places)              \
    macro(ompt_get_place_proc_ids)          \
    macro(ompt_get_place_num)               \
    macro(ompt_get_partition_place_nums)    \
    macro(ompt_get_proc_id)                 \
                                            \
    macro(ompt_get_target_info)             \
    macro(ompt_get_num_devices)

#define FOREACH_OMPT_STATE(macro)                                                                \
                                                                                                 \
    /* first available state */                                                                  \
    macro (ompt_state_undefined, 0x102)      /* undefined thread state */                        \
                                                                                                 \
    /* work states (0..15) */                                                                    \
    macro (ompt_state_work_serial, 0x000)    /* working outside parallel */                      \
    macro (ompt_state_work_parallel, 0x001)  /* working within parallel */                       \
    macro (ompt_state_work_reduction, 0x002) /* performing a reduction */                        \
                                                                                                 \
    /* barrier wait states (16..31) */                                                           \
    macro (ompt_state_wait_barrier, 0x010)   /* waiting at a barrier */                          \
    macro (ompt_state_wait_barrier_implicit_parallel, 0x011)                                     \
                                            /* implicit barrier at the end of parallel region */ \
    macro (ompt_state_wait_barrier_implicit_workshare, 0x012)                                    \
                                            /* implicit barrier at the end of worksharing */     \
    macro (ompt_state_wait_barrier_implicit, 0x013)  /* implicit barrier */                      \
    macro (ompt_state_wait_barrier_explicit, 0x014)  /* explicit barrier */                      \
                                                                                                 \
    /* task wait states (32..63) */                                                              \
    macro (ompt_state_wait_taskwait, 0x020)  /* waiting at a taskwait */                         \
    macro (ompt_state_wait_taskgroup, 0x021) /* waiting at a taskgroup */                        \
                                                                                                 \
    /* mutex wait states (64..127) */                                                            \
    macro (ompt_state_wait_mutex, 0x040)                                                         \
    macro (ompt_state_wait_lock, 0x041)      /* waiting for lock */                              \
    macro (ompt_state_wait_critical, 0x042)  /* waiting for critical */                          \
    macro (ompt_state_wait_atomic, 0x043)    /* waiting for atomic */                            \
    macro (ompt_state_wait_ordered, 0x044)   /* waiting for ordered */                           \
                                                                                                 \
    /* target wait states (128..255) */                                                          \
    macro (ompt_state_wait_target, 0x080)        /* waiting for target region */                 \
    macro (ompt_state_wait_target_map, 0x081)    /* waiting for target data mapping operation */ \
    macro (ompt_state_wait_target_update, 0x082) /* waiting for target update operation */       \
                                                                                                 \
    /* misc (256..511) */                                                                        \
    macro (ompt_state_idle, 0x100)           /* waiting for work */                              \
    macro (ompt_state_overhead, 0x101)       /* overhead excluding wait states */                \
                                                                                                 \
    /* implementation-specific states (512..) */


#define FOREACH_KMP_MUTEX_IMPL(macro)                                                \
    macro (kmp_mutex_impl_none, 0)         /* unknown implementation */              \
    macro (kmp_mutex_impl_spin, 1)         /* based on spin */                       \
    macro (kmp_mutex_impl_queuing, 2)      /* based on some fair policy */           \
    macro (kmp_mutex_impl_speculative, 3)  /* based on HW-supported speculation */

#define FOREACH_OMPT_EVENT(macro)                                                                                        \
                                                                                                                         \
    /*--- Mandatory Events ---*/                                                                                         \
    macro (ompt_callback_thread_begin,      ompt_callback_thread_begin_t,       1) /* thread begin                    */ \
    macro (ompt_callback_thread_end,        ompt_callback_thread_end_t,         2) /* thread end                      */ \
                                                                                                                         \
    macro (ompt_callback_parallel_begin,    ompt_callback_parallel_begin_t,     3) /* parallel begin                  */ \
    macro (ompt_callback_parallel_end,      ompt_callback_parallel_end_t,       4) /* parallel end                    */ \
                                                                                                                         \
    macro (ompt_callback_task_create,       ompt_callback_task_create_t,        5) /* task begin                      */ \
    macro (ompt_callback_task_schedule,     ompt_callback_task_schedule_t,      6) /* task schedule                   */ \
    macro (ompt_callback_implicit_task,     ompt_callback_implicit_task_t,      7) /* implicit task                   */ \
                                                                                                                         \
    macro (ompt_callback_target,            ompt_callback_target_t,             8) /* target                          */ \
    macro (ompt_callback_target_data_op,    ompt_callback_target_data_op_t,     9) /* target data op                  */ \
    macro (ompt_callback_target_submit,     ompt_callback_target_submit_t,     10) /* target  submit                  */ \
                                                                                                                         \
    macro (ompt_callback_control_tool,      ompt_callback_control_tool_t,      11) /* control tool                    */ \
                                                                                                                         \
    macro (ompt_callback_device_initialize, ompt_callback_device_initialize_t, 12) /* device initialize               */ \
    macro (ompt_callback_device_finalize,   ompt_callback_device_finalize_t,   13) /* device finalize                 */ \
                                                                                                                         \
    macro (ompt_callback_device_load,       ompt_callback_device_load_t,       14) /* device load                     */ \
    macro (ompt_callback_device_unload,     ompt_callback_device_unload_t,     15) /* device unload                   */ \
                                                                                                                         \
    /* Optional Events */                                                                                                \
    macro (ompt_callback_sync_region_wait,  ompt_callback_sync_region_t,       16) /* sync region wait begin or end   */ \
                                                                                                                         \
    macro (ompt_callback_mutex_released,    ompt_callback_mutex_t,             17) /* mutex released                  */ \
                                                                                                                         \
    macro (ompt_callback_dependences,       ompt_callback_dependences_t,       18) /* report task/doacross dependences*/ \
    macro (ompt_callback_task_dependence,   ompt_callback_task_dependence_t,   19) /* report task dependence          */ \
                                                                                                                         \
    macro (ompt_callback_work,              ompt_callback_work_t,              20) /* task at work begin or end       */ \
                                                                                                                         \
    macro (ompt_callback_master,            ompt_callback_master_t,            21) /* task at master begin or end     */ \
                                                                                                                         \
    macro (ompt_callback_target_map,        ompt_callback_target_map_t,        22) /* target map                      */ \
                                                                                                                         \
    macro (ompt_callback_sync_region,       ompt_callback_sync_region_t,       23) /* sync region begin or end        */ \
                                                                                                                         \
    macro (ompt_callback_lock_init,         ompt_callback_mutex_acquire_t,     24) /* lock init                       */ \
    macro (ompt_callback_lock_destroy,      ompt_callback_mutex_t,             25) /* lock destroy                    */ \
                                                                                                                         \
    macro (ompt_callback_mutex_acquire,     ompt_callback_mutex_acquire_t,     26) /* mutex acquire                   */ \
    macro (ompt_callback_mutex_acquired,    ompt_callback_mutex_t,             27) /* mutex acquired                  */ \
                                                                                                                         \
    macro (ompt_callback_nest_lock,         ompt_callback_nest_lock_t,         28) /* nest lock                       */ \
                                                                                                                         \
    macro (ompt_callback_flush,             ompt_callback_flush_t,             29) /* after executing flush           */ \
                                                                                                                         \
    macro (ompt_callback_cancel,            ompt_callback_cancel_t,            30) /* cancel innermost binding region */ \
                                                                                                                         \
    macro (ompt_callback_reduction,         ompt_callback_sync_region_t,       31) /* reduction begin or end          */ \
                                                                                                                         \
    macro (ompt_callback_dispatch,          ompt_callback_dispatch_t,          32) /* loop iteration or section       */

/*****************************************************************************
 * implementation specific types
 *****************************************************************************/

typedef enum kmp_mutex_impl_t {
#define kmp_mutex_impl_macro(impl, code) impl = code,
    FOREACH_KMP_MUTEX_IMPL(kmp_mutex_impl_macro)
#undef kmp_mutex_impl_macro
} kmp_mutex_impl_t;

/*****************************************************************************
 * definitions generated from spec
 *****************************************************************************/

typedef enum ompt_callbacks_t {
  ompt_callback_thread_begin             = 1,
  ompt_callback_thread_end               = 2,
  ompt_callback_parallel_begin           = 3,
  ompt_callback_parallel_end             = 4,
  ompt_callback_task_create              = 5,
  ompt_callback_task_schedule            = 6,
  ompt_callback_implicit_task            = 7,
  ompt_callback_target                   = 8,
  ompt_callback_target_data_op           = 9,
  ompt_callback_target_submit            = 10,
  ompt_callback_control_tool             = 11,
  ompt_callback_device_initialize        = 12,
  ompt_callback_device_finalize          = 13,
  ompt_callback_device_load              = 14,
  ompt_callback_device_unload            = 15,
  ompt_callback_sync_region_wait         = 16,
  ompt_callback_mutex_released           = 17,
  ompt_callback_dependences              = 18,
  ompt_callback_task_dependence          = 19,
  ompt_callback_work                     = 20,
  ompt_callback_master                   = 21,
  ompt_callback_target_map               = 22,
  ompt_callback_sync_region              = 23,
  ompt_callback_lock_init                = 24,
  ompt_callback_lock_destroy             = 25,
  ompt_callback_mutex_acquire            = 26,
  ompt_callback_mutex_acquired           = 27,
  ompt_callback_nest_lock                = 28,
  ompt_callback_flush                    = 29,
  ompt_callback_cancel                   = 30,
  ompt_callback_reduction                = 31,
  ompt_callback_dispatch                 = 32
} ompt_callbacks_t;

typedef enum ompt_record_t {
  ompt_record_ompt               = 1,
  ompt_record_native             = 2,
  ompt_record_invalid            = 3
} ompt_record_t;

typedef enum ompt_record_native_t {
  ompt_record_native_info  = 1,
  ompt_record_native_event = 2
} ompt_record_native_t;

typedef enum ompt_set_result_t {
  ompt_set_error            = 0,
  ompt_set_never            = 1,
  ompt_set_impossible       = 2,
  ompt_set_sometimes        = 3,
  ompt_set_sometimes_paired = 4,
  ompt_set_always           = 5
} ompt_set_result_t;

typedef uint64_t ompt_id_t;

typedef uint64_t ompt_device_time_t;

typedef uint64_t ompt_buffer_cursor_t;

typedef enum ompt_thread_t {
  ompt_thread_initial                 = 1,
  ompt_thread_worker                  = 2,
  ompt_thread_other                   = 3,
  ompt_thread_unknown                 = 4
} ompt_thread_t;

typedef enum ompt_scope_endpoint_t {
  ompt_scope_begin                    = 1,
  ompt_scope_end                      = 2
} ompt_scope_endpoint_t;

typedef enum ompt_dispatch_t {
  ompt_dispatch_iteration             = 1,
  ompt_dispatch_section               = 2
} ompt_dispatch_t;

typedef enum ompt_sync_region_t {
  ompt_sync_region_barrier                = 1,
  ompt_sync_region_barrier_implicit       = 2,
  ompt_sync_region_barrier_explicit       = 3,
  ompt_sync_region_barrier_implementation = 4,
  ompt_sync_region_taskwait               = 5,
  ompt_sync_region_taskgroup              = 6,
  ompt_sync_region_reduction              = 7
} ompt_sync_region_t;

typedef enum ompt_target_data_op_t {
  ompt_target_data_alloc                = 1,
  ompt_target_data_transfer_to_device   = 2,
  ompt_target_data_transfer_from_device = 3,
  ompt_target_data_delete               = 4,
  ompt_target_data_associate            = 5,
  ompt_target_data_disassociate         = 6
} ompt_target_data_op_t;

typedef enum ompt_work_t {
  ompt_work_loop               = 1,
  ompt_work_sections           = 2,
  ompt_work_single_executor    = 3,
  ompt_work_single_other       = 4,
  ompt_work_workshare          = 5,
  ompt_work_distribute         = 6,
  ompt_work_taskloop           = 7
} ompt_work_t;

typedef enum ompt_mutex_t {
  ompt_mutex_lock                     = 1,
  ompt_mutex_test_lock                = 2,
  ompt_mutex_nest_lock                = 3,
  ompt_mutex_test_nest_lock           = 4,
  ompt_mutex_critical                 = 5,
  ompt_mutex_atomic                   = 6,
  ompt_mutex_ordered                  = 7
} ompt_mutex_t;

typedef enum ompt_native_mon_flag_t {
  ompt_native_data_motion_explicit    = 0x01,
  ompt_native_data_motion_implicit    = 0x02,
  ompt_native_kernel_invocation       = 0x04,
  ompt_native_kernel_execution        = 0x08,
  ompt_native_driver                  = 0x10,
  ompt_native_runtime                 = 0x20,
  ompt_native_overhead                = 0x40,
  ompt_native_idleness                = 0x80
} ompt_native_mon_flag_t;

typedef enum ompt_task_flag_t {
  ompt_task_initial                   = 0x00000001,
  ompt_task_implicit                  = 0x00000002,
  ompt_task_explicit                  = 0x00000004,
  ompt_task_target                    = 0x00000008,
  ompt_task_undeferred                = 0x08000000,
  ompt_task_untied                    = 0x10000000,
  ompt_task_final                     = 0x20000000,
  ompt_task_mergeable                 = 0x40000000,
  ompt_task_merged                    = 0x80000000
} ompt_task_flag_t;

typedef enum ompt_task_status_t {
  ompt_task_complete      = 1,
  ompt_task_yield         = 2,
  ompt_task_cancel        = 3,
  ompt_task_detach        = 4,
  ompt_task_early_fulfill = 5,
  ompt_task_late_fulfill  = 6,
  ompt_task_switch        = 7
} ompt_task_status_t;

typedef enum ompt_target_t {
  ompt_target                         = 1,
  ompt_target_enter_data              = 2,
  ompt_target_exit_data               = 3,
  ompt_target_update                  = 4
} ompt_target_t;

typedef enum ompt_parallel_flag_t {
  ompt_parallel_invoker_program = 0x00000001,
  ompt_parallel_invoker_runtime = 0x00000002,
  ompt_parallel_league          = 0x40000000,
  ompt_parallel_team            = 0x80000000
} ompt_parallel_flag_t;

typedef enum ompt_target_map_flag_t {
  ompt_target_map_flag_to             = 0x01,
  ompt_target_map_flag_from           = 0x02,
  ompt_target_map_flag_alloc          = 0x04,
  ompt_target_map_flag_release        = 0x08,
  ompt_target_map_flag_delete         = 0x10,
  ompt_target_map_flag_implicit       = 0x20
} ompt_target_map_flag_t;

typedef enum ompt_dependence_type_t {
  ompt_dependence_type_in              = 1,
  ompt_dependence_type_out             = 2,
  ompt_dependence_type_inout           = 3,
  ompt_dependence_type_mutexinoutset   = 4,
  ompt_dependence_type_source          = 5,
  ompt_dependence_type_sink            = 6
} ompt_dependence_type_t;

typedef enum ompt_cancel_flag_t {
  ompt_cancel_parallel       = 0x01,
  ompt_cancel_sections       = 0x02,
  ompt_cancel_loop           = 0x04,
  ompt_cancel_taskgroup      = 0x08,
  ompt_cancel_activated      = 0x10,
  ompt_cancel_detected       = 0x20,
  ompt_cancel_discarded_task = 0x40
} ompt_cancel_flag_t;

typedef uint64_t ompt_hwid_t;

typedef uint64_t ompt_wait_id_t;

typedef enum ompt_frame_flag_t {
  ompt_frame_runtime        = 0x00,
  ompt_frame_application    = 0x01,
  ompt_frame_cfa            = 0x10,
  ompt_frame_framepointer   = 0x20,
  ompt_frame_stackaddress   = 0x30
} ompt_frame_flag_t;

typedef enum ompt_state_t {
  ompt_state_work_serial                      = 0x000,
  ompt_state_work_parallel                    = 0x001,
  ompt_state_work_reduction                   = 0x002,

  ompt_state_wait_barrier                     = 0x010,
  ompt_state_wait_barrier_implicit_parallel   = 0x011,
  ompt_state_wait_barrier_implicit_workshare  = 0x012,
  ompt_state_wait_barrier_implicit            = 0x013,
  ompt_state_wait_barrier_explicit            = 0x014,

  ompt_state_wait_taskwait                    = 0x020,
  ompt_state_wait_taskgroup                   = 0x021,

  ompt_state_wait_mutex                       = 0x040,
  ompt_state_wait_lock                        = 0x041,
  ompt_state_wait_critical                    = 0x042,
  ompt_state_wait_atomic                      = 0x043,
  ompt_state_wait_ordered                     = 0x044,

  ompt_state_wait_target                      = 0x080,
  ompt_state_wait_target_map                  = 0x081,
  ompt_state_wait_target_update               = 0x082,

  ompt_state_idle                             = 0x100,
  ompt_state_overhead                         = 0x101,
  ompt_state_undefined                        = 0x102
} ompt_state_t;

typedef uint64_t (*ompt_get_unique_id_t) (void);

typedef uint64_t ompd_size_t;

typedef uint64_t ompd_wait_id_t;

typedef uint64_t ompd_addr_t;
typedef int64_t  ompd_word_t;
typedef uint64_t ompd_seg_t;

typedef uint64_t ompd_device_t;

typedef uint64_t ompd_thread_id_t;

typedef enum ompd_scope_t {
  ompd_scope_global = 1,
  ompd_scope_address_space = 2,
  ompd_scope_thread = 3,
  ompd_scope_parallel = 4,
  ompd_scope_implicit_task = 5,
  ompd_scope_task = 6
} ompd_scope_t;

typedef uint64_t ompd_icv_id_t;

typedef enum ompd_rc_t {
  ompd_rc_ok = 0,
  ompd_rc_unavailable = 1,
  ompd_rc_stale_handle = 2,
  ompd_rc_bad_input = 3,
  ompd_rc_error = 4,
  ompd_rc_unsupported = 5,
  ompd_rc_needs_state_tracking = 6,
  ompd_rc_incompatible = 7,
  ompd_rc_device_read_error = 8,
  ompd_rc_device_write_error = 9,
  ompd_rc_nomem = 10,
} ompd_rc_t;

typedef void (*ompt_interface_fn_t) (void);

typedef ompt_interface_fn_t (*ompt_function_lookup_t) (
  const char *interface_function_name
);

typedef union ompt_data_t {
  uint64_t value;
  void *ptr;
} ompt_data_t;

typedef struct ompt_frame_t {
  ompt_data_t exit_frame;
  ompt_data_t enter_frame;
  int exit_frame_flags;
  int enter_frame_flags;
} ompt_frame_t;

typedef void (*ompt_callback_t) (void);

typedef void ompt_device_t;

typedef void ompt_buffer_t;

typedef void (*ompt_callback_buffer_request_t) (
  int device_num,
  ompt_buffer_t **buffer,
  size_t *bytes
);

typedef void (*ompt_callback_buffer_complete_t) (
  int device_num,
  ompt_buffer_t *buffer,
  size_t bytes,
  ompt_buffer_cursor_t begin,
  int buffer_owned
);

typedef void (*ompt_finalize_t) (
  ompt_data_t *tool_data
);

typedef int (*ompt_initialize_t) (
  ompt_function_lookup_t lookup,
  int initial_device_num,
  ompt_data_t *tool_data
);

typedef struct ompt_start_tool_result_t {
  ompt_initialize_t initialize;
  ompt_finalize_t finalize;
  ompt_data_t tool_data;
} ompt_start_tool_result_t;

typedef struct ompt_record_abstract_t {
  ompt_record_native_t rclass;
  const char *type;
  ompt_device_time_t start_time;
  ompt_device_time_t end_time;
  ompt_hwid_t hwid;
} ompt_record_abstract_t;

typedef struct ompt_dependence_t {
  ompt_data_t variable;
  ompt_dependence_type_t dependence_type;
} ompt_dependence_t;

typedef int (*ompt_enumerate_states_t) (
  int current_state,
  int *next_state,
  const char **next_state_name
);

typedef int (*ompt_enumerate_mutex_impls_t) (
  int current_impl,
  int *next_impl,
  const char **next_impl_name
);

typedef ompt_set_result_t (*ompt_set_callback_t) (
  ompt_callbacks_t event,
  ompt_callback_t callback
);

typedef int (*ompt_get_callback_t) (
  ompt_callbacks_t event,
  ompt_callback_t *callback
);

typedef ompt_data_t *(*ompt_get_thread_data_t) (void);

typedef int (*ompt_get_num_procs_t) (void);

typedef int (*ompt_get_num_places_t) (void);

typedef int (*ompt_get_place_proc_ids_t) (
  int place_num,
  int ids_size,
  int *ids
);

typedef int (*ompt_get_place_num_t) (void);

typedef int (*ompt_get_partition_place_nums_t) (
  int place_nums_size,
  int *place_nums
);

typedef int (*ompt_get_proc_id_t) (void);

typedef int (*ompt_get_state_t) (
  ompt_wait_id_t *wait_id
);

typedef int (*ompt_get_parallel_info_t) (
  int ancestor_level,
  ompt_data_t **parallel_data,
  int *team_size
);

typedef int (*ompt_get_task_info_t) (
  int ancestor_level,
  int *flags,
  ompt_data_t **task_data,
  ompt_frame_t **task_frame,
  ompt_data_t **parallel_data,
  int *thread_num
);

typedef int (*ompt_get_task_memory_t)(
  void **addr,
  size_t *size,
  int block
);

typedef int (*ompt_get_target_info_t) (
  uint64_t *device_num,
  ompt_id_t *target_id,
  ompt_id_t *host_op_id
);

typedef int (*ompt_get_num_devices_t) (void);

typedef void (*ompt_finalize_tool_t) (void);

typedef int (*ompt_get_device_num_procs_t) (
  ompt_device_t *device
);

typedef ompt_device_time_t (*ompt_get_device_time_t) (
  ompt_device_t *device
);

typedef double (*ompt_translate_time_t) (
  ompt_device_t *device,
  ompt_device_time_t time
);

typedef ompt_set_result_t (*ompt_set_trace_ompt_t) (
  ompt_device_t *device,
  unsigned int enable,
  unsigned int etype
);

typedef ompt_set_result_t (*ompt_set_trace_native_t) (
  ompt_device_t *device,
  int enable,
  int flags
);

typedef int (*ompt_start_trace_t) (
  ompt_device_t *device,
  ompt_callback_buffer_request_t request,
  ompt_callback_buffer_complete_t complete
);

typedef int (*ompt_pause_trace_t) (
  ompt_device_t *device,
  int begin_pause
);

typedef int (*ompt_flush_trace_t) (
  ompt_device_t *device
);

typedef int (*ompt_stop_trace_t) (
  ompt_device_t *device
);

typedef int (*ompt_advance_buffer_cursor_t) (
  ompt_device_t *device,
  ompt_buffer_t *buffer,
  size_t size,
  ompt_buffer_cursor_t current,
  ompt_buffer_cursor_t *next
);

typedef ompt_record_t (*ompt_get_record_type_t) (
  ompt_buffer_t *buffer,
  ompt_buffer_cursor_t current
);

typedef void *(*ompt_get_record_native_t) (
  ompt_buffer_t *buffer,
  ompt_buffer_cursor_t current,
  ompt_id_t *host_op_id
);

typedef ompt_record_abstract_t *
(*ompt_get_record_abstract_t) (
  void *native_record
);

/// OMPT entry extensions
typedef int (*ompt_ext_get_num_teams_t) (
  ompt_id_t target_id
);
typedef int (*ompt_ext_get_thread_limit_t) (
  ompt_id_t target_id
);
typedef const char *(*ompt_ext_get_code_location_t) (
  const void *codeptr_ra
);

typedef void (*ompt_callback_thread_begin_t) (
  ompt_thread_t thread_type,
  ompt_data_t *thread_data
);

typedef struct ompt_record_thread_begin_t {
  ompt_thread_t thread_type;
} ompt_record_thread_begin_t;

typedef void (*ompt_callback_thread_end_t) (
  ompt_data_t *thread_data
);

typedef void (*ompt_callback_parallel_begin_t) (
  ompt_data_t *encountering_task_data,
  const ompt_frame_t *encountering_task_frame,
  ompt_data_t *parallel_data,
  unsigned int requested_parallelism,
  int flags,
  const void *codeptr_ra
);

typedef struct ompt_record_parallel_begin_t {
  ompt_id_t encountering_task_id;
  ompt_id_t parallel_id;
  unsigned int requested_parallelism;
  int flags;
  const void *codeptr_ra;
} ompt_record_parallel_begin_t;

typedef void (*ompt_callback_parallel_end_t) (
  ompt_data_t *parallel_data,
  ompt_data_t *encountering_task_data,
  int flags,
  const void *codeptr_ra
);

typedef struct ompt_record_parallel_end_t {
  ompt_id_t parallel_id;
  ompt_id_t encountering_task_id;
  int flags;
  const void *codeptr_ra;
} ompt_record_parallel_end_t;

typedef void (*ompt_callback_work_t) (
  ompt_work_t wstype,
  ompt_scope_endpoint_t endpoint,
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  uint64_t count,
  const void *codeptr_ra
);

typedef struct ompt_record_work_t {
  ompt_work_t wstype;
  ompt_scope_endpoint_t endpoint;
  ompt_id_t parallel_id;
  ompt_id_t task_id;
  uint64_t count;
  const void *codeptr_ra;
} ompt_record_work_t;

typedef void (*ompt_callback_dispatch_t) (
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  ompt_dispatch_t kind,
  ompt_data_t instance
);

typedef struct ompt_record_dispatch_t {
  ompt_id_t parallel_id;
  ompt_id_t task_id;
  ompt_dispatch_t kind;
  ompt_data_t instance;
} ompt_record_dispatch_t;

typedef void (*ompt_callback_task_create_t) (
  ompt_data_t *encountering_task_data,
  const ompt_frame_t *encountering_task_frame,
  ompt_data_t *new_task_data,
  int flags,
  int has_dependences,
  const void *codeptr_ra
);

typedef struct ompt_record_task_create_t {
  ompt_id_t encountering_task_id;
  ompt_id_t new_task_id;
  int flags;
  int has_dependences;
  const void *codeptr_ra;
} ompt_record_task_create_t;

typedef void (*ompt_callback_dependences_t) (
  ompt_data_t *task_data,
  const ompt_dependence_t *deps,
  int ndeps
);

typedef struct ompt_record_dependences_t {
  ompt_id_t task_id;
  ompt_dependence_t dep;
  int ndeps;
} ompt_record_dependences_t;

typedef void (*ompt_callback_task_dependence_t) (
  ompt_data_t *src_task_data,
  ompt_data_t *sink_task_data
);

typedef struct ompt_record_task_dependence_t {
  ompt_id_t src_task_id;
  ompt_id_t sink_task_id;
} ompt_record_task_dependence_t;

typedef void (*ompt_callback_task_schedule_t) (
  ompt_data_t *prior_task_data,
  ompt_task_status_t prior_task_status,
  ompt_data_t *next_task_data
);

typedef struct ompt_record_task_schedule_t {
  ompt_id_t prior_task_id;
  ompt_task_status_t prior_task_status;
  ompt_id_t next_task_id;
} ompt_record_task_schedule_t;

typedef void (*ompt_callback_implicit_task_t) (
  ompt_scope_endpoint_t endpoint,
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  unsigned int actual_parallelism,
  unsigned int index,
  int flags
);

typedef struct ompt_record_implicit_task_t {
  ompt_scope_endpoint_t endpoint;
  ompt_id_t parallel_id;
  ompt_id_t task_id;
  unsigned int actual_parallelism;
  unsigned int index;
  int flags;
} ompt_record_implicit_task_t;

typedef void (*ompt_callback_master_t) (
  ompt_scope_endpoint_t endpoint,
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  const void *codeptr_ra
);

typedef struct ompt_record_master_t {
  ompt_scope_endpoint_t endpoint;
  ompt_id_t parallel_id;
  ompt_id_t task_id;
  const void *codeptr_ra;
} ompt_record_master_t;

typedef void (*ompt_callback_sync_region_t) (
  ompt_sync_region_t kind,
  ompt_scope_endpoint_t endpoint,
  ompt_data_t *parallel_data,
  ompt_data_t *task_data,
  const void *codeptr_ra
);

typedef struct ompt_record_sync_region_t {
  ompt_sync_region_t kind;
  ompt_scope_endpoint_t endpoint;
  ompt_id_t parallel_id;
  ompt_id_t task_id;
  const void *codeptr_ra;
} ompt_record_sync_region_t;

typedef void (*ompt_callback_mutex_acquire_t) (
  ompt_mutex_t kind,
  unsigned int hint,
  unsigned int impl,
  ompt_wait_id_t wait_id,
  const void *codeptr_ra
);

typedef struct ompt_record_mutex_acquire_t {
  ompt_mutex_t kind;
  unsigned int hint;
  unsigned int impl;
  ompt_wait_id_t wait_id;
  const void *codeptr_ra;
} ompt_record_mutex_acquire_t;

typedef void (*ompt_callback_mutex_t) (
  ompt_mutex_t kind,
  ompt_wait_id_t wait_id,
  const void *codeptr_ra
);

typedef struct ompt_record_mutex_t {
  ompt_mutex_t kind;
  ompt_wait_id_t wait_id;
  const void *codeptr_ra;
} ompt_record_mutex_t;

typedef void (*ompt_callback_nest_lock_t) (
  ompt_scope_endpoint_t endpoint,
  ompt_wait_id_t wait_id,
  const void *codeptr_ra
);

typedef struct ompt_record_nest_lock_t {
  ompt_scope_endpoint_t endpoint;
  ompt_wait_id_t wait_id;
  const void *codeptr_ra;
} ompt_record_nest_lock_t;

typedef void (*ompt_callback_flush_t) (
  ompt_data_t *thread_data,
  const void *codeptr_ra
);

typedef struct ompt_record_flush_t {
  const void *codeptr_ra;
} ompt_record_flush_t;

typedef void (*ompt_callback_cancel_t) (
  ompt_data_t *task_data,
  int flags,
  const void *codeptr_ra
);

typedef struct ompt_record_cancel_t {
  ompt_id_t task_id;
  int flags;
  const void *codeptr_ra;
} ompt_record_cancel_t;

typedef void (*ompt_callback_device_initialize_t) (
  int device_num,
  const char *type,
  ompt_device_t *device,
  ompt_function_lookup_t lookup,
  const char *documentation
);

typedef void (*ompt_callback_device_finalize_t) (
  int device_num
);

typedef void (*ompt_callback_device_load_t) (
  int device_num,
  const char *filename,
  int64_t offset_in_file,
  void *vma_in_file,
  size_t bytes,
  void *host_addr,
  void *device_addr,
  uint64_t module_id
);

typedef void (*ompt_callback_device_unload_t) (
  int device_num,
  uint64_t module_id
);

// TODO: "endpoint" is non-conforming extension proposed for 5.1
typedef void (*ompt_callback_target_data_op_t) (
  ompt_scope_endpoint_t endpoint,
  ompt_id_t target_id,
  ompt_id_t host_op_id,
  ompt_target_data_op_t optype,
  void *src_addr,
  int src_device_num,
  void *dest_addr,
  int dest_device_num,
  size_t bytes,
  const void *codeptr_ra
);

typedef struct ompt_record_target_data_op_t {
  ompt_id_t host_op_id;
  ompt_target_data_op_t optype;
  void *src_addr;
  int src_device_num;
  void *dest_addr;
  int dest_device_num;
  size_t bytes;
  ompt_device_time_t end_time;
  const void *codeptr_ra;
} ompt_record_target_data_op_t;

typedef void (*ompt_callback_target_t) (
  ompt_target_t kind,
  ompt_scope_endpoint_t endpoint,
  int device_num,
  ompt_data_t *task_data,
  ompt_id_t target_id,
  const void *codeptr_ra
);

typedef struct ompt_record_target_t {
  ompt_target_t kind;
  ompt_scope_endpoint_t endpoint;
  int device_num;
  ompt_id_t task_id;
  ompt_id_t target_id;
  const void *codeptr_ra;
} ompt_record_target_t;

typedef void (*ompt_callback_target_map_t) (
  ompt_id_t target_id,
  unsigned int nitems,
  void **host_addr,
  void **device_addr,
  size_t *bytes,
  unsigned int *mapping_flags,
  const void *codeptr_ra
);

typedef struct ompt_record_target_map_t {
  ompt_id_t target_id;
  unsigned int nitems;
  void **host_addr;
  void **device_addr;
  size_t *bytes;
  unsigned int *mapping_flags;
  const void *codeptr_ra;
} ompt_record_target_map_t;

// TODO: "endpoint" is non-conforming extension proposed for 5.1
typedef void (*ompt_callback_target_submit_t) (
  ompt_scope_endpoint_t endpoint,
  ompt_id_t target_id,
  ompt_id_t host_op_id,
  unsigned int requested_num_teams
);

typedef struct ompt_record_target_kernel_t {
  ompt_id_t host_op_id;
  unsigned int requested_num_teams;
  unsigned int granted_num_teams;
  ompt_device_time_t end_time;
} ompt_record_target_kernel_t;

typedef int (*ompt_callback_control_tool_t) (
  uint64_t command,
  uint64_t modifier,
  void *arg,
  const void *codeptr_ra
);

typedef struct ompt_record_control_tool_t {
  uint64_t command;
  uint64_t modifier;
  const void *codeptr_ra;
} ompt_record_control_tool_t;

typedef struct ompd_address_t {
  ompd_seg_t segment;
  ompd_addr_t address;
} ompd_address_t;

typedef struct ompd_frame_info_t {
  ompd_address_t frame_address;
  ompd_word_t frame_flag;
} ompd_frame_info_t;

typedef struct _ompd_aspace_handle ompd_address_space_handle_t;
typedef struct _ompd_thread_handle ompd_thread_handle_t;
typedef struct _ompd_parallel_handle ompd_parallel_handle_t;
typedef struct _ompd_task_handle ompd_task_handle_t;

typedef struct _ompd_aspace_cont ompd_address_space_context_t;
typedef struct _ompd_thread_cont ompd_thread_context_t;

typedef struct ompd_device_type_sizes_t {
  uint8_t sizeof_char;
  uint8_t sizeof_short;
  uint8_t sizeof_int;
  uint8_t sizeof_long;
  uint8_t sizeof_long_long;
  uint8_t sizeof_pointer;
} ompd_device_type_sizes_t;

typedef struct ompt_record_ompt_t {
  ompt_callbacks_t type;
  ompt_device_time_t time;
  ompt_id_t thread_id;
  ompt_id_t target_id;
  union {
    ompt_record_thread_begin_t thread_begin;
    ompt_record_parallel_begin_t parallel_begin;
    ompt_record_parallel_end_t parallel_end;
    ompt_record_work_t work;
    ompt_record_dispatch_t dispatch;
    ompt_record_task_create_t task_create;
    ompt_record_dependences_t dependences;
    ompt_record_task_dependence_t task_dependence;
    ompt_record_task_schedule_t task_schedule;
    ompt_record_implicit_task_t implicit_task;
    ompt_record_master_t master;
    ompt_record_sync_region_t sync_region;
    ompt_record_mutex_acquire_t mutex_acquire;
    ompt_record_mutex_t mutex;
    ompt_record_nest_lock_t nest_lock;
    ompt_record_flush_t flush;
    ompt_record_cancel_t cancel;
    ompt_record_target_t target;
    ompt_record_target_data_op_t target_data_op;
    ompt_record_target_map_t target_map;
    ompt_record_target_kernel_t target_kernel;
    ompt_record_control_tool_t control_tool;
  } record;
} ompt_record_ompt_t;

typedef ompt_record_ompt_t *(*ompt_get_record_ompt_t) (
  ompt_buffer_t *buffer,
  ompt_buffer_cursor_t current
);

#define ompt_id_none 0
#define ompt_data_none {0}
#define ompt_time_none 0
#define ompt_hwid_none 0
#define ompt_addr_none ~0
#define ompt_mutex_impl_none 0
#define ompt_wait_id_none 0

#define ompd_segment_none 0
#define ompd_icv_undefined 0

#ifdef __cplusplus
extern "C"
#endif
#ifdef _WIN32
__declspec(dllexport)
#endif
ompt_start_tool_result_t *ompt_start_tool(
  unsigned int omp_version,
  const char * runtime_version
);

/*****************************************************************************
 * OMPD ICV list
 *****************************************************************************/

/* supported ICV */
#define FOREACH_OMPD_ICV(macro)                                                \
    macro(undef, undef, global, 0)                                             \
    macro(dyn, dyn-var, task, 1)                                               \
    macro(nthreads, nthreads-var, task, 3)                                     \
    macro(run_sched, run-sched-var, task, 4)                                   \
    macro(bind, bind-var, task, 5)                                             \
    macro(thread_limit, thread-limit-var, address_space, 6)                    \
    macro(max_active_levels, max-active-levels-var, task, 7)                   \
    macro(active_levels, active-levels-var, parallel, 8)                       \
    macro(levels, levels-var, parallel, 9)                                     \
    macro(cancel, cancel-var, global, 10)                                      \
    macro(affinity_format, affinity-format-var, address_space, 11)             \
    macro(default_device, default-device-var, task, 12)                        \
    macro(max_task_priority, max-task-priority-var, global, 13)                \
    macro(ompd_num_procs, ompd-num-procs-var, address_space, 14)               \
    macro(ompd_thread_num, ompd-thread-num-var, thread, 15)                    \
    macro(ompd_final, ompd-final-var, task, 16)                                \
    macro(ompd_implicit, ompd-implicit-var, task, 17)                          \
    macro(ompd_team_size, ompd-team-size-var, parallel, 18)

/*****************************************************************************
 * OMPD recommended device/thread definitions (copied from ompd_types.h)
 *****************************************************************************/

#define OMPD_TYPES_VERSION   20180906 /* YYYYMMDD Format */

/* Kinds of device threads  */
#define OMPD_THREAD_ID_PTHREAD      ((ompd_thread_id_t)0)
#define OMPD_THREAD_ID_LWP          ((ompd_thread_id_t)1)
#define OMPD_THREAD_ID_WINTHREAD    ((ompd_thread_id_t)2)
#define OMPD_THREAD_ID_CUDALOGICAL  ((ompd_thread_id_t)3)
/* The range of non-standard implementation defined values */
#define OMPD_THREAD_ID_LO       ((ompd_thread_id_t)1000000)
#define OMPD_THREAD_ID_HI       ((ompd_thread_id_t)1100000)

/* Memory Access Segment definitions for Host and Target Devices */
#define OMPD_SEGMENT_UNSPECIFIED             ((ompd_seg_t)0)
/* OMPD_SEGMENT* constants were taken from 0 to 15 */

/* Kinds of device device address spaces */
#define OMPD_DEVICE_KIND_HOST     ((ompd_device_t)1)
#define OMPD_DEVICE_KIND_CUDA     ((ompd_device_t)2)
/* The range of non-standard implementation defined values */
#define OMPD_DEVICE_IMPL_LO       ((ompd_device_t)1000000)
#define OMPD_DEVICE_IMPL_HI       ((ompd_device_t)1100000)

/*****************************************************************************
 * OMPD Tool Callback Interface
 *****************************************************************************/

typedef ompd_rc_t (*ompd_callback_memory_alloc_fn_t)(
    ompd_size_t nbytes,
    void **ptr
);

typedef ompd_rc_t (*ompd_callback_memory_free_fn_t)(
    void *ptr
);

typedef ompd_rc_t (*ompd_callback_get_thread_context_for_thread_id_fn_t)(
    ompd_address_space_context_t *address_space_context,
    ompd_thread_id_t kind,
    ompd_size_t sizeof_thread_id,
    const void *thread_id,
    ompd_thread_context_t **thread_context
);

typedef ompd_rc_t (*ompd_callback_sizeof_fn_t)(
    ompd_address_space_context_t *address_space_context,
    ompd_device_type_sizes_t *sizes
);

typedef ompd_rc_t (*ompd_callback_symbol_addr_fn_t)(
    ompd_address_space_context_t *address_space_context,
    ompd_thread_context_t *thread_context,
    const char *symbol_name,
    ompd_address_t *symbol_addr,
    const char *file_name
);

typedef ompd_rc_t (*ompd_callback_memory_read_fn_t)(
    ompd_address_space_context_t *address_space_context,
    ompd_thread_context_t *thread_context,
    const ompd_address_t *addr,
    ompd_size_t nbytes,
    void *buffer
);

typedef ompd_rc_t (*ompd_callback_memory_write_fn_t)(
    ompd_address_space_context_t *address_space_context,
    ompd_thread_context_t *thread_context,
    const ompd_address_t *addr,
    ompd_size_t nbytes,
    const void *buffer
);

typedef ompd_rc_t (*ompd_callback_device_host_fn_t)(
    ompd_address_space_context_t *address_space_context,
    const void *input,
    ompd_size_t unit_size,
    ompd_size_t count,
    void *output
);

typedef ompd_rc_t (*ompd_callback_print_string_fn_t)(
    const char *string,
    int category
);

typedef struct {
    ompd_callback_memory_alloc_fn_t alloc_memory;
    ompd_callback_memory_free_fn_t free_memory;
    ompd_callback_print_string_fn_t print_string;
    ompd_callback_sizeof_fn_t sizeof_type;
    ompd_callback_symbol_addr_fn_t symbol_addr_lookup;
    ompd_callback_memory_read_fn_t read_memory;
    ompd_callback_memory_write_fn_t write_memory;
    ompd_callback_memory_read_fn_t read_string;
    ompd_callback_device_host_fn_t device_to_host;
    ompd_callback_device_host_fn_t host_to_device;
    ompd_callback_get_thread_context_for_thread_id_fn_t
        get_thread_context_for_thread_id;
} ompd_callbacks_t;


/*****************************************************************************
 * OMPD Tool Interface Routines
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define OMPD_EXPORT __declspec(dllexport)
#else
#define OMPD_EXPORT
#endif

/* initializes the OMPD library */
OMPD_EXPORT
ompd_rc_t ompd_initialize(
    ompd_word_t api_version,
    const ompd_callbacks_t *callbacks
);

/* returns the OMPD API version */
OMPD_EXPORT
ompd_rc_t ompd_get_api_version(ompd_word_t *version);

/* returns a descriptive string for the OMPD API version */
OMPD_EXPORT
ompd_rc_t ompd_get_version_string(const char **string);

/* finalizes the OMPD library */
OMPD_EXPORT
ompd_rc_t ompd_finalize(void);

/* initializes a live sesson on a live process or core file */
OMPD_EXPORT
ompd_rc_t ompd_process_initialize(
    ompd_address_space_context_t *context,
    ompd_address_space_handle_t **handle
);

/* obtains an adress space handle for a device */
OMPD_EXPORT
ompd_rc_t ompd_device_initialize(
    ompd_address_space_handle_t *process_handle,
    ompd_address_space_context_t *device_context,
    ompd_device_t kind,
    ompd_size_t sizeof_id,
    void *id,
    ompd_address_space_handle_t **device_handle
);

/* releases an address space handle */
OMPD_EXPORT
ompd_rc_t ompd_rel_address_space_handle(
    ompd_address_space_handle_t *handle
);

/* obtains the version of the OpenMP API associated with an address space */
OMPD_EXPORT
ompd_rc_t ompd_get_omp_version(
    ompd_address_space_handle_t *address_space,
    ompd_word_t *omp_version
);

/* returns a descriptive string for the OpenMP API version */
OMPD_EXPORT
ompd_rc_t ompd_get_omp_version_string(
    ompd_address_space_handle_t *address_space,
    const char **string
);

/* obtains a thread handle associated with a parallel region */
OMPD_EXPORT
ompd_rc_t ompd_get_thread_in_parallel(
    ompd_parallel_handle_t *prallel_handle,
    int thread_num,
    ompd_thread_handle_t **thread_handle
);

/* maps a native thread to an OMPD thread handle */
OMPD_EXPORT
ompd_rc_t ompd_get_thread_handle(
    ompd_address_space_handle_t *handle,
    ompd_thread_id_t kind,
    ompd_size_t sizeof_thread_id,
    const void *thread_id,
    ompd_thread_handle_t **thread_handle
);

/* releases a thread handle */
OMPD_EXPORT
ompd_rc_t ompd_rel_thread_handle(ompd_thread_handle_t *thread_handle);

/* compares two thread handles */
OMPD_EXPORT
ompd_rc_t ompd_thread_handle_compare(
    ompd_thread_handle_t *thread_handle_1,
    ompd_thread_handle_t *thread_handle_2,
    int *cmp_value
);

/* maps an OMPD thread handle to a native thread */
OMPD_EXPORT
ompd_rc_t ompd_get_thread_id(
    ompd_thread_handle_t *thread_handle,
    ompd_thread_id_t kind,
    ompd_size_t sizeof_thread_id,
    void *thread_id
);

/* obtains a parallel handle for the current parallel region associated with an
 * OpenMP thread */
OMPD_EXPORT
ompd_rc_t ompd_get_curr_parallel_handle(
    ompd_thread_handle_t *thread_handle,
    ompd_parallel_handle_t **parallel_handle
);

/* obtains a parallel handle for the parallel region that encloses the parallel
 * region specified by the input parallel handle */
OMPD_EXPORT
ompd_rc_t ompd_get_enclosing_parallel_handle(
    ompd_parallel_handle_t *parallel_handle,
    ompd_parallel_handle_t **enclosing_parallel_handle
);

/* obtains a parallel handle for the parallel region that encloses the task
 * region specified by the task handle */
OMPD_EXPORT
ompd_rc_t ompd_get_task_parallel_handle(
    ompd_task_handle_t *task_handle,
    ompd_parallel_handle_t **task_parallel_handle
);

/* releases a parallel region handle */
OMPD_EXPORT
ompd_rc_t ompd_rel_parallel_handle(ompd_parallel_handle_t *parallel_handle);

/* compares two parallel handles */
OMPD_EXPORT
ompd_rc_t ompd_parallel_handle_compare(
    ompd_parallel_handle_t *parallel_handle_1,
    ompd_parallel_handle_t *parallel_handle_2,
    int *cmp_value
);

/* obtains a task handle for the current task region associated with an OpenMP
 * thread */
OMPD_EXPORT
ompd_rc_t ompd_get_curr_task_handle(
    ompd_thread_handle_t *thread_handle,
    ompd_task_handle_t **task_handle
);

/* obtains a task handle for the task that encountered the OpenMP construct
 * which caused the specified task to be created */
OMPD_EXPORT
ompd_rc_t ompd_get_generating_task_handle(
    ompd_task_handle_t *task_handle,
    ompd_task_handle_t **generating_task_handle
);

/* obtains a task handle for the scheduling parent of the specified task */
OMPD_EXPORT
ompd_rc_t ompd_get_scheduling_task_handle(
    ompd_task_handle_t *task_handle,
    ompd_task_handle_t **scheduling_task_handle
);

/* obtains a task handle for the implicit tasks associated with a parallel
 * region */
OMPD_EXPORT
ompd_rc_t ompd_get_task_in_parallel(
    ompd_parallel_handle_t *parallel_handle,
    int thread_num,
    ompd_task_handle_t **task_handle
);

/* releases a task handle */
OMPD_EXPORT
ompd_rc_t ompd_rel_task_handle(ompd_task_handle_t *task_handle);

/* compares two task handles */
OMPD_EXPORT
ompd_rc_t ompd_task_handle_compare(
    ompd_task_handle_t *task_handle_1,
    ompd_task_handle_t *task_handle_2,
    int *cmp_value
);

/* returns the entry point of a task region */
OMPD_EXPORT
ompd_rc_t ompd_get_task_function(
    ompd_task_handle_t *task_handle,
    ompd_address_t *entry_point
);

/* extracts the task's frame pointers maintained by an OpenMP implementation */
OMPD_EXPORT
ompd_rc_t ompd_get_task_frame(
    ompd_task_handle_t *task_handle,
    ompd_frame_info_t *exit_frame,
    ompd_frame_info_t *enter_frame
);

/* enumerates thread states supported by an OpenMP implementation */
OMPD_EXPORT
ompd_rc_t ompd_enumerate_states(
    ompd_address_space_handle_t *address_space_handle,
    ompd_word_t current_state,
    ompd_word_t *next_state,
    const char **next_state_name,
    ompd_word_t *more_enums
);

/* returns the state of an OpenMP thread */
OMPD_EXPORT
ompd_rc_t ompd_get_state(
    ompd_thread_handle_t *thread_handle,
    ompd_word_t *state,
    ompt_wait_id_t *wait_id
);

/* returns a list of name/value pairs for the OpenMP control variables */
OMPD_EXPORT
ompd_rc_t ompd_get_display_control_vars(
    ompd_address_space_handle_t *address_space_handle,
    const char *const **control_vars
);

/* releases a list of name/value pairs of OpenMP control variables */
OMPD_EXPORT
ompd_rc_t ompd_rel_display_control_vars(const char *const **control_vars);

/* enumerates ICVs supported by an OpenMP implementation */
OMPD_EXPORT
ompd_rc_t ompd_enumerate_icvs(
    ompd_address_space_handle_t *handle,
    ompd_icv_id_t current,
    ompd_icv_id_t *next_id,
    const char **next_icv_name,
    ompd_scope_t *next_scope,
    int *more
);

/* returns the value of an ICV as present in the provide scope */
OMPD_EXPORT
ompd_rc_t ompd_get_icv_from_scope(
    void *handle,
    ompd_scope_t scope,
    ompd_icv_id_t icv_id,
    ompd_word_t *icv_value
);

/* returns the string value of an ICV as present in the provided scope */
OMPD_EXPORT
ompd_rc_t ompd_get_icv_string_from_scope(
    void *handle,
    ompd_scope_t scope,
    ompd_icv_id_t icv_id,
    const char **icv_string
);

/* provides access to the OMPT data variable stored for each OpenMP scope */
OMPD_EXPORT
ompd_rc_t ompd_get_tool_data(
    void *handle,
    ompd_scope_t scope,
    ompd_word_t *value,
    ompd_address_t *ptr
);


/*****************************************************************************
 * Runtime Entry Points for OMPD
 *****************************************************************************/

/* breakpoint for all parallel region begins */
void ompd_bp_parallel_begin(void);

/* breakpoint for all parallel region ends */
void ompd_bp_parallel_end(void);

/* breakpoint for all task region begins */
void ompd_bp_task_begin(void);

/* breakpoint for all task region ends */
void ompd_bp_task_end(void);

/* breakpoint for all thread begins */
void ompd_bp_thread_begin(void);

/* breakpoint for all thread ends */
void ompd_bp_thread_end(void);

/* breakpoint for all device begins */
void ompd_bp_device_begin(void);

/* breakpoint for all device ends */
void ompd_bp_device_end(void);


/*****************************************************************************
 * Other global symbols for OMPD
 *****************************************************************************/

/* locations of compatible OMPD libraries */
extern const char **ompd_dll_locations;

/* program point that guarantees valid ompd_dll_locations */
void ompd_dll_locations_valid(void);

#undef OMPD_EXPORT

#ifdef __cplusplus
} // extern "C"
#endif

#endif // USE_LIBOMP_OMP_TOOLS_H

///
/// Data types shared between host runtime, libomptarget, and plugin
///

#ifndef __cplusplus
#error Requires a C++ compiler
#endif

#include <atomic>
#include <string>
#include <cstring>
#include <map>
#include <mutex>
#include "omptarget.h"

#define OMPT_SUCCESS 1
#define OMPT_FAIL 0
#ifndef HOST_DEVICE
#define HOST_DEVICE -10
#endif

extern "C" {
#ifdef _WIN32
int32_t __cdecl __kmpc_global_thread_num(void *);
void __kmpc_get_ompt_callbacks(void **, void **);
#else
int32_t __kmpc_global_thread_num(void *) __attribute__((weak));
void __kmpc_get_ompt_callbacks(void **, void **) __attribute__((weak));
#endif
}

// Internal data structure to store callbacks
typedef struct {
#define OMPT_EVENT_FN(event, callback, event_id)                               \
  callback event;
  FOREACH_OMPT_EVENT(OMPT_EVENT_FN)
#undef OMPT_EVENT_FN
} OmptCallbacksTy;

// Internal data structure to store callbacks' state
typedef struct {
  uint32_t enabled : 1;
#define OMPT_EVENT_FN(event, callback, event_id)                               \
  uint32_t event : 1;
  FOREACH_OMPT_EVENT(OMPT_EVENT_FN)
#undef OMPT_EVENT_FN
} OmptEnabledTy;

#define OMPT_ENABLED (OmptGlobal && OmptGlobal->Enabled.enabled)

#define OMPT_CALLBACK(event, ...)                                              \
  do {                                                                         \
    if (OMPT_ENABLED && OmptGlobal->Enabled.event)                             \
      OmptGlobal->Callbacks.event(__VA_ARGS__);                                \
  } while (0)

#define OMPT_TRACE(Event)                                                      \
  do {                                                                         \
    if (OMPT_ENABLED) {                                                        \
      OmptGlobal->getTrace().Event;                                            \
    }                                                                          \
  } while (0)

#ifdef OMPTARGET_DEBUG
#define DPOMPT(...)                                                            \
  do {                                                                         \
    if (getDebugLevel() > 0) {                                                      \
      DEBUGP("Libomptarget", __VA_ARGS__);                                     \
    }                                                                          \
  } while (0)
#else // OMPTARGET_DEBUG
#define DPOMPT(...) {}
#endif // OMPTARGET_DEBUG

struct OmptTraceTy;

struct OmptGlobalTy {
  std::atomic<ompt_id_t> TargetId;
  std::atomic<ompt_id_t> HostOpId;
  std::mutex Mutex;
  std::map<int32_t, OmptTraceTy> Traces; // per-thread trace
  OmptCallbacksTy Callbacks;
  OmptEnabledTy Enabled;

  explicit OmptGlobalTy() = default;
  OmptGlobalTy(const OmptGlobalTy &) = delete;

  void init() {
    TargetId = 1;
    HostOpId = 1;
    // Call host runtime to get Callbacks, Enabled
    OmptCallbacksTy *pCallbacks = nullptr;
    OmptEnabledTy *pEnabled = nullptr;
    std::memset(&Enabled, 0, sizeof(Enabled));
#ifndef _WIN32
    if (!__kmpc_get_ompt_callbacks) {
      DPOMPT("Warning: OMPT is disabled\n");
      return;
    }
#endif
    __kmpc_get_ompt_callbacks((void **)&pCallbacks, (void **)&pEnabled);
    if (!pCallbacks || !pEnabled) {
      DPOMPT("Warning: cannot initialize OMPT\n");
      return;
    }
    Enabled = *pEnabled;
    Callbacks = *pCallbacks;
    DPOMPT("Initialized OMPT\n");
  }

  OmptTraceTy &getTrace();
};

extern OmptGlobalTy *OmptGlobal;

/// Callback interface. This is based on community code (tool's committee) which
/// relies on forthcoming changes to the callback signatures.
struct OmptTraceTy {
  ompt_id_t TargetId = 0;
  ompt_id_t HostOpId = 0;
  int32_t NumTeams = 0;
  int32_t ThreadLimit = 0;
  void *ReturnAddress = nullptr;
  std::map<const void *, std::string> CodeLocation;

  void targetDataAllocBegin(int64_t deviceId, size_t bytes) {
    HostOpId = OmptGlobal->HostOpId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_begin, TargetId,
                  HostOpId, ompt_target_data_alloc, nullptr, deviceId, nullptr,
                  deviceId, bytes, ReturnAddress);
  }

  void targetDataAllocEnd(int64_t deviceId, size_t bytes, void *ptr) {
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_end, TargetId,
                  HostOpId, ompt_target_data_alloc, ptr, deviceId, ptr,
                  deviceId, bytes, ReturnAddress);
    HostOpId = 0;
  }

  void targetDataDeleteBegin(int64_t deviceId, void *ptr) {
    HostOpId = OmptGlobal->HostOpId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_begin, TargetId,
                  HostOpId, ompt_target_data_delete, ptr, deviceId, ptr,
                  deviceId, 0, ReturnAddress);
  }

  void targetDataDeleteEnd(int64_t deviceId, void *ptr) {
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_end, TargetId,
                  HostOpId, ompt_target_data_delete, ptr, deviceId, ptr,
                  deviceId, 0, ReturnAddress);
    HostOpId = 0;
  }

  void targetDataSubmitBegin(
      int64_t deviceId, void *tgtPtr, void *hstPtr, size_t bytes) {
    HostOpId = OmptGlobal->HostOpId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_begin, TargetId,
                  HostOpId, ompt_target_data_transfer_to_device, hstPtr,
                  HOST_DEVICE, tgtPtr, deviceId, bytes, ReturnAddress);
  }

  void targetDataSubmitEnd(
      int64_t deviceId, void *tgtPtr, void *hstPtr, size_t bytes) {
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_end, TargetId,
                  HostOpId, ompt_target_data_transfer_to_device, hstPtr,
                  HOST_DEVICE, tgtPtr, deviceId, bytes, ReturnAddress);
    HostOpId = 0;
  }

  void targetDataRetrieveBegin(
      int64_t deviceId, void *hstPtr, void *tgtPtr, size_t bytes) {
    HostOpId = OmptGlobal->HostOpId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_begin, TargetId,
                  HostOpId, ompt_target_data_transfer_from_device, tgtPtr,
                  deviceId, hstPtr, HOST_DEVICE, bytes, ReturnAddress);
  }

  void targetDataRetrieveEnd(
      int64_t deviceId, void *hstPtr, void *tgtPtr, size_t bytes) {
    OMPT_CALLBACK(ompt_callback_target_data_op, ompt_scope_end, TargetId,
                  HostOpId, ompt_target_data_transfer_from_device, tgtPtr,
                  deviceId, hstPtr, HOST_DEVICE, bytes, ReturnAddress);
    HostOpId = 0;
  }

  void targetSubmitBegin(int64_t deviceId, uint32_t numTeams) {
    HostOpId = OmptGlobal->HostOpId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target_submit, ompt_scope_begin, TargetId,
                  HostOpId, numTeams);
  }

  void targetSubmitEnd(int64_t deviceId, uint32_t numTeams) {
    OMPT_CALLBACK(ompt_callback_target_submit, ompt_scope_end, TargetId,
                  HostOpId, numTeams);
    HostOpId = 0;
  }

  void targetDataEnterBegin(int64_t deviceId) {
    TargetId = OmptGlobal->TargetId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target, ompt_target_enter_data,
                  ompt_scope_begin, deviceId, nullptr /* TODO: task_data */,
                  TargetId, ReturnAddress);
  }

  void targetDataEnterEnd(int64_t deviceId) {
    OMPT_CALLBACK(ompt_callback_target, ompt_target_enter_data, ompt_scope_end,
                  deviceId, nullptr /* TODO: task_data */, TargetId,
                  ReturnAddress);
    popTarget();
  }

  void targetDataExitBegin(int64_t deviceId) {
    TargetId = OmptGlobal->TargetId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target, ompt_target_exit_data, ompt_scope_begin,
                  deviceId, nullptr /* TODO: task_data */, TargetId,
                  ReturnAddress);
  }

  void targetDataExitEnd(int64_t deviceId) {
    OMPT_CALLBACK(ompt_callback_target, ompt_target_exit_data, ompt_scope_end,
                  deviceId, nullptr /* TODO: task_data */, TargetId,
                  ReturnAddress);
    popTarget();
  }

  void targetDataUpdateBegin(int64_t deviceId) {
    TargetId = OmptGlobal->TargetId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target, ompt_target_update, ompt_scope_begin,
                  deviceId, nullptr /* TODO: task_data */, TargetId,
                  ReturnAddress);
  }

  void targetDataUpdateEnd(int64_t deviceId) {
    OMPT_CALLBACK(ompt_callback_target, ompt_target_update, ompt_scope_end,
                  deviceId, nullptr /* TODO: task_data */, TargetId,
                  ReturnAddress);
    popTarget();
  }

  void targetBegin(int64_t deviceId) {
    TargetId = OmptGlobal->TargetId.fetch_add(1);
    OMPT_CALLBACK(ompt_callback_target, ompt_target, ompt_scope_begin, deviceId,
                  nullptr /* TODO: task data */, TargetId, ReturnAddress);
  }

  void targetEnd(int64_t deviceId) {
    OMPT_CALLBACK(ompt_callback_target, ompt_target, ompt_scope_end, deviceId,
                  nullptr /* TODO; task data */, TargetId, ReturnAddress);
    popTarget();
  }

  // Store code location determinted by compiler
  void pushCodeLocation(const char *location, void *returnAddress) {
    if (!OmptGlobal->Enabled.enabled || !returnAddress)
      return;
    ReturnAddress = returnAddress;
    // We expect the following format from the generated code.
    // ;<file_name>;<function_name>;<line_number>;<column_number>;;
    // Just remove the heading/trailing separators for now.
    if (location) {
      std::string loc(location);
      CodeLocation.emplace(std::make_pair(
          ReturnAddress, loc.substr(1, loc.size() - 3)));
    } else {
      // Fallback string
      CodeLocation.emplace(std::make_pair(
          ReturnAddress, std::to_string((int64_t)ReturnAddress)));
    }
  }

  // Retrieve code location string associated with the return address
  const char *getCodeLocation(const void *returnAddress) {
    if (CodeLocation.find(returnAddress) != CodeLocation.end())
      return CodeLocation[returnAddress].c_str();
    else
      return nullptr;
  }

  // Record work size associated with the current target ID
  void pushWorkSize(int32_t numTeams, int32_t threadLimit) {
    NumTeams = numTeams;
    ThreadLimit = threadLimit;
  }

  // Clear data associated with the current target
  void popTarget() {
    TargetId = 0;
    NumTeams = 0;
    ThreadLimit = 0;
    CodeLocation.erase(ReturnAddress);
    ReturnAddress = nullptr;
  }
};

inline OmptTraceTy &OmptGlobalTy::getTrace() {
    int gtid = __kmpc_global_thread_num(nullptr);
    Mutex.lock();
    if (Traces.count(gtid) == 0)
      Traces.emplace(gtid, OmptTraceTy());
    auto &trace = Traces[gtid];
    Mutex.unlock();
    return trace;
}

extern const char *OmptDocument;
extern ompt_interface_fn_t omptLookupEntries(const char *);

#endif // _OMPTARGET_TOOLS_H_
#endif // INTEL_CUSTOMIZATION
