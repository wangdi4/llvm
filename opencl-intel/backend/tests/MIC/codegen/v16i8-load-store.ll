; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define void @test(<16 x i8>* %a, <16 x i8>* %b) nounwind {
; KNC: vpandd (%{{[a-z]*}}){uint8}, %zmm{{[0-9]*}}, %zmm{{[0-9]*}}
  %1 = load <16 x i8>* %a
; KNC: vmovdqa32  %zmm{{[0-9]*}}{uint8}, (%{{[a-z]+}})
  store <16 x i8> %1, <16 x i8>* %b, align 16
  ret void
}

