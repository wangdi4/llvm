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
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 6: preprocessor, {5}, cpp-output, (device-openmp)
// CHK-PHASES: 7: compiler, {6}, ir, (device-openmp)
// CHK-PHASES: 8: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {7}, ir
// CHK-PHASES: 9: backend, {8}, ir, (device-openmp)
// CHK-PHASES: 10: linker, {9}, image, (device-openmp)
// CHK-PHASES: 11: llvm-spirv, {10}, image, (device-openmp)
// CHK-PHASES: 12: offload, "device-openmp (spir64)" {11}, image
// CHK-PHASES: 13: clang-offload-wrapper, {12}, ir, (host-openmp)
// CHK-PHASES: 14: backend, {13}, assembler, (host-openmp)
// CHK-PHASES: 15: assembler, {14}, object, (host-openmp)
// CHK-PHASES: 16: linker, {4, 15}, image, (host-openmp)


/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-o" "[[BCFILE:.+\.bc]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[BCFILE]]"     
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[BCFILE]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" "-o" "[[OFFBCFILE:.+\.bc]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[OFFBCFILE]]" "-o" "[[LINKEDBCFILE:.+\.out]]"
// CHK-COMMANDS: llvm-spirv{{.*}}" "-o" {{.*}} "[[LINKEDBCFILE]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[WRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[TARGOBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-COMMANDS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[TARGOBJ]]" {{.*}} "-lomptarget"

/// Check additional options passed through
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64="-DFOO -DBAR -mllvm -dummy-opt -Xclang -cc1dummy" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TARGOPTS %s
// CHK-TARGOPTS: clang{{.*}} "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc" {{.*}} "-D" "FOO" "-D" "BAR" {{.*}} "-cc1dummy" "-mllvm" "-dummy-opt" {{.*}} "-fopenmp-targets=spir64"

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
// RUN:   %clang -### -ccc-print-phases -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %t.o -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -DINPUT=%t.o -check-prefix=CHK-UBACTIONS %s
// CHK-UBACTIONS: 0: input, "[[INPUT]]", object, (host-openmp)
// CHK-UBACTIONS: 1: clang-offload-unbundler, {0}, object, (host-openmp)
// CHK-UBACTIONS: 2: linker, {1}, image, (device-openmp)
// CHK-UBACTIONS: 3: llvm-spirv, {2}, image, (device-openmp)
// CHK-UBACTIONS: 4: offload, "device-openmp (spir64)" {3}, image
// CHK-UBACTIONS: 5: clang-offload-wrapper, {4}, ir, (host-openmp)
// CHK-UBACTIONS: 6: backend, {5}, assembler, (host-openmp)
// CHK-UBACTIONS: 7: assembler, {6}, object, (host-openmp)
// CHK-UBACTIONS: 8: linker, {1, 7}, image, (host-openmp)

/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUJOBS %s
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-o" "[[BCFILE:.+\.bc]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[BCFILE]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" "-o" "[[OFFBCFILE:.+\.bc]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[BCFILE]]"
// CHK-BUJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64,host-x86_64-unknown-linux-gnu" "-outputs={{.*}}" "-inputs=[[OFFBCFILE]],[[HOSTOBJ]]"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.o
// RUN:   %clang -### -fiopenmp %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS %s
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-inputs={{.*}}" "-outputs=[[HOSTOBJ:.+\.o]],[[OFFBCFILE:.+\.o]]" "-unbundle"
// CHK-UBJOBS: llvm-link{{.*}} "[[OFFBCFILE]]" "-o" "[[LINKEDBCFILE:.+\.out]]"
// CHK-UBJOBS: llvm-spirv{{.*}}" "-o" {{.*}} "[[LINKEDBCFILE]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[WRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64"
// CHK-UBJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[TARGOBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-UBJOBS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[TARGOBJ]]" {{.*}} "-lomptarget"

/// ###########################################################################

/// Check that driver forces -O0 for device compilation with --intel
// RUN: %clang -### -fiopenmp -fopenmp-targets=spir64 --intel -c %s -o %t.o 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-DEVICE-O0 %s
// CHK-DEVICE-O0: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-mlong-double-64" {{.*}} "-O0" {{.*}} "-fopenmp-is-device"

/// ###########################################################################

/// Check that driver links device objects with libomptarget-opencl.bc with --intel
// RUN: %clang -### -fiopenmp -fopenmp-targets=spir64 --intel -o %t.out %t.o 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-LINK-OPENCLBC %s
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
// FOFFLOAD_STATIC_LIB_MULTI_O: ld{{(.exe)?}}" "-r" "-o" {{.*}} "[[INPUT:.+\-1.o]]" "[[INPUT:.+\-2.o]]" "[[INPUT:.+\-3.o]]" "[[INPUT:.+\.a]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=oo"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "@{{.*}}"

/// Check for proper object usage for partial link.  llvm-link should not
/// contain the input object (consumed in partial link)
// RUN: %clang -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 %t-1.o %t.a -### 2>&1 \
// RUN::  | FileCheck %s -check-prefix=PARTIAL_LINK_CHECK
// PARTIAL_LINK_CHECK: ld{{(.exe)?}}" "-r" "-o" {{.*}} "[[INPUTO:.+\.o]]" "[[INPUTA:.+\.a]]"
// PARTIAL_LINK_CHECK-NOT: llvm-link{{.*}} "[[INPUTO]]"
// PARTIAL_LINK_CHECK: ld{{.*}} "[[INPUTA]]"

/// ###########################################################################

/// Check phases when -foffload-static-lib=<lib> is used.
// RUN: touch %t.a
// RUN: %clang -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -ccc-print-phases %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_SRC
// FOFFLOAD_STATIC_LIB_SRC: 0: input, "[[INPUT1:.+\.a]]", object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 1: input, "[[INPUT2:.+\.c]]", c, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 2: preprocessor, {1}, cpp-output, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 3: compiler, {2}, ir, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 4: backend, {3}, assembler, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 5: assembler, {4}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 6: input, "[[INPUT2]]", c, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 7: preprocessor, {6}, cpp-output, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 8: compiler, {7}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 9: offload, "host-openmp (x86_64-unknown-linux-gnu)" {3}, "device-openmp (spir64)" {8}, ir
// FOFFLOAD_STATIC_LIB_SRC: 10: backend, {9}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 11: input, "[[INPUT1]]", archive
// FOFFLOAD_STATIC_LIB_SRC: 12: partial-link, {5, 11}, object
// FOFFLOAD_STATIC_LIB_SRC: 13: clang-offload-unbundler, {12}, object
// FOFFLOAD_STATIC_LIB_SRC: 14: linker, {10, 13}, image, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 15: llvm-spirv, {14}, image, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 16: offload, "device-openmp (spir64)" {15}, image
// FOFFLOAD_STATIC_LIB_SRC: 17: clang-offload-wrapper, {16}, ir, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 18: backend, {17}, assembler, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 19: assembler, {18}, object, (host-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 20: linker, {0, 5, 19}, image, (host-openmp)

/// check diagnostic when -fiopenmp isn't used
// RUN: %clangxx -target x86_64-unknown-linux-gnu -fopenmp -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=FOPENMP_ERROR
// FOPENMP_ERROR: The use of '-fopenmp-targets=spir64' requires '-fiopenmp'
