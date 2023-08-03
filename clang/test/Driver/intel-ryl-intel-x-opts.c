// INTEL_FEATURE_CPU_RYL
// REQUIRES: intel_feature_cpu_ryl
/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xROYAL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XROYAL,ADV_OPT %s
// RUN: %clang_cl -### -c /QxROYAL %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XROYAL,ADV_OPT %s
// XROYAL: "-target-cpu" "royal"

// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-npm-multiversioning"
// end INTEL_FEATURE_CPU_RYL
