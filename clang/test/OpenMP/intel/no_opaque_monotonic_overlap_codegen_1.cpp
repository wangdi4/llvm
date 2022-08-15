// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -std=c++11 -fopenmp-late-outline \
// RUN:  -no-opaque-pointers -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// expected-no-diagnostics

struct S {
  int a;
  S();
  ~S();
  template <int t>
  int apply() {
#pragma omp simd
    for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_monotonic(a:t)
      a++;
    }
#pragma omp simd
    for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(a++)
      a++;
    }
    return 10;
  }
};
template<typename T>
void foo(T x) {
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_monotonic(x:10)
    x++;
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(x)
    x++;
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(10)
    x++;
  }
#pragma omp simd
  for (int i = 0; i < 1024; i++) {
#pragma omp ordered simd ompx_overlap(i)
    x++;
  }
}
// CHECK-LABEL: @_Z12use_templatev(
// CHECK-NEXT:  entry:
// CHECK:    call void @_Z3fooIiEvT_(i32 noundef 10)
// CHECK-NEXT: call void @_ZN1SD1Ev
// CHECK-NEXT:    ret void
//
void use_template() {
   S obj;
   obj.apply<10>();
   foo(10);
}
// CHECK: define {{.*}}@_ZN1S5applyILi10EEEiv(
// CHECK: "DIR.OMP.SIMD"
// CHECK: "DIR.OMP.ORDERED"
// CHECK: "QUAL.OMP.MONOTONIC"(i32* %a, i32 10)
// CHECK: "DIR.OMP.END.ORDERED"
// CHECK: "DIR.OMP.END.SIMD"
// CHECK: "DIR.OMP.SIMD"
// CHECK: [[L11:%11]] = load i32, i32* %a13
// CHECK: "DIR.OMP.ORDERED"
// CHECK: "QUAL.OMP.OVERLAP"(i32 [[L11]]
// CHECK: "DIR.OMP.END.ORDERED"
// CHECK: "DIR.OMP.END.SIMD"

// CHECK: define {{.*}}@_Z3fooIiEvT_(
// CHECK: "DIR.OMP.SIMD"
// CHECK: "DIR.OMP.ORDERED"
// CHECK-SAME: "QUAL.OMP.ORDERED.SIMD"
// CHECK-SAME: "QUAL.OMP.MONOTONIC"(i32* %x.addr, i32 10)
// CHECK: "DIR.OMP.END.ORDERED"
// CHECK: "DIR.OMP.END.SIMD"
// CHECK: "DIR.OMP.SIMD"
// CHECK: "DIR.OMP.ORDERED"
// CHECK: "QUAL.OMP.OVERLAP"(i32 10)
// CHECK: "DIR.OMP.END.ORDERED"
// CHECK: "DIR.OMP.END.SIMD"
// CHECK: "DIR.OMP.SIMD"
// CHECK: [[L26:%26]] = load i32, i32* %i38
// CHECK: "DIR.OMP.ORDERED"
// CHECK: "QUAL.OMP.OVERLAP"(i32 [[L26]])
// CHECK: "DIR.OMP.END.ORDERED"
// CHECK: "DIR.OMP.END.SIMD"

