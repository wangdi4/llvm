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

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8

define <16 x float> @div1(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; KNF: vrcpresps [[R1:%v[0-9]+]], [[R2:%v[0-9]+]]
; KNF: vrcprefineps [[R1]], [[R2]], [[R3:%v[0-9]+]]
; KNF: vmsubr23c1ps [[R1]], [[R3]], [[R4:%v[0-9]+]]
; KNF: vmadd132ps [[R4]], [[R3]], [[R3]]
; KNF: vmsubr23c1ps [[R1]], [[R3]], [[R5:%v[0-9]+]]
; KNF: vmadd132ps [[R5]], [[R3]], [[R3]]
; KNF: vfixupps $100418, [[R1]], [[R3]]
; KNF: vmulps  [[R3]], [[R6:%v[0-9]+]], [[R7:%v[0-9]+]]
; KNF: vmsubr231ps [[R7]], [[R1]], [[R6]]
; KNF: vmadd132ps [[R6]], [[R7]], [[R3]]
; KNF: vfixupps $132737, [[R7]], [[R3]]



; KNFmpa: vrcpresps [[R1:%v[0-9]+]], [[R2:%v[0-9]+]]
; KNFmpa: vrcprefineps [[R1]], [[R2]], [[R3:%v[0-9]+]]
; KNFmpa: vmsubr23c1ps [[R1]], [[R3]], [[R4:%v[0-9]+]]
; KNFmpa: vmadd132ps [[R4]], [[R3]], [[R3]]
; KNFmpa: vmsubr23c1ps [[R1]], [[R3]], [[R5:%v[0-9]+]]
; KNFmpa: vmadd132ps [[R5]], [[R3]], [[R3]]
; KNFmpa: vfixupps $100418, [[R1]], [[R3]]
; KNFmpa: vmulps  [[R3]], [[R6:%v[0-9]+]], [[R7:%v[0-9]+]]
; KNFmpa: vmsubr231ps [[R7]], [[R1]], [[R6]]
; KNFmpa: vmadd132ps [[R6]], [[R7]], [[R3]]
; KNFmpa: vfixupps $132737, [[R7]], [[R3]]

  %div = fdiv <16 x float> %a, %b
  ret <16 x float> %div
}

define <16 x float> @div2(<16 x float>* nocapture %a, <16 x float> %b) nounwind readonly ssp {
entry:
; KNF: vrcpresps [[R1:%v[0-9]+]], 
; KNF: vrcprefineps [[R1]], 
; KNF: vmulps 
; KNF: vmadd132ps 
; KNF: vfixupps $132737, 



; KNFmpa: vrcpresps [[R1:%v[0-9]+]], 
; KNFmpa: vrcprefineps [[R1]], 
; KNFmpa: vmulps 
; KNFmpa: vmadd132ps 
; KNFmpa: vfixupps $132737, 

  %tmp1 = load <16 x float>* %a, align 64
  %div = fdiv <16 x float> %tmp1, %b
  ret <16 x float> %div
}

define <16 x float> @div3(<16 x float> %a, <16 x float>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: vrcpresps 
; KNF: vrcprefineps 
; KNF: vmsubr23c1ps
; KNF: vmadd132ps 
; KNF: vfixupps $100418, 
; KNF: vmulps 
; KNF: vfixupps  $132737,


; KNFmpa: vrcpresps 
; KNFmpa: vrcprefineps 
; KNFmpa: vmsubr23c1ps
; KNFmpa: vmadd132ps 
; KNFmpa: vfixupps $100418, 
; KNFmpa: vmulps 
; KNFmpa: vfixupps  $132737,


  %tmp2 = load <16 x float>* %b, align 64
  %div = fdiv <16 x float> %a, %tmp2
  ret <16 x float> %div
}


define <16 x float> @div4(<16 x float> %a) nounwind readonly ssp {
entry: 
; KNF: vrcpresps  
; KNF: vrcprefineps 
; KNF: vmulps 
; KNF: vmadd132ps 
; KNF: vfixupps $132737,



; KNFmpa: vrcpresps  
; KNFmpa: vrcprefineps 
; KNFmpa: vmulps 
; KNFmpa: vmadd132ps 
; KNFmpa: vfixupps $132737,


  %tmp1 = load <16 x float>* @gb, align 64
  %div = fdiv <16 x float> %a, %tmp1
  ret <16 x float> %div
}

define <16 x float> @div5(<16 x float> %a) nounwind readonly ssp {
entry:

; KNF: vrcpresps 
; KNF: vrcprefineps 
; KNF: vmsubr23c1ps 
; KNF: vmadd132ps
; KNF: vfixupps 
; KNF: vmulps 
; KNF: vfixupps




; KNFmpa: vrcpresps 
; KNFmpa: vrcprefineps 
; KNFmpa: vmsubr23c1ps 
; KNFmpa: vmadd132ps
; KNFmpa: vfixupps 
; KNFmpa: vmulps 
; KNFmpa: vfixupps


  %tmp1 = load <16 x float>** @pgb, align 8
  %tmp2 = load <16 x float>* %tmp1, align 64
  %div = fdiv <16 x float> %a, %tmp2
  ret <16 x float> %div
}

