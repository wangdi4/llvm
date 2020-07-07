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
