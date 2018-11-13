//RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility \
//RUN:   -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

int i;
__int128 j;
//CHECK: define{{.*}}foo
void foo()
{
  #pragma omp target
  {
    //CHECK: atomicrmw add{{.*}}monotonic
    #pragma omp atomic
    i++;

    //CHECK: [[TOK:%[0-9]+]] = call token @llvm.directive.region.entry()
    //CHECK-SAME: [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.UPDATE"() ]
    //CHECK: [[LD:%[0-9]+]] = load{{.*}}j
    //CHECK-NEXT: add{{.*}}[[LD]], 1
    //CHECK: call void @llvm.directive.region.exit(token [[TOK]])
    //CHECK-SAME: [ "DIR.OMP.END.ATOMIC"() ]
    #pragma omp atomic
    j++;
  }
}
