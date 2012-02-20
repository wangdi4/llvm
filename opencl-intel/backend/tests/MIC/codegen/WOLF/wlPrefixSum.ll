; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__prefixSumStep1_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare void @__prefixSumStep2_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32) nounwind

declare void @____Vectorized_.prefixSumStep1_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32) nounwind

declare void @____Vectorized_.prefixSumStep2_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32) nounwind

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

define void @__prefixSumStep1_separated_args(i32 addrspace(1)* nocapture %puiInputArray, i32 addrspace(1)* nocapture %puiOutputArray, i32 addrspace(1)* nocapture %puiTmpArray, i32 %szElementsPerItem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = zext i32 %szElementsPerItem to i64
  %1 = icmp eq i32 %szElementsPerItem, 0
  %2 = add i32 %szElementsPerItem, -1
  %3 = zext i32 %2 to i64
  %4 = lshr i32 %szElementsPerItem, 1
  %5 = icmp eq i32 %4, 0
  %6 = zext i32 %4 to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %8 = load i64* %7, align 8
  %9 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = add i64 %8, %10
  %12 = mul i64 %11, %0
  br i1 %1, label %._crit_edge10, label %bb.nph12

bb.nph12:                                         ; preds = %SyncBB, %bb.nph12
  %i.011 = phi i64 [ %14, %bb.nph12 ], [ 0, %SyncBB ]
  %tmp27 = add i64 %12, %i.011
  %scevgep28 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %tmp27
  %scevgep29 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %tmp27
  %13 = load i32 addrspace(1)* %scevgep28, align 4
  store i32 %13, i32 addrspace(1)* %scevgep29, align 4
  %14 = add i64 %i.011, 1
  %exitcond25 = icmp eq i64 %14, %0
  br i1 %exitcond25, label %._crit_edge13, label %bb.nph12

._crit_edge13:                                    ; preds = %bb.nph12
  br i1 %5, label %._crit_edge10, label %bb.nph9

bb.nph9:                                          ; preds = %._crit_edge13
  %tmp17 = add i64 %12, -1
  br label %15

; <label>:15                                      ; preds = %._crit_edge, %bb.nph9
  %h.08 = phi i64 [ %6, %bb.nph9 ], [ %22, %._crit_edge ]
  %offset.07 = phi i64 [ 1, %bb.nph9 ], [ %23, %._crit_edge ]
  %16 = icmp eq i64 %h.08, 0
  br i1 %16, label %._crit_edge10, label %bb.nph

bb.nph:                                           ; preds = %15
  %tmp = shl i64 %offset.07, 1
  %tmp18 = add i64 %tmp17, %offset.07
  %tmp21 = add i64 %tmp17, %tmp
  br label %17

; <label>:17                                      ; preds = %17, %bb.nph
  %p.06 = phi i64 [ 0, %bb.nph ], [ %21, %17 ]
  %tmp14 = mul i64 %tmp, %p.06
  %tmp19 = add i64 %tmp18, %tmp14
  %scevgep = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %tmp19
  %tmp22 = add i64 %tmp21, %tmp14
  %scevgep23 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %tmp22
  %18 = load i32 addrspace(1)* %scevgep, align 4
  %19 = load i32 addrspace(1)* %scevgep23, align 4
  %20 = add i32 %19, %18
  store i32 %20, i32 addrspace(1)* %scevgep23, align 4
  %21 = add i64 %p.06, 1
  %exitcond = icmp eq i64 %21, %h.08
  br i1 %exitcond, label %._crit_edge, label %17

._crit_edge:                                      ; preds = %17
  %22 = lshr i64 %h.08, 1
  %23 = shl i64 %offset.07, 1
  %24 = icmp eq i64 %22, 0
  br i1 %24, label %._crit_edge10, label %15

._crit_edge10:                                    ; preds = %._crit_edge13, %._crit_edge, %15, %SyncBB
  %.sum = add i64 %12, %3
  %25 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %.sum
  %26 = load i32 addrspace(1)* %25, align 4
  %27 = getelementptr inbounds i32 addrspace(1)* %puiTmpArray, i64 %11
  store i32 %26, i32 addrspace(1)* %27, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB30

thenBB:                                           ; preds = %._crit_edge10
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB30:                                         ; preds = %._crit_edge10
  ret void
}

define void @__prefixSumStep2_separated_args(i32 addrspace(1)* nocapture %puiOutputArray, i32 addrspace(1)* nocapture %puiValueToAddArray, i32 %szElementsPerItem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = zext i32 %szElementsPerItem to i64
  %1 = lshr i32 %szElementsPerItem, 1
  %2 = zext i32 %1 to i64
  %3 = add i32 %szElementsPerItem, -1
  %4 = zext i32 %3 to i64
  %5 = icmp eq i32 %1, 0
  %6 = icmp eq i32 %szElementsPerItem, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %8 = load i64* %7, align 8
  %9 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = add i64 %8, %10
  %12 = mul i64 %11, %0
  %13 = getelementptr inbounds i32 addrspace(1)* %puiValueToAddArray, i64 %11
  %.sum = add i64 %12, %4
  %14 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %.sum
  store i32 0, i32 addrspace(1)* %14, align 4
  br i1 %5, label %.preheader, label %bb.nph13

.preheader:                                       ; preds = %._crit_edge10, %SyncBB
  br i1 %6, label %._crit_edge, label %bb.nph

bb.nph13:                                         ; preds = %SyncBB
  %tmp20 = add i64 %12, -1
  br label %15

; <label>:15                                      ; preds = %._crit_edge10, %bb.nph13
  %h.012 = phi i64 [ 1, %bb.nph13 ], [ %24, %._crit_edge10 ]
  %offset.011 = phi i64 [ %0, %bb.nph13 ], [ %16, %._crit_edge10 ]
  %16 = lshr i64 %offset.011, 1
  %17 = icmp eq i64 %h.012, 0
  br i1 %17, label %._crit_edge10, label %bb.nph9

bb.nph9:                                          ; preds = %15
  %18 = and i64 %offset.011, 9223372036854775806
  %tmp23 = add i64 %tmp20, %18
  %tmp27 = add i64 %tmp20, %16
  br label %19

; <label>:19                                      ; preds = %19, %bb.nph9
  %p.08 = phi i64 [ 0, %bb.nph9 ], [ %23, %19 ]
  %tmp17 = mul i64 %18, %p.08
  %tmp24 = add i64 %tmp23, %tmp17
  %scevgep25 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %tmp24
  %tmp28 = add i64 %tmp27, %tmp17
  %scevgep29 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %tmp28
  %20 = load i32 addrspace(1)* %scevgep25, align 4
  %21 = load i32 addrspace(1)* %scevgep29, align 4
  %22 = add i32 %20, %21
  store i32 %22, i32 addrspace(1)* %scevgep25, align 4
  store i32 %20, i32 addrspace(1)* %scevgep29, align 4
  %23 = add i64 %p.08, 1
  %exitcond16 = icmp eq i64 %23, %h.012
  br i1 %exitcond16, label %._crit_edge10, label %19

._crit_edge10:                                    ; preds = %19, %15
  %24 = shl i64 %h.012, 1
  %25 = icmp ugt i64 %24, %2
  br i1 %25, label %.preheader, label %15

bb.nph:                                           ; preds = %.preheader, %bb.nph
  %i.06 = phi i64 [ %29, %bb.nph ], [ 0, %.preheader ]
  %tmp15 = add i64 %12, %i.06
  %scevgep = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %tmp15
  %26 = load i32 addrspace(1)* %13, align 4
  %27 = load i32 addrspace(1)* %scevgep, align 4
  %28 = add i32 %27, %26
  store i32 %28, i32 addrspace(1)* %scevgep, align 4
  %29 = add i64 %i.06, 1
  %exitcond = icmp eq i64 %29, %0
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %.preheader
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB30

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB30:                                         ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.prefixSumStep1_separated_args(i32 addrspace(1)* nocapture %puiInputArray, i32 addrspace(1)* nocapture %puiOutputArray, i32 addrspace(1)* nocapture %puiTmpArray, i32 %szElementsPerItem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = zext i32 %szElementsPerItem to i64
  %temp = insertelement <16 x i64> undef, i64 %0, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %1 = icmp eq i32 %szElementsPerItem, 0
  %2 = add i32 %szElementsPerItem, -1
  %3 = zext i32 %2 to i64
  %temp110 = insertelement <16 x i64> undef, i64 %3, i32 0
  %vector111 = shufflevector <16 x i64> %temp110, <16 x i64> undef, <16 x i32> zeroinitializer
  %4 = lshr i32 %szElementsPerItem, 1
  %5 = icmp eq i32 %4, 0
  %6 = zext i32 %4 to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %8 = load i64* %7, align 8
  %9 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = add i64 %8, %10
  %broadcast1 = insertelement <16 x i64> undef, i64 %11, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %12 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %extract129 = extractelement <16 x i64> %12, i32 0
  %13 = mul <16 x i64> %12, %vector
  br i1 %1, label %._crit_edge10, label %bb.nph12

bb.nph12:                                         ; preds = %SyncBB, %bb.nph12
  %i.011 = phi i64 [ %62, %bb.nph12 ], [ 0, %SyncBB ]
  %temp2 = insertelement <16 x i64> undef, i64 %i.011, i32 0
  %vector3 = shufflevector <16 x i64> %temp2, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp274 = add <16 x i64> %13, %vector3
  %extract = extractelement <16 x i64> %tmp274, i32 0
  %extract5 = extractelement <16 x i64> %tmp274, i32 1
  %extract6 = extractelement <16 x i64> %tmp274, i32 2
  %extract7 = extractelement <16 x i64> %tmp274, i32 3
  %extract8 = extractelement <16 x i64> %tmp274, i32 4
  %extract9 = extractelement <16 x i64> %tmp274, i32 5
  %extract10 = extractelement <16 x i64> %tmp274, i32 6
  %extract11 = extractelement <16 x i64> %tmp274, i32 7
  %extract12 = extractelement <16 x i64> %tmp274, i32 8
  %extract13 = extractelement <16 x i64> %tmp274, i32 9
  %extract14 = extractelement <16 x i64> %tmp274, i32 10
  %extract15 = extractelement <16 x i64> %tmp274, i32 11
  %extract16 = extractelement <16 x i64> %tmp274, i32 12
  %extract17 = extractelement <16 x i64> %tmp274, i32 13
  %extract18 = extractelement <16 x i64> %tmp274, i32 14
  %extract19 = extractelement <16 x i64> %tmp274, i32 15
  %14 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract
  %15 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract5
  %16 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract6
  %17 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract7
  %18 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract8
  %19 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract9
  %20 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract10
  %21 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract11
  %22 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract12
  %23 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract13
  %24 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract14
  %25 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract15
  %26 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract16
  %27 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract17
  %28 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract18
  %29 = getelementptr i32 addrspace(1)* %puiInputArray, i64 %extract19
  %30 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract
  %31 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract5
  %32 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract6
  %33 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract7
  %34 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract8
  %35 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract9
  %36 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract10
  %37 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract11
  %38 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract12
  %39 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract13
  %40 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract14
  %41 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract15
  %42 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract16
  %43 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract17
  %44 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract18
  %45 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract19
  %46 = load i32 addrspace(1)* %14, align 4
  %47 = load i32 addrspace(1)* %15, align 4
  %48 = load i32 addrspace(1)* %16, align 4
  %49 = load i32 addrspace(1)* %17, align 4
  %50 = load i32 addrspace(1)* %18, align 4
  %51 = load i32 addrspace(1)* %19, align 4
  %52 = load i32 addrspace(1)* %20, align 4
  %53 = load i32 addrspace(1)* %21, align 4
  %54 = load i32 addrspace(1)* %22, align 4
  %55 = load i32 addrspace(1)* %23, align 4
  %56 = load i32 addrspace(1)* %24, align 4
  %57 = load i32 addrspace(1)* %25, align 4
  %58 = load i32 addrspace(1)* %26, align 4
  %59 = load i32 addrspace(1)* %27, align 4
  %60 = load i32 addrspace(1)* %28, align 4
  %61 = load i32 addrspace(1)* %29, align 4
  store i32 %46, i32 addrspace(1)* %30, align 4
  store i32 %47, i32 addrspace(1)* %31, align 4
  store i32 %48, i32 addrspace(1)* %32, align 4
  store i32 %49, i32 addrspace(1)* %33, align 4
  store i32 %50, i32 addrspace(1)* %34, align 4
  store i32 %51, i32 addrspace(1)* %35, align 4
  store i32 %52, i32 addrspace(1)* %36, align 4
  store i32 %53, i32 addrspace(1)* %37, align 4
  store i32 %54, i32 addrspace(1)* %38, align 4
  store i32 %55, i32 addrspace(1)* %39, align 4
  store i32 %56, i32 addrspace(1)* %40, align 4
  store i32 %57, i32 addrspace(1)* %41, align 4
  store i32 %58, i32 addrspace(1)* %42, align 4
  store i32 %59, i32 addrspace(1)* %43, align 4
  store i32 %60, i32 addrspace(1)* %44, align 4
  store i32 %61, i32 addrspace(1)* %45, align 4
  %62 = add i64 %i.011, 1
  %exitcond25 = icmp eq i64 %62, %0
  br i1 %exitcond25, label %._crit_edge13, label %bb.nph12

._crit_edge13:                                    ; preds = %bb.nph12
  br i1 %5, label %._crit_edge10, label %bb.nph9

bb.nph9:                                          ; preds = %._crit_edge13
  %tmp1720 = add <16 x i64> %13, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  br label %63

; <label>:63                                      ; preds = %._crit_edge, %bb.nph9
  %h.08 = phi i64 [ %6, %bb.nph9 ], [ %132, %._crit_edge ]
  %offset.07 = phi i64 [ 1, %bb.nph9 ], [ %133, %._crit_edge ]
  %64 = icmp eq i64 %h.08, 0
  br i1 %64, label %._crit_edge10, label %bb.nph

bb.nph:                                           ; preds = %63
  %temp21 = insertelement <16 x i64> undef, i64 %offset.07, i32 0
  %vector22 = shufflevector <16 x i64> %temp21, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp = shl i64 %offset.07, 1
  %temp24 = insertelement <16 x i64> undef, i64 %tmp, i32 0
  %vector25 = shufflevector <16 x i64> %temp24, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1823 = add <16 x i64> %tmp1720, %vector22
  %tmp2126 = add <16 x i64> %tmp1720, %vector25
  br label %65

; <label>:65                                      ; preds = %65, %bb.nph
  %p.06 = phi i64 [ 0, %bb.nph ], [ %131, %65 ]
  %tmp14 = mul i64 %tmp, %p.06
  %temp27 = insertelement <16 x i64> undef, i64 %tmp14, i32 0
  %vector28 = shufflevector <16 x i64> %temp27, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1929 = add <16 x i64> %tmp1823, %vector28
  %extract30 = extractelement <16 x i64> %tmp1929, i32 0
  %extract31 = extractelement <16 x i64> %tmp1929, i32 1
  %extract32 = extractelement <16 x i64> %tmp1929, i32 2
  %extract33 = extractelement <16 x i64> %tmp1929, i32 3
  %extract34 = extractelement <16 x i64> %tmp1929, i32 4
  %extract35 = extractelement <16 x i64> %tmp1929, i32 5
  %extract36 = extractelement <16 x i64> %tmp1929, i32 6
  %extract37 = extractelement <16 x i64> %tmp1929, i32 7
  %extract38 = extractelement <16 x i64> %tmp1929, i32 8
  %extract39 = extractelement <16 x i64> %tmp1929, i32 9
  %extract40 = extractelement <16 x i64> %tmp1929, i32 10
  %extract41 = extractelement <16 x i64> %tmp1929, i32 11
  %extract42 = extractelement <16 x i64> %tmp1929, i32 12
  %extract43 = extractelement <16 x i64> %tmp1929, i32 13
  %extract44 = extractelement <16 x i64> %tmp1929, i32 14
  %extract45 = extractelement <16 x i64> %tmp1929, i32 15
  %66 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract30
  %67 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract31
  %68 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract32
  %69 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract33
  %70 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract34
  %71 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract35
  %72 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract36
  %73 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract37
  %74 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract38
  %75 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract39
  %76 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract40
  %77 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract41
  %78 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract42
  %79 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract43
  %80 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract44
  %81 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract45
  %tmp2246 = add <16 x i64> %tmp2126, %vector28
  %extract47 = extractelement <16 x i64> %tmp2246, i32 0
  %extract48 = extractelement <16 x i64> %tmp2246, i32 1
  %extract49 = extractelement <16 x i64> %tmp2246, i32 2
  %extract50 = extractelement <16 x i64> %tmp2246, i32 3
  %extract51 = extractelement <16 x i64> %tmp2246, i32 4
  %extract52 = extractelement <16 x i64> %tmp2246, i32 5
  %extract53 = extractelement <16 x i64> %tmp2246, i32 6
  %extract54 = extractelement <16 x i64> %tmp2246, i32 7
  %extract55 = extractelement <16 x i64> %tmp2246, i32 8
  %extract56 = extractelement <16 x i64> %tmp2246, i32 9
  %extract57 = extractelement <16 x i64> %tmp2246, i32 10
  %extract58 = extractelement <16 x i64> %tmp2246, i32 11
  %extract59 = extractelement <16 x i64> %tmp2246, i32 12
  %extract60 = extractelement <16 x i64> %tmp2246, i32 13
  %extract61 = extractelement <16 x i64> %tmp2246, i32 14
  %extract62 = extractelement <16 x i64> %tmp2246, i32 15
  %82 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract47
  %83 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract48
  %84 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract49
  %85 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract50
  %86 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract51
  %87 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract52
  %88 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract53
  %89 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract54
  %90 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract55
  %91 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract56
  %92 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract57
  %93 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract58
  %94 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract59
  %95 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract60
  %96 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract61
  %97 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract62
  %98 = load i32 addrspace(1)* %66, align 4
  %99 = load i32 addrspace(1)* %67, align 4
  %100 = load i32 addrspace(1)* %68, align 4
  %101 = load i32 addrspace(1)* %69, align 4
  %102 = load i32 addrspace(1)* %70, align 4
  %103 = load i32 addrspace(1)* %71, align 4
  %104 = load i32 addrspace(1)* %72, align 4
  %105 = load i32 addrspace(1)* %73, align 4
  %106 = load i32 addrspace(1)* %74, align 4
  %107 = load i32 addrspace(1)* %75, align 4
  %108 = load i32 addrspace(1)* %76, align 4
  %109 = load i32 addrspace(1)* %77, align 4
  %110 = load i32 addrspace(1)* %78, align 4
  %111 = load i32 addrspace(1)* %79, align 4
  %112 = load i32 addrspace(1)* %80, align 4
  %113 = load i32 addrspace(1)* %81, align 4
  %temp.vect78 = insertelement <16 x i32> undef, i32 %98, i32 0
  %temp.vect79 = insertelement <16 x i32> %temp.vect78, i32 %99, i32 1
  %temp.vect80 = insertelement <16 x i32> %temp.vect79, i32 %100, i32 2
  %temp.vect81 = insertelement <16 x i32> %temp.vect80, i32 %101, i32 3
  %temp.vect82 = insertelement <16 x i32> %temp.vect81, i32 %102, i32 4
  %temp.vect83 = insertelement <16 x i32> %temp.vect82, i32 %103, i32 5
  %temp.vect84 = insertelement <16 x i32> %temp.vect83, i32 %104, i32 6
  %temp.vect85 = insertelement <16 x i32> %temp.vect84, i32 %105, i32 7
  %temp.vect86 = insertelement <16 x i32> %temp.vect85, i32 %106, i32 8
  %temp.vect87 = insertelement <16 x i32> %temp.vect86, i32 %107, i32 9
  %temp.vect88 = insertelement <16 x i32> %temp.vect87, i32 %108, i32 10
  %temp.vect89 = insertelement <16 x i32> %temp.vect88, i32 %109, i32 11
  %temp.vect90 = insertelement <16 x i32> %temp.vect89, i32 %110, i32 12
  %temp.vect91 = insertelement <16 x i32> %temp.vect90, i32 %111, i32 13
  %temp.vect92 = insertelement <16 x i32> %temp.vect91, i32 %112, i32 14
  %temp.vect93 = insertelement <16 x i32> %temp.vect92, i32 %113, i32 15
  %114 = load i32 addrspace(1)* %82, align 4
  %115 = load i32 addrspace(1)* %83, align 4
  %116 = load i32 addrspace(1)* %84, align 4
  %117 = load i32 addrspace(1)* %85, align 4
  %118 = load i32 addrspace(1)* %86, align 4
  %119 = load i32 addrspace(1)* %87, align 4
  %120 = load i32 addrspace(1)* %88, align 4
  %121 = load i32 addrspace(1)* %89, align 4
  %122 = load i32 addrspace(1)* %90, align 4
  %123 = load i32 addrspace(1)* %91, align 4
  %124 = load i32 addrspace(1)* %92, align 4
  %125 = load i32 addrspace(1)* %93, align 4
  %126 = load i32 addrspace(1)* %94, align 4
  %127 = load i32 addrspace(1)* %95, align 4
  %128 = load i32 addrspace(1)* %96, align 4
  %129 = load i32 addrspace(1)* %97, align 4
  %temp.vect = insertelement <16 x i32> undef, i32 %114, i32 0
  %temp.vect63 = insertelement <16 x i32> %temp.vect, i32 %115, i32 1
  %temp.vect64 = insertelement <16 x i32> %temp.vect63, i32 %116, i32 2
  %temp.vect65 = insertelement <16 x i32> %temp.vect64, i32 %117, i32 3
  %temp.vect66 = insertelement <16 x i32> %temp.vect65, i32 %118, i32 4
  %temp.vect67 = insertelement <16 x i32> %temp.vect66, i32 %119, i32 5
  %temp.vect68 = insertelement <16 x i32> %temp.vect67, i32 %120, i32 6
  %temp.vect69 = insertelement <16 x i32> %temp.vect68, i32 %121, i32 7
  %temp.vect70 = insertelement <16 x i32> %temp.vect69, i32 %122, i32 8
  %temp.vect71 = insertelement <16 x i32> %temp.vect70, i32 %123, i32 9
  %temp.vect72 = insertelement <16 x i32> %temp.vect71, i32 %124, i32 10
  %temp.vect73 = insertelement <16 x i32> %temp.vect72, i32 %125, i32 11
  %temp.vect74 = insertelement <16 x i32> %temp.vect73, i32 %126, i32 12
  %temp.vect75 = insertelement <16 x i32> %temp.vect74, i32 %127, i32 13
  %temp.vect76 = insertelement <16 x i32> %temp.vect75, i32 %128, i32 14
  %temp.vect77 = insertelement <16 x i32> %temp.vect76, i32 %129, i32 15
  %130 = add <16 x i32> %temp.vect77, %temp.vect93
  %extract94 = extractelement <16 x i32> %130, i32 0
  %extract95 = extractelement <16 x i32> %130, i32 1
  %extract96 = extractelement <16 x i32> %130, i32 2
  %extract97 = extractelement <16 x i32> %130, i32 3
  %extract98 = extractelement <16 x i32> %130, i32 4
  %extract99 = extractelement <16 x i32> %130, i32 5
  %extract100 = extractelement <16 x i32> %130, i32 6
  %extract101 = extractelement <16 x i32> %130, i32 7
  %extract102 = extractelement <16 x i32> %130, i32 8
  %extract103 = extractelement <16 x i32> %130, i32 9
  %extract104 = extractelement <16 x i32> %130, i32 10
  %extract105 = extractelement <16 x i32> %130, i32 11
  %extract106 = extractelement <16 x i32> %130, i32 12
  %extract107 = extractelement <16 x i32> %130, i32 13
  %extract108 = extractelement <16 x i32> %130, i32 14
  %extract109 = extractelement <16 x i32> %130, i32 15
  store i32 %extract94, i32 addrspace(1)* %82, align 4
  store i32 %extract95, i32 addrspace(1)* %83, align 4
  store i32 %extract96, i32 addrspace(1)* %84, align 4
  store i32 %extract97, i32 addrspace(1)* %85, align 4
  store i32 %extract98, i32 addrspace(1)* %86, align 4
  store i32 %extract99, i32 addrspace(1)* %87, align 4
  store i32 %extract100, i32 addrspace(1)* %88, align 4
  store i32 %extract101, i32 addrspace(1)* %89, align 4
  store i32 %extract102, i32 addrspace(1)* %90, align 4
  store i32 %extract103, i32 addrspace(1)* %91, align 4
  store i32 %extract104, i32 addrspace(1)* %92, align 4
  store i32 %extract105, i32 addrspace(1)* %93, align 4
  store i32 %extract106, i32 addrspace(1)* %94, align 4
  store i32 %extract107, i32 addrspace(1)* %95, align 4
  store i32 %extract108, i32 addrspace(1)* %96, align 4
  store i32 %extract109, i32 addrspace(1)* %97, align 4
  %131 = add i64 %p.06, 1
  %exitcond = icmp eq i64 %131, %h.08
  br i1 %exitcond, label %._crit_edge, label %65

._crit_edge:                                      ; preds = %65
  %132 = lshr i64 %h.08, 1
  %133 = shl i64 %offset.07, 1
  %134 = icmp eq i64 %132, 0
  br i1 %134, label %._crit_edge10, label %63

._crit_edge10:                                    ; preds = %._crit_edge13, %._crit_edge, %63, %SyncBB
  %.sum112 = add <16 x i64> %13, %vector111
  %extract113 = extractelement <16 x i64> %.sum112, i32 0
  %extract114 = extractelement <16 x i64> %.sum112, i32 1
  %extract115 = extractelement <16 x i64> %.sum112, i32 2
  %extract116 = extractelement <16 x i64> %.sum112, i32 3
  %extract117 = extractelement <16 x i64> %.sum112, i32 4
  %extract118 = extractelement <16 x i64> %.sum112, i32 5
  %extract119 = extractelement <16 x i64> %.sum112, i32 6
  %extract120 = extractelement <16 x i64> %.sum112, i32 7
  %extract121 = extractelement <16 x i64> %.sum112, i32 8
  %extract122 = extractelement <16 x i64> %.sum112, i32 9
  %extract123 = extractelement <16 x i64> %.sum112, i32 10
  %extract124 = extractelement <16 x i64> %.sum112, i32 11
  %extract125 = extractelement <16 x i64> %.sum112, i32 12
  %extract126 = extractelement <16 x i64> %.sum112, i32 13
  %extract127 = extractelement <16 x i64> %.sum112, i32 14
  %extract128 = extractelement <16 x i64> %.sum112, i32 15
  %135 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract113
  %136 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract114
  %137 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract115
  %138 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract116
  %139 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract117
  %140 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract118
  %141 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract119
  %142 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract120
  %143 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract121
  %144 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract122
  %145 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract123
  %146 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract124
  %147 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract125
  %148 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract126
  %149 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract127
  %150 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract128
  %151 = load i32 addrspace(1)* %135, align 4
  %152 = load i32 addrspace(1)* %136, align 4
  %153 = load i32 addrspace(1)* %137, align 4
  %154 = load i32 addrspace(1)* %138, align 4
  %155 = load i32 addrspace(1)* %139, align 4
  %156 = load i32 addrspace(1)* %140, align 4
  %157 = load i32 addrspace(1)* %141, align 4
  %158 = load i32 addrspace(1)* %142, align 4
  %159 = load i32 addrspace(1)* %143, align 4
  %160 = load i32 addrspace(1)* %144, align 4
  %161 = load i32 addrspace(1)* %145, align 4
  %162 = load i32 addrspace(1)* %146, align 4
  %163 = load i32 addrspace(1)* %147, align 4
  %164 = load i32 addrspace(1)* %148, align 4
  %165 = load i32 addrspace(1)* %149, align 4
  %166 = load i32 addrspace(1)* %150, align 4
  %temp.vect145 = insertelement <16 x i32> undef, i32 %151, i32 0
  %temp.vect146 = insertelement <16 x i32> %temp.vect145, i32 %152, i32 1
  %temp.vect147 = insertelement <16 x i32> %temp.vect146, i32 %153, i32 2
  %temp.vect148 = insertelement <16 x i32> %temp.vect147, i32 %154, i32 3
  %temp.vect149 = insertelement <16 x i32> %temp.vect148, i32 %155, i32 4
  %temp.vect150 = insertelement <16 x i32> %temp.vect149, i32 %156, i32 5
  %temp.vect151 = insertelement <16 x i32> %temp.vect150, i32 %157, i32 6
  %temp.vect152 = insertelement <16 x i32> %temp.vect151, i32 %158, i32 7
  %temp.vect153 = insertelement <16 x i32> %temp.vect152, i32 %159, i32 8
  %temp.vect154 = insertelement <16 x i32> %temp.vect153, i32 %160, i32 9
  %temp.vect155 = insertelement <16 x i32> %temp.vect154, i32 %161, i32 10
  %temp.vect156 = insertelement <16 x i32> %temp.vect155, i32 %162, i32 11
  %temp.vect157 = insertelement <16 x i32> %temp.vect156, i32 %163, i32 12
  %temp.vect158 = insertelement <16 x i32> %temp.vect157, i32 %164, i32 13
  %temp.vect159 = insertelement <16 x i32> %temp.vect158, i32 %165, i32 14
  %temp.vect160 = insertelement <16 x i32> %temp.vect159, i32 %166, i32 15
  %167 = getelementptr inbounds i32 addrspace(1)* %puiTmpArray, i64 %extract129
  %ptrTypeCast = bitcast i32 addrspace(1)* %167 to <16 x i32> addrspace(1)*
  store <16 x i32> %temp.vect160, <16 x i32> addrspace(1)* %ptrTypeCast, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB162

thenBB:                                           ; preds = %._crit_edge10
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB162:                                        ; preds = %._crit_edge10
  ret void
}

define void @____Vectorized_.prefixSumStep2_separated_args(i32 addrspace(1)* nocapture %puiOutputArray, i32 addrspace(1)* nocapture %puiValueToAddArray, i32 %szElementsPerItem, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = zext i32 %szElementsPerItem to i64
  %temp = insertelement <16 x i64> undef, i64 %0, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %1 = lshr i32 %szElementsPerItem, 1
  %2 = zext i32 %1 to i64
  %3 = add i32 %szElementsPerItem, -1
  %4 = zext i32 %3 to i64
  %temp16 = insertelement <16 x i64> undef, i64 %4, i32 0
  %vector17 = shufflevector <16 x i64> %temp16, <16 x i64> undef, <16 x i32> zeroinitializer
  %5 = icmp eq i32 %1, 0
  %6 = icmp eq i32 %szElementsPerItem, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %8 = load i64* %7, align 8
  %9 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = add i64 %8, %10
  %broadcast1 = insertelement <16 x i64> undef, i64 %11, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %12 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %extract = extractelement <16 x i64> %12, i32 0
  %13 = mul <16 x i64> %12, %vector
  %14 = getelementptr inbounds i32 addrspace(1)* %puiValueToAddArray, i64 %extract
  %.sum18 = add <16 x i64> %13, %vector17
  %extract19 = extractelement <16 x i64> %.sum18, i32 0
  %extract20 = extractelement <16 x i64> %.sum18, i32 1
  %extract21 = extractelement <16 x i64> %.sum18, i32 2
  %extract22 = extractelement <16 x i64> %.sum18, i32 3
  %extract23 = extractelement <16 x i64> %.sum18, i32 4
  %extract24 = extractelement <16 x i64> %.sum18, i32 5
  %extract25 = extractelement <16 x i64> %.sum18, i32 6
  %extract26 = extractelement <16 x i64> %.sum18, i32 7
  %extract27 = extractelement <16 x i64> %.sum18, i32 8
  %extract28 = extractelement <16 x i64> %.sum18, i32 9
  %extract29 = extractelement <16 x i64> %.sum18, i32 10
  %extract30 = extractelement <16 x i64> %.sum18, i32 11
  %extract31 = extractelement <16 x i64> %.sum18, i32 12
  %extract32 = extractelement <16 x i64> %.sum18, i32 13
  %extract33 = extractelement <16 x i64> %.sum18, i32 14
  %extract34 = extractelement <16 x i64> %.sum18, i32 15
  %15 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract19
  %16 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract20
  %17 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract21
  %18 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract22
  %19 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract23
  %20 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract24
  %21 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract25
  %22 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract26
  %23 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract27
  %24 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract28
  %25 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract29
  %26 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract30
  %27 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract31
  %28 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract32
  %29 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract33
  %30 = getelementptr inbounds i32 addrspace(1)* %puiOutputArray, i64 %extract34
  store i32 0, i32 addrspace(1)* %15, align 4
  store i32 0, i32 addrspace(1)* %16, align 4
  store i32 0, i32 addrspace(1)* %17, align 4
  store i32 0, i32 addrspace(1)* %18, align 4
  store i32 0, i32 addrspace(1)* %19, align 4
  store i32 0, i32 addrspace(1)* %20, align 4
  store i32 0, i32 addrspace(1)* %21, align 4
  store i32 0, i32 addrspace(1)* %22, align 4
  store i32 0, i32 addrspace(1)* %23, align 4
  store i32 0, i32 addrspace(1)* %24, align 4
  store i32 0, i32 addrspace(1)* %25, align 4
  store i32 0, i32 addrspace(1)* %26, align 4
  store i32 0, i32 addrspace(1)* %27, align 4
  store i32 0, i32 addrspace(1)* %28, align 4
  store i32 0, i32 addrspace(1)* %29, align 4
  store i32 0, i32 addrspace(1)* %30, align 4
  br i1 %5, label %.preheader, label %bb.nph13

.preheader:                                       ; preds = %._crit_edge10, %SyncBB
  br i1 %6, label %._crit_edge, label %bb.nph.preheader

bb.nph.preheader:                                 ; preds = %.preheader
  %ptrTypeCast = bitcast i32 addrspace(1)* %14 to <16 x i32> addrspace(1)*
  br label %bb.nph

bb.nph13:                                         ; preds = %SyncBB
  %tmp2035 = add <16 x i64> %13, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  br label %31

; <label>:31                                      ; preds = %._crit_edge10, %bb.nph13
  %h.012 = phi i64 [ 1, %bb.nph13 ], [ %102, %._crit_edge10 ]
  %offset.011 = phi i64 [ %0, %bb.nph13 ], [ %32, %._crit_edge10 ]
  %32 = lshr i64 %offset.011, 1
  %33 = icmp eq i64 %h.012, 0
  br i1 %33, label %._crit_edge10, label %bb.nph9

bb.nph9:                                          ; preds = %31
  %temp39 = insertelement <16 x i64> undef, i64 %32, i32 0
  %vector40 = shufflevector <16 x i64> %temp39, <16 x i64> undef, <16 x i32> zeroinitializer
  %34 = and i64 %offset.011, 9223372036854775806
  %temp36 = insertelement <16 x i64> undef, i64 %34, i32 0
  %vector37 = shufflevector <16 x i64> %temp36, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp2338 = add <16 x i64> %tmp2035, %vector37
  %tmp2741 = add <16 x i64> %tmp2035, %vector40
  br label %35

; <label>:35                                      ; preds = %35, %bb.nph9
  %p.08 = phi i64 [ 0, %bb.nph9 ], [ %101, %35 ]
  %tmp17 = mul i64 %34, %p.08
  %temp42 = insertelement <16 x i64> undef, i64 %tmp17, i32 0
  %vector43 = shufflevector <16 x i64> %temp42, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp2444 = add <16 x i64> %tmp2338, %vector43
  %extract45 = extractelement <16 x i64> %tmp2444, i32 0
  %extract46 = extractelement <16 x i64> %tmp2444, i32 1
  %extract47 = extractelement <16 x i64> %tmp2444, i32 2
  %extract48 = extractelement <16 x i64> %tmp2444, i32 3
  %extract49 = extractelement <16 x i64> %tmp2444, i32 4
  %extract50 = extractelement <16 x i64> %tmp2444, i32 5
  %extract51 = extractelement <16 x i64> %tmp2444, i32 6
  %extract52 = extractelement <16 x i64> %tmp2444, i32 7
  %extract53 = extractelement <16 x i64> %tmp2444, i32 8
  %extract54 = extractelement <16 x i64> %tmp2444, i32 9
  %extract55 = extractelement <16 x i64> %tmp2444, i32 10
  %extract56 = extractelement <16 x i64> %tmp2444, i32 11
  %extract57 = extractelement <16 x i64> %tmp2444, i32 12
  %extract58 = extractelement <16 x i64> %tmp2444, i32 13
  %extract59 = extractelement <16 x i64> %tmp2444, i32 14
  %extract60 = extractelement <16 x i64> %tmp2444, i32 15
  %36 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract45
  %37 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract46
  %38 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract47
  %39 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract48
  %40 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract49
  %41 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract50
  %42 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract51
  %43 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract52
  %44 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract53
  %45 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract54
  %46 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract55
  %47 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract56
  %48 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract57
  %49 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract58
  %50 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract59
  %51 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract60
  %tmp2861 = add <16 x i64> %tmp2741, %vector43
  %extract62 = extractelement <16 x i64> %tmp2861, i32 0
  %extract63 = extractelement <16 x i64> %tmp2861, i32 1
  %extract64 = extractelement <16 x i64> %tmp2861, i32 2
  %extract65 = extractelement <16 x i64> %tmp2861, i32 3
  %extract66 = extractelement <16 x i64> %tmp2861, i32 4
  %extract67 = extractelement <16 x i64> %tmp2861, i32 5
  %extract68 = extractelement <16 x i64> %tmp2861, i32 6
  %extract69 = extractelement <16 x i64> %tmp2861, i32 7
  %extract70 = extractelement <16 x i64> %tmp2861, i32 8
  %extract71 = extractelement <16 x i64> %tmp2861, i32 9
  %extract72 = extractelement <16 x i64> %tmp2861, i32 10
  %extract73 = extractelement <16 x i64> %tmp2861, i32 11
  %extract74 = extractelement <16 x i64> %tmp2861, i32 12
  %extract75 = extractelement <16 x i64> %tmp2861, i32 13
  %extract76 = extractelement <16 x i64> %tmp2861, i32 14
  %extract77 = extractelement <16 x i64> %tmp2861, i32 15
  %52 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract62
  %53 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract63
  %54 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract64
  %55 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract65
  %56 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract66
  %57 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract67
  %58 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract68
  %59 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract69
  %60 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract70
  %61 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract71
  %62 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract72
  %63 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract73
  %64 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract74
  %65 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract75
  %66 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract76
  %67 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract77
  %68 = load i32 addrspace(1)* %36, align 4
  %69 = load i32 addrspace(1)* %37, align 4
  %70 = load i32 addrspace(1)* %38, align 4
  %71 = load i32 addrspace(1)* %39, align 4
  %72 = load i32 addrspace(1)* %40, align 4
  %73 = load i32 addrspace(1)* %41, align 4
  %74 = load i32 addrspace(1)* %42, align 4
  %75 = load i32 addrspace(1)* %43, align 4
  %76 = load i32 addrspace(1)* %44, align 4
  %77 = load i32 addrspace(1)* %45, align 4
  %78 = load i32 addrspace(1)* %46, align 4
  %79 = load i32 addrspace(1)* %47, align 4
  %80 = load i32 addrspace(1)* %48, align 4
  %81 = load i32 addrspace(1)* %49, align 4
  %82 = load i32 addrspace(1)* %50, align 4
  %83 = load i32 addrspace(1)* %51, align 4
  %temp.vect = insertelement <16 x i32> undef, i32 %68, i32 0
  %temp.vect78 = insertelement <16 x i32> %temp.vect, i32 %69, i32 1
  %temp.vect79 = insertelement <16 x i32> %temp.vect78, i32 %70, i32 2
  %temp.vect80 = insertelement <16 x i32> %temp.vect79, i32 %71, i32 3
  %temp.vect81 = insertelement <16 x i32> %temp.vect80, i32 %72, i32 4
  %temp.vect82 = insertelement <16 x i32> %temp.vect81, i32 %73, i32 5
  %temp.vect83 = insertelement <16 x i32> %temp.vect82, i32 %74, i32 6
  %temp.vect84 = insertelement <16 x i32> %temp.vect83, i32 %75, i32 7
  %temp.vect85 = insertelement <16 x i32> %temp.vect84, i32 %76, i32 8
  %temp.vect86 = insertelement <16 x i32> %temp.vect85, i32 %77, i32 9
  %temp.vect87 = insertelement <16 x i32> %temp.vect86, i32 %78, i32 10
  %temp.vect88 = insertelement <16 x i32> %temp.vect87, i32 %79, i32 11
  %temp.vect89 = insertelement <16 x i32> %temp.vect88, i32 %80, i32 12
  %temp.vect90 = insertelement <16 x i32> %temp.vect89, i32 %81, i32 13
  %temp.vect91 = insertelement <16 x i32> %temp.vect90, i32 %82, i32 14
  %temp.vect92 = insertelement <16 x i32> %temp.vect91, i32 %83, i32 15
  %84 = load i32 addrspace(1)* %52, align 4
  %85 = load i32 addrspace(1)* %53, align 4
  %86 = load i32 addrspace(1)* %54, align 4
  %87 = load i32 addrspace(1)* %55, align 4
  %88 = load i32 addrspace(1)* %56, align 4
  %89 = load i32 addrspace(1)* %57, align 4
  %90 = load i32 addrspace(1)* %58, align 4
  %91 = load i32 addrspace(1)* %59, align 4
  %92 = load i32 addrspace(1)* %60, align 4
  %93 = load i32 addrspace(1)* %61, align 4
  %94 = load i32 addrspace(1)* %62, align 4
  %95 = load i32 addrspace(1)* %63, align 4
  %96 = load i32 addrspace(1)* %64, align 4
  %97 = load i32 addrspace(1)* %65, align 4
  %98 = load i32 addrspace(1)* %66, align 4
  %99 = load i32 addrspace(1)* %67, align 4
  %temp.vect93 = insertelement <16 x i32> undef, i32 %84, i32 0
  %temp.vect94 = insertelement <16 x i32> %temp.vect93, i32 %85, i32 1
  %temp.vect95 = insertelement <16 x i32> %temp.vect94, i32 %86, i32 2
  %temp.vect96 = insertelement <16 x i32> %temp.vect95, i32 %87, i32 3
  %temp.vect97 = insertelement <16 x i32> %temp.vect96, i32 %88, i32 4
  %temp.vect98 = insertelement <16 x i32> %temp.vect97, i32 %89, i32 5
  %temp.vect99 = insertelement <16 x i32> %temp.vect98, i32 %90, i32 6
  %temp.vect100 = insertelement <16 x i32> %temp.vect99, i32 %91, i32 7
  %temp.vect101 = insertelement <16 x i32> %temp.vect100, i32 %92, i32 8
  %temp.vect102 = insertelement <16 x i32> %temp.vect101, i32 %93, i32 9
  %temp.vect103 = insertelement <16 x i32> %temp.vect102, i32 %94, i32 10
  %temp.vect104 = insertelement <16 x i32> %temp.vect103, i32 %95, i32 11
  %temp.vect105 = insertelement <16 x i32> %temp.vect104, i32 %96, i32 12
  %temp.vect106 = insertelement <16 x i32> %temp.vect105, i32 %97, i32 13
  %temp.vect107 = insertelement <16 x i32> %temp.vect106, i32 %98, i32 14
  %temp.vect108 = insertelement <16 x i32> %temp.vect107, i32 %99, i32 15
  %100 = add <16 x i32> %temp.vect92, %temp.vect108
  %extract109 = extractelement <16 x i32> %100, i32 0
  %extract110 = extractelement <16 x i32> %100, i32 1
  %extract111 = extractelement <16 x i32> %100, i32 2
  %extract112 = extractelement <16 x i32> %100, i32 3
  %extract113 = extractelement <16 x i32> %100, i32 4
  %extract114 = extractelement <16 x i32> %100, i32 5
  %extract115 = extractelement <16 x i32> %100, i32 6
  %extract116 = extractelement <16 x i32> %100, i32 7
  %extract117 = extractelement <16 x i32> %100, i32 8
  %extract118 = extractelement <16 x i32> %100, i32 9
  %extract119 = extractelement <16 x i32> %100, i32 10
  %extract120 = extractelement <16 x i32> %100, i32 11
  %extract121 = extractelement <16 x i32> %100, i32 12
  %extract122 = extractelement <16 x i32> %100, i32 13
  %extract123 = extractelement <16 x i32> %100, i32 14
  %extract124 = extractelement <16 x i32> %100, i32 15
  store i32 %extract109, i32 addrspace(1)* %36, align 4
  store i32 %extract110, i32 addrspace(1)* %37, align 4
  store i32 %extract111, i32 addrspace(1)* %38, align 4
  store i32 %extract112, i32 addrspace(1)* %39, align 4
  store i32 %extract113, i32 addrspace(1)* %40, align 4
  store i32 %extract114, i32 addrspace(1)* %41, align 4
  store i32 %extract115, i32 addrspace(1)* %42, align 4
  store i32 %extract116, i32 addrspace(1)* %43, align 4
  store i32 %extract117, i32 addrspace(1)* %44, align 4
  store i32 %extract118, i32 addrspace(1)* %45, align 4
  store i32 %extract119, i32 addrspace(1)* %46, align 4
  store i32 %extract120, i32 addrspace(1)* %47, align 4
  store i32 %extract121, i32 addrspace(1)* %48, align 4
  store i32 %extract122, i32 addrspace(1)* %49, align 4
  store i32 %extract123, i32 addrspace(1)* %50, align 4
  store i32 %extract124, i32 addrspace(1)* %51, align 4
  store i32 %68, i32 addrspace(1)* %52, align 4
  store i32 %69, i32 addrspace(1)* %53, align 4
  store i32 %70, i32 addrspace(1)* %54, align 4
  store i32 %71, i32 addrspace(1)* %55, align 4
  store i32 %72, i32 addrspace(1)* %56, align 4
  store i32 %73, i32 addrspace(1)* %57, align 4
  store i32 %74, i32 addrspace(1)* %58, align 4
  store i32 %75, i32 addrspace(1)* %59, align 4
  store i32 %76, i32 addrspace(1)* %60, align 4
  store i32 %77, i32 addrspace(1)* %61, align 4
  store i32 %78, i32 addrspace(1)* %62, align 4
  store i32 %79, i32 addrspace(1)* %63, align 4
  store i32 %80, i32 addrspace(1)* %64, align 4
  store i32 %81, i32 addrspace(1)* %65, align 4
  store i32 %82, i32 addrspace(1)* %66, align 4
  store i32 %83, i32 addrspace(1)* %67, align 4
  %101 = add i64 %p.08, 1
  %exitcond16 = icmp eq i64 %101, %h.012
  br i1 %exitcond16, label %._crit_edge10, label %35

._crit_edge10:                                    ; preds = %35, %31
  %102 = shl i64 %h.012, 1
  %103 = icmp ugt i64 %102, %2
  br i1 %103, label %.preheader, label %31

bb.nph:                                           ; preds = %bb.nph, %bb.nph.preheader
  %i.06 = phi i64 [ %138, %bb.nph ], [ 0, %bb.nph.preheader ]
  %temp125 = insertelement <16 x i64> undef, i64 %i.06, i32 0
  %vector126 = shufflevector <16 x i64> %temp125, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp15127 = add <16 x i64> %13, %vector126
  %extract128 = extractelement <16 x i64> %tmp15127, i32 0
  %extract129 = extractelement <16 x i64> %tmp15127, i32 1
  %extract130 = extractelement <16 x i64> %tmp15127, i32 2
  %extract131 = extractelement <16 x i64> %tmp15127, i32 3
  %extract132 = extractelement <16 x i64> %tmp15127, i32 4
  %extract133 = extractelement <16 x i64> %tmp15127, i32 5
  %extract134 = extractelement <16 x i64> %tmp15127, i32 6
  %extract135 = extractelement <16 x i64> %tmp15127, i32 7
  %extract136 = extractelement <16 x i64> %tmp15127, i32 8
  %extract137 = extractelement <16 x i64> %tmp15127, i32 9
  %extract138 = extractelement <16 x i64> %tmp15127, i32 10
  %extract139 = extractelement <16 x i64> %tmp15127, i32 11
  %extract140 = extractelement <16 x i64> %tmp15127, i32 12
  %extract141 = extractelement <16 x i64> %tmp15127, i32 13
  %extract142 = extractelement <16 x i64> %tmp15127, i32 14
  %extract143 = extractelement <16 x i64> %tmp15127, i32 15
  %104 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract128
  %105 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract129
  %106 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract130
  %107 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract131
  %108 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract132
  %109 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract133
  %110 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract134
  %111 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract135
  %112 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract136
  %113 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract137
  %114 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract138
  %115 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract139
  %116 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract140
  %117 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract141
  %118 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract142
  %119 = getelementptr i32 addrspace(1)* %puiOutputArray, i64 %extract143
  %120 = load <16 x i32> addrspace(1)* %ptrTypeCast, align 4
  %121 = load i32 addrspace(1)* %104, align 4
  %122 = load i32 addrspace(1)* %105, align 4
  %123 = load i32 addrspace(1)* %106, align 4
  %124 = load i32 addrspace(1)* %107, align 4
  %125 = load i32 addrspace(1)* %108, align 4
  %126 = load i32 addrspace(1)* %109, align 4
  %127 = load i32 addrspace(1)* %110, align 4
  %128 = load i32 addrspace(1)* %111, align 4
  %129 = load i32 addrspace(1)* %112, align 4
  %130 = load i32 addrspace(1)* %113, align 4
  %131 = load i32 addrspace(1)* %114, align 4
  %132 = load i32 addrspace(1)* %115, align 4
  %133 = load i32 addrspace(1)* %116, align 4
  %134 = load i32 addrspace(1)* %117, align 4
  %135 = load i32 addrspace(1)* %118, align 4
  %136 = load i32 addrspace(1)* %119, align 4
  %temp.vect144 = insertelement <16 x i32> undef, i32 %121, i32 0
  %temp.vect145 = insertelement <16 x i32> %temp.vect144, i32 %122, i32 1
  %temp.vect146 = insertelement <16 x i32> %temp.vect145, i32 %123, i32 2
  %temp.vect147 = insertelement <16 x i32> %temp.vect146, i32 %124, i32 3
  %temp.vect148 = insertelement <16 x i32> %temp.vect147, i32 %125, i32 4
  %temp.vect149 = insertelement <16 x i32> %temp.vect148, i32 %126, i32 5
  %temp.vect150 = insertelement <16 x i32> %temp.vect149, i32 %127, i32 6
  %temp.vect151 = insertelement <16 x i32> %temp.vect150, i32 %128, i32 7
  %temp.vect152 = insertelement <16 x i32> %temp.vect151, i32 %129, i32 8
  %temp.vect153 = insertelement <16 x i32> %temp.vect152, i32 %130, i32 9
  %temp.vect154 = insertelement <16 x i32> %temp.vect153, i32 %131, i32 10
  %temp.vect155 = insertelement <16 x i32> %temp.vect154, i32 %132, i32 11
  %temp.vect156 = insertelement <16 x i32> %temp.vect155, i32 %133, i32 12
  %temp.vect157 = insertelement <16 x i32> %temp.vect156, i32 %134, i32 13
  %temp.vect158 = insertelement <16 x i32> %temp.vect157, i32 %135, i32 14
  %temp.vect159 = insertelement <16 x i32> %temp.vect158, i32 %136, i32 15
  %137 = add <16 x i32> %temp.vect159, %120
  %extract160 = extractelement <16 x i32> %137, i32 0
  %extract161 = extractelement <16 x i32> %137, i32 1
  %extract162 = extractelement <16 x i32> %137, i32 2
  %extract163 = extractelement <16 x i32> %137, i32 3
  %extract164 = extractelement <16 x i32> %137, i32 4
  %extract165 = extractelement <16 x i32> %137, i32 5
  %extract166 = extractelement <16 x i32> %137, i32 6
  %extract167 = extractelement <16 x i32> %137, i32 7
  %extract168 = extractelement <16 x i32> %137, i32 8
  %extract169 = extractelement <16 x i32> %137, i32 9
  %extract170 = extractelement <16 x i32> %137, i32 10
  %extract171 = extractelement <16 x i32> %137, i32 11
  %extract172 = extractelement <16 x i32> %137, i32 12
  %extract173 = extractelement <16 x i32> %137, i32 13
  %extract174 = extractelement <16 x i32> %137, i32 14
  %extract175 = extractelement <16 x i32> %137, i32 15
  store i32 %extract160, i32 addrspace(1)* %104, align 4
  store i32 %extract161, i32 addrspace(1)* %105, align 4
  store i32 %extract162, i32 addrspace(1)* %106, align 4
  store i32 %extract163, i32 addrspace(1)* %107, align 4
  store i32 %extract164, i32 addrspace(1)* %108, align 4
  store i32 %extract165, i32 addrspace(1)* %109, align 4
  store i32 %extract166, i32 addrspace(1)* %110, align 4
  store i32 %extract167, i32 addrspace(1)* %111, align 4
  store i32 %extract168, i32 addrspace(1)* %112, align 4
  store i32 %extract169, i32 addrspace(1)* %113, align 4
  store i32 %extract170, i32 addrspace(1)* %114, align 4
  store i32 %extract171, i32 addrspace(1)* %115, align 4
  store i32 %extract172, i32 addrspace(1)* %116, align 4
  store i32 %extract173, i32 addrspace(1)* %117, align 4
  store i32 %extract174, i32 addrspace(1)* %118, align 4
  store i32 %extract175, i32 addrspace(1)* %119, align 4
  %138 = add i64 %i.06, 1
  %exitcond = icmp eq i64 %138, %0
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %.preheader
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB176

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB176:                                        ; preds = %._crit_edge
  ret void
}

define void @prefixSumStep2(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
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
  %17 = zext i32 %7 to i64
  %18 = lshr i32 %7, 1
  %19 = zext i32 %18 to i64
  %20 = add i32 %7, -1
  %21 = zext i32 %20 to i64
  %22 = icmp eq i32 %18, 0
  %23 = icmp eq i32 %7, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %24 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %25 = load i64* %24, align 8
  %26 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = add i64 %25, %27
  %29 = mul i64 %28, %17
  %30 = getelementptr inbounds i32 addrspace(1)* %4, i64 %28
  %.sum.i = add i64 %29, %21
  %31 = getelementptr inbounds i32 addrspace(1)* %1, i64 %.sum.i
  store i32 0, i32 addrspace(1)* %31, align 4
  br i1 %22, label %.preheader.i, label %bb.nph13.i

.preheader.i:                                     ; preds = %._crit_edge10.i, %SyncBB.i
  br i1 %23, label %._crit_edge.i, label %bb.nph.i

bb.nph13.i:                                       ; preds = %SyncBB.i
  %tmp20.i = add i64 %29, -1
  br label %32

; <label>:32                                      ; preds = %._crit_edge10.i, %bb.nph13.i
  %h.012.i = phi i64 [ 1, %bb.nph13.i ], [ %41, %._crit_edge10.i ]
  %offset.011.i = phi i64 [ %17, %bb.nph13.i ], [ %33, %._crit_edge10.i ]
  %33 = lshr i64 %offset.011.i, 1
  %34 = icmp eq i64 %h.012.i, 0
  br i1 %34, label %._crit_edge10.i, label %bb.nph9.i

bb.nph9.i:                                        ; preds = %32
  %35 = and i64 %offset.011.i, 9223372036854775806
  %tmp23.i = add i64 %tmp20.i, %35
  %tmp27.i = add i64 %tmp20.i, %33
  br label %36

; <label>:36                                      ; preds = %36, %bb.nph9.i
  %p.08.i = phi i64 [ 0, %bb.nph9.i ], [ %40, %36 ]
  %tmp17.i = mul i64 %35, %p.08.i
  %tmp24.i = add i64 %tmp23.i, %tmp17.i
  %scevgep25.i = getelementptr i32 addrspace(1)* %1, i64 %tmp24.i
  %tmp28.i = add i64 %tmp27.i, %tmp17.i
  %scevgep29.i = getelementptr i32 addrspace(1)* %1, i64 %tmp28.i
  %37 = load i32 addrspace(1)* %scevgep25.i, align 4
  %38 = load i32 addrspace(1)* %scevgep29.i, align 4
  %39 = add i32 %37, %38
  store i32 %39, i32 addrspace(1)* %scevgep25.i, align 4
  store i32 %37, i32 addrspace(1)* %scevgep29.i, align 4
  %40 = add i64 %p.08.i, 1
  %exitcond16.i = icmp eq i64 %40, %h.012.i
  br i1 %exitcond16.i, label %._crit_edge10.i, label %36

._crit_edge10.i:                                  ; preds = %36, %32
  %41 = shl i64 %h.012.i, 1
  %42 = icmp ugt i64 %41, %19
  br i1 %42, label %.preheader.i, label %32

bb.nph.i:                                         ; preds = %bb.nph.i, %.preheader.i
  %i.06.i = phi i64 [ %46, %bb.nph.i ], [ 0, %.preheader.i ]
  %tmp15.i = add i64 %29, %i.06.i
  %scevgep.i = getelementptr i32 addrspace(1)* %1, i64 %tmp15.i
  %43 = load i32 addrspace(1)* %30, align 4
  %44 = load i32 addrspace(1)* %scevgep.i, align 4
  %45 = add i32 %44, %43
  store i32 %45, i32 addrspace(1)* %scevgep.i, align 4
  %46 = add i64 %i.06.i, 1
  %exitcond.i = icmp eq i64 %46, %17
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %.preheader.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__prefixSumStep2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__prefixSumStep2_separated_args.exit:             ; preds = %._crit_edge.i
  ret void
}

define void @prefixSumStep1(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %20 = zext i32 %10 to i64
  %21 = icmp eq i32 %10, 0
  %22 = add i32 %10, -1
  %23 = zext i32 %22 to i64
  %24 = lshr i32 %10, 1
  %25 = icmp eq i32 %24, 0
  %26 = zext i32 %24 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %27 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = add i64 %28, %30
  %32 = mul i64 %31, %20
  br i1 %21, label %._crit_edge10.i, label %bb.nph12.i

bb.nph12.i:                                       ; preds = %bb.nph12.i, %SyncBB.i
  %i.011.i = phi i64 [ %34, %bb.nph12.i ], [ 0, %SyncBB.i ]
  %tmp27.i = add i64 %32, %i.011.i
  %scevgep28.i = getelementptr i32 addrspace(1)* %1, i64 %tmp27.i
  %scevgep29.i = getelementptr i32 addrspace(1)* %4, i64 %tmp27.i
  %33 = load i32 addrspace(1)* %scevgep28.i, align 4
  store i32 %33, i32 addrspace(1)* %scevgep29.i, align 4
  %34 = add i64 %i.011.i, 1
  %exitcond25.i = icmp eq i64 %34, %20
  br i1 %exitcond25.i, label %._crit_edge13.i, label %bb.nph12.i

._crit_edge13.i:                                  ; preds = %bb.nph12.i
  br i1 %25, label %._crit_edge10.i, label %bb.nph9.i

bb.nph9.i:                                        ; preds = %._crit_edge13.i
  %tmp17.i = add i64 %32, -1
  br label %35

; <label>:35                                      ; preds = %._crit_edge.i, %bb.nph9.i
  %h.08.i = phi i64 [ %26, %bb.nph9.i ], [ %42, %._crit_edge.i ]
  %offset.07.i = phi i64 [ 1, %bb.nph9.i ], [ %43, %._crit_edge.i ]
  %36 = icmp eq i64 %h.08.i, 0
  br i1 %36, label %._crit_edge10.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %35
  %tmp.i = shl i64 %offset.07.i, 1
  %tmp18.i = add i64 %tmp17.i, %offset.07.i
  %tmp21.i = add i64 %tmp17.i, %tmp.i
  br label %37

; <label>:37                                      ; preds = %37, %bb.nph.i
  %p.06.i = phi i64 [ 0, %bb.nph.i ], [ %41, %37 ]
  %tmp14.i = mul i64 %tmp.i, %p.06.i
  %tmp19.i = add i64 %tmp18.i, %tmp14.i
  %scevgep.i = getelementptr i32 addrspace(1)* %4, i64 %tmp19.i
  %tmp22.i = add i64 %tmp21.i, %tmp14.i
  %scevgep23.i = getelementptr i32 addrspace(1)* %4, i64 %tmp22.i
  %38 = load i32 addrspace(1)* %scevgep.i, align 4
  %39 = load i32 addrspace(1)* %scevgep23.i, align 4
  %40 = add i32 %39, %38
  store i32 %40, i32 addrspace(1)* %scevgep23.i, align 4
  %41 = add i64 %p.06.i, 1
  %exitcond.i = icmp eq i64 %41, %h.08.i
  br i1 %exitcond.i, label %._crit_edge.i, label %37

._crit_edge.i:                                    ; preds = %37
  %42 = lshr i64 %h.08.i, 1
  %43 = shl i64 %offset.07.i, 1
  %44 = icmp eq i64 %42, 0
  br i1 %44, label %._crit_edge10.i, label %35

._crit_edge10.i:                                  ; preds = %._crit_edge.i, %35, %._crit_edge13.i, %SyncBB.i
  %.sum.i = add i64 %32, %23
  %45 = getelementptr inbounds i32 addrspace(1)* %4, i64 %.sum.i
  %46 = load i32 addrspace(1)* %45, align 4
  %47 = getelementptr inbounds i32 addrspace(1)* %7, i64 %31
  store i32 %46, i32 addrspace(1)* %47, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %__prefixSumStep1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge10.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__prefixSumStep1_separated_args.exit:             ; preds = %._crit_edge10.i
  ret void
}

define void @__Vectorized_.prefixSumStep1(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %20 = zext i32 %10 to i64
  %temp.i = insertelement <16 x i64> undef, i64 %20, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %21 = icmp eq i32 %10, 0
  %22 = add i32 %10, -1
  %23 = zext i32 %22 to i64
  %temp110.i = insertelement <16 x i64> undef, i64 %23, i32 0
  %vector111.i = shufflevector <16 x i64> %temp110.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %24 = lshr i32 %10, 1
  %25 = icmp eq i32 %24, 0
  %26 = zext i32 %24 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %27 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = add i64 %28, %30
  %broadcast1.i = insertelement <16 x i64> undef, i64 %31, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %32 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %extract129.i = extractelement <16 x i64> %32, i32 0
  %33 = mul <16 x i64> %32, %vector.i
  br i1 %21, label %._crit_edge10.i, label %bb.nph12.i

bb.nph12.i:                                       ; preds = %bb.nph12.i, %SyncBB.i
  %i.011.i = phi i64 [ %82, %bb.nph12.i ], [ 0, %SyncBB.i ]
  %temp2.i = insertelement <16 x i64> undef, i64 %i.011.i, i32 0
  %vector3.i = shufflevector <16 x i64> %temp2.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp274.i = add <16 x i64> %33, %vector3.i
  %extract.i = extractelement <16 x i64> %tmp274.i, i32 0
  %extract5.i = extractelement <16 x i64> %tmp274.i, i32 1
  %extract6.i = extractelement <16 x i64> %tmp274.i, i32 2
  %extract7.i = extractelement <16 x i64> %tmp274.i, i32 3
  %extract8.i = extractelement <16 x i64> %tmp274.i, i32 4
  %extract9.i = extractelement <16 x i64> %tmp274.i, i32 5
  %extract10.i = extractelement <16 x i64> %tmp274.i, i32 6
  %extract11.i = extractelement <16 x i64> %tmp274.i, i32 7
  %extract12.i = extractelement <16 x i64> %tmp274.i, i32 8
  %extract13.i = extractelement <16 x i64> %tmp274.i, i32 9
  %extract14.i = extractelement <16 x i64> %tmp274.i, i32 10
  %extract15.i = extractelement <16 x i64> %tmp274.i, i32 11
  %extract16.i = extractelement <16 x i64> %tmp274.i, i32 12
  %extract17.i = extractelement <16 x i64> %tmp274.i, i32 13
  %extract18.i = extractelement <16 x i64> %tmp274.i, i32 14
  %extract19.i = extractelement <16 x i64> %tmp274.i, i32 15
  %34 = getelementptr i32 addrspace(1)* %1, i64 %extract.i
  %35 = getelementptr i32 addrspace(1)* %1, i64 %extract5.i
  %36 = getelementptr i32 addrspace(1)* %1, i64 %extract6.i
  %37 = getelementptr i32 addrspace(1)* %1, i64 %extract7.i
  %38 = getelementptr i32 addrspace(1)* %1, i64 %extract8.i
  %39 = getelementptr i32 addrspace(1)* %1, i64 %extract9.i
  %40 = getelementptr i32 addrspace(1)* %1, i64 %extract10.i
  %41 = getelementptr i32 addrspace(1)* %1, i64 %extract11.i
  %42 = getelementptr i32 addrspace(1)* %1, i64 %extract12.i
  %43 = getelementptr i32 addrspace(1)* %1, i64 %extract13.i
  %44 = getelementptr i32 addrspace(1)* %1, i64 %extract14.i
  %45 = getelementptr i32 addrspace(1)* %1, i64 %extract15.i
  %46 = getelementptr i32 addrspace(1)* %1, i64 %extract16.i
  %47 = getelementptr i32 addrspace(1)* %1, i64 %extract17.i
  %48 = getelementptr i32 addrspace(1)* %1, i64 %extract18.i
  %49 = getelementptr i32 addrspace(1)* %1, i64 %extract19.i
  %50 = getelementptr i32 addrspace(1)* %4, i64 %extract.i
  %51 = getelementptr i32 addrspace(1)* %4, i64 %extract5.i
  %52 = getelementptr i32 addrspace(1)* %4, i64 %extract6.i
  %53 = getelementptr i32 addrspace(1)* %4, i64 %extract7.i
  %54 = getelementptr i32 addrspace(1)* %4, i64 %extract8.i
  %55 = getelementptr i32 addrspace(1)* %4, i64 %extract9.i
  %56 = getelementptr i32 addrspace(1)* %4, i64 %extract10.i
  %57 = getelementptr i32 addrspace(1)* %4, i64 %extract11.i
  %58 = getelementptr i32 addrspace(1)* %4, i64 %extract12.i
  %59 = getelementptr i32 addrspace(1)* %4, i64 %extract13.i
  %60 = getelementptr i32 addrspace(1)* %4, i64 %extract14.i
  %61 = getelementptr i32 addrspace(1)* %4, i64 %extract15.i
  %62 = getelementptr i32 addrspace(1)* %4, i64 %extract16.i
  %63 = getelementptr i32 addrspace(1)* %4, i64 %extract17.i
  %64 = getelementptr i32 addrspace(1)* %4, i64 %extract18.i
  %65 = getelementptr i32 addrspace(1)* %4, i64 %extract19.i
  %66 = load i32 addrspace(1)* %34, align 4
  %67 = load i32 addrspace(1)* %35, align 4
  %68 = load i32 addrspace(1)* %36, align 4
  %69 = load i32 addrspace(1)* %37, align 4
  %70 = load i32 addrspace(1)* %38, align 4
  %71 = load i32 addrspace(1)* %39, align 4
  %72 = load i32 addrspace(1)* %40, align 4
  %73 = load i32 addrspace(1)* %41, align 4
  %74 = load i32 addrspace(1)* %42, align 4
  %75 = load i32 addrspace(1)* %43, align 4
  %76 = load i32 addrspace(1)* %44, align 4
  %77 = load i32 addrspace(1)* %45, align 4
  %78 = load i32 addrspace(1)* %46, align 4
  %79 = load i32 addrspace(1)* %47, align 4
  %80 = load i32 addrspace(1)* %48, align 4
  %81 = load i32 addrspace(1)* %49, align 4
  store i32 %66, i32 addrspace(1)* %50, align 4
  store i32 %67, i32 addrspace(1)* %51, align 4
  store i32 %68, i32 addrspace(1)* %52, align 4
  store i32 %69, i32 addrspace(1)* %53, align 4
  store i32 %70, i32 addrspace(1)* %54, align 4
  store i32 %71, i32 addrspace(1)* %55, align 4
  store i32 %72, i32 addrspace(1)* %56, align 4
  store i32 %73, i32 addrspace(1)* %57, align 4
  store i32 %74, i32 addrspace(1)* %58, align 4
  store i32 %75, i32 addrspace(1)* %59, align 4
  store i32 %76, i32 addrspace(1)* %60, align 4
  store i32 %77, i32 addrspace(1)* %61, align 4
  store i32 %78, i32 addrspace(1)* %62, align 4
  store i32 %79, i32 addrspace(1)* %63, align 4
  store i32 %80, i32 addrspace(1)* %64, align 4
  store i32 %81, i32 addrspace(1)* %65, align 4
  %82 = add i64 %i.011.i, 1
  %exitcond25.i = icmp eq i64 %82, %20
  br i1 %exitcond25.i, label %._crit_edge13.i, label %bb.nph12.i

._crit_edge13.i:                                  ; preds = %bb.nph12.i
  br i1 %25, label %._crit_edge10.i, label %bb.nph9.i

bb.nph9.i:                                        ; preds = %._crit_edge13.i
  %tmp1720.i = add <16 x i64> %33, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  br label %83

; <label>:83                                      ; preds = %._crit_edge.i, %bb.nph9.i
  %h.08.i = phi i64 [ %26, %bb.nph9.i ], [ %152, %._crit_edge.i ]
  %offset.07.i = phi i64 [ 1, %bb.nph9.i ], [ %153, %._crit_edge.i ]
  %84 = icmp eq i64 %h.08.i, 0
  br i1 %84, label %._crit_edge10.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %83
  %temp21.i = insertelement <16 x i64> undef, i64 %offset.07.i, i32 0
  %vector22.i = shufflevector <16 x i64> %temp21.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp.i = shl i64 %offset.07.i, 1
  %temp24.i = insertelement <16 x i64> undef, i64 %tmp.i, i32 0
  %vector25.i = shufflevector <16 x i64> %temp24.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1823.i = add <16 x i64> %tmp1720.i, %vector22.i
  %tmp2126.i = add <16 x i64> %tmp1720.i, %vector25.i
  br label %85

; <label>:85                                      ; preds = %85, %bb.nph.i
  %p.06.i = phi i64 [ 0, %bb.nph.i ], [ %151, %85 ]
  %tmp14.i = mul i64 %tmp.i, %p.06.i
  %temp27.i = insertelement <16 x i64> undef, i64 %tmp14.i, i32 0
  %vector28.i = shufflevector <16 x i64> %temp27.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1929.i = add <16 x i64> %tmp1823.i, %vector28.i
  %extract30.i = extractelement <16 x i64> %tmp1929.i, i32 0
  %extract31.i = extractelement <16 x i64> %tmp1929.i, i32 1
  %extract32.i = extractelement <16 x i64> %tmp1929.i, i32 2
  %extract33.i = extractelement <16 x i64> %tmp1929.i, i32 3
  %extract34.i = extractelement <16 x i64> %tmp1929.i, i32 4
  %extract35.i = extractelement <16 x i64> %tmp1929.i, i32 5
  %extract36.i = extractelement <16 x i64> %tmp1929.i, i32 6
  %extract37.i = extractelement <16 x i64> %tmp1929.i, i32 7
  %extract38.i = extractelement <16 x i64> %tmp1929.i, i32 8
  %extract39.i = extractelement <16 x i64> %tmp1929.i, i32 9
  %extract40.i = extractelement <16 x i64> %tmp1929.i, i32 10
  %extract41.i = extractelement <16 x i64> %tmp1929.i, i32 11
  %extract42.i = extractelement <16 x i64> %tmp1929.i, i32 12
  %extract43.i = extractelement <16 x i64> %tmp1929.i, i32 13
  %extract44.i = extractelement <16 x i64> %tmp1929.i, i32 14
  %extract45.i = extractelement <16 x i64> %tmp1929.i, i32 15
  %86 = getelementptr i32 addrspace(1)* %4, i64 %extract30.i
  %87 = getelementptr i32 addrspace(1)* %4, i64 %extract31.i
  %88 = getelementptr i32 addrspace(1)* %4, i64 %extract32.i
  %89 = getelementptr i32 addrspace(1)* %4, i64 %extract33.i
  %90 = getelementptr i32 addrspace(1)* %4, i64 %extract34.i
  %91 = getelementptr i32 addrspace(1)* %4, i64 %extract35.i
  %92 = getelementptr i32 addrspace(1)* %4, i64 %extract36.i
  %93 = getelementptr i32 addrspace(1)* %4, i64 %extract37.i
  %94 = getelementptr i32 addrspace(1)* %4, i64 %extract38.i
  %95 = getelementptr i32 addrspace(1)* %4, i64 %extract39.i
  %96 = getelementptr i32 addrspace(1)* %4, i64 %extract40.i
  %97 = getelementptr i32 addrspace(1)* %4, i64 %extract41.i
  %98 = getelementptr i32 addrspace(1)* %4, i64 %extract42.i
  %99 = getelementptr i32 addrspace(1)* %4, i64 %extract43.i
  %100 = getelementptr i32 addrspace(1)* %4, i64 %extract44.i
  %101 = getelementptr i32 addrspace(1)* %4, i64 %extract45.i
  %tmp2246.i = add <16 x i64> %tmp2126.i, %vector28.i
  %extract47.i = extractelement <16 x i64> %tmp2246.i, i32 0
  %extract48.i = extractelement <16 x i64> %tmp2246.i, i32 1
  %extract49.i = extractelement <16 x i64> %tmp2246.i, i32 2
  %extract50.i = extractelement <16 x i64> %tmp2246.i, i32 3
  %extract51.i = extractelement <16 x i64> %tmp2246.i, i32 4
  %extract52.i = extractelement <16 x i64> %tmp2246.i, i32 5
  %extract53.i = extractelement <16 x i64> %tmp2246.i, i32 6
  %extract54.i = extractelement <16 x i64> %tmp2246.i, i32 7
  %extract55.i = extractelement <16 x i64> %tmp2246.i, i32 8
  %extract56.i = extractelement <16 x i64> %tmp2246.i, i32 9
  %extract57.i = extractelement <16 x i64> %tmp2246.i, i32 10
  %extract58.i = extractelement <16 x i64> %tmp2246.i, i32 11
  %extract59.i = extractelement <16 x i64> %tmp2246.i, i32 12
  %extract60.i = extractelement <16 x i64> %tmp2246.i, i32 13
  %extract61.i = extractelement <16 x i64> %tmp2246.i, i32 14
  %extract62.i = extractelement <16 x i64> %tmp2246.i, i32 15
  %102 = getelementptr i32 addrspace(1)* %4, i64 %extract47.i
  %103 = getelementptr i32 addrspace(1)* %4, i64 %extract48.i
  %104 = getelementptr i32 addrspace(1)* %4, i64 %extract49.i
  %105 = getelementptr i32 addrspace(1)* %4, i64 %extract50.i
  %106 = getelementptr i32 addrspace(1)* %4, i64 %extract51.i
  %107 = getelementptr i32 addrspace(1)* %4, i64 %extract52.i
  %108 = getelementptr i32 addrspace(1)* %4, i64 %extract53.i
  %109 = getelementptr i32 addrspace(1)* %4, i64 %extract54.i
  %110 = getelementptr i32 addrspace(1)* %4, i64 %extract55.i
  %111 = getelementptr i32 addrspace(1)* %4, i64 %extract56.i
  %112 = getelementptr i32 addrspace(1)* %4, i64 %extract57.i
  %113 = getelementptr i32 addrspace(1)* %4, i64 %extract58.i
  %114 = getelementptr i32 addrspace(1)* %4, i64 %extract59.i
  %115 = getelementptr i32 addrspace(1)* %4, i64 %extract60.i
  %116 = getelementptr i32 addrspace(1)* %4, i64 %extract61.i
  %117 = getelementptr i32 addrspace(1)* %4, i64 %extract62.i
  %118 = load i32 addrspace(1)* %86, align 4
  %119 = load i32 addrspace(1)* %87, align 4
  %120 = load i32 addrspace(1)* %88, align 4
  %121 = load i32 addrspace(1)* %89, align 4
  %122 = load i32 addrspace(1)* %90, align 4
  %123 = load i32 addrspace(1)* %91, align 4
  %124 = load i32 addrspace(1)* %92, align 4
  %125 = load i32 addrspace(1)* %93, align 4
  %126 = load i32 addrspace(1)* %94, align 4
  %127 = load i32 addrspace(1)* %95, align 4
  %128 = load i32 addrspace(1)* %96, align 4
  %129 = load i32 addrspace(1)* %97, align 4
  %130 = load i32 addrspace(1)* %98, align 4
  %131 = load i32 addrspace(1)* %99, align 4
  %132 = load i32 addrspace(1)* %100, align 4
  %133 = load i32 addrspace(1)* %101, align 4
  %temp.vect78.i = insertelement <16 x i32> undef, i32 %118, i32 0
  %temp.vect79.i = insertelement <16 x i32> %temp.vect78.i, i32 %119, i32 1
  %temp.vect80.i = insertelement <16 x i32> %temp.vect79.i, i32 %120, i32 2
  %temp.vect81.i = insertelement <16 x i32> %temp.vect80.i, i32 %121, i32 3
  %temp.vect82.i = insertelement <16 x i32> %temp.vect81.i, i32 %122, i32 4
  %temp.vect83.i = insertelement <16 x i32> %temp.vect82.i, i32 %123, i32 5
  %temp.vect84.i = insertelement <16 x i32> %temp.vect83.i, i32 %124, i32 6
  %temp.vect85.i = insertelement <16 x i32> %temp.vect84.i, i32 %125, i32 7
  %temp.vect86.i = insertelement <16 x i32> %temp.vect85.i, i32 %126, i32 8
  %temp.vect87.i = insertelement <16 x i32> %temp.vect86.i, i32 %127, i32 9
  %temp.vect88.i = insertelement <16 x i32> %temp.vect87.i, i32 %128, i32 10
  %temp.vect89.i = insertelement <16 x i32> %temp.vect88.i, i32 %129, i32 11
  %temp.vect90.i = insertelement <16 x i32> %temp.vect89.i, i32 %130, i32 12
  %temp.vect91.i = insertelement <16 x i32> %temp.vect90.i, i32 %131, i32 13
  %temp.vect92.i = insertelement <16 x i32> %temp.vect91.i, i32 %132, i32 14
  %temp.vect93.i = insertelement <16 x i32> %temp.vect92.i, i32 %133, i32 15
  %134 = load i32 addrspace(1)* %102, align 4
  %135 = load i32 addrspace(1)* %103, align 4
  %136 = load i32 addrspace(1)* %104, align 4
  %137 = load i32 addrspace(1)* %105, align 4
  %138 = load i32 addrspace(1)* %106, align 4
  %139 = load i32 addrspace(1)* %107, align 4
  %140 = load i32 addrspace(1)* %108, align 4
  %141 = load i32 addrspace(1)* %109, align 4
  %142 = load i32 addrspace(1)* %110, align 4
  %143 = load i32 addrspace(1)* %111, align 4
  %144 = load i32 addrspace(1)* %112, align 4
  %145 = load i32 addrspace(1)* %113, align 4
  %146 = load i32 addrspace(1)* %114, align 4
  %147 = load i32 addrspace(1)* %115, align 4
  %148 = load i32 addrspace(1)* %116, align 4
  %149 = load i32 addrspace(1)* %117, align 4
  %temp.vect.i = insertelement <16 x i32> undef, i32 %134, i32 0
  %temp.vect63.i = insertelement <16 x i32> %temp.vect.i, i32 %135, i32 1
  %temp.vect64.i = insertelement <16 x i32> %temp.vect63.i, i32 %136, i32 2
  %temp.vect65.i = insertelement <16 x i32> %temp.vect64.i, i32 %137, i32 3
  %temp.vect66.i = insertelement <16 x i32> %temp.vect65.i, i32 %138, i32 4
  %temp.vect67.i = insertelement <16 x i32> %temp.vect66.i, i32 %139, i32 5
  %temp.vect68.i = insertelement <16 x i32> %temp.vect67.i, i32 %140, i32 6
  %temp.vect69.i = insertelement <16 x i32> %temp.vect68.i, i32 %141, i32 7
  %temp.vect70.i = insertelement <16 x i32> %temp.vect69.i, i32 %142, i32 8
  %temp.vect71.i = insertelement <16 x i32> %temp.vect70.i, i32 %143, i32 9
  %temp.vect72.i = insertelement <16 x i32> %temp.vect71.i, i32 %144, i32 10
  %temp.vect73.i = insertelement <16 x i32> %temp.vect72.i, i32 %145, i32 11
  %temp.vect74.i = insertelement <16 x i32> %temp.vect73.i, i32 %146, i32 12
  %temp.vect75.i = insertelement <16 x i32> %temp.vect74.i, i32 %147, i32 13
  %temp.vect76.i = insertelement <16 x i32> %temp.vect75.i, i32 %148, i32 14
  %temp.vect77.i = insertelement <16 x i32> %temp.vect76.i, i32 %149, i32 15
  %150 = add <16 x i32> %temp.vect77.i, %temp.vect93.i
  %extract94.i = extractelement <16 x i32> %150, i32 0
  %extract95.i = extractelement <16 x i32> %150, i32 1
  %extract96.i = extractelement <16 x i32> %150, i32 2
  %extract97.i = extractelement <16 x i32> %150, i32 3
  %extract98.i = extractelement <16 x i32> %150, i32 4
  %extract99.i = extractelement <16 x i32> %150, i32 5
  %extract100.i = extractelement <16 x i32> %150, i32 6
  %extract101.i = extractelement <16 x i32> %150, i32 7
  %extract102.i = extractelement <16 x i32> %150, i32 8
  %extract103.i = extractelement <16 x i32> %150, i32 9
  %extract104.i = extractelement <16 x i32> %150, i32 10
  %extract105.i = extractelement <16 x i32> %150, i32 11
  %extract106.i = extractelement <16 x i32> %150, i32 12
  %extract107.i = extractelement <16 x i32> %150, i32 13
  %extract108.i = extractelement <16 x i32> %150, i32 14
  %extract109.i = extractelement <16 x i32> %150, i32 15
  store i32 %extract94.i, i32 addrspace(1)* %102, align 4
  store i32 %extract95.i, i32 addrspace(1)* %103, align 4
  store i32 %extract96.i, i32 addrspace(1)* %104, align 4
  store i32 %extract97.i, i32 addrspace(1)* %105, align 4
  store i32 %extract98.i, i32 addrspace(1)* %106, align 4
  store i32 %extract99.i, i32 addrspace(1)* %107, align 4
  store i32 %extract100.i, i32 addrspace(1)* %108, align 4
  store i32 %extract101.i, i32 addrspace(1)* %109, align 4
  store i32 %extract102.i, i32 addrspace(1)* %110, align 4
  store i32 %extract103.i, i32 addrspace(1)* %111, align 4
  store i32 %extract104.i, i32 addrspace(1)* %112, align 4
  store i32 %extract105.i, i32 addrspace(1)* %113, align 4
  store i32 %extract106.i, i32 addrspace(1)* %114, align 4
  store i32 %extract107.i, i32 addrspace(1)* %115, align 4
  store i32 %extract108.i, i32 addrspace(1)* %116, align 4
  store i32 %extract109.i, i32 addrspace(1)* %117, align 4
  %151 = add i64 %p.06.i, 1
  %exitcond.i = icmp eq i64 %151, %h.08.i
  br i1 %exitcond.i, label %._crit_edge.i, label %85

._crit_edge.i:                                    ; preds = %85
  %152 = lshr i64 %h.08.i, 1
  %153 = shl i64 %offset.07.i, 1
  %154 = icmp eq i64 %152, 0
  br i1 %154, label %._crit_edge10.i, label %83

._crit_edge10.i:                                  ; preds = %._crit_edge.i, %83, %._crit_edge13.i, %SyncBB.i
  %.sum112.i = add <16 x i64> %33, %vector111.i
  %extract113.i = extractelement <16 x i64> %.sum112.i, i32 0
  %extract114.i = extractelement <16 x i64> %.sum112.i, i32 1
  %extract115.i = extractelement <16 x i64> %.sum112.i, i32 2
  %extract116.i = extractelement <16 x i64> %.sum112.i, i32 3
  %extract117.i = extractelement <16 x i64> %.sum112.i, i32 4
  %extract118.i = extractelement <16 x i64> %.sum112.i, i32 5
  %extract119.i = extractelement <16 x i64> %.sum112.i, i32 6
  %extract120.i = extractelement <16 x i64> %.sum112.i, i32 7
  %extract121.i = extractelement <16 x i64> %.sum112.i, i32 8
  %extract122.i = extractelement <16 x i64> %.sum112.i, i32 9
  %extract123.i = extractelement <16 x i64> %.sum112.i, i32 10
  %extract124.i = extractelement <16 x i64> %.sum112.i, i32 11
  %extract125.i = extractelement <16 x i64> %.sum112.i, i32 12
  %extract126.i = extractelement <16 x i64> %.sum112.i, i32 13
  %extract127.i = extractelement <16 x i64> %.sum112.i, i32 14
  %extract128.i = extractelement <16 x i64> %.sum112.i, i32 15
  %155 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract113.i
  %156 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract114.i
  %157 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract115.i
  %158 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract116.i
  %159 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract117.i
  %160 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract118.i
  %161 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract119.i
  %162 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract120.i
  %163 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract121.i
  %164 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract122.i
  %165 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract123.i
  %166 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract124.i
  %167 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract125.i
  %168 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract126.i
  %169 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract127.i
  %170 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract128.i
  %171 = load i32 addrspace(1)* %155, align 4
  %172 = load i32 addrspace(1)* %156, align 4
  %173 = load i32 addrspace(1)* %157, align 4
  %174 = load i32 addrspace(1)* %158, align 4
  %175 = load i32 addrspace(1)* %159, align 4
  %176 = load i32 addrspace(1)* %160, align 4
  %177 = load i32 addrspace(1)* %161, align 4
  %178 = load i32 addrspace(1)* %162, align 4
  %179 = load i32 addrspace(1)* %163, align 4
  %180 = load i32 addrspace(1)* %164, align 4
  %181 = load i32 addrspace(1)* %165, align 4
  %182 = load i32 addrspace(1)* %166, align 4
  %183 = load i32 addrspace(1)* %167, align 4
  %184 = load i32 addrspace(1)* %168, align 4
  %185 = load i32 addrspace(1)* %169, align 4
  %186 = load i32 addrspace(1)* %170, align 4
  %temp.vect145.i = insertelement <16 x i32> undef, i32 %171, i32 0
  %temp.vect146.i = insertelement <16 x i32> %temp.vect145.i, i32 %172, i32 1
  %temp.vect147.i = insertelement <16 x i32> %temp.vect146.i, i32 %173, i32 2
  %temp.vect148.i = insertelement <16 x i32> %temp.vect147.i, i32 %174, i32 3
  %temp.vect149.i = insertelement <16 x i32> %temp.vect148.i, i32 %175, i32 4
  %temp.vect150.i = insertelement <16 x i32> %temp.vect149.i, i32 %176, i32 5
  %temp.vect151.i = insertelement <16 x i32> %temp.vect150.i, i32 %177, i32 6
  %temp.vect152.i = insertelement <16 x i32> %temp.vect151.i, i32 %178, i32 7
  %temp.vect153.i = insertelement <16 x i32> %temp.vect152.i, i32 %179, i32 8
  %temp.vect154.i = insertelement <16 x i32> %temp.vect153.i, i32 %180, i32 9
  %temp.vect155.i = insertelement <16 x i32> %temp.vect154.i, i32 %181, i32 10
  %temp.vect156.i = insertelement <16 x i32> %temp.vect155.i, i32 %182, i32 11
  %temp.vect157.i = insertelement <16 x i32> %temp.vect156.i, i32 %183, i32 12
  %temp.vect158.i = insertelement <16 x i32> %temp.vect157.i, i32 %184, i32 13
  %temp.vect159.i = insertelement <16 x i32> %temp.vect158.i, i32 %185, i32 14
  %temp.vect160.i = insertelement <16 x i32> %temp.vect159.i, i32 %186, i32 15
  %187 = getelementptr inbounds i32 addrspace(1)* %7, i64 %extract129.i
  %ptrTypeCast.i = bitcast i32 addrspace(1)* %187 to <16 x i32> addrspace(1)*
  store <16 x i32> %temp.vect160.i, <16 x i32> addrspace(1)* %ptrTypeCast.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.prefixSumStep1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge10.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.prefixSumStep1_separated_args.exit: ; preds = %._crit_edge10.i
  ret void
}

define void @__Vectorized_.prefixSumStep2(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
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
  %17 = zext i32 %7 to i64
  %temp.i = insertelement <16 x i64> undef, i64 %17, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %18 = lshr i32 %7, 1
  %19 = zext i32 %18 to i64
  %20 = add i32 %7, -1
  %21 = zext i32 %20 to i64
  %temp16.i = insertelement <16 x i64> undef, i64 %21, i32 0
  %vector17.i = shufflevector <16 x i64> %temp16.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %22 = icmp eq i32 %18, 0
  %23 = icmp eq i32 %7, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %24 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %25 = load i64* %24, align 8
  %26 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = add i64 %25, %27
  %broadcast1.i = insertelement <16 x i64> undef, i64 %28, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %29 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %extract.i = extractelement <16 x i64> %29, i32 0
  %30 = mul <16 x i64> %29, %vector.i
  %31 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract.i
  %.sum18.i = add <16 x i64> %30, %vector17.i
  %extract19.i = extractelement <16 x i64> %.sum18.i, i32 0
  %extract20.i = extractelement <16 x i64> %.sum18.i, i32 1
  %extract21.i = extractelement <16 x i64> %.sum18.i, i32 2
  %extract22.i = extractelement <16 x i64> %.sum18.i, i32 3
  %extract23.i = extractelement <16 x i64> %.sum18.i, i32 4
  %extract24.i = extractelement <16 x i64> %.sum18.i, i32 5
  %extract25.i = extractelement <16 x i64> %.sum18.i, i32 6
  %extract26.i = extractelement <16 x i64> %.sum18.i, i32 7
  %extract27.i = extractelement <16 x i64> %.sum18.i, i32 8
  %extract28.i = extractelement <16 x i64> %.sum18.i, i32 9
  %extract29.i = extractelement <16 x i64> %.sum18.i, i32 10
  %extract30.i = extractelement <16 x i64> %.sum18.i, i32 11
  %extract31.i = extractelement <16 x i64> %.sum18.i, i32 12
  %extract32.i = extractelement <16 x i64> %.sum18.i, i32 13
  %extract33.i = extractelement <16 x i64> %.sum18.i, i32 14
  %extract34.i = extractelement <16 x i64> %.sum18.i, i32 15
  %32 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract19.i
  %33 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract20.i
  %34 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract21.i
  %35 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract22.i
  %36 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract23.i
  %37 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract24.i
  %38 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract25.i
  %39 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract26.i
  %40 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract27.i
  %41 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract28.i
  %42 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract29.i
  %43 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract30.i
  %44 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract31.i
  %45 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract32.i
  %46 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract33.i
  %47 = getelementptr inbounds i32 addrspace(1)* %1, i64 %extract34.i
  store i32 0, i32 addrspace(1)* %32, align 4
  store i32 0, i32 addrspace(1)* %33, align 4
  store i32 0, i32 addrspace(1)* %34, align 4
  store i32 0, i32 addrspace(1)* %35, align 4
  store i32 0, i32 addrspace(1)* %36, align 4
  store i32 0, i32 addrspace(1)* %37, align 4
  store i32 0, i32 addrspace(1)* %38, align 4
  store i32 0, i32 addrspace(1)* %39, align 4
  store i32 0, i32 addrspace(1)* %40, align 4
  store i32 0, i32 addrspace(1)* %41, align 4
  store i32 0, i32 addrspace(1)* %42, align 4
  store i32 0, i32 addrspace(1)* %43, align 4
  store i32 0, i32 addrspace(1)* %44, align 4
  store i32 0, i32 addrspace(1)* %45, align 4
  store i32 0, i32 addrspace(1)* %46, align 4
  store i32 0, i32 addrspace(1)* %47, align 4
  br i1 %22, label %.preheader.i, label %bb.nph13.i

.preheader.i:                                     ; preds = %._crit_edge10.i, %SyncBB.i
  br i1 %23, label %._crit_edge.i, label %bb.nph.preheader.i

bb.nph.preheader.i:                               ; preds = %.preheader.i
  %ptrTypeCast.i = bitcast i32 addrspace(1)* %31 to <16 x i32> addrspace(1)*
  br label %bb.nph.i

bb.nph13.i:                                       ; preds = %SyncBB.i
  %tmp2035.i = add <16 x i64> %30, <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1>
  br label %48

; <label>:48                                      ; preds = %._crit_edge10.i, %bb.nph13.i
  %h.012.i = phi i64 [ 1, %bb.nph13.i ], [ %119, %._crit_edge10.i ]
  %offset.011.i = phi i64 [ %17, %bb.nph13.i ], [ %49, %._crit_edge10.i ]
  %49 = lshr i64 %offset.011.i, 1
  %50 = icmp eq i64 %h.012.i, 0
  br i1 %50, label %._crit_edge10.i, label %bb.nph9.i

bb.nph9.i:                                        ; preds = %48
  %temp39.i = insertelement <16 x i64> undef, i64 %49, i32 0
  %vector40.i = shufflevector <16 x i64> %temp39.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %51 = and i64 %offset.011.i, 9223372036854775806
  %temp36.i = insertelement <16 x i64> undef, i64 %51, i32 0
  %vector37.i = shufflevector <16 x i64> %temp36.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp2338.i = add <16 x i64> %tmp2035.i, %vector37.i
  %tmp2741.i = add <16 x i64> %tmp2035.i, %vector40.i
  br label %52

; <label>:52                                      ; preds = %52, %bb.nph9.i
  %p.08.i = phi i64 [ 0, %bb.nph9.i ], [ %118, %52 ]
  %tmp17.i = mul i64 %51, %p.08.i
  %temp42.i = insertelement <16 x i64> undef, i64 %tmp17.i, i32 0
  %vector43.i = shufflevector <16 x i64> %temp42.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp2444.i = add <16 x i64> %tmp2338.i, %vector43.i
  %extract45.i = extractelement <16 x i64> %tmp2444.i, i32 0
  %extract46.i = extractelement <16 x i64> %tmp2444.i, i32 1
  %extract47.i = extractelement <16 x i64> %tmp2444.i, i32 2
  %extract48.i = extractelement <16 x i64> %tmp2444.i, i32 3
  %extract49.i = extractelement <16 x i64> %tmp2444.i, i32 4
  %extract50.i = extractelement <16 x i64> %tmp2444.i, i32 5
  %extract51.i = extractelement <16 x i64> %tmp2444.i, i32 6
  %extract52.i = extractelement <16 x i64> %tmp2444.i, i32 7
  %extract53.i = extractelement <16 x i64> %tmp2444.i, i32 8
  %extract54.i = extractelement <16 x i64> %tmp2444.i, i32 9
  %extract55.i = extractelement <16 x i64> %tmp2444.i, i32 10
  %extract56.i = extractelement <16 x i64> %tmp2444.i, i32 11
  %extract57.i = extractelement <16 x i64> %tmp2444.i, i32 12
  %extract58.i = extractelement <16 x i64> %tmp2444.i, i32 13
  %extract59.i = extractelement <16 x i64> %tmp2444.i, i32 14
  %extract60.i = extractelement <16 x i64> %tmp2444.i, i32 15
  %53 = getelementptr i32 addrspace(1)* %1, i64 %extract45.i
  %54 = getelementptr i32 addrspace(1)* %1, i64 %extract46.i
  %55 = getelementptr i32 addrspace(1)* %1, i64 %extract47.i
  %56 = getelementptr i32 addrspace(1)* %1, i64 %extract48.i
  %57 = getelementptr i32 addrspace(1)* %1, i64 %extract49.i
  %58 = getelementptr i32 addrspace(1)* %1, i64 %extract50.i
  %59 = getelementptr i32 addrspace(1)* %1, i64 %extract51.i
  %60 = getelementptr i32 addrspace(1)* %1, i64 %extract52.i
  %61 = getelementptr i32 addrspace(1)* %1, i64 %extract53.i
  %62 = getelementptr i32 addrspace(1)* %1, i64 %extract54.i
  %63 = getelementptr i32 addrspace(1)* %1, i64 %extract55.i
  %64 = getelementptr i32 addrspace(1)* %1, i64 %extract56.i
  %65 = getelementptr i32 addrspace(1)* %1, i64 %extract57.i
  %66 = getelementptr i32 addrspace(1)* %1, i64 %extract58.i
  %67 = getelementptr i32 addrspace(1)* %1, i64 %extract59.i
  %68 = getelementptr i32 addrspace(1)* %1, i64 %extract60.i
  %tmp2861.i = add <16 x i64> %tmp2741.i, %vector43.i
  %extract62.i = extractelement <16 x i64> %tmp2861.i, i32 0
  %extract63.i = extractelement <16 x i64> %tmp2861.i, i32 1
  %extract64.i = extractelement <16 x i64> %tmp2861.i, i32 2
  %extract65.i = extractelement <16 x i64> %tmp2861.i, i32 3
  %extract66.i = extractelement <16 x i64> %tmp2861.i, i32 4
  %extract67.i = extractelement <16 x i64> %tmp2861.i, i32 5
  %extract68.i = extractelement <16 x i64> %tmp2861.i, i32 6
  %extract69.i = extractelement <16 x i64> %tmp2861.i, i32 7
  %extract70.i = extractelement <16 x i64> %tmp2861.i, i32 8
  %extract71.i = extractelement <16 x i64> %tmp2861.i, i32 9
  %extract72.i = extractelement <16 x i64> %tmp2861.i, i32 10
  %extract73.i = extractelement <16 x i64> %tmp2861.i, i32 11
  %extract74.i = extractelement <16 x i64> %tmp2861.i, i32 12
  %extract75.i = extractelement <16 x i64> %tmp2861.i, i32 13
  %extract76.i = extractelement <16 x i64> %tmp2861.i, i32 14
  %extract77.i = extractelement <16 x i64> %tmp2861.i, i32 15
  %69 = getelementptr i32 addrspace(1)* %1, i64 %extract62.i
  %70 = getelementptr i32 addrspace(1)* %1, i64 %extract63.i
  %71 = getelementptr i32 addrspace(1)* %1, i64 %extract64.i
  %72 = getelementptr i32 addrspace(1)* %1, i64 %extract65.i
  %73 = getelementptr i32 addrspace(1)* %1, i64 %extract66.i
  %74 = getelementptr i32 addrspace(1)* %1, i64 %extract67.i
  %75 = getelementptr i32 addrspace(1)* %1, i64 %extract68.i
  %76 = getelementptr i32 addrspace(1)* %1, i64 %extract69.i
  %77 = getelementptr i32 addrspace(1)* %1, i64 %extract70.i
  %78 = getelementptr i32 addrspace(1)* %1, i64 %extract71.i
  %79 = getelementptr i32 addrspace(1)* %1, i64 %extract72.i
  %80 = getelementptr i32 addrspace(1)* %1, i64 %extract73.i
  %81 = getelementptr i32 addrspace(1)* %1, i64 %extract74.i
  %82 = getelementptr i32 addrspace(1)* %1, i64 %extract75.i
  %83 = getelementptr i32 addrspace(1)* %1, i64 %extract76.i
  %84 = getelementptr i32 addrspace(1)* %1, i64 %extract77.i
  %85 = load i32 addrspace(1)* %53, align 4
  %86 = load i32 addrspace(1)* %54, align 4
  %87 = load i32 addrspace(1)* %55, align 4
  %88 = load i32 addrspace(1)* %56, align 4
  %89 = load i32 addrspace(1)* %57, align 4
  %90 = load i32 addrspace(1)* %58, align 4
  %91 = load i32 addrspace(1)* %59, align 4
  %92 = load i32 addrspace(1)* %60, align 4
  %93 = load i32 addrspace(1)* %61, align 4
  %94 = load i32 addrspace(1)* %62, align 4
  %95 = load i32 addrspace(1)* %63, align 4
  %96 = load i32 addrspace(1)* %64, align 4
  %97 = load i32 addrspace(1)* %65, align 4
  %98 = load i32 addrspace(1)* %66, align 4
  %99 = load i32 addrspace(1)* %67, align 4
  %100 = load i32 addrspace(1)* %68, align 4
  %temp.vect.i = insertelement <16 x i32> undef, i32 %85, i32 0
  %temp.vect78.i = insertelement <16 x i32> %temp.vect.i, i32 %86, i32 1
  %temp.vect79.i = insertelement <16 x i32> %temp.vect78.i, i32 %87, i32 2
  %temp.vect80.i = insertelement <16 x i32> %temp.vect79.i, i32 %88, i32 3
  %temp.vect81.i = insertelement <16 x i32> %temp.vect80.i, i32 %89, i32 4
  %temp.vect82.i = insertelement <16 x i32> %temp.vect81.i, i32 %90, i32 5
  %temp.vect83.i = insertelement <16 x i32> %temp.vect82.i, i32 %91, i32 6
  %temp.vect84.i = insertelement <16 x i32> %temp.vect83.i, i32 %92, i32 7
  %temp.vect85.i = insertelement <16 x i32> %temp.vect84.i, i32 %93, i32 8
  %temp.vect86.i = insertelement <16 x i32> %temp.vect85.i, i32 %94, i32 9
  %temp.vect87.i = insertelement <16 x i32> %temp.vect86.i, i32 %95, i32 10
  %temp.vect88.i = insertelement <16 x i32> %temp.vect87.i, i32 %96, i32 11
  %temp.vect89.i = insertelement <16 x i32> %temp.vect88.i, i32 %97, i32 12
  %temp.vect90.i = insertelement <16 x i32> %temp.vect89.i, i32 %98, i32 13
  %temp.vect91.i = insertelement <16 x i32> %temp.vect90.i, i32 %99, i32 14
  %temp.vect92.i = insertelement <16 x i32> %temp.vect91.i, i32 %100, i32 15
  %101 = load i32 addrspace(1)* %69, align 4
  %102 = load i32 addrspace(1)* %70, align 4
  %103 = load i32 addrspace(1)* %71, align 4
  %104 = load i32 addrspace(1)* %72, align 4
  %105 = load i32 addrspace(1)* %73, align 4
  %106 = load i32 addrspace(1)* %74, align 4
  %107 = load i32 addrspace(1)* %75, align 4
  %108 = load i32 addrspace(1)* %76, align 4
  %109 = load i32 addrspace(1)* %77, align 4
  %110 = load i32 addrspace(1)* %78, align 4
  %111 = load i32 addrspace(1)* %79, align 4
  %112 = load i32 addrspace(1)* %80, align 4
  %113 = load i32 addrspace(1)* %81, align 4
  %114 = load i32 addrspace(1)* %82, align 4
  %115 = load i32 addrspace(1)* %83, align 4
  %116 = load i32 addrspace(1)* %84, align 4
  %temp.vect93.i = insertelement <16 x i32> undef, i32 %101, i32 0
  %temp.vect94.i = insertelement <16 x i32> %temp.vect93.i, i32 %102, i32 1
  %temp.vect95.i = insertelement <16 x i32> %temp.vect94.i, i32 %103, i32 2
  %temp.vect96.i = insertelement <16 x i32> %temp.vect95.i, i32 %104, i32 3
  %temp.vect97.i = insertelement <16 x i32> %temp.vect96.i, i32 %105, i32 4
  %temp.vect98.i = insertelement <16 x i32> %temp.vect97.i, i32 %106, i32 5
  %temp.vect99.i = insertelement <16 x i32> %temp.vect98.i, i32 %107, i32 6
  %temp.vect100.i = insertelement <16 x i32> %temp.vect99.i, i32 %108, i32 7
  %temp.vect101.i = insertelement <16 x i32> %temp.vect100.i, i32 %109, i32 8
  %temp.vect102.i = insertelement <16 x i32> %temp.vect101.i, i32 %110, i32 9
  %temp.vect103.i = insertelement <16 x i32> %temp.vect102.i, i32 %111, i32 10
  %temp.vect104.i = insertelement <16 x i32> %temp.vect103.i, i32 %112, i32 11
  %temp.vect105.i = insertelement <16 x i32> %temp.vect104.i, i32 %113, i32 12
  %temp.vect106.i = insertelement <16 x i32> %temp.vect105.i, i32 %114, i32 13
  %temp.vect107.i = insertelement <16 x i32> %temp.vect106.i, i32 %115, i32 14
  %temp.vect108.i = insertelement <16 x i32> %temp.vect107.i, i32 %116, i32 15
  %117 = add <16 x i32> %temp.vect92.i, %temp.vect108.i
  %extract109.i = extractelement <16 x i32> %117, i32 0
  %extract110.i = extractelement <16 x i32> %117, i32 1
  %extract111.i = extractelement <16 x i32> %117, i32 2
  %extract112.i = extractelement <16 x i32> %117, i32 3
  %extract113.i = extractelement <16 x i32> %117, i32 4
  %extract114.i = extractelement <16 x i32> %117, i32 5
  %extract115.i = extractelement <16 x i32> %117, i32 6
  %extract116.i = extractelement <16 x i32> %117, i32 7
  %extract117.i = extractelement <16 x i32> %117, i32 8
  %extract118.i = extractelement <16 x i32> %117, i32 9
  %extract119.i = extractelement <16 x i32> %117, i32 10
  %extract120.i = extractelement <16 x i32> %117, i32 11
  %extract121.i = extractelement <16 x i32> %117, i32 12
  %extract122.i = extractelement <16 x i32> %117, i32 13
  %extract123.i = extractelement <16 x i32> %117, i32 14
  %extract124.i = extractelement <16 x i32> %117, i32 15
  store i32 %extract109.i, i32 addrspace(1)* %53, align 4
  store i32 %extract110.i, i32 addrspace(1)* %54, align 4
  store i32 %extract111.i, i32 addrspace(1)* %55, align 4
  store i32 %extract112.i, i32 addrspace(1)* %56, align 4
  store i32 %extract113.i, i32 addrspace(1)* %57, align 4
  store i32 %extract114.i, i32 addrspace(1)* %58, align 4
  store i32 %extract115.i, i32 addrspace(1)* %59, align 4
  store i32 %extract116.i, i32 addrspace(1)* %60, align 4
  store i32 %extract117.i, i32 addrspace(1)* %61, align 4
  store i32 %extract118.i, i32 addrspace(1)* %62, align 4
  store i32 %extract119.i, i32 addrspace(1)* %63, align 4
  store i32 %extract120.i, i32 addrspace(1)* %64, align 4
  store i32 %extract121.i, i32 addrspace(1)* %65, align 4
  store i32 %extract122.i, i32 addrspace(1)* %66, align 4
  store i32 %extract123.i, i32 addrspace(1)* %67, align 4
  store i32 %extract124.i, i32 addrspace(1)* %68, align 4
  store i32 %85, i32 addrspace(1)* %69, align 4
  store i32 %86, i32 addrspace(1)* %70, align 4
  store i32 %87, i32 addrspace(1)* %71, align 4
  store i32 %88, i32 addrspace(1)* %72, align 4
  store i32 %89, i32 addrspace(1)* %73, align 4
  store i32 %90, i32 addrspace(1)* %74, align 4
  store i32 %91, i32 addrspace(1)* %75, align 4
  store i32 %92, i32 addrspace(1)* %76, align 4
  store i32 %93, i32 addrspace(1)* %77, align 4
  store i32 %94, i32 addrspace(1)* %78, align 4
  store i32 %95, i32 addrspace(1)* %79, align 4
  store i32 %96, i32 addrspace(1)* %80, align 4
  store i32 %97, i32 addrspace(1)* %81, align 4
  store i32 %98, i32 addrspace(1)* %82, align 4
  store i32 %99, i32 addrspace(1)* %83, align 4
  store i32 %100, i32 addrspace(1)* %84, align 4
  %118 = add i64 %p.08.i, 1
  %exitcond16.i = icmp eq i64 %118, %h.012.i
  br i1 %exitcond16.i, label %._crit_edge10.i, label %52

._crit_edge10.i:                                  ; preds = %52, %48
  %119 = shl i64 %h.012.i, 1
  %120 = icmp ugt i64 %119, %19
  br i1 %120, label %.preheader.i, label %48

bb.nph.i:                                         ; preds = %bb.nph.i, %bb.nph.preheader.i
  %i.06.i = phi i64 [ %155, %bb.nph.i ], [ 0, %bb.nph.preheader.i ]
  %temp125.i = insertelement <16 x i64> undef, i64 %i.06.i, i32 0
  %vector126.i = shufflevector <16 x i64> %temp125.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp15127.i = add <16 x i64> %30, %vector126.i
  %extract128.i = extractelement <16 x i64> %tmp15127.i, i32 0
  %extract129.i = extractelement <16 x i64> %tmp15127.i, i32 1
  %extract130.i = extractelement <16 x i64> %tmp15127.i, i32 2
  %extract131.i = extractelement <16 x i64> %tmp15127.i, i32 3
  %extract132.i = extractelement <16 x i64> %tmp15127.i, i32 4
  %extract133.i = extractelement <16 x i64> %tmp15127.i, i32 5
  %extract134.i = extractelement <16 x i64> %tmp15127.i, i32 6
  %extract135.i = extractelement <16 x i64> %tmp15127.i, i32 7
  %extract136.i = extractelement <16 x i64> %tmp15127.i, i32 8
  %extract137.i = extractelement <16 x i64> %tmp15127.i, i32 9
  %extract138.i = extractelement <16 x i64> %tmp15127.i, i32 10
  %extract139.i = extractelement <16 x i64> %tmp15127.i, i32 11
  %extract140.i = extractelement <16 x i64> %tmp15127.i, i32 12
  %extract141.i = extractelement <16 x i64> %tmp15127.i, i32 13
  %extract142.i = extractelement <16 x i64> %tmp15127.i, i32 14
  %extract143.i = extractelement <16 x i64> %tmp15127.i, i32 15
  %121 = getelementptr i32 addrspace(1)* %1, i64 %extract128.i
  %122 = getelementptr i32 addrspace(1)* %1, i64 %extract129.i
  %123 = getelementptr i32 addrspace(1)* %1, i64 %extract130.i
  %124 = getelementptr i32 addrspace(1)* %1, i64 %extract131.i
  %125 = getelementptr i32 addrspace(1)* %1, i64 %extract132.i
  %126 = getelementptr i32 addrspace(1)* %1, i64 %extract133.i
  %127 = getelementptr i32 addrspace(1)* %1, i64 %extract134.i
  %128 = getelementptr i32 addrspace(1)* %1, i64 %extract135.i
  %129 = getelementptr i32 addrspace(1)* %1, i64 %extract136.i
  %130 = getelementptr i32 addrspace(1)* %1, i64 %extract137.i
  %131 = getelementptr i32 addrspace(1)* %1, i64 %extract138.i
  %132 = getelementptr i32 addrspace(1)* %1, i64 %extract139.i
  %133 = getelementptr i32 addrspace(1)* %1, i64 %extract140.i
  %134 = getelementptr i32 addrspace(1)* %1, i64 %extract141.i
  %135 = getelementptr i32 addrspace(1)* %1, i64 %extract142.i
  %136 = getelementptr i32 addrspace(1)* %1, i64 %extract143.i
  %137 = load <16 x i32> addrspace(1)* %ptrTypeCast.i, align 4
  %138 = load i32 addrspace(1)* %121, align 4
  %139 = load i32 addrspace(1)* %122, align 4
  %140 = load i32 addrspace(1)* %123, align 4
  %141 = load i32 addrspace(1)* %124, align 4
  %142 = load i32 addrspace(1)* %125, align 4
  %143 = load i32 addrspace(1)* %126, align 4
  %144 = load i32 addrspace(1)* %127, align 4
  %145 = load i32 addrspace(1)* %128, align 4
  %146 = load i32 addrspace(1)* %129, align 4
  %147 = load i32 addrspace(1)* %130, align 4
  %148 = load i32 addrspace(1)* %131, align 4
  %149 = load i32 addrspace(1)* %132, align 4
  %150 = load i32 addrspace(1)* %133, align 4
  %151 = load i32 addrspace(1)* %134, align 4
  %152 = load i32 addrspace(1)* %135, align 4
  %153 = load i32 addrspace(1)* %136, align 4
  %temp.vect144.i = insertelement <16 x i32> undef, i32 %138, i32 0
  %temp.vect145.i = insertelement <16 x i32> %temp.vect144.i, i32 %139, i32 1
  %temp.vect146.i = insertelement <16 x i32> %temp.vect145.i, i32 %140, i32 2
  %temp.vect147.i = insertelement <16 x i32> %temp.vect146.i, i32 %141, i32 3
  %temp.vect148.i = insertelement <16 x i32> %temp.vect147.i, i32 %142, i32 4
  %temp.vect149.i = insertelement <16 x i32> %temp.vect148.i, i32 %143, i32 5
  %temp.vect150.i = insertelement <16 x i32> %temp.vect149.i, i32 %144, i32 6
  %temp.vect151.i = insertelement <16 x i32> %temp.vect150.i, i32 %145, i32 7
  %temp.vect152.i = insertelement <16 x i32> %temp.vect151.i, i32 %146, i32 8
  %temp.vect153.i = insertelement <16 x i32> %temp.vect152.i, i32 %147, i32 9
  %temp.vect154.i = insertelement <16 x i32> %temp.vect153.i, i32 %148, i32 10
  %temp.vect155.i = insertelement <16 x i32> %temp.vect154.i, i32 %149, i32 11
  %temp.vect156.i = insertelement <16 x i32> %temp.vect155.i, i32 %150, i32 12
  %temp.vect157.i = insertelement <16 x i32> %temp.vect156.i, i32 %151, i32 13
  %temp.vect158.i = insertelement <16 x i32> %temp.vect157.i, i32 %152, i32 14
  %temp.vect159.i = insertelement <16 x i32> %temp.vect158.i, i32 %153, i32 15
  %154 = add <16 x i32> %temp.vect159.i, %137
  %extract160.i = extractelement <16 x i32> %154, i32 0
  %extract161.i = extractelement <16 x i32> %154, i32 1
  %extract162.i = extractelement <16 x i32> %154, i32 2
  %extract163.i = extractelement <16 x i32> %154, i32 3
  %extract164.i = extractelement <16 x i32> %154, i32 4
  %extract165.i = extractelement <16 x i32> %154, i32 5
  %extract166.i = extractelement <16 x i32> %154, i32 6
  %extract167.i = extractelement <16 x i32> %154, i32 7
  %extract168.i = extractelement <16 x i32> %154, i32 8
  %extract169.i = extractelement <16 x i32> %154, i32 9
  %extract170.i = extractelement <16 x i32> %154, i32 10
  %extract171.i = extractelement <16 x i32> %154, i32 11
  %extract172.i = extractelement <16 x i32> %154, i32 12
  %extract173.i = extractelement <16 x i32> %154, i32 13
  %extract174.i = extractelement <16 x i32> %154, i32 14
  %extract175.i = extractelement <16 x i32> %154, i32 15
  store i32 %extract160.i, i32 addrspace(1)* %121, align 4
  store i32 %extract161.i, i32 addrspace(1)* %122, align 4
  store i32 %extract162.i, i32 addrspace(1)* %123, align 4
  store i32 %extract163.i, i32 addrspace(1)* %124, align 4
  store i32 %extract164.i, i32 addrspace(1)* %125, align 4
  store i32 %extract165.i, i32 addrspace(1)* %126, align 4
  store i32 %extract166.i, i32 addrspace(1)* %127, align 4
  store i32 %extract167.i, i32 addrspace(1)* %128, align 4
  store i32 %extract168.i, i32 addrspace(1)* %129, align 4
  store i32 %extract169.i, i32 addrspace(1)* %130, align 4
  store i32 %extract170.i, i32 addrspace(1)* %131, align 4
  store i32 %extract171.i, i32 addrspace(1)* %132, align 4
  store i32 %extract172.i, i32 addrspace(1)* %133, align 4
  store i32 %extract173.i, i32 addrspace(1)* %134, align 4
  store i32 %extract174.i, i32 addrspace(1)* %135, align 4
  store i32 %extract175.i, i32 addrspace(1)* %136, align 4
  %155 = add i64 %i.06.i, 1
  %exitcond.i = icmp eq i64 %155, %17
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %.preheader.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.prefixSumStep2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.prefixSumStep2_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__prefixSumStep1_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint", metadata !"opencl_prefixSumStep1_locals_anchor", void (i8*)* @prefixSumStep1}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__prefixSumStep2_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint __attribute__((address_space(1))) *, uint const __attribute__((address_space(1))) *, uint", metadata !"opencl_prefixSumStep2_locals_anchor", void (i8*)* @prefixSumStep2}
