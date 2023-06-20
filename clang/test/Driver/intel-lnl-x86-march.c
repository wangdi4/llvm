// INTEL_FEATURE_CPU_LNL
// REQUIRES: intel_feature_cpu_lnl
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=lunarlake 2>&1 \
// RUN:   | FileCheck %s -check-prefix=lunarlake
// lunarlake: "-target-cpu" "lunarlake"

// end INTEL_FEATURE_CPU_LNL
