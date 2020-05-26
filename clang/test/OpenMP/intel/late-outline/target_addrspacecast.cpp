// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
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
//CHECK:[[SASC:%s.ascast]] = addrspacecast %struct.foo* %s to %struct.foo addrspace(4)*
//CHECK: [[L:%[0-9]+]] = bitcast %struct.foo addrspace(4)* %s.ascast to i8 addrspace(4)*
//CHECK: [[L1:%[0-9]+]] = getelementptr i8, i8 addrspace(4)* [[L]], i64 55
//CHECK: [[L27:%[0-9]+]] = getelementptr i8, i8 addrspace(4)* [[L1]], i32 1
  #pragma omp target map(to:s, s.ptr1 [0:1], s.ptrBase1 [0:1])
  {
   s.val++; s.ptr1[0]++; s.ptrBase1[0] = 10001;
  }
}
// end INTEL_COLLAB
