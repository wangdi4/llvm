// Test if PGO instrumentation and use pass are invoked.
//
// TODO: remove redundant `-fno-legacy-pass-manager` key from RUN-lines once we move to NewPM by default.
//
// Ensure Pass PGOInstrumentationGenPass is invoked.
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// FIXME: Remove -fno-legacy-pass-manager when xmain changes to use the
//        new pass manager by default
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument=llvm %s  -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOGENPASS-INVOKED-INSTR-GEN --check-prefix=CHECK-INSTRPROF
// end INTEL_CUSTOMIZATION
=======
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument=llvm %s  -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOGENPASS-INVOKED-INSTR-GEN --check-prefix=CHECK-INSTRPROF
>>>>>>> 7fac21021d303bcc836e1cf11b62f8efe4d1f7ba
// CHECK-PGOGENPASS-INVOKED-INSTR-GEN: Running pass: PGOInstrumentationGen on
// CHECK-INSTRPROF: Running pass: InstrProfiling on
//
// Ensure Pass PGOInstrumentationGenPass is not invoked.
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// FIXME: Remove -fno-legacy-pass-manager when xmain changes to use the
//        new pass manager by default
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument=clang %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOGENPASS-INVOKED-INSTR-GEN-CLANG
// end INTEL_CUSTOMIZATION
// CHECK-PGOGENPASS-INVOKED-INSTR-GEN-CLANG-NOT: Running pass: PGOInstrumentationGen on

// INTEL_CUSTOMIZATION
// FIXME: Remove -fno-legacy-pass-manager when xmain changes to use the
//        new pass manager by default
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument=clang %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-CLANG-INSTRPROF
// RUN: %clang_cc1 -fno-legacy-pass-manager -O0 -fprofile-instrument=clang %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-CLANG-INSTRPROF
// end INTEL_CUSTOMIZATION
=======
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument=clang %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOGENPASS-INVOKED-INSTR-GEN-CLANG
// CHECK-PGOGENPASS-INVOKED-INSTR-GEN-CLANG-NOT: Running pass: PGOInstrumentationGen on

// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument=clang %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-CLANG-INSTRPROF
// RUN: %clang_cc1 -fno-legacy-pass-manager -O0 -fprofile-instrument=clang %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-CLANG-INSTRPROF
>>>>>>> 7fac21021d303bcc836e1cf11b62f8efe4d1f7ba
// CHECK-CLANG-INSTRPROF: Running pass: InstrProfiling on

// Ensure Pass PGOInstrumentationUsePass is invoked.
// RUN: llvm-profdata merge -o %t.profdata %S/Inputs/pgotestir.profraw
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// FIXME: Remove -fno-legacy-pass-manager when xmain changes to use the
//        new pass manager by default
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument-use-path=%t.profdata %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOUSEPASS-INVOKED-INSTR-USE
// end INTEL_CUSTOMIZATION
=======
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument-use-path=%t.profdata %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOUSEPASS-INVOKED-INSTR-USE
>>>>>>> 7fac21021d303bcc836e1cf11b62f8efe4d1f7ba
// CHECK-PGOUSEPASS-INVOKED-INSTR-USE: Running pass: PGOInstrumentationUse on
//
// Ensure Pass PGOInstrumentationUsePass is not invoked.
// RUN: llvm-profdata merge -o %t.profdata %S/Inputs/pgotestclang.profraw
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// FIXME: Remove -fno-legacy-pass-manager when xmain changes to use the
//        new pass manager by default
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument-use-path=%t.profdata %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOUSEPASS-INVOKED-USE-CLANG
// end INTEL_CUSTOMIZATION
=======
// RUN: %clang_cc1 -fno-legacy-pass-manager -O2 -fprofile-instrument-use-path=%t.profdata %s -fdebug-pass-manager -emit-llvm -o - 2>&1 | FileCheck %s -check-prefix=CHECK-PGOUSEPASS-INVOKED-USE-CLANG
>>>>>>> 7fac21021d303bcc836e1cf11b62f8efe4d1f7ba
// CHECK-PGOUSEPASS-INVOKED-USE-CLANG-NOT: Running pass: PGOInstrumentationUse on
