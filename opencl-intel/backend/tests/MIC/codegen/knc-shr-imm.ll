; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
	
target datalayout = "e-p:64:64"
	
define <16 x i32>  @f1(<16 x i32> %arg0) {
; KNC: vpsrad    $1, %zmm0, %zmm0
entry:
  %res = ashr <16 x i32> %arg0, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  ret <16 x i32> %res
}

define <16 x i32>  @f2(<16 x i32> %arg0, <16 x i32> %arg1) {
; KNC: vpermd    %zmm1, %zmm2, %zmm3
; KNC: vpsravd   %zmm3, %zmm0, %zmm0
entry:
  %amount = shufflevector <16 x i32> %arg1, <16 x i32> undef, <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %res = ashr <16 x i32> %arg0, %amount
  ret <16 x i32> %res
}

define <16 x i32>  @f3(<16 x i32> %arg0, <16 x i32> %arg1) {
; KNC: vpsravd   %zmm1, %zmm0, %zmm0
entry:
  %res = ashr <16 x i32> %arg0, %arg1
  ret <16 x i32> %res
}

