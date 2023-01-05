///
/// Perform driver tests for OpenMP offloading with -fiopenmp and target spir64
/// with -device-math-lib option on Windows.
///

// REQUIRES: x86-registered-target
// REQUIRES: intel_libompdevice
// The driver looks for .obj devicelib files, that only exist on Windows:
// REQUIRES: system-windows

/// ###########################################################################

/// Check explicit linking for device libs (-fopenmp-device-lib=all default):
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s
// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 6: preprocessor, {5}, cpp-output, (device-openmp)
// CHK-PHASES: 7: compiler, {6}, ir, (device-openmp)
// CHK-PHASES: 8: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {7}, ir
// CHK-PHASES: 9: backend, {8}, ir, (device-openmp)
// CHK-PHASES: 10: input, "{{.*libomp-spirvdevicertl.obj}}", object
// CHK-PHASES: 11: clang-offload-unbundler, {10}, object
// CHK-PHASES: 12: input, "{{.*libomp-msvc.obj}}", object
// CHK-PHASES: 13: clang-offload-unbundler, {12}, object
// CHK-PHASES: 14: input, "{{.*libomp-complex.obj}}", object
// CHK-PHASES: 15: clang-offload-unbundler, {14}, object
// CHK-PHASES: 16: input, "{{.*libomp-complex-fp64.obj}}", object
// CHK-PHASES: 17: clang-offload-unbundler, {16}, object
// CHK-PHASES: 18: input, "{{.*libomp-cmath.obj}}", object
// CHK-PHASES: 19: clang-offload-unbundler, {18}, object
// CHK-PHASES: 20: input, "{{.*libomp-cmath-fp64.obj}}", object
// CHK-PHASES: 21: clang-offload-unbundler, {20}, object
// CHK-PHASES: 22: input, "{{.*libomp-fallback-cassert.obj}}", object
// CHK-PHASES: 23: clang-offload-unbundler, {22}, object
// CHK-PHASES: 24: input, "{{.*libomp-fallback-cstring.obj}}", object
// CHK-PHASES: 25: clang-offload-unbundler, {24}, object
// CHK-PHASES: 26: input, "{{.*libomp-fallback-complex.obj}}", object
// CHK-PHASES: 27: clang-offload-unbundler, {26}, object
// CHK-PHASES: 28: input, "{{.*libomp-fallback-complex-fp64.obj}}", object
// CHK-PHASES: 29: clang-offload-unbundler, {28}, object
// CHK-PHASES: 30: input, "{{.*libomp-fallback-cmath.obj}}", object
// CHK-PHASES: 31: clang-offload-unbundler, {30}, object
// CHK-PHASES: 32: input, "{{.*libomp-fallback-cmath-fp64.obj}}", object
// CHK-PHASES: 33: clang-offload-unbundler, {32}, object
// CHK-PHASES: 34: input, "{{.*libomp-itt-user-wrappers.obj}}", object
// CHK-PHASES: 35: clang-offload-unbundler, {34}, object
// CHK-PHASES: 36: input, "{{.*libomp-itt-compiler-wrappers.obj}}", object
// CHK-PHASES: 37: clang-offload-unbundler, {36}, object
// CHK-PHASES: 38: input, "{{.*libomp-itt-stubs.obj}}", object
// CHK-PHASES: 39: clang-offload-unbundler, {38}, object
// CHK-PHASES: 40: linker, {9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39}, ir, (device-openmp)
// CHK-PHASES: 41: sycl-post-link, {40}, ir, (device-openmp)
// CHK-PHASES: 42: llvm-spirv, {41}, spirv, (device-openmp)
// CHK-PHASES: 43: offload, "device-openmp (spir64)" {42}, ir
// CHK-PHASES: 44: clang-offload-wrapper, {43}, ir, (host-openmp)
// CHK-PHASES: 45: backend, {44}, assembler, (host-openmp)
// CHK-PHASES: 46: assembler, {45}, object, (host-openmp)
// CHK-PHASES: 47: linker, {4, 46}, image, (host-openmp)

/// Check explicit linking for libc
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fno-openmp-device-lib=all -fopenmp-device-lib=libc %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-LIBC %s

// CHK-PHASES-LIBC: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-LIBC: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-LIBC: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-LIBC: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-LIBC: 5: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-LIBC: 6: preprocessor, {5}, cpp-output, (device-openmp)
// CHK-PHASES-LIBC: 7: compiler, {6}, ir, (device-openmp)
// CHK-PHASES-LIBC: 8: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {7}, ir
// CHK-PHASES-LIBC: 9: backend, {8}, ir, (device-openmp)
// CHK-PHASES-LIBC: 10: input, "{{.*libomp-spirvdevicertl.obj.*}}", object
// CHK-PHASES-LIBC: 11: clang-offload-unbundler, {10}, object
// CHK-PHASES-LIBC: 12: input, "{{.*libomp-msvc.obj.*}}", object
// CHK-PHASES-LIBC: 13: clang-offload-unbundler, {12}, object
// CHK-PHASES-LIBC: 14: input, "{{.*libomp-fallback-cassert.obj.*}}", object
// CHK-PHASES-LIBC: 15: clang-offload-unbundler, {14}, object
// CHK-PHASES-LIBC: 16: input, "{{.*libomp-fallback-cstring.obj.*}}", object
// CHK-PHASES-LIBC: 17: clang-offload-unbundler, {16}, object
// CHK-PHASES-LIBC: 18: input, "{{.*libomp-itt-user-wrappers.obj.*}}", object
// CHK-PHASES-LIBC: 19: clang-offload-unbundler, {18}, object
// CHK-PHASES-LIBC: 20: input, "{{.*libomp-itt-compiler-wrappers.obj.*}}", object
// CHK-PHASES-LIBC: 21: clang-offload-unbundler, {20}, object
// CHK-PHASES-LIBC: 22: input, "{{.*libomp-itt-stubs.obj.*}}", object
// CHK-PHASES-LIBC: 23: clang-offload-unbundler, {22}, object
// CHK-PHASES-LIBC: 24: linker, {9, 11, 13, 15, 17, 19, 21, 23}, ir, (device-openmp)
// CHK-PHASES-LIBC: 25: sycl-post-link, {24}, ir, (device-openmp)
// CHK-PHASES-LIBC: 26: llvm-spirv, {25}, spirv, (device-openmp)
// CHK-PHASES-LIBC: 27: offload, "device-openmp (spir64)" {26}, ir
// CHK-PHASES-LIBC: 28: clang-offload-wrapper, {27}, ir, (host-openmp)
// CHK-PHASES-LIBC: 29: backend, {28}, assembler, (host-openmp)
// CHK-PHASES-LIBC: 30: assembler, {29}, object, (host-openmp)
// CHK-PHASES-LIBC: 31: linker, {4, 30}, image, (host-openmp)

/// Check explicit linking of device libs w/ target-simd enabled:
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fopenmp-target-simd %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-SIMD %s
// CHK-PHASES-SIMD: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-SIMD: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-SIMD: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-SIMD: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-SIMD: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-SIMD: 5: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-SIMD: 6: preprocessor, {5}, cpp-output, (device-openmp)
// CHK-PHASES-SIMD: 7: compiler, {6}, ir, (device-openmp)
// CHK-PHASES-SIMD: 8: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {7}, ir
// CHK-PHASES-SIMD: 9: backend, {8}, ir, (device-openmp)
// CHK-PHASES-SIMD: 10: input, "{{.*libomp-spirvdevicertl.obj}}", object
// CHK-PHASES-SIMD: 11: clang-offload-unbundler, {10}, object
// CHK-PHASES-SIMD: 12: input, "{{.*libomp-msvc.obj}}", object
// CHK-PHASES-SIMD: 13: clang-offload-unbundler, {12}, object
// CHK-PHASES-SIMD: 14: input, "{{.*libomp-complex.obj}}", object
// CHK-PHASES-SIMD: 15: clang-offload-unbundler, {14}, object
// CHK-PHASES-SIMD: 16: input, "{{.*libomp-complex-fp64.obj}}", object
// CHK-PHASES-SIMD: 17: clang-offload-unbundler, {16}, object
// CHK-PHASES-SIMD: 18: input, "{{.*libomp-cmath.obj}}", object
// CHK-PHASES-SIMD: 19: clang-offload-unbundler, {18}, object
// CHK-PHASES-SIMD: 20: input, "{{.*libomp-cmath-fp64.obj}}", object
// CHK-PHASES-SIMD: 21: clang-offload-unbundler, {20}, object
// CHK-PHASES-SIMD: 22: input, "{{.*libomp-fallback-cassert.obj}}", object
// CHK-PHASES-SIMD: 23: clang-offload-unbundler, {22}, object
// CHK-PHASES-SIMD: 24: input, "{{.*libomp-fallback-cstring.obj}}", object
// CHK-PHASES-SIMD: 25: clang-offload-unbundler, {24}, object
// CHK-PHASES-SIMD: 26: input, "{{.*libomp-fallback-complex.obj}}", object
// CHK-PHASES-SIMD: 27: clang-offload-unbundler, {26}, object
// CHK-PHASES-SIMD: 28: input, "{{.*libomp-fallback-complex-fp64.obj}}", object
// CHK-PHASES-SIMD: 29: clang-offload-unbundler, {28}, object
// CHK-PHASES-SIMD: 30: input, "{{.*libomp-fallback-cmath.obj}}", object
// CHK-PHASES-SIMD: 31: clang-offload-unbundler, {30}, object
// CHK-PHASES-SIMD: 32: input, "{{.*libomp-fallback-cmath-fp64.obj}}", object
// CHK-PHASES-SIMD: 33: clang-offload-unbundler, {32}, object
// CHK-PHASES-SIMD: 34: input, "{{.*libomp-itt-user-wrappers.obj}}", object
// CHK-PHASES-SIMD: 35: clang-offload-unbundler, {34}, object
// CHK-PHASES-SIMD: 36: input, "{{.*libomp-itt-compiler-wrappers.obj}}", object
// CHK-PHASES-SIMD: 37: clang-offload-unbundler, {36}, object
// CHK-PHASES-SIMD: 38: input, "{{.*libomp-itt-stubs.obj}}", object
// CHK-PHASES-SIMD: 39: clang-offload-unbundler, {38}, object
// CHK-PHASES-SIMD: 40: input, "{{.*libomp-device-svml.obj}}", object
// CHK-PHASES-SIMD: 41: clang-offload-unbundler, {40}, object
// CHK-PHASES-SIMD: 42: linker, {9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41}, ir, (device-openmp)
// CHK-PHASES-SIMD: 43: sycl-post-link, {42}, ir, (device-openmp)
// CHK-PHASES-SIMD: 44: llvm-spirv, {43}, spirv, (device-openmp)
// CHK-PHASES-SIMD: 45: offload, "device-openmp (spir64)" {44}, ir
// CHK-PHASES-SIMD: 46: clang-offload-wrapper, {45}, ir, (host-openmp)
// CHK-PHASES-SIMD: 47: backend, {46}, assembler, (host-openmp)
// CHK-PHASES-SIMD: 48: assembler, {47}, object, (host-openmp)
// CHK-PHASES-SIMD: 49: linker, {4, 48}, image, (host-openmp)

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
// RUN: %clang -target x86_64-pc-windows-msvc -fiopenmp -fopenmp-targets=spir64 -device-math-lib=fp32 -### %s 2>&1 \
// RUN:   | FileCheck -check-prefix=WARNING_CHECK %s
// WARNING_CHECK: warning: argument '-device-math-lib=fp32' is deprecated, use '-f[no-]openmp-device-lib' instead [-Wdeprecated]
