;; [LLVM 3.6 UPGRADE] The following LLVM IR is translated into similar but different ASM sequence.
;; Don't see any point to test codegen here. Consider to remove the test or move it to LLVM if it is necessary.
; XFAIL: *

; XFAIL: i686-pc-win32
; RUN: oclopt -runtimelib=clbltfne9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfnl9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK-AVX2



define <4 x i32> @foo(<4 x i32>* nocapture %pLoadAdd){
entry:
   %xOut = alloca  <4 x i32>
   %yOut = alloca  <4 x i32>
   %zOut = alloca  <4 x i32>
   %wOut = alloca  <4 x i32>
   call void @__ocl_load_transpose_int_4x4(<4 x i32>* nocapture %pLoadAdd, <4 x i32>* nocapture %xOut, <4 x i32>* nocapture %yOut, <4 x i32>* nocapture %zOut, <4 x i32>* nocapture %wOut) nounwind
   %temp1 = load <4 x i32>* %xOut
   %temp2 = load <4 x i32>* %yOut
   %temp3 = load <4 x i32>* %zOut
   %temp4 = load <4 x i32>* %wOut
   %re0 = add <4 x i32> %temp1, %temp2
   %re1 = add <4 x i32> %temp3, %temp4
   %ret0 = add <4 x i32> %re0, %re1
   ret <4 x i32> %ret0
}


declare void @__ocl_load_transpose_int_4x4(<4 x i32>* nocapture %pLoadAdd, <4 x i32>* nocapture %xOut, <4 x i32>* nocapture %yOut, <4 x i32>* nocapture %zOut, <4 x i32>* nocapture %wOut) nounwind

;CHECK-AVX:	vmovdqa	([[RCX:%[a-z]+]]), [[XMM0:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	16([[RCX]]), [[XMM1:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	32([[RCX]]), [[XMM2:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	48([[RCX]]), [[XMM3:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM3]], [[XMM1]], [[XMM6:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM2]], [[XMM0]], [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM6]], [[XMM4]], [[XMM8:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM6]], [[XMM4]], [[XMM9:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM3]], [[XMM1]], [[XMM7:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM2]], [[XMM0]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM7]], [[XMM5]], [[XMM10:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM7]], [[XMM5]], [[XMM11:%xmm[0-9]+]]

;CHECK-AVX2:	vpermd	[[YMM10:%ymm[0-9]+]], [[SHUF:%ymm[0-9]+]], [[YMM1:%ymm[0-9]+]]
;CHECK-AVX2:	vpermd	[[YMM00:%ymm[0-9]+]], [[SHUF]], [[YMM3:%ymm[0-9]+]]
;CHECK-AVX2:	vpalignr	$8, [[YMM1]], [[YMM3]], [[YMM0:%ymm[0-9]+]]
;CHECK-AVX2:	vextracti128	$1, [[YMM0]], [[XMM2:%xmm[0-9]+]]
;CHECK-AVX2:	vpblendd	$204, [[YMM3]], [[YMM1]], [[YMM12:%ymm[0-9]+]]
;CHECK-AVX2:	vextracti128	$1, [[YMM12]], [[XMM3:%xmm[0-9]+]]
;CHECK-AVX2:	vpaddd	[[XMM2]], [[XMM3]], [[XMM21:%xmm[0-9]+]]
;CHECK-AVX2:	vpaddd	[[XMM0:%xmm[0-9]+]], [[XMM1:%xmm[0-9]+]], [[XMM01:%xmm[0-9]+]]
;CHECK-AVX2:	vpaddd	[[XMM21]], [[XMM01]], [[XMM02:%xmm[0-9]+]]
