; XFAIL: x86
; RUN: oclopt -runtimelib=clbltfng9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfns9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX2


define <4 x float> @foo(<4 x float>* nocapture %pLoadAdd){
entry:
   %xOut = alloca  <4 x float>
   %yOut = alloca  <4 x float>
   %zOut = alloca  <4 x float>
   %wOut = alloca  <4 x float>
   call void @__ocl_load_transpose_float_4x4(<4 x float>* nocapture %pLoadAdd, <4 x float>* nocapture %xOut, <4 x float>* nocapture %yOut, <4 x float>* nocapture %zOut, <4 x float>* nocapture %wOut) nounwind
   %temp1 = load <4 x float>* %xOut
   %temp2 = load <4 x float>* %yOut
   %temp3 = load <4 x float>* %zOut
   %temp4 = load <4 x float>* %wOut
   %re0 = fadd <4 x float> %temp1, %temp2
   %re1 = fadd <4 x float> %temp3, %temp4
   %ret0 = fadd <4 x float> %re0, %re1
   ret <4 x float> %ret0
}

declare void @__ocl_load_transpose_float_4x4(<4 x float>* nocapture %pLoadAdd, <4 x float>* nocapture %xOut, <4 x float>* nocapture %yOut, <4 x float>* nocapture %zOut, <4 x float>* nocapture %wOut) nounwind

;CHECK-AVX:	.type    [[FOO:[_a-z]+]],@function
;CHECK-AVX:    [[FOO]]:
;CHECK-AVX:	vmovaps	48([[EAX:%[a-z]+]]), [[XMM1:%xmm[0-9]+]]
;CHECK-AVX:	vmovaps	([[EAX]]), [[XMM0:%xmm[0-9]+]]
;CHECK-AVX:	vmovaps	16([[EAX]]), [[XMM2:%xmm[0-9]+]]
;CHECK-AVX:	vunpckhps	[[XMM1]], [[XMM2]], [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	vmovaps	32([[EAX]]), [[XMM3:%xmm[0-9]+]]
;CHECK-AVX:	vunpckhps	[[XMM3]], [[XMM0]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vunpckhps	[[XMM4]], [[XMM5]], [[XMM6:%xmm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[XMM4]], [[XMM5]], [[XMM41:%xmm[0-9]+]]
;CHECK-AVX:	vaddps  	[[XMM6]], [[XMM41]], [[XMM42:%xmm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[XMM1]], [[XMM2]], [[XMM11:%xmm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[XMM3]], [[XMM0]], [[XMM01:%xmm[0-9]+]]
;CHECK-AVX:	vunpckhps	[[XMM11]], [[XMM01]], [[XMM21:%xmm[0-9]+]]
;CHECK-AVX:	vunpcklps	[[XMM11]], [[XMM01]], [[XMM02:%xmm[0-9]+]]
;CHECK-AVX:	vaddps	[[XMM21]], [[XMM02]], [[XMM04:%xmm[0-9]+]]
;CHECK-AVX:	vaddps	[[XMM42]], [[XMM04]], [[XMM05:%xmm[0-9]+]]
;CHECK-AVX:    .type	[[LOAD:[_a-z]+]]_transpose_float_4x4,@function

;CHECK-AVX2:	.type    [[FOO:[_a-z]+]],@function
;CHECK-AVX2:    [[FOO]]:
;CHECK-AVX2:	vmovups	([[EAX:%[a-z]+]]), [[YMM1:%ymm[0-9]+]]
;CHECK-AVX2:	vmovups	32([[EAX]]), [[YMM0:%ymm[0-9]+]]
;CHECK-AVX2:	vpermps	[[YMM1]], [[YMM2:%ymm[0-9]+]], [[YMM11:%ymm[0-9]+]]
;CHECK-AVX2:	vpermps	[[YMM0]], [[YMM2]], [[YMM3:%ymm[0-9]+]]
;CHECK-AVX2:	vpalignr	$8, [[YMM11]], [[YMM3]], [[YMM01:%ymm[0-9]+]]
;CHECK-AVX2:	vextracti128	$1, [[YMM0]], [[XMM21:%xmm[0-9]+]]
;CHECK-AVX2:	vblendps	$204, [[YMM3]], [[YMM11]], [[YMM12:%ymm[0-9]+]]
;CHECK-AVX2:	vextractf128	$1, [[YMM12]], [[XMM3:%xmm[0-9]+]]
;CHECK-AVX2:	vaddps	[[XMM21]], [[XMM3]], [[XMM2:%xmm[0-9]+]]
;CHECK-AVX2:	vaddps	[[XMM01:%xmm[0-9]+]], [[XMM1:%xmm[0-9]+]], [[XMM0:%xmm[0-9]+]]
;CHECK-AVX2:	vaddps	[[XMM2]], [[XMM0]], [[XMM02:%xmm[0-9]+]]
;CHECK-AVX2:    .type	[[LOAD:[_a-z]+]]_transpose_float_4x4,@function
