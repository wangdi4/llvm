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
//RUN: -o %t_host.bc -o - %s \
//RUN: | FileCheck %s --check-prefix=Check-HOST

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
//RUN: -o %t_targ.bc -o - %s \
//RUN: | FileCheck %s --check-prefix=Check-TARG

//expected-no-diagnostics

int main()
{
#pragma omp target
#pragma omp parallel for
    for (int i=0; i<8; i+=4){
    }
    return 0;
}

//Check-HOST: define dso_local noundef i32 @main() #0 {

//Check-TARG: define hidden noundef i32 @main() #0 {


