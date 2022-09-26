// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fintel-compatibility \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s

extern long A[256];

void foo(int a) {
  #pragma omp simd ompx_assert
  for (int i = 0; i < 256; ++i) A[i] = i;
//CHECK: "DIR.OMP.SIMD"
//CHECK: br label {{.*}} !llvm.loop [[LOOP4:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
  #pragma omp parallel
  #pragma omp simd if (true) ompx_assert
//CHECK: "DIR.OMP.PARALLEL"
//CHECK: "DIR.OMP.SIMD"
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < i; ++j)
    A[i] = j;
  }
//CHECK: br label {{.*}} !llvm.loop [[LOOP8:![0-9]+]]
//CHECK: br label {{.*}} !llvm.loop [[LOOP10:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.PARALLEL"
  #pragma omp simd collapse(4) ompx_assert
  for (int i = 1; i < 3; i++)
    for (int j = 2; j < 5; j++)
      for (int k = 3; k <= 6; k++)
        for (int l = 4; l < 9; ++l);
//CHECK: "DIR.OMP.SIMD"
//CHECK: br label {{.*}} !llvm.loop [[LOOP11:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target parallel for simd ompx_assert
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.TARGET"
//CHECK: "DIR.OMP.PARALLEL.LOOP"
//CHECK: "DIR.OMP.SIMD"
//CHECK: br label {{.*}} !llvm.loop [[LOOP12:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.PARALLEL.LOOP"
//CHECK: "DIR.OMP.END.TARGET"
  #pragma omp parallel master taskloop simd ompx_assert
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.PARALLEL"
//CHECK: "DIR.OMP.MASTER"
//CHECK: "DIR.OMP.TASKGROUP"
//CHECK: "DIR.OMP.TASKLOOP"()
//CHECK: "DIR.OMP.SIMD"
//CHECK: br label {{.*}} !llvm.loop [[LOOP13:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.TASKLOOP"
//CHECK: "DIR.OMP.END.TASKGROUP"
//CHECK: "DIR.OMP.END.MASTER"
//CHECK: "DIR.OMP.END.PARALLEL"
  #pragma omp distribute simd ompx_assert
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.DISTRIBUTE"
//CHECK: "DIR.OMP.SIMD"
//CHECK: br label {{.*}} !llvm.loop [[LOOP14:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.DISTRIBUTE"
  #pragma omp teams distribute simd ompx_assert
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.TEAMS"
//CHECK: "DIR.OMP.DISTRIBUTE"
//CHECK: "DIR.OMP.SIMD"
//CHECK: br label {{.*}} !llvm.loop [[LOOP15:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.DISTRIBUTE"
//CHECK: "DIR.OMP.END.TEAMS"
}

extern struct T t;
struct Base {
  float a;
};
struct T : public Base {
  void foo() {
#pragma omp simd
    for (int i = 0; i < 10; ++i) {
      Base::a = 0;
      t.a = 0;
    }
//CHECK: "DIR.OMP.SIMD"
//CHECK: br label {{.*}} !llvm.loop [[LOOP16:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
  }
} t;

void barfoo() {
  t.foo();

}
void zoo() {
  barfoo();
}
// CHECK: [[LOOP4]] = distinct !{[[LOOP4]], [[VECT5:![0-9]]], [[VECT6:![0-9]]], [[VECT7:![0-9]]], [[VECT6:![0-9]]]}
// CHECK: [[VECT5]] = !{!"llvm.loop.vectorize.ignore_profitability"}
// CHECK: [[VECT6]] = !{!"llvm.loop.vectorize.enable", i1 true}
// CHECK: [[VECT7]] = !{!"llvm.loop.intel.vector.assert"}
// CHECK: [[LOOP8]] = distinct !{[[LOOP8]], ![[MUSTPROGRESS:.+]]}
// CHECK: [[MUSTPROGRESS]] = !{!"llvm.loop.mustprogress"}
// CHECK: [[LOOP10]] = distinct !{[[LOOP10]], [[VECT5]], [[VECT6]], [[VECT7]], [[VECT6]]}
// CHECK: [[LOOP11]] = distinct !{[[LOOP11]], [[VECT5]], [[VECT6]], [[VECT7]], [[VECT6]]}
// CHECK: [[LOOP12]] = distinct !{[[LOOP12]], [[VECT5]], [[VECT6]], [[VECT7]], [[VECT6]]}
// CHECK: [[LOOP13]] = distinct !{[[LOOP13]], [[VECT5]], [[VECT6]], [[VECT7]], [[VECT6]]}
// CHECK: [[LOOP14]] = distinct !{[[LOOP14]], [[VECT5]], [[VECT6]], [[VECT7]], [[VECT6]]}
// CHECK: [[LOOP15]] = distinct !{[[LOOP15]], [[VECT5]], [[VECT6]], [[VECT7]], [[VECT6]]}
// CHECK: [[LOOP16]] = distinct !{[[LOOP16]], [[VECT6]]}
