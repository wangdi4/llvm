// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: sde -- SATest -config=%s.cfg.xml -cpuarch=knf -tsize=1 \
// RUN: -single_wg -trace=false -vtune=false -cpufeatures="" -detailed_stat
__kernel void test_int8(__global int *out, __global const char *a, __global const char *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}

__kernel void test_uint8(__global int *out, __global const uchar *a, __global const uchar *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}

__kernel void test_int16(__global int *out, __global const short *a, __global const short *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}

__kernel void test_uint16(__global int *out, __global const ushort *a, __global const ushort *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}

__kernel void test_int32(__global int *out, __global const int *a, __global const int *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}

__kernel void test_uint32(__global int *out, __global const uint *a, __global const uint *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}

__kernel void test_int64(__global int *out, __global const long *a, __global const long *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}

__kernel void test_uint64(__global int *out, __global const ulong *a, __global const ulong *b) {
    size_t id = get_global_id(0);
    out[id] = a[id] == b[id];
}
