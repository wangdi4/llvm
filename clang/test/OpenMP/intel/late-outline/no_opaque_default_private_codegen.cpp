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
#pragma omp parallel default(private)
    {
      targetDev++;
    }
  }
};

// CHECK-LABEL: define {{.*}}@_ZN10SomeKernel5applyILj32EEEvv
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[THIS_ADDR:%.*]] = alloca %struct.SomeKernel*, align 8
// CHECK-NEXT:    store %struct.SomeKernel* [[THIS:%.*]], %struct.SomeKernel** [[THIS_ADDR]], align 8
// CHECK-NEXT:    [[THIS1:%.*]] = load %struct.SomeKernel*, %struct.SomeKernel** [[THIS_ADDR]], align 8
// CHECK-NEXT:    [[TARGETDEV:%.*]] = getelementptr inbounds %struct.SomeKernel, %struct.SomeKernel* [[THIS1]]
// CHECK:          "DIR.OMP.PARALLEL"()
// CHECK-SAME:     "QUAL.OMP.DEFAULT.PRIVATE"()
// CHECK-SAME:     "QUAL.OMP.PRIVATE"(i32* [[TARGETDEV]]
// CHECK:          "DIR.OMP.END.PARALLEL"()
void use_template() {
  SomeKernel aKern;
  aKern.apply<32>();
}

// CHECK-LABEL: define {{.*}} @_Z3foov(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[A:%.*]] = alloca i32, align 4
// CHECK-NEXT:    "DIR.OMP.PARALLEL"()
// CHECK-SAME:    "QUAL.OMP.DEFAULT.PRIVATE"()
// CHECK-SAME:    "QUAL.OMP.PRIVATE"(i32* [[A]]
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
// CHECK-LABEL: define {{.*}} @_Z3barv(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[A:%.*]] = alloca [[STRUCT_ST:%.*]], align 4
// CHECK:         "DIR.OMP.PARALLEL"()
// CHECK-SAME     "QUAL.OMP.DEFAULT.PRIVATE"()
// CHECK-SAME     "QUAL.OMP.PRIVATE:NONPOD"(%struct.St* [[A]], %struct.St* (%struct.St*)* @_ZTS2St.omp.def_constr, void (%struct.St*)* @_ZTS2St.omp.destr)
// CHECK-SAME     "QUAL.OMP.PRIVATE"(i32* @_ZZ3barvE2yy) ]
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
