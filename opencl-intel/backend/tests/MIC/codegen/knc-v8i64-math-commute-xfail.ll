; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@g = common global <8 x i64> zeroinitializer, align 64

; Bad scalarization sequence with too many spills
define <8 x i64> @div1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: div1
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: cqto
; CHECK: idivq
; CHECK: movq %rax
; CHECK: ret
  %tmp1 = load <8 x i64>* @g, align 64
  %div = sdiv <8 x i64> %a, %tmp1
  ret <8 x i64> %div
}
