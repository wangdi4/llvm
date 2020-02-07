// INTEL_COLLAB
// RUN: %clang_cc1 -O0 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm %s -o - \
// RUN:  | FileCheck %s --check-prefix HOST
//
// RUN: %clang_cc1 -O0 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -O0 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s --check-prefix TARG
//
// expected-no-diagnostics

void bar_one() {}
void bar_two() {}
void bar_three() {}
void bar_four() {}

__attribute__((always_inline)) void foo_one()
{
    #pragma omp target
    bar_one();
}

__attribute__((always_inline)) void foo_two()
{
    #pragma omp target parallel for
    for(int i=0;i<10;i++) bar_two();
}

void foo_three()
{
    #pragma omp target
    bar_three();
}

void foo_four()
{
    #pragma omp target parallel for
    for(int i=0;i<10;i++) bar_four();
}


//HOST: define{{.*}}foo_one{{.*}} #[[ALWAYS:[0-9]+]]
//HOST: define{{.*}}foo_two{{.*}} #[[ALWAYS]]
//HOST: define{{.*}}foo_three{{.*}} #[[NOINL:[0-9]+]]
//HOST: define{{.*}}foo_four{{.*}} #[[NOINL]]
//HOST: attributes #[[ALWAYS]] = { {{.*}}alwaysinline
//HOST: attributes #[[NOINL]] = { {{.*}}noinline

//TARG: define{{.*}}foo_one{{.*}} #[[NOINL1:[0-9]+]]
//TARG: define{{.*}}foo_two{{.*}} #[[NOINL1]]
//TARG: define{{.*}}foo_three{{.*}} #[[NOINL2:[0-9]+]]
//TARG: define{{.*}}foo_four{{.*}} #[[NOINL2]]
//TARG: attributes #[[NOINL1]] = { {{.*}}noinline{{.*}}contains-openmp-target
//TARG: attributes #[[NOINL2]] = { {{.*}}noinline{{.*}}contains-openmp-target

// end INTEL_COLLAB
