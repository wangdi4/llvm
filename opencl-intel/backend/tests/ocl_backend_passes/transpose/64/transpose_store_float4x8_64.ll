; XFAIL: i686-pc-win32
; RUN: oclopt -runtimelib=clbltfne9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK
; RUN: oclopt -runtimelib=clbltfnl9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK


define <8 x float> @foo(<4 x float>* nocapture %pStoreAdd, <8 x float> %xIn, <8 x float> %yIn, <8 x float> %zIn, <8 x float> %wIn) nounwind{
entry:

    call void @__ocl_transpose_store_float_4x8(<4 x float>* nocapture %pStoreAdd, <8 x float> %xIn, <8 x float> %yIn, <8 x float> %zIn, <8 x float> %wIn) nounwind
    %re0 = fadd <8 x float> %xIn, %yIn
    %re1 = fadd <8 x float> %zIn, %wIn
    %ret0 = fadd <8 x float> %re0, %re1
    ret <8 x float> %ret0
}
 declare void @__ocl_transpose_store_float_4x8(<4 x float>* nocapture %pStoreAdd, <8 x float> %xIn, <8 x float> %yIn, <8 x float> %zIn, <8 x float> %wIn) nounwind


;CHECK:	.type    [[FOO:[_a-z]+]],@function
;CHECK: [[FOO]]: # @foo
;CHECK:	vunpcklps	[[YMM0:%ymm[0-9]+]], [[YMM1:%ymm[0-9]+]], [[YMM4:%ymm[0-9]+]]
;CHECK:	vunpcklps	[[YMM2:%ymm[0-9]+]], [[YMM3:%ymm[0-9]+]], [[YMM5:%ymm[0-9]+]]
;CHECK:	vunpcklpd	[[YMM4]], [[YMM5]], [[YMM6:%ymm[0-9]+]]
;CHECK:	vunpckhpd	[[YMM4]], [[YMM5]], [[YMM41:%ymm[0-9]+]]
;CHECK:	vunpckhps	[[YMM0]], [[YMM1]], [[YMM7:%ymm[0-9]+]]
;CHECK:	vunpckhps	[[YMM2]], [[YMM3]], [[YMM8:%ymm[0-9]+]]
;CHECK:	vunpcklpd	[[YMM7]], [[YMM8]], [[YMM51:%ymm[0-9]+]]
;CHECK:	vunpckhpd	[[YMM7]], [[YMM8]], [[YMM71:%ymm[0-9]+]]
;CHECK:	vaddps	[[YMM0]], [[YMM1]], [[YMM01:%ymm[0-9]+]]
;CHECK:	vaddps	[[YMM2]], [[YMM3]], [[YMM11:%ymm[0-9]+]]
;CHECK:	vextractf128	$1, [[YMM6]], 64([[RCX:%[_a-z]+]])
;CHECK:	vaddps	[[YMM01]], [[YMM11]], [[YMM02:%ymm[0-9]+]]
;CHECK:	vextractf128	$1, [[YMM41]], 80([[RCX]])
;CHECK:	vextractf128	$1, [[YMM51]], 96([[RCX]])
;CHECK:	vextractf128	$1, [[YMM71]], 112([[RCX]])
;CHECK:	.type	    [[TRANSPOSE:[_a-z]+]]_store_float_4x8,@function
