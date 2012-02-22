// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1

__kernel void test_v8u64cmplt(__global long8 *out, __global const ulong8 *a, __global const ulong8 *b) {
    int index = get_global_id(0);
    out[index] = a[index] < b[index];
}
