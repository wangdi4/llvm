; XFAIL: x86
; RUN: oclopt -runtimelib=clbltfng9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfns9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX2


define <8 x float> @foo(<4 x float>* nocapture %pStoreAdd, <8 x float> %xIn, <8 x float> %yIn, <8 x float> %zIn, <8 x float> %wIn) nounwind{
entry:

    call void @__ocl_transpose_store_float_4x8(<4 x float>* nocapture %pStoreAdd, <8 x float> %xIn, <8 x float> %yIn, <8 x float> %zIn, <8 x float> %wIn) nounwind
    %re0 = fadd <8 x float> %xIn, %yIn
    %re1 = fadd <8 x float> %zIn, %wIn
    %ret0 = fadd <8 x float> %re0, %re1
    ret <8 x float> %ret0
}
 declare void @__ocl_transpose_store_float_4x8(<4 x float>* nocapture %pStoreAdd, <8 x float> %xIn, <8 x float> %yIn, <8 x float> %zIn, <8 x float> %wIn) nounwind


;CHECK-AVX:	.type    [[FOO:[_a-z]+]],@function
;CHECK-AVX: [[FOO]]:
;CHECK-AVX:	vunpcklps	[[YMM3:%ymm[0-9]+]], [[YMM2:%ymm[0-9]+]], [[YMM5:%ymm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[YMM1:%ymm[0-9]+]], [[YMM0:%ymm[0-9]+]], [[YMM6:%ymm[0-9]+]]
;CHECK-AVX:	vunpcklpd	[[YMM5]], [[YMM6]], [[YMM4:%ymm[0-9]+]]
;CHECK-AVX:	vmovapd	[[YMM4]], ([[ESP:%[a-z]+]])
;CHECK-AVX:	vunpckhpd	[[YMM5]], [[YMM6]], [[YMM51:%ymm[0-9]+]]
;CHECK-AVX:	vunpckhps	[[YMM3]], [[YMM2]], [[YMM7:%ymm[0-9]+]]
;CHECK-AVX:	vunpckhps	[[YMM1]], [[YMM0]], [[YMM41:%ymm[0-9]+]]
;CHECK-AVX:	vunpcklpd	[[YMM7]], [[YMM41]], [[YMM61:%ymm[0-9]+]]
;CHECK-AVX:	vunpckhpd	[[YMM7]], [[YMM41]], [[YMM71:%ymm[0-9]+]]
;CHECK-AVX:	vmovaps	([[ESP]]), [[YMM42:%ymm[0-9]+]]
;CHECK-AVX:	vextractf128	$1, [[YMM42]], 64([[EAX:%[a-z]+]])
;CHECK-AVX:	vextractf128	$1, [[YMM5]], 80([[EAX]])
;CHECK-AVX:	vextractf128	$1, [[YMM6]], 96([[EAX]])
;CHECK-AVX:	vextractf128	$1, [[YMM7]], 112([[EAX]])
;CHECK-AVX:	vaddps	[[YMM3]], [[YMM2]], [[YMM21:%ymm[0-9]+]]
;CHECK-AVX:	vaddps	[[YMM1]], [[YMM0]], [[YMM01:%ymm[0-9]+]]
;CHECK-AVX:	vaddps	[[YMM21]], [[YMM01]], [[YMM02:%ymm[0-9]+]]
;CHECK-AVX:	.type	    [[TRANSPOSE:[_a-z]+]]_store_float_4x8,@function

;CHECK-AVX2:	.type    [[FOO:[_a-z]+]],@function
;CHECK-AVX2:    [[FOO]]:
;CHECK-AVX2:	vunpcklps	[[YMM3:%ymm[0-9]+]], [[YMM2:%ymm[0-9]+]], [[YMM5:%ymm[0-9]+]]
;CHECK-AVX2:	vunpcklps	[[YMM1:%ymm[0-9]+]], [[YMM0:%ymm[0-9]+]], [[YMM6:%ymm[0-9]+]]
;CHECK-AVX2:	vunpcklpd	[[YMM5]], [[YMM6]], [[YMM4:%ymm[0-9]+]]
;CHECK-AVX2:	vmovapd	[[YMM4]], ([[ESP:%[a-z]+]])
;CHECK-AVX2:	vunpckhpd	[[YMM5]], [[YMM6]], [[YMM51:%ymm[0-9]+]]
;CHECK-AVX2:	vunpckhps	[[YMM3]], [[YMM2]], [[YMM7:%ymm[0-9]+]]
;CHECK-AVX2:	vunpckhps	[[YMM1]], [[YMM0]], [[YMM41:%ymm[0-9]+]]
;CHECK-AVX2:	vunpcklpd	[[YMM7]], [[YMM41]], [[YMM61:%ymm[0-9]+]]
;CHECK-AVX2:	vunpckhpd	[[YMM7]], [[YMM41]], [[YMM71:%ymm[0-9]+]]
;CHECK-AVX2:	vmovaps	([[ESP]]), [[YMM41]]
;CHECK-AVX2:	vextractf128	$1, [[YMM41]], 64([[EAX:%[a-z]+]])
;CHECK-AVX2:	vextractf128	$1, [[YMM51]], 80([[EAX]])
;CHECK-AVX2:	vextractf128	$1, [[YMM61]], 96([[EAX]])
;CHECK-AVX2:	vextractf128	$1, [[YMM71]], 112([[EAX]])
;CHECK-AVX2:	vaddps	[[YMM3]], [[YMM2]], [[YMM21:%ymm[0-9]+]]
;CHECK-AVX2:	vaddps	[[YMM1]], [[YMM0]], [[YMM01:%ymm[0-9]+]]
;CHECK-AVX2:	vaddps	[[YMM21]], [[YMM01]], [[YMM02:%ymm[0-9]+]]
;CHECK-AVX2:	.type	    [[TRANSPOSE:[_a-z]+]]_store_float_4x8,@function
