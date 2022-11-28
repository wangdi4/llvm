// INTEL_FEATURE_CPU_RYL
// REQUIRES: intel_feature_cpu_ryl
// Ensure we support the various CPU architecture names.
//
// RUN: %clang --target=x86_64 -c -### %s -march=royal 2>&1 \
// RUN:   | FileCheck %s -check-prefix=royal
// royal: "-target-cpu" "royal"

// end INTEL_FEATURE_CPU_RYL
