// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s

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

  // CHECK: [[LY:%[0-9]+]] = load i32*, i32** [[Y_ADDR]], align 8
  // CHECK: [[LZ:%[0-9]+]] = load i32*, i32** [[Z_ADDR]], align 8
  // CHECK: [[LX:%[0-9]+]] = load i32*, i32** [[X_ADDR]], align 8
  // CHECK: [[LQ:%[0-9]+]] = load i32*, i32** [[Q_ADDR]], align 8
  // CHECK: region.entry() [ "DIR.OMP.SIMD"()
  // CHECK-SAME: "QUAL.OMP.SAFELEN"(i32 4)
  // CHECK-SAME: "QUAL.OMP.SIMDLEN"(i32 4)
  // CHECK-SAME: "QUAL.OMP.COLLAPSE"(i32 2)
  // CHECK-SAME: "QUAL.OMP.ALIGNED"(i32* [[LY]], i32* [[LZ]], i32 8)
  // CHECK-SAME: "QUAL.OMP.ALIGNED"(i32* [[LX]], i32 4)
  // CHECK-SAME: "QUAL.OMP.ALIGNED"(i32* [[LQ]], i32 0)
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE"(double* [[SIMPLE_ADDR]])
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE:NONPOD"(%struct.A* @obj,
  // CHECK-SAME: %struct.A* (%struct.A*)* @_ZTS1A.omp.def_constr,
  // CHECK-SAME: void (%struct.A*, %struct.A*)* @_ZTS1A.omp.copy_assign,
  // CHECK-SAME: void (%struct.A*)* @_ZTS1A.omp.destr)
  // CHECK-SAME: QUAL.OMP.LASTPRIVATE:NONPOD"([4 x %struct.A]* @objarr,
  // CHECK-SAME: [4 x %struct.A]* ([4 x %struct.A]*)*
  // CHECK-SAME: @_ZTSA4_1A.omp.def_constr, void ([4 x %struct.A]*,
  // CHECK-SAME: [4 x %struct.A]*)* @_ZTSA4_1A.omp.copy_assign,
  // CHECK-SAME: void ([4 x %struct.A]*)* @_ZTSA4_1A.omp.destr)
  // CHECK: region.exit{{.*}}"DIR.OMP.END.SIMD"()
  #pragma omp simd safelen(4) simdlen(4) collapse(2) \
            aligned(y,z:8) aligned(x:4) aligned(q) \
            lastprivate(simple) lastprivate(obj) lastprivate(objarr)
  for (i=0;i<10;++i)
  for (j=0;j<10;++j) {}
}

// CHECK: define internal void @_ZTS1A.omp.copy_assign
// CHECK: define internal void @_ZTSA4_1A.omp.copy_assign
// end INTEL_COLLAB
