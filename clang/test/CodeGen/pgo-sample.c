// Test if PGO sample use passes are invoked.
//
// TODO: remove redundant `-fno-legacy-pass-manager` key from RUN-lines once we move to NewPM by default.
//
// Ensure Pass PGOInstrumentationGenPass is invoked.
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// FIXME: Remove -fno-legacy-pass-manager when xmain changes to use the
//        new pass manager by default
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-sample-use=%S/Inputs/pgo-sample.prof %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s
// end INTEL_CUSTOMIZATION
=======
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-sample-use=%S/Inputs/pgo-sample.prof %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s
>>>>>>> 7fac21021d303bcc836e1cf11b62f8efe4d1f7ba

// CHECK: SimplifyCFGPass
// CHECK: SampleProfileLoaderPass

int func(int a) { return a; }
