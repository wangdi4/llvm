// INTEL_FEATURE_CPU_EMR
// REQUIRES: intel_feature_cpu_emr

// RUN: %clang_cl -### -c /arch:EMERALDRAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-EMERALDRAPIDS,ARCH-WARN %s
// ARCH-EMERALDRAPIDS: "-target-cpu" "emeraldrapids"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
// end INTEL_FEATURE_CPU_EMR
