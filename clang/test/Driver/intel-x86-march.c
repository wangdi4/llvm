// INTEL_FEATURE_CPU_GLC
// REQUIRES: intel_feature_cpu_glc
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=goldencove 2>&1 \
// RUN:   | FileCheck %s -check-prefix=goldencove
// goldencove: "-target-cpu" "goldencove"

// end INTEL_FEATURE_CPU_GLC
