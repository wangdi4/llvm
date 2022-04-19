// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,PTR
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm -opaque-pointers %s -o - | FileCheck %s --check-prefixes=CHECK,OPQ
// Test with variable length array.
//   %vla = alloca i8*, i64 %1, align 16
//
// I don't think the VLA size should affect it, so should just encode as:
//   %vla = alloca i8*, i64 %1, align 16, !dtrans_type !1
//   !1 = !{i8 0, i32 1}  ; i8
int a;
void b() {
  void *c[a];
  // PTR: alloca i8*, i64 %{{.+}}, align 16, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
  // OPQ: alloca ptr, i64 %{{.+}}, align 16, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
  void *d[a][a];
  // PTR: alloca i8*, i64 %{{.+}}, align 16, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
  // OPQ: alloca ptr, i64 %{{.+}}, align 16, !intel_dtrans_type ![[VOID_PTR:[0-9]+]]
  int *e[a];
  // PTR: alloca i32*, i64 %{{.+}}, align 16, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
  // OPQ: alloca ptr, i64 %{{.+}}, align 16, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
  int f[a];
  // CHECK: alloca i32, i64 %{{.+}}, align 16, !intel_dtrans_type ![[INT:[0-9]+]]
}

// CHECK: ![[VOID_PTR]] = !{i8 0, i32 1}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}
// CHECK: ![[INT]] = !{i32 0, i32 0}
