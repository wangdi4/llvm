///
/// Driver tests for SYCL + OpenMP offloading with -fiopenmp and target spir64
///

// REQUIRES: x86-registered-target
// UNSUPPORTED: system-windows
//

/// ###########################################################################

/// Check phases graph when using single source file.
// RUN:   %clang -ccc-print-phases --sysroot=%S/Inputs/SYCL --intel -fsycl -fno-sycl-use-footer -fno-sycl-device-lib=all -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c++, (host-sycl)
// CHK-PHASES: 1: preprocessor, {0}, c++-cpp-output, (host-sycl)
// CHK-PHASES: 2: input, "[[INPUT]]", c++, (device-sycl)
// CHK-PHASES: 3: preprocessor, {2}, c++-cpp-output, (device-sycl)
// CHK-PHASES: 4: compiler, {3}, ir, (device-sycl)
// CHK-PHASES: 5: offload, "host-sycl (x86_64-unknown-linux-gnu)" {1}, "device-sycl (spir64-unknown-unknown)" {4}, c++-cpp-output
// CHK-PHASES: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-PHASES: 7: backend, {6}, assembler, (host-openmp-sycl)
// CHK-PHASES: 8: assembler, {7}, object, (host-openmp-sycl)
// CHK-PHASES: 9: input, "[[INPUT]]", c++, (device-openmp)
// CHK-PHASES: 10: preprocessor, {9}, c++-cpp-output, (device-openmp)
// CHK-PHASES: 11: compiler, {10}, ir, (device-openmp)
// CHK-PHASES: 12: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {6}, "device-openmp (spir64)" {11}, ir
// CHK-PHASES: 13: backend, {12}, ir, (device-openmp)
// CHK-PHASES: 14: input, "{{.*libomp-spirvdevicertl.o.*}}", object
// CHK-PHASES: 15: clang-offload-unbundler, {14}, object
// CHK-PHASES: 16: input, "{{.*libomp-itt-user-wrappers.o.*}}", object
// CHK-PHASES: 17: clang-offload-unbundler, {16}, object
// CHK-PHASES: 18: input, "{{.*libomp-itt-compiler-wrappers.o.*}}", object
// CHK-PHASES: 19: clang-offload-unbundler, {18}, object
// CHK-PHASES: 20: input, "{{.*libomp-itt-stubs.o.*}}", object
// CHK-PHASES: 21: clang-offload-unbundler, {20}, object
// CHK-PHASES: 22: linker, {13, 15, 17, 19, 21}, ir, (device-openmp)
// CHK-PHASES: 23: sycl-post-link, {22}, ir, (device-openmp)
// CHK-PHASES: 24: llvm-spirv, {23}, spirv, (device-openmp)
// CHK-PHASES: 25: offload, "device-openmp (spir64)" {24}, ir
// CHK-PHASES: 26: clang-offload-wrapper, {25}, ir, (host-openmp-sycl)
// CHK-PHASES: 27: backend, {26}, assembler, (host-openmp-sycl)
// CHK-PHASES: 28: assembler, {27}, object, (host-openmp-sycl)
// CHK-PHASES: 29: linker, {8, 28}, image, (host-openmp-sycl)
// CHK-PHASES: 30: linker, {4}, ir, (device-sycl)
// CHK-PHASES: 31: input, "{{.*libsycl-itt-user-wrappers.o.*}}", object
// CHK-PHASES: 32: clang-offload-unbundler, {31}, object
// CHK-PHASES: 33: offload, " (spir64-unknown-unknown)" {32}, object
// CHK-PHASES: 34: input, "{{.*libsycl-itt-compiler-wrappers.o.*}}", object
// CHK-PHASES: 35: clang-offload-unbundler, {34}, object
// CHK-PHASES: 36: offload, " (spir64-unknown-unknown)" {35}, object
// CHK-PHASES: 37: input, "{{.*libsycl-itt-stubs.o.*}}", object
// CHK-PHASES: 38: clang-offload-unbundler, {37}, object
// CHK-PHASES: 39: offload, " (spir64-unknown-unknown)" {38}, object
// CHK-PHASES: 40: linker, {30, 33, 36, 39}, ir, (device-sycl)
// CHK-PHASES: 41: sycl-post-link, {40}, tempfiletable, (device-sycl)
// CHK-PHASES: 42: file-table-tform, {41}, tempfilelist, (device-sycl)
// CHK-PHASES: 43: llvm-spirv, {42}, tempfilelist, (device-sycl)
// CHK-PHASES: 44: file-table-tform, {41, 43}, tempfiletable, (device-sycl)
// CHK-PHASES: 45: clang-offload-wrapper, {44}, object, (device-sycl)
// CHK-PHASES: 46: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {29}, "device-sycl (spir64-unknown-unknown)" {45}, image

/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### --sysroot=%S/Inputs/SYCL --intel -fsycl -fno-sycl-use-footer -fno-sycl-device-lib=all -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s

// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown"{{.*}} "-fsycl-is-device"{{.*}} "-fsycl-int-header=[[SYCLHEADER:.+\.h]]"{{.*}} "-emit-llvm-bc"{{.*}} "-std=c++17"{{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC1:.+\.bc]]"
// CHK-COMMANDS: clang-offload-bundler{{.*}} "-type=o" "-targets=sycl-spir64-unknown-unknown" "-input={{.*}}libsycl-itt-user-wrappers.o" "-output=[[ITTUSERSYCL:.+\.o]]" "-unbundle" "-allow-missing-bundles" "-base-temp-dir={{.*}}"
// CHK-COMMANDS: clang-offload-bundler{{.*}} "-type=o" "-targets=sycl-spir64-unknown-unknown" "-input={{.*}}libsycl-itt-compiler-wrappers.o" "-output=[[ITTCOMPSYCL:.+\.o]]" "-unbundle" "-allow-missing-bundles" "-base-temp-dir={{.*}}"
// CHK-COMMANDS: clang-offload-bundler{{.*}} "-type=o" "-targets=sycl-spir64-unknown-unknown" "-input={{.*}}libsycl-itt-stubs.o" "-output=[[ITTSTUBSYCL:.+\.o]]" "-unbundle" "-allow-missing-bundles" "-base-temp-dir={{.*}}"
// CHK-COMMANDS: llvm-link{{.*}} "-only-needed" "[[SYCLLINKEDBC1]]" "[[ITTUSERSYCL]]" "[[ITTCOMPSYCL]]" "[[ITTSTUBSYCL]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{.*}} "-o" "[[TABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-o" "[[FILELIST:.+\.txt]]" "[[TABLE]]"
// CHK-COMMANDS: llvm-foreach{{.*}} "--in-file-list=[[FILELIST]]"{{.*}}"--out-file-list=[[SPVOUTPUT:[^ ]+\.txt]]"
// CHK-COMMANDS-SAME: llvm-spirv{{.*}} "-o" "[[SPVOUTPUT]]" {{.*}} "[[FILELIST]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-o" "[[TABLEFORWRAPPER:.+\.table]]" "[[TABLE]]" "[[SPVOUTPUT]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl"{{.*}} "[[TABLEFORWRAPPER]]"
// CHK-COMMANDS: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu"{{.*}} "-include" "[[SYCLHEADER]]" "-dependency-filter" "[[SYCLHEADER]]" "-fsycl-enable-int-header-diags" "-fsycl-is-host"{{.*}} "-emit-llvm-bc"{{.*}}  "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-std=c++17"{{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp"{{.*}} "-mllvm" "-paropt=31"{{.*}} "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-std=c++17"{{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]"{{.*}} "-fsycl-is-host" "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-spirvdevicertl.o" "-output=[[RTLTGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-user-wrappers.o" "-output=[[ITT1TGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-compiler-wrappers.o" "-output=[[ITT2TGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-stubs.o" "-output=[[ITT3TGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: llvm-link{{.*}} "[[OMPBC]]" "[[RTLTGTOMP]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-COMMANDS: llvm-link{{.*}} "--only-needed" "[[UNBUNDLED]]" "[[ITT1TGTOMP]]" "[[ITT2TGTOMP]]" "[[ITT3TGTOMP]]" "-o" "[[OMPLINKEDBC:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-device-globals" "-o" "[[OMPPOSTLINK:.+\.bc]]" "[[OMPLINKEDBC]]"
// CHK-COMMANDS: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.spv]]" {{.*}} "-spirv-allow-unknown-intrinsics" {{.*}} "[[OMPPOSTLINK]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-COMMANDS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

/// ###########################################################################

/// Check separate compilation with offloading - bundling actions
// RUN:   %clang -### -ccc-print-phases --intel -fsycl -fno-sycl-use-footer -fno-sycl-device-lib=all -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUACTIONS %s

// CHK-BUACTIONS: 0: input, "[[INPUT:.+\.c]]", c++, (host-sycl)
// CHK-BUACTIONS: 1: preprocessor, {0}, c++-cpp-output, (host-sycl)
// CHK-BUACTIONS: 2: input, "[[INPUT]]", c++, (device-sycl)
// CHK-BUACTIONS: 3: preprocessor, {2}, c++-cpp-output, (device-sycl)
// CHK-BUACTIONS: 4: compiler, {3}, ir, (device-sycl)
// CHK-BUACTIONS: 5: offload, "host-sycl (x86_64-unknown-linux-gnu)" {1}, "device-sycl (spir64-unknown-unknown)" {4}, c++-cpp-output
// CHK-BUACTIONS: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-BUACTIONS: 7: input, "[[INPUT]]", c++, (device-openmp)
// CHK-BUACTIONS: 8: preprocessor, {7}, c++-cpp-output, (device-openmp)
// CHK-BUACTIONS: 9: compiler, {8}, ir, (device-openmp)
// CHK-BUACTIONS: 10: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {6}, "device-openmp (spir64)" {9}, ir
// CHK-BUACTIONS: 11: backend, {10}, ir, (device-openmp)
// CHK-BUACTIONS: 12: offload, "device-openmp (spir64)" {11}, ir
// CHK-BUACTIONS: 13: offload, "device-sycl (spir64-unknown-unknown)" {4}, ir
// CHK-BUACTIONS: 14: backend, {6}, assembler, (host-openmp-sycl)
// CHK-BUACTIONS: 15: assembler, {14}, object, (host-openmp-sycl)
// CHK-BUACTIONS: 16: clang-offload-bundler, {12, 13, 15}, object, (host-openmp-sycl)

/// ###########################################################################

/// Check separate compilation with offloading - unbundling actions
// RUN:   touch %t.o
// RUN:   %clang -### -ccc-print-phases --sysroot=%S/Inputs/SYCL --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %t.o -no-canonical-prefixes -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBACTIONS %s

// CHK-UBACTIONS: 0: input, "[[INPUT:.+\.o]]", object, (host-openmp-sycl)
// CHK-UBACTIONS: 1: clang-offload-unbundler, {0}, object, (host-openmp-sycl)
// CHK-UBACTIONS: 2: spirv-to-ir-wrapper, {1}, ir, (device-openmp)
// CHK-UBACTIONS: 3: input, "{{.*libomp-spirvdevicertl.o.*}}", object
// CHK-UBACTIONS: 4: clang-offload-unbundler, {3}, object
// CHK-UBACTIONS: 5: input, "{{.*libomp-itt-user-wrappers.o.*}}", object
// CHK-UBACTIONS: 6: clang-offload-unbundler, {5}, object
// CHK-UBACTIONS: 7: input, "{{.*libomp-itt-compiler-wrappers.o.*}}", object
// CHK-UBACTIONS: 8: clang-offload-unbundler, {7}, object
// CHK-UBACTIONS: 9: input, "{{.*libomp-itt-stubs.o.*}}", object
// CHK-UBACTIONS: 10: clang-offload-unbundler, {9}, object
// CHK-UBACTIONS: 11: linker, {2, 4, 6, 8, 10}, ir, (device-openmp)
// CHK-UBACTIONS: 12: sycl-post-link, {11}, ir, (device-openmp)
// CHK-UBACTIONS: 13: llvm-spirv, {12}, spirv, (device-openmp)
// CHK-UBACTIONS: 14: offload, "device-openmp (spir64)" {13}, ir
// CHK-UBACTIONS: 15: clang-offload-wrapper, {14}, ir, (host-openmp-sycl)
// CHK-UBACTIONS: 16: backend, {15}, assembler, (host-openmp-sycl)
// CHK-UBACTIONS: 17: assembler, {16}, object, (host-openmp-sycl)
// CHK-UBACTIONS: 18: linker, {1, 17}, image, (host-openmp-sycl)
// CHK-UBACTIONS: 19: spirv-to-ir-wrapper, {1}, ir, (device-sycl)
// CHK-UBACTIONS: 20: linker, {19}, ir, (device-sycl)
// CHK-UBACTIONS: 21: input, "{{.*libsycl-itt-user-wrappers.o.*}}", object
// CHK-UBACTIONS: 22: clang-offload-unbundler, {21}, object
// CHK-UBACTIONS: 23: offload, " (spir64-unknown-unknown)" {22}, object
// CHK-UBACTIONS: 24: input, "{{.*libsycl-itt-compiler-wrappers.o.*}}", object
// CHK-UBACTIONS: 25: clang-offload-unbundler, {24}, object
// CHK-UBACTIONS: 26: offload, " (spir64-unknown-unknown)" {25}, object
// CHK-UBACTIONS: 27: input, "{{.*libsycl-itt-stubs.o.*}}", object
// CHK-UBACTIONS: 28: clang-offload-unbundler, {27}, object
// CHK-UBACTIONS: 29: offload, " (spir64-unknown-unknown)" {28}, object
// CHK-UBACTIONS: 30: linker, {20, 23, 26, 29}, ir, (device-sycl)
// CHK-UBACTIONS: 31: sycl-post-link, {30}, tempfiletable, (device-sycl)
// CHK-UBACTIONS: 32: file-table-tform, {31}, tempfilelist, (device-sycl)
// CHK-UBACTIONS: 33: llvm-spirv, {32}, tempfilelist, (device-sycl)
// CHK-UBACTIONS: 34: file-table-tform, {31, 33}, tempfiletable, (device-sycl)
// CHK-UBACTIONS: 35: clang-offload-wrapper, {34}, object, (device-sycl)
// CHK-UBACTIONS: 36: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {18}, "device-sycl (spir64-unknown-unknown)" {35}, image

/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### --intel -fsycl -fno-sycl-use-footer -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUJOBS -DFATOBJ=%t.o %s

// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown"{{.*}} "-fsycl-is-device" {{.*}} "-fsycl-int-header=[[SYCLHEADER:.+\.h]]"{{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu"{{.*}} "-include" "[[SYCLHEADER]]" "-dependency-filter" "[[SYCLHEADER]]" "-fsycl-enable-int-header-diags" "-fsycl-is-host"{{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp"{{.*}} "-mllvm" "-paropt=31"{{.*}} "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]"{{.*}} "-fsycl-is-host" "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31"{{.*}} "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-BUJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64,sycl-spir64-unknown-unknown,host-x86_64-unknown-linux-gnu" "-output=[[FATOBJ]]" "-input=[[OMPBC]]" "-input=[[SYCLBC]]" "-input=[[HOSTOBJ]]"

/// Check that -paropt is not passed for -fsycl device
// RUN:   %clang -### --intel -fsycl -fiopenmp -c -o %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PAROPT -DFATOBJ=%t.o %s
// CHK-PAROPT-NOT: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown"{{.*}} "-fsycl-is-device" {{.*}} "-mllvm" "-paropt{{.*}}"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.o
// RUN:   %clang -### --sysroot=%S/Inputs/SYCL --intel -fsycl -fno-sycl-device-lib=all -fiopenmp %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -no-canonical-prefixes -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS -DFATOBJ=%t.o %s
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64,sycl-spir64-unknown-unknown" "-input=[[FATOBJ]]" "-output=[[HOSTOBJ:.+\.o]]" "-output=[[OMPBCO:.+\.o]]" "-output=[[SYCLBCO:.+\.o]]" "-unbundle"
// CHK-UBJOBS: spirv-to-ir-wrapper{{.*}} "[[SYCLBCO]]" "-o" "[[SYCLBC:.+\.bc]]"
// CHK-UBJOBS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC1:.+\.bc]]"
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=sycl-spir64-unknown-unknown" "-input={{.*}}libsycl-itt-user-wrappers.o" "-output=[[ITTUSERSYCL:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=sycl-spir64-unknown-unknown" "-input={{.*}}libsycl-itt-compiler-wrappers.o" "-output=[[ITTCOMPSYCL:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=sycl-spir64-unknown-unknown" "-input={{.*}}libsycl-itt-stubs.o" "-output=[[ITTSTUBSYCL:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: llvm-link{{.*}} "-only-needed" "[[SYCLLINKEDBC1]]" "[[ITTUSERSYCL]]" "[[ITTCOMPSYCL]]" "[[ITTSTUBSYCL]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{.*}} "-symbols" "-emit-exported-symbols" "-split-esimd" "-lower-esimd" "-O2" "-spec-const=rt" "-device-globals" "-o" "[[SYCLTABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[SYCLTABLEOUT:.+\.txt]]" "[[SYCLTABLE]]"
// CHK-UBJOBS: llvm-foreach{{.*}} "--in-file-list=[[SYCLTABLEOUT]]" "--in-replace=[[SYCLTABLEOUT]]" "--out-ext=spv" "--out-file-list=[[SYCLLLVMFOROUT:.+\.txt]]" "--out-replace=[[SYCLLLVMFOROUT]]" "--" "{{.*}}llvm-spirv" "-o" "[[SYCLLLVMFOROUT]]" "-spirv-max-version=1.3" "-spirv-debug-info-version=ocl-100" "-spirv-allow-extra-diexpressions" "-spirv-allow-unknown-intrinsics=llvm.genx."{{.*}} "[[SYCLTABLEOUT]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[SYCLSPIRV:.+\.table]]" "[[SYCLTABLE]]" "[[SYCLLLVMFOROUT]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl" "-batch" "[[SYCLSPIRV]]"
// CHK-UBJOBS: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-UBJOBS: spirv-to-ir-wrapper{{.*}} "[[OMPBCO]]" "-o" "[[OMPBC:.+\.bc]]"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-spirvdevicertl.o" "-output=[[RTLTGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-user-wrappers.o" "-output=[[ITT1TGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-compiler-wrappers.o" "-output=[[ITT2TGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-stubs.o" "-output=[[ITT3TGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: llvm-link{{.*}} "[[OMPBC]]" "[[RTLTGTOMP]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-UBJOBS: llvm-link{{.*}} "[[UNBUNDLED]]" "[[ITT1TGTOMP]]" "[[ITT2TGTOMP]]" "[[ITT3TGTOMP]]" "-o" "[[OMPLINKEDBC:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-device-globals" "-o" "[[OMPPOSTLINK:.+\.bc]]" "[[OMPLINKEDBC]]"
// CHK-UBJOBS: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.spv]]" {{.*}} "-spirv-allow-unknown-intrinsics" {{.*}} "[[OMPPOSTLINK]]"
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
// RUN: %clang -target x86_64-unknown-linux-gnu --sysroot=%S/Inputs/SYCL --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -### %t-1.o %t-2.o %t-3.o -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_MULTI_O
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-bundler{{.*}} "-type=aoo" "-targets=openmp-spir64,sycl-spir64-unknown-unknown" "-input=[[INPUTA:.+\.a]]" "-output=[[OMPLIB:.+\.txt]]" "-output=[[SYCLLIB:.+\.txt]]" "-unbundle"
// FOFFLOAD_STATIC_LIB_MULTI_O: spirv-to-ir-wrapper{{.*}} "[[SYCLLIB]]" "-o" "[[SYCLLIBLIST:.+\.txt]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "-o" "[[SYCLLINKEDBCPRE1:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "--only-needed" "[[SYCLLINKEDBCPRE1]]" "@[[SYCLLIBLIST]]" "-o" "[[SYCLLINKEDBCPRE2:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "-only-needed" "[[SYCLLINKEDBCPRE2]]"{{.*}} "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: sycl-post-link{{.*}} "-symbols" "-emit-exported-symbols" "-split-esimd" "-lower-esimd" "-O2" "-spec-const=rt" "-device-globals" "-o" "[[SYCLTABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[SYCLTABLEOUT:.+\.txt]]" "[[SYCLTABLE]]"
// INTEL_CUSTOMIZATION
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-foreach{{.*}} "--in-file-list=[[SYCLTABLEOUT]]" "--in-replace=[[SYCLTABLEOUT]]" "--out-ext=spv" "--out-file-list=[[SYCLLLVMFOROUT:.+\.txt]]" "--out-replace=[[SYCLLLVMFOROUT]]" "--" "{{.*}}llvm-spirv" "-o" "[[SYCLLLVMFOROUT]]" "-spirv-max-version=1.3" "-spirv-debug-info-version=ocl-100" "-spirv-allow-extra-diexpressions" "-spirv-allow-unknown-intrinsics=llvm.genx."{{.*}} "[[SYCLTABLEOUT]]"
// end INTEL_CUSTOMIZATION
// FOFFLOAD_STATIC_LIB_MULTI_O: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[SYCLSPIRV:.+\.table]]" "[[SYCLTABLE]]" "[[SYCLLLVMFOROUT]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64" "-kind=sycl" "-batch" "[[SYCLSPIRV]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-spirvdevicertl.o" "-output=[[RTLTGTOMP:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "-o" "[[LINKOUT2:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "--only-needed" "[[LINKOUT2]]" {{.*}} "-o" "[[LINKOUT1:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-link{{.*}} "--only-needed" "[[LINKOUT1]]" {{.*}} "-o" "[[LINKOUT2:.+\.bc]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: sycl-post-link{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-device-globals" "-o" "[[OMPPOSTLINK:.+\.bc]]" "[[LINKOUT2]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.spv]]" {{.*}} "-spirv-allow-unknown-intrinsics" {{.*}} "[[OMPPOSTLINK]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// FOFFLOAD_STATIC_LIB_MULTI_O: ld{{.*}} "-o" {{.*}} "[[INPUTA]]"{{.*}} "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]" {{.*}} "-liomp5" "-lomptarget" {{.*}} "-lsycl"

/// ###########################################################################

/// Check phases when -foffload-static-lib=<lib> is used.
// RUN: touch %t.a
// RUN: %clang -target x86_64-unknown-linux-gnu --sysroot=%S/Inputs/SYCL --intel -fsycl -fno-sycl-use-footer -fno-sycl-device-lib=all -fiopenmp -fopenmp-targets=spir64 -foffload-static-lib=%t.a -ccc-print-phases %s -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck %s -check-prefix=FOFFLOAD_STATIC_LIB_SRC
// FOFFLOAD_STATIC_LIB_SRC: 0: input, "[[INPUT1:.+\.a]]", object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 1: input, "[[INPUT2:.+\.c]]", c++, (host-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 2: preprocessor, {1}, c++-cpp-output, (host-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 3: input, "[[INPUT2]]", c++, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 4: preprocessor, {3}, c++-cpp-output, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 5: compiler, {4}, ir, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 6: offload, "host-sycl (x86_64-unknown-linux-gnu)" {2}, "device-sycl (spir64-unknown-unknown)" {5}, c++-cpp-output
// FOFFLOAD_STATIC_LIB_SRC: 7: compiler, {6}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 8: backend, {7}, assembler, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 9: assembler, {8}, object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 10: input, "[[INPUT2]]", c++, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 11: preprocessor, {10}, c++-cpp-output, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 12: compiler, {11}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 13: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {7}, "device-openmp (spir64)" {12}, ir
// FOFFLOAD_STATIC_LIB_SRC: 14: backend, {13}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 15: linker, {0, 9}, host_dep_image, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 16: clang-offload-deps, {15}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 17: input, "[[INPUT1]]", archive
// FOFFLOAD_STATIC_LIB_SRC: 18: clang-offload-unbundler, {17}, tempfilelist
// FOFFLOAD_STATIC_LIB_SRC: 19: spirv-to-ir-wrapper, {18}, tempfilelist, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 20: input, "{{.*libomp-spirvdevicertl.o.*}}", object
// FOFFLOAD_STATIC_LIB_SRC: 21: clang-offload-unbundler, {20}, object
// FOFFLOAD_STATIC_LIB_SRC: 22: input, "{{.*libomp-itt-user-wrappers.o.*}}", object
// FOFFLOAD_STATIC_LIB_SRC: 23: clang-offload-unbundler, {22}, object
// FOFFLOAD_STATIC_LIB_SRC: 24: input, "{{.*libomp-itt-compiler-wrappers.o.*}}", object
// FOFFLOAD_STATIC_LIB_SRC: 25: clang-offload-unbundler, {24}, object
// FOFFLOAD_STATIC_LIB_SRC: 26: input, "{{.*libomp-itt-stubs.o.*}}", object
// FOFFLOAD_STATIC_LIB_SRC: 27: clang-offload-unbundler, {26}, object
// FOFFLOAD_STATIC_LIB_SRC: 28: linker, {14, 16, 19, 21, 23, 25, 27}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 29: sycl-post-link, {28}, ir, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 30: llvm-spirv, {29}, spirv, (device-openmp)
// FOFFLOAD_STATIC_LIB_SRC: 31: offload, "device-openmp (spir64)" {30}, ir
// FOFFLOAD_STATIC_LIB_SRC: 32: clang-offload-wrapper, {31}, ir, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 33: backend, {32}, assembler, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 34: assembler, {33}, object, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 35: linker, {0, 9, 34}, image, (host-openmp-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 36: spirv-to-ir-wrapper, {18}, tempfilelist, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 37: linker, {5, 16, 36}, ir, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 38: input, "{{.*libsycl-itt-user-wrappers.o.*}}", object
// FOFFLOAD_STATIC_LIB_SRC: 39: clang-offload-unbundler, {38}, object
// FOFFLOAD_STATIC_LIB_SRC: 40: offload, " (spir64-unknown-unknown)" {39}, object
// FOFFLOAD_STATIC_LIB_SRC: 41: input, "{{.*libsycl-itt-compiler-wrappers.o.*}}", object
// FOFFLOAD_STATIC_LIB_SRC: 42: clang-offload-unbundler, {41}, object
// FOFFLOAD_STATIC_LIB_SRC: 43: offload, " (spir64-unknown-unknown)" {42}, object
// FOFFLOAD_STATIC_LIB_SRC: 44: input, "{{.*libsycl-itt-stubs.o.*}}", object
// FOFFLOAD_STATIC_LIB_SRC: 45: clang-offload-unbundler, {44}, object
// FOFFLOAD_STATIC_LIB_SRC: 46: offload, " (spir64-unknown-unknown)" {45}, object
// FOFFLOAD_STATIC_LIB_SRC: 47: linker, {37, 40, 43, 46}, ir, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 48: sycl-post-link, {47}, tempfiletable, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 49: file-table-tform, {48}, tempfilelist, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 50: llvm-spirv, {49}, tempfilelist, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 51: file-table-tform, {48, 50}, tempfiletable, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 52: clang-offload-wrapper, {51}, object, (device-sycl)
// FOFFLOAD_STATIC_LIB_SRC: 53: offload, "host-openmp-sycl (x86_64-unknown-linux-gnu)" {35}, "device-sycl (spir64-unknown-unknown)" {52}, image

/// ###########################################################################

/// Check -std= is passed to all clang calls when performing interop (OpenMP+SYCL)
// RUN:   %clang_cl -### -fsycl -Qiopenmp -Qopenmp-targets=spir64 /Qstd:c++20 -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-STD %s
// RUN:   %clang_cl -### -fsycl -Qiopenmp -Qopenmp-targets=spir64 /std:c++20 -c %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-STD %s

// CHK-STD-NOT: clang{{.*}} warning: argument unused during compilation: '/std:c++20'
// CHK-STD: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown"{{.*}} "-std=c++20"
// CHK-STD: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}} "-include"{{.*}} "-std=c++20"
// CHK-STD: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-std=c++20"
