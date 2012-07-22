; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;

target datalayout = "e-p:64:64"


@x = global i32 2, align 4
@y = global i32 3, align 4
@z = global [2 x i32*] [i32* @x, i32* @y], align 16

define i32 @func() nounwind {
; CHECK: func:
; CHECK: movq      z(%rip), %rdx
; CHECK: movl      (%rdx), %eax
  %p = getelementptr [2 x i32*]* @z, i32 0, i64 0
  %1 = load i32** %p
  %2 = load i32* %1
  ret i32 %2
}
; CHECK: z:
; CHECK: .quad x
; CHECK: .quad y

