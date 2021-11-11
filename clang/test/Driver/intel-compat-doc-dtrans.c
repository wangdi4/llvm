// INTEL_FEATURE_SW_DTRANS
// REQUIRES: intel_feature_sw_dtrans

// Check output from -fintel-compatibility-doc using all values that are
// allowed when a DTrans build is enabled.
//
// RUN: %clang -c -Xclang -fintel-compatibility-doc=WholeProgramVTableWrap %s 2>&1 | FileCheck --match-full-lines %s

// CHECK: WholeProgramVTableWrap
// CHECK-NEXT: ======================
// CHECK-NEXT: When used with ``-fwhole-program-vtables`` option, this optimization enables
// CHECK-NEXT: devirtualization with whole program analysis without hidden LTO visibility.

// end INTEL_FEATURE_SW_DTRANS