__attribute__((noinline)) int foo(global int *dst) {
  dst[get_global_id(0)] = 0;
}

kernel void test(global int *dst) { foo(dst); }
