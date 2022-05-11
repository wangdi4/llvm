// REQUIRES: x86-registered-target

/// For sycl, should use -f[no-]sycl-device-lib instead of -device-math-lib
// RUN: %clang -### -fsycl -device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-NOT %s
// RUN: %clang_cl -### -fsycl -device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-NOT %s
// RUN: %clang -### -fsycl -no-device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-NOT %s
// RUN: %clang_cl -### -fsycl -no-device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT-NOT %s
// CHECK-OPT-NOT: argument unused
