/// Tests general behaviors of -fsycl-pstl-offload

// RUN: %clangxx --target=x86_64-unknown-linux-gnu -fsycl -fsycl-pstl-offload \
// RUN:          -I order_check_dir -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefixes=PSTL_DEFAULT,PSTL_DEFAULT_LINUX
// RUN: %clang_cl /fsycl /fsycl-pstl-offload -I order_check_dir -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_DEFAULT
// PSTL_DEFAULT: clang{{.*}} "-fsycl-is-device"
// PSTL_DEFAULT-SAME: "-D__SYCL_PSTL_OFFLOAD__=1"
// PSTL_DEFAULT-SAME: "-I" "{{.*}}..{{(/|\\\\)}}..{{(/|\\\\)}}include{{(/|\\\\)}}pstl_offload"
// PSTL_DEFAULT-SAME: "-I" "order_check_dir"
// PSTL_DEFAULT: clang{{.*}} "-fsycl-is-host"
// PSTL_DEFAULT-SAME: "-D__SYCL_PSTL_OFFLOAD__=1"
// PSTL_DEFAULT-SAME: "-I" "{{.*}}..{{(/|\\\\)}}..{{(/|\\\\)}}include{{(/|\\\\)}}pstl_offload"
// PSTL_DEFAULT-SAME: "-I" "order_check_dir"
// PSTL_DEFAULT_LINUX: ld{{.*}} "-L{{.*}}..{{(/|\\\\)}}..{{(/|\\\\)}}lib"
// PSTL_DEFAULT_LINUX-SAME: "-lpstloffload"

// RUN: %clangxx -fsycl -fsycl-pstl-offload=gpu -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_GPU
// RUN: %clang_cl /fsycl /fsycl-pstl-offload:gpu -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_GPU
// PSTL_GPU: clang{{.*}} "-fsycl-is-device"
// PSTL_GPU-SAME: "-D__SYCL_PSTL_OFFLOAD__=3"
// PSTL_GPU: clang{{.*}} "-fsycl-is-host"
// PSTL_GPU-SAME: "-D__SYCL_PSTL_OFFLOAD__=3"

// RUN: %clangxx -fsycl -fsycl-pstl-offload=cpu -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_CPU
// RUN: %clang_cl /fsycl /fsycl-pstl-offload:cpu -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_CPU
// PSTL_CPU: clang{{.*}} "-fsycl-is-device"
// PSTL_CPU-SAME: "-D__SYCL_PSTL_OFFLOAD__=2"
// PSTL_CPU: clang{{.*}} "-fsycl-is-host"
// PSTL_CPU-SAME: "-D__SYCL_PSTL_OFFLOAD__=2"

// RUN: %clangxx -fsycl -fno-sycl-pstl-offload -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=NO_PSTL
// RUN: %clang_cl /fsycl /fno-sycl-pstl-offload -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=NO_PSTL
// NO_PSTL: clang{{.*}} "-fsycl-is-device"
// NO_PSTL-SAME: "-D__SYCL_PSTL_OFFLOAD__=0"
// NO_PSTL: clang{{.*}} "-fsycl-is-host"
// NO_PSTL-SAME: "-D__SYCL_PSTL_OFFLOAD__=0"

// RUN: %clangxx -fsycl-pstl-offload -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_UNUSED
// RUN: %clang_cl /fsycl-pstl-offload -c -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_UNUSED
// PSTL_UNUSED: argument unused during compilation: '{{.*}}fsycl-pstl-offload'

// RUN: env DPL_ROOT=dplrootval \
// RUN: %clangxx -fsycl -fsycl-pstl-offload --target=x86_64-unknown-linux-gnu -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefixes=PSTL_DPL_ROOT,PSTL_DPL_ROOT_LINUX
// RUN: env DPL_ROOT=dplrootval \
// RUN: %clang_cl /fsycl /fsycl-pstl-offload -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_DPL_ROOT
// PSTL_DPL_ROOT: clang{{.*}} "-fsycl-is-device"
// PSTL_DPL_ROOT-SAME: "-I" "dplrootval{{(/|\\\\)}}include{{(/|\\\\)}}pstl_offload"
// PSTL_DPL_ROOT: clang{{.*}} "-fsycl-is-host"
// PSTL_DPL_ROOT-SAME: "-I" "dplrootval{{(/|\\\\)}}include{{(/|\\\\)}}pstl_offload"
// PSTL_DPL_ROOT_LINUX: ld{{.*}} "-Ldplrootval{{(/|\\\\)}}lib"
// PSTL_DPL_ROOT_LINUX-SAME: "-lpstloffload"

// RUN: %clangxx -help -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_HELP
// PSTL_HELP: fsycl-pstl-offload

// RUN: %clang_cl -help -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=PSTL_WIN_HELP
// PSTL_WIN_HELP-NOT: fsycl-pstl-offload
