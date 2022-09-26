// INTEL_FEATURE_CPU_GNR
// REQUIRES: intel_feature_cpu_gnr
/// Test behaviors for -x and /Qx options

// RUN: %clang -### -c -xGRANITERAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XGRANITERAPIDS,ADV_OPT %s
// RUN: %clang_cl -### -c /QxGRANITERAPIDS %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=XGRANITERAPIDS,ADV_OPT %s
// XGRANITERAPIDS: "-target-cpu" "graniterapids"

// ADV_OPT-SAME: "-fintel-advanced-optim"
// ADV_OPT-NOT: "-enable-multiversioning"
// end INTEL_FEATURE_CPU_GNR
