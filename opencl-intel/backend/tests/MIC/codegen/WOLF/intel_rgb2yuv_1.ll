; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__intel_rgb2yuv_scalar_original(<4 x float> addrspace(1)* nocapture, <4 x float> addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare void @__intel_rgb2yuv_vector_original(<4 x float> addrspace(1)* nocapture, <4 x float> addrspace(1)* nocapture, i32) nounwind

define float @_Z3dotDv4_fS_(<4 x float> %x, <4 x float> %y) nounwind readnone {
entry:
  %tmp2 = shufflevector <4 x float> %x, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp3 = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp2, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %tmp6 = shufflevector <4 x float> %y, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp7 = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp6, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %0 = tail call <16 x float> @llvm.x86.mic.mul.ps(<16 x float> %tmp3, <16 x float> %tmp7) nounwind
  %1 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %0, i32 1) nounwind
  %2 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %0, <16 x float> %1) nounwind
  %3 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %2, i32 2) nounwind
  %4 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %2, <16 x float> %3) nounwind
  %tmp25 = extractelement <16 x float> %4, i32 0
  ret float %tmp25
}

declare void @____Vectorized_.intel_rgb2yuv_scalar_original(<4 x float> addrspace(1)* nocapture, <4 x float> addrspace(1)* nocapture, i32) nounwind

declare void @____Vectorized_.intel_rgb2yuv_vector_original(<4 x float> addrspace(1)* nocapture, <4 x float> addrspace(1)* nocapture, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__intel_rgb2yuv_scalar_separated_args(<4 x float> addrspace(1)* nocapture %pSrc, <4 x float> addrspace(1)* nocapture %pDst, i32 %PixelNum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %PixelNum, 0
  %tmp = zext i32 %PixelNum to i64
  %tmp15 = sext i32 %PixelNum to i64
  br label %SyncBB21

SyncBB21:                                         ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB21
  %1 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %1, align 8
  %4 = load i64* %2, align 8
  %5 = add i64 %4, %3
  %tmp16 = mul i64 %5, %tmp15
  %sext = shl i64 %tmp16, 32
  %tmp18 = ashr i64 %sext, 32
  br label %6

; <label>:6                                       ; preds = %6, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %6 ]
  %tmp19 = add i64 %tmp18, %indvar
  %scevgep = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %tmp19
  %scevgep20 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %tmp19
  %7 = load <4 x float> addrspace(1)* %scevgep, align 16
  %8 = extractelement <4 x float> %7, i32 0
  %9 = fmul float %8, 0x3FD322D0E0000000
  %10 = extractelement <4 x float> %7, i32 1
  %11 = fmul float %10, 0x3FE2C8B440000000
  %12 = fadd float %9, %11
  %13 = extractelement <4 x float> %7, i32 2
  %14 = fmul float %13, 0x3FBD2F1AA0000000
  %15 = fadd float %12, %14
  %16 = load <4 x float> addrspace(1)* %scevgep20, align 16
  %17 = insertelement <4 x float> %16, float %15, i32 0
  store <4 x float> %17, <4 x float> addrspace(1)* %scevgep20, align 16
  %18 = load <4 x float> addrspace(1)* %scevgep, align 16
  %19 = extractelement <4 x float> %18, i32 0
  %20 = fmul float %19, 0xBFC2D0E560000000
  %21 = extractelement <4 x float> %18, i32 1
  %22 = fmul float %21, 0x3FD27EF9E0000000
  %23 = fsub float %20, %22
  %24 = extractelement <4 x float> %18, i32 2
  %25 = fmul float %24, 0x3FDBE76C80000000
  %26 = fadd float %23, %25
  %27 = insertelement <4 x float> %17, float %26, i32 1
  store <4 x float> %27, <4 x float> addrspace(1)* %scevgep20, align 16
  %28 = load <4 x float> addrspace(1)* %scevgep, align 16
  %29 = extractelement <4 x float> %28, i32 0
  %30 = fmul float %29, 0x3FE3AE1480000000
  %31 = extractelement <4 x float> %28, i32 1
  %32 = fmul float %31, 0x3FE07AE140000000
  %33 = fsub float %30, %32
  %34 = extractelement <4 x float> %28, i32 2
  %35 = fmul float %34, 0x3FB99999A0000000
  %36 = fsub float %33, %35
  %37 = insertelement <4 x float> %27, float %36, i32 2
  store <4 x float> %37, <4 x float> addrspace(1)* %scevgep20, align 16
  %38 = load <4 x float> addrspace(1)* %scevgep, align 16
  %39 = shufflevector <4 x float> %37, <4 x float> %38, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %39, <4 x float> addrspace(1)* %scevgep20, align 16
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp
  br i1 %exitcond, label %._crit_edge, label %6

._crit_edge:                                      ; preds = %6, %SyncBB21
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB21

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @__intel_rgb2yuv_vector_separated_args(<4 x float> addrspace(1)* nocapture %pSrc, <4 x float> addrspace(1)* nocapture %pDst, i32 %PixelNum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %PixelNum, 0
  %tmp = zext i32 %PixelNum to i64
  %tmp9 = sext i32 %PixelNum to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %1 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %1, align 8
  %4 = load i64* %2, align 8
  %5 = add i64 %4, %3
  %tmp10 = mul i64 %5, %tmp9
  %sext = shl i64 %tmp10, 32
  %tmp12 = ashr i64 %sext, 32
  br label %6

; <label>:6                                       ; preds = %6, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %6 ]
  %tmp13 = add i64 %tmp12, %indvar
  %scevgep = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %tmp13
  %scevgep14 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %tmp13
  %7 = load <4 x float> addrspace(1)* %scevgep, align 16
  %tmp6.i = shufflevector <4 x float> %7, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp7.i = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp6.i, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %8 = tail call <16 x float> @llvm.x86.mic.mul.ps(<16 x float> <float 0x3FD322D0E0000000, float 0x3FE2C8B440000000, float 0x3FBD2F1AA0000000, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp7.i) nounwind
  %9 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %8, i32 1) nounwind
  %10 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %8, <16 x float> %9) nounwind
  %11 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %10, i32 2) nounwind
  %12 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %10, <16 x float> %11) nounwind
  %tmp25.i = extractelement <16 x float> %12, i32 0
  %13 = load <4 x float> addrspace(1)* %scevgep14, align 16
  %14 = insertelement <4 x float> %13, float %tmp25.i, i32 0
  store <4 x float> %14, <4 x float> addrspace(1)* %scevgep14, align 16
  %15 = load <4 x float> addrspace(1)* %scevgep, align 16
  %tmp6.i1 = shufflevector <4 x float> %15, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp7.i2 = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp6.i1, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %16 = tail call <16 x float> @llvm.x86.mic.mul.ps(<16 x float> <float 0xBFC2D0E560000000, float 0xBFD27EF9E0000000, float 0x3FDBE76C80000000, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp7.i2) nounwind
  %17 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %16, i32 1) nounwind
  %18 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %16, <16 x float> %17) nounwind
  %19 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %18, i32 2) nounwind
  %20 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %18, <16 x float> %19) nounwind
  %tmp25.i3 = extractelement <16 x float> %20, i32 0
  %21 = load <4 x float> addrspace(1)* %scevgep14, align 16
  %22 = insertelement <4 x float> %21, float %tmp25.i3, i32 1
  store <4 x float> %22, <4 x float> addrspace(1)* %scevgep14, align 16
  %23 = load <4 x float> addrspace(1)* %scevgep, align 16
  %tmp6.i4 = shufflevector <4 x float> %23, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp7.i5 = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp6.i4, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %24 = tail call <16 x float> @llvm.x86.mic.mul.ps(<16 x float> <float 0x3FE3AE1480000000, float 0xBFE07AE140000000, float 0xBFB99999A0000000, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp7.i5) nounwind
  %25 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %24, i32 1) nounwind
  %26 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %24, <16 x float> %25) nounwind
  %27 = tail call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %26, i32 2) nounwind
  %28 = tail call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %26, <16 x float> %27) nounwind
  %tmp25.i6 = extractelement <16 x float> %28, i32 0
  %29 = load <4 x float> addrspace(1)* %scevgep14, align 16
  %30 = insertelement <4 x float> %29, float %tmp25.i6, i32 2
  store <4 x float> %30, <4 x float> addrspace(1)* %scevgep14, align 16
  %31 = load <4 x float> addrspace(1)* %scevgep, align 16
  %32 = shufflevector <4 x float> %30, <4 x float> %31, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %32, <4 x float> addrspace(1)* %scevgep14, align 16
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp
  br i1 %exitcond, label %._crit_edge, label %6

._crit_edge:                                      ; preds = %6, %SyncBB
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB15

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB15:                                         ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.intel_rgb2yuv_scalar_separated_args(<4 x float> addrspace(1)* nocapture %pSrc, <4 x float> addrspace(1)* nocapture %pDst, i32 %PixelNum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %PixelNum, 0
  %tmp = zext i32 %PixelNum to i64
  %tmp15 = sext i32 %PixelNum to i64
  %temp = insertelement <16 x i64> undef, i64 %tmp15, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB248

SyncBB248:                                        ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB248
  %1 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %1, align 8
  %4 = load i64* %2, align 8
  %5 = add i64 %4, %3
  %broadcast1 = insertelement <16 x i64> undef, i64 %5, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %6 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %tmp1635 = mul <16 x i64> %6, %vector
  %sext36 = shl <16 x i64> %tmp1635, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %tmp1837 = ashr <16 x i64> %sext36, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  br label %7

; <label>:7                                       ; preds = %7, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %7 ]
  %temp38 = insertelement <16 x i64> undef, i64 %indvar, i32 0
  %vector39 = shufflevector <16 x i64> %temp38, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1940 = add <16 x i64> %tmp1837, %vector39
  %extract = extractelement <16 x i64> %tmp1940, i32 0
  %extract41 = extractelement <16 x i64> %tmp1940, i32 1
  %extract42 = extractelement <16 x i64> %tmp1940, i32 2
  %extract43 = extractelement <16 x i64> %tmp1940, i32 3
  %extract44 = extractelement <16 x i64> %tmp1940, i32 4
  %extract45 = extractelement <16 x i64> %tmp1940, i32 5
  %extract46 = extractelement <16 x i64> %tmp1940, i32 6
  %extract47 = extractelement <16 x i64> %tmp1940, i32 7
  %extract48 = extractelement <16 x i64> %tmp1940, i32 8
  %extract49 = extractelement <16 x i64> %tmp1940, i32 9
  %extract50 = extractelement <16 x i64> %tmp1940, i32 10
  %extract51 = extractelement <16 x i64> %tmp1940, i32 11
  %extract52 = extractelement <16 x i64> %tmp1940, i32 12
  %extract53 = extractelement <16 x i64> %tmp1940, i32 13
  %extract54 = extractelement <16 x i64> %tmp1940, i32 14
  %extract55 = extractelement <16 x i64> %tmp1940, i32 15
  %8 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract
  %9 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract41
  %10 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract42
  %11 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract43
  %12 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract44
  %13 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract45
  %14 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract46
  %15 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract47
  %16 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract48
  %17 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract49
  %18 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract50
  %19 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract51
  %20 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract52
  %21 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract53
  %22 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract54
  %23 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract55
  %24 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract
  %25 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract41
  %26 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract42
  %27 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract43
  %28 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract44
  %29 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract45
  %30 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract46
  %31 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract47
  %32 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract48
  %33 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract49
  %34 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract50
  %35 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract51
  %36 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract52
  %37 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract53
  %38 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract54
  %39 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract55
  %40 = load <4 x float> addrspace(1)* %8, align 16
  %41 = load <4 x float> addrspace(1)* %9, align 16
  %42 = load <4 x float> addrspace(1)* %10, align 16
  %43 = load <4 x float> addrspace(1)* %11, align 16
  %44 = load <4 x float> addrspace(1)* %12, align 16
  %45 = load <4 x float> addrspace(1)* %13, align 16
  %46 = load <4 x float> addrspace(1)* %14, align 16
  %47 = load <4 x float> addrspace(1)* %15, align 16
  %48 = load <4 x float> addrspace(1)* %16, align 16
  %49 = load <4 x float> addrspace(1)* %17, align 16
  %50 = load <4 x float> addrspace(1)* %18, align 16
  %51 = load <4 x float> addrspace(1)* %19, align 16
  %52 = load <4 x float> addrspace(1)* %20, align 16
  %53 = load <4 x float> addrspace(1)* %21, align 16
  %54 = load <4 x float> addrspace(1)* %22, align 16
  %55 = load <4 x float> addrspace(1)* %23, align 16
  %56 = extractelement <4 x float> %40, i32 0
  %57 = extractelement <4 x float> %41, i32 0
  %58 = extractelement <4 x float> %42, i32 0
  %59 = extractelement <4 x float> %43, i32 0
  %60 = extractelement <4 x float> %44, i32 0
  %61 = extractelement <4 x float> %45, i32 0
  %62 = extractelement <4 x float> %46, i32 0
  %63 = extractelement <4 x float> %47, i32 0
  %64 = extractelement <4 x float> %48, i32 0
  %65 = extractelement <4 x float> %49, i32 0
  %66 = extractelement <4 x float> %50, i32 0
  %67 = extractelement <4 x float> %51, i32 0
  %68 = extractelement <4 x float> %52, i32 0
  %69 = extractelement <4 x float> %53, i32 0
  %70 = extractelement <4 x float> %54, i32 0
  %71 = extractelement <4 x float> %55, i32 0
  %temp.vect56 = insertelement <16 x float> undef, float %56, i32 0
  %temp.vect57 = insertelement <16 x float> %temp.vect56, float %57, i32 1
  %temp.vect58 = insertelement <16 x float> %temp.vect57, float %58, i32 2
  %temp.vect59 = insertelement <16 x float> %temp.vect58, float %59, i32 3
  %temp.vect60 = insertelement <16 x float> %temp.vect59, float %60, i32 4
  %temp.vect61 = insertelement <16 x float> %temp.vect60, float %61, i32 5
  %temp.vect62 = insertelement <16 x float> %temp.vect61, float %62, i32 6
  %temp.vect63 = insertelement <16 x float> %temp.vect62, float %63, i32 7
  %temp.vect64 = insertelement <16 x float> %temp.vect63, float %64, i32 8
  %temp.vect65 = insertelement <16 x float> %temp.vect64, float %65, i32 9
  %temp.vect66 = insertelement <16 x float> %temp.vect65, float %66, i32 10
  %temp.vect67 = insertelement <16 x float> %temp.vect66, float %67, i32 11
  %temp.vect68 = insertelement <16 x float> %temp.vect67, float %68, i32 12
  %temp.vect69 = insertelement <16 x float> %temp.vect68, float %69, i32 13
  %temp.vect70 = insertelement <16 x float> %temp.vect69, float %70, i32 14
  %temp.vect71 = insertelement <16 x float> %temp.vect70, float %71, i32 15
  %72 = extractelement <4 x float> %40, i32 1
  %73 = extractelement <4 x float> %41, i32 1
  %74 = extractelement <4 x float> %42, i32 1
  %75 = extractelement <4 x float> %43, i32 1
  %76 = extractelement <4 x float> %44, i32 1
  %77 = extractelement <4 x float> %45, i32 1
  %78 = extractelement <4 x float> %46, i32 1
  %79 = extractelement <4 x float> %47, i32 1
  %80 = extractelement <4 x float> %48, i32 1
  %81 = extractelement <4 x float> %49, i32 1
  %82 = extractelement <4 x float> %50, i32 1
  %83 = extractelement <4 x float> %51, i32 1
  %84 = extractelement <4 x float> %52, i32 1
  %85 = extractelement <4 x float> %53, i32 1
  %86 = extractelement <4 x float> %54, i32 1
  %87 = extractelement <4 x float> %55, i32 1
  %temp.vect72 = insertelement <16 x float> undef, float %72, i32 0
  %temp.vect73 = insertelement <16 x float> %temp.vect72, float %73, i32 1
  %temp.vect74 = insertelement <16 x float> %temp.vect73, float %74, i32 2
  %temp.vect75 = insertelement <16 x float> %temp.vect74, float %75, i32 3
  %temp.vect76 = insertelement <16 x float> %temp.vect75, float %76, i32 4
  %temp.vect77 = insertelement <16 x float> %temp.vect76, float %77, i32 5
  %temp.vect78 = insertelement <16 x float> %temp.vect77, float %78, i32 6
  %temp.vect79 = insertelement <16 x float> %temp.vect78, float %79, i32 7
  %temp.vect80 = insertelement <16 x float> %temp.vect79, float %80, i32 8
  %temp.vect81 = insertelement <16 x float> %temp.vect80, float %81, i32 9
  %temp.vect82 = insertelement <16 x float> %temp.vect81, float %82, i32 10
  %temp.vect83 = insertelement <16 x float> %temp.vect82, float %83, i32 11
  %temp.vect84 = insertelement <16 x float> %temp.vect83, float %84, i32 12
  %temp.vect85 = insertelement <16 x float> %temp.vect84, float %85, i32 13
  %temp.vect86 = insertelement <16 x float> %temp.vect85, float %86, i32 14
  %temp.vect87 = insertelement <16 x float> %temp.vect86, float %87, i32 15
  %88 = extractelement <4 x float> %40, i32 2
  %89 = extractelement <4 x float> %41, i32 2
  %90 = extractelement <4 x float> %42, i32 2
  %91 = extractelement <4 x float> %43, i32 2
  %92 = extractelement <4 x float> %44, i32 2
  %93 = extractelement <4 x float> %45, i32 2
  %94 = extractelement <4 x float> %46, i32 2
  %95 = extractelement <4 x float> %47, i32 2
  %96 = extractelement <4 x float> %48, i32 2
  %97 = extractelement <4 x float> %49, i32 2
  %98 = extractelement <4 x float> %50, i32 2
  %99 = extractelement <4 x float> %51, i32 2
  %100 = extractelement <4 x float> %52, i32 2
  %101 = extractelement <4 x float> %53, i32 2
  %102 = extractelement <4 x float> %54, i32 2
  %103 = extractelement <4 x float> %55, i32 2
  %temp.vect88 = insertelement <16 x float> undef, float %88, i32 0
  %temp.vect89 = insertelement <16 x float> %temp.vect88, float %89, i32 1
  %temp.vect90 = insertelement <16 x float> %temp.vect89, float %90, i32 2
  %temp.vect91 = insertelement <16 x float> %temp.vect90, float %91, i32 3
  %temp.vect92 = insertelement <16 x float> %temp.vect91, float %92, i32 4
  %temp.vect93 = insertelement <16 x float> %temp.vect92, float %93, i32 5
  %temp.vect94 = insertelement <16 x float> %temp.vect93, float %94, i32 6
  %temp.vect95 = insertelement <16 x float> %temp.vect94, float %95, i32 7
  %temp.vect96 = insertelement <16 x float> %temp.vect95, float %96, i32 8
  %temp.vect97 = insertelement <16 x float> %temp.vect96, float %97, i32 9
  %temp.vect98 = insertelement <16 x float> %temp.vect97, float %98, i32 10
  %temp.vect99 = insertelement <16 x float> %temp.vect98, float %99, i32 11
  %temp.vect100 = insertelement <16 x float> %temp.vect99, float %100, i32 12
  %temp.vect101 = insertelement <16 x float> %temp.vect100, float %101, i32 13
  %temp.vect102 = insertelement <16 x float> %temp.vect101, float %102, i32 14
  %temp.vect103 = insertelement <16 x float> %temp.vect102, float %103, i32 15
  %104 = fmul <16 x float> %temp.vect71, <float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000>
  %105 = fmul <16 x float> %temp.vect87, <float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000>
  %106 = fadd <16 x float> %104, %105
  %107 = fmul <16 x float> %temp.vect103, <float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000>
  %108 = fadd <16 x float> %106, %107
  %extract104 = extractelement <16 x float> %108, i32 0
  %extract105 = extractelement <16 x float> %108, i32 1
  %extract106 = extractelement <16 x float> %108, i32 2
  %extract107 = extractelement <16 x float> %108, i32 3
  %extract108 = extractelement <16 x float> %108, i32 4
  %extract109 = extractelement <16 x float> %108, i32 5
  %extract110 = extractelement <16 x float> %108, i32 6
  %extract111 = extractelement <16 x float> %108, i32 7
  %extract112 = extractelement <16 x float> %108, i32 8
  %extract113 = extractelement <16 x float> %108, i32 9
  %extract114 = extractelement <16 x float> %108, i32 10
  %extract115 = extractelement <16 x float> %108, i32 11
  %extract116 = extractelement <16 x float> %108, i32 12
  %extract117 = extractelement <16 x float> %108, i32 13
  %extract118 = extractelement <16 x float> %108, i32 14
  %extract119 = extractelement <16 x float> %108, i32 15
  %109 = load <4 x float> addrspace(1)* %24, align 16
  %110 = load <4 x float> addrspace(1)* %25, align 16
  %111 = load <4 x float> addrspace(1)* %26, align 16
  %112 = load <4 x float> addrspace(1)* %27, align 16
  %113 = load <4 x float> addrspace(1)* %28, align 16
  %114 = load <4 x float> addrspace(1)* %29, align 16
  %115 = load <4 x float> addrspace(1)* %30, align 16
  %116 = load <4 x float> addrspace(1)* %31, align 16
  %117 = load <4 x float> addrspace(1)* %32, align 16
  %118 = load <4 x float> addrspace(1)* %33, align 16
  %119 = load <4 x float> addrspace(1)* %34, align 16
  %120 = load <4 x float> addrspace(1)* %35, align 16
  %121 = load <4 x float> addrspace(1)* %36, align 16
  %122 = load <4 x float> addrspace(1)* %37, align 16
  %123 = load <4 x float> addrspace(1)* %38, align 16
  %124 = load <4 x float> addrspace(1)* %39, align 16
  %125 = insertelement <4 x float> undef, float %extract104, i32 0
  %126 = insertelement <4 x float> undef, float %extract105, i32 0
  %127 = insertelement <4 x float> undef, float %extract106, i32 0
  %128 = insertelement <4 x float> undef, float %extract107, i32 0
  %129 = insertelement <4 x float> undef, float %extract108, i32 0
  %130 = insertelement <4 x float> undef, float %extract109, i32 0
  %131 = insertelement <4 x float> undef, float %extract110, i32 0
  %132 = insertelement <4 x float> undef, float %extract111, i32 0
  %133 = insertelement <4 x float> undef, float %extract112, i32 0
  %134 = insertelement <4 x float> undef, float %extract113, i32 0
  %135 = insertelement <4 x float> undef, float %extract114, i32 0
  %136 = insertelement <4 x float> undef, float %extract115, i32 0
  %137 = insertelement <4 x float> undef, float %extract116, i32 0
  %138 = insertelement <4 x float> undef, float %extract117, i32 0
  %139 = insertelement <4 x float> undef, float %extract118, i32 0
  %140 = insertelement <4 x float> undef, float %extract119, i32 0
  %141 = shufflevector <4 x float> %125, <4 x float> %109, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %142 = shufflevector <4 x float> %126, <4 x float> %110, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %143 = shufflevector <4 x float> %127, <4 x float> %111, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %144 = shufflevector <4 x float> %128, <4 x float> %112, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %145 = shufflevector <4 x float> %129, <4 x float> %113, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %146 = shufflevector <4 x float> %130, <4 x float> %114, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %147 = shufflevector <4 x float> %131, <4 x float> %115, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %148 = shufflevector <4 x float> %132, <4 x float> %116, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %149 = shufflevector <4 x float> %133, <4 x float> %117, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %150 = shufflevector <4 x float> %134, <4 x float> %118, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %151 = shufflevector <4 x float> %135, <4 x float> %119, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %152 = shufflevector <4 x float> %136, <4 x float> %120, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %153 = shufflevector <4 x float> %137, <4 x float> %121, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %154 = shufflevector <4 x float> %138, <4 x float> %122, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %155 = shufflevector <4 x float> %139, <4 x float> %123, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %156 = shufflevector <4 x float> %140, <4 x float> %124, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  store <4 x float> %141, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %142, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %143, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %144, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %145, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %146, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %147, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %148, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %149, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %150, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %151, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %152, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %153, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %154, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %155, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %156, <4 x float> addrspace(1)* %39, align 16
  %157 = load <4 x float> addrspace(1)* %8, align 16
  %158 = load <4 x float> addrspace(1)* %9, align 16
  %159 = load <4 x float> addrspace(1)* %10, align 16
  %160 = load <4 x float> addrspace(1)* %11, align 16
  %161 = load <4 x float> addrspace(1)* %12, align 16
  %162 = load <4 x float> addrspace(1)* %13, align 16
  %163 = load <4 x float> addrspace(1)* %14, align 16
  %164 = load <4 x float> addrspace(1)* %15, align 16
  %165 = load <4 x float> addrspace(1)* %16, align 16
  %166 = load <4 x float> addrspace(1)* %17, align 16
  %167 = load <4 x float> addrspace(1)* %18, align 16
  %168 = load <4 x float> addrspace(1)* %19, align 16
  %169 = load <4 x float> addrspace(1)* %20, align 16
  %170 = load <4 x float> addrspace(1)* %21, align 16
  %171 = load <4 x float> addrspace(1)* %22, align 16
  %172 = load <4 x float> addrspace(1)* %23, align 16
  %173 = extractelement <4 x float> %157, i32 0
  %174 = extractelement <4 x float> %158, i32 0
  %175 = extractelement <4 x float> %159, i32 0
  %176 = extractelement <4 x float> %160, i32 0
  %177 = extractelement <4 x float> %161, i32 0
  %178 = extractelement <4 x float> %162, i32 0
  %179 = extractelement <4 x float> %163, i32 0
  %180 = extractelement <4 x float> %164, i32 0
  %181 = extractelement <4 x float> %165, i32 0
  %182 = extractelement <4 x float> %166, i32 0
  %183 = extractelement <4 x float> %167, i32 0
  %184 = extractelement <4 x float> %168, i32 0
  %185 = extractelement <4 x float> %169, i32 0
  %186 = extractelement <4 x float> %170, i32 0
  %187 = extractelement <4 x float> %171, i32 0
  %188 = extractelement <4 x float> %172, i32 0
  %temp.vect120 = insertelement <16 x float> undef, float %173, i32 0
  %temp.vect121 = insertelement <16 x float> %temp.vect120, float %174, i32 1
  %temp.vect122 = insertelement <16 x float> %temp.vect121, float %175, i32 2
  %temp.vect123 = insertelement <16 x float> %temp.vect122, float %176, i32 3
  %temp.vect124 = insertelement <16 x float> %temp.vect123, float %177, i32 4
  %temp.vect125 = insertelement <16 x float> %temp.vect124, float %178, i32 5
  %temp.vect126 = insertelement <16 x float> %temp.vect125, float %179, i32 6
  %temp.vect127 = insertelement <16 x float> %temp.vect126, float %180, i32 7
  %temp.vect128 = insertelement <16 x float> %temp.vect127, float %181, i32 8
  %temp.vect129 = insertelement <16 x float> %temp.vect128, float %182, i32 9
  %temp.vect130 = insertelement <16 x float> %temp.vect129, float %183, i32 10
  %temp.vect131 = insertelement <16 x float> %temp.vect130, float %184, i32 11
  %temp.vect132 = insertelement <16 x float> %temp.vect131, float %185, i32 12
  %temp.vect133 = insertelement <16 x float> %temp.vect132, float %186, i32 13
  %temp.vect134 = insertelement <16 x float> %temp.vect133, float %187, i32 14
  %temp.vect135 = insertelement <16 x float> %temp.vect134, float %188, i32 15
  %189 = extractelement <4 x float> %157, i32 1
  %190 = extractelement <4 x float> %158, i32 1
  %191 = extractelement <4 x float> %159, i32 1
  %192 = extractelement <4 x float> %160, i32 1
  %193 = extractelement <4 x float> %161, i32 1
  %194 = extractelement <4 x float> %162, i32 1
  %195 = extractelement <4 x float> %163, i32 1
  %196 = extractelement <4 x float> %164, i32 1
  %197 = extractelement <4 x float> %165, i32 1
  %198 = extractelement <4 x float> %166, i32 1
  %199 = extractelement <4 x float> %167, i32 1
  %200 = extractelement <4 x float> %168, i32 1
  %201 = extractelement <4 x float> %169, i32 1
  %202 = extractelement <4 x float> %170, i32 1
  %203 = extractelement <4 x float> %171, i32 1
  %204 = extractelement <4 x float> %172, i32 1
  %temp.vect136 = insertelement <16 x float> undef, float %189, i32 0
  %temp.vect137 = insertelement <16 x float> %temp.vect136, float %190, i32 1
  %temp.vect138 = insertelement <16 x float> %temp.vect137, float %191, i32 2
  %temp.vect139 = insertelement <16 x float> %temp.vect138, float %192, i32 3
  %temp.vect140 = insertelement <16 x float> %temp.vect139, float %193, i32 4
  %temp.vect141 = insertelement <16 x float> %temp.vect140, float %194, i32 5
  %temp.vect142 = insertelement <16 x float> %temp.vect141, float %195, i32 6
  %temp.vect143 = insertelement <16 x float> %temp.vect142, float %196, i32 7
  %temp.vect144 = insertelement <16 x float> %temp.vect143, float %197, i32 8
  %temp.vect145 = insertelement <16 x float> %temp.vect144, float %198, i32 9
  %temp.vect146 = insertelement <16 x float> %temp.vect145, float %199, i32 10
  %temp.vect147 = insertelement <16 x float> %temp.vect146, float %200, i32 11
  %temp.vect148 = insertelement <16 x float> %temp.vect147, float %201, i32 12
  %temp.vect149 = insertelement <16 x float> %temp.vect148, float %202, i32 13
  %temp.vect150 = insertelement <16 x float> %temp.vect149, float %203, i32 14
  %temp.vect151 = insertelement <16 x float> %temp.vect150, float %204, i32 15
  %205 = extractelement <4 x float> %157, i32 2
  %206 = extractelement <4 x float> %158, i32 2
  %207 = extractelement <4 x float> %159, i32 2
  %208 = extractelement <4 x float> %160, i32 2
  %209 = extractelement <4 x float> %161, i32 2
  %210 = extractelement <4 x float> %162, i32 2
  %211 = extractelement <4 x float> %163, i32 2
  %212 = extractelement <4 x float> %164, i32 2
  %213 = extractelement <4 x float> %165, i32 2
  %214 = extractelement <4 x float> %166, i32 2
  %215 = extractelement <4 x float> %167, i32 2
  %216 = extractelement <4 x float> %168, i32 2
  %217 = extractelement <4 x float> %169, i32 2
  %218 = extractelement <4 x float> %170, i32 2
  %219 = extractelement <4 x float> %171, i32 2
  %220 = extractelement <4 x float> %172, i32 2
  %temp.vect152 = insertelement <16 x float> undef, float %205, i32 0
  %temp.vect153 = insertelement <16 x float> %temp.vect152, float %206, i32 1
  %temp.vect154 = insertelement <16 x float> %temp.vect153, float %207, i32 2
  %temp.vect155 = insertelement <16 x float> %temp.vect154, float %208, i32 3
  %temp.vect156 = insertelement <16 x float> %temp.vect155, float %209, i32 4
  %temp.vect157 = insertelement <16 x float> %temp.vect156, float %210, i32 5
  %temp.vect158 = insertelement <16 x float> %temp.vect157, float %211, i32 6
  %temp.vect159 = insertelement <16 x float> %temp.vect158, float %212, i32 7
  %temp.vect160 = insertelement <16 x float> %temp.vect159, float %213, i32 8
  %temp.vect161 = insertelement <16 x float> %temp.vect160, float %214, i32 9
  %temp.vect162 = insertelement <16 x float> %temp.vect161, float %215, i32 10
  %temp.vect163 = insertelement <16 x float> %temp.vect162, float %216, i32 11
  %temp.vect164 = insertelement <16 x float> %temp.vect163, float %217, i32 12
  %temp.vect165 = insertelement <16 x float> %temp.vect164, float %218, i32 13
  %temp.vect166 = insertelement <16 x float> %temp.vect165, float %219, i32 14
  %temp.vect167 = insertelement <16 x float> %temp.vect166, float %220, i32 15
  %221 = fmul <16 x float> %temp.vect135, <float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000>
  %222 = fmul <16 x float> %temp.vect151, <float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000>
  %223 = fsub <16 x float> %221, %222
  %224 = fmul <16 x float> %temp.vect167, <float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000>
  %225 = fadd <16 x float> %223, %224
  %extract168 = extractelement <16 x float> %225, i32 0
  %extract169 = extractelement <16 x float> %225, i32 1
  %extract170 = extractelement <16 x float> %225, i32 2
  %extract171 = extractelement <16 x float> %225, i32 3
  %extract172 = extractelement <16 x float> %225, i32 4
  %extract173 = extractelement <16 x float> %225, i32 5
  %extract174 = extractelement <16 x float> %225, i32 6
  %extract175 = extractelement <16 x float> %225, i32 7
  %extract176 = extractelement <16 x float> %225, i32 8
  %extract177 = extractelement <16 x float> %225, i32 9
  %extract178 = extractelement <16 x float> %225, i32 10
  %extract179 = extractelement <16 x float> %225, i32 11
  %extract180 = extractelement <16 x float> %225, i32 12
  %extract181 = extractelement <16 x float> %225, i32 13
  %extract182 = extractelement <16 x float> %225, i32 14
  %extract183 = extractelement <16 x float> %225, i32 15
  %226 = insertelement <4 x float> undef, float %extract104, i32 0
  %227 = insertelement <4 x float> undef, float %extract105, i32 0
  %228 = insertelement <4 x float> undef, float %extract106, i32 0
  %229 = insertelement <4 x float> undef, float %extract107, i32 0
  %230 = insertelement <4 x float> undef, float %extract108, i32 0
  %231 = insertelement <4 x float> undef, float %extract109, i32 0
  %232 = insertelement <4 x float> undef, float %extract110, i32 0
  %233 = insertelement <4 x float> undef, float %extract111, i32 0
  %234 = insertelement <4 x float> undef, float %extract112, i32 0
  %235 = insertelement <4 x float> undef, float %extract113, i32 0
  %236 = insertelement <4 x float> undef, float %extract114, i32 0
  %237 = insertelement <4 x float> undef, float %extract115, i32 0
  %238 = insertelement <4 x float> undef, float %extract116, i32 0
  %239 = insertelement <4 x float> undef, float %extract117, i32 0
  %240 = insertelement <4 x float> undef, float %extract118, i32 0
  %241 = insertelement <4 x float> undef, float %extract119, i32 0
  %242 = insertelement <4 x float> %226, float %extract168, i32 1
  %243 = insertelement <4 x float> %227, float %extract169, i32 1
  %244 = insertelement <4 x float> %228, float %extract170, i32 1
  %245 = insertelement <4 x float> %229, float %extract171, i32 1
  %246 = insertelement <4 x float> %230, float %extract172, i32 1
  %247 = insertelement <4 x float> %231, float %extract173, i32 1
  %248 = insertelement <4 x float> %232, float %extract174, i32 1
  %249 = insertelement <4 x float> %233, float %extract175, i32 1
  %250 = insertelement <4 x float> %234, float %extract176, i32 1
  %251 = insertelement <4 x float> %235, float %extract177, i32 1
  %252 = insertelement <4 x float> %236, float %extract178, i32 1
  %253 = insertelement <4 x float> %237, float %extract179, i32 1
  %254 = insertelement <4 x float> %238, float %extract180, i32 1
  %255 = insertelement <4 x float> %239, float %extract181, i32 1
  %256 = insertelement <4 x float> %240, float %extract182, i32 1
  %257 = insertelement <4 x float> %241, float %extract183, i32 1
  %258 = shufflevector <4 x float> %242, <4 x float> %109, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %259 = shufflevector <4 x float> %243, <4 x float> %110, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %260 = shufflevector <4 x float> %244, <4 x float> %111, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %261 = shufflevector <4 x float> %245, <4 x float> %112, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %262 = shufflevector <4 x float> %246, <4 x float> %113, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %263 = shufflevector <4 x float> %247, <4 x float> %114, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %264 = shufflevector <4 x float> %248, <4 x float> %115, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %265 = shufflevector <4 x float> %249, <4 x float> %116, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %266 = shufflevector <4 x float> %250, <4 x float> %117, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %267 = shufflevector <4 x float> %251, <4 x float> %118, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %268 = shufflevector <4 x float> %252, <4 x float> %119, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %269 = shufflevector <4 x float> %253, <4 x float> %120, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %270 = shufflevector <4 x float> %254, <4 x float> %121, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %271 = shufflevector <4 x float> %255, <4 x float> %122, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %272 = shufflevector <4 x float> %256, <4 x float> %123, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %273 = shufflevector <4 x float> %257, <4 x float> %124, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  store <4 x float> %258, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %259, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %260, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %261, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %262, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %263, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %264, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %265, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %266, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %267, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %268, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %269, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %270, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %271, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %272, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %273, <4 x float> addrspace(1)* %39, align 16
  %274 = load <4 x float> addrspace(1)* %8, align 16
  %275 = load <4 x float> addrspace(1)* %9, align 16
  %276 = load <4 x float> addrspace(1)* %10, align 16
  %277 = load <4 x float> addrspace(1)* %11, align 16
  %278 = load <4 x float> addrspace(1)* %12, align 16
  %279 = load <4 x float> addrspace(1)* %13, align 16
  %280 = load <4 x float> addrspace(1)* %14, align 16
  %281 = load <4 x float> addrspace(1)* %15, align 16
  %282 = load <4 x float> addrspace(1)* %16, align 16
  %283 = load <4 x float> addrspace(1)* %17, align 16
  %284 = load <4 x float> addrspace(1)* %18, align 16
  %285 = load <4 x float> addrspace(1)* %19, align 16
  %286 = load <4 x float> addrspace(1)* %20, align 16
  %287 = load <4 x float> addrspace(1)* %21, align 16
  %288 = load <4 x float> addrspace(1)* %22, align 16
  %289 = load <4 x float> addrspace(1)* %23, align 16
  %290 = extractelement <4 x float> %274, i32 0
  %291 = extractelement <4 x float> %275, i32 0
  %292 = extractelement <4 x float> %276, i32 0
  %293 = extractelement <4 x float> %277, i32 0
  %294 = extractelement <4 x float> %278, i32 0
  %295 = extractelement <4 x float> %279, i32 0
  %296 = extractelement <4 x float> %280, i32 0
  %297 = extractelement <4 x float> %281, i32 0
  %298 = extractelement <4 x float> %282, i32 0
  %299 = extractelement <4 x float> %283, i32 0
  %300 = extractelement <4 x float> %284, i32 0
  %301 = extractelement <4 x float> %285, i32 0
  %302 = extractelement <4 x float> %286, i32 0
  %303 = extractelement <4 x float> %287, i32 0
  %304 = extractelement <4 x float> %288, i32 0
  %305 = extractelement <4 x float> %289, i32 0
  %temp.vect184 = insertelement <16 x float> undef, float %290, i32 0
  %temp.vect185 = insertelement <16 x float> %temp.vect184, float %291, i32 1
  %temp.vect186 = insertelement <16 x float> %temp.vect185, float %292, i32 2
  %temp.vect187 = insertelement <16 x float> %temp.vect186, float %293, i32 3
  %temp.vect188 = insertelement <16 x float> %temp.vect187, float %294, i32 4
  %temp.vect189 = insertelement <16 x float> %temp.vect188, float %295, i32 5
  %temp.vect190 = insertelement <16 x float> %temp.vect189, float %296, i32 6
  %temp.vect191 = insertelement <16 x float> %temp.vect190, float %297, i32 7
  %temp.vect192 = insertelement <16 x float> %temp.vect191, float %298, i32 8
  %temp.vect193 = insertelement <16 x float> %temp.vect192, float %299, i32 9
  %temp.vect194 = insertelement <16 x float> %temp.vect193, float %300, i32 10
  %temp.vect195 = insertelement <16 x float> %temp.vect194, float %301, i32 11
  %temp.vect196 = insertelement <16 x float> %temp.vect195, float %302, i32 12
  %temp.vect197 = insertelement <16 x float> %temp.vect196, float %303, i32 13
  %temp.vect198 = insertelement <16 x float> %temp.vect197, float %304, i32 14
  %temp.vect199 = insertelement <16 x float> %temp.vect198, float %305, i32 15
  %306 = extractelement <4 x float> %274, i32 1
  %307 = extractelement <4 x float> %275, i32 1
  %308 = extractelement <4 x float> %276, i32 1
  %309 = extractelement <4 x float> %277, i32 1
  %310 = extractelement <4 x float> %278, i32 1
  %311 = extractelement <4 x float> %279, i32 1
  %312 = extractelement <4 x float> %280, i32 1
  %313 = extractelement <4 x float> %281, i32 1
  %314 = extractelement <4 x float> %282, i32 1
  %315 = extractelement <4 x float> %283, i32 1
  %316 = extractelement <4 x float> %284, i32 1
  %317 = extractelement <4 x float> %285, i32 1
  %318 = extractelement <4 x float> %286, i32 1
  %319 = extractelement <4 x float> %287, i32 1
  %320 = extractelement <4 x float> %288, i32 1
  %321 = extractelement <4 x float> %289, i32 1
  %temp.vect200 = insertelement <16 x float> undef, float %306, i32 0
  %temp.vect201 = insertelement <16 x float> %temp.vect200, float %307, i32 1
  %temp.vect202 = insertelement <16 x float> %temp.vect201, float %308, i32 2
  %temp.vect203 = insertelement <16 x float> %temp.vect202, float %309, i32 3
  %temp.vect204 = insertelement <16 x float> %temp.vect203, float %310, i32 4
  %temp.vect205 = insertelement <16 x float> %temp.vect204, float %311, i32 5
  %temp.vect206 = insertelement <16 x float> %temp.vect205, float %312, i32 6
  %temp.vect207 = insertelement <16 x float> %temp.vect206, float %313, i32 7
  %temp.vect208 = insertelement <16 x float> %temp.vect207, float %314, i32 8
  %temp.vect209 = insertelement <16 x float> %temp.vect208, float %315, i32 9
  %temp.vect210 = insertelement <16 x float> %temp.vect209, float %316, i32 10
  %temp.vect211 = insertelement <16 x float> %temp.vect210, float %317, i32 11
  %temp.vect212 = insertelement <16 x float> %temp.vect211, float %318, i32 12
  %temp.vect213 = insertelement <16 x float> %temp.vect212, float %319, i32 13
  %temp.vect214 = insertelement <16 x float> %temp.vect213, float %320, i32 14
  %temp.vect215 = insertelement <16 x float> %temp.vect214, float %321, i32 15
  %322 = extractelement <4 x float> %274, i32 2
  %323 = extractelement <4 x float> %275, i32 2
  %324 = extractelement <4 x float> %276, i32 2
  %325 = extractelement <4 x float> %277, i32 2
  %326 = extractelement <4 x float> %278, i32 2
  %327 = extractelement <4 x float> %279, i32 2
  %328 = extractelement <4 x float> %280, i32 2
  %329 = extractelement <4 x float> %281, i32 2
  %330 = extractelement <4 x float> %282, i32 2
  %331 = extractelement <4 x float> %283, i32 2
  %332 = extractelement <4 x float> %284, i32 2
  %333 = extractelement <4 x float> %285, i32 2
  %334 = extractelement <4 x float> %286, i32 2
  %335 = extractelement <4 x float> %287, i32 2
  %336 = extractelement <4 x float> %288, i32 2
  %337 = extractelement <4 x float> %289, i32 2
  %temp.vect216 = insertelement <16 x float> undef, float %322, i32 0
  %temp.vect217 = insertelement <16 x float> %temp.vect216, float %323, i32 1
  %temp.vect218 = insertelement <16 x float> %temp.vect217, float %324, i32 2
  %temp.vect219 = insertelement <16 x float> %temp.vect218, float %325, i32 3
  %temp.vect220 = insertelement <16 x float> %temp.vect219, float %326, i32 4
  %temp.vect221 = insertelement <16 x float> %temp.vect220, float %327, i32 5
  %temp.vect222 = insertelement <16 x float> %temp.vect221, float %328, i32 6
  %temp.vect223 = insertelement <16 x float> %temp.vect222, float %329, i32 7
  %temp.vect224 = insertelement <16 x float> %temp.vect223, float %330, i32 8
  %temp.vect225 = insertelement <16 x float> %temp.vect224, float %331, i32 9
  %temp.vect226 = insertelement <16 x float> %temp.vect225, float %332, i32 10
  %temp.vect227 = insertelement <16 x float> %temp.vect226, float %333, i32 11
  %temp.vect228 = insertelement <16 x float> %temp.vect227, float %334, i32 12
  %temp.vect229 = insertelement <16 x float> %temp.vect228, float %335, i32 13
  %temp.vect230 = insertelement <16 x float> %temp.vect229, float %336, i32 14
  %temp.vect231 = insertelement <16 x float> %temp.vect230, float %337, i32 15
  %338 = fmul <16 x float> %temp.vect199, <float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000>
  %339 = fmul <16 x float> %temp.vect215, <float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000>
  %340 = fsub <16 x float> %338, %339
  %341 = fmul <16 x float> %temp.vect231, <float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000>
  %342 = fsub <16 x float> %340, %341
  %extract232 = extractelement <16 x float> %342, i32 0
  %extract233 = extractelement <16 x float> %342, i32 1
  %extract234 = extractelement <16 x float> %342, i32 2
  %extract235 = extractelement <16 x float> %342, i32 3
  %extract236 = extractelement <16 x float> %342, i32 4
  %extract237 = extractelement <16 x float> %342, i32 5
  %extract238 = extractelement <16 x float> %342, i32 6
  %extract239 = extractelement <16 x float> %342, i32 7
  %extract240 = extractelement <16 x float> %342, i32 8
  %extract241 = extractelement <16 x float> %342, i32 9
  %extract242 = extractelement <16 x float> %342, i32 10
  %extract243 = extractelement <16 x float> %342, i32 11
  %extract244 = extractelement <16 x float> %342, i32 12
  %extract245 = extractelement <16 x float> %342, i32 13
  %extract246 = extractelement <16 x float> %342, i32 14
  %extract247 = extractelement <16 x float> %342, i32 15
  %343 = insertelement <4 x float> undef, float %extract104, i32 0
  %344 = insertelement <4 x float> undef, float %extract105, i32 0
  %345 = insertelement <4 x float> undef, float %extract106, i32 0
  %346 = insertelement <4 x float> undef, float %extract107, i32 0
  %347 = insertelement <4 x float> undef, float %extract108, i32 0
  %348 = insertelement <4 x float> undef, float %extract109, i32 0
  %349 = insertelement <4 x float> undef, float %extract110, i32 0
  %350 = insertelement <4 x float> undef, float %extract111, i32 0
  %351 = insertelement <4 x float> undef, float %extract112, i32 0
  %352 = insertelement <4 x float> undef, float %extract113, i32 0
  %353 = insertelement <4 x float> undef, float %extract114, i32 0
  %354 = insertelement <4 x float> undef, float %extract115, i32 0
  %355 = insertelement <4 x float> undef, float %extract116, i32 0
  %356 = insertelement <4 x float> undef, float %extract117, i32 0
  %357 = insertelement <4 x float> undef, float %extract118, i32 0
  %358 = insertelement <4 x float> undef, float %extract119, i32 0
  %359 = insertelement <4 x float> %343, float %extract168, i32 1
  %360 = insertelement <4 x float> %344, float %extract169, i32 1
  %361 = insertelement <4 x float> %345, float %extract170, i32 1
  %362 = insertelement <4 x float> %346, float %extract171, i32 1
  %363 = insertelement <4 x float> %347, float %extract172, i32 1
  %364 = insertelement <4 x float> %348, float %extract173, i32 1
  %365 = insertelement <4 x float> %349, float %extract174, i32 1
  %366 = insertelement <4 x float> %350, float %extract175, i32 1
  %367 = insertelement <4 x float> %351, float %extract176, i32 1
  %368 = insertelement <4 x float> %352, float %extract177, i32 1
  %369 = insertelement <4 x float> %353, float %extract178, i32 1
  %370 = insertelement <4 x float> %354, float %extract179, i32 1
  %371 = insertelement <4 x float> %355, float %extract180, i32 1
  %372 = insertelement <4 x float> %356, float %extract181, i32 1
  %373 = insertelement <4 x float> %357, float %extract182, i32 1
  %374 = insertelement <4 x float> %358, float %extract183, i32 1
  %375 = insertelement <4 x float> %359, float %extract232, i32 2
  %376 = insertelement <4 x float> %360, float %extract233, i32 2
  %377 = insertelement <4 x float> %361, float %extract234, i32 2
  %378 = insertelement <4 x float> %362, float %extract235, i32 2
  %379 = insertelement <4 x float> %363, float %extract236, i32 2
  %380 = insertelement <4 x float> %364, float %extract237, i32 2
  %381 = insertelement <4 x float> %365, float %extract238, i32 2
  %382 = insertelement <4 x float> %366, float %extract239, i32 2
  %383 = insertelement <4 x float> %367, float %extract240, i32 2
  %384 = insertelement <4 x float> %368, float %extract241, i32 2
  %385 = insertelement <4 x float> %369, float %extract242, i32 2
  %386 = insertelement <4 x float> %370, float %extract243, i32 2
  %387 = insertelement <4 x float> %371, float %extract244, i32 2
  %388 = insertelement <4 x float> %372, float %extract245, i32 2
  %389 = insertelement <4 x float> %373, float %extract246, i32 2
  %390 = insertelement <4 x float> %374, float %extract247, i32 2
  %391 = shufflevector <4 x float> %375, <4 x float> %109, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %392 = shufflevector <4 x float> %376, <4 x float> %110, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %393 = shufflevector <4 x float> %377, <4 x float> %111, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %394 = shufflevector <4 x float> %378, <4 x float> %112, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %395 = shufflevector <4 x float> %379, <4 x float> %113, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %396 = shufflevector <4 x float> %380, <4 x float> %114, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %397 = shufflevector <4 x float> %381, <4 x float> %115, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %398 = shufflevector <4 x float> %382, <4 x float> %116, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %399 = shufflevector <4 x float> %383, <4 x float> %117, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %400 = shufflevector <4 x float> %384, <4 x float> %118, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %401 = shufflevector <4 x float> %385, <4 x float> %119, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %402 = shufflevector <4 x float> %386, <4 x float> %120, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %403 = shufflevector <4 x float> %387, <4 x float> %121, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %404 = shufflevector <4 x float> %388, <4 x float> %122, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %405 = shufflevector <4 x float> %389, <4 x float> %123, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %406 = shufflevector <4 x float> %390, <4 x float> %124, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %391, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %392, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %393, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %394, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %395, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %396, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %397, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %398, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %399, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %400, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %401, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %402, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %403, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %404, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %405, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %406, <4 x float> addrspace(1)* %39, align 16
  %407 = load <4 x float> addrspace(1)* %8, align 16
  %408 = load <4 x float> addrspace(1)* %9, align 16
  %409 = load <4 x float> addrspace(1)* %10, align 16
  %410 = load <4 x float> addrspace(1)* %11, align 16
  %411 = load <4 x float> addrspace(1)* %12, align 16
  %412 = load <4 x float> addrspace(1)* %13, align 16
  %413 = load <4 x float> addrspace(1)* %14, align 16
  %414 = load <4 x float> addrspace(1)* %15, align 16
  %415 = load <4 x float> addrspace(1)* %16, align 16
  %416 = load <4 x float> addrspace(1)* %17, align 16
  %417 = load <4 x float> addrspace(1)* %18, align 16
  %418 = load <4 x float> addrspace(1)* %19, align 16
  %419 = load <4 x float> addrspace(1)* %20, align 16
  %420 = load <4 x float> addrspace(1)* %21, align 16
  %421 = load <4 x float> addrspace(1)* %22, align 16
  %422 = load <4 x float> addrspace(1)* %23, align 16
  %423 = insertelement <4 x float> undef, float %extract104, i32 0
  %424 = insertelement <4 x float> undef, float %extract105, i32 0
  %425 = insertelement <4 x float> undef, float %extract106, i32 0
  %426 = insertelement <4 x float> undef, float %extract107, i32 0
  %427 = insertelement <4 x float> undef, float %extract108, i32 0
  %428 = insertelement <4 x float> undef, float %extract109, i32 0
  %429 = insertelement <4 x float> undef, float %extract110, i32 0
  %430 = insertelement <4 x float> undef, float %extract111, i32 0
  %431 = insertelement <4 x float> undef, float %extract112, i32 0
  %432 = insertelement <4 x float> undef, float %extract113, i32 0
  %433 = insertelement <4 x float> undef, float %extract114, i32 0
  %434 = insertelement <4 x float> undef, float %extract115, i32 0
  %435 = insertelement <4 x float> undef, float %extract116, i32 0
  %436 = insertelement <4 x float> undef, float %extract117, i32 0
  %437 = insertelement <4 x float> undef, float %extract118, i32 0
  %438 = insertelement <4 x float> undef, float %extract119, i32 0
  %439 = insertelement <4 x float> %423, float %extract168, i32 1
  %440 = insertelement <4 x float> %424, float %extract169, i32 1
  %441 = insertelement <4 x float> %425, float %extract170, i32 1
  %442 = insertelement <4 x float> %426, float %extract171, i32 1
  %443 = insertelement <4 x float> %427, float %extract172, i32 1
  %444 = insertelement <4 x float> %428, float %extract173, i32 1
  %445 = insertelement <4 x float> %429, float %extract174, i32 1
  %446 = insertelement <4 x float> %430, float %extract175, i32 1
  %447 = insertelement <4 x float> %431, float %extract176, i32 1
  %448 = insertelement <4 x float> %432, float %extract177, i32 1
  %449 = insertelement <4 x float> %433, float %extract178, i32 1
  %450 = insertelement <4 x float> %434, float %extract179, i32 1
  %451 = insertelement <4 x float> %435, float %extract180, i32 1
  %452 = insertelement <4 x float> %436, float %extract181, i32 1
  %453 = insertelement <4 x float> %437, float %extract182, i32 1
  %454 = insertelement <4 x float> %438, float %extract183, i32 1
  %455 = insertelement <4 x float> %439, float %extract232, i32 2
  %456 = insertelement <4 x float> %440, float %extract233, i32 2
  %457 = insertelement <4 x float> %441, float %extract234, i32 2
  %458 = insertelement <4 x float> %442, float %extract235, i32 2
  %459 = insertelement <4 x float> %443, float %extract236, i32 2
  %460 = insertelement <4 x float> %444, float %extract237, i32 2
  %461 = insertelement <4 x float> %445, float %extract238, i32 2
  %462 = insertelement <4 x float> %446, float %extract239, i32 2
  %463 = insertelement <4 x float> %447, float %extract240, i32 2
  %464 = insertelement <4 x float> %448, float %extract241, i32 2
  %465 = insertelement <4 x float> %449, float %extract242, i32 2
  %466 = insertelement <4 x float> %450, float %extract243, i32 2
  %467 = insertelement <4 x float> %451, float %extract244, i32 2
  %468 = insertelement <4 x float> %452, float %extract245, i32 2
  %469 = insertelement <4 x float> %453, float %extract246, i32 2
  %470 = insertelement <4 x float> %454, float %extract247, i32 2
  %471 = shufflevector <4 x float> %455, <4 x float> %407, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %472 = shufflevector <4 x float> %456, <4 x float> %408, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %473 = shufflevector <4 x float> %457, <4 x float> %409, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %474 = shufflevector <4 x float> %458, <4 x float> %410, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %475 = shufflevector <4 x float> %459, <4 x float> %411, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %476 = shufflevector <4 x float> %460, <4 x float> %412, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %477 = shufflevector <4 x float> %461, <4 x float> %413, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %478 = shufflevector <4 x float> %462, <4 x float> %414, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %479 = shufflevector <4 x float> %463, <4 x float> %415, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %480 = shufflevector <4 x float> %464, <4 x float> %416, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %481 = shufflevector <4 x float> %465, <4 x float> %417, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %482 = shufflevector <4 x float> %466, <4 x float> %418, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %483 = shufflevector <4 x float> %467, <4 x float> %419, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %484 = shufflevector <4 x float> %468, <4 x float> %420, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %485 = shufflevector <4 x float> %469, <4 x float> %421, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %486 = shufflevector <4 x float> %470, <4 x float> %422, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %471, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %472, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %473, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %474, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %475, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %476, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %477, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %478, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %479, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %480, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %481, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %482, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %483, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %484, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %485, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %486, <4 x float> addrspace(1)* %39, align 16
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp
  br i1 %exitcond, label %._crit_edge, label %7

._crit_edge:                                      ; preds = %7, %SyncBB248
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB248

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.intel_rgb2yuv_vector_separated_args(<4 x float> addrspace(1)* nocapture %pSrc, <4 x float> addrspace(1)* nocapture %pDst, i32 %PixelNum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %PixelNum, 0
  %tmp = zext i32 %PixelNum to i64
  %tmp9 = sext i32 %PixelNum to i64
  %temp = insertelement <16 x i64> undef, i64 %tmp9, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB367

SyncBB367:                                        ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB367
  %1 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %1, align 8
  %4 = load i64* %2, align 8
  %5 = add i64 %4, %3
  %broadcast1 = insertelement <16 x i64> undef, i64 %5, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %6 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %tmp1085 = mul <16 x i64> %6, %vector
  %sext86 = shl <16 x i64> %tmp1085, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %tmp1287 = ashr <16 x i64> %sext86, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  br label %7

; <label>:7                                       ; preds = %7, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %7 ]
  %temp88 = insertelement <16 x i64> undef, i64 %indvar, i32 0
  %vector89 = shufflevector <16 x i64> %temp88, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1390 = add <16 x i64> %tmp1287, %vector89
  %extract = extractelement <16 x i64> %tmp1390, i32 0
  %extract91 = extractelement <16 x i64> %tmp1390, i32 1
  %extract92 = extractelement <16 x i64> %tmp1390, i32 2
  %extract93 = extractelement <16 x i64> %tmp1390, i32 3
  %extract94 = extractelement <16 x i64> %tmp1390, i32 4
  %extract95 = extractelement <16 x i64> %tmp1390, i32 5
  %extract96 = extractelement <16 x i64> %tmp1390, i32 6
  %extract97 = extractelement <16 x i64> %tmp1390, i32 7
  %extract98 = extractelement <16 x i64> %tmp1390, i32 8
  %extract99 = extractelement <16 x i64> %tmp1390, i32 9
  %extract100 = extractelement <16 x i64> %tmp1390, i32 10
  %extract101 = extractelement <16 x i64> %tmp1390, i32 11
  %extract102 = extractelement <16 x i64> %tmp1390, i32 12
  %extract103 = extractelement <16 x i64> %tmp1390, i32 13
  %extract104 = extractelement <16 x i64> %tmp1390, i32 14
  %extract105 = extractelement <16 x i64> %tmp1390, i32 15
  %8 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract
  %9 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract91
  %10 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract92
  %11 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract93
  %12 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract94
  %13 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract95
  %14 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract96
  %15 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract97
  %16 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract98
  %17 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract99
  %18 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract100
  %19 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract101
  %20 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract102
  %21 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract103
  %22 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract104
  %23 = getelementptr <4 x float> addrspace(1)* %pSrc, i64 %extract105
  %24 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract
  %25 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract91
  %26 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract92
  %27 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract93
  %28 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract94
  %29 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract95
  %30 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract96
  %31 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract97
  %32 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract98
  %33 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract99
  %34 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract100
  %35 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract101
  %36 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract102
  %37 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract103
  %38 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract104
  %39 = getelementptr <4 x float> addrspace(1)* %pDst, i64 %extract105
  %40 = load <4 x float> addrspace(1)* %8, align 16
  %41 = load <4 x float> addrspace(1)* %9, align 16
  %42 = load <4 x float> addrspace(1)* %10, align 16
  %43 = load <4 x float> addrspace(1)* %11, align 16
  %44 = load <4 x float> addrspace(1)* %12, align 16
  %45 = load <4 x float> addrspace(1)* %13, align 16
  %46 = load <4 x float> addrspace(1)* %14, align 16
  %47 = load <4 x float> addrspace(1)* %15, align 16
  %48 = load <4 x float> addrspace(1)* %16, align 16
  %49 = load <4 x float> addrspace(1)* %17, align 16
  %50 = load <4 x float> addrspace(1)* %18, align 16
  %51 = load <4 x float> addrspace(1)* %19, align 16
  %52 = load <4 x float> addrspace(1)* %20, align 16
  %53 = load <4 x float> addrspace(1)* %21, align 16
  %54 = load <4 x float> addrspace(1)* %22, align 16
  %55 = load <4 x float> addrspace(1)* %23, align 16
  %56 = extractelement <4 x float> %40, i32 0
  %57 = extractelement <4 x float> %41, i32 0
  %58 = extractelement <4 x float> %42, i32 0
  %59 = extractelement <4 x float> %43, i32 0
  %60 = extractelement <4 x float> %44, i32 0
  %61 = extractelement <4 x float> %45, i32 0
  %62 = extractelement <4 x float> %46, i32 0
  %63 = extractelement <4 x float> %47, i32 0
  %64 = extractelement <4 x float> %48, i32 0
  %65 = extractelement <4 x float> %49, i32 0
  %66 = extractelement <4 x float> %50, i32 0
  %67 = extractelement <4 x float> %51, i32 0
  %68 = extractelement <4 x float> %52, i32 0
  %69 = extractelement <4 x float> %53, i32 0
  %70 = extractelement <4 x float> %54, i32 0
  %71 = extractelement <4 x float> %55, i32 0
  %temp.vect106 = insertelement <16 x float> undef, float %56, i32 0
  %temp.vect107 = insertelement <16 x float> %temp.vect106, float %57, i32 1
  %temp.vect108 = insertelement <16 x float> %temp.vect107, float %58, i32 2
  %temp.vect109 = insertelement <16 x float> %temp.vect108, float %59, i32 3
  %temp.vect110 = insertelement <16 x float> %temp.vect109, float %60, i32 4
  %temp.vect111 = insertelement <16 x float> %temp.vect110, float %61, i32 5
  %temp.vect112 = insertelement <16 x float> %temp.vect111, float %62, i32 6
  %temp.vect113 = insertelement <16 x float> %temp.vect112, float %63, i32 7
  %temp.vect114 = insertelement <16 x float> %temp.vect113, float %64, i32 8
  %temp.vect115 = insertelement <16 x float> %temp.vect114, float %65, i32 9
  %temp.vect116 = insertelement <16 x float> %temp.vect115, float %66, i32 10
  %temp.vect117 = insertelement <16 x float> %temp.vect116, float %67, i32 11
  %temp.vect118 = insertelement <16 x float> %temp.vect117, float %68, i32 12
  %temp.vect119 = insertelement <16 x float> %temp.vect118, float %69, i32 13
  %temp.vect120 = insertelement <16 x float> %temp.vect119, float %70, i32 14
  %temp.vect121 = insertelement <16 x float> %temp.vect120, float %71, i32 15
  %72 = extractelement <4 x float> %40, i32 1
  %73 = extractelement <4 x float> %41, i32 1
  %74 = extractelement <4 x float> %42, i32 1
  %75 = extractelement <4 x float> %43, i32 1
  %76 = extractelement <4 x float> %44, i32 1
  %77 = extractelement <4 x float> %45, i32 1
  %78 = extractelement <4 x float> %46, i32 1
  %79 = extractelement <4 x float> %47, i32 1
  %80 = extractelement <4 x float> %48, i32 1
  %81 = extractelement <4 x float> %49, i32 1
  %82 = extractelement <4 x float> %50, i32 1
  %83 = extractelement <4 x float> %51, i32 1
  %84 = extractelement <4 x float> %52, i32 1
  %85 = extractelement <4 x float> %53, i32 1
  %86 = extractelement <4 x float> %54, i32 1
  %87 = extractelement <4 x float> %55, i32 1
  %temp.vect123 = insertelement <16 x float> undef, float %72, i32 0
  %temp.vect124 = insertelement <16 x float> %temp.vect123, float %73, i32 1
  %temp.vect125 = insertelement <16 x float> %temp.vect124, float %74, i32 2
  %temp.vect126 = insertelement <16 x float> %temp.vect125, float %75, i32 3
  %temp.vect127 = insertelement <16 x float> %temp.vect126, float %76, i32 4
  %temp.vect128 = insertelement <16 x float> %temp.vect127, float %77, i32 5
  %temp.vect129 = insertelement <16 x float> %temp.vect128, float %78, i32 6
  %temp.vect130 = insertelement <16 x float> %temp.vect129, float %79, i32 7
  %temp.vect131 = insertelement <16 x float> %temp.vect130, float %80, i32 8
  %temp.vect132 = insertelement <16 x float> %temp.vect131, float %81, i32 9
  %temp.vect133 = insertelement <16 x float> %temp.vect132, float %82, i32 10
  %temp.vect134 = insertelement <16 x float> %temp.vect133, float %83, i32 11
  %temp.vect135 = insertelement <16 x float> %temp.vect134, float %84, i32 12
  %temp.vect136 = insertelement <16 x float> %temp.vect135, float %85, i32 13
  %temp.vect137 = insertelement <16 x float> %temp.vect136, float %86, i32 14
  %temp.vect138 = insertelement <16 x float> %temp.vect137, float %87, i32 15
  %88 = extractelement <4 x float> %40, i32 2
  %89 = extractelement <4 x float> %41, i32 2
  %90 = extractelement <4 x float> %42, i32 2
  %91 = extractelement <4 x float> %43, i32 2
  %92 = extractelement <4 x float> %44, i32 2
  %93 = extractelement <4 x float> %45, i32 2
  %94 = extractelement <4 x float> %46, i32 2
  %95 = extractelement <4 x float> %47, i32 2
  %96 = extractelement <4 x float> %48, i32 2
  %97 = extractelement <4 x float> %49, i32 2
  %98 = extractelement <4 x float> %50, i32 2
  %99 = extractelement <4 x float> %51, i32 2
  %100 = extractelement <4 x float> %52, i32 2
  %101 = extractelement <4 x float> %53, i32 2
  %102 = extractelement <4 x float> %54, i32 2
  %103 = extractelement <4 x float> %55, i32 2
  %temp.vect141 = insertelement <16 x float> undef, float %88, i32 0
  %temp.vect142 = insertelement <16 x float> %temp.vect141, float %89, i32 1
  %temp.vect143 = insertelement <16 x float> %temp.vect142, float %90, i32 2
  %temp.vect144 = insertelement <16 x float> %temp.vect143, float %91, i32 3
  %temp.vect145 = insertelement <16 x float> %temp.vect144, float %92, i32 4
  %temp.vect146 = insertelement <16 x float> %temp.vect145, float %93, i32 5
  %temp.vect147 = insertelement <16 x float> %temp.vect146, float %94, i32 6
  %temp.vect148 = insertelement <16 x float> %temp.vect147, float %95, i32 7
  %temp.vect149 = insertelement <16 x float> %temp.vect148, float %96, i32 8
  %temp.vect150 = insertelement <16 x float> %temp.vect149, float %97, i32 9
  %temp.vect151 = insertelement <16 x float> %temp.vect150, float %98, i32 10
  %temp.vect152 = insertelement <16 x float> %temp.vect151, float %99, i32 11
  %temp.vect153 = insertelement <16 x float> %temp.vect152, float %100, i32 12
  %temp.vect154 = insertelement <16 x float> %temp.vect153, float %101, i32 13
  %temp.vect155 = insertelement <16 x float> %temp.vect154, float %102, i32 14
  %temp.vect156 = insertelement <16 x float> %temp.vect155, float %103, i32 15
  %104 = extractelement <4 x float> %40, i32 3
  %105 = extractelement <4 x float> %41, i32 3
  %106 = extractelement <4 x float> %42, i32 3
  %107 = extractelement <4 x float> %43, i32 3
  %108 = extractelement <4 x float> %44, i32 3
  %109 = extractelement <4 x float> %45, i32 3
  %110 = extractelement <4 x float> %46, i32 3
  %111 = extractelement <4 x float> %47, i32 3
  %112 = extractelement <4 x float> %48, i32 3
  %113 = extractelement <4 x float> %49, i32 3
  %114 = extractelement <4 x float> %50, i32 3
  %115 = extractelement <4 x float> %51, i32 3
  %116 = extractelement <4 x float> %52, i32 3
  %117 = extractelement <4 x float> %53, i32 3
  %118 = extractelement <4 x float> %54, i32 3
  %119 = extractelement <4 x float> %55, i32 3
  %temp.vect159 = insertelement <16 x float> undef, float %104, i32 0
  %temp.vect160 = insertelement <16 x float> %temp.vect159, float %105, i32 1
  %temp.vect161 = insertelement <16 x float> %temp.vect160, float %106, i32 2
  %temp.vect162 = insertelement <16 x float> %temp.vect161, float %107, i32 3
  %temp.vect163 = insertelement <16 x float> %temp.vect162, float %108, i32 4
  %temp.vect164 = insertelement <16 x float> %temp.vect163, float %109, i32 5
  %temp.vect165 = insertelement <16 x float> %temp.vect164, float %110, i32 6
  %temp.vect166 = insertelement <16 x float> %temp.vect165, float %111, i32 7
  %temp.vect167 = insertelement <16 x float> %temp.vect166, float %112, i32 8
  %temp.vect168 = insertelement <16 x float> %temp.vect167, float %113, i32 9
  %temp.vect169 = insertelement <16 x float> %temp.vect168, float %114, i32 10
  %temp.vect170 = insertelement <16 x float> %temp.vect169, float %115, i32 11
  %temp.vect171 = insertelement <16 x float> %temp.vect170, float %116, i32 12
  %temp.vect172 = insertelement <16 x float> %temp.vect171, float %117, i32 13
  %temp.vect173 = insertelement <16 x float> %temp.vect172, float %118, i32 14
  %temp.vect174 = insertelement <16 x float> %temp.vect173, float %119, i32 15
  %mul_dot122 = fmul <16 x float> %temp.vect121, <float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000>
  %mul_dot8139 = fmul <16 x float> %temp.vect138, <float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000>
  %sum_dot140 = fadd <16 x float> %mul_dot122, %mul_dot8139
  %mul_dot9157 = fmul <16 x float> %temp.vect156, <float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000>
  %sum_dot10158 = fadd <16 x float> %sum_dot140, %mul_dot9157
  %mul_dot11175 = fmul <16 x float> %temp.vect174, zeroinitializer
  %sum_dot12176 = fadd <16 x float> %sum_dot10158, %mul_dot11175
  %extract177 = extractelement <16 x float> %sum_dot12176, i32 0
  %extract178 = extractelement <16 x float> %sum_dot12176, i32 1
  %extract179 = extractelement <16 x float> %sum_dot12176, i32 2
  %extract180 = extractelement <16 x float> %sum_dot12176, i32 3
  %extract181 = extractelement <16 x float> %sum_dot12176, i32 4
  %extract182 = extractelement <16 x float> %sum_dot12176, i32 5
  %extract183 = extractelement <16 x float> %sum_dot12176, i32 6
  %extract184 = extractelement <16 x float> %sum_dot12176, i32 7
  %extract185 = extractelement <16 x float> %sum_dot12176, i32 8
  %extract186 = extractelement <16 x float> %sum_dot12176, i32 9
  %extract187 = extractelement <16 x float> %sum_dot12176, i32 10
  %extract188 = extractelement <16 x float> %sum_dot12176, i32 11
  %extract189 = extractelement <16 x float> %sum_dot12176, i32 12
  %extract190 = extractelement <16 x float> %sum_dot12176, i32 13
  %extract191 = extractelement <16 x float> %sum_dot12176, i32 14
  %extract192 = extractelement <16 x float> %sum_dot12176, i32 15
  %120 = load <4 x float> addrspace(1)* %24, align 16
  %121 = load <4 x float> addrspace(1)* %25, align 16
  %122 = load <4 x float> addrspace(1)* %26, align 16
  %123 = load <4 x float> addrspace(1)* %27, align 16
  %124 = load <4 x float> addrspace(1)* %28, align 16
  %125 = load <4 x float> addrspace(1)* %29, align 16
  %126 = load <4 x float> addrspace(1)* %30, align 16
  %127 = load <4 x float> addrspace(1)* %31, align 16
  %128 = load <4 x float> addrspace(1)* %32, align 16
  %129 = load <4 x float> addrspace(1)* %33, align 16
  %130 = load <4 x float> addrspace(1)* %34, align 16
  %131 = load <4 x float> addrspace(1)* %35, align 16
  %132 = load <4 x float> addrspace(1)* %36, align 16
  %133 = load <4 x float> addrspace(1)* %37, align 16
  %134 = load <4 x float> addrspace(1)* %38, align 16
  %135 = load <4 x float> addrspace(1)* %39, align 16
  %136 = insertelement <4 x float> undef, float %extract177, i32 0
  %137 = insertelement <4 x float> undef, float %extract178, i32 0
  %138 = insertelement <4 x float> undef, float %extract179, i32 0
  %139 = insertelement <4 x float> undef, float %extract180, i32 0
  %140 = insertelement <4 x float> undef, float %extract181, i32 0
  %141 = insertelement <4 x float> undef, float %extract182, i32 0
  %142 = insertelement <4 x float> undef, float %extract183, i32 0
  %143 = insertelement <4 x float> undef, float %extract184, i32 0
  %144 = insertelement <4 x float> undef, float %extract185, i32 0
  %145 = insertelement <4 x float> undef, float %extract186, i32 0
  %146 = insertelement <4 x float> undef, float %extract187, i32 0
  %147 = insertelement <4 x float> undef, float %extract188, i32 0
  %148 = insertelement <4 x float> undef, float %extract189, i32 0
  %149 = insertelement <4 x float> undef, float %extract190, i32 0
  %150 = insertelement <4 x float> undef, float %extract191, i32 0
  %151 = insertelement <4 x float> undef, float %extract192, i32 0
  %152 = shufflevector <4 x float> %136, <4 x float> %120, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %153 = shufflevector <4 x float> %137, <4 x float> %121, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %154 = shufflevector <4 x float> %138, <4 x float> %122, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %155 = shufflevector <4 x float> %139, <4 x float> %123, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %156 = shufflevector <4 x float> %140, <4 x float> %124, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %157 = shufflevector <4 x float> %141, <4 x float> %125, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %158 = shufflevector <4 x float> %142, <4 x float> %126, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %159 = shufflevector <4 x float> %143, <4 x float> %127, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %160 = shufflevector <4 x float> %144, <4 x float> %128, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %161 = shufflevector <4 x float> %145, <4 x float> %129, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %162 = shufflevector <4 x float> %146, <4 x float> %130, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %163 = shufflevector <4 x float> %147, <4 x float> %131, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %164 = shufflevector <4 x float> %148, <4 x float> %132, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %165 = shufflevector <4 x float> %149, <4 x float> %133, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %166 = shufflevector <4 x float> %150, <4 x float> %134, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %167 = shufflevector <4 x float> %151, <4 x float> %135, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  store <4 x float> %152, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %153, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %154, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %155, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %156, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %157, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %158, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %159, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %160, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %161, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %162, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %163, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %164, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %165, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %166, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %167, <4 x float> addrspace(1)* %39, align 16
  %168 = load <4 x float> addrspace(1)* %8, align 16
  %169 = load <4 x float> addrspace(1)* %9, align 16
  %170 = load <4 x float> addrspace(1)* %10, align 16
  %171 = load <4 x float> addrspace(1)* %11, align 16
  %172 = load <4 x float> addrspace(1)* %12, align 16
  %173 = load <4 x float> addrspace(1)* %13, align 16
  %174 = load <4 x float> addrspace(1)* %14, align 16
  %175 = load <4 x float> addrspace(1)* %15, align 16
  %176 = load <4 x float> addrspace(1)* %16, align 16
  %177 = load <4 x float> addrspace(1)* %17, align 16
  %178 = load <4 x float> addrspace(1)* %18, align 16
  %179 = load <4 x float> addrspace(1)* %19, align 16
  %180 = load <4 x float> addrspace(1)* %20, align 16
  %181 = load <4 x float> addrspace(1)* %21, align 16
  %182 = load <4 x float> addrspace(1)* %22, align 16
  %183 = load <4 x float> addrspace(1)* %23, align 16
  %184 = extractelement <4 x float> %168, i32 0
  %185 = extractelement <4 x float> %169, i32 0
  %186 = extractelement <4 x float> %170, i32 0
  %187 = extractelement <4 x float> %171, i32 0
  %188 = extractelement <4 x float> %172, i32 0
  %189 = extractelement <4 x float> %173, i32 0
  %190 = extractelement <4 x float> %174, i32 0
  %191 = extractelement <4 x float> %175, i32 0
  %192 = extractelement <4 x float> %176, i32 0
  %193 = extractelement <4 x float> %177, i32 0
  %194 = extractelement <4 x float> %178, i32 0
  %195 = extractelement <4 x float> %179, i32 0
  %196 = extractelement <4 x float> %180, i32 0
  %197 = extractelement <4 x float> %181, i32 0
  %198 = extractelement <4 x float> %182, i32 0
  %199 = extractelement <4 x float> %183, i32 0
  %temp.vect193 = insertelement <16 x float> undef, float %184, i32 0
  %temp.vect194 = insertelement <16 x float> %temp.vect193, float %185, i32 1
  %temp.vect195 = insertelement <16 x float> %temp.vect194, float %186, i32 2
  %temp.vect196 = insertelement <16 x float> %temp.vect195, float %187, i32 3
  %temp.vect197 = insertelement <16 x float> %temp.vect196, float %188, i32 4
  %temp.vect198 = insertelement <16 x float> %temp.vect197, float %189, i32 5
  %temp.vect199 = insertelement <16 x float> %temp.vect198, float %190, i32 6
  %temp.vect200 = insertelement <16 x float> %temp.vect199, float %191, i32 7
  %temp.vect201 = insertelement <16 x float> %temp.vect200, float %192, i32 8
  %temp.vect202 = insertelement <16 x float> %temp.vect201, float %193, i32 9
  %temp.vect203 = insertelement <16 x float> %temp.vect202, float %194, i32 10
  %temp.vect204 = insertelement <16 x float> %temp.vect203, float %195, i32 11
  %temp.vect205 = insertelement <16 x float> %temp.vect204, float %196, i32 12
  %temp.vect206 = insertelement <16 x float> %temp.vect205, float %197, i32 13
  %temp.vect207 = insertelement <16 x float> %temp.vect206, float %198, i32 14
  %temp.vect208 = insertelement <16 x float> %temp.vect207, float %199, i32 15
  %200 = extractelement <4 x float> %168, i32 1
  %201 = extractelement <4 x float> %169, i32 1
  %202 = extractelement <4 x float> %170, i32 1
  %203 = extractelement <4 x float> %171, i32 1
  %204 = extractelement <4 x float> %172, i32 1
  %205 = extractelement <4 x float> %173, i32 1
  %206 = extractelement <4 x float> %174, i32 1
  %207 = extractelement <4 x float> %175, i32 1
  %208 = extractelement <4 x float> %176, i32 1
  %209 = extractelement <4 x float> %177, i32 1
  %210 = extractelement <4 x float> %178, i32 1
  %211 = extractelement <4 x float> %179, i32 1
  %212 = extractelement <4 x float> %180, i32 1
  %213 = extractelement <4 x float> %181, i32 1
  %214 = extractelement <4 x float> %182, i32 1
  %215 = extractelement <4 x float> %183, i32 1
  %temp.vect210 = insertelement <16 x float> undef, float %200, i32 0
  %temp.vect211 = insertelement <16 x float> %temp.vect210, float %201, i32 1
  %temp.vect212 = insertelement <16 x float> %temp.vect211, float %202, i32 2
  %temp.vect213 = insertelement <16 x float> %temp.vect212, float %203, i32 3
  %temp.vect214 = insertelement <16 x float> %temp.vect213, float %204, i32 4
  %temp.vect215 = insertelement <16 x float> %temp.vect214, float %205, i32 5
  %temp.vect216 = insertelement <16 x float> %temp.vect215, float %206, i32 6
  %temp.vect217 = insertelement <16 x float> %temp.vect216, float %207, i32 7
  %temp.vect218 = insertelement <16 x float> %temp.vect217, float %208, i32 8
  %temp.vect219 = insertelement <16 x float> %temp.vect218, float %209, i32 9
  %temp.vect220 = insertelement <16 x float> %temp.vect219, float %210, i32 10
  %temp.vect221 = insertelement <16 x float> %temp.vect220, float %211, i32 11
  %temp.vect222 = insertelement <16 x float> %temp.vect221, float %212, i32 12
  %temp.vect223 = insertelement <16 x float> %temp.vect222, float %213, i32 13
  %temp.vect224 = insertelement <16 x float> %temp.vect223, float %214, i32 14
  %temp.vect225 = insertelement <16 x float> %temp.vect224, float %215, i32 15
  %216 = extractelement <4 x float> %168, i32 2
  %217 = extractelement <4 x float> %169, i32 2
  %218 = extractelement <4 x float> %170, i32 2
  %219 = extractelement <4 x float> %171, i32 2
  %220 = extractelement <4 x float> %172, i32 2
  %221 = extractelement <4 x float> %173, i32 2
  %222 = extractelement <4 x float> %174, i32 2
  %223 = extractelement <4 x float> %175, i32 2
  %224 = extractelement <4 x float> %176, i32 2
  %225 = extractelement <4 x float> %177, i32 2
  %226 = extractelement <4 x float> %178, i32 2
  %227 = extractelement <4 x float> %179, i32 2
  %228 = extractelement <4 x float> %180, i32 2
  %229 = extractelement <4 x float> %181, i32 2
  %230 = extractelement <4 x float> %182, i32 2
  %231 = extractelement <4 x float> %183, i32 2
  %temp.vect228 = insertelement <16 x float> undef, float %216, i32 0
  %temp.vect229 = insertelement <16 x float> %temp.vect228, float %217, i32 1
  %temp.vect230 = insertelement <16 x float> %temp.vect229, float %218, i32 2
  %temp.vect231 = insertelement <16 x float> %temp.vect230, float %219, i32 3
  %temp.vect232 = insertelement <16 x float> %temp.vect231, float %220, i32 4
  %temp.vect233 = insertelement <16 x float> %temp.vect232, float %221, i32 5
  %temp.vect234 = insertelement <16 x float> %temp.vect233, float %222, i32 6
  %temp.vect235 = insertelement <16 x float> %temp.vect234, float %223, i32 7
  %temp.vect236 = insertelement <16 x float> %temp.vect235, float %224, i32 8
  %temp.vect237 = insertelement <16 x float> %temp.vect236, float %225, i32 9
  %temp.vect238 = insertelement <16 x float> %temp.vect237, float %226, i32 10
  %temp.vect239 = insertelement <16 x float> %temp.vect238, float %227, i32 11
  %temp.vect240 = insertelement <16 x float> %temp.vect239, float %228, i32 12
  %temp.vect241 = insertelement <16 x float> %temp.vect240, float %229, i32 13
  %temp.vect242 = insertelement <16 x float> %temp.vect241, float %230, i32 14
  %temp.vect243 = insertelement <16 x float> %temp.vect242, float %231, i32 15
  %232 = extractelement <4 x float> %168, i32 3
  %233 = extractelement <4 x float> %169, i32 3
  %234 = extractelement <4 x float> %170, i32 3
  %235 = extractelement <4 x float> %171, i32 3
  %236 = extractelement <4 x float> %172, i32 3
  %237 = extractelement <4 x float> %173, i32 3
  %238 = extractelement <4 x float> %174, i32 3
  %239 = extractelement <4 x float> %175, i32 3
  %240 = extractelement <4 x float> %176, i32 3
  %241 = extractelement <4 x float> %177, i32 3
  %242 = extractelement <4 x float> %178, i32 3
  %243 = extractelement <4 x float> %179, i32 3
  %244 = extractelement <4 x float> %180, i32 3
  %245 = extractelement <4 x float> %181, i32 3
  %246 = extractelement <4 x float> %182, i32 3
  %247 = extractelement <4 x float> %183, i32 3
  %temp.vect246 = insertelement <16 x float> undef, float %232, i32 0
  %temp.vect247 = insertelement <16 x float> %temp.vect246, float %233, i32 1
  %temp.vect248 = insertelement <16 x float> %temp.vect247, float %234, i32 2
  %temp.vect249 = insertelement <16 x float> %temp.vect248, float %235, i32 3
  %temp.vect250 = insertelement <16 x float> %temp.vect249, float %236, i32 4
  %temp.vect251 = insertelement <16 x float> %temp.vect250, float %237, i32 5
  %temp.vect252 = insertelement <16 x float> %temp.vect251, float %238, i32 6
  %temp.vect253 = insertelement <16 x float> %temp.vect252, float %239, i32 7
  %temp.vect254 = insertelement <16 x float> %temp.vect253, float %240, i32 8
  %temp.vect255 = insertelement <16 x float> %temp.vect254, float %241, i32 9
  %temp.vect256 = insertelement <16 x float> %temp.vect255, float %242, i32 10
  %temp.vect257 = insertelement <16 x float> %temp.vect256, float %243, i32 11
  %temp.vect258 = insertelement <16 x float> %temp.vect257, float %244, i32 12
  %temp.vect259 = insertelement <16 x float> %temp.vect258, float %245, i32 13
  %temp.vect260 = insertelement <16 x float> %temp.vect259, float %246, i32 14
  %temp.vect261 = insertelement <16 x float> %temp.vect260, float %247, i32 15
  %mul_dot21209 = fmul <16 x float> %temp.vect208, <float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000>
  %mul_dot22226 = fmul <16 x float> %temp.vect225, <float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000>
  %sum_dot23227 = fadd <16 x float> %mul_dot21209, %mul_dot22226
  %mul_dot24244 = fmul <16 x float> %temp.vect243, <float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000>
  %sum_dot25245 = fadd <16 x float> %sum_dot23227, %mul_dot24244
  %mul_dot26262 = fmul <16 x float> %temp.vect261, zeroinitializer
  %sum_dot27263 = fadd <16 x float> %sum_dot25245, %mul_dot26262
  %extract264 = extractelement <16 x float> %sum_dot27263, i32 0
  %extract265 = extractelement <16 x float> %sum_dot27263, i32 1
  %extract266 = extractelement <16 x float> %sum_dot27263, i32 2
  %extract267 = extractelement <16 x float> %sum_dot27263, i32 3
  %extract268 = extractelement <16 x float> %sum_dot27263, i32 4
  %extract269 = extractelement <16 x float> %sum_dot27263, i32 5
  %extract270 = extractelement <16 x float> %sum_dot27263, i32 6
  %extract271 = extractelement <16 x float> %sum_dot27263, i32 7
  %extract272 = extractelement <16 x float> %sum_dot27263, i32 8
  %extract273 = extractelement <16 x float> %sum_dot27263, i32 9
  %extract274 = extractelement <16 x float> %sum_dot27263, i32 10
  %extract275 = extractelement <16 x float> %sum_dot27263, i32 11
  %extract276 = extractelement <16 x float> %sum_dot27263, i32 12
  %extract277 = extractelement <16 x float> %sum_dot27263, i32 13
  %extract278 = extractelement <16 x float> %sum_dot27263, i32 14
  %extract279 = extractelement <16 x float> %sum_dot27263, i32 15
  %248 = load <4 x float> addrspace(1)* %24, align 16
  %249 = load <4 x float> addrspace(1)* %25, align 16
  %250 = load <4 x float> addrspace(1)* %26, align 16
  %251 = load <4 x float> addrspace(1)* %27, align 16
  %252 = load <4 x float> addrspace(1)* %28, align 16
  %253 = load <4 x float> addrspace(1)* %29, align 16
  %254 = load <4 x float> addrspace(1)* %30, align 16
  %255 = load <4 x float> addrspace(1)* %31, align 16
  %256 = load <4 x float> addrspace(1)* %32, align 16
  %257 = load <4 x float> addrspace(1)* %33, align 16
  %258 = load <4 x float> addrspace(1)* %34, align 16
  %259 = load <4 x float> addrspace(1)* %35, align 16
  %260 = load <4 x float> addrspace(1)* %36, align 16
  %261 = load <4 x float> addrspace(1)* %37, align 16
  %262 = load <4 x float> addrspace(1)* %38, align 16
  %263 = load <4 x float> addrspace(1)* %39, align 16
  %264 = extractelement <4 x float> %248, i32 0
  %265 = extractelement <4 x float> %249, i32 0
  %266 = extractelement <4 x float> %250, i32 0
  %267 = extractelement <4 x float> %251, i32 0
  %268 = extractelement <4 x float> %252, i32 0
  %269 = extractelement <4 x float> %253, i32 0
  %270 = extractelement <4 x float> %254, i32 0
  %271 = extractelement <4 x float> %255, i32 0
  %272 = extractelement <4 x float> %256, i32 0
  %273 = extractelement <4 x float> %257, i32 0
  %274 = extractelement <4 x float> %258, i32 0
  %275 = extractelement <4 x float> %259, i32 0
  %276 = extractelement <4 x float> %260, i32 0
  %277 = extractelement <4 x float> %261, i32 0
  %278 = extractelement <4 x float> %262, i32 0
  %279 = extractelement <4 x float> %263, i32 0
  %280 = insertelement <4 x float> undef, float %264, i32 0
  %281 = insertelement <4 x float> undef, float %265, i32 0
  %282 = insertelement <4 x float> undef, float %266, i32 0
  %283 = insertelement <4 x float> undef, float %267, i32 0
  %284 = insertelement <4 x float> undef, float %268, i32 0
  %285 = insertelement <4 x float> undef, float %269, i32 0
  %286 = insertelement <4 x float> undef, float %270, i32 0
  %287 = insertelement <4 x float> undef, float %271, i32 0
  %288 = insertelement <4 x float> undef, float %272, i32 0
  %289 = insertelement <4 x float> undef, float %273, i32 0
  %290 = insertelement <4 x float> undef, float %274, i32 0
  %291 = insertelement <4 x float> undef, float %275, i32 0
  %292 = insertelement <4 x float> undef, float %276, i32 0
  %293 = insertelement <4 x float> undef, float %277, i32 0
  %294 = insertelement <4 x float> undef, float %278, i32 0
  %295 = insertelement <4 x float> undef, float %279, i32 0
  %296 = insertelement <4 x float> %280, float %extract264, i32 1
  %297 = insertelement <4 x float> %281, float %extract265, i32 1
  %298 = insertelement <4 x float> %282, float %extract266, i32 1
  %299 = insertelement <4 x float> %283, float %extract267, i32 1
  %300 = insertelement <4 x float> %284, float %extract268, i32 1
  %301 = insertelement <4 x float> %285, float %extract269, i32 1
  %302 = insertelement <4 x float> %286, float %extract270, i32 1
  %303 = insertelement <4 x float> %287, float %extract271, i32 1
  %304 = insertelement <4 x float> %288, float %extract272, i32 1
  %305 = insertelement <4 x float> %289, float %extract273, i32 1
  %306 = insertelement <4 x float> %290, float %extract274, i32 1
  %307 = insertelement <4 x float> %291, float %extract275, i32 1
  %308 = insertelement <4 x float> %292, float %extract276, i32 1
  %309 = insertelement <4 x float> %293, float %extract277, i32 1
  %310 = insertelement <4 x float> %294, float %extract278, i32 1
  %311 = insertelement <4 x float> %295, float %extract279, i32 1
  %312 = shufflevector <4 x float> %296, <4 x float> %248, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %313 = shufflevector <4 x float> %297, <4 x float> %249, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %314 = shufflevector <4 x float> %298, <4 x float> %250, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %315 = shufflevector <4 x float> %299, <4 x float> %251, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %316 = shufflevector <4 x float> %300, <4 x float> %252, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %317 = shufflevector <4 x float> %301, <4 x float> %253, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %318 = shufflevector <4 x float> %302, <4 x float> %254, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %319 = shufflevector <4 x float> %303, <4 x float> %255, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %320 = shufflevector <4 x float> %304, <4 x float> %256, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %321 = shufflevector <4 x float> %305, <4 x float> %257, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %322 = shufflevector <4 x float> %306, <4 x float> %258, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %323 = shufflevector <4 x float> %307, <4 x float> %259, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %324 = shufflevector <4 x float> %308, <4 x float> %260, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %325 = shufflevector <4 x float> %309, <4 x float> %261, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %326 = shufflevector <4 x float> %310, <4 x float> %262, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %327 = shufflevector <4 x float> %311, <4 x float> %263, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  store <4 x float> %312, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %313, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %314, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %315, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %316, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %317, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %318, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %319, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %320, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %321, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %322, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %323, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %324, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %325, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %326, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %327, <4 x float> addrspace(1)* %39, align 16
  %328 = load <4 x float> addrspace(1)* %8, align 16
  %329 = load <4 x float> addrspace(1)* %9, align 16
  %330 = load <4 x float> addrspace(1)* %10, align 16
  %331 = load <4 x float> addrspace(1)* %11, align 16
  %332 = load <4 x float> addrspace(1)* %12, align 16
  %333 = load <4 x float> addrspace(1)* %13, align 16
  %334 = load <4 x float> addrspace(1)* %14, align 16
  %335 = load <4 x float> addrspace(1)* %15, align 16
  %336 = load <4 x float> addrspace(1)* %16, align 16
  %337 = load <4 x float> addrspace(1)* %17, align 16
  %338 = load <4 x float> addrspace(1)* %18, align 16
  %339 = load <4 x float> addrspace(1)* %19, align 16
  %340 = load <4 x float> addrspace(1)* %20, align 16
  %341 = load <4 x float> addrspace(1)* %21, align 16
  %342 = load <4 x float> addrspace(1)* %22, align 16
  %343 = load <4 x float> addrspace(1)* %23, align 16
  %344 = extractelement <4 x float> %328, i32 0
  %345 = extractelement <4 x float> %329, i32 0
  %346 = extractelement <4 x float> %330, i32 0
  %347 = extractelement <4 x float> %331, i32 0
  %348 = extractelement <4 x float> %332, i32 0
  %349 = extractelement <4 x float> %333, i32 0
  %350 = extractelement <4 x float> %334, i32 0
  %351 = extractelement <4 x float> %335, i32 0
  %352 = extractelement <4 x float> %336, i32 0
  %353 = extractelement <4 x float> %337, i32 0
  %354 = extractelement <4 x float> %338, i32 0
  %355 = extractelement <4 x float> %339, i32 0
  %356 = extractelement <4 x float> %340, i32 0
  %357 = extractelement <4 x float> %341, i32 0
  %358 = extractelement <4 x float> %342, i32 0
  %359 = extractelement <4 x float> %343, i32 0
  %temp.vect280 = insertelement <16 x float> undef, float %344, i32 0
  %temp.vect281 = insertelement <16 x float> %temp.vect280, float %345, i32 1
  %temp.vect282 = insertelement <16 x float> %temp.vect281, float %346, i32 2
  %temp.vect283 = insertelement <16 x float> %temp.vect282, float %347, i32 3
  %temp.vect284 = insertelement <16 x float> %temp.vect283, float %348, i32 4
  %temp.vect285 = insertelement <16 x float> %temp.vect284, float %349, i32 5
  %temp.vect286 = insertelement <16 x float> %temp.vect285, float %350, i32 6
  %temp.vect287 = insertelement <16 x float> %temp.vect286, float %351, i32 7
  %temp.vect288 = insertelement <16 x float> %temp.vect287, float %352, i32 8
  %temp.vect289 = insertelement <16 x float> %temp.vect288, float %353, i32 9
  %temp.vect290 = insertelement <16 x float> %temp.vect289, float %354, i32 10
  %temp.vect291 = insertelement <16 x float> %temp.vect290, float %355, i32 11
  %temp.vect292 = insertelement <16 x float> %temp.vect291, float %356, i32 12
  %temp.vect293 = insertelement <16 x float> %temp.vect292, float %357, i32 13
  %temp.vect294 = insertelement <16 x float> %temp.vect293, float %358, i32 14
  %temp.vect295 = insertelement <16 x float> %temp.vect294, float %359, i32 15
  %360 = extractelement <4 x float> %328, i32 1
  %361 = extractelement <4 x float> %329, i32 1
  %362 = extractelement <4 x float> %330, i32 1
  %363 = extractelement <4 x float> %331, i32 1
  %364 = extractelement <4 x float> %332, i32 1
  %365 = extractelement <4 x float> %333, i32 1
  %366 = extractelement <4 x float> %334, i32 1
  %367 = extractelement <4 x float> %335, i32 1
  %368 = extractelement <4 x float> %336, i32 1
  %369 = extractelement <4 x float> %337, i32 1
  %370 = extractelement <4 x float> %338, i32 1
  %371 = extractelement <4 x float> %339, i32 1
  %372 = extractelement <4 x float> %340, i32 1
  %373 = extractelement <4 x float> %341, i32 1
  %374 = extractelement <4 x float> %342, i32 1
  %375 = extractelement <4 x float> %343, i32 1
  %temp.vect297 = insertelement <16 x float> undef, float %360, i32 0
  %temp.vect298 = insertelement <16 x float> %temp.vect297, float %361, i32 1
  %temp.vect299 = insertelement <16 x float> %temp.vect298, float %362, i32 2
  %temp.vect300 = insertelement <16 x float> %temp.vect299, float %363, i32 3
  %temp.vect301 = insertelement <16 x float> %temp.vect300, float %364, i32 4
  %temp.vect302 = insertelement <16 x float> %temp.vect301, float %365, i32 5
  %temp.vect303 = insertelement <16 x float> %temp.vect302, float %366, i32 6
  %temp.vect304 = insertelement <16 x float> %temp.vect303, float %367, i32 7
  %temp.vect305 = insertelement <16 x float> %temp.vect304, float %368, i32 8
  %temp.vect306 = insertelement <16 x float> %temp.vect305, float %369, i32 9
  %temp.vect307 = insertelement <16 x float> %temp.vect306, float %370, i32 10
  %temp.vect308 = insertelement <16 x float> %temp.vect307, float %371, i32 11
  %temp.vect309 = insertelement <16 x float> %temp.vect308, float %372, i32 12
  %temp.vect310 = insertelement <16 x float> %temp.vect309, float %373, i32 13
  %temp.vect311 = insertelement <16 x float> %temp.vect310, float %374, i32 14
  %temp.vect312 = insertelement <16 x float> %temp.vect311, float %375, i32 15
  %376 = extractelement <4 x float> %328, i32 2
  %377 = extractelement <4 x float> %329, i32 2
  %378 = extractelement <4 x float> %330, i32 2
  %379 = extractelement <4 x float> %331, i32 2
  %380 = extractelement <4 x float> %332, i32 2
  %381 = extractelement <4 x float> %333, i32 2
  %382 = extractelement <4 x float> %334, i32 2
  %383 = extractelement <4 x float> %335, i32 2
  %384 = extractelement <4 x float> %336, i32 2
  %385 = extractelement <4 x float> %337, i32 2
  %386 = extractelement <4 x float> %338, i32 2
  %387 = extractelement <4 x float> %339, i32 2
  %388 = extractelement <4 x float> %340, i32 2
  %389 = extractelement <4 x float> %341, i32 2
  %390 = extractelement <4 x float> %342, i32 2
  %391 = extractelement <4 x float> %343, i32 2
  %temp.vect315 = insertelement <16 x float> undef, float %376, i32 0
  %temp.vect316 = insertelement <16 x float> %temp.vect315, float %377, i32 1
  %temp.vect317 = insertelement <16 x float> %temp.vect316, float %378, i32 2
  %temp.vect318 = insertelement <16 x float> %temp.vect317, float %379, i32 3
  %temp.vect319 = insertelement <16 x float> %temp.vect318, float %380, i32 4
  %temp.vect320 = insertelement <16 x float> %temp.vect319, float %381, i32 5
  %temp.vect321 = insertelement <16 x float> %temp.vect320, float %382, i32 6
  %temp.vect322 = insertelement <16 x float> %temp.vect321, float %383, i32 7
  %temp.vect323 = insertelement <16 x float> %temp.vect322, float %384, i32 8
  %temp.vect324 = insertelement <16 x float> %temp.vect323, float %385, i32 9
  %temp.vect325 = insertelement <16 x float> %temp.vect324, float %386, i32 10
  %temp.vect326 = insertelement <16 x float> %temp.vect325, float %387, i32 11
  %temp.vect327 = insertelement <16 x float> %temp.vect326, float %388, i32 12
  %temp.vect328 = insertelement <16 x float> %temp.vect327, float %389, i32 13
  %temp.vect329 = insertelement <16 x float> %temp.vect328, float %390, i32 14
  %temp.vect330 = insertelement <16 x float> %temp.vect329, float %391, i32 15
  %392 = extractelement <4 x float> %328, i32 3
  %393 = extractelement <4 x float> %329, i32 3
  %394 = extractelement <4 x float> %330, i32 3
  %395 = extractelement <4 x float> %331, i32 3
  %396 = extractelement <4 x float> %332, i32 3
  %397 = extractelement <4 x float> %333, i32 3
  %398 = extractelement <4 x float> %334, i32 3
  %399 = extractelement <4 x float> %335, i32 3
  %400 = extractelement <4 x float> %336, i32 3
  %401 = extractelement <4 x float> %337, i32 3
  %402 = extractelement <4 x float> %338, i32 3
  %403 = extractelement <4 x float> %339, i32 3
  %404 = extractelement <4 x float> %340, i32 3
  %405 = extractelement <4 x float> %341, i32 3
  %406 = extractelement <4 x float> %342, i32 3
  %407 = extractelement <4 x float> %343, i32 3
  %temp.vect333 = insertelement <16 x float> undef, float %392, i32 0
  %temp.vect334 = insertelement <16 x float> %temp.vect333, float %393, i32 1
  %temp.vect335 = insertelement <16 x float> %temp.vect334, float %394, i32 2
  %temp.vect336 = insertelement <16 x float> %temp.vect335, float %395, i32 3
  %temp.vect337 = insertelement <16 x float> %temp.vect336, float %396, i32 4
  %temp.vect338 = insertelement <16 x float> %temp.vect337, float %397, i32 5
  %temp.vect339 = insertelement <16 x float> %temp.vect338, float %398, i32 6
  %temp.vect340 = insertelement <16 x float> %temp.vect339, float %399, i32 7
  %temp.vect341 = insertelement <16 x float> %temp.vect340, float %400, i32 8
  %temp.vect342 = insertelement <16 x float> %temp.vect341, float %401, i32 9
  %temp.vect343 = insertelement <16 x float> %temp.vect342, float %402, i32 10
  %temp.vect344 = insertelement <16 x float> %temp.vect343, float %403, i32 11
  %temp.vect345 = insertelement <16 x float> %temp.vect344, float %404, i32 12
  %temp.vect346 = insertelement <16 x float> %temp.vect345, float %405, i32 13
  %temp.vect347 = insertelement <16 x float> %temp.vect346, float %406, i32 14
  %temp.vect348 = insertelement <16 x float> %temp.vect347, float %407, i32 15
  %mul_dot36296 = fmul <16 x float> %temp.vect295, <float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000>
  %mul_dot37313 = fmul <16 x float> %temp.vect312, <float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000>
  %sum_dot38314 = fadd <16 x float> %mul_dot36296, %mul_dot37313
  %mul_dot39331 = fmul <16 x float> %temp.vect330, <float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000>
  %sum_dot40332 = fadd <16 x float> %sum_dot38314, %mul_dot39331
  %mul_dot41349 = fmul <16 x float> %temp.vect348, zeroinitializer
  %sum_dot42350 = fadd <16 x float> %sum_dot40332, %mul_dot41349
  %extract351 = extractelement <16 x float> %sum_dot42350, i32 0
  %extract352 = extractelement <16 x float> %sum_dot42350, i32 1
  %extract353 = extractelement <16 x float> %sum_dot42350, i32 2
  %extract354 = extractelement <16 x float> %sum_dot42350, i32 3
  %extract355 = extractelement <16 x float> %sum_dot42350, i32 4
  %extract356 = extractelement <16 x float> %sum_dot42350, i32 5
  %extract357 = extractelement <16 x float> %sum_dot42350, i32 6
  %extract358 = extractelement <16 x float> %sum_dot42350, i32 7
  %extract359 = extractelement <16 x float> %sum_dot42350, i32 8
  %extract360 = extractelement <16 x float> %sum_dot42350, i32 9
  %extract361 = extractelement <16 x float> %sum_dot42350, i32 10
  %extract362 = extractelement <16 x float> %sum_dot42350, i32 11
  %extract363 = extractelement <16 x float> %sum_dot42350, i32 12
  %extract364 = extractelement <16 x float> %sum_dot42350, i32 13
  %extract365 = extractelement <16 x float> %sum_dot42350, i32 14
  %extract366 = extractelement <16 x float> %sum_dot42350, i32 15
  %408 = load <4 x float> addrspace(1)* %24, align 16
  %409 = load <4 x float> addrspace(1)* %25, align 16
  %410 = load <4 x float> addrspace(1)* %26, align 16
  %411 = load <4 x float> addrspace(1)* %27, align 16
  %412 = load <4 x float> addrspace(1)* %28, align 16
  %413 = load <4 x float> addrspace(1)* %29, align 16
  %414 = load <4 x float> addrspace(1)* %30, align 16
  %415 = load <4 x float> addrspace(1)* %31, align 16
  %416 = load <4 x float> addrspace(1)* %32, align 16
  %417 = load <4 x float> addrspace(1)* %33, align 16
  %418 = load <4 x float> addrspace(1)* %34, align 16
  %419 = load <4 x float> addrspace(1)* %35, align 16
  %420 = load <4 x float> addrspace(1)* %36, align 16
  %421 = load <4 x float> addrspace(1)* %37, align 16
  %422 = load <4 x float> addrspace(1)* %38, align 16
  %423 = load <4 x float> addrspace(1)* %39, align 16
  %424 = extractelement <4 x float> %408, i32 0
  %425 = extractelement <4 x float> %409, i32 0
  %426 = extractelement <4 x float> %410, i32 0
  %427 = extractelement <4 x float> %411, i32 0
  %428 = extractelement <4 x float> %412, i32 0
  %429 = extractelement <4 x float> %413, i32 0
  %430 = extractelement <4 x float> %414, i32 0
  %431 = extractelement <4 x float> %415, i32 0
  %432 = extractelement <4 x float> %416, i32 0
  %433 = extractelement <4 x float> %417, i32 0
  %434 = extractelement <4 x float> %418, i32 0
  %435 = extractelement <4 x float> %419, i32 0
  %436 = extractelement <4 x float> %420, i32 0
  %437 = extractelement <4 x float> %421, i32 0
  %438 = extractelement <4 x float> %422, i32 0
  %439 = extractelement <4 x float> %423, i32 0
  %440 = extractelement <4 x float> %408, i32 1
  %441 = extractelement <4 x float> %409, i32 1
  %442 = extractelement <4 x float> %410, i32 1
  %443 = extractelement <4 x float> %411, i32 1
  %444 = extractelement <4 x float> %412, i32 1
  %445 = extractelement <4 x float> %413, i32 1
  %446 = extractelement <4 x float> %414, i32 1
  %447 = extractelement <4 x float> %415, i32 1
  %448 = extractelement <4 x float> %416, i32 1
  %449 = extractelement <4 x float> %417, i32 1
  %450 = extractelement <4 x float> %418, i32 1
  %451 = extractelement <4 x float> %419, i32 1
  %452 = extractelement <4 x float> %420, i32 1
  %453 = extractelement <4 x float> %421, i32 1
  %454 = extractelement <4 x float> %422, i32 1
  %455 = extractelement <4 x float> %423, i32 1
  %456 = insertelement <4 x float> undef, float %424, i32 0
  %457 = insertelement <4 x float> undef, float %425, i32 0
  %458 = insertelement <4 x float> undef, float %426, i32 0
  %459 = insertelement <4 x float> undef, float %427, i32 0
  %460 = insertelement <4 x float> undef, float %428, i32 0
  %461 = insertelement <4 x float> undef, float %429, i32 0
  %462 = insertelement <4 x float> undef, float %430, i32 0
  %463 = insertelement <4 x float> undef, float %431, i32 0
  %464 = insertelement <4 x float> undef, float %432, i32 0
  %465 = insertelement <4 x float> undef, float %433, i32 0
  %466 = insertelement <4 x float> undef, float %434, i32 0
  %467 = insertelement <4 x float> undef, float %435, i32 0
  %468 = insertelement <4 x float> undef, float %436, i32 0
  %469 = insertelement <4 x float> undef, float %437, i32 0
  %470 = insertelement <4 x float> undef, float %438, i32 0
  %471 = insertelement <4 x float> undef, float %439, i32 0
  %472 = insertelement <4 x float> %456, float %440, i32 1
  %473 = insertelement <4 x float> %457, float %441, i32 1
  %474 = insertelement <4 x float> %458, float %442, i32 1
  %475 = insertelement <4 x float> %459, float %443, i32 1
  %476 = insertelement <4 x float> %460, float %444, i32 1
  %477 = insertelement <4 x float> %461, float %445, i32 1
  %478 = insertelement <4 x float> %462, float %446, i32 1
  %479 = insertelement <4 x float> %463, float %447, i32 1
  %480 = insertelement <4 x float> %464, float %448, i32 1
  %481 = insertelement <4 x float> %465, float %449, i32 1
  %482 = insertelement <4 x float> %466, float %450, i32 1
  %483 = insertelement <4 x float> %467, float %451, i32 1
  %484 = insertelement <4 x float> %468, float %452, i32 1
  %485 = insertelement <4 x float> %469, float %453, i32 1
  %486 = insertelement <4 x float> %470, float %454, i32 1
  %487 = insertelement <4 x float> %471, float %455, i32 1
  %488 = insertelement <4 x float> %472, float %extract351, i32 2
  %489 = insertelement <4 x float> %473, float %extract352, i32 2
  %490 = insertelement <4 x float> %474, float %extract353, i32 2
  %491 = insertelement <4 x float> %475, float %extract354, i32 2
  %492 = insertelement <4 x float> %476, float %extract355, i32 2
  %493 = insertelement <4 x float> %477, float %extract356, i32 2
  %494 = insertelement <4 x float> %478, float %extract357, i32 2
  %495 = insertelement <4 x float> %479, float %extract358, i32 2
  %496 = insertelement <4 x float> %480, float %extract359, i32 2
  %497 = insertelement <4 x float> %481, float %extract360, i32 2
  %498 = insertelement <4 x float> %482, float %extract361, i32 2
  %499 = insertelement <4 x float> %483, float %extract362, i32 2
  %500 = insertelement <4 x float> %484, float %extract363, i32 2
  %501 = insertelement <4 x float> %485, float %extract364, i32 2
  %502 = insertelement <4 x float> %486, float %extract365, i32 2
  %503 = insertelement <4 x float> %487, float %extract366, i32 2
  %504 = shufflevector <4 x float> %488, <4 x float> %408, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %505 = shufflevector <4 x float> %489, <4 x float> %409, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %506 = shufflevector <4 x float> %490, <4 x float> %410, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %507 = shufflevector <4 x float> %491, <4 x float> %411, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %508 = shufflevector <4 x float> %492, <4 x float> %412, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %509 = shufflevector <4 x float> %493, <4 x float> %413, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %510 = shufflevector <4 x float> %494, <4 x float> %414, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %511 = shufflevector <4 x float> %495, <4 x float> %415, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %512 = shufflevector <4 x float> %496, <4 x float> %416, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %513 = shufflevector <4 x float> %497, <4 x float> %417, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %514 = shufflevector <4 x float> %498, <4 x float> %418, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %515 = shufflevector <4 x float> %499, <4 x float> %419, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %516 = shufflevector <4 x float> %500, <4 x float> %420, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %517 = shufflevector <4 x float> %501, <4 x float> %421, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %518 = shufflevector <4 x float> %502, <4 x float> %422, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %519 = shufflevector <4 x float> %503, <4 x float> %423, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %504, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %505, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %506, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %507, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %508, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %509, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %510, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %511, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %512, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %513, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %514, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %515, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %516, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %517, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %518, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %519, <4 x float> addrspace(1)* %39, align 16
  %520 = load <4 x float> addrspace(1)* %8, align 16
  %521 = load <4 x float> addrspace(1)* %9, align 16
  %522 = load <4 x float> addrspace(1)* %10, align 16
  %523 = load <4 x float> addrspace(1)* %11, align 16
  %524 = load <4 x float> addrspace(1)* %12, align 16
  %525 = load <4 x float> addrspace(1)* %13, align 16
  %526 = load <4 x float> addrspace(1)* %14, align 16
  %527 = load <4 x float> addrspace(1)* %15, align 16
  %528 = load <4 x float> addrspace(1)* %16, align 16
  %529 = load <4 x float> addrspace(1)* %17, align 16
  %530 = load <4 x float> addrspace(1)* %18, align 16
  %531 = load <4 x float> addrspace(1)* %19, align 16
  %532 = load <4 x float> addrspace(1)* %20, align 16
  %533 = load <4 x float> addrspace(1)* %21, align 16
  %534 = load <4 x float> addrspace(1)* %22, align 16
  %535 = load <4 x float> addrspace(1)* %23, align 16
  %536 = insertelement <4 x float> undef, float %424, i32 0
  %537 = insertelement <4 x float> undef, float %425, i32 0
  %538 = insertelement <4 x float> undef, float %426, i32 0
  %539 = insertelement <4 x float> undef, float %427, i32 0
  %540 = insertelement <4 x float> undef, float %428, i32 0
  %541 = insertelement <4 x float> undef, float %429, i32 0
  %542 = insertelement <4 x float> undef, float %430, i32 0
  %543 = insertelement <4 x float> undef, float %431, i32 0
  %544 = insertelement <4 x float> undef, float %432, i32 0
  %545 = insertelement <4 x float> undef, float %433, i32 0
  %546 = insertelement <4 x float> undef, float %434, i32 0
  %547 = insertelement <4 x float> undef, float %435, i32 0
  %548 = insertelement <4 x float> undef, float %436, i32 0
  %549 = insertelement <4 x float> undef, float %437, i32 0
  %550 = insertelement <4 x float> undef, float %438, i32 0
  %551 = insertelement <4 x float> undef, float %439, i32 0
  %552 = insertelement <4 x float> %536, float %440, i32 1
  %553 = insertelement <4 x float> %537, float %441, i32 1
  %554 = insertelement <4 x float> %538, float %442, i32 1
  %555 = insertelement <4 x float> %539, float %443, i32 1
  %556 = insertelement <4 x float> %540, float %444, i32 1
  %557 = insertelement <4 x float> %541, float %445, i32 1
  %558 = insertelement <4 x float> %542, float %446, i32 1
  %559 = insertelement <4 x float> %543, float %447, i32 1
  %560 = insertelement <4 x float> %544, float %448, i32 1
  %561 = insertelement <4 x float> %545, float %449, i32 1
  %562 = insertelement <4 x float> %546, float %450, i32 1
  %563 = insertelement <4 x float> %547, float %451, i32 1
  %564 = insertelement <4 x float> %548, float %452, i32 1
  %565 = insertelement <4 x float> %549, float %453, i32 1
  %566 = insertelement <4 x float> %550, float %454, i32 1
  %567 = insertelement <4 x float> %551, float %455, i32 1
  %568 = insertelement <4 x float> %552, float %extract351, i32 2
  %569 = insertelement <4 x float> %553, float %extract352, i32 2
  %570 = insertelement <4 x float> %554, float %extract353, i32 2
  %571 = insertelement <4 x float> %555, float %extract354, i32 2
  %572 = insertelement <4 x float> %556, float %extract355, i32 2
  %573 = insertelement <4 x float> %557, float %extract356, i32 2
  %574 = insertelement <4 x float> %558, float %extract357, i32 2
  %575 = insertelement <4 x float> %559, float %extract358, i32 2
  %576 = insertelement <4 x float> %560, float %extract359, i32 2
  %577 = insertelement <4 x float> %561, float %extract360, i32 2
  %578 = insertelement <4 x float> %562, float %extract361, i32 2
  %579 = insertelement <4 x float> %563, float %extract362, i32 2
  %580 = insertelement <4 x float> %564, float %extract363, i32 2
  %581 = insertelement <4 x float> %565, float %extract364, i32 2
  %582 = insertelement <4 x float> %566, float %extract365, i32 2
  %583 = insertelement <4 x float> %567, float %extract366, i32 2
  %584 = shufflevector <4 x float> %568, <4 x float> %520, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %585 = shufflevector <4 x float> %569, <4 x float> %521, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %586 = shufflevector <4 x float> %570, <4 x float> %522, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %587 = shufflevector <4 x float> %571, <4 x float> %523, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %588 = shufflevector <4 x float> %572, <4 x float> %524, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %589 = shufflevector <4 x float> %573, <4 x float> %525, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %590 = shufflevector <4 x float> %574, <4 x float> %526, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %591 = shufflevector <4 x float> %575, <4 x float> %527, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %592 = shufflevector <4 x float> %576, <4 x float> %528, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %593 = shufflevector <4 x float> %577, <4 x float> %529, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %594 = shufflevector <4 x float> %578, <4 x float> %530, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %595 = shufflevector <4 x float> %579, <4 x float> %531, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %596 = shufflevector <4 x float> %580, <4 x float> %532, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %597 = shufflevector <4 x float> %581, <4 x float> %533, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %598 = shufflevector <4 x float> %582, <4 x float> %534, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %599 = shufflevector <4 x float> %583, <4 x float> %535, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %584, <4 x float> addrspace(1)* %24, align 16
  store <4 x float> %585, <4 x float> addrspace(1)* %25, align 16
  store <4 x float> %586, <4 x float> addrspace(1)* %26, align 16
  store <4 x float> %587, <4 x float> addrspace(1)* %27, align 16
  store <4 x float> %588, <4 x float> addrspace(1)* %28, align 16
  store <4 x float> %589, <4 x float> addrspace(1)* %29, align 16
  store <4 x float> %590, <4 x float> addrspace(1)* %30, align 16
  store <4 x float> %591, <4 x float> addrspace(1)* %31, align 16
  store <4 x float> %592, <4 x float> addrspace(1)* %32, align 16
  store <4 x float> %593, <4 x float> addrspace(1)* %33, align 16
  store <4 x float> %594, <4 x float> addrspace(1)* %34, align 16
  store <4 x float> %595, <4 x float> addrspace(1)* %35, align 16
  store <4 x float> %596, <4 x float> addrspace(1)* %36, align 16
  store <4 x float> %597, <4 x float> addrspace(1)* %37, align 16
  store <4 x float> %598, <4 x float> addrspace(1)* %38, align 16
  store <4 x float> %599, <4 x float> addrspace(1)* %39, align 16
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp
  br i1 %exitcond, label %._crit_edge, label %7

._crit_edge:                                      ; preds = %7, %SyncBB367
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB367

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

declare <16 x float> @llvm.x86.mic.add.ps(<16 x float>, <16 x float>) nounwind readnone

declare <16 x float> @llvm.x86.mic.mul.ps(<16 x float>, <16 x float>) nounwind readnone

declare <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float>, i32) nounwind readnone

define void @intel_rgb2yuv_scalar(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x float> addrspace(1)**
  %4 = load <4 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = icmp sgt i32 %7, 0
  %tmp.i = zext i32 %7 to i64
  %tmp15.i = sext i32 %7 to i64
  br label %SyncBB21.i

SyncBB21.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %17, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB21.i
  %18 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %19 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %20 = load i64* %18, align 8
  %21 = load i64* %19, align 8
  %22 = add i64 %21, %20
  %tmp16.i = mul i64 %22, %tmp15.i
  %sext.i = shl i64 %tmp16.i, 32
  %tmp18.i = ashr i64 %sext.i, 32
  br label %23

; <label>:23                                      ; preds = %23, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %23 ]
  %tmp19.i = add i64 %tmp18.i, %indvar.i
  %scevgep.i = getelementptr <4 x float> addrspace(1)* %1, i64 %tmp19.i
  %scevgep20.i = getelementptr <4 x float> addrspace(1)* %4, i64 %tmp19.i
  %24 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %25 = extractelement <4 x float> %24, i32 0
  %26 = fmul float %25, 0x3FD322D0E0000000
  %27 = extractelement <4 x float> %24, i32 1
  %28 = fmul float %27, 0x3FE2C8B440000000
  %29 = fadd float %26, %28
  %30 = extractelement <4 x float> %24, i32 2
  %31 = fmul float %30, 0x3FBD2F1AA0000000
  %32 = fadd float %29, %31
  %33 = load <4 x float> addrspace(1)* %scevgep20.i, align 16
  %34 = insertelement <4 x float> %33, float %32, i32 0
  store <4 x float> %34, <4 x float> addrspace(1)* %scevgep20.i, align 16
  %35 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %36 = extractelement <4 x float> %35, i32 0
  %37 = fmul float %36, 0xBFC2D0E560000000
  %38 = extractelement <4 x float> %35, i32 1
  %39 = fmul float %38, 0x3FD27EF9E0000000
  %40 = fsub float %37, %39
  %41 = extractelement <4 x float> %35, i32 2
  %42 = fmul float %41, 0x3FDBE76C80000000
  %43 = fadd float %40, %42
  %44 = insertelement <4 x float> %34, float %43, i32 1
  store <4 x float> %44, <4 x float> addrspace(1)* %scevgep20.i, align 16
  %45 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %46 = extractelement <4 x float> %45, i32 0
  %47 = fmul float %46, 0x3FE3AE1480000000
  %48 = extractelement <4 x float> %45, i32 1
  %49 = fmul float %48, 0x3FE07AE140000000
  %50 = fsub float %47, %49
  %51 = extractelement <4 x float> %45, i32 2
  %52 = fmul float %51, 0x3FB99999A0000000
  %53 = fsub float %50, %52
  %54 = insertelement <4 x float> %44, float %53, i32 2
  store <4 x float> %54, <4 x float> addrspace(1)* %scevgep20.i, align 16
  %55 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %56 = shufflevector <4 x float> %54, <4 x float> %55, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %56, <4 x float> addrspace(1)* %scevgep20.i, align 16
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp.i
  br i1 %exitcond.i, label %._crit_edge.i, label %23

._crit_edge.i:                                    ; preds = %23, %SyncBB21.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__intel_rgb2yuv_scalar_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB21.i

__intel_rgb2yuv_scalar_separated_args.exit:       ; preds = %._crit_edge.i
  ret void
}

define void @intel_rgb2yuv_vector(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x float> addrspace(1)**
  %4 = load <4 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = icmp sgt i32 %7, 0
  %tmp.i = zext i32 %7 to i64
  %tmp9.i = sext i32 %7 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %17, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %18 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %19 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %20 = load i64* %18, align 8
  %21 = load i64* %19, align 8
  %22 = add i64 %21, %20
  %tmp10.i = mul i64 %22, %tmp9.i
  %sext.i = shl i64 %tmp10.i, 32
  %tmp12.i = ashr i64 %sext.i, 32
  br label %23

; <label>:23                                      ; preds = %23, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %23 ]
  %tmp13.i = add i64 %tmp12.i, %indvar.i
  %scevgep.i = getelementptr <4 x float> addrspace(1)* %1, i64 %tmp13.i
  %scevgep14.i = getelementptr <4 x float> addrspace(1)* %4, i64 %tmp13.i
  %24 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %tmp6.i.i = shufflevector <4 x float> %24, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp7.i.i = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp6.i.i, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %25 = call <16 x float> @llvm.x86.mic.mul.ps(<16 x float> <float 0x3FD322D0E0000000, float 0x3FE2C8B440000000, float 0x3FBD2F1AA0000000, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp7.i.i) nounwind
  %26 = call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %25, i32 1) nounwind
  %27 = call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %25, <16 x float> %26) nounwind
  %28 = call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %27, i32 2) nounwind
  %29 = call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %27, <16 x float> %28) nounwind
  %tmp25.i.i = extractelement <16 x float> %29, i32 0
  %30 = load <4 x float> addrspace(1)* %scevgep14.i, align 16
  %31 = insertelement <4 x float> %30, float %tmp25.i.i, i32 0
  store <4 x float> %31, <4 x float> addrspace(1)* %scevgep14.i, align 16
  %32 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %tmp6.i1.i = shufflevector <4 x float> %32, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp7.i2.i = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp6.i1.i, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %33 = call <16 x float> @llvm.x86.mic.mul.ps(<16 x float> <float 0xBFC2D0E560000000, float 0xBFD27EF9E0000000, float 0x3FDBE76C80000000, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp7.i2.i) nounwind
  %34 = call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %33, i32 1) nounwind
  %35 = call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %33, <16 x float> %34) nounwind
  %36 = call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %35, i32 2) nounwind
  %37 = call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %35, <16 x float> %36) nounwind
  %tmp25.i3.i = extractelement <16 x float> %37, i32 0
  %38 = load <4 x float> addrspace(1)* %scevgep14.i, align 16
  %39 = insertelement <4 x float> %38, float %tmp25.i3.i, i32 1
  store <4 x float> %39, <4 x float> addrspace(1)* %scevgep14.i, align 16
  %40 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %tmp6.i4.i = shufflevector <4 x float> %40, <4 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp7.i5.i = shufflevector <16 x float> <float undef, float undef, float undef, float undef, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp6.i4.i, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %41 = call <16 x float> @llvm.x86.mic.mul.ps(<16 x float> <float 0x3FE3AE1480000000, float 0xBFE07AE140000000, float 0xBFB99999A0000000, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00>, <16 x float> %tmp7.i5.i) nounwind
  %42 = call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %41, i32 1) nounwind
  %43 = call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %41, <16 x float> %42) nounwind
  %44 = call <16 x float> @llvm.x86.mic.swizzle.ps(<16 x float> %43, i32 2) nounwind
  %45 = call <16 x float> @llvm.x86.mic.add.ps(<16 x float> %43, <16 x float> %44) nounwind
  %tmp25.i6.i = extractelement <16 x float> %45, i32 0
  %46 = load <4 x float> addrspace(1)* %scevgep14.i, align 16
  %47 = insertelement <4 x float> %46, float %tmp25.i6.i, i32 2
  store <4 x float> %47, <4 x float> addrspace(1)* %scevgep14.i, align 16
  %48 = load <4 x float> addrspace(1)* %scevgep.i, align 16
  %49 = shufflevector <4 x float> %47, <4 x float> %48, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %49, <4 x float> addrspace(1)* %scevgep14.i, align 16
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp.i
  br i1 %exitcond.i, label %._crit_edge.i, label %23

._crit_edge.i:                                    ; preds = %23, %SyncBB.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__intel_rgb2yuv_vector_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__intel_rgb2yuv_vector_separated_args.exit:       ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.intel_rgb2yuv_scalar(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x float> addrspace(1)**
  %4 = load <4 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = icmp sgt i32 %7, 0
  %tmp.i = zext i32 %7 to i64
  %tmp15.i = sext i32 %7 to i64
  %temp.i = insertelement <16 x i64> undef, i64 %tmp15.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB248.i

SyncBB248.i:                                      ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %17, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB248.i
  %18 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %19 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %20 = load i64* %18, align 8
  %21 = load i64* %19, align 8
  %22 = add i64 %21, %20
  %broadcast1.i = insertelement <16 x i64> undef, i64 %22, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %23 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %tmp1635.i = mul <16 x i64> %23, %vector.i
  %sext36.i = shl <16 x i64> %tmp1635.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %tmp1837.i = ashr <16 x i64> %sext36.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  br label %24

; <label>:24                                      ; preds = %24, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %24 ]
  %temp38.i = insertelement <16 x i64> undef, i64 %indvar.i, i32 0
  %vector39.i = shufflevector <16 x i64> %temp38.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1940.i = add <16 x i64> %tmp1837.i, %vector39.i
  %extract.i = extractelement <16 x i64> %tmp1940.i, i32 0
  %extract41.i = extractelement <16 x i64> %tmp1940.i, i32 1
  %extract42.i = extractelement <16 x i64> %tmp1940.i, i32 2
  %extract43.i = extractelement <16 x i64> %tmp1940.i, i32 3
  %extract44.i = extractelement <16 x i64> %tmp1940.i, i32 4
  %extract45.i = extractelement <16 x i64> %tmp1940.i, i32 5
  %extract46.i = extractelement <16 x i64> %tmp1940.i, i32 6
  %extract47.i = extractelement <16 x i64> %tmp1940.i, i32 7
  %extract48.i = extractelement <16 x i64> %tmp1940.i, i32 8
  %extract49.i = extractelement <16 x i64> %tmp1940.i, i32 9
  %extract50.i = extractelement <16 x i64> %tmp1940.i, i32 10
  %extract51.i = extractelement <16 x i64> %tmp1940.i, i32 11
  %extract52.i = extractelement <16 x i64> %tmp1940.i, i32 12
  %extract53.i = extractelement <16 x i64> %tmp1940.i, i32 13
  %extract54.i = extractelement <16 x i64> %tmp1940.i, i32 14
  %extract55.i = extractelement <16 x i64> %tmp1940.i, i32 15
  %25 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract.i
  %26 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract41.i
  %27 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract42.i
  %28 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract43.i
  %29 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract44.i
  %30 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract45.i
  %31 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract46.i
  %32 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract47.i
  %33 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract48.i
  %34 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract49.i
  %35 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract50.i
  %36 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract51.i
  %37 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract52.i
  %38 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract53.i
  %39 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract54.i
  %40 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract55.i
  %41 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract.i
  %42 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract41.i
  %43 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract42.i
  %44 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract43.i
  %45 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract44.i
  %46 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract45.i
  %47 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract46.i
  %48 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract47.i
  %49 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract48.i
  %50 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract49.i
  %51 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract50.i
  %52 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract51.i
  %53 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract52.i
  %54 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract53.i
  %55 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract54.i
  %56 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract55.i
  %57 = load <4 x float> addrspace(1)* %25, align 16
  %58 = load <4 x float> addrspace(1)* %26, align 16
  %59 = load <4 x float> addrspace(1)* %27, align 16
  %60 = load <4 x float> addrspace(1)* %28, align 16
  %61 = load <4 x float> addrspace(1)* %29, align 16
  %62 = load <4 x float> addrspace(1)* %30, align 16
  %63 = load <4 x float> addrspace(1)* %31, align 16
  %64 = load <4 x float> addrspace(1)* %32, align 16
  %65 = load <4 x float> addrspace(1)* %33, align 16
  %66 = load <4 x float> addrspace(1)* %34, align 16
  %67 = load <4 x float> addrspace(1)* %35, align 16
  %68 = load <4 x float> addrspace(1)* %36, align 16
  %69 = load <4 x float> addrspace(1)* %37, align 16
  %70 = load <4 x float> addrspace(1)* %38, align 16
  %71 = load <4 x float> addrspace(1)* %39, align 16
  %72 = load <4 x float> addrspace(1)* %40, align 16
  %73 = extractelement <4 x float> %57, i32 0
  %74 = extractelement <4 x float> %58, i32 0
  %75 = extractelement <4 x float> %59, i32 0
  %76 = extractelement <4 x float> %60, i32 0
  %77 = extractelement <4 x float> %61, i32 0
  %78 = extractelement <4 x float> %62, i32 0
  %79 = extractelement <4 x float> %63, i32 0
  %80 = extractelement <4 x float> %64, i32 0
  %81 = extractelement <4 x float> %65, i32 0
  %82 = extractelement <4 x float> %66, i32 0
  %83 = extractelement <4 x float> %67, i32 0
  %84 = extractelement <4 x float> %68, i32 0
  %85 = extractelement <4 x float> %69, i32 0
  %86 = extractelement <4 x float> %70, i32 0
  %87 = extractelement <4 x float> %71, i32 0
  %88 = extractelement <4 x float> %72, i32 0
  %temp.vect56.i = insertelement <16 x float> undef, float %73, i32 0
  %temp.vect57.i = insertelement <16 x float> %temp.vect56.i, float %74, i32 1
  %temp.vect58.i = insertelement <16 x float> %temp.vect57.i, float %75, i32 2
  %temp.vect59.i = insertelement <16 x float> %temp.vect58.i, float %76, i32 3
  %temp.vect60.i = insertelement <16 x float> %temp.vect59.i, float %77, i32 4
  %temp.vect61.i = insertelement <16 x float> %temp.vect60.i, float %78, i32 5
  %temp.vect62.i = insertelement <16 x float> %temp.vect61.i, float %79, i32 6
  %temp.vect63.i = insertelement <16 x float> %temp.vect62.i, float %80, i32 7
  %temp.vect64.i = insertelement <16 x float> %temp.vect63.i, float %81, i32 8
  %temp.vect65.i = insertelement <16 x float> %temp.vect64.i, float %82, i32 9
  %temp.vect66.i = insertelement <16 x float> %temp.vect65.i, float %83, i32 10
  %temp.vect67.i = insertelement <16 x float> %temp.vect66.i, float %84, i32 11
  %temp.vect68.i = insertelement <16 x float> %temp.vect67.i, float %85, i32 12
  %temp.vect69.i = insertelement <16 x float> %temp.vect68.i, float %86, i32 13
  %temp.vect70.i = insertelement <16 x float> %temp.vect69.i, float %87, i32 14
  %temp.vect71.i = insertelement <16 x float> %temp.vect70.i, float %88, i32 15
  %89 = extractelement <4 x float> %57, i32 1
  %90 = extractelement <4 x float> %58, i32 1
  %91 = extractelement <4 x float> %59, i32 1
  %92 = extractelement <4 x float> %60, i32 1
  %93 = extractelement <4 x float> %61, i32 1
  %94 = extractelement <4 x float> %62, i32 1
  %95 = extractelement <4 x float> %63, i32 1
  %96 = extractelement <4 x float> %64, i32 1
  %97 = extractelement <4 x float> %65, i32 1
  %98 = extractelement <4 x float> %66, i32 1
  %99 = extractelement <4 x float> %67, i32 1
  %100 = extractelement <4 x float> %68, i32 1
  %101 = extractelement <4 x float> %69, i32 1
  %102 = extractelement <4 x float> %70, i32 1
  %103 = extractelement <4 x float> %71, i32 1
  %104 = extractelement <4 x float> %72, i32 1
  %temp.vect72.i = insertelement <16 x float> undef, float %89, i32 0
  %temp.vect73.i = insertelement <16 x float> %temp.vect72.i, float %90, i32 1
  %temp.vect74.i = insertelement <16 x float> %temp.vect73.i, float %91, i32 2
  %temp.vect75.i = insertelement <16 x float> %temp.vect74.i, float %92, i32 3
  %temp.vect76.i = insertelement <16 x float> %temp.vect75.i, float %93, i32 4
  %temp.vect77.i = insertelement <16 x float> %temp.vect76.i, float %94, i32 5
  %temp.vect78.i = insertelement <16 x float> %temp.vect77.i, float %95, i32 6
  %temp.vect79.i = insertelement <16 x float> %temp.vect78.i, float %96, i32 7
  %temp.vect80.i = insertelement <16 x float> %temp.vect79.i, float %97, i32 8
  %temp.vect81.i = insertelement <16 x float> %temp.vect80.i, float %98, i32 9
  %temp.vect82.i = insertelement <16 x float> %temp.vect81.i, float %99, i32 10
  %temp.vect83.i = insertelement <16 x float> %temp.vect82.i, float %100, i32 11
  %temp.vect84.i = insertelement <16 x float> %temp.vect83.i, float %101, i32 12
  %temp.vect85.i = insertelement <16 x float> %temp.vect84.i, float %102, i32 13
  %temp.vect86.i = insertelement <16 x float> %temp.vect85.i, float %103, i32 14
  %temp.vect87.i = insertelement <16 x float> %temp.vect86.i, float %104, i32 15
  %105 = extractelement <4 x float> %57, i32 2
  %106 = extractelement <4 x float> %58, i32 2
  %107 = extractelement <4 x float> %59, i32 2
  %108 = extractelement <4 x float> %60, i32 2
  %109 = extractelement <4 x float> %61, i32 2
  %110 = extractelement <4 x float> %62, i32 2
  %111 = extractelement <4 x float> %63, i32 2
  %112 = extractelement <4 x float> %64, i32 2
  %113 = extractelement <4 x float> %65, i32 2
  %114 = extractelement <4 x float> %66, i32 2
  %115 = extractelement <4 x float> %67, i32 2
  %116 = extractelement <4 x float> %68, i32 2
  %117 = extractelement <4 x float> %69, i32 2
  %118 = extractelement <4 x float> %70, i32 2
  %119 = extractelement <4 x float> %71, i32 2
  %120 = extractelement <4 x float> %72, i32 2
  %temp.vect88.i = insertelement <16 x float> undef, float %105, i32 0
  %temp.vect89.i = insertelement <16 x float> %temp.vect88.i, float %106, i32 1
  %temp.vect90.i = insertelement <16 x float> %temp.vect89.i, float %107, i32 2
  %temp.vect91.i = insertelement <16 x float> %temp.vect90.i, float %108, i32 3
  %temp.vect92.i = insertelement <16 x float> %temp.vect91.i, float %109, i32 4
  %temp.vect93.i = insertelement <16 x float> %temp.vect92.i, float %110, i32 5
  %temp.vect94.i = insertelement <16 x float> %temp.vect93.i, float %111, i32 6
  %temp.vect95.i = insertelement <16 x float> %temp.vect94.i, float %112, i32 7
  %temp.vect96.i = insertelement <16 x float> %temp.vect95.i, float %113, i32 8
  %temp.vect97.i = insertelement <16 x float> %temp.vect96.i, float %114, i32 9
  %temp.vect98.i = insertelement <16 x float> %temp.vect97.i, float %115, i32 10
  %temp.vect99.i = insertelement <16 x float> %temp.vect98.i, float %116, i32 11
  %temp.vect100.i = insertelement <16 x float> %temp.vect99.i, float %117, i32 12
  %temp.vect101.i = insertelement <16 x float> %temp.vect100.i, float %118, i32 13
  %temp.vect102.i = insertelement <16 x float> %temp.vect101.i, float %119, i32 14
  %temp.vect103.i = insertelement <16 x float> %temp.vect102.i, float %120, i32 15
  %121 = fmul <16 x float> %temp.vect71.i, <float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000>
  %122 = fmul <16 x float> %temp.vect87.i, <float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000>
  %123 = fadd <16 x float> %121, %122
  %124 = fmul <16 x float> %temp.vect103.i, <float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000>
  %125 = fadd <16 x float> %123, %124
  %extract104.i = extractelement <16 x float> %125, i32 0
  %extract105.i = extractelement <16 x float> %125, i32 1
  %extract106.i = extractelement <16 x float> %125, i32 2
  %extract107.i = extractelement <16 x float> %125, i32 3
  %extract108.i = extractelement <16 x float> %125, i32 4
  %extract109.i = extractelement <16 x float> %125, i32 5
  %extract110.i = extractelement <16 x float> %125, i32 6
  %extract111.i = extractelement <16 x float> %125, i32 7
  %extract112.i = extractelement <16 x float> %125, i32 8
  %extract113.i = extractelement <16 x float> %125, i32 9
  %extract114.i = extractelement <16 x float> %125, i32 10
  %extract115.i = extractelement <16 x float> %125, i32 11
  %extract116.i = extractelement <16 x float> %125, i32 12
  %extract117.i = extractelement <16 x float> %125, i32 13
  %extract118.i = extractelement <16 x float> %125, i32 14
  %extract119.i = extractelement <16 x float> %125, i32 15
  %126 = load <4 x float> addrspace(1)* %41, align 16
  %127 = load <4 x float> addrspace(1)* %42, align 16
  %128 = load <4 x float> addrspace(1)* %43, align 16
  %129 = load <4 x float> addrspace(1)* %44, align 16
  %130 = load <4 x float> addrspace(1)* %45, align 16
  %131 = load <4 x float> addrspace(1)* %46, align 16
  %132 = load <4 x float> addrspace(1)* %47, align 16
  %133 = load <4 x float> addrspace(1)* %48, align 16
  %134 = load <4 x float> addrspace(1)* %49, align 16
  %135 = load <4 x float> addrspace(1)* %50, align 16
  %136 = load <4 x float> addrspace(1)* %51, align 16
  %137 = load <4 x float> addrspace(1)* %52, align 16
  %138 = load <4 x float> addrspace(1)* %53, align 16
  %139 = load <4 x float> addrspace(1)* %54, align 16
  %140 = load <4 x float> addrspace(1)* %55, align 16
  %141 = load <4 x float> addrspace(1)* %56, align 16
  %142 = insertelement <4 x float> undef, float %extract104.i, i32 0
  %143 = insertelement <4 x float> undef, float %extract105.i, i32 0
  %144 = insertelement <4 x float> undef, float %extract106.i, i32 0
  %145 = insertelement <4 x float> undef, float %extract107.i, i32 0
  %146 = insertelement <4 x float> undef, float %extract108.i, i32 0
  %147 = insertelement <4 x float> undef, float %extract109.i, i32 0
  %148 = insertelement <4 x float> undef, float %extract110.i, i32 0
  %149 = insertelement <4 x float> undef, float %extract111.i, i32 0
  %150 = insertelement <4 x float> undef, float %extract112.i, i32 0
  %151 = insertelement <4 x float> undef, float %extract113.i, i32 0
  %152 = insertelement <4 x float> undef, float %extract114.i, i32 0
  %153 = insertelement <4 x float> undef, float %extract115.i, i32 0
  %154 = insertelement <4 x float> undef, float %extract116.i, i32 0
  %155 = insertelement <4 x float> undef, float %extract117.i, i32 0
  %156 = insertelement <4 x float> undef, float %extract118.i, i32 0
  %157 = insertelement <4 x float> undef, float %extract119.i, i32 0
  %158 = shufflevector <4 x float> %142, <4 x float> %126, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %159 = shufflevector <4 x float> %143, <4 x float> %127, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %160 = shufflevector <4 x float> %144, <4 x float> %128, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %161 = shufflevector <4 x float> %145, <4 x float> %129, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %162 = shufflevector <4 x float> %146, <4 x float> %130, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %163 = shufflevector <4 x float> %147, <4 x float> %131, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %164 = shufflevector <4 x float> %148, <4 x float> %132, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %165 = shufflevector <4 x float> %149, <4 x float> %133, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %166 = shufflevector <4 x float> %150, <4 x float> %134, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %167 = shufflevector <4 x float> %151, <4 x float> %135, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %168 = shufflevector <4 x float> %152, <4 x float> %136, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %169 = shufflevector <4 x float> %153, <4 x float> %137, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %170 = shufflevector <4 x float> %154, <4 x float> %138, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %171 = shufflevector <4 x float> %155, <4 x float> %139, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %172 = shufflevector <4 x float> %156, <4 x float> %140, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %173 = shufflevector <4 x float> %157, <4 x float> %141, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  store <4 x float> %158, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %159, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %160, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %161, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %162, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %163, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %164, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %165, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %166, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %167, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %168, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %169, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %170, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %171, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %172, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %173, <4 x float> addrspace(1)* %56, align 16
  %174 = load <4 x float> addrspace(1)* %25, align 16
  %175 = load <4 x float> addrspace(1)* %26, align 16
  %176 = load <4 x float> addrspace(1)* %27, align 16
  %177 = load <4 x float> addrspace(1)* %28, align 16
  %178 = load <4 x float> addrspace(1)* %29, align 16
  %179 = load <4 x float> addrspace(1)* %30, align 16
  %180 = load <4 x float> addrspace(1)* %31, align 16
  %181 = load <4 x float> addrspace(1)* %32, align 16
  %182 = load <4 x float> addrspace(1)* %33, align 16
  %183 = load <4 x float> addrspace(1)* %34, align 16
  %184 = load <4 x float> addrspace(1)* %35, align 16
  %185 = load <4 x float> addrspace(1)* %36, align 16
  %186 = load <4 x float> addrspace(1)* %37, align 16
  %187 = load <4 x float> addrspace(1)* %38, align 16
  %188 = load <4 x float> addrspace(1)* %39, align 16
  %189 = load <4 x float> addrspace(1)* %40, align 16
  %190 = extractelement <4 x float> %174, i32 0
  %191 = extractelement <4 x float> %175, i32 0
  %192 = extractelement <4 x float> %176, i32 0
  %193 = extractelement <4 x float> %177, i32 0
  %194 = extractelement <4 x float> %178, i32 0
  %195 = extractelement <4 x float> %179, i32 0
  %196 = extractelement <4 x float> %180, i32 0
  %197 = extractelement <4 x float> %181, i32 0
  %198 = extractelement <4 x float> %182, i32 0
  %199 = extractelement <4 x float> %183, i32 0
  %200 = extractelement <4 x float> %184, i32 0
  %201 = extractelement <4 x float> %185, i32 0
  %202 = extractelement <4 x float> %186, i32 0
  %203 = extractelement <4 x float> %187, i32 0
  %204 = extractelement <4 x float> %188, i32 0
  %205 = extractelement <4 x float> %189, i32 0
  %temp.vect120.i = insertelement <16 x float> undef, float %190, i32 0
  %temp.vect121.i = insertelement <16 x float> %temp.vect120.i, float %191, i32 1
  %temp.vect122.i = insertelement <16 x float> %temp.vect121.i, float %192, i32 2
  %temp.vect123.i = insertelement <16 x float> %temp.vect122.i, float %193, i32 3
  %temp.vect124.i = insertelement <16 x float> %temp.vect123.i, float %194, i32 4
  %temp.vect125.i = insertelement <16 x float> %temp.vect124.i, float %195, i32 5
  %temp.vect126.i = insertelement <16 x float> %temp.vect125.i, float %196, i32 6
  %temp.vect127.i = insertelement <16 x float> %temp.vect126.i, float %197, i32 7
  %temp.vect128.i = insertelement <16 x float> %temp.vect127.i, float %198, i32 8
  %temp.vect129.i = insertelement <16 x float> %temp.vect128.i, float %199, i32 9
  %temp.vect130.i = insertelement <16 x float> %temp.vect129.i, float %200, i32 10
  %temp.vect131.i = insertelement <16 x float> %temp.vect130.i, float %201, i32 11
  %temp.vect132.i = insertelement <16 x float> %temp.vect131.i, float %202, i32 12
  %temp.vect133.i = insertelement <16 x float> %temp.vect132.i, float %203, i32 13
  %temp.vect134.i = insertelement <16 x float> %temp.vect133.i, float %204, i32 14
  %temp.vect135.i = insertelement <16 x float> %temp.vect134.i, float %205, i32 15
  %206 = extractelement <4 x float> %174, i32 1
  %207 = extractelement <4 x float> %175, i32 1
  %208 = extractelement <4 x float> %176, i32 1
  %209 = extractelement <4 x float> %177, i32 1
  %210 = extractelement <4 x float> %178, i32 1
  %211 = extractelement <4 x float> %179, i32 1
  %212 = extractelement <4 x float> %180, i32 1
  %213 = extractelement <4 x float> %181, i32 1
  %214 = extractelement <4 x float> %182, i32 1
  %215 = extractelement <4 x float> %183, i32 1
  %216 = extractelement <4 x float> %184, i32 1
  %217 = extractelement <4 x float> %185, i32 1
  %218 = extractelement <4 x float> %186, i32 1
  %219 = extractelement <4 x float> %187, i32 1
  %220 = extractelement <4 x float> %188, i32 1
  %221 = extractelement <4 x float> %189, i32 1
  %temp.vect136.i = insertelement <16 x float> undef, float %206, i32 0
  %temp.vect137.i = insertelement <16 x float> %temp.vect136.i, float %207, i32 1
  %temp.vect138.i = insertelement <16 x float> %temp.vect137.i, float %208, i32 2
  %temp.vect139.i = insertelement <16 x float> %temp.vect138.i, float %209, i32 3
  %temp.vect140.i = insertelement <16 x float> %temp.vect139.i, float %210, i32 4
  %temp.vect141.i = insertelement <16 x float> %temp.vect140.i, float %211, i32 5
  %temp.vect142.i = insertelement <16 x float> %temp.vect141.i, float %212, i32 6
  %temp.vect143.i = insertelement <16 x float> %temp.vect142.i, float %213, i32 7
  %temp.vect144.i = insertelement <16 x float> %temp.vect143.i, float %214, i32 8
  %temp.vect145.i = insertelement <16 x float> %temp.vect144.i, float %215, i32 9
  %temp.vect146.i = insertelement <16 x float> %temp.vect145.i, float %216, i32 10
  %temp.vect147.i = insertelement <16 x float> %temp.vect146.i, float %217, i32 11
  %temp.vect148.i = insertelement <16 x float> %temp.vect147.i, float %218, i32 12
  %temp.vect149.i = insertelement <16 x float> %temp.vect148.i, float %219, i32 13
  %temp.vect150.i = insertelement <16 x float> %temp.vect149.i, float %220, i32 14
  %temp.vect151.i = insertelement <16 x float> %temp.vect150.i, float %221, i32 15
  %222 = extractelement <4 x float> %174, i32 2
  %223 = extractelement <4 x float> %175, i32 2
  %224 = extractelement <4 x float> %176, i32 2
  %225 = extractelement <4 x float> %177, i32 2
  %226 = extractelement <4 x float> %178, i32 2
  %227 = extractelement <4 x float> %179, i32 2
  %228 = extractelement <4 x float> %180, i32 2
  %229 = extractelement <4 x float> %181, i32 2
  %230 = extractelement <4 x float> %182, i32 2
  %231 = extractelement <4 x float> %183, i32 2
  %232 = extractelement <4 x float> %184, i32 2
  %233 = extractelement <4 x float> %185, i32 2
  %234 = extractelement <4 x float> %186, i32 2
  %235 = extractelement <4 x float> %187, i32 2
  %236 = extractelement <4 x float> %188, i32 2
  %237 = extractelement <4 x float> %189, i32 2
  %temp.vect152.i = insertelement <16 x float> undef, float %222, i32 0
  %temp.vect153.i = insertelement <16 x float> %temp.vect152.i, float %223, i32 1
  %temp.vect154.i = insertelement <16 x float> %temp.vect153.i, float %224, i32 2
  %temp.vect155.i = insertelement <16 x float> %temp.vect154.i, float %225, i32 3
  %temp.vect156.i = insertelement <16 x float> %temp.vect155.i, float %226, i32 4
  %temp.vect157.i = insertelement <16 x float> %temp.vect156.i, float %227, i32 5
  %temp.vect158.i = insertelement <16 x float> %temp.vect157.i, float %228, i32 6
  %temp.vect159.i = insertelement <16 x float> %temp.vect158.i, float %229, i32 7
  %temp.vect160.i = insertelement <16 x float> %temp.vect159.i, float %230, i32 8
  %temp.vect161.i = insertelement <16 x float> %temp.vect160.i, float %231, i32 9
  %temp.vect162.i = insertelement <16 x float> %temp.vect161.i, float %232, i32 10
  %temp.vect163.i = insertelement <16 x float> %temp.vect162.i, float %233, i32 11
  %temp.vect164.i = insertelement <16 x float> %temp.vect163.i, float %234, i32 12
  %temp.vect165.i = insertelement <16 x float> %temp.vect164.i, float %235, i32 13
  %temp.vect166.i = insertelement <16 x float> %temp.vect165.i, float %236, i32 14
  %temp.vect167.i = insertelement <16 x float> %temp.vect166.i, float %237, i32 15
  %238 = fmul <16 x float> %temp.vect135.i, <float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000>
  %239 = fmul <16 x float> %temp.vect151.i, <float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000, float 0x3FD27EF9E0000000>
  %240 = fsub <16 x float> %238, %239
  %241 = fmul <16 x float> %temp.vect167.i, <float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000>
  %242 = fadd <16 x float> %240, %241
  %extract168.i = extractelement <16 x float> %242, i32 0
  %extract169.i = extractelement <16 x float> %242, i32 1
  %extract170.i = extractelement <16 x float> %242, i32 2
  %extract171.i = extractelement <16 x float> %242, i32 3
  %extract172.i = extractelement <16 x float> %242, i32 4
  %extract173.i = extractelement <16 x float> %242, i32 5
  %extract174.i = extractelement <16 x float> %242, i32 6
  %extract175.i = extractelement <16 x float> %242, i32 7
  %extract176.i = extractelement <16 x float> %242, i32 8
  %extract177.i = extractelement <16 x float> %242, i32 9
  %extract178.i = extractelement <16 x float> %242, i32 10
  %extract179.i = extractelement <16 x float> %242, i32 11
  %extract180.i = extractelement <16 x float> %242, i32 12
  %extract181.i = extractelement <16 x float> %242, i32 13
  %extract182.i = extractelement <16 x float> %242, i32 14
  %extract183.i = extractelement <16 x float> %242, i32 15
  %243 = insertelement <4 x float> undef, float %extract104.i, i32 0
  %244 = insertelement <4 x float> undef, float %extract105.i, i32 0
  %245 = insertelement <4 x float> undef, float %extract106.i, i32 0
  %246 = insertelement <4 x float> undef, float %extract107.i, i32 0
  %247 = insertelement <4 x float> undef, float %extract108.i, i32 0
  %248 = insertelement <4 x float> undef, float %extract109.i, i32 0
  %249 = insertelement <4 x float> undef, float %extract110.i, i32 0
  %250 = insertelement <4 x float> undef, float %extract111.i, i32 0
  %251 = insertelement <4 x float> undef, float %extract112.i, i32 0
  %252 = insertelement <4 x float> undef, float %extract113.i, i32 0
  %253 = insertelement <4 x float> undef, float %extract114.i, i32 0
  %254 = insertelement <4 x float> undef, float %extract115.i, i32 0
  %255 = insertelement <4 x float> undef, float %extract116.i, i32 0
  %256 = insertelement <4 x float> undef, float %extract117.i, i32 0
  %257 = insertelement <4 x float> undef, float %extract118.i, i32 0
  %258 = insertelement <4 x float> undef, float %extract119.i, i32 0
  %259 = insertelement <4 x float> %243, float %extract168.i, i32 1
  %260 = insertelement <4 x float> %244, float %extract169.i, i32 1
  %261 = insertelement <4 x float> %245, float %extract170.i, i32 1
  %262 = insertelement <4 x float> %246, float %extract171.i, i32 1
  %263 = insertelement <4 x float> %247, float %extract172.i, i32 1
  %264 = insertelement <4 x float> %248, float %extract173.i, i32 1
  %265 = insertelement <4 x float> %249, float %extract174.i, i32 1
  %266 = insertelement <4 x float> %250, float %extract175.i, i32 1
  %267 = insertelement <4 x float> %251, float %extract176.i, i32 1
  %268 = insertelement <4 x float> %252, float %extract177.i, i32 1
  %269 = insertelement <4 x float> %253, float %extract178.i, i32 1
  %270 = insertelement <4 x float> %254, float %extract179.i, i32 1
  %271 = insertelement <4 x float> %255, float %extract180.i, i32 1
  %272 = insertelement <4 x float> %256, float %extract181.i, i32 1
  %273 = insertelement <4 x float> %257, float %extract182.i, i32 1
  %274 = insertelement <4 x float> %258, float %extract183.i, i32 1
  %275 = shufflevector <4 x float> %259, <4 x float> %126, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %276 = shufflevector <4 x float> %260, <4 x float> %127, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %277 = shufflevector <4 x float> %261, <4 x float> %128, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %278 = shufflevector <4 x float> %262, <4 x float> %129, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %279 = shufflevector <4 x float> %263, <4 x float> %130, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %280 = shufflevector <4 x float> %264, <4 x float> %131, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %281 = shufflevector <4 x float> %265, <4 x float> %132, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %282 = shufflevector <4 x float> %266, <4 x float> %133, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %283 = shufflevector <4 x float> %267, <4 x float> %134, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %284 = shufflevector <4 x float> %268, <4 x float> %135, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %285 = shufflevector <4 x float> %269, <4 x float> %136, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %286 = shufflevector <4 x float> %270, <4 x float> %137, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %287 = shufflevector <4 x float> %271, <4 x float> %138, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %288 = shufflevector <4 x float> %272, <4 x float> %139, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %289 = shufflevector <4 x float> %273, <4 x float> %140, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %290 = shufflevector <4 x float> %274, <4 x float> %141, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  store <4 x float> %275, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %276, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %277, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %278, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %279, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %280, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %281, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %282, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %283, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %284, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %285, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %286, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %287, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %288, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %289, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %290, <4 x float> addrspace(1)* %56, align 16
  %291 = load <4 x float> addrspace(1)* %25, align 16
  %292 = load <4 x float> addrspace(1)* %26, align 16
  %293 = load <4 x float> addrspace(1)* %27, align 16
  %294 = load <4 x float> addrspace(1)* %28, align 16
  %295 = load <4 x float> addrspace(1)* %29, align 16
  %296 = load <4 x float> addrspace(1)* %30, align 16
  %297 = load <4 x float> addrspace(1)* %31, align 16
  %298 = load <4 x float> addrspace(1)* %32, align 16
  %299 = load <4 x float> addrspace(1)* %33, align 16
  %300 = load <4 x float> addrspace(1)* %34, align 16
  %301 = load <4 x float> addrspace(1)* %35, align 16
  %302 = load <4 x float> addrspace(1)* %36, align 16
  %303 = load <4 x float> addrspace(1)* %37, align 16
  %304 = load <4 x float> addrspace(1)* %38, align 16
  %305 = load <4 x float> addrspace(1)* %39, align 16
  %306 = load <4 x float> addrspace(1)* %40, align 16
  %307 = extractelement <4 x float> %291, i32 0
  %308 = extractelement <4 x float> %292, i32 0
  %309 = extractelement <4 x float> %293, i32 0
  %310 = extractelement <4 x float> %294, i32 0
  %311 = extractelement <4 x float> %295, i32 0
  %312 = extractelement <4 x float> %296, i32 0
  %313 = extractelement <4 x float> %297, i32 0
  %314 = extractelement <4 x float> %298, i32 0
  %315 = extractelement <4 x float> %299, i32 0
  %316 = extractelement <4 x float> %300, i32 0
  %317 = extractelement <4 x float> %301, i32 0
  %318 = extractelement <4 x float> %302, i32 0
  %319 = extractelement <4 x float> %303, i32 0
  %320 = extractelement <4 x float> %304, i32 0
  %321 = extractelement <4 x float> %305, i32 0
  %322 = extractelement <4 x float> %306, i32 0
  %temp.vect184.i = insertelement <16 x float> undef, float %307, i32 0
  %temp.vect185.i = insertelement <16 x float> %temp.vect184.i, float %308, i32 1
  %temp.vect186.i = insertelement <16 x float> %temp.vect185.i, float %309, i32 2
  %temp.vect187.i = insertelement <16 x float> %temp.vect186.i, float %310, i32 3
  %temp.vect188.i = insertelement <16 x float> %temp.vect187.i, float %311, i32 4
  %temp.vect189.i = insertelement <16 x float> %temp.vect188.i, float %312, i32 5
  %temp.vect190.i = insertelement <16 x float> %temp.vect189.i, float %313, i32 6
  %temp.vect191.i = insertelement <16 x float> %temp.vect190.i, float %314, i32 7
  %temp.vect192.i = insertelement <16 x float> %temp.vect191.i, float %315, i32 8
  %temp.vect193.i = insertelement <16 x float> %temp.vect192.i, float %316, i32 9
  %temp.vect194.i = insertelement <16 x float> %temp.vect193.i, float %317, i32 10
  %temp.vect195.i = insertelement <16 x float> %temp.vect194.i, float %318, i32 11
  %temp.vect196.i = insertelement <16 x float> %temp.vect195.i, float %319, i32 12
  %temp.vect197.i = insertelement <16 x float> %temp.vect196.i, float %320, i32 13
  %temp.vect198.i = insertelement <16 x float> %temp.vect197.i, float %321, i32 14
  %temp.vect199.i = insertelement <16 x float> %temp.vect198.i, float %322, i32 15
  %323 = extractelement <4 x float> %291, i32 1
  %324 = extractelement <4 x float> %292, i32 1
  %325 = extractelement <4 x float> %293, i32 1
  %326 = extractelement <4 x float> %294, i32 1
  %327 = extractelement <4 x float> %295, i32 1
  %328 = extractelement <4 x float> %296, i32 1
  %329 = extractelement <4 x float> %297, i32 1
  %330 = extractelement <4 x float> %298, i32 1
  %331 = extractelement <4 x float> %299, i32 1
  %332 = extractelement <4 x float> %300, i32 1
  %333 = extractelement <4 x float> %301, i32 1
  %334 = extractelement <4 x float> %302, i32 1
  %335 = extractelement <4 x float> %303, i32 1
  %336 = extractelement <4 x float> %304, i32 1
  %337 = extractelement <4 x float> %305, i32 1
  %338 = extractelement <4 x float> %306, i32 1
  %temp.vect200.i = insertelement <16 x float> undef, float %323, i32 0
  %temp.vect201.i = insertelement <16 x float> %temp.vect200.i, float %324, i32 1
  %temp.vect202.i = insertelement <16 x float> %temp.vect201.i, float %325, i32 2
  %temp.vect203.i = insertelement <16 x float> %temp.vect202.i, float %326, i32 3
  %temp.vect204.i = insertelement <16 x float> %temp.vect203.i, float %327, i32 4
  %temp.vect205.i = insertelement <16 x float> %temp.vect204.i, float %328, i32 5
  %temp.vect206.i = insertelement <16 x float> %temp.vect205.i, float %329, i32 6
  %temp.vect207.i = insertelement <16 x float> %temp.vect206.i, float %330, i32 7
  %temp.vect208.i = insertelement <16 x float> %temp.vect207.i, float %331, i32 8
  %temp.vect209.i = insertelement <16 x float> %temp.vect208.i, float %332, i32 9
  %temp.vect210.i = insertelement <16 x float> %temp.vect209.i, float %333, i32 10
  %temp.vect211.i = insertelement <16 x float> %temp.vect210.i, float %334, i32 11
  %temp.vect212.i = insertelement <16 x float> %temp.vect211.i, float %335, i32 12
  %temp.vect213.i = insertelement <16 x float> %temp.vect212.i, float %336, i32 13
  %temp.vect214.i = insertelement <16 x float> %temp.vect213.i, float %337, i32 14
  %temp.vect215.i = insertelement <16 x float> %temp.vect214.i, float %338, i32 15
  %339 = extractelement <4 x float> %291, i32 2
  %340 = extractelement <4 x float> %292, i32 2
  %341 = extractelement <4 x float> %293, i32 2
  %342 = extractelement <4 x float> %294, i32 2
  %343 = extractelement <4 x float> %295, i32 2
  %344 = extractelement <4 x float> %296, i32 2
  %345 = extractelement <4 x float> %297, i32 2
  %346 = extractelement <4 x float> %298, i32 2
  %347 = extractelement <4 x float> %299, i32 2
  %348 = extractelement <4 x float> %300, i32 2
  %349 = extractelement <4 x float> %301, i32 2
  %350 = extractelement <4 x float> %302, i32 2
  %351 = extractelement <4 x float> %303, i32 2
  %352 = extractelement <4 x float> %304, i32 2
  %353 = extractelement <4 x float> %305, i32 2
  %354 = extractelement <4 x float> %306, i32 2
  %temp.vect216.i = insertelement <16 x float> undef, float %339, i32 0
  %temp.vect217.i = insertelement <16 x float> %temp.vect216.i, float %340, i32 1
  %temp.vect218.i = insertelement <16 x float> %temp.vect217.i, float %341, i32 2
  %temp.vect219.i = insertelement <16 x float> %temp.vect218.i, float %342, i32 3
  %temp.vect220.i = insertelement <16 x float> %temp.vect219.i, float %343, i32 4
  %temp.vect221.i = insertelement <16 x float> %temp.vect220.i, float %344, i32 5
  %temp.vect222.i = insertelement <16 x float> %temp.vect221.i, float %345, i32 6
  %temp.vect223.i = insertelement <16 x float> %temp.vect222.i, float %346, i32 7
  %temp.vect224.i = insertelement <16 x float> %temp.vect223.i, float %347, i32 8
  %temp.vect225.i = insertelement <16 x float> %temp.vect224.i, float %348, i32 9
  %temp.vect226.i = insertelement <16 x float> %temp.vect225.i, float %349, i32 10
  %temp.vect227.i = insertelement <16 x float> %temp.vect226.i, float %350, i32 11
  %temp.vect228.i = insertelement <16 x float> %temp.vect227.i, float %351, i32 12
  %temp.vect229.i = insertelement <16 x float> %temp.vect228.i, float %352, i32 13
  %temp.vect230.i = insertelement <16 x float> %temp.vect229.i, float %353, i32 14
  %temp.vect231.i = insertelement <16 x float> %temp.vect230.i, float %354, i32 15
  %355 = fmul <16 x float> %temp.vect199.i, <float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000>
  %356 = fmul <16 x float> %temp.vect215.i, <float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000, float 0x3FE07AE140000000>
  %357 = fsub <16 x float> %355, %356
  %358 = fmul <16 x float> %temp.vect231.i, <float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000, float 0x3FB99999A0000000>
  %359 = fsub <16 x float> %357, %358
  %extract232.i = extractelement <16 x float> %359, i32 0
  %extract233.i = extractelement <16 x float> %359, i32 1
  %extract234.i = extractelement <16 x float> %359, i32 2
  %extract235.i = extractelement <16 x float> %359, i32 3
  %extract236.i = extractelement <16 x float> %359, i32 4
  %extract237.i = extractelement <16 x float> %359, i32 5
  %extract238.i = extractelement <16 x float> %359, i32 6
  %extract239.i = extractelement <16 x float> %359, i32 7
  %extract240.i = extractelement <16 x float> %359, i32 8
  %extract241.i = extractelement <16 x float> %359, i32 9
  %extract242.i = extractelement <16 x float> %359, i32 10
  %extract243.i = extractelement <16 x float> %359, i32 11
  %extract244.i = extractelement <16 x float> %359, i32 12
  %extract245.i = extractelement <16 x float> %359, i32 13
  %extract246.i = extractelement <16 x float> %359, i32 14
  %extract247.i = extractelement <16 x float> %359, i32 15
  %360 = insertelement <4 x float> undef, float %extract104.i, i32 0
  %361 = insertelement <4 x float> undef, float %extract105.i, i32 0
  %362 = insertelement <4 x float> undef, float %extract106.i, i32 0
  %363 = insertelement <4 x float> undef, float %extract107.i, i32 0
  %364 = insertelement <4 x float> undef, float %extract108.i, i32 0
  %365 = insertelement <4 x float> undef, float %extract109.i, i32 0
  %366 = insertelement <4 x float> undef, float %extract110.i, i32 0
  %367 = insertelement <4 x float> undef, float %extract111.i, i32 0
  %368 = insertelement <4 x float> undef, float %extract112.i, i32 0
  %369 = insertelement <4 x float> undef, float %extract113.i, i32 0
  %370 = insertelement <4 x float> undef, float %extract114.i, i32 0
  %371 = insertelement <4 x float> undef, float %extract115.i, i32 0
  %372 = insertelement <4 x float> undef, float %extract116.i, i32 0
  %373 = insertelement <4 x float> undef, float %extract117.i, i32 0
  %374 = insertelement <4 x float> undef, float %extract118.i, i32 0
  %375 = insertelement <4 x float> undef, float %extract119.i, i32 0
  %376 = insertelement <4 x float> %360, float %extract168.i, i32 1
  %377 = insertelement <4 x float> %361, float %extract169.i, i32 1
  %378 = insertelement <4 x float> %362, float %extract170.i, i32 1
  %379 = insertelement <4 x float> %363, float %extract171.i, i32 1
  %380 = insertelement <4 x float> %364, float %extract172.i, i32 1
  %381 = insertelement <4 x float> %365, float %extract173.i, i32 1
  %382 = insertelement <4 x float> %366, float %extract174.i, i32 1
  %383 = insertelement <4 x float> %367, float %extract175.i, i32 1
  %384 = insertelement <4 x float> %368, float %extract176.i, i32 1
  %385 = insertelement <4 x float> %369, float %extract177.i, i32 1
  %386 = insertelement <4 x float> %370, float %extract178.i, i32 1
  %387 = insertelement <4 x float> %371, float %extract179.i, i32 1
  %388 = insertelement <4 x float> %372, float %extract180.i, i32 1
  %389 = insertelement <4 x float> %373, float %extract181.i, i32 1
  %390 = insertelement <4 x float> %374, float %extract182.i, i32 1
  %391 = insertelement <4 x float> %375, float %extract183.i, i32 1
  %392 = insertelement <4 x float> %376, float %extract232.i, i32 2
  %393 = insertelement <4 x float> %377, float %extract233.i, i32 2
  %394 = insertelement <4 x float> %378, float %extract234.i, i32 2
  %395 = insertelement <4 x float> %379, float %extract235.i, i32 2
  %396 = insertelement <4 x float> %380, float %extract236.i, i32 2
  %397 = insertelement <4 x float> %381, float %extract237.i, i32 2
  %398 = insertelement <4 x float> %382, float %extract238.i, i32 2
  %399 = insertelement <4 x float> %383, float %extract239.i, i32 2
  %400 = insertelement <4 x float> %384, float %extract240.i, i32 2
  %401 = insertelement <4 x float> %385, float %extract241.i, i32 2
  %402 = insertelement <4 x float> %386, float %extract242.i, i32 2
  %403 = insertelement <4 x float> %387, float %extract243.i, i32 2
  %404 = insertelement <4 x float> %388, float %extract244.i, i32 2
  %405 = insertelement <4 x float> %389, float %extract245.i, i32 2
  %406 = insertelement <4 x float> %390, float %extract246.i, i32 2
  %407 = insertelement <4 x float> %391, float %extract247.i, i32 2
  %408 = shufflevector <4 x float> %392, <4 x float> %126, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %409 = shufflevector <4 x float> %393, <4 x float> %127, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %410 = shufflevector <4 x float> %394, <4 x float> %128, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %411 = shufflevector <4 x float> %395, <4 x float> %129, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %412 = shufflevector <4 x float> %396, <4 x float> %130, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %413 = shufflevector <4 x float> %397, <4 x float> %131, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %414 = shufflevector <4 x float> %398, <4 x float> %132, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %415 = shufflevector <4 x float> %399, <4 x float> %133, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %416 = shufflevector <4 x float> %400, <4 x float> %134, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %417 = shufflevector <4 x float> %401, <4 x float> %135, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %418 = shufflevector <4 x float> %402, <4 x float> %136, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %419 = shufflevector <4 x float> %403, <4 x float> %137, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %420 = shufflevector <4 x float> %404, <4 x float> %138, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %421 = shufflevector <4 x float> %405, <4 x float> %139, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %422 = shufflevector <4 x float> %406, <4 x float> %140, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %423 = shufflevector <4 x float> %407, <4 x float> %141, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %408, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %409, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %410, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %411, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %412, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %413, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %414, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %415, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %416, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %417, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %418, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %419, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %420, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %421, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %422, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %423, <4 x float> addrspace(1)* %56, align 16
  %424 = load <4 x float> addrspace(1)* %25, align 16
  %425 = load <4 x float> addrspace(1)* %26, align 16
  %426 = load <4 x float> addrspace(1)* %27, align 16
  %427 = load <4 x float> addrspace(1)* %28, align 16
  %428 = load <4 x float> addrspace(1)* %29, align 16
  %429 = load <4 x float> addrspace(1)* %30, align 16
  %430 = load <4 x float> addrspace(1)* %31, align 16
  %431 = load <4 x float> addrspace(1)* %32, align 16
  %432 = load <4 x float> addrspace(1)* %33, align 16
  %433 = load <4 x float> addrspace(1)* %34, align 16
  %434 = load <4 x float> addrspace(1)* %35, align 16
  %435 = load <4 x float> addrspace(1)* %36, align 16
  %436 = load <4 x float> addrspace(1)* %37, align 16
  %437 = load <4 x float> addrspace(1)* %38, align 16
  %438 = load <4 x float> addrspace(1)* %39, align 16
  %439 = load <4 x float> addrspace(1)* %40, align 16
  %440 = insertelement <4 x float> undef, float %extract104.i, i32 0
  %441 = insertelement <4 x float> undef, float %extract105.i, i32 0
  %442 = insertelement <4 x float> undef, float %extract106.i, i32 0
  %443 = insertelement <4 x float> undef, float %extract107.i, i32 0
  %444 = insertelement <4 x float> undef, float %extract108.i, i32 0
  %445 = insertelement <4 x float> undef, float %extract109.i, i32 0
  %446 = insertelement <4 x float> undef, float %extract110.i, i32 0
  %447 = insertelement <4 x float> undef, float %extract111.i, i32 0
  %448 = insertelement <4 x float> undef, float %extract112.i, i32 0
  %449 = insertelement <4 x float> undef, float %extract113.i, i32 0
  %450 = insertelement <4 x float> undef, float %extract114.i, i32 0
  %451 = insertelement <4 x float> undef, float %extract115.i, i32 0
  %452 = insertelement <4 x float> undef, float %extract116.i, i32 0
  %453 = insertelement <4 x float> undef, float %extract117.i, i32 0
  %454 = insertelement <4 x float> undef, float %extract118.i, i32 0
  %455 = insertelement <4 x float> undef, float %extract119.i, i32 0
  %456 = insertelement <4 x float> %440, float %extract168.i, i32 1
  %457 = insertelement <4 x float> %441, float %extract169.i, i32 1
  %458 = insertelement <4 x float> %442, float %extract170.i, i32 1
  %459 = insertelement <4 x float> %443, float %extract171.i, i32 1
  %460 = insertelement <4 x float> %444, float %extract172.i, i32 1
  %461 = insertelement <4 x float> %445, float %extract173.i, i32 1
  %462 = insertelement <4 x float> %446, float %extract174.i, i32 1
  %463 = insertelement <4 x float> %447, float %extract175.i, i32 1
  %464 = insertelement <4 x float> %448, float %extract176.i, i32 1
  %465 = insertelement <4 x float> %449, float %extract177.i, i32 1
  %466 = insertelement <4 x float> %450, float %extract178.i, i32 1
  %467 = insertelement <4 x float> %451, float %extract179.i, i32 1
  %468 = insertelement <4 x float> %452, float %extract180.i, i32 1
  %469 = insertelement <4 x float> %453, float %extract181.i, i32 1
  %470 = insertelement <4 x float> %454, float %extract182.i, i32 1
  %471 = insertelement <4 x float> %455, float %extract183.i, i32 1
  %472 = insertelement <4 x float> %456, float %extract232.i, i32 2
  %473 = insertelement <4 x float> %457, float %extract233.i, i32 2
  %474 = insertelement <4 x float> %458, float %extract234.i, i32 2
  %475 = insertelement <4 x float> %459, float %extract235.i, i32 2
  %476 = insertelement <4 x float> %460, float %extract236.i, i32 2
  %477 = insertelement <4 x float> %461, float %extract237.i, i32 2
  %478 = insertelement <4 x float> %462, float %extract238.i, i32 2
  %479 = insertelement <4 x float> %463, float %extract239.i, i32 2
  %480 = insertelement <4 x float> %464, float %extract240.i, i32 2
  %481 = insertelement <4 x float> %465, float %extract241.i, i32 2
  %482 = insertelement <4 x float> %466, float %extract242.i, i32 2
  %483 = insertelement <4 x float> %467, float %extract243.i, i32 2
  %484 = insertelement <4 x float> %468, float %extract244.i, i32 2
  %485 = insertelement <4 x float> %469, float %extract245.i, i32 2
  %486 = insertelement <4 x float> %470, float %extract246.i, i32 2
  %487 = insertelement <4 x float> %471, float %extract247.i, i32 2
  %488 = shufflevector <4 x float> %472, <4 x float> %424, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %489 = shufflevector <4 x float> %473, <4 x float> %425, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %490 = shufflevector <4 x float> %474, <4 x float> %426, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %491 = shufflevector <4 x float> %475, <4 x float> %427, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %492 = shufflevector <4 x float> %476, <4 x float> %428, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %493 = shufflevector <4 x float> %477, <4 x float> %429, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %494 = shufflevector <4 x float> %478, <4 x float> %430, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %495 = shufflevector <4 x float> %479, <4 x float> %431, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %496 = shufflevector <4 x float> %480, <4 x float> %432, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %497 = shufflevector <4 x float> %481, <4 x float> %433, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %498 = shufflevector <4 x float> %482, <4 x float> %434, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %499 = shufflevector <4 x float> %483, <4 x float> %435, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %500 = shufflevector <4 x float> %484, <4 x float> %436, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %501 = shufflevector <4 x float> %485, <4 x float> %437, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %502 = shufflevector <4 x float> %486, <4 x float> %438, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %503 = shufflevector <4 x float> %487, <4 x float> %439, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %488, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %489, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %490, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %491, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %492, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %493, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %494, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %495, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %496, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %497, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %498, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %499, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %500, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %501, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %502, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %503, <4 x float> addrspace(1)* %56, align 16
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp.i
  br i1 %exitcond.i, label %._crit_edge.i, label %24

._crit_edge.i:                                    ; preds = %24, %SyncBB248.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.intel_rgb2yuv_scalar_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB248.i

____Vectorized_.intel_rgb2yuv_scalar_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.intel_rgb2yuv_vector(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x float> addrspace(1)**
  %1 = load <4 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x float> addrspace(1)**
  %4 = load <4 x float> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = icmp sgt i32 %7, 0
  %tmp.i = zext i32 %7 to i64
  %tmp9.i = sext i32 %7 to i64
  %temp.i = insertelement <16 x i64> undef, i64 %tmp9.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB367.i

SyncBB367.i:                                      ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %17, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB367.i
  %18 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %19 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %20 = load i64* %18, align 8
  %21 = load i64* %19, align 8
  %22 = add i64 %21, %20
  %broadcast1.i = insertelement <16 x i64> undef, i64 %22, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %23 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %tmp1085.i = mul <16 x i64> %23, %vector.i
  %sext86.i = shl <16 x i64> %tmp1085.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %tmp1287.i = ashr <16 x i64> %sext86.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  br label %24

; <label>:24                                      ; preds = %24, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %24 ]
  %temp88.i = insertelement <16 x i64> undef, i64 %indvar.i, i32 0
  %vector89.i = shufflevector <16 x i64> %temp88.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1390.i = add <16 x i64> %tmp1287.i, %vector89.i
  %extract.i = extractelement <16 x i64> %tmp1390.i, i32 0
  %extract91.i = extractelement <16 x i64> %tmp1390.i, i32 1
  %extract92.i = extractelement <16 x i64> %tmp1390.i, i32 2
  %extract93.i = extractelement <16 x i64> %tmp1390.i, i32 3
  %extract94.i = extractelement <16 x i64> %tmp1390.i, i32 4
  %extract95.i = extractelement <16 x i64> %tmp1390.i, i32 5
  %extract96.i = extractelement <16 x i64> %tmp1390.i, i32 6
  %extract97.i = extractelement <16 x i64> %tmp1390.i, i32 7
  %extract98.i = extractelement <16 x i64> %tmp1390.i, i32 8
  %extract99.i = extractelement <16 x i64> %tmp1390.i, i32 9
  %extract100.i = extractelement <16 x i64> %tmp1390.i, i32 10
  %extract101.i = extractelement <16 x i64> %tmp1390.i, i32 11
  %extract102.i = extractelement <16 x i64> %tmp1390.i, i32 12
  %extract103.i = extractelement <16 x i64> %tmp1390.i, i32 13
  %extract104.i = extractelement <16 x i64> %tmp1390.i, i32 14
  %extract105.i = extractelement <16 x i64> %tmp1390.i, i32 15
  %25 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract.i
  %26 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract91.i
  %27 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract92.i
  %28 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract93.i
  %29 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract94.i
  %30 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract95.i
  %31 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract96.i
  %32 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract97.i
  %33 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract98.i
  %34 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract99.i
  %35 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract100.i
  %36 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract101.i
  %37 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract102.i
  %38 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract103.i
  %39 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract104.i
  %40 = getelementptr <4 x float> addrspace(1)* %1, i64 %extract105.i
  %41 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract.i
  %42 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract91.i
  %43 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract92.i
  %44 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract93.i
  %45 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract94.i
  %46 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract95.i
  %47 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract96.i
  %48 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract97.i
  %49 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract98.i
  %50 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract99.i
  %51 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract100.i
  %52 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract101.i
  %53 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract102.i
  %54 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract103.i
  %55 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract104.i
  %56 = getelementptr <4 x float> addrspace(1)* %4, i64 %extract105.i
  %57 = load <4 x float> addrspace(1)* %25, align 16
  %58 = load <4 x float> addrspace(1)* %26, align 16
  %59 = load <4 x float> addrspace(1)* %27, align 16
  %60 = load <4 x float> addrspace(1)* %28, align 16
  %61 = load <4 x float> addrspace(1)* %29, align 16
  %62 = load <4 x float> addrspace(1)* %30, align 16
  %63 = load <4 x float> addrspace(1)* %31, align 16
  %64 = load <4 x float> addrspace(1)* %32, align 16
  %65 = load <4 x float> addrspace(1)* %33, align 16
  %66 = load <4 x float> addrspace(1)* %34, align 16
  %67 = load <4 x float> addrspace(1)* %35, align 16
  %68 = load <4 x float> addrspace(1)* %36, align 16
  %69 = load <4 x float> addrspace(1)* %37, align 16
  %70 = load <4 x float> addrspace(1)* %38, align 16
  %71 = load <4 x float> addrspace(1)* %39, align 16
  %72 = load <4 x float> addrspace(1)* %40, align 16
  %73 = extractelement <4 x float> %57, i32 0
  %74 = extractelement <4 x float> %58, i32 0
  %75 = extractelement <4 x float> %59, i32 0
  %76 = extractelement <4 x float> %60, i32 0
  %77 = extractelement <4 x float> %61, i32 0
  %78 = extractelement <4 x float> %62, i32 0
  %79 = extractelement <4 x float> %63, i32 0
  %80 = extractelement <4 x float> %64, i32 0
  %81 = extractelement <4 x float> %65, i32 0
  %82 = extractelement <4 x float> %66, i32 0
  %83 = extractelement <4 x float> %67, i32 0
  %84 = extractelement <4 x float> %68, i32 0
  %85 = extractelement <4 x float> %69, i32 0
  %86 = extractelement <4 x float> %70, i32 0
  %87 = extractelement <4 x float> %71, i32 0
  %88 = extractelement <4 x float> %72, i32 0
  %temp.vect106.i = insertelement <16 x float> undef, float %73, i32 0
  %temp.vect107.i = insertelement <16 x float> %temp.vect106.i, float %74, i32 1
  %temp.vect108.i = insertelement <16 x float> %temp.vect107.i, float %75, i32 2
  %temp.vect109.i = insertelement <16 x float> %temp.vect108.i, float %76, i32 3
  %temp.vect110.i = insertelement <16 x float> %temp.vect109.i, float %77, i32 4
  %temp.vect111.i = insertelement <16 x float> %temp.vect110.i, float %78, i32 5
  %temp.vect112.i = insertelement <16 x float> %temp.vect111.i, float %79, i32 6
  %temp.vect113.i = insertelement <16 x float> %temp.vect112.i, float %80, i32 7
  %temp.vect114.i = insertelement <16 x float> %temp.vect113.i, float %81, i32 8
  %temp.vect115.i = insertelement <16 x float> %temp.vect114.i, float %82, i32 9
  %temp.vect116.i = insertelement <16 x float> %temp.vect115.i, float %83, i32 10
  %temp.vect117.i = insertelement <16 x float> %temp.vect116.i, float %84, i32 11
  %temp.vect118.i = insertelement <16 x float> %temp.vect117.i, float %85, i32 12
  %temp.vect119.i = insertelement <16 x float> %temp.vect118.i, float %86, i32 13
  %temp.vect120.i = insertelement <16 x float> %temp.vect119.i, float %87, i32 14
  %temp.vect121.i = insertelement <16 x float> %temp.vect120.i, float %88, i32 15
  %89 = extractelement <4 x float> %57, i32 1
  %90 = extractelement <4 x float> %58, i32 1
  %91 = extractelement <4 x float> %59, i32 1
  %92 = extractelement <4 x float> %60, i32 1
  %93 = extractelement <4 x float> %61, i32 1
  %94 = extractelement <4 x float> %62, i32 1
  %95 = extractelement <4 x float> %63, i32 1
  %96 = extractelement <4 x float> %64, i32 1
  %97 = extractelement <4 x float> %65, i32 1
  %98 = extractelement <4 x float> %66, i32 1
  %99 = extractelement <4 x float> %67, i32 1
  %100 = extractelement <4 x float> %68, i32 1
  %101 = extractelement <4 x float> %69, i32 1
  %102 = extractelement <4 x float> %70, i32 1
  %103 = extractelement <4 x float> %71, i32 1
  %104 = extractelement <4 x float> %72, i32 1
  %temp.vect123.i = insertelement <16 x float> undef, float %89, i32 0
  %temp.vect124.i = insertelement <16 x float> %temp.vect123.i, float %90, i32 1
  %temp.vect125.i = insertelement <16 x float> %temp.vect124.i, float %91, i32 2
  %temp.vect126.i = insertelement <16 x float> %temp.vect125.i, float %92, i32 3
  %temp.vect127.i = insertelement <16 x float> %temp.vect126.i, float %93, i32 4
  %temp.vect128.i = insertelement <16 x float> %temp.vect127.i, float %94, i32 5
  %temp.vect129.i = insertelement <16 x float> %temp.vect128.i, float %95, i32 6
  %temp.vect130.i = insertelement <16 x float> %temp.vect129.i, float %96, i32 7
  %temp.vect131.i = insertelement <16 x float> %temp.vect130.i, float %97, i32 8
  %temp.vect132.i = insertelement <16 x float> %temp.vect131.i, float %98, i32 9
  %temp.vect133.i = insertelement <16 x float> %temp.vect132.i, float %99, i32 10
  %temp.vect134.i = insertelement <16 x float> %temp.vect133.i, float %100, i32 11
  %temp.vect135.i = insertelement <16 x float> %temp.vect134.i, float %101, i32 12
  %temp.vect136.i = insertelement <16 x float> %temp.vect135.i, float %102, i32 13
  %temp.vect137.i = insertelement <16 x float> %temp.vect136.i, float %103, i32 14
  %temp.vect138.i = insertelement <16 x float> %temp.vect137.i, float %104, i32 15
  %105 = extractelement <4 x float> %57, i32 2
  %106 = extractelement <4 x float> %58, i32 2
  %107 = extractelement <4 x float> %59, i32 2
  %108 = extractelement <4 x float> %60, i32 2
  %109 = extractelement <4 x float> %61, i32 2
  %110 = extractelement <4 x float> %62, i32 2
  %111 = extractelement <4 x float> %63, i32 2
  %112 = extractelement <4 x float> %64, i32 2
  %113 = extractelement <4 x float> %65, i32 2
  %114 = extractelement <4 x float> %66, i32 2
  %115 = extractelement <4 x float> %67, i32 2
  %116 = extractelement <4 x float> %68, i32 2
  %117 = extractelement <4 x float> %69, i32 2
  %118 = extractelement <4 x float> %70, i32 2
  %119 = extractelement <4 x float> %71, i32 2
  %120 = extractelement <4 x float> %72, i32 2
  %temp.vect141.i = insertelement <16 x float> undef, float %105, i32 0
  %temp.vect142.i = insertelement <16 x float> %temp.vect141.i, float %106, i32 1
  %temp.vect143.i = insertelement <16 x float> %temp.vect142.i, float %107, i32 2
  %temp.vect144.i = insertelement <16 x float> %temp.vect143.i, float %108, i32 3
  %temp.vect145.i = insertelement <16 x float> %temp.vect144.i, float %109, i32 4
  %temp.vect146.i = insertelement <16 x float> %temp.vect145.i, float %110, i32 5
  %temp.vect147.i = insertelement <16 x float> %temp.vect146.i, float %111, i32 6
  %temp.vect148.i = insertelement <16 x float> %temp.vect147.i, float %112, i32 7
  %temp.vect149.i = insertelement <16 x float> %temp.vect148.i, float %113, i32 8
  %temp.vect150.i = insertelement <16 x float> %temp.vect149.i, float %114, i32 9
  %temp.vect151.i = insertelement <16 x float> %temp.vect150.i, float %115, i32 10
  %temp.vect152.i = insertelement <16 x float> %temp.vect151.i, float %116, i32 11
  %temp.vect153.i = insertelement <16 x float> %temp.vect152.i, float %117, i32 12
  %temp.vect154.i = insertelement <16 x float> %temp.vect153.i, float %118, i32 13
  %temp.vect155.i = insertelement <16 x float> %temp.vect154.i, float %119, i32 14
  %temp.vect156.i = insertelement <16 x float> %temp.vect155.i, float %120, i32 15
  %121 = extractelement <4 x float> %57, i32 3
  %122 = extractelement <4 x float> %58, i32 3
  %123 = extractelement <4 x float> %59, i32 3
  %124 = extractelement <4 x float> %60, i32 3
  %125 = extractelement <4 x float> %61, i32 3
  %126 = extractelement <4 x float> %62, i32 3
  %127 = extractelement <4 x float> %63, i32 3
  %128 = extractelement <4 x float> %64, i32 3
  %129 = extractelement <4 x float> %65, i32 3
  %130 = extractelement <4 x float> %66, i32 3
  %131 = extractelement <4 x float> %67, i32 3
  %132 = extractelement <4 x float> %68, i32 3
  %133 = extractelement <4 x float> %69, i32 3
  %134 = extractelement <4 x float> %70, i32 3
  %135 = extractelement <4 x float> %71, i32 3
  %136 = extractelement <4 x float> %72, i32 3
  %temp.vect159.i = insertelement <16 x float> undef, float %121, i32 0
  %temp.vect160.i = insertelement <16 x float> %temp.vect159.i, float %122, i32 1
  %temp.vect161.i = insertelement <16 x float> %temp.vect160.i, float %123, i32 2
  %temp.vect162.i = insertelement <16 x float> %temp.vect161.i, float %124, i32 3
  %temp.vect163.i = insertelement <16 x float> %temp.vect162.i, float %125, i32 4
  %temp.vect164.i = insertelement <16 x float> %temp.vect163.i, float %126, i32 5
  %temp.vect165.i = insertelement <16 x float> %temp.vect164.i, float %127, i32 6
  %temp.vect166.i = insertelement <16 x float> %temp.vect165.i, float %128, i32 7
  %temp.vect167.i = insertelement <16 x float> %temp.vect166.i, float %129, i32 8
  %temp.vect168.i = insertelement <16 x float> %temp.vect167.i, float %130, i32 9
  %temp.vect169.i = insertelement <16 x float> %temp.vect168.i, float %131, i32 10
  %temp.vect170.i = insertelement <16 x float> %temp.vect169.i, float %132, i32 11
  %temp.vect171.i = insertelement <16 x float> %temp.vect170.i, float %133, i32 12
  %temp.vect172.i = insertelement <16 x float> %temp.vect171.i, float %134, i32 13
  %temp.vect173.i = insertelement <16 x float> %temp.vect172.i, float %135, i32 14
  %temp.vect174.i = insertelement <16 x float> %temp.vect173.i, float %136, i32 15
  %mul_dot122.i = fmul <16 x float> %temp.vect121.i, <float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000, float 0x3FD322D0E0000000>
  %mul_dot8139.i = fmul <16 x float> %temp.vect138.i, <float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000, float 0x3FE2C8B440000000>
  %sum_dot140.i = fadd <16 x float> %mul_dot122.i, %mul_dot8139.i
  %mul_dot9157.i = fmul <16 x float> %temp.vect156.i, <float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000, float 0x3FBD2F1AA0000000>
  %sum_dot10158.i = fadd <16 x float> %sum_dot140.i, %mul_dot9157.i
  %mul_dot11175.i = fmul <16 x float> %temp.vect174.i, zeroinitializer
  %sum_dot12176.i = fadd <16 x float> %sum_dot10158.i, %mul_dot11175.i
  %extract177.i = extractelement <16 x float> %sum_dot12176.i, i32 0
  %extract178.i = extractelement <16 x float> %sum_dot12176.i, i32 1
  %extract179.i = extractelement <16 x float> %sum_dot12176.i, i32 2
  %extract180.i = extractelement <16 x float> %sum_dot12176.i, i32 3
  %extract181.i = extractelement <16 x float> %sum_dot12176.i, i32 4
  %extract182.i = extractelement <16 x float> %sum_dot12176.i, i32 5
  %extract183.i = extractelement <16 x float> %sum_dot12176.i, i32 6
  %extract184.i = extractelement <16 x float> %sum_dot12176.i, i32 7
  %extract185.i = extractelement <16 x float> %sum_dot12176.i, i32 8
  %extract186.i = extractelement <16 x float> %sum_dot12176.i, i32 9
  %extract187.i = extractelement <16 x float> %sum_dot12176.i, i32 10
  %extract188.i = extractelement <16 x float> %sum_dot12176.i, i32 11
  %extract189.i = extractelement <16 x float> %sum_dot12176.i, i32 12
  %extract190.i = extractelement <16 x float> %sum_dot12176.i, i32 13
  %extract191.i = extractelement <16 x float> %sum_dot12176.i, i32 14
  %extract192.i = extractelement <16 x float> %sum_dot12176.i, i32 15
  %137 = load <4 x float> addrspace(1)* %41, align 16
  %138 = load <4 x float> addrspace(1)* %42, align 16
  %139 = load <4 x float> addrspace(1)* %43, align 16
  %140 = load <4 x float> addrspace(1)* %44, align 16
  %141 = load <4 x float> addrspace(1)* %45, align 16
  %142 = load <4 x float> addrspace(1)* %46, align 16
  %143 = load <4 x float> addrspace(1)* %47, align 16
  %144 = load <4 x float> addrspace(1)* %48, align 16
  %145 = load <4 x float> addrspace(1)* %49, align 16
  %146 = load <4 x float> addrspace(1)* %50, align 16
  %147 = load <4 x float> addrspace(1)* %51, align 16
  %148 = load <4 x float> addrspace(1)* %52, align 16
  %149 = load <4 x float> addrspace(1)* %53, align 16
  %150 = load <4 x float> addrspace(1)* %54, align 16
  %151 = load <4 x float> addrspace(1)* %55, align 16
  %152 = load <4 x float> addrspace(1)* %56, align 16
  %153 = insertelement <4 x float> undef, float %extract177.i, i32 0
  %154 = insertelement <4 x float> undef, float %extract178.i, i32 0
  %155 = insertelement <4 x float> undef, float %extract179.i, i32 0
  %156 = insertelement <4 x float> undef, float %extract180.i, i32 0
  %157 = insertelement <4 x float> undef, float %extract181.i, i32 0
  %158 = insertelement <4 x float> undef, float %extract182.i, i32 0
  %159 = insertelement <4 x float> undef, float %extract183.i, i32 0
  %160 = insertelement <4 x float> undef, float %extract184.i, i32 0
  %161 = insertelement <4 x float> undef, float %extract185.i, i32 0
  %162 = insertelement <4 x float> undef, float %extract186.i, i32 0
  %163 = insertelement <4 x float> undef, float %extract187.i, i32 0
  %164 = insertelement <4 x float> undef, float %extract188.i, i32 0
  %165 = insertelement <4 x float> undef, float %extract189.i, i32 0
  %166 = insertelement <4 x float> undef, float %extract190.i, i32 0
  %167 = insertelement <4 x float> undef, float %extract191.i, i32 0
  %168 = insertelement <4 x float> undef, float %extract192.i, i32 0
  %169 = shufflevector <4 x float> %153, <4 x float> %137, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %170 = shufflevector <4 x float> %154, <4 x float> %138, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %171 = shufflevector <4 x float> %155, <4 x float> %139, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %172 = shufflevector <4 x float> %156, <4 x float> %140, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %173 = shufflevector <4 x float> %157, <4 x float> %141, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %174 = shufflevector <4 x float> %158, <4 x float> %142, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %175 = shufflevector <4 x float> %159, <4 x float> %143, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %176 = shufflevector <4 x float> %160, <4 x float> %144, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %177 = shufflevector <4 x float> %161, <4 x float> %145, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %178 = shufflevector <4 x float> %162, <4 x float> %146, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %179 = shufflevector <4 x float> %163, <4 x float> %147, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %180 = shufflevector <4 x float> %164, <4 x float> %148, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %181 = shufflevector <4 x float> %165, <4 x float> %149, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %182 = shufflevector <4 x float> %166, <4 x float> %150, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %183 = shufflevector <4 x float> %167, <4 x float> %151, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  %184 = shufflevector <4 x float> %168, <4 x float> %152, <4 x i32> <i32 0, i32 5, i32 6, i32 7>
  store <4 x float> %169, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %170, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %171, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %172, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %173, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %174, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %175, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %176, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %177, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %178, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %179, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %180, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %181, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %182, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %183, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %184, <4 x float> addrspace(1)* %56, align 16
  %185 = load <4 x float> addrspace(1)* %25, align 16
  %186 = load <4 x float> addrspace(1)* %26, align 16
  %187 = load <4 x float> addrspace(1)* %27, align 16
  %188 = load <4 x float> addrspace(1)* %28, align 16
  %189 = load <4 x float> addrspace(1)* %29, align 16
  %190 = load <4 x float> addrspace(1)* %30, align 16
  %191 = load <4 x float> addrspace(1)* %31, align 16
  %192 = load <4 x float> addrspace(1)* %32, align 16
  %193 = load <4 x float> addrspace(1)* %33, align 16
  %194 = load <4 x float> addrspace(1)* %34, align 16
  %195 = load <4 x float> addrspace(1)* %35, align 16
  %196 = load <4 x float> addrspace(1)* %36, align 16
  %197 = load <4 x float> addrspace(1)* %37, align 16
  %198 = load <4 x float> addrspace(1)* %38, align 16
  %199 = load <4 x float> addrspace(1)* %39, align 16
  %200 = load <4 x float> addrspace(1)* %40, align 16
  %201 = extractelement <4 x float> %185, i32 0
  %202 = extractelement <4 x float> %186, i32 0
  %203 = extractelement <4 x float> %187, i32 0
  %204 = extractelement <4 x float> %188, i32 0
  %205 = extractelement <4 x float> %189, i32 0
  %206 = extractelement <4 x float> %190, i32 0
  %207 = extractelement <4 x float> %191, i32 0
  %208 = extractelement <4 x float> %192, i32 0
  %209 = extractelement <4 x float> %193, i32 0
  %210 = extractelement <4 x float> %194, i32 0
  %211 = extractelement <4 x float> %195, i32 0
  %212 = extractelement <4 x float> %196, i32 0
  %213 = extractelement <4 x float> %197, i32 0
  %214 = extractelement <4 x float> %198, i32 0
  %215 = extractelement <4 x float> %199, i32 0
  %216 = extractelement <4 x float> %200, i32 0
  %temp.vect193.i = insertelement <16 x float> undef, float %201, i32 0
  %temp.vect194.i = insertelement <16 x float> %temp.vect193.i, float %202, i32 1
  %temp.vect195.i = insertelement <16 x float> %temp.vect194.i, float %203, i32 2
  %temp.vect196.i = insertelement <16 x float> %temp.vect195.i, float %204, i32 3
  %temp.vect197.i = insertelement <16 x float> %temp.vect196.i, float %205, i32 4
  %temp.vect198.i = insertelement <16 x float> %temp.vect197.i, float %206, i32 5
  %temp.vect199.i = insertelement <16 x float> %temp.vect198.i, float %207, i32 6
  %temp.vect200.i = insertelement <16 x float> %temp.vect199.i, float %208, i32 7
  %temp.vect201.i = insertelement <16 x float> %temp.vect200.i, float %209, i32 8
  %temp.vect202.i = insertelement <16 x float> %temp.vect201.i, float %210, i32 9
  %temp.vect203.i = insertelement <16 x float> %temp.vect202.i, float %211, i32 10
  %temp.vect204.i = insertelement <16 x float> %temp.vect203.i, float %212, i32 11
  %temp.vect205.i = insertelement <16 x float> %temp.vect204.i, float %213, i32 12
  %temp.vect206.i = insertelement <16 x float> %temp.vect205.i, float %214, i32 13
  %temp.vect207.i = insertelement <16 x float> %temp.vect206.i, float %215, i32 14
  %temp.vect208.i = insertelement <16 x float> %temp.vect207.i, float %216, i32 15
  %217 = extractelement <4 x float> %185, i32 1
  %218 = extractelement <4 x float> %186, i32 1
  %219 = extractelement <4 x float> %187, i32 1
  %220 = extractelement <4 x float> %188, i32 1
  %221 = extractelement <4 x float> %189, i32 1
  %222 = extractelement <4 x float> %190, i32 1
  %223 = extractelement <4 x float> %191, i32 1
  %224 = extractelement <4 x float> %192, i32 1
  %225 = extractelement <4 x float> %193, i32 1
  %226 = extractelement <4 x float> %194, i32 1
  %227 = extractelement <4 x float> %195, i32 1
  %228 = extractelement <4 x float> %196, i32 1
  %229 = extractelement <4 x float> %197, i32 1
  %230 = extractelement <4 x float> %198, i32 1
  %231 = extractelement <4 x float> %199, i32 1
  %232 = extractelement <4 x float> %200, i32 1
  %temp.vect210.i = insertelement <16 x float> undef, float %217, i32 0
  %temp.vect211.i = insertelement <16 x float> %temp.vect210.i, float %218, i32 1
  %temp.vect212.i = insertelement <16 x float> %temp.vect211.i, float %219, i32 2
  %temp.vect213.i = insertelement <16 x float> %temp.vect212.i, float %220, i32 3
  %temp.vect214.i = insertelement <16 x float> %temp.vect213.i, float %221, i32 4
  %temp.vect215.i = insertelement <16 x float> %temp.vect214.i, float %222, i32 5
  %temp.vect216.i = insertelement <16 x float> %temp.vect215.i, float %223, i32 6
  %temp.vect217.i = insertelement <16 x float> %temp.vect216.i, float %224, i32 7
  %temp.vect218.i = insertelement <16 x float> %temp.vect217.i, float %225, i32 8
  %temp.vect219.i = insertelement <16 x float> %temp.vect218.i, float %226, i32 9
  %temp.vect220.i = insertelement <16 x float> %temp.vect219.i, float %227, i32 10
  %temp.vect221.i = insertelement <16 x float> %temp.vect220.i, float %228, i32 11
  %temp.vect222.i = insertelement <16 x float> %temp.vect221.i, float %229, i32 12
  %temp.vect223.i = insertelement <16 x float> %temp.vect222.i, float %230, i32 13
  %temp.vect224.i = insertelement <16 x float> %temp.vect223.i, float %231, i32 14
  %temp.vect225.i = insertelement <16 x float> %temp.vect224.i, float %232, i32 15
  %233 = extractelement <4 x float> %185, i32 2
  %234 = extractelement <4 x float> %186, i32 2
  %235 = extractelement <4 x float> %187, i32 2
  %236 = extractelement <4 x float> %188, i32 2
  %237 = extractelement <4 x float> %189, i32 2
  %238 = extractelement <4 x float> %190, i32 2
  %239 = extractelement <4 x float> %191, i32 2
  %240 = extractelement <4 x float> %192, i32 2
  %241 = extractelement <4 x float> %193, i32 2
  %242 = extractelement <4 x float> %194, i32 2
  %243 = extractelement <4 x float> %195, i32 2
  %244 = extractelement <4 x float> %196, i32 2
  %245 = extractelement <4 x float> %197, i32 2
  %246 = extractelement <4 x float> %198, i32 2
  %247 = extractelement <4 x float> %199, i32 2
  %248 = extractelement <4 x float> %200, i32 2
  %temp.vect228.i = insertelement <16 x float> undef, float %233, i32 0
  %temp.vect229.i = insertelement <16 x float> %temp.vect228.i, float %234, i32 1
  %temp.vect230.i = insertelement <16 x float> %temp.vect229.i, float %235, i32 2
  %temp.vect231.i = insertelement <16 x float> %temp.vect230.i, float %236, i32 3
  %temp.vect232.i = insertelement <16 x float> %temp.vect231.i, float %237, i32 4
  %temp.vect233.i = insertelement <16 x float> %temp.vect232.i, float %238, i32 5
  %temp.vect234.i = insertelement <16 x float> %temp.vect233.i, float %239, i32 6
  %temp.vect235.i = insertelement <16 x float> %temp.vect234.i, float %240, i32 7
  %temp.vect236.i = insertelement <16 x float> %temp.vect235.i, float %241, i32 8
  %temp.vect237.i = insertelement <16 x float> %temp.vect236.i, float %242, i32 9
  %temp.vect238.i = insertelement <16 x float> %temp.vect237.i, float %243, i32 10
  %temp.vect239.i = insertelement <16 x float> %temp.vect238.i, float %244, i32 11
  %temp.vect240.i = insertelement <16 x float> %temp.vect239.i, float %245, i32 12
  %temp.vect241.i = insertelement <16 x float> %temp.vect240.i, float %246, i32 13
  %temp.vect242.i = insertelement <16 x float> %temp.vect241.i, float %247, i32 14
  %temp.vect243.i = insertelement <16 x float> %temp.vect242.i, float %248, i32 15
  %249 = extractelement <4 x float> %185, i32 3
  %250 = extractelement <4 x float> %186, i32 3
  %251 = extractelement <4 x float> %187, i32 3
  %252 = extractelement <4 x float> %188, i32 3
  %253 = extractelement <4 x float> %189, i32 3
  %254 = extractelement <4 x float> %190, i32 3
  %255 = extractelement <4 x float> %191, i32 3
  %256 = extractelement <4 x float> %192, i32 3
  %257 = extractelement <4 x float> %193, i32 3
  %258 = extractelement <4 x float> %194, i32 3
  %259 = extractelement <4 x float> %195, i32 3
  %260 = extractelement <4 x float> %196, i32 3
  %261 = extractelement <4 x float> %197, i32 3
  %262 = extractelement <4 x float> %198, i32 3
  %263 = extractelement <4 x float> %199, i32 3
  %264 = extractelement <4 x float> %200, i32 3
  %temp.vect246.i = insertelement <16 x float> undef, float %249, i32 0
  %temp.vect247.i = insertelement <16 x float> %temp.vect246.i, float %250, i32 1
  %temp.vect248.i = insertelement <16 x float> %temp.vect247.i, float %251, i32 2
  %temp.vect249.i = insertelement <16 x float> %temp.vect248.i, float %252, i32 3
  %temp.vect250.i = insertelement <16 x float> %temp.vect249.i, float %253, i32 4
  %temp.vect251.i = insertelement <16 x float> %temp.vect250.i, float %254, i32 5
  %temp.vect252.i = insertelement <16 x float> %temp.vect251.i, float %255, i32 6
  %temp.vect253.i = insertelement <16 x float> %temp.vect252.i, float %256, i32 7
  %temp.vect254.i = insertelement <16 x float> %temp.vect253.i, float %257, i32 8
  %temp.vect255.i = insertelement <16 x float> %temp.vect254.i, float %258, i32 9
  %temp.vect256.i = insertelement <16 x float> %temp.vect255.i, float %259, i32 10
  %temp.vect257.i = insertelement <16 x float> %temp.vect256.i, float %260, i32 11
  %temp.vect258.i = insertelement <16 x float> %temp.vect257.i, float %261, i32 12
  %temp.vect259.i = insertelement <16 x float> %temp.vect258.i, float %262, i32 13
  %temp.vect260.i = insertelement <16 x float> %temp.vect259.i, float %263, i32 14
  %temp.vect261.i = insertelement <16 x float> %temp.vect260.i, float %264, i32 15
  %mul_dot21209.i = fmul <16 x float> %temp.vect208.i, <float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000, float 0xBFC2D0E560000000>
  %mul_dot22226.i = fmul <16 x float> %temp.vect225.i, <float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000, float 0xBFD27EF9E0000000>
  %sum_dot23227.i = fadd <16 x float> %mul_dot21209.i, %mul_dot22226.i
  %mul_dot24244.i = fmul <16 x float> %temp.vect243.i, <float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000, float 0x3FDBE76C80000000>
  %sum_dot25245.i = fadd <16 x float> %sum_dot23227.i, %mul_dot24244.i
  %mul_dot26262.i = fmul <16 x float> %temp.vect261.i, zeroinitializer
  %sum_dot27263.i = fadd <16 x float> %sum_dot25245.i, %mul_dot26262.i
  %extract264.i = extractelement <16 x float> %sum_dot27263.i, i32 0
  %extract265.i = extractelement <16 x float> %sum_dot27263.i, i32 1
  %extract266.i = extractelement <16 x float> %sum_dot27263.i, i32 2
  %extract267.i = extractelement <16 x float> %sum_dot27263.i, i32 3
  %extract268.i = extractelement <16 x float> %sum_dot27263.i, i32 4
  %extract269.i = extractelement <16 x float> %sum_dot27263.i, i32 5
  %extract270.i = extractelement <16 x float> %sum_dot27263.i, i32 6
  %extract271.i = extractelement <16 x float> %sum_dot27263.i, i32 7
  %extract272.i = extractelement <16 x float> %sum_dot27263.i, i32 8
  %extract273.i = extractelement <16 x float> %sum_dot27263.i, i32 9
  %extract274.i = extractelement <16 x float> %sum_dot27263.i, i32 10
  %extract275.i = extractelement <16 x float> %sum_dot27263.i, i32 11
  %extract276.i = extractelement <16 x float> %sum_dot27263.i, i32 12
  %extract277.i = extractelement <16 x float> %sum_dot27263.i, i32 13
  %extract278.i = extractelement <16 x float> %sum_dot27263.i, i32 14
  %extract279.i = extractelement <16 x float> %sum_dot27263.i, i32 15
  %265 = load <4 x float> addrspace(1)* %41, align 16
  %266 = load <4 x float> addrspace(1)* %42, align 16
  %267 = load <4 x float> addrspace(1)* %43, align 16
  %268 = load <4 x float> addrspace(1)* %44, align 16
  %269 = load <4 x float> addrspace(1)* %45, align 16
  %270 = load <4 x float> addrspace(1)* %46, align 16
  %271 = load <4 x float> addrspace(1)* %47, align 16
  %272 = load <4 x float> addrspace(1)* %48, align 16
  %273 = load <4 x float> addrspace(1)* %49, align 16
  %274 = load <4 x float> addrspace(1)* %50, align 16
  %275 = load <4 x float> addrspace(1)* %51, align 16
  %276 = load <4 x float> addrspace(1)* %52, align 16
  %277 = load <4 x float> addrspace(1)* %53, align 16
  %278 = load <4 x float> addrspace(1)* %54, align 16
  %279 = load <4 x float> addrspace(1)* %55, align 16
  %280 = load <4 x float> addrspace(1)* %56, align 16
  %281 = extractelement <4 x float> %265, i32 0
  %282 = extractelement <4 x float> %266, i32 0
  %283 = extractelement <4 x float> %267, i32 0
  %284 = extractelement <4 x float> %268, i32 0
  %285 = extractelement <4 x float> %269, i32 0
  %286 = extractelement <4 x float> %270, i32 0
  %287 = extractelement <4 x float> %271, i32 0
  %288 = extractelement <4 x float> %272, i32 0
  %289 = extractelement <4 x float> %273, i32 0
  %290 = extractelement <4 x float> %274, i32 0
  %291 = extractelement <4 x float> %275, i32 0
  %292 = extractelement <4 x float> %276, i32 0
  %293 = extractelement <4 x float> %277, i32 0
  %294 = extractelement <4 x float> %278, i32 0
  %295 = extractelement <4 x float> %279, i32 0
  %296 = extractelement <4 x float> %280, i32 0
  %297 = insertelement <4 x float> undef, float %281, i32 0
  %298 = insertelement <4 x float> undef, float %282, i32 0
  %299 = insertelement <4 x float> undef, float %283, i32 0
  %300 = insertelement <4 x float> undef, float %284, i32 0
  %301 = insertelement <4 x float> undef, float %285, i32 0
  %302 = insertelement <4 x float> undef, float %286, i32 0
  %303 = insertelement <4 x float> undef, float %287, i32 0
  %304 = insertelement <4 x float> undef, float %288, i32 0
  %305 = insertelement <4 x float> undef, float %289, i32 0
  %306 = insertelement <4 x float> undef, float %290, i32 0
  %307 = insertelement <4 x float> undef, float %291, i32 0
  %308 = insertelement <4 x float> undef, float %292, i32 0
  %309 = insertelement <4 x float> undef, float %293, i32 0
  %310 = insertelement <4 x float> undef, float %294, i32 0
  %311 = insertelement <4 x float> undef, float %295, i32 0
  %312 = insertelement <4 x float> undef, float %296, i32 0
  %313 = insertelement <4 x float> %297, float %extract264.i, i32 1
  %314 = insertelement <4 x float> %298, float %extract265.i, i32 1
  %315 = insertelement <4 x float> %299, float %extract266.i, i32 1
  %316 = insertelement <4 x float> %300, float %extract267.i, i32 1
  %317 = insertelement <4 x float> %301, float %extract268.i, i32 1
  %318 = insertelement <4 x float> %302, float %extract269.i, i32 1
  %319 = insertelement <4 x float> %303, float %extract270.i, i32 1
  %320 = insertelement <4 x float> %304, float %extract271.i, i32 1
  %321 = insertelement <4 x float> %305, float %extract272.i, i32 1
  %322 = insertelement <4 x float> %306, float %extract273.i, i32 1
  %323 = insertelement <4 x float> %307, float %extract274.i, i32 1
  %324 = insertelement <4 x float> %308, float %extract275.i, i32 1
  %325 = insertelement <4 x float> %309, float %extract276.i, i32 1
  %326 = insertelement <4 x float> %310, float %extract277.i, i32 1
  %327 = insertelement <4 x float> %311, float %extract278.i, i32 1
  %328 = insertelement <4 x float> %312, float %extract279.i, i32 1
  %329 = shufflevector <4 x float> %313, <4 x float> %265, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %330 = shufflevector <4 x float> %314, <4 x float> %266, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %331 = shufflevector <4 x float> %315, <4 x float> %267, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %332 = shufflevector <4 x float> %316, <4 x float> %268, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %333 = shufflevector <4 x float> %317, <4 x float> %269, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %334 = shufflevector <4 x float> %318, <4 x float> %270, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %335 = shufflevector <4 x float> %319, <4 x float> %271, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %336 = shufflevector <4 x float> %320, <4 x float> %272, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %337 = shufflevector <4 x float> %321, <4 x float> %273, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %338 = shufflevector <4 x float> %322, <4 x float> %274, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %339 = shufflevector <4 x float> %323, <4 x float> %275, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %340 = shufflevector <4 x float> %324, <4 x float> %276, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %341 = shufflevector <4 x float> %325, <4 x float> %277, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %342 = shufflevector <4 x float> %326, <4 x float> %278, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %343 = shufflevector <4 x float> %327, <4 x float> %279, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  %344 = shufflevector <4 x float> %328, <4 x float> %280, <4 x i32> <i32 0, i32 1, i32 6, i32 7>
  store <4 x float> %329, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %330, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %331, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %332, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %333, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %334, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %335, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %336, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %337, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %338, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %339, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %340, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %341, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %342, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %343, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %344, <4 x float> addrspace(1)* %56, align 16
  %345 = load <4 x float> addrspace(1)* %25, align 16
  %346 = load <4 x float> addrspace(1)* %26, align 16
  %347 = load <4 x float> addrspace(1)* %27, align 16
  %348 = load <4 x float> addrspace(1)* %28, align 16
  %349 = load <4 x float> addrspace(1)* %29, align 16
  %350 = load <4 x float> addrspace(1)* %30, align 16
  %351 = load <4 x float> addrspace(1)* %31, align 16
  %352 = load <4 x float> addrspace(1)* %32, align 16
  %353 = load <4 x float> addrspace(1)* %33, align 16
  %354 = load <4 x float> addrspace(1)* %34, align 16
  %355 = load <4 x float> addrspace(1)* %35, align 16
  %356 = load <4 x float> addrspace(1)* %36, align 16
  %357 = load <4 x float> addrspace(1)* %37, align 16
  %358 = load <4 x float> addrspace(1)* %38, align 16
  %359 = load <4 x float> addrspace(1)* %39, align 16
  %360 = load <4 x float> addrspace(1)* %40, align 16
  %361 = extractelement <4 x float> %345, i32 0
  %362 = extractelement <4 x float> %346, i32 0
  %363 = extractelement <4 x float> %347, i32 0
  %364 = extractelement <4 x float> %348, i32 0
  %365 = extractelement <4 x float> %349, i32 0
  %366 = extractelement <4 x float> %350, i32 0
  %367 = extractelement <4 x float> %351, i32 0
  %368 = extractelement <4 x float> %352, i32 0
  %369 = extractelement <4 x float> %353, i32 0
  %370 = extractelement <4 x float> %354, i32 0
  %371 = extractelement <4 x float> %355, i32 0
  %372 = extractelement <4 x float> %356, i32 0
  %373 = extractelement <4 x float> %357, i32 0
  %374 = extractelement <4 x float> %358, i32 0
  %375 = extractelement <4 x float> %359, i32 0
  %376 = extractelement <4 x float> %360, i32 0
  %temp.vect280.i = insertelement <16 x float> undef, float %361, i32 0
  %temp.vect281.i = insertelement <16 x float> %temp.vect280.i, float %362, i32 1
  %temp.vect282.i = insertelement <16 x float> %temp.vect281.i, float %363, i32 2
  %temp.vect283.i = insertelement <16 x float> %temp.vect282.i, float %364, i32 3
  %temp.vect284.i = insertelement <16 x float> %temp.vect283.i, float %365, i32 4
  %temp.vect285.i = insertelement <16 x float> %temp.vect284.i, float %366, i32 5
  %temp.vect286.i = insertelement <16 x float> %temp.vect285.i, float %367, i32 6
  %temp.vect287.i = insertelement <16 x float> %temp.vect286.i, float %368, i32 7
  %temp.vect288.i = insertelement <16 x float> %temp.vect287.i, float %369, i32 8
  %temp.vect289.i = insertelement <16 x float> %temp.vect288.i, float %370, i32 9
  %temp.vect290.i = insertelement <16 x float> %temp.vect289.i, float %371, i32 10
  %temp.vect291.i = insertelement <16 x float> %temp.vect290.i, float %372, i32 11
  %temp.vect292.i = insertelement <16 x float> %temp.vect291.i, float %373, i32 12
  %temp.vect293.i = insertelement <16 x float> %temp.vect292.i, float %374, i32 13
  %temp.vect294.i = insertelement <16 x float> %temp.vect293.i, float %375, i32 14
  %temp.vect295.i = insertelement <16 x float> %temp.vect294.i, float %376, i32 15
  %377 = extractelement <4 x float> %345, i32 1
  %378 = extractelement <4 x float> %346, i32 1
  %379 = extractelement <4 x float> %347, i32 1
  %380 = extractelement <4 x float> %348, i32 1
  %381 = extractelement <4 x float> %349, i32 1
  %382 = extractelement <4 x float> %350, i32 1
  %383 = extractelement <4 x float> %351, i32 1
  %384 = extractelement <4 x float> %352, i32 1
  %385 = extractelement <4 x float> %353, i32 1
  %386 = extractelement <4 x float> %354, i32 1
  %387 = extractelement <4 x float> %355, i32 1
  %388 = extractelement <4 x float> %356, i32 1
  %389 = extractelement <4 x float> %357, i32 1
  %390 = extractelement <4 x float> %358, i32 1
  %391 = extractelement <4 x float> %359, i32 1
  %392 = extractelement <4 x float> %360, i32 1
  %temp.vect297.i = insertelement <16 x float> undef, float %377, i32 0
  %temp.vect298.i = insertelement <16 x float> %temp.vect297.i, float %378, i32 1
  %temp.vect299.i = insertelement <16 x float> %temp.vect298.i, float %379, i32 2
  %temp.vect300.i = insertelement <16 x float> %temp.vect299.i, float %380, i32 3
  %temp.vect301.i = insertelement <16 x float> %temp.vect300.i, float %381, i32 4
  %temp.vect302.i = insertelement <16 x float> %temp.vect301.i, float %382, i32 5
  %temp.vect303.i = insertelement <16 x float> %temp.vect302.i, float %383, i32 6
  %temp.vect304.i = insertelement <16 x float> %temp.vect303.i, float %384, i32 7
  %temp.vect305.i = insertelement <16 x float> %temp.vect304.i, float %385, i32 8
  %temp.vect306.i = insertelement <16 x float> %temp.vect305.i, float %386, i32 9
  %temp.vect307.i = insertelement <16 x float> %temp.vect306.i, float %387, i32 10
  %temp.vect308.i = insertelement <16 x float> %temp.vect307.i, float %388, i32 11
  %temp.vect309.i = insertelement <16 x float> %temp.vect308.i, float %389, i32 12
  %temp.vect310.i = insertelement <16 x float> %temp.vect309.i, float %390, i32 13
  %temp.vect311.i = insertelement <16 x float> %temp.vect310.i, float %391, i32 14
  %temp.vect312.i = insertelement <16 x float> %temp.vect311.i, float %392, i32 15
  %393 = extractelement <4 x float> %345, i32 2
  %394 = extractelement <4 x float> %346, i32 2
  %395 = extractelement <4 x float> %347, i32 2
  %396 = extractelement <4 x float> %348, i32 2
  %397 = extractelement <4 x float> %349, i32 2
  %398 = extractelement <4 x float> %350, i32 2
  %399 = extractelement <4 x float> %351, i32 2
  %400 = extractelement <4 x float> %352, i32 2
  %401 = extractelement <4 x float> %353, i32 2
  %402 = extractelement <4 x float> %354, i32 2
  %403 = extractelement <4 x float> %355, i32 2
  %404 = extractelement <4 x float> %356, i32 2
  %405 = extractelement <4 x float> %357, i32 2
  %406 = extractelement <4 x float> %358, i32 2
  %407 = extractelement <4 x float> %359, i32 2
  %408 = extractelement <4 x float> %360, i32 2
  %temp.vect315.i = insertelement <16 x float> undef, float %393, i32 0
  %temp.vect316.i = insertelement <16 x float> %temp.vect315.i, float %394, i32 1
  %temp.vect317.i = insertelement <16 x float> %temp.vect316.i, float %395, i32 2
  %temp.vect318.i = insertelement <16 x float> %temp.vect317.i, float %396, i32 3
  %temp.vect319.i = insertelement <16 x float> %temp.vect318.i, float %397, i32 4
  %temp.vect320.i = insertelement <16 x float> %temp.vect319.i, float %398, i32 5
  %temp.vect321.i = insertelement <16 x float> %temp.vect320.i, float %399, i32 6
  %temp.vect322.i = insertelement <16 x float> %temp.vect321.i, float %400, i32 7
  %temp.vect323.i = insertelement <16 x float> %temp.vect322.i, float %401, i32 8
  %temp.vect324.i = insertelement <16 x float> %temp.vect323.i, float %402, i32 9
  %temp.vect325.i = insertelement <16 x float> %temp.vect324.i, float %403, i32 10
  %temp.vect326.i = insertelement <16 x float> %temp.vect325.i, float %404, i32 11
  %temp.vect327.i = insertelement <16 x float> %temp.vect326.i, float %405, i32 12
  %temp.vect328.i = insertelement <16 x float> %temp.vect327.i, float %406, i32 13
  %temp.vect329.i = insertelement <16 x float> %temp.vect328.i, float %407, i32 14
  %temp.vect330.i = insertelement <16 x float> %temp.vect329.i, float %408, i32 15
  %409 = extractelement <4 x float> %345, i32 3
  %410 = extractelement <4 x float> %346, i32 3
  %411 = extractelement <4 x float> %347, i32 3
  %412 = extractelement <4 x float> %348, i32 3
  %413 = extractelement <4 x float> %349, i32 3
  %414 = extractelement <4 x float> %350, i32 3
  %415 = extractelement <4 x float> %351, i32 3
  %416 = extractelement <4 x float> %352, i32 3
  %417 = extractelement <4 x float> %353, i32 3
  %418 = extractelement <4 x float> %354, i32 3
  %419 = extractelement <4 x float> %355, i32 3
  %420 = extractelement <4 x float> %356, i32 3
  %421 = extractelement <4 x float> %357, i32 3
  %422 = extractelement <4 x float> %358, i32 3
  %423 = extractelement <4 x float> %359, i32 3
  %424 = extractelement <4 x float> %360, i32 3
  %temp.vect333.i = insertelement <16 x float> undef, float %409, i32 0
  %temp.vect334.i = insertelement <16 x float> %temp.vect333.i, float %410, i32 1
  %temp.vect335.i = insertelement <16 x float> %temp.vect334.i, float %411, i32 2
  %temp.vect336.i = insertelement <16 x float> %temp.vect335.i, float %412, i32 3
  %temp.vect337.i = insertelement <16 x float> %temp.vect336.i, float %413, i32 4
  %temp.vect338.i = insertelement <16 x float> %temp.vect337.i, float %414, i32 5
  %temp.vect339.i = insertelement <16 x float> %temp.vect338.i, float %415, i32 6
  %temp.vect340.i = insertelement <16 x float> %temp.vect339.i, float %416, i32 7
  %temp.vect341.i = insertelement <16 x float> %temp.vect340.i, float %417, i32 8
  %temp.vect342.i = insertelement <16 x float> %temp.vect341.i, float %418, i32 9
  %temp.vect343.i = insertelement <16 x float> %temp.vect342.i, float %419, i32 10
  %temp.vect344.i = insertelement <16 x float> %temp.vect343.i, float %420, i32 11
  %temp.vect345.i = insertelement <16 x float> %temp.vect344.i, float %421, i32 12
  %temp.vect346.i = insertelement <16 x float> %temp.vect345.i, float %422, i32 13
  %temp.vect347.i = insertelement <16 x float> %temp.vect346.i, float %423, i32 14
  %temp.vect348.i = insertelement <16 x float> %temp.vect347.i, float %424, i32 15
  %mul_dot36296.i = fmul <16 x float> %temp.vect295.i, <float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000, float 0x3FE3AE1480000000>
  %mul_dot37313.i = fmul <16 x float> %temp.vect312.i, <float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000, float 0xBFE07AE140000000>
  %sum_dot38314.i = fadd <16 x float> %mul_dot36296.i, %mul_dot37313.i
  %mul_dot39331.i = fmul <16 x float> %temp.vect330.i, <float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000, float 0xBFB99999A0000000>
  %sum_dot40332.i = fadd <16 x float> %sum_dot38314.i, %mul_dot39331.i
  %mul_dot41349.i = fmul <16 x float> %temp.vect348.i, zeroinitializer
  %sum_dot42350.i = fadd <16 x float> %sum_dot40332.i, %mul_dot41349.i
  %extract351.i = extractelement <16 x float> %sum_dot42350.i, i32 0
  %extract352.i = extractelement <16 x float> %sum_dot42350.i, i32 1
  %extract353.i = extractelement <16 x float> %sum_dot42350.i, i32 2
  %extract354.i = extractelement <16 x float> %sum_dot42350.i, i32 3
  %extract355.i = extractelement <16 x float> %sum_dot42350.i, i32 4
  %extract356.i = extractelement <16 x float> %sum_dot42350.i, i32 5
  %extract357.i = extractelement <16 x float> %sum_dot42350.i, i32 6
  %extract358.i = extractelement <16 x float> %sum_dot42350.i, i32 7
  %extract359.i = extractelement <16 x float> %sum_dot42350.i, i32 8
  %extract360.i = extractelement <16 x float> %sum_dot42350.i, i32 9
  %extract361.i = extractelement <16 x float> %sum_dot42350.i, i32 10
  %extract362.i = extractelement <16 x float> %sum_dot42350.i, i32 11
  %extract363.i = extractelement <16 x float> %sum_dot42350.i, i32 12
  %extract364.i = extractelement <16 x float> %sum_dot42350.i, i32 13
  %extract365.i = extractelement <16 x float> %sum_dot42350.i, i32 14
  %extract366.i = extractelement <16 x float> %sum_dot42350.i, i32 15
  %425 = load <4 x float> addrspace(1)* %41, align 16
  %426 = load <4 x float> addrspace(1)* %42, align 16
  %427 = load <4 x float> addrspace(1)* %43, align 16
  %428 = load <4 x float> addrspace(1)* %44, align 16
  %429 = load <4 x float> addrspace(1)* %45, align 16
  %430 = load <4 x float> addrspace(1)* %46, align 16
  %431 = load <4 x float> addrspace(1)* %47, align 16
  %432 = load <4 x float> addrspace(1)* %48, align 16
  %433 = load <4 x float> addrspace(1)* %49, align 16
  %434 = load <4 x float> addrspace(1)* %50, align 16
  %435 = load <4 x float> addrspace(1)* %51, align 16
  %436 = load <4 x float> addrspace(1)* %52, align 16
  %437 = load <4 x float> addrspace(1)* %53, align 16
  %438 = load <4 x float> addrspace(1)* %54, align 16
  %439 = load <4 x float> addrspace(1)* %55, align 16
  %440 = load <4 x float> addrspace(1)* %56, align 16
  %441 = extractelement <4 x float> %425, i32 0
  %442 = extractelement <4 x float> %426, i32 0
  %443 = extractelement <4 x float> %427, i32 0
  %444 = extractelement <4 x float> %428, i32 0
  %445 = extractelement <4 x float> %429, i32 0
  %446 = extractelement <4 x float> %430, i32 0
  %447 = extractelement <4 x float> %431, i32 0
  %448 = extractelement <4 x float> %432, i32 0
  %449 = extractelement <4 x float> %433, i32 0
  %450 = extractelement <4 x float> %434, i32 0
  %451 = extractelement <4 x float> %435, i32 0
  %452 = extractelement <4 x float> %436, i32 0
  %453 = extractelement <4 x float> %437, i32 0
  %454 = extractelement <4 x float> %438, i32 0
  %455 = extractelement <4 x float> %439, i32 0
  %456 = extractelement <4 x float> %440, i32 0
  %457 = extractelement <4 x float> %425, i32 1
  %458 = extractelement <4 x float> %426, i32 1
  %459 = extractelement <4 x float> %427, i32 1
  %460 = extractelement <4 x float> %428, i32 1
  %461 = extractelement <4 x float> %429, i32 1
  %462 = extractelement <4 x float> %430, i32 1
  %463 = extractelement <4 x float> %431, i32 1
  %464 = extractelement <4 x float> %432, i32 1
  %465 = extractelement <4 x float> %433, i32 1
  %466 = extractelement <4 x float> %434, i32 1
  %467 = extractelement <4 x float> %435, i32 1
  %468 = extractelement <4 x float> %436, i32 1
  %469 = extractelement <4 x float> %437, i32 1
  %470 = extractelement <4 x float> %438, i32 1
  %471 = extractelement <4 x float> %439, i32 1
  %472 = extractelement <4 x float> %440, i32 1
  %473 = insertelement <4 x float> undef, float %441, i32 0
  %474 = insertelement <4 x float> undef, float %442, i32 0
  %475 = insertelement <4 x float> undef, float %443, i32 0
  %476 = insertelement <4 x float> undef, float %444, i32 0
  %477 = insertelement <4 x float> undef, float %445, i32 0
  %478 = insertelement <4 x float> undef, float %446, i32 0
  %479 = insertelement <4 x float> undef, float %447, i32 0
  %480 = insertelement <4 x float> undef, float %448, i32 0
  %481 = insertelement <4 x float> undef, float %449, i32 0
  %482 = insertelement <4 x float> undef, float %450, i32 0
  %483 = insertelement <4 x float> undef, float %451, i32 0
  %484 = insertelement <4 x float> undef, float %452, i32 0
  %485 = insertelement <4 x float> undef, float %453, i32 0
  %486 = insertelement <4 x float> undef, float %454, i32 0
  %487 = insertelement <4 x float> undef, float %455, i32 0
  %488 = insertelement <4 x float> undef, float %456, i32 0
  %489 = insertelement <4 x float> %473, float %457, i32 1
  %490 = insertelement <4 x float> %474, float %458, i32 1
  %491 = insertelement <4 x float> %475, float %459, i32 1
  %492 = insertelement <4 x float> %476, float %460, i32 1
  %493 = insertelement <4 x float> %477, float %461, i32 1
  %494 = insertelement <4 x float> %478, float %462, i32 1
  %495 = insertelement <4 x float> %479, float %463, i32 1
  %496 = insertelement <4 x float> %480, float %464, i32 1
  %497 = insertelement <4 x float> %481, float %465, i32 1
  %498 = insertelement <4 x float> %482, float %466, i32 1
  %499 = insertelement <4 x float> %483, float %467, i32 1
  %500 = insertelement <4 x float> %484, float %468, i32 1
  %501 = insertelement <4 x float> %485, float %469, i32 1
  %502 = insertelement <4 x float> %486, float %470, i32 1
  %503 = insertelement <4 x float> %487, float %471, i32 1
  %504 = insertelement <4 x float> %488, float %472, i32 1
  %505 = insertelement <4 x float> %489, float %extract351.i, i32 2
  %506 = insertelement <4 x float> %490, float %extract352.i, i32 2
  %507 = insertelement <4 x float> %491, float %extract353.i, i32 2
  %508 = insertelement <4 x float> %492, float %extract354.i, i32 2
  %509 = insertelement <4 x float> %493, float %extract355.i, i32 2
  %510 = insertelement <4 x float> %494, float %extract356.i, i32 2
  %511 = insertelement <4 x float> %495, float %extract357.i, i32 2
  %512 = insertelement <4 x float> %496, float %extract358.i, i32 2
  %513 = insertelement <4 x float> %497, float %extract359.i, i32 2
  %514 = insertelement <4 x float> %498, float %extract360.i, i32 2
  %515 = insertelement <4 x float> %499, float %extract361.i, i32 2
  %516 = insertelement <4 x float> %500, float %extract362.i, i32 2
  %517 = insertelement <4 x float> %501, float %extract363.i, i32 2
  %518 = insertelement <4 x float> %502, float %extract364.i, i32 2
  %519 = insertelement <4 x float> %503, float %extract365.i, i32 2
  %520 = insertelement <4 x float> %504, float %extract366.i, i32 2
  %521 = shufflevector <4 x float> %505, <4 x float> %425, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %522 = shufflevector <4 x float> %506, <4 x float> %426, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %523 = shufflevector <4 x float> %507, <4 x float> %427, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %524 = shufflevector <4 x float> %508, <4 x float> %428, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %525 = shufflevector <4 x float> %509, <4 x float> %429, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %526 = shufflevector <4 x float> %510, <4 x float> %430, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %527 = shufflevector <4 x float> %511, <4 x float> %431, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %528 = shufflevector <4 x float> %512, <4 x float> %432, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %529 = shufflevector <4 x float> %513, <4 x float> %433, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %530 = shufflevector <4 x float> %514, <4 x float> %434, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %531 = shufflevector <4 x float> %515, <4 x float> %435, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %532 = shufflevector <4 x float> %516, <4 x float> %436, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %533 = shufflevector <4 x float> %517, <4 x float> %437, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %534 = shufflevector <4 x float> %518, <4 x float> %438, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %535 = shufflevector <4 x float> %519, <4 x float> %439, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %536 = shufflevector <4 x float> %520, <4 x float> %440, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %521, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %522, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %523, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %524, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %525, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %526, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %527, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %528, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %529, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %530, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %531, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %532, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %533, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %534, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %535, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %536, <4 x float> addrspace(1)* %56, align 16
  %537 = load <4 x float> addrspace(1)* %25, align 16
  %538 = load <4 x float> addrspace(1)* %26, align 16
  %539 = load <4 x float> addrspace(1)* %27, align 16
  %540 = load <4 x float> addrspace(1)* %28, align 16
  %541 = load <4 x float> addrspace(1)* %29, align 16
  %542 = load <4 x float> addrspace(1)* %30, align 16
  %543 = load <4 x float> addrspace(1)* %31, align 16
  %544 = load <4 x float> addrspace(1)* %32, align 16
  %545 = load <4 x float> addrspace(1)* %33, align 16
  %546 = load <4 x float> addrspace(1)* %34, align 16
  %547 = load <4 x float> addrspace(1)* %35, align 16
  %548 = load <4 x float> addrspace(1)* %36, align 16
  %549 = load <4 x float> addrspace(1)* %37, align 16
  %550 = load <4 x float> addrspace(1)* %38, align 16
  %551 = load <4 x float> addrspace(1)* %39, align 16
  %552 = load <4 x float> addrspace(1)* %40, align 16
  %553 = insertelement <4 x float> undef, float %441, i32 0
  %554 = insertelement <4 x float> undef, float %442, i32 0
  %555 = insertelement <4 x float> undef, float %443, i32 0
  %556 = insertelement <4 x float> undef, float %444, i32 0
  %557 = insertelement <4 x float> undef, float %445, i32 0
  %558 = insertelement <4 x float> undef, float %446, i32 0
  %559 = insertelement <4 x float> undef, float %447, i32 0
  %560 = insertelement <4 x float> undef, float %448, i32 0
  %561 = insertelement <4 x float> undef, float %449, i32 0
  %562 = insertelement <4 x float> undef, float %450, i32 0
  %563 = insertelement <4 x float> undef, float %451, i32 0
  %564 = insertelement <4 x float> undef, float %452, i32 0
  %565 = insertelement <4 x float> undef, float %453, i32 0
  %566 = insertelement <4 x float> undef, float %454, i32 0
  %567 = insertelement <4 x float> undef, float %455, i32 0
  %568 = insertelement <4 x float> undef, float %456, i32 0
  %569 = insertelement <4 x float> %553, float %457, i32 1
  %570 = insertelement <4 x float> %554, float %458, i32 1
  %571 = insertelement <4 x float> %555, float %459, i32 1
  %572 = insertelement <4 x float> %556, float %460, i32 1
  %573 = insertelement <4 x float> %557, float %461, i32 1
  %574 = insertelement <4 x float> %558, float %462, i32 1
  %575 = insertelement <4 x float> %559, float %463, i32 1
  %576 = insertelement <4 x float> %560, float %464, i32 1
  %577 = insertelement <4 x float> %561, float %465, i32 1
  %578 = insertelement <4 x float> %562, float %466, i32 1
  %579 = insertelement <4 x float> %563, float %467, i32 1
  %580 = insertelement <4 x float> %564, float %468, i32 1
  %581 = insertelement <4 x float> %565, float %469, i32 1
  %582 = insertelement <4 x float> %566, float %470, i32 1
  %583 = insertelement <4 x float> %567, float %471, i32 1
  %584 = insertelement <4 x float> %568, float %472, i32 1
  %585 = insertelement <4 x float> %569, float %extract351.i, i32 2
  %586 = insertelement <4 x float> %570, float %extract352.i, i32 2
  %587 = insertelement <4 x float> %571, float %extract353.i, i32 2
  %588 = insertelement <4 x float> %572, float %extract354.i, i32 2
  %589 = insertelement <4 x float> %573, float %extract355.i, i32 2
  %590 = insertelement <4 x float> %574, float %extract356.i, i32 2
  %591 = insertelement <4 x float> %575, float %extract357.i, i32 2
  %592 = insertelement <4 x float> %576, float %extract358.i, i32 2
  %593 = insertelement <4 x float> %577, float %extract359.i, i32 2
  %594 = insertelement <4 x float> %578, float %extract360.i, i32 2
  %595 = insertelement <4 x float> %579, float %extract361.i, i32 2
  %596 = insertelement <4 x float> %580, float %extract362.i, i32 2
  %597 = insertelement <4 x float> %581, float %extract363.i, i32 2
  %598 = insertelement <4 x float> %582, float %extract364.i, i32 2
  %599 = insertelement <4 x float> %583, float %extract365.i, i32 2
  %600 = insertelement <4 x float> %584, float %extract366.i, i32 2
  %601 = shufflevector <4 x float> %585, <4 x float> %537, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %602 = shufflevector <4 x float> %586, <4 x float> %538, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %603 = shufflevector <4 x float> %587, <4 x float> %539, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %604 = shufflevector <4 x float> %588, <4 x float> %540, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %605 = shufflevector <4 x float> %589, <4 x float> %541, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %606 = shufflevector <4 x float> %590, <4 x float> %542, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %607 = shufflevector <4 x float> %591, <4 x float> %543, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %608 = shufflevector <4 x float> %592, <4 x float> %544, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %609 = shufflevector <4 x float> %593, <4 x float> %545, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %610 = shufflevector <4 x float> %594, <4 x float> %546, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %611 = shufflevector <4 x float> %595, <4 x float> %547, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %612 = shufflevector <4 x float> %596, <4 x float> %548, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %613 = shufflevector <4 x float> %597, <4 x float> %549, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %614 = shufflevector <4 x float> %598, <4 x float> %550, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %615 = shufflevector <4 x float> %599, <4 x float> %551, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %616 = shufflevector <4 x float> %600, <4 x float> %552, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  store <4 x float> %601, <4 x float> addrspace(1)* %41, align 16
  store <4 x float> %602, <4 x float> addrspace(1)* %42, align 16
  store <4 x float> %603, <4 x float> addrspace(1)* %43, align 16
  store <4 x float> %604, <4 x float> addrspace(1)* %44, align 16
  store <4 x float> %605, <4 x float> addrspace(1)* %45, align 16
  store <4 x float> %606, <4 x float> addrspace(1)* %46, align 16
  store <4 x float> %607, <4 x float> addrspace(1)* %47, align 16
  store <4 x float> %608, <4 x float> addrspace(1)* %48, align 16
  store <4 x float> %609, <4 x float> addrspace(1)* %49, align 16
  store <4 x float> %610, <4 x float> addrspace(1)* %50, align 16
  store <4 x float> %611, <4 x float> addrspace(1)* %51, align 16
  store <4 x float> %612, <4 x float> addrspace(1)* %52, align 16
  store <4 x float> %613, <4 x float> addrspace(1)* %53, align 16
  store <4 x float> %614, <4 x float> addrspace(1)* %54, align 16
  store <4 x float> %615, <4 x float> addrspace(1)* %55, align 16
  store <4 x float> %616, <4 x float> addrspace(1)* %56, align 16
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp.i
  br i1 %exitcond.i, label %._crit_edge.i, label %24

._crit_edge.i:                                    ; preds = %24, %SyncBB367.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.intel_rgb2yuv_vector_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB367.i

____Vectorized_.intel_rgb2yuv_vector_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__intel_rgb2yuv_scalar_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int", metadata !"opencl_intel_rgb2yuv_scalar_locals_anchor", void (i8*)* @intel_rgb2yuv_scalar}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__intel_rgb2yuv_vector_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int", metadata !"opencl_intel_rgb2yuv_vector_locals_anchor", void (i8*)* @intel_rgb2yuv_vector}
