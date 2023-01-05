// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm %s -o - | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s

template <class FunctorTy>
struct Closure {
  const FunctorTy m_functor;
  Closure(const FunctorTy &arg_functor) : m_functor(arg_functor) {}

  void execute() const {
    FunctorTy a_functor(m_functor);
    #pragma omp target teams distribute parallel for map(to : a_functor)
    for (int i = 0; i < 16; i++)
      a_functor(i);
  }
};

template <class FunctorTy>
void bar(const FunctorTy &functor) {
  Closure<FunctorTy> closure(functor);
  closure.execute();
}

template <typename Functor>
void not_used_functor_wrapper(Functor f) { }

void not_used() {
  not_used_functor_wrapper([=] (int i) { });
}

void foo() {
  bar([=] (int i) { });
  bar([=] (int i) { });
}

//CHECK: define {{.*}}_ZNK7ClosureIZ3foovEUliE_E7executeEv
//CHECK: "DIR.OMP.TARGET"()
//CHECK: "DIR.OMP.END.TARGET"()
//CHECK: define {{.*}}_ZNK7ClosureIZ3foovEUliE0_E7executeEv
//CHECK: "DIR.OMP.TARGET"()
//CHECK: "DIR.OMP.END.TARGET"()

//CHECK: !omp_offload.info = !{!{{[0-9]+}}, !{{[0-9]+}}}
//CHECK-DAG: !{{[0-9]+}} = !{{{.*}}!"_ZNK7ClosureIZ3foovEUliE_E7executeEv",{{.*}}}
//CHECK-DAG: !{{[0-9]+}} = !{{{.*}}!"_ZNK7ClosureIZ3foovEUliE0_E7executeEv",{{.*}}}

// end INTEL_COLLAB
