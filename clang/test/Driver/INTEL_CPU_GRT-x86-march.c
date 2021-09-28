#if INTEL_FEATURE_CPU_GRT
// REQUIRES: intel_feature_cpu_grt
// RUN: %clang -target x86_64-unknown-unknown -c -### %s -march=gracemont 2>&1 \
// RUN:   | FileCheck %s -check-prefix=gracemont
// gracemont: "-target-cpu" "gracemont"
#endif // INTEL_FEATURE_CPU_GRT
