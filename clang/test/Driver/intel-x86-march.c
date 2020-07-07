// INTEL_FEATURE_CPU_SPR
// REQUIRES: intel_feature_cpu_spr
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=sapphirerapids 2>&1 \
// RUN:   | FileCheck %s -check-prefix=sapphirerapids
// sapphirerapids: "-target-cpu" "sapphirerapids"

// end INTEL_FEATURE_CPU_SPR
