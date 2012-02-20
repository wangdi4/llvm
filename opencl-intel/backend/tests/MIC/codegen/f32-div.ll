; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; REVIEW: Both tests should generate FMA. Using FAM here is required for 
; correctness
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf -disable-excess-fp-precision \
; RUN:     | FileCheck %s -check-prefix=KNFmpa
;

target datalayout = "e-p:64:64"

define float @div(float %a, float %b) nounwind readnone ssp {
entry:
; KNF: vrcpresps [[R1:%v[0-9]+]]
; KNF: vrcprefineps [[R1]]
; KNF: vmulps 
; KNF: vmadd132ps 
; KNF: vfixupps $132737, 


; KNFmpa: vrcpresps [[R1:%v[0-9]+]]
; KNFmpa: vrcprefineps [[R1]]
; KNFmpa: vmulps 
; KNFmpa: vmadd132ps 
; KNFmpa: vfixupps $132737, 


  %div = fdiv float %a, %b                        ; <float> [#uses=1]
  ret float %div
}

