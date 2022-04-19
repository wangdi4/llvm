// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
void a(_Complex double) {}
void b(_Complex double*) {}

// CHECK: define dso_local void @_Z1aCd(double noundef %{{.+}}, double noundef %{{.+}})

// PTR: define dso_local void @_Z1bPCd({ double, double }* noundef "intel_dtrans_func_index"="1" %{{.+}}){{.+}} !intel.dtrans.func.type ![[B:[0-9]+]]
// OPQ: define dso_local void @_Z1bPCd(ptr noundef "intel_dtrans_func_index"="1" %{{.+}}){{.+}} !intel.dtrans.func.type ![[B:[0-9]+]]
// CHECK: ![[B]] = distinct !{![[DD_PTR:[0-9]+]]}
// CHECK: ![[DD_PTR]] = !{![[DD_LITERAL:[0-9]+]], i32 1}
// CHECK: ![[DD_LITERAL]] = !{!"L", i32 2, ![[DOUBLE:[0-9]+]], ![[DOUBLE]]}
// CHECK: ![[DOUBLE]] = !{double 0.0{{.+}}, i32 0}
