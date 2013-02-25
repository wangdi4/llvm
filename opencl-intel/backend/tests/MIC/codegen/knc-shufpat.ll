; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

; this is an interleaved sequence of high and low parts extension
define <16 x i32> @vect_shuff(<16 x i32> %v0) {
; KNC: vect_shuff:
; KNC: vmovaps   _const_0(%rip), [[Z1:%zmm[0-9]+]]
; KNC: vpermd    %zmm0, [[Z1]], %zmm0
  %v = shufflevector <16 x i32> %v0, <16 x i32> undef, <16 x i32> <i32 0, i32 16, i32 1, i32 16, i32 2, i32 16, i32 3, i32 16, i32 4, i32 16, i32 5, i32 16, i32 6, i32 16, i32 7, i32 16>
  ret <16 x i32> %v
}

;; shuffle from 2 vectors
define <16 x i32> @vect_shuff2_v16i32(<16 x i32> %v0, <16 x i32> %v1) {
; KNC: vect_shuff2_v16i32:
; KNC: vpermf32x4 $16, %zmm0, %zmm0
; KNC: movl      $61680, %eax
; KNC: kmov      %eax, %k1
; KNC: vpermf32x4 $64, %zmm1, %zmm0{%k1}
;
  %v = shufflevector <16 x i32> %v0, <16 x i32> %v1, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 20, i32 21, i32 22, i32 23>
  ret <16 x i32> %v
}

;; shuffle from 2 vectors
define <8 x double> @vect_shuff2_v8f64(<8 x double> %v0, <8 x double> %v1) {
; KNC: vect_shuff2_v8f64:
; KNC: movl      $65280, %eax
; KNC: kmov      %eax, %k1
; KNC: vpermf32x4 $64, %zmm1, %zmm0{%k1}
;
  %v = shufflevector <8 x double> %v0, <8 x double> %v1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  ret <8 x double> %v
}

;; this is an interleaved sequence of high and low parts extension
define <16 x i32> @vect_shuff3(<16 x i32> %v0, <16 x i32> %v1) {
; KNC: vect_shuff3:
; KNC: vpshufd   $2, %zmm0, %zmm0
; KNC: movl      $2, %eax
; KNC: kmov      %eax, %k1
; KNC: vpermf32x4 $3, %zmm1, %zmm0{%k1}
;
  %v = shufflevector <16 x i32> %v0, <16 x i32> %v1, <16 x i32> <i32 2, i32 29, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %v
}
