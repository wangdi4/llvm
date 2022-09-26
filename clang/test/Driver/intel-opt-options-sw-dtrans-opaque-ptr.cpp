// INTEL_FEATURE_SW_DTRANS
// REQUIRES: x86-registered-target, intel_feature_sw_dtrans, enable-opaque-pointers

/// Intel specific optimization options that are available only under
/// INTEL FEATURE SW_DTRANS. This test case is the same as
/// intel-opt-options.cpp but only checks that the flags guarded with
/// INTEL FEATURE SW_DTRANS were added correctly when opaque pointers
/// are enabled.

// RUN: %clang -qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_DTRANS %s
// RUN: %clang_cl -Qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_DTRANS %s
// RUN: %clang -qopt-mem-layout-trans=2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_DTRANS %s
// RUN: %clang_cl -Qopt-mem-layout-trans:2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT_DTRANS %s
// LAYOUT_TRANS_DEFAULT_DTRANS: "-emit-dtrans-info"
// LAYOUT_TRANS_DEFAULT_DTRANS: "-mllvm" "-enable-dtrans"
// LAYOUT_TRANS_DEFAULT_DTRANS: "-mllvm" "-enable-npm-dtrans"
// LAYOUT_TRANS_DEFAULT_DTRANS: "-mllvm" "-dtrans-mem-layout-level=2"
// LAYOUT_TRANS_DEFAULT_DTRANS: "-mllvm" "-dtrans-outofboundsok=false"
// LAYOUT_TRANS_DEFAULT_DTRANS: "-mllvm" "-dtrans-usecrulecompat=true"
// LAYOUT_TRANS_DEFAULT_DTRANS: "-mllvm" "-dtrans-inline-heuristics=true"

// RUN: touch %t.o
// RUN: %clang -qopt-mem-layout-trans -target x86_64-unknown-linux-gnu -flto -### %t.o 2>&1 \
// RUN:  | FileCheck -DOPTION=-plugin-opt= -check-prefix=LAYOUT_TRANS_LTO_DTRANS %s
// RUN: %clang_cl /Qopt-mem-layout-trans -flto -fuse-ld=lld -### %t.o 2>&1 \
// RUN:  | FileCheck -DOPTION=-mllvm: -check-prefix=LAYOUT_TRANS_LTO_DTRANS %s
// LAYOUT_TRANS_LTO_DTRANS: "[[OPTION]]-enable-dtrans"
// LAYOUT_TRANS_LTO_DTRANS: "[[OPTION]]-enable-npm-dtrans"
// LAYOUT_TRANS_LTO_DTRANS: "[[OPTION]]-dtrans-mem-layout-level=2"
// LAYOUT_TRANS_LTO_DTRANS: "[[OPTION]]-dtrans-outofboundsok=false"
// LAYOUT_TRANS_LTO_DTRANS: "[[OPTION]]-dtrans-usecrulecompat=true"
// LAYOUT_TRANS_LTO_DTRANS: "[[OPTION]]-dtrans-inline-heuristics=true"

// RUN: %clang -qopt-mem-layout-trans=3 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_3 %s
// RUN: %clang_cl -Qopt-mem-layout-trans=3 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_3 %s
// LAYOUT_TRANS_3: "-mllvm" "-dtrans-mem-layout-level=3"

// RUN: %clang -qopt-mem-layout-trans=4 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_4 %s
// RUN: %clang_cl -Qopt-mem-layout-trans:4 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_4 %s
// LAYOUT_TRANS_4: "-mllvm" "-dtrans-mem-layout-level=4"

// RUN: %clang -qopt-mem-layout-trans=8 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_INVALID %s
// RUN: %clang_cl -Qopt-mem-layout-trans:8 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_INVALID %s
// LAYOUT_TRANS_INVALID: error: invalid argument '8'

// RUN: %clang -qopt-mem-layout-trans=3 -qno-opt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS %s
// RUN: %clang_cl -Qopt-mem-layout-trans:3 -Qopt-mem-layout-trans- -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS %s
// RUN: %clang -qopt-mem-layout-trans=0 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS %s
// RUN: %clang_cl -Qopt-mem-layout-trans:0 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS %s
// LAYOUT_TRANS-NOT: "-mllvm" "-dtrans-mem-layout-level{{.+}}"

// end INTEL_FEATURE_SW_DTRANS