// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void zero_imm(__global double *outD, __global float *outF, __global const double *inD, __global const float *inF) {
    int index = get_global_id(0);
    outF[index] = 0.000000e+00f - inF[index];
    outD[index] = 0.000000e+00 - inD[index];
}

