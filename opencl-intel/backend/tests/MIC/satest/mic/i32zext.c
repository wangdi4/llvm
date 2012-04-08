// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=0

__kernel void test_i32zext (__global unsigned long *out, 
                            __global const unsigned int *in1) {
    int index = get_global_id(0);
    out[index] = in1[index];
}
