// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,NO_OPQ

// An incomplete function pointer type results in just an empty literal-struct,
// so ensure that we generate code for this properly, as it shows up in a pair
// of the 700-series spec tests.

class Incomplete;
using FPtr = int(*)(int *, Incomplete *, int, Incomplete);

void use() {
  FPtr ptr;
}

// CHECK: define{{.*}} void @_Z3usev()
// OPQ: alloca ptr, align 8, !intel_dtrans_type ![[FPTR:[0-9]+]]
// NO_OPQ: alloca {}*, align 8, !intel_dtrans_type ![[FPTR:[0-9]+]]
// CHECK: ![[FPTR]] = !{![[EMPTY_LIT:[0-9]+]], i32 1}
// CHECK: ![[EMPTY_LIT]] = !{!"L", i32 0}

