// INTEL_COLLAB
//RUN: %clang_cc1 -opaque-pointers -fopenmp -fopenmp-late-outline \
//RUN:  -fopenmp-typed-clauses -triple x86_64-unknown-linux-gnu -emit-llvm \
//RUN:  -o - %s | FileCheck %s

static int y_Array[3][4][5];
#define LOOP for(int i=0; i<8; ++i) {}

void foo()
{
  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 60, i64 0)
  #pragma omp parallel for reduction(+:y_Array[:][:][:])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 60, i64 0)
  #pragma omp parallel for reduction(+:y_Array[0:3][0:4][0:5])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 60, i64 0)
  #pragma omp parallel for reduction(+:y_Array[0:][0:][0:])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 20, i64 20)
  #pragma omp parallel for reduction(+:y_Array[1][:][:])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 20, i64 20)
  #pragma omp parallel for reduction(+:y_Array[1][0:4][0:5])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 20, i64 20)
  #pragma omp parallel for reduction(+:y_Array[1][0:][0:])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 5, i64 30)
  #pragma omp parallel for reduction(+:y_Array[1][2][:])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 5, i64 30)
  #pragma omp parallel for reduction(+:y_Array[1][2][0:5])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 5, i64 30)
  #pragma omp parallel for reduction(+:y_Array[1][2][0:5])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 1, i64 33)
  #pragma omp parallel for reduction(+:y_Array[1][2][3])
  LOOP

  //CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}y_Array, i32 0, i64 40, i64 20)
  #pragma omp parallel for reduction(+:y_Array[1:2][:][:])
  LOOP
}
// end INTEL_COLLAB
