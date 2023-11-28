// INTEL_COLLAB

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
//RUN:   -std=c++14 -fexceptions -fcxx-exceptions -verify  \
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
#pragma omp parallel default(private)
    {
      targetDev++;
    }
  }
};

// CHECK-LABEL: define {{.*}}@_ZN10SomeKernel5applyILj32EEEvv
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[THIS_ADDR:%.*]] = alloca ptr, align 8
// CHECK-NEXT:    store ptr [[THIS:%.*]], ptr [[THIS_ADDR]], align 8
// CHECK-NEXT:    [[THIS1:%.*]] = load ptr, ptr [[THIS_ADDR]], align 8
// CHECK-NEXT:    [[TARGETDEV:%.*]] = getelementptr inbounds %struct.SomeKernel, ptr [[THIS1]]
// CHECK:          "DIR.OMP.PARALLEL"()
// CHECK-SAME:     "QUAL.OMP.DEFAULT.PRIVATE"()
// CHECK-SAME:     "QUAL.OMP.PRIVATE:TYPED"(ptr [[TARGETDEV]]
// CHECK:          "DIR.OMP.END.PARALLEL"()
void use_template() {
  SomeKernel aKern;
  aKern.apply<32>();
}

// CHECK-LABEL: {{.*}} @_Z3foov(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[A:%.*]] = alloca i32, align 4
// CHECK-NEXT:    "DIR.OMP.PARALLEL"()
// CHECK-SAME:    "QUAL.OMP.DEFAULT.PRIVATE"()
// CHECK-SAME:    "QUAL.OMP.PRIVATE:TYPED"(ptr [[A]]
// CHECK:         "DIR.OMP.END.PARALLEL"()
// CHECK-NEXT:    ret void
//
void foo() {
  int a;
#pragma omp parallel default(private)
  a++;
}

struct St {
  int a, b;
  int y;
  ~St() {}
};
// CHECK-LABEL: define {{.*}}@_Z3barv(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[A:%.*]] = alloca [[STRUCT_ST:%.*]], align 4
// CHECK:         "DIR.OMP.PARALLEL"()
// CHECK-SAME     "QUAL.OMP.DEFAULT.PRIVATE"()
// CHECK-SAME     "QUAL.OMP.PRIVATE:NONPOD"(ptr [[A]], ptr @_ZTS2St.omp.def_constr, ptr @_ZTS2St.omp.destr)
// CHECK-SAME     "QUAL.OMP.PRIVATE"(ptr @_ZZ3barvE2yy) ]
// CHECK:         "DIR.OMP.END.PARALLEL"()
//
void bar() {
  St a = St();
  static int yy = 0;
#pragma omp parallel default(private)
  {
    a.a += 1;
    a.b += 1;
    a.y++;
    yy++;
  }
}
// end INTEL_COLLAB
