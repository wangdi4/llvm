// INTEL_FEATURE_CPU_LNL
// REQUIRES: intel_feature_cpu_lnl
// RUN: %clang_cc1 -verify -triple x86_64-linux-gnu -emit-llvm -o - %s
// RUN: %clang_cc1 -verify -triple x86_64-windows-pc -fms-compatibility -emit-llvm -o - %s

// expected-no-diagnostics

#ifdef _WIN64
#define ATTR(X) __declspec(X)
#else
#define ATTR(X) __attribute__((X))
#endif // _WIN64

ATTR(cpu_specific(generic)) void CPU(void){}
ATTR(cpu_specific(lunarlake)) void CPU(void){}

// end INTEL_FEATURE_CPU_LNL
