// RUN: %clang_cc1 -fintel-compatibility -dM -E -triple x86_64-apple-darwin %s | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -dM -E -triple i686-apple-darwin %s | FileCheck %s
// RUN: %clang_cc1 -dM -E -triple x86_64-apple-darwin %s | FileCheck %s -check-prefix CHECK2
// RUN: %clang_cc1 -fintel-compatibility -dM -E -triple x86_64-unknown-linux %s | FileCheck %s -check-prefix CHECK3

// CHECK-NOT: __clang_minor__
// CHECK-NOT: __clang_major__
// CHECK-NOT: __clang_version__
// CHECK-NOT: __APPLE_CC__
// CHECK-NOT: __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__

// CHECK2-DAG: __clang_minor__
// CHECK2-DAG: __clang_major__
// CHECK2-DAG: __clang_version__
// CHECK2-DAG: __APPLE_CC__
// CHECK2-DAG: __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__

// CHECK3-DAG: __clang_minor__
// CHECK3-DAG: __clang_major__
// CHECK3-DAG: __clang_version__

