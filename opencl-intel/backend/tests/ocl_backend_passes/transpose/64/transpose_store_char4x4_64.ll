; XFAIL: i686-pc-win32
; RUN: oclopt -runtimelib=clbltfne9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK
; RUN: oclopt -runtimelib=clbltfnl9.rtl  -builtin-import -shuffle-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=x86_64-pc-Win64 | FileCheck %s -check-prefix=CHECK


define <4 x i8> @foo(<4 x i8>* %pStoreAdd, <4 x i8> %xIn, <4 x i8> %yIn, <4 x i8> %zIn, <4 x i8> %wIn) nounwind{
entry:
   call void @__ocl_transpose_store_char_4x4(<4 x i8>* %pStoreAdd, <4 x i8> %xIn, <4 x i8> %yIn, <4 x i8> %zIn, <4 x i8> %wIn) nounwind
   %re0 = add <4 x i8> %xIn, %yIn
   %re1 = add <4 x i8> %zIn, %wIn
   %ret0 = add <4 x i8> %re0, %re1
   ret <4 x i8> %ret0
}

declare void @__ocl_transpose_store_char_4x4(<4 x i8>* %pStoreAdd, <4 x i8> %xIn, <4 x i8> %yIn, <4 x i8> %zIn, <4 x i8> %wIn) nounwind


;CHECK:    .type    [[FOO:[_a-z]+]],@function
;CHECK:    [[FOO]]
;CHECK:    vpshufb	    [[XMM4:%xmm[0-9]+]], [[XMM00:%xmm[0-9]+]], [[XMM01:%[xm0-9]+]]
;CHECK:    vpshufb	    [[XMM4]], [[XMM10:%xmm[0-9]+]], [[XMM11:%xmm[0-9]+]]
;CHECK:    vpunpcklbw	[[XMM01]], [[XMM11]], [[XMM2:%xmm[0-9]+]]
;CHECK:    vpshufb	    [[XMM4]], [[XMM30:%xmm[0-9]+]], [[XMM31:%xmm[0-9]+]]
;CHECK:    vpshufb	    [[XMM4]], [[XMM50:%xmm[0-9]+]], [[XMM51:%xmm[0-9]+]]
;CHECK:    vpunpcklbw	[[XMM31]], [[XMM51]], [[XMM6:%xmm[0-9]+]]
;CHECK:    vpunpcklwd	[[XMM2]], [[XMM6]], [[XMM7:%xmm[0-9]+]]
;CHECK:    vmovdqu	    [[XMM7]], ([[MEM:%[a-z]+]])
;CHECK:	.type	    [[TRANSPOSE:[_a-z]+]]_store_char_4x4,@function


