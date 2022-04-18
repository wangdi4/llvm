//RUN: %clang_cc1 -no-opaque-pointers -std=c++14 -fopenmp \
//RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu \
//RUN:   -emit-llvm -o - %s | FileCheck %s

//RUN: %clang_cc1 -no-opaque-pointers -std=c++14 -fopenmp \
//RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu \
//RUN:   -emit-llvm-bc -o %t_host.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -std=c++14 -fopenmp \
//RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
//RUN:   -triple spir64 -fopenmp-is-device \
//RUN:   -fopenmp-host-ir-file-path %t_host.bc \
//RUN:   -emit-llvm -o - %s | FileCheck %s


//CHECK: define {{.*}}test_imperfect_nest
void test_imperfect_nest(const double *A, double *B)
{
  //CHECK: [[LB1:%.omp.uncollapsed.lb.*]] = alloca i32,
  //CHECK: [[UB1:%.omp.uncollapsed.ub.*]] = alloca i32,
  //CHECK: [[LB2:%.omp.uncollapsed.lb.*]] = alloca i32,
  //CHECK: [[UB2:%.omp.uncollapsed.ub.*]] = alloca i32,
  //CHECK: [[IV1:%.omp.uncollapsed.iv.*]] = alloca i32,
  //CHECK: [[IV2:%.omp.uncollapsed.iv.*]] = alloca i32,
  //CHECK: [[SI:%sourceIndex.*]] = alloca i32,
  //CHECK: [[DI:%destinationIndex.*]] = alloca i32,
  //CHECK: [[OI:%otherIndex.*]] = alloca i32,
  //
  //CHECK: "DIR.OMP.TARGET"(),
  //CHECK: "DIR.OMP.PARALLEL.LOOP"(),
  #pragma omp target
  #pragma omp parallel for collapse(2)
  for(int k=0;k<5;k++)
  {
    //CHECK: store i32 27, i32{{.*}} [[SI]]
    int sourceIndex      = 27;
    //CHECK: store i32 42, i32{{.*}} [[DI]]
    int destinationIndex = 42;
    for(int l=0;l<5;l++)
    {
      int otherIndex = 86;
      //CHECK: store i32 86, i32{{.*}} [[OI]]
      B[destinationIndex] = A[sourceIndex+otherIndex];
    }
  }
  //CHECK: "DIR.OMP.END.PARALLEL.LOOP"()
  //CHECK: "DIR.OMP.END.TARGET"()
}
