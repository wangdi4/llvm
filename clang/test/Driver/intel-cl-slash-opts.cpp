// These are tests for slash alias of core options.

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

// /fcf-protection:, /fcf-protection=
// RUN:   %clang_cl -### /fcf-protection:full %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fcf-protection=full %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fcoverage-compilation-dir:, /fcoverage-compilation-dir=
// RUN:   %clang_cl -### /fcoverage-compilation-dir:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fcoverage-compilation-dir=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fcoverage-mapping, /fno-coverage-mapping
// RUN:   %clang_cl -### /fcoverage-mapping /fprofile-instr-generate %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fno-coverage-mapping /fprofile-instr-generate %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fcs-profile-generate, /fcs-profile-generate:, /fcs-profile-generate=
// RUN:   %clang_cl -### /fcs-profile-generate %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fcs-profile-generate:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fcs-profile-generate=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /ffile-compilation-dir:, /ffile-compilation-dir=
// RUN:   %clang_cl -### /ffile-compilation-dir:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /ffile-compilation-dir=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-list:, /fprofile-list=
// RUN:   %clang_cl -### /fprofile-list:%s %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-list=%s %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /forder-file-instrumentation
// RUN:   %clang_cl -### /forder-file-instrumentation %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-exclude-files:, /fprofile-exclude-files=
// RUN:   %clang_cl -### /fprofile-exclude-files:foo -coverage %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-exclude-files=foo -coverage %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-filter-files:, /fprofile-filter-files=
// RUN:   %clang_cl -### /fprofile-filter-files:foo -coverage %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-filter-files=foo -coverage %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-generate, /fno-profile-generate, /fprofile-generate:, /fprofile-generate=
// RUN:   %clang_cl -### /fprofile-generate %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fno-profile-generate %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-generate:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-generate=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-instr-generate, /fno-profile-instr-generate, /fprofile-instr-generate:, fprofile-instr-generate=
// RUN:   %clang_cl -### /fprofile-instr-generate %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fno-profile-instr-generate %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-instr-generate:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-instr-generate=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-instr-use, /fno-profile-instr-use, /fprofile-instr-use:, /fprofile-instr-use=
// RUN:   %clang_cl -### /fprofile-instr-use %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fno-profile-instr-use %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-instr-use:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-instr-use=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-remapping-file:, /fprofile-remapping-file=
// RUN:   %clang_cl -### /fprofile-remapping-file:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-remapping-file=:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-sample-use, /fno-profile-sample-use, /fprofile-sample-use:, /fprofile-sample-use=
// RUN:   %clang_cl -### /fprofile-sample-use %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fno-profile-sample-use %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-sample-use:%s %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-sample-use=%s %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fprofile-update:, /fprofile-update=
// RUN:   %clang_cl -### /fprofile-update:single %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fprofile-update=single %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsplit-lto-unit
// RUN:   %clang_cl -### /fsplit-lto-unit %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl, /fno-sycl
// RUN:   %clang_cl -### /fsycl %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fno-sycl %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-add-targets:, /fsycl-add-targets=
// RUN:   %clang_cl -### /fsycl /fsycl-add-targets:spir64-unknown-unknown:dummy.spv %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-add-targets=spir64-unknown-unknown:dummy.spv %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-device-code-split:, /fsycl-device-code-split=
// RUN:   %clang_cl -### /fsycl /fsycl-device-code-split:off %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-device-code-split=off %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-device-only
// RUN:   %clang_cl -### /fsycl /fsycl-device-only %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-enable-function-pointers
// RUN:   %clang_cl -### /fsycl /fsycl-enable-function-pointers %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-footer-path:, /fsycl-footer-path=, /fno-sycl-use-footer
// RUN:   %clang_cl -### /fsycl /fsycl-footer-path:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-footer-path=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fno-sycl-use-footer %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-help:, /fsycl-help=
// RUN:   %clang_cl -### /fsycl /fsycl-help:all %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-help=all %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-host-compiler:, /fsycl-host-compiler=, /fsycl-host-compiler-options:, /fsycl-host-compiler-options=
// RUN:   %clang_cl -### /fsycl /fsycl-host-compiler:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-host-compiler=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-host-compiler:foo /fsycl-host-compiler-options:bar %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-host-compiler=foo /fsycl-host-compiler-options=bar %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-id-queries-fit-in-int, /fno-sycl-id-queries-fit-in-int
// RUN:   %clang_cl -### /fsycl /fsycl-id-queries-fit-in-int %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fno-sycl-id-queries-fit-in-int %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-instrument-device-code, /fno-sycl-instrument-device-code
// RUN:   %clang_cl -### /fsycl /fsycl-instrument-device-code %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fno-sycl-instrument-device-code %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-libspirv-path:, /fsycl-libspirv-path=, /fno-sycl-libspirv
// RUN:   %clang_cl -### /fsycl /fsycl-libspirv-path:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-libspirv-path=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fno-sycl-libspirv %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fno-sycl-link-spirv
// RUN:   %clang_cl -### /fsycl /fno-sycl-link-spirv %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-link-targets:, /fsycl-link-targets=
// RUN:   %clang_cl -### /fsycl /fsycl-link-targets:spir64 %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-link-targets=spir64 %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fsycl-max-parallel-link-jobs:, /fsycl-max-parallel-link-jobs=
// RUN:   %clang_cl -### /fsycl /fsycl-max-parallel-link-jobs:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fsycl /fsycl-max-parallel-link-jobs=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fuse-ld:, /fuse-ld=
// RUN:   %clang_cl -### /fuse-ld:foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /fuse-ld=foo %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fvirtual-function-elimination, /fno-virtual-function-elimination
// RUN:   %clang_cl -### /Qipo /fuse-ld=lld /fvirtual-function-elimination %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /Qipo /fuse-ld=lld /fno-virtual-function-elimination %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /fwhole-program-vtables, /fno-whole-program-vtables
// RUN:   %clang_cl -### /Qipo /fuse-ld=lld /fwhole-program-vtables %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s
// RUN:   %clang_cl -### /Qipo /fuse-ld=lld /fno-whole-program-vtables %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// /W
// RUN:   %clang_cl -### /W5 %s 2>&1 | FileCheck -check-prefix=CHECK-ERROR %s

// CHECK-ERROR-NOT: error:
