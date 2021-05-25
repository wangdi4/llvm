// Test to make sure --dpcpp usage does not emit help or support any cuda
// options.

// RUN: %clangxx -help --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=HELP-CHECK %s
// RUN: %clang_cl -help --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=HELP-CHECK %s
// HELP-CHECK-NOT: cuda
// HELP-CHECK-NOT: fhls 
// HELP-CHECK-NOT: intel-long-double-size

// RUN: not %clangxx -### --dpcpp -fhls --cuda-device-only %s 2>&1 \
// RUN:  | FileCheck -check-prefix=SUPPORT-CHECK %s
// SUPPORT-CHECK: unsupported option '-fhls'
// SUPPORT-CHECK: unsupported option '--cuda-device-only'

// RUN: %clangxx -### --dpcpp -fsycl -fsycl-targets=nvptx64-nvidia-cuda-sycl-device %s 2>&1 \
// RUN:  | FileCheck -check-prefix=SUPPORT-CUDA-CHECK %s
// SUPPORT-CUDA-CHECK: SYCL target is invalid: 'nvptx64-nvidia-cuda-sycl-device'

// RUN: %clang_cl -### --dpcpp -fsycl -fsycl-targets=nvptx64-nvidia-cuda-sycl-device %s 2>&1 \
// RUN:  | FileCheck -check-prefix=SUPPORT-CUDA-CHECK-CL %s
// SUPPORT-CUDA-CHECK-CL: SYCL target is invalid: 'nvptx64-nvidia-cuda-sycl-device'

// Use of --dpcpp also should not enable the Cuda installation check.  There
// is no obvious cuda value to check, so just make sure the diagnostic we are
// trying to suppress is not seen.
// RUN: %clangxx -c --dpcpp -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CUDA-WARN %s
// RUN: %clang_cl -c --dpcpp -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CUDA-WARN %s
// CUDA-WARN-NOT: Unknown CUDA version
