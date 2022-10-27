/// Tests various items in regards to -fopenmp-target-loopopt
// RUN: %clangxx --intel -fiopenmp -fopenmp-targets=spir64 \
// RUN:          -fopenmp-target-loopopt -fopenmp-target-simd -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_ENABLED
// RUN: %clangxx --intel -fiopenmp -fopenmp-targets=spir64 \
// RUN:          -fopenmp-target-loopopt -fopenmp-target-simd \
// RUN:           -qopt-mem-layout-trans=1 -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_ENABLED
// RUN: %clangxx --intel -fiopenmp -fopenmp-targets=spir64 \
// RUN:          -fopenmp-target-loopopt -fopenmp-target-simd \
// RUN:           -qopt-mem-layout-trans=3 -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_ENABLED
// LOOPOPT_ENABLED: clang{{.*}} "-triple" "spir64"
// LOOPOPT_ENABLED-NOT: "-disable-intel-proprietary-opts"
// LOOPOPT_ENABLED-NOT: "-dtrans-mem-layout-level={{.*}}"
// LOOPOPT_ENABLED: "-mllvm" "-loopopt=1"

// RUN: %clangxx --intel -fiopenmp -fopenmp-targets=spir64 \
// RUN:          -fopenmp-target-loopopt -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_NOT_ENABLED
// RUN: %clangxx --intel -fiopenmp -fopenmp-targets=spir64 -### %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=LOOPOPT_NOT_ENABLED
// LOOPOPT_NOT_ENABLED: clang{{.*}} "-triple" "spir64"
// LOOPOPT_NOT_ENABLED: "-disable-intel-proprietary-opts"
// LOOPOPT_NOT_ENABLED-NOT: "-mllvm" "-loopopt=1" "-mllvm" "-intel-abi-compatible=true"
