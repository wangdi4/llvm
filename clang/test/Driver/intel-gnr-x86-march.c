// INTEL_FEATURE_CPU_GNR
// REQUIRES: intel_feature_cpu_gnr
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=graniterapids 2>&1 \
// RUN:   | FileCheck %s -check-prefix=graniterapids
// graniterapids: "-target-cpu" "graniterapids"

// end INTEL_FEATURE_CPU_GNR
