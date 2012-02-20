// This is a sample.
// RUN: pwd
// RUN: clang -cc1 -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ -include opencl_.h -o %s.bc %s
// RUN: sde -- SATest -config=%s.cfg.xml -cpuarch=knf -tsize=1 -single_wg -trace=false -vtune=false -cpufeatures="" -detailed_stat

__kernel
void test_fadd4(__global float4 *buf1, __global float4 *buf2, unsigned count) {
    buf1[0] = buf1[0] + buf2[0];
}
