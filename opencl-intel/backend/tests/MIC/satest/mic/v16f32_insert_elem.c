// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_v16f32_insert_elem(__global float16* vf, __global double8* vd, __global float* f, __global double* d) {
   int index = get_global_id(0);
   vf[index].s7 = 2.2;
   vf[index].s1 = 3.14;
   vf[index].s5 = f[index];
   vd[index].s3 = d[index];
   vd[index] *= vd[index];
}
