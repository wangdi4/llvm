// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=0

__kernel void test_i64trunc(__global int *out,
                            __global const long *in1) {
    int index = get_global_id(0);
    out[index] = in1[index];
}
