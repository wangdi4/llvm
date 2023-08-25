// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=x86_64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:    | FileCheck %s --check-prefixes ALL,HOST

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=x86_64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple x86_64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=x86_64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

//RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu -emit-pch %s -o %t

//RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu -include-pch %t -emit-llvm %s -o - \
//RUN:   | FileCheck %s --check-prefixes ALL,HOST

//expected-no-diagnostics

#ifndef HEADER
#define HEADER

extern  int printf(const char *,...);
#pragma omp begin declare variant match(device={kind(nohost)})
static void f1() { printf("device\n"); }
#pragma omp end declare variant

#pragma omp begin declare variant match(device={kind(host)})
static void f1() { printf("host\n"); }
#pragma omp end declare variant

int main() {
#pragma omp target
// ALL:  "DIR.OMP.TARGET"
// HOST: call void @"f1{{.*}}() #[[DECLARE_TARGET:[0-9]+]]
// TARG: call void @"f1{{.*}}"() #[[DECLARE_TARGET:[0-9]+]]
// ALL: "DIR.OMP.END.TARGET"
  f1();
}

// HOST: define internal void @"f1$ompvariant$S2$s6$Phost"() #[[DECLARE_TARGET1:[0-9]+]]
#endif // HEADER

//TARG: define{{.*}} void @"f1$ompvariant$S2$s6$Pnohost"() #[[DECLARE_TARGET1:[0-9]+]]

//HOST-NOT: attributes #[[DECLARE_TARGET1]] = {{.*}} "openmp-target-declare"="true"
//TARG: attributes #[[DECLARE_TARGET1]] = {{.*}} "openmp-target-declare"="true"

//HOST-NOT: attributes #[[DECLARE_TARGET]] = {{.*}} "openmp-target-declare"="true"
//TARG-NOT: attributes #[[DECLARE_TARGET]] = {{.*}} "openmp-target-declare"="true"
// end INTEL_COLLAB
