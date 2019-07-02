// Behavior with -ipo/Qipo option
// RUN: %clang -### -c -ipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// RUN: %clang_cl -### -c /Qipo %s 2>&1 | FileCheck -check-prefix CHECK-IPO %s
// CHECK-IPO: "-flto"
// CHECK-IPO: "-flto-unit"

// Behavior with -no-ansi-alias option
// RUN: %clang -### -c -no-ansi-alias %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// RUN: %clang_cl -### -c /Qansi-alias- %s 2>&1 | FileCheck -check-prefix CHECK-NO_ANSI_ALIAS %s
// CHECK-NO_ANSI_ALIAS: "-relaxed-aliasing"

// Behavior with restrict/Qrestrict option
// RUN: %clang -### -c -restrict %s 2>&1 | FileCheck -check-prefix CHECK-RESTRICT %s
// RUN: %clang_cl -### -c /Qrestrict %s 2>&1 | FileCheck -check-prefix CHECK-RESTRICT %s
// CHECK-RESTRICT: "-restrict"

// Behavior with no-restrict/Qrestrict- option
// RUN: %clang -### -c -no-restrict %s 2>&1 | FileCheck -check-prefix CHECK-NO-RESTRICT %s
// RUN: %clang_cl -### -c /Qrestrict- %s 2>&1 | FileCheck -check-prefix CHECK-NO-RESTRICT %s
// CHECK-NO-RESTRICT: "-no-restrict"

// Behavior with -fno-alias option
// RUN: %clang -### -c -fno-alias %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// RUN: %clang_cl -### -c /Oa %s 2>&1 | FileCheck -check-prefix CHECK-FNO_ALIAS %s
// CHECK-FNO_ALIAS: "-fargument-noalias"

