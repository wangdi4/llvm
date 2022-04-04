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
// CHECK-DPCPP-LIBS: "-L{{.*}}..{{(/|\\\\)}}compiler{{(/|\\\\)}}lib{{(/|\\\\)}}intel64_lin" "-L{{.*}}bin{{(/|\\\\)}}..{{(/|\\\\)}}lib"
// CHECK-DPCPP-LIBS: "-L[[SYSROOT]]/usr/lib/gcc/x86_64-unknown-linux/10.2.0"
// CHECK-DPCPP-LIBS: "-lsvml"
// CHECK-DPCPP-LIBS: "-lirc"

/// --dpcpp on Windows will allow for Linux based options given the required
/// enabling option.
/// Check for default behaviors (Linux no allowed)
// RUN: %clang_cl --dpcpp -### -qopenmp -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DPCPP_CL_DEFAULT %s
// DPCPP_CL_DEFAULT: unknown argument ignored in clang-cl '-qopenmp'
// DPCPP_CL_DEFAULT-NOT: "-fopenmp"
/// Check for allowing Linux options
// RUN: %clang_cl --dpcpp /Q_allow-linux -### -qopenmp -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DPCPP_CL_ALLOW_LINUX %s
// DPCPP_CL_ALLOW_LINUX-NOT: unknown argument ignored in clang-cl '-qopenmp'
// DPCPP_CL_ALLOW_LINUX: "-fopenmp"

/// --dpcpp with -i_allow-all-opts will allow for all Windows and Linux options
/// to be enabled.
/// Check for default behaviors (Windows not allowed)
// RUN: not %clangxx --dpcpp -### -Qiopenmp -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DPCPP_DEFAULT %s
// DPCPP_DEFAULT: unknown argument '-Qiopenmp'
// DPCPP_DEFAULT-NOT: "-fopenmp"

/// Check for allowing Windows options
// RUN: %clangxx --dpcpp -i_allow-all-opts -### /Qiopenmp -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DPCPP_ALLOW_WIN %s
// DPCPP_ALLOW_WIN-NOT: unknown argument
// DPCPP_ALLOW_WIN: "-fopenmp"
// DPCPP_ALLOW_WIN-SAME: "-fexceptions"
