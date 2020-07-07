// INTEL_FEATURE_CPU_RKL
// REQUIRES: intel_feature_cpu_rkl
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=rocketlake 2>&1 \
// RUN:   | FileCheck %s -check-prefix=rocketlake
// rocketlake: "-target-cpu" "rocketlake"

// end INTEL_FEATURE_CPU_RKL
