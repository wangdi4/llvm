// RUN: %clang_cc1 -triple x86_64-linux-pc -fintel-compatibility -emit-llvm -o - %s -verify -DTEST1
// RUN: %clang_cc1 -triple x86_64-linux-pc -fintel-compatibility -emit-llvm -o - %s -verify -DTEST2
// RUN: %clang_cc1 -triple x86_64-linux-pc -fintel-compatibility -emit-llvm -o - %s -verify -DTEST3

inline void __attribute__((always_inline, target("sgx")))
IntrinSGX() {}
inline void __attribute__((always_inline, target("enqcmd")))
IntrinENQCMD() {}

#ifdef TEST1
void UsesSGX() {
  // expected-error@+1{{always_inline function 'IntrinSGX' requires target feature 'sgx', but would be inlined into function 'UsesSGX' that is compiled without support for 'sgx'}}
  IntrinSGX();
}
#endif // TEST1

#ifdef TEST2
void UsesENQCMD() {
  // expected-error@+1{{always_inline function 'IntrinENQCMD' requires target feature 'enqcmd', but would be inlined into function 'UsesENQCMD' that is compiled without support for 'enqcmd'}}
  IntrinENQCMD();
}
#endif // TEST2

#define SGX (1ULL << 53)
#define ENQCMD (1ULL << (69 - 64))

#ifdef TEST3
// expected-no-diagnostics
void __attribute__((allow_cpu_features(SGX)))
UsesSGXGood() {
  IntrinSGX();
}

void __attribute__((allow_cpu_features(0, ENQCMD)))
UsesENQCMDGood() {
  IntrinENQCMD();
}

void __attribute__((allow_cpu_features(SGX, ENQCMD)))
UsesBoth() {
  IntrinENQCMD();
}
#endif // TEST3
