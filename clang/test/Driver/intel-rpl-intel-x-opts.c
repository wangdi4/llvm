// INTEL_FEATURE_CPU_RPL
// REQUIRES: intel_feature_cpu_rpl
/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xRAPTORLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XRAPTORLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxRAPTORLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XRAPTORLAKE,ADV_OPT %s
// XRAPTORLAKE: "-target-cpu" "raptorlake"

// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-multiversioning"
// end INTEL_FEATURE_CPU_RPL
