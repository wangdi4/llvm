; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8


define void @sub1(<16 x i64> %a, <16 x i64> %b, <16 x i64>* %s) nounwind readnone ssp {
entry:
; CHECK: sub1
; CHECK: vpsubsetbd %zmm2, %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsubsetbd %zmm3, %k{{[1-7]+}}, [[Z1:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    %zmm2, %k{{[1-7]+}}, [[Z0]]{%k
; CHECK: vpsbbd    %zmm3, %k{{[1-7]+}}, [[Z1]]{%k
; CHECK: ret
  %sub = sub nsw <16 x i64> %a, %b
  store <16 x i64> %sub, <16 x i64>* %s
  ret void
}

define void @sub2(<16 x i64>* nocapture %a, <16 x i64> %b, <16 x i64>* %s) nounwind ssp {
entry:
; CHECK: sub2
; CHECK: vpsubsetbd %zmm0, %k{{[1-7]+}}, [[Z3:%zmm[0-9]+]]{%k
; CHECK: vpsubsetbd %zmm1, %k{{[1-7]+}}, [[Z2:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    %zmm{{[0-9]+}}, %k{{[1-7]+}}, [[Z3]]{%k
; CHECK: vpsbbd    %zmm{{[0-9]+}}, %k{{[1-7]+}}, [[Z2]]{%k
; CHECK: ret
  %tmp1 = load <16 x i64>* %a, align 128
  %sub = sub nsw <16 x i64> %tmp1, %b
  store <16 x i64> %sub, <16 x i64>* %s
  ret void
}

define void @sub3(<16 x i64> %a, <16 x i64>* nocapture %b, <16 x i64>* %s) nounwind ssp {
entry:
; CHECK: sub3
; CHECK: vpsubsetbd (%rdi), %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsubsetbd 64(%rdi), %k{{[1-7]+}}, [[Z1:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    (%rdi), %k{{[1-7]+}}, [[Z0]]{%k
; CHECK: vpsbbd    64(%rdi), %k{{[1-7]+}}, [[Z1]]{%k
; CHECK: ret
  %tmp2 = load <16 x i64>* %b, align 128
  %sub = sub nsw <16 x i64> %a, %tmp2
  store <16 x i64> %sub, <16 x i64>* %s
  ret void
}

define void @sub4(<16 x i64> %a, <16 x i64>* %s) nounwind ssp {
entry:
; CHECK: sub4
; CHECK: vpsubsetbd 64+gb(%rip), %k{{[1-7]+}}, [[Z1:%zmm[0-9]+]]{%k
; CHECK: vpsubsetbd gb(%rip), %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    64+gb(%rip), %k{{[1-7]+}}, [[Z1]]{%k
; CHECK: vpsbbd    gb(%rip), %k{{[1-7]+}}, [[Z0]]{%k
; CHECK: ret
  %tmp1 = load <16 x i64>* @gb, align 128
  %sub = sub nsw <16 x i64> %a, %tmp1
  store <16 x i64> %sub, <16 x i64>* %s
  ret void
}

define void @sub5(<16 x i64> %a, <16 x i64>* %s) nounwind ssp {
entry:
; CHECK: sub5
; CHECK: vpsubsetbd ([[R0:%r[a-z]+]]), %k{{[1-7]+}}, [[Z0:%zmm[0-9]+]]{%k
; CHECK: vpsubsetbd 64([[R0]]), %k{{[1-7]+}}, [[Z1:%zmm[0-9]+]]{%k
; CHECK: vpsbbd    ([[R0]]), %k2, [[Z0]]{%k5}
; CHECK: vpsbbd    64([[R0]]), %k6, [[Z1]]{%k5}
; CHECK: ret
  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %sub = sub nsw <16 x i64> %a, %tmp2
  store <16 x i64> %sub, <16 x i64>* %s
  ret void
}
