// INTEL_COLLAB

// RUN: %clang_cc1 -verify -fopenmp -x c++ -triple x86_64-unknown-unknown \
// RUN: -fopenmp-late-outline   -emit-llvm %s -fexceptions \
// RUN: -fcxx-exceptions -o - | FileCheck %s

// expected-no-diagnostics

extern long A[256];

void foo(int a) {
  #pragma omp simd
  for (int i = 0; i < 256; ++i) A[i] = i;
//CHECK: "DIR.OMP.SIMD"
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP4:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
  #pragma omp parallel
  #pragma omp simd if (true)
//CHECK: "DIR.OMP.PARALLEL"
//CHECK: "DIR.OMP.SIMD"
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < i; ++j)
    A[i] = j;
  }
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP6:![0-9]+]]
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP8:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.PARALLEL"
  #pragma omp simd collapse(4)
  for (int i = 1; i < 3; i++)
    for (int j = 2; j < 5; j++)
      for (int k = 3; k <= 6; k++)
        for (int l = 4; l < 9; ++l);
//CHECK: "DIR.OMP.SIMD"
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP9:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
  #pragma omp target parallel for simd
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.TARGET"
//CHECK: "DIR.OMP.PARALLEL.LOOP"
//CHECK: "DIR.OMP.SIMD"
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP10:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.PARALLEL.LOOP"
//CHECK: "DIR.OMP.END.TARGET"
  #pragma omp parallel master taskloop simd
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.PARALLEL"
//CHECK: "DIR.OMP.MASTER"
//CHECK: "DIR.OMP.TASKGROUP"
//CHECK: "DIR.OMP.TASKLOOP"()
//CHECK: "DIR.OMP.SIMD"
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP11:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.TASKLOOP"
//CHECK: "DIR.OMP.END.TASKGROUP"
//CHECK: "DIR.OMP.END.MASTER"
//CHECK: "DIR.OMP.END.PARALLEL"
  #pragma omp distribute simd
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.DISTRIBUTE"
//CHECK: "DIR.OMP.SIMD"
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP12:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
//CHECK: "DIR.OMP.END.DISTRIBUTE"
  #pragma omp teams distribute simd
  for (int i = 1; i < 3; i++);
//CHECK: "DIR.OMP.TEAMS"
//CHECK: "DIR.OMP.DISTRIBUTE"
//CHECK: "DIR.OMP.SIMD"
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP13:![0-9]+]]
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
//CHECK: br {{.*}}label {{.*}} !llvm.loop [[LOOP13:![0-9]+]]
//CHECK: "DIR.OMP.END.SIMD"
  }
} t;

void barfoo() {
  t.foo();

}
void zoo() {
  barfoo();
}
// CHECK: [[LOOP4]] = distinct !{[[LOOP4]], ![[VECT:.+]]}
// CHECK: ![[VECT]] = !{!"llvm.loop.vectorize.enable", i1 true}
// CHECK: [[LOOP6]] = distinct !{[[LOOP6]], ![[MUSTPROGRESS:.+]]}
// CHECK: [[MUSTPROGRESS]] = !{!"llvm.loop.mustprogress"}
// CHECK: [[LOOP8]] = distinct !{[[LOOP8]], ![[VECT]]}
// CHECK: [[LOOP9]] = distinct !{[[LOOP9]], ![[VECT]]}
// CHECK: [[LOOP10]] = distinct !{[[LOOP10]], ![[VECT]]}
// CHECK: [[LOOP11]] = distinct !{[[LOOP11]], ![[VECT]]}
// CHECK: [[LOOP12]] = distinct !{[[LOOP12]], ![[VECT]]}
// CHECK: [[LOOP13]] = distinct !{[[LOOP13]], ![[VECT]]}
// end INTEL_COLLAB
