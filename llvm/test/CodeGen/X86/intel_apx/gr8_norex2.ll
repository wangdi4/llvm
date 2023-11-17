; Check ah is not allocatable for register class gr8_norex2
; RUN: not llc < %s -mtriple=x86_64-unknown-unknown 2>&1 | FileCheck %s

define dso_local void @gr8_norex2() {
entry:
; CHECK: error: inline assembly requires more registers than available
; CHECK-NOT: error: can't encode 'ah' in an instruction requiring REX prefix
  %0 = tail call i8 asm sideeffect "movb %r14b, $0", "=r,~{al},~{rbx},~{rcx},~{rdx},~{rdi},~{rsi},~{rbp},~{rsp},~{r8},~{r9},~{r10},~{r11},~{r12},~{r13},~{r14},~{r15},~{dirflag},~{fpsr},~{flags}"()
  ret void
}

