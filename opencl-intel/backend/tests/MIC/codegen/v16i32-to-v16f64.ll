; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;


target datalayout = "e-p:64:64"

define void @A(<16 x i32> %a, <16 x double>* %p) nounwind readnone ssp {
entry:
; KNF: vshuf128x32 $228, $14, %v0, [[R0:%v[0-9]+]]
; KNF: vcvtpi2pd $0, %v0, [[R1:%v[0-9]+]]
; KNF: vcvtpi2pd $0, [[R0]], [[R2:%v[0-9]+]]
;
; KNC: vshuf128x32 $228, $14, %zmm0, [[R0:%zmm[0-9]+]]
; KNC: vcvtpi2pd $0, %zmm0, [[R1:%zmm[0-9]+]]
; KNC: vcvtpi2pd $0, [[R0]], [[R2:%zmm[0-9]+]]
  %c = sitofp <16 x i32> %a to <16 x double>
  store <16 x double> %c, <16 x double>* %p
  ret void
}

@g = common global <16 x i32> zeroinitializer, align 64

define void @B(<16 x double>* %p) nounwind readnone ssp {
entry:
; KNF: vshuf128x32 $228, $14, g(%rip), [[R0:%v[0-9]+]]
; KNF: vcvtpi2pd $0, g(%rip), [[R1:%v[0-9]+]]
; KNF: vcvtpi2pd $0, [[R0]], [[R2:%v[0-9]+]]
;
; KNC: vshuf128x32 $228, $14, g(%rip), [[R0:%zmm[0-9]+]]
; KNC: vcvtpi2pd $0, g(%rip), [[R1:%zmm[0-9]+]]
; KNC: vcvtpi2pd $0, [[R0]], [[R2:%zmm[0-9]+]]
  %i = load <16 x i32>* @g
  %c = sitofp <16 x i32> %i to <16 x double>
  store <16 x double> %c, <16 x double>* %p
  ret void
}
