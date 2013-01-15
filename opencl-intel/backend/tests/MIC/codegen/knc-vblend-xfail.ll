; XFAIL: *
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc     \
; RUN: | FileCheck %s -check-prefix=KNC

define <8 x i64> @A_i64(<8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: movl $1, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovapd %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <8 x i64> %a, <8 x i64> %b, <8 x i32> <i32 8, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <8 x i64> %m
}

define <8 x double> @A_f64(<8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNC: movl $1, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovapd %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <8 x double> %a, <8 x double> %b, <8 x i32> <i32 8, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <8 x double> %m
}

define <8 x i64> @B_i64(<8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: movl $80, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovapd %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <8 x i64> %a, <8 x i64> %b, <8 x i32> <i32 undef, i32 undef, i32 2, i32 undef, i32 12, i32 5, i32 14, i32 undef>
  ret <8 x i64> %m
}

define <8 x double> @B_f64(<8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNC: movl $80, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovpad %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <8 x double> %a, <8 x double> %b, <8 x i32> <i32 undef, i32 undef, i32 2, i32 undef, i32 12, i32 5, i32 14, i32 undef>
  ret <8 x double> %m
}
