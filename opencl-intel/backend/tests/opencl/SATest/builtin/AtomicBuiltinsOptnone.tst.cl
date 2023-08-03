__kernel void test_atomic_init_int(__global int *object, int value) {
  atomic_init((volatile atomic_int *)object, value);
}
__kernel void test_atomic_init_uint(__global uint *object, uint value) {
  atomic_init((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_init_long(__global long *object, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_init((volatile atomic_long *)object, value);
}
__kernel void test_atomic_init_ulong(__global ulong *object, ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_init((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_init_float(__global float *object, float value) {
  atomic_init((volatile atomic_float *)object, value);
}
__kernel void test_atomic_init_double(__global double *object, double value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_init((volatile atomic_double *)object, value);
}
__kernel void test_atomic_compare_exchange_weak_int(__global int *lock,
                                                    __global int *desired) {
  int expected = 0;
  *desired =
      atomic_compare_exchange_weak((volatile atomic_int *)lock, &expected, 1);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_int(__global int *lock,
                                               __global int *desired) {
  int expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_scope_int(__global int *lock,
                                                     __global int *desired) {
  int expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_int__global(
    __global int *lock, __global int *expected, __global int *desired) {
  *desired =
      atomic_compare_exchange_weak((volatile atomic_int *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_int__global(
    __global int *lock, __global int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_int__global(
    __global int *lock, __global int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_int__local(
    __global int *lock, __local int *expected, __global int *desired) {
  *desired =
      atomic_compare_exchange_weak((volatile atomic_int *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_int__local(
    __global int *lock, __local int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_int__local(
    __global int *lock, __local int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_int__private(__global int *lock,
                                               __global int *desired) {
  __private int expected = 0;
  *desired =
      atomic_compare_exchange_weak((volatile atomic_int *)lock, &expected, 1);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_int__private(__global int *lock,
                                                        __global int *desired) {
  __private int expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_int__private(
    __global int *lock, __global int *desired) {
  __private int expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_uint(__global uint *lock,
                                                     __global uint *desired) {
  uint expected = 0;
  *desired =
      atomic_compare_exchange_weak((volatile atomic_uint *)lock, &expected, 1);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_uint(__global uint *lock,
                                                __global uint *desired) {
  uint expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_scope_uint(__global uint *lock,
                                                      __global uint *desired) {
  uint expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_uint__global(
    __global uint *lock, __global uint *expected, __global uint *desired) {
  *desired =
      atomic_compare_exchange_weak((volatile atomic_uint *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_uint__global(
    __global uint *lock, __global uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_uint__global(
    __global uint *lock, __global uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_uint__local(
    __global uint *lock, __local uint *expected, __global uint *desired) {
  *desired =
      atomic_compare_exchange_weak((volatile atomic_uint *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_uint__local(
    __global uint *lock, __local uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_uint__local(
    __global uint *lock, __local uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_uint__private(__global uint *lock,
                                                __global uint *desired) {
  __private uint expected = 0;
  *desired =
      atomic_compare_exchange_weak((volatile atomic_uint *)lock, &expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_uint__private(
    __global uint *lock, __global uint *desired) {
  __private uint expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_uint__private(
    __global uint *lock, __global uint *desired) {
  __private uint expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_long(__global long *lock,
                                                     __global long *desired) {
  long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_long *)lock, &expected, 1);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_long(__global long *lock,
                                                __global long *desired) {
  long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_scope_long(__global long *lock,
                                                      __global long *desired) {
  long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_long__global(
    __global long *lock, __global long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_long *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_long__global(
    __global long *lock, __global long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_long__global(
    __global long *lock, __global long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_long__local(
    __global long *lock, __local long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_long *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_long__local(
    __global long *lock, __local long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_long__local(
    __global long *lock, __local long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_long__private(__global long *lock,
                                                __global long *desired) {
  __private long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_long *)lock, &expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_long__private(
    __global long *lock, __global long *desired) {
  __private long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_long__private(
    __global long *lock, __global long *desired) {
  __private long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_ulong(__global ulong *lock,
                                                      __global ulong *desired) {
  ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_ulong *)lock, &expected, 1);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_ulong(__global ulong *lock,
                                                 __global ulong *desired) {
  ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_ulong(
    __global ulong *lock, __global ulong *desired) {
  ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_ulong__global(
    __global ulong *lock, __global ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_ulong *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_ulong__global(
    __global ulong *lock, __global ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_ulong__global(
    __global ulong *lock, __global ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_ulong__local(
    __global ulong *lock, __local ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_ulong *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_ulong__local(
    __global ulong *lock, __local ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_ulong__local(
    __global ulong *lock, __local ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_ulong__private(__global ulong *lock,
                                                 __global ulong *desired) {
  __private ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_ulong *)lock, &expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_ulong__private(
    __global ulong *lock, __global ulong *desired) {
  __private ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_ulong__private(
    __global ulong *lock, __global ulong *desired) {
  __private ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_float(__global float *lock,
                                                      __global float *desired) {
  float expected = 0;
  *desired =
      atomic_compare_exchange_weak((volatile atomic_float *)lock, &expected, 1);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_float(__global float *lock,
                                                 __global float *desired) {
  float expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_float(
    __global float *lock, __global float *desired) {
  float expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_float__global(
    __global float *lock, __global float *expected, __global float *desired) {
  *desired =
      atomic_compare_exchange_weak((volatile atomic_float *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_float__global(
    __global float *lock, __global float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_float__global(
    __global float *lock, __global float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_float__local(
    __global float *lock, __local float *expected, __global float *desired) {
  *desired =
      atomic_compare_exchange_weak((volatile atomic_float *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_float__local(
    __global float *lock, __local float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_float__local(
    __global float *lock, __local float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_float__private(__global float *lock,
                                                 __global float *desired) {
  __private float expected = 0;
  *desired =
      atomic_compare_exchange_weak((volatile atomic_float *)lock, &expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_float__private(
    __global float *lock, __global float *desired) {
  __private float expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_float__private(
    __global float *lock, __global float *desired) {
  __private float expected = 0;
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_double(__global double *lock,
                                         __global double *desired) {
  double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak((volatile atomic_double *)lock,
                                          &expected, 1);
}
__kernel void
test_atomic_compare_exchange_weak_explicit_double(__global double *lock,
                                                  __global double *desired) {
  double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_double(
    __global double *lock, __global double *desired) {
  double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_double__global(__global double *lock,
                                                 __global double *expected,
                                                 __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_double *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_double__global(
    __global double *lock, __global double *expected,
    __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_double__global(
    __global double *lock, __global double *expected,
    __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_weak_double__local(
    __global double *lock, __local double *expected, __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_weak((volatile atomic_double *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_double__local(
    __global double *lock, __local double *expected, __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_double__local(
    __global double *lock, __local double *expected, __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_weak_double__private(__global double *lock,
                                                  __global double *desired) {
  __private double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak((volatile atomic_double *)lock,
                                          &expected, 1);
}
__kernel void test_atomic_compare_exchange_weak_explicit_double__private(
    __global double *lock, __global double *desired) {
  __private double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_weak_explicit_scope_double__private(
    __global double *lock, __global double *desired) {
  __private double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_weak_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_int(__global int *lock,
                                                      __global int *desired) {
  int expected = 0;
  *desired =
      atomic_compare_exchange_strong((volatile atomic_int *)lock, &expected, 1);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_int(__global int *lock,
                                                 __global int *desired) {
  int expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_scope_int(__global int *lock,
                                                       __global int *desired) {
  int expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_int__global(
    __global int *lock, __global int *expected, __global int *desired) {
  *desired =
      atomic_compare_exchange_strong((volatile atomic_int *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_int__global(
    __global int *lock, __global int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_int__global(
    __global int *lock, __global int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_int__local(
    __global int *lock, __local int *expected, __global int *desired) {
  *desired =
      atomic_compare_exchange_strong((volatile atomic_int *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_int__local(
    __global int *lock, __local int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_int__local(
    __global int *lock, __local int *expected, __global int *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_int__private(__global int *lock,
                                                 __global int *desired) {
  __private int expected = 0;
  *desired =
      atomic_compare_exchange_strong((volatile atomic_int *)lock, &expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_int__private(
    __global int *lock, __global int *desired) {
  __private int expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_int__private(
    __global int *lock, __global int *desired) {
  __private int expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_int *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_uint(__global uint *lock,
                                                       __global uint *desired) {
  uint expected = 0;
  *desired = atomic_compare_exchange_strong((volatile atomic_uint *)lock,
                                            &expected, 1);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_uint(__global uint *lock,
                                                  __global uint *desired) {
  uint expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_uint(
    __global uint *lock, __global uint *desired) {
  uint expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_uint__global(
    __global uint *lock, __global uint *expected, __global uint *desired) {
  *desired =
      atomic_compare_exchange_strong((volatile atomic_uint *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_uint__global(
    __global uint *lock, __global uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_uint__global(
    __global uint *lock, __global uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_uint__local(
    __global uint *lock, __local uint *expected, __global uint *desired) {
  *desired =
      atomic_compare_exchange_strong((volatile atomic_uint *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_uint__local(
    __global uint *lock, __local uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_uint__local(
    __global uint *lock, __local uint *expected, __global uint *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_uint__private(__global uint *lock,
                                                  __global uint *desired) {
  __private uint expected = 0;
  *desired = atomic_compare_exchange_strong((volatile atomic_uint *)lock,
                                            &expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_uint__private(
    __global uint *lock, __global uint *desired) {
  __private uint expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_uint__private(
    __global uint *lock, __global uint *desired) {
  __private uint expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_uint *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_long(__global long *lock,
                                                       __global long *desired) {
  long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_long *)lock,
                                            &expected, 1);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_long(__global long *lock,
                                                  __global long *desired) {
  long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_long(
    __global long *lock, __global long *desired) {
  long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_long__global(
    __global long *lock, __global long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_strong((volatile atomic_long *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_long__global(
    __global long *lock, __global long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_long__global(
    __global long *lock, __global long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_long__local(
    __global long *lock, __local long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_compare_exchange_strong((volatile atomic_long *)lock, expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_long__local(
    __global long *lock, __local long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_long__local(
    __global long *lock, __local long *expected, __global long *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_long__private(__global long *lock,
                                                  __global long *desired) {
  __private long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_long *)lock,
                                            &expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_long__private(
    __global long *lock, __global long *desired) {
  __private long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_long__private(
    __global long *lock, __global long *desired) {
  __private long expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_long *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_ulong(__global ulong *lock,
                                          __global ulong *desired) {
  ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_ulong *)lock,
                                            &expected, 1);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_ulong(__global ulong *lock,
                                                   __global ulong *desired) {
  ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_ulong(
    __global ulong *lock, __global ulong *desired) {
  ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_ulong__global(
    __global ulong *lock, __global ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_ulong *)lock,
                                            expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_ulong__global(
    __global ulong *lock, __global ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_ulong__global(
    __global ulong *lock, __global ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_ulong__local(
    __global ulong *lock, __local ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_ulong *)lock,
                                            expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_ulong__local(
    __global ulong *lock, __local ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_ulong__local(
    __global ulong *lock, __local ulong *expected, __global ulong *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_ulong__private(__global ulong *lock,
                                                   __global ulong *desired) {
  __private ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_ulong *)lock,
                                            &expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_ulong__private(
    __global ulong *lock, __global ulong *desired) {
  __private ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_ulong__private(
    __global ulong *lock, __global ulong *desired) {
  __private ulong expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_ulong *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_float(__global float *lock,
                                          __global float *desired) {
  float expected = 0;
  *desired = atomic_compare_exchange_strong((volatile atomic_float *)lock,
                                            &expected, 1);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_float(__global float *lock,
                                                   __global float *desired) {
  float expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_float(
    __global float *lock, __global float *desired) {
  float expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_float__global(
    __global float *lock, __global float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_strong((volatile atomic_float *)lock,
                                            expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_float__global(
    __global float *lock, __global float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_float__global(
    __global float *lock, __global float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_float__local(
    __global float *lock, __local float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_strong((volatile atomic_float *)lock,
                                            expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_float__local(
    __global float *lock, __local float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_float__local(
    __global float *lock, __local float *expected, __global float *desired) {
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_float__private(__global float *lock,
                                                   __global float *desired) {
  __private float expected = 0;
  *desired = atomic_compare_exchange_strong((volatile atomic_float *)lock,
                                            &expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_float__private(
    __global float *lock, __global float *desired) {
  __private float expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_float__private(
    __global float *lock, __global float *desired) {
  __private float expected = 0;
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_float *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_double(__global double *lock,
                                           __global double *desired) {
  double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_double *)lock,
                                            &expected, 1);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_double(__global double *lock,
                                                    __global double *desired) {
  double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_double(
    __global double *lock, __global double *desired) {
  double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_double__global(__global double *lock,
                                                   __global double *expected,
                                                   __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_double *)lock,
                                            expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_double__global(
    __global double *lock, __global double *expected,
    __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_double__global(
    __global double *lock, __global double *expected,
    __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_compare_exchange_strong_double__local(
    __global double *lock, __local double *expected, __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_double *)lock,
                                            expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_double__local(
    __global double *lock, __local double *expected, __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void test_atomic_compare_exchange_strong_explicit_scope_double__local(
    __global double *lock, __local double *expected, __global double *desired) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void
test_atomic_compare_exchange_strong_double__private(__global double *lock,
                                                    __global double *desired) {
  __private double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong((volatile atomic_double *)lock,
                                            &expected, 1);
}
__kernel void test_atomic_compare_exchange_strong_explicit_double__private(
    __global double *lock, __global double *desired) {
  __private double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_relaxed);
}
__kernel void
test_atomic_compare_exchange_strong_explicit_scope_double__private(
    __global double *lock, __global double *desired) {
  __private double expected = 0;
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_compare_exchange_strong_explicit(
      (volatile atomic_double *)lock, &expected, 1, memory_order_relaxed,
      memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_store_int(__global int *object, int value) {
  atomic_store((volatile atomic_int *)object, value);
}
__kernel void test_atomic_store_explicit_int(__global int *object, int value) {
  atomic_store_explicit((volatile atomic_int *)object, value,
                        memory_order_relaxed);
}
__kernel void atomic_store_explicit_scope_int(__global int *object, int value) {
  atomic_store_explicit((volatile atomic_int *)object, value,
                        memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_store_uint(__global uint *object, uint value) {
  atomic_store((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_store_explicit_uint(__global uint *object,
                                              uint value) {
  atomic_store_explicit((volatile atomic_uint *)object, value,
                        memory_order_relaxed);
}
__kernel void atomic_store_explicit_scope_uint(__global uint *object,
                                               uint value) {
  atomic_store_explicit((volatile atomic_uint *)object, value,
                        memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_store_long(__global long *object, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store((volatile atomic_long *)object, value);
}
__kernel void test_atomic_store_explicit_long(__global long *object,
                                              long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store_explicit((volatile atomic_long *)object, value,
                        memory_order_relaxed);
}
__kernel void atomic_store_explicit_scope_long(__global long *object,
                                               long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store_explicit((volatile atomic_long *)object, value,
                        memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_store_ulong(__global ulong *object, ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_store_explicit_ulong(__global ulong *object,
                                               ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store_explicit((volatile atomic_ulong *)object, value,
                        memory_order_relaxed);
}
__kernel void atomic_store_explicit_scope_ulong(__global ulong *object,
                                                ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store_explicit((volatile atomic_ulong *)object, value,
                        memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_store_float(__global float *object, float value) {
  atomic_store((volatile atomic_float *)object, value);
}
__kernel void test_atomic_store_explicit_float(__global float *object,
                                               float value) {
  atomic_store_explicit((volatile atomic_float *)object, value,
                        memory_order_relaxed);
}
__kernel void atomic_store_explicit_scope_float(__global float *object,
                                                float value) {
  atomic_store_explicit((volatile atomic_float *)object, value,
                        memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_store_double(__global double *object, double value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store((volatile atomic_double *)object, value);
}
__kernel void test_atomic_store_explicit_double(__global double *object,
                                                double value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store_explicit((volatile atomic_double *)object, value,
                        memory_order_relaxed);
}
__kernel void atomic_store_explicit_scope_double(__global double *object,
                                                 double value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  atomic_store_explicit((volatile atomic_double *)object, value,
                        memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_load_int(__global int *object) {
  int value = atomic_load((volatile atomic_int *)object);
}
__kernel void test_atomic_load_explicit_int(__global int *object) {
  int value =
      atomic_load_explicit((volatile atomic_int *)object, memory_order_relaxed);
}
__kernel void atomic_load_explicit_scope_int(__global int *object) {
  int value = atomic_load_explicit((volatile atomic_int *)object,
                                   memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_load_uint(__global uint *object) {
  uint value = atomic_load((volatile atomic_uint *)object);
}
__kernel void test_atomic_load_explicit_uint(__global uint *object) {
  uint value = atomic_load_explicit((volatile atomic_uint *)object,
                                    memory_order_relaxed);
}
__kernel void atomic_load_explicit_scope_uint(__global uint *object) {
  uint value = atomic_load_explicit((volatile atomic_uint *)object,
                                    memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_load_long(__global long *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  long value = atomic_load((volatile atomic_long *)object);
}
__kernel void test_atomic_load_explicit_long(__global long *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  long value = atomic_load_explicit((volatile atomic_long *)object,
                                    memory_order_relaxed);
}
__kernel void atomic_load_explicit_scope_long(__global long *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  long value = atomic_load_explicit((volatile atomic_long *)object,
                                    memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_load_ulong(__global ulong *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  ulong value = atomic_load((volatile atomic_ulong *)object);
}
__kernel void test_atomic_load_explicit_ulong(__global ulong *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  ulong value = atomic_load_explicit((volatile atomic_ulong *)object,
                                     memory_order_relaxed);
}
__kernel void atomic_load_explicit_scope_ulong(__global ulong *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  ulong value = atomic_load_explicit((volatile atomic_ulong *)object,
                                     memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_load_float(__global float *object) {
  float value = atomic_load((volatile atomic_float *)object);
}
__kernel void test_atomic_load_explicit_float(__global float *object) {
  float value = atomic_load_explicit((volatile atomic_float *)object,
                                     memory_order_relaxed);
}
__kernel void atomic_load_explicit_scope_float(__global float *object) {
  float value = atomic_load_explicit((volatile atomic_float *)object,
                                     memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_load_double(__global double *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  double value = atomic_load((volatile atomic_double *)object);
}
__kernel void test_atomic_load_explicit_double(__global double *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  double value = atomic_load_explicit((volatile atomic_double *)object,
                                      memory_order_relaxed);
}
__kernel void atomic_load_explicit_scope_double(__global double *object) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  double value =
      atomic_load_explicit((volatile atomic_double *)object,
                           memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_exchange_int(__global int *object,
                                       __global int *desired, int value) {
  *desired = atomic_exchange((volatile atomic_int *)object, value);
}
__kernel void test_atomic_exchange_explicit_int(__global int *object,
                                                __global int *desired,
                                                int value) {
  *desired = atomic_exchange_explicit((volatile atomic_int *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_exchange_explicit_scope_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired =
      atomic_exchange_explicit((volatile atomic_int *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_exchange_uint(__global uint *object,
                                        __global uint *desired, uint value) {
  *desired = atomic_exchange((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_exchange_explicit_uint(__global uint *object,
                                                 __global uint *desired,
                                                 uint value) {
  *desired = atomic_exchange_explicit((volatile atomic_uint *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_exchange_explicit_scope_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired =
      atomic_exchange_explicit((volatile atomic_uint *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_exchange_long(__global long *object,
                                        __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_exchange((volatile atomic_long *)object, value);
}
__kernel void test_atomic_exchange_explicit_long(__global long *object,
                                                 __global long *desired,
                                                 long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_exchange_explicit((volatile atomic_long *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_exchange_explicit_scope_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_exchange_explicit((volatile atomic_long *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_exchange_ulong(__global ulong *object,
                                         __global ulong *desired, ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_exchange((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_exchange_explicit_ulong(__global ulong *object,
                                                  __global ulong *desired,
                                                  ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_exchange_explicit((volatile atomic_ulong *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_exchange_explicit_scope_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_exchange_explicit((volatile atomic_ulong *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_exchange_float(__global float *object,
                                         __global float *desired, float value) {
  *desired = atomic_exchange((volatile atomic_float *)object, value);
}
__kernel void test_atomic_exchange_explicit_float(__global float *object,
                                                  __global float *desired,
                                                  float value) {
  *desired = atomic_exchange_explicit((volatile atomic_float *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_exchange_explicit_scope_float(__global float *object,
                                                   __global float *desired,
                                                   float value) {
  *desired =
      atomic_exchange_explicit((volatile atomic_float *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_exchange_double(__global double *object,
                                          __global double *desired,
                                          double value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_exchange((volatile atomic_double *)object, value);
}
__kernel void test_atomic_exchange_explicit_double(__global double *object,
                                                   __global double *desired,
                                                   double value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_exchange_explicit((volatile atomic_double *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_exchange_explicit_scope_double(__global double *object,
                                                    __global double *desired,
                                                    double value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_exchange_explicit((volatile atomic_double *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_int(__global int *object,
                                        __global int *desired, int value) {
  *desired = atomic_fetch_add((volatile atomic_int *)object, value);
}
__kernel void test_atomic_fetch_add_explicit_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired = atomic_fetch_add_explicit((volatile atomic_int *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_int(__global int *object,
                                                  __global int *desired,
                                                  int value) {
  *desired =
      atomic_fetch_add_explicit((volatile atomic_int *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_uint(__global uint *object,
                                         __global uint *desired, uint value) {
  *desired = atomic_fetch_add((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_fetch_add_explicit_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired = atomic_fetch_add_explicit((volatile atomic_uint *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_uint(__global uint *object,
                                                   __global uint *desired,
                                                   uint value) {
  *desired =
      atomic_fetch_add_explicit((volatile atomic_uint *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_long(__global long *object,
                                         __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_add((volatile atomic_long *)object, value);
}
__kernel void test_atomic_fetch_add_explicit_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_add_explicit((volatile atomic_long *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_long(__global long *object,
                                                   __global long *desired,
                                                   long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_add_explicit((volatile atomic_long *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_ulong(__global ulong *object,
                                          __global ulong *desired,
                                          ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_add((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_fetch_add_explicit_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_add_explicit((volatile atomic_ulong *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_ulong(__global ulong *object,
                                                    __global ulong *desired,
                                                    ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_add_explicit((volatile atomic_ulong *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}

// atomic add with float and double type
__kernel void test_atomic_fetch_add_explicit_float(__global float *object,
                                                   __global float *desired,
                                                   float value) {
  *desired = atomic_fetch_add_explicit((volatile atomic_float *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_float(__global float *object,
                                                    __global float *desired,
                                                    float value) {
  *desired =
      atomic_fetch_add_explicit((volatile atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_explicit_double(__global double *object,
                                                    __global double *desired,
                                                    double value) {
  *desired = atomic_fetch_add_explicit((volatile atomic_double *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_double(__global double *object,
                                                     __global double *desired,
                                                     double value) {
  *desired =
      atomic_fetch_add_explicit((volatile atomic_double *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_explicit_float_g(__global float *object,
                                                     __global float *desired,
                                                     float value) {
  *desired = atomic_fetch_add_explicit((volatile __global atomic_float *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_float_g(__global float *object,
                                                      __global float *desired,
                                                      float value) {
  *desired =
      atomic_fetch_add_explicit((volatile __global atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_explicit_double_g(__global double *object,
                                                      __global double *desired,
                                                      double value) {
  *desired = atomic_fetch_add_explicit(
      (volatile __global atomic_double *)object, value, memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_double_g(__global double *object,
                                                       __global double *desired,
                                                       double value) {
  *desired = atomic_fetch_add_explicit(
      (volatile __global atomic_double *)object, value, memory_order_seq_cst,
      memory_scope_device);
}
__kernel void test_atomic_fetch_add_explicit_float_l(__local float *object,
                                                     __global float *desired,
                                                     float value) {
  *desired = atomic_fetch_add_explicit((volatile __local atomic_float *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_float_l(__local float *object,
                                                      __global float *desired,
                                                      float value) {
  *desired =
      atomic_fetch_add_explicit((volatile __local atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_add_explicit_double_l(__local double *object,
                                                      __global double *desired,
                                                      double value) {
  *desired = atomic_fetch_add_explicit((volatile __local atomic_double *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_add_explicit_scope_double_l(__local double *object,
                                                       __global double *desired,
                                                       double value) {
  *desired =
      atomic_fetch_add_explicit((volatile __local atomic_double *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}

__kernel void test_atomic_fetch_sub_int(__global int *object,
                                        __global int *desired, int value) {
  *desired = atomic_fetch_sub((volatile atomic_int *)object, value);
}
__kernel void test_atomic_fetch_sub_explicit_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired = atomic_fetch_sub_explicit((volatile atomic_int *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_sub_explicit_scope_int(__global int *object,
                                                  __global int *desired,
                                                  int value) {
  *desired =
      atomic_fetch_sub_explicit((volatile atomic_int *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_sub_uint(__global uint *object,
                                         __global uint *desired, uint value) {
  *desired = atomic_fetch_sub((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_fetch_sub_explicit_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired = atomic_fetch_sub_explicit((volatile atomic_uint *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_sub_explicit_scope_uint(__global uint *object,
                                                   __global uint *desired,
                                                   uint value) {
  *desired =
      atomic_fetch_sub_explicit((volatile atomic_uint *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_sub_long(__global long *object,
                                         __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_sub((volatile atomic_long *)object, value);
}
__kernel void test_atomic_fetch_sub_explicit_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_sub_explicit((volatile atomic_long *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_sub_explicit_scope_long(__global long *object,
                                                   __global long *desired,
                                                   long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_sub_explicit((volatile atomic_long *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_sub_ulong(__global ulong *object,
                                          __global ulong *desired,
                                          ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_sub((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_fetch_sub_explicit_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_sub_explicit((volatile atomic_ulong *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_sub_explicit_scope_ulong(__global ulong *object,
                                                    __global ulong *desired,
                                                    ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_sub_explicit((volatile atomic_ulong *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_or_int(__global int *object,
                                       __global int *desired, int value) {
  *desired = atomic_fetch_or((volatile atomic_int *)object, value);
}
__kernel void test_atomic_fetch_or_explicit_int(__global int *object,
                                                __global int *desired,
                                                int value) {
  *desired = atomic_fetch_or_explicit((volatile atomic_int *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_fetch_or_explicit_scope_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired =
      atomic_fetch_or_explicit((volatile atomic_int *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_or_uint(__global uint *object,
                                        __global uint *desired, uint value) {
  *desired = atomic_fetch_or((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_fetch_or_explicit_uint(__global uint *object,
                                                 __global uint *desired,
                                                 uint value) {
  *desired = atomic_fetch_or_explicit((volatile atomic_uint *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_fetch_or_explicit_scope_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired =
      atomic_fetch_or_explicit((volatile atomic_uint *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_or_long(__global long *object,
                                        __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_or((volatile atomic_long *)object, value);
}
__kernel void test_atomic_fetch_or_explicit_long(__global long *object,
                                                 __global long *desired,
                                                 long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_or_explicit((volatile atomic_long *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_fetch_or_explicit_scope_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_or_explicit((volatile atomic_long *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_or_ulong(__global ulong *object,
                                         __global ulong *desired, ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_or((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_fetch_or_explicit_ulong(__global ulong *object,
                                                  __global ulong *desired,
                                                  ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_or_explicit((volatile atomic_ulong *)object, value,
                                      memory_order_relaxed);
}
__kernel void atomic_fetch_or_explicit_scope_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_or_explicit((volatile atomic_ulong *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_xor_int(__global int *object,
                                        __global int *desired, int value) {
  *desired = atomic_fetch_xor((volatile atomic_int *)object, value);
}
__kernel void test_atomic_fetch_xor_explicit_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired = atomic_fetch_xor_explicit((volatile atomic_int *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_xor_explicit_scope_int(__global int *object,
                                                  __global int *desired,
                                                  int value) {
  *desired =
      atomic_fetch_xor_explicit((volatile atomic_int *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_xor_uint(__global uint *object,
                                         __global uint *desired, uint value) {
  *desired = atomic_fetch_xor((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_fetch_xor_explicit_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired = atomic_fetch_xor_explicit((volatile atomic_uint *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_xor_explicit_scope_uint(__global uint *object,
                                                   __global uint *desired,
                                                   uint value) {
  *desired =
      atomic_fetch_xor_explicit((volatile atomic_uint *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_xor_long(__global long *object,
                                         __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_xor((volatile atomic_long *)object, value);
}
__kernel void test_atomic_fetch_xor_explicit_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_xor_explicit((volatile atomic_long *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_xor_explicit_scope_long(__global long *object,
                                                   __global long *desired,
                                                   long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_xor_explicit((volatile atomic_long *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_xor_ulong(__global ulong *object,
                                          __global ulong *desired,
                                          ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_xor((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_fetch_xor_explicit_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_xor_explicit((volatile atomic_ulong *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_xor_explicit_scope_ulong(__global ulong *object,
                                                    __global ulong *desired,
                                                    ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_xor_explicit((volatile atomic_ulong *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_and_int(__global int *object,
                                        __global int *desired, int value) {
  *desired = atomic_fetch_and((volatile atomic_int *)object, value);
}
__kernel void test_atomic_fetch_and_explicit_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired = atomic_fetch_and_explicit((volatile atomic_int *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_and_explicit_scope_int(__global int *object,
                                                  __global int *desired,
                                                  int value) {
  *desired =
      atomic_fetch_and_explicit((volatile atomic_int *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_and_uint(__global uint *object,
                                         __global uint *desired, uint value) {
  *desired = atomic_fetch_and((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_fetch_and_explicit_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired = atomic_fetch_and_explicit((volatile atomic_uint *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_and_explicit_scope_uint(__global uint *object,
                                                   __global uint *desired,
                                                   uint value) {
  *desired =
      atomic_fetch_and_explicit((volatile atomic_uint *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_and_long(__global long *object,
                                         __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_and((volatile atomic_long *)object, value);
}
__kernel void test_atomic_fetch_and_explicit_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_and_explicit((volatile atomic_long *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_and_explicit_scope_long(__global long *object,
                                                   __global long *desired,
                                                   long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_and_explicit((volatile atomic_long *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_and_ulong(__global ulong *object,
                                          __global ulong *desired,
                                          ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_and((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_fetch_and_explicit_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_and_explicit((volatile atomic_ulong *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_and_explicit_scope_ulong(__global ulong *object,
                                                    __global ulong *desired,
                                                    ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_and_explicit((volatile atomic_ulong *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_int(__global int *object,
                                        __global int *desired, int value) {
  *desired = atomic_fetch_min((volatile atomic_int *)object, value);
}
__kernel void test_atomic_fetch_min_explicit_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired = atomic_fetch_min_explicit((volatile atomic_int *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_int(__global int *object,
                                                  __global int *desired,
                                                  int value) {
  *desired =
      atomic_fetch_min_explicit((volatile atomic_int *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_uint(__global uint *object,
                                         __global uint *desired, uint value) {
  *desired = atomic_fetch_min((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_fetch_min_explicit_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired = atomic_fetch_min_explicit((volatile atomic_uint *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_uint(__global uint *object,
                                                   __global uint *desired,
                                                   uint value) {
  *desired =
      atomic_fetch_min_explicit((volatile atomic_uint *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_long(__global long *object,
                                         __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_min((volatile atomic_long *)object, value);
}
__kernel void test_atomic_fetch_min_explicit_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_min_explicit((volatile atomic_long *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_long(__global long *object,
                                                   __global long *desired,
                                                   long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_min_explicit((volatile atomic_long *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_ulong(__global ulong *object,
                                          __global ulong *desired,
                                          ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_min((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_fetch_min_explicit_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_min_explicit((volatile atomic_ulong *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_ulong(__global ulong *object,
                                                    __global ulong *desired,
                                                    ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_min_explicit((volatile atomic_ulong *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}

// atomic min with float and double type
__kernel void test_atomic_fetch_min_explicit_float(__global float *object,
                                                   __global float *desired,
                                                   float value) {
  *desired = atomic_fetch_min_explicit((volatile atomic_float *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_float(__global float *object,
                                                    __global float *desired,
                                                    float value) {
  *desired =
      atomic_fetch_min_explicit((volatile atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_explicit_double(__global double *object,
                                                    __global double *desired,
                                                    double value) {
  *desired = atomic_fetch_min_explicit((volatile atomic_double *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_double(__global double *object,
                                                     __global double *desired,
                                                     double value) {
  *desired =
      atomic_fetch_min_explicit((volatile atomic_double *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_explicit_float_g(__global float *object,
                                                     __global float *desired,
                                                     float value) {
  *desired = atomic_fetch_min_explicit((volatile __global atomic_float *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_float_g(__global float *object,
                                                      __global float *desired,
                                                      float value) {
  *desired =
      atomic_fetch_min_explicit((volatile __global atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_explicit_double_g(__global double *object,
                                                      __global double *desired,
                                                      double value) {
  *desired = atomic_fetch_min_explicit(
      (volatile __global atomic_double *)object, value, memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_double_g(__global double *object,
                                                       __global double *desired,
                                                       double value) {
  *desired = atomic_fetch_min_explicit(
      (volatile __global atomic_double *)object, value, memory_order_seq_cst,
      memory_scope_device);
}
__kernel void test_atomic_fetch_min_explicit_float_l(__local float *object,
                                                     __global float *desired,
                                                     float value) {
  *desired = atomic_fetch_min_explicit((volatile __local atomic_float *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_float_l(__local float *object,
                                                      __global float *desired,
                                                      float value) {
  *desired =
      atomic_fetch_min_explicit((volatile __local atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_explicit_double_l(__local double *object,
                                                      __global double *desired,
                                                      double value) {
  *desired = atomic_fetch_min_explicit((volatile __local atomic_double *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_min_explicit_scope_double_l(__local double *object,
                                                       __global double *desired,
                                                       double value) {
  *desired =
      atomic_fetch_min_explicit((volatile __local atomic_double *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}

__kernel void test_atomic_fetch_max_int(__global int *object,
                                        __global int *desired, int value) {
  *desired = atomic_fetch_max((volatile atomic_int *)object, value);
}
__kernel void test_atomic_fetch_max_explicit_int(__global int *object,
                                                 __global int *desired,
                                                 int value) {
  *desired = atomic_fetch_max_explicit((volatile atomic_int *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_int(__global int *object,
                                                  __global int *desired,
                                                  int value) {
  *desired =
      atomic_fetch_max_explicit((volatile atomic_int *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_uint(__global uint *object,
                                         __global uint *desired, uint value) {
  *desired = atomic_fetch_max((volatile atomic_uint *)object, value);
}
__kernel void test_atomic_fetch_max_explicit_uint(__global uint *object,
                                                  __global uint *desired,
                                                  uint value) {
  *desired = atomic_fetch_max_explicit((volatile atomic_uint *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_uint(__global uint *object,
                                                   __global uint *desired,
                                                   uint value) {
  *desired =
      atomic_fetch_max_explicit((volatile atomic_uint *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_long(__global long *object,
                                         __global long *desired, long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_max((volatile atomic_long *)object, value);
}
__kernel void test_atomic_fetch_max_explicit_long(__global long *object,
                                                  __global long *desired,
                                                  long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_max_explicit((volatile atomic_long *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_long(__global long *object,
                                                   __global long *desired,
                                                   long value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_max_explicit((volatile atomic_long *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_ulong(__global ulong *object,
                                          __global ulong *desired,
                                          ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_max((volatile atomic_ulong *)object, value);
}
__kernel void test_atomic_fetch_max_explicit_ulong(__global ulong *object,
                                                   __global ulong *desired,
                                                   ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired = atomic_fetch_max_explicit((volatile atomic_ulong *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_ulong(__global ulong *object,
                                                    __global ulong *desired,
                                                    ulong value) {
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
  *desired =
      atomic_fetch_max_explicit((volatile atomic_ulong *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}

__kernel void test_atomic_fetch_add_ptrdiff_t(__global ptrdiff_t *object,
                                              __global ptrdiff_t *desired) {
  ptrdiff_t value = 1;
  *desired = atomic_fetch_add((volatile atomic_uintptr_t *)object, value);
}
__kernel void
test_atomic_fetch_add_explicit_ptrdiff_t(__global ptrdiff_t *object,
                                         __global ptrdiff_t *desired) {
  ptrdiff_t value = 1;
  *desired = atomic_fetch_add_explicit((volatile atomic_uintptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_add_explicit_scope_ptrdiff_t(__global ptrdiff_t *object,
                                          __global ptrdiff_t *desired) {
  ptrdiff_t value = 1;
  *desired =
      atomic_fetch_add_explicit((volatile atomic_uintptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_sub_ptrdiff_t(__global ptrdiff_t *object,
                                              __global ptrdiff_t *desired) {
  ptrdiff_t value = 1;
  *desired = atomic_fetch_sub((volatile atomic_uintptr_t *)object, value);
}
__kernel void
test_atomic_fetch_sub_explicit_ptrdiff_t(__global ptrdiff_t *object,
                                         __global ptrdiff_t *desired) {
  ptrdiff_t value = 1;
  *desired = atomic_fetch_sub_explicit((volatile atomic_uintptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_sub_explicit_scope_ptrdiff_t(__global ptrdiff_t *object,
                                          __global ptrdiff_t *desired) {
  ptrdiff_t value = 1;
  *desired =
      atomic_fetch_sub_explicit((volatile atomic_uintptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_or_uintptr_t(__global uintptr_t *object,
                                             __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_or((volatile atomic_intptr_t *)object, value);
}
__kernel void
test_atomic_fetch_or_explicit_uintptr_t(__global uintptr_t *object,
                                        __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_or_explicit((volatile atomic_intptr_t *)object, value,
                                      memory_order_relaxed);
}
__kernel void
atomic_fetch_or_explicit_scope_uintptr_t(__global uintptr_t *object,
                                         __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired =
      atomic_fetch_or_explicit((volatile atomic_intptr_t *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_or_intptr_t(__global intptr_t *object,
                                            __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_or((volatile atomic_uintptr_t *)object, value);
}
__kernel void
test_atomic_fetch_or_explicit_intptr_t(__global intptr_t *object,
                                       __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_or_explicit((volatile atomic_uintptr_t *)object,
                                      value, memory_order_relaxed);
}
__kernel void
atomic_fetch_or_explicit_scope_intptr_t(__global intptr_t *object,
                                        __global intptr_t *desired) {
  intptr_t value = 1;
  *desired =
      atomic_fetch_or_explicit((volatile atomic_uintptr_t *)object, value,
                               memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_and_uintptr_t(__global uintptr_t *object,
                                              __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_and((volatile atomic_intptr_t *)object, value);
}
__kernel void
test_atomic_fetch_and_explicit_uintptr_t(__global uintptr_t *object,
                                         __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_and_explicit((volatile atomic_intptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_and_explicit_scope_uintptr_t(__global uintptr_t *object,
                                          __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired =
      atomic_fetch_and_explicit((volatile atomic_intptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_and_intptr_t(__global intptr_t *object,
                                             __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_and((volatile atomic_uintptr_t *)object, value);
}
__kernel void
test_atomic_fetch_and_explicit_intptr_t(__global intptr_t *object,
                                        __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_and_explicit((volatile atomic_uintptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_and_explicit_scope_intptr_t(__global intptr_t *object,
                                         __global intptr_t *desired) {
  intptr_t value = 1;
  *desired =
      atomic_fetch_and_explicit((volatile atomic_uintptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_uintptr_t(__global uintptr_t *object,
                                              __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_max((volatile atomic_intptr_t *)object, value);
}
__kernel void
test_atomic_fetch_max_explicit_uintptr_t(__global uintptr_t *object,
                                         __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_max_explicit((volatile atomic_intptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_max_explicit_scope_uintptr_t(__global uintptr_t *object,
                                          __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired =
      atomic_fetch_max_explicit((volatile atomic_intptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_intptr_t(__global intptr_t *object,
                                             __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_max((volatile atomic_uintptr_t *)object, value);
}
__kernel void
test_atomic_fetch_max_explicit_intptr_t(__global intptr_t *object,
                                        __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_max_explicit((volatile atomic_uintptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_max_explicit_scope_intptr_t(__global intptr_t *object,
                                         __global intptr_t *desired) {
  intptr_t value = 1;
  *desired =
      atomic_fetch_max_explicit((volatile atomic_uintptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}

// atomic max with float and double type
__kernel void test_atomic_fetch_max_explicit_float(__global float *object,
                                                   __global float *desired,
                                                   float value) {
  *desired = atomic_fetch_max_explicit((volatile atomic_float *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_float(__global float *object,
                                                    __global float *desired,
                                                    float value) {
  *desired =
      atomic_fetch_max_explicit((volatile atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_explicit_double(__global double *object,
                                                    __global double *desired,
                                                    double value) {
  *desired = atomic_fetch_max_explicit((volatile atomic_double *)object, value,
                                       memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_double(__global double *object,
                                                     __global double *desired,
                                                     double value) {
  *desired =
      atomic_fetch_max_explicit((volatile atomic_double *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_explicit_float_g(__global float *object,
                                                     __global float *desired,
                                                     float value) {
  *desired = atomic_fetch_max_explicit((volatile __global atomic_float *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_float_g(__global float *object,
                                                      __global float *desired,
                                                      float value) {
  *desired =
      atomic_fetch_max_explicit((volatile __global atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_explicit_double_g(__global double *object,
                                                      __global double *desired,
                                                      double value) {
  *desired = atomic_fetch_max_explicit(
      (volatile __global atomic_double *)object, value, memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_double_g(__global double *object,
                                                       __global double *desired,
                                                       double value) {
  *desired = atomic_fetch_max_explicit(
      (volatile __global atomic_double *)object, value, memory_order_seq_cst,
      memory_scope_device);
}
__kernel void test_atomic_fetch_max_explicit_float_l(__local float *object,
                                                     __global float *desired,
                                                     float value) {
  *desired = atomic_fetch_max_explicit((volatile __local atomic_float *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_float_l(__local float *object,
                                                      __global float *desired,
                                                      float value) {
  *desired =
      atomic_fetch_max_explicit((volatile __local atomic_float *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_max_explicit_double_l(__local double *object,
                                                      __global double *desired,
                                                      double value) {
  *desired = atomic_fetch_max_explicit((volatile __local atomic_double *)object,
                                       value, memory_order_relaxed);
}
__kernel void atomic_fetch_max_explicit_scope_double_l(__local double *object,
                                                       __global double *desired,
                                                       double value) {
  *desired =
      atomic_fetch_max_explicit((volatile __local atomic_double *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}

__kernel void test_atomic_fetch_min_uintptr_t(__global uintptr_t *object,
                                              __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_min((volatile atomic_intptr_t *)object, value);
}
__kernel void
test_atomic_fetch_min_explicit_uintptr_t(__global uintptr_t *object,
                                         __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_min_explicit((volatile atomic_intptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_min_explicit_scope_uintptr_t(__global uintptr_t *object,
                                          __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired =
      atomic_fetch_min_explicit((volatile atomic_intptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_min_intptr_t(__global intptr_t *object,
                                             __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_min((volatile atomic_uintptr_t *)object, value);
}
__kernel void
test_atomic_fetch_min_explicit_intptr_t(__global intptr_t *object,
                                        __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_min_explicit((volatile atomic_uintptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_min_explicit_scope_intptr_t(__global intptr_t *object,
                                         __global intptr_t *desired) {
  intptr_t value = 1;
  *desired =
      atomic_fetch_min_explicit((volatile atomic_uintptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_xor_uintptr_t(__global uintptr_t *object,
                                              __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_xor((volatile atomic_intptr_t *)object, value);
}
__kernel void
test_atomic_fetch_xor_explicit_uintptr_t(__global uintptr_t *object,
                                         __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired = atomic_fetch_xor_explicit((volatile atomic_intptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_xor_explicit_scope_uintptr_t(__global uintptr_t *object,
                                          __global uintptr_t *desired) {
  uintptr_t value = 1;
  *desired =
      atomic_fetch_xor_explicit((volatile atomic_intptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_fetch_xor_intptr_t(__global intptr_t *object,
                                             __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_xor((volatile atomic_uintptr_t *)object, value);
}
__kernel void
test_atomic_fetch_xor_explicit_intptr_t(__global intptr_t *object,
                                        __global intptr_t *desired) {
  intptr_t value = 1;
  *desired = atomic_fetch_xor_explicit((volatile atomic_uintptr_t *)object,
                                       value, memory_order_relaxed);
}
__kernel void
atomic_fetch_xor_explicit_scope_intptr_t(__global intptr_t *object,
                                         __global intptr_t *desired) {
  intptr_t value = 1;
  *desired =
      atomic_fetch_xor_explicit((volatile atomic_uintptr_t *)object, value,
                                memory_order_seq_cst, memory_scope_device);
}
__kernel void test_atomic_flag_test_and_set_bool(__global bool *object) {
  bool value = atomic_flag_test_and_set((volatile atomic_flag *)object);
}
__kernel void
test_atomic_flag_test_and_set_explicit_bool(__global bool *object) {
  bool value = atomic_flag_test_and_set_explicit((volatile atomic_flag *)object,
                                                 memory_order_relaxed);
}
__kernel void
atomic_flag_test_and_set_explicit_scope_bool(__global bool *object) {
  bool value = atomic_flag_test_and_set_explicit((volatile atomic_flag *)object,
                                                 memory_order_seq_cst,
                                                 memory_scope_device);
}
__kernel void test_atomic_flag_clear_bool(__global bool *object) {
  atomic_flag_clear((volatile atomic_flag *)object);
}
__kernel void test_atomic_flag_clear_explicit_bool(__global bool *object) {
  atomic_flag_clear_explicit((volatile atomic_flag *)object,
                             memory_order_relaxed);
}
__kernel void atomic_flag_clear_explicit_scope_bool(__global bool *object) {
  atomic_flag_clear_explicit((volatile atomic_flag *)object,
                             memory_order_seq_cst, memory_scope_device);
}
