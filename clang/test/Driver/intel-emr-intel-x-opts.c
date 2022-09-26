// INTEL_FEATURE_CPU_EMR
// REQUIRES: intel_feature_cpu_emr
/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xEMERALDRAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XEMERALDRAPIDS,ADV_OPT %s
// RUN: %clang_cl -### -c /QxEMERALDRAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XEMERALDRAPIDS,ADV_OPT %s
// XEMERALDRAPIDS: "-target-cpu" "emeraldrapids"

// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-multiversioning"
// end INTEL_FEATURE_CPU_EMR
