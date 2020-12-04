// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -disable-llvm-passes    \
// RUN:  -debug-info-kind=limited -dwarf-version=4                \
// RUN:  -mllvm -debug-line-version=2 %s | FileCheck %s

int a;
struct c { ~c(); };
c::~c() { a--; }

struct g { c e; void f(); };

void g::f() {
  #pragma omp for lastprivate(e)
  for (int h = 0; h < 2; h++)
    ;
  #pragma omp for firstprivate(e)
  for (int h = 0; h < 2; h++)
    ;
}

// CHECK: define {{.*}}omp.def_constr
// CHECK-NOT: dbg
// CHECK: define {{.*}}omp.copy_assign
// CHECK-NOT: dbg
// CHECK: define {{.*}}omp.destr
// CHECK-NOT: dbg
// CHECK: define {{.*}}omp.copy_constr
// CHECK-NOT: dbg
// CHECK: attributes

// end INTEL_COLLAB
