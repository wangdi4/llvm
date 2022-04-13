// Test that we print pass structure with new and legacy PM.
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

void f(void) {}
