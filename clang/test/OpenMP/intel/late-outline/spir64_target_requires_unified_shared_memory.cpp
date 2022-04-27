// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm -o - %s \
// RUN:  | FileCheck %s  --check-prefix HOST
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - %s \
// RUN:  | FileCheck %s  --check-prefix TARG
//
// expected-no-diagnostics

#define SIZE 256

#pragma omp requires unified_shared_memory

#pragma omp declare target
float array[SIZE];
#pragma omp end declare target

int *arr;
#pragma omp declare target to(arr)

void foo() {
  #pragma omp target
  array[1] = 0;
  #pragma omp target
  arr[0] = 1;
}
//HOST: [[ARR:@arr]] = global ptr null, align 8
//HOST: [[ATAG:@arr_decl_tgt_ref_ptr]] = weak target_declare global ptr [[ARR]]
//HOST: [[ARRAY:@array]] = global [256 x float] zeroinitializer, align 16
//HOST: [[ARRTAG:@array_decl_tgt_ref_ptr]] = weak target_declare global ptr [[ARRAY]]
//HOST: !{i32 1, !"array_decl_tgt_ref_ptr", i32 0, i32 1, ptr [[ARRTAG]]}
//HOST: !{i32 1, !"arr_decl_tgt_ref_ptr", i32 0, i32 0, ptr [[ATAG]]}
//TARG: [[ARRTAG:@array_decl_tgt_ref_ptr]] = weak target_declare addrspace(1) global ptr addrspace(4) null
//TARG: [[ATAG:@arr_decl_tgt_ref_ptr]] = weak target_declare addrspace(1) global ptr addrspace(4) null
//TARG: !{i32 1, !"array_decl_tgt_ref_ptr", i32 0, i32 1, ptr addrspace(1) [[ARRTAG]]}
//TARG: !{i32 1, !"arr_decl_tgt_ref_ptr", i32 0, i32 0, ptr addrspace(1) [[ATAG]]}
//end INTEL_COLLAB
