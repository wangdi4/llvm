///
/// Perform driver tests for OpenMP offloading with -fiopenmp and target spir64
/// on Windows.
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target

/// ###########################################################################

/// Check the phases graph when using a single target, different from the host.
/// We should have an offload action joining the host compile and device
/// preprocessor and another one joining the device linking outputs to the host
/// action.
// RUN:   %clang -ccc-print-phases -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fno-openmp-device-lib=all %s 2>&1 \
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
// CHK-PHASES: 10: input, "{{.*libomp-spirvdevicertl.o.*}}", object
// CHK-PHASES: 11: clang-offload-unbundler, {10}, object
// CHK-PHASES: 12: input, "{{.*libomp-itt-user-wrappers.o.*}}", object
// CHK-PHASES: 13: clang-offload-unbundler, {12}, object
// CHK-PHASES: 14: input, "{{.*libomp-itt-compiler-wrappers.o.*}}", object
// CHK-PHASES: 15: clang-offload-unbundler, {14}, object
// CHK-PHASES: 16: input, "{{.*libomp-itt-stubs.o.*}}", object
// CHK-PHASES: 17: clang-offload-unbundler, {16}, object
// CHK-PHASES: 18: linker, {9, 11, 13, 15, 17}, ir, (device-openmp)
// CHK-PHASES: 19: sycl-post-link, {18}, ir, (device-openmp)
// CHK-PHASES: 20: llvm-spirv, {19}, spirv, (device-openmp)
// CHK-PHASES: 21: offload, "device-openmp (spir64)" {20}, ir
// CHK-PHASES: 22: clang-offload-wrapper, {21}, ir, (host-openmp)
// CHK-PHASES: 23: backend, {22}, assembler, (host-openmp)
// CHK-PHASES: 24: assembler, {23}, object, (host-openmp)
// CHK-PHASES: 25: linker, {4, 24}, image, (host-openmp)

/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fno-openmp-device-lib=all %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s
// CHK-COMMANDS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTBC:.+\.bc]]"
// CHK-COMMANDS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-obj" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-COMMANDS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-pc-windows-msvc" "-fms-extensions" "-fms-compatibility" "-fdelayed-template-parsing" "-fms-compatibility-version={{.*}}" "-disable-lifetime-markers" "-disable-intel-proprietary-opts" "-Wspir-compat" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp"{{.*}} "-fsycl-instrument-device-code" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]"{{.*}} "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" "-o" "[[TGTBC:.+\.bc]]"
// CHK-COMMANDS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-spirvdevicertl.obj" "-output=[[RTLTGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-user-wrappers.obj" "-output=[[ITT1TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-compiler-wrappers.obj" "-output=[[ITT2TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-stubs.obj" "-output=[[ITT3TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-COMMANDS: llvm-link{{.*}} "[[TGTBC]]" "[[RTLTGT]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-COMMANDS: llvm-link{{(.exe)?}}{{.*}} "[[UNBUNDLED]]" "[[ITT1TGT]]" "[[ITT2TGT]]" "[[ITT3TGT]]" "-o" "[[TGTLINKEDBC:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{(.exe)?}}{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2" "-spec-const=rt" "-o" "[[TGTPOSTLINK:.+\.bc]]" "[[TGTLINKEDBC]]"
// CHK-COMMANDS: llvm-spirv{{(.exe)?}}{{.*}}" "-o" "[[TGTSPIRV:.+\.spv]]" {{.*}} "-spirv-allow-unknown-intrinsics" {{.*}} "[[TGTPOSTLINK]]"
// CHK-COMMANDS: clang-offload-wrapper{{(.exe)?}}{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[WRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[TGTSPIRV]]"
// CHK-COMMANDS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-obj" {{.*}} "-o" "[[WRAPPEROBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-COMMANDS: link{{(.exe)?}}{{.*}} "-out:{{.*}}"{{.*}} "-defaultlib:libiomp5md.lib" "-defaultlib:omptarget.lib"{{.*}} "[[HOSTOBJ]]" "[[WRAPPEROBJ]]"

/// ###########################################################################

/// Check separate compilation with offloading - bundling actions
// RUN:   %clang -### -ccc-print-phases -fiopenmp -c -o %t.obj -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
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

/// ###########################################################################

/// Check separate compilation with offloading - unbundling actions
// RUN:   touch %t.obj
// RUN:   %clang -### -ccc-print-phases -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -fno-openmp-device-lib=all %t.obj 2>&1 \
// RUN:   | FileCheck -DINPUT=%t.obj -check-prefix=CHK-UBACTIONS %s
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
// CHK-UBACTIONS: 12: sycl-post-link, {11}, ir, (device-openmp)
// CHK-UBACTIONS: 13: llvm-spirv, {12}, spirv, (device-openmp)
// CHK-UBACTIONS: 14: offload, "device-openmp (spir64)" {13}, ir
// CHK-UBACTIONS: 15: clang-offload-wrapper, {14}, ir, (host-openmp)
// CHK-UBACTIONS: 16: backend, {15}, assembler, (host-openmp)
// CHK-UBACTIONS: 17: assembler, {16}, object, (host-openmp)
// CHK-UBACTIONS: 18: linker, {1, 17}, image, (host-openmp)

/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### -fiopenmp -c -o %t.obj -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUJOBS %s
// CHK-BUJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTBC:.+\.bc]]"
// CHK-BUJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-pc-windows-msvc" "-fms-extensions" "-fms-compatibility" "-fdelayed-template-parsing" "-fms-compatibility-version={{.*}}" "-disable-lifetime-markers" "-disable-intel-proprietary-opts" "-Wspir-compat" "-emit-llvm-bc"{{.*}} "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl"{{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]"{{.*}} "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" "-o" "[[TGTBC:.+\.bc]]"
// CHK-BUJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-obj" {{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-BUJOBS: clang-offload-bundler{{(.exe)?}}{{.*}} "-type=o" "-targets=openmp-spir64,host-x86_64-pc-windows-msvc" "-output=[[FATOBJ:.+\.obj]]" "-input=[[TGTBC]]" "-input=[[HOSTOBJ]]"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.obj
// RUN:   %clang -### -fiopenmp %t.obj -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 --intel -fno-openmp-device-lib=all 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS %s
// CHK-UBJOBS: clang-offload-bundler{{(.exe)?}}{{.*}} "-type=o" "-targets=host-x86_64-pc-windows-msvc,openmp-spir64" "-input=[[FATOBJ:.+\.obj]]" "-output=[[HOSTOBJ:.+\.o]]" "-output=[[TGTOBJ:.+\.o]]" "-unbundle"
// CHK-UBJOBS: spirv-to-ir-wrapper{{.*}} "[[TGTOBJ]]" "-o" "[[TGTBC:.+\.bc]]"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-spirvdevicertl.obj" "-output=[[RTLTGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-user-wrappers.obj" "-output=[[ITT1TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-compiler-wrappers.obj" "-output=[[ITT2TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: "{{.*}}clang-offload-bundler" "-type=o" "-targets=openmp-spir64" "-input={{.*}}libomp-itt-stubs.obj" "-output=[[ITT3TGT:.+\.o]]" "-unbundle" "-allow-missing-bundles"
// CHK-UBJOBS: llvm-link{{.*}} "[[TGTBC]]" "[[RTLTGT]]" "-o" "[[UNBUNDLED:.+\.bc]]"
// CHK-UBJOBS: llvm-link{{(.exe)?}}{{.*}} "[[UNBUNDLED]]" "[[ITT1TGT]]" "[[ITT2TGT]]" "[[ITT3TGT]]" "-o" "[[TGTLINKEDBC:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{(.exe)?}}{{.*}} "--ompoffload-link-entries" "--ompoffload-sort-entries" "--ompoffload-make-globals-static" "-ir-output-only" "-O2"  "-spec-const=rt" "-o" "[[TGTPOSTLINK:.+\.bc]]" "[[TGTLINKEDBC]]"
// CHK-UBJOBS: llvm-spirv{{(.exe)?}}{{.*}} "-o" "[[TGTSPIRV:.+\.spv]]" {{.*}} "-spirv-allow-unknown-intrinsics" {{.*}} "[[TGTPOSTLINK]]"
// CHK-UBJOBS: clang-offload-wrapper{{(.exe)?}}{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[WRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[TGTSPIRV]]"
// CHK-UBJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-obj" {{.*}} "-o" "[[WRAPPEROBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-UBJOBS: link{{(.exe)?}}{{.*}} "-out:{{.+}}"{{.*}} "-defaultlib:libiomp5md.lib" "-defaultlib:omptarget.lib"{{.*}} "[[HOSTOBJ]]" "[[WRAPPEROBJ]]"

/// Use of /LD not supported
// RUN: %clang_cl -### /LD /Qiopenmp /Qopenmp-targets=spir64 -c %s 2>&1 \
// RUN:  FileCheck -check-prefix=LD_ERROR %s
// LD_ERROR: error: The use of '/LD' is not supported with '/Qiopenmp /Qopenmp-targets=spir64'.

/// test llvm-link behavior for fopenmp offload
// RUN: %clang -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefix=OPENMP_LLVM_LINK_DEVICE_LIB
// OPENMP_LLVM_LINK_DEVICE_LIB: llvm-link{{.*}} "--only-needed" "{{.*}}"

/// Check spirv-opt
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -O3 -fopenmp-targets=spir64 -fno-openmp-device-lib=all  %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-SPIRV-OPT %s
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -O2 -fopenmp-targets=spir64 -fno-openmp-device-lib=all  %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-SPIRV-OPT %s
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -O1 -fopenmp-targets=spir64 -fno-openmp-device-lib=all  %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-SPIRV-OPT %s
// CHK-SPIRV-OPT: clang{{.*}} "-triple" "spir64" "-aux-triple" "x86_64-pc-windows-msvc"{{.*}} "-mllvm" "-spirv-opt"
