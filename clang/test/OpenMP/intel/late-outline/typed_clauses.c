// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp  \
// RUN:  -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -fopenmp-version=51 -emit-llvm %s -o - \
// RUN:  -fopenmp-typed-clauses | FileCheck %s
//
// expected-no-diagnostics

//CHECK: define {{.*}}test_linear_function_pointer
void test_linear_function_pointer() {

  //CHECK: [[FPTR:%fptr.*]] = alloca ptr, align 8
  void (*fptr)(void);

  // function pointer type is i8.
  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(ptr [[FPTR]], i8 0, i32 1, i32 2)
    #pragma omp parallel for linear(fptr : 2)
    for (int i = 0; i < 10; ++i) {
        fptr+=2;
    }
}
// end INTEL_COLLAB
