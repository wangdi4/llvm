// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-linux-unknown -emit-llvm %s -o - | FileCheck %s

extern __int64 __cdecl _rdtsc(void);

void foo() {
  auto a = _rdtsc();
  // CHECK:call i64 @llvm.x86.rdtsc()

}
