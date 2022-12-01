/// Test the Windows ocloc split functionality

/// Single device tests
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device skl" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_SKL
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device dg2" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_DG2
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device tgllp" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_TGLLP
// OCLOC_SKL: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}iris{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "skl"
// OCLOC_DG2: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}dgpu{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "dg2"
// OCLOC_TGLLP: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}xe{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "tgllp"

/// Multiple device test
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device dg2,skl,tgllp" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_MULTIPLE
// OCLOC_MULTIPLE: llvm-spirv" "-o" "[[LLVMSPIRVOUT:.+\.txt]]" "-spirv-max-version{{.*}}"
// OCLOC_MULTIPLE: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}iris{{(/|\\\\)}}ocloc.exe" "-output" "[[SKLOUT:.+\.out]]" "-file" "[[LLVMSPIRVOUT]]" {{.*}} "-device" "skl"
// OCLOC_MULTIPLE: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}xe{{(/|\\\\)}}ocloc.exe" "-output" "[[TGLLPOUT:.+\.out]]" "-file" "[[LLVMSPIRVOUT]]" {{.*}} "-device" "tgllp"
// OCLOC_MULTIPLE: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}dgpu{{(/|\\\\)}}ocloc.exe" "-output" "[[DGPUOUT:.+\.out]]" "-file" "[[LLVMSPIRVOUT]]" {{.*}} "-device" "dg2"
// OCLOC_MULTIPLE: {{.*}}ocloc{{(/|\\\\)}}xe{{(/|\\\\)}}ocloc.exe" "concat" "[[TGLLPOUT]]" "[[DGPUOUT]]" "-out" "[[CONCATOUT1:.+\.out]]"
// OCLOC_MULTIPLE: {{.*}}ocloc{{(/|\\\\)}}xe{{(/|\\\\)}}ocloc.exe" "concat" "[[CONCATOUT1]]" "[[SKLOUT]]" "-out" "[[CONCATOUT:.+\.out]]"
// OCLOC_MULTIPLE: {{.*}}file-table-tform" "-replace=Code,Code" "-o" {{.*}} "[[CONCATOUT]]"

/// Verify -device *
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device *" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_STAR
// OCLOC_STAR: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}iris{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "gen9,gen11"
// OCLOC_STAR: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}xe{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "*"
// OCLOC_STAR: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}dgpu{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "XE_HPG_CORE"
// OCLOC_STAR: {{.*}}ocloc{{(/|\\\\)}}xe{{(/|\\\\)}}ocloc.exe" "concat"

/// phases with split ocloc
// RUN: touch %t.o
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device *" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -ccc-print-phases %t.o \
// RUN:   -fno-sycl-device-lib=all -fno-sycl-instrument-device-code \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_PHASES
// OCLOC_PHASES: 0: input, "{{.*}}", object, (host-sycl)
// OCLOC_PHASES: 1: clang-offload-unbundler, {0}, object, (host-sycl)
// OCLOC_PHASES: 2: linker, {1}, image, (host-sycl)
// OCLOC_PHASES: 3: spirv-to-ir-wrapper, {1}, ir, (device-sycl)
// OCLOC_PHASES: 4: linker, {3}, ir, (device-sycl)
// OCLOC_PHASES: 5: sycl-post-link, {4}, tempfiletable, (device-sycl)
// OCLOC_PHASES: 6: file-table-tform, {5}, tempfilelist, (device-sycl)
// OCLOC_PHASES: 7: llvm-spirv, {6}, tempfilelist, (device-sycl)
// OCLOC_PHASES: 8: backend-compiler, {7}, image, (device-sycl)
// OCLOC_PHASES: 9: file-table-tform, {5, 8}, tempfiletable, (device-sycl)
// OCLOC_PHASES: 10: clang-offload-wrapper, {9}, object, (device-sycl)
// OCLOC_PHASES: 11: offload, "host-sycl (x86_64-unknown-linux-gnu)" {2}, "device-sycl (spir64_gen-unknown-unknown)" {10}, image
