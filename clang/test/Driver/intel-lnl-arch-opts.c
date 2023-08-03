// INTEL_FEATURE_CPU_LNL
// REQUIRES: intel_feature_cpu_lnl

// RUN: %clang_cl -### -c /arch:LUNARLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-LUNARLAKE,ARCH-WARN %s
// ARCH-LUNARLAKE: "-target-cpu" "lunarlake"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
// end INTEL_FEATURE_CPU_LNL
