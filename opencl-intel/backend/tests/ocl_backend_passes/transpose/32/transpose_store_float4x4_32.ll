;; [LLVM 3.6 UPGRADE] The following LLVM IR is translated into similar but different ASM sequence.
;; Don't see any point to test codegen here. Consider to remove the test or move it to LLVM if it is necessary.
; XFAIL: *

; XFAIL: x86
; RUN: oclopt -runtimelib=clbltfng9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfns9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX2


define <4 x float> @foo(<4 x float>* nocapture %pStoreAdd, <4 x float> %xIn, <4 x float> %yIn, <4 x float> %zIn, <4 x float> %wIn) nounwind{
entry:
    call void @__ocl_transpose_store_float_4x4(<4 x float>* nocapture %pStoreAdd, <4 x float> %xIn, <4 x float> %yIn, <4 x float> %zIn, <4 x float> %wIn) nounwind
    %re0 = fadd <4 x float> %xIn, %yIn
    %re1 = fadd <4 x float> %zIn, %wIn
    %ret0 = fadd <4 x float> %re0, %re1
    ret <4 x float> %ret0
}

declare void @__ocl_transpose_store_float_4x4(<4 x float>* nocapture %pStoreAdd, <4 x float> %xIn, <4 x float> %yIn, <4 x float> %zIn, <4 x float> %wIn) nounwind


;CHECK-AVX:	vunpcklps	[[XMM3:%xmm[0-9]+]], [[XMM1:%xmm[0-9]+]], [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[XMM2:%xmm[0-9]+]], [[XMM0:%xmm[0-9]+]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[XMM4]], [[XMM5]], [[XMM6:%xmm[0-9]+]]
;CHECK-AVX:	vmovaps	[[XMM6]], ([[EAX:%[a-z]+]])
;CHECK-AVX:	vunpckhps	[[XMM4]], [[XMM5]], [[XMM41:%xmm[0-9]+]]
;CHECK-AVX:	vmovaps	[[XMM41]], 16([[EAX]])
;CHECK-AVX:	vunpckhps	[[XMM3]], [[XMM1]], [[XMM42:%xmm[0-9]+]]
;CHECK-AVX:	vunpckhps	[[XMM2]], [[XMM0]], [[XMM51:%xmm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[XMM42]], [[XMM51]], [[XMM61:%xmm[0-9]+]]
;CHECK-AVX:	vmovaps	[[XMM61]], 32([[EAX]])
;CHECK-AVX:	vunpckhps	[[XMM42]], [[XMM51]], [[XMM43:%xmm[0-9]+]]
;CHECK-AVX:	vmovaps	[[XMM43]], 48([[EAX]])
;CHECK-AVX:	vaddps	[[XMM3]], [[XMM2]], [[XMM21:%xmm[0-9]+]]
;CHECK-AVX:	vaddps	[[XMM1]], [[XMM0]], [[XMM01:%xmm[0-9]+]]
;CHECK-AVX:	vaddps	[[XMM21]], [[XMM01]], [[XMM02:%xmm[0-9]+]]

;CHECK-AVX2:	vinsertf128	$1, [[XMM3:%xmm[0-9]+]], [[YMM2:%ymm[0-9]+]], [[YMM4:%ymm[0-9]+]]
;CHECK-AVX2:	vpermps	[[YMM4]], [[YMM5:%ymm[0-9]+]], [[YMM41:%ymm[0-9]+]]
;CHECK-AVX2:	vinsertf128	$1, [[XMM1:%xmm[0-9]+]], [[YMM0:%ymm[0-9]+]], [[YMM5:%ymm[0-9]+]]
;CHECK-AVX2:	vpermps	[[YMM5]], [[YMM6:%ymm[0-9]+]], [[YMM51:%ymm[0-9]+]]
;CHECK-AVX2:	vblendps	$204, [[YMM41]], [[YMM51]], [[YMM61:%ymm[0-9]+]]
;CHECK-AVX2:	vmovups	[[YMM61]], ([[EAX:%[a-z]+]])
;CHECK-AVX2:	vpalignr	$8, [[YMM51]], [[YMM41]], [[YMM42:%ymm[0-9]+]]
;CHECK-AVX2:	vmovdqu	[[YMM42]], 32([[EAX]])
;CHECK-AVX2:	vaddps	[[XMM3:%xmm[0-9]+]], [[XMM21:%xmm[0-9]+]], [[XMM2:%xmm[0-9]+]]
;CHECK-AVX2:	vaddps	[[XMM1:%xmm[0-9]+]], [[XMM01:%xmm[0-9]+]], [[XMM0:%xmm[0-9]+]]
;CHECK-AVX2:	vaddps	[[XMM2]], [[XMM0]], [[XMM0:%xmm[0-9]+]]
