; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s

target datalayout = "e-p:64:64"

define void @vect_trunc(<16 x i32>* %pout, <16 x i64>*%pin) {
; CHECK:  vmovaps   _const_0(%rip), [[V0:%zmm[0-9]+]]
; CHECK:  vpermd    64(%rsi), [[V0]], [[V2:%zmm[0-9]+]]
; CHECK:  kmov      %eax, [[K1:%k[1-7]+]]
; CHECK:  vpermd    (%rsi), [[V0]], [[V2]]{[[K1]]}


entry:
  %vin = load <16 x i64>* %pin
  %vext = trunc <16 x i64> %vin to <16 x i32>
  store <16 x i32> %vext, <16 x i32>* %pout, align 64
  ret void
}

