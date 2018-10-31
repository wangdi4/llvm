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

  //CHECK-FEOUTLINE: define internal void @__omp_offloading
  //CHECK-FEOUTLINE: [[I:%i[0-9]*]] = alloca i32, align
  //CHECK-FEOUTLINE-NOT: @glob
  //CHECK-FEOUTLINE: store i32 %0, i32* [[I]], align
  //CHECK-FEOUTLINE-NEXT: ret void

  #pragma omp target
  {
    int i = glob;
  }
}
