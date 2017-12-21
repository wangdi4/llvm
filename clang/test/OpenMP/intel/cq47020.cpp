// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

void bar(int);
int icnt, jcnt, kcnt;
int other;

// CHECK-LABEL: @_Z4foo1v
void foo1() {
  //CHECK: [[TVAL1:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL.LOOP{{.*}}LASTPRIVATE{{.*}}@icnt{{.*}}LASTPRIVATE{{.*}}@kcnt{{.*}}PRIVATE{{.*}}@jcnt
  //CHECK: %div{{[0-9]*}} = sdiv i32 %{{[0-9]+}}, 500
  //CHECK: store i32 %add{{[0-9]*}}, i32* @icnt
  //CHECK: %rem{{[0-9]*}} = srem i32 %div{{[0-9]+}}, 100
  //CHECK: store i32 %add{{[0-9]*}}, i32* @jcnt
  //CHECK: store i32 %add{{[0-9]*}}, i32* @kcnt
  //CHECK: store i32 80, i32* @icnt, align 4
  //CHECK-NOT: store i32 200, i32* @jcnt, align 4
  //CHECK: store i32 40, i32* @kcnt, align 4
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

// Test from CMPLRS-42598
float a[100];
// CHECK-LABEL: @_Z4foo2v
void foo2() {
 int j;
  //CHECK: [[TVAL2:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL.LOOP{{.*}}LASTPRIVATE{{.*}}([100 x float]* @a)
  //CHECK: region.exit(token [[TVAL2]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
 #pragma omp parallel for schedule(static, 1) lastprivate(a)
 for (j = 0; j < 4; j++) { a[j] = a[j]+1; }
}
