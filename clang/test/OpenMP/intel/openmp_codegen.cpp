// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple %itanium_abi_triple | FileCheck %s
struct S1 {
  ~S1() {}
};

struct S2 {
  S2() {}
};

void foo() {}

// CHECK-LABEL: @main
int main(int argc, char **argv) {
  // CHECK: alloca i32,
  // CHECK: [[ARGC_ADDR:%.+]] = alloca i32,
  // CHECK: [[ARGV_ADDR:%.+]] = alloca i8**,
  // CHECK: [[S1_ADDR:%.+]] = alloca %struct.S1,
  // CHECK: [[S2_ADDR:%.+]] = alloca %struct.S2,
  // CHECK: [[ARR1_ADDR:%.+]] = alloca [10 x %struct.S1],
  // CHECK: [[ARR2_ADDR:%.+]] = alloca [10 x %struct.S2],
  // CHECK: [[ARGCREF_ADDR:%.+]] = alloca i32*,
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
  S1 arr1[10];
  S2 arr2[10];
  double arr3[argc];
  int &argcref = argc;
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", i8*** [[ARGV_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", metadata !"QUAL.OPND.NONPOD", %struct.S1* [[S1_ADDR]], %struct.S1* (%struct.S1*)* @_ZTS2S1.omp.def_constr, void (%struct.S1*)* @_ZTS2S1.omp.destr, metadata !"QUAL.OPND.NONPOD", %struct.S2* [[S2_ADDR]], %struct.S2* (%struct.S2*)* @_ZTS2S2.omp.def_constr, void (%struct.S2*)* @_ZTS2S2.omp.destr, metadata !"QUAL.OPND.NONPOD", [10 x %struct.S1]* [[ARR1_ADDR]], [10 x %struct.S1]* ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.def_constr, void ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.destr, metadata !"QUAL.OPND.NONPOD", [10 x %struct.S2]* [[ARR2_ADDR]], [10 x %struct.S2]* ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.def_constr, void ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.destr, double* [[ARR3_ADDR]])
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel shared(argv) private(s1, s2, arr1, arr2, arr3)
  // CHECK: call void @_Z3foov()
  foo();
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.SHARED", i8*** [[ARGV_ADDR]])
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", metadata !"QUAL.OPND.NONPOD", %struct.S1* [[S1_ADDR]], %struct.S1* (%struct.S1*)* @_ZTS2S1.omp.def_constr, void (%struct.S1*)* @_ZTS2S1.omp.destr, metadata !"QUAL.OPND.NONPOD", %struct.S2* [[S2_ADDR]], %struct.S2* (%struct.S2*)* @_ZTS2S2.omp.def_constr, void (%struct.S2*)* @_ZTS2S2.omp.destr, metadata !"QUAL.OPND.NONPOD", [10 x %struct.S1]* [[ARR1_ADDR]], [10 x %struct.S1]* ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.def_constr, void ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.destr, metadata !"QUAL.OPND.NONPOD", [10 x %struct.S2]* [[ARR2_ADDR]], [10 x %struct.S2]* ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.def_constr, void ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.destr, double* [[ARR3_ADDR]])
// CHECK: [[ARGC_VAL:%.+]] = load i32, i32* [[ARGC_ADDR]],
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR.VAL", i32* [[ARGC_ADDR]], i32 [[ARGC_VAL]])
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel for shared(argv) private(s1, s2, arr1, arr2, arr3) linear(val(argc) : argc)
  for (int i = 0; i < argc; ++i)
    // CHECK: call void @_Z3foov()
    foo();
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")

// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", metadata !"QUAL.OPND.NONPOD", %struct.S1* [[S1_ADDR]], %struct.S1* (%struct.S1*)* @_ZTS2S1.omp.def_constr, void (%struct.S1*)* @_ZTS2S1.omp.destr, metadata !"QUAL.OPND.NONPOD", %struct.S2* [[S2_ADDR]], %struct.S2* (%struct.S2*)* @_ZTS2S2.omp.def_constr, void (%struct.S2*)* @_ZTS2S2.omp.destr, metadata !"QUAL.OPND.NONPOD", [10 x %struct.S1]* [[ARR1_ADDR]], [10 x %struct.S1]* ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.def_constr, void ([10 x %struct.S1]*)* @_ZTSA10_2S1.omp.destr, metadata !"QUAL.OPND.NONPOD", [10 x %struct.S2]* [[ARR2_ADDR]], [10 x %struct.S2]* ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.def_constr, void ([10 x %struct.S2]*)* @_ZTSA10_2S2.omp.destr, double* [[ARR3_ADDR]])
// CHECK: [[ARGCREF:%.+]] = load i32*, i32** [[ARGCREF_ADDR]],
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR.UVAL", i32* [[ARGCREF]], i32 1)
// CHECK: call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LINEAR.VAL", i8*** [[ARGV_ADDR]], i32 1)
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
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic update
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.READ")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic read
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.WRITE")
// CHECK-NEXT: store i64 1, i64*
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic write
  n1 = 1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.CAPTURE")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic capture
  n2 = ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE.SEQ_CST")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic seq_cst
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE.SEQ_CST")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic seq_cst update
  ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.READ.SEQ_CST")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic read, seq_cst
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.WRITE.SEQ_CST")
// CHECK-NEXT: store i64 1, i64*
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic write seq_cst
  n1 = 1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.CAPTURE.SEQ_CST")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: add nsw i64 %{{.+}}, 1
// CHECK-NEXT: store i64
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
#pragma omp atomic seq_cst, capture
  n2 = ++n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.CRITICAL")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.CRITICAL")
#pragma omp critical
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.MASTER")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.MASTER")
#pragma omp master
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.SINGLE")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.SINGLE")
#pragma omp single
  n2 = n1;
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.PARALLEL.LOOP")
// CHECK-NEXT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.ORDERED")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.ORDERED")
// CHECK-NEXT: load i64, i64*
// CHECK-NEXT: store i64
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.OMP.END.ORDERED")
// CHECK: call void @llvm.intel.directive(metadata !"DIR.OMP.END.PARALLEL.LOOP")
// CHECK-NEXT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
#pragma omp parallel for ordered
  for (int i = 0; i < 10; ++i) {
#pragma omp ordered
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

