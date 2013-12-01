; XFAIL: x86
; RUN: oclopt -runtimelib=clbltfng9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK
; RUN: oclopt -runtimelib=clbltfns9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK


define <8 x float> @foo(<4 x float>* nocapture %pLoadAdd) nounwind{
entry:
   %xOut = alloca  <8 x float>
   %yOut = alloca  <8 x float>
   %zOut = alloca  <8 x float>
   %wOut = alloca  <8 x float>
   call void @__ocl_load_transpose_float_4x8(<4 x float>* nocapture %pLoadAdd, <8 x float>* nocapture %xOut, <8 x float>* nocapture %yOut, <8 x float>* nocapture %zOut, <8 x float>* nocapture %wOut) nounwind
   %temp1 = load <8 x float>* %xOut
   %temp2 = load <8 x float>* %yOut
   %temp3 = load <8 x float>* %zOut
   %temp4 = load <8 x float>* %wOut
   %re0 = fadd <8 x float> %temp1, %temp2
   %re1 = fadd <8 x float> %temp3, %temp4
   %ret0 = fadd <8 x float> %re0, %re1
   ret <8 x float> %ret0
}


declare void @__ocl_load_transpose_float_4x8(<4 x float>* nocapture %pLoadAdd, <8 x float>* nocapture %xOut, <8 x float>* nocapture %yOut, <8 x float>* nocapture %zOut, <8 x float>* nocapture %wOut) nounwind



;CHECK:	.type    [[FOO:[_a-z]+]],@function
;CHECK:    [[FOO]]:
;CHECK:	vmovaps	48([[EAX:%[a-z]+]]), [[XMM0:%xmm[0-9]+]]
;CHECK:	vinsertf128	$1, 112([[EAX]]), [[YMM01:%ymm[0-9]+]], [[YMM0:%ymm[0-9]+]]
;CHECK:	vmovaps	([[EAX]]), [[XMM2:%xmm[0-9]+]]
;CHECK:	vmovaps	16([[EAX]]), [[XMM11:%xmm[0-9]+]]
;CHECK:	vinsertf128	$1, 80([[EAX]]), [[YMM11:%ymm[0-9]+]], [[YMM1:%ymm[0-9]+]]
;CHECK:	vunpckhps	[[YMM0]], [[YMM1]], [[YMM3:%ymm[0-9]+]]
;CHECK:	vmovaps	32([[EAX]]), [[XMM4:%xmm[0-9]+]]
;CHECK:	vinsertf128	$1, 96([[EAX]]), [[YMM41:%ymm[0-9]+]], [[YMM4:%ymm[0-9]+]]
;CHECK:	vinsertf128	$1, 64([[EAX]]), [[YMM21:%ymm[0-9]+]], [[YMM5:%ymm[0-9]+]]
;CHECK:	vunpckhps	[[YMM4]], [[YMM5]], [[YMM6:%ymm[0-9]+]]
;CHECK:	vunpckhps	[[YMM3]], [[YMM6]], [[YMM2:%ymm[0-9]+]]
;CHECK:	vunpcklps	[[YMM3]], [[YMM6]], [[YMM32:%ymm[0-9]+]]
;CHECK:	vunpcklps	[[YMM01]], [[YMM1]], [[YMM02:%ymm[0-9]+]]
;CHECK:	vunpcklps	[[YMM4]], [[YMM5]], [[YMM31:%ymm[0-9]+]]
;CHECK:	vunpckhps	[[YMM02]], [[YMM31]], [[YMM12:%ymm[0-9]+]]
;CHECK:	vunpcklps	[[YMM02]], [[YMM31]], [[YMM03:%ymm[0-9]+]]
;CHECK:    .type	[[LOAD:[_a-z]+]]_transpose_float_4x8,@function

















