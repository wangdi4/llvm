// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=1

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test_v8f64_insert_elem(__global double8* f) {
   int index = get_global_id(0);
    f[index].s1 = 3.14;
    f[index].s5 = 3.14;
    f[index].s6 = 1.772;
}

