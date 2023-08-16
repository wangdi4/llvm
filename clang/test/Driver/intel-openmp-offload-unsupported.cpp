/// Diagnose unsupported options specific to OpenMP offloading
// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64 -fsanitize=address -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=IGNORE_OPT -DARCH=spir64 -DOPT=-fsanitize=address
// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64_gen -fsanitize=address -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=IGNORE_OPT -DARCH=spir64_gen -DOPT=-fsanitize=address
// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64_x86_64 -fsanitize=address -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=IGNORE_OPT -DARCH=spir64_x86_64 -DOPT=-fsanitize=address

// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64 -traceback -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=IGNORE_OPT -DARCH=spir64 -DOPT=-traceback
// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64_gen -traceback -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=IGNORE_OPT -DARCH=spir64_gen -DOPT=-traceback
// RUN: %clangxx -fiopenmp -fopenmp-targets=spir64_x86_64 -traceback -### %s 2>&1 \
// RUN:  | FileCheck %s --check-prefix=IGNORE_OPT -DARCH=spir64_x86_64 -DOPT=-traceback

// IGNORE_OPT: ignoring '[[OPT]]' option as it is not currently supported for target '[[ARCH]]{{.*}}' [-Woption-ignored]
