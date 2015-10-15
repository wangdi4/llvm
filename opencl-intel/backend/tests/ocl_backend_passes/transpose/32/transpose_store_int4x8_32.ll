;; [LLVM 3.6 UPGRADE] The following LLVM IR is translated into similar but different ASM sequence.
;; Don't see any point to test codegen here. Consider to remove the test or move it to LLVM if it is necessary.
; XFAIL: *

; XFAIL: x86
; RUN: oclopt -runtimelib=clbltfng9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t1.ll
; RUN: llc < %t1.ll -mattr=+avx -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX
; RUN: oclopt -runtimelib=clbltfns9.rtl  -builtin-import -builtin-call-to-inst  -instcombine -inline -scalarrepl -S %s -o %t2.ll
; RUN: llc < %t2.ll -mattr=+avx2 -mtriple=i686-pc-Win32 | FileCheck %s -check-prefix=CHECK-AVX2



define void @foo(<4 x i32>* nocapture %pStoreAdd, <8 x i32> %xIn, <8 x i32> %yIn, <8 x i32> %zIn, <8 x i32> %wIn)nounwind{
entry:
    %xIn.addr = alloca <8 x i32>, align 4
    store <8 x i32> %xIn, <8 x i32>* %xIn.addr, align 4
    %yIn.addr = alloca <8 x i32>, align 4
    store <8 x i32> %yIn, <8 x i32>* %yIn.addr, align 4
	%zIn.addr = alloca <8 x i32>, align 4
    store <8 x i32> %zIn, <8 x i32>* %zIn.addr, align 4
	%wIn.addr = alloca <8 x i32>, align 4
    store <8 x i32> %wIn, <8 x i32>* %wIn.addr, align 4
    call void @__ocl_transpose_store_int_4x8(<4 x i32>* nocapture %pStoreAdd, <8 x i32> %xIn, <8 x i32> %yIn, <8 x i32> %zIn, <8 x i32> %wIn) nounwind
    ret void
}

declare void @__ocl_transpose_store_int_4x8(<4 x i32>* nocapture %pStoreAdd, <8 x i32> %xIn, <8 x i32> %yIn, <8 x i32> %zIn, <8 x i32> %wIn) nounwind


;CHECK-AVX:	vpunpckldq	[[XMM3:%xmm[0-9]+]], [[XMM1:%xmm[0-9]+]], [[XMM6:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM2:%xmm[0-9]+]], [[XMM0:%xmm[0-9]+]], [[XMM4:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM6]], [[XMM4]], [[XMM00:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM00]], ([[EAX:%[a-z]+]])
;CHECK-AVX:	vpunpckhdq	[[XMM6]], [[XMM4]], [[XMM11:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM11]], 16([[EAX]])
;CHECK-AVX:	vpunpckhdq	[[XMM3]], [[XMM1]], [[XMM7:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM2]], [[XMM0]], [[XMM5:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM7]], [[XMM5]], [[XMM22:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM22]], 32([[EAX]])
;CHECK-AVX:	vpunpckhdq	[[XMM7]], [[XMM5]], [[XMM33:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM33]], 48([[EAX]])
;CHECK-AVX:	vextractf128	$1, [[YMM3:%ymm[0-9]+]], [[XMM3]]
;CHECK-AVX:	vextractf128	$1, [[YMM1:%ymm[0-9]+]], [[XMM1]]
;CHECK-AVX:	vpunpckldq	[[XMM3]], [[XMM1]], [[XMM8:%xmm[0-9]+]]
;CHECK-AVX:	vextractf128	$1, [[YMM2:%ymm[0-9]+]], [[XMM2]]
;CHECK-AVX:	vextractf128	$1, [[YMM0:%ymm[0-9]+]], [[XMM0]]
;CHECK-AVX:	vpunpckldq	[[XMM2]], [[XMM0]], [[XMM9:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM8]], [[XMM9]], [[XMM10:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM10]], 64([[EAX]])
;CHECK-AVX:	vpunpckhdq	[[XMM8]], [[XMM9]], [[XMM11:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM11]], 80([[EAX]])
;CHECK-AVX:	vpunpckhdq	[[XMM3]], [[XMM1]], [[XMM12:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckhdq	[[XMM2]], [[XMM0]], [[XMM13:%xmm[0-9]+]]
;CHECK-AVX:	vpunpckldq	[[XMM12]], [[XMM13]], [[XMM14:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM14]], 96([[EAX]])
;CHECK-AVX:	vpunpckhdq	[[XMM12]], [[XMM13]], [[XMM15:%xmm[0-9]+]]
;CHECK-AVX:	vmovdqa	[[XMM15]], 112([[EAX]])

;CHECK-AVX2:	vpunpckldq	[[YMM3:%ymm[0-9]+]], [[YMM1:%ymm[0-9]+]], [[YMM5:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckldq	[[YMM2:%ymm[0-9]+]], [[YMM0:%ymm[0-9]+]], [[YMM6:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckldq	[[YMM5]], [[YMM6]], [[YMM4:%ymm[0-9]+]]
;CHECK-AVX2:	vmovdqa	[[XMM4:%xmm[0-9]+]], ([[EAX:%[a-z]+]])
;CHECK-AVX2:	vpunpckhdq	[[YMM5]], [[YMM6]], [[YMM51:%ymm[0-9]+]]
;CHECK-AVX2:	vmovdqa	[[XMM5:%xmm[0-9]+]], 16([[EAX]])
;CHECK-AVX2:	vpunpckhdq	[[YMM3]], [[YMM1]], [[YMM11:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckhdq	[[YMM2]], [[YMM0]], [[YMM21:%ymm[0-9]+]]
;CHECK-AVX2:	vpunpckldq	[[YMM11]], [[YMM21]], [[YMM01:%ymm[0-9]+]]
;CHECK-AVX2:	vmovdqa	[[XMM0:%xmm[0-9]+]], 32([[EAX]])
;CHECK-AVX2:	vpunpckhdq	[[YMM11]], [[YMM21]], [[YMM12:%ymm[0-9]+]]
;CHECK-AVX2:	vmovdqa	%xmm1, 48([[EAX]])
;CHECK-AVX2:	vextracti128	$1, [[YMM4]], 64([[EAX]])
;CHECK-AVX2:	vextracti128	$1, [[YMM51]], 80([[EAX]])
;CHECK-AVX2:	vextracti128	$1, [[YMM01]], 96([[EAX]])
;CHECK-AVX2:	vextracti128	$1, [[YMM12]], 112([[EAX]])
