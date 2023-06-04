// INTEL_FEATURE_ISA_BF16_BASE
// REQUIRES: intel_feature_isa_bf16_base
// RUN: %clang_cc1 -triple i386-unknown-unknown -target-feature +sse2 -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -target-feature +sse2 -emit-llvm -o - %s | FileCheck %s

// CHECK: define {{.*}}void @_Z3fooDF16b(bfloat noundef %b)
void foo(__bf16 b) {}

// end INTEL_FEATURE_ISA_BF16_BASE
