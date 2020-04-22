/// tests for --dpcpp behaviors
//
// REQUIRES: clang-driver
// REQUIRES: x86-registered-target

// --dpcpp -fsycl implies -fsycl-unnamed-lambda
// RUN: %clang -### -c --dpcpp -fsycl %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-DPCPP-SYCL %s
// CHECK-DPCPP-SYCL: clang{{.*}} "-fsycl-is-device" {{.*}} "-fsycl-unnamed-lambda"

// RUN: %clang -### -c --dpcpp -fsycl -fno-sycl-unnamed-lambda %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-DPCPP-SYCL2 %s
// CHECK-DPCPP-SYCL2: clang{{.*}} "-fsycl-is-device"
// CHECK-DPCPP-SYCL2-NOT: "-fsycl-unnamed-lambda"

