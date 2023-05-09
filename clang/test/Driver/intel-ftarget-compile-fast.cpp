// Test Intel specific -ftarget-compile-fast behaviors

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp \
// RUN:    -fopenmp-targets=spir64_gen -ftarget-compile-fast %s 2>&1 \
// RUN:   | FileCheck -check-prefix=TARGET_COMPILE_FAST_GEN %s
// RUN: %clang_cl -### --target=x86_64-pc-windows-msvc -Qiopenmp \
// RUN:     -Qopenmp-targets:spir64_gen -ftarget-compile-fast %s 2>&1 \
// RUN:   | FileCheck -check-prefix=TARGET_COMPILE_FAST_GEN %s

// TARGET_COMPILE_FAST_GEN: ocloc{{.*}} "-output"
// TARGET_COMPILE_FAST_GEN: "-options"
// TARGET_COMPILE_FAST_GEN: -igc_opts 'PartitionUnit=1,SubroutineThreshold=50000'
