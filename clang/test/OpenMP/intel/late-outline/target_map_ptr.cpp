// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

double *f_global;

// CHECK-LABEL: foo
void foo(double *f_local) {
  int i;
// CHECK: [[F_L:%f_local.addr]] = alloca ptr,
// CHECK: [[P_L_MAP:%f_local.map.ptr.tmp]] = alloca ptr,
// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[P_L_MAP]]) ]
// CHECK: store ptr {{.*}}, ptr [[P_L_MAP]]
// CHECK: load ptr, ptr [[P_L_MAP]]

#pragma omp target  map(f_global[:100], f_local[:100])
    {
        f_global[i] = i;
        f_local[i] = i;
    }
}

// end INTEL_COLLAB
