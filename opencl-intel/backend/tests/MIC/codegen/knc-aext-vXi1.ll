; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC

;
;
;

target datalayout = "e-p:64:64"

;; this is an interleaved sequence of high and low parts extension
define void @vect_sext1(<16 x i1>* %pin, <16 x i32>*%pout) {
entry:
; KNC: vect_sext1:
; KNC: vblendmps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+{%k[1-7]}}
  %vin = load <16 x i1>* %pin, align 8
  %vext = sext <16 x i1> %vin to <16 x i32>
  store <16 x i32> %vext, <16 x i32>* %pout, align 64

  ret void
}

define void @vect_sext2(<16 x i1>* %pin, <16 x i64>*%pout) {
entry:
; KNC: vect_sext2:
; KNC: vblendmpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+{%k[1-7]}}
; KNC: vblendmpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+{%k[1-7]}}
  %vin = load <16 x i1>* %pin, align 8
  %vext = sext <16 x i1> %vin to <16 x i64>
  store <16 x i64> %vext, <16 x i64>* %pout, align 64

  ret void
}

define void @vect_zext1(<16 x i1>* %pin, <16 x i32>*%pout) {
entry:
; KNC: vect_zext1:
; KNC: vblendmps {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+{%k[1-7]}}
  %vin = load <16 x i1>* %pin, align 8
  %vext = zext <16 x i1> %vin to <16 x i32>
  store <16 x i32> %vext, <16 x i32>* %pout, align 64

  ret void
}

define void @vect_zext2(<16 x i1>* %pin, <16 x i64>*%pout) {
entry:
; KNC: vect_zext2:
; KNC: vblendmpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+{%k[1-7]}}
; KNC: vblendmpd {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+{%k[1-7]}}
  %vin = load <16 x i1>* %pin, align 8
  %vext = zext <16 x i1> %vin to <16 x i64>
  store <16 x i64> %vext, <16 x i64>* %pout, align 64

  ret void
}

