// REQUIRES: x86-registered-target

/// ###########################################################################
/// Tests for non-spirv CPU device AOT compilation.
/// These tests should be moved to according files when upstreaming.
/// For now keep these as Intel-customized.

/// Ahead of Time compilation for nonspirv cpu
// RUN:   %clang -target x86_64-unknown-linux-gnu -ccc-print-phases -fsycl -fno-sycl-device-lib=all -fno-sycl-instrument-device-code -fsycl-device-code-split -fsycl-targets=x86_64-unknown-unknown %s 2>&1 \
// RUN:    | FileCheck %s -check-prefixes=CHK-PHASES-AOT,CHK-PHASES-NONSPIRV-CPU
// CHK-PHASES-AOT: 0: input, "[[INPUT:.+\.c]]", c++, (host-sycl)
// CHK-PHASES-AOT: 1: append-footer, {0}, c++, (host-sycl)
// CHK-PHASES-AOT: 2: preprocessor, {1}, c++-cpp-output, (host-sycl)
// CHK-PHASES-AOT: 3: input, "[[INPUT:.+\.c]]", c++, (device-sycl)
// CHK-PHASES-AOT: 4: preprocessor, {3}, c++-cpp-output, (device-sycl)
// CHK-PHASES-AOT: 5: compiler, {4}, ir, (device-sycl)
// CHK-PHASES-NONSPIRV-CPU: 6: offload, "host-sycl (x86_64-unknown-linux-gnu)" {2}, "device-sycl (spir64_x86_64-unknown-unknown)" {5}, c++-cpp-output
// CHK-PHASES-AOT: 7: compiler, {6}, ir, (host-sycl)
// CHK-PHASES-AOT: 8: backend, {7}, assembler, (host-sycl)
// CHK-PHASES-AOT: 9: assembler, {8}, object, (host-sycl)
// CHK-PHASES-AOT: 10: linker, {9}, image, (host-sycl)
// CHK-PHASES-AOT: 11: linker, {5}, ir, (device-sycl)
// CHK-PHASES-AOT: 12: sycl-post-link, {11}, tempfiletable, (device-sycl)
// CHK-PHASES-AOT: 13: file-table-tform, {12}, tempfilelist, (device-sycl)
// CHK-PHASES-NONSPIRV-CPU-NOT: llvm-spirv
// CHK-PHASES-NONSPIRV-CPU: 14: backend-compiler, {13}, tempfilelist, (device-sycl)
// CHK-PHASES-NONSPIRV-CPU: 15: file-table-tform, {12, 14}, tempfiletable, (device-sycl)
// CHK-PHASES-NONSPIRV-CPU: 16: clang-offload-wrapper, {15}, object, (device-sycl)
// CHK-PHASES-NONSPIRV-CPU: 17: offload, "host-sycl (x86_64-unknown-linux-gnu)" {10}, "device-sycl (spir64_x86_64-unknown-unknown)" {16}, image

/// Ahead of Time compilation for nonspirv cpu - tool invocation
// RUN: %clang -target x86_64-unknown-linux-gnu -fsycl -fno-sycl-device-lib=all -fno-sycl-instrument-device-code -fsycl-device-code-split -fsycl-targets=x86_64-unknown-unknown %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-AOT,CHK-TOOLS-NONSPIRV-CPU
// CHK-TOOLS-AOT: clang{{.*}} "-fsycl-is-device"{{.*}} "-fsycl-int-header=[[INPUT1:.+\-header.+\.h]]" "-fsycl-int-footer={{.*}}"{{.*}} "-o" "[[OUTPUT1:.+\.bc]]"
// CHK-TOOLS-AOT: llvm-link{{.*}} "[[OUTPUT1]]" "-o" "[[OUTPUT2:.+\.bc]]"
// CHK-TOOLS-AOT: sycl-post-link{{.*}} "-split=auto" {{.*}} "-spec-const=emulation" "-device-globals" "-o" "[[OUTPUT3:.+\.table]]" "[[OUTPUT2]]"
// CHK-TOOLS-AOT: file-table-tform{{.*}} "-o" "[[OUTPUT4:.+\.txt]]" "[[OUTPUT3]]"
// CHK-TOOLS-NONSPIRV-CPU-NOT: llvm-foreach{{.*}}llvm-spirv{{.*}}
// CHK-TOOLS-NONSPIRV-CPU: llvm-foreach{{.*}} "--out-file-list=[[OUTPUT6:.+\.txt]]{{.*}} "--" "{{.*}}opencl-aot{{.*}} "-o=[[OUTPUT6]]" "--device=cpu" "[[OUTPUT4]]"
// CHK-TOOLS-AOT: file-table-tform{{.*}} "-o" "[[OUTPUT7:.+\.table]]" "[[OUTPUT3]]" "[[OUTPUT6]]"
// CHK-TOOLS-NONSPIRV-CPU: clang-offload-wrapper{{.*}} "-o=[[OUTPUT8:.+\.bc]]" "-host=x86_64-unknown-linux-gnu" "-target=spir64_x86_64" "-kind=sycl" "-batch" "[[OUTPUT7]]"
// CHK-TOOLS-AOT: llc{{.*}} "-filetype=obj" "-o" "[[OUTPUT9:.+\.o]]" "[[OUTPUT8]]"
// CHK-TOOLS-AOT: clang{{.*}} "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OUTPUT10:.+\.o]]"
// CHK-TOOLS-AOT: ld{{.*}} "[[OUTPUT10]]" "[[OUTPUT9]]" {{.*}} "-lsycl"

// Check to be sure that for windows, the 'exe' tools are called
// RUN: %clang_cl -fsycl -fsycl-targets=x86_64-unknown-unknown %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-CPU-WIN
// RUN: %clang -target x86_64-pc-windows-msvc -fsycl -fsycl-targets=x86_64-unknown-unknown %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-CPU-WIN
// CHK-TOOLS-CPU-WIN: opencl-aot.exe{{.*}}

/// Check -Xsycl-target-backend option passing
// RUN:   %clang -### -target x86_64-unknown-linux-gnu -fsycl -fsycl-targets=x86_64-unknown-unknown -Xsycl-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS %s
// CHK-TOOLS-CPU-OPTS: opencl-aot{{.*}} "-DFOO1" "-DFOO2"
// CHK-TOOLS-CPU-OPTS-NOT: clang-offload-wrapper{{.*}} "-compile-opts={{.*}}

// RUN:   %clang -### -target x86_64-unknown-linux-gnu -fsycl -fsycl-targets=x86_64-unknown-unknown -Xsycl-target-backend "--bo='\"-DFOO1 -DFOO2\"'" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS3 %s
// CHK-TOOLS-CPU-OPTS3: opencl-aot{{.*}} "--bo=\"-DFOO1 -DFOO2\""

/// Check for implied options (-g -O0)
// RUN:   %clang -### -target x86_64-unknown-linux-gnu -fsycl -fsycl-targets=x86_64-unknown-unknown -g -O0 -Xsycl-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS-CPU %s
// RUN:   %clang_cl -### -fsycl -fsycl-targets=x86_64-unknown-unknown -Zi -Od -Xsycl-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS-CPU %s
// CHK-TOOLS-IMPLIED-OPTS-CPU: opencl-aot{{.*}} "--bo=-g -cl-opt-disable" "-DFOO1" "-DFOO2"

/// Check -Xsycl-target-linker option passing
// RUN:   %clang -### -target x86_64-unknown-linux-gnu -fsycl -fsycl-targets=x86_64-unknown-unknown -Xsycl-target-linker "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS2 %s
// CHK-TOOLS-CPU-OPTS2: opencl-aot{{.*}} "-DFOO1" "-DFOO2"
// CHK-TOOLS-CPU-OPTS2-NOT: clang-offload-wrapper{{.*}} "-link-opts=-DFOO1 -DFOO2"
