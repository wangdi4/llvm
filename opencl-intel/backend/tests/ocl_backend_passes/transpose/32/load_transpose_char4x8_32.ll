;; [LLVM 3.6 UPGRADE] The following LLVM IR is translated into similar but different ASM sequence.
;; Don't see any point to test codegen here. Consider to remove the test or move it to LLVM if it is necessary.
; XFAIL: *

; XFAIL: x86
; RUN: oclopt -runtimelib=clbltfng9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfns9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX2


define <8 x i8> @foo(<4 x i8>* %pLoadAdd){
entry:
   %xOut = alloca  <8 x i8>
   %yOut = alloca  <8 x i8>
   %zOut = alloca  <8 x i8>
   %wOut = alloca  <8 x i8>
   call void @__ocl_load_transpose_char_4x8(<4 x i8>* %pLoadAdd, <8 x i8>* nocapture %xOut, <8 x i8>* nocapture %yOut, <8 x i8>* nocapture %zOut, <8 x i8>* nocapture %wOut) nounwind
   %temp1 = load <8 x i8>* %xOut
   %temp2 = load <8 x i8>* %yOut
   %temp3 = load <8 x i8>* %zOut
   %temp4 = load <8 x i8>* %wOut
   %re0 = add <8 x i8> %temp1, %temp2
   %re1 = add <8 x i8> %temp3, %temp4
   %ret0 = add <8 x i8> %re0, %re1
   ret <8 x i8> %ret0
}

declare  void @__ocl_load_transpose_char_4x8(<4 x i8>* %pLoadAdd, <8 x i8>* nocapture %xOut, <8 x i8>* nocapture %yOut, <8 x i8>* nocapture %zOut, <8 x i8>* nocapture %wOut) nounwind


;CHECK-AVX:   vmovdqu	 ([[MEM:%[a-z]+]]), [[XMM1:%xmm[0-9]+]]
;CHECK-AVX:	  vmovdqu	 16([[MEM]]), [[XMM0:%xmm[0-9]+]]
;CHECK-AVX:	  vpshufb	 [[SHUF:%xmm[0-9]+]], [[XMM0]], [[XMM3:%xmm[0-9]+]]
;CHECK-AVX:	  vpshufb	 [[SHUF]], [[XMM1]], [[XMM2:%xmm[0-9]+]]
;CHECK-AVX:	  vpunpckhdq	[[XMM3]], [[XMM2]], [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	  vpunpckhbw	[[XMM3]], [[XMM4]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	  vpmovzxbw	[[XMM4]], [[XMM6:%xmm[0-9]+]]
;CHECK-AVX:	  vpunpckldq	[[XMM3]], [[XMM2]], [[XMM7:%xmm[0-9]+]]
;CHECK-AVX:	  vpunpckhbw	[[XMM3]], [[XMM7]], [[XMM8:%xmm[0-9]+]]
;CHECK-AVX:	  vpmovzxbw	[[XMM7]], [[XMM9:%xmm[0-9]+]]


;CHECK-AVX2:  vmovdqu	 ([[MEM:%[a-z]+]]), [[YMM0:%ymm[0-9]+]]
;CHECK-AVX2:  vpshufb	[[LCPI:.[_A-Z0-9]+]], [[YMM0]], [[YMM01:%ymm[0-9]+]]
;CHECK-AVX2:  vpermd	[[YMM01]], [[YMM10:%ymm[0-9]+]], [[YMM1:%ymm[0-9]+]]
;CHECK-AVX2:  vpunpckhbw	[[YMM01]], [[YMM1]], [[YMM02:%ymm[0-9]+]]
;CHECK-AVX2:  vpunpcklbw	[[YMM02]], [[YMM1]], [[YMM11:%ymm[0-9]+]]
;CHECK-AVX2:  vextracti128	$1, [[YMM11]], [[XMM3:%xmm[0-9]+]]
;CHECK-AVX2:  vpaddw	[[XMM20:%xmm[0-9]+]], [[XMM3]], [[XMM2:%xmm[0-9]+]]
;CHECK-AVX2:  vpaddw	[[XMM00:%xmm[0-9]+]], [[XMM1:%xmm[0-9]+]], [[XMM0:%xmm[0-9]+]]
;CHECK-AVX2:  vpaddw	[[XMM2]], [[XMM0]], [[XMM01:%xmm[0-9]+]]
