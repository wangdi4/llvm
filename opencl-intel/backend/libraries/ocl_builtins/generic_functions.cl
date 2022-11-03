// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

__attribute__((overloadable)) cl_mem_fence_flags get_fence(void *genptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable)) cl_mem_fence_flags get_fence(const void *genptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable)) cl_mem_fence_flags
get_fence(__global void *globptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable)) cl_mem_fence_flags
get_fence(__global const void *globptr) {
  return CLK_GLOBAL_MEM_FENCE;
}

__attribute__((overloadable)) cl_mem_fence_flags
get_fence(__local void *localptr) {
  return CLK_LOCAL_MEM_FENCE;
}

__attribute__((overloadable)) cl_mem_fence_flags
get_fence(__local const void *localptr) {
  return CLK_LOCAL_MEM_FENCE;
}

__attribute__((overloadable)) cl_mem_fence_flags
get_fence(__private void *privptr) {
  return 0;
}

__attribute__((overloadable)) cl_mem_fence_flags
get_fence(__private const void *privptr) {
  return 0;
}
