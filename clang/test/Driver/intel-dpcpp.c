/// tests for --dpcpp behaviors
//
// REQUIRES: x86-registered-target

// --dpcpp implies -fveclib=SVML
// RUN: %clang -### -c -fsycl --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK-DPCPP-DEFAULTS %s
// RUN: %clang_cl -### -c -fsycl --dpcpp %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=CHECK-DPCPP-DEFAULTS,CHECK-DPCPP-DEFAULTS-WIN %s
// CHECK-DPCPP-DEFAULTS: "-fveclib=SVML"
// CHECK-DPCPP-DEFAULTS-WIN: "--dependent-lib=svml_dispmd"

/// --dpcpp on Windows will allow for Linux based options given the required
/// enabling option.
/// Check for default behaviors (Linux no allowed)
// RUN: %clang_cl --dpcpp -### -qopenmp -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DPCPP_CL_DEFAULT %s
// DPCPP_CL_DEFAULT: unknown argument ignored in clang-cl '-qopenmp'
// DPCPP_CL_DEFAULT-NOT: "-fopenmp"

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

/// Use of -Zi with 'dpcpp' should trigger debug information
// RUN: %clangxx --dpcpp -i_allow-all-opts -### /Zi -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DPCPP_ZI_WIN %s
// DPCPP_ZI_WIN: "-debug-info-kind=limited"

/// Check for deprecation message
// RUN: %clangxx --dpcpp --intel -### -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DEPRECATED %s -DDPCPP_NAME=dpcpp -DICX_NAME=icpx
// RUN: %clang_cl --dpcpp --intel -### -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DEPRECATED %s -DDPCPP_NAME=dpcpp-cl -DICX_NAME=icx-cl
// DEPRECATED: use of '[[DPCPP_NAME]]' is deprecated and will be removed in a future release. Use '[[ICX_NAME]] -fsycl'
