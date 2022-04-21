// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s --check-prefixes=TARG

typedef _Atomic(int) atomic_int;

//TARG-LABEL: main
int main()
{
  int acc = 1, compare = 0, value = 0;

  #pragma omp target map(tofrom: acc, compare, value)
  //TARG: "DIR.OMP.TARGET"()
  //TARG: [[A:%[0-9]+]] = cmpxchg ptr addrspace(4) {{[^,]*}}, i32 {{[^,]*}}, i32 {{[^,]*}} seq_cst seq_cst
  //TARG-NEXT: extractvalue { i32, i1 } [[A]], 0
  //TARG-NEXT: extractvalue { i32, i1 } [[A]], 1
  __c11_atomic_compare_exchange_strong(
          (atomic_int *) &acc, &compare, value, 5, 5);
  //TARG: "DIR.OMP.END.TARGET"()
 return 0;
}
