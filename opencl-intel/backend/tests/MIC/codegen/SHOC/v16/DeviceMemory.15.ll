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

declare void @____Vectorized_.readImg_original(i32, float addrspace(1)* nocapture, %struct._image2d_t*, i32, i32, i32) nounwind

declare void @____Vectorized_.readInCache_original(i32, float addrspace(1)* nocapture, %struct._image2d_t*, i32) nounwind

declare void @____Vectorized_.readRand_original(i32, float addrspace(1)* nocapture, %struct._image2d_t*, i32, i32, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

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

define void @____Vectorized_.readImg_separated_args(i32 %n, float addrspace(1)* nocapture %d_out, %struct._image2d_t* %img, i32 %samp, i32 %w, i32 %h, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = add nsw i32 %w, -1
  %temp42 = insertelement <16 x i32> undef, i32 %0, i32 0
  %vector43 = shufflevector <16 x i32> %temp42, <16 x i32> undef, <16 x i32> zeroinitializer
  %1 = icmp sgt i32 %n, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %broadcast1 = insertelement <16 x i64> undef, i64 %6, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %7 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %8 = trunc <16 x i64> %7 to <16 x i32>
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %10 = load i64* %9, align 8
  %11 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %12 = load i64* %11, align 8
  %13 = add i64 %10, %12
  %temp8 = insertelement <16 x i64> undef, i64 %13, i32 0
  %vector9 = shufflevector <16 x i64> %temp8, <16 x i64> undef, <16 x i32> zeroinitializer
  %14 = sext <16 x i32> %8 to <16 x i64>
  %15 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 1
  %16 = load i64* %15, align 8
  %temp = insertelement <16 x i64> undef, i64 %16, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %17 = mul <16 x i64> %14, %vector
  %18 = add <16 x i64> %17, %vector9
  br i1 %1, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %19 = trunc i64 %13 to i32
  br label %20

; <label>:20                                      ; preds = %20, %bb.nph
  %i.05 = phi i32 [ 0, %bb.nph ], [ %118, %20 ]
  %vectorPHI = phi <16 x float> [ zeroinitializer, %bb.nph ], [ %117, %20 ]
  %vectorPHI10 = phi <16 x i32> [ %8, %bb.nph ], [ %120, %20 ]
  %extract = extractelement <16 x i32> %vectorPHI10, i32 0
  %extract11 = extractelement <16 x i32> %vectorPHI10, i32 1
  %extract12 = extractelement <16 x i32> %vectorPHI10, i32 2
  %extract13 = extractelement <16 x i32> %vectorPHI10, i32 3
  %extract14 = extractelement <16 x i32> %vectorPHI10, i32 4
  %extract15 = extractelement <16 x i32> %vectorPHI10, i32 5
  %extract16 = extractelement <16 x i32> %vectorPHI10, i32 6
  %extract17 = extractelement <16 x i32> %vectorPHI10, i32 7
  %extract18 = extractelement <16 x i32> %vectorPHI10, i32 8
  %extract19 = extractelement <16 x i32> %vectorPHI10, i32 9
  %extract20 = extractelement <16 x i32> %vectorPHI10, i32 10
  %extract21 = extractelement <16 x i32> %vectorPHI10, i32 11
  %extract22 = extractelement <16 x i32> %vectorPHI10, i32 12
  %extract23 = extractelement <16 x i32> %vectorPHI10, i32 13
  %extract24 = extractelement <16 x i32> %vectorPHI10, i32 14
  %extract25 = extractelement <16 x i32> %vectorPHI10, i32 15
  %21 = insertelement <2 x i32> undef, i32 %extract, i32 0
  %22 = insertelement <2 x i32> undef, i32 %extract11, i32 0
  %23 = insertelement <2 x i32> undef, i32 %extract12, i32 0
  %24 = insertelement <2 x i32> undef, i32 %extract13, i32 0
  %25 = insertelement <2 x i32> undef, i32 %extract14, i32 0
  %26 = insertelement <2 x i32> undef, i32 %extract15, i32 0
  %27 = insertelement <2 x i32> undef, i32 %extract16, i32 0
  %28 = insertelement <2 x i32> undef, i32 %extract17, i32 0
  %29 = insertelement <2 x i32> undef, i32 %extract18, i32 0
  %30 = insertelement <2 x i32> undef, i32 %extract19, i32 0
  %31 = insertelement <2 x i32> undef, i32 %extract20, i32 0
  %32 = insertelement <2 x i32> undef, i32 %extract21, i32 0
  %33 = insertelement <2 x i32> undef, i32 %extract22, i32 0
  %34 = insertelement <2 x i32> undef, i32 %extract23, i32 0
  %35 = insertelement <2 x i32> undef, i32 %extract24, i32 0
  %36 = insertelement <2 x i32> undef, i32 %extract25, i32 0
  %37 = insertelement <2 x i32> %21, i32 %19, i32 1
  %38 = insertelement <2 x i32> %22, i32 %19, i32 1
  %39 = insertelement <2 x i32> %23, i32 %19, i32 1
  %40 = insertelement <2 x i32> %24, i32 %19, i32 1
  %41 = insertelement <2 x i32> %25, i32 %19, i32 1
  %42 = insertelement <2 x i32> %26, i32 %19, i32 1
  %43 = insertelement <2 x i32> %27, i32 %19, i32 1
  %44 = insertelement <2 x i32> %28, i32 %19, i32 1
  %45 = insertelement <2 x i32> %29, i32 %19, i32 1
  %46 = insertelement <2 x i32> %30, i32 %19, i32 1
  %47 = insertelement <2 x i32> %31, i32 %19, i32 1
  %48 = insertelement <2 x i32> %32, i32 %19, i32 1
  %49 = insertelement <2 x i32> %33, i32 %19, i32 1
  %50 = insertelement <2 x i32> %34, i32 %19, i32 1
  %51 = insertelement <2 x i32> %35, i32 %19, i32 1
  %52 = insertelement <2 x i32> %36, i32 %19, i32 1
  %53 = bitcast <2 x i32> %37 to <1 x double>
  %54 = bitcast <2 x i32> %38 to <1 x double>
  %55 = bitcast <2 x i32> %39 to <1 x double>
  %56 = bitcast <2 x i32> %40 to <1 x double>
  %57 = bitcast <2 x i32> %41 to <1 x double>
  %58 = bitcast <2 x i32> %42 to <1 x double>
  %59 = bitcast <2 x i32> %43 to <1 x double>
  %60 = bitcast <2 x i32> %44 to <1 x double>
  %61 = bitcast <2 x i32> %45 to <1 x double>
  %62 = bitcast <2 x i32> %46 to <1 x double>
  %63 = bitcast <2 x i32> %47 to <1 x double>
  %64 = bitcast <2 x i32> %48 to <1 x double>
  %65 = bitcast <2 x i32> %49 to <1 x double>
  %66 = bitcast <2 x i32> %50 to <1 x double>
  %67 = bitcast <2 x i32> %51 to <1 x double>
  %68 = bitcast <2 x i32> %52 to <1 x double>
  %69 = extractelement <1 x double> %53, i32 0
  %70 = extractelement <1 x double> %54, i32 0
  %71 = extractelement <1 x double> %55, i32 0
  %72 = extractelement <1 x double> %56, i32 0
  %73 = extractelement <1 x double> %57, i32 0
  %74 = extractelement <1 x double> %58, i32 0
  %75 = extractelement <1 x double> %59, i32 0
  %76 = extractelement <1 x double> %60, i32 0
  %77 = extractelement <1 x double> %61, i32 0
  %78 = extractelement <1 x double> %62, i32 0
  %79 = extractelement <1 x double> %63, i32 0
  %80 = extractelement <1 x double> %64, i32 0
  %81 = extractelement <1 x double> %65, i32 0
  %82 = extractelement <1 x double> %66, i32 0
  %83 = extractelement <1 x double> %67, i32 0
  %84 = extractelement <1 x double> %68, i32 0
  %85 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %69) nounwind
  %86 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %70) nounwind
  %87 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %71) nounwind
  %88 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %72) nounwind
  %89 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %73) nounwind
  %90 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %74) nounwind
  %91 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %75) nounwind
  %92 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %76) nounwind
  %93 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %77) nounwind
  %94 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %78) nounwind
  %95 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %79) nounwind
  %96 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %80) nounwind
  %97 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %81) nounwind
  %98 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %82) nounwind
  %99 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %83) nounwind
  %100 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %84) nounwind
  %101 = extractelement <4 x float> %85, i32 0
  %102 = extractelement <4 x float> %86, i32 0
  %103 = extractelement <4 x float> %87, i32 0
  %104 = extractelement <4 x float> %88, i32 0
  %105 = extractelement <4 x float> %89, i32 0
  %106 = extractelement <4 x float> %90, i32 0
  %107 = extractelement <4 x float> %91, i32 0
  %108 = extractelement <4 x float> %92, i32 0
  %109 = extractelement <4 x float> %93, i32 0
  %110 = extractelement <4 x float> %94, i32 0
  %111 = extractelement <4 x float> %95, i32 0
  %112 = extractelement <4 x float> %96, i32 0
  %113 = extractelement <4 x float> %97, i32 0
  %114 = extractelement <4 x float> %98, i32 0
  %115 = extractelement <4 x float> %99, i32 0
  %116 = extractelement <4 x float> %100, i32 0
  %temp.vect26 = insertelement <16 x float> undef, float %101, i32 0
  %temp.vect27 = insertelement <16 x float> %temp.vect26, float %102, i32 1
  %temp.vect28 = insertelement <16 x float> %temp.vect27, float %103, i32 2
  %temp.vect29 = insertelement <16 x float> %temp.vect28, float %104, i32 3
  %temp.vect30 = insertelement <16 x float> %temp.vect29, float %105, i32 4
  %temp.vect31 = insertelement <16 x float> %temp.vect30, float %106, i32 5
  %temp.vect32 = insertelement <16 x float> %temp.vect31, float %107, i32 6
  %temp.vect33 = insertelement <16 x float> %temp.vect32, float %108, i32 7
  %temp.vect34 = insertelement <16 x float> %temp.vect33, float %109, i32 8
  %temp.vect35 = insertelement <16 x float> %temp.vect34, float %110, i32 9
  %temp.vect36 = insertelement <16 x float> %temp.vect35, float %111, i32 10
  %temp.vect37 = insertelement <16 x float> %temp.vect36, float %112, i32 11
  %temp.vect38 = insertelement <16 x float> %temp.vect37, float %113, i32 12
  %temp.vect39 = insertelement <16 x float> %temp.vect38, float %114, i32 13
  %temp.vect40 = insertelement <16 x float> %temp.vect39, float %115, i32 14
  %temp.vect41 = insertelement <16 x float> %temp.vect40, float %116, i32 15
  %117 = fadd <16 x float> %vectorPHI, %temp.vect41
  %118 = add nsw i32 %i.05, 1
  %119 = add nsw <16 x i32> %vectorPHI10, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %120 = and <16 x i32> %119, %vector43
  %exitcond = icmp eq i32 %118, %n
  br i1 %exitcond, label %._crit_edge, label %20

._crit_edge:                                      ; preds = %20, %SyncBB
  %vectorPHI44 = phi <16 x float> [ zeroinitializer, %SyncBB ], [ %117, %20 ]
  %extract62 = extractelement <16 x float> %vectorPHI44, i32 0
  %extract63 = extractelement <16 x float> %vectorPHI44, i32 1
  %extract64 = extractelement <16 x float> %vectorPHI44, i32 2
  %extract65 = extractelement <16 x float> %vectorPHI44, i32 3
  %extract66 = extractelement <16 x float> %vectorPHI44, i32 4
  %extract67 = extractelement <16 x float> %vectorPHI44, i32 5
  %extract68 = extractelement <16 x float> %vectorPHI44, i32 6
  %extract69 = extractelement <16 x float> %vectorPHI44, i32 7
  %extract70 = extractelement <16 x float> %vectorPHI44, i32 8
  %extract71 = extractelement <16 x float> %vectorPHI44, i32 9
  %extract72 = extractelement <16 x float> %vectorPHI44, i32 10
  %extract73 = extractelement <16 x float> %vectorPHI44, i32 11
  %extract74 = extractelement <16 x float> %vectorPHI44, i32 12
  %extract75 = extractelement <16 x float> %vectorPHI44, i32 13
  %extract76 = extractelement <16 x float> %vectorPHI44, i32 14
  %extract77 = extractelement <16 x float> %vectorPHI44, i32 15
  %sext45 = shl <16 x i64> %18, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %121 = ashr <16 x i64> %sext45, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract46 = extractelement <16 x i64> %121, i32 0
  %extract47 = extractelement <16 x i64> %121, i32 1
  %extract48 = extractelement <16 x i64> %121, i32 2
  %extract49 = extractelement <16 x i64> %121, i32 3
  %extract50 = extractelement <16 x i64> %121, i32 4
  %extract51 = extractelement <16 x i64> %121, i32 5
  %extract52 = extractelement <16 x i64> %121, i32 6
  %extract53 = extractelement <16 x i64> %121, i32 7
  %extract54 = extractelement <16 x i64> %121, i32 8
  %extract55 = extractelement <16 x i64> %121, i32 9
  %extract56 = extractelement <16 x i64> %121, i32 10
  %extract57 = extractelement <16 x i64> %121, i32 11
  %extract58 = extractelement <16 x i64> %121, i32 12
  %extract59 = extractelement <16 x i64> %121, i32 13
  %extract60 = extractelement <16 x i64> %121, i32 14
  %extract61 = extractelement <16 x i64> %121, i32 15
  %122 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract46
  %123 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract47
  %124 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract48
  %125 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract49
  %126 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract50
  %127 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract51
  %128 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract52
  %129 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract53
  %130 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract54
  %131 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract55
  %132 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract56
  %133 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract57
  %134 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract58
  %135 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract59
  %136 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract60
  %137 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract61
  store float %extract62, float addrspace(1)* %122, align 4
  store float %extract63, float addrspace(1)* %123, align 4
  store float %extract64, float addrspace(1)* %124, align 4
  store float %extract65, float addrspace(1)* %125, align 4
  store float %extract66, float addrspace(1)* %126, align 4
  store float %extract67, float addrspace(1)* %127, align 4
  store float %extract68, float addrspace(1)* %128, align 4
  store float %extract69, float addrspace(1)* %129, align 4
  store float %extract70, float addrspace(1)* %130, align 4
  store float %extract71, float addrspace(1)* %131, align 4
  store float %extract72, float addrspace(1)* %132, align 4
  store float %extract73, float addrspace(1)* %133, align 4
  store float %extract74, float addrspace(1)* %134, align 4
  store float %extract75, float addrspace(1)* %135, align 4
  store float %extract76, float addrspace(1)* %136, align 4
  store float %extract77, float addrspace(1)* %137, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB78

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB78:                                         ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.readInCache_separated_args(i32 %n, float addrspace(1)* nocapture %d_out, %struct._image2d_t* %img, i32 %samp, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %n, 0
  br label %SyncBB73

SyncBB73:                                         ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %broadcast1 = insertelement <16 x i64> undef, i64 %5, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %6 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %7 = trunc <16 x i64> %6 to <16 x i32>
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %9 = load i64* %8, align 8
  %10 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %11 = load i64* %10, align 8
  %12 = add i64 %9, %11
  %temp6 = insertelement <16 x i64> undef, i64 %12, i32 0
  %vector7 = shufflevector <16 x i64> %temp6, <16 x i64> undef, <16 x i32> zeroinitializer
  %13 = sext <16 x i32> %7 to <16 x i64>
  %14 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 1
  %15 = load i64* %14, align 8
  %temp = insertelement <16 x i64> undef, i64 %15, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %16 = mul <16 x i64> %13, %vector
  %17 = add <16 x i64> %16, %vector7
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB73
  %extract22 = extractelement <16 x i32> %7, i32 15
  %extract21 = extractelement <16 x i32> %7, i32 14
  %extract20 = extractelement <16 x i32> %7, i32 13
  %extract19 = extractelement <16 x i32> %7, i32 12
  %extract18 = extractelement <16 x i32> %7, i32 11
  %extract17 = extractelement <16 x i32> %7, i32 10
  %extract16 = extractelement <16 x i32> %7, i32 9
  %extract15 = extractelement <16 x i32> %7, i32 8
  %extract14 = extractelement <16 x i32> %7, i32 7
  %extract13 = extractelement <16 x i32> %7, i32 6
  %extract12 = extractelement <16 x i32> %7, i32 5
  %extract11 = extractelement <16 x i32> %7, i32 4
  %extract10 = extractelement <16 x i32> %7, i32 3
  %extract9 = extractelement <16 x i32> %7, i32 2
  %extract8 = extractelement <16 x i32> %7, i32 1
  %extract = extractelement <16 x i32> %7, i32 0
  %18 = trunc i64 %12 to i32
  %19 = insertelement <2 x i32> undef, i32 %extract, i32 0
  %20 = insertelement <2 x i32> undef, i32 %extract8, i32 0
  %21 = insertelement <2 x i32> undef, i32 %extract9, i32 0
  %22 = insertelement <2 x i32> undef, i32 %extract10, i32 0
  %23 = insertelement <2 x i32> undef, i32 %extract11, i32 0
  %24 = insertelement <2 x i32> undef, i32 %extract12, i32 0
  %25 = insertelement <2 x i32> undef, i32 %extract13, i32 0
  %26 = insertelement <2 x i32> undef, i32 %extract14, i32 0
  %27 = insertelement <2 x i32> undef, i32 %extract15, i32 0
  %28 = insertelement <2 x i32> undef, i32 %extract16, i32 0
  %29 = insertelement <2 x i32> undef, i32 %extract17, i32 0
  %30 = insertelement <2 x i32> undef, i32 %extract18, i32 0
  %31 = insertelement <2 x i32> undef, i32 %extract19, i32 0
  %32 = insertelement <2 x i32> undef, i32 %extract20, i32 0
  %33 = insertelement <2 x i32> undef, i32 %extract21, i32 0
  %34 = insertelement <2 x i32> undef, i32 %extract22, i32 0
  %35 = insertelement <2 x i32> %19, i32 %18, i32 1
  %36 = insertelement <2 x i32> %20, i32 %18, i32 1
  %37 = insertelement <2 x i32> %21, i32 %18, i32 1
  %38 = insertelement <2 x i32> %22, i32 %18, i32 1
  %39 = insertelement <2 x i32> %23, i32 %18, i32 1
  %40 = insertelement <2 x i32> %24, i32 %18, i32 1
  %41 = insertelement <2 x i32> %25, i32 %18, i32 1
  %42 = insertelement <2 x i32> %26, i32 %18, i32 1
  %43 = insertelement <2 x i32> %27, i32 %18, i32 1
  %44 = insertelement <2 x i32> %28, i32 %18, i32 1
  %45 = insertelement <2 x i32> %29, i32 %18, i32 1
  %46 = insertelement <2 x i32> %30, i32 %18, i32 1
  %47 = insertelement <2 x i32> %31, i32 %18, i32 1
  %48 = insertelement <2 x i32> %32, i32 %18, i32 1
  %49 = insertelement <2 x i32> %33, i32 %18, i32 1
  %50 = insertelement <2 x i32> %34, i32 %18, i32 1
  %51 = bitcast <2 x i32> %35 to <1 x double>
  %52 = bitcast <2 x i32> %36 to <1 x double>
  %53 = bitcast <2 x i32> %37 to <1 x double>
  %54 = bitcast <2 x i32> %38 to <1 x double>
  %55 = bitcast <2 x i32> %39 to <1 x double>
  %56 = bitcast <2 x i32> %40 to <1 x double>
  %57 = bitcast <2 x i32> %41 to <1 x double>
  %58 = bitcast <2 x i32> %42 to <1 x double>
  %59 = bitcast <2 x i32> %43 to <1 x double>
  %60 = bitcast <2 x i32> %44 to <1 x double>
  %61 = bitcast <2 x i32> %45 to <1 x double>
  %62 = bitcast <2 x i32> %46 to <1 x double>
  %63 = bitcast <2 x i32> %47 to <1 x double>
  %64 = bitcast <2 x i32> %48 to <1 x double>
  %65 = bitcast <2 x i32> %49 to <1 x double>
  %66 = bitcast <2 x i32> %50 to <1 x double>
  %67 = extractelement <1 x double> %51, i32 0
  %68 = extractelement <1 x double> %52, i32 0
  %69 = extractelement <1 x double> %53, i32 0
  %70 = extractelement <1 x double> %54, i32 0
  %71 = extractelement <1 x double> %55, i32 0
  %72 = extractelement <1 x double> %56, i32 0
  %73 = extractelement <1 x double> %57, i32 0
  %74 = extractelement <1 x double> %58, i32 0
  %75 = extractelement <1 x double> %59, i32 0
  %76 = extractelement <1 x double> %60, i32 0
  %77 = extractelement <1 x double> %61, i32 0
  %78 = extractelement <1 x double> %62, i32 0
  %79 = extractelement <1 x double> %63, i32 0
  %80 = extractelement <1 x double> %64, i32 0
  %81 = extractelement <1 x double> %65, i32 0
  %82 = extractelement <1 x double> %66, i32 0
  br label %83

; <label>:83                                      ; preds = %83, %bb.nph
  %i.04 = phi i32 [ 0, %bb.nph ], [ %117, %83 ]
  %vectorPHI = phi <16 x float> [ zeroinitializer, %bb.nph ], [ %116, %83 ]
  %84 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %67) nounwind
  %85 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %68) nounwind
  %86 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %69) nounwind
  %87 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %70) nounwind
  %88 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %71) nounwind
  %89 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %72) nounwind
  %90 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %73) nounwind
  %91 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %74) nounwind
  %92 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %75) nounwind
  %93 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %76) nounwind
  %94 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %77) nounwind
  %95 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %78) nounwind
  %96 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %79) nounwind
  %97 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %80) nounwind
  %98 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %81) nounwind
  %99 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %82) nounwind
  %100 = extractelement <4 x float> %84, i32 0
  %101 = extractelement <4 x float> %85, i32 0
  %102 = extractelement <4 x float> %86, i32 0
  %103 = extractelement <4 x float> %87, i32 0
  %104 = extractelement <4 x float> %88, i32 0
  %105 = extractelement <4 x float> %89, i32 0
  %106 = extractelement <4 x float> %90, i32 0
  %107 = extractelement <4 x float> %91, i32 0
  %108 = extractelement <4 x float> %92, i32 0
  %109 = extractelement <4 x float> %93, i32 0
  %110 = extractelement <4 x float> %94, i32 0
  %111 = extractelement <4 x float> %95, i32 0
  %112 = extractelement <4 x float> %96, i32 0
  %113 = extractelement <4 x float> %97, i32 0
  %114 = extractelement <4 x float> %98, i32 0
  %115 = extractelement <4 x float> %99, i32 0
  %temp.vect23 = insertelement <16 x float> undef, float %100, i32 0
  %temp.vect24 = insertelement <16 x float> %temp.vect23, float %101, i32 1
  %temp.vect25 = insertelement <16 x float> %temp.vect24, float %102, i32 2
  %temp.vect26 = insertelement <16 x float> %temp.vect25, float %103, i32 3
  %temp.vect27 = insertelement <16 x float> %temp.vect26, float %104, i32 4
  %temp.vect28 = insertelement <16 x float> %temp.vect27, float %105, i32 5
  %temp.vect29 = insertelement <16 x float> %temp.vect28, float %106, i32 6
  %temp.vect30 = insertelement <16 x float> %temp.vect29, float %107, i32 7
  %temp.vect31 = insertelement <16 x float> %temp.vect30, float %108, i32 8
  %temp.vect32 = insertelement <16 x float> %temp.vect31, float %109, i32 9
  %temp.vect33 = insertelement <16 x float> %temp.vect32, float %110, i32 10
  %temp.vect34 = insertelement <16 x float> %temp.vect33, float %111, i32 11
  %temp.vect35 = insertelement <16 x float> %temp.vect34, float %112, i32 12
  %temp.vect36 = insertelement <16 x float> %temp.vect35, float %113, i32 13
  %temp.vect37 = insertelement <16 x float> %temp.vect36, float %114, i32 14
  %temp.vect38 = insertelement <16 x float> %temp.vect37, float %115, i32 15
  %116 = fadd <16 x float> %vectorPHI, %temp.vect38
  %117 = add nsw i32 %i.04, 1
  %exitcond = icmp eq i32 %117, %n
  br i1 %exitcond, label %._crit_edge, label %83

._crit_edge:                                      ; preds = %83, %SyncBB73
  %vectorPHI39 = phi <16 x float> [ zeroinitializer, %SyncBB73 ], [ %116, %83 ]
  %extract57 = extractelement <16 x float> %vectorPHI39, i32 0
  %extract58 = extractelement <16 x float> %vectorPHI39, i32 1
  %extract59 = extractelement <16 x float> %vectorPHI39, i32 2
  %extract60 = extractelement <16 x float> %vectorPHI39, i32 3
  %extract61 = extractelement <16 x float> %vectorPHI39, i32 4
  %extract62 = extractelement <16 x float> %vectorPHI39, i32 5
  %extract63 = extractelement <16 x float> %vectorPHI39, i32 6
  %extract64 = extractelement <16 x float> %vectorPHI39, i32 7
  %extract65 = extractelement <16 x float> %vectorPHI39, i32 8
  %extract66 = extractelement <16 x float> %vectorPHI39, i32 9
  %extract67 = extractelement <16 x float> %vectorPHI39, i32 10
  %extract68 = extractelement <16 x float> %vectorPHI39, i32 11
  %extract69 = extractelement <16 x float> %vectorPHI39, i32 12
  %extract70 = extractelement <16 x float> %vectorPHI39, i32 13
  %extract71 = extractelement <16 x float> %vectorPHI39, i32 14
  %extract72 = extractelement <16 x float> %vectorPHI39, i32 15
  %sext40 = shl <16 x i64> %17, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %118 = ashr <16 x i64> %sext40, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract41 = extractelement <16 x i64> %118, i32 0
  %extract42 = extractelement <16 x i64> %118, i32 1
  %extract43 = extractelement <16 x i64> %118, i32 2
  %extract44 = extractelement <16 x i64> %118, i32 3
  %extract45 = extractelement <16 x i64> %118, i32 4
  %extract46 = extractelement <16 x i64> %118, i32 5
  %extract47 = extractelement <16 x i64> %118, i32 6
  %extract48 = extractelement <16 x i64> %118, i32 7
  %extract49 = extractelement <16 x i64> %118, i32 8
  %extract50 = extractelement <16 x i64> %118, i32 9
  %extract51 = extractelement <16 x i64> %118, i32 10
  %extract52 = extractelement <16 x i64> %118, i32 11
  %extract53 = extractelement <16 x i64> %118, i32 12
  %extract54 = extractelement <16 x i64> %118, i32 13
  %extract55 = extractelement <16 x i64> %118, i32 14
  %extract56 = extractelement <16 x i64> %118, i32 15
  %119 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract41
  %120 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract42
  %121 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract43
  %122 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract44
  %123 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract45
  %124 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract46
  %125 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract47
  %126 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract48
  %127 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract49
  %128 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract50
  %129 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract51
  %130 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract52
  %131 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract53
  %132 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract54
  %133 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract55
  %134 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract56
  store float %extract57, float addrspace(1)* %119, align 4
  store float %extract58, float addrspace(1)* %120, align 4
  store float %extract59, float addrspace(1)* %121, align 4
  store float %extract60, float addrspace(1)* %122, align 4
  store float %extract61, float addrspace(1)* %123, align 4
  store float %extract62, float addrspace(1)* %124, align 4
  store float %extract63, float addrspace(1)* %125, align 4
  store float %extract64, float addrspace(1)* %126, align 4
  store float %extract65, float addrspace(1)* %127, align 4
  store float %extract66, float addrspace(1)* %128, align 4
  store float %extract67, float addrspace(1)* %129, align 4
  store float %extract68, float addrspace(1)* %130, align 4
  store float %extract69, float addrspace(1)* %131, align 4
  store float %extract70, float addrspace(1)* %132, align 4
  store float %extract71, float addrspace(1)* %133, align 4
  store float %extract72, float addrspace(1)* %134, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB73

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.readRand_separated_args(i32 %n, float addrspace(1)* nocapture %d_out, %struct._image2d_t* %img, i32 %samp, i32 %w, i32 %h, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = add nsw i32 %w, -1
  %temp42 = insertelement <16 x i32> undef, i32 %0, i32 0
  %vector43 = shufflevector <16 x i32> %temp42, <16 x i32> undef, <16 x i32> zeroinitializer
  %1 = add nsw i32 %h, -1
  %2 = icmp sgt i32 %n, 0
  br label %SyncBB78

SyncBB78:                                         ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %broadcast1 = insertelement <16 x i64> undef, i64 %7, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %8 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %9 = trunc <16 x i64> %8 to <16 x i32>
  %10 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %11 = load i64* %10, align 8
  %12 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %13 = load i64* %12, align 8
  %14 = add i64 %11, %13
  %temp8 = insertelement <16 x i64> undef, i64 %14, i32 0
  %vector9 = shufflevector <16 x i64> %temp8, <16 x i64> undef, <16 x i32> zeroinitializer
  %15 = sext <16 x i32> %9 to <16 x i64>
  %16 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 1
  %17 = load i64* %16, align 8
  %temp = insertelement <16 x i64> undef, i64 %17, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %18 = mul <16 x i64> %15, %vector
  %19 = add <16 x i64> %18, %vector9
  br i1 %2, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB78
  %20 = trunc i64 %14 to i32
  br label %21

; <label>:21                                      ; preds = %21, %bb.nph
  %i.05 = phi i32 [ 0, %bb.nph ], [ %119, %21 ]
  %vectorPHI = phi <16 x float> [ zeroinitializer, %bb.nph ], [ %118, %21 ]
  %vectorPHI10 = phi <16 x i32> [ %9, %bb.nph ], [ %124, %21 ]
  %ridx.032 = phi i32 [ %20, %bb.nph ], [ %125, %21 ]
  %extract = extractelement <16 x i32> %vectorPHI10, i32 0
  %extract11 = extractelement <16 x i32> %vectorPHI10, i32 1
  %extract12 = extractelement <16 x i32> %vectorPHI10, i32 2
  %extract13 = extractelement <16 x i32> %vectorPHI10, i32 3
  %extract14 = extractelement <16 x i32> %vectorPHI10, i32 4
  %extract15 = extractelement <16 x i32> %vectorPHI10, i32 5
  %extract16 = extractelement <16 x i32> %vectorPHI10, i32 6
  %extract17 = extractelement <16 x i32> %vectorPHI10, i32 7
  %extract18 = extractelement <16 x i32> %vectorPHI10, i32 8
  %extract19 = extractelement <16 x i32> %vectorPHI10, i32 9
  %extract20 = extractelement <16 x i32> %vectorPHI10, i32 10
  %extract21 = extractelement <16 x i32> %vectorPHI10, i32 11
  %extract22 = extractelement <16 x i32> %vectorPHI10, i32 12
  %extract23 = extractelement <16 x i32> %vectorPHI10, i32 13
  %extract24 = extractelement <16 x i32> %vectorPHI10, i32 14
  %extract25 = extractelement <16 x i32> %vectorPHI10, i32 15
  %22 = insertelement <2 x i32> undef, i32 %extract, i32 0
  %23 = insertelement <2 x i32> undef, i32 %extract11, i32 0
  %24 = insertelement <2 x i32> undef, i32 %extract12, i32 0
  %25 = insertelement <2 x i32> undef, i32 %extract13, i32 0
  %26 = insertelement <2 x i32> undef, i32 %extract14, i32 0
  %27 = insertelement <2 x i32> undef, i32 %extract15, i32 0
  %28 = insertelement <2 x i32> undef, i32 %extract16, i32 0
  %29 = insertelement <2 x i32> undef, i32 %extract17, i32 0
  %30 = insertelement <2 x i32> undef, i32 %extract18, i32 0
  %31 = insertelement <2 x i32> undef, i32 %extract19, i32 0
  %32 = insertelement <2 x i32> undef, i32 %extract20, i32 0
  %33 = insertelement <2 x i32> undef, i32 %extract21, i32 0
  %34 = insertelement <2 x i32> undef, i32 %extract22, i32 0
  %35 = insertelement <2 x i32> undef, i32 %extract23, i32 0
  %36 = insertelement <2 x i32> undef, i32 %extract24, i32 0
  %37 = insertelement <2 x i32> undef, i32 %extract25, i32 0
  %38 = insertelement <2 x i32> %22, i32 %ridx.032, i32 1
  %39 = insertelement <2 x i32> %23, i32 %ridx.032, i32 1
  %40 = insertelement <2 x i32> %24, i32 %ridx.032, i32 1
  %41 = insertelement <2 x i32> %25, i32 %ridx.032, i32 1
  %42 = insertelement <2 x i32> %26, i32 %ridx.032, i32 1
  %43 = insertelement <2 x i32> %27, i32 %ridx.032, i32 1
  %44 = insertelement <2 x i32> %28, i32 %ridx.032, i32 1
  %45 = insertelement <2 x i32> %29, i32 %ridx.032, i32 1
  %46 = insertelement <2 x i32> %30, i32 %ridx.032, i32 1
  %47 = insertelement <2 x i32> %31, i32 %ridx.032, i32 1
  %48 = insertelement <2 x i32> %32, i32 %ridx.032, i32 1
  %49 = insertelement <2 x i32> %33, i32 %ridx.032, i32 1
  %50 = insertelement <2 x i32> %34, i32 %ridx.032, i32 1
  %51 = insertelement <2 x i32> %35, i32 %ridx.032, i32 1
  %52 = insertelement <2 x i32> %36, i32 %ridx.032, i32 1
  %53 = insertelement <2 x i32> %37, i32 %ridx.032, i32 1
  %54 = bitcast <2 x i32> %38 to <1 x double>
  %55 = bitcast <2 x i32> %39 to <1 x double>
  %56 = bitcast <2 x i32> %40 to <1 x double>
  %57 = bitcast <2 x i32> %41 to <1 x double>
  %58 = bitcast <2 x i32> %42 to <1 x double>
  %59 = bitcast <2 x i32> %43 to <1 x double>
  %60 = bitcast <2 x i32> %44 to <1 x double>
  %61 = bitcast <2 x i32> %45 to <1 x double>
  %62 = bitcast <2 x i32> %46 to <1 x double>
  %63 = bitcast <2 x i32> %47 to <1 x double>
  %64 = bitcast <2 x i32> %48 to <1 x double>
  %65 = bitcast <2 x i32> %49 to <1 x double>
  %66 = bitcast <2 x i32> %50 to <1 x double>
  %67 = bitcast <2 x i32> %51 to <1 x double>
  %68 = bitcast <2 x i32> %52 to <1 x double>
  %69 = bitcast <2 x i32> %53 to <1 x double>
  %70 = extractelement <1 x double> %54, i32 0
  %71 = extractelement <1 x double> %55, i32 0
  %72 = extractelement <1 x double> %56, i32 0
  %73 = extractelement <1 x double> %57, i32 0
  %74 = extractelement <1 x double> %58, i32 0
  %75 = extractelement <1 x double> %59, i32 0
  %76 = extractelement <1 x double> %60, i32 0
  %77 = extractelement <1 x double> %61, i32 0
  %78 = extractelement <1 x double> %62, i32 0
  %79 = extractelement <1 x double> %63, i32 0
  %80 = extractelement <1 x double> %64, i32 0
  %81 = extractelement <1 x double> %65, i32 0
  %82 = extractelement <1 x double> %66, i32 0
  %83 = extractelement <1 x double> %67, i32 0
  %84 = extractelement <1 x double> %68, i32 0
  %85 = extractelement <1 x double> %69, i32 0
  %86 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %70) nounwind
  %87 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %71) nounwind
  %88 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %72) nounwind
  %89 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %73) nounwind
  %90 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %74) nounwind
  %91 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %75) nounwind
  %92 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %76) nounwind
  %93 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %77) nounwind
  %94 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %78) nounwind
  %95 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %79) nounwind
  %96 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %80) nounwind
  %97 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %81) nounwind
  %98 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %82) nounwind
  %99 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %83) nounwind
  %100 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %84) nounwind
  %101 = tail call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %img, i32 %samp, double %85) nounwind
  %102 = extractelement <4 x float> %86, i32 0
  %103 = extractelement <4 x float> %87, i32 0
  %104 = extractelement <4 x float> %88, i32 0
  %105 = extractelement <4 x float> %89, i32 0
  %106 = extractelement <4 x float> %90, i32 0
  %107 = extractelement <4 x float> %91, i32 0
  %108 = extractelement <4 x float> %92, i32 0
  %109 = extractelement <4 x float> %93, i32 0
  %110 = extractelement <4 x float> %94, i32 0
  %111 = extractelement <4 x float> %95, i32 0
  %112 = extractelement <4 x float> %96, i32 0
  %113 = extractelement <4 x float> %97, i32 0
  %114 = extractelement <4 x float> %98, i32 0
  %115 = extractelement <4 x float> %99, i32 0
  %116 = extractelement <4 x float> %100, i32 0
  %117 = extractelement <4 x float> %101, i32 0
  %temp.vect26 = insertelement <16 x float> undef, float %102, i32 0
  %temp.vect27 = insertelement <16 x float> %temp.vect26, float %103, i32 1
  %temp.vect28 = insertelement <16 x float> %temp.vect27, float %104, i32 2
  %temp.vect29 = insertelement <16 x float> %temp.vect28, float %105, i32 3
  %temp.vect30 = insertelement <16 x float> %temp.vect29, float %106, i32 4
  %temp.vect31 = insertelement <16 x float> %temp.vect30, float %107, i32 5
  %temp.vect32 = insertelement <16 x float> %temp.vect31, float %108, i32 6
  %temp.vect33 = insertelement <16 x float> %temp.vect32, float %109, i32 7
  %temp.vect34 = insertelement <16 x float> %temp.vect33, float %110, i32 8
  %temp.vect35 = insertelement <16 x float> %temp.vect34, float %111, i32 9
  %temp.vect36 = insertelement <16 x float> %temp.vect35, float %112, i32 10
  %temp.vect37 = insertelement <16 x float> %temp.vect36, float %113, i32 11
  %temp.vect38 = insertelement <16 x float> %temp.vect37, float %114, i32 12
  %temp.vect39 = insertelement <16 x float> %temp.vect38, float %115, i32 13
  %temp.vect40 = insertelement <16 x float> %temp.vect39, float %116, i32 14
  %temp.vect41 = insertelement <16 x float> %temp.vect40, float %117, i32 15
  %118 = fadd <16 x float> %vectorPHI, %temp.vect41
  %119 = add nsw i32 %i.05, 1
  %120 = mul nsw <16 x i32> %vectorPHI10, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %121 = mul nsw i32 %ridx.032, 5
  %122 = add nsw <16 x i32> %120, <i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29>
  %123 = add nsw i32 %121, 11
  %124 = and <16 x i32> %122, %vector43
  %125 = and i32 %123, %1
  %exitcond = icmp eq i32 %119, %n
  br i1 %exitcond, label %._crit_edge, label %21

._crit_edge:                                      ; preds = %21, %SyncBB78
  %vectorPHI44 = phi <16 x float> [ zeroinitializer, %SyncBB78 ], [ %118, %21 ]
  %extract62 = extractelement <16 x float> %vectorPHI44, i32 0
  %extract63 = extractelement <16 x float> %vectorPHI44, i32 1
  %extract64 = extractelement <16 x float> %vectorPHI44, i32 2
  %extract65 = extractelement <16 x float> %vectorPHI44, i32 3
  %extract66 = extractelement <16 x float> %vectorPHI44, i32 4
  %extract67 = extractelement <16 x float> %vectorPHI44, i32 5
  %extract68 = extractelement <16 x float> %vectorPHI44, i32 6
  %extract69 = extractelement <16 x float> %vectorPHI44, i32 7
  %extract70 = extractelement <16 x float> %vectorPHI44, i32 8
  %extract71 = extractelement <16 x float> %vectorPHI44, i32 9
  %extract72 = extractelement <16 x float> %vectorPHI44, i32 10
  %extract73 = extractelement <16 x float> %vectorPHI44, i32 11
  %extract74 = extractelement <16 x float> %vectorPHI44, i32 12
  %extract75 = extractelement <16 x float> %vectorPHI44, i32 13
  %extract76 = extractelement <16 x float> %vectorPHI44, i32 14
  %extract77 = extractelement <16 x float> %vectorPHI44, i32 15
  %sext45 = shl <16 x i64> %19, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %126 = ashr <16 x i64> %sext45, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract46 = extractelement <16 x i64> %126, i32 0
  %extract47 = extractelement <16 x i64> %126, i32 1
  %extract48 = extractelement <16 x i64> %126, i32 2
  %extract49 = extractelement <16 x i64> %126, i32 3
  %extract50 = extractelement <16 x i64> %126, i32 4
  %extract51 = extractelement <16 x i64> %126, i32 5
  %extract52 = extractelement <16 x i64> %126, i32 6
  %extract53 = extractelement <16 x i64> %126, i32 7
  %extract54 = extractelement <16 x i64> %126, i32 8
  %extract55 = extractelement <16 x i64> %126, i32 9
  %extract56 = extractelement <16 x i64> %126, i32 10
  %extract57 = extractelement <16 x i64> %126, i32 11
  %extract58 = extractelement <16 x i64> %126, i32 12
  %extract59 = extractelement <16 x i64> %126, i32 13
  %extract60 = extractelement <16 x i64> %126, i32 14
  %extract61 = extractelement <16 x i64> %126, i32 15
  %127 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract46
  %128 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract47
  %129 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract48
  %130 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract49
  %131 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract50
  %132 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract51
  %133 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract52
  %134 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract53
  %135 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract54
  %136 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract55
  %137 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract56
  %138 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract57
  %139 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract58
  %140 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract59
  %141 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract60
  %142 = getelementptr inbounds float addrspace(1)* %d_out, i64 %extract61
  store float %extract62, float addrspace(1)* %127, align 4
  store float %extract63, float addrspace(1)* %128, align 4
  store float %extract64, float addrspace(1)* %129, align 4
  store float %extract65, float addrspace(1)* %130, align 4
  store float %extract66, float addrspace(1)* %131, align 4
  store float %extract67, float addrspace(1)* %132, align 4
  store float %extract68, float addrspace(1)* %133, align 4
  store float %extract69, float addrspace(1)* %134, align 4
  store float %extract70, float addrspace(1)* %135, align 4
  store float %extract71, float addrspace(1)* %136, align 4
  store float %extract72, float addrspace(1)* %137, align 4
  store float %extract73, float addrspace(1)* %138, align 4
  store float %extract74, float addrspace(1)* %139, align 4
  store float %extract75, float addrspace(1)* %140, align 4
  store float %extract76, float addrspace(1)* %141, align 4
  store float %extract77, float addrspace(1)* %142, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB78

SyncBB:                                           ; preds = %._crit_edge
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

define void @__Vectorized_.readImg(i8* %pBuffer) {
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
  %temp42.i = insertelement <16 x i32> undef, i32 %26, i32 0
  %vector43.i = shufflevector <16 x i32> %temp42.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %27 = icmp sgt i32 %1, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %28 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %29 = load i64* %28, align 8
  %30 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %31 = load i64* %30, align 8
  %32 = add i64 %29, %31
  %broadcast1.i = insertelement <16 x i64> undef, i64 %32, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %33 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %34 = trunc <16 x i64> %33 to <16 x i32>
  %35 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 1
  %36 = load i64* %35, align 8
  %37 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 1
  %38 = load i64* %37, align 8
  %39 = add i64 %36, %38
  %temp8.i = insertelement <16 x i64> undef, i64 %39, i32 0
  %vector9.i = shufflevector <16 x i64> %temp8.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %40 = sext <16 x i32> %34 to <16 x i64>
  %41 = getelementptr %struct.WorkDim* %16, i64 0, i32 2, i64 1
  %42 = load i64* %41, align 8
  %temp.i = insertelement <16 x i64> undef, i64 %42, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %43 = mul <16 x i64> %40, %vector.i
  %44 = add <16 x i64> %43, %vector9.i
  br i1 %27, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %45 = trunc i64 %39 to i32
  br label %46

; <label>:46                                      ; preds = %46, %bb.nph.i
  %i.05.i = phi i32 [ 0, %bb.nph.i ], [ %144, %46 ]
  %vectorPHI.i = phi <16 x float> [ zeroinitializer, %bb.nph.i ], [ %143, %46 ]
  %vectorPHI10.i = phi <16 x i32> [ %34, %bb.nph.i ], [ %146, %46 ]
  %extract.i = extractelement <16 x i32> %vectorPHI10.i, i32 0
  %extract11.i = extractelement <16 x i32> %vectorPHI10.i, i32 1
  %extract12.i = extractelement <16 x i32> %vectorPHI10.i, i32 2
  %extract13.i = extractelement <16 x i32> %vectorPHI10.i, i32 3
  %extract14.i = extractelement <16 x i32> %vectorPHI10.i, i32 4
  %extract15.i = extractelement <16 x i32> %vectorPHI10.i, i32 5
  %extract16.i = extractelement <16 x i32> %vectorPHI10.i, i32 6
  %extract17.i = extractelement <16 x i32> %vectorPHI10.i, i32 7
  %extract18.i = extractelement <16 x i32> %vectorPHI10.i, i32 8
  %extract19.i = extractelement <16 x i32> %vectorPHI10.i, i32 9
  %extract20.i = extractelement <16 x i32> %vectorPHI10.i, i32 10
  %extract21.i = extractelement <16 x i32> %vectorPHI10.i, i32 11
  %extract22.i = extractelement <16 x i32> %vectorPHI10.i, i32 12
  %extract23.i = extractelement <16 x i32> %vectorPHI10.i, i32 13
  %extract24.i = extractelement <16 x i32> %vectorPHI10.i, i32 14
  %extract25.i = extractelement <16 x i32> %vectorPHI10.i, i32 15
  %47 = insertelement <2 x i32> undef, i32 %extract.i, i32 0
  %48 = insertelement <2 x i32> undef, i32 %extract11.i, i32 0
  %49 = insertelement <2 x i32> undef, i32 %extract12.i, i32 0
  %50 = insertelement <2 x i32> undef, i32 %extract13.i, i32 0
  %51 = insertelement <2 x i32> undef, i32 %extract14.i, i32 0
  %52 = insertelement <2 x i32> undef, i32 %extract15.i, i32 0
  %53 = insertelement <2 x i32> undef, i32 %extract16.i, i32 0
  %54 = insertelement <2 x i32> undef, i32 %extract17.i, i32 0
  %55 = insertelement <2 x i32> undef, i32 %extract18.i, i32 0
  %56 = insertelement <2 x i32> undef, i32 %extract19.i, i32 0
  %57 = insertelement <2 x i32> undef, i32 %extract20.i, i32 0
  %58 = insertelement <2 x i32> undef, i32 %extract21.i, i32 0
  %59 = insertelement <2 x i32> undef, i32 %extract22.i, i32 0
  %60 = insertelement <2 x i32> undef, i32 %extract23.i, i32 0
  %61 = insertelement <2 x i32> undef, i32 %extract24.i, i32 0
  %62 = insertelement <2 x i32> undef, i32 %extract25.i, i32 0
  %63 = insertelement <2 x i32> %47, i32 %45, i32 1
  %64 = insertelement <2 x i32> %48, i32 %45, i32 1
  %65 = insertelement <2 x i32> %49, i32 %45, i32 1
  %66 = insertelement <2 x i32> %50, i32 %45, i32 1
  %67 = insertelement <2 x i32> %51, i32 %45, i32 1
  %68 = insertelement <2 x i32> %52, i32 %45, i32 1
  %69 = insertelement <2 x i32> %53, i32 %45, i32 1
  %70 = insertelement <2 x i32> %54, i32 %45, i32 1
  %71 = insertelement <2 x i32> %55, i32 %45, i32 1
  %72 = insertelement <2 x i32> %56, i32 %45, i32 1
  %73 = insertelement <2 x i32> %57, i32 %45, i32 1
  %74 = insertelement <2 x i32> %58, i32 %45, i32 1
  %75 = insertelement <2 x i32> %59, i32 %45, i32 1
  %76 = insertelement <2 x i32> %60, i32 %45, i32 1
  %77 = insertelement <2 x i32> %61, i32 %45, i32 1
  %78 = insertelement <2 x i32> %62, i32 %45, i32 1
  %79 = bitcast <2 x i32> %63 to <1 x double>
  %80 = bitcast <2 x i32> %64 to <1 x double>
  %81 = bitcast <2 x i32> %65 to <1 x double>
  %82 = bitcast <2 x i32> %66 to <1 x double>
  %83 = bitcast <2 x i32> %67 to <1 x double>
  %84 = bitcast <2 x i32> %68 to <1 x double>
  %85 = bitcast <2 x i32> %69 to <1 x double>
  %86 = bitcast <2 x i32> %70 to <1 x double>
  %87 = bitcast <2 x i32> %71 to <1 x double>
  %88 = bitcast <2 x i32> %72 to <1 x double>
  %89 = bitcast <2 x i32> %73 to <1 x double>
  %90 = bitcast <2 x i32> %74 to <1 x double>
  %91 = bitcast <2 x i32> %75 to <1 x double>
  %92 = bitcast <2 x i32> %76 to <1 x double>
  %93 = bitcast <2 x i32> %77 to <1 x double>
  %94 = bitcast <2 x i32> %78 to <1 x double>
  %95 = extractelement <1 x double> %79, i32 0
  %96 = extractelement <1 x double> %80, i32 0
  %97 = extractelement <1 x double> %81, i32 0
  %98 = extractelement <1 x double> %82, i32 0
  %99 = extractelement <1 x double> %83, i32 0
  %100 = extractelement <1 x double> %84, i32 0
  %101 = extractelement <1 x double> %85, i32 0
  %102 = extractelement <1 x double> %86, i32 0
  %103 = extractelement <1 x double> %87, i32 0
  %104 = extractelement <1 x double> %88, i32 0
  %105 = extractelement <1 x double> %89, i32 0
  %106 = extractelement <1 x double> %90, i32 0
  %107 = extractelement <1 x double> %91, i32 0
  %108 = extractelement <1 x double> %92, i32 0
  %109 = extractelement <1 x double> %93, i32 0
  %110 = extractelement <1 x double> %94, i32 0
  %111 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %95) nounwind
  %112 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %96) nounwind
  %113 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %97) nounwind
  %114 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %98) nounwind
  %115 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %99) nounwind
  %116 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %100) nounwind
  %117 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %101) nounwind
  %118 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %102) nounwind
  %119 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %103) nounwind
  %120 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %104) nounwind
  %121 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %105) nounwind
  %122 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %106) nounwind
  %123 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %107) nounwind
  %124 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %108) nounwind
  %125 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %109) nounwind
  %126 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %110) nounwind
  %127 = extractelement <4 x float> %111, i32 0
  %128 = extractelement <4 x float> %112, i32 0
  %129 = extractelement <4 x float> %113, i32 0
  %130 = extractelement <4 x float> %114, i32 0
  %131 = extractelement <4 x float> %115, i32 0
  %132 = extractelement <4 x float> %116, i32 0
  %133 = extractelement <4 x float> %117, i32 0
  %134 = extractelement <4 x float> %118, i32 0
  %135 = extractelement <4 x float> %119, i32 0
  %136 = extractelement <4 x float> %120, i32 0
  %137 = extractelement <4 x float> %121, i32 0
  %138 = extractelement <4 x float> %122, i32 0
  %139 = extractelement <4 x float> %123, i32 0
  %140 = extractelement <4 x float> %124, i32 0
  %141 = extractelement <4 x float> %125, i32 0
  %142 = extractelement <4 x float> %126, i32 0
  %temp.vect26.i = insertelement <16 x float> undef, float %127, i32 0
  %temp.vect27.i = insertelement <16 x float> %temp.vect26.i, float %128, i32 1
  %temp.vect28.i = insertelement <16 x float> %temp.vect27.i, float %129, i32 2
  %temp.vect29.i = insertelement <16 x float> %temp.vect28.i, float %130, i32 3
  %temp.vect30.i = insertelement <16 x float> %temp.vect29.i, float %131, i32 4
  %temp.vect31.i = insertelement <16 x float> %temp.vect30.i, float %132, i32 5
  %temp.vect32.i = insertelement <16 x float> %temp.vect31.i, float %133, i32 6
  %temp.vect33.i = insertelement <16 x float> %temp.vect32.i, float %134, i32 7
  %temp.vect34.i = insertelement <16 x float> %temp.vect33.i, float %135, i32 8
  %temp.vect35.i = insertelement <16 x float> %temp.vect34.i, float %136, i32 9
  %temp.vect36.i = insertelement <16 x float> %temp.vect35.i, float %137, i32 10
  %temp.vect37.i = insertelement <16 x float> %temp.vect36.i, float %138, i32 11
  %temp.vect38.i = insertelement <16 x float> %temp.vect37.i, float %139, i32 12
  %temp.vect39.i = insertelement <16 x float> %temp.vect38.i, float %140, i32 13
  %temp.vect40.i = insertelement <16 x float> %temp.vect39.i, float %141, i32 14
  %temp.vect41.i = insertelement <16 x float> %temp.vect40.i, float %142, i32 15
  %143 = fadd <16 x float> %vectorPHI.i, %temp.vect41.i
  %144 = add nsw i32 %i.05.i, 1
  %145 = add nsw <16 x i32> %vectorPHI10.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %146 = and <16 x i32> %145, %vector43.i
  %exitcond.i = icmp eq i32 %144, %1
  br i1 %exitcond.i, label %._crit_edge.i, label %46

._crit_edge.i:                                    ; preds = %46, %SyncBB.i
  %vectorPHI44.i = phi <16 x float> [ zeroinitializer, %SyncBB.i ], [ %143, %46 ]
  %extract62.i = extractelement <16 x float> %vectorPHI44.i, i32 0
  %extract63.i = extractelement <16 x float> %vectorPHI44.i, i32 1
  %extract64.i = extractelement <16 x float> %vectorPHI44.i, i32 2
  %extract65.i = extractelement <16 x float> %vectorPHI44.i, i32 3
  %extract66.i = extractelement <16 x float> %vectorPHI44.i, i32 4
  %extract67.i = extractelement <16 x float> %vectorPHI44.i, i32 5
  %extract68.i = extractelement <16 x float> %vectorPHI44.i, i32 6
  %extract69.i = extractelement <16 x float> %vectorPHI44.i, i32 7
  %extract70.i = extractelement <16 x float> %vectorPHI44.i, i32 8
  %extract71.i = extractelement <16 x float> %vectorPHI44.i, i32 9
  %extract72.i = extractelement <16 x float> %vectorPHI44.i, i32 10
  %extract73.i = extractelement <16 x float> %vectorPHI44.i, i32 11
  %extract74.i = extractelement <16 x float> %vectorPHI44.i, i32 12
  %extract75.i = extractelement <16 x float> %vectorPHI44.i, i32 13
  %extract76.i = extractelement <16 x float> %vectorPHI44.i, i32 14
  %extract77.i = extractelement <16 x float> %vectorPHI44.i, i32 15
  %sext45.i = shl <16 x i64> %44, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %147 = ashr <16 x i64> %sext45.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract46.i = extractelement <16 x i64> %147, i32 0
  %extract47.i = extractelement <16 x i64> %147, i32 1
  %extract48.i = extractelement <16 x i64> %147, i32 2
  %extract49.i = extractelement <16 x i64> %147, i32 3
  %extract50.i = extractelement <16 x i64> %147, i32 4
  %extract51.i = extractelement <16 x i64> %147, i32 5
  %extract52.i = extractelement <16 x i64> %147, i32 6
  %extract53.i = extractelement <16 x i64> %147, i32 7
  %extract54.i = extractelement <16 x i64> %147, i32 8
  %extract55.i = extractelement <16 x i64> %147, i32 9
  %extract56.i = extractelement <16 x i64> %147, i32 10
  %extract57.i = extractelement <16 x i64> %147, i32 11
  %extract58.i = extractelement <16 x i64> %147, i32 12
  %extract59.i = extractelement <16 x i64> %147, i32 13
  %extract60.i = extractelement <16 x i64> %147, i32 14
  %extract61.i = extractelement <16 x i64> %147, i32 15
  %148 = getelementptr inbounds float addrspace(1)* %4, i64 %extract46.i
  %149 = getelementptr inbounds float addrspace(1)* %4, i64 %extract47.i
  %150 = getelementptr inbounds float addrspace(1)* %4, i64 %extract48.i
  %151 = getelementptr inbounds float addrspace(1)* %4, i64 %extract49.i
  %152 = getelementptr inbounds float addrspace(1)* %4, i64 %extract50.i
  %153 = getelementptr inbounds float addrspace(1)* %4, i64 %extract51.i
  %154 = getelementptr inbounds float addrspace(1)* %4, i64 %extract52.i
  %155 = getelementptr inbounds float addrspace(1)* %4, i64 %extract53.i
  %156 = getelementptr inbounds float addrspace(1)* %4, i64 %extract54.i
  %157 = getelementptr inbounds float addrspace(1)* %4, i64 %extract55.i
  %158 = getelementptr inbounds float addrspace(1)* %4, i64 %extract56.i
  %159 = getelementptr inbounds float addrspace(1)* %4, i64 %extract57.i
  %160 = getelementptr inbounds float addrspace(1)* %4, i64 %extract58.i
  %161 = getelementptr inbounds float addrspace(1)* %4, i64 %extract59.i
  %162 = getelementptr inbounds float addrspace(1)* %4, i64 %extract60.i
  %163 = getelementptr inbounds float addrspace(1)* %4, i64 %extract61.i
  store float %extract62.i, float addrspace(1)* %148, align 4
  store float %extract63.i, float addrspace(1)* %149, align 4
  store float %extract64.i, float addrspace(1)* %150, align 4
  store float %extract65.i, float addrspace(1)* %151, align 4
  store float %extract66.i, float addrspace(1)* %152, align 4
  store float %extract67.i, float addrspace(1)* %153, align 4
  store float %extract68.i, float addrspace(1)* %154, align 4
  store float %extract69.i, float addrspace(1)* %155, align 4
  store float %extract70.i, float addrspace(1)* %156, align 4
  store float %extract71.i, float addrspace(1)* %157, align 4
  store float %extract72.i, float addrspace(1)* %158, align 4
  store float %extract73.i, float addrspace(1)* %159, align 4
  store float %extract74.i, float addrspace(1)* %160, align 4
  store float %extract75.i, float addrspace(1)* %161, align 4
  store float %extract76.i, float addrspace(1)* %162, align 4
  store float %extract77.i, float addrspace(1)* %163, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.readImg_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.readImg_separated_args.exit:      ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.readRand(i8* %pBuffer) {
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
  %temp42.i = insertelement <16 x i32> undef, i32 %29, i32 0
  %vector43.i = shufflevector <16 x i32> %temp42.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %30 = add nsw i32 %16, -1
  %31 = icmp sgt i32 %1, 0
  br label %SyncBB78.i

SyncBB78.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %32 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %33 = load i64* %32, align 8
  %34 = getelementptr %struct.PaddedDimId* %22, i64 0, i32 0, i64 0
  %35 = load i64* %34, align 8
  %36 = add i64 %33, %35
  %broadcast1.i = insertelement <16 x i64> undef, i64 %36, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %37 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %38 = trunc <16 x i64> %37 to <16 x i32>
  %39 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 1
  %40 = load i64* %39, align 8
  %41 = getelementptr %struct.PaddedDimId* %22, i64 0, i32 0, i64 1
  %42 = load i64* %41, align 8
  %43 = add i64 %40, %42
  %temp8.i = insertelement <16 x i64> undef, i64 %43, i32 0
  %vector9.i = shufflevector <16 x i64> %temp8.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %44 = sext <16 x i32> %38 to <16 x i64>
  %45 = getelementptr %struct.WorkDim* %19, i64 0, i32 2, i64 1
  %46 = load i64* %45, align 8
  %temp.i = insertelement <16 x i64> undef, i64 %46, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %47 = mul <16 x i64> %44, %vector.i
  %48 = add <16 x i64> %47, %vector9.i
  br i1 %31, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB78.i
  %49 = trunc i64 %43 to i32
  br label %50

; <label>:50                                      ; preds = %50, %bb.nph.i
  %i.05.i = phi i32 [ 0, %bb.nph.i ], [ %148, %50 ]
  %vectorPHI.i = phi <16 x float> [ zeroinitializer, %bb.nph.i ], [ %147, %50 ]
  %vectorPHI10.i = phi <16 x i32> [ %38, %bb.nph.i ], [ %153, %50 ]
  %ridx.032.i = phi i32 [ %49, %bb.nph.i ], [ %154, %50 ]
  %extract.i = extractelement <16 x i32> %vectorPHI10.i, i32 0
  %extract11.i = extractelement <16 x i32> %vectorPHI10.i, i32 1
  %extract12.i = extractelement <16 x i32> %vectorPHI10.i, i32 2
  %extract13.i = extractelement <16 x i32> %vectorPHI10.i, i32 3
  %extract14.i = extractelement <16 x i32> %vectorPHI10.i, i32 4
  %extract15.i = extractelement <16 x i32> %vectorPHI10.i, i32 5
  %extract16.i = extractelement <16 x i32> %vectorPHI10.i, i32 6
  %extract17.i = extractelement <16 x i32> %vectorPHI10.i, i32 7
  %extract18.i = extractelement <16 x i32> %vectorPHI10.i, i32 8
  %extract19.i = extractelement <16 x i32> %vectorPHI10.i, i32 9
  %extract20.i = extractelement <16 x i32> %vectorPHI10.i, i32 10
  %extract21.i = extractelement <16 x i32> %vectorPHI10.i, i32 11
  %extract22.i = extractelement <16 x i32> %vectorPHI10.i, i32 12
  %extract23.i = extractelement <16 x i32> %vectorPHI10.i, i32 13
  %extract24.i = extractelement <16 x i32> %vectorPHI10.i, i32 14
  %extract25.i = extractelement <16 x i32> %vectorPHI10.i, i32 15
  %51 = insertelement <2 x i32> undef, i32 %extract.i, i32 0
  %52 = insertelement <2 x i32> undef, i32 %extract11.i, i32 0
  %53 = insertelement <2 x i32> undef, i32 %extract12.i, i32 0
  %54 = insertelement <2 x i32> undef, i32 %extract13.i, i32 0
  %55 = insertelement <2 x i32> undef, i32 %extract14.i, i32 0
  %56 = insertelement <2 x i32> undef, i32 %extract15.i, i32 0
  %57 = insertelement <2 x i32> undef, i32 %extract16.i, i32 0
  %58 = insertelement <2 x i32> undef, i32 %extract17.i, i32 0
  %59 = insertelement <2 x i32> undef, i32 %extract18.i, i32 0
  %60 = insertelement <2 x i32> undef, i32 %extract19.i, i32 0
  %61 = insertelement <2 x i32> undef, i32 %extract20.i, i32 0
  %62 = insertelement <2 x i32> undef, i32 %extract21.i, i32 0
  %63 = insertelement <2 x i32> undef, i32 %extract22.i, i32 0
  %64 = insertelement <2 x i32> undef, i32 %extract23.i, i32 0
  %65 = insertelement <2 x i32> undef, i32 %extract24.i, i32 0
  %66 = insertelement <2 x i32> undef, i32 %extract25.i, i32 0
  %67 = insertelement <2 x i32> %51, i32 %ridx.032.i, i32 1
  %68 = insertelement <2 x i32> %52, i32 %ridx.032.i, i32 1
  %69 = insertelement <2 x i32> %53, i32 %ridx.032.i, i32 1
  %70 = insertelement <2 x i32> %54, i32 %ridx.032.i, i32 1
  %71 = insertelement <2 x i32> %55, i32 %ridx.032.i, i32 1
  %72 = insertelement <2 x i32> %56, i32 %ridx.032.i, i32 1
  %73 = insertelement <2 x i32> %57, i32 %ridx.032.i, i32 1
  %74 = insertelement <2 x i32> %58, i32 %ridx.032.i, i32 1
  %75 = insertelement <2 x i32> %59, i32 %ridx.032.i, i32 1
  %76 = insertelement <2 x i32> %60, i32 %ridx.032.i, i32 1
  %77 = insertelement <2 x i32> %61, i32 %ridx.032.i, i32 1
  %78 = insertelement <2 x i32> %62, i32 %ridx.032.i, i32 1
  %79 = insertelement <2 x i32> %63, i32 %ridx.032.i, i32 1
  %80 = insertelement <2 x i32> %64, i32 %ridx.032.i, i32 1
  %81 = insertelement <2 x i32> %65, i32 %ridx.032.i, i32 1
  %82 = insertelement <2 x i32> %66, i32 %ridx.032.i, i32 1
  %83 = bitcast <2 x i32> %67 to <1 x double>
  %84 = bitcast <2 x i32> %68 to <1 x double>
  %85 = bitcast <2 x i32> %69 to <1 x double>
  %86 = bitcast <2 x i32> %70 to <1 x double>
  %87 = bitcast <2 x i32> %71 to <1 x double>
  %88 = bitcast <2 x i32> %72 to <1 x double>
  %89 = bitcast <2 x i32> %73 to <1 x double>
  %90 = bitcast <2 x i32> %74 to <1 x double>
  %91 = bitcast <2 x i32> %75 to <1 x double>
  %92 = bitcast <2 x i32> %76 to <1 x double>
  %93 = bitcast <2 x i32> %77 to <1 x double>
  %94 = bitcast <2 x i32> %78 to <1 x double>
  %95 = bitcast <2 x i32> %79 to <1 x double>
  %96 = bitcast <2 x i32> %80 to <1 x double>
  %97 = bitcast <2 x i32> %81 to <1 x double>
  %98 = bitcast <2 x i32> %82 to <1 x double>
  %99 = extractelement <1 x double> %83, i32 0
  %100 = extractelement <1 x double> %84, i32 0
  %101 = extractelement <1 x double> %85, i32 0
  %102 = extractelement <1 x double> %86, i32 0
  %103 = extractelement <1 x double> %87, i32 0
  %104 = extractelement <1 x double> %88, i32 0
  %105 = extractelement <1 x double> %89, i32 0
  %106 = extractelement <1 x double> %90, i32 0
  %107 = extractelement <1 x double> %91, i32 0
  %108 = extractelement <1 x double> %92, i32 0
  %109 = extractelement <1 x double> %93, i32 0
  %110 = extractelement <1 x double> %94, i32 0
  %111 = extractelement <1 x double> %95, i32 0
  %112 = extractelement <1 x double> %96, i32 0
  %113 = extractelement <1 x double> %97, i32 0
  %114 = extractelement <1 x double> %98, i32 0
  %115 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %99) nounwind
  %116 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %100) nounwind
  %117 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %101) nounwind
  %118 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %102) nounwind
  %119 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %103) nounwind
  %120 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %104) nounwind
  %121 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %105) nounwind
  %122 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %106) nounwind
  %123 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %107) nounwind
  %124 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %108) nounwind
  %125 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %109) nounwind
  %126 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %110) nounwind
  %127 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %111) nounwind
  %128 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %112) nounwind
  %129 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %113) nounwind
  %130 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %114) nounwind
  %131 = extractelement <4 x float> %115, i32 0
  %132 = extractelement <4 x float> %116, i32 0
  %133 = extractelement <4 x float> %117, i32 0
  %134 = extractelement <4 x float> %118, i32 0
  %135 = extractelement <4 x float> %119, i32 0
  %136 = extractelement <4 x float> %120, i32 0
  %137 = extractelement <4 x float> %121, i32 0
  %138 = extractelement <4 x float> %122, i32 0
  %139 = extractelement <4 x float> %123, i32 0
  %140 = extractelement <4 x float> %124, i32 0
  %141 = extractelement <4 x float> %125, i32 0
  %142 = extractelement <4 x float> %126, i32 0
  %143 = extractelement <4 x float> %127, i32 0
  %144 = extractelement <4 x float> %128, i32 0
  %145 = extractelement <4 x float> %129, i32 0
  %146 = extractelement <4 x float> %130, i32 0
  %temp.vect26.i = insertelement <16 x float> undef, float %131, i32 0
  %temp.vect27.i = insertelement <16 x float> %temp.vect26.i, float %132, i32 1
  %temp.vect28.i = insertelement <16 x float> %temp.vect27.i, float %133, i32 2
  %temp.vect29.i = insertelement <16 x float> %temp.vect28.i, float %134, i32 3
  %temp.vect30.i = insertelement <16 x float> %temp.vect29.i, float %135, i32 4
  %temp.vect31.i = insertelement <16 x float> %temp.vect30.i, float %136, i32 5
  %temp.vect32.i = insertelement <16 x float> %temp.vect31.i, float %137, i32 6
  %temp.vect33.i = insertelement <16 x float> %temp.vect32.i, float %138, i32 7
  %temp.vect34.i = insertelement <16 x float> %temp.vect33.i, float %139, i32 8
  %temp.vect35.i = insertelement <16 x float> %temp.vect34.i, float %140, i32 9
  %temp.vect36.i = insertelement <16 x float> %temp.vect35.i, float %141, i32 10
  %temp.vect37.i = insertelement <16 x float> %temp.vect36.i, float %142, i32 11
  %temp.vect38.i = insertelement <16 x float> %temp.vect37.i, float %143, i32 12
  %temp.vect39.i = insertelement <16 x float> %temp.vect38.i, float %144, i32 13
  %temp.vect40.i = insertelement <16 x float> %temp.vect39.i, float %145, i32 14
  %temp.vect41.i = insertelement <16 x float> %temp.vect40.i, float %146, i32 15
  %147 = fadd <16 x float> %vectorPHI.i, %temp.vect41.i
  %148 = add nsw i32 %i.05.i, 1
  %149 = mul nsw <16 x i32> %vectorPHI10.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %150 = mul nsw i32 %ridx.032.i, 5
  %151 = add nsw <16 x i32> %149, <i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29, i32 29>
  %152 = add nsw i32 %150, 11
  %153 = and <16 x i32> %151, %vector43.i
  %154 = and i32 %152, %30
  %exitcond.i = icmp eq i32 %148, %1
  br i1 %exitcond.i, label %._crit_edge.i, label %50

._crit_edge.i:                                    ; preds = %50, %SyncBB78.i
  %vectorPHI44.i = phi <16 x float> [ zeroinitializer, %SyncBB78.i ], [ %147, %50 ]
  %extract62.i = extractelement <16 x float> %vectorPHI44.i, i32 0
  %extract63.i = extractelement <16 x float> %vectorPHI44.i, i32 1
  %extract64.i = extractelement <16 x float> %vectorPHI44.i, i32 2
  %extract65.i = extractelement <16 x float> %vectorPHI44.i, i32 3
  %extract66.i = extractelement <16 x float> %vectorPHI44.i, i32 4
  %extract67.i = extractelement <16 x float> %vectorPHI44.i, i32 5
  %extract68.i = extractelement <16 x float> %vectorPHI44.i, i32 6
  %extract69.i = extractelement <16 x float> %vectorPHI44.i, i32 7
  %extract70.i = extractelement <16 x float> %vectorPHI44.i, i32 8
  %extract71.i = extractelement <16 x float> %vectorPHI44.i, i32 9
  %extract72.i = extractelement <16 x float> %vectorPHI44.i, i32 10
  %extract73.i = extractelement <16 x float> %vectorPHI44.i, i32 11
  %extract74.i = extractelement <16 x float> %vectorPHI44.i, i32 12
  %extract75.i = extractelement <16 x float> %vectorPHI44.i, i32 13
  %extract76.i = extractelement <16 x float> %vectorPHI44.i, i32 14
  %extract77.i = extractelement <16 x float> %vectorPHI44.i, i32 15
  %sext45.i = shl <16 x i64> %48, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %155 = ashr <16 x i64> %sext45.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract46.i = extractelement <16 x i64> %155, i32 0
  %extract47.i = extractelement <16 x i64> %155, i32 1
  %extract48.i = extractelement <16 x i64> %155, i32 2
  %extract49.i = extractelement <16 x i64> %155, i32 3
  %extract50.i = extractelement <16 x i64> %155, i32 4
  %extract51.i = extractelement <16 x i64> %155, i32 5
  %extract52.i = extractelement <16 x i64> %155, i32 6
  %extract53.i = extractelement <16 x i64> %155, i32 7
  %extract54.i = extractelement <16 x i64> %155, i32 8
  %extract55.i = extractelement <16 x i64> %155, i32 9
  %extract56.i = extractelement <16 x i64> %155, i32 10
  %extract57.i = extractelement <16 x i64> %155, i32 11
  %extract58.i = extractelement <16 x i64> %155, i32 12
  %extract59.i = extractelement <16 x i64> %155, i32 13
  %extract60.i = extractelement <16 x i64> %155, i32 14
  %extract61.i = extractelement <16 x i64> %155, i32 15
  %156 = getelementptr inbounds float addrspace(1)* %4, i64 %extract46.i
  %157 = getelementptr inbounds float addrspace(1)* %4, i64 %extract47.i
  %158 = getelementptr inbounds float addrspace(1)* %4, i64 %extract48.i
  %159 = getelementptr inbounds float addrspace(1)* %4, i64 %extract49.i
  %160 = getelementptr inbounds float addrspace(1)* %4, i64 %extract50.i
  %161 = getelementptr inbounds float addrspace(1)* %4, i64 %extract51.i
  %162 = getelementptr inbounds float addrspace(1)* %4, i64 %extract52.i
  %163 = getelementptr inbounds float addrspace(1)* %4, i64 %extract53.i
  %164 = getelementptr inbounds float addrspace(1)* %4, i64 %extract54.i
  %165 = getelementptr inbounds float addrspace(1)* %4, i64 %extract55.i
  %166 = getelementptr inbounds float addrspace(1)* %4, i64 %extract56.i
  %167 = getelementptr inbounds float addrspace(1)* %4, i64 %extract57.i
  %168 = getelementptr inbounds float addrspace(1)* %4, i64 %extract58.i
  %169 = getelementptr inbounds float addrspace(1)* %4, i64 %extract59.i
  %170 = getelementptr inbounds float addrspace(1)* %4, i64 %extract60.i
  %171 = getelementptr inbounds float addrspace(1)* %4, i64 %extract61.i
  store float %extract62.i, float addrspace(1)* %156, align 4
  store float %extract63.i, float addrspace(1)* %157, align 4
  store float %extract64.i, float addrspace(1)* %158, align 4
  store float %extract65.i, float addrspace(1)* %159, align 4
  store float %extract66.i, float addrspace(1)* %160, align 4
  store float %extract67.i, float addrspace(1)* %161, align 4
  store float %extract68.i, float addrspace(1)* %162, align 4
  store float %extract69.i, float addrspace(1)* %163, align 4
  store float %extract70.i, float addrspace(1)* %164, align 4
  store float %extract71.i, float addrspace(1)* %165, align 4
  store float %extract72.i, float addrspace(1)* %166, align 4
  store float %extract73.i, float addrspace(1)* %167, align 4
  store float %extract74.i, float addrspace(1)* %168, align 4
  store float %extract75.i, float addrspace(1)* %169, align 4
  store float %extract76.i, float addrspace(1)* %170, align 4
  store float %extract77.i, float addrspace(1)* %171, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.readRand_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB78.i

____Vectorized_.readRand_separated_args.exit:     ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.readInCache(i8* %pBuffer) {
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
  br label %SyncBB73.i

SyncBB73.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %24 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %25 = load i64* %24, align 8
  %26 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = add i64 %25, %27
  %broadcast1.i = insertelement <16 x i64> undef, i64 %28, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %29 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %30 = trunc <16 x i64> %29 to <16 x i32>
  %31 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 1
  %32 = load i64* %31, align 8
  %33 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 1
  %34 = load i64* %33, align 8
  %35 = add i64 %32, %34
  %temp6.i = insertelement <16 x i64> undef, i64 %35, i32 0
  %vector7.i = shufflevector <16 x i64> %temp6.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %36 = sext <16 x i32> %30 to <16 x i64>
  %37 = getelementptr %struct.WorkDim* %13, i64 0, i32 2, i64 1
  %38 = load i64* %37, align 8
  %temp.i = insertelement <16 x i64> undef, i64 %38, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %39 = mul <16 x i64> %36, %vector.i
  %40 = add <16 x i64> %39, %vector7.i
  br i1 %23, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB73.i
  %extract22.i = extractelement <16 x i32> %30, i32 15
  %extract21.i = extractelement <16 x i32> %30, i32 14
  %extract20.i = extractelement <16 x i32> %30, i32 13
  %extract19.i = extractelement <16 x i32> %30, i32 12
  %extract18.i = extractelement <16 x i32> %30, i32 11
  %extract17.i = extractelement <16 x i32> %30, i32 10
  %extract16.i = extractelement <16 x i32> %30, i32 9
  %extract15.i = extractelement <16 x i32> %30, i32 8
  %extract14.i = extractelement <16 x i32> %30, i32 7
  %extract13.i = extractelement <16 x i32> %30, i32 6
  %extract12.i = extractelement <16 x i32> %30, i32 5
  %extract11.i = extractelement <16 x i32> %30, i32 4
  %extract10.i = extractelement <16 x i32> %30, i32 3
  %extract9.i = extractelement <16 x i32> %30, i32 2
  %extract8.i = extractelement <16 x i32> %30, i32 1
  %extract.i = extractelement <16 x i32> %30, i32 0
  %41 = trunc i64 %35 to i32
  %42 = insertelement <2 x i32> undef, i32 %extract.i, i32 0
  %43 = insertelement <2 x i32> undef, i32 %extract8.i, i32 0
  %44 = insertelement <2 x i32> undef, i32 %extract9.i, i32 0
  %45 = insertelement <2 x i32> undef, i32 %extract10.i, i32 0
  %46 = insertelement <2 x i32> undef, i32 %extract11.i, i32 0
  %47 = insertelement <2 x i32> undef, i32 %extract12.i, i32 0
  %48 = insertelement <2 x i32> undef, i32 %extract13.i, i32 0
  %49 = insertelement <2 x i32> undef, i32 %extract14.i, i32 0
  %50 = insertelement <2 x i32> undef, i32 %extract15.i, i32 0
  %51 = insertelement <2 x i32> undef, i32 %extract16.i, i32 0
  %52 = insertelement <2 x i32> undef, i32 %extract17.i, i32 0
  %53 = insertelement <2 x i32> undef, i32 %extract18.i, i32 0
  %54 = insertelement <2 x i32> undef, i32 %extract19.i, i32 0
  %55 = insertelement <2 x i32> undef, i32 %extract20.i, i32 0
  %56 = insertelement <2 x i32> undef, i32 %extract21.i, i32 0
  %57 = insertelement <2 x i32> undef, i32 %extract22.i, i32 0
  %58 = insertelement <2 x i32> %42, i32 %41, i32 1
  %59 = insertelement <2 x i32> %43, i32 %41, i32 1
  %60 = insertelement <2 x i32> %44, i32 %41, i32 1
  %61 = insertelement <2 x i32> %45, i32 %41, i32 1
  %62 = insertelement <2 x i32> %46, i32 %41, i32 1
  %63 = insertelement <2 x i32> %47, i32 %41, i32 1
  %64 = insertelement <2 x i32> %48, i32 %41, i32 1
  %65 = insertelement <2 x i32> %49, i32 %41, i32 1
  %66 = insertelement <2 x i32> %50, i32 %41, i32 1
  %67 = insertelement <2 x i32> %51, i32 %41, i32 1
  %68 = insertelement <2 x i32> %52, i32 %41, i32 1
  %69 = insertelement <2 x i32> %53, i32 %41, i32 1
  %70 = insertelement <2 x i32> %54, i32 %41, i32 1
  %71 = insertelement <2 x i32> %55, i32 %41, i32 1
  %72 = insertelement <2 x i32> %56, i32 %41, i32 1
  %73 = insertelement <2 x i32> %57, i32 %41, i32 1
  %74 = bitcast <2 x i32> %58 to <1 x double>
  %75 = bitcast <2 x i32> %59 to <1 x double>
  %76 = bitcast <2 x i32> %60 to <1 x double>
  %77 = bitcast <2 x i32> %61 to <1 x double>
  %78 = bitcast <2 x i32> %62 to <1 x double>
  %79 = bitcast <2 x i32> %63 to <1 x double>
  %80 = bitcast <2 x i32> %64 to <1 x double>
  %81 = bitcast <2 x i32> %65 to <1 x double>
  %82 = bitcast <2 x i32> %66 to <1 x double>
  %83 = bitcast <2 x i32> %67 to <1 x double>
  %84 = bitcast <2 x i32> %68 to <1 x double>
  %85 = bitcast <2 x i32> %69 to <1 x double>
  %86 = bitcast <2 x i32> %70 to <1 x double>
  %87 = bitcast <2 x i32> %71 to <1 x double>
  %88 = bitcast <2 x i32> %72 to <1 x double>
  %89 = bitcast <2 x i32> %73 to <1 x double>
  %90 = extractelement <1 x double> %74, i32 0
  %91 = extractelement <1 x double> %75, i32 0
  %92 = extractelement <1 x double> %76, i32 0
  %93 = extractelement <1 x double> %77, i32 0
  %94 = extractelement <1 x double> %78, i32 0
  %95 = extractelement <1 x double> %79, i32 0
  %96 = extractelement <1 x double> %80, i32 0
  %97 = extractelement <1 x double> %81, i32 0
  %98 = extractelement <1 x double> %82, i32 0
  %99 = extractelement <1 x double> %83, i32 0
  %100 = extractelement <1 x double> %84, i32 0
  %101 = extractelement <1 x double> %85, i32 0
  %102 = extractelement <1 x double> %86, i32 0
  %103 = extractelement <1 x double> %87, i32 0
  %104 = extractelement <1 x double> %88, i32 0
  %105 = extractelement <1 x double> %89, i32 0
  br label %106

; <label>:106                                     ; preds = %106, %bb.nph.i
  %i.04.i = phi i32 [ 0, %bb.nph.i ], [ %140, %106 ]
  %vectorPHI.i = phi <16 x float> [ zeroinitializer, %bb.nph.i ], [ %139, %106 ]
  %107 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %90) nounwind
  %108 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %91) nounwind
  %109 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %92) nounwind
  %110 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %93) nounwind
  %111 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %94) nounwind
  %112 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %95) nounwind
  %113 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %96) nounwind
  %114 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %97) nounwind
  %115 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %98) nounwind
  %116 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %99) nounwind
  %117 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %100) nounwind
  %118 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %101) nounwind
  %119 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %102) nounwind
  %120 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %103) nounwind
  %121 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %104) nounwind
  %122 = call <4 x float> @_Z11read_imagefP10_image2d_tjDv2_i(%struct._image2d_t* %7, i32 %10, double %105) nounwind
  %123 = extractelement <4 x float> %107, i32 0
  %124 = extractelement <4 x float> %108, i32 0
  %125 = extractelement <4 x float> %109, i32 0
  %126 = extractelement <4 x float> %110, i32 0
  %127 = extractelement <4 x float> %111, i32 0
  %128 = extractelement <4 x float> %112, i32 0
  %129 = extractelement <4 x float> %113, i32 0
  %130 = extractelement <4 x float> %114, i32 0
  %131 = extractelement <4 x float> %115, i32 0
  %132 = extractelement <4 x float> %116, i32 0
  %133 = extractelement <4 x float> %117, i32 0
  %134 = extractelement <4 x float> %118, i32 0
  %135 = extractelement <4 x float> %119, i32 0
  %136 = extractelement <4 x float> %120, i32 0
  %137 = extractelement <4 x float> %121, i32 0
  %138 = extractelement <4 x float> %122, i32 0
  %temp.vect23.i = insertelement <16 x float> undef, float %123, i32 0
  %temp.vect24.i = insertelement <16 x float> %temp.vect23.i, float %124, i32 1
  %temp.vect25.i = insertelement <16 x float> %temp.vect24.i, float %125, i32 2
  %temp.vect26.i = insertelement <16 x float> %temp.vect25.i, float %126, i32 3
  %temp.vect27.i = insertelement <16 x float> %temp.vect26.i, float %127, i32 4
  %temp.vect28.i = insertelement <16 x float> %temp.vect27.i, float %128, i32 5
  %temp.vect29.i = insertelement <16 x float> %temp.vect28.i, float %129, i32 6
  %temp.vect30.i = insertelement <16 x float> %temp.vect29.i, float %130, i32 7
  %temp.vect31.i = insertelement <16 x float> %temp.vect30.i, float %131, i32 8
  %temp.vect32.i = insertelement <16 x float> %temp.vect31.i, float %132, i32 9
  %temp.vect33.i = insertelement <16 x float> %temp.vect32.i, float %133, i32 10
  %temp.vect34.i = insertelement <16 x float> %temp.vect33.i, float %134, i32 11
  %temp.vect35.i = insertelement <16 x float> %temp.vect34.i, float %135, i32 12
  %temp.vect36.i = insertelement <16 x float> %temp.vect35.i, float %136, i32 13
  %temp.vect37.i = insertelement <16 x float> %temp.vect36.i, float %137, i32 14
  %temp.vect38.i = insertelement <16 x float> %temp.vect37.i, float %138, i32 15
  %139 = fadd <16 x float> %vectorPHI.i, %temp.vect38.i
  %140 = add nsw i32 %i.04.i, 1
  %exitcond.i = icmp eq i32 %140, %1
  br i1 %exitcond.i, label %._crit_edge.i, label %106

._crit_edge.i:                                    ; preds = %106, %SyncBB73.i
  %vectorPHI39.i = phi <16 x float> [ zeroinitializer, %SyncBB73.i ], [ %139, %106 ]
  %extract57.i = extractelement <16 x float> %vectorPHI39.i, i32 0
  %extract58.i = extractelement <16 x float> %vectorPHI39.i, i32 1
  %extract59.i = extractelement <16 x float> %vectorPHI39.i, i32 2
  %extract60.i = extractelement <16 x float> %vectorPHI39.i, i32 3
  %extract61.i = extractelement <16 x float> %vectorPHI39.i, i32 4
  %extract62.i = extractelement <16 x float> %vectorPHI39.i, i32 5
  %extract63.i = extractelement <16 x float> %vectorPHI39.i, i32 6
  %extract64.i = extractelement <16 x float> %vectorPHI39.i, i32 7
  %extract65.i = extractelement <16 x float> %vectorPHI39.i, i32 8
  %extract66.i = extractelement <16 x float> %vectorPHI39.i, i32 9
  %extract67.i = extractelement <16 x float> %vectorPHI39.i, i32 10
  %extract68.i = extractelement <16 x float> %vectorPHI39.i, i32 11
  %extract69.i = extractelement <16 x float> %vectorPHI39.i, i32 12
  %extract70.i = extractelement <16 x float> %vectorPHI39.i, i32 13
  %extract71.i = extractelement <16 x float> %vectorPHI39.i, i32 14
  %extract72.i = extractelement <16 x float> %vectorPHI39.i, i32 15
  %sext40.i = shl <16 x i64> %40, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %141 = ashr <16 x i64> %sext40.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract41.i = extractelement <16 x i64> %141, i32 0
  %extract42.i = extractelement <16 x i64> %141, i32 1
  %extract43.i = extractelement <16 x i64> %141, i32 2
  %extract44.i = extractelement <16 x i64> %141, i32 3
  %extract45.i = extractelement <16 x i64> %141, i32 4
  %extract46.i = extractelement <16 x i64> %141, i32 5
  %extract47.i = extractelement <16 x i64> %141, i32 6
  %extract48.i = extractelement <16 x i64> %141, i32 7
  %extract49.i = extractelement <16 x i64> %141, i32 8
  %extract50.i = extractelement <16 x i64> %141, i32 9
  %extract51.i = extractelement <16 x i64> %141, i32 10
  %extract52.i = extractelement <16 x i64> %141, i32 11
  %extract53.i = extractelement <16 x i64> %141, i32 12
  %extract54.i = extractelement <16 x i64> %141, i32 13
  %extract55.i = extractelement <16 x i64> %141, i32 14
  %extract56.i = extractelement <16 x i64> %141, i32 15
  %142 = getelementptr inbounds float addrspace(1)* %4, i64 %extract41.i
  %143 = getelementptr inbounds float addrspace(1)* %4, i64 %extract42.i
  %144 = getelementptr inbounds float addrspace(1)* %4, i64 %extract43.i
  %145 = getelementptr inbounds float addrspace(1)* %4, i64 %extract44.i
  %146 = getelementptr inbounds float addrspace(1)* %4, i64 %extract45.i
  %147 = getelementptr inbounds float addrspace(1)* %4, i64 %extract46.i
  %148 = getelementptr inbounds float addrspace(1)* %4, i64 %extract47.i
  %149 = getelementptr inbounds float addrspace(1)* %4, i64 %extract48.i
  %150 = getelementptr inbounds float addrspace(1)* %4, i64 %extract49.i
  %151 = getelementptr inbounds float addrspace(1)* %4, i64 %extract50.i
  %152 = getelementptr inbounds float addrspace(1)* %4, i64 %extract51.i
  %153 = getelementptr inbounds float addrspace(1)* %4, i64 %extract52.i
  %154 = getelementptr inbounds float addrspace(1)* %4, i64 %extract53.i
  %155 = getelementptr inbounds float addrspace(1)* %4, i64 %extract54.i
  %156 = getelementptr inbounds float addrspace(1)* %4, i64 %extract55.i
  %157 = getelementptr inbounds float addrspace(1)* %4, i64 %extract56.i
  store float %extract57.i, float addrspace(1)* %142, align 4
  store float %extract58.i, float addrspace(1)* %143, align 4
  store float %extract59.i, float addrspace(1)* %144, align 4
  store float %extract60.i, float addrspace(1)* %145, align 4
  store float %extract61.i, float addrspace(1)* %146, align 4
  store float %extract62.i, float addrspace(1)* %147, align 4
  store float %extract63.i, float addrspace(1)* %148, align 4
  store float %extract64.i, float addrspace(1)* %149, align 4
  store float %extract65.i, float addrspace(1)* %150, align 4
  store float %extract66.i, float addrspace(1)* %151, align 4
  store float %extract67.i, float addrspace(1)* %152, align 4
  store float %extract68.i, float addrspace(1)* %153, align 4
  store float %extract69.i, float addrspace(1)* %154, align 4
  store float %extract70.i, float addrspace(1)* %155, align 4
  store float %extract71.i, float addrspace(1)* %156, align 4
  store float %extract72.i, float addrspace(1)* %157, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.readInCache_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB73.i

____Vectorized_.readInCache_separated_args.exit:  ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0, !2, !3}

!0 = metadata !{void (i32, float addrspace(1)*, %struct._image2d_t*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readImg_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int, float __attribute__((address_space(1))) *, __rd image2d_t, sampler_t, int, int", metadata !"opencl_readImg_locals_anchor", void (i8*)* @readImg}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i32, float addrspace(1)*, %struct._image2d_t*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readInCache_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int, float __attribute__((address_space(1))) *, __rd image2d_t, sampler_t", metadata !"opencl_readInCache_locals_anchor", void (i8*)* @readInCache}
!3 = metadata !{void (i32, float addrspace(1)*, %struct._image2d_t*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readRand_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int, float __attribute__((address_space(1))) *, __rd image2d_t, sampler_t, int, int", metadata !"opencl_readRand_locals_anchor", void (i8*)* @readRand}
