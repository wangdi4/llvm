// Behavior with -ipo/Qipo option
// RUN: %clang -### -c -ipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// RUN: %clang_cl -### -c /Qipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// CHECK-IPO: "-flto"
// CHECK-IPO: "-flto-unit"

// Behavior with -no-ansi-alias option
// RUN: %clang -### -c -no-ansi-alias %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// RUN: %clang_cl -### -c /Qansi-alias- %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// CHECK-NO_ANSI_ALIAS: "-relaxed-aliasing"

// Behavior with -fno-alias option
// RUN: %clang -### -c -fno-alias %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// RUN: %clang_cl -### -c /Oa %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// CHECK-FNO_ALIAS: "-fargument-noalias"

// Behavior with regcall option
// RUN: %clang -### -c -regcall %s 2>&1 | FileCheck -check-prefix CHECK-REGCALL %s
// RUN: %clang_cl -### -c /Qregcall %s 2>&1 | FileCheck -check-prefix CHECK-REGCALL %s
// CHECK-REGCALL: "-fdefault-calling-conv=regcall"

// Behavior with qopenmp/Qopenmp option
// RUN: %clang -### -c -target x86_64-linux-gnu -qopenmp %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP %s
// RUN: %clang_cl -### -c -target x86_64-windows-gnu /Qopenmp %s 2>&1 | FileCheck -check-prefix CHECK-QOPENMP %s
// RUN: %clang -### -target x86_64-linux-gnu --intel -qopenmp %s -o %t 2>&1 | FileCheck %s -check-prefix CHECK-LD-IOMP5
// Default behavior with -fopenmp should be liomp5
// RUN: %clang -### -target x86_64-linux-gnu -fopenmp %s -o %t 2>&1 | FileCheck %s -check-prefix CHECK-LD-IOMP5
// CHECK-QOPENMP: "-fopenmp"
// CHECK-LD-IOMP5: "-liomp5"

// Behavior with Qopt-jump-tables-,qno-opt-jump-tables option
// RUN: %clang -### -c -qno-opt-jump-tables %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES %s
// RUN: %clang_cl -### -c /Qopt-jump-tables- %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES %s
// RUN: %clang -### -c -qopt-jump-tables %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES2 %s
// RUN: %clang_cl -### -c /Qopt-jump-tables %s 2>&1 | FileCheck -check-prefix CHECK-QOPT-JUMP-TABLES2 %s
// CHECK-QOPT-JUMP-TABLES: "-fno-jump-tables"
// CHECK-QOPT-JUMP-TABLES2-NOT: "-fno-jump-tables"

// -mintrinsic-promote and /Qintrinsic-promote
// RUN: %clang -### -c -mintrinsic-promote %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE %s
// RUN: %clang_cl -### -c /Qintrinsic-promote %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE %s
// RUN: %clang -### -c -mno-intrinsic-promote %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// RUN: %clang_cl -### -c /Qintrinsic-promote- %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// off by default
// RUN: %clang -### -c %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// RUN: %clang_cl -### -c %s 2>&1 | FileCheck -check-prefix CHECK-INTRINSIC-PROMOTE-OFF %s
// CHECK-INTRINSIC-PROMOTE: "-mintrinsic-promote"
// CHECK-INTRINSIC-PROMOTE-OFF-NOT: "-mintrinsic-promote"

// RUN: %clang_cl -### -- %s 2>&1 | FileCheck -check-prefix=CL-LIBMMT %s
// CL-LIBMMT: "--dependent-lib=libmmt"
// CL-LIBMMT-NOT: "--dependent-lib=libmmd"

// RUN: %clang_cl -MD /clang:-MD -### -- %s 2>&1 | FileCheck -check-prefix=CL-LIBMMD %s
// CL-LIBMMD: "--dependent-lib=libmmd"
// CL-LIBMMD-NOT: "--dependent-lib=libcmt"

// RUN: %clang -### -target x86_64-unknown-windows-msvc -- %s 2>&1 | FileCheck -check-prefix=LIBMMT %s
// LIBMMT: "-defaultlib:libmmt"

// --dpcpp -fopenmp not supported
// When running the 'dpcpp' driver, the --dpcpp option is used to signify the
// source of how the compiler is invoked.
// RUN: %clang -### -c --dpcpp -fopenmp %s 2>&1 | FileCheck -check-prefix CHECK-DPCPP-FOPENMP %s
// RUN: %clang -### -c --dpcpp -fopenmp=libiomp5 %s 2>&1 | FileCheck -check-prefix CHECK-DPCPP-FOPENMP-ARG %s
// CHECK-DPCPP-FOPENMP: error: unsupported option '-fopenmp'
// CHECK-DPCPP-FOPENMP-ARG: error: unsupported option '-fopenmp=libiomp5'
