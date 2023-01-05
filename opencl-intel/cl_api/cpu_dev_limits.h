// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "common_dev_limits.h"

// Maximum values that could be specified for WI in one dimension
#define CPU_DEV_MAX_WI_SIZE (8 * 1024)

// Default CPU local memory size of 32KB
#define CPU_DEV_LCL_MEM_SIZE (32 * 1024)
// Many FPGA designs require more than 32KB local memory.
// 256KB should be enough for most of them.
#define FPGA_DEV_LCL_MEM_SIZE (256 * 1024)

#define CPU_DEV_DCU_LINE_SIZE 64
#define CPU_DEV_MAXIMUM_ALIGN (DEV_MAXIMUM_ALIGN)

#define ADJUST_SIZE_TO_DCU_LINE(X)                                             \
  (((X) + CPU_DEV_DCU_LINE_SIZE - 1) & (~(CPU_DEV_DCU_LINE_SIZE - 1)))
#define MIN_PARAM(X, Y) ((X) < (Y) ? (X) : (Y))

// Maximum number of arguments to be passed to the kernel
#define CPU_MAX_PARAMETER_SIZE (4096 - 256)
#define CPU_MAX_PARAM_COUNT (CPU_MAX_PARAMETER_SIZE / 8)
#define CPU_KERNEL_MAX_ARG_COUNT                                               \
  (MIN_PARAM((CPU_MAX_PARAMETER_SIZE / sizeof(void *)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_SAMPLERS                                                       \
  (MIN_PARAM((CPU_MAX_PARAMETER_SIZE / sizeof(void *)), CPU_MAX_PARAM_COUNT))
/**
 * Maximum dimension size (in pixels) for 3D image. Taken from GEN.
 * Image size limits have no clear definition in varying-memory CPU case,
 * so we use the GPU hardware limits. It is an acceptable value because it
 * represents real GPGPU use cases.
 */
#define GEN_IMAGE3D_MAX_DIM_SIZE 2048
#define CPU_IMAGE3D_MAX_DIM_SIZE GEN_IMAGE3D_MAX_DIM_SIZE
// Maximum dimension size (in pixels) for 2D image. Taken from GEN.
#define GEN_IMAGE2D_MAX_DIM_SIZE 16384
#define CPU_IMAGE2D_MAX_DIM_SIZE GEN_IMAGE2D_MAX_DIM_SIZE
// Maximum image array size. Taken from GEN.
#define GEN_MAX_ARRAY_SIZE 2048
#define CPU_MAX_ARRAY_SIZE GEN_MAX_ARRAY_SIZE
#define CPU_MAX_READ_IMAGE_ARGS                                                \
  (MIN_PARAM((CPU_MAX_PARAMETER_SIZE / sizeof(void *)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_WRITE_IMAGE_ARGS                                               \
  (MIN_PARAM((CPU_MAX_PARAMETER_SIZE / sizeof(void *)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_READ_WRITE_IMAGE_ARGS                                          \
  (MIN_PARAM((CPU_MAX_PARAMETER_SIZE / sizeof(void *)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_CONSTANT_BUFFER_SIZE (128 * 1024)
#define CPU_MAX_CONSTANT_ARGS                                                  \
  (MIN_PARAM((CPU_MAX_PARAMETER_SIZE / sizeof(void *)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_LOCAL_ARGS                                                     \
  (MIN_PARAM((CPU_MAX_PARAMETER_SIZE / sizeof(void *)), CPU_MAX_PARAM_COUNT))
#define CPU_MEM_BASE_ADDR_ALIGN (CPU_DEV_MAXIMUM_ALIGN * 8) // In bits
#define CPU_MAX_WORK_ITEM_DIMENSIONS MAX_WORK_DIM

// Must be power of 2, No API to get max number of fibers
#define CPU_MAX_WORK_GROUP_SIZE (8 * 1024)
#define FPGA_MAX_WORK_GROUP_SIZE (64 * 1024 * 1024)
// Upper bound of CL_CONFIG_CPU_FORCE_MAX_WORK_GROUP_SIZE
#define CPU_MAX_WORK_GROUP_SIZE_UPPER_BOUND (64 * 1024 * 1024)

#define CPU_DEFAULT_WG_SIZE 128
#define CPU_MIN_ACTUAL_PARAM_SIZE sizeof(size_t)
#define CPU_MIN_ACTUAL_PARAM_PTR size_t *
// Minimum vector size, XMM == 16bytes
#define CPU_MIN_VECTOR_SIZE 16
#define CPU_MAX_PRINTF_BUFFER_SIZE 1024 * 1024

// We'd like TBB worker thread stack size to be 8MB on 64-bit system and 4MB on
// 32-bit system.
// Application can use CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE and
// CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE to change TBB stack size.
#if defined(__x86_64__) || defined(_M_X64)
#define CPU_DEV_TBB_STACK_SIZE (8 * 1024 * 1024)
#else
#define CPU_DEV_TBB_STACK_SIZE (4 * 1024 * 1024)
#endif

// CPU_DEV_MAX_WG_STACK_SIZE is the maximum stack size that could be allocated
// for WG execution. It takes both master thread and TBB worker threads into
// account. It is the sum of CPU_DEV_BASE_STACK_SIZE, CPU_DEV_LCL_MEM_SIZE and
// CPU_DEV_MAX_WG_PRIVATE_SIZE.
// Refer to TBB worker thread stack size calculation in TBBTaskExecutor::Init.
#define CPU_DEV_MAX_WG_STACK_SIZE CPU_DEV_TBB_STACK_SIZE

// Base stack size covers the stack that kernel parameters use and the stack
// that a TBB worker thread already uses before launching a kernel.
#define CPU_DEV_BASE_STACK_SIZE (512 * 1024)

// Maximum memory size that could be allocated for private variables of a WG
// that is executed by a thread.
// We'd like TBB worker thread stack size to be 6MB, which is the sum of
// CPU_DEV_BASE_STACK_SIZE, CPU_DEV_LCL_MEM_SIZE and
// CPU_DEV_MAX_WG_PRIVATE_SIZE.
// Refer to TBB worker thread stack size calculation in TBBTaskExecutor::Init.
#define CPU_DEV_MAX_WG_PRIVATE_SIZE                                            \
  (CPU_DEV_MAX_WG_STACK_SIZE - CPU_DEV_BASE_STACK_SIZE - CPU_DEV_LCL_MEM_SIZE)

// Maximum private memory size that could be allocated for WG execution for
// FPGA, total 8MB/WG. Only used when auto memory allocation is enabled.
#define FPGA_DEV_MAX_WG_PRIVATE_SIZE (8 * 1024 * 1024)

// Maximum memory size that could be allocated for WG execution. This is the
// sum of WG Private memory size + Kernel parameters size (twice to cover the
// hidden parameters) + Local IDs buffer.
#define CPU_DEV_MAX_WG_TOTAL_SIZE                                              \
  (CPU_DEV_MAX_WG_PRIVATE_SIZE + (2 * CPU_MAX_PARAMETER_SIZE) +                \
   (CPU_MAX_WORK_GROUP_SIZE * MAX_WI_DIM_POW_OF_2 * sizeof(size_t)))
#define FPGA_DEV_MAX_WG_TOTAL_SIZE                                             \
  (CPU_DEV_MAX_WG_PRIVATE_SIZE + (2 * CPU_MAX_PARAMETER_SIZE) +                \
   (FPGA_MAX_WORK_GROUP_SIZE * MAX_WI_DIM_POW_OF_2 * sizeof(size_t)))

// Default stack size for kernel execution
#define CPU_DEV_STACK_DEFAULT_SIZE (4 * 1024 * 1024)
// Extra stack size for execution of builtins and third-party functions
#define CPU_DEV_STACK_EXTRA_SIZE (1024 * 1024)

// Minmum size in bytes that triggers parallel mem copy/fill.
#define DEV_PARALLEL_COPY_MIN_SIZE (512)

// Supported subgroup sizes on cpu device.
#define CPU_DEV_SUB_GROUP_SIZES                                                \
  { 4, 8, 16, 32, 64 }

#define CPU_DEV_MAX_SG_SIZE 64
