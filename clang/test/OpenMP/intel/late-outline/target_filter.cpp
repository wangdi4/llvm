// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

void goo(), moo(), moo2(), moo3(), another();
const int& min(const int& __a, const int& __b);

#pragma omp declare target
//CHECK: define {{.*}}boov() #[[DECLARE_TARGET:[0-9]+]]
void boo() { another(); }                 // declare-target
#pragma omp end declare target

//CHECK: define {{.*}}anotherv() #[[DECLARE_TARGET]]
void another() {}


//CHECK: define {{.*}}dorunv() #[[NOATTRS:[0-9]+]]
void dorun() {                // neither declare-target nor contains-target
       goo();
    []() {
       #pragma omp target
       { moo(); }
    }();
}

//CHECK: define {{.*}}ZZ5dorunvENK3$_0clEv{{.*}} #[[CONTAINS_TARGET:[0-9]+]]


//CHECK: define {{.*}}barv() #[[CONTAINS_TARGET]]
void bar() {                  // contains-target
       #pragma omp target
       moo2();
}

//CHECK: define {{.*}}moo2v() #[[DECLARE_TARGET]]
void moo2() { moo3(); }       // declare-target

//CHECK: define {{.*}}moo3v() #[[DECLARE_TARGET]]
void moo3() { }

//CHECK: define {{.*}}Compute{{.*}} #[[CONTAINS_TARGET]]
void Compute(int *A) {
  #pragma omp target parallel for map(tofrom: A)
  for (int i = 0; i < 10; i++)
    A[i] = min(A[i],5);
}

//CHECK: define {{.*}}min{{.*}} #[[DECLARE_TARGET]]
const int& min(const int& __a, const int& __b) {
  if (__b < __a) return __b;
  return __a;
}

//CHECK: define {{.*}}moov() #[[DECLARE_TARGET]]
void moo() { }                // declare-target

//CHECK: attributes #[[DECLARE_TARGET]] = {{.*}}"openmp-target-declare"="true"
//CHECK: attributes #[[NOATTRS]]
//CHECK: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"

// end INTEL_COLLAB
