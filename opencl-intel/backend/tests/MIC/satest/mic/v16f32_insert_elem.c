// RUN: clang -cc1 -cl-kernel-arg-info -emit-llvm-bc -x cl -I ../../../../../../../src/backend/clang_headers/ \
// RUN: -include opencl_.h -o %s.bc %s
// RUN: sde -- SATest -config=%s.cfg.xml -cpuarch=knf -tsize=1 \
// RUN: -single_wg -trace=false -vtune=false -cpufeatures="" -detailed_stat
__kernel void test_v16f32_insert_elem(__global float16* f) {
   int index = get_global_id(0);
   f[index].s7 = 2.2;
   f[index].s1 = 3.14;
}
