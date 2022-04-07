///
/// Driver tests for SYCL + OpenMP offloading with -fiopenmp and target spir64
/// on Windows.
///

// REQUIRES: clang-driver
// REQUIRES: x86-registered-target
// REQUIRES: system-windows
#if INTEL_CUSTOMIZATION
// Temporaily disable the test
// JIRA:CMPLRLLVM-20771
// UNSUPPORTED: system-windows
#endif // INTEL_CUSTOMIZATION

/// ###########################################################################

/// Check phases graph when using single source file.
// RUN:   %clang -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s
// RUN:   %clang_cl -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -Qiopenmp --target=x86_64-pc-windows-msvc -Qopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-PHASES %s

// CHK-PHASES: 0: input, "[[INPUT:.+\.c]]", c, (host-sycl)
// CHK-PHASES: 1: preprocessor, {0}, cpp-output, (host-sycl)
// CHK-PHASES: 2: input, "[[INPUT]]", c, (device-sycl)
// CHK-PHASES: 3: preprocessor, {2}, cpp-output, (device-sycl)
// CHK-PHASES: 4: compiler, {3}, sycl-header, (device-sycl)
// CHK-PHASES: 5: offload, "host-sycl (x86_64-pc-windows-msvc)" {1}, "device-sycl (spir64-unknown-unknown-sycldevice{{(-coff)?}})" {4}, cpp-output
// CHK-PHASES: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-PHASES: 7: backend, {6}, assembler, (host-openmp-sycl)
// CHK-PHASES: 8: assembler, {7}, object, (host-openmp-sycl)
// CHK-PHASES: 9: input, "[[INPUT]]", c, (device-openmp)
// CHK-PHASES: 10: preprocessor, {9}, cpp-output, (device-openmp)
// CHK-PHASES: 11: compiler, {10}, ir, (device-openmp)
// CHK-PHASES: 12: offload, "host-openmp-sycl (x86_64-pc-windows-msvc)" {6}, "device-openmp (spir64)" {11}, ir
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
// CHK-PHASES: 23: sycl-post-link, {22}, tempfiletable, (device-sycl)
// CHK-PHASES: 24: file-table-tform, {23}, tempfilelist, (device-sycl)
// CHK-PHASES: 25: llvm-spirv, {24}, tempfilelist, (device-sycl)
// CHK-PHASES: 26: file-table-tform, {23, 25}, tempfiletable, (device-sycl)
// CHK-PHASES: 27: clang-offload-wrapper, {26}, object, (device-sycl)
// CHK-PHASES: 28: offload, "host-openmp-sycl (x86_64-pc-windows-msvc)" {20}, "device-sycl (spir64-unknown-unknown-sycldevice{{(-coff)?}})" {27}, image

/// ###########################################################################

/// Check of the commands passed to each tool when using valid OpenMP targets.
// RUN:   %clang -### --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefixes=CHK-COMMANDS,CHK-COMMANDS-NOCL %s
// RUN:   %clang_cl -### --intel -fsycl -fno-sycl-device-lib=all -Qiopenmp -o %t.out --target=x86_64-pc-windows-msvc -Qopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefixes=CHK-COMMANDS,CHK-COMMANDS-CL %s

// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice{{(-coff)?}}" "-fsycl-is-device" {{.*}} "-emit-llvm-bc" {{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-COMMANDS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-COMMANDS: sycl-post-link{{.*}} "-o" "[[TABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-o" "[[FILELIST:.+\.txt]]" "[[TABLE]]"
// CHK-COMMANDS: llvm-foreach{{.*}} "--in-file-list=[[FILELIST]]"{{.*}}"--out-file-list=[[SPVOUTPUT:[^ ]+\.txt]]"
// CHK-COMMANDS-SAME: llvm-spirv{{.*}} "-o" "[[SPVOUTPUT]]" {{.*}} "[[FILELIST]]"
// CHK-COMMANDS: file-table-tform{{.*}} "-o" "[[TABLEFORWRAPPER:.+\.table]]" "[[TABLE]]" "[[SPVOUTPUT]]"
// CHK-COMMANDS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-pc-windows-msvc" "-target=spir64" "-kind=sycl"{{.*}} "[[TABLEFORWRAPPER]]"
// CHK-COMMANDS-NOCL: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-COMMANDS-CL: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJCL:.+\.obj]]" "[[SYCLWRAPPERBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice{{.*}}" "-fsycl-is-device" {{.*}} "-fsycl-int-header={{[a-zA-Z]:}}[[SYCLHEADER:.+\.h]]" "-mllvm" "-paropt=31" {{.*}} "-o" "{{.*}}[[SYCLHEADER]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-include" "{{.*}}[[SYCLHEADER]]" "-dependency-filter" "{{.*}}[[SYCLHEADER]]" "-fsycl-is-host" "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown-sycldevice{{.*}}" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS-NOCL: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-COMMANDS-CL: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJCL:.+\.obj]]" "-x" "ir" "[[HOSTBC]]"
// CHK-COMMANDS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-fsycl-is-host" "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-COMMANDS-NOCL: llvm-link{{.*}} "[[OMPBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBC:.+\.out]]"
// CHK-COMMANDS-NOCL: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.out]]" "[[OMPLINKEDBC]]"
// CHK-COMMANDS-NOCL: clang-offload-wrapper{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// CHK-COMMANDS-NOCL: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-COMMANDS-NOCL: link{{.*}} "-out:{{.*}}" {{.*}} "-defaultlib:sycl.lib" {{.*}} "-defaultlib:libiomp5md.lib" "-defaultlib:omptarget.lib" "[[HOSTOBJ]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]"
// CHK-COMMANDS-CL: llvm-link{{.*}} "[[OMPBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBCCL:.+\.exe]]"
// CHK-COMMANDS-CL: llvm-spirv{{.*}} "-o" "[[OMPSPIRVCL:.+\.exe]]" "[[OMPLINKEDBCCL]]"
// CHK-COMMANDS-CL: clang-offload-wrapper{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRVCL]]"
// CHK-COMMANDS-CL: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-o" "[[OMPWRAPPEROBJCL:.+\.obj]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-COMMANDS-CL: link{{.*}} "-out:{{.*}}" "-defaultlib:sycl.lib" {{.*}} "-defaultlib:libiomp5md.lib" "-defaultlib:omptarget.lib" "[[HOSTOBJCL]]" "[[OMPWRAPPEROBJCL]]" "[[SYCLWRAPPEROBJCL]]"

/// ###########################################################################

/// Check separate compilation with offloading - bundling actions
// RUN:   %clang -### -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -c -o %t.o -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUACTIONS %s
// RUN:   %clang_cl -### -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -Qiopenmp -c -Fo%t.o --target=x86_64-pc-windows-msvc -Qopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-BUACTIONS %s

// CHK-BUACTIONS: 0: input, "[[INPUT:.+\.c]]", c, (host-sycl)
// CHK-BUACTIONS: 1: preprocessor, {0}, cpp-output, (host-sycl)
// CHK-BUACTIONS: 2: input, "[[INPUT]]", c, (device-sycl)
// CHK-BUACTIONS: 3: preprocessor, {2}, cpp-output, (device-sycl)
// CHK-BUACTIONS: 4: compiler, {3}, sycl-header, (device-sycl)
// CHK-BUACTIONS: 5: offload, "host-sycl (x86_64-pc-windows-msvc)" {1}, "device-sycl (spir64-unknown-unknown-sycldevice{{(-coff)?}})" {4}, cpp-output
// CHK-BUACTIONS: 6: compiler, {5}, ir, (host-openmp-sycl)
// CHK-BUACTIONS: 7: input, "[[INPUT]]", c, (device-openmp)
// CHK-BUACTIONS: 8: preprocessor, {7}, cpp-output, (device-openmp)
// CHK-BUACTIONS: 9: compiler, {8}, ir, (device-openmp)
// CHK-BUACTIONS: 10: offload, "host-openmp-sycl (x86_64-pc-windows-msvc)" {6}, "device-openmp (spir64)" {9}, ir
// CHK-BUACTIONS: 11: backend, {10}, ir, (device-openmp)
// CHK-BUACTIONS: 12: offload, "device-openmp (spir64)" {11}, ir
// CHK-BUACTIONS: 13: compiler, {3}, ir, (device-sycl)
// CHK-BUACTIONS: 14: offload, "device-sycl (spir64-unknown-unknown-sycldevice{{(-coff)?}})" {13}, ir
// CHK-BUACTIONS: 15: backend, {6}, assembler, (host-openmp-sycl)
// CHK-BUACTIONS: 16: assembler, {15}, object, (host-openmp-sycl)
// CHK-BUACTIONS: 17: clang-offload-bundler, {12, 14, 16}, object, (host-openmp-sycl)

/// ###########################################################################

/// Check separate compilation with offloading - unbundling actions
// RUN:   touch %t.o
// RUN:   %clang -### -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -o %t.out -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %t.o -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBACTIONS %s
// RUN:   %clang_cl -### -ccc-print-phases --intel -fsycl -fno-sycl-device-lib=all -Qiopenmp -o %t.out --target=x86_64-pc-windows-msvc -Qopenmp-targets=spir64 %t.o 2>&1 \
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
// CHK-UBACTIONS: 10: sycl-post-link, {9}, tempfiletable, (device-sycl)
// CHK-UBACTIONS: 11: file-table-tform, {10}, tempfilelist, (device-sycl)
// CHK-UBACTIONS: 12: llvm-spirv, {11}, tempfilelist, (device-sycl)
// CHK-UBACTIONS: 13: file-table-tform, {10, 12}, tempfiletable, (device-sycl)
// CHK-UBACTIONS: 14: clang-offload-wrapper, {13}, object, (device-sycl)
// CHK-UBACTIONS: 15: offload, "host-openmp-sycl (x86_64-pc-windows-msvc)" {8}, "device-sycl (spir64-unknown-unknown-sycldevice{{(-coff)?}})" {14}, image

/// ###########################################################################

/// Check separate compilation with offloading - bundling jobs construct
// RUN:   %clang -### --intel -fsycl -fno-sycl-device-lib=all -fiopenmp -c -o %t.o -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 %s -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefixes=CHK-BUJOBS,CHK-BUJOBS-NOCL %s
// RUN:   %clang_cl -### --intel -fsycl -fno-sycl-device-lib=all -Qiopenmp -c -Fo%t.o --target=x86_64-pc-windows-msvc -Qopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefixes=CHK-BUJOBS,CHK-BUJOBS-CL %s

// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice{{.*}}" "-fsycl-is-device" {{.*}} "-fsycl-int-header={{[a-zA-Z]:}}[[SYCLHEADER:.+\.h]]" "-mllvm" "-paropt=31" {{.*}} "-o" "{{.*}}[[SYCLHEADER]]" {{.*}} "[[INPUT:.+\.c]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-include" "{{.*}}[[SYCLHEADER]]" "-dependency-filter" "{{.*}}[[SYCLHEADER]]" "-fsycl-is-host" "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" "-fsycl-targets=spir64-unknown-unknown-sycldevice{{.*}}" {{.*}} "-o" "[[HOSTBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64" {{.*}} "-emit-llvm-bc" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-fopenmp-is-device" "-fopenmp-host-ir-file-path" "[[HOSTBC]]" "-internal-isystem" "{{.*}}bin{{[/\\]+}}..{{[/\\]+}}include{{[/\\]+}}sycl" "-fsycl-is-host" "-mllvm" "-paropt=63"{{.*}} "-fopenmp-targets=spir64" {{.*}} "-o" "[[OMPBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS: clang{{.*}} "-cc1" "-triple" "spir64-unknown-unknown-sycldevice{{.*}}" "-fsycl-is-device" {{.*}} "-emit-llvm-bc" {{.*}} "-o" "[[SYCLBC:.+\.bc]]" {{.*}} "[[INPUT]]"
// CHK-BUJOBS-NOCL: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJ:.+\.o]]" "-x" "ir" "[[HOSTBC]]"
// CHK-BUJOBS-NOCL: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64,sycl-spir64-unknown-unknown-sycldevice,host-x86_64-pc-windows-msvc" "-output=[[FATOBJ:.+\.o]]" "-input=[[OMPBC]]" "-input=[[SYCLBC]]" "-input=[[HOSTOBJ]]"
// CHK-BUJOBS-CL: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-emit-obj" {{.*}} "-fopenmp-late-outline" {{.*}} "-fopenmp" {{.*}} "-mllvm" "-paropt=31" "-fopenmp-targets=spir64" {{.*}} "-o" "[[HOSTOBJCL:.+\.obj]]" "-x" "ir" "[[HOSTBC]]"
// CHK-BUJOBS-CL: clang-offload-bundler{{.*}} "-type=o" "-targets=openmp-spir64,sycl-spir64-unknown-unknown-sycldevice{{(-coff)?}},host-x86_64-pc-windows-msvc" "-output=[[FATOBJ:.+\.o]]" "-input=[[OMPBC]]" "-input=[[SYCLBC]]" "-input=[[HOSTOBJCL]]"

/// Check separate compilation with offloading - unbundling jobs construct
// RUN:   touch %t.o
// RUN:   %clang -### --intel -fsycl -fno-sycl-device-lib=all -fiopenmp %t.o -target x86_64-pc-windows-msvc -fopenmp-targets=spir64 -no-canonical-prefixes 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS %s
// CHK-UBJOBS: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-pc-windows-msvc,openmp-spir64,sycl-spir64-unknown-unknown-sycldevice" "-input=[[FATOBJ:.+\.o]]" "-output=[[HOSTOBJ:.+\.o]]" "-output=[[OMPBC:.+\.o]]" "-output=[[SYCLBC:.+\.o]]" "-unbundle"
// CHK-UBJOBS: llvm-link{{.*}} "[[SYCLBC]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-UBJOBS: sycl-post-link{{.*}} "-symbols" "-spec-const=rt" "-o" "[[SYCLTABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[SYCLTABLEOUT:.+\.txt]]" "[[SYCLTABLE]]"
// CHK-UBJOBS: llvm-foreach{{.*}} "--in-file-list=[[SYCLTABLEOUT]]" "--in-replace=[[SYCLTABLEOUT]]" "--out-ext=spv" "--out-file-list=[[SYCLLLVMFOROUT:.+\.txt]]" "--out-replace=[[SYCLLLVMFOROUT]]" "--" "{{.*}}llvm-spirv" "-o" "[[SYCLLLVMFOROUT]]" "-spirv-max-version=1.1" {{.*}} "[[SYCLTABLEOUT]]"
// CHK-UBJOBS: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[SYCLSPIRV:.+\.table]]" "[[SYCLTABLE]]" "[[SYCLLLVMFOROUT]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-pc-windows-msvc{{.*}}" "-target=spir64" "-kind=sycl" "-batch" "[[SYCLSPIRV]]"
// CHK-UBJOBS: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJ:.+\.o]]" "[[SYCLWRAPPERBC]]"
// CHK-UBJOBS: llvm-link{{.*}} "[[OMPBC]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBC:.+\.out]]"
// CHK-UBJOBS: llvm-spirv{{.*}} "-o" "[[OMPSPIRV:.+\.out]]" "[[OMPLINKEDBC]]"
// CHK-UBJOBS: clang-offload-wrapper{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRV]]"
// CHK-UBJOBS: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-o" "[[OMPWRAPPEROBJ:.+\.o]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-UBJOBS: link{{.*}} "-out:{{.*}}" {{.*}} "-defaultlib:sycl.lib" {{.*}} "-defaultlib:libiomp5md.lib" "-defaultlib:omptarget.lib" "[[HOSTOBJ]]" "[[OMPWRAPPEROBJ]]" "[[SYCLWRAPPEROBJ]]"

// RUN:   touch %t.o
// RUN:   %clang_cl -### --intel -fsycl -fno-sycl-device-lib=all -Qiopenmp %t.o --target=x86_64-pc-windows-msvc -Qopenmp-targets=spir64  2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-UBJOBS-CL %s
// CHK-UBJOBS-CL: clang-offload-bundler{{.*}} "-type=o" "-targets=host-x86_64-pc-windows-msvc,openmp-spir64,sycl-spir64-unknown-unknown-sycldevice{{(-coff)?}}" "-input=[[FATOBJ:.+\.o]]" "-output=[[HOSTOBJCL:.+\.obj]]" "-output=[[OMPBCCL:.+\.obj]]" "-output=[[SYCLBCCL:.+\.obj]]" "-unbundle"
// CHK-UBJOBS-CL: llvm-link{{.*}} "[[SYCLBCCL]]" "-o" "[[SYCLLINKEDBC:.+\.bc]]"
// CHK-UBJOBS-CL: sycl-post-link{{.*}} "-symbols" "-spec-const=rt" "-o" "[[SYCLTABLE:.+\.table]]" "[[SYCLLINKEDBC]]"
// CHK-UBJOBS-CL: file-table-tform{{.*}} "-extract=Code" "-drop_titles" "-o" "[[SYCLTABLEOUT:.+\.txt]]" "[[SYCLTABLE]]"
// CHK-UBJOBS-CL: llvm-foreach{{.*}} "--in-file-list=[[SYCLTABLEOUT]]" "--in-replace=[[SYCLTABLEOUT]]" "--out-ext=spv" "--out-file-list=[[SYCLLLVMFOROUT:.+\.txt]]" "--out-replace=[[SYCLLLVMFOROUT]]" "--" "{{.*}}llvm-spirv" "-o" "[[SYCLLLVMFOROUT]]" "-spirv-max-version=1.1" {{.*}} "[[SYCLTABLEOUT]]"
// CHK-UBJOBS-CL: file-table-tform{{.*}} "-replace=Code,Code" "-o" "[[SYCLSPIRV:.+\.table]]" "[[SYCLTABLE]]" "[[SYCLLLVMFOROUT]]"
// CHK-UBJOBS-CL: clang-offload-wrapper{{.*}} "-o=[[SYCLWRAPPERBC:.+\.bc]]" "-host=x86_64-pc-windows-msvc{{.*}}" "-target=spir64" "-kind=sycl" "-batch" "[[SYCLSPIRV]]"
// CHK-UBJOBS-CL: llc{{.*}} "-filetype=obj" "-o" "[[SYCLWRAPPEROBJCL:.+\.obj]]" "[[SYCLWRAPPERBC]]"
// CHK-UBJOBS-CL: llvm-link{{.*}} "[[OMPBCCL]]" "{{.*}}libomptarget-opencl.bc" "-o" "[[OMPLINKEDBCCL:.+\.exe]]"
// CHK-UBJOBS-CL: llvm-spirv{{.*}} "-o" "[[OMPSPIRVCL:.+\.exe]]" "[[OMPLINKEDBCCL]]"
// CHK-UBJOBS-CL: clang-offload-wrapper{{.*}} "-host" "x86_64-pc-windows-msvc{{.*}}" "-o" "[[OMPWRAPPERBC:.+\.bc]]" "-kind=openmp" "-target=spir64" "[[OMPSPIRVCL]]"
// CHK-UBJOBS-CL: clang{{.*}} "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" {{.*}} "-o" "[[OMPWRAPPEROBJCL:.+\.obj]]" {{.*}} "[[OMPWRAPPERBC]]"
// CHK-UBJOBS-CL: link{{.*}} "-out:{{.*}}" "-defaultlib:sycl.lib" {{.*}} "-defaultlib:libiomp5md.lib" "-defaultlib:omptarget.lib" "[[HOSTOBJCL]]" "[[OMPWRAPPEROBJCL]]" "[[SYCLWRAPPEROBJCL]]"
