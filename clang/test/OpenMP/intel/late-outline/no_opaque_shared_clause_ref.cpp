// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void bar(int);

// CHECK-LABEL: test_ref_shared
void test_ref_shared(int &A) {
  //CHECK: [[A:%A.addr.*]] = alloca i32*,

  //CHECK: region.entry{{.*}}"DIR.OMP.PARALLEL"
  //CHECK-SAME: "QUAL.OMP.SHARED:BYREF"{{.*}}[[A]]
  #pragma omp parallel shared(A)
  {
    bar(A);
  }
  //CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"

  //CHECK: region.entry{{.*}}"DIR.OMP.PARALLEL"
  //CHECK-SAME: "QUAL.OMP.SHARED:BYREF"{{.*}}[[A]]
  #pragma omp parallel
  {
    bar(A);
  }
  //CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"

  //CHECK: region.entry{{.*}}"DIR.OMP.PARALLEL"
  //CHECK-SAME: "QUAL.OMP.SHARED:BYREF"{{.*}}[[A]]
  #pragma omp parallel default(shared)
  {
    bar(A);
  }
  //CHECK: region.exit{{.*}}"DIR.OMP.END.PARALLEL"
}
// end INTEL_COLLAB
