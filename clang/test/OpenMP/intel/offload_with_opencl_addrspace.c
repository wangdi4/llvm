//RUN: %clang_cc1 -triple \
//RUN: x86_64-pc-windows-msvc -emit-llvm-bc \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -target-cpu \
//RUN: x86-64 -fopenmp-late-outline \
//RUN: -o %t_host.bc %s

//RUN: %clang_cc1 -triple x86_64-pc-windows-msvc \
//RUN: -emit-llvm -disable-llvm-passes \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -target-cpu \
//RUN: x86-64 -fopenmp-late-outline \
//RUN: -o - %s \
//RUN: | FileCheck %s --check-prefix=CHECK-HOST

//RUN: %clang_cc1 -triple spir64 -aux-triple \
//RUN: x86_64-pc-windows-msvc -emit-llvm-bc \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device \
//RUN: -fopenmp-host-ir-file-path %t_host.bc \
//RUN: -fopenmp-late-outline \
//RUN: -o %t_targ.bc %s

//RUN: %clang_cc1 -triple spir64 -aux-triple \
//RUN: x86_64-pc-windows-msvc \
//RUN: -emit-llvm -disable-llvm-passes \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device \
//RUN: -fopenmp-host-ir-file-path %t_host.bc \
//RUN: -fopenmp-late-outline \
//RUN: -o - %s \
//RUN: | FileCheck %s --check-prefix=CHECK-TARG

//expected-no-diagnostics

// The attributes must not affect the host code generation:
// CHECK-HOST-NOT: addrspace({{[0-9]+}})

// Verify that the attributes are honored for the target compilation:
// CHECK-TARG-DAG: @foo.strAS2 = internal addrspace(2) constant [7 x i8] c"qwerty\00"

#pragma omp declare target
char foo(int i) {
  static const __attribute__((opencl_constant)) char strAS2[] =
    "qwerty";
  return strAS2[i];
}
#pragma omp end declare target
