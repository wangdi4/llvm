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
void boo() { another(); }                 // declare-target
#pragma omp end declare target

void another() {}


void dorun() {                // neither declare-target nor contains-target
       goo();
    []() {
       #pragma omp target
       { moo(); }
    }();
}



void bar() {                  // contains-target
       #pragma omp target
       moo2();
}

void moo2() { moo3(); }       // declare-target

void moo3() { }

void Compute(int *A) {
  #pragma omp target parallel for map(tofrom: A)
  for (int i = 0; i < 10; i++)
    A[i] = min(A[i],5);
}

const int& min(const int& __a, const int& __b) {
  if (__b < __a) return __b;
  return __a;
}

void moo() { }                // declare-target

//CHECK: define {{.*}}dorunv() #[[NOATTRS:[0-9]+]]
//CHECK: define {{.*}}barv() #[[CONTAINS_TARGET:[0-9]+]]
//CHECK: define {{.*}}Compute{{.*}} #[[CONTAINS_TARGET]]
//CHECK: define {{.*}}boov() #[[DECLARE_TARGET:[0-9]+]]
//CHECK: define {{.*}}anotherv() #[[DECLARE_TARGET]]
//CHECK: define {{.*}}ZZ5dorunvENKUlvE_clEv{{.*}} #[[CONTAINS_TARGET]]
//CHECK: define {{.*}}moov() #[[DECLARE_TARGET]]
//CHECK: define {{.*}}moo2v() #[[DECLARE_TARGET]]
//CHECK: define {{.*}}moo3v() #[[DECLARE_TARGET]]
//CHECK: define {{.*}}min{{.*}} #[[DECLARE_TARGET]]

//CHECK: attributes #[[NOATTRS]]
//CHECK: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"
//CHECK: attributes #[[DECLARE_TARGET]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
