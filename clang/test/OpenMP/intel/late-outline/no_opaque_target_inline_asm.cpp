// INTEL_COLLAB

// RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc -o %t-host.bc %s
//
// RUN: %clang_cc1 -no-opaque-pointers -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc -emit-llvm -o - %s | \
// RUN:  FileCheck %s

struct cf { float real, imag; };

//CHECK-LABEL: foo
void foo(cf *pA, cf *pB){
  // Input and Outputs
  const int L = 262144;

  // Main program
  #pragma omp target map(from: pA[0:L]) map(to: pB[0:L])
  for (int i = 0 ; i < L ; i++)
  {
    #if !defined(SPIR64)
    pA[ i ] = pB [ i ];
    #else
    // CHECK: [[L1:%[0-9]+]] = bitcast %struct.cf addrspace(4)*
    // CHECK-SAME: to i64 addrspace(4)*
    // CHECK: [[L2:%[0-9]+]] = load i64, i64 addrspace(4)* [[L1]], align 4
    // CHECK: call void asm sideeffect {{.*}}svm_scatter.8.1 {{.*}}[[L2]]
    __asm__ volatile (
        "svm_scatter.8.1 (M1, 16) %[pAi].0 %[pBi].0"
        :
        : [pAi] "+rw" (&pA[i]), [pBi] "rw" (pB[i]));
    #endif
  }
}
// end INTEL_COLLAB
