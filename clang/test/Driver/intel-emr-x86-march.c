// INTEL_FEATURE_CPU_EMR
// REQUIRES: intel_feature_cpu_emr
// Ensure we support the various CPU architecture names.
//
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=emeraldrapids 2>&1 \
// RUN:   | FileCheck %s -check-prefix=emeraldrapids
// emeraldrapids: "-target-cpu" "emeraldrapids"

// end INTEL_FEATURE_CPU_EMR
