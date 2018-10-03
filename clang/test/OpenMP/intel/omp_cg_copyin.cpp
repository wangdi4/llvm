// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility -fintel-openmp-region -fopenmp-threadprivate-legacy -triple x86_64-unknown-linux-gnu %s| FileCheck %s -check-prefix CHECK-TPL

void foo() {}

int     afoo;
#pragma omp threadprivate(afoo)
static int     afoo_static;
#pragma omp threadprivate(afoo_static)

// CHECK-TPL: @afoo {{.*}}thread_private
// CHECK-NOT: @afoo {{.*}}thread_private
// CHECK-TPL: afoo_static {{.*}}thread_private
// CHECK-TPL: afoo_local_static {{.*}}thread_private
// CHECK-LABEL: @main
int main(int argc, char **argv) {
static int     afoo_local_static;
#pragma omp threadprivate(afoo_local_static)
// CHECK: call {{.*}}"DIR.OMP.PARALLEL"
// CHECK-SAME: "QUAL.OMP.COPYIN"(i32*
// CHECK-TPL: "QUAL.OMP.COPYIN"(i32* @afoo
#pragma omp parallel copyin(afoo, afoo_static, afoo_local_static)
// CHECK: call void @_Z3foov()
  foo();
// CHECK: call {{.*}}"DIR.OMP.END.PARALLEL"

  return 0;
}
