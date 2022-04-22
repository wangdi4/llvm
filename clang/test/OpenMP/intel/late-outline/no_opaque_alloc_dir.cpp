// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
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

  // CHECK: [[V1_ALLOC:%.v1..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* null)
  // CHECK-NEXT: [[V1ADDR:%.v1..addr[0-9]*]] = bitcast i8* [[V1_ALLOC]] to i32*
  // CHECK-NEXT: store i32 0, i32* [[V1ADDR]], align 4
  // CHECK: [[V2_ALLOC:%.v2..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 1 to i8*))
  // CHECK-NEXT: [[V2ADDR:%.v2..addr[0-9]*]] = bitcast i8* [[V2_ALLOC]] to i32*
  // CHECK-NEXT: store i32 10, i32* [[V2ADDR]], align 4
  // CHECK: [[V3_ALLOC:%.v3..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 2 to i8*))
  // CHECK-NEXT: [[V3ADDR:%.v3..addr[0-9]*]] = bitcast i8* [[V3_ALLOC]] to i32*
  // CHECK-NEXT: store i32 20, i32* [[V3ADDR]], align 4
  // CHECK: [[V4_ALLOC:%.v4..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 3 to i8*))
  // CHECK-NEXT: [[V4ADDR:%.v4..addr[0-9]*]] = bitcast i8* [[V4_ALLOC]] to i32*
  // CHECK-NEXT: store i32 30, i32* [[V4ADDR]], align 4
  // CHECK: [[V5_ALLOC:%.v5..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 4 to i8*))
  // CHECK-NEXT: [[V5ADDR:%.v5..addr[0-9]*]] = bitcast i8* [[V5_ALLOC]] to i32*
  // CHECK-NEXT: store i32 40, i32* [[V5ADDR]], align 4
  // CHECK: [[V6_ALLOC:%.v6..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 5 to i8*))
  // CHECK-NEXT: [[V6ADDR:%.v6..addr[0-9]*]] = bitcast i8* [[V6_ALLOC]] to i32*
  // CHECK-NEXT: store i32 50, i32* [[V6ADDR]], align 4
  // CHECK: [[V7_ALLOC:%.v7..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 6 to i8*))
  // CHECK-NEXT: [[V7ADDR:%.v7..addr[0-9]*]] = bitcast i8* [[V7_ALLOC]] to i32*
  // CHECK-NEXT: store i32 60, i32* [[V7ADDR]], align 4
  // CHECK: [[V8_ALLOC:%.v8..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 7 to i8*))
  // CHECK-NEXT: [[V8ADDR:%.v8..addr[0-9]*]] = bitcast i8* [[V8_ALLOC]] to i32*
  // CHECK-NEXT: store i32 70, i32* [[V8ADDR]], align 4
  // CHECK: [[V9_ALLOC:%.v9..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 8 to i8*))
  // CHECK-NEXT: [[V9ADDR:%.v9..addr[0-9]*]] = bitcast i8* [[V9_ALLOC]] to i32*
  // CHECK-NEXT: store i32 80, i32* [[V9ADDR]], align 4

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
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* %.v1..addr)
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* %.v2..addr)
  // CHECK: DIR.OMP.END.PARALLEL
#pragma omp parallel
  { v1 = v2; }

  // CHECK: [[V1ALLOC:%.v1..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 2 to i8*))
  // CHECK-NEXT: [[V1:%.v1..addr[0-9]*]] = bitcast i8* [[V1ALLOC]] to i32*
  // CHECK-NEXT: [[V2ALLOC:%.v2..void.addr[0-9]*]] = call i8* @__kmpc_alloc(i32 %0, i64 4, i8* inttoptr (i64 3 to i8*))
  // CHECK-NEXT: [[V2:%.v2..addr[0-9]*]] = bitcast i8* [[V2ALLOC]] to i32*

  // CHECK: DIR.OMP.LOOP
  // CHECK-LABEL: omp.loop.exit:
  // CHECK: DIR.OMP.END.LOOP

  // CHECK: [[ADDRV2:%[0-9]+]] = bitcast i32* [[V2]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[ADDRV2]], i8* inttoptr (i64 3 to i8*))
  // CHECK-NEXT: [[ADDRV1:%[0-9]+]] = bitcast i32* [[V1]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[ADDRV1]], i8* inttoptr (i64 2 to i8*))
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

  // CHECK: [[V9CAST:%[0-9]+]] = bitcast i32* [[V9ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V9CAST]], i8* inttoptr (i64 8 to i8*))
  // CHECK-NEXT: [[V8CAST:%[0-9]+]] = bitcast i32* [[V8ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V8CAST]], i8* inttoptr (i64 7 to i8*))
  // CHECK-NEXT: [[V7CAST:%[0-9]+]] = bitcast i32* [[V7ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V7CAST]], i8* inttoptr (i64 6 to i8*))
  // CHECK-NEXT: [[V6CAST:%[0-9]+]] = bitcast i32* [[V6ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V6CAST]], i8* inttoptr (i64 5 to i8*))
  // CHECK-NEXT: [[V5CAST:%[0-9]+]] = bitcast i32* [[V5ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V5CAST]], i8* inttoptr (i64 4 to i8*))
  // CHECK-NEXT: [[V4CAST:%[0-9]+]] = bitcast i32* [[V4ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V4CAST]], i8* inttoptr (i64 3 to i8*))
  // CHECK-NEXT: [[V3CAST:%[0-9]+]] = bitcast i32* [[V3ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V3CAST]], i8* inttoptr (i64 2 to i8*))
  // CHECK-NEXT: [[V2CAST:%[0-9]+]] = bitcast i32* [[V2ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V2CAST]], i8* inttoptr (i64 1 to i8*))
  // CHECK-NEXT: [[V1CAST:%[0-9]+]] = bitcast i32* [[V1ADDR]] to i8*
  // CHECK-NEXT: call void @__kmpc_free(i32 %0, i8* [[V1CAST]], i8* null)
#pragma omp single
  { v1 -= (v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9); }
  return v1;
}
// end INTEL_COLLAB
