// INTEL_FEATURE_ISA_SERIALIZE
// REQUIRES: intel_feature_isa_serialize
// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-unknown-unknown -target-feature +serialize -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -ffreestanding -triple i386-unknown-unknown -target-feature +serialize -emit-llvm -o - | FileCheck %s

#include <immintrin.h>

void test_serialize(void)
{
// CHECK-LABEL: test_serialize
// CHECK: call void @llvm.x86.serialize()
  _serialize();
}
// end INTEL_FEATURE_ISA_SERIALIZE
