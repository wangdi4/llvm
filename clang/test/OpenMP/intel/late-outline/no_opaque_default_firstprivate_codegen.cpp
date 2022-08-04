// INTEL_COLLAB

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -no-opaque-pointers -std=c++14 -fexceptions -fcxx-exceptions -verify \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses -fopenmp-late-outline \
//RUN:   -x c++  -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK

// expected-no-diagnostics

struct SomeKernel {
  int targetDev;
  float devPtr;
  SomeKernel();
  ~SomeKernel();

  template <unsigned int nRHS>
  void apply() {
#pragma omp parallel default(firstprivate)
    {
      targetDev++;
    }
  }
};

// CHECK-LABEL: define {{.*}}@_ZN10SomeKernel5applyILj32EEEvv
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[THIS_ADDR:%.*]] = alloca %struct.SomeKernel*, align 8
// CHECK:         store %struct.SomeKernel* [[THIS:%.*]], %struct.SomeKernel** [[THIS_ADDR]], align 8
// CHECK-NEXT:    [[THIS1:%.*]] = load %struct.SomeKernel*, %struct.SomeKernel** [[THIS_ADDR]], align 8
// CHECK:          "DIR.OMP.PARALLEL"()
// CHECK-SAME:     "QUAL.OMP.DEFAULT.FIRSTPRIVATE"
// CHECK-SAME:     "QUAL.OMP.FIRSTPRIVATE"(i32* %targetDev3)
// CHECK:          "DIR.OMP.END.PARALLEL"()
void use_template() {
  SomeKernel aKern;
  aKern.apply<32>();
}

// CHECK-LABEL: define {{.*}} @_Z3foov(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[A:%.*]] = alloca i32, align 4
// CHECK-NEXT:    "DIR.OMP.PARALLEL"()
// CHECK-SAME:    "QUAL.OMP.DEFAULT.FIRSTPRIVATE"()
// CHECK-SAME:    "QUAL.OMP.FIRSTPRIVATE"(i32* [[A]]
// CHECK:         "DIR.OMP.END.PARALLEL"()
// CHECK-NEXT:    ret void
//
void foo() {
  int a;
#pragma omp parallel default(firstprivate)
  a++;
}

struct St {
  int a, b;
  static int y;
  ~St() {}
};
// CHECK-LABEL: define {{.*}} @_Z3barv(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[A:%.*]] = alloca [[STRUCT_ST:%.*]], align 4
// CHECK:         DIR.OMP.PARALLEL
// CHECK-SAME     QUAL.OMP.DEFAULT.FIRSTPRIVATE
// CHECK-SAME     "QUAL.OMP.FIRSTPRIVATE"(i32* @_ZN2St1yE)
// CHECK-SAME     "QUAL.OMP.FIRSTPRIVATE"(%struct.St* [[A]])
// CHECK-SAME     "QUAL.OMP.FIRSTPRIVATE"(i32* @_ZZ3barvE2yy) ]
// CHECK:         "DIR.OMP.END.PARALLEL"
//
void bar() {
  St a = St();
  static int yy = 0;
#pragma omp parallel default(firstprivate)
  {
    a.a += 1;
    a.b += 1;
    a.y++;
    yy++;
  }
}

// CHECK-LABEL: define {{.*}}@_Z5test1s
void test1(short A) {
  short *Ap = &A;
// CHECK: [[A:%A.addr]] = alloca i16
// CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"
// CHECK-SAME: "QUAL.OMP.DEFAULT.FIRSTPRIVATE"
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i16* [[A]])
// CHECK: DIR.OMP.END.DISTRIBUTE.PARLOOP"
  #pragma omp distribute parallel for shared(Ap) default(firstprivate)
  for (int i = 0; i < 10; i++)
  {
    A = 222;
    if (&A == Ap) A++;
  }
// CHECK: "DIR.OMP.TARGET"()
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i16* [[A]])
// CHECK: "DIR.OMP.PARALLEL.LOOP"
// CHECK-SAME: "QUAL.OMP.DEFAULT.FIRSTPRIVATE"()
// CHECK-SAME: "QUAL.OMP.SHARED"(i16* [[A]])
// CHECK: "DIR.OMP.END.PARALLEL.LOOP"
// CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target parallel for shared(Ap) default(firstprivate)
  for (int i = 0; i < 10; i++)
  {
    A = 222;
    if (&A == Ap) A++;
  }
// CHECK: "DIR.OMP.PARALLEL.LOOP"
// CHECK: "QUAL.OMP.DEFAULT.FIRSTPRIVATE"
// CHECK: "QUAL.OMP.FIRSTPRIVATE"(i16* [[A]])
// CHECK: "DIR.OMP.SIMD"()
// CHECK: "DIR.OMP.END.SIMD"
// CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for simd shared(Ap) default(firstprivate)
  for (int i = 0; i < 10; i++)
  {
    A = 222;
    if (&A == Ap) A++;
  }
// CHECK: "DIR.OMP.TEAMS"
// CHECK-SAME: "QUAL.OMP.DEFAULT.FIRSTPRIVATE"
// CHECK-SAME: "QUAL.OMP.SHARED"(i16* [[A]])
// CHECK: "DIR.OMP.DISTRIBUTE"
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i16* [[A]]
// CHECK: "DIR.OMP.END.DISTRIBUTE"
// CHECK: "DIR.OMP.END.TEAMS"
  #pragma omp teams distribute shared(Ap) default(firstprivate)
  for (int i = 0; i < 10; i++)
  {
    A = 222;
    if (&A == Ap) A++;
  }
// CHECK: "DIR.OMP.TEAMS"()
// CHECK-SAME: "QUAL.OMP.DEFAULT.FIRSTPRIVATE"
// CHECK-SAME: "QUAL.OMP.SHARED"(i16* [[A]])
// CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i16* [[A]]
// CHECK: "DIR.OMP.END.DISTRIBUTE.PARLOOP"
// CHECK: "DIR.OMP.END.TEAMS"
  #pragma omp teams distribute parallel for shared(Ap) default(firstprivate)
  for (int i = 0; i < 10; i++)
  {
    A = 222;
    if (&A == Ap) A++;
  }
// CHECK: "DIR.OMP.TARGET"
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i16* [[A]]
// CHECK: "DIR.OMP.TEAMS"
// CHECK-SAME: "QUAL.OMP.DEFAULT.FIRSTPRIVATE"
// CHECK-SAME: "QUAL.OMP.SHARED"(i16* [[A]])
// CHECK: "DIR.OMP.DISTRIBUTE"
// CHECK: "DIR.OMP.END.DISTRIBUTE"
// CHECK: "DIR.OMP.END.TEAMS"
// CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target teams distribute shared(Ap) default(firstprivate)
  for (int i = 0; i < 10; i++)
  {
    A = 222;
    if (&A == Ap) A++;
  }
// CHECK: "DIR.OMP.TEAMS"
// CHECK-SAME: "QUAL.OMP.DEFAULT.FIRSTPRIVATE"
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i16* [[A]])
// CHECK:"DIR.OMP.GENERICLOOP"
// CHECK-SAME: "QUAL.OMP.SHARED"(i16* [[A]])
// CHECK: "DIR.OMP.END.GENERICLOOP"
// CHECK: "DIR.OMP.END.TEAMS"
  #pragma omp teams loop shared(Ap) default(firstprivate)
  for (int i = 0; i < 10; i++)
  {
    A = 222;
    if (&A == Ap) A++;
  }
}
// end INTEL_COLLAB
