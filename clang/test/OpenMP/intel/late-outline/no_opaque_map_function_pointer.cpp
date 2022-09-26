// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -fopenmp-typed-clauses -no-opaque-pointers \
// RUN:   -triple x86_64-unknown-linux-gnu -x c++ %s | FileCheck %s

extern void foo(void);
void bar() {
    void (*fptr)(void) = foo;
// CHECK: "DIR.OMP.TARGET"()
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM:FPTR"(void ()* %0
// CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"(void ()** %fptr)
#pragma omp target firstprivate(fptr)
    fptr();
// CHECK: "DIR.OMP.END.TARGET"()
// CHECK: "DIR.OMP.TARGET.UPDATE"()
// CHECK-SAME: "QUAL.OMP.MAP.TO:FPTR"(void ()** %fptr
#pragma omp target update to(fptr)
// CHECK: "DIR.OMP.END.TARGET.UPDATE"

}
// end INTEL_COLLAB
