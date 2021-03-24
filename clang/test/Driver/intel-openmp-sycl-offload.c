///
/// Driver tests for SYCL + OpenMP offloading with -fiopenmp and target spir64
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// UNSUPPORTED: system-windows
//

/// ###########################################################################

/// Check phases graph when using single source file.
// RUN:   %clang -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c++, (host-sycl)
// CHK-PHASES: 1: preprocessor, {0}, c++-cpp-output, (host-sycl)
// CHK-PHASES: 2: input, "[[INPUT]]", c++, (device-sycl)
// CHK-PHASES: 3: preprocessor, {2}, c++-cpp-output, (device-sycl)
// CHK-PHASES: 4: compiler, {3}, sycl-header, (device-sycl)
// CHK-PHASES: 5: offload, "host-sycl (x86_64-unknown-linux-gnu)" {1}, "device-sycl (spir64-unknown-unknown-sycldevice)" {4}, c++-cpp-output
// CHK-PHASES: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-PHASES: 7: backend, {6}, assembler, (host-openmp-sycl)
// CHK-PHASES: 8: assembler, {7}, object, (host-openmp-sycl)
// CHK-PHASES: 9: input, "[[INPUT]]", c++, (device-openmp)
// CHK-PHASES: 10: preprocessor, {9}, c++-cpp-output, (device-openmp)
// CHK-PHASES: 11: compiler, {10}, ir, (device-openmp)
// CHK-PHASES: 12: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {6}, "device-openmp (spir64)" {11}, ir
// CHK-PHASES: 13: backend, {12}, ir, (device-openmp)
// CHK-PHASES: 14: linker, {13}, ir, (device-openmp)
// CHK-PHASES: 15: sycl-post-link, {14}, ir, (device-openmp)
// CHK-PHASES: 16: llvm-spirv, {15}, spirv, (device-openmp)
// CHK-PHASES: 17: offload, "device-openmp (spir64)" {16}, ir
// CHK-PHASES: 18: clang-offload-wrapper, {17}, ir, (host-openmp-sycl)
// CHK-PHASES: 19: backend, {18}, assembler, (host-openmp-sycl)
// CHK-PHASES: 20: assembler, {19}, object, (host-openmp-sycl)
// CHK-PHASES: 21: linker, {8, 20}, image, (host-openmp-sycl)
// CHK-PHASES: 22: compiler, {3}, ir, (device-sycl)
// CHK-PHASES: 23: linker, {22}, ir, (device-sycl)
// CHK-PHASES: 24: sycl-post-link, {23}, tempfiletable, (device-sycl)
// CHK-PHASES: 25: file-table-tform, {24}, tempfilelist, (device-sycl)
// CHK-PHASES: 26: llvm-spirv, {25}, tempfilelist, (device-sycl)
// CHK-PHASES: 27: file-table-tform, {24, 26}, tempfiletable, (device-sycl)
// CHK-PHASES: 28: clang-offload-wrapper, {27}, object, (device-sycl)
// CHK-PHASES: 29: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {21}, "device-sycl (spir64-unknown-unknown-sycldevice)" {28}, image


/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s

// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-emit-llvm-bc" {{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{.*}} "-o" "[[TABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-o" "[[FILELIST:.+\.txt]]" "[[TABLE]]"
// CHK-COMMANDS: llvm-foreach{{.*}} "--in-file-list=[[FILELIST]]"{{.*}}"--out-file-list=[[SPVOUTPUT:[^ ]+\.txt]]"
// CHK-COMMANDS-SAME: llvm-spirv{{.*}} "-o" "[[SPVOUTPUT]]" {{.*}} "[[FILELIST]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-o" "[[TABLEFORWRAPPER:.+\.table]]" "[[TABLE]]" "[[SPVOUTPUT]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl"{{.*}} "[[TABLEFORWRAPPER]]"
// CHK-COMMANDS: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-fsycl-int-header=[[SYCLHEADER:.+\.h]]" {{.*}} "-o" "[[SYCLHEADER]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-include" "[[SYCLHEADER]]" "-dependency-filter" "[[SYCLHEADER]]" "-fsycl-is-host" "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown-sycldevice" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-fsycl" "-fsycl-is-host" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[OMPBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBC:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-o" "[[OMPPOSTLINK:.+\.bc]]" "[[OMPLINKEDBC]]"
// CHK-COMMANDS: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.spv]]" "-spirv-ext=+all,-SPV_INTEL_fpga_buffer_location" "-spirv-allow-unknown-intrinsics" "[[OMPPOSTLINK]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-COMMANDS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

/// ###########################################################################

/// Check separate compilation with offloading - bundling actions
// RUN:   %clang -### -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUACTIONS %s

// CHK-BUACTIONS: 0: input, "[[INPUT:.+\.c]]", c++, (host-sycl)
// CHK-BUACTIONS: 1: preprocessor, {0}, c++-cpp-output, (host-sycl)
// CHK-BUACTIONS: 2: input, "[[INPUT]]", c++, (device-sycl)
// CHK-BUACTIONS: 3: preprocessor, {2}, c++-cpp-output, (device-sycl)
// CHK-BUACTIONS: 4: compiler, {3}, sycl-header, (device-sycl)
// CHK-BUACTIONS: 5: offload, "host-sycl (x86_64-unknown-linux-gnu)" {1}, "device-sycl (spir64-unknown-unknown-sycldevice)" {4}, c++-cpp-output
// CHK-BUACTIONS: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-BUACTIONS: 7: input, "[[INPUT]]", c++, (device-openmp)
// CHK-BUACTIONS: 8: preprocessor, {7}, c++-cpp-output, (device-openmp)
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
// RUN:   %clang -### -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %t.o -no-canonical-prefixes -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBACTIONS %s

// CHK-UBACTIONS: 0: input, "[[INPUT:.+\.o]]", object, (host-openmp-sycl)
// CHK-UBACTIONS: 1: clang-offload-unbundler, {0}, object, (host-openmp-sycl)
// CHK-UBACTIONS: 2: linker, {1}, ir, (device-openmp)
// CHK-UBACTIONS: 3: sycl-post-link, {2}, ir, (device-openmp)
// CHK-UBACTIONS: 4: llvm-spirv, {3}, spirv, (device-openmp)
// CHK-UBACTIONS: 5: offload, "device-openmp (spir64)" {4}, ir
// CHK-UBACTIONS: 6: clang-offload-wrapper, {5}, ir, (host-openmp-sycl)
// CHK-UBACTIONS: 7: backend, {6}, assembler, (host-openmp-sycl)
// CHK-UBACTIONS: 8: assembler, {7}, object, (host-openmp-sycl)
// CHK-UBACTIONS: 9: linker, {1, 8}, image, (host-openmp-sycl)
// CHK-UBACTIONS: 10: linker, {1}, ir, (device-sycl)
// CHK-UBACTIONS: 11: sycl-post-link, {10}, tempfiletable, (device-sycl)
// CHK-UBACTIONS: 12: file-table-tform, {11}, tempfilelist, (device-sycl)
// CHK-UBACTIONS: 13: llvm-spirv, {12}, tempfilelist, (device-sycl)
// CHK-UBACTIONS: 14: file-table-tform, {11, 13}, tempfiletable, (device-sycl)
// CHK-UBACTIONS: 15: clang-offload-wrapper, {14}, object, (device-sycl)
// CHK-UBACTIONS: 16: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {9}, "device-sycl (spir64-unknown-unknown-sycldevice)" {15}, image

/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### --intel -fsycl -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUJOBS -DFATOBJ=%t.o %s

// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-fsycl-int-header=[[SYCLHEADER:.+\.h]]" {{.*}} "-o" "[[SYCLHEADER]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-include" "[[SYCLHEADER]]" "-dependency-filter" "[[SYCLHEADER]]" "-fsycl-is-host" "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown-sycldevice" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-fsycl" "-fsycl-is-host" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-emit-llvm-bc" {{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-BUJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64,sycl-spir64-unknown-unknown-sycldevice,host-x86_64-unknown-linux-gnu" "-outputs=[[FATOBJ]]" "-inputs=[[OMPBC]],[[SYCLBC]],[[HOSTOBJ]]"

/// Check that -paropt is not passed for -fsycl device
// RUN:   %clang -### --intel -fsycl -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PAROPT -DFATOBJ=%t.o %s
// CHK-PAROPT-NOT: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice" "-fsycl-is-device" {{.*}} "-mllvm" "-paropt{{.*}}"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.o
// RUN:   %clang -### --intel -fsycl -fno-sycl-device-lib=all -fiopenmp %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -no-canonical-prefixes -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS -DFATOBJ=%t.o %s

// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64,sycl-spir64-unknown-unknown-sycldevice" "-inputs=[[FATOBJ]]" "-outputs=[[HOSTOBJ:.+\.o]],[[OMPBC:.+\.o]],[[SYCLBC:.+\.o]]" "-unbundle"
// CHK-UBJOBS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{.*}} "-symbols" "-split-esimd" "-lower-esimd" "-O2" "-spec-const=rt" "-o" "[[SYCLTABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[SYCLTABLEOUT:.+\.txt]]" "[[SYCLTABLE]]"
// CHK-UBJOBS: llvm-foreach{{.*}} "--in-file-list=[[SYCLTABLEOUT]]" "--in-replace=[[SYCLTABLEOUT]]" "--out-ext=spv" "--out-file-list=[[SYCLLLVMFOROUT:.+\.txt]]" "--out-replace=[[SYCLLLVMFOROUT]]" "--" "{{.*}}llvm-spirv" "-o" "[[SYCLLLVMFOROUT]]" "-spirv-max-version=1.3" "-spirv-debug-info-version=legacy" "-spirv-allow-extra-diexpressions" "-spirv-ext=+all,-SPV_INTEL_usm_storage_classes,-SPV_INTEL_optnone" "[[SYCLTABLEOUT]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[SYCLSPIRV:.+\.table]]" "[[SYCLTABLE]]" "[[SYCLLLVMFOROUT]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl" "-batch" "[[SYCLSPIRV]]"
// CHK-UBJOBS: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-UBJOBS: llvm-link{{.*}} "[[OMPBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBC:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-o" "[[OMPPOSTLINK:.+\.bc]]" "[[OMPLINKEDBC]]"
// CHK-UBJOBS: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.spv]]" "-spirv-ext=+all,-SPV_INTEL_fpga_buffer_location" "-spirv-allow-unknown-intrinsics" "[[OMPPOSTLINK]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// CHK-UBJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-UBJOBS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

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
// RUN: %clang -target x86_64-unknown-linux-gnu --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -### %t-1.o %t-2.o %t-3.o -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_MULTI_O
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=a" "-targets=openmp-spir64,sycl-spir64-unknown-unknown-sycldevice" "-inputs=[[INPUTA:.+\.a]]" "-outputs=[[OMPLIB:.+\.a]],[[SYCLLIB:.+\.a]]" "-unbundle"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "[[SYCLLIB]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: sycl-post-link{{.*}} "-symbols" "-split-esimd" "-lower-esimd" "-O2" "-spec-const=rt" "-o" "[[SYCLTABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[SYCLTABLEOUT:.+\.txt]]" "[[SYCLTABLE]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-foreach{{.*}} "--in-file-list=[[SYCLTABLEOUT]]" "--in-replace=[[SYCLTABLEOUT]]" "--out-ext=spv" "--out-file-list=[[SYCLLLVMFOROUT:.+\.txt]]" "--out-replace=[[SYCLLLVMFOROUT]]" "--" "{{.*}}llvm-spirv" "-o" "[[SYCLLLVMFOROUT]]" "-spirv-max-version=1.3" "-spirv-debug-info-version=legacy" "-spirv-allow-extra-diexpressions" "-spirv-ext=+all,-SPV_INTEL_usm_storage_classes,-SPV_INTEL_optnone" "[[SYCLTABLEOUT]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[SYCLSPIRV:.+\.table]]" "[[SYCLTABLE]]" "[[SYCLLLVMFOROUT]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl" "-batch" "[[SYCLSPIRV]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "{{.*}}libomptarget-opencl.bc" "-o" "[[LINKOUT1:.+\.bc]]" "--suppress-warnings"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "[[LINKOUT1]]" "[[OMPLIB]]" "-o" "[[OMPLINKEDBC:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-o" "[[OMPPOSTLINK:.+\.bc]]" "[[OMPLINKEDBC]]
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.spv]]" "-spirv-ext=+all,-SPV_INTEL_fpga_buffer_location" "-spirv-allow-unknown-intrinsics" "[[OMPPOSTLINK]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: ld{{.*}} "-o" {{.*}} "[[INPUTA]]"{{.*}} "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

/// ###########################################################################

/// Check phases when -foffload-static-lib=<lib> is used.
// RUN: touch %t.a
// RUN: %clang -target x86_64-unknown-linux-gnu --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -ccc-print-phases %s -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_SRC

// FOFFLOAD_STATIC_LIB_SRC: 0: input, "[[INPUT1:.+\.a]]", object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 1: input, "[[INPUT2:.+\.c]]", c++, (host-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 2: preprocessor, {1}, c++-cpp-output, (host-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 3: input, "[[INPUT2]]", c++, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 4: preprocessor, {3}, c++-cpp-output, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 5: compiler, {4}, sycl-header, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 6: offload, "host-sycl (x86_64-unknown-linux-gnu)" {2}, "device-sycl (spir64-unknown-unknown-sycldevice)" {5}, c++-cpp-output
// FOFFLOAD_STATIC_LIB_SRC: 7: compiler, {6}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 8: backend, {7}, assembler, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 9: assembler, {8}, object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 10: input, "[[INPUT2]]", c++, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 11: preprocessor, {10}, c++-cpp-output, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 12: compiler, {11}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 13: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {7}, "device-openmp (spir64)" {12}, ir
// FOFFLOAD_STATIC_LIB_SRC: 14: backend, {13}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 15: linker, {0, 9}, image, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 16: clang-offload-deps, {15}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 17: input, "[[INPUT1]]", archive
// FOFFLOAD_STATIC_LIB_SRC: 18: clang-offload-unbundler, {17}, archive
// FOFFLOAD_STATIC_LIB_SRC: 19: linker, {14, 16, 18}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 20: sycl-post-link, {19}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 21: llvm-spirv, {20}, spirv, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 22: offload, "device-openmp (spir64)" {21}, ir
// FOFFLOAD_STATIC_LIB_SRC: 23: clang-offload-wrapper, {22}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 24: backend, {23}, assembler, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 25: assembler, {24}, object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 26: linker, {0, 9, 25}, image, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 27: compiler, {4}, ir, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 28: linker, {27, 16, 18}, ir, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 29: sycl-post-link, {28}, tempfiletable, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 30: file-table-tform, {29}, tempfilelist, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 31: llvm-spirv, {30}, tempfilelist, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 32: file-table-tform, {29, 31}, tempfiletable, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 33: clang-offload-wrapper, {32}, object, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 34: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {26}, "device-sycl (spir64-unknown-unknown-sycldevice)" {33}, image
