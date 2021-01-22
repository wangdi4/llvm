// INTEL_COLLAB
// RUN: %clang_cc1 -O2 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -disable-llvm-passes -fintel-compatibility -fopenmp-late-outline \
// RUN:  -Wno-openmp-mapping -fopenmp-targets=spir64 -emit-llvm-bc %s \
// RUN:  -o %t-host.bc
//
// RUN: %clang_cc1 -O2 -verify -triple spir64 -fopenmp \
// RUN:  -disable-llvm-passes -fintel-compatibility -fopenmp-late-outline \
// RUN:  -Wno-openmp-mapping -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

struct A {
  int *h_array;
};

void foo(A *ap, int size) {
  int *theArray = ap->h_array;
  int theSize = size;
  // CHECK: [[A:%[0-9]+]] = addrspacecast {{.*}}%theArray.ascast
  // CHECK: call void @llvm.lifetime.start.p0i8(i64 8, i8* [[A]])
  // CHECK: [[B:%[0-9]+]] = addrspacecast {{.*}}%theSize.ascast
  // CHECK: call void @llvm.lifetime.start.p0i8(i64 4, i8* [[B]])
  // CHECK: DIR.OMP.TARGET
  #pragma omp target map(theArray[0:8]) map(theSize)
  {
    // CHECK: [[C:%[0-9]+]] = addrspacecast {{.*}}%i.ascast
    // CHECK: call void @llvm.lifetime.start.p0i8(i64 4, i8* [[C]])
    for (int i = 0; i < theSize; ++i)
      theArray[i] += 1;
  }
  // CHECK: [[CE:%[0-9]+]] = addrspacecast {{.*}}%i.ascast
  // CHECK: call void @llvm.lifetime.end.p0i8(i64 4, i8* [[CE]])
  // CHECK: DIR.OMP.END.TARGET

  // CHECK: [[BE:%[0-9]+]] = addrspacecast {{.*}}%theSize.ascast
  // CHECK: call void @llvm.lifetime.end.p0i8(i64 4, i8* [[BE]])
  // CHECK: [[AE:%[0-9]+]] = addrspacecast {{.*}}%theArray.ascast
  // CHECK: call void @llvm.lifetime.end.p0i8(i64 8, i8* [[AE]])
}

struct complex {
  double r,i;
  complex& operator=(const complex& __t);
};

complex operator+(const complex& __x, const complex& __y);

// CHECK-LABEL: test_target
void test_target() {
   // CHECK: [[C:%[0-9]+]] = addrspacecast {{.*}}%counter_target.ascast
   // CHECK: call void @llvm.lifetime.start.p0i8(i64 16, i8* [[C]])
  complex counter_target{};
  // CHECK: DIR.OMP.TARGET
  #pragma omp target map(tofrom: counter_target)
  {
    // CHECK: [[T1:%[0-9]+]] = addrspacecast {{.*}}%ref.tmp
    // CHECK: call void @llvm.lifetime.start.p0i8(i64 16, i8* [[T1]])
    // CHECK: [[T2:%[0-9]+]] = addrspacecast {{.*}}%ref.tmp
    // CHECK: call void @llvm.lifetime.start.p0i8(i64 16, i8* [[T2]])
    counter_target = counter_target + complex {  1. };
    // CHECK: [[T2E:%[0-9]+]] = addrspacecast {{.*}}%ref.tmp
    // CHECK: call void @llvm.lifetime.end.p0i8(i64 16, i8* [[T2E]])
    // CHECK: [[T1E:%[0-9]+]] = addrspacecast {{.*}}%ref.tmp
    // CHECK: call void @llvm.lifetime.end.p0i8(i64 16, i8* [[T1E]])
  }
  // CHECK: DIR.OMP.END.TARGET
  // CHECK: [[CE:%[0-9]+]] = addrspacecast {{.*}}%counter_target.ascast
  // CHECK: call void @llvm.lifetime.end.p0i8(i64 16, i8* [[CE]])
}

// end INTEL_COLLAB
