// INTEL_FEATURE_SW_ADVANCED
// REQUIRES: x86-registered-target, intel_feature_sw_advanced

/// Intel specific optimization options that are available only under
/// INTEL FEATURE SW_ADVANCED. This test case is the same as
/// intel-opt-options.cpp but only checks that the flags guarded with
/// INTEL FEATURE SW_ADVANCED were added correctly.

// RUN: %clang -qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_ADVANCED %s
// RUN: %clang_cl -Qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_ADVANCED %s
// RUN: %clang -qopt-mem-layout-trans=2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_ADVANCED %s
// RUN: %clang_cl -Qopt-mem-layout-trans:2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_ADVANCED %s
// LAYOUT_TRANS_DEFAULT_ADVANCED: "-mllvm" "-dtrans-partial-inline=true"

// RUN: touch %t.o
// RUN: %clang -qopt-mem-layout-trans -target x86_64-unknown-linux-gnu -flto -### %t.o 2>&1 \
// RUN:  | FileCheck -DOPTION=-plugin-opt= -check-prefix=LAYOUT_TRANS_LTO_ADVANCED %s
// RUN: %clang_cl /Qopt-mem-layout-trans -flto -fuse-ld=lld -### %t.o 2>&1 \
// RUN:  | FileCheck -DOPTION=-mllvm: -check-prefix=LAYOUT_TRANS_LTO_ADVANCED %s
// LAYOUT_TRANS_LTO_ADVANCED: "[[OPTION]]-dtrans-partial-inline=true"

// end INTEL_FEATURE_SW_ADVANCED