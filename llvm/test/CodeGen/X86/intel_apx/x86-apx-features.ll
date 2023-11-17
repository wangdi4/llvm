; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512vl,+egpr -o %t1
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512vl -x86-apx-features=egpr -o %t2
; RUN:  cmp %t1 %t2

; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512vl,+egpr,+push2pop2 -o %t3
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512vl -x86-apx-features=egpr+push2pop2 -o %t4
; RUN:  cmp %t3 %t4

define void @csr2() nounwind {
entry:
  tail call void asm sideeffect "", "~{rbp},~{r15},~{dirflag},~{fpsr},~{flags}"()
  ret void
}

define void @test_x86_vcvtps2ph_256_m(ptr nocapture %d, <8 x float> %a) nounwind {
entry:
  %0 = load i32, ptr %d, align 4
  call void asm sideeffect "", "~{rax},~{rbx},~{rcx},~{rdx},~{rdi},~{rsi},~{rbp},~{rsp},~{r8},~{r9},~{r10},~{r11},~{r12},~{r13},~{r14},~{r15},~{dirflag},~{fpsr},~{flags}"()
  %1 = tail call <8 x i16> @llvm.x86.vcvtps2ph.256(<8 x float> %a, i32 3)
  store <8 x i16> %1, ptr %d, align 16
  ret void
}

declare <8 x i16> @llvm.x86.vcvtps2ph.256(<8 x float>, i32) nounwind readonly
