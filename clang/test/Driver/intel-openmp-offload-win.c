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
// CHK-PHASES: 10: linker, {9}, image, (device-openmp)
// CHK-PHASES: 11: llvm-spirv, {10}, image, (device-openmp)
// CHK-PHASES: 12: offload, "device-openmp (spir64)" {11}, image
// CHK-PHASES: 13: clang-offload-wrapper, {12}, ir, (host-openmp)
// CHK-PHASES: 14: backend, {13}, assembler, (host-openmp)
// CHK-PHASES: 15: assembler, {14}, object, (host-openmp)
// CHK-PHASES: 16: linker, {4, 15}, image, (host-openmp)


/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-COMMANDS %s
// CHK-COMMANDS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTBC:.+\.bc]]"
// CHK-COMMANDS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-obj" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"     
// CHK-COMMANDS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-pc-windows-msvc" "-fms-extensions" "-fms-compatibility" "-fdelayed-template-parsing" "-fms-compatibility-version={{.*}}" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" "-o" "[[TGTBC:.+\.bc]]"
// CHK-COMMANDS: llvm-link{{(.exe)?}}{{.*}} "[[TGTBC]]" "-o" "[[TGTLINKEDBC:.+\.out]]"
// CHK-COMMANDS: llvm-spirv{{(.exe)?}}{{.*}}" "-o" "[[TGTSPIRV:.+\.out]]" "[[TGTLINKEDBC]]"
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
// RUN:   %clang -### -ccc-print-phases -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %t.obj 2>&1 \
// RUN:   | FileCheck -DINPUT=%t.obj -check-prefix=CHK-UBACTIONS %s
// CHK-UBACTIONS: 0: input, "[[INPUT]]", object, (host-openmp)
// CHK-UBACTIONS: 1: clang-offload-unbundler, {0}, object, (host-openmp)
// CHK-UBACTIONS: 2: linker, {1}, image, (device-openmp)
// CHK_UBACTIONS: 3: offload, "device-openmp (spir64)" {2}, image
// CHK_UBACTIONS: 4: clang-offload-wrapper, {3}, ir, (host-openmp)
// CHK_UBACTIONS: 5: backend, {4}, assembler, (host-openmp)
// CHK_UBACTIONS: 6: assembler, {5}, object, (host-openmp)
// CHK_UBACTIONS: 7: linker, {1, 6}, image, (host-openmp)

/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### -fiopenmp -c -o %t.obj -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUJOBS %s
// CHK-BUJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTBC:.+\.bc]]"
// CHK-BUJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "spir64" "-aux-triple" "x86_64-pc-windows-msvc" "-fms-extensions" "-fms-compatibility" "-fdelayed-template-parsing" "-fms-compatibility-version={{.*}}" "-emit-llvm-bc" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-mllvm" "-paropt=63" "-fopenmp-targets=spir64" "-o" "[[TGTBC:.+\.bc]]"
// CHK-BUJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-obj" {{.*}} "-fopenmp" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-BUJOBS: clang-offload-bundler{{(.exe)?}}{{.*}} "-type=o" "-targets=openmp-spir64,host-x86_64-pc-windows-msvc" "-outputs=[[FATOBJ:.+\.obj]]" "-inputs=[[TGTBC]],[[HOSTOBJ]]"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.obj
// RUN:   %clang -### -fiopenmp %t.obj -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 --intel 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS %s
// CHK-UBJOBS: clang-offload-bundler{{(.exe)?}}{{.*}} "-type=o" "-targets=host-x86_64-pc-windows-msvc,openmp-spir64" "-inputs=[[FATOBJ:.+\.obj]]" "-outputs=[[HOSTOBJ:.+\.o]],[[TGTBC:.+\.o]]" "-unbundle"
// CHK-UBJOBS: llvm-link{{(.exe)?}}{{.*}} "[[TGTBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[TGTLINKEDBC:.+\.out]]"
// CHK-UBJOBS: llvm-spirv{{(.exe)?}}{{.*}} "-o" "[[TGTSPIRV:.+\.out]]" "[[TGTLINKEDBC]]"
// CHK-UBJOBS: clang-offload-wrapper{{(.exe)?}}{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[WRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[TGTSPIRV]]"
// CHK-UBJOBS: clang{{(.exe)?}}{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-emit-obj" {{.*}} "-o" "[[WRAPPEROBJ:.+\.o]]" "-x" "ir" "[[WRAPPERBC]]"
// CHK-UBJOBS: link{{(.exe)?}}{{.*}} "-out:{{.+}}"{{.*}} "-defaultlib:libiomp5md.lib" "-defaultlib:omptarget.lib"{{.*}} "[[HOSTOBJ]]" "[[WRAPPEROBJ]]"
