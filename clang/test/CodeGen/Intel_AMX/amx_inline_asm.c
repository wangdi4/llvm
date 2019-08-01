// REQUIRES: intel_feature_isa_amx
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +amx-int8 -target-feature +amx-bf16 -emit-llvm -o - -Wall -Werror -pedantic | FileCheck %s --check-prefixes=CHECK,X86_64

void f_tmul(short a)
{
  //CHECK:  call void asm sideeffect "tileloadd 0(%rsi,%r13,4), %tmm0   \0A\09tileloadd 0(%rdx,%r14,4), %tmm8   \0A\09tdpbf16ps %tmm8, %tmm0, %tmm15    \0A\09tilestored %tmm15, 0(%r12,%r15,4) \0A\09", "~{memory},~{tmm0},~{tmm8},~{tmm15},~{dirflag},~{fpsr},~{flags}"()
  __asm__ volatile ("tileloadd 0(%%rsi,%%r13,4), %%tmm0   \n\t"
                    "tileloadd 0(%%rdx,%%r14,4), %%tmm8   \n\t"
                    "tdpbf16ps %%tmm8, %%tmm0, %%tmm15    \n\t"
                    "tilestored %%tmm15, 0(%%r12,%%r15,4) \n\t"
          ::: "memory", "tmm0", "tmm8", "tmm15");
}
