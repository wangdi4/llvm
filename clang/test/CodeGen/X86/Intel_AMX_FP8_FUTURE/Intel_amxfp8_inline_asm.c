// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown  -target-feature +amx-fp8 -emit-llvm -o - -Wall -Werror -pedantic | FileCheck %s
// REQUIRES: intel_feature_isa_amx_fp8_future
void f_tilemul(short a)
{
  //CHECK:  call void asm sideeffect "tileloadd 0(%rsi,%r13,4), %tmm0   \0A\09tileloadd 0(%rdx,%r14,4), %tmm6   \0A\09ttdpbf8ps %tmm6, %tmm0, %tmm7    \0A\09tilestored %tmm7, 0(%r12,%r15,4) \0A\09", "~{memory},~{tmm0},~{tmm6},~{tmm7},~{dirflag},~{fpsr},~{flags}"()
  __asm__ volatile ("tileloadd 0(%%rsi,%%r13,4), %%tmm0   \n\t"
                    "tileloadd 0(%%rdx,%%r14,4), %%tmm6   \n\t"
                    "ttdpbf8ps %%tmm6, %%tmm0, %%tmm7    \n\t"
                    "tilestored %%tmm7, 0(%%r12,%%r15,4) \n\t"
          ::: "memory", "tmm0", "tmm6", "tmm7");

  //CHECK:  call void asm sideeffect "tileloadd 0(%rsi,%r13,4), %tmm0   \0A\09tileloadd 0(%rdx,%r14,4), %tmm6   \0A\09ttdpbhf8ps %tmm6, %tmm0, %tmm7    \0A\09tilestored %tmm7, 0(%r12,%r15,4) \0A\09", "~{memory},~{tmm0},~{tmm6},~{tmm7},~{dirflag},~{fpsr},~{flags}"()
  __asm__ volatile ("tileloadd 0(%%rsi,%%r13,4), %%tmm0   \n\t"
                    "tileloadd 0(%%rdx,%%r14,4), %%tmm6   \n\t"
                    "ttdpbhf8ps %%tmm6, %%tmm0, %%tmm7    \n\t"
                    "tilestored %%tmm7, 0(%%r12,%%r15,4) \n\t"
          ::: "memory", "tmm0", "tmm6", "tmm7");

  //CHECK:  call void asm sideeffect "tileloadd 0(%rsi,%r13,4), %tmm0   \0A\09tileloadd 0(%rdx,%r14,4), %tmm6   \0A\09ttdphbf8ps %tmm6, %tmm0, %tmm7    \0A\09tilestored %tmm7, 0(%r12,%r15,4) \0A\09", "~{memory},~{tmm0},~{tmm6},~{tmm7},~{dirflag},~{fpsr},~{flags}"()
  __asm__ volatile ("tileloadd 0(%%rsi,%%r13,4), %%tmm0   \n\t"
                    "tileloadd 0(%%rdx,%%r14,4), %%tmm6   \n\t"
                    "ttdphbf8ps %%tmm6, %%tmm0, %%tmm7    \n\t"
                    "tilestored %%tmm7, 0(%%r12,%%r15,4) \n\t"
          ::: "memory", "tmm0", "tmm6", "tmm7");

  //CHECK:  call void asm sideeffect "tileloadd 0(%rsi,%r13,4), %tmm0   \0A\09tileloadd 0(%rdx,%r14,4), %tmm6   \0A\09ttdphf8ps %tmm6, %tmm0, %tmm7    \0A\09tilestored %tmm7, 0(%r12,%r15,4) \0A\09", "~{memory},~{tmm0},~{tmm6},~{tmm7},~{dirflag},~{fpsr},~{flags}"()
  __asm__ volatile ("tileloadd 0(%%rsi,%%r13,4), %%tmm0   \n\t"
                    "tileloadd 0(%%rdx,%%r14,4), %%tmm6   \n\t"
                    "ttdphf8ps %%tmm6, %%tmm0, %%tmm7    \n\t"
                    "tilestored %%tmm7, 0(%%r12,%%r15,4) \n\t"
          ::: "memory", "tmm0", "tmm6", "tmm7");
}
