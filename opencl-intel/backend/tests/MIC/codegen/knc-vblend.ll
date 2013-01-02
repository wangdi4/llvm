; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc     \
; RUN: | FileCheck %s -check-prefix=KNC

define <16 x i32> @A_i32(<16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNC: movl $1, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovaps %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <16 x i32> %a, <16 x i32> %b, <16 x i32> <i32 16, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x float> @A_f32(<16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNC: movl $1, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovaps %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <16 x float> %a, <16 x float> %b, <16 x i32> <i32 16, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x float> %m
}

define <16 x i32> @B_i32(<16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNC: movl $1058, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovaps %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <16 x i32> %a, <16 x i32> %b, <16 x i32> <i32 undef, i32 17, i32 undef, i32 undef, i32 4, i32 21, i32 6, i32 undef, i32 undef, i32 undef, i32 26, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x i32> %m
}

define <16 x float> @B_f32(<16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNC: movl $1058, [[R0:%[a-z]+]]
; KNC: kmov [[R0]], [[K0:%k[1-7]+]]
; KNC: vmovaps %zmm1, %zmm0{[[K0]]}
  %m = shufflevector <16 x float> %a, <16 x float> %b, <16 x i32> <i32 undef, i32 17, i32 undef, i32 undef, i32 4, i32 21, i32 6, i32 undef, i32 undef, i32 undef, i32 26, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  ret <16 x float> %m
}



