// INTEL_FEATURE_CPU_LNL
// REQUIRES: intel_feature_cpu_lnl
/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xLUNARLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XLUNARLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxLUNARLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XLUNARLAKE,ADV_OPT %s
// XLUNARLAKE: "-target-cpu" "lunarlake"

// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-npm-multiversioning"
// end INTEL_FEATURE_CPU_LNL
