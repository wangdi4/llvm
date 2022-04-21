// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm-bc -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-late-outline \
// RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s -DHOST

// RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-host-ir-file-path %t_host.bc \
// RUN:  -Wsource-uses-openmp -o - %s -DHOST \
// RUN:  | FileCheck --check-prefixes=HOST %s

// Compiling for device target
// RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
// RUN: -emit-llvm-bc -disable-llvm-passes \
// RUN: -fopenmp -fopenmp-targets=spir64 \
// RUN: -fopenmp-late-outline -fopenmp-target-malloc \
// RUN: -o %t_spir.bc %s

// RUN: %clang_cc1 -no-opaque-pointers -triple spir64 \
// RUN: -aux-triple x86_64-unknown-linux-gnu \
// RUN: -emit-llvm -disable-llvm-passes \
// RUN: -fopenmp -fopenmp-targets=spir64 \
// RUN: -fopenmp-late-outline -fopenmp-target-malloc \
// RUN: -fopenmp-is-device -fopenmp-host-ir-file-path %t_spir.bc -o - \
// RUN: %s | FileCheck --check-prefixes=TARGET %s

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

#ifdef HOST
// HOST-LABEL: @"@tid.addr"
void foo() {
  // HOST: "DIR.OMP.PARALLEL"()
  // HOST-DAG: [[MYTID:%my.tid[0-9]*]] = load i32, i32* @"@tid.addr"
  // HOST-DAG: [[VA:%.bar..void.addr]] = call i8* @__kmpc_alloc(i32 [[MYTID]], i64 4, i8* inttoptr (i64 7 to i8*))
  // HOST-DAG: [[MA:%.bar..addr]] = bitcast i8* [[VA]] to i32*
  // HOST-DAG: [[BC:%[0-9]*]] = bitcast i32* [[MA]] to i8*
  // HOST-DAG: call void @__kmpc_free(i32 [[MYTID]], i8* [[BC]], i8* inttoptr (i64 7 to i8*)
  // HOST: "DIR.OMP.END.PARALLEL"()
  #pragma omp parallel
  for(int t = 0; t < 32; ++t) {
    int bar;
    #pragma omp allocate(bar) allocator(omp_pteam_mem_alloc)
  }
}
#else // HOST
// TARGET-LABEL: @"@tid.addr"
void foo_target() {
  // TARGET: "DIR.OMP.TARGET"()
  // TARGET: "DIR.OMP.PARALLEL"()
  // TARGET-DAG: [[MYTID:%my.tid[0-9]*]] = load i32, i32* @"@tid.addr"
  // TARGET-DAG: [[VA:%.bar..void.addr]] = call spir_func i8 addrspace(4)* @__kmpc_alloc(i32 [[MYTID]], i64 4, i8 addrspace(4)* inttoptr (i64 7 to i8 addrspace(4)*))
  // TARGET-DAG: [[MA:%.bar..addr]] = bitcast i8 addrspace(4)* [[VA]] to i32 addrspace(4)*
  // TARGET-DAG: [[BC:%[0-9]*]] = bitcast i32 addrspace(4)* %.bar..addr to i8 addrspace(4)*
  // TARGET-DAG: call spir_func void @__kmpc_free(i32 [[MYTID]], i8 addrspace(4)* [[BC]], i8 addrspace(4)* inttoptr (i64 7 to i8 addrspace(4)*))
  // TARGET-DAG: "DIR.OMP.END.PARALLEL"()
  // TARGET: "DIR.OMP.END.TARGET"()
  #pragma omp target
  #pragma omp parallel
  for(int t = 0; t < 32; ++t) {
    int bar;
    #pragma omp allocate(bar) allocator(omp_pteam_mem_alloc)
  }
}
#endif // HOST
// end INTEL_COLLAB
