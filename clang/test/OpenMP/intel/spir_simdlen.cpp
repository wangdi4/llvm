//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Wsimd-unsupported

//RUN: %clang_cc1 -triple i386-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Wsimd-unsupported

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Wsimd-unsupported -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsimd-unsupported -o - %s

//RUN: %clang_cc1 -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Wsimd-unsupported -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify  -o - %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wno-simd-unsupported -DNOWARNING -o - %s

//RUN: %clang_cc1 -triple spir \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wno-simd-unsupported -DNOWARNING -o - %s

// Verify -fopenmp-target-simd also eliminates simdlen warning.
//RUN: %clang_cc1 -triple spir64 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -fopenmp-target-simd -DNOWARNING -o - %s

//RUN: %clang_cc1 -triple spir \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -fopenmp-target-simd -DNOWARNING -o - %s
#ifdef NOWARNING
// expected-no-diagnostics
#endif

void foo()
{
#ifndef NOWARNING
  //expected-warning@+2 {{simdlen clause is not fully supported for this target}}
#endif
  #pragma omp simd simdlen(8)
  for (int i = 0; i < 10000; ++i);
}
