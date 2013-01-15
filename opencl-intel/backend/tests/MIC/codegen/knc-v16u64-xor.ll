; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define void @xor1(<16 x i64> %a, <16 x i64> %b, <16 x i64>* %s) nounwind readnone ssp {
entry:
; CHECK: pxorq    %zmm2, %zmm0, %zmm4
; CHECK: pxorq    %zmm3, %zmm1, %zmm5
  %xor = xor <16 x i64> %a, %b
  store <16 x i64> %xor, <16 x i64>* %s
  ret void
}

define void @xor2(<16 x i64>* nocapture %a, <16 x i64> %b, <16 x i64>* %s) nounwind readonly ssp {
entry:
; CHECK: vpxorq    (%rdi), %zmm0, %zmm3
; CHECK: vpxorq    64(%rdi), %zmm1, %zmm2
  %tmp1 = load <16 x i64>* %a, align 128
  %xor = xor <16 x i64> %tmp1, %b
  store <16 x i64> %xor, <16 x i64>* %s
  ret void
}

define void @xor3(<16 x i64> %a, <16 x i64>* nocapture %b, <16 x i64>* %s) nounwind readonly ssp {
entry:
; CHECK: vpxorq    (%rdi), %zmm0, %zmm3
; CHECK: vpxorq    64(%rdi), %zmm1, %zmm2
  %tmp2 = load <16 x i64>* %b, align 128
  %xor = xor <16 x i64> %tmp2, %a
  store <16 x i64> %xor, <16 x i64>* %s
  ret void
}

define void @xor4(<16 x i64> %a, <16 x i64>* %s) nounwind readonly ssp {
entry:
; CHECK: vpxorq    64+gb(%rip), %zmm1, %zmm2
; CHECK: vpxorq    gb(%rip), %zmm0, %zmm3
  %tmp1 = load <16 x i64>* @gb, align 128
  %xor = xor <16 x i64> %tmp1, %a
  store <16 x i64> %xor, <16 x i64>* %s
  ret void
}

define void @xor5(<16 x i64> %a, <16 x i64>* %s) nounwind readonly ssp {
entry:
; CHECK: vpxorq    ([[R0:%r[a-z]+]]), %zmm0, %zmm2
; CHECK: vpxorq    64([[R0]]), %zmm1, %zmm3
  %tmp1 = load <16 x i64>** @pgb, align 8
  %tmp2 = load <16 x i64>* %tmp1, align 128
  %xor = xor <16 x i64> %tmp2, %a
  store <16 x i64> %xor, <16 x i64>* %s
  ret void
}
