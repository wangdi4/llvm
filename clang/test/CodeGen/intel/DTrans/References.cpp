// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

int G1, G2;

// CHECK: define dso_local noundef i32 @_Z3barRiOi(i32* noundef nonnull align 4 dereferenceable(4) "intel_dtrans_func_index"="1" %A, i32* noundef nonnull align 4 dereferenceable(4) "intel_dtrans_func_index"="2" %B) {{.*}}!intel.dtrans.func.type ![[BAR_MD:[0-9]+]]
int bar(int &A, int &&B) {
// CHECK: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR:[0-9]+]]
// CHECK: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR]]
  return 0;
}


// CHECK: define dso_local noundef "intel_dtrans_func_index"="1" i32* @_Z3fooRKPiS1_(i32** noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="2" %A, i32** noundef nonnull align 8 dereferenceable(8) "intel_dtrans_func_index"="3" %B) {{.*}}!intel.dtrans.func.type ![[FOO_MD:[0-9]+]]
int *foo(int *const &A, int *const &B) {
// CHECK: alloca i32**, align 8, !intel_dtrans_type ![[INT_PTR_PTR:[0-9]+]]
// CHECK: alloca i32**, align 8, !intel_dtrans_type ![[INT_PTR_PTR]]
  return (int*)0;
}

// CHECK: define dso_local noundef i32 @main
int main() {
// CHECK: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR]]
// CHECK: alloca i32*, align 8, !intel_dtrans_type ![[INT_PTR]]
  (void)foo(&G1, &G2);
  return 0;
}

// CHECK: ![[BAR_MD]] = distinct !{![[INT_PTR]], ![[INT_PTR]]}
// CHECK: ![[INT_PTR]] = !{i32 0, i32 1}

// CHECK: ![[FOO_MD]] = distinct !{![[INT_PTR]], ![[INT_PTR_PTR]], ![[INT_PTR_PTR]]}
// CHECK: ![[INT_PTR_PTR]] = !{i32 0, i32 2}
