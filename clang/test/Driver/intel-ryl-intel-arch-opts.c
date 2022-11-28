// INTEL_FEATURE_CPU_RYL
// REQUIRES: intel_feature_cpu_ryl

// RUN: %clang_cl -### -c /arch:ROYAL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-ROYAL,ARCH-WARN %s
// ARCH-ROYAL: "-target-cpu" "royal"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
// end INTEL_FEATURE_CPU_RYL
