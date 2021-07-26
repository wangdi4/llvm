///
/// Perform driver tests for OpenMP offloading with -fiopenmp and target spir64
/// pulling in the device libs.
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// REQUIRES: intel_libompdevice
// The driver looks for .o devicelib files, that only exist on Linux:
// REQUIRES: system-linux

/// ###########################################################################

/// Check explicit linking of device libs (-fopenmp-device-lib=all default):
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s
// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, "{{.*libomp-spirvdevicertl.o.*}}", object, (host-openmp)
// CHK-PHASES: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES: 7: input, "{{.*libomp-glibc.o.*}}", object, (host-openmp)
// CHK-PHASES: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES: 9: input, "{{.*libomp-complex.o.*}}", object, (host-openmp)
// CHK-PHASES: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES: 11: input, "{{.*libomp-complex-fp64.o.*}}", object, (host-openmp)
// CHK-PHASES: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES: 13: input, "{{.*libomp-cmath.o.*}}", object, (host-openmp)
// CHK-PHASES: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES: 15: input, "{{.*libomp-cmath-fp64.o.*}}", object, (host-openmp)
// CHK-PHASES: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES: 17: input, "{{.*libomp-fallback-cassert.o.*}}", object, (host-openmp)
// CHK-PHASES: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES: 19: input, "{{.*libomp-fallback-cstring.o.*}}", object, (host-openmp)
// CHK-PHASES: 20: clang-offload-unbundler, {19}, object, (host-openmp)
// CHK-PHASES: 21: input, "{{.*libomp-fallback-complex.o.*}}", object, (host-openmp)
// CHK-PHASES: 22: clang-offload-unbundler, {21}, object, (host-openmp)
// CHK-PHASES: 23: input, "{{.*libomp-fallback-complex-fp64.o.*}}", object, (host-openmp)
// CHK-PHASES: 24: clang-offload-unbundler, {23}, object, (host-openmp)
// CHK-PHASES: 25: input, "{{.*libomp-fallback-cmath.o.*}}", object, (host-openmp)
// CHK-PHASES: 26: clang-offload-unbundler, {25}, object, (host-openmp)
// CHK-PHASES: 27: input, "{{.*libomp-fallback-cmath-fp64.o.*}}", object, (host-openmp)
// CHK-PHASES: 28: clang-offload-unbundler, {27}, object, (host-openmp)
// CHK-PHASES: 29: input, "{{.*libomp-itt-user-wrappers.o.*}}", object, (host-openmp)
// CHK-PHASES: 30: clang-offload-unbundler, {29}, object, (host-openmp)
// CHK-PHASES: 31: input, "{{.*libomp-itt-compiler-wrappers.o.*}}", object, (host-openmp)
// CHK-PHASES: 32: clang-offload-unbundler, {31}, object, (host-openmp)
// CHK-PHASES: 33: input, "{{.*libomp-itt-stubs.o.*}}", object, (host-openmp)
// CHK-PHASES: 34: clang-offload-unbundler, {33}, object, (host-openmp)
// CHK-PHASES: 35: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 36: preprocessor, {35}, cpp-output, (device-openmp)
// CHK-PHASES: 37: compiler, {36}, ir, (device-openmp)
// CHK-PHASES: 38: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {37}, ir
// CHK-PHASES: 39: backend, {38}, ir, (device-openmp)
// CHK-PHASES: 40: linker, {39, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34}, ir, (device-openmp)
// CHK-PHASES: 41: sycl-post-link, {40}, ir, (device-openmp)
// CHK-PHASES: 42: llvm-spirv, {41}, spirv, (device-openmp)
// CHK-PHASES: 43: offload, "device-openmp (spir64)" {42}, ir
// CHK-PHASES: 44: clang-offload-wrapper, {43}, ir, (host-openmp)
// CHK-PHASES: 45: backend, {44}, assembler, (host-openmp)
// CHK-PHASES: 46: assembler, {45}, object, (host-openmp)
// CHK-PHASES: 47: linker, {4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 46}, image, (host-openmp)

/// Check -fopenmp-device-lib=libc phases
// RUN: %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fno-openmp-device-lib=all -fopenmp-device-lib=libc %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-LIBC %s
// CHK-PHASES-LIBC: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-LIBC: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-LIBC: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-LIBC: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-LIBC: 5: input, {{.*libomp-spirvdevicertl.o.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES-LIBC: 7: input, "{{.*libomp-glibc.o.*}}", object, (host-openmp)
// CHK-PHASES-LIBC: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES-LIBC: 9: input, "{{.*libomp-fallback-cassert.o.*}}", object, (host-openmp)
// CHK-PHASES-LIBC: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES-LIBC: 11: input, "{{.*libomp-fallback-cstring.o.*}}", object, (host-openmp)
// CHK-PHASES-LIBC: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES-LIBC: 13: input, {{.*libomp-itt-user-wrappers.o.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES-LIBC: 15: input, {{.*libomp-itt-compiler-wrappers.o.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES-LIBC: 17: input, {{.*libomp-itt-stubs.o.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES-LIBC: 19: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-LIBC: 20: preprocessor, {19}, cpp-output, (device-openmp)
// CHK-PHASES-LIBC: 21: compiler, {20}, ir, (device-openmp)
// CHK-PHASES-LIBC: 22: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {21}, ir
// CHK-PHASES-LIBC: 23: backend, {22}, ir, (device-openmp)
// CHK-PHASES-LIBC: 24: linker, {23, 6, 8, 10, 12, 14, 16, 18}, ir, (device-openmp)
// CHK-PHASES-LIBC: 25: sycl-post-link, {24}, ir, (device-openmp)
// CHK-PHASES-LIBC: 26: llvm-spirv, {25}, spirv, (device-openmp)
// CHK-PHASES-LIBC: 27: offload, "device-openmp (spir64)" {26}, ir
// CHK-PHASES-LIBC: 28: clang-offload-wrapper, {27}, ir, (host-openmp)
// CHK-PHASES-LIBC: 29: backend, {28}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 30: assembler, {29}, object, (host-openmp)
// CHK-PHASES-LIBC: 31: linker, {4, 6, 8, 10, 12, 14, 16, 18, 30}, image, (host-openmp)

/// Check that -fopenmp-device-lib does not affect separate compilation:
// RUN:   %clang -### -ccc-print-phases -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fopenmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUACTIONS %s

// CHK-BUACTIONS: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-BUACTIONS: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-BUACTIONS: 2: compiler, {1}, ir, (host-openmp)
// CHK-BUACTIONS: 3: input, "[[INPUT]]", c, (device-openmp)
// CHK-BUACTIONS: 4: preprocessor, {3}, cpp-output, (device-openmp)
// CHK-BUACTIONS: 5: compiler, {4}, ir, (device-openmp)
// CHK-BUACTIONS: 6: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {5}, ir
// CHK-BUACTIONS: 7: backend, {6}, ir, (device-openmp)
// CHK-BUACTIONS: 8: offload, "device-openmp (spir64)" {7}, ir
// CHK-BUACTIONS: 9: backend, {2}, assembler, (host-openmp)
// CHK-BUACTIONS: 10: assembler, {9}, object, (host-openmp)
// CHK-BUACTIONS: 11: clang-offload-bundler, {8, 10}, object, (host-openmp)

/// Check for deprecation of option
// RUN: %clang -fiopenmp -fopenmp-targets=spir64 -device-math-lib=fp32 -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=WARNING_CHECK %s
// WARNING_CHECK: warning: argument '-device-math-lib=fp32' is deprecated, use '-f[no-]openmp-device-lib' instead [-Wdeprecated]

/// Check that the proper device libs are linked with --only-needed
// RUN: %clang -fiopenmp -fopenmp-targets=spir64 -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix=ONLY_NEEDED %s
// ONLY_NEEDED: llvm-link{{.*}} {{.*}}libomp-spirvdevicertl{{.*}}
// ONLY_NEEDED: llvm-link{{.*}} "--only-needed"
// ONLY_NEEDED-SAME: {{.*}}libomp-glibc{{.*}} {{.*}}libomp-complex{{.*}} {{.*}}libomp-complex-fp64{{.*}} {{.*}}libomp-cmath{{.*}} {{.*}}libomp-cmath-fp64{{.*}} {{.*}}libomp-fallback-cassert{{.*}} {{.*}}libomp-fallback-cstring{{.*}} {{.*}}libomp-fallback-complex{{.*}} {{.*}}libomp-fallback-complex-fp64{{.*}} {{.*}}libomp-fallback-cmath{{.*}} {{.*}}libomp-fallback-cmath-fp64{{.*}} {{.*}}libomp-itt-user-wrappers{{.*}} {{.*}}libomp-itt-compiler-wrappers{{.*}} {{.*}}libomp-itt-stubs{{.*}}
