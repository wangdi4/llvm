// RUN: %clang_cc1 -ffreestanding -triple i686-unknown-linux-gnu -emit-llvm %s -opaque-pointers -o - | FileCheck %s --check-prefix=X86
// RUN: %clang_cc1 -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm %s -opaque-pointers -o - | FileCheck %s --check-prefix=X64

#include <immintrin.h>

void test__cpuid(int *info, int level) {
  __cpuid(info, level);
}
// X86-LABEL: define {{.*}} @test__cpuid(ptr noundef %{{.*}}, i32 noundef %{{.*}})
// X86: call { i32, i32, i32, i32 } asm "cpuid",
// X86-SAME:   "={ax},={bx},={cx},={dx},{ax},{cx}"
// X86-SAME:   (i32 %{{.*}}, i32 0)
//
// X64-LABEL:define {{.*}} @test__cpuid(ptr noundef %{{.*}}, i32 noundef %{{.*}})
// X64: call { i32, i32, i32, i32 } asm "xchgq %rbx, ${1:q}\0Acpuid\0Axchgq %rbx, ${1:q}",
// X64-SAME: "={ax},=r,={cx},={dx},0,2"
// X64-SAME: (i32 %{{.*}}, i32 0)
