// Test that we print pass structure with new and legacy PM.
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// RUN: %clang -fexperimental-new-pass-manager -fdebug-pass-structure -fintegrated-as -O3 -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=NEWPM --strict-whitespace
// RUN: %clang -fexperimental-new-pass-manager -flegacy-pass-manager -fdebug-pass-structure -O0 -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=LEGACYPM
// end INTEL_CUSTOMIZATION
// REQUIRES: asserts

// should have proper indentation, should not print any analysis information
// NEWPM-NOT: Running analysis
// NEWPM: Running{{.*}}GlobalOptPass on [module] ;INTEL
// NEWPM: Running pass: RequireAnalysisPass<{{.*}}> on [module] ;INTEL
// NEWPM: GlobalOptPass on [module] ;INTEL
// NEWPM-NOT: Invalidating analysis

// LEGACYPM: Pass Arguments:
=======
// RUN: %clang -fdebug-pass-structure -fintegrated-as -O3 -S -emit-llvm %s -o /dev/null 2>&1 | FileCheck %s --strict-whitespace
// REQUIRES: asserts

// should have proper indentation, should not print any analysis information
// CHECK-NOT: Running analysis
// CHECK: {{^}}Running{{.*}}GlobalOptPass
// CHECK: {{^}}  Running{{.*}}RequireAnalysisPass{{.*}}GlobalsAA
// CHECK: GlobalOptPass
// CHECK-NOT: Invalidating analysis
>>>>>>> 29363f80a80685fcab6b963ec128a923d05dc151

void f(void) {}
