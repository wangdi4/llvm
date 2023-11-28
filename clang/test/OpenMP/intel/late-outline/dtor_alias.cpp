// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp  \
//RUN:  -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -O2 -disable-llvm-passes                             \
//RUN:  -mconstructor-aliases %s -emit-llvm-bc -o %t-host.bc

//RUN: %clang_cc1 -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc      \
//RUN:  -O2 -disable-llvm-passes                                      \
//RUN:  -mconstructor-aliases %s -emit-llvm -o - | FileCheck %s

//RUN: %clang_cc1 -triple x86_64 -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc      \
//RUN:  -O2 -disable-llvm-passes                                      \
//RUN:  -mconstructor-aliases %s -emit-llvm -o - | FileCheck %s

// Verifies a linkable result when complete destructor is emitted as an
// alias.

void another() { }

struct foo{ ~foo();  };
foo::~foo() { another(); }

template <class FunctorTy>
void execute(const FunctorTy &functor) {

  #pragma omp target teams distribute parallel for
  for (int i = 0; i < 1; ++i) {
    functor(i);
  }
}

void func() {
  execute( [=]( int i ) { foo bar; });
}

//CHECK-DAG: @_ZN3fooD1Ev = {{.*}}alias {{.*}}@_ZN3fooD2Ev
//CHECK-DAG: define {{.*}}_Z7executeIZ4funcvEUliE_EvRKT_{{.*}}#[[CONTAINS:[0-9]]]
//CHECK-DAG: define {{.*}}_ZZ4funcvENKUliE_clEi{{.*}}#[[DECLTARG1:[0-9]]]
//CHECK-DAG: define {{.*}}_ZN3fooD2Ev{{.*}}#[[DECLTARG2:[0-9]]]
//CHECK-DAG: define {{.*}}_Z7anotherv{{.*}}#[[DECLTARG2:[0-9]]]

//CHECK-DAG: attributes #[[CONTAINS]] = {{.*}}"contains-openmp-target"="true"
//CHECK-DAG: attributes #[[DECLTARG1]] = {{.*}}"openmp-target-declare"="true"
//CHECK-DAG: attributes #[[DECLTARG2]] = {{.*}}"openmp-target-declare"="true"
// end INTEL_COLLAB
