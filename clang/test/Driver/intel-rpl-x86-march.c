// INTEL_FEATURE_CPU_RPL
// REQUIRES: intel_feature_cpu_rpl
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=raptorlake 2>&1 \
// RUN:   | FileCheck %s -check-prefix=raptorlake
// raptorlake: "-target-cpu" "raptorlake"

// end INTEL_FEATURE_CPU_RPL
