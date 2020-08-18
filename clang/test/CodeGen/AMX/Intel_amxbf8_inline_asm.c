// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +amx-bf8 -emit-llvm -o - -Wall -Werror -pedantic | FileCheck %s --check-prefixes=CHECK,X86_64
// REQUIRES: intel_feature_isa_amx_bf8
void f_tilemul(short a)
{
  //CHECK:  call void asm sideeffect "tileloadd 0(%rsi,%r13,4), %tmm0   \0A\09tileloadd 0(%rdx,%r14,4), %tmm6   \0A\09tdpbf8ps %tmm6, %tmm0, %tmm7    \0A\09tilestored %tmm7, 0(%r12,%r15,4) \0A\09", "~{memory},~{tmm0},~{tmm6},~{tmm7},~{dirflag},~{fpsr},~{flags}"()
  __asm__ volatile ("tileloadd 0(%%rsi,%%r13,4), %%tmm0   \n\t"
                    "tileloadd 0(%%rdx,%%r14,4), %%tmm6   \n\t"
                    "tdpbf8ps %%tmm6, %%tmm0, %%tmm7    \n\t"
                    "tilestored %%tmm7, 0(%%r12,%%r15,4) \n\t"
          ::: "memory", "tmm0", "tmm6", "tmm7");
}
