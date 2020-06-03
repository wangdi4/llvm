// REQUIRES: clang-driver
// REQUIRES: x86-registered-target

/// Simple tests for -device-math-lib acceptance.  More complex testing can
/// be accomplished with full environment, as the objects are only pulled in
/// when they exist.
// RUN: %clang -### -fsycl -device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT %s
// RUN: %clang_cl -### -fsycl -device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT %s
// RUN: %clang -### -fsycl -no-device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT %s
// RUN: %clang_cl -### -fsycl -no-device-math-lib=fp64 %s 2>&1 | FileCheck -check-prefix=CHECK-OPT %s
// CHECK-OPT-NOT: argument unused
// CHECK-OPT-NOT: unknown argument
