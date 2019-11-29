//
// This test verifies that the GNUC macros are not predefined when the
// -fintel-compatibility flag is on, but that they are on without that
// flag.
//

// Check that the macros are properly defined with or without the
// -fintel-compatibility option.  This doesn't check for any specific values
// of these predefined macros.

// RUN: %clang_cc1 -fgnuc-version=4.2.1 -E -dM -triple i686-pc-linux %s -o - | \
// RUN:     FileCheck %s -check-prefix CHECK-GNU-VERSION

// RUN: %clang_cc1 -fgnuc-version=4.2.1 -E -dM -triple i686-pc-linux \
// RUN:    -fintel-compatibility %s -o - | \
// RUN:    FileCheck %s -check-prefix CHECK-GNU-VERSION

// CHECK-GNU-VERSION: __GNUC_MINOR__
// CHECK-GNU-VERSION: __GNUC_PATCHLEVEL__
// CHECK-GNU-VERSION: __GNUC__
// CHECK-GNU-VERSION: __GXX_ABI_VERSION
