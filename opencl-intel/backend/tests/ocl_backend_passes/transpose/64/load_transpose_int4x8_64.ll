; XFAIL: i686-pc-win32
; RUN: oclopt -runtimelib=clbltfne9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfnl9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK-AVX2



define <8 x i32> @foo(<4 x i32>* nocapture %pLoadAdd) nounwind{
entry:
   %xOut = alloca  <8 x i32>
   %yOut = alloca  <8 x i32>
   %zOut = alloca  <8 x i32>
   %wOut = alloca  <8 x i32>
   call void @__ocl_load_transpose_int_4x8(<4 x i32>* nocapture %pLoadAdd, <8 x i32>* nocapture %xOut, <8 x i32>* nocapture %yOut, <8 x i32>* nocapture %zOut, <8 x i32>* nocapture %wOut) nounwind
   %temp1 = load <8 x i32>* %xOut
   %temp2 = load <8 x i32>* %yOut
   %temp3 = load <8 x i32>* %zOut
   %temp4 = load <8 x i32>* %wOut
   %re0 = add <8 x i32> %temp1, %temp2
   %re1 = add <8 x i32> %temp3, %temp4
   %ret0 = add <8 x i32> %re0, %re1
   ret <8 x i32> %ret0
}

declare void @__ocl_load_transpose_int_4x8(<4 x i32>* nocapture %pLoadAdd, <8 x i32>* nocapture %xOut, <8 x i32>* nocapture %yOut, <8 x i32>* nocapture %zOut, <8 x i32>* nocapture %wOut) nounwind



;CHECK-AVX:	.type    [[FOO:[_a-z]+]],@function
;CHECK-AVX: [[FOO]]
;CHECK-AVX:	vmovdqa	112([[RCX:%[a-z]+]]), [[XMM0:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	80([[RCX]]), [[XMM1:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM0]], [[XMM1]], [[XMM3:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	96([[RCX]]), [[XMM2:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	64([[RCX]]), [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM2]], [[XMM4]], [[XMM6:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM3]], [[XMM6]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM3]], [[XMM6]], [[XMM31:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM0]], [[XMM1]], [[XMM01:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM2]], [[XMM4]], [[XMM21:%xmm[0-9]+]]
;CHECK-AVX:	vpaddd	[[XMM5]], [[XMM31]], [[XMM32:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM01]], [[XMM21]], [[XMM11:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM01]], [[XMM21]], [[XMM011:%xmm[0-9]+]]
;CHECK-AVX:	vpaddd	[[XMM11]], [[XMM011]], [[XMM02:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	([[RCX]]), [[XMM12:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	16([[RCX]]), [[XMM33:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	32([[RCX]]), [[XMM51:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	48([[RCX]]), [[XMM22:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM22]], [[XMM33]], [[XMM41:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM51]], [[XMM12]], [[XMM7:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM41]], [[XMM7]], [[XMM61:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM41]], [[XMM7]], [[XMM42:%xmm[0-9]+]]
;CHECK-AVX:	vpaddd	[[XMM32]], [[XMM02]], [[XMM03:%xmm[0-9]+]]
;CHECK-AVX:	vpaddd	[[XMM61]], [[XMM42]], [[XMM43:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM22]], [[XMM33]], [[XMM23:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM51]], [[XMM12]], [[XMM34:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM23]], [[XMM34]], [[XMM13:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM23]], [[XMM34]], [[XMM24:%xmm[0-9]+]]
;CHECK-AVX:	vpaddd	[[XMM13]], [[XMM24]], [[XMM14:%xmm[0-9]+]]
;CHECK-AVX:	vpaddd	[[XMM43]], [[XMM14]], [[XMM15:%xmm[0-9]+]]
;CHECK-AVX:    .type	[[LOAD:[_a-z]+]]_transpose_int_4x8,@function

;CHECK-AVX2:	.type    [[FOO:[_a-z]+]],@function
;CHECK-AVX2:    [[FOO]]
;CHECK-AVX2:	vinserti128	$1, 112([[EAX:%[a-z]+]]), [[YMM00:%ymm[0-9]+]], [[YMM0:%ymm[0-9]+]]
;CHECK-AVX2:	vinserti128	$1, 80([[EAX]]), [[YMM10:%ymm[0-9]+]], [[YMM1:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckhdq	[[YMM0]], [[YMM1]], [[YMM3:%ymm[0-9]+]]
;CHECK-AVX2:	vinserti128	$1, 96([[EAX]]), [[YMM40:%ymm[0-9]+]], [[YMM4:%ymm[0-9]+]]
;CHECK-AVX2:	vinserti128	$1, 64([[EAX]]), [[YMM20:%ymm[0-9]+]], [[YMM5:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckhdq	[[YMM4]], [[YMM5]], [[YMM6:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckhdq	[[YMM3]], [[YMM6]], [[YMM2:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckldq	[[YMM3]], [[YMM6]], [[YMM31:%ymm[0-9]+]]
;CHECK-AVX2:	vpaddd	[[YMM2]], [[YMM31]], [[YMM21:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckldq	[[YMM0]], [[YMM1]], [[YMM01:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckldq	[[YMM4]], [[YMM5]], [[YMM32:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckhdq	[[YMM01]], [[YMM32]], [[YMM11:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckldq	[[YMM01]], [[YMM32]], [[YMM02:%ymm[0-9]+]]
;CHECK-AVX2:	vpaddd	[[YMM11]], [[YMM02]], [[YMM03:%ymm[0-9]+]]
;CHECK-AVX2:	vpaddd	[[YMM21]], [[YMM03]], [[YMM04:%ymm[0-9]+]]
;CHECK-AVX2:    .type	[[LOAD:[_a-z]+]]_transpose_int_4x8,@function
