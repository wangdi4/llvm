// INTEL_COLLAB
//RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

static int y_Array[3][4][5];

//CHECK-LABEL: arrsecred_array
void arrsecred_array() {
//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: (ptr{{.*}}y_Array,  i32 0, i64 20, i64 20),
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

//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
//CHECK-SAME: (ptr{{.*}}y_Arr_ptr, i32 0,
//CHECK-SAME: i64 %sec.number_of_elements{{[^,]*}},
//CHECK-SAME: i64 %sec.offset_in_elements{{[^,]*}})
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

  // CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
  // CHECK-SAME: (ptr {{.*}}yglobptr{{.*}}, i16 0, i64 3, i64 6),
  #pragma omp parallel for reduction(+:yglobptr[6:SZ])
  for (int i = 6; i < 9; i++) {
    yglobptr[i] += 1;
  }
}


//CHECK-LABEL: arrsecred_arrayref
void arrsecred_arrayref(int (&y_Arr_ref)[3][4][5]) {
//CHECK: [[YARRREF:%y_Arr_ref.*]] = alloca ptr, align
//CHECK: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.TYPED"
//CHECK-SAME: (ptr [[YARRREF]], i32 0, i64 5, i64 30)
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
//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: ptr{{.*}}y_Sub, i32 0, i64 1, i64 1)
#pragma omp parallel for reduction(+ : y_Sub[1])
  for (int j = 0; j < N_Sub; j++) {
    y_Sub[1] += 1;
  }
}

static int y_TwoSub[4][8];
static const int N_TwoSub = 4;

//CHECK-LABEL: arrsecred_arraytwosubscript
void arrsecred_arraytwosubscript() {
//CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
//CHECK-SAME: ptr{{.*}}y_TwoSub, i32 0, i64 1, i64 20)
#pragma omp parallel for reduction(+ : y_TwoSub[2][4])
  for (int j = 0; j < N_TwoSub; j++) {
    y_TwoSub[2][4] += 1;
  }
}

void doSomething();

// Test equivalence of array subscript and length one array section.
//CHECK-LABEL: arraysec_depend_length_one
void arraysec_depend_length_one()
{
  int i = 10;
  char*a, *b, *c, d;
  a = &d; b = &d; c = &d;
  //CHECK: [[DARR1:%.*]] = getelementptr inbounds [3 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  //CHECK: "DIR.OMP.TASK"()
  //CHECK: "QUAL.OMP.DEPARRAY"(i32 3, ptr [[DARR1]])
  //CHECK: "DIR.OMP.END.TASK"()
  #pragma omp task depend(in:a[1:1], b[0:1]) depend(out: c) firstprivate(i)
  {
    doSomething();
  }
  //CHECK: [[DARR2:%.*]] = getelementptr inbounds [3 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  //CHECK: "DIR.OMP.TASK"()
  //CHECK: "QUAL.OMP.DEPARRAY"(i32 3, ptr [[DARR2]])
  //CHECK: "DIR.OMP.END.TASK"()
  #pragma omp task depend(in:a[1], b[0]) depend(out: c) firstprivate(i)
  {
    doSomething();
  }
}
// end INTEL_COLLAB
