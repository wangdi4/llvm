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
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device ats-m75" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_ATS_M75

/// Using OCLOC env variables for finding ocloc bits
// RUN: env OCLOCROOT="%S/Inputs/SYCL/lib" OCLOCVER="ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device skl" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_SKL
// RUN: env OCLOCROOT="%S/Inputs/SYCL/lib" OCLOCVER="ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device dg2" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_DG2
// RUN: env OCLOCROOT="%S/Inputs/SYCL/lib" OCLOCVER="ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device tgllp" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_TGLLP
// RUN: env OCLOCROOT="%S/Inputs/SYCL/lib" OCLOCVER="ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device ats-m75" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_ATS_M75

/// Using LIB env variable for finding ocloc bits
// RUN: env OCLOCROOT= OCLOCVER= LIB="%S/Inputs/SYCL/lib/ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device skl" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_SKL
// RUN: env OCLOCROOT= OCLOCVER= LIB="%S/Inputs/SYCL/lib/ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device dg2" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_DG2
// RUN: env OCLOCROOT= OCLOCVER= LIB="%S/Inputs/SYCL/lib/ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device tgllp" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_TGLLP
// RUN: env OCLOCROOT= OCLOCVER= LIB="%S/Inputs/SYCL/lib/ocloc" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device ats-m75" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefix=OCLOC_ATS_M75
// OCLOC_SKL: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen9-11{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "skl"
// OCLOC_DG2: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "dg2"
// OCLOC_TGLLP: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "tgllp"
// OCLOC_ATS_M75: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "ats-m75"

/// Multiple device test
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device dg2,skl,tgllp" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_MULTIPLE
// OCLOC_MULTIPLE: llvm-spirv" "-o" "[[LLVMSPIRVOUT:.+\.txt]]" "-spirv-max-version{{.*}}"
// OCLOC_MULTIPLE: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen9-11{{(/|\\\\)}}ocloc.exe" "-output" "[[SKLOUT:.+\.out]]" "-file" "[[LLVMSPIRVOUT]]" {{.*}} "-device" "skl"
// OCLOC_MULTIPLE: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" "-output" "[[TGLLPOUT:.+\.out]]" "-file" "[[LLVMSPIRVOUT]]" {{.*}} "-device" "dg2,tgllp"
// OCLOC_MULTIPLE: {{.*}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" "concat" "[[TGLLPOUT]]" "[[SKLOUT]]" "-out" "[[CONCATOUT:.+\.out]]"
// OCLOC_MULTIPLE: {{.*}}file-table-tform" "-replace=Code,Code" "-o" {{.*}} "[[CONCATOUT]]"

/// Verify -device *
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device *" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_STAR
// OCLOC_STAR: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen9-11{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "*"
// OCLOC_STAR: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "*"
// OCLOC_STAR: {{.*}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" "concat"

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

/// Verify OpenMP behaviors (splitting not default)
// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device *" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_OMP_STAR
// OCLOC_OMP_STAR-NOT: llvm-foreach{{.*}}
// OCLOC_OMP_STAR: "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen9-11{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "*"
// OCLOC_OMP_STAR: "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "*"
// OCLOC_OMP_STAR-NOT: llvm-foreach{{.*}}
// OCLOC_OMP_STAR: "{{.*}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" "concat"

// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device *" \
// RUN:   --sysroot=%S/Inputs/SYCL --enable-ocloc-split -### %s \
// RUN:   -fopenmp-device-code-split=per_kernel \
// RUN:   -target x86_64-unknown-linux-gnu 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_OMP_SPLIT_STAR
// OCLOC_OMP_SPLIT_STAR: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen9-11{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "*"
// OCLOC_OMP_SPLIT_STAR: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "*"
// OCLOC_OMP_SPLIT_STAR: llvm-foreach{{.*}} "--" "{{.*}}ocloc{{(/|\\\\)}}gen12+{{(/|\\\\)}}ocloc.exe" "concat"

/// Verify ocloc can be found with OCLOCROOT/OCLOCVER or LIB for non-splitting
/// environments.
// RUN: env OCLOCROOT="%S/Inputs/SYCL/lib" OCLOCVER="ocloc-nosplit" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device skl" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_NOSPLIT
// RUN: env OCLOCROOT= OCLOCVER= LIB="%S/Inputs/SYCL/lib/ocloc-nosplit" \
// RUN: %clangxx -fsycl -fsycl-targets=spir64_gen -Xs "-device skl" \
// RUN:   -### %s -target x86_64-pc-windows-msvc 2>&1 \
// RUN:   | FileCheck %s -check-prefixes=OCLOC_NOSPLIT
// OCLOC_NOSPLIT: llvm-foreach{{.*}} "--" "{{.*}}Inputs{{(/|\\\\)}}SYCL{{(/|\\\\)}}lib{{(/|\\\\)}}ocloc-nosplit{{(/|\\\\)}}ocloc.exe" {{.*}} "-device" "skl"
