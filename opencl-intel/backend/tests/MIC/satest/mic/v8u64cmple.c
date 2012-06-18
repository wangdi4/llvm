// RUN: pwd
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1

__kernel void test_v8u64cmple(__global long8 *out, __global const ulong8 *a, __global const ulong8 *b) {
    int index = get_global_id(0);
    out[index] = a[index] <= b[index];
}
