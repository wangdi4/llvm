__attribute__((noinline)) int foo(int a) { return a * 2; }

kernel void test(global int *dst) { dst[0] = foo(dst[0]); }
