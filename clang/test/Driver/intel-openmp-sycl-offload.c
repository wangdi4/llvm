///
/// Driver tests for SYCL + OpenMP offloading with -fiopenmp and target spir64
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// UNSUPPORTED: system-windows

/// ###########################################################################

/// Check phases graph when using single source file.
// RUN:   %clang -ccc-print-phases --intel -fsycl -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-sycl)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-sycl)
// CHK-PHASES: 2: input, "[[INPUT]]", c, (device-sycl)
// CHK-PHASES: 3: preprocessor, {2}, cpp-output, (device-sycl)
// CHK-PHASES: 4: compiler, {3}, sycl-header, (device-sycl)
// CHK-PHASES: 5: offload, "host-sycl (x86_64-unknown-linux-gnu)" {1}, "device-sycl (spir64-unknown-unknown-sycldevice)" {4}, cpp-output
// CHK-PHASES: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-PHASES: 7: backend, {6}, assembler, (host-openmp-sycl)
// CHK-PHASES: 8: assembler, {7}, object, (host-openmp-sycl)
// CHK-PHASES: 9: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 10: preprocessor, {9}, cpp-output, (device-openmp)
// CHK-PHASES: 11: compiler, {10}, ir, (device-openmp)
// CHK-PHASES: 12: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {6}, "device-openmp (spir64)" {11}, ir
// CHK-PHASES: 13: backend, {12}, ir, (device-openmp)
// CHK-PHASES: 14: linker, {13}, image, (device-openmp)
// CHK-PHASES: 15: llvm-spirv, {14}, image, (device-openmp)
// CHK-PHASES: 16: offload, "device-openmp (spir64)" {15}, image
// CHK-PHASES: 17: clang-offload-wrapper, {16}, ir, (host-openmp-sycl)
// CHK-PHASES: 18: backend, {17}, assembler, (host-openmp-sycl)
// CHK-PHASES: 19: assembler, {18}, object, (host-openmp-sycl)
// CHK-PHASES: 20: linker, {8, 19}, image, (host-openmp-sycl)
// CHK-PHASES: 21: compiler, {3}, ir, (device-sycl)
// CHK-PHASES: 22: linker, {21}, ir, (device-sycl)
// CHK-PHASES: 23: llvm-spirv, {22}, spirv, (device-sycl)
// CHK-PHASES: 24: clang-offload-wrapper, {23}, object, (device-sycl)
// CHK-PHASES: 25: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {20}, "device-sycl (spir64-unknown-unknown-sycldevice)" {24}, image

/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### --intel -fsycl -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s

// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-disable-llvm-passes" {{.*}} "-emit-llvm-bc" {{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-COMMANDS: llvm-spirv{{.*}} "-o" "[[SYCLSPIRV:.+\.spv]]" {{.*}} "[[SYCLLINKEDBC]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl" "[[SYCLSPIRV]]"
// CHK-COMMANDS: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-disable-llvm-passes" {{.*}} "-fsycl-int-header=[[SYCLHEADER:.+\.h]]" "-mllvm" "-paropt=31" {{.*}} "-o" "[[SYCLHEADER]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-disable-llvm-passes" "-include" "[[SYCLHEADER]]" "-dependency-filter" "[[SYCLHEADER]]" "-fsycl-is-host" "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown-sycldevice" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[OMPBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBC:.+\.out]]"
// CHK-COMMANDS: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.out]]" "[[OMPLINKEDBC]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-COMMANDS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

/// ###########################################################################

/// Check separate compilation with offloading - bundling actions
// RUN:   %clang -### -ccc-print-phases --intel -fsycl -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUACTIONS %s

// CHK-BUACTIONS: 0: input, "[[INPUT:.+\.c]]", c, (host-sycl)
// CHK-BUACTIONS: 1: preprocessor, {0}, cpp-output, (host-sycl)
// CHK-BUACTIONS: 2: input, "[[INPUT]]", c, (device-sycl)
// CHK-BUACTIONS: 3: preprocessor, {2}, cpp-output, (device-sycl)
// CHK-BUACTIONS: 4: compiler, {3}, sycl-header, (device-sycl)
// CHK-BUACTIONS: 5: offload, "host-sycl (x86_64-unknown-linux-gnu)" {1}, "device-sycl (spir64-unknown-unknown-sycldevice)" {4}, cpp-output
// CHK-BUACTIONS: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-BUACTIONS: 7: input, "[[INPUT]]", c, (device-openmp)
// CHK-BUACTIONS: 8: preprocessor, {7}, cpp-output, (device-openmp)
// CHK-BUACTIONS: 9: compiler, {8}, ir, (device-openmp)
// CHK-BUACTIONS: 10: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {6}, "device-openmp (spir64)" {9}, ir
// CHK-BUACTIONS: 11: backend, {10}, ir, (device-openmp)
// CHK-BUACTIONS: 12: offload, "device-openmp (spir64)" {11}, ir
// CHK-BUACTIONS: 13: compiler, {3}, ir, (device-sycl)
// CHK-BUACTIONS: 14: offload, "device-sycl (spir64-unknown-unknown-sycldevice)" {13}, ir
// CHK-BUACTIONS: 15: backend, {6}, assembler, (host-openmp-sycl)
// CHK-BUACTIONS: 16: assembler, {15}, object, (host-openmp-sycl)
// CHK-BUACTIONS: 17: clang-offload-bundler, {12, 14, 16}, object, (host-openmp-sycl)

/// ###########################################################################

/// Check separate compilation with offloading - unbundling actions
// RUN:   touch %t.o
// RUN:   %clang -### -ccc-print-phases --intel -fsycl -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %t.o -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBACTIONS %s

// CHK-UBACTIONS: 0: input, "[[INPUT:.+\.o]]", object, (host-openmp-sycl)
// CHK-UBACTIONS: 1: clang-offload-unbundler, {0}, object, (host-openmp-sycl)
// CHK-UBACTIONS: 2: linker, {1}, image, (device-openmp)
// CHK-UBACTIONS: 3: llvm-spirv, {2}, image, (device-openmp)
// CHK-UBACTIONS: 4: offload, "device-openmp (spir64)" {3}, image
// CHK-UBACTIONS: 5: clang-offload-wrapper, {4}, ir, (host-openmp-sycl)
// CHK-UBACTIONS: 6: backend, {5}, assembler, (host-openmp-sycl)
// CHK-UBACTIONS: 7: assembler, {6}, object, (host-openmp-sycl)
// CHK-UBACTIONS: 8: linker, {1, 7}, image, (host-openmp-sycl)
// CHK-UBACTIONS: 9: linker, {1}, ir, (device-sycl)
// CHK-UBACTIONS: 10: llvm-spirv, {9}, spirv, (device-sycl)
// CHK-UBACTIONS: 11: clang-offload-wrapper, {10}, object, (device-sycl)
// CHK-UBACTIONS: 12: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {8}, "device-sycl (spir64-unknown-unknown-sycldevice)" {11}, image

/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### --intel -fsycl -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUJOBS -DFATOBJ=%t.o %s

// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-disable-llvm-passes" {{.*}} "-fsycl-int-header=[[SYCLHEADER:.+\.h]]" "-mllvm" "-paropt=31" {{.*}} "-o" "[[SYCLHEADER]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-disable-llvm-passes" "-include" "[[SYCLHEADER]]" "-dependency-filter" "[[SYCLHEADER]]" "-fsycl-is-host" "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown-sycldevice" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-disable-llvm-passes" {{.*}} "-emit-llvm-bc" {{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-BUJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64,sycl-spir64-unknown-unknown-sycldevice,host-x86_64-unknown-linux-gnu" "-outputs=[[FATOBJ]]" "-inputs=[[OMPBC]],[[SYCLBC]],[[HOSTOBJ]]"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.o
// RUN:   %clang -### --intel -fsycl -fiopenmp %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS -DFATOBJ=%t.o %s

// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64,sycl-spir64-unknown-unknown-sycldevice" "-inputs=[[FATOBJ]]" "-outputs=[[HOSTOBJ:.+\.o]],[[OMPBC:.+\.o]],[[SYCLBC:.+\.o]]" "-unbundle"
// CHK-UBJOBS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-UBJOBS: llvm-spirv{{.*}} "-o" "[[SYCLSPIRV:.+\.spv]]" {{.*}} "[[SYCLLINKEDBC]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl" "[[SYCLSPIRV]]"
// CHK-UBJOBS: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-UBJOBS: llvm-link{{.*}} "[[OMPBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBC:.+\.out]]"
// CHK-UBJOBS: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.out]]" "[[OMPLINKEDBC]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// CHK-UBJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-UBJOBS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

/// ###########################################################################

/// Check that driver partially links objects with offload library when -foffload-static-lib=<lib> is used.
// RUN: touch %t.a
// RUN: touch %t-1.o
// RUN: touch %t-2.o
// RUN: touch %t-3.o
// RUN: %clang -target x86_64-unknown-linux-gnu --intel -fsycl -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -### %t-1.o %t-2.o %t-3.o 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_MULTI_O

// FOFFLOAD_STATIC_LIB_MULTI_O: ld{{.*}} "-r" "-o" "[[FATOBJ:.+\.o]]" "{{.*}}crt1.o" {{.*}} "[[INPUT1:.+\-1.o]]" "[[INPUT2:.+\-2.o]]" "[[INPUT3:.+\-3.o]]" "[[INPUT:.+\.a]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=oo" "-targets=openmp-spir64,sycl-spir64-unknown-unknown-sycldevice" "-inputs=[[FATOBJ]]" "-outputs=[[OMPLIST:.+\.txt]],[[SYCLLIST:.+\.txt]]" "-unbundle"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "@[[SYCLLIST]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-spirv{{.*}} "-o" "[[SYCLSPIRV:.+\.spv]]" {{.*}} "[[SYCLLINKEDBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl" "[[SYCLSPIRV]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "@[[OMPLIST]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBC:.+\.out]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.out]]" "[[OMPLINKEDBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: ld{{.*}} "-o" {{.*}} "[[INPUT1]]" "[[INPUT2]]" "[[INPUT3]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

/// ###########################################################################

/// Check phases when -foffload-static-lib=<lib> is used.
// RUN: touch %t.a
// RUN: %clang -target x86_64-unknown-linux-gnu --intel -fsycl -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -ccc-print-phases %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_SRC

// FOFFLOAD_STATIC_LIB_SRC: 0: input, "[[INPUT1:.+\.a]]", object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 1: input, "[[INPUT2:.+\.c]]", c, (host-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 2: preprocessor, {1}, cpp-output, (host-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 3: input, "[[INPUT2]]", c, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 4: preprocessor, {3}, cpp-output, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 5: compiler, {4}, sycl-header, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 6: offload, "host-sycl (x86_64-unknown-linux-gnu)" {2}, "device-sycl (spir64-unknown-unknown-sycldevice)" {5}, cpp-output
// FOFFLOAD_STATIC_LIB_SRC: 7: compiler, {6}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 8: backend, {7}, assembler, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 9: assembler, {8}, object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 10: input, "[[INPUT2]]", c, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 11: preprocessor, {10}, cpp-output, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 12: compiler, {11}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 13: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {7}, "device-openmp (spir64)" {12}, ir
// FOFFLOAD_STATIC_LIB_SRC: 14: backend, {13}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 15: input, "[[INPUT1]]", archive
// FOFFLOAD_STATIC_LIB_SRC: 16: clang-offload-unbundler, {15}, object
// FOFFLOAD_STATIC_LIB_SRC: 17: linker, {14, 16}, image, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 18: llvm-spirv, {17}, image, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 19: offload, "device-openmp (spir64)" {18}, image
// FOFFLOAD_STATIC_LIB_SRC: 20: clang-offload-wrapper, {19}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 21: backend, {20}, assembler, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 22: assembler, {21}, object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 23: linker, {0, 9, 22}, image, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 24: compiler, {4}, ir, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 25: linker, {24, 16}, ir, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 26: llvm-spirv, {25}, spirv, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 27: clang-offload-wrapper, {26}, object, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 28: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {23}, "device-sycl (spir64-unknown-unknown-sycldevice)" {27}, image
