///
/// Perform driver tests for OpenMP offloading with -fiopenmp and target spir64
/// with -device-math-lib option on Windows.
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// REQUIRES: intel_libompdevice
// The driver looks for .obj devicelib files, that only exist on Windows:
// REQUIRES: system-windows

/// ###########################################################################

/// Check explicit linking under -device-math-lib:
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fopenmp-device-lib=libm-fp32,libm-fp64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, {{.*libomp-spirvdevicertl.obj.*}}, object, (host-openmp)
// CHK-PHASES: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES: 7: input, {{.*libomp-complex.obj.*}}, object, (host-openmp)
// CHK-PHASES: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES: 9: input, {{.*libomp-complex-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES: 11: input, {{.*libomp-cmath.obj.*}}, object, (host-openmp)
// CHK-PHASES: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES: 13: input, {{.*libomp-cmath-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES: 15: input, {{.*libomp-fallback-complex.obj.*}}, object, (host-openmp)
// CHK-PHASES: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES: 17: input, {{.*libomp-fallback-complex-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES: 19: input, {{.*libomp-fallback-cmath.obj.*}}, object, (host-openmp)
// CHK-PHASES: 20: clang-offload-unbundler, {19}, object, (host-openmp)
// CHK-PHASES: 21: input, {{.*libomp-fallback-cmath-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 22: clang-offload-unbundler, {21}, object, (host-openmp)
// CHK-PHASES: 23: input, {{.*libomp-itt-user-wrappers.obj.*}}, object, (host-openmp)
// CHK-PHASES: 24: clang-offload-unbundler, {23}, object, (host-openmp)
// CHK-PHASES: 25: input, {{.*libomp-itt-compiler-wrappers.obj.*}}, object, (host-openmp)
// CHK-PHASES: 26: clang-offload-unbundler, {25}, object, (host-openmp)
// CHK-PHASES: 27: input, {{.*libomp-itt-stubs.obj.*}}, object, (host-openmp)
// CHK-PHASES: 28: clang-offload-unbundler, {27}, object, (host-openmp)
// CHK-PHASES: 29: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 30: preprocessor, {29}, cpp-output, (device-openmp)
// CHK-PHASES: 31: compiler, {30}, ir, (device-openmp)
// CHK-PHASES: 32: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {31}, ir
// CHK-PHASES: 33: backend, {32}, ir, (device-openmp)
// CHK-PHASES: 34: linker, {33, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28}, ir, (device-openmp)
// CHK-PHASES: 35: sycl-post-link, {34}, ir, (device-openmp)
// CHK-PHASES: 36: llvm-spirv, {35}, spirv, (device-openmp)
// CHK-PHASES: 37: offload, "device-openmp (spir64)" {36}, ir
// CHK-PHASES: 38: clang-offload-wrapper, {37}, ir, (host-openmp)
// CHK-PHASES: 39: backend, {38}, assembler, (host-openmp)
// CHK-PHASES: 40: assembler, {39}, object, (host-openmp)
// CHK-PHASES: 41: linker, {4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 40}, image, (host-openmp)

/// Check default linking:
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-DEF %s

// CHK-PHASES-DEF: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-DEF: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-DEF: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-DEF: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-DEF: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-DEF: 5: input, {{.*libomp-spirvdevicertl.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES-DEF: 7: input, {{.*libomp-complex.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES-DEF: 9: input, {{.*libomp-complex-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES-DEF: 11: input, {{.*libomp-cmath.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES-DEF: 13: input, {{.*libomp-cmath-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES-DEF: 15: input, {{.*libomp-fallback-complex.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES-DEF: 17: input, {{.*libomp-fallback-complex-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES-DEF: 19: input, {{.*libomp-fallback-cmath.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 20: clang-offload-unbundler, {19}, object, (host-openmp)
// CHK-PHASES-DEF: 21: input, {{.*libomp-fallback-cmath-fp64.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 22: clang-offload-unbundler, {21}, object, (host-openmp)
// CHK-PHASES-DEF: 23: input, {{.*libomp-itt-user-wrappers.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 24: clang-offload-unbundler, {23}, object, (host-openmp)
// CHK-PHASES-DEF: 25: input, {{.*libomp-itt-compiler-wrappers.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 26: clang-offload-unbundler, {25}, object, (host-openmp)
// CHK-PHASES-DEF: 27: input, {{.*libomp-itt-stubs.o.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 28: clang-offload-unbundler, {27}, object, (host-openmp)
// CHK-PHASES-DEF: 29: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-DEF: 30: preprocessor, {29}, cpp-output, (device-openmp)
// CHK-PHASES-DEF: 31: compiler, {30}, ir, (device-openmp)
// CHK-PHASES-DEF: 32: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {31}, ir
// CHK-PHASES-DEF: 33: backend, {32}, ir, (device-openmp)
// CHK-PHASES-DEF: 34: linker, {33, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28}, ir, (device-openmp)
// CHK-PHASES-DEF: 35: sycl-post-link, {34}, ir, (device-openmp)
// CHK-PHASES-DEF: 36: llvm-spirv, {35}, spirv, (device-openmp)
// CHK-PHASES-DEF: 37: offload, "device-openmp (spir64)" {36}, ir
// CHK-PHASES-DEF: 38: clang-offload-wrapper, {37}, ir, (host-openmp)
// CHK-PHASES-DEF: 39: backend, {38}, assembler, (host-openmp)
// CHK-PHASES-DEF: 40: assembler, {39}, object, (host-openmp)
// CHK-PHASES-DEF: 41: linker, {4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 40}, image, (host-openmp)

/// Check explicit linking for libc
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fno-openmp-device-lib=all -fopenmp-device-lib=libc %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-LIBC %s

// CHK-PHASES-LIBC: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-LIBC: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-LIBC: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-LIBC: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-LIBC: 5: input, {{.*libomp-spirvdevicertl.obj.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES-LIBC: 7: input, "{{.*libomp-msvc.obj.*}}", object, (host-openmp)
// CHK-PHASES-LIBC: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES-LIBC: 9: input, "{{.*libomp-fallback-cassert.obj.*}}", object, (host-openmp)
// CHK-PHASES-LIBC: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES-LIBC: 11: input, {{.*libomp-itt-user-wrappers.obj.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES-LIBC: 13: input, {{.*libomp-itt-compiler-wrappers.obj.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES-LIBC: 15: input, {{.*libomp-itt-stubs.obj.*}}, object, (host-openmp)
// CHK-PHASES-LIBC: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES-LIBC: 17: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-LIBC: 18: preprocessor, {17}, cpp-output, (device-openmp)
// CHK-PHASES-LIBC: 19: compiler, {18}, ir, (device-openmp)
// CHK-PHASES-LIBC: 20: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {19}, ir
// CHK-PHASES-LIBC: 21: backend, {20}, ir, (device-openmp)
// CHK-PHASES-LIBC: 22: linker, {21, 6, 8, 10, 12, 14, 16}, ir, (device-openmp)
// CHK-PHASES-LIBC: 23: sycl-post-link, {22}, ir, (device-openmp)
// CHK-PHASES-LIBC: 24: llvm-spirv, {23}, spirv, (device-openmp)
// CHK-PHASES-LIBC: 25: offload, "device-openmp (spir64)" {24}, ir
// CHK-PHASES-LIBC: 26: clang-offload-wrapper, {25}, ir, (host-openmp)
// CHK-PHASES-LIBC: 27: backend, {26}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 28: assembler, {27}, object, (host-openmp)
// CHK-PHASES-LIBC: 29: linker, {4, 6, 8, 10, 12, 14, 16, 28}, image, (host-openmp)

/// Check that -fopenmp-device-lib does not affect separate compilation:
// RUN:   %clang -### -ccc-print-phases -fiopenmp -c -o %t.obj -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fopenmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUACTIONS %s
// CHK-BUACTIONS: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-BUACTIONS: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-BUACTIONS: 2: compiler, {1}, ir, (host-openmp)
// CHK-BUACTIONS: 3: input, "[[INPUT]]", c, (device-openmp)
// CHK-BUACTIONS: 4: preprocessor, {3}, cpp-output, (device-openmp)
// CHK-BUACTIONS: 5: compiler, {4}, ir, (device-openmp)
// CHK-BUACTIONS: 6: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {5}, ir
// CHK-BUACTIONS: 7: backend, {6}, ir, (device-openmp)
// CHK-BUACTIONS: 8: offload, "device-openmp (spir64)" {7}, ir
// CHK-BUACTIONS: 9: backend, {2}, assembler, (host-openmp)
// CHK-BUACTIONS: 10: assembler, {9}, object, (host-openmp)
// CHK-BUACTIONS: 11: clang-offload-bundler, {8, 10}, object, (host-openmp)

/// Check for deprecation of option
// RUN: %clang -target x86_64-pc-windows-msvc -fiopenmp -fopenmp-targets=spir64 -device-math-lib=fp32 -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=WARNING_CHECK %s
// WARNING_CHECK: warning: argument '-device-math-lib=fp32' is deprecated, use '-f[no-]openmp-device-lib' instead [-Wdeprecated]
