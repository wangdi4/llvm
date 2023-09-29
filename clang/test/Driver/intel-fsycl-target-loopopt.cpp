/// Tests various items in regards to -fsycl-target-loopopt
// RUN: %clangxx --intel -fsycl -fsycl-targets=spir64 \
// RUN:          -fsycl-target-loopopt -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_ENABLED
// RUN: %clangxx --intel -fsycl-device-only -fsycl-targets=spir64 \
// RUN:          -fsycl-target-loopopt -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_ENABLED
// LOOPOPT_ENABLED: clang{{.*}} "-triple" "spir64-unknown-unknown"
// LOOPOPT_ENABLED: "-mllvm" "-loopopt=1"
// LOOPOPT_ENABLED-SAME: "-floopopt-pipeline=light"
// LOOPOPT_ENABLED-SAME: "-disable-hir-vec-dir-insert"
// LOOPOPT_ENABLED-SAME: "-enable-o0-vectorization=false"
// LOOPOPT_ENABLED-SAME: "-vplan-driver=false"
// LOOPOPT_ENABLED-SAME: "-vplan-driver-hir=false"

// RUN: %clangxx --intel -fsycl-device-only -fsycl-targets=spir64 -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_NOT_ENABLED
// LOOPOPT_NOT_ENABLED: clang{{.*}} "-triple" "spir64-unknown-unknown"
// LOOPOPT_NOT_ENABLED-NOT: "-mllvm" "-loopopt=1"
