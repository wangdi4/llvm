///
/// Tests for -fsycl-preserve-device-nonsemantic-metadata
///

// RUN: touch %tfoo.o
// RUN: %clangxx -fsycl -fsycl-preserve-device-nonsemantic-metadata -### %tfoo.o 2>&1 | \
// RUN:  FileCheck -check-prefix CHECK-WITH %s
// RUN: %clangxx -fsycl -### %tfoo.o 2>&1 | \
// RUN:  FileCheck -check-prefix CHECK-WITHOUT %s

// CHECK-WITH: llvm-spirv{{.*}} "--spirv-preserve-auxdata"
// CHECK-WITH-SAME: "-spirv-ext=-all,{{.*}},+SPV_KHR_non_semantic_info"

// CHECK-WITHOUT: "{{.*}}llvm-spirv"
// CHECK-WITHOUT-NOT: --spirv-preserve-auxdata
<<<<<<< HEAD
// CHECK-WITHOUT:-spirv-ext=
// CHECK-WITHOUT-NOT: +SPV_KHR_non_semantic_info
=======

// INTEL_CUSTOMIZATION
// CHECK-WITHOUT:-spirv-ext=
// CHECK-WITHOUT: +SPV_KHR_non_semantic_info
// end INTEL_CUSTOMIZATION
>>>>>>> f6225fab81ee733d22a1f786c79282212d5370ca
