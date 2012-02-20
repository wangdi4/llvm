
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define float @ld_extract_16f32(<16 x float> *%a) nounwind {
entry:
;KNF: vloadd 12(%{{[a-z]+}}){1to16}, [[R0:%v[0-9]+]]
  %vec = load <16 x float> *%a  ;
  %elmnt = extractelement <16 x float> %vec, i32 3   ; 
  ret float %elmnt
}

define double @ld_extract_8f64(<8 x double> *%a) nounwind {
entry:
;KNF: vloadq 24(%{{[a-z]+}}){1to8}, [[R0:%v[0-9]+]]
  %vec = load <8 x double> *%a  ;
  %elmnt = extractelement <8 x double> %vec, i32 3   ; 
  ret double %elmnt
}



define float @extract_16f32_15(<16 x float> %a) nounwind {
entry:
;KNF: vshuf128x32 $255, $255, %v0, %v0
  %elmnt = extractelement <16 x float> %a, i32 15
  ret float %elmnt
}

define float @extract_16f32_1(<16 x float> %a) nounwind {
entry:
;KNF: vshuf128x32 $85, $0, %v0, %v0
  %elmnt = extractelement <16 x float> %a, i32 1
  ret float %elmnt
}



define double @extract_16f64_15(<16 x double> %a) nounwind {
entry:
;KNF: vshuf128x32 $238, $255, %v0, [[R0:%v[0-9]+]]
  %e1 = extractelement <16 x double> %a, i32 15
;KNF: vshuf128x32 $238, $255, %v1, [[R1:%v[0-9]+]]
  %e2 = extractelement <16 x double> %a, i32 7
;KNF: vaddpd [[R0]], [[R1]]
  %add = fadd double %e1, %e2
  ret double %add
}

define double @extract_8f64_4(<8 x double> %a) nounwind {
entry:
;KNF: vshuf128x32 $68, $170, %v0, %v0
  %e1 = extractelement <8 x double> %a, i32 4
  ret double %e1
}


define void @extract_st_16f32(<16 x float> %a, float *%b) nounwind {
entry:
; KNF: vshuf128x32 $255, $0, %v0, [[R0:%v[0-9]+]]
  %e1 = extractelement <16 x float> %a, i32 3   ; 
; KNF: vstored [[R0]]{a}, (%{{[a-z]+}})
  store float %e1, float* %b
  ret void
}

define void @ld_extract_st_16f32(<16 x float> *%a, float *%b) nounwind {
entry:
; KNF: movl      12(%rdi), [[R0:%[a-z]+]]
  %vec = load <16 x float> *%a  ;
  %e1 = extractelement <16 x float> %vec, i32 3
; KNF: movl [[R0]], (%rsi)
  store float %e1, float* %b
  ret void
}


define <16 x float> @load_1extract(<16 x float> * %pin, float *%f2) nounwind {
entry:
;KNF:   movl  (%rdi), [[R0:%[a-z]+]]
;KNF:   movl  (%rsi), [[R1:%[a-z]+]]
;KNF:   movl  [[R0]], -64(%rsp)
;KNF:   movl  [[R1]], -60(%rsp)
;KNF:   vloadq  -64(%rsp), %v0
  %0 = load <16 x float> *%pin, align 64
  %1 = extractelement <16 x float> %0, i32 0
  %2 = load float *%f2
  %3 = insertelement <16 x float> undef, float %1, i32 0
  %4 = insertelement <16 x float> %3, float %2, i32 1
  %5 = insertelement <16 x float> %4, float %1, i32 2
  %6 = insertelement <16 x float> %5, float %2, i32 3
  %7 = insertelement <16 x float> %6, float %1, i32 4
  %8 = insertelement <16 x float> %7, float %2, i32 5
  %9 = insertelement <16 x float> %8, float %1, i32 6
  %10 = insertelement <16 x float> %9, float %2, i32 7
  %11 = insertelement <16 x float> %10, float %1, i32 8
  %12 = insertelement <16 x float> %11, float %2, i32 9
  %13 = insertelement <16 x float> %12, float %1, i32 10
  %14 = insertelement <16 x float> %13, float %2, i32 11
  %15 = insertelement <16 x float> %14, float %1, i32 12
  %16 = insertelement <16 x float> %15, float %2, i32 13
  %17 = insertelement <16 x float> %16, float %1, i32 14
  %18 = insertelement <16 x float> %17, float %2, i32 15
  ret <16 x float> %18
}


define <16 x float> @load_2extracts(<16 x float> * %pin) nounwind {
entry:
;KNF:    vloadd  (%rdi), [[R0:%v[0-9]+]]
;KNF:    vshuf128x32     $85, $0, (%rdi), [[R1:%v[0-9]+]]
;KNF:    vstored [[R0]]{a}, -64(%rsp)
;KNF:    vstored [[R1]]{a}, -60(%rsp)
;KNF:    vloadq  -64(%rsp), %v0
  %0 = load <16 x float> *%pin, align 64
  %1 = extractelement <16 x float> %0, i32 0
  %2 = extractelement <16 x float> %0, i32 1
  %3 = insertelement <16 x float> undef, float %1, i32 0
  %4 = insertelement <16 x float> %3, float %2, i32 1
  %5 = insertelement <16 x float> %4, float %1, i32 2
  %6 = insertelement <16 x float> %5, float %2, i32 3
  %7 = insertelement <16 x float> %6, float %1, i32 4
  %8 = insertelement <16 x float> %7, float %2, i32 5
  %9 = insertelement <16 x float> %8, float %1, i32 6
  %10 = insertelement <16 x float> %9, float %2, i32 7
  %11 = insertelement <16 x float> %10, float %1, i32 8
  %12 = insertelement <16 x float> %11, float %2, i32 9
  %13 = insertelement <16 x float> %12, float %1, i32 10
  %14 = insertelement <16 x float> %13, float %2, i32 11
  %15 = insertelement <16 x float> %14, float %1, i32 12
  %16 = insertelement <16 x float> %15, float %2, i32 13
  %17 = insertelement <16 x float> %16, float %1, i32 14
  %18 = insertelement <16 x float> %17, float %2, i32 15
  ret <16 x float> %18
}



define <16 x float> @test05(<16 x float> * %pin) nounwind {
entry:
;KNF:   vloadd  (%rdi), [[R0:%v[0-9]+]]
;KNF:   vshuf128x32     $170, $0, (%rdi), [[R2:%v[0-9]+]]
;KNF:   vshuf128x32     $85, $0, (%rdi), [[R1:%v[0-9]+]]
;KNF:   vstored [[R0]]{a}, -64(%rsp)
;KNF:   vstored [[R2]]{a}, -56(%rsp)
;KNF:   vstored [[R1]]{a}, -60(%rsp)
;KNF:   vloadq  -64(%rsp), %v0
  %0 = load <16 x float> *%pin, align 64
  %1 = extractelement <16 x float> %0, i32 0
  %2 = extractelement <16 x float> %0, i32 1
  %a = extractelement <16 x float> %0, i32 2
  %3 = insertelement <16 x float> undef, float %1, i32 0
  %4 = insertelement <16 x float> %3, float %2, i32 1
  %5 = insertelement <16 x float> %4, float %a, i32 2
  %6 = insertelement <16 x float> %5, float %2, i32 3
  %7 = insertelement <16 x float> %6, float %1, i32 4
  %8 = insertelement <16 x float> %7, float %a, i32 5
  %9 = insertelement <16 x float> %8, float %1, i32 6
  %10 = insertelement <16 x float> %9, float %2, i32 7
  %11 = insertelement <16 x float> %10, float %a, i32 8
  %12 = insertelement <16 x float> %11, float %2, i32 9
  %13 = insertelement <16 x float> %12, float %1, i32 10
  %14 = insertelement <16 x float> %13, float %a, i32 11
  %15 = insertelement <16 x float> %14, float %1, i32 12
  %16 = insertelement <16 x float> %15, float %2, i32 13
  %17 = insertelement <16 x float> %16, float %a, i32 14
  %18 = insertelement <16 x float> %17, float %2, i32 15
  ret <16 x float> %18
}


