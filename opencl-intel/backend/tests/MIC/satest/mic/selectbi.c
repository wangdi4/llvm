// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml -tsize=0

__kernel void test_selectbi (__global unsigned char *out, 
			   __global const unsigned char *in1,
			   __global const unsigned char *in2,
                           __global const unsigned char *in3) {
    int index = get_global_id(0);
    out[index] = select (in1[index], in2[index], in3[index]);
}
