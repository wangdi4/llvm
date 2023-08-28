// INTEL_FEATURE_ISA_PREFETCHST2
// REQUIRES: intel_feature_isa_prefetchst2
// RUN: %clang_cc1 -no-opaque-pointers -ffreestanding -triple x86_64-unknown-unknown -target-feature +prefetchst2 -emit-llvm -o - %s | FileCheck %s


#include <x86intrin.h>

void test_m_prefetch(void *p) {
  return _m_prefetch(p);
  // CHECK-LABEL: define{{.*}} void @test_m_prefetch
  // CHECK: call void @llvm.prefetch.p0i8({{.*}}, i32 0, i32 3, i32 1)
}

void test_m_prefetch_w(void *p) {
  return _m_prefetchw(p);
  // CHECK-LABEL: define{{.*}} void @test_m_prefetch_w
  // CHECK: call void @llvm.prefetch.p0i8({{.*}}, i32 1, i32 3, i32 1)
}

void test_m_prefetch_s(void *p) {
  return _m_prefetchs(p);
  // CHECK-LABEL: define{{.*}} void @test_m_prefetch_s
  // CHECK: call void @llvm.prefetch.p0i8({{.*}}, i32 2, i32 1, i32 1)
}
// end INTEL_FEATURE_ISA_PREFETCHST2
