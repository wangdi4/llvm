// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN: -mconstructor-aliases \
// RUN: -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

double zoo1(double a) {
 return a + 1.0;
}
class MyT {
public:
  double val;
  MyT();
};
#pragma omp declare target

// CHECK: define {{.*}}MyTC2Ev{{.*}}#[[DECLARE_TARGET:[0-9]+]]
MyT::MyT() {
 val = zoo1(0.0);
}
// CHECK: define {{.*}}zoo1{{.*}}#[[DECLARE_TARGET]]
// CHECK: define {{.*}}Comp{{.*}}#[[DECLARE_TARGET]]
MyT Comp(MyT a, MyT b) { return a.val < b.val ? a : b; }

#pragma omp declare reduction(Min : MyT : omp_out = Comp(omp_out, omp_in))
#pragma omp end declare target

// CHECK: define {{.*}}foo{{.*}}#[[CONTAINS:[0-9]+]]
void foo()
{
  MyT my;

#pragma omp parallel for reduction(Min:my)
  for(int i=0;i<8;++i) {}
#pragma omp target teams distribute parallel for reduction(Min:my)
  for(int i=0;i<8;++i) {}
}
// CHECK: define internal void @.omp_combiner{{.*}}#[[DECLARE_TARGET1:[0-9]+]]
// CHECK: define internal{{.*}}.omp.def_constr{{.*}}#[[DECLARE_TARGET1]]
// CHECK: define internal{{.*}}_ZTS3MyT.omp.destr{{.*}} #[[DECLARE_TARGET1]]

// CHECK: define {{.*}}_Z4initRi{{.*}}#[[DECLARE_TARGET]]
// CHECK: define {{.*}}_Z3bar{{.*}} #[[DECLARE_TARGET]]
// CHECK: define {{.*}}_Z3bar{{.*}} #[[DECLARE_TARGET]]
// CHECK: define {{.*}}zoo{{.*}}#[[CONTAINS]]
// CHECK: define internal void @.omp_combiner..1{{.*}}#[[DECLARE_TARGET1]]
// CHECK: define internal void @.omp_initializer.{{.*}}#[[DECLARE_TARGET1]]

// CHECK: attributes #[[DECLARE_TARGET]] = {{.*}}"openmp-target-declare"="true"
// CHECK: attributes #[[CONTAINS]] = {{.*}}"contains-openmp-target"="true"
// CHECK: attributes #[[DECLARE_TARGET1]] = {{.*}}"openmp-target-declare"="true"
#pragma omp declare target
template<int i>
void bar() { }
void init(int &x) { bar<1>(); bar<2>(); }
#pragma omp declare reduction(myadd : int : omp_out += omp_in) initializer (init(omp_priv = 0))
#pragma omp end declare target

void  zoo()
{
  int i;
  int sum_udr=0;

#pragma omp target teams distribute parallel for reduction(myadd:sum_udr)
  for (i=0; i<1000; i++) {
    sum_udr = sum_udr+i;
  }
}


// end INTEL_COLLAB
