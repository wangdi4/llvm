// Test to make sure --dpcpp usage does not emit help and has some option
// unsupported restrictions.

// RUN: %clangxx -help --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=HELP-CHECK %s
// RUN: %clang_cl -help --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=HELP-CHECK %s
// HELP-CHECK-NOT: /MT
// HELP-CHECK-NOT: /MTd
// HELP-CHECK-NOT: fhls 
// HELP-CHECK-NOT: intel-long-double-size

// RUN: not %clangxx -### --dpcpp -fhls %s 2>&1 \
// RUN:  | FileCheck -check-prefix=SUPPORT-CHECK %s
// SUPPORT-CHECK: unsupported option '-fhls'

// RUN: not %clang_cl --dpcpp -### /MT %s 2>&1 \
// RUN:  | FileCheck -check-prefix=MT-SUPPORT-CHECK %s
// MT-SUPPORT-CHECK: unsupported option '/MT'

// RUN: not %clang_cl --dpcpp -### /MTd %s 2>&1 \
// RUN:  | FileCheck -check-prefix=MTD-SUPPORT-CHECK %s
// MTD-SUPPORT-CHECK: unsupported option '/MTd'
