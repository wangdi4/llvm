; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc
;
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }
%struct._image2d_t = type opaque

declare void @__readImg_original(i32, float addrspace(1)* nocapture, %struct._image2d_t*, i32, i32, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t*, i32, double)

declare void @__readInCache_original(i32, float addrspace(1)* nocapture, %struct._image2d_t*, i32) nounwind

declare void @__readRand_original(i32, float addrspace(1)* nocapture, %struct._image2d_t*, i32, i32, i32) nounwind

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__readImg_separated_args(i32 %n, float addrspace(1)* nocapture %d_out, %struct._image2d_t* %img, i32 %samp, i32 %w, i32 %h, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = add nsw i32 %w, -1
  %1 = icmp sgt i32 %n, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %7 = trunc i64 %6 to i32
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %9 = load i64* %8, align 8
  %10 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %11 = load i64* %10, align 8
  %12 = add i64 %9, %11
  %13 = sext i32 %7 to i64
  %14 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 1
  %15 = load i64* %14, align 8
  %16 = mul i64 %13, %15
  %17 = add i64 %16, %12
  br i1 %1, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %18 = trunc i64 %12 to i32
  %19 = insertelement <2 x i32> undef, i32 %7, i32 0
  %20 = insertelement <2 x i32> %19, i32 %18, i32 1
  br label %21

; <label>:21                                      ; preds = %21, %bb.nph
  %i.05 = phi i32 [ 0, %bb.nph ], [ %25, %21 ]
  %sum.04 = phi float [ 0.000000e+00, %bb.nph ], [ %24, %21 ]
  %ridx.03 = phi <2 x i32> [ %20, %bb.nph ], [ %29, %21 ]
  %tmp2 = bitcast <2 x i32> %ridx.03 to <1 x double>
  %tmp1 = extractelement <1 x double> %tmp2, i32 0
  %22 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %tmp1) nounwind
  %23 = extractelement <4 x float> %22, i32 0
  %24 = fadd float %sum.04, %23
  %25 = add nsw i32 %i.05, 1
  %26 = extractelement <2 x i32> %ridx.03, i32 0
  %27 = add nsw i32 %26, 1
  %28 = and i32 %27, %0
  %29 = insertelement <2 x i32> %ridx.03, i32 %28, i32 0
  %exitcond = icmp eq i32 %25, %n
  br i1 %exitcond, label %._crit_edge, label %21

._crit_edge:                                      ; preds = %21, %SyncBB
  %sum.0.lcssa = phi float [ 0.000000e+00, %SyncBB ], [ %24, %21 ]
  %sext = shl i64 %17, 32
  %30 = ashr i64 %sext, 32
  %31 = getelementptr inbounds float addrspace(1)* %d_out, i64 %30
  store float %sum.0.lcssa, float addrspace(1)* %31, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB6

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB6:                                          ; preds = %._crit_edge
  ret void
}

define void @__readInCache_separated_args(i32 %n, float addrspace(1)* nocapture %d_out, %struct._image2d_t* %img, i32 %samp, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %n, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %8 = load i64* %7, align 8
  %9 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %10 = load i64* %9, align 8
  %11 = add i64 %8, %10
  %12 = sext i32 %6 to i64
  %13 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 1
  %14 = load i64* %13, align 8
  %15 = mul i64 %12, %14
  %16 = add i64 %15, %11
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %17 = trunc i64 %11 to i32
  %18 = insertelement <2 x i32> undef, i32 %6, i32 0
  %19 = insertelement <2 x i32> %18, i32 %17, i32 1
  %tmp2 = bitcast <2 x i32> %19 to <1 x double>
  %tmp1 = extractelement <1 x double> %tmp2, i32 0
  br label %20

; <label>:20                                      ; preds = %20, %bb.nph
  %i.04 = phi i32 [ 0, %bb.nph ], [ %24, %20 ]
  %sum.03 = phi float [ 0.000000e+00, %bb.nph ], [ %23, %20 ]
  %21 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %tmp1) nounwind
  %22 = extractelement <4 x float> %21, i32 0
  %23 = fadd float %sum.03, %22
  %24 = add nsw i32 %i.04, 1
  %exitcond = icmp eq i32 %24, %n
  br i1 %exitcond, label %._crit_edge, label %20

._crit_edge:                                      ; preds = %20, %SyncBB
  %sum.0.lcssa = phi float [ 0.000000e+00, %SyncBB ], [ %23, %20 ]
  %sext = shl i64 %16, 32
  %25 = ashr i64 %sext, 32
  %26 = getelementptr inbounds float addrspace(1)* %d_out, i64 %25
  store float %sum.0.lcssa, float addrspace(1)* %26, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB5

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB5:                                          ; preds = %._crit_edge
  ret void
}

define void @__readRand_separated_args(i32 %n, float addrspace(1)* nocapture %d_out, %struct._image2d_t* %img, i32 %samp, i32 %w, i32 %h, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = add nsw i32 %w, -1
  %1 = add nsw i32 %h, -1
  %2 = icmp sgt i32 %n, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %8 = trunc i64 %7 to i32
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %10 = load i64* %9, align 8
  %11 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %12 = load i64* %11, align 8
  %13 = add i64 %10, %12
  %14 = sext i32 %8 to i64
  %15 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 1
  %16 = load i64* %15, align 8
  %17 = mul i64 %14, %16
  %18 = add i64 %17, %13
  br i1 %2, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %19 = trunc i64 %13 to i32
  %20 = insertelement <2 x i32> undef, i32 %8, i32 0
  %21 = insertelement <2 x i32> %20, i32 %19, i32 1
  br label %22

; <label>:22                                      ; preds = %22, %bb.nph
  %i.05 = phi i32 [ 0, %bb.nph ], [ %26, %22 ]
  %sum.04 = phi float [ 0.000000e+00, %bb.nph ], [ %25, %22 ]
  %ridx.03 = phi <2 x i32> [ %21, %bb.nph ], [ %36, %22 ]
  %tmp2 = bitcast <2 x i32> %ridx.03 to <1 x double>
  %tmp1 = extractelement <1 x double> %tmp2, i32 0
  %23 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %tmp1) nounwind
  %24 = extractelement <4 x float> %23, i32 0
  %25 = fadd float %sum.04, %24
  %26 = add nsw i32 %i.05, 1
  %27 = extractelement <2 x i32> %ridx.03, i32 0
  %28 = extractelement <2 x i32> %ridx.03, i32 1
  %29 = mul nsw i32 %27, 3
  %30 = mul nsw i32 %28, 5
  %31 = add nsw i32 %29, 29
  %32 = add nsw i32 %30, 11
  %33 = and i32 %31, %0
  %34 = and i32 %32, %1
  %35 = insertelement <2 x i32> undef, i32 %33, i32 0
  %36 = insertelement <2 x i32> %35, i32 %34, i32 1
  %exitcond = icmp eq i32 %26, %n
  br i1 %exitcond, label %._crit_edge, label %22

._crit_edge:                                      ; preds = %22, %SyncBB
  %sum.0.lcssa = phi float [ 0.000000e+00, %SyncBB ], [ %25, %22 ]
  %sext = shl i64 %18, 32
  %37 = ashr i64 %sext, 32
  %38 = getelementptr inbounds float addrspace(1)* %d_out, i64 %37
  store float %sum.0.lcssa, float addrspace(1)* %38, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB6

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB6:                                          ; preds = %._crit_edge
  ret void
}

define void @readInCache(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32*
  %1 = load i32* %0, align 4
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to %struct._image2d_t**
  %7 = load %struct._image2d_t** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to %struct.WorkDim**
  %13 = load %struct.WorkDim** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = icmp sgt i32 %1, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %24 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %25 = load i64* %24, align 8
  %26 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = add i64 %25, %27
  %29 = trunc i64 %28 to i32
  %30 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 1
  %31 = load i64* %30, align 8
  %32 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 1
  %33 = load i64* %32, align 8
  %34 = add i64 %31, %33
  %35 = sext i32 %29 to i64
  %36 = getelementptr %struct.WorkDim* %13, i64 0, i32 2, i64 1
  %37 = load i64* %36, align 8
  %38 = mul i64 %35, %37
  %39 = add i64 %38, %34
  br i1 %23, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %40 = trunc i64 %34 to i32
  %41 = insertelement <2 x i32> undef, i32 %29, i32 0
  %42 = insertelement <2 x i32> %41, i32 %40, i32 1
  %tmp2.i = bitcast <2 x i32> %42 to <1 x double>
  %tmp1.i = extractelement <1 x double> %tmp2.i, i32 0
  br label %43

; <label>:43                                      ; preds = %43, %bb.nph.i
  %i.04.i = phi i32 [ 0, %bb.nph.i ], [ %47, %43 ]
  %sum.03.i = phi float [ 0.000000e+00, %bb.nph.i ], [ %46, %43 ]
  %44 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %tmp1.i) nounwind
  %45 = extractelement <4 x float> %44, i32 0
  %46 = fadd float %sum.03.i, %45
  %47 = add nsw i32 %i.04.i, 1
  %exitcond.i = icmp eq i32 %47, %1
  br i1 %exitcond.i, label %._crit_edge.i, label %43

._crit_edge.i:                                    ; preds = %43, %SyncBB.i
  %sum.0.lcssa.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %46, %43 ]
  %sext.i = shl i64 %39, 32
  %48 = ashr i64 %sext.i, 32
  %49 = getelementptr inbounds float addrspace(1)* %4, i64 %48
  store float %sum.0.lcssa.i, float addrspace(1)* %49, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %__readInCache_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__readInCache_separated_args.exit:                ; preds = %._crit_edge.i
  ret void
}

define void @readRand(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32*
  %1 = load i32* %0, align 4
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to %struct._image2d_t**
  %7 = load %struct._image2d_t** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to %struct.WorkDim**
  %19 = load %struct.WorkDim** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 64
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %29 = add nsw i32 %13, -1
  %30 = add nsw i32 %16, -1
  %31 = icmp sgt i32 %1, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %32 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %33 = load i64* %32, align 8
  %34 = getelementptr %struct.PaddedDimId* %22, i64 0, i32 0, i64 0
  %35 = load i64* %34, align 8
  %36 = add i64 %33, %35
  %37 = trunc i64 %36 to i32
  %38 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 1
  %39 = load i64* %38, align 8
  %40 = getelementptr %struct.PaddedDimId* %22, i64 0, i32 0, i64 1
  %41 = load i64* %40, align 8
  %42 = add i64 %39, %41
  %43 = sext i32 %37 to i64
  %44 = getelementptr %struct.WorkDim* %19, i64 0, i32 2, i64 1
  %45 = load i64* %44, align 8
  %46 = mul i64 %43, %45
  %47 = add i64 %46, %42
  br i1 %31, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %48 = trunc i64 %42 to i32
  %49 = insertelement <2 x i32> undef, i32 %37, i32 0
  %50 = insertelement <2 x i32> %49, i32 %48, i32 1
  br label %51

; <label>:51                                      ; preds = %51, %bb.nph.i
  %i.05.i = phi i32 [ 0, %bb.nph.i ], [ %55, %51 ]
  %sum.04.i = phi float [ 0.000000e+00, %bb.nph.i ], [ %54, %51 ]
  %ridx.03.i = phi <2 x i32> [ %50, %bb.nph.i ], [ %65, %51 ]
  %tmp2.i = bitcast <2 x i32> %ridx.03.i to <1 x double>
  %tmp1.i = extractelement <1 x double> %tmp2.i, i32 0
  %52 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %tmp1.i) nounwind
  %53 = extractelement <4 x float> %52, i32 0
  %54 = fadd float %sum.04.i, %53
  %55 = add nsw i32 %i.05.i, 1
  %56 = extractelement <2 x i32> %ridx.03.i, i32 0
  %57 = extractelement <2 x i32> %ridx.03.i, i32 1
  %58 = mul nsw i32 %56, 3
  %59 = mul nsw i32 %57, 5
  %60 = add nsw i32 %58, 29
  %61 = add nsw i32 %59, 11
  %62 = and i32 %60, %29
  %63 = and i32 %61, %30
  %64 = insertelement <2 x i32> undef, i32 %62, i32 0
  %65 = insertelement <2 x i32> %64, i32 %63, i32 1
  %exitcond.i = icmp eq i32 %55, %1
  br i1 %exitcond.i, label %._crit_edge.i, label %51

._crit_edge.i:                                    ; preds = %51, %SyncBB.i
  %sum.0.lcssa.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %54, %51 ]
  %sext.i = shl i64 %47, 32
  %66 = ashr i64 %sext.i, 32
  %67 = getelementptr inbounds float addrspace(1)* %4, i64 %66
  store float %sum.0.lcssa.i, float addrspace(1)* %67, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %__readRand_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__readRand_separated_args.exit:                   ; preds = %._crit_edge.i
  ret void
}

define void @readImg(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32*
  %1 = load i32* %0, align 4
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to %struct._image2d_t**
  %7 = load %struct._image2d_t** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 48
  %15 = bitcast i8* %14 to %struct.WorkDim**
  %16 = load %struct.WorkDim** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = add nsw i32 %13, -1
  %27 = icmp sgt i32 %1, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %28 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %29 = load i64* %28, align 8
  %30 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %31 = load i64* %30, align 8
  %32 = add i64 %29, %31
  %33 = trunc i64 %32 to i32
  %34 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 1
  %35 = load i64* %34, align 8
  %36 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 1
  %37 = load i64* %36, align 8
  %38 = add i64 %35, %37
  %39 = sext i32 %33 to i64
  %40 = getelementptr %struct.WorkDim* %16, i64 0, i32 2, i64 1
  %41 = load i64* %40, align 8
  %42 = mul i64 %39, %41
  %43 = add i64 %42, %38
  br i1 %27, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %44 = trunc i64 %38 to i32
  %45 = insertelement <2 x i32> undef, i32 %33, i32 0
  %46 = insertelement <2 x i32> %45, i32 %44, i32 1
  br label %47

; <label>:47                                      ; preds = %47, %bb.nph.i
  %i.05.i = phi i32 [ 0, %bb.nph.i ], [ %51, %47 ]
  %sum.04.i = phi float [ 0.000000e+00, %bb.nph.i ], [ %50, %47 ]
  %ridx.03.i = phi <2 x i32> [ %46, %bb.nph.i ], [ %55, %47 ]
  %tmp2.i = bitcast <2 x i32> %ridx.03.i to <1 x double>
  %tmp1.i = extractelement <1 x double> %tmp2.i, i32 0
  %48 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %tmp1.i) nounwind
  %49 = extractelement <4 x float> %48, i32 0
  %50 = fadd float %sum.04.i, %49
  %51 = add nsw i32 %i.05.i, 1
  %52 = extractelement <2 x i32> %ridx.03.i, i32 0
  %53 = add nsw i32 %52, 1
  %54 = and i32 %53, %26
  %55 = insertelement <2 x i32> %ridx.03.i, i32 %54, i32 0
  %exitcond.i = icmp eq i32 %51, %1
  br i1 %exitcond.i, label %._crit_edge.i, label %47

._crit_edge.i:                                    ; preds = %47, %SyncBB.i
  %sum.0.lcssa.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %50, %47 ]
  %sext.i = shl i64 %43, 32
  %56 = ashr i64 %sext.i, 32
  %57 = getelementptr inbounds float addrspace(1)* %4, i64 %56
  store float %sum.0.lcssa.i, float addrspace(1)* %57, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %__readImg_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__readImg_separated_args.exit:                    ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0, !2, !3}

!0 = metadata !{void (i32, float addrspace(1)*, %struct._image2d_t*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readImg_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int, float __attribute__((address_space(1))) *, __rd image2d_t, sampler_t, int, int", metadata !"opencl_readImg_locals_anchor", void (i8*)* @readImg}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i32, float addrspace(1)*, %struct._image2d_t*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readInCache_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int, float __attribute__((address_space(1))) *, __rd image2d_t, sampler_t", metadata !"opencl_readInCache_locals_anchor", void (i8*)* @readInCache}
!3 = metadata !{void (i32, float addrspace(1)*, %struct._image2d_t*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readRand_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int, float __attribute__((address_space(1))) *, __rd image2d_t, sampler_t, int, int", metadata !"opencl_readRand_locals_anchor", void (i8*)* @readRand}
