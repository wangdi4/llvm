// RUN: pwd
// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: sde -- SATest -config=%s.cfg.xml -cpuarch=knf -tsize=1 \
// RUN: -single_wg -trace=false -vtune=false -cpufeatures="" -detailed_stat
__kernel void test_fxor32(__global float *out, __global const float *in) {
    int index = get_global_id(0);
    out[index] = -0.000000e+00 - in[index];
}
