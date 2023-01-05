__attribute__((noinline)) void foo(global int *dst) { *dst = 0; }

kernel void test(global int *dst) { foo(dst); }
