//RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

static int y_Array[3][4][5];

//CHECK-LABEL: arrsecred_array
void arrsecred_array() {
//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
//CHECK-SAME: ([3 x [4 x [5 x i32]]]*{{.*}}y_Array
//CHECK-SAME: i64 3,
//CHECK-SAME: i64 1, i64 1, i64 1,
//CHECK-SAME: i64 0, i64 4, i64 1,
//CHECK-SAME: i64 0, i64 5, i64 1)
  #pragma omp parallel for reduction(+:y_Array[1][:][:])
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 5; k++) {
        y_Array[1][2][k] += 1;
      }
    }
  }
}

static int (*y_Arr_ptr)[3][4][5];

//CHECK-LABEL: arrsecred_arrayptr
void arrsecred_arrayptr() {

//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
//CHECK-SAME: ([3 x [4 x [5 x i32]]]**{{.*}}y_Arr_ptr
//CHECK-SAME: i64 4,
//CHECK-SAME: i64 0, i64 1, i64 1,
//CHECK-SAME: i64 1, i64 1, i64 1,
//CHECK-SAME: i64 2, i64 1, i64 1,
//CHECK-SAME: i64 0, i64 5, i64 1)
  #pragma omp parallel for reduction(+:y_Arr_ptr[0][1][2][0:5])
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 5; k++) {
        (*y_Arr_ptr)[1][2][k] += 1;
      }
    }
  }
}

static short *yglobptr;
const int SZ = 3;

//CHECK-LABEL: arrsecred_ptr
void arrsecred_ptr() {

  // CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
  // CHECK-SAME: (i16** {{.*}}yglobptr{{.*}}, i64 1, i64 6, i64 3, i64 1)
  #pragma omp parallel for reduction(+:yglobptr[6:SZ])
  for (int i = 6; i < 9; i++) {
    yglobptr[i] += 1;
  }
}


//CHECK-LABEL: arrsecred_arrayref
void arrsecred_arrayref(int (&y_Arr_ref)[3][4][5]) {

//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
//CHECK-SAME: ([3 x [4 x [5 x i32]]]*{{.*}}, i64 3,
//CHECK-SAME: i64 1, i64 1, i64 1,
//CHECK-SAME: i64 2, i64 1, i64 1,
//CHECK-SAME: i64 0, i64 5, i64 1)
  #pragma omp parallel for reduction(+:y_Arr_ref[1][2][0:5])
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 5; k++) {
        y_Arr_ref[1][2][k] += 1;
      }
    }
  }
}

static int y_Sub[4];
static const int N_Sub = 4;

//CHECK-LABEL: arrsecred_arraysubscript
void arrsecred_arraysubscript() {
//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
//CHECK-SAME: [4 x i32]*{{.*}}y_Sub
//CHECK-SAME: , i64 1, i64 1, i64 1, i64 1)
#pragma omp parallel for reduction(+ : y_Sub[1])
  for (int j = 0; j < N_Sub; j++) {
    y_Sub[1] += 1;
  }
}

static int y_TwoSub[4][8];
static const int N_TwoSub = 4;

//CHECK-LABEL: arrsecred_arraytwosubscript
void arrsecred_arraytwosubscript() {
//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
//CHECK-SAME: [4 x [8 x i32]]*{{.*}}y_TwoSub
//CHECK-SAME: , i64 2, i64 2, i64 1, i64 1, i64 4, i64 1, i64 1)
#pragma omp parallel for reduction(+ : y_TwoSub[2][4])
  for (int j = 0; j < N_TwoSub; j++) {
    y_TwoSub[2][4] += 1;
  }
}
