; RUN: llc < %s -mcpu=corei7-avx -march=x86 -mtriple=i386-linux-gnu > %t
; RUN: grep {movl  %gs:0, %eax} %t | count 1
; RUN: grep {leal  i@NTPOFF(%eax), %ecx} %t
; RUN: grep {leal  j@NTPOFF(%eax), %eax} %t
; RUN: llc < %s -mcpu=corei7-avx -march=x86-64 -mtriple=x86_64-linux-gnu > %t2
; RUN: grep {movq  %fs:0, %rax} %t2 | count 1
; RUN: grep {leaq  i@TPOFF(%rax), %rcx} %t2
; RUN: grep {leaq  j@TPOFF(%rax), %rax} %t2

@i = thread_local global i32 0
@j = thread_local global i32 0

define void @f(ptr %a, ptr %b) {
entry:
  store ptr @i, ptr %a, align 8
  store ptr @j, ptr %b, align 8
  ret void
}
