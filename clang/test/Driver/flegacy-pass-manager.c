/// -fno-legacy-pass-manager and -fexperimental-new-pass-manager are retained
// as no-ops. The inverted options are no longer supported.

// RUN: %clang -### -c -fno-legacy-pass-manager %s 2>&1 | FileCheck %s
// INTEL_CUSTOMIZATION
// RUN: %clang -### -flegacy-pass-manager %s
// end INTEL_CUSTOMIZATION

// RUN: %clang -### -c -fexperimental-new-pass-manager %s 2>&1 | FileCheck %s
// INTEL_CUSTOMIZATION
// RUN: %clang -### -fno-experimental-new-pass-manager %s
// end INTEL_CUSTOMIZATION

// Just check that there is no argument unused warning. There is no need to
// pass any cc1 options.

// CHECK-NOT: warning: argument unused
