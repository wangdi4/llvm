// INTEL_COLLAB
//RUN: %clang_cc1 -emit-pch -o %t -std=c++14 -fopenmp \
//RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s

//RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline \
//RUN:  -include-pch %t \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s -check-prefix HOST

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s -check-prefix TARG

#ifndef HEADER
#define HEADER

#pragma ompx declare target function
int foo() { return 0; }

int bar() { return 1; }

//HOST: define {{.*}}foo
//HOST: define {{.*}}bar

//TARG-NOT:define {{.*}}bar
//TARG: define {{.*}}foo
//TARG-NOT:define {{.*}}bar

#endif // HEADER
// end INTEL_COLLAB
