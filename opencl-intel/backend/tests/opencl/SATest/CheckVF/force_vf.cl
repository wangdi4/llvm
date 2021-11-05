kernel void test(global int *dst) { dst[get_global_id(0)] = 0; }
