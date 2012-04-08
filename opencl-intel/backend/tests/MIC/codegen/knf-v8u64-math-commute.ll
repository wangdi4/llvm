; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s
;

target datalayout = "e-p:64:64"

@g = common global <8 x i64> zeroinitializer, align 64

define <8 x i64> @add1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
  %tmp1 = load <8 x i64>* @g, align 64
  %add = add <8 x i64> %tmp1, %a
  ret <8 x i64> %add
}

define <8 x i64> @add2(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
; CHECK: addq
  %tmp = load <8 x i64>* @g, align 64
  %add = add <8 x i64> %tmp, %a
  ret <8 x i64> %add
}

define <8 x i64> @mul1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
  %tmp1 = load <8 x i64>* @g, align 64
  %mul = mul <8 x i64> %tmp1, %a
  ret <8 x i64> %mul
}

define <8 x i64> @mul2(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
; CHECK: imulq
  %tmp = load <8 x i64>* @g, align 64
  %mul = mul <8 x i64> %tmp, %a
  ret <8 x i64> %mul
}

define <8 x i64> @sub1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
  %tmp1 = load <8 x i64>* @g, align 64
  %sub = sub <8 x i64> %a, %tmp1
  ret <8 x i64> %sub
}

define <8 x i64> @sub2(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
; CHECK: subq
  %tmp = load <8 x i64>* @g, align 64
  %sub = sub <8 x i64> %tmp, %a
  ret <8 x i64> %sub
}

define <8 x i64> @div1(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
  %tmp1 = load <8 x i64>* @g, align 64
  %div = udiv <8 x i64> %a, %tmp1
  ret <8 x i64> %div
}

define <8 x i64> @div2(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
; CHECK: movq {{[0-9]*}}(%rsp), %rax
; CHECK: xorl %edx, %edx
; CHECK: divq
; CHECK: movq %rax
  %tmp = load <8 x i64>* @g, align 64
  %div = udiv <8 x i64> %tmp, %a
  ret <8 x i64> %div
}
