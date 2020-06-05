// INTEL_FEATURE_CPU_ADL
// REQUIRES: intel_feature_cpu_adl
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=alderlake 2>&1 \
// RUN:   | FileCheck %s -check-prefix=alderlake
// alderlake: "-target-cpu" "alderlake"

// end INTEL_FEATURE_CPU_ADL
