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

// --dpcpp implies -fveclib=SVML
// RUN: %clang -### -c --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK-DPCPP-DEFAULTS %s
// RUN: %clang_cl -### -c --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=CHECK-DPCPP-DEFAULTS,CHECK-DPCPP-DEFAULTS-WIN %s
// CHECK-DPCPP-DEFAULTS: "-fveclib=SVML"
// CHECK-DPCPP-DEFAULTS-WIN: "--dependent-lib=svml_dispmt"

// --dpcpp link defaults
// RUN: touch %t.o
// RUN: %clang -### -no-canonical-prefixes --dpcpp -target x86_64-unknown-linux --gcc-toolchain="" --sysroot=%S/Inputs/basic_linux_tree %t.o 2>&1 | FileCheck -check-prefix CHECK-DPCPP-LIBS %s
// CHECK-DPCPP-LIBS: "{{.*}}ld{{(.exe)?}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-DPCPP-LIBS: "-L{{.*}}../compiler/lib/intel64_lin" "-L{{.*}}bin/../lib"
// CHECK-DPCPP-LIBS: "-L[[SYSROOT]]/usr/lib/gcc/x86_64-unknown-linux/4.6.0"
// CHECK-DPCPP-LIBS: "-lsvml"
// CHECK-DPCPP-LIBS: "-lirc"

