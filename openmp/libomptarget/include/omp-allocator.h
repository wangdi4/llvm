#if INTEL_CUSTOMIZATION
//===--------- omp-allocator.h - Data types for OMP allocator -- C++ -*----===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2023 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Definitions from omp.h and host runtime.
//
//===----------------------------------------------------------------------===//
#ifndef _OMP_ALLOCATOR_H_
#define _OMP_ALLOCATOR_H_

/// Copied from omp.h.

typedef enum {
  omp_atk_sync_hint = 1,
  omp_atk_alignment = 2,
  omp_atk_access = 3,
  omp_atk_pool_size = 4,
  omp_atk_fallback = 5,
  omp_atk_fb_data = 6,
  omp_atk_pinned = 7,
  omp_atk_partition = 8,
  omp_atk_pin_device = 9,
  omp_atk_preferred_device = 10,
  omp_atk_device_access = 11,
  omp_atk_target_access = 12,
  omp_atk_atomic_scope = 13,
  omp_atk_part_size = 14
} omp_alloctrait_key_t;

typedef enum {
  omp_atv_false = 0,
  omp_atv_true = 1,
  omp_atv_contended = 3,
  omp_atv_uncontended = 4,
  omp_atv_serialized = 5,
  omp_atv_sequential = omp_atv_serialized, // (deprecated)
  omp_atv_private = 6,
  omp_atv_device = 7,
  omp_atv_thread = 8,
  omp_atv_pteam = 9,
  omp_atv_cgroup = 10,
  omp_atv_default_mem_fb = 11,
  omp_atv_null_fb = 12,
  omp_atv_abort_fb = 13,
  omp_atv_allocator_fb = 14,
  omp_atv_environment = 15,
  omp_atv_nearest = 16,
  omp_atv_blocked = 17,
  omp_atv_interleaved = 18,
  omp_atv_all = 19,
  omp_atv_single = 20,
  omp_atv_multiple = 21,
  omp_atv_memspace = 22
} omp_alloctrait_value_t;

typedef uintptr_t omp_uintptr_t;
#define omp_atv_default ((omp_uintptr_t)-1)

typedef struct {
  omp_alloctrait_key_t key;
  omp_uintptr_t value;
} omp_alloctrait_t;

// For now, do not consider predefined allocator.
typedef omp_uintptr_t omp_allocator_handle_t;
constexpr omp_allocator_handle_t kmp_max_mem_alloc = 1024;

typedef omp_uintptr_t omp_memspace_handle_t;
constexpr omp_memspace_handle_t omp_null_mem_space = 0;
constexpr omp_memspace_handle_t omp_default_mem_space = 99;
constexpr omp_memspace_handle_t omp_large_cap_mem_space = 1;
constexpr omp_memspace_handle_t omp_const_mem_space = 2;
constexpr omp_memspace_handle_t omp_high_bw_mem_space = 3;
constexpr omp_memspace_handle_t omp_low_lat_mem_space = 4;
constexpr omp_memspace_handle_t kmp_max_mem_space = 1024;

/// Memory space information is shared with offload runtime.
typedef struct kmp_memspace_t {
  omp_memspace_handle_t memspace; // predefined input memory space
  int num_resources = 0;          // number of available resources
  int *resources = nullptr;       // available resources
  kmp_memspace_t *next = nullptr; // next memory space handle
} kmp_memspace_t;

/// Memory allocator information is shared with offload runtime.
typedef struct kmp_allocator_t {
  omp_memspace_handle_t memspace;
  void **memkind; // pointer to memkind
  size_t alignment;
  omp_alloctrait_value_t fb;
  kmp_allocator_t *fb_data;
  uint64_t pool_size;
  uint64_t pool_used;
  bool pinned;
  omp_alloctrait_value_t partition;
  int pin_device;
  int preferred_device;
  omp_alloctrait_value_t target_access;
  omp_alloctrait_value_t atomic_scope;
  size_t part_size;
  omp_alloctrait_value_t membind; // hwloc access
} kmp_allocator_t;

#endif // _OMP_ALLOCATOR_H_
#endif // INTEL_CUSTOMIZATION
