; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define float @f1(<16 x i32>* %arg0) {
; KNC: valignd   $11, (%rdi), [[R1:%zmm[0-9]+]], [[R2:%zmm[0-9]+]]
; KNC: valignd   $10, (%rdi), [[R1]], [[R3:%zmm[0-9]+]]
; KNC: vaddps    [[R2]], [[R3]], %zmm0{%k1}

entry:
  %v = load <16 x i32>* %arg0
  %v2 = bitcast <16 x i32> %v to <8 x double>
  %v1 = bitcast <8 x double> %v2 to <16 x float> 
  %res = extractelement <16 x float> %v1, i32 10
  %res1 = extractelement <16 x float> %v1, i32 11
  %res2 = fadd float %res, %res1
  ret float %res2
}

