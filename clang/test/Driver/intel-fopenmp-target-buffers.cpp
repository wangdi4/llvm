// Test -fopenmp-target-buffers behaviors

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen -fopenmp-target-buffers=4GB %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_4GB_GEN %s
// RUN: %clang_cl -### --target-x86_64-pc-windows-msvc -Qiopenmp -Qopenmp-targets:spir64_gen -Qopenmp-target-buffers:4GB %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_4GB_GEN %s
// OPENMP_BUFFERS_4GB_GEN: ocloc{{.*}} "-output" {{.*}} "-options" "{{.*}}-cl-intel-greater-than-4GB-buffer-required"

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-buffers=4GB %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_4GB %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qiopenmp -Qopenmp-targets=spir64 -Qopenmp-target-buffers=4GB %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_4GB %s
// OPENMP_BUFFERS_4GB: clang-offload-wrapper{{.*}} "-compile-opts=-cl-intel-greater-than-4GB-buffer-required{{.*}}"

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -fopenmp-target-buffers=4GB %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_4GB_CPU %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qiopenmp -Qopenmp-targets=spir64_x86_64 -Qopenmp-target-buffers=4GB %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_4GB_CPU %s
// OPENMP_BUFFERS_4GB_CPU-NOT: -cl-intel-greater-than-4GB-buffer-required

// RUN: not %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-buffers=blah %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_BADARG %s
// RUN: not %clang_cl -### --target=x86_64-pc-windows-msvc -Qiopenmp -Qopenmp-targets=spir64 -Qopenmp-target-buffers=blah %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_BADARG %s
// OPENMP_BUFFERS_BADARG: error: unsupported argument 'blah' to option

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64 -fopenmp-target-buffers=default %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_DEFAULT %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qiopenmp -Qopenmp-targets=spir64 -Qopenmp-target-buffers=default %s 2>&1 \
// RUN:   | FileCheck -check-prefix=OPENMP_BUFFERS_DEFAULT %s
// OPENMP_BUFFERS_DEFAULT-NOT: unsupported argument
