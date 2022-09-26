// INTEL_FEATURE_ISA_PREFETCHI
// REQUIRES: intel_feature_isa_prefetchi
// RUN: %clang_cc1 -no-opaque-pointers -ffreestanding -triple x86_64-unknown-unknown -target-feature +prefetchi -emit-llvm -o - %s | FileCheck %s


#include <x86intrin.h>

void test_m_prefetch_it0(void *p) {
  return _m_prefetchit0(p);
  // CHECK-LABEL: define{{.*}} void @test_m_prefetch_it0
  // CHECK: call void @llvm.prefetch.p0i8({{.*}}, i32 1, i32 3, i32 0)
}

void test_m_prefetch_it1(void *p) {
  return _m_prefetchit1(p);
  // CHECK-LABEL: define{{.*}} void @test_m_prefetch_it1
  // CHECK: call void @llvm.prefetch.p0i8({{.*}}, i32 1, i32 2, i32 0)
}
// end INTEL_FEATURE_ISA_PREFETCHI
