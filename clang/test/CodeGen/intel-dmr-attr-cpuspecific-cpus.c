// INTEL_FEATURE_CPU_DMR
// REQUIRES: intel_feature_cpu_dmr
// RUN: %clang_cc1 -verify -triple x86_64-linux-gnu -emit-llvm -o - %s
// RUN: %clang_cc1 -verify -triple x86_64-windows-pc -fms-compatibility -emit-llvm -o - %s

// expected-no-diagnostics

#ifdef _WIN64
#define ATTR(X) __declspec(X)
#else
#define ATTR(X) __attribute__((X))
#endif // _WIN64

ATTR(cpu_specific(generic)) void CPU(void){}
ATTR(cpu_specific(diamondrapids)) void CPU(void){}

// end INTEL_FEATURE_CPU_DMR
