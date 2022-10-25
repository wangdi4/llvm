///
/// Perform driver tests for OpenMP offloading with -fiopenmp
/// -fopenmp-targets=<triple> -fopenmp-device-code-split
///

/// ###########################################################################

/// Check the phases graph when using a single target, different from the host.
/// We should have an offload action joining the host compile and device
/// preprocessor and another one joining the device linking outputs to the host
/// action.
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fopenmp-device-code-split=per_kernel -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.cpp]]", c++, (host-openmp)
// CHK-PHASES: 1: preprocessor, {0}, c++-cpp-output, (host-openmp)
// CHK-PHASES: 2: compiler, {1}, ir, (host-openmp)
// CHK-PHASES: 3: backend, {2}, assembler, (host-openmp)
// CHK-PHASES: 4: assembler, {3}, object, (host-openmp)
// CHK-PHASES: 5: input, "[[INPUT]]", c++, (device-openmp)
// CHK-PHASES: 6: preprocessor, {5}, c++-cpp-output, (device-openmp)
// CHK-PHASES: 7: compiler, {6}, ir, (device-openmp)
// CHK-PHASES: 8: offload, "host-openmp (x86_64-unknown-linux-gnu)" {2}, "device-openmp (spir64)" {7}, ir
// CHK-PHASES: 9: backend, {8}, ir, (device-openmp)
// CHK-PHASES: 10: input, "{{.*libomp-spirvdevicertl.o.*}}", object
// CHK-PHASES: 11: clang-offload-unbundler, {10}, object
// CHK-PHASES: 12: input, "{{.*libomp-itt-user-wrappers.o.*}}", object
// CHK-PHASES: 13: clang-offload-unbundler, {12}, object
// CHK-PHASES: 14: input, "{{.*libomp-itt-compiler-wrappers.o.*}}", object
// CHK-PHASES: 15: clang-offload-unbundler, {14}, object
// CHK-PHASES: 16: input, "{{.*libomp-itt-stubs.o.*}}", object
// CHK-PHASES: 17: clang-offload-unbundler, {16}, object
// CHK-PHASES: 18: linker, {9, 11, 13, 15, 17}, ir, (device-openmp)
// CHK-PHASES: 19: sycl-post-link, {18}, tempfiletable, (device-openmp)
// CHK-PHASES: 20: file-table-tform, {19}, tempfilelist, (device-openmp)
// CHK-PHASES: 21: llvm-spirv, {20}, tempfilelist, (device-openmp)
// CHK-PHASES: 22: file-table-tform, {19, 21}, tempfiletable, (device-openmp)
// CHK-PHASES: 23: offload, "device-openmp (spir64)" {22}, ir
// CHK-PHASES: 24: clang-offload-wrapper, {23}, ir, (host-openmp)
// CHK-PHASES: 25: backend, {24}, assembler, (host-openmp)
// CHK-PHASES: 26: assembler, {25}, object, (host-openmp)
// CHK-PHASES: 27: linker, {4, 26}, image, (host-openmp)

/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fopenmp-device-code-split=per_kernel -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp" {{.*}} "-o" "[[BCFILE:.+\.bc]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[BCFILE]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-unknown-linux-gnu" "-disable-lifetime-markers" "-disable-intel-proprietary-opts" "-Wspir-compat" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp"{{.*}} "-fsycl-instrument-device-code" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[BCFILE]]"{{.*}} "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" "-o" "[[OFFBCFILE:.+\.bc]]"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-spirvdevicertl.o" "-output=[[RTLTGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-user-wrappers.o" "-output=[[ITT1TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-compiler-wrappers.o" "-output=[[ITT2TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-stubs.o" "-output=[[ITT3TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: llvm-link{{.*}} "[[OFFBCFILE]]" "[[RTLTGT]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-COMMANDS: llvm-link{{.*}} "--only-needed" "[[UNBUNDLED]]" "[[ITT1TGT]]" "[[ITT2TGT]]" "[[ITT3TGT]]" "-o" "[[LINKEDBCFILE:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{.*}} "-split=kernel" "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-symbols" "-emit-exported-symbols" "-split-esimd" "-lower-esimd" "-O2" "-spec-const=rt" "-device-globals" "-o" "[[POSTLINKFILE:.+\.table]]" "[[LINKEDBCFILE]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[TABLEOUT:.+\.txt]]" "[[POSTLINKFILE]]"
// CHK-COMMANDS: llvm-foreach{{.*}} "--in-file-list=[[TABLEOUT]]" "--in-replace=[[TABLEOUT]]" "--out-ext=spv" "--out-file-list=[[OUTFILESPV:.+\.txt]]" "--out-replace=[[OUTFILESPV]]" "--out-dir={{.*}}" "--"
// CHK-COMMANDS-SAME: llvm-spirv{{.*}}" "-o" "[[OUTFILESPV]]" {{.*}} "[[TABLEOUT]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[TABLEOUT2:.+\.table]]" "[[POSTLINKFILE]]" "[[OUTFILESPV]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[WRAPPERBC:.+\.bc]]" "-batch" "-kind=openmp" "-target=spir64" "[[TABLEOUT2]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[TARGOBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-COMMANDS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[TARGOBJ]]" {{.*}} "-lomptarget"

/// ###########################################################################

/// Check separate compilation with offloading - unbundling actions
// RUN:   touch %t.o
// RUN:   %clang -### -ccc-print-phases -fiopenmp -o %t.out -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 %t.o -fopenmp-device-code-split=per_kernel -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -DINPUT=%t.o -check-prefix=CHK-UBACTIONS %s
// CHK-UBACTIONS: 0: input, "[[INPUT]]", object, (host-openmp)
// CHK-UBACTIONS: 1: clang-offload-unbundler, {0}, object, (host-openmp)
// CHK-UBACTIONS: 2: spirv-to-ir-wrapper, {1}, ir, (device-openmp)
// CHK-UBACTIONS: 3: input, {{.*libomp-spirvdevicertl.o.*}}, object
// CHK-UBACTIONS: 4: clang-offload-unbundler, {3}, object
// CHK-UBACTIONS: 5: input, {{.*libomp-itt-user-wrappers.o.*}}, object
// CHK-UBACTIONS: 6: clang-offload-unbundler, {5}, object
// CHK-UBACTIONS: 7: input, {{.*libomp-itt-compiler-wrappers.o.*}}, object
// CHK-UBACTIONS: 8: clang-offload-unbundler, {7}, object
// CHK-UBACTIONS: 9: input, {{.*libomp-itt-stubs.o.*}}, object
// CHK-UBACTIONS: 10: clang-offload-unbundler, {9}, object
// CHK-UBACTIONS: 11: linker, {2, 4, 6, 8, 10}, ir, (device-openmp)
// CHK-UBACTIONS: 12: sycl-post-link, {11}, tempfiletable, (device-openmp)
// CHK-UBACTIONS: 13: file-table-tform, {12}, tempfilelist, (device-openmp)
// CHK-UBACTIONS: 14: llvm-spirv, {13}, tempfilelist, (device-openmp)
// CHK-UBACTIONS: 15: file-table-tform, {12, 14}, tempfiletable, (device-openmp)
// CHK-UBACTIONS: 16: offload, "device-openmp (spir64)" {15}, ir
// CHK-UBACTIONS: 17: clang-offload-wrapper, {16}, ir, (host-openmp)
// CHK-UBACTIONS: 18: backend, {17}, assembler, (host-openmp)
// CHK-UBACTIONS: 19: assembler, {18}, object, (host-openmp)
// CHK-UBACTIONS: 20: linker, {1, 19}, image, (host-openmp)

/// ###########################################################################

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.o
// RUN:   %clang -### -fiopenmp %t.o -target x86_64-unknown-linux-gnu -fopenmp-targets=spir64 -fopenmp-device-code-split=per_kernel -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS %s
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-unknown-linux-gnu,openmp-spir64" "-input={{.*}}" "-output=[[HOSTOBJ:.+\.o]]" "-output=[[OFFOBJ:.+\.o]]" "-unbundle"
// CHK-UBJOBS: spirv-to-ir-wrapper{{.*}} "[[OFFOBJ]]" "-o" "[[OFFBCFILE:.+\.bc]]" "-skip-unknown-input"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-spirvdevicertl.o" "-output=[[RTLTGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: clang-offload-bundler{{.*}}"-type=o"{{.*}}"-input={{.*}}libomp-itt-user-wrappers.o"{{.*}}"-output=[[ITT1TGT:.+\.o]]" "-unbundle"
// CHK-UBJOBS: clang-offload-bundler{{.*}}"-type=o"{{.*}}"-input={{.*}}libomp-itt-compiler-wrappers.o"{{.*}}"-output=[[ITT2TGT:.+\.o]]" "-unbundle"
// CHK-UBJOBS: clang-offload-bundler{{.*}}"-type=o"{{.*}}"-input={{.*}}libomp-itt-stubs.o"{{.*}}"-output=[[ITT3TGT:.+\.o]]" "-unbundle"
// CHK-UBJOBS: llvm-link{{.*}} "[[OFFBCFILE]]" "[[RTLTGT]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-UBJOBS: llvm-link{{.*}} "--only-needed" "[[UNBUNDLED]]" "[[ITT1TGT]]" "[[ITT2TGT]]" "[[ITT3TGT]]" "-o" "[[LINKEDBCFILE:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{.*}} "-split=kernel" "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-symbols" "-emit-exported-symbols" "-split-esimd" "-lower-esimd" "-O2" "-spec-const=rt" "-device-globals" "-o" "[[POSTLINKFILE:.+\.table]]" "[[LINKEDBCFILE]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[TABLEOUT:.+\.txt]]" "[[POSTLINKFILE]]"
// CHK-UBJOBS: llvm-foreach{{.*}} "--in-file-list=[[TABLEOUT]]" "--in-replace=[[TABLEOUT]]" "--out-ext=spv" "--out-file-list=[[OUTFILESPV:.+\.txt]]" "--out-replace=[[OUTFILESPV]]" "--out-dir={{.*}}" "--"
// CHK-UBJOBS-SAME: llvm-spirv{{.*}}" "-o" "[[OUTFILESPV]]" {{.*}} "[[TABLEOUT]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[TABLEOUT2:.+\.table]]" "[[POSTLINKFILE]]" "[[OUTFILESPV]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[WRAPPERBC:.+\.bc]]" "-batch" "-kind=openmp" "-target=spir64" "[[TABLEOUT2]]"
// CHK-UBJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-unknown-linux-gnu" "-emit-obj" {{.*}} "-o" "[[TARGOBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-UBJOBS: ld{{.*}} "-o" {{.*}} "[[HOSTOBJ]]" "[[TARGOBJ]]" {{.*}} "-lomptarget"
