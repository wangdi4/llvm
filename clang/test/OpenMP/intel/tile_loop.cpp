// RUN: %clang_cc1 -std=c++14 -fopenmp -fintel-compatibility \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-pch -o %t %s

// RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fopenmp -fintel-compatibility \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu \
// RUN:  -include-pch %t %s | FileCheck %s

// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu | FileCheck %s

// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu | FileCheck %s

#ifndef HEADER
#define HEADER

void bar(int) noexcept;

// CHECK-LABEL: tile_test
const int M = 64;
const int N = 32;
void tile_test(int v_ptr[M][N])
{
  //CHECK: [[VARI:%i.*]] = alloca i32,
  //CHECK: [[VARJ:%j.*]] = alloca i32,
  //CHECK: [[IVOUTER:%.omp.uncollapsed.iv.*]] = alloca i32,
  //CHECK: [[IVINNER:%.omp.uncollapsed.iv.*]] = alloca i32,
  //CHECK: [[LBOUTER:%.omp.uncollapsed.lb.*]] = alloca i32,
  //CHECK: [[UBOUTER:%.omp.uncollapsed.ub.*]] = alloca i32,
  //CHECK: [[LBINNER:%.omp.uncollapsed.lb.*]] = alloca i32,
  //CHECK: [[UBINNER:%.omp.uncollapsed.ub.*]] = alloca i32,
  //CHECK: store i32 0, i32* [[LBOUTER]],
  //CHECK: store i32 31, i32* [[UBOUTER]],
  //CHECK: store i32 0, i32* [[LBINNER]],
  //CHECK: store i32 15, i32* [[UBINNER]],
  int i, j;
  //CHECK: region.entry{{.*}}OMP.PARALLEL.LOOP
  //CHECK-SAME-DIAG: QUAL.OMP.TILE"(i32 8, i32 16)
  //CHECK-SAME-DIAG: QUAL.OMP.FIRSTPRIVATE"(i32* [[LBOUTER]])
  //CHECK-SAME-DIAG: QUAL.OMP.FIRSTPRIVATE"(i32* [[LBINNER]])
  //CHECK-SAME-DIAG: QUAL.OMP.PRIVATE"(i32* [[IVAR]])
  //CHECK-SAME-DIAG: QUAL.OMP.PRIVATE"(i32* [[JVAR]])
  //CHECK-SAME-DIAG: QUAL.OMP.NORMALIZED.IV"(i32* [[IVOUTER]], i32* [[IVINNER]])
  //CHECK-SAME-DIAG: QUAL.OMP.NORMALIZED.UB"(i32* [[UBOUTER]], i32* [[UBINNER]])
  #pragma omp parallel for tile(8,16)
  for (i = 16; i < 48; i++) {
    for (j = 8; j < 24; j++) {
      //CHECK: call {{.*}}bar
      bar(v_ptr[i][j]);
    }
  }
  // CHECK: region.exit{{.*}}OMP.END.PARALLEL.LOOP
}

void baz(int*,int,int);
template<int t1, int t2>
void foo(int *vp) {
  int i,j;
  #pragma omp parallel for tile(t1,t2)
  for (i = 16; i < 48; i++) {
    for (j = 8; j < 24; j++) {
      baz(vp,i,j);
    }
  }
}
int arr[2000];

//CHECK-LABEL: other
void other()
{
  foo<4,12>(arr);
  foo<2,16>(arr);
}

//CHECK-DAG: OMP.PARALLEL.LOOP{{.*}}OMP.TILE"(i32 4, i32 12)
//CHECK-DAG: OMP.PARALLEL.LOOP{{.*}}OMP.TILE"(i32 2, i32 16)

#endif
