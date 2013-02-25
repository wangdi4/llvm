; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;
;

target datalayout = "e-p:64:64"

@g = common global <16 x i32> zeroinitializer, align 64
@pg = common global <16 x i32>* null, align 8

define <16 x i32> @negate1(<16 x i32> %a) nounwind readnone ssp {
entry:
; KNC: vpxord {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, [[R1:%zmm[0-9]+]]
; KNC: vpsubd {{%zmm[0-9]+}}, [[R1]], {{%zmm[0-9]+}}
; KNC: ret
  %sub = sub nsw <16 x i32> zeroinitializer, %a
  ret <16 x i32> %sub
}

define <16 x i32> @negate2() nounwind readonly ssp {
entry:
; KNC: vpxord {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, [[R2:%zmm[0-9]+]]
; KNC: vpsubd g(%rip), [[R2]], {{%zmm[0-9]+}}
; KNC: ret
  %tmp = load <16 x i32>* @g, align 64
  %sub = sub nsw <16 x i32> zeroinitializer, %tmp
  ret <16 x i32> %sub
}

define <16 x i32> @negate3() nounwind readonly ssp {
entry:
; KNC: vpxord {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, [[R2:%zmm[0-9]+]]
; KNC: vpsubd ({{%[a-z]+}}), [[R2]], {{%zmm[0-9]+}}
; KNC: ret
  %tmp = load <16 x i32>** @pg, align 8
  %tmp1 = load <16 x i32>* %tmp, align 64
  %sub = sub nsw <16 x i32> zeroinitializer, %tmp1
  ret <16 x i32> %sub
}
