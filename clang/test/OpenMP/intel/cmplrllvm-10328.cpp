//RUN: %clang_cc1 -triple \
//RUN: x86_64-pc-windows-msvc -emit-llvm-bc \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -target-cpu \
//RUN: x86-64 -fintel-openmp-region \
//RUN: -o %t_host.bc %s

//RUN: %clang_cc1 -triple x86_64-pc-windows-msvc \
//RUN: -emit-llvm -disable-llvm-passes \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -target-cpu \
//RUN: x86-64 -fintel-openmp-region \
//RUN: -o %t_host.bc -o - %s \
//RUN: | FileCheck %s --check-prefix=Check-HOST

//RUN: %clang_cc1 -triple spir64 -aux-triple \
//RUN: x86_64-pc-windows-msvc -emit-llvm-bc \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device \
//RUN: -fopenmp-host-ir-file-path %t_host.bc \
//RUN: -fintel-openmp-region \
//RUN: -o %t_targ.bc %s

//RUN: %clang_cc1 -triple spir64 -aux-triple \
//RUN: x86_64-pc-windows-msvc \
//RUN: -emit-llvm -disable-llvm-passes \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fintel-ms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device \
//RUN: -fopenmp-host-ir-file-path %t_host.bc \
//RUN: -fintel-openmp-region \
//RUN: -o %t_targ.bc -o - %s \
//RUN: | FileCheck %s --check-prefix=Check-TARG

//expected-no-diagnostics

int foo()
{
#pragma omp target
#pragma omp parallel for
    for (int i=0; i<8; i+=4){
    }
    return 0;
}

//Check-HOST: define dso_local i32 @"?foo@@YAHXZ"() #0 {

//Check-TARG: define dso_local spir_func i32 @"?foo@@YAHXZ"() #0 {


