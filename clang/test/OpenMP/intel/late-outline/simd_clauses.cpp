// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -fopenmp-version=50 -DOMP50 -triple x86_64-unknown-linux-gnu %s \
// RUN: | FileCheck %s --check-prefixes=CHECK,CHECK50

struct A {
  A();
  A(const A&);
  A& operator=(const A&);
  ~A();
  int *ip;
} obj, objarr[4];

// CHECK-LABEL: @_Z3foov
void foo()
{
  // CHECK: [[I_ADDR:%.+]] = alloca i32,
  // CHECK: [[J_ADDR:%.+]] = alloca i32,
  // CHECK: [[Y_ADDR:%.+]] = alloca ptr,
  // CHECK: [[Z_ADDR:%.+]] = alloca ptr,
  // CHECK: [[X_ADDR:%.+]] = alloca ptr,
  // CHECK: [[Q_ADDR:%.+]] = alloca ptr,
  // CHECK: [[SIMPLE_ADDR:%.+]] = alloca double,
  int i,j;
  int *y,*z,*x, *q;
  double simple;

  // CHECK: region.entry() [ "DIR.OMP.SIMD"()
  // CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
  // CHECK-SAME: "QUAL.OMP.SIMDLEN"(i32 4)
  // CHECK-SAME: "QUAL.OMP.COLLAPSE"(i32 2)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr [[Y_ADDR]], i32 8)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr [[Z_ADDR]], i32 8)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr [[X_ADDR]], i32 4)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(ptr [[Q_ADDR]], i32 0)
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE"(ptr [[SIMPLE_ADDR]])
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE:NONPOD"(ptr @obj,
  // CHECK-SAME: ptr @_ZTS1A.omp.def_constr,
  // CHECK-SAME: ptr @_ZTS1A.omp.copy_assign,
  // CHECK-SAME: ptr @_ZTS1A.omp.destr)
  // CHECK-SAME: QUAL.OMP.LASTPRIVATE:NONPOD"(ptr @objarr,
  // CHECK-SAME: ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.copy_assign,
  // CHECK-SAME: ptr @_ZTS1A.omp.destr)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.SIMD"()
  #pragma omp simd safelen(4) simdlen(4) collapse(2) \
            aligned(y,z:8) aligned(x:4) aligned(q) \
            lastprivate(simple) lastprivate(obj) lastprivate(objarr)
  for (i=0;i<10;++i)
  for (j=0;j<10;++j) {}
}

// CHECK: define internal void @_ZTS1A.omp.copy_assign
// CHECK-NOT: define internal void @_ZTSA4_1A.omp.copy_assign

#ifdef OMP50
struct S {
  int a, b;
};

//CHECK50-LABEL: simple_nontemporal
void simple_nontemporal(float *a, float *b) {
  //CHECK50: [[A:%a.*]] = alloca ptr,
  //CHECK50: [[B:%b.*]] = alloca ptr,
  //CHECK50: [[S:%s.*]] = alloca %struct.S,
  S s, *p = &s;

  //CHECK50: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr [[A]]
  //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr [[B]]
  //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL"(ptr [[S]]
  //CHECK50: region.exit{{.*}}"DIR.OMP.END.SIMD"()
  #pragma omp simd nontemporal(a, b) nontemporal(s)
  for (int i = 3; i < 32; i += 5) {
    a[i] = b[i] + s.a + p->a;
  }
}

struct NT_test {
  int a;
  int &ar;
  NT_test() : ar(a) {}
  //CHECK50-LABEL: member
  void member() {
    //CHECK50: [[A:%a.*]] = getelementptr {{.*}}i32 0, i32 0
    //CHECK50: [[AR:%ar.*]] = getelementptr {{.*}}i32 0, i32 1
    //CHECK50-NEXT: [[LAR:%[0-9].*]] = load ptr, ptr [[AR]],
    //CHECK50: region.entry() [ "DIR.OMP.SIMD"()
    //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL"(ptr [[A]])
    //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL"(ptr [[LAR]])
    //CHECK50: region.exit{{.*}}"DIR.OMP.END.SIMD"()
    #pragma omp simd nontemporal(a) nontemporal(ar)
    for (int i = 0; i < 32; i += 4) {
      a += i; ar += i;
    }
  }
};
void callit()
{
  NT_test n;  n.member();
}
#endif
// end INTEL_COLLAB
