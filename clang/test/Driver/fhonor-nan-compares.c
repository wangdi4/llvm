// Test that the -fhonor-nan-compare flag gets passed on to the Clang
// frontend.
//
// REQUIRES: clang-driver
//
// RUN: %clang -### -fhonor-nan-compares -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-HONOR-NAN %s
// INTEL_CUSTOMIZATION
// RUN: %clang_cl -### /Qhonor-nan-compares -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-HONOR-NAN %s
// end INTEL_CUSTOMIZATION
// CHECK-HONOR-NAN: "-cc1"
// CHECK-HONOR-NAN-NOT: "-fno-honor-nan-compares"
// CHECK-HONOR-NAN: "-fhonor-nan-compares"
// CHECK-HONOR-NAN-NOT: "-fno-honor-nan-compares"
//
// The flag is omitted if -fno-honor-nan-compare is used.
// RUN: %clang -### -fno-honor-nan-compares -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-NO-HONOR-NAN %s
// INTEL_CUSTOMIZATION
// RUN: %clang_cl -### /Qhonor-nan-compares- -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-NO-HONOR-NAN %s
// end INTEL_CUSTOMIZATION
// CHECK-NO-HONOR-NAN: "-cc1"
// CHECK-NO-HONOR-NAN-NOT: "-fhonor-nan-compares"
//
// Test that the last option takes precedence.
// RUN: %clang -### -fhonor-nan-compares -fno-honor-nan-compares -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-NO-HONOR-NAN %s
// RUN: %clang -### -fno-honor-nan-compares -fhonor-nan-compares -c %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-HONOR-NAN %s
