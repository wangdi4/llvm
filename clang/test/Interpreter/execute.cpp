// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows

// RUN: clang-repl "int i = 10;" 'extern "C" int printf(const char*,...);' \
// RUN:            'auto r1 = printf("i = %d\n", i);' | FileCheck --check-prefix=CHECK-DRIVER %s
// REQUIRES: host-supports-jit
// UNSUPPORTED: powerpc64

// CHECK-DRIVER: i = 10

// RUN: cat %s | clang-repl | FileCheck %s

extern "C" int printf(const char *, ...);
int i = 42;
auto r1 = printf("i = %d\n", i);
// CHECK: i = 42

struct S { float f = 1.0; S *m = nullptr;} s;

auto r2 = printf("S[f=%f, m=0x%llx]\n", s.f, reinterpret_cast<unsigned long long>(s.m));
// CHECK-NEXT: S[f=1.000000, m=0x0]

quit
