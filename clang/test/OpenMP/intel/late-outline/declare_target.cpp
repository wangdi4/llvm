// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm %s -o - | FileCheck -check-prefix=HOST %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck -check-prefix=TARG %s

// test device_type

void host_fun() {}
#pragma omp declare target to(host_fun) device_type(host)
void device_fun() {}
#pragma omp declare target to(device_fun) device_type(nohost)
void any_fun() {}
#pragma omp declare target to(any_fun) device_type(any)

// HOST-NOT: define {{.*}}void {{.*}}device_fun
// HOST: define {{.*}}void {{.*}}host_fun
// HOST-NOT: define {{.*}}void {{.*}}device_fun

// TARG-NOT: define {{.*}}void {{.*}}host_fun
// TARG: define {{.*}}void {{.*}}device_fun
// TARG-NOT: define {{.*}}void {{.*}}host_fun

// HOST: define {{.*}}void {{.*}}any_fun
// TARG: define {{.*}}void {{.*}}any_fun

// Test nested declare target

#pragma omp declare target
  int foo() { return 1; }
  #pragma omp declare target
    int bar() { return 2;}
  #pragma omp end declare target
    int baz() { return 3;}
#pragma omp end declare target
int other() { return 4; }

// TARG: define {{.*}}i32 {{.*}}foo
// TARG: define {{.*}}i32 {{.*}}bar
// TARG: define {{.*}}i32 {{.*}}baz
// TARG-NOT: define {{.*}}i32 {{.*}}other
