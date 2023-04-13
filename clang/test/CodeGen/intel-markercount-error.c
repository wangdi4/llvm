// INTEL_FEATURE_MARKERCOUNT
// RUN: not %clang_cc1 -S -triple=x86_64-linux-gnu -emit-llvm -O1 -fmarker-count=me %s 2>&1 | FileCheck --check-prefix=ERROR %s
void f() {}

// CEHCK: ERROR: error: unknown argument: '-fmarker-count=me'
// end INTEL_FEATURE_MARKERCOUNT
