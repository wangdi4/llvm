// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s

int x[5][3][0];
int y[5][3][0];

// CHECK: define {{.*}}A
void A(int m, int n, int o) {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[K:%k.*]] = alloca i32,

  //CHECK: region.entry() [ "DIR.OMP.LOOP"()
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[I]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[J]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[K]])
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[I]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[J]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[K]])
  #pragma omp for simd collapse(3)
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < o; k++) {
        y[i][j][k] += x[i][j][k];
      }
    }
  }
  //CHECK: "DIR.OMP.END.SIMD"()
  //CHECK: "DIR.OMP.END.LOOP"()
}

// CHECK: define {{.*}}B
void B(int m, int n, int o) {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[K:%k.*]] = alloca i32,
  int i,j,k;

  //CHECK: region.entry() [ "DIR.OMP.LOOP"()
  //CHECK-SAME:"QUAL.OMP.LASTPRIVATE"(i32* [[I]])
  //CHECK-SAME:"QUAL.OMP.LASTPRIVATE"(i32* [[J]])
  //CHECK-SAME:"QUAL.OMP.LASTPRIVATE"(i32* [[K]])
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-SAME:"QUAL.OMP.LASTPRIVATE"(i32* [[I]])
  //CHECK-SAME:"QUAL.OMP.LASTPRIVATE"(i32* [[J]])
  //CHECK-SAME:"QUAL.OMP.LASTPRIVATE"(i32* [[K]])
  #pragma omp for simd collapse(3)
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      for (k = 0; k < o; k++) {
        y[i][j][k] += x[i][j][k];
      }
    }
  }
  //CHECK: "DIR.OMP.END.SIMD"()
  //CHECK: "DIR.OMP.END.LOOP"()
}

// CHECK: define {{.*}}C
void C(int m, int n, int o) {
  // CHECK: [[I:%i.*]] = alloca i32,
  int i, j, k;

  //CHECK: region.entry() [ "DIR.OMP.LOOP"()
  //CHECK-SAME:"QUAL.OMP.LASTPRIVATE"(i32* [[I]])
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-SAME:"QUAL.OMP.LINEAR:IV"(i32* [[I]],
  #pragma omp for simd // No collapse
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      for (k = 0; k < o; k++) {
        y[i][j][k] += x[i][j][k];
      }
    }
  }
  //CHECK: "DIR.OMP.END.SIMD"()
  //CHECK: "DIR.OMP.END.LOOP"()
}

// CHECK: define {{.*}}D
void D(int m, int n, int o) {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[K:%k.*]] = alloca i32,
  // CHECK: [[Z:%z.*]] = alloca i32,

  //CHECK: region.entry() [ "DIR.OMP.LOOP"()
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[I]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[J]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[K]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[Z]])
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-SAME:"QUAL.OMP.LINEAR:IV"(i32* [[I]],
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[J]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[K]])
  //CHECK-SAME:"QUAL.OMP.PRIVATE"(i32* [[Z]])
  #pragma omp for simd // No collapse
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < o; k++) {
        int z = 10;
        y[i][j][k] += x[i][j][k] + z;
      }
    }
  }
}

// CHECK: define {{.*}}E
void E() {
  int n = 2, s = 1, data[2] = { 1, 2};
  //CHECK: [[K:%k.*]] = alloca i32,
  //CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
  //CHECK: [[L9:%[1-9]*]] = load i32, i32*
  //CHECK: [[SUB:%.*]] = sub nsw i32 0, [[L9]]
  //CHECK: region.entry() [ "DIR.OMP.SIMD"()
  //CHECK-SAME: "QUAL.OMP.LINEAR:IV"(i32* [[K]], i32 [[SUB]])
  #pragma omp parallel for simd
  for (int k = n-1; k > 0; k -= 2*s)
    data[k] += data[k-s];
}

// end INTEL_COLLAB
