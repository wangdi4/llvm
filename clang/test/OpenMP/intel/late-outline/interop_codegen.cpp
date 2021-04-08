// INTEL_COLLAB
// RUN: %clang_cc1 -I%S/Inputs -emit-pch -o %t -std=c++14 -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s

// RUN: %clang_cc1 -I%S/Inputs -emit-llvm -o - -std=c++14 -fopenmp \
// RUN:  -fopenmp-late-outline -include-pch %t -verify \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// expected-no-diagnostics
#ifndef HEADER
#define HEADER

#include <omp.h>

//CHECK-LABEL: foo1
void foo1(int dev) {
  int arr[10] = { 0 };

  //CHECK: [[O1:%obj1.*]] = alloca [[T:.*]],
  //CHECK: [[O2:%obj2.*]] = alloca [[T]],
  //CHECK: [[O3:%obj3.*]] = alloca [[T]],
  //CHECK: [[O4:%obj4.*]] = alloca [[T]],
  //CHECK: [[O5:%obj5.*]] = alloca [[T]],
  omp_interop_t obj1, obj2, obj3, obj4, obj5;

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.INIT:TARGETSYNC"([[T]]* [[O1]])
  //CHECK-SAME: QUAL.OMP.DEPEND
  //CHECK-SAME: QUAL.OMP.DEVICE
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop init(targetsync:obj1) depend(inout:arr) device(dev)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.USE"([[T]]* [[O1]])
  //CHECK-SAME: QUAL.OMP.NOWAIT
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop use(obj1) nowait

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.USE"([[T]]* [[O1]])
  //CHECK-SAME: QUAL.OMP.DEPEND
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop use(obj1) depend(inout:arr)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.DESTROY"([[T]]* [[O1]])
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop destroy(obj1)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.DESTROY"([[T]]* [[O1]])
  //CHECK-SAME: QUAL.OMP.DEPEND
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop destroy(obj1) depend(inout:arr)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.INIT:TARGET"([[T]]* [[O2]])
  //CHECK-SAME: QUAL.OMP.DEVICE
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop init(target:obj2) device(dev + 1)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.DESTROY"([[T]]* [[O2]])
  //CHECK-SAME: QUAL.OMP.NOWAIT
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop destroy(obj2) nowait

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.INIT:TARGET.TARGETSYNC
  //CHECK-SAME: .PREFER"([[T]]* [[O3]], i64 3, i64 4, i64 6)
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop init(prefer_type(3,"sycl",6),target,targetsync:obj3)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.INIT:TARGET.TARGETSYNC
  //CHECK-SAME: .PREFER"([[T]]* [[O4]], i64 3, i64 4, i64 6)
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop \
      init(prefer_type("opencl",4,"level_zero"),target,targetsync:obj4)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.INIT:TARGET.TARGETSYNC
  //CHECK-SAME: .PREFER"([[T]]* [[O5]], i64 6, i64 4, i64 3)
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop init(prefer_type(6,"sycl",3),target,targetsync:obj5)

  //CHECK: DIR.OMP.INTEROP
  //CHECK-SAME: QUAL.OMP.INIT:TARGET.TARGETSYNC
  //CHECK-SAME: .PREFER"([[T]]* [[O5]], i64 6, i64 4, i64 3)
  //CHECK: DIR.OMP.END.INTEROP
  #pragma omp interop \
    init(prefer_type(6,12,"sycl","foobar",3),target,targetsync:obj5)
}
#endif // HEADER
// end INTEL_COLLAB
