// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void bar(int);
int icnt, jcnt, kcnt;
int other;

// CHECK-LABEL: @_Z4foo1v
void foo1() {
  //CHECK: [[TVAL1:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL.LOOP{{.*}}LASTPRIVATE{{.*}}@icnt{{.*}}LASTPRIVATE{{.*}}@kcnt{{.*}}PRIVATE{{.*}}@jcnt

  // Setting the orignal loop counters.
  //CHECK: %div{{[0-9]*}} = sdiv i32 %{{[0-9]+}}, 500
  //CHECK: store i32 %add{{[0-9]*}}, ptr @icnt

  //CHECK: %div{{.*}}, 500
  //CHECK: %mul{{.*}}, 500
  //CHECK: %sub
  //CHECK: %div{{.*}}, 5
  //CHECK: %mul{{.*}}, 2
  //CHECK: store i32 %add{{[0-9]*}}, ptr @jcnt

  //CHECK: %div{{.*}}, 500
  //CHECK: %mul{{.*}}, 500
  //CHECK: %sub
  //CHECK: %div{{.*}}, 500
  //CHECK: %mul{{.*}}, 500
  //CHECK: %sub
  //CHECK: %div{{.*}}, 5
  //CHECK: %mul{{.*}}, 5
  //CHECK: %sub
  //CHECK: %mul{{.*}}, 8
  //CHECK: store i32 %add{{[0-9]*}}, ptr @kcnt

  // Setting the lastprivate original loop counters.
  //CHECK: store i32 80, ptr @icnt, align 4
  //CHECK-NOT: store i32 200, ptr @jcnt, align 4
  //CHECK: store i32 40, ptr @kcnt, align 4

  //CHECK: region.exit(token [[TVAL1]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for lastprivate(icnt,kcnt) collapse(3)
  for (icnt = 0; icnt < 80; icnt += 4) {
    for (jcnt = 0; jcnt < 200; jcnt += 2) {
      for (kcnt = 0; kcnt < 40; kcnt += 8) {
        bar(other+icnt+jcnt+kcnt);
      } /* for */
    } /* for */
  } /* for */
}

float a[100];
// CHECK-LABEL: @_Z4foo2v
void foo2() {
 int j;
  //CHECK: [[TVAL2:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL.LOOP
  //CHECK-SAME: LASTPRIVATE{{.*}}(ptr @a)
  //CHECK: region.exit(token [[TVAL2]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
 #pragma omp parallel for schedule(static, 1) lastprivate(a)
 for (j = 0; j < 4; j++) { a[j] = a[j]+1; }
}

// CHECK-LABEL: @_Z4foo3v
void foo3() {
  int i, j;

  //CHECK: [[I:%i.*]] = alloca i32,
  //CHECK: [[J:%j.*]] = alloca i32,
  //CHECK: [[K:%k.*]] = alloca i32,
  //CHECK: [[L:%l.*]] = alloca i32,

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK: "QUAL.OMP.LASTPRIVATE"(ptr [[I]])
  //CHECK: "QUAL.OMP.LASTPRIVATE"(ptr [[J]])
  #pragma omp parallel for simd collapse(2)
  for (i = 0; i < 77; ++i)
    for (j = 0; j < 99; ++j) bar(i+j);
  //CHECK: {{call|invoke}}{{.*}}bar
  //CHECK: store i32 77, ptr [[I]]
  //CHECK: store i32 99, ptr [[J]]
  //CHECK: DIR.OMP.END.PARALLEL.LOOP

  //CHECK: DIR.OMP.PARALLEL.LOOP
  //CHECK: "QUAL.OMP.PRIVATE"(ptr [[K]])
  //CHECK: "QUAL.OMP.PRIVATE"(ptr [[L]])
  #pragma omp parallel for simd collapse(2)
  for (int k = 0; k < 33; ++k)
    for (int l = 0; l < 55; ++l) bar(k+l);
  //CHECK: {{call|invoke}}{{.*}}bar
  //CHECK-NOT: store i32 33, ptr [[K]]
  //CHECK-NOT: store i32 55, ptr [[L]]
  //CHECK: DIR.OMP.END.PARALLEL.LOOP
}

// end INTEL_COLLAB
