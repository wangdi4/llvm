; XFAIL: i686-pc-win32
; RUN: oclopt -runtimelib=clbltfne9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfnl9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK-AVX2




define <8 x i8> @foo(<4 x i8>* %pStoreAdd, <8 x i8> %xIn, <8 x i8> %yIn, <8 x i8> %zIn, <8 x i8> %wIn) nounwind{
entry:

    call void @__ocl_transpose_store_char_4x8(<4 x i8>* %pStoreAdd, <8 x i8> %xIn, <8 x i8> %yIn, <8 x i8> %zIn, <8 x i8> %wIn) nounwind
    %re0 = add <8 x i8> %xIn, %yIn
    %re1 = add <8 x i8> %zIn, %wIn
    %ret0 = add <8 x i8> %re0, %re1
    ret <8 x i8> %ret0
}
declare void @__ocl_transpose_store_char_4x8(<4 x i8>* %pStoreAdd, <8 x i8> %xIn, <8 x i8> %yIn, <8 x i8> %zIn, <8 x i8> %wIn) nounwind



;-------------------------------------------------------------------------------
;CHECK-AVX:	.type    [[FOO:[_a-z]+]],@function
;CHECK-AVX: [[FOO]]
;CHECK-AVX:	vpshufb	 [[SHUF:%xmm[0-9]+]], [[TMP:%xmm[0-9]+]], [[XMM0:%xmm[0-9]+]]
;CHECK-AVX:	vpshufb	 [[SHUF]], [[TMP1:%xmm[0-9]+]], [[XMM2:%xmm[0-9]+]]
;CHECK-AVX:	vpunpcklbw	[[XMM0]], [[XMM2]], [[XMM1:%xmm[0-9]+]]
;CHECK-AVX:	vpshufb	 [[SHUF]], [[TMP2:%xmm[0-9]+]], [[XMM3:%xmm[0-9]+]]
;CHECK-AVX:	vpshufb	 [[SHUF]], [[TMP3:%xmm[0-9]+]], [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	vpunpcklbw	[[XMM3]], [[XMM4]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vpunpcklwd	[[XMM1]], [[XMM5]], [[XMM6:%xmm[0-9]+]] 
;CHECK-AVX:	vmovdqu	[[XMM6]], ([[MEM:%[a-z]+]])
;CHECK-AVX:	vpunpckhwd	[[XMM1]], [[XMM5]], [[XMM7:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqu	[[XMM7]], 16([[MEM]])
;CHECK-AVX:	ret
;CHECK-AVX:	.type	 [[TRANSPOSE:[_a-z]+]]_store_char_4x8,@function


;-------------------------------------------------------------------------------
;CHECK-AVX2:   .type    [[FOO:[_a-z]+]],@function
;CHECK-AVX2:   [[FOO]]
;CHECK-AVX2:   vinserti128	$1, [[XMM0:%xmm[0-9]+]], [[YMM1:%ymm[0-9]+]], [[YMM2:%ymm[0-9]+]]
;CHECK-AVX2:   vpshufb	[[YMM5:%ymm[0-9]+]], [[YMM2]], [[YMM4:%ymm[0-9]+]]
;CHECK-AVX2:   vinserti128	$1, [[XMM24:%xmm[0-9]+]], [[YMM3:%ymm[0-9]+]], [[YMM6:%ymm[0-9]+]]
;CHECK-AVX2:   vpshufb	[[YMM5]], [[YMM6]], [[YMM51:%ymm[0-9]+]]
;CHECK-AVX2:   vpunpcklbw	[[YMM4]], [[YMM51]], [[YMM41:%ymm[0-9]+]]
;CHECK-AVX2:   vmovdqa	.[[LCPI0:[_A-Z0-9]+]]([[RIP:%[_a-z]+]]), [[YMM52:%ymm[0-9]+]]
;CHECK-AVX2:   vpermd	[[YMM41]], [[YMM52]], [[YMM42:%ymm[0-9]+]]
;CHECK-AVX2:   vpshufb	.[[LCPI1:[_A-Z0-9]+]]([[RIP]]), [[YMM51]], [[YMM43:%ymm[0-9]+]]
;CHECK-AVX2:   vpaddw	[[XMM00:%xmm[0-9]+]], [[XMM2:%xmm[0-9]+]], [[XMM0:%xmm[0-9]+]]
;CHECK-AVX2:   vpaddw	[[XMM10:%xmm[0-9]+]], [[XMM3:%xmm[0-9]+]], [[XMM1:%xmm[0-9]+]]
;CHECK-AVX2:   vpaddw	[[XMM0]], [[XMM1]], [[XMM01:%xmm[0-9]+]]
;CHECK-AVX2:   ret
;CHECK-AVX2:   .type	 [[TRANSPOSE:[_a-z]+]]_store_char_4x8,@function
