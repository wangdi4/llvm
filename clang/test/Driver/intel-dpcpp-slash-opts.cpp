// These are tests for slash alias of dpcpp options.

// /fintelfpga
// RUN:   %clang_cl -### -fsycl /fintelfpga %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fopenmp-device-lib
// RUN:   %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64 /fopenmp-device-lib=libc %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fno-openmp-device-lib
// RUN:   %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64 /fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-dead-args-optimization
// RUN:   %clang_cl -### -fsycl /fsycl-dead-args-optimization %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fno-sycl-dead-args-optimization
// RUN:   %clang_cl -### -fsycl /fno-sycl-dead-args-optimization %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-device-lib
// RUN: %clang_cl -### -fsycl /fsycl-device-lib=libc %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fno-sycl-device-lib=libc
// RUN: %clang_cl -### -fsycl /fno-sycl-device-lib=libc %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-early-optimizations
// RUN:   %clang_cl -### -fsycl /fsycl-early-optimizations %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fno-sycl-early-optimizations
// RUN:   %clang_cl -### -fsycl /fno-sycl-early-optimizations %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-link
// RUN:   %clang_cl -### -fsycl /fsycl-link %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-targets
// RUN:   %clang_cl -### -fsycl /fsycl-targets=spir64_x86_64-unknown-unknown-sycldevice %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-unnamed-lambda
// RUN: %clang_cl -### -fsycl /fsycl-unnamed-lambda %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-ERROR %s

// /fno-sycl-unnamed-lambda
// RUN: %clang_cl -### -fsycl /fno-sycl-unnamed-lambda %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /v
// RUN: %clang_cl -### /v %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /Xopenmp-target-frontend
// RUN: %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64 /Xopenmp-target-frontend -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /Xopenmp-target-backend
// RUN: %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64 /Xopenmp-target-backend -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /Xopenmp-target-linker
// RUN: %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64 /Xopenmp-target-linker -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /Xs
// RUN:   %clang_cl -### -fsycl /XsDFOO1 /XsDFOO2 /Xshardware %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### -fsycl /Xs "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /Xsycl-target-frontend
// RUN:   %clang_cl -### -fsycl -fsycl-targets=spir64-unknown-unknown-sycldevice /Xsycl-target-frontend -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /Xsycl-target-backend
// RUN:   %clang_cl -### -fsycl -fsycl-targets=spir64-unknown-unknown-sycldevice /Xsycl-target-backend -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// /Xsycl-target-linker
// RUN:   %clang_cl -### -fsycl -fsycl-targets=spir64-unknown-unknown-sycldevice /Xsycl-target-linker -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK-ERROR %s

// CHECK-ERROR-NOT: error: