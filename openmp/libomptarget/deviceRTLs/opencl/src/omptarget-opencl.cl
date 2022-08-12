#if INTEL_COLLAB
//===--- omptarget-opencl.cl - OpenMP device runtime for OpenCL -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains OpenMP device runtime code for OpenCL devices
//
//===----------------------------------------------------------------------===//

#include "omptarget-opencl.h"
#include "internal.h"

///
/// Runtime data stored in global address space
///

kmp_global_state_t __omp_spirv_global_data = {
  .g_barrier = {{ATOMIC_VAR_INIT(0u), ATOMIC_VAR_INIT(0u)}},
  .assume_simple_spmd_mode = 1,
  .spmd_num_threads = 0
};

kmp_program_data_t __omp_spirv_program_data = {
  .initialized = 0,
  .num_devices = 0,
  .device_num = -1,
  .total_eus = 0,
  .hw_threads_per_eu = 0,
  .dyna_mem_cur = 0,
  .dyna_mem_ub = 0,
  .device_type = 0,
  .dyna_mem_pool = NULL,
  .teams_thread_limit = 0
};

ushort __omp_spirv_spmd_num_threads[KMP_MAX_SPMD_NUM_GROUPS];

kmp_local_state_t __omp_spirv_local_data[KMP_MAX_NUM_GROUPS];

kmp_thread_state_t __omp_spirv_thread_data[KMP_MAX_NUM_GROUPS];

// Host to target pointer map for user functions that may be called indirectly.
__attribute__((weak)) __global __omp_offloading_fptr_map_t *
    __omp_offloading_fptr_map_p = 0;
// Number of entries in __omp_offloading_fptr_map_p map.
__attribute__((weak)) ulong __omp_offloading_fptr_map_size = 0;

EXTERN void *__kmpc_target_translate_fptr(ulong fn_ptr) {
  ulong size = __omp_offloading_fptr_map_size;
  if (size == 0 || !__omp_offloading_fptr_map_p)
    return (void *)fn_ptr;

  // Do a binary search of fn_ptr in __omp_offloading_fptr_map_p
  // comparing it to the host_ptr keys.
  ulong left = 0;
  ulong right = size - 1;

  // Do a quick check for fn_ptr being outside of the host code region.
  // This should speed-up lookups for cases when the function pointer
  // already holds a device address.
  if (fn_ptr < __omp_offloading_fptr_map_p[left].host_ptr ||
      fn_ptr > __omp_offloading_fptr_map_p[right].host_ptr)
    return (void *)fn_ptr;

  while (left < right) {
    ulong middle = (left + right) / 2;
    if (__omp_offloading_fptr_map_p[middle].host_ptr < fn_ptr)
      left = middle + 1;
    else
      right = middle;
  }

  if (__omp_offloading_fptr_map_p[left].host_ptr == fn_ptr)
    return (void *)__omp_offloading_fptr_map_p[left].tgt_ptr;

  return (void *)fn_ptr;
}

#endif // INTEL_COLLAB
