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

define double @div(double %a, double %b) nounwind readnone ssp {
entry:
; KNF: vrcpresps [[R1:%v[0-9]+]],
; KNF: vrcprefineps [[R1]],
; KNF: vcvtps2pd
; KNF: vmsubr23c1pd [[R1]]
; KNF: vmadd231pd
; KNF: vmsubr231pd
; KNF: vmadd231pd


; KNFmpa: vrcpresps [[R1:%v[0-9]+]],
; KNFmpa: vrcprefineps [[R1]],
; KNFmpa: vcvtps2pd
; KNFmpa: vmsubr23c1pd [[R1]]
; KNFmpa: vmadd231pd
; KNFmpa: vmsubr231pd
; KNFmpa: vmadd231pd

  %div = fdiv double %a, %b                        ; <double> [#uses=1]
  ret double %div
}

     
