///
/// Perform driver tests for OpenMP offloading with -fiopenmp and target spir64
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target

/// ###########################################################################

/// Check the phases graph when using a single target, different from the host.
/// We should have an offload action joining the host compile and device
/// preprocessor and another one joining the device linking outputs to the host
/// action.
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, {{.*libomp-spirvdevicertl.o.*}}, object, (host-openmp)
// CHK-PHASES: 6: clang-offload-unbundler, {5}, object, (host-openmp)
// CHK-PHASES: 7: input, {{.*libomp-itt-user-wrappers.o.*}}, object, (host-openmp)
// CHK-PHASES: 8: clang-offload-unbundler, {7}, object, (host-openmp)
// CHK-PHASES: 9: input, {{.*libomp-itt-compiler-wrappers.o.*}}, object, (host-openmp)
// CHK-PHASES: 10: clang-offload-unbundler, {9}, object, (host-openmp)
// CHK-PHASES: 11: input, {{.*libomp-itt-stubs.o.*}}, object, (host-openmp)
// CHK-PHASES: 12: clang-offload-unbundler, {11}, object, (host-openmp)
// CHK-PHASES: 13: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 14: preprocessor, {13}, cpp-output, (device-openmp)
// CHK-PHASES: 15: compiler, {14}, ir, (device-openmp)
// CHK-PHASES: 16: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {15}, ir
// CHK-PHASES: 17: backend, {16}, ir, (device-openmp)
// CHK-PHASES: 18: linker, {17, 6, 8, 10, 12}, ir, (device-openmp)
// CHK-PHASES: 19: sycl-post-link, {18}, ir, (device-openmp)
// CHK-PHASES: 20: llvm-spirv, {19}, spirv, (device-openmp)
// CHK-PHASES: 21: offload, "device-openmp (spir64)" {20}, ir
// CHK-PHASES: 22: clang-offload-wrapper, {21}, ir, (host-openmp)
// CHK-PHASES: 23: backend, {22}, assembler, (host-openmp)
// CHK-PHASES: 24: assembler, {23}, object, (host-openmp)
// CHK-PHASES: 25: linker, {4, 6, 8, 10, 12, 24}, image, (host-openmp)

/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp" {{.*}} "-o" "[[BCFILE:.+\.bc]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[BCFILE]]"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-inputs={{.*}}libomp-spirvdevicertl.o" "-outputs=[[RTLHOST:.+\.o]],[[RTLTGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-inputs={{.*}}libomp-itt-user-wrappers.o" "-outputs=[[ITT1HOST:.+\.o]],[[ITT1TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-inputs={{.*}}libomp-itt-compiler-wrappers.o" "-outputs=[[ITT2HOST:.+\.o]],[[ITT2TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-inputs={{.*}}libomp-itt-stubs.o" "-outputs=[[ITT3HOST:.+\.o]],[[ITT3TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-disable-lifetime-markers" "-disable-intel-proprietary-opts" "-Wspir-compat" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp"{{.*}} "-fsycl-instrument-device-code" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[BCFILE]]"{{.*}} "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" "-o" "[[OFFBCFILE:.+\.bc]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[OFFBCFILE]]" "[[RTLTGT]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-COMMANDS: llvm-link{{.*}} "--only-needed" "[[UNBUNDLED]]" "[[ITT1TGT]]" "[[ITT2TGT]]" "[[ITT3TGT]]" "-o" "[[LINKEDBCFILE:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-emit-param-info" "-ir-output-only" "-O2" "-spec-const=rt" "-o" "[[POSTLINKFILE:.+\.bc]]" "[[LINKEDBCFILE]]"
// CHK-COMMANDS: llvm-spirv{{.*}}" {{.*}}"-o" {{.*}} "-spirv-allow-unknown-intrinsics"{{.*}} "[[POSTLINKFILE]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[WRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[TARGOBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-COMMANDS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[RTLHOST]]" "[[ITT1HOST]]" "[[ITT2HOST]]" "[[ITT3HOST]]" "[[TARGOBJ]]" {{.*}} "-lomptarget"

/// Check to be sure exception handling is not enabled for device
// RUN: %clangxx -### -fiopenmp -c -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-EXCEPT %s
// CHK-EXCEPT: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-fexceptions"
// CHK-EXCEPT-NOT: clang{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu"{{.*}} "-fexceptions"

// RUN: %clang_cl -### -Qiopenmp -EHsc -c --target=x86_64-pc-windows-msvc -Qopenmp-targets:spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-EXCEPT-WIN %s
// CHK-EXCEPT-WIN: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-llvm-bc" {{.*}} "-fexceptions" {{.*}} "-fopenmp"
// CHK-EXCEPT-WIN-NOT: clang{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-pc-windows-msvc{{.*}} "-fexceptions"

/// Check additional options passed through
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64="-DFOO -DBAR -mllvm -dummy-opt -Xclang -cc1dummy -O3" -fno-openmp-device-lib=all  %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TARGOPTS %s
// RUN:   %clang -### --intel -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64="-DFOO -DBAR -mllvm -dummy-opt -Xclang -cc1dummy -O3" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TARGOPTS %s
// CHK-TARGOPTS: clang{{.*}} "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-disable-lifetime-markers" "-disable-intel-proprietary-opts" "-Wspir-compat" "-emit-llvm-bc" {{.*}} "-D" "FOO" "-D" "BAR" {{.*}} "-O3" {{.*}} "-cc1dummy" "-mllvm" "-dummy-opt" {{.*}} "-fopenmp-targets=spir64"

/// -O2 is the default when in Intel mode
// RUN:   %clang -### --intel -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64="-DFOO" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TARGOPTS-DEFOPT %s
// CHK-TARGOPTS-DEFOPT: clang{{.*}} "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-disable-lifetime-markers" "-disable-intel-proprietary-opts" "-Wspir-compat" "-emit-llvm-bc" {{.*}} "-D" "FOO" {{.*}} "-O2" {{.*}} "-fopenmp-targets=spir64"

/// Check vectorizer not enabled
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64="-O3" -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TARGOPTS-NOVEC %s
// CHK-TARGOPTS-NOVEC-NOT: clang{{.*}} "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" {{.*}} "-O3" {{.*}} "-vectorize-loops" "-vectorize-slp" {{.*}} "-fopenmp-targets=spir64"

/// ###########################################################################

/// Check separate compilation with offloading - bundling actions
// RUN:   %clang -### -ccc-print-phases -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
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

/// ###########################################################################

/// Check separate compilation with offloading - unbundling actions
// RUN:   touch %t.o
// RUN:   %clang -### -ccc-print-phases -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %t.o -no-canonical-prefixes -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -DINPUT=%t.o -check-prefix=CHK-UBACTIONS %s
// CHK-UBACTIONS: 0: input, "[[INPUT]]", object, (host-openmp)
// CHK-UBACTIONS: 1: clang-offload-unbundler, {0}, object, (host-openmp)
// CHK-UBACTIONS: 2: input, "{{.*libomp-spirvdevicertl.o.*}}", object, (host-openmp)
// CHK-UBACTIONS: 3: clang-offload-unbundler, {2}, object, (host-openmp)
// CHK-UBACTIONS: 4: input, "{{.*libomp-itt-user-wrappers.o.*}}", object, (host-openmp)
// CHK-UBACTIONS: 5: clang-offload-unbundler, {4}, object, (host-openmp)
// CHK-UBACTIONS: 6: input, "{{.*libomp-itt-compiler-wrappers.o.*}}", object, (host-openmp)
// CHK-UBACTIONS: 7: clang-offload-unbundler, {6}, object, (host-openmp)
// CHK-UBACTIONS: 8: input, "{{.*libomp-itt-stubs.o.*}}", object, (host-openmp)
// CHK-UBACTIONS: 9: clang-offload-unbundler, {8}, object, (host-openmp)
// CHK-UBACTIONS: 10: linker, {1, 3, 5, 7, 9}, ir, (device-openmp)
// CHK-UBACTIONS: 11: sycl-post-link, {10}, ir, (device-openmp)
// CHK-UBACTIONS: 12: llvm-spirv, {11}, spirv, (device-openmp)
// CHK-UBACTIONS: 13: offload, "device-openmp (spir64)" {12}, ir
// CHK-UBACTIONS: 14: clang-offload-wrapper, {13}, ir, (host-openmp)
// CHK-UBACTIONS: 15: backend, {14}, assembler, (host-openmp)
// CHK-UBACTIONS: 16: assembler, {15}, object, (host-openmp)
// CHK-UBACTIONS: 17: linker, {1, 3, 5, 7, 9, 16}, image, (host-openmp)


/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUJOBS %s
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-disable-intel-proprietary-opts" {{.*}} "-o" "[[BCFILE:.+\.bc]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-disable-lifetime-markers" "-disable-intel-proprietary-opts" "-Wspir-compat" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[BCFILE]]"{{.*}} "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" "-o" "[[OFFBCFILE:.+\.bc]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[BCFILE]]"
// CHK-BUJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64,host-x86_64-unknown-linux-gnu" "-outputs={{.*}}" "-inputs=[[OFFBCFILE]],[[HOSTOBJ]]"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.o
// RUN:   %clang -### -fiopenmp %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -no-canonical-prefixes -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS %s
// Use of -march should not append to the unbundle value with spir64 targets
// RUN:   %clang -### -fiopenmp %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -march=native -no-canonical-prefixes -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS %s
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-inputs={{.*}}" "-outputs=[[HOSTOBJ:.+\.o]],[[OFFBCFILE:.+\.o]]" "-unbundle"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-inputs={{.*}}libomp-spirvdevicertl.o" "-outputs=[[RTLHOST:.+\.o]],[[RTLTGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: clang-offload-bundler{{.*}}"-type=o"{{.*}}"-inputs={{.*}}libomp-itt-user-wrappers.o"{{.*}}"-outputs=[[ITT1HOST:.+\.o]],[[ITT1TGT:.+\.o]]" "-unbundle"
// CHK-UBJOBS: clang-offload-bundler{{.*}}"-type=o"{{.*}}"-inputs={{.*}}libomp-itt-compiler-wrappers.o"{{.*}}"-outputs=[[ITT2HOST:.+\.o]],[[ITT2TGT:.+\.o]]" "-unbundle"
// CHK-UBJOBS: clang-offload-bundler{{.*}}"-type=o"{{.*}}"-inputs={{.*}}libomp-itt-stubs.o"{{.*}}"-outputs=[[ITT3HOST:.+\.o]],[[ITT3TGT:.+\.o]]" "-unbundle"
// CHK-UBJOBS: llvm-link{{.*}} "[[OFFBCFILE]]" "[[RTLTGT]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-UBJOBS: llvm-link{{.*}} "--only-needed" "[[UNBUNDLED]]" "[[ITT1TGT]]" "[[ITT2TGT]]" "[[ITT3TGT]]" "-o" "[[LINKEDBCFILE:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-emit-param-info" "-ir-output-only" "-O2" "-spec-const=rt" "-o" "[[POSTLINKFILE:.+\.bc]]" "[[LINKEDBCFILE]]"
// CHK-UBJOBS: llvm-spirv{{.*}}" "-o" {{.*}} "[[POSTLINKFILE]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[WRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64"
// CHK-UBJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[TARGOBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-UBJOBS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[RTLHOST]]" "[[ITT1HOST]]" "[[ITT2HOST]]" "[[ITT3HOST]]" "[[TARGOBJ]]" {{.*}} "-lomptarget"

/// ###########################################################################

/// Check that driver forces -O2 for device compilation with --intel
// RUN: %clang -### -fiopenmp -fopenmp-targets=spir64 --intel -c %s -o %t.o 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-DEVICE-O2 %s
// CHK-DEVICE-O2: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-O2" {{.*}} "-fopenmp-is-device"

/// ###########################################################################

/// Check that the device compilation optlevel overrides the host one with --intel
// RUN: %clang -### -fiopenmp -fopenmp-targets=spir64="-O0" -O3 --intel -c %s -o %t.o 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-DEVICE-O3-O0 %s
// CHK-DEVICE-O3-O0: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-O0" {{.*}} "-fopenmp-is-device"

// RUN: %clang -### -fiopenmp -fopenmp-targets=spir64="-O0" --intel -c %s -o %t.o 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-DEVICE-DEF-O0 %s
// CHK-DEVICE-DEF-O0: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-O0" {{.*}} "-fopenmp-is-device"

/// ###########################################################################

/// Check that driver links device objects with libomptarget-opencl.bc with --intel
/// FIXME: we've changed driver to linking libomp-spirvdevicertl.o[bj] the same way
///
// RUN-FAIL: %clang -### -fiopenmp -fopenmp-targets=spir64 --intel -o %t.out %t.o 2>&1 \
// RUN-FAIL:   | FileCheck -check-prefix=CHK-LINK-OPENCLBC %s
// CHK-LINK-OPENCLBC: llvm-link{{.*}}libomptarget-opencl.bc

/// ###########################################################################

// Build a fat static lib that will be used for all tests
// RUN: echo "void foo(void) {}" > %t1.cpp
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 %t1.cpp -c -o %t1_bundle.o
// RUN: llvm-ar cr %t.a %t1_bundle.o

/// Check that driver partially links objects with offload library when -foffload-static-lib=<lib> is used.
// RUN: touch %t.a
// RUN: touch %t-1.o
// RUN: touch %t-2.o
// RUN: touch %t-3.o
// RUN: %clang -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -### %t-1.o %t-2.o %t-3.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_MULTI_O
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=a"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}}

/// ###########################################################################

/// Check phases when -foffload-static-lib=<lib> is used.
// RUN: touch %t.a
// RUN: %clang -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -ccc-print-phases -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_SRC
// FOFFLOAD_STATIC_LIB_SRC: 0: input, "[[INPUT1:.+\.a]]", object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 1: input, "[[INPUT2:.+\.c]]", c, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 2: preprocessor, {1}, cpp-output, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 3: compiler, {2}, ir, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 4: backend, {3}, assembler, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 5: assembler, {4}, object, (host-openmp)

// FOFFLOAD_STATIC_LIB_SRC: 6: input, {{.*libomp-spirvdevicertl.o.*}}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 7: clang-offload-unbundler, {6}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 8: input, {{.*libomp-itt-user-wrappers.o.*}}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 9: clang-offload-unbundler, {8}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 10: input, {{.*libomp-itt-compiler-wrappers.o.*}}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 11: clang-offload-unbundler, {10}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 12: input, {{.*libomp-itt-stubs.o.*}}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 13: clang-offload-unbundler, {12}, object, (host-openmp)

// FOFFLOAD_STATIC_LIB_SRC: 14: input, "[[INPUT2]]", c, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 15: preprocessor, {14}, cpp-output, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 16: compiler, {15}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 17: offload, "host-openmp (x86_64-unknown-linux-gnu)" {3}, "device-openmp (spir64)" {16}, ir
// FOFFLOAD_STATIC_LIB_SRC: 18: backend, {17}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 19: linker, {0, 5, 7, 9, 11, 13}, host_dep_image, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 20: clang-offload-deps, {19}, ir, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 21: input, "[[INPUT1]]", archive
// FOFFLOAD_STATIC_LIB_SRC: 22: clang-offload-unbundler, {21}, archive
// FIXME: the device RTL and the devicelib parts must be linked
//        after the unbundled archive library, when we add -only-needed.
// FOFFLOAD_STATIC_LIB_SRC: 23: linker, {18, 7, 9, 11, 13, 20, 22}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 24: sycl-post-link, {23}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 25: llvm-spirv, {24}, spirv, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 26: offload, "device-openmp (spir64)" {25}, ir
// FOFFLOAD_STATIC_LIB_SRC: 27: clang-offload-wrapper, {26}, ir, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 28: backend, {27}, assembler, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 29: assembler, {28}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 30: linker, {0, 5, 7, 9, 11, 13, 29}, image, (host-openmp)

/// check diagnostic when -fiopenmp isn't used
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_ERROR
// FOPENMP_ERROR: The use of '-fopenmp-targets=spir64' requires '-fiopenmp'
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -openmp -Qopenmp-targets:spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=QOPENMP_ERROR
// QOPENMP_ERROR: The use of '-Qopenmp-targets=spir64' requires '-Qiopenmp'


/// check particular options aren't passed to gcc
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -fopenmp-targets=x86_64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_NOGCC_OPT
// FOPENMP_NOGCC_OPT-NOT: gcc{{.*}} "-fiopenmp" {{.*}} "-fheinous-gnu-extensions" "-fveclib=SVML"

/// -fsycl-device-code-split=per_kernel should not be used for OpenMP
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_NO_SPLIT_KERNEL
// FOPENMP_NO_SPLIT_KERNEL-NOT: sycl-post-link{{.*}} -split=per_kernel

/// -fsycl-device-code-split=auto should not be default for OpenMP
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_NO_SPLIT_AUTO
// FOPENMP_NO_SPLIT_AUTO-NOT: sycl-post-link{{.*}} -split=auto

/// check -fopenmp-target-simd behavior
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fopenmp -fopenmp-targets=spir64 -fopenmp-target-simd %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMD
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fopenmp -fopenmp-targets=spir64 -qopenmp-target-simd %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMD
// RUN: %clang_cl --target=x86_64-pc-windows-msvc --intel -Qopenmp -Qopenmp-targets:spir64 -Qopenmp-target-simd %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMD
// FOPENMP_TARGET_SIMD: clang{{.*}} "-triple" "spir64" {{.*}} "-fopenmp-target-simd" "-mllvm" "-vpo-paropt-enable-device-simd-codegen"
// FOPENMP_TARGET_SIMD: "-mllvm" "-vpo-paropt-emit-spirv-builtins"
// FOPENMP_TARGET_SIMD: "-mllvm" "-vpo-paropt-gpu-execution-scheme=0"
// FOPENMP_TARGET_SIMD: "-mllvm" "-enable-device-simd"
// FOPENMP_TARGET_SIMD: "-mllvm" "-vplan-target-vf=16"
// FOPENMP_TARGET_SIMD: "-O2"
// FOPENMP_TARGET_SIMD: sycl-post-link{{.*}} "--ompoffload-explicit-simd"
// FOPENMP_TARGET_SIMD: llvm-spirv{{.*}}" {{.*}}"-o" {{.*}} "-spirv-allow-unknown-intrinsics" {{.*}}
// FOPENMP_TARGET_SIMD: clang-offload-wrapper{{.*}} "-compile-opts=-vc-codegen"

/// check -fopenmp-target-simdlen=n behavior
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-simd -fopenmp-target-simdlen=8 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMDLEN -DSIMDLEN=8
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-simd -fopenmp-target-simdlen=32 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMDLEN -DSIMDLEN=32
// RUN: %clang_cl --target=x86_64-pc-windows-msvc --intel -Qopenmp -Qopenmp-targets:spir64 -Qopenmp-target-simd -Qopenmp-target-simdlen:64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMDLEN -DSIMDLEN=64
// FOPENMP_TARGET_SIMDLEN: clang{{.*}} "-triple" "spir64" {{.*}} "-fopenmp-target-simd" "-mllvm" "-vpo-paropt-enable-device-simd-codegen"
// FOPENMP_TARGET_SIMDLEN: "-mllvm" "-vplan-target-vf=[[SIMDLEN]]"

// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-simd -fopenmp-target-simdlen=4 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMDLEN_ERROR
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-simd -fopenmp-target-simdlen=128 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMDLEN_ERROR
// RUN: %clangxx -target x86_64-unknown-linux-gnu --intel -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-simd -fopenmp-target-simdlen=hello %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_TARGET_SIMDLEN_ERROR
// FOPENMP_TARGET_SIMDLEN_ERROR: invalid integral value

/// Test for compile and link opts that are passed to the wrapper
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-OPTS %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qopenmp -Qopenmp-targets=spir64 -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-OPTS %s
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -Xopenmp-target-backend=spir64 "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-OPTS %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qopenmp -Qopenmp-targets=spir64 -Xopenmp-target-backend=spir64 "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-OPTS %s
// CHK-TOOLS-OPTS: clang-offload-wrapper{{.*}} "-compile-opts=-DFOO1 -DFOO2"

/// Check for implied options (-g -O0)
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -g -O0 -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qopenmp -Qopenmp-targets=spir64 -Zi -Od -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS %s
// CHK-TOOLS-IMPLIED-OPTS: clang-offload-wrapper{{.*}} "-compile-opts=-g -cl-opt-disable -DFOO1 -DFOO2"

/// Check for proper override (device vs host)
// RUN: %clang -### -target x86_64-unknown-linux-gnu -g -fiopenmp -fopenmp-targets=spir64="-g0" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-G-OVERRIDE %s
// CHK-TOOLS-G-OVERRIDE-NOT: clang-offload-wrapper{{.*}} "-compile-opts=-g"

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -Xopenmp-target-linker "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS2 %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qopenmp -Qopenmp-targets=spir64_x86_64 -Xopenmp-target-linker "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS2 %s
// CHK-TOOLS-CPU-OPTS2: opencl-aot{{.*}} "-DFOO1" "-DFOO2"
// CHK-TOOLS-CPU-OPTS2-NOT: clang-offload-wrapper{{.*}} "-link-opts=-DFOO1 -DFOO2"

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -Xopenmp-target-linker "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-OPTS2 %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qopenmp -Qopenmp-targets=spir64 -Xopenmp-target-linker "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHK-TOOLS-OPTS2 %s
// CHK-TOOLS-OPTS2: clang-offload-wrapper{{.*}} "-link-opts=-DFOO1 -DFOO2"

/// test llvm-link behavior for fopenmp offload
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=OMP_LLVM_LINK_DEVICE_LIB
// OMP_LLVM_LINK_DEVICE_LIB: llvm-link{{.*}} "--only-needed" "{{.*}}"

/// -S -emit-llvm should generate textual IR for device.
// RUN: %clangxx -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -S -emit-llvm %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHECK_S_LLVM %s
// CHECK_S_LLVM: clang{{.*}} "-triple" "spir64"{{.*}} "-emit-llvm"{{.*}} "-o" "[[DEVICE:.+\.ll]]"
// CHECK_S_LLVM: clang{{.*}} "-triple" "x86_64-unknown-linux-gnu"{{.*}} "-emit-llvm"{{.*}} "-o" "[[HOST:.+\.ll]]"
// CHECK_S_LLVM: clang-offload-bundler{{.*}} "-type=ll"{{.*}} "-inputs=[[DEVICE]],[[HOST]]"

/// OpenMP offloading with LTO should produce objects for device
// RUN: %clangxx -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=x86_64 -flto %s 2>&1 \
// RUN:   | FileCheck -check-prefix=LTO_OBJ_CHECK %s
// LTO_OBJ_CHECK: clang{{.*}} "-triple" "x86_64"{{.*}} "-emit-obj"

/// Check spirv-opt
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -O3 -fopenmp-targets=spir64 -fno-openmp-device-lib=all  %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-SPIRV-OPT %s
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -O2 -fopenmp-targets=spir64 -fno-openmp-device-lib=all  %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-SPIRV-OPT %s
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -O1 -fopenmp-targets=spir64 -fno-openmp-device-lib=all  %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-SPIRV-OPT %s
// CHK-SPIRV-OPT: clang{{.*}} "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu"{{.*}} "-mllvm" "-spirv-opt"
