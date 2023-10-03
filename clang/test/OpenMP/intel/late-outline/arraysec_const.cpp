// INTEL_COLLAB
//RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu -emit-llvm \
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

void *getP();

#define LOOPA(X) for(int i=0; i<8; ++i) { X[i] = i; }

//CHECK: define{{.*}}foo_simple_cases
void foo_simple_cases(unsigned long bound, unsigned long lower, unsigned long length) {

  //CHECK: [[LOWER:%lower.addr.*]] = alloca i64,
  //CHECK: [[LENGTH:%length.addr.*]] = alloca i64,
  //CHECK: [[ARRP:%ArrP.*]] = alloca ptr,
  //CHECK: [[ARRPREF:%ArrPRef.*]] = alloca ptr,
  //CHECK: [[ARR:%Arr.*]] = alloca [10 x i32],
  //CHECK: [[VLA:%vla.*]] = alloca i32, i64 %{{[0-9]+}}

  int *ArrP = (int*)getP();
  int *&ArrPRef = ArrP;
  int Arr[10];
  int VLA[bound];

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRP]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrP[0:2])
  LOOPA(ArrP)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRP]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrP[:2])
  LOOPA(ArrP)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRP]], i32 0, i64 3, i64 6)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrP[6:3])
  LOOPA(ArrP)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRP]], i32 0, i64 1, i64 4)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrP[4])
  LOOPA(ArrP)

  //CHECK: [[LEN:%[0-9]+]] = load i64, ptr [[LENGTH]]
  //CHECK: [[LWR:%[0-9]+]] = load i64, ptr [[LOWER]]
  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRP]], i32 0, i64 [[LEN]], i64 [[LWR]])
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrP[lower:length])
  LOOPA(ArrP)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRPREF]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrPRef[0:2])
  LOOPA(ArrPRef)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRPREF]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrPRef[:2])
  LOOPA(ArrPRef)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRPREF]], i32 0, i64 3, i64 6)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrPRef[6:3])
  LOOPA(ArrPRef)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRPREF]], i32 0, i64 1, i64 4)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrPRef[4])
  LOOPA(ArrPRef)

  //CHECK: [[LEN:%[0-9]+]] = load i64, ptr [[LENGTH]]
  //CHECK: [[LWR:%[0-9]+]] = load i64, ptr [[LOWER]]
  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.PTR_TO_PTR.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARRPREF]], i32 0, i64 [[LEN]], i64 [[LWR]])
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:ArrPRef[lower:length])
  LOOPA(ArrPRef)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARR]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:Arr[0:2])
  LOOPA(Arr)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARR]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:Arr[:2])
  LOOPA(Arr)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARR]], i32 0, i64 3, i64 6)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:Arr[6:3])
  LOOPA(Arr)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARR]], i32 0, i64 1, i64 4)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:Arr[4])
  LOOPA(Arr)

  //CHECK: [[LEN:%[0-9]+]] = load i64, ptr [[LENGTH]]
  //CHECK: [[LWR:%[0-9]+]] = load i64, ptr [[LOWER]]
  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[ARR]], i32 0, i64 [[LEN]], i64 [[LWR]])
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:Arr[lower:length])
  LOOPA(Arr)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[VLA]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:VLA[0:2])
  LOOPA(VLA)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[VLA]], i32 0, i64 2, i64 0)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:VLA[:2])
  LOOPA(VLA)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[VLA]], i32 0, i64 3, i64 6)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:VLA[6:3])
  LOOPA(VLA)

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[VLA]], i32 0, i64 1, i64 4)
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:VLA[4])
  LOOPA(VLA)

  //CHECK: [[LEN:%[0-9]+]] = load i64, ptr [[LENGTH]]
  //CHECK: [[LWR:%[0-9]+]] = load i64, ptr [[LOWER]]
  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"
  //CHECK-SAME: (ptr{{.*}}[[VLA]], i32 0, i64 [[LEN]], i64 [[LWR]])
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
  #pragma omp parallel for reduction(+:VLA[lower:length])
  LOOPA(VLA)
}
// end INTEL_COLLAB
