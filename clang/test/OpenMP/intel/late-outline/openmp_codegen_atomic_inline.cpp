// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:   -triple x86_64-unknown-linux-gnu %s \
// RUN:   | FileCheck %s --check-prefix=CHECK

void marker(int);

// CHECK-LABEL: foo
void foo() {
  //CHECK: [[N1:%n1.*]] = alloca i64, align 8
  //CHECK: [[N2:%n2.*]] = alloca i64, align 8
  long long int n1 = 0, n2;
  //CHECK: [[D1:%d1.*]] = alloca x86_fp80, align 16
  //CHECK: [[D2:%d2.*]] = alloca x86_fp80, align 16
  long double d1 = 0.0, d2;
  //CHECK: [[T1:%atomic-temp.*]] = alloca x86_fp80, align 16
  //CHECK: [[T2:%atomic-temp.*]] = alloca x86_fp80, align 16
  //CHECK: [[T3:%atomic-temp.*]] = alloca x86_fp80, align 16

  //CHECK: [[T:%[0-9].*]] = {{.*}}"DIR.OMP.PARALLEL"
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[N1]]
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[N2]]
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[T1]]
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[T2]]
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[T3]]

  #pragma omp parallel
  {
    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 1)
    marker(1);

    //CHECK: atomicrmw add ptr [[N1]], i64 1 monotonic
    #pragma omp atomic
    ++n1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 2)
    marker(2);

    //CHECK: atomicrmw add ptr [[N1]], i64 1 monotonic
    #pragma omp atomic update
    ++n1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 3)
    marker(3);

    //CHECK: [[AL:%atom.*]] = load atomic i64, ptr [[N1]] monotonic, align 8
    //CHECK: store i64 [[AL]], ptr [[N2]], align 8
    #pragma omp atomic read
    n2 = n1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 4)
    marker(4);

    //CHECK: store atomic i64 1, ptr [[N1]] monotonic, align 8
    #pragma omp atomic write
    n1 = 1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 5)
    marker(5);

    //CHECK: [[LOAD:%[0-9]+]] = atomicrmw add ptr [[N1]], i64 1 monotonic
    //CHECK: [[ADD:%add.*]] = add nsw i64 [[LOAD]], 1
    //CHECK: store i64 [[ADD]], ptr [[N2]], align 8
    #pragma omp atomic capture
    n2 = ++n1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 6)
    marker(6);

    //CHECK: [[LOAD:%[0-9]+]] = atomicrmw add ptr [[N1]], i64 1 seq_cst
    //CHECK: call void @__kmpc_flush
    #pragma omp atomic seq_cst
    ++n1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 7)
    marker(7);

    //CHECK: [[LOAD:%[0-9]+]] = atomicrmw add ptr [[N1]], i64 1 seq_cst
    //CHECK: call void @__kmpc_flush
    #pragma omp atomic seq_cst update
    ++n1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 8)
    marker(8);

    //CHECK: [[AL:%atom.*]] = load atomic i64, ptr [[N1]] seq_cst, align 8
    //CHECK: call void @__kmpc_flush
    //CHECK: store i64 [[AL]], ptr [[N2]], align 8
    #pragma omp atomic read, seq_cst
    n2 = n1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 9)
    marker(9);

    //CHECK: store atomic i64 1, ptr [[N1]] seq_cst, align 8
    //CHECK: call void @__kmpc_flush
    #pragma omp atomic write seq_cst
    n1 = 1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 10)
    marker(10);

    //CHECK: [[LOAD:%[0-9]+]] = atomicrmw add ptr [[N1]], i64 1 seq_cst
    //CHECK: [[ADD:%add.*]] = add nsw i64 [[LOAD]], 1
    //CHECK: store i64 [[ADD]], ptr [[N2]], align 8
    //CHECK: call void @__kmpc_flush
    #pragma omp atomic seq_cst, capture
    n2 = ++n1;

    // 80-bit data to show __atomic calls.

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 11)
    marker(11);

    //CHECK: __atomic_load(i64 noundef 16, ptr noundef [[D1]], ptr noundef [[T1]], i32 noundef 0
    //
    //CHECK: [[LOAD:%[0-9]+]] = load x86_fp80, ptr [[T1]], align 16
    //CHECK: store x86_fp80 [[LOAD]], ptr [[T2]], align 16

    //CHECK: [[LOAD:%[0-9]+]] = load x86_fp80, ptr [[T1]], align 16
    //CHECK: [[ADD:%add.*]] = fadd x86_fp80 [[LOAD]], 0xK3FFF8000000000000000
    //CHECK: store x86_fp80 [[ADD]], ptr [[T2]], align 16

    //CHECK: __atomic_compare_exchange(i64 noundef 16, ptr noundef [[D1]]
    //CHECK-SAME: ptr noundef [[T1]], ptr noundef [[T2]], i32 noundef 0, i32 noundef 0)
    #pragma omp atomic
    ++d1;

    //CHECK: call{{.*}}marker{{.*}}(i32 noundef 12)
    marker(12);

    //CHECK: store x86_fp80 0xK3FFF8000000000000000, ptr [[T3]]
    //CHECK: __atomic_store(i64 noundef 16, ptr noundef [[D1]], ptr noundef [[T3]], i32 noundef 0)
    #pragma omp atomic write
    d1 = 1;
  }
  //CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.PARALLEL"()
}
// end INTEL_COLLAB
