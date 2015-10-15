;; [LLVM 3.6 UPGRADE] The following LLVM IR is translated into similar but different ASM sequence.
;; Don't see any point to test codegen here. Consider to remove the test or move it to LLVM if it is necessary.
; XFAIL: *

; XFAIL: x86
; RUN: oclopt -runtimelib=clbltfng9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfns9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX2




define void @foo(<4 x i32>* nocapture %pStoreAdd, <4 x i32> %xIn, <4 x i32> %yIn, <4 x i32> %zIn, <4 x i32> %wIn) nounwind{
entry:
    %xIn.addr = alloca <4 x i32>, align 4
    store <4 x i32> %xIn, <4 x i32>* %xIn.addr, align 4
    %yIn.addr = alloca <4 x i32>, align 4
    store <4 x i32> %yIn, <4 x i32>* %yIn.addr, align 4
	%zIn.addr = alloca <4 x i32>, align 4
    store <4 x i32> %zIn, <4 x i32>* %zIn.addr, align 4
	%wIn.addr = alloca <4 x i32>, align 4
    store <4 x i32> %wIn, <4 x i32>* %wIn.addr, align 4
    call void @__ocl_transpose_store_int_4x4(<4 x i32>* nocapture %pStoreAdd, <4 x i32> %xIn, <4 x i32> %yIn, <4 x i32> %zIn, <4 x i32> %wIn) nounwind
	ret void
}

declare void @__ocl_transpose_store_int_4x4(<4 x i32>* nocapture %pStoreAdd, <4 x i32> %xIn, <4 x i32> %yIn, <4 x i32> %zIn, <4 x i32> %wIn) nounwind


;CHECK-AVX: vpunpckldq	[[XMM0:%xmm[0-9]+]], [[XMM2:%xmm[0-9]+]], [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM1:%xmm[0-9]+]], [[XMM3:%xmm[0-9]+]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM4]], [[XMM5]], [[XMM7:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM7]], ([[RCX:%[a-z]+]])
;CHECK-AVX:	vpunpckhdq	[[XMM4]], [[XMM5]], [[XMM8:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM8]], 16([[RCX]])
;CHECK-AVX:	vpunpckhdq	[[XMM0]], [[XMM2]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM1]], [[XMM3]], [[XMM6:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM5]], [[XMM6]], [[XMM9:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM9]], 32([[RCX]])
;CHECK-AVX:	vpunpckhdq	[[XMM5]], [[XMM6]], [[XMM10:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM10]], 48([[RCX]])

;CHECK-AVX2:	vinserti128	$1, [[XMM3:%xmm[0-9]+]], [[YMM20:%ymm[0-9]+]], [[YMM2:%ymm[0-9]+]]
;CHECK-AVX2:	vpermd	[[YMM2]], [[YMM3:%ymm[0-9]+]], [[YMM21:%ymm[0-9]+]]
;CHECK-AVX2:	vinserti128	$1, [[XMM1:%xmm[0-9]+]], [[YMM00:%ymm[0-9]+]], [[YMM0:%ymm[0-9]+]]
;CHECK-AVX2:	vpermd	[[YMM0]], [[YMM1:%ymm[0-9]+]], [[YMM01:%ymm[0-9]+]]
;CHECK-AVX2:	vpblendd	$204, [[YMM21]], [[YMM01]], [[YMM11:%ymm[0-9]+]]
;CHECK-AVX2:	vmovdqu	[[YMM11]], ([[EAX:%[a-z]+]])
;CHECK-AVX2:	vpalignr	$8, [[YMM01]], [[YMM21]], [[YMM02:%ymm[0-9]+]]
;CHECK-AVX2:	vmovdqu	[[YMM02]], 32([[EAX]])
