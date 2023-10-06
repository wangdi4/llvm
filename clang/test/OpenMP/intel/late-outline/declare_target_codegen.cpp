// INTEL_COLLAB
// RUN: %clang_cc1 -fopenmp-version=60 -emit-llvm -o - -fopenmp  \
// RUN:  -fopenmp-late-outline -verify -triple x86_64-unknown-linux-gnu  \
// RUN:  -Wno-openmp-groupprivate %s | FileCheck %s -check-prefix HOST

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -Wno-openmp-groupprivate \
// RUN:  -emit-llvm-bc -disable-llvm-passes -fopenmp -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline -fopenmp-version=60 -Werror -Wsource-uses-openmp -o \
// RUN:  %t_host.bc %s

// RUN: %clang_cc1 -triple spir64 \
// RUN:  -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm -disable-llvm-passes -fopenmp-version=60 \
// RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline\
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
// RUN:  -Wsource-uses-openmp -Wno-openmp-groupprivate -o - %s \
// RUN:  | FileCheck %s -check-prefix TARG

// expected-no-diagnostics

#pragma omp begin declare target
char *glob_str =  (char*)"TESTING";
#pragma omp end declare target

int c1;
#pragma omp declare target local(c1)

#pragma omp begin declare target
int y;
#pragma omp declare target local(y)
int test2() {
  y = y + 1;
  return y;
}
struct name_val {
    char *name;
    long long int value;  /* Mostly holds small integers, sometimes pointers */
};
static struct name_val name_val_table[] = {
   (char*)"SIGHUP",        1,
   (char*)"SIGQUIT",       2,
   (char*)"SIGTRAP",       3
};
#pragma omp end declare target

int main() {
#pragma omp target
  {
   c1 = 10;
  }
#pragma omp target
  test2();
}
//HOST: @c1 = global i32 0, align 4
//HOST: @y = global i32 0, align 4
//HOST: @glob_str = target_declare global ptr @.str, align 8
//HOST: @.str = private target_declare unnamed_addr constant [8 x i8] c"TESTING\00", align 1
//HOST: @.str.1 = private target_declare unnamed_addr constant [7 x i8] c"SIGHUP\00", align 1
//HOST: @.str.2 = private target_declare unnamed_addr constant [8 x i8] c"SIGQUIT\00", align 1
//HOST: @.str.3 = private target_declare unnamed_addr constant [8 x i8] c"SIGTRAP\00", align 1
//TARG: @c1 = {{.*}}target_declare addrspace(1) global i32 0, align 4
//TARG: @y = {{.*}}target_declare addrspace(1) global i32 0, align 4
//TARG: @glob_str = {{.*}}target_declare addrspace(1) global ptr addrspace(4) addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), align 8
//TARG: @.str = private target_declare unnamed_addr addrspace(1) constant [8 x i8] c"TESTING\00", align 1
//TARG: @.str.1 = private target_declare unnamed_addr addrspace(1) constant [7 x i8] c"SIGHUP\00", align 1
//TARG: @.str.2 = private target_declare unnamed_addr addrspace(1) constant [8 x i8] c"SIGQUIT\00", align 1
//TARG: @.str.3 = private target_declare unnamed_addr addrspace(1) constant [8 x i8] c"SIGTRAP\00", align 1
//TARG-NOT: {i32 1, !"_Z1y", i32 0, i32 0, ptr addrspace(3) @y}

// end INTEL_COLLAB
