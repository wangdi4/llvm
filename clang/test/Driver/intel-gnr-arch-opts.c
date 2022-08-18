// INTEL_FEATURE_CPU_GNR
// REQUIRES: intel_feature_cpu_gnr

// RUN: %clang_cl -### -c /arch:GRANITERAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-GRANITERAPIDS,ARCH-WARN %s
// ARCH-GRANITERAPIDS: "-target-cpu" "graniterapids"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
// end INTEL_FEATURE_CPU_GNR
