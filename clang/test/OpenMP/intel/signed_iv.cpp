// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void call_foo(float *);

//CHECK-LABEL: foo
void foo(float *A, int N, int S, long long N64)
{
  //CHECK: [[I:%i.*]] = alloca i32,
  int i;

  // Keep zero lower bound with positive step signed.

  //CHECK: "DIR.OMP.SIMD"
  //CHECK: icmp sle i32
  //CHECK: call void {{.*}}call_foo
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nsw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  #pragma omp simd
  for (i=0;i<=N;i++) { call_foo(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // Ugly positive step.

  //CHECK: "DIR.OMP.SIMD"
  //CHECK: icmp sle i32
  //CHECK: call void {{.*}}call_foo
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nsw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  #pragma omp simd
  for (i=0;i<=N;i = i - -1) { call_foo(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // Non-constant step is unsigned

  //CHECK: "DIR.OMP.SIMD"
  //CHECK: icmp ule i32
  //CHECK: call void {{.*}}call_foo
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nuw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  #pragma omp simd
  for (i=0;i<=N;i+=S) { call_foo(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // Negative step: unsigned
  //CHECK: "DIR.OMP.SIMD"
  //CHECK: icmp ule i32
  //CHECK: call void {{.*}}call_foo
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nuw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  #pragma omp simd
  for (i=0;i>=-2147483647;i--) { call_foo(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // Step 4, signed
  //CHECK: "DIR.OMP.SIMD"
  //CHECK: icmp sle i32
  //CHECK: call void {{.*}}call_foo
  //CHECK: load i32, ptr %.omp.iv
  //CHECK-NEXT: add nsw i32 {{.*}}1{{$}}
  //CHECK-NEXT: store i32 {{.*}}%.omp.iv
  #pragma omp simd
  for (i=0;i<=N;i+=4) { call_foo(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"

  // 64-bit too.

  long long li;

  //CHECK: "DIR.OMP.SIMD"
  //CHECK: icmp sle i64
  //CHECK: call void {{.*}}call_foo
  //CHECK: load i64, ptr %.omp.iv
  //CHECK-NEXT: add nsw i64 {{.*}}1{{$}}
  //CHECK-NEXT: store i64 {{.*}}%.omp.iv
  #pragma omp simd
  for (li=0;li<=N64;li++) { call_foo(&A[i]); }
  //CHECK: "DIR.OMP.END.SIMD"
}
