/// Covers optimization level setting for offload
// RUN: %clangxx --intel -fiopenmp -fopenmp-targets=spir64 -target x86_64-unknown-linux-gnu -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix DEFAULT_OPTS %s
// RUN: %clang_cl --intel -Qiopenmp -Qopenmp-targets=spir64 --target=x86_64-pc-windows-msvc -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix DEFAULT_OPTS %s
// RUN: %clangxx --intel -O2 -fiopenmp -fopenmp-targets=spir64 -target x86_64-unknown-linux-gnu -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix DEFAULT_OPTS %s
// RUN: %clang_cl --intel -O2 -Qiopenmp -Qopenmp-targets=spir64 --target=x86_64-pc-windows-msvc -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix DEFAULT_OPTS %s
// DEFAULT_OPTS: clang{{.*}} "-triple" "x86_64-{{.*}}"{{.*}} "-O2"
// DEFAULT_OPTS: clang{{.*}} "-triple" "spir64"{{.*}} "-O2"
// DEFAULT_OPTS: sycl-post-link{{.*}} "-O2"

// RUN: %clangxx --intel -O3 -fiopenmp -fopenmp-targets=spir64 -target x86_64-unknown-linux-gnu -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix O3_OPTS %s
// RUN: %clang_cl --intel -O3 -Qiopenmp -Qopenmp-targets=spir64 --target=x86_64-pc-windows-msvc -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix O3_OPTS %s
// O3_OPTS: clang{{.*}} "-triple" "x86_64-{{.*}}"{{.*}} "-O3"
// O3_OPTS: clang{{.*}} "-triple" "spir64"{{.*}} "-O3"
// O3_OPTS: sycl-post-link{{.*}} "-O3"

// RUN: %clangxx --intel -O0 -fiopenmp -fopenmp-targets=spir64 -target x86_64-unknown-linux-gnu -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix O0_OPTS %s
// RUN: %clang_cl --intel -Od -Qiopenmp -Qopenmp-targets=spir64 --target=x86_64-pc-windows-msvc -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix O0_OPTS %s
// O0_OPTS: clang{{.*}} "-triple" "x86_64-{{.*}}"{{.*}} "-O0"
// O0_OPTS: clang{{.*}} "-triple" "spir64"{{.*}} "-O0"
// O0_OPTS: sycl-post-link{{.*}} "-O0"

/// -fopenmp-target-fast-atomics
// RUN: %clangxx --intel -fiopenmp -fopenmp-targets=spir64 -target x86_64-unknown-linux-gnu -fopenmp-target-fast-atomics -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix FAST_ATOMICS %s
// RUN: %clang_cl --intel -Qiopenmp -Qopenmp-targets=spir64 --target=x86_64-pc-windows-msvc -Qopenmp-target-fast-atomics -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix FAST_ATOMICS %s
// FAST_ATOMICS: clang{{.*}} "-triple" "x86_64-{{.*}}"
// FAST_ATOMICS: clang{{.*}} "-triple" "spir64"
// FAST_ATOMICS-SAME: "-mllvm" "-vpo-paropt-enable-64bit-opencl-atomics=true"
