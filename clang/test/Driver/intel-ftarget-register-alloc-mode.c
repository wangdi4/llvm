// INTEL_CUSTOMIZATION

// Test Intel-specific -ftarget-register-alloc-mode

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64_gen %s 2>&1 \
// RUN:   | FileCheck -check-prefix=NO_OPT_AOT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64_gen -ftarget-register-alloc-mode=pvc:auto %s 2>&1 \
// RUN:   | FileCheck -check-prefix=AUTO_AOT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64_gen -ftarget-register-alloc-mode=pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=LARGE_AOT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64_gen -ftarget-register-alloc-mode=pvc:small %s 2>&1 \
// RUN:   | FileCheck -check-prefix=SMALL_AOT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64_gen -ftarget-register-alloc-mode=pvc:default %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DEFAULT_AOT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64_gen -ftarget-register-alloc-mode=pvc:small,pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=MULTIPLE_ARGS_AOT %s

// RUN: %clang_cl -### /Qiopenmp \
// RUN:    /Qopenmp-targets=spir64_gen /Qtarget-register-alloc-mode=pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CL_AOT %s

// RUN: %clang_cl -### /Qiopenmp \
// RUN:    /Qopenmp-targets=spir64_gen /Qtarget-register-alloc-mode:pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CL_AOT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64 %s 2>&1 \
// RUN:   | FileCheck -check-prefix=NO_OPT_JIT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64 -ftarget-register-alloc-mode=pvc:auto %s 2>&1 \
// RUN:   | FileCheck -check-prefix=AUTO_JIT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64 -ftarget-register-alloc-mode=pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=LARGE_JIT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64 -ftarget-register-alloc-mode=pvc:small %s 2>&1 \
// RUN:   | FileCheck -check-prefix=SMALL_JIT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64 -ftarget-register-alloc-mode=pvc:default %s 2>&1 \
// RUN:   | FileCheck -check-prefix=DEFAULT_JIT %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64 -ftarget-register-alloc-mode=pvc:small,pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=MULTIPLE_ARGS_JIT %s

// RUN: %clang_cl -### /Qiopenmp \
// RUN:    /Qopenmp-targets=spir64 /Qtarget-register-alloc-mode=pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CL_JIT %s

// RUN: %clang_cl -### /Qiopenmp \
// RUN:    /Qopenmp-targets=spir64 /Qtarget-register-alloc-mode:pvc:large %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CL_JIT %s

// NO_OPT_AOT: ocloc{{.*}} "-output"
// NO_OPT_AOT: -device_options
// NO_OPT_AOT: pvc
// NO_OPT_AOT: "-options -ze-intel-enable-auto-large-GRF-mode"

// AUTO_AOT: ocloc{{.*}} "-output"
// AUTO_AOT: -device_options
// AUTO_AOT: pvc
// AUTO_AOT: "-options -ze-intel-enable-auto-large-GRF-mode"

// LARGE_AOT: ocloc{{.*}} "-output"
// LARGE_AOT: -device_options
// LARGE_AOT: pvc
// LARGE_AOT: "-options -ze-opt-large-register-file"

// SMALL_AOT: ocloc{{.*}} "-output"
// SMALL_AOT: -device_options
// SMALL_AOT: pvc
// SMALL_AOT: "-options -ze-intel-128-GRF-per-thread"

// DEFAULT_AOT-NOT: -device_options

// MULTIPLE_ARGS_AOT: ocloc{{.*}} "-output"
// MULTIPLE_ARGS_AOT: -device_options
// MULTIPLE_ARGS_AOT: pvc
// MULTIPLE_ARGS_AOT: "-options -ze-intel-128-GRF-per-thread"
// MULTIPLE_ARGS_AOT: -device_options
// MULTIPLE_ARGS_AOT: pvc
// MULTIPLE_ARGS_AOT: "-options -ze-opt-large-register-file"

// CL_AOT: ocloc{{.*}} "-output"
// CL_AOT: -device_options
// CL_AOT: pvc
// CL_AOT: "-options -ze-opt-large-register-file"

// NO_OPT_JIT: clang-offload-wrapper{{.*}} "-host"
// NO_OPT_JIT: "-compile-opts=-ftarget-register-alloc-mode=pvc:-ze-intel-enable-auto-large-GRF-mode"

// AUTO_JIT: clang-offload-wrapper{{.*}} "-host"
// AUTO_JIT: "-compile-opts=-ftarget-register-alloc-mode=pvc:-ze-intel-enable-auto-large-GRF-mode"

// LARGE_JIT: clang-offload-wrapper{{.*}} "-host"
// LARGE_JIT: "-compile-opts=-ftarget-register-alloc-mode=pvc:-ze-opt-large-register-file"

// SMALL_JIT: clang-offload-wrapper{{.*}} "-host"
// SMALL_JIT: "-compile-opts=-ftarget-register-alloc-mode=pvc:-ze-intel-128-GRF-per-thread"

// DEFAULT_JIT-NOT: -ftarget-register-alloc-mode=

// MULTIPLE_ARGS_JIT: clang-offload-wrapper{{.*}} "-host"
// MULTIPLE_ARGS_JIT: "-compile-opts=-ftarget-register-alloc-mode=pvc:-ze-intel-128-GRF-per-thread -ftarget-register-alloc-mode=pvc:-ze-opt-large-register-file"

// CL_JIT: clang-offload-wrapper{{.*}} "-host"
// CL_JIT: "-compile-opts=-ftarget-register-alloc-mode=pvc:-ze-opt-large-register-file"

// end INTEL_CUSTOMIZATION
