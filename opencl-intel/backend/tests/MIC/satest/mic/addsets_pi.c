// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=0

__kernel void test_addsets_pi (__global int *out, 
			       __global const int *in1,
			       __global const int *in2) {
    int index = get_global_id(0);
    out[index] = add_sat(in1[index], in2[index]);
}
