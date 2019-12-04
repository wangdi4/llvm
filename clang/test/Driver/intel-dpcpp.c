/// tests for --dpcpp behaviors
//
// REQUIRES: clang-driver
// REQUIRES: x86-registered-target

// --dpcpp -fopenmp not supported
// When running the 'dpcpp' driver, the --dpcpp option is used to signify the
// source of how the compiler is invoked.
// RUN: %clang -### -c --dpcpp -fopenmp %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-DPCPP-FOPENMP %s
// RUN: %clang -### -c --dpcpp -fopenmp=libiomp5 %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-DPCPP-FOPENMP-ARG %s
// CHECK-DPCPP-FOPENMP: error: unsupported option '-fopenmp'
// CHECK-DPCPP-FOPENMP-ARG: error: unsupported option '-fopenmp=libiomp5'

// --dpcpp -fsycl implies -fsycl-unnamed-lambda
// RUN: %clang -### -c --dpcpp -fsycl %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-DPCPP-SYCL %s
// CHECK-DPCPP-SYCL: clang{{.*}} "-fsycl-is-device" {{.*}} "-fsycl-unnamed-lambda"

// RUN: %clang -### -c --dpcpp -fsycl -fno-sycl-unnamed-lambda %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-DPCPP-SYCL2 %s
// CHECK-DPCPP-SYCL2: clang{{.*}} "-fsycl-is-device"
// CHECK-DPCPP-SYCL2-NOT: "-fsycl-unnamed-lambda"

