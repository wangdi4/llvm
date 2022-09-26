// INTEL_FEATURE_CPU_RPL
// REQUIRES: intel_feature_cpu_rpl

// RUN: %clang_cl -### -c /arch:RAPTORLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-RAPTORLAKE,ARCH-WARN %s
// ARCH-RAPTORLAKE: "-target-cpu" "raptorlake"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
// end INTEL_FEATURE_CPU_RPL
