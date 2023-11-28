#pragma OPENCL EXTENSION cl_khr_fp16 : enable

__kernel void test_atomic_f16(__global half *addend,
                              __global atomic_half *resAddSum,
                              __global atomic_half *resSubSum) {
  atomic_fetch_add(resAddSum, *addend);
  atomic_fetch_add_explicit(resAddSum, *addend, memory_order_relaxed);
  atomic_fetch_add_explicit(resAddSum, *addend, memory_order_relaxed);
  atomic_fetch_add_explicit(resAddSum, *addend, memory_order_relaxed,
                            memory_scope_device);
  atomic_fetch_add_explicit(resAddSum, *addend, memory_order_relaxed,
                            memory_scope_device);

  atomic_fetch_sub(resSubSum, *addend);
  atomic_fetch_sub_explicit(resSubSum, *addend, memory_order_relaxed);
  atomic_fetch_sub_explicit(resSubSum, *addend, memory_order_relaxed);
  atomic_fetch_sub_explicit(resSubSum, *addend, memory_order_relaxed,
                            memory_scope_device);
  atomic_fetch_sub_explicit(resSubSum, *addend, memory_order_relaxed,
                            memory_scope_device);
};
