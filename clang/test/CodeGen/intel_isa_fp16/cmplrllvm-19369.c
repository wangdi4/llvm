// REQUIRES: intel_feature_isa_fp16
// RUN: %clang_cc1 -S -fdenormal-fp-math-f16=ieee %s -emit-llvm -o - | FileCheck %s --check-prefix=CHECK-IEEE
// RUN: %clang_cc1 -S -fdenormal-fp-math-f16=preserve-sign %s -emit-llvm -o - | FileCheck %s --check-prefix=CHECK-PS
// RUN: %clang_cc1 -S -fdenormal-fp-math-f16=positive-zero %s -emit-llvm -o - | FileCheck %s --check-prefix=CHECK-PZ

// CHECK-LABEL: main

// The ieee,ieee is the default, so omit the attribute
// CHECK-IEEE-NOT:"denormal-fp-math-f16"
// CHECK-PS: attributes #0 = {{.*}}"denormal-fp-math-f16"="preserve-sign,preserve-sign"{{.*}}
// CHECK-PZ: attributes #0 = {{.*}}"denormal-fp-math-f16"="positive-zero,positive-zero"{{.*}}

int main() {
  return 0;
}
