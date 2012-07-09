; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC

;
;
;

target datalayout = "e-p:64:64"

;; this is an interleaved sequence of high and low parts extension
define void @vect_sext2(<16 x i32>* %pin, <16 x i64>*%pout) {
; KNF:  movl      $61680, [[R1:%[a-z]+]]
; KNF:  vshuf128x32 $80, $250, [[V0:%v[0-9]+]], [[V1:%v[0-9]+]]
; KNF:  vkmov     [[R1]], [[K1:%k[1-7]+]]
; KNF:  vshuf128x32 $80, $80, [[V0]], [[V2:%v[0-9]+]]
; KNF:  vshuf128x32 $250, $250, [[V0]], [[V1]]{[[K1]]}
; KNF:  vshuf128x32 $250, $80, [[V0]], [[V2]]{[[K1]]}
; KNF:  vsrapi    {{%v[0-9]+}}, [[V1]], {{%v[0-9]+}}
; KNF:  vsrapi    {{%v[0-9]+}}, [[V2]], {{%v[0-9]+}}
;
; KNC:  movl      $43690, %eax
; KNC:  kmov      %eax, [[K1:%k[1-7]+]]
; KNC:  vmovaps   _const_0(%rip), [[V0:%zmm[0-9]+]]
; KNC:  vmovaps   _const_1(%rip), [[V1:%zmm[0-9]+]]
; KNC:  vpermd    [[V2:%zmm[0-9]+]], [[V0]], [[V3:%zmm[0-9]+]]
; KNC:  vpermd    [[V2]], [[V1]], [[V4:%zmm[0-9]+]]
; KNC:  vpsrad    $31, [[V3]], [[V3]]{[[K1]]}
; KNC:  vpsrad    $31, [[V4]], [[V4]]{[[K1]]}

entry:
  %vin = load <16 x i32>* %pin, align 8
  %vext = sext <16 x i32> %vin to <16 x i64>
  store <16 x i64> %vext, <16 x i64>* %pout, align 64

  ret void
}

define void @vect_zext2(<16 x i32>* %pin, <16 x i64>*%pout) {
entry:
; KNF:  movl      $61680, [[R1:%[a-z]+]]
; KNF:  vkmov     [[R1]], [[K1:%k[1-7]+]]
; KNF:  movl      $43690, [[R2:%[a-z]+]]
; KNF:  vshuf128x32 $80, $250, [[V0:%v[0-9]+]], [[V1:%v[0-9]+]]
; KNF:  vshuf128x32 $80, $80, [[V0]], [[V2:%v[0-9]+]]
; KNF:  vshuf128x32 $250, $250, [[V0]], [[V1]]{[[K1]]}
; KNF:  vshuf128x32 $250, $80, [[V0]], [[V2]]{[[K1]]}
; KNF:  vxorpi    [[V1]], [[V1]], [[V1]]{%k{{[1-7]+}}}
; KNF:  vxorpi    [[V2]], [[V2]], [[V2]]{%k{{[1-7]+}}}
;
; KNC:  movl      $43690, %eax
; KNC:  kmov      %eax, [[K1:%k[1-7]+]]
; KNC:  vmovaps   _const_2(%rip), [[V0:%zmm[0-9]+]]
; KNC:  vmovaps   _const_3(%rip), [[V1:%zmm[0-9]+]]
; KNC:  vpermd    [[V2:%zmm[0-9]+]], [[V0]], [[V4:%zmm[0-9]+]]
; KNC:  vpermd    [[V2]], [[V1]], [[V5:%zmm[0-9]+]]
; KNC:  vmovaps   [[V3]], [[V4]]{[[K1]]}
; KNC:  vmovaps   [[V3]], [[V5]]{[[K1]]}

  %vin = load <16 x i32>* %pin, align 8
  %vext = zext <16 x i32> %vin to <16 x i64>
  store <16 x i64> %vext, <16 x i64>* %pout, align 64

  ret void
}
