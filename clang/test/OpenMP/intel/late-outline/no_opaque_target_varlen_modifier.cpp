// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s | FileCheck %s

extern "C" int printf(const char *, ...);
void foo() {
  short n = 2;
  int x[n];

  //CHECK:DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:VARLEN"(i32* %vla
  //CHECK-SAME: "QUAL.OMP.MAP.TO:VARLEN"(i32* %vla
  //CHECK:DIR.OMP.END.TARGET
  #pragma omp target firstprivate(x)
  printf("%p\n", x);

  //CHECK:DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.PRIVATE:VARLEN"(i32* %vla
  //CHECK:DIR.OMP.END.TARGET
  #pragma omp target private(x)
  printf("%p\n", x);

  //CHECK:DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:VARLEN"(i32* %vla
  //CHECK:DIR.OMP.END.TARGET
  #pragma omp target
  printf("%p\n", x);
}

// end INTEL_COLLAB
