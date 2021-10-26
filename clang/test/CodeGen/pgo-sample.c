// Test if PGO sample use passes are invoked.
//
// Ensure Pass PGOInstrumentationGenPass is invoked.
// INTEL_CUSTOMIZATION
// FIXME: Remove -fno-legacy-pass-manager when xmain changes to use the
//        new pass manager by default
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-sample-use=%S/Inputs/pgo-sample.prof %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s
// end INTEL_CUSTOMIZATION

// CHECK: SimplifyCFGPass
// CHECK: SampleProfileLoaderPass

int func(int a) { return a; }
