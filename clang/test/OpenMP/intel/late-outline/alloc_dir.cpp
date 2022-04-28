// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// Verify use of allocate directive referencing all valid pre-defined
// memory allocators.

enum omp_allocator_handle_t {
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
};

int main() {
  int i;
  int v1 = 0;
  int v2 = 10;
  int v3 = 20;
  int v4 = 30;
  int v5 = 40;
  int v6 = 50;
  int v7 = 60;
  int v8 = 70;
  int v9 = 80;

  // CHECK: [[V1_ALLOC:%.v1..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr null)
  // CHECK-NEXT: store i32 0, ptr [[V1_ALLOC]], align 4
  // CHECK: [[V2_ALLOC:%.v2..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 1 to ptr))
  // CHECK-NEXT: store i32 10, ptr [[V2_ALLOC]], align 4
  // CHECK: [[V3_ALLOC:%.v3..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 2 to ptr))
  // CHECK-NEXT: store i32 20, ptr [[V3_ALLOC]], align 4
  // CHECK: [[V4_ALLOC:%.v4..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 3 to ptr))
  // CHECK-NEXT: store i32 30, ptr [[V4_ALLOC]], align 4
  // CHECK: [[V5_ALLOC:%.v5..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 4 to ptr))
  // CHECK-NEXT: store i32 40, ptr [[V5_ALLOC]], align 4
  // CHECK: [[V6_ALLOC:%.v6..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 5 to ptr))
  // CHECK-NEXT: store i32 50, ptr [[V6_ALLOC]], align 4
  // CHECK: [[V7_ALLOC:%.v7..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 6 to ptr))
  // CHECK-NEXT: store i32 60, ptr [[V7_ALLOC]], align 4
  // CHECK: [[V8_ALLOC:%.v8..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 7 to ptr))
  // CHECK-NEXT: store i32 70, ptr [[V8_ALLOC]], align 4
  // CHECK: [[V9_ALLOC:%.v9..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 8 to ptr))
  // CHECK-NEXT: store i32 80, ptr [[V9_ALLOC]], align 4

#pragma omp allocate(v1) allocator(omp_null_allocator)
#pragma omp allocate(v2) allocator(omp_default_mem_alloc)
#pragma omp allocate(v3) allocator(omp_large_cap_mem_alloc)
#pragma omp allocate(v4) allocator(omp_const_mem_alloc)
#pragma omp allocate(v5) allocator(omp_high_bw_mem_alloc)
#pragma omp allocate(v6) allocator(omp_low_lat_mem_alloc)
#pragma omp allocate(v7) allocator(omp_cgroup_mem_alloc)
#pragma omp allocate(v8) allocator(omp_pteam_mem_alloc)
#pragma omp allocate(v9) allocator(omp_thread_mem_alloc)

  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[V1_ALLOC]]
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[V2_ALLOC]]
  // CHECK: DIR.OMP.END.PARALLEL
#pragma omp parallel
  { v1 = v2; }

  // CHECK: [[V1ALLOC:%.v1..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 2 to ptr))
  // CHECK-NEXT: [[V2ALLOC:%.v2..void.addr[0-9]*]] = call ptr @__kmpc_alloc(i32 %0, i64 4, ptr inttoptr (i64 3 to ptr))

  // CHECK: DIR.OMP.LOOP
  // CHECK-LABEL: omp.loop.exit:
  // CHECK: DIR.OMP.END.LOOP

  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V2ALLOC]], ptr inttoptr (i64 3 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V1ALLOC]], ptr inttoptr (i64 2 to ptr))
  {
    int v1;
    int v2;
#pragma omp allocate(v1) allocator(omp_large_cap_mem_alloc)
#pragma omp allocate(v2) allocator(omp_const_mem_alloc)
#pragma omp for
    for(i=0; i < 10; i++)
      v1 += v2;
  }

  // CHECK: DIR.OMP.SINGLE
  // CHECK: DIR.OMP.END.SINGLE

  // CHECK: call void @__kmpc_free(i32 %0, ptr [[V9_ALLOC]], ptr inttoptr (i64 8 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V8_ALLOC]], ptr inttoptr (i64 7 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V7_ALLOC]], ptr inttoptr (i64 6 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V6_ALLOC]], ptr inttoptr (i64 5 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V5_ALLOC]], ptr inttoptr (i64 4 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V4_ALLOC]], ptr inttoptr (i64 3 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V3_ALLOC]], ptr inttoptr (i64 2 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V2_ALLOC]], ptr inttoptr (i64 1 to ptr))
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, ptr [[V1_ALLOC]], ptr null)
#pragma omp single
  { v1 -= (v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9); }
  return v1;
}
// end INTEL_COLLAB
