// Verify that Dead Arguments Elimination for SYCL kernels is/is not added to the PM.

// INTEL_CUSTOMIZATION
// RUN: %clang -cc1 -triple spir64-unknown-unknown -fno-legacy-pass-manager -fsycl-is-device  -emit-llvm-bc  -O1 -fdebug-pass-manager -fenable-sycl-dae -o /dev/null -x c++ < %s 2>&1 | FileCheck %s
// RUN: %clang -cc1 -triple spir64-unknown-unknown -fno-legacy-pass-manager -fsycl-is-device  -emit-llvm-bc  -O0 -fdebug-pass-manager  -o /dev/null -x c++ < %s 2>&1 | FileCheck %s --check-prefix DISABLE
// RUN: %clang -cc1 -triple spir64-unknown-unknown -fno-legacy-pass-manager -fsycl-is-device  -emit-llvm-bc  -O1 -fdebug-pass-manager -fenable-sycl-dae -disable-llvm-passes -o /dev/null -x c++ < %s 2>&1 | FileCheck %s --check-prefix DISABLE
// end INTEL_CUSTOMIZATION


// CHECK: Running pass: DeadArgumentEliminationSYCLPass on [module]
// DISABLE-NOT: DeadArgumentEliminationSYCLPass
