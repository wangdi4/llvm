// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm -o - %s \
// RUN:  | FileCheck %s  --check-prefix HOST
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
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
//HOST: [[ARR:@arr]] = target_declare global i32* null, align 8
//HOST: [[ATAG:@arr_decl_tgt_ref_ptr]] = weak target_declare global i32** [[ARR]]
//HOST: [[Nodes_Base:@.+]] = target_declare global %struct.NODE* null
//HOST: [[Nodes_TGT_REF_PTR:@.+]] = weak target_declare global %struct.NODE** [[Nodes_Base]]
//HOST: [[ARRAY:@array]] = target_declare global [256 x float] zeroinitializer, align 16
//HOST: [[ARRTAG:@array_decl_tgt_ref_ptr]] = weak target_declare global [256 x float]* [[ARRAY]]

//TARG-DAG: [[ARRTAG:@array_decl_tgt_ref_ptr]] = weak target_declare addrspace(1) global [256 x float] addrspace(4)* null
//TARG-DAG: [[ATAG:@arr_decl_tgt_ref_ptr]] = weak target_declare addrspace(1) global i32 addrspace(4)* addrspace(4)* null
//TARG-DAG: [[Nodes_TGT_REF_PTR:@.+]] = weak target_declare addrspace(1) global %struct.NODE{{.*}} addrspace(4)* addrspace(4)* null

void * malloc(unsigned long);
struct NODE
{
  int a;
  int b;
};
struct NODE *Nodes_base;
#pragma omp declare target to(Nodes_base)
void bar()
{
   Nodes_base = (NODE *)malloc(8);
   Nodes_base[0].a = 1;
   Nodes_base[0].b = 2;
   #pragma omp target map(tofrom:Nodes_base[0:2])
     {
       Nodes_base[0].a = 3;
       Nodes_base[0].b = 4;
     }
}
int main() {
  bar();
}

//HOST: @_Z3barv()
//HOST: [[LOAD1:%.+]] = load %struct.NODE*, %struct.NODE** [[Nodes_Base]]
//HOST: [[LOAD2:%.+]] = load %struct.NODE**, %struct.NODE*** [[Nodes_TGT_REF_PTR]]
//HOST: [[LOAD3:%.+]] = load %struct.NODE*, %struct.NODE** [[LOAD2]]

//TARG: @_Z3barv()
//TARG: [[LOAD1:%.+]] = load {{.*}} [[Nodes_TGT_REF_PTR]]
//TARG: [[LOAD2:%.+]] = load {{.*}} [[Nodes_TGT_REF_PTR]]
//TARG: [[LOAD3:%.+]] = load {{.*}} [[LOAD2]]

//HOST: !{i32 1, !"Nodes_base_decl_tgt_ref_ptr", i32 0, i32 {{.*}}, %struct.NODE*** [[Nodes_TGT_REF_PTR]]
//HOST: !{i32 1, !"arr_decl_tgt_ref_ptr", i32 0, i32 {{.*}}, i32*** [[ATAG]]}
//HOST: !{i32 1, !"array_decl_tgt_ref_ptr", i32 0, i32 {{.*}}, [256 x float]** [[ARRTAG]]}
//TARG-DAG: !{i32 1, !"array_decl_tgt_ref_ptr", i32 0, i32 {{.*}}, [256 x float] addrspace(4)* addrspace(1)*  [[ARRTAG]]}
//TARG-DAG: !{i32 1, !"Nodes_base_decl_tgt_ref_ptr", i32 0, i32 1, %struct.NODE addrspace(4)* addrspace(4)* addrspace(1)* @Nodes_base_decl_tgt_ref_ptr}
//TARG-DAG: !{i32 1, !"arr_decl_tgt_ref_ptr", i32 0, i32 {{.*}}, i32 addrspace(4)* addrspace(4)* addrspace(1)* [[ATAG]]}
//end INTEL_COLLAB
