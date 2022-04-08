// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -mllvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ

bool bar(decltype(nullptr));

void use() {
  bar(nullptr);
}
// PTR: declare !intel.dtrans.func.type ![[BAR:[0-9]+]] noundef zeroext i1 @_Z3barDn(i8* "intel_dtrans_func_index"="1")
// OPQ: declare !intel.dtrans.func.type ![[BAR:[0-9]+]] noundef zeroext i1 @_Z3barDn(ptr "intel_dtrans_func_index"="1")

// CHECK: ![[BAR]] = distinct !{![[NULLPTR:[0-9]+]]}
// CHECK: ![[NULLPTR]] = !{i8 0, i32 1}
