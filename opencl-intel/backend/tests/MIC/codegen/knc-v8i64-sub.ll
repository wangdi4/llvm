; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

define <8 x i64> @sub1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; CHECK: sub1
; CHECK: vpsubsetbd %zmm1, %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    %zmm1, %k{{[1-7]+}}, [[Z0]]{%k
; CHECK: ret
  %sub = sub nsw <8 x i64> %a, %b
  ret <8 x i64> %sub
}

define <8 x i64> @sub2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; CHECK: sub2
; CHECK: vpsubsetbd %zmm0, %k{{[1-7]+}}, [[Z3:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    %zmm{{[0-9]+}}, %k{{[1-7]+}}, [[Z3]]{%k
; CHECK: ret
  %tmp1 = load <8 x i64>* %a, align 64
  %sub = sub nsw <8 x i64> %tmp1, %b
  ret <8 x i64> %sub
}

define <8 x i64> @sub3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: sub3
; CHECK: vpsubsetbd (%rdi), %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    (%rdi), %k{{[1-7]+}}, [[Z0]]{%k
; CHECK: ret
  %tmp2 = load <8 x i64>* %b, align 64
  %sub = sub nsw <8 x i64> %a, %tmp2
  ret <8 x i64> %sub
}

define <8 x i64> @sub4(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: sub4
; CHECK: vpsubsetbd gb(%rip), %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    gb(%rip), %k{{[1-7]+}}, [[Z0]]{%k
; CHECK: ret
  %tmp1 = load <8 x i64>* @gb, align 64
  %sub = sub nsw <8 x i64> %a, %tmp1
  ret <8 x i64> %sub
}

define <8 x i64> @sub5(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: sub5
; CHECK: vpsubsetbd ([[R0:%r[a-z]+]]), %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    ([[R0]]), %k{{[1-7]+}}, [[Z0]]{%k
; CHECK: ret
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %sub = sub nsw <8 x i64> %a, %tmp2
  ret <8 x i64> %sub
}
