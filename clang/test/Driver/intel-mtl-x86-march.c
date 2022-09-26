// INTEL_FEATURE_CPU_MTL
// REQUIRES: intel_feature_cpu_mtl
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=meteorlake 2>&1 \
// RUN:   | FileCheck %s -check-prefix=meteorlake
// meteorlake: "-target-cpu" "meteorlake"

// end INTEL_FEATURE_CPU_MTL
