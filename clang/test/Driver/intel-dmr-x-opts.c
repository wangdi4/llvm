// INTEL_FEATURE_CPU_DMR
// REQUIRES: intel_feature_cpu_dmr
/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xDIAMONDRAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XDIAMONDRAPIDS,ADV_OPT %s
// RUN: %clang_cl -### -c /QxDIAMONDRAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XDIAMONDRAPIDS,ADV_OPT %s
// XDIAMONDRAPIDS: "-target-cpu" "diamondrapids"

// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-multiversioning"
// end INTEL_FEATURE_CPU_DMR
