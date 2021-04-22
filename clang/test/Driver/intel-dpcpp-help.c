/// tests for usage of dpcpp

// RUN: %clang --dpcpp -help %s 2>&1 \
// RUN:  | FileCheck -check-prefix HELP-CHECK %s
// HELP-CHECK: USAGE: dpcpp [options] file...

// RUN: %clang_cl -help --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix HELP-CHECK1 %s
// HELP-CHECK1: USAGE: dpcpp [options] file...

// RUN: not %clang --dpcpp --- %s 2>&1 \
// RUN:  | FileCheck -check-prefix SUPPORT-CHECK %s
// SUPPORT-CHECK: dpcpp: error: unsupported option '---'

// RUN: not %clang_cl --dpcpp --- %s 2>&1 \
// RUN:  | FileCheck -check-prefix SUPPORT-CHECK1 %s
// SUPPORT-CHECK1: dpcpp: warning: unknown argument ignored in clang-cl: '---' [-Wunknown-argument]

/// -help information for dpcpp contains only DPC++ specific options
/// Use -v with -help for the full list.
// RUN: %clang --dpcpp -help 2>&1 \
// RUN:  | FileCheck -check-prefix HELP-LIMITED-CHECK %s
// HELP-LIMITED-CHECK: fsycl
// HELP-LIMITED-CHECK: Xsycl-target-backend
// HELP-LIMITED-CHECK: Use '-help -v'
// HELP-LIMITED-CHECK-NOT: Xpreprocessor

// RUN: %clang --dpcpp -help -v 2>&1 \
// RUN:  | FileCheck -check-prefix HELP-FULL-CHECK %s
// HELP-FULL-CHECK: fsycl
// HELP-FULL-CHECK: Xpreprocessor
// HELP-FULL-CHECK: Xsycl-target-backend
// HELP-FULL-CHECK-NOT: Use '-help -v'
