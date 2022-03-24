#if INTEL_FEATURE_XUCC
// REQUIRES: intel_feature_xucc

// RUN: %clang -### -c -mxucc %s 2>&1 | FileCheck %s
// CHECK: "-triple" "x86_64_xucc-unknown-unknown"

#endif // INTEL_FEATURE_XUCC
