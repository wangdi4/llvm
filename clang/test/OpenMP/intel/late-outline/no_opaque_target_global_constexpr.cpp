// INTEL_COLLAB

//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes -fopenmp-version=51 \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o - %s | FileCheck %s -check-prefix HOST
//
//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -fopenmp-version=51 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s
//
//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-version=51 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Wsource-uses-openmp -o - %s | FileCheck %s -check-prefix TARG

//HOST: @d = target_declare global { i32*, i32 } zeroinitializer, align 8
//TARG: @d = target_declare addrspace(1) global { i32 addrspace(4)*, i32 } zeroinitializer, align 8

struct ConstExprA {
  int *b;
  int c;
  constexpr ConstExprA() : b(), c() {}
};
#pragma omp declare target
ConstExprA d;
#pragma omp end declare target

//HOST: !0 = !{i32 1, !"_Z1d", i32 0, i32 0, { i32*, i32 }* @d}
//TARG: !0 = !{i32 1, !"_Z1d", i32 0, i32 0, { i32 addrspace(4)*, i32 } addrspace(1)* @d}
// end INTEL_COLLAB
