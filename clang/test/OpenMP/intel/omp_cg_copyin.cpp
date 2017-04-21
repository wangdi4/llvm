// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -fopenmp-threadprivate-legacy -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix CHECK-1

void foo() {}

int     afoo;
#pragma omp threadprivate(afoo)

// CHECK-1: @afoo {{.*}}thread_private 
// CHECK-NOT: @afoo {{.*}}thread_private 
// CHECK-LABEL: @main
int main(int argc, char **argv) {
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.COPYIN", i32*
// CHECK-1: "QUAL.OMP.COPYIN", i32* @afoo
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel copyin(afoo)
// CHECK: call void @_Z3foov()
  foo();
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

  return 0;
}
