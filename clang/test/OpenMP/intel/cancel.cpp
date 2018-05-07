// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s
void bar(int);

void foo(int val) {

  // CHECK: DIR.OMP.PARALLEL
  #pragma omp parallel
  {
    // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
    // CHECK-SAME: QUAL.OMP.CANCEL.PARALLEL
    // CHECK: region.exit{{.*}}DIR.OMP.END.CANCELLATION.POINT
    #pragma omp cancellation point parallel
    // CHECK: region.entry{{.*}}DIR.OMP.CANCEL{{.*}}QUAL.OMP.CANCEL.PARALLEL
    // CHECK-SAME: "QUAL.OMP.IF
    // CHECK: region.exit{{.*}}"DIR.OMP.END.CANCEL"()
    #pragma omp cancel parallel if(val)
    bar(val);
  }
  // CHECK: DIR.OMP.END.PARALLEL

  // CHECK: DIR.OMP.SECTIONS
  #pragma omp sections
  {
    {
      // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancellation point sections
      // CHECK: region.entry{{.*}}"DIR.OMP.CANCEL"()
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancel sections
    }
  }
  // CHECK: DIR.OMP.END.SECTIONS

  // CHECK: DIR.OMP.SECTIONS
  #pragma omp sections
  {
    // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
    // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
    #pragma omp cancellation point sections
    #pragma omp section
    {
      // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancellation point sections
      // CHECK: region.entry{{.*}}"DIR.OMP.CANCEL"()
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancel sections
    }
  }
  // CHECK: DIR.OMP.END.SECTIONS

  // CHECK: DIR.OMP.LOOP
  #pragma omp for
  for (int i = 0; i < 8; ++i) {
    // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
    // CHECK-SAME: QUAL.OMP.CANCEL.LOOP
    #pragma omp cancellation point for
    // CHECK: region.entry{{.*}}"DIR.OMP.CANCEL"()
    // CHECK-SAME: QUAL.OMP.CANCEL.LOOP
    #pragma omp cancel for
  }
  // CHECK: DIR.OMP.END.LOOP

  // CHECK: DIR.OMP.TASK
  #pragma omp task
  {
    // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
    // CHECK-SAME: QUAL.OMP.CANCEL.TASKGROUP
    #pragma omp cancellation point taskgroup
    // CHECK: region.entry{{.*}}"DIR.OMP.CANCEL"()
    // CHECK-SAME: QUAL.OMP.CANCEL.TASKGROUP
    #pragma omp cancel taskgroup
  }
  // CHECK: DIR.OMP.END.TASK

  // CHECK: DIR.OMP.TASK
  #pragma omp task
  {
    // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
    // CHECK-SAME: QUAL.OMP.CANCEL.TASKGROUP
    #pragma omp cancellation point taskgroup
  }
  // CHECK: DIR.OMP.END.TASK

  // CHECK: DIR.OMP.PARALLEL.SECTIONS
  #pragma omp parallel sections
  {
    {
      // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancellation point sections
      // CHECK: region.entry{{.*}}"DIR.OMP.CANCEL"()
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancel sections
    }
  }
  // CHECK: DIR.OMP.END.PARALLEL.SECTIONS

  // CHECK: DIR.OMP.PARALLEL.SECTIONS
  #pragma omp parallel sections
  {
    {
      // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancellation point sections
      // CHECK: region.entry{{.*}}"DIR.OMP.CANCEL"()
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancel sections
    }
    // CHECK: "DIR.OMP.SECTION"()
    #pragma omp section
    {
      // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
      // CHECK-SAME: QUAL.OMP.CANCEL.SECTIONS
      #pragma omp cancellation point sections
    }
    // CHECK: "DIR.OMP.END.SECTION"()
  }
  // CHECK: DIR.OMP.END.PARALLEL.SECTIONS

  // CHECK: DIR.OMP.PARALLEL.LOOP
  #pragma omp parallel for
  for (int i = 0; i < 8; ++i) {
    // CHECK: region.entry{{.*}}DIR.OMP.CANCELLATION.POINT
    // CHECK-SAME: QUAL.OMP.CANCEL.LOOP
    #pragma omp cancellation point for
    // CHECK: region.entry{{.*}}"DIR.OMP.CANCEL"()
    // CHECK-SAME: QUAL.OMP.CANCEL.LOOP
    #pragma omp cancel for
  }
  // CHECK: DIR.OMP.END.PARALLEL.LOOP
}
