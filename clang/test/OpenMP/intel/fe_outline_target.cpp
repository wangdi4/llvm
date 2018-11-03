//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility \
//RUN:   -fintel-openmp-region -triple csa %s \
//RUN:   | FileCheck %s

//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility \
//RUN:   -fintel-openmp-region -fno-intel-openmp-offload -triple csa %s \
//RUN:   | FileCheck %s -check-prefix=CHECK-FEOUTLINE

int glob = 1;
//CHECK-LABEL: foo
//CHECK-FEOUTLINE-LABEL: foo
void foo() {

  //CHECK: [[I:%i[0-9]*]] = alloca i32, align
  //CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //CHECK-SAME: [ "DIR.OMP.TARGET"()
  //CHECK: [[L0:%[0-9]+]] = load i32, i32* @glob, align
  //CHECK: store i32 [[L0]], i32* [[I]], align
  //CHECK: call void @llvm.directive.region.exit(token [[T0]])
  //CHECK-SAME: [ "DIR.OMP.END.TARGET"() ]

  //CHECK-FEOUTLINE: define internal void @__omp_offloading{{.*}}foo
  //CHECK-FEOUTLINE: [[I:%i[0-9]*]] = alloca i32, align
  //CHECK-FEOUTLINE-NOT: @glob
  //CHECK-FEOUTLINE: store i32 %0, i32* [[I]], align
  //CHECK-FEOUTLINE-NEXT: ret void

  #pragma omp target
  {
    int i = glob;
  }
}

extern int **ext_glob_array[10];
//CHECK-FEOUTLINE-LABEL: bar
void bar() {

  #pragma omp target map(ext_glob_array[0:1])
  {
    #pragma omp parallel for
    for(int j=0; j<100; j++) {
      int ifoo = *ext_glob_array[1][j];
    }
  }
  //CHECK-FEOUTLINE: define internal void @__omp_offloading{{.*}}bar
  //CHECK-FEOUTLINE: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //CHECK-FEOUTLINE-SAME: [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-FEOUTLINE-NOT: @ext_glob_array
  //CHECK-FEOUTLINE: call void @llvm.directive.region.exit(token [[T0]])
  //CHECK-FEOUTLINE-SAME: [ "DIR.OMP.END.PARALLEL.LOOP"() ]
}
