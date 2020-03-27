// INTEL_FEATURE_ISA_HRESET
// REQUIRES: intel_feature_isa_hreset
// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-unknown-unknown -target-feature +hreset -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -ffreestanding -triple i386-unknown-unknown -target-feature +hreset -emit-llvm -o - | FileCheck %s

#include <immintrin.h>

void test_hreset(int a)
{
// CHECK-LABEL: test_hreset
// CHECK: call void asm sideeffect "hreset $0", "i,{ax},~{dirflag},~{fpsr},~{flags}"(i32 1, i32 %0)
  _hreset(1, a);
}
// end INTEL_FEATURE_ISA_HRESET
