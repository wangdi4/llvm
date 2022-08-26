// INTEL_FEATURE_CPU_MTL
// REQUIRES: intel_feature_cpu_mtl
/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xMETEORLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XMETEORLAKE,ADV_OPT %s
// RUN: %clang_cl -### -c /QxMETEORLAKE %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XMETEORLAKE,ADV_OPT %s
// XMETEORLAKE: "-target-cpu" "meteorlake"

// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-multiversioning"
// end INTEL_FEATURE_CPU_MTL
