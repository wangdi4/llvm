// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

typedef struct
{
  int *ptrBase;
  int *valBase;
  int *ptrBase1;
} Base;
typedef struct foo : public Base
{
  int *ptr;
  int *ptr2;
  int val;
   int *ptr1;
} StructWithPtr;

void map_with_overlap_elems() {
  StructWithPtr s;
//CHECK:[[S:%s]] = alloca %struct.foo, align 8
//CHECK:[[SASC:%s.ascast]] = addrspacecast ptr [[S]] to ptr addrspace(4)
//CHECK: [[ptrBase:%[^ ]+]] = getelementptr inbounds %struct.Base, ptr addrspace(4) [[SASC]], i32 0, i32 2
//CHECK: [[L27:%[0-9]+]] = getelementptr %struct.foo, ptr addrspace(4) [[SASC]], i32 1
  #pragma omp target map(to:s, s.ptr1 [0:1], s.ptrBase1 [0:1])
  {
   s.val++; s.ptr1[0]++; s.ptrBase1[0] = 10001;
  }
}

int N = 10;
int *x;
void test_delete() {
  int i;
//CHECK-NOT: %x.map.ptr.tmp
#pragma omp target map(to: x[:N])
  x[i]=1;
}

typedef struct { int a; double *b; } C;
#pragma omp declare mapper(id: C s) map(s.a)

void foo() {
  C s;
  s.a = 10;
  double x[2]; x[0] = 20;
  s.b = &x[0];
  #pragma omp target map(mapper(id), to:s)
  s.a++;
}
//CHECK: declare void @__tgt_push_mapper_component(ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), i64, i64, ptr addrspace(4))
//CHECK: declare i64 @__tgt_mapper_num_components(ptr addrspace(4)) #1

// end INTEL_COLLAB
