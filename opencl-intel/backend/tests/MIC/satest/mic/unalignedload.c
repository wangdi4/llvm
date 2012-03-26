// This is a sample.
// This is a sample.
// RUN: pwd
// RUN: clang -cc1 -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ -include opencl_.h -o %s.bc %s
// RUN: python ../../bin/SATest.py -config=%s.cfg.xml

__kernel
void test_fadd(__global char *buf1, __global char *buf2, unsigned count) {
  size_t i=get_global_id(0);
  buf2[i+1] = buf1[i];
}
