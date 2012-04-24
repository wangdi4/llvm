; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define void @test(<16 x i16>* %a, <16 x i16>* %b) nounwind {
; KNF: vloadd    (%{{[a-z]+}}){uint16i}, [[R0:%v[0-9]+]]
  %1 = load <16 x i16>* %a
; KNF: vstored    [[R0]]{uint16i}, (%{{[a-z]+}})
  store <16 x i16> %1, <16 x i16>* %b, align 32
  ret void
}

