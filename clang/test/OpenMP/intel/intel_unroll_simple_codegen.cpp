// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN: -fopenmp-late-outline -fopenmp-intel-unroll -emit-llvm \
// RUN: %s -o - | FileCheck %s

// expected-no-diagnostics

// placeholder for loop body code.
extern "C" void body(...) {}

// CHECK: define {{.*}}foo
extern "C" void foo(int start, int end, int step) {
// CHECK: "DIR.OMP.UNROLL"(), "QUAL.OMP.PARTIAL"(i64 4)
// CHECK: icmp {{.*}}10

  #pragma omp unroll partial(4)
  for (int i = 0; i < 10; i+=1)
    body(start, end, step, i);

// CHECK: call {{.*}}body
// CHECK: "DIR.OMP.END.UNROLL"()
}

// CHECK: define {{.*}}foo2
extern "C" void foo2(int start, int end, int step) {
// CHECK: "DIR.OMP.UNROLL"(), "QUAL.OMP.PARTIAL"(i64 4)
// CHECK: icmp {{.*}}10

  #pragma omp unroll partial(4)
  for (int i = 0; i < 10; i+=1) {
// CHECK: "DIR.OMP.PARALLEL"()
    #pragma omp parallel
    body(i);
// CHECK: call {{.*}}body
// CHECK: "DIR.OMP.END.PARALLEL"()
  }
// CHECK: "DIR.OMP.END.UNROLL"()
}

// CHECK: define {{.*}}bar
extern "C" void bar(int start, int end, int step) {
// CHECK: store i32 19, ptr %.omp.ub
// CHECK: "DIR.OMP.LOOP"()
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"
// CHECK: "DIR.OMP.UNROLL"(), "QUAL.OMP.PARTIAL"(i64 5)

  #pragma omp for
  #pragma omp unroll partial(5)
  for (int i = 0; i < 20; i+=1)
    body(start, end, step, i);

// CHECK: call {{.*}}body
// CHECK: "DIR.OMP.END.UNROLL"()
// CHECK: "DIR.OMP.END.LOOP"()
}

// CHECK: define {{.*}}zoo
// CHECK-NOT: "DIR.OMP.UNROLL"
extern "C" void zoo(int start, int end, int step) {
  #pragma omp for collapse(2)
  for (int i = start; i < end; i+=step) {
    #pragma omp unroll partial
    for (int j = start; j < end; j+=step)
        body(j, end, step, i);
// CHECK: call {{.*}}body
  }
}

// CHECK: define {{.*}}moo1
extern "C" void moo1()
{
  int a[10];
// CHECK: "DIR.OMP.TARGET"()
// CHECK: "DIR.OMP.UNROLL"()

  #pragma omp target
  #pragma omp unroll
  for (int i = 0; i < 10; ++i)
    a[i] = i, body(i);

// CHECK: call {{.*}}body
// CHECK: "DIR.OMP.END.UNROLL"()
// CHECK: "DIR.OMP.END.TARGET"()
}


// CHECK: define {{.*}}moo2
extern "C" void moo2()
{
  int a[10];
// CHECK: "DIR.OMP.TARGET"
// CHECK: store i32 29, ptr %.omp.ub
// CHECK: "DIR.OMP.PARALLEL.LOOP"
// CHECK: "DIR.OMP.UNROLL"(), "QUAL.OMP.PARTIAL"(i64 2)

  #pragma omp target parallel for
  #pragma omp unroll partial(2)
  for (int i = 0; i < 30; ++i)
      a[i] = i, body(i);

// CHECK: call {{.*}}body
// CHECK: "DIR.OMP.END.UNROLL"()
// CHECK: "DIR.OMP.END.PARALLEL.LOOP"
// CHECK: "DIR.OMP.END.TARGET"()
}

// CHECK: define {{.*}}zoo1
// CHECK-NOT: "DIR.OMP.UNROLL"
void zoo1() {
  #pragma omp tile sizes(4)
  #pragma omp unroll partial
  for (int i = 0; i < 10; ++i) {
// CHECK: call {{.*}}body
    body(i);
  }
}

