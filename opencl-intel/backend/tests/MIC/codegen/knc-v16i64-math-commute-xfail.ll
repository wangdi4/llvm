; XFAIL: *
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@g = common global <16 x i64> zeroinitializer, align 128
; Suboptimal scalarization
define <16 x i64> @div1(<16 x i64> %a) nounwind readonly ssp {
entry:
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
  %tmp1 = load <16 x i64>* @g, align 128
  %div = sdiv <16 x i64> %a, %tmp1
  ret <16 x i64> %div
}

define <16 x i64> @div2(<16 x i64> %a) nounwind readonly ssp {
entry:
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
  %tmp = load <16 x i64>* @g, align 128
  %div = sdiv <16 x i64> %tmp, %a
  ret <16 x i64> %div
}
