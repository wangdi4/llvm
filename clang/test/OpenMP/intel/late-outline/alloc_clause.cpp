// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -x c -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// Verify allocate clause is emitted properly when late outlining.

typedef enum omp_allocator_handle_t {
  omp_null_allocator = 0,
  omp_default_mem_alloc = 1,
  omp_large_cap_mem_alloc = 2,
  omp_const_mem_alloc = 3,
  omp_high_bw_mem_alloc = 4,
  omp_low_lat_mem_alloc = 5,
  omp_cgroup_mem_alloc = 6,
  omp_pteam_mem_alloc = 7,
  omp_thread_mem_alloc = 8,
  KMP_ALLOCATOR_MAX_HANDLE = __UINTPTR_MAX__
} omp_allocator_handle_t;

int main() {
  int i;
  char v1 = 0;
  short v2 = 10;
  int v3 = 20;

  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: "QUAL.OMP.ALLOCATE"(i64 1, ptr %v1)
  // CHECK: DIR.OMP.END.PARALLEL
#pragma omp parallel allocate(v1) firstprivate(v1, v2)
  { v1 = v2; }

  // CHECK: "DIR.OMP.LOOP"()
  // CHECK-SAME: "QUAL.OMP.ALLOCATE"(i64 1, ptr %v1, i64 5)
  // CHECK-SAME: "QUAL.OMP.ALLOCATE"(i64 2, ptr %v2, i64 5)
  // CHECK: DIR.OMP.END.LOOP
#pragma omp for allocate(omp_low_lat_mem_alloc: v1, v2) private(v1, v2)
  for(i=0; i < 10; i++)
    v1 += v2;

  // CHECK: store i64 2, ptr %MyAlloc
  // CHECK-NEXT:[[L1:%[0-9]+]] = load i64, ptr %MyAlloc
  // CHECK: DIR.OMP.SINGLE
  // CHECK-SAME: "QUAL.OMP.ALLOCATE"(i64 1, ptr %v1, i64 [[L1]])
  // CHECK-SAME: "QUAL.OMP.ALLOCATE"(i64 2, ptr %v2, i64 [[L1]])
  // CHECK-SAME: "QUAL.OMP.ALLOCATE"(i64 4, ptr %v3, i64 [[L1]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr %v1)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr %v2)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr %v3)
  // CHECK: DIR.OMP.END.SINGLE
  omp_allocator_handle_t MyAlloc = omp_large_cap_mem_alloc;
#pragma omp single allocate(MyAlloc: v1, v2, v3) private(v1, v2, v3)
  { v1 -= (v2 + v3); }

  return 0;
}
// end INTEL_COLLAB
