// This is a sample.
// This is a sample.
// RUN: pwd
// RUN: clang -cc1 -emit-llvm-bc -x cl -I
// ../../../../../../../src/backend/clang_headers/ -include opencl_.h -o %s.bc
// %s RUN: python ../../bin/SATest.py -config=%s.cfg.xml

__kernel void test_fadd(__global float *buf1, __global float *buf2,
                        unsigned count) {
  buf1[0] = buf1[0] + buf2[0];
}
