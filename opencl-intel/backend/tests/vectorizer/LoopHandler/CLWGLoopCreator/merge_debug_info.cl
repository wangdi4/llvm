// RUN: SATest -BUILD -config=%s.cfg -dump-llvm-file=%t.merged.ll
// RUN: FileCheck %s --input-file=%t.merged.ll
//
// Test what the vector subprogram DIE is removed along w\ the vectorized kernel
// and only the scalar subprogram DIE remains in the module
//
// CHECK:       define void @test
// CHECK-NOT:   define void @__Vectorized_.test
// CHECK:       @test{{.+}}DW_TAG_subprogram
// CHECK-NOT:   @__Vectorized_.test{{.+}}DW_TAG_subprogram

kernel void test(global float* lhs, global float* rhs, global float* out) {
  size_t gid = get_global_id(0);
  out[gid] = lhs[gid] * rhs[gid];
}
