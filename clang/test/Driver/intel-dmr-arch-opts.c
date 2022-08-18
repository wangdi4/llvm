// INTEL_FEATURE_CPU_DMR
// REQUIRES: intel_feature_cpu_dmr

// RUN: %clang_cl -### -c /arch:DIAMONDRAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-DIAMONDRAPIDS,ARCH-WARN %s
// ARCH-DIAMONDRAPIDS: "-target-cpu" "diamondrapids"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
// end INTEL_FEATURE_CPU_DMR
