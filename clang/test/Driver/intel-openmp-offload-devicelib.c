///
/// Perform driver tests for OpenMP offloading with -fiopenmp and target spir64
/// with -device-math-lib option.
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// REQUIRES: intel_libompdevice
// The driver looks for .o devicelib files, that only exist on Linux:
// REQUIRES: system-linux

/// ###########################################################################

/// Check explicit linking under -device-math-lib:
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fopenmp-device-lib=libm-fp32,libm-fp64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64="-DFOO" -fopenmp-device-lib=libm-fp32,libm-fp64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, {{.*libomp-complex.o.*}}, object, (host-openmp)
// CHK-PHASES: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES: 7: input, {{.*libomp-complex-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES: 9: input, {{.*libomp-cmath.o.*}}, object, (host-openmp)
// CHK-PHASES: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES: 11: input, {{.*libomp-cmath-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES: 13: input, {{.*libomp-fallback-complex.o.*}}, object, (host-openmp)
// CHK-PHASES: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES: 15: input, {{.*libomp-fallback-complex-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES: 17: input, {{.*libomp-fallback-cmath.o.*}}, object, (host-openmp)
// CHK-PHASES: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES: 19: input, {{.*libomp-fallback-cmath-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES: 20: clang-offload-unbundler, {19}, object, (host-openmp)
// CHK-PHASES: 21: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 22: preprocessor, {21}, cpp-output, (device-openmp)
// CHK-PHASES: 23: compiler, {22}, ir, (device-openmp)
// CHK-PHASES: 24: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {23}, ir
// CHK-PHASES: 25: backend, {24}, ir, (device-openmp)
// CHK-PHASES: 26: linker, {25, 6, 8, 10, 12, 14, 16, 18, 20}, ir, (device-openmp)
// CHK-PHASES: 27: sycl-post-link, {26}, ir, (device-openmp)
// CHK-PHASES: 28: llvm-spirv, {27}, spirv, (device-openmp)
// CHK-PHASES: 29: offload, "device-openmp (spir64)" {28}, ir
// CHK-PHASES: 30: clang-offload-wrapper, {29}, ir, (host-openmp)
// CHK-PHASES: 31: backend, {30}, assembler, (host-openmp)
// CHK-PHASES: 32: assembler, {31}, object, (host-openmp)
// CHK-PHASES: 33: linker, {4, 6, 8, 10, 12, 14, 16, 18, 20, 32}, image, (host-openmp)

/// Check default linking:
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-DEF %s

// CHK-PHASES-DEF: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-DEF: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-DEF: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-DEF: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-DEF: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-DEF: 5: input, {{.*libomp-complex.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES-DEF: 7: input, {{.*libomp-complex-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES-DEF: 9: input, {{.*libomp-cmath.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES-DEF: 11: input, {{.*libomp-cmath-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES-DEF: 13: input, {{.*libomp-fallback-complex.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES-DEF: 15: input, {{.*libomp-fallback-complex-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES-DEF: 17: input, {{.*libomp-fallback-cmath.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES-DEF: 19: input, {{.*libomp-fallback-cmath-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 20: clang-offload-unbundler, {19}, object, (host-openmp)
// CHK-PHASES-DEF: 21: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-DEF: 22: preprocessor, {21}, cpp-output, (device-openmp)
// CHK-PHASES-DEF: 23: compiler, {22}, ir, (device-openmp)
// CHK-PHASES-DEF: 24: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {23}, ir
// CHK-PHASES-DEF: 25: backend, {24}, ir, (device-openmp)
// CHK-PHASES-DEF: 26: linker, {25, 6, 8, 10, 12, 14, 16, 18, 20}, ir, (device-openmp)
// CHK-PHASES-DEF: 27: sycl-post-link, {26}, ir, (device-openmp)
// CHK-PHASES-DEF: 28: llvm-spirv, {27}, spirv, (device-openmp)
// CHK-PHASES-DEF: 29: offload, "device-openmp (spir64)" {28}, ir
// CHK-PHASES-DEF: 30: clang-offload-wrapper, {29}, ir, (host-openmp)
// CHK-PHASES-DEF: 31: backend, {30}, assembler, (host-openmp)
// CHK-PHASES-DEF: 32: assembler, {31}, object, (host-openmp)
// CHK-PHASES-DEF: 33: linker, {4, 6, 8, 10, 12, 14, 16, 18, 20, 32}, image, (host-openmp)

/// Check -fopenmp-device-lib=libc phases
// RUN: %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fno-openmp-device-lib=all -fopenmp-device-lib=libc %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-LIBC %s
// CHK-PHASES-LIBC: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-LIBC: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-LIBC: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-LIBC: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-LIBC: 5: input, "{{.*libomp-glibc.o.*}}", object, (host-openmp)
// CHK-PHASES-LIBC: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES-LIBC: 7: input, "{{.*libomp-fallback-cassert.o.*}}", object, (host-openmp)
// CHK-PHASES-LIBC: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES-LIBC: 9: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-LIBC: 10: preprocessor, {9}, cpp-output, (device-openmp)
// CHK-PHASES-LIBC: 11: compiler, {10}, ir, (device-openmp)
// CHK-PHASES-LIBC: 12: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {11}, ir
// CHK-PHASES-LIBC: 13: backend, {12}, ir, (device-openmp)
// CHK-PHASES-LIBC: 14: linker, {13, 6, 8}, ir, (device-openmp)
// CHK-PHASES-LIBC: 15: sycl-post-link, {14}, ir, (device-openmp)
// CHK-PHASES-LIBC: 16: llvm-spirv, {15}, spirv, (device-openmp)
// CHK-PHASES-LIBC: 17: offload, "device-openmp (spir64)" {16}, ir
// CHK-PHASES-LIBC: 18: clang-offload-wrapper, {17}, ir, (host-openmp)
// CHK-PHASES-LIBC: 19: backend, {18}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 20: assembler, {19}, object, (host-openmp)
// CHK-PHASES-LIBC: 21: linker, {4, 6, 8, 20}, image, (host-openmp)

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
