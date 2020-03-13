// Test to make sure --dpcpp usage does not emit help or support any OpenMP
// or cuda options.

// RUN: %clangxx -help --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=HELP-CHECK %s
// RUN: %clang_cl -help --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=HELP-CHECK %s
// HELP-CHECK-NOT: openmp
// HELP-CHECK-NOT: cuda
// HELP-CHECK-NOT: fhls 
// HELP-CHECK-NOT: intel-long-double-size

// RUN: not %clangxx -### --dpcpp -fopenmp -fhls --cuda-device-only %s 2>&1 \
// RUN:  | FileCheck -check-prefix=SUPPORT-CHECK %s
// SUPPORT-CHECK: unsupported option '-fopenmp'
// SUPPORT-CHECK: unsupported option '-fhls'
// SUPPORT-CHECK: unsupported option '--cuda-device-only'

// RUN: not %clang_cl -### --dpcpp -openmp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=SUPPORT-CHECK-CL %s
// SUPPORT-CHECK-CL: unsupported option '-openmp'
