; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;
;

target datalayout = "e-p:64:64"

define void @vect_trunc(<16 x i32>* %pout, <16 x i64>*%pin) {
; KNF:  movl      $2570, [[R1:%[a-z]+]]
; KNF:  vkmov     [[R1]], [[K1:%k[1-7]+]]
; KNF:  movl      $65280, [[R2:%[a-z]+]]
; KNF:  vshuf128x32 $160, $49, [[V0:%v[0-9]+]], {{%v[0-9]+}}{[[K1]]}
; KNF:  vkmov     [[R2]], [[K2:%k[1-7]+]]
; KNF:  vshuf128x32 $160, $49, [[V1:%v[0-9]+]], {{%v[0-9]+}}{[[K1]]}
; KNF:  vshuf128x32 $216, $136, [[V0]], [[V2:%v[0-9]+]]
; KNF:  vshuf128x32 $216, $136, [[V1]], [[V2]]{[[K2]]}

entry:
  %vin = load <16 x i64>* %pin
  %vext = trunc <16 x i64> %vin to <16 x i32>
  store <16 x i32> %vext, <16 x i32>* %pout, align 64
  ret void
}

