// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir -fsyntax-only -verify %s

__attribute__((task)) // expected-warning{{unknown attribute 'task' ignored}}
__kernel void bar(__global int* out)
{
    *out = 0;
}

__attribute__((num_compute_units(43))) // expected-warning{{unknown attribute 'num_compute_units' ignored}}
__kernel void far(__global int* out)
{
    *out = 0;
}

__attribute__((num_simd_work_items(43))) // expected-warning{{unknown attribute 'num_simd_work_items' ignored}}
__kernel void tar(__global int* out)
{
    *out = 0;
}

__kernel void car(__local __attribute__((local_mem_size(43))) int* out) // expected-warning{{unknown attribute 'local_mem_size' ignored}}
{
    *out = 0;
}

__kernel void rar(__global __attribute__((buffer_location("DDR"))) int* out) // expected-warning{{unknown attribute 'buffer_location' ignored}}
{
    *out = 0;
}

__attribute__((max_work_group_size(43, 43, 43))) // expected-warning{{unknown attribute 'max_work_group_size' ignored}}
__kernel void war(__global int* out)
{
    *out = 0;
}
