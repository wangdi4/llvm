// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-REG
// RUN: %clang_cc1 -emit-llvm -o - %s -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=CHECK-REG
struct S1 {
  ~S1() {}
};

struct S2 {
  S2() {}
};

struct S3 {
  S3() {}
  S3(const S3& s) : i(s.i) {}
  ~S3() {}
  int i;
};

struct S4 {
  int i;
};

void foo() {}
void bar(int);

int glob_int = 2;
int *glob_ptr;

// CHECK-LABEL: @main
int main(int argc, char **argv) {
  // CHECK: alloca i32,
  // CHECK: [[ARGC_ADDR:%.+]] = alloca i32,
  // CHECK: [[ARGV_ADDR:%.+]] = alloca i8**,
  // CHECK: [[S1_ADDR:%.+]] = alloca %struct.S1,
  // CHECK: [[S2_ADDR:%.+]] = alloca %struct.S2,
  // CHECK: [[S3_ADDR:%.+]] = alloca %struct.S3,
  // CHECK: [[ARR1_ADDR:%.+]] = alloca [10 x %struct.S1],
  // CHECK: [[ARR2_ADDR:%.+]] = alloca [10 x %struct.S2],
  // CHECK: [[S3ARR_ADDR:%.+]] = alloca [10 x %struct.S3],
  // CHECK: [[ARGCREF_ADDR:%.+]] = alloca i32*,
  // CHECK: [[Z_ADDR:%.+]] = alloca i32,
  // CHECK: [[ZARR_ADDR:%.+]] = alloca [20 x i32],
  // CHECK: [[N1_ADDR:%.+]] = alloca i64,
  // CHECK: [[N2_ADDR:%.+]] = alloca i64,
  // CHECK: [[N3_ADDR:%.+]] = alloca i64,
  // CHECK: [[N4_ADDR:%.+]] = alloca i64,
  // CHECK: [[N5_ADDR:%.+]] = alloca i64,
  // CHECK: [[N6_ADDR:%.+]] = alloca i64,
  // CHECK: [[N7_ADDR:%.+]] = alloca i64,
  // CHECK: [[N8_ADDR:%.+]] = alloca i64,
  // CHECK: [[N9_ADDR:%.+]] = alloca i64,
  // CHECK: [[N10_ADDR:%.+]] = alloca i64,
  // CHECK: call i8* @llvm.stacksave()
  // CHECK: [[ARR3_ADDR:%.+]] = alloca double, i64
  S1 s1;
  S2 s2;
  S3 s3;
  S1 arr1[10];
  S2 arr2[10];
  S3 s3arr[10];
  double arr3[argc];
  int &argcref = argc;
  int z = 4;
  int zarr[20];
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", i8*** [[ARGV_ADDR]])

// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", %struct.S1* [[S1_ADDR]], %struct.S1* (%struct.S1*)* @_ZTS2S1.omp.def_constr, void (%struct.S1*)* @_ZTS2S1.omp.destr

// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", %struct.S2* [[S2_ADDR]], %struct.S2* (%struct.S2*)* @_ZTS2S2.omp.def_constr, void (%struct.S2*)* @_ZTS2S2.omp.destr

// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", [10 x %struct.S1]* [[ARR1_ADDR]], [10 x %struct.S1]* ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.def_constr, void ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.destr

// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", [10 x %struct.S2]* [[ARR2_ADDR]], [10 x %struct.S2]* ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.def_constr, void ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.destr

// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", double* [[ARR3_ADDR]])

// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE:NONPOD", %struct.S3* [[S3_ADDR]], void (%struct.S3*, %struct.S3*)* @_ZTS2S3.omp.copy_constr, void (%struct.S3*)* @_ZTS2S3.omp.destr)
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE:NONPOD", [10 x %struct.S3]* [[S3ARR_ADDR]], void ([10 x %struct.S3]*, [10 x %struct.S3]*)* @_ZTSA10_2S3.omp.copy_constr, void ([10 x %struct.S3]*)* @_ZTSA10_2S3.omp.destr)
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i32* [[Z_ADDR]]
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", [20 x i32]* [[ZARR_ADDR]])
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel shared(argv) private(s1, s2, arr1, arr2, arr3) \
                                  firstprivate(s3) firstprivate(s3arr) \
                                  firstprivate(z,zarr)
  // CHECK: call void @_Z3foov()
  foo();
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", i8*** [[ARGV_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", %struct.S1* [[S1_ADDR]], %struct.S1* (%struct.S1*)* @_ZTS2S1.omp.def_constr, void (%struct.S1*)* @_ZTS2S1.omp.destr
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", %struct.S2* [[S2_ADDR]], %struct.S2* (%struct.S2*)* @_ZTS2S2.omp.def_constr, void (%struct.S2*)* @_ZTS2S2.omp.destr
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", [10 x %struct.S1]* [[ARR1_ADDR]], [10 x %struct.S1]* ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.def_constr, void ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.destr
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", [10 x %struct.S2]* [[ARR2_ADDR]], [10 x %struct.S2]* ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.def_constr, void ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.destr)
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", double* [[ARR3_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR", i32* [[ARGC_ADDR]], i32
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel for shared(argv) private(s1, s2, arr1, arr2, arr3) linear(val(argc) : argc)
  for (int i = 0; i < argc; ++i)
    // CHECK: call void @_Z3foov()
    foo();
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

// CHECK: [[ARGCREF:%.+]] = load i32*, i32** [[ARGCREF_ADDR]],
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", %struct.S1* [[S1_ADDR]], %struct.S1* (%struct.S1*)* @_ZTS2S1.omp.def_constr, void (%struct.S1*)* @_ZTS2S1.omp.destr
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", %struct.S2* [[S2_ADDR]], %struct.S2* (%struct.S2*)* @_ZTS2S2.omp.def_constr, void (%struct.S2*)* @_ZTS2S2.omp.destr
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", [10 x %struct.S1]* [[ARR1_ADDR]], [10 x %struct.S1]* ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.def_constr, void ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.destr
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE:NONPOD", [10 x %struct.S2]* [[ARR2_ADDR]], [10 x %struct.S2]* ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.def_constr, void ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.destr)
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", double* [[ARR3_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR", i32* [[ARGCREF]], i32 1)
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR", i8*** [[ARGV_ADDR]], i32 1)
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp simd private(s1, s2, arr1, arr2, arr3) linear(uval(argcref)) linear(argv)
  for (int i = 0; i < argc; ++i)
    // CHECK: call void @_Z3foov()
    foo();
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

  long int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10;
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.ADD", i64* [[N1_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.SUB", i64* [[N2_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.MAX", i64* [[N3_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.MIN", i64* [[N4_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.BAND", i64* [[N5_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.BOR", i64* [[N6_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.AND", i64* [[N7_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.OR", i64* [[N8_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.BXOR", i64* [[N9_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.MUL", i64* [[N10_ADDR]])
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel reduction(+:n1) reduction(-:n2) reduction(max:n3) reduction (min:n4) reduction(&:n5) reduction (|:n6) reduction(&&:n7) reduction(||:n8) reduction(^:n9) reduction(*:n10)
  // CHECK: call void @_Z3foov()
  foo();
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  n1 = 0;
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic update
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.READ")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic read
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.WRITE")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: store i64 1, i64*
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic write
  n1 = 1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.CAPTURE")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic capture
  n2 = ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE.SEQ_CST")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic seq_cst
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE.SEQ_CST")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic seq_cst update
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.READ.SEQ_CST")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic read, seq_cst
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.WRITE.SEQ_CST")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: store i64 1, i64*
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic write seq_cst
  n1 = 1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.CAPTURE.SEQ_CST")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp atomic seq_cst, capture
  n2 = ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp critical
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.MASTER")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.MASTER")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp master
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.SINGLE")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: fence acquire
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: fence release
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.SINGLE")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp single
  n2 = n1;
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: opnd.i32(metadata !"QUAL.OMP.ORDERED", i32 0)
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.ORDERED")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ORDERED")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel for ordered
  for (int i = 0; i < 10; ++i) {
#pragma omp ordered
    n2 = n1;
  }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 0)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 0)
#pragma omp parallel for schedule(static)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.STATIC", i32 64)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 64)
#pragma omp parallel for schedule(static, 64)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC", i32 1)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 1)
#pragma omp parallel for schedule(dynamic)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC", i32 128)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 128)
#pragma omp parallel for schedule(dynamic, 128)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.GUIDED", i32 1)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.GUIDED"(i32 1)
#pragma omp parallel for schedule(guided)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.GUIDED", i32 256)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.GUIDED"(i32 256)
#pragma omp parallel for schedule(guided, 256)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.AUTO", i32 0)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.AUTO"(i32 0)
#pragma omp parallel for schedule(auto)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.RUNTIME", i32 0)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.RUNTIME"(i32 0)
#pragma omp parallel for schedule(runtime)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC", i32 16)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC"(i32 16)
#pragma omp parallel for schedule(monotonic:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC", i32 16)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC"(i32 16)
#pragma omp parallel for schedule(nonmonotonic:dynamic,16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC:SIMD", i32 16)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:SIMD"(i32 16)
#pragma omp parallel for schedule(simd:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC:SIMD.MONOTONIC", i32 16)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:SIMD.MONOTONIC"(i32 16)
#pragma omp parallel for schedule(simd,monotonic:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC:SIMD.NONMONOTONIC", i32 16)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:SIMD.NONMONOTONIC"(i32 16)
#pragma omp parallel for schedule(simd,nonmonotonic:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC.SIMD", i32 16)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC.SIMD"(i32 16)
#pragma omp parallel for schedule(monotonic,simd:dynamic, 16)
  for (int i = 0; i < 10; ++i) { }

//CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC.SIMD", i32 16)
//CHECK-REG: region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC.SIMD"(i32 16)
#pragma omp parallel for schedule(nonmonotonic,simd:dynamic,16)
  for (int i = 0; i < 10; ++i) { }

  int N = 10;
  const int M = 10;
  long input1[N];
  long **input2 = 0;
  long result[M][M][N];
// DISABLE until we resolve ARRSIZE issue
// NOCHECK: [[VLA1:%.+]] = alloca i64
// NOCHECK: [[VLA2:%.+]] = alloca i64
// NOCHECK-NEXT: [[N:%.+]] = load i32, i32* [[N_ADDR:%.+]],
// NOCHECK-NEXT: [[NS:%.+]] = sext i32 [[N]] to i64
// NOCHECK-NEXT: [[N1:%.+]] = load i32, i32* [[N_ADDR]]
// NOCHECK-NEXT: [[N1S:%.+]] = sext i32 [[N1]] to i64
// NOCHECK-NEXT: [[N2:%.+]] = load i32, i32* [[N_ADDR]]
// NOCHECK-NEXT: [[N2S:%.+]] = sext i32 [[N2]] to i64
// NOCHECK-NEXT: [[NA:%.+]] = load i32, i32* [[N_ADDR]]
// NOCHECK-NEXT: [[NAS:%.+]] = sext i32 [[NA]] to i64
// NOCHECK-NEXT: [[SIZE:%.+]] = sub i64 [[NAS]], 0
// NOCHECK-NEXT: [[NA1:%.+]] = load i32, i32* [[N_ADDR]]
// NOCHECK-NEXT: [[NA1S:%.+]] = sext i32 [[NA1]] to i64
// NOCHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.TARGET")
// NOCHECK-NEXT: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.MAP.TO", metadata !"QUAL.OPND.ARRSECT", i64* [[VLA1]], i64 1, metadata !"QUAL.OPND.ARRSIZE", i64 [[N1S]], i64 2, i64 [[NS]], i64 1, metadata !"QUAL.OPND.ARRSECT", i64*** %{{.+}}, i64 2, i64 0, i64 [[N2S]], i64 1, i64 0, i64 10, i64 1)
// NOCHECK-NEXT: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.MAP.FROM", metadata !"QUAL.OPND.ARRSECT", i64* [[VLA2]], i64 3, metadata !"QUAL.OPND.ARRSIZE", i64 10, i64 10, i64 [[NA1S]], i64 2, i64 8, i64 1, i64 0, i64 10, i64 1, i64 0, i64 [[SIZE]], i64 1)
// NOCHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// NOCHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// NOCHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// NOCHECK: load i64, i64*
// NOCHECK: store i64
// NOCHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// NOCHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// NOCHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.TARGET")
// NOCHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp target map(to : input1[2:N], input2[0:N][:M]) map(from : result[2:][:M][0:])
#pragma omp parallel for
  for (int i = 0; i < N; i++)
    result[i][i][i] = input1[i] + input2[i][i];
  {
    int *a, *b;
    int z = 3, y = 9;
// CHECK-REG: [[TARG_TOKENVAL:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.DEVICE"(i32 4), "QUAL.OMP.IS_DEVICE_PTR"(i32** %a{{.*}}, i32** %b{{.*}}), "QUAL.OMP.DEFAULTMAP.TOFROM.SCALAR"(), "QUAL.OMP.NOWAIT"() ]
// CHECK-REG: region.exit(token [[TARG_TOKENVAL]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target device(4) is_device_ptr(a,b) \
                       defaultmap(tofrom:scalar) nowait
    {
    }
// CHECK-REG: [[TARGD_TOKENVAL:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM"(i32* %z{{.*}}), "QUAL.OMP.USE_DEVICE_PTR"(i32** %a{{.*}}, i32** %b{{.*}}) ]
// CHECK-REG: region.exit(token [[TARGD_TOKENVAL]]) [ "DIR.OMP.END.TARGET.DATA"() ]
    #pragma omp target data map(tofrom:z) use_device_ptr(a,b)
    {
    }
// CHECK-REG: [[TARGU_TOKENVAL:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET.UPDATE"(), "QUAL.OMP.TO"(i32* %z{{.*}}), "QUAL.OMP.FROM"(i32* %y{{.*}}) ]
// CHECK-REG: region.exit(token [[TARGU_TOKENVAL]]) [ "DIR.OMP.END.TARGET.UPDATE"() ]
    #pragma omp target update to(z) from(y)
  }
  {
    int local_int = 1;
// CHECK-REG: [[TARG2_TOKENVAL:%[0-9]+]] = call token{{.*}}TARGET{{.*}}IS_DEVICE_PTR{{.*}}glob_ptr{{.*}}FIRSTPRIVATE{{.*}}local_int{{.*}}FIRSTPRIVATE{{.*}}glob_int
// CHECK-REG: region.exit(token [[TARG2_TOKENVAL]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target is_device_ptr(glob_ptr)
    {
      glob_int = local_int + 1;
      glob_ptr++;
    }
    S4 s4;
// CHECK-REG: [[TARG3_TOKENVAL:%[0-9]+]] = call token{{.*}}TARGET{{.*}}MAP.TOFROM{{.*}}%struct.S4
// CHECK-REG: region.exit(token [[TARG3_TOKENVAL]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target
    {
      s4.i = 1;
    }
// CHECK-REG: [[TARG4_TOKENVAL:%[0-9]+]] = call token{{.*}}TARGET{{.*}}DEFAULTMAP.TOFROM.SCALAR{{.*}}MAP.TOFROM{{.*}}glob_int{{.*}}MAP.TOFROM{{.*}}local_int
// CHECK-REG: region.exit(token [[TARG4_TOKENVAL]]) [ "DIR.OMP.END.TARGET"() ]
    #pragma omp target defaultmap(tofrom:scalar)
    {
      glob_int = local_int + 1;
    }
  }
// CHECK-REG: [[TARGTE_TV:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TARGET"()
// CHECK-REG: [[TE_TV:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.TEAMS"(){{.*}}"QUAL.OMP.NUM_TEAMS"(i32 16), "QUAL.OMP.THREAD_LIMIT"(i32 4)
// CHECK-REG: [[DIST_TV:%[0-9]+]] = call token{{.*}}region.entry() [ "DIR.OMP.DISTRIBUTE"(){{.*}}"QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 8)
// CHECK-REG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb
// CHECK-REG-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv
// CHECK-REG-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub
// CHECK-REG-SAME: "QUAL.OMP.PRIVATE"(i32* %i
  #pragma omp target
  #pragma omp teams num_teams(16) thread_limit(4)
  {
    #pragma omp distribute dist_schedule(static, 8)
    for (int i = 0; i < N; i++)
      foo();
  }
// CHECK-REG: region.exit(token [[DIST_TV]]) [ "DIR.OMP.END.DISTRIBUTE"() ]
// CHECK-REG: region.exit(token [[TE_TV]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK-REG: region.exit(token [[TARGTE_TV]]) [ "DIR.OMP.END.TARGET"() ]

// CHECK-REG: [[BARRIER_TOKENVAL:%[0-9]+]] = call token{{.*}}DIR.OMP.BARRIER
// CHECK-REG: fence acq_rel
// CHECK-REG: region.exit(token [[BARRIER_TOKENVAL]]) [ "DIR.OMP.END.BARRIER"
  #pragma omp barrier
// CHECK-REG: [[FLUSH_TOKENVAL:%[0-9]+]] = call token{{.*}}DIR.OMP.FLUSH
// CHECK-REG: region.exit(token [[FLUSH_TOKENVAL]]) [ "DIR.OMP.END.FLUSH"
  #pragma omp flush

  {
    int fli = 3, flj = 4;
// CHECK-REG: [[FLUSH_TOKENVAL1:%[0-9]+]] = call token{{.*}}DIR.OMP.FLUSH{{.*}}QUAL.OMP.FLUSH{{.*}}fli{{.*}}flj
// CHECK-REG: region.exit(token [[FLUSH_TOKENVAL1]]) [ "DIR.OMP.END.FLUSH"
    #pragma omp flush(fli,flj)
  }

// CHECK-REG: [[SECT1ATV:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL
// CHECK-REG: [[SECT1BTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTIONS{{.*}}NOWAIT{{.*}}PRIVATE{{.*}}sect1{{.*}}FIRSTPRIVATE{{.*}}sect2{{.*}}LASTPRIVATE{{.*}}sect3{{.*}}REDUCTION.ADD{{.*}}sect4
// CHECK-REG: [[SECT1CTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK-REG: {{call|invoke}}{{.*}}bari(i32 1)
// CHECK-REG: region.exit(token [[SECT1CTV]]) [ "DIR.OMP.END.SECTION"
// CHECK-REG: [[SECT1DTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK-REG: {{call|invoke}}{{.*}}bari(i32 2)
// CHECK-REG: region.exit(token [[SECT1DTV]]) [ "DIR.OMP.END.SECTION"
// CHECK-REG: [[SECT1ETV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK-REG: {{call|invoke}}{{.*}}bari(i32 3)
// CHECK-REG: region.exit(token [[SECT1ETV]]) [ "DIR.OMP.END.SECTION"
// CHECK-REG: region.exit(token [[SECT1BTV]]) [ "DIR.OMP.END.SECTIONS"
// CHECK-REG: region.exit(token [[SECT1ATV]]) [ "DIR.OMP.END.PARALLEL"
  {
    int sect1=0,sect2=0,sect3=0,sect4=0;
    #pragma omp parallel
    {
      #pragma omp sections nowait private(sect1) firstprivate(sect2) \
                           lastprivate(sect3) reduction(+:sect4)
      {
        #pragma omp section
        {
          bar(1);
        }
        #pragma omp section
        {
          bar(2);
        }
        #pragma omp section
        {
          bar(3);
        }
      }
    }
  }
// CHECK-REG: [[SECT2BTV:%[0-9]+]] = call token{{.*}}DIR.OMP.PARALLEL.SECTIONS{{.*}}PRIVATE{{.*}}sect1{{.*}}FIRSTPRIVATE{{.*}}sect2{{.*}}LASTPRIVATE{{.*}}sect3{{.*}}REDUCTION.ADD{{.*}}sect4
// CHECK-REG: [[SECT2CTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK-REG: {{call|invoke}}{{.*}}bari(i32 1)
// CHECK-REG: region.exit(token [[SECT2CTV]]) [ "DIR.OMP.END.SECTION"
// CHECK-REG: [[SECT2DTV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK-REG: {{call|invoke}}{{.*}}bari(i32 2)
// CHECK-REG: region.exit(token [[SECT2DTV]]) [ "DIR.OMP.END.SECTION"
// CHECK-REG: [[SECT2ETV:%[0-9]+]] = call token{{.*}}DIR.OMP.SECTION
// CHECK-REG: {{call|invoke}}{{.*}}bari(i32 3)
// CHECK-REG: region.exit(token [[SECT2ETV]]) [ "DIR.OMP.END.SECTION"
// CHECK-REG: region.exit(token [[SECT2BTV]]) [ "DIR.OMP.END.PARALLEL.SECTIONS"
  {
    int sect1=0,sect2=0,sect3=0,sect4=0;
    #pragma omp parallel sections private(sect1) firstprivate(sect2) \
                                  lastprivate(sect3) reduction(+:sect4)
    {
      #pragma omp section
      {
        bar(1);
      }
      #pragma omp section
      {
        bar(2);
      }
      #pragma omp section
      {
        bar(3);
      }
    }
  }
  #pragma omp parallel
  {
    // CHECK-REG: DIR.OMP.CRITICAL
    // CHECK-REG-SAME: "QUAL.OMP.NAME"([7 x i8] c"critfoo")
    #pragma omp critical(critfoo)
    n2 = n1;
    // CHECK-REG: DIR.OMP.CRITICAL
    // CHECK-REG-SAME: "QUAL.OMP.NAME"([10 x i8] c"critbarbaz")
    // CHECK-REG-SAME: "QUAL.OMP.HINT"(i32 42)
    #pragma omp critical(critbarbaz) hint(42)
    n2 = n1;
  }
  return 0;
}

// CHECK: define internal %struct.S1* @_ZTS2S1.omp.def_constr(%struct.S1*)
// CHECK: alloca %struct.S1*,
// CHECK-NEXT: alloca %struct.S1*,
// CHECK-NEXT:  store %struct.S1* %{{.+}}, %struct.S1** %
// CHECK-NEXT:  load %struct.S1*, %struct.S1** %
// CHECK-NEXT:  ret %struct.S1* %
// CHECK-NEXT: }

// CHECK: define internal void @_ZTS2S1.omp.destr(%struct.S1*)
// CHECK: alloca %struct.S1*,
// CHECK-NEXT: store %struct.S1* %{{.+}}, %struct.S1** %
// CHECK-NEXT: load %struct.S1*, %struct.S1** %
// CHECK-NEXT: call void @_ZN2S1D1Ev(%struct.S1* %{{.+}})
// CHECK-NEXT: ret void
// CHECK-NEXT: }

// CHECK: define internal %struct.S2* @_ZTS2S2.omp.def_constr(%struct.S2*)
// CHECK: alloca %struct.S2*,
// CHECK-NEXT: store %struct.S2* %{{.+}}, %struct.S2** %
// CHECK-NEXT: load %struct.S2*, %struct.S2** %
// CHECK-NEXT: call void @_ZN2S2C1Ev(%struct.S2* %{{.+}})
// CHECK-NEXT: ret %struct.S2* %
// CHECK-NEXT: }

// CHECK: define internal void @_ZTS2S2.omp.destr(%struct.S2*)
// CHECK: alloca %struct.S2*,
// CHECK-NEXT: store %struct.S2* %{{.+}}, %struct.S2** %
// CHECK-NEXT: ret void
// CHECK-NEXT: }

// CHECK: define internal [10 x %struct.S1]* @_ZTSA10_2S1.omp.def_constr([10 x %struct.S1]*)
// CHECK: alloca [10 x %struct.S1]*,
// CHECK-NEXT: alloca [10 x %struct.S1]*,
// CHECK-NEXT: store [10 x %struct.S1]* %{{.+}}, [10 x %struct.S1]** %
// CHECK-NEXT: load [10 x %struct.S1]*, [10 x %struct.S1]** %
// CHECK-NEXT: ret [10 x %struct.S1]* %
// CHECK-NEXT: }

// CHECK: define internal void @_ZTSA10_2S1.omp.destr([10 x %struct.S1]*)
// CHECK: alloca [10 x %struct.S1]*,
// CHECK-NEXT: store [10 x %struct.S1]* %{{.+}}, [10 x %struct.S1]** %
// CHECK-NEXT: load [10 x %struct.S1]*, [10 x %struct.S1]** %
// CHECK:  call void @_ZN2S1D1Ev(%struct.S1* %{{.+}})
// CHECK:  ret void
// CHECK-NEXT: }

// CHECK: define internal [10 x %struct.S2]* @_ZTSA10_2S2.omp.def_constr([10 x %struct.S2]*)
// CHECK: alloca [10 x %struct.S2]*,
// CHECK-NEXT: store [10 x %struct.S2]* %{{.+}}, [10 x %struct.S2]** %
// CHECK-NEXT: load [10 x %struct.S2]*, [10 x %struct.S2]** %
// CHECK: call void @_ZN2S2C1Ev(%struct.S2* %{{.+}})
// CHECK: ret [10 x %struct.S2]* %
// CHECK-NEXT: }

// CHECK: define internal void @_ZTSA10_2S2.omp.destr([10 x %struct.S2]*)
// CHECK: alloca [10 x %struct.S2]*,
// CHECK-NEXT: store [10 x %struct.S2]* %{{.+}}, [10 x %struct.S2]** %
// CHECK-NEXT: ret void
// CHECK-NEXT: }

