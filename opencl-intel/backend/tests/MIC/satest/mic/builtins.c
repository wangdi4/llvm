// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1 -neat

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_builtins(__global float *out, __global const float *in) {
    int index = get_global_id(0);
    out[index] = exp( in[index]);
}
