// REQUIRES: llvm-backend
//
// This test verifies that the GNUC macros are not predefined when the
// -fintel-compatibility flag is on, but that they are on without that
// flag.
//

// Check that the macros are properly defined without the
// -fintel-compatibility option.  This doesn't check for any specific values
// of these predefined macros.

// RUN: %clang_cc1 -E -dM -triple i686-pc-linux %s -o - | \
// RUN:     FileCheck %s -check-prefix CHECK-GNU-VERSION

// CHECK-GNU-VERSION: __GNUC_MINOR__
// CHECK-GNU-VERSION: __GNUC_PATCHLEVEL__
// CHECK-GNU-VERSION: __GNUC__
// CHECK-GNU-VERSION: __GXX_ABI_VERSION

// Check that the macros are not defined when the -fintel-compatibility
// option is used.

// RUN: %clang_cc1 -E -dM -triple i686-pc-linux \
// RUN:    -fintel-compatibility %s -o - | \
// RUN:    FileCheck %s -check-prefix CHECK-NO-GNU-VERSION

// CHECK-NO-GNU-VERSION-NOT: __GNUC_MINOR__
// CHECK-NO-GNU-VERSION-NOT: __GNUC_PATCHLEVEL__
// CHECK-NO-GNU-VERSION-NOT: __GNUC__
// CHECK-NO-GNU-VERSION-NOT: __GXX_ABI_VERSION
