/// Intel specific optimization options
// REQUIRES: x86-registered-target

// RUN: %clang -qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// RUN: %clang_cl -Qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// RUN: %clang -qopt-mem-layout-trans=2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// RUN: %clang_cl -Qopt-mem-layout-trans:2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-enable-dtrans"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-enable-npm-dtrans"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-dtrans-mem-layout-level=2"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-dtrans-outofboundsok=false"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-dtrans-usecrulecompat=true"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-dtrans-inline-heuristics=true"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-dtrans-partial-inline=true"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-irmover-type-merging=false"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-spill-freq-boost=true"

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

/// -unroll support
// RUN: %clang -unroll3 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL,UNROLL3 %s
// RUN: %clang_cl -Qunroll3 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL,UNROLL3 %s
// RUN: %clang -unroll -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL %s
// RUN: %clang_cl -Qunroll -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL %s
// UNROLL: "-funroll-loops"
// UNROLL3: "-mllvm" "-hir-general-unroll-max-factor=3"
