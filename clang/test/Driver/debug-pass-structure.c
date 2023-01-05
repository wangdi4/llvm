// Test that we print pass structure with new and legacy PM.
// INTEL_CUSTOMIZATION
// RUN: %clang -fdebug-pass-structure -fintegrated-as -O3 -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --strict-whitespace
// end INTEL_CUSTOMIZATION
// REQUIRES: asserts

// should have proper indentation, should not print any analysis information
// CHECK-NOT: Running analysis
// CHECK: Running{{.*}}GlobalOptPass on [module] ;INTEL
// CHECK: Running pass: RequireAnalysisPass<{{.*}}> on [module] ;INTEL
// CHECK: GlobalOptPass on [module] ;INTEL
// CHECK-NOT: Invalidating analysis

void f(void) {}
