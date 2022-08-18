// INTEL_FEATURE_CPU_DMR
// REQUIRES: intel_feature_cpu_dmr
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=diamondrapids 2>&1 \
// RUN:   | FileCheck %s -check-prefix=diamondrapids
// diamondrapids: "-target-cpu" "diamondrapids"

// end INTEL_FEATURE_CPU_DMR
