// RUN: %clang_cc1 -ffreestanding -triple i686-unknown-linux-gnu -emit-llvm %s -o - | FileCheck %s --check-prefix=X86
// RUN: %clang_cc1 -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm %s -o - | FileCheck %s --check-prefix=X64

#include <immintrin.h>

void test__cpuid(int *info, int level) {
  __cpuid(info, level);
}
// X86-LABEL: define {{.*}} @test__cpuid(i32* %{{.*}}, i32 %{{.*}})
// X86: call { i32, i32, i32, i32 } asm "cpuid",
// X86-SAME:   "={ax},={bx},={cx},={dx},{ax},{cx},~{dirflag},~{fpsr},~{flags}"
// X86-SAME:   (i32 %{{.*}}, i32 0)
//
// X64-LABEL: define {{.*}} @test__cpuid(i32* %{{.*}}, i32 %{{.*}})
// X64: call { i32, i32, i32, i32 } asm " xchgq %rbx,${1:q}\0A cpuid\0A xchgq %rbx,${1:q}",
// X64-SAME:   "={ax},=r,={cx},={dx},{ax},{cx},~{dirflag},~{fpsr},~{flags}"
// X64-SAME:   (i32 %{{.*}}, i32 0)
