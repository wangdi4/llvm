// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
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
  // CHECK: [[Y_ADDR:%.+]] = alloca i32*,
  // CHECK: [[Z_ADDR:%.+]] = alloca i32*,
  // CHECK: [[X_ADDR:%.+]] = alloca i32*,
  // CHECK: [[Q_ADDR:%.+]] = alloca i32*,
  // CHECK: [[SIMPLE_ADDR:%.+]] = alloca double,
  int i,j;
  int *y,*z,*x, *q;
  double simple;

  // CHECK: region.entry() [ "DIR.OMP.SIMD"()
  // CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
  // CHECK-SAME: "QUAL.OMP.SIMDLEN"(i32 4)
  // CHECK-SAME: "QUAL.OMP.COLLAPSE"(i32 2)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(i32** [[Y_ADDR]], i32 8)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(i32** [[Z_ADDR]], i32 8)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(i32** [[X_ADDR]], i32 4)
  // CHECK-SAME: "QUAL.OMP.ALIGNED:PTR_TO_PTR"(i32** [[Q_ADDR]], i32 0)
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE"(double* [[SIMPLE_ADDR]])
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE:NONPOD"(%struct.A* @obj,
  // CHECK-SAME: %struct.A* (%struct.A*)* @_ZTS1A.omp.def_constr,
  // CHECK-SAME: void (%struct.A*, %struct.A*)* @_ZTS1A.omp.copy_assign,
  // CHECK-SAME: void (%struct.A*)* @_ZTS1A.omp.destr)
  // CHECK-SAME: QUAL.OMP.LASTPRIVATE:NONPOD"([4 x %struct.A]* @objarr,
  // CHECK-SAME: %struct.A* (%struct.A*)*
  // CHECK-SAME: @_ZTS1A.omp.def_constr, void (%struct.A*,
  // CHECK-SAME: %struct.A*)* @_ZTS1A.omp.copy_assign,
  // CHECK-SAME: void (%struct.A*)* @_ZTS1A.omp.destr)
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
  //CHECK50: [[A:%a.*]] = alloca float*,
  //CHECK50: [[B:%b.*]] = alloca float*,
  //CHECK50: [[S:%s.*]] = alloca %struct.S,
  S s, *p = &s;

  //CHECK50: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(float** [[A]]
  //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(float** [[B]]
  //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL"(%struct.S* [[S]]
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
    //CHECK50-NEXT: [[LAR:%[0-9].*]] = load i32*, i32** [[AR]],
    //CHECK50: region.entry() [ "DIR.OMP.SIMD"()
    //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL"(i32* [[A]])
    //CHECK50-SAME: "QUAL.OMP.NONTEMPORAL"(i32* [[LAR]])
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
