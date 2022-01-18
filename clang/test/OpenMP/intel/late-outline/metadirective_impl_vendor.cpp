// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu -emit-pch %s -o %t
//
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu \
// RUN: -include-pch %t -emit-llvm %s -o - | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp \
// RUN: -triple x86_64-unknown-linux-gnu %s | FileCheck %s -check-prefix LLVM
//
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=51 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s
//
// expected-no-diagnostics
//
#ifndef HEADER
#define HEADER

// Intention is that -fiopenmp : vendor(intel)
//                   -fopenmp  : vendor(llvm)
//
#pragma omp declare target
void func()
{
  //CHECK: "QUAL.OMP.NUM_THREADS"(i32 8)
  //LLVM: call {{.*}}__kmpc_push_num_threads({{.*}}, i32 4)
  #pragma omp metadirective                                           \
    when(implementation={vendor(arm)}: parallel for)                  \
    when(implementation={vendor(llvm)}: parallel for num_threads(4))  \
    when(implementation={vendor(intel)}: parallel for num_threads(8)) \
    default()
  for (int i=0;i<8; ++i) { }
}
#pragma omp end declare target
#endif
// end INTEL_COLLAB
