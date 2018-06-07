//RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility \
//RUN:   -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

//RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility \
//RUN:   -fintel-openmp-region -triple csa \
//RUN:   | FileCheck %s -check-prefix=CHECK-CSA

int i;
__int128 j;
//CHECK: define{{.*}}foo
void foo()
{
  #pragma omp target
  {
    //CHECK: atomicrmw add{{.*}}monotonic
    //CHECK-CSA: atomicrmw add{{.*}}monotonic
    #pragma omp atomic
    i++;

    //CHECK: [[TOK:%[0-9]+]] = call token @llvm.directive.region.entry()
    //CHECK-SAME: [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
    //CHECK: [[LD:%[0-9]+]] = load{{.*}}j
    //CHECK-NEXT: add{{.*}}[[LD]], 1
    //CHECK: call void @llvm.directive.region.exit(token [[TOK]])
    //CHECK-SAME: [ "DIR.OMP.END.ATOMIC"() ]
    //CHECK-NOT-CSA: "DIR.OMP.ATOMIC"()
    #pragma omp atomic
    j++;
  }
}
