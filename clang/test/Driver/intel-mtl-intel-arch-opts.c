// INTEL_FEATURE_CPU_MTL
// REQUIRES: intel_feature_cpu_mtl

// RUN: %clang_cl -### -c /arch:METEORLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=ARCH-METEORLAKE,ARCH-WARN %s
// ARCH-METEORLAKE: "-target-cpu" "meteorlake"

// ARCH-WARN-NOT: ignoring invalid /arch: argument
// end INTEL_FEATURE_CPU_MTL
