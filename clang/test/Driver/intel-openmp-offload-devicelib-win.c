// UNSUPPORTED: intel_opencl && i686-pc-windows

///
/// Perform driver tests for OpenMP offloading with -fiopenmp and target spir64
/// with -device-math-lib option on Windows.
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// The driver looks for .obj devicelib files, that only exist on Windows:
// REQUIRES: system-windows

/// ###########################################################################

/// Check explicit linking under -device-math-lib:
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -device-math-lib=fp32,fp64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, {{.*libomp-complex.obj.*}}, object, (host-openmp)
// CHK-PHASES: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES: 7: input, {{.*libomp-complex-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES: 9: input, {{.*libomp-cmath.obj.*}}, object, (host-openmp)
// CHK-PHASES: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES: 11: input, {{.*libomp-cmath-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES: 13: input, {{.*libomp-fallback-complex.obj.*}}, object, (host-openmp)
// CHK-PHASES: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES: 15: input, {{.*libomp-fallback-complex-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES: 17: input, {{.*libomp-fallback-cmath.obj.*}}, object, (host-openmp)
// CHK-PHASES: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES: 19: input, {{.*libomp-fallback-cmath-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES: 20: clang-offload-unbundler, {19}, object, (host-openmp)
// CHK-PHASES: 21: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 22: preprocessor, {21}, cpp-output, (device-openmp)
// CHK-PHASES: 23: compiler, {22}, ir, (device-openmp)
// CHK-PHASES: 24: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {23}, ir
// CHK-PHASES: 25: backend, {24}, ir, (device-openmp)
// CHK-PHASES: 26: linker, {25, 6, 8, 10, 12, 14, 16, 18, 20}, image, (device-openmp)
// CHK-PHASES: 27: sycl-post-link, {26}, ir, (device-openmp)
// CHK-PHASES: 28: llvm-spirv, {27}, image, (device-openmp)
// CHK-PHASES: 29: offload, "device-openmp (spir64)" {28}, image
// CHK-PHASES: 30: clang-offload-wrapper, {29}, ir, (host-openmp)
// CHK-PHASES: 31: backend, {30}, assembler, (host-openmp)
// CHK-PHASES: 32: assembler, {31}, object, (host-openmp)
// CHK-PHASES: 33: linker, {4, 6, 8, 10, 12, 14, 16, 18, 20, 32}, image, (host-openmp)

/// Check default linking:
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES-DEF %s

// CHK-PHASES-DEF: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES-DEF: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES-DEF: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES-DEF: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES-DEF: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES-DEF: 5: input, {{.*libomp-complex.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES-DEF: 7: input, {{.*libomp-complex-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES-DEF: 9: input, {{.*libomp-cmath.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES-DEF: 11: input, {{.*libomp-cmath-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES-DEF: 13: input, {{.*libomp-fallback-complex.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 14: clang-offload-unbundler, {13}, object, (host-openmp)
// CHK-PHASES-DEF: 15: input, {{.*libomp-fallback-complex-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 16: clang-offload-unbundler, {15}, object, (host-openmp)
// CHK-PHASES-DEF: 17: input, {{.*libomp-fallback-cmath.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 18: clang-offload-unbundler, {17}, object, (host-openmp)
// CHK-PHASES-DEF: 19: input, {{.*libomp-fallback-cmath-fp64.obj.*}}, object, (host-openmp)
// CHK-PHASES-DEF: 20: clang-offload-unbundler, {19}, object, (host-openmp)
// CHK-PHASES-DEF: 21: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES-DEF: 22: preprocessor, {21}, cpp-output, (device-openmp)
// CHK-PHASES-DEF: 23: compiler, {22}, ir, (device-openmp)
// CHK-PHASES-DEF: 24: offload, "host-openmp (x86_64-pc-windows-msvc)" {2}, "device-openmp (spir64)" {23}, ir
// CHK-PHASES-DEF: 25: backend, {24}, ir, (device-openmp)
// CHK-PHASES-DEF: 26: linker, {25, 6, 8, 10, 12, 14, 16, 18, 20}, image, (device-openmp)
// CHK-PHASES-DEF: 27: sycl-post-link, {26}, ir, (device-openmp)
// CHK-PHASES-DEF: 28: llvm-spirv, {27}, image, (device-openmp)
// CHK-PHASES-DEF: 29: offload, "device-openmp (spir64)" {28}, image
// CHK-PHASES-DEF: 30: clang-offload-wrapper, {29}, ir, (host-openmp)
// CHK-PHASES-DEF: 31: backend, {30}, assembler, (host-openmp)
// CHK-PHASES-DEF: 32: assembler, {31}, object, (host-openmp)
// CHK-PHASES-DEF: 33: linker, {4, 6, 8, 10, 12, 14, 16, 18, 20, 32}, image, (host-openmp)

/// Check that -device-math-lib does not affect separate compilation:
// RUN:   %clang -### -ccc-print-phases -fiopenmp -c -o %t.obj -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -device-math-lib=fp32,fp64 %s 2>&1 \
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
