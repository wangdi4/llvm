; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__histogram_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32, i16 addrspace(3)* nocapture) nounwind

declare i64 @get_local_id(i32)

declare i64 @get_global_id(i32)

declare i64 @get_group_id(i32)

declare void @barrier(i64)

declare void @__permute_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32, i16 addrspace(3)* nocapture, i32 addrspace(1)* nocapture) nounwind

declare void @____Vectorized_.histogram_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32, i16 addrspace(3)* nocapture) nounwind

declare void @____Vectorized_.permute_original(i32 addrspace(1)* nocapture, i32 addrspace(1)* nocapture, i32, i16 addrspace(3)* nocapture, i32 addrspace(1)* nocapture) nounwind

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

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__histogram_separated_args(i32 addrspace(1)* nocapture %unsortedData, i32 addrspace(1)* nocapture %buckets, i32 %shiftCount, i16 addrspace(3)* nocapture %sharedArray, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph6:
  br label %SyncBB44

SyncBB44:                                         ; preds = %bb.nph6, %thenBB47
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride53", %thenBB47 ], [ 0, %bb.nph6 ]
  %CurrWI..1 = phi i64 [ %"CurrWI++51", %thenBB47 ], [ 0, %bb.nph6 ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %1 = load i64* %0, align 8
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %1, i64* %CastToValueType, align 8
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %"&(pSB[currWI].offset)341" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset35" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)341"
  %CastToValueType36 = bitcast i8* %"&pSB[currWI].offset35" to i64*
  store i64 %6, i64* %CastToValueType36, align 8
  %7 = load i64* %pWGId, align 8
  %tmp21 = shl i64 %1, 8
  br label %8

; <label>:8                                       ; preds = %8, %SyncBB44
  %indvar18 = phi i64 [ 0, %SyncBB44 ], [ %indvar.next19, %8 ]
  %tmp22 = add i64 %tmp21, %indvar18
  %scevgep23 = getelementptr i16 addrspace(3)* %sharedArray, i64 %tmp22
  store i16 0, i16 addrspace(3)* %scevgep23, align 2
  %indvar.next19 = add i64 %indvar18, 1
  %exitcond20 = icmp eq i64 %indvar.next19, 256
  br i1 %exitcond20, label %bb.nph3, label %8

bb.nph3:                                          ; preds = %8
  %check.WI.iter50 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter50, label %thenBB47, label %SyncBB43

thenBB47:                                         ; preds = %bb.nph3
  %"CurrWI++51" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride53" = add nuw i64 %CurrSBIndex..1, 640
  br label %SyncBB44

SyncBB43:                                         ; preds = %bb.nph3, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %bb.nph3 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph3 ]
  %"&pSB[currWI].offset30" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType31 = bitcast i8* %"&pSB[currWI].offset30" to i64*
  %loadedValue32 = load i64* %CastToValueType31, align 8
  %9 = shl i64 %loadedValue32, 8
  %"&(pSB[currWI].offset)382" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset39" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)382"
  %CastToValueType40 = bitcast i8* %"&pSB[currWI].offset39" to i64*
  %loadedValue41 = load i64* %CastToValueType40, align 8
  %tmp15 = shl i64 %loadedValue41, 8
  br label %10

; <label>:10                                      ; preds = %10, %SyncBB43
  %indvar12 = phi i64 [ 0, %SyncBB43 ], [ %indvar.next13, %10 ]
  %tmp16 = add i64 %tmp15, %indvar12
  %scevgep17 = getelementptr i32 addrspace(1)* %unsortedData, i64 %tmp16
  %11 = load i32 addrspace(1)* %scevgep17, align 4
  %12 = lshr i32 %11, %shiftCount
  %13 = and i32 %12, 255
  %14 = zext i32 %13 to i64
  %15 = or i64 %14, %9
  %16 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %15
  %17 = load i16 addrspace(3)* %16, align 2
  %18 = add i16 %17, 1
  store i16 %18, i16 addrspace(3)* %16, align 2
  %indvar.next13 = add i64 %indvar12, 1
  %exitcond14 = icmp eq i64 %indvar.next13, 256
  br i1 %exitcond14, label %bb.nph, label %10

bb.nph:                                           ; preds = %10
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %bb.nph
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 640
  br label %SyncBB43

elseBB:                                           ; preds = %bb.nph
  %tmp9 = shl i64 %7, 12
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB54, %elseBB
  %CurrSBIndex..2 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride60", %thenBB54 ]
  %CurrWI..2 = phi i64 [ 0, %elseBB ], [ %"CurrWI++58", %thenBB54 ]
  %"&pSB[currWI].offset26" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..2
  %CastToValueType27 = bitcast i8* %"&pSB[currWI].offset26" to i64*
  %loadedValue = load i64* %CastToValueType27, align 8
  %tmp = shl i64 %loadedValue, 8
  %tmp10 = add i64 %tmp9, %tmp
  br label %19

; <label>:19                                      ; preds = %19, %SyncBB
  %indvar = phi i64 [ 0, %SyncBB ], [ %indvar.next, %19 ]
  %tmp8 = add i64 %tmp, %indvar
  %scevgep = getelementptr i16 addrspace(3)* %sharedArray, i64 %tmp8
  %tmp11 = add i64 %tmp10, %indvar
  %20 = load i16 addrspace(3)* %scevgep, align 2
  %21 = zext i16 %20 to i32
  %22 = and i64 %tmp11, 4294967295
  %23 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %22
  store i32 %21, i32 addrspace(1)* %23, align 4
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 256
  br i1 %exitcond, label %._crit_edge, label %19

._crit_edge:                                      ; preds = %19
  %check.WI.iter57 = icmp ult i64 %CurrWI..2, %iterCount
  br i1 %check.WI.iter57, label %thenBB54, label %SyncBB45

thenBB54:                                         ; preds = %._crit_edge
  %"CurrWI++58" = add nuw i64 %CurrWI..2, 1
  %"loadedCurrSB+Stride60" = add nuw i64 %CurrSBIndex..2, 640
  br label %SyncBB

SyncBB45:                                         ; preds = %._crit_edge
  ret void
}

define void @__permute_separated_args(i32 addrspace(1)* nocapture %unsortedData, i32 addrspace(1)* nocapture %prescanedBuckets, i32 %shiftCount, i16 addrspace(3)* nocapture %sharedBuckets, i32 addrspace(1)* nocapture %sortedData, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph3:
  br label %SyncBB28

SyncBB28:                                         ; preds = %bb.nph3, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph3 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %bb.nph3 ]
  %0 = load i64* %pWGId, align 8
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %"&(pSB[currWI].offset)1" = or i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %2, i64* %CastToValueType, align 8
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %"&(pSB[currWI].offset)202" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset21" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)202"
  %CastToValueType22 = bitcast i8* %"&pSB[currWI].offset21" to i64*
  store i64 %7, i64* %CastToValueType22, align 8
  %tmp9 = shl i64 %2, 8
  %tmp12 = shl i64 %0, 12
  %tmp13 = add i64 %tmp12, %tmp9
  br label %8

; <label>:8                                       ; preds = %8, %SyncBB28
  %indvar6 = phi i64 [ 0, %SyncBB28 ], [ %indvar.next7, %8 ]
  %tmp10 = add i64 %tmp9, %indvar6
  %scevgep11 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %tmp10
  %tmp14 = add i64 %tmp13, %indvar6
  %9 = and i64 %tmp14, 4294967295
  %10 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %9
  %11 = load i32 addrspace(1)* %10, align 4
  %12 = trunc i32 %11 to i16
  store i16 %12, i16 addrspace(3)* %scevgep11, align 2
  %indvar.next7 = add i64 %indvar6, 1
  %exitcond8 = icmp eq i64 %indvar.next7, 256
  br i1 %exitcond8, label %bb.nph, label %8

bb.nph:                                           ; preds = %8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %bb.nph
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 640
  br label %SyncBB28

SyncBB:                                           ; preds = %bb.nph, %thenBB31
  %CurrWI..1 = phi i64 [ %"CurrWI++35", %thenBB31 ], [ 0, %bb.nph ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride37", %thenBB31 ], [ 0, %bb.nph ]
  %"&(pSB[currWI].offset)163" = or i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset17" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)163"
  %CastToValueType18 = bitcast i8* %"&pSB[currWI].offset17" to i64*
  %loadedValue = load i64* %CastToValueType18, align 8
  %13 = shl i64 %loadedValue, 8
  %"&(pSB[currWI].offset)244" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset25" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)244"
  %CastToValueType26 = bitcast i8* %"&pSB[currWI].offset25" to i64*
  %loadedValue27 = load i64* %CastToValueType26, align 8
  %tmp = shl i64 %loadedValue27, 8
  br label %14

; <label>:14                                      ; preds = %14, %SyncBB
  %indvar = phi i64 [ 0, %SyncBB ], [ %indvar.next, %14 ]
  %tmp5 = add i64 %tmp, %indvar
  %scevgep = getelementptr i32 addrspace(1)* %unsortedData, i64 %tmp5
  %15 = load i32 addrspace(1)* %scevgep, align 4
  %16 = lshr i32 %15, %shiftCount
  %17 = and i32 %16, 255
  %18 = zext i32 %17 to i64
  %19 = or i64 %18, %13
  %20 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %19
  %21 = load i16 addrspace(3)* %20, align 2
  %22 = zext i16 %21 to i64
  %23 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %22
  store i32 %15, i32 addrspace(1)* %23, align 4
  %24 = add i16 %21, 1
  store i16 %24, i16 addrspace(3)* %20, align 2
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 256
  br i1 %exitcond, label %._crit_edge, label %14

._crit_edge:                                      ; preds = %14
  %check.WI.iter34 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter34, label %thenBB31, label %SyncBB29

thenBB31:                                         ; preds = %._crit_edge
  %"CurrWI++35" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride37" = add nuw i64 %CurrSBIndex..1, 640
  br label %SyncBB

SyncBB29:                                         ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.histogram_separated_args(i32 addrspace(1)* nocapture %unsortedData, i32 addrspace(1)* nocapture %buckets, i32 %shiftCount, i16 addrspace(3)* nocapture %sharedArray, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph6:
  br label %SyncBB199

SyncBB199:                                        ; preds = %bb.nph6, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %bb.nph6 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph6 ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %broadcast1 = insertelement <16 x i64> undef, i64 %1, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %2 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 128
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to <16 x i64>*
  store <16 x i64> %2, <16 x i64>* %CastToValueType, align 128
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %broadcast11 = insertelement <16 x i64> undef, i64 %7, i32 0
  %broadcast22 = shufflevector <16 x i64> %broadcast11, <16 x i64> undef, <16 x i32> zeroinitializer
  %8 = add <16 x i64> %broadcast22, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)187" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset188" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)187"
  %CastToValueType189 = bitcast i8* %"&pSB[currWI].offset188" to <16 x i64>*
  store <16 x i64> %8, <16 x i64>* %CastToValueType189, align 128
  %9 = load i64* %pWGId, align 8
  %tmp213 = shl <16 x i64> %2, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  br label %10

; <label>:10                                      ; preds = %10, %SyncBB199
  %indvar18 = phi i64 [ 0, %SyncBB199 ], [ %indvar.next19, %10 ]
  %temp = insertelement <16 x i64> undef, i64 %indvar18, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp224 = add <16 x i64> %tmp213, %vector
  %extract = extractelement <16 x i64> %tmp224, i32 0
  %extract5 = extractelement <16 x i64> %tmp224, i32 1
  %extract6 = extractelement <16 x i64> %tmp224, i32 2
  %extract7 = extractelement <16 x i64> %tmp224, i32 3
  %extract8 = extractelement <16 x i64> %tmp224, i32 4
  %extract9 = extractelement <16 x i64> %tmp224, i32 5
  %extract10 = extractelement <16 x i64> %tmp224, i32 6
  %extract11 = extractelement <16 x i64> %tmp224, i32 7
  %extract12 = extractelement <16 x i64> %tmp224, i32 8
  %extract13 = extractelement <16 x i64> %tmp224, i32 9
  %extract14 = extractelement <16 x i64> %tmp224, i32 10
  %extract15 = extractelement <16 x i64> %tmp224, i32 11
  %extract16 = extractelement <16 x i64> %tmp224, i32 12
  %extract17 = extractelement <16 x i64> %tmp224, i32 13
  %extract18 = extractelement <16 x i64> %tmp224, i32 14
  %extract19 = extractelement <16 x i64> %tmp224, i32 15
  %11 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract
  %12 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract5
  %13 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract6
  %14 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract7
  %15 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract8
  %16 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract9
  %17 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract10
  %18 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract11
  %19 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract12
  %20 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract13
  %21 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract14
  %22 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract15
  %23 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract16
  %24 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract17
  %25 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract18
  %26 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract19
  store i16 0, i16 addrspace(3)* %11, align 2
  store i16 0, i16 addrspace(3)* %12, align 2
  store i16 0, i16 addrspace(3)* %13, align 2
  store i16 0, i16 addrspace(3)* %14, align 2
  store i16 0, i16 addrspace(3)* %15, align 2
  store i16 0, i16 addrspace(3)* %16, align 2
  store i16 0, i16 addrspace(3)* %17, align 2
  store i16 0, i16 addrspace(3)* %18, align 2
  store i16 0, i16 addrspace(3)* %19, align 2
  store i16 0, i16 addrspace(3)* %20, align 2
  store i16 0, i16 addrspace(3)* %21, align 2
  store i16 0, i16 addrspace(3)* %22, align 2
  store i16 0, i16 addrspace(3)* %23, align 2
  store i16 0, i16 addrspace(3)* %24, align 2
  store i16 0, i16 addrspace(3)* %25, align 2
  store i16 0, i16 addrspace(3)* %26, align 2
  %indvar.next19 = add i64 %indvar18, 1
  %exitcond20 = icmp eq i64 %indvar.next19, 256
  br i1 %exitcond20, label %bb.nph3, label %10

bb.nph3:                                          ; preds = %10
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %bb.nph3
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 640
  br label %SyncBB199

elseBB:                                           ; preds = %bb.nph3
  %temp55 = insertelement <16 x i32> undef, i32 %shiftCount, i32 0
  %vector56 = shufflevector <16 x i32> %temp55, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB202, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride208", %thenBB202 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++206", %thenBB202 ]
  %"&(pSB[currWI].offset)182" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset183" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)182"
  %CastToValueType184 = bitcast i8* %"&pSB[currWI].offset183" to <16 x i64>*
  %loadedValue185 = load <16 x i64>* %CastToValueType184, align 128
  %27 = shl <16 x i64> %loadedValue185, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %"&(pSB[currWI].offset)191" = add nuw i64 %CurrSBIndex..1, 256
  %"&pSB[currWI].offset192" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)191"
  %CastToValueType193 = bitcast i8* %"&pSB[currWI].offset192" to <16 x i64>*
  %loadedValue194 = load <16 x i64>* %CastToValueType193, align 128
  %tmp1520 = shl <16 x i64> %loadedValue194, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  br label %28

; <label>:28                                      ; preds = %28, %SyncBB
  %indvar12 = phi i64 [ 0, %SyncBB ], [ %indvar.next13, %28 ]
  %temp21 = insertelement <16 x i64> undef, i64 %indvar12, i32 0
  %vector22 = shufflevector <16 x i64> %temp21, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1623 = add <16 x i64> %tmp1520, %vector22
  %extract24 = extractelement <16 x i64> %tmp1623, i32 0
  %extract25 = extractelement <16 x i64> %tmp1623, i32 1
  %extract26 = extractelement <16 x i64> %tmp1623, i32 2
  %extract27 = extractelement <16 x i64> %tmp1623, i32 3
  %extract28 = extractelement <16 x i64> %tmp1623, i32 4
  %extract29 = extractelement <16 x i64> %tmp1623, i32 5
  %extract30 = extractelement <16 x i64> %tmp1623, i32 6
  %extract31 = extractelement <16 x i64> %tmp1623, i32 7
  %extract32 = extractelement <16 x i64> %tmp1623, i32 8
  %extract33 = extractelement <16 x i64> %tmp1623, i32 9
  %extract34 = extractelement <16 x i64> %tmp1623, i32 10
  %extract35 = extractelement <16 x i64> %tmp1623, i32 11
  %extract36 = extractelement <16 x i64> %tmp1623, i32 12
  %extract37 = extractelement <16 x i64> %tmp1623, i32 13
  %extract38 = extractelement <16 x i64> %tmp1623, i32 14
  %extract39 = extractelement <16 x i64> %tmp1623, i32 15
  %29 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract24
  %30 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract25
  %31 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract26
  %32 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract27
  %33 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract28
  %34 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract29
  %35 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract30
  %36 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract31
  %37 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract32
  %38 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract33
  %39 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract34
  %40 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract35
  %41 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract36
  %42 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract37
  %43 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract38
  %44 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract39
  %45 = load i32 addrspace(1)* %29, align 4
  %46 = load i32 addrspace(1)* %30, align 4
  %47 = load i32 addrspace(1)* %31, align 4
  %48 = load i32 addrspace(1)* %32, align 4
  %49 = load i32 addrspace(1)* %33, align 4
  %50 = load i32 addrspace(1)* %34, align 4
  %51 = load i32 addrspace(1)* %35, align 4
  %52 = load i32 addrspace(1)* %36, align 4
  %53 = load i32 addrspace(1)* %37, align 4
  %54 = load i32 addrspace(1)* %38, align 4
  %55 = load i32 addrspace(1)* %39, align 4
  %56 = load i32 addrspace(1)* %40, align 4
  %57 = load i32 addrspace(1)* %41, align 4
  %58 = load i32 addrspace(1)* %42, align 4
  %59 = load i32 addrspace(1)* %43, align 4
  %60 = load i32 addrspace(1)* %44, align 4
  %temp.vect = insertelement <16 x i32> undef, i32 %45, i32 0
  %temp.vect40 = insertelement <16 x i32> %temp.vect, i32 %46, i32 1
  %temp.vect41 = insertelement <16 x i32> %temp.vect40, i32 %47, i32 2
  %temp.vect42 = insertelement <16 x i32> %temp.vect41, i32 %48, i32 3
  %temp.vect43 = insertelement <16 x i32> %temp.vect42, i32 %49, i32 4
  %temp.vect44 = insertelement <16 x i32> %temp.vect43, i32 %50, i32 5
  %temp.vect45 = insertelement <16 x i32> %temp.vect44, i32 %51, i32 6
  %temp.vect46 = insertelement <16 x i32> %temp.vect45, i32 %52, i32 7
  %temp.vect47 = insertelement <16 x i32> %temp.vect46, i32 %53, i32 8
  %temp.vect48 = insertelement <16 x i32> %temp.vect47, i32 %54, i32 9
  %temp.vect49 = insertelement <16 x i32> %temp.vect48, i32 %55, i32 10
  %temp.vect50 = insertelement <16 x i32> %temp.vect49, i32 %56, i32 11
  %temp.vect51 = insertelement <16 x i32> %temp.vect50, i32 %57, i32 12
  %temp.vect52 = insertelement <16 x i32> %temp.vect51, i32 %58, i32 13
  %temp.vect53 = insertelement <16 x i32> %temp.vect52, i32 %59, i32 14
  %temp.vect54 = insertelement <16 x i32> %temp.vect53, i32 %60, i32 15
  %61 = lshr <16 x i32> %temp.vect54, %vector56
  %62 = and <16 x i32> %61, <i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255>
  %63 = zext <16 x i32> %62 to <16 x i64>
  %64 = or <16 x i64> %63, %27
  %extract57 = extractelement <16 x i64> %64, i32 0
  %extract58 = extractelement <16 x i64> %64, i32 1
  %extract59 = extractelement <16 x i64> %64, i32 2
  %extract60 = extractelement <16 x i64> %64, i32 3
  %extract61 = extractelement <16 x i64> %64, i32 4
  %extract62 = extractelement <16 x i64> %64, i32 5
  %extract63 = extractelement <16 x i64> %64, i32 6
  %extract64 = extractelement <16 x i64> %64, i32 7
  %extract65 = extractelement <16 x i64> %64, i32 8
  %extract66 = extractelement <16 x i64> %64, i32 9
  %extract67 = extractelement <16 x i64> %64, i32 10
  %extract68 = extractelement <16 x i64> %64, i32 11
  %extract69 = extractelement <16 x i64> %64, i32 12
  %extract70 = extractelement <16 x i64> %64, i32 13
  %extract71 = extractelement <16 x i64> %64, i32 14
  %extract72 = extractelement <16 x i64> %64, i32 15
  %65 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract57
  %66 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract58
  %67 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract59
  %68 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract60
  %69 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract61
  %70 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract62
  %71 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract63
  %72 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract64
  %73 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract65
  %74 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract66
  %75 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract67
  %76 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract68
  %77 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract69
  %78 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract70
  %79 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract71
  %80 = getelementptr inbounds i16 addrspace(3)* %sharedArray, i64 %extract72
  %81 = load i16 addrspace(3)* %65, align 2
  %82 = load i16 addrspace(3)* %66, align 2
  %83 = load i16 addrspace(3)* %67, align 2
  %84 = load i16 addrspace(3)* %68, align 2
  %85 = load i16 addrspace(3)* %69, align 2
  %86 = load i16 addrspace(3)* %70, align 2
  %87 = load i16 addrspace(3)* %71, align 2
  %88 = load i16 addrspace(3)* %72, align 2
  %89 = load i16 addrspace(3)* %73, align 2
  %90 = load i16 addrspace(3)* %74, align 2
  %91 = load i16 addrspace(3)* %75, align 2
  %92 = load i16 addrspace(3)* %76, align 2
  %93 = load i16 addrspace(3)* %77, align 2
  %94 = load i16 addrspace(3)* %78, align 2
  %95 = load i16 addrspace(3)* %79, align 2
  %96 = load i16 addrspace(3)* %80, align 2
  %temp.vect73 = insertelement <16 x i16> undef, i16 %81, i32 0
  %temp.vect74 = insertelement <16 x i16> %temp.vect73, i16 %82, i32 1
  %temp.vect75 = insertelement <16 x i16> %temp.vect74, i16 %83, i32 2
  %temp.vect76 = insertelement <16 x i16> %temp.vect75, i16 %84, i32 3
  %temp.vect77 = insertelement <16 x i16> %temp.vect76, i16 %85, i32 4
  %temp.vect78 = insertelement <16 x i16> %temp.vect77, i16 %86, i32 5
  %temp.vect79 = insertelement <16 x i16> %temp.vect78, i16 %87, i32 6
  %temp.vect80 = insertelement <16 x i16> %temp.vect79, i16 %88, i32 7
  %temp.vect81 = insertelement <16 x i16> %temp.vect80, i16 %89, i32 8
  %temp.vect82 = insertelement <16 x i16> %temp.vect81, i16 %90, i32 9
  %temp.vect83 = insertelement <16 x i16> %temp.vect82, i16 %91, i32 10
  %temp.vect84 = insertelement <16 x i16> %temp.vect83, i16 %92, i32 11
  %temp.vect85 = insertelement <16 x i16> %temp.vect84, i16 %93, i32 12
  %temp.vect86 = insertelement <16 x i16> %temp.vect85, i16 %94, i32 13
  %temp.vect87 = insertelement <16 x i16> %temp.vect86, i16 %95, i32 14
  %temp.vect88 = insertelement <16 x i16> %temp.vect87, i16 %96, i32 15
  %97 = add <16 x i16> %temp.vect88, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %extract89 = extractelement <16 x i16> %97, i32 0
  %extract90 = extractelement <16 x i16> %97, i32 1
  %extract91 = extractelement <16 x i16> %97, i32 2
  %extract92 = extractelement <16 x i16> %97, i32 3
  %extract93 = extractelement <16 x i16> %97, i32 4
  %extract94 = extractelement <16 x i16> %97, i32 5
  %extract95 = extractelement <16 x i16> %97, i32 6
  %extract96 = extractelement <16 x i16> %97, i32 7
  %extract97 = extractelement <16 x i16> %97, i32 8
  %extract98 = extractelement <16 x i16> %97, i32 9
  %extract99 = extractelement <16 x i16> %97, i32 10
  %extract100 = extractelement <16 x i16> %97, i32 11
  %extract101 = extractelement <16 x i16> %97, i32 12
  %extract102 = extractelement <16 x i16> %97, i32 13
  %extract103 = extractelement <16 x i16> %97, i32 14
  %extract104 = extractelement <16 x i16> %97, i32 15
  store i16 %extract89, i16 addrspace(3)* %65, align 2
  store i16 %extract90, i16 addrspace(3)* %66, align 2
  store i16 %extract91, i16 addrspace(3)* %67, align 2
  store i16 %extract92, i16 addrspace(3)* %68, align 2
  store i16 %extract93, i16 addrspace(3)* %69, align 2
  store i16 %extract94, i16 addrspace(3)* %70, align 2
  store i16 %extract95, i16 addrspace(3)* %71, align 2
  store i16 %extract96, i16 addrspace(3)* %72, align 2
  store i16 %extract97, i16 addrspace(3)* %73, align 2
  store i16 %extract98, i16 addrspace(3)* %74, align 2
  store i16 %extract99, i16 addrspace(3)* %75, align 2
  store i16 %extract100, i16 addrspace(3)* %76, align 2
  store i16 %extract101, i16 addrspace(3)* %77, align 2
  store i16 %extract102, i16 addrspace(3)* %78, align 2
  store i16 %extract103, i16 addrspace(3)* %79, align 2
  store i16 %extract104, i16 addrspace(3)* %80, align 2
  %indvar.next13 = add i64 %indvar12, 1
  %exitcond14 = icmp eq i64 %indvar.next13, 256
  br i1 %exitcond14, label %bb.nph, label %28

bb.nph:                                           ; preds = %28
  %check.WI.iter205 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter205, label %thenBB202, label %elseBB203

thenBB202:                                        ; preds = %bb.nph
  %"CurrWI++206" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride208" = add nuw i64 %CurrSBIndex..1, 640
  br label %SyncBB

elseBB203:                                        ; preds = %bb.nph
  %tmp9 = shl i64 %9, 12
  %temp106 = insertelement <16 x i64> undef, i64 %tmp9, i32 0
  %vector107 = shufflevector <16 x i64> %temp106, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB198

SyncBB198:                                        ; preds = %thenBB209, %elseBB203
  %CurrSBIndex..2 = phi i64 [ 0, %elseBB203 ], [ %"loadedCurrSB+Stride215", %thenBB209 ]
  %CurrWI..2 = phi i64 [ 0, %elseBB203 ], [ %"CurrWI++213", %thenBB209 ]
  %"&(pSB[currWI].offset)178" = add nuw i64 %CurrSBIndex..2, 128
  %"&pSB[currWI].offset179" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)178"
  %CastToValueType180 = bitcast i8* %"&pSB[currWI].offset179" to <16 x i64>*
  %loadedValue = load <16 x i64>* %CastToValueType180, align 128
  %tmp105 = shl <16 x i64> %loadedValue, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %tmp10108 = add <16 x i64> %vector107, %tmp105
  br label %98

; <label>:98                                      ; preds = %98, %SyncBB198
  %indvar = phi i64 [ 0, %SyncBB198 ], [ %indvar.next, %98 ]
  %temp109 = insertelement <16 x i64> undef, i64 %indvar, i32 0
  %vector110 = shufflevector <16 x i64> %temp109, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp8111 = add <16 x i64> %tmp105, %vector110
  %extract112 = extractelement <16 x i64> %tmp8111, i32 0
  %extract113 = extractelement <16 x i64> %tmp8111, i32 1
  %extract114 = extractelement <16 x i64> %tmp8111, i32 2
  %extract115 = extractelement <16 x i64> %tmp8111, i32 3
  %extract116 = extractelement <16 x i64> %tmp8111, i32 4
  %extract117 = extractelement <16 x i64> %tmp8111, i32 5
  %extract118 = extractelement <16 x i64> %tmp8111, i32 6
  %extract119 = extractelement <16 x i64> %tmp8111, i32 7
  %extract120 = extractelement <16 x i64> %tmp8111, i32 8
  %extract121 = extractelement <16 x i64> %tmp8111, i32 9
  %extract122 = extractelement <16 x i64> %tmp8111, i32 10
  %extract123 = extractelement <16 x i64> %tmp8111, i32 11
  %extract124 = extractelement <16 x i64> %tmp8111, i32 12
  %extract125 = extractelement <16 x i64> %tmp8111, i32 13
  %extract126 = extractelement <16 x i64> %tmp8111, i32 14
  %extract127 = extractelement <16 x i64> %tmp8111, i32 15
  %99 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract112
  %100 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract113
  %101 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract114
  %102 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract115
  %103 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract116
  %104 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract117
  %105 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract118
  %106 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract119
  %107 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract120
  %108 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract121
  %109 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract122
  %110 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract123
  %111 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract124
  %112 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract125
  %113 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract126
  %114 = getelementptr i16 addrspace(3)* %sharedArray, i64 %extract127
  %tmp11128 = add <16 x i64> %tmp10108, %vector110
  %115 = load i16 addrspace(3)* %99, align 2
  %116 = load i16 addrspace(3)* %100, align 2
  %117 = load i16 addrspace(3)* %101, align 2
  %118 = load i16 addrspace(3)* %102, align 2
  %119 = load i16 addrspace(3)* %103, align 2
  %120 = load i16 addrspace(3)* %104, align 2
  %121 = load i16 addrspace(3)* %105, align 2
  %122 = load i16 addrspace(3)* %106, align 2
  %123 = load i16 addrspace(3)* %107, align 2
  %124 = load i16 addrspace(3)* %108, align 2
  %125 = load i16 addrspace(3)* %109, align 2
  %126 = load i16 addrspace(3)* %110, align 2
  %127 = load i16 addrspace(3)* %111, align 2
  %128 = load i16 addrspace(3)* %112, align 2
  %129 = load i16 addrspace(3)* %113, align 2
  %130 = load i16 addrspace(3)* %114, align 2
  %temp.vect129 = insertelement <16 x i16> undef, i16 %115, i32 0
  %temp.vect130 = insertelement <16 x i16> %temp.vect129, i16 %116, i32 1
  %temp.vect131 = insertelement <16 x i16> %temp.vect130, i16 %117, i32 2
  %temp.vect132 = insertelement <16 x i16> %temp.vect131, i16 %118, i32 3
  %temp.vect133 = insertelement <16 x i16> %temp.vect132, i16 %119, i32 4
  %temp.vect134 = insertelement <16 x i16> %temp.vect133, i16 %120, i32 5
  %temp.vect135 = insertelement <16 x i16> %temp.vect134, i16 %121, i32 6
  %temp.vect136 = insertelement <16 x i16> %temp.vect135, i16 %122, i32 7
  %temp.vect137 = insertelement <16 x i16> %temp.vect136, i16 %123, i32 8
  %temp.vect138 = insertelement <16 x i16> %temp.vect137, i16 %124, i32 9
  %temp.vect139 = insertelement <16 x i16> %temp.vect138, i16 %125, i32 10
  %temp.vect140 = insertelement <16 x i16> %temp.vect139, i16 %126, i32 11
  %temp.vect141 = insertelement <16 x i16> %temp.vect140, i16 %127, i32 12
  %temp.vect142 = insertelement <16 x i16> %temp.vect141, i16 %128, i32 13
  %temp.vect143 = insertelement <16 x i16> %temp.vect142, i16 %129, i32 14
  %temp.vect144 = insertelement <16 x i16> %temp.vect143, i16 %130, i32 15
  %131 = zext <16 x i16> %temp.vect144 to <16 x i32>
  %extract161 = extractelement <16 x i32> %131, i32 0
  %extract162 = extractelement <16 x i32> %131, i32 1
  %extract163 = extractelement <16 x i32> %131, i32 2
  %extract164 = extractelement <16 x i32> %131, i32 3
  %extract165 = extractelement <16 x i32> %131, i32 4
  %extract166 = extractelement <16 x i32> %131, i32 5
  %extract167 = extractelement <16 x i32> %131, i32 6
  %extract168 = extractelement <16 x i32> %131, i32 7
  %extract169 = extractelement <16 x i32> %131, i32 8
  %extract170 = extractelement <16 x i32> %131, i32 9
  %extract171 = extractelement <16 x i32> %131, i32 10
  %extract172 = extractelement <16 x i32> %131, i32 11
  %extract173 = extractelement <16 x i32> %131, i32 12
  %extract174 = extractelement <16 x i32> %131, i32 13
  %extract175 = extractelement <16 x i32> %131, i32 14
  %extract176 = extractelement <16 x i32> %131, i32 15
  %132 = and <16 x i64> %tmp11128, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract145 = extractelement <16 x i64> %132, i32 0
  %extract146 = extractelement <16 x i64> %132, i32 1
  %extract147 = extractelement <16 x i64> %132, i32 2
  %extract148 = extractelement <16 x i64> %132, i32 3
  %extract149 = extractelement <16 x i64> %132, i32 4
  %extract150 = extractelement <16 x i64> %132, i32 5
  %extract151 = extractelement <16 x i64> %132, i32 6
  %extract152 = extractelement <16 x i64> %132, i32 7
  %extract153 = extractelement <16 x i64> %132, i32 8
  %extract154 = extractelement <16 x i64> %132, i32 9
  %extract155 = extractelement <16 x i64> %132, i32 10
  %extract156 = extractelement <16 x i64> %132, i32 11
  %extract157 = extractelement <16 x i64> %132, i32 12
  %extract158 = extractelement <16 x i64> %132, i32 13
  %extract159 = extractelement <16 x i64> %132, i32 14
  %extract160 = extractelement <16 x i64> %132, i32 15
  %133 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract145
  %134 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract146
  %135 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract147
  %136 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract148
  %137 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract149
  %138 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract150
  %139 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract151
  %140 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract152
  %141 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract153
  %142 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract154
  %143 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract155
  %144 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract156
  %145 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract157
  %146 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract158
  %147 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract159
  %148 = getelementptr inbounds i32 addrspace(1)* %buckets, i64 %extract160
  store i32 %extract161, i32 addrspace(1)* %133, align 4
  store i32 %extract162, i32 addrspace(1)* %134, align 4
  store i32 %extract163, i32 addrspace(1)* %135, align 4
  store i32 %extract164, i32 addrspace(1)* %136, align 4
  store i32 %extract165, i32 addrspace(1)* %137, align 4
  store i32 %extract166, i32 addrspace(1)* %138, align 4
  store i32 %extract167, i32 addrspace(1)* %139, align 4
  store i32 %extract168, i32 addrspace(1)* %140, align 4
  store i32 %extract169, i32 addrspace(1)* %141, align 4
  store i32 %extract170, i32 addrspace(1)* %142, align 4
  store i32 %extract171, i32 addrspace(1)* %143, align 4
  store i32 %extract172, i32 addrspace(1)* %144, align 4
  store i32 %extract173, i32 addrspace(1)* %145, align 4
  store i32 %extract174, i32 addrspace(1)* %146, align 4
  store i32 %extract175, i32 addrspace(1)* %147, align 4
  store i32 %extract176, i32 addrspace(1)* %148, align 4
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 256
  br i1 %exitcond, label %._crit_edge, label %98

._crit_edge:                                      ; preds = %98
  %check.WI.iter212 = icmp ult i64 %CurrWI..2, %iterCount
  br i1 %check.WI.iter212, label %thenBB209, label %SyncBB200

thenBB209:                                        ; preds = %._crit_edge
  %"CurrWI++213" = add nuw i64 %CurrWI..2, 1
  %"loadedCurrSB+Stride215" = add nuw i64 %CurrSBIndex..2, 640
  br label %SyncBB198

SyncBB200:                                        ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.permute_separated_args(i32 addrspace(1)* nocapture %unsortedData, i32 addrspace(1)* nocapture %prescanedBuckets, i32 %shiftCount, i16 addrspace(3)* nocapture %sharedBuckets, i32 addrspace(1)* nocapture %sortedData, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph3:
  br label %SyncBB188

SyncBB188:                                        ; preds = %bb.nph3, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %bb.nph3 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph3 ]
  %0 = load i64* %pWGId, align 8
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %broadcast1 = insertelement <16 x i64> undef, i64 %2, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %3 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 384
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to <16 x i64>*
  store <16 x i64> %3, <16 x i64>* %CastToValueType, align 128
  %4 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %7 = load i64* %6, align 8
  %8 = add i64 %5, %7
  %broadcast11 = insertelement <16 x i64> undef, i64 %8, i32 0
  %broadcast22 = shufflevector <16 x i64> %broadcast11, <16 x i64> undef, <16 x i32> zeroinitializer
  %9 = add <16 x i64> %broadcast22, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)178" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset179" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)178"
  %CastToValueType180 = bitcast i8* %"&pSB[currWI].offset179" to <16 x i64>*
  store <16 x i64> %9, <16 x i64>* %CastToValueType180, align 128
  %tmp93 = shl <16 x i64> %3, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %tmp12 = shl i64 %0, 12
  %temp = insertelement <16 x i64> undef, i64 %tmp12, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp134 = add <16 x i64> %vector, %tmp93
  br label %10

; <label>:10                                      ; preds = %10, %SyncBB188
  %indvar6 = phi i64 [ 0, %SyncBB188 ], [ %indvar.next7, %10 ]
  %temp5 = insertelement <16 x i64> undef, i64 %indvar6, i32 0
  %vector6 = shufflevector <16 x i64> %temp5, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp107 = add <16 x i64> %tmp93, %vector6
  %extract = extractelement <16 x i64> %tmp107, i32 0
  %extract8 = extractelement <16 x i64> %tmp107, i32 1
  %extract9 = extractelement <16 x i64> %tmp107, i32 2
  %extract10 = extractelement <16 x i64> %tmp107, i32 3
  %extract11 = extractelement <16 x i64> %tmp107, i32 4
  %extract12 = extractelement <16 x i64> %tmp107, i32 5
  %extract13 = extractelement <16 x i64> %tmp107, i32 6
  %extract14 = extractelement <16 x i64> %tmp107, i32 7
  %extract15 = extractelement <16 x i64> %tmp107, i32 8
  %extract16 = extractelement <16 x i64> %tmp107, i32 9
  %extract17 = extractelement <16 x i64> %tmp107, i32 10
  %extract18 = extractelement <16 x i64> %tmp107, i32 11
  %extract19 = extractelement <16 x i64> %tmp107, i32 12
  %extract20 = extractelement <16 x i64> %tmp107, i32 13
  %extract21 = extractelement <16 x i64> %tmp107, i32 14
  %extract22 = extractelement <16 x i64> %tmp107, i32 15
  %11 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract
  %12 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract8
  %13 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract9
  %14 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract10
  %15 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract11
  %16 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract12
  %17 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract13
  %18 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract14
  %19 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract15
  %20 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract16
  %21 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract17
  %22 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract18
  %23 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract19
  %24 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract20
  %25 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract21
  %26 = getelementptr i16 addrspace(3)* %sharedBuckets, i64 %extract22
  %tmp1423 = add <16 x i64> %tmp134, %vector6
  %27 = and <16 x i64> %tmp1423, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract24 = extractelement <16 x i64> %27, i32 0
  %extract25 = extractelement <16 x i64> %27, i32 1
  %extract26 = extractelement <16 x i64> %27, i32 2
  %extract27 = extractelement <16 x i64> %27, i32 3
  %extract28 = extractelement <16 x i64> %27, i32 4
  %extract29 = extractelement <16 x i64> %27, i32 5
  %extract30 = extractelement <16 x i64> %27, i32 6
  %extract31 = extractelement <16 x i64> %27, i32 7
  %extract32 = extractelement <16 x i64> %27, i32 8
  %extract33 = extractelement <16 x i64> %27, i32 9
  %extract34 = extractelement <16 x i64> %27, i32 10
  %extract35 = extractelement <16 x i64> %27, i32 11
  %extract36 = extractelement <16 x i64> %27, i32 12
  %extract37 = extractelement <16 x i64> %27, i32 13
  %extract38 = extractelement <16 x i64> %27, i32 14
  %extract39 = extractelement <16 x i64> %27, i32 15
  %28 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract24
  %29 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract25
  %30 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract26
  %31 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract27
  %32 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract28
  %33 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract29
  %34 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract30
  %35 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract31
  %36 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract32
  %37 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract33
  %38 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract34
  %39 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract35
  %40 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract36
  %41 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract37
  %42 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract38
  %43 = getelementptr inbounds i32 addrspace(1)* %prescanedBuckets, i64 %extract39
  %44 = load i32 addrspace(1)* %28, align 4
  %45 = load i32 addrspace(1)* %29, align 4
  %46 = load i32 addrspace(1)* %30, align 4
  %47 = load i32 addrspace(1)* %31, align 4
  %48 = load i32 addrspace(1)* %32, align 4
  %49 = load i32 addrspace(1)* %33, align 4
  %50 = load i32 addrspace(1)* %34, align 4
  %51 = load i32 addrspace(1)* %35, align 4
  %52 = load i32 addrspace(1)* %36, align 4
  %53 = load i32 addrspace(1)* %37, align 4
  %54 = load i32 addrspace(1)* %38, align 4
  %55 = load i32 addrspace(1)* %39, align 4
  %56 = load i32 addrspace(1)* %40, align 4
  %57 = load i32 addrspace(1)* %41, align 4
  %58 = load i32 addrspace(1)* %42, align 4
  %59 = load i32 addrspace(1)* %43, align 4
  %temp.vect = insertelement <16 x i32> undef, i32 %44, i32 0
  %temp.vect40 = insertelement <16 x i32> %temp.vect, i32 %45, i32 1
  %temp.vect41 = insertelement <16 x i32> %temp.vect40, i32 %46, i32 2
  %temp.vect42 = insertelement <16 x i32> %temp.vect41, i32 %47, i32 3
  %temp.vect43 = insertelement <16 x i32> %temp.vect42, i32 %48, i32 4
  %temp.vect44 = insertelement <16 x i32> %temp.vect43, i32 %49, i32 5
  %temp.vect45 = insertelement <16 x i32> %temp.vect44, i32 %50, i32 6
  %temp.vect46 = insertelement <16 x i32> %temp.vect45, i32 %51, i32 7
  %temp.vect47 = insertelement <16 x i32> %temp.vect46, i32 %52, i32 8
  %temp.vect48 = insertelement <16 x i32> %temp.vect47, i32 %53, i32 9
  %temp.vect49 = insertelement <16 x i32> %temp.vect48, i32 %54, i32 10
  %temp.vect50 = insertelement <16 x i32> %temp.vect49, i32 %55, i32 11
  %temp.vect51 = insertelement <16 x i32> %temp.vect50, i32 %56, i32 12
  %temp.vect52 = insertelement <16 x i32> %temp.vect51, i32 %57, i32 13
  %temp.vect53 = insertelement <16 x i32> %temp.vect52, i32 %58, i32 14
  %temp.vect54 = insertelement <16 x i32> %temp.vect53, i32 %59, i32 15
  %60 = trunc <16 x i32> %temp.vect54 to <16 x i16>
  %extract55 = extractelement <16 x i16> %60, i32 0
  %extract56 = extractelement <16 x i16> %60, i32 1
  %extract57 = extractelement <16 x i16> %60, i32 2
  %extract58 = extractelement <16 x i16> %60, i32 3
  %extract59 = extractelement <16 x i16> %60, i32 4
  %extract60 = extractelement <16 x i16> %60, i32 5
  %extract61 = extractelement <16 x i16> %60, i32 6
  %extract62 = extractelement <16 x i16> %60, i32 7
  %extract63 = extractelement <16 x i16> %60, i32 8
  %extract64 = extractelement <16 x i16> %60, i32 9
  %extract65 = extractelement <16 x i16> %60, i32 10
  %extract66 = extractelement <16 x i16> %60, i32 11
  %extract67 = extractelement <16 x i16> %60, i32 12
  %extract68 = extractelement <16 x i16> %60, i32 13
  %extract69 = extractelement <16 x i16> %60, i32 14
  %extract70 = extractelement <16 x i16> %60, i32 15
  store i16 %extract55, i16 addrspace(3)* %11, align 2
  store i16 %extract56, i16 addrspace(3)* %12, align 2
  store i16 %extract57, i16 addrspace(3)* %13, align 2
  store i16 %extract58, i16 addrspace(3)* %14, align 2
  store i16 %extract59, i16 addrspace(3)* %15, align 2
  store i16 %extract60, i16 addrspace(3)* %16, align 2
  store i16 %extract61, i16 addrspace(3)* %17, align 2
  store i16 %extract62, i16 addrspace(3)* %18, align 2
  store i16 %extract63, i16 addrspace(3)* %19, align 2
  store i16 %extract64, i16 addrspace(3)* %20, align 2
  store i16 %extract65, i16 addrspace(3)* %21, align 2
  store i16 %extract66, i16 addrspace(3)* %22, align 2
  store i16 %extract67, i16 addrspace(3)* %23, align 2
  store i16 %extract68, i16 addrspace(3)* %24, align 2
  store i16 %extract69, i16 addrspace(3)* %25, align 2
  store i16 %extract70, i16 addrspace(3)* %26, align 2
  %indvar.next7 = add i64 %indvar6, 1
  %exitcond8 = icmp eq i64 %indvar.next7, 256
  br i1 %exitcond8, label %bb.nph, label %10

bb.nph:                                           ; preds = %10
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %bb.nph
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 640
  br label %SyncBB188

elseBB:                                           ; preds = %bb.nph
  %temp107 = insertelement <16 x i32> undef, i32 %shiftCount, i32 0
  %vector108 = shufflevector <16 x i32> %temp107, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB191, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride197", %thenBB191 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++195", %thenBB191 ]
  %"&(pSB[currWI].offset)174" = add nuw i64 %CurrSBIndex..1, 384
  %"&pSB[currWI].offset175" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)174"
  %CastToValueType176 = bitcast i8* %"&pSB[currWI].offset175" to <16 x i64>*
  %loadedValue = load <16 x i64>* %CastToValueType176, align 128
  %61 = shl <16 x i64> %loadedValue, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %"&(pSB[currWI].offset)182" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset183" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)182"
  %CastToValueType184 = bitcast i8* %"&pSB[currWI].offset183" to <16 x i64>*
  %loadedValue185 = load <16 x i64>* %CastToValueType184, align 128
  %tmp71 = shl <16 x i64> %loadedValue185, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  br label %62

; <label>:62                                      ; preds = %62, %SyncBB
  %indvar = phi i64 [ 0, %SyncBB ], [ %indvar.next, %62 ]
  %temp72 = insertelement <16 x i64> undef, i64 %indvar, i32 0
  %vector73 = shufflevector <16 x i64> %temp72, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp574 = add <16 x i64> %tmp71, %vector73
  %extract75 = extractelement <16 x i64> %tmp574, i32 0
  %extract76 = extractelement <16 x i64> %tmp574, i32 1
  %extract77 = extractelement <16 x i64> %tmp574, i32 2
  %extract78 = extractelement <16 x i64> %tmp574, i32 3
  %extract79 = extractelement <16 x i64> %tmp574, i32 4
  %extract80 = extractelement <16 x i64> %tmp574, i32 5
  %extract81 = extractelement <16 x i64> %tmp574, i32 6
  %extract82 = extractelement <16 x i64> %tmp574, i32 7
  %extract83 = extractelement <16 x i64> %tmp574, i32 8
  %extract84 = extractelement <16 x i64> %tmp574, i32 9
  %extract85 = extractelement <16 x i64> %tmp574, i32 10
  %extract86 = extractelement <16 x i64> %tmp574, i32 11
  %extract87 = extractelement <16 x i64> %tmp574, i32 12
  %extract88 = extractelement <16 x i64> %tmp574, i32 13
  %extract89 = extractelement <16 x i64> %tmp574, i32 14
  %extract90 = extractelement <16 x i64> %tmp574, i32 15
  %63 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract75
  %64 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract76
  %65 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract77
  %66 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract78
  %67 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract79
  %68 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract80
  %69 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract81
  %70 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract82
  %71 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract83
  %72 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract84
  %73 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract85
  %74 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract86
  %75 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract87
  %76 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract88
  %77 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract89
  %78 = getelementptr i32 addrspace(1)* %unsortedData, i64 %extract90
  %79 = load i32 addrspace(1)* %63, align 4
  %80 = load i32 addrspace(1)* %64, align 4
  %81 = load i32 addrspace(1)* %65, align 4
  %82 = load i32 addrspace(1)* %66, align 4
  %83 = load i32 addrspace(1)* %67, align 4
  %84 = load i32 addrspace(1)* %68, align 4
  %85 = load i32 addrspace(1)* %69, align 4
  %86 = load i32 addrspace(1)* %70, align 4
  %87 = load i32 addrspace(1)* %71, align 4
  %88 = load i32 addrspace(1)* %72, align 4
  %89 = load i32 addrspace(1)* %73, align 4
  %90 = load i32 addrspace(1)* %74, align 4
  %91 = load i32 addrspace(1)* %75, align 4
  %92 = load i32 addrspace(1)* %76, align 4
  %93 = load i32 addrspace(1)* %77, align 4
  %94 = load i32 addrspace(1)* %78, align 4
  %temp.vect91 = insertelement <16 x i32> undef, i32 %79, i32 0
  %temp.vect92 = insertelement <16 x i32> %temp.vect91, i32 %80, i32 1
  %temp.vect93 = insertelement <16 x i32> %temp.vect92, i32 %81, i32 2
  %temp.vect94 = insertelement <16 x i32> %temp.vect93, i32 %82, i32 3
  %temp.vect95 = insertelement <16 x i32> %temp.vect94, i32 %83, i32 4
  %temp.vect96 = insertelement <16 x i32> %temp.vect95, i32 %84, i32 5
  %temp.vect97 = insertelement <16 x i32> %temp.vect96, i32 %85, i32 6
  %temp.vect98 = insertelement <16 x i32> %temp.vect97, i32 %86, i32 7
  %temp.vect99 = insertelement <16 x i32> %temp.vect98, i32 %87, i32 8
  %temp.vect100 = insertelement <16 x i32> %temp.vect99, i32 %88, i32 9
  %temp.vect101 = insertelement <16 x i32> %temp.vect100, i32 %89, i32 10
  %temp.vect102 = insertelement <16 x i32> %temp.vect101, i32 %90, i32 11
  %temp.vect103 = insertelement <16 x i32> %temp.vect102, i32 %91, i32 12
  %temp.vect104 = insertelement <16 x i32> %temp.vect103, i32 %92, i32 13
  %temp.vect105 = insertelement <16 x i32> %temp.vect104, i32 %93, i32 14
  %temp.vect106 = insertelement <16 x i32> %temp.vect105, i32 %94, i32 15
  %95 = lshr <16 x i32> %temp.vect106, %vector108
  %96 = and <16 x i32> %95, <i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255>
  %97 = zext <16 x i32> %96 to <16 x i64>
  %98 = or <16 x i64> %97, %61
  %extract109 = extractelement <16 x i64> %98, i32 0
  %extract110 = extractelement <16 x i64> %98, i32 1
  %extract111 = extractelement <16 x i64> %98, i32 2
  %extract112 = extractelement <16 x i64> %98, i32 3
  %extract113 = extractelement <16 x i64> %98, i32 4
  %extract114 = extractelement <16 x i64> %98, i32 5
  %extract115 = extractelement <16 x i64> %98, i32 6
  %extract116 = extractelement <16 x i64> %98, i32 7
  %extract117 = extractelement <16 x i64> %98, i32 8
  %extract118 = extractelement <16 x i64> %98, i32 9
  %extract119 = extractelement <16 x i64> %98, i32 10
  %extract120 = extractelement <16 x i64> %98, i32 11
  %extract121 = extractelement <16 x i64> %98, i32 12
  %extract122 = extractelement <16 x i64> %98, i32 13
  %extract123 = extractelement <16 x i64> %98, i32 14
  %extract124 = extractelement <16 x i64> %98, i32 15
  %99 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract109
  %100 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract110
  %101 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract111
  %102 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract112
  %103 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract113
  %104 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract114
  %105 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract115
  %106 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract116
  %107 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract117
  %108 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract118
  %109 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract119
  %110 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract120
  %111 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract121
  %112 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract122
  %113 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract123
  %114 = getelementptr inbounds i16 addrspace(3)* %sharedBuckets, i64 %extract124
  %115 = load i16 addrspace(3)* %99, align 2
  %116 = load i16 addrspace(3)* %100, align 2
  %117 = load i16 addrspace(3)* %101, align 2
  %118 = load i16 addrspace(3)* %102, align 2
  %119 = load i16 addrspace(3)* %103, align 2
  %120 = load i16 addrspace(3)* %104, align 2
  %121 = load i16 addrspace(3)* %105, align 2
  %122 = load i16 addrspace(3)* %106, align 2
  %123 = load i16 addrspace(3)* %107, align 2
  %124 = load i16 addrspace(3)* %108, align 2
  %125 = load i16 addrspace(3)* %109, align 2
  %126 = load i16 addrspace(3)* %110, align 2
  %127 = load i16 addrspace(3)* %111, align 2
  %128 = load i16 addrspace(3)* %112, align 2
  %129 = load i16 addrspace(3)* %113, align 2
  %130 = load i16 addrspace(3)* %114, align 2
  %temp.vect125 = insertelement <16 x i16> undef, i16 %115, i32 0
  %temp.vect126 = insertelement <16 x i16> %temp.vect125, i16 %116, i32 1
  %temp.vect127 = insertelement <16 x i16> %temp.vect126, i16 %117, i32 2
  %temp.vect128 = insertelement <16 x i16> %temp.vect127, i16 %118, i32 3
  %temp.vect129 = insertelement <16 x i16> %temp.vect128, i16 %119, i32 4
  %temp.vect130 = insertelement <16 x i16> %temp.vect129, i16 %120, i32 5
  %temp.vect131 = insertelement <16 x i16> %temp.vect130, i16 %121, i32 6
  %temp.vect132 = insertelement <16 x i16> %temp.vect131, i16 %122, i32 7
  %temp.vect133 = insertelement <16 x i16> %temp.vect132, i16 %123, i32 8
  %temp.vect134 = insertelement <16 x i16> %temp.vect133, i16 %124, i32 9
  %temp.vect135 = insertelement <16 x i16> %temp.vect134, i16 %125, i32 10
  %temp.vect136 = insertelement <16 x i16> %temp.vect135, i16 %126, i32 11
  %temp.vect137 = insertelement <16 x i16> %temp.vect136, i16 %127, i32 12
  %temp.vect138 = insertelement <16 x i16> %temp.vect137, i16 %128, i32 13
  %temp.vect139 = insertelement <16 x i16> %temp.vect138, i16 %129, i32 14
  %temp.vect140 = insertelement <16 x i16> %temp.vect139, i16 %130, i32 15
  %131 = zext i16 %115 to i64
  %132 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %131
  %133 = zext i16 %116 to i64
  %134 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %133
  %135 = zext i16 %117 to i64
  %136 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %135
  %137 = zext i16 %118 to i64
  %138 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %137
  %139 = zext i16 %119 to i64
  %140 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %139
  %141 = zext i16 %120 to i64
  %142 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %141
  %143 = zext i16 %121 to i64
  %144 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %143
  %145 = zext i16 %122 to i64
  %146 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %145
  %147 = zext i16 %123 to i64
  %148 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %147
  %149 = zext i16 %124 to i64
  %150 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %149
  %151 = zext i16 %125 to i64
  %152 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %151
  %153 = zext i16 %126 to i64
  %154 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %153
  %155 = zext i16 %127 to i64
  %156 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %155
  %157 = zext i16 %128 to i64
  %158 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %157
  %159 = zext i16 %129 to i64
  %160 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %159
  %161 = zext i16 %130 to i64
  %162 = getelementptr inbounds i32 addrspace(1)* %sortedData, i64 %161
  store i32 %79, i32 addrspace(1)* %132, align 4
  store i32 %80, i32 addrspace(1)* %134, align 4
  store i32 %81, i32 addrspace(1)* %136, align 4
  store i32 %82, i32 addrspace(1)* %138, align 4
  store i32 %83, i32 addrspace(1)* %140, align 4
  store i32 %84, i32 addrspace(1)* %142, align 4
  store i32 %85, i32 addrspace(1)* %144, align 4
  store i32 %86, i32 addrspace(1)* %146, align 4
  store i32 %87, i32 addrspace(1)* %148, align 4
  store i32 %88, i32 addrspace(1)* %150, align 4
  store i32 %89, i32 addrspace(1)* %152, align 4
  store i32 %90, i32 addrspace(1)* %154, align 4
  store i32 %91, i32 addrspace(1)* %156, align 4
  store i32 %92, i32 addrspace(1)* %158, align 4
  store i32 %93, i32 addrspace(1)* %160, align 4
  store i32 %94, i32 addrspace(1)* %162, align 4
  %163 = add <16 x i16> %temp.vect140, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %extract157 = extractelement <16 x i16> %163, i32 0
  %extract158 = extractelement <16 x i16> %163, i32 1
  %extract159 = extractelement <16 x i16> %163, i32 2
  %extract160 = extractelement <16 x i16> %163, i32 3
  %extract161 = extractelement <16 x i16> %163, i32 4
  %extract162 = extractelement <16 x i16> %163, i32 5
  %extract163 = extractelement <16 x i16> %163, i32 6
  %extract164 = extractelement <16 x i16> %163, i32 7
  %extract165 = extractelement <16 x i16> %163, i32 8
  %extract166 = extractelement <16 x i16> %163, i32 9
  %extract167 = extractelement <16 x i16> %163, i32 10
  %extract168 = extractelement <16 x i16> %163, i32 11
  %extract169 = extractelement <16 x i16> %163, i32 12
  %extract170 = extractelement <16 x i16> %163, i32 13
  %extract171 = extractelement <16 x i16> %163, i32 14
  %extract172 = extractelement <16 x i16> %163, i32 15
  store i16 %extract157, i16 addrspace(3)* %99, align 2
  store i16 %extract158, i16 addrspace(3)* %100, align 2
  store i16 %extract159, i16 addrspace(3)* %101, align 2
  store i16 %extract160, i16 addrspace(3)* %102, align 2
  store i16 %extract161, i16 addrspace(3)* %103, align 2
  store i16 %extract162, i16 addrspace(3)* %104, align 2
  store i16 %extract163, i16 addrspace(3)* %105, align 2
  store i16 %extract164, i16 addrspace(3)* %106, align 2
  store i16 %extract165, i16 addrspace(3)* %107, align 2
  store i16 %extract166, i16 addrspace(3)* %108, align 2
  store i16 %extract167, i16 addrspace(3)* %109, align 2
  store i16 %extract168, i16 addrspace(3)* %110, align 2
  store i16 %extract169, i16 addrspace(3)* %111, align 2
  store i16 %extract170, i16 addrspace(3)* %112, align 2
  store i16 %extract171, i16 addrspace(3)* %113, align 2
  store i16 %extract172, i16 addrspace(3)* %114, align 2
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, 256
  br i1 %exitcond, label %._crit_edge, label %62

._crit_edge:                                      ; preds = %62
  %check.WI.iter194 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter194, label %thenBB191, label %SyncBB189

thenBB191:                                        ; preds = %._crit_edge
  %"CurrWI++195" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride197" = add nuw i64 %CurrSBIndex..1, 640
  br label %SyncBB

SyncBB189:                                        ; preds = %._crit_edge
  ret void
}

define void @histogram(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i16 addrspace(3)**
  %10 = load i16 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to i64**
  %13 = load i64** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  br label %SyncBB44.i

SyncBB44.i:                                       ; preds = %thenBB47.i, %entry
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride53.i", %thenBB47.i ], [ 0, %entry ]
  %CurrWI..1.i = phi i64 [ %"CurrWI++51.i", %thenBB47.i ], [ 0, %entry ]
  %26 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..1.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..1.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %27, i64* %CastToValueType.i, align 8
  %28 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..1.i, i32 0, i64 0
  %29 = load i64* %28, align 8
  %30 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 0
  %31 = load i64* %30, align 8
  %32 = add i64 %29, %31
  %"&(pSB[currWI].offset)341.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset35.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)341.i"
  %CastToValueType36.i = bitcast i8* %"&pSB[currWI].offset35.i" to i64*
  store i64 %32, i64* %CastToValueType36.i, align 8
  %33 = load i64* %13, align 8
  %tmp21.i = shl i64 %27, 8
  br label %34

; <label>:34                                      ; preds = %34, %SyncBB44.i
  %indvar18.i = phi i64 [ 0, %SyncBB44.i ], [ %indvar.next19.i, %34 ]
  %tmp22.i = add i64 %tmp21.i, %indvar18.i
  %scevgep23.i = getelementptr i16 addrspace(3)* %10, i64 %tmp22.i
  store i16 0, i16 addrspace(3)* %scevgep23.i, align 2
  %indvar.next19.i = add i64 %indvar18.i, 1
  %exitcond20.i = icmp eq i64 %indvar.next19.i, 256
  br i1 %exitcond20.i, label %bb.nph3.i, label %34

bb.nph3.i:                                        ; preds = %34
  %check.WI.iter50.i = icmp ult i64 %CurrWI..1.i, %22
  br i1 %check.WI.iter50.i, label %thenBB47.i, label %SyncBB43.i

thenBB47.i:                                       ; preds = %bb.nph3.i
  %"CurrWI++51.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride53.i" = add nuw i64 %CurrSBIndex..1.i, 640
  br label %SyncBB44.i

SyncBB43.i:                                       ; preds = %thenBB.i, %bb.nph3.i
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %bb.nph3.i ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %bb.nph3.i ]
  %"&pSB[currWI].offset30.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..0.i
  %CastToValueType31.i = bitcast i8* %"&pSB[currWI].offset30.i" to i64*
  %loadedValue32.i = load i64* %CastToValueType31.i, align 8
  %35 = shl i64 %loadedValue32.i, 8
  %"&(pSB[currWI].offset)382.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)382.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to i64*
  %loadedValue41.i = load i64* %CastToValueType40.i, align 8
  %tmp15.i = shl i64 %loadedValue41.i, 8
  br label %36

; <label>:36                                      ; preds = %36, %SyncBB43.i
  %indvar12.i = phi i64 [ 0, %SyncBB43.i ], [ %indvar.next13.i, %36 ]
  %tmp16.i = add i64 %tmp15.i, %indvar12.i
  %scevgep17.i = getelementptr i32 addrspace(1)* %1, i64 %tmp16.i
  %37 = load i32 addrspace(1)* %scevgep17.i, align 4
  %38 = lshr i32 %37, %7
  %39 = and i32 %38, 255
  %40 = zext i32 %39 to i64
  %41 = or i64 %40, %35
  %42 = getelementptr inbounds i16 addrspace(3)* %10, i64 %41
  %43 = load i16 addrspace(3)* %42, align 2
  %44 = add i16 %43, 1
  store i16 %44, i16 addrspace(3)* %42, align 2
  %indvar.next13.i = add i64 %indvar12.i, 1
  %exitcond14.i = icmp eq i64 %indvar.next13.i, 256
  br i1 %exitcond14.i, label %bb.nph.i, label %36

bb.nph.i:                                         ; preds = %36
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %bb.nph.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 640
  br label %SyncBB43.i

elseBB.i:                                         ; preds = %bb.nph.i
  %tmp9.i = shl i64 %33, 12
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB54.i, %elseBB.i
  %CurrSBIndex..2.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride60.i", %thenBB54.i ]
  %CurrWI..2.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++58.i", %thenBB54.i ]
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..2.i
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to i64*
  %loadedValue.i = load i64* %CastToValueType27.i, align 8
  %tmp.i = shl i64 %loadedValue.i, 8
  %tmp10.i = add i64 %tmp9.i, %tmp.i
  br label %45

; <label>:45                                      ; preds = %45, %SyncBB.i
  %indvar.i = phi i64 [ 0, %SyncBB.i ], [ %indvar.next.i, %45 ]
  %tmp8.i = add i64 %tmp.i, %indvar.i
  %scevgep.i = getelementptr i16 addrspace(3)* %10, i64 %tmp8.i
  %tmp11.i = add i64 %tmp10.i, %indvar.i
  %46 = load i16 addrspace(3)* %scevgep.i, align 2
  %47 = zext i16 %46 to i32
  %48 = and i64 %tmp11.i, 4294967295
  %49 = getelementptr inbounds i32 addrspace(1)* %4, i64 %48
  store i32 %47, i32 addrspace(1)* %49, align 4
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, 256
  br i1 %exitcond.i, label %._crit_edge.i, label %45

._crit_edge.i:                                    ; preds = %45
  %check.WI.iter57.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter57.i, label %thenBB54.i, label %__histogram_separated_args.exit

thenBB54.i:                                       ; preds = %._crit_edge.i
  %"CurrWI++58.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride60.i" = add nuw i64 %CurrSBIndex..2.i, 640
  br label %SyncBB.i

__histogram_separated_args.exit:                  ; preds = %._crit_edge.i
  ret void
}

define void @permute(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i16 addrspace(3)**
  %10 = load i16 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32 addrspace(1)**
  %13 = load i32 addrspace(1)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 96
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  br label %SyncBB28.i

SyncBB28.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %29 = load i64* %16, align 8
  %30 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %31 = load i64* %30, align 8
  %"&(pSB[currWI].offset)1.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)1.i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %31, i64* %CastToValueType.i, align 8
  %32 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %33 = load i64* %32, align 8
  %34 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %35 = load i64* %34, align 8
  %36 = add i64 %33, %35
  %"&(pSB[currWI].offset)202.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)202.i"
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i64*
  store i64 %36, i64* %CastToValueType22.i, align 8
  %tmp9.i = shl i64 %31, 8
  %tmp12.i = shl i64 %29, 12
  %tmp13.i = add i64 %tmp12.i, %tmp9.i
  br label %37

; <label>:37                                      ; preds = %37, %SyncBB28.i
  %indvar6.i = phi i64 [ 0, %SyncBB28.i ], [ %indvar.next7.i, %37 ]
  %tmp10.i = add i64 %tmp9.i, %indvar6.i
  %scevgep11.i = getelementptr i16 addrspace(3)* %10, i64 %tmp10.i
  %tmp14.i = add i64 %tmp13.i, %indvar6.i
  %38 = and i64 %tmp14.i, 4294967295
  %39 = getelementptr inbounds i32 addrspace(1)* %4, i64 %38
  %40 = load i32 addrspace(1)* %39, align 4
  %41 = trunc i32 %40 to i16
  store i16 %41, i16 addrspace(3)* %scevgep11.i, align 2
  %indvar.next7.i = add i64 %indvar6.i, 1
  %exitcond8.i = icmp eq i64 %indvar.next7.i, 256
  br i1 %exitcond8.i, label %bb.nph.i, label %37

bb.nph.i:                                         ; preds = %37
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %bb.nph.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 640
  br label %SyncBB28.i

SyncBB.i:                                         ; preds = %thenBB31.i, %bb.nph.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++35.i", %thenBB31.i ], [ 0, %bb.nph.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride37.i", %thenBB31.i ], [ 0, %bb.nph.i ]
  %"&(pSB[currWI].offset)163.i" = or i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset17.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)163.i"
  %CastToValueType18.i = bitcast i8* %"&pSB[currWI].offset17.i" to i64*
  %loadedValue.i = load i64* %CastToValueType18.i, align 8
  %42 = shl i64 %loadedValue.i, 8
  %"&(pSB[currWI].offset)244.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset25.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)244.i"
  %CastToValueType26.i = bitcast i8* %"&pSB[currWI].offset25.i" to i64*
  %loadedValue27.i = load i64* %CastToValueType26.i, align 8
  %tmp.i = shl i64 %loadedValue27.i, 8
  br label %43

; <label>:43                                      ; preds = %43, %SyncBB.i
  %indvar.i = phi i64 [ 0, %SyncBB.i ], [ %indvar.next.i, %43 ]
  %tmp5.i = add i64 %tmp.i, %indvar.i
  %scevgep.i = getelementptr i32 addrspace(1)* %1, i64 %tmp5.i
  %44 = load i32 addrspace(1)* %scevgep.i, align 4
  %45 = lshr i32 %44, %7
  %46 = and i32 %45, 255
  %47 = zext i32 %46 to i64
  %48 = or i64 %47, %42
  %49 = getelementptr inbounds i16 addrspace(3)* %10, i64 %48
  %50 = load i16 addrspace(3)* %49, align 2
  %51 = zext i16 %50 to i64
  %52 = getelementptr inbounds i32 addrspace(1)* %13, i64 %51
  store i32 %44, i32 addrspace(1)* %52, align 4
  %53 = add i16 %50, 1
  store i16 %53, i16 addrspace(3)* %49, align 2
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, 256
  br i1 %exitcond.i, label %._crit_edge.i, label %43

._crit_edge.i:                                    ; preds = %43
  %check.WI.iter34.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter34.i, label %thenBB31.i, label %__permute_separated_args.exit

thenBB31.i:                                       ; preds = %._crit_edge.i
  %"CurrWI++35.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride37.i" = add nuw i64 %CurrSBIndex..1.i, 640
  br label %SyncBB.i

__permute_separated_args.exit:                    ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.permute(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i16 addrspace(3)**
  %10 = load i16 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32 addrspace(1)**
  %13 = load i32 addrspace(1)** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 96
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  br label %SyncBB188.i

SyncBB188.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %29 = load i64* %16, align 8
  %30 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %31 = load i64* %30, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %31, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %32 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 384
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i64>*
  store <16 x i64> %32, <16 x i64>* %CastToValueType.i, align 128
  %33 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %34 = load i64* %33, align 8
  %35 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %36 = load i64* %35, align 8
  %37 = add i64 %34, %36
  %broadcast11.i = insertelement <16 x i64> undef, i64 %37, i32 0
  %broadcast22.i = shufflevector <16 x i64> %broadcast11.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %38 = add <16 x i64> %broadcast22.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)178.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset179.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)178.i"
  %CastToValueType180.i = bitcast i8* %"&pSB[currWI].offset179.i" to <16 x i64>*
  store <16 x i64> %38, <16 x i64>* %CastToValueType180.i, align 128
  %tmp93.i = shl <16 x i64> %32, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %tmp12.i = shl i64 %29, 12
  %temp.i = insertelement <16 x i64> undef, i64 %tmp12.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp134.i = add <16 x i64> %vector.i, %tmp93.i
  br label %39

; <label>:39                                      ; preds = %39, %SyncBB188.i
  %indvar6.i = phi i64 [ 0, %SyncBB188.i ], [ %indvar.next7.i, %39 ]
  %temp5.i = insertelement <16 x i64> undef, i64 %indvar6.i, i32 0
  %vector6.i = shufflevector <16 x i64> %temp5.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp107.i = add <16 x i64> %tmp93.i, %vector6.i
  %extract.i = extractelement <16 x i64> %tmp107.i, i32 0
  %extract8.i = extractelement <16 x i64> %tmp107.i, i32 1
  %extract9.i = extractelement <16 x i64> %tmp107.i, i32 2
  %extract10.i = extractelement <16 x i64> %tmp107.i, i32 3
  %extract11.i = extractelement <16 x i64> %tmp107.i, i32 4
  %extract12.i = extractelement <16 x i64> %tmp107.i, i32 5
  %extract13.i = extractelement <16 x i64> %tmp107.i, i32 6
  %extract14.i = extractelement <16 x i64> %tmp107.i, i32 7
  %extract15.i = extractelement <16 x i64> %tmp107.i, i32 8
  %extract16.i = extractelement <16 x i64> %tmp107.i, i32 9
  %extract17.i = extractelement <16 x i64> %tmp107.i, i32 10
  %extract18.i = extractelement <16 x i64> %tmp107.i, i32 11
  %extract19.i = extractelement <16 x i64> %tmp107.i, i32 12
  %extract20.i = extractelement <16 x i64> %tmp107.i, i32 13
  %extract21.i = extractelement <16 x i64> %tmp107.i, i32 14
  %extract22.i = extractelement <16 x i64> %tmp107.i, i32 15
  %40 = getelementptr i16 addrspace(3)* %10, i64 %extract.i
  %41 = getelementptr i16 addrspace(3)* %10, i64 %extract8.i
  %42 = getelementptr i16 addrspace(3)* %10, i64 %extract9.i
  %43 = getelementptr i16 addrspace(3)* %10, i64 %extract10.i
  %44 = getelementptr i16 addrspace(3)* %10, i64 %extract11.i
  %45 = getelementptr i16 addrspace(3)* %10, i64 %extract12.i
  %46 = getelementptr i16 addrspace(3)* %10, i64 %extract13.i
  %47 = getelementptr i16 addrspace(3)* %10, i64 %extract14.i
  %48 = getelementptr i16 addrspace(3)* %10, i64 %extract15.i
  %49 = getelementptr i16 addrspace(3)* %10, i64 %extract16.i
  %50 = getelementptr i16 addrspace(3)* %10, i64 %extract17.i
  %51 = getelementptr i16 addrspace(3)* %10, i64 %extract18.i
  %52 = getelementptr i16 addrspace(3)* %10, i64 %extract19.i
  %53 = getelementptr i16 addrspace(3)* %10, i64 %extract20.i
  %54 = getelementptr i16 addrspace(3)* %10, i64 %extract21.i
  %55 = getelementptr i16 addrspace(3)* %10, i64 %extract22.i
  %tmp1423.i = add <16 x i64> %tmp134.i, %vector6.i
  %56 = and <16 x i64> %tmp1423.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract24.i = extractelement <16 x i64> %56, i32 0
  %extract25.i = extractelement <16 x i64> %56, i32 1
  %extract26.i = extractelement <16 x i64> %56, i32 2
  %extract27.i = extractelement <16 x i64> %56, i32 3
  %extract28.i = extractelement <16 x i64> %56, i32 4
  %extract29.i = extractelement <16 x i64> %56, i32 5
  %extract30.i = extractelement <16 x i64> %56, i32 6
  %extract31.i = extractelement <16 x i64> %56, i32 7
  %extract32.i = extractelement <16 x i64> %56, i32 8
  %extract33.i = extractelement <16 x i64> %56, i32 9
  %extract34.i = extractelement <16 x i64> %56, i32 10
  %extract35.i = extractelement <16 x i64> %56, i32 11
  %extract36.i = extractelement <16 x i64> %56, i32 12
  %extract37.i = extractelement <16 x i64> %56, i32 13
  %extract38.i = extractelement <16 x i64> %56, i32 14
  %extract39.i = extractelement <16 x i64> %56, i32 15
  %57 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract24.i
  %58 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract25.i
  %59 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract26.i
  %60 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract27.i
  %61 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract28.i
  %62 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract29.i
  %63 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract30.i
  %64 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract31.i
  %65 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract32.i
  %66 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract33.i
  %67 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract34.i
  %68 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract35.i
  %69 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract36.i
  %70 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract37.i
  %71 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract38.i
  %72 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract39.i
  %73 = load i32 addrspace(1)* %57, align 4
  %74 = load i32 addrspace(1)* %58, align 4
  %75 = load i32 addrspace(1)* %59, align 4
  %76 = load i32 addrspace(1)* %60, align 4
  %77 = load i32 addrspace(1)* %61, align 4
  %78 = load i32 addrspace(1)* %62, align 4
  %79 = load i32 addrspace(1)* %63, align 4
  %80 = load i32 addrspace(1)* %64, align 4
  %81 = load i32 addrspace(1)* %65, align 4
  %82 = load i32 addrspace(1)* %66, align 4
  %83 = load i32 addrspace(1)* %67, align 4
  %84 = load i32 addrspace(1)* %68, align 4
  %85 = load i32 addrspace(1)* %69, align 4
  %86 = load i32 addrspace(1)* %70, align 4
  %87 = load i32 addrspace(1)* %71, align 4
  %88 = load i32 addrspace(1)* %72, align 4
  %temp.vect.i = insertelement <16 x i32> undef, i32 %73, i32 0
  %temp.vect40.i = insertelement <16 x i32> %temp.vect.i, i32 %74, i32 1
  %temp.vect41.i = insertelement <16 x i32> %temp.vect40.i, i32 %75, i32 2
  %temp.vect42.i = insertelement <16 x i32> %temp.vect41.i, i32 %76, i32 3
  %temp.vect43.i = insertelement <16 x i32> %temp.vect42.i, i32 %77, i32 4
  %temp.vect44.i = insertelement <16 x i32> %temp.vect43.i, i32 %78, i32 5
  %temp.vect45.i = insertelement <16 x i32> %temp.vect44.i, i32 %79, i32 6
  %temp.vect46.i = insertelement <16 x i32> %temp.vect45.i, i32 %80, i32 7
  %temp.vect47.i = insertelement <16 x i32> %temp.vect46.i, i32 %81, i32 8
  %temp.vect48.i = insertelement <16 x i32> %temp.vect47.i, i32 %82, i32 9
  %temp.vect49.i = insertelement <16 x i32> %temp.vect48.i, i32 %83, i32 10
  %temp.vect50.i = insertelement <16 x i32> %temp.vect49.i, i32 %84, i32 11
  %temp.vect51.i = insertelement <16 x i32> %temp.vect50.i, i32 %85, i32 12
  %temp.vect52.i = insertelement <16 x i32> %temp.vect51.i, i32 %86, i32 13
  %temp.vect53.i = insertelement <16 x i32> %temp.vect52.i, i32 %87, i32 14
  %temp.vect54.i = insertelement <16 x i32> %temp.vect53.i, i32 %88, i32 15
  %89 = trunc <16 x i32> %temp.vect54.i to <16 x i16>
  %extract55.i = extractelement <16 x i16> %89, i32 0
  %extract56.i = extractelement <16 x i16> %89, i32 1
  %extract57.i = extractelement <16 x i16> %89, i32 2
  %extract58.i = extractelement <16 x i16> %89, i32 3
  %extract59.i = extractelement <16 x i16> %89, i32 4
  %extract60.i = extractelement <16 x i16> %89, i32 5
  %extract61.i = extractelement <16 x i16> %89, i32 6
  %extract62.i = extractelement <16 x i16> %89, i32 7
  %extract63.i = extractelement <16 x i16> %89, i32 8
  %extract64.i = extractelement <16 x i16> %89, i32 9
  %extract65.i = extractelement <16 x i16> %89, i32 10
  %extract66.i = extractelement <16 x i16> %89, i32 11
  %extract67.i = extractelement <16 x i16> %89, i32 12
  %extract68.i = extractelement <16 x i16> %89, i32 13
  %extract69.i = extractelement <16 x i16> %89, i32 14
  %extract70.i = extractelement <16 x i16> %89, i32 15
  store i16 %extract55.i, i16 addrspace(3)* %40, align 2
  store i16 %extract56.i, i16 addrspace(3)* %41, align 2
  store i16 %extract57.i, i16 addrspace(3)* %42, align 2
  store i16 %extract58.i, i16 addrspace(3)* %43, align 2
  store i16 %extract59.i, i16 addrspace(3)* %44, align 2
  store i16 %extract60.i, i16 addrspace(3)* %45, align 2
  store i16 %extract61.i, i16 addrspace(3)* %46, align 2
  store i16 %extract62.i, i16 addrspace(3)* %47, align 2
  store i16 %extract63.i, i16 addrspace(3)* %48, align 2
  store i16 %extract64.i, i16 addrspace(3)* %49, align 2
  store i16 %extract65.i, i16 addrspace(3)* %50, align 2
  store i16 %extract66.i, i16 addrspace(3)* %51, align 2
  store i16 %extract67.i, i16 addrspace(3)* %52, align 2
  store i16 %extract68.i, i16 addrspace(3)* %53, align 2
  store i16 %extract69.i, i16 addrspace(3)* %54, align 2
  store i16 %extract70.i, i16 addrspace(3)* %55, align 2
  %indvar.next7.i = add i64 %indvar6.i, 1
  %exitcond8.i = icmp eq i64 %indvar.next7.i, 256
  br i1 %exitcond8.i, label %bb.nph.i, label %39

bb.nph.i:                                         ; preds = %39
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %bb.nph.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 640
  br label %SyncBB188.i

elseBB.i:                                         ; preds = %bb.nph.i
  %temp107.i = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector108.i = shufflevector <16 x i32> %temp107.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB191.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride197.i", %thenBB191.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++195.i", %thenBB191.i ]
  %"&(pSB[currWI].offset)174.i" = add nuw i64 %CurrSBIndex..1.i, 384
  %"&pSB[currWI].offset175.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)174.i"
  %CastToValueType176.i = bitcast i8* %"&pSB[currWI].offset175.i" to <16 x i64>*
  %loadedValue.i = load <16 x i64>* %CastToValueType176.i, align 128
  %90 = shl <16 x i64> %loadedValue.i, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %"&(pSB[currWI].offset)182.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset183.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)182.i"
  %CastToValueType184.i = bitcast i8* %"&pSB[currWI].offset183.i" to <16 x i64>*
  %loadedValue185.i = load <16 x i64>* %CastToValueType184.i, align 128
  %tmp71.i = shl <16 x i64> %loadedValue185.i, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  br label %91

; <label>:91                                      ; preds = %91, %SyncBB.i
  %indvar.i = phi i64 [ 0, %SyncBB.i ], [ %indvar.next.i, %91 ]
  %temp72.i = insertelement <16 x i64> undef, i64 %indvar.i, i32 0
  %vector73.i = shufflevector <16 x i64> %temp72.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp574.i = add <16 x i64> %tmp71.i, %vector73.i
  %extract75.i = extractelement <16 x i64> %tmp574.i, i32 0
  %extract76.i = extractelement <16 x i64> %tmp574.i, i32 1
  %extract77.i = extractelement <16 x i64> %tmp574.i, i32 2
  %extract78.i = extractelement <16 x i64> %tmp574.i, i32 3
  %extract79.i = extractelement <16 x i64> %tmp574.i, i32 4
  %extract80.i = extractelement <16 x i64> %tmp574.i, i32 5
  %extract81.i = extractelement <16 x i64> %tmp574.i, i32 6
  %extract82.i = extractelement <16 x i64> %tmp574.i, i32 7
  %extract83.i = extractelement <16 x i64> %tmp574.i, i32 8
  %extract84.i = extractelement <16 x i64> %tmp574.i, i32 9
  %extract85.i = extractelement <16 x i64> %tmp574.i, i32 10
  %extract86.i = extractelement <16 x i64> %tmp574.i, i32 11
  %extract87.i = extractelement <16 x i64> %tmp574.i, i32 12
  %extract88.i = extractelement <16 x i64> %tmp574.i, i32 13
  %extract89.i = extractelement <16 x i64> %tmp574.i, i32 14
  %extract90.i = extractelement <16 x i64> %tmp574.i, i32 15
  %92 = getelementptr i32 addrspace(1)* %1, i64 %extract75.i
  %93 = getelementptr i32 addrspace(1)* %1, i64 %extract76.i
  %94 = getelementptr i32 addrspace(1)* %1, i64 %extract77.i
  %95 = getelementptr i32 addrspace(1)* %1, i64 %extract78.i
  %96 = getelementptr i32 addrspace(1)* %1, i64 %extract79.i
  %97 = getelementptr i32 addrspace(1)* %1, i64 %extract80.i
  %98 = getelementptr i32 addrspace(1)* %1, i64 %extract81.i
  %99 = getelementptr i32 addrspace(1)* %1, i64 %extract82.i
  %100 = getelementptr i32 addrspace(1)* %1, i64 %extract83.i
  %101 = getelementptr i32 addrspace(1)* %1, i64 %extract84.i
  %102 = getelementptr i32 addrspace(1)* %1, i64 %extract85.i
  %103 = getelementptr i32 addrspace(1)* %1, i64 %extract86.i
  %104 = getelementptr i32 addrspace(1)* %1, i64 %extract87.i
  %105 = getelementptr i32 addrspace(1)* %1, i64 %extract88.i
  %106 = getelementptr i32 addrspace(1)* %1, i64 %extract89.i
  %107 = getelementptr i32 addrspace(1)* %1, i64 %extract90.i
  %108 = load i32 addrspace(1)* %92, align 4
  %109 = load i32 addrspace(1)* %93, align 4
  %110 = load i32 addrspace(1)* %94, align 4
  %111 = load i32 addrspace(1)* %95, align 4
  %112 = load i32 addrspace(1)* %96, align 4
  %113 = load i32 addrspace(1)* %97, align 4
  %114 = load i32 addrspace(1)* %98, align 4
  %115 = load i32 addrspace(1)* %99, align 4
  %116 = load i32 addrspace(1)* %100, align 4
  %117 = load i32 addrspace(1)* %101, align 4
  %118 = load i32 addrspace(1)* %102, align 4
  %119 = load i32 addrspace(1)* %103, align 4
  %120 = load i32 addrspace(1)* %104, align 4
  %121 = load i32 addrspace(1)* %105, align 4
  %122 = load i32 addrspace(1)* %106, align 4
  %123 = load i32 addrspace(1)* %107, align 4
  %temp.vect91.i = insertelement <16 x i32> undef, i32 %108, i32 0
  %temp.vect92.i = insertelement <16 x i32> %temp.vect91.i, i32 %109, i32 1
  %temp.vect93.i = insertelement <16 x i32> %temp.vect92.i, i32 %110, i32 2
  %temp.vect94.i = insertelement <16 x i32> %temp.vect93.i, i32 %111, i32 3
  %temp.vect95.i = insertelement <16 x i32> %temp.vect94.i, i32 %112, i32 4
  %temp.vect96.i = insertelement <16 x i32> %temp.vect95.i, i32 %113, i32 5
  %temp.vect97.i = insertelement <16 x i32> %temp.vect96.i, i32 %114, i32 6
  %temp.vect98.i = insertelement <16 x i32> %temp.vect97.i, i32 %115, i32 7
  %temp.vect99.i = insertelement <16 x i32> %temp.vect98.i, i32 %116, i32 8
  %temp.vect100.i = insertelement <16 x i32> %temp.vect99.i, i32 %117, i32 9
  %temp.vect101.i = insertelement <16 x i32> %temp.vect100.i, i32 %118, i32 10
  %temp.vect102.i = insertelement <16 x i32> %temp.vect101.i, i32 %119, i32 11
  %temp.vect103.i = insertelement <16 x i32> %temp.vect102.i, i32 %120, i32 12
  %temp.vect104.i = insertelement <16 x i32> %temp.vect103.i, i32 %121, i32 13
  %temp.vect105.i = insertelement <16 x i32> %temp.vect104.i, i32 %122, i32 14
  %temp.vect106.i = insertelement <16 x i32> %temp.vect105.i, i32 %123, i32 15
  %124 = lshr <16 x i32> %temp.vect106.i, %vector108.i
  %125 = and <16 x i32> %124, <i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255>
  %126 = zext <16 x i32> %125 to <16 x i64>
  %127 = or <16 x i64> %126, %90
  %extract109.i = extractelement <16 x i64> %127, i32 0
  %extract110.i = extractelement <16 x i64> %127, i32 1
  %extract111.i = extractelement <16 x i64> %127, i32 2
  %extract112.i = extractelement <16 x i64> %127, i32 3
  %extract113.i = extractelement <16 x i64> %127, i32 4
  %extract114.i = extractelement <16 x i64> %127, i32 5
  %extract115.i = extractelement <16 x i64> %127, i32 6
  %extract116.i = extractelement <16 x i64> %127, i32 7
  %extract117.i = extractelement <16 x i64> %127, i32 8
  %extract118.i = extractelement <16 x i64> %127, i32 9
  %extract119.i = extractelement <16 x i64> %127, i32 10
  %extract120.i = extractelement <16 x i64> %127, i32 11
  %extract121.i = extractelement <16 x i64> %127, i32 12
  %extract122.i = extractelement <16 x i64> %127, i32 13
  %extract123.i = extractelement <16 x i64> %127, i32 14
  %extract124.i = extractelement <16 x i64> %127, i32 15
  %128 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract109.i
  %129 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract110.i
  %130 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract111.i
  %131 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract112.i
  %132 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract113.i
  %133 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract114.i
  %134 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract115.i
  %135 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract116.i
  %136 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract117.i
  %137 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract118.i
  %138 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract119.i
  %139 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract120.i
  %140 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract121.i
  %141 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract122.i
  %142 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract123.i
  %143 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract124.i
  %144 = load i16 addrspace(3)* %128, align 2
  %145 = load i16 addrspace(3)* %129, align 2
  %146 = load i16 addrspace(3)* %130, align 2
  %147 = load i16 addrspace(3)* %131, align 2
  %148 = load i16 addrspace(3)* %132, align 2
  %149 = load i16 addrspace(3)* %133, align 2
  %150 = load i16 addrspace(3)* %134, align 2
  %151 = load i16 addrspace(3)* %135, align 2
  %152 = load i16 addrspace(3)* %136, align 2
  %153 = load i16 addrspace(3)* %137, align 2
  %154 = load i16 addrspace(3)* %138, align 2
  %155 = load i16 addrspace(3)* %139, align 2
  %156 = load i16 addrspace(3)* %140, align 2
  %157 = load i16 addrspace(3)* %141, align 2
  %158 = load i16 addrspace(3)* %142, align 2
  %159 = load i16 addrspace(3)* %143, align 2
  %temp.vect125.i = insertelement <16 x i16> undef, i16 %144, i32 0
  %temp.vect126.i = insertelement <16 x i16> %temp.vect125.i, i16 %145, i32 1
  %temp.vect127.i = insertelement <16 x i16> %temp.vect126.i, i16 %146, i32 2
  %temp.vect128.i = insertelement <16 x i16> %temp.vect127.i, i16 %147, i32 3
  %temp.vect129.i = insertelement <16 x i16> %temp.vect128.i, i16 %148, i32 4
  %temp.vect130.i = insertelement <16 x i16> %temp.vect129.i, i16 %149, i32 5
  %temp.vect131.i = insertelement <16 x i16> %temp.vect130.i, i16 %150, i32 6
  %temp.vect132.i = insertelement <16 x i16> %temp.vect131.i, i16 %151, i32 7
  %temp.vect133.i = insertelement <16 x i16> %temp.vect132.i, i16 %152, i32 8
  %temp.vect134.i = insertelement <16 x i16> %temp.vect133.i, i16 %153, i32 9
  %temp.vect135.i = insertelement <16 x i16> %temp.vect134.i, i16 %154, i32 10
  %temp.vect136.i = insertelement <16 x i16> %temp.vect135.i, i16 %155, i32 11
  %temp.vect137.i = insertelement <16 x i16> %temp.vect136.i, i16 %156, i32 12
  %temp.vect138.i = insertelement <16 x i16> %temp.vect137.i, i16 %157, i32 13
  %temp.vect139.i = insertelement <16 x i16> %temp.vect138.i, i16 %158, i32 14
  %temp.vect140.i = insertelement <16 x i16> %temp.vect139.i, i16 %159, i32 15
  %160 = zext i16 %144 to i64
  %161 = getelementptr inbounds i32 addrspace(1)* %13, i64 %160
  %162 = zext i16 %145 to i64
  %163 = getelementptr inbounds i32 addrspace(1)* %13, i64 %162
  %164 = zext i16 %146 to i64
  %165 = getelementptr inbounds i32 addrspace(1)* %13, i64 %164
  %166 = zext i16 %147 to i64
  %167 = getelementptr inbounds i32 addrspace(1)* %13, i64 %166
  %168 = zext i16 %148 to i64
  %169 = getelementptr inbounds i32 addrspace(1)* %13, i64 %168
  %170 = zext i16 %149 to i64
  %171 = getelementptr inbounds i32 addrspace(1)* %13, i64 %170
  %172 = zext i16 %150 to i64
  %173 = getelementptr inbounds i32 addrspace(1)* %13, i64 %172
  %174 = zext i16 %151 to i64
  %175 = getelementptr inbounds i32 addrspace(1)* %13, i64 %174
  %176 = zext i16 %152 to i64
  %177 = getelementptr inbounds i32 addrspace(1)* %13, i64 %176
  %178 = zext i16 %153 to i64
  %179 = getelementptr inbounds i32 addrspace(1)* %13, i64 %178
  %180 = zext i16 %154 to i64
  %181 = getelementptr inbounds i32 addrspace(1)* %13, i64 %180
  %182 = zext i16 %155 to i64
  %183 = getelementptr inbounds i32 addrspace(1)* %13, i64 %182
  %184 = zext i16 %156 to i64
  %185 = getelementptr inbounds i32 addrspace(1)* %13, i64 %184
  %186 = zext i16 %157 to i64
  %187 = getelementptr inbounds i32 addrspace(1)* %13, i64 %186
  %188 = zext i16 %158 to i64
  %189 = getelementptr inbounds i32 addrspace(1)* %13, i64 %188
  %190 = zext i16 %159 to i64
  %191 = getelementptr inbounds i32 addrspace(1)* %13, i64 %190
  store i32 %108, i32 addrspace(1)* %161, align 4
  store i32 %109, i32 addrspace(1)* %163, align 4
  store i32 %110, i32 addrspace(1)* %165, align 4
  store i32 %111, i32 addrspace(1)* %167, align 4
  store i32 %112, i32 addrspace(1)* %169, align 4
  store i32 %113, i32 addrspace(1)* %171, align 4
  store i32 %114, i32 addrspace(1)* %173, align 4
  store i32 %115, i32 addrspace(1)* %175, align 4
  store i32 %116, i32 addrspace(1)* %177, align 4
  store i32 %117, i32 addrspace(1)* %179, align 4
  store i32 %118, i32 addrspace(1)* %181, align 4
  store i32 %119, i32 addrspace(1)* %183, align 4
  store i32 %120, i32 addrspace(1)* %185, align 4
  store i32 %121, i32 addrspace(1)* %187, align 4
  store i32 %122, i32 addrspace(1)* %189, align 4
  store i32 %123, i32 addrspace(1)* %191, align 4
  %192 = add <16 x i16> %temp.vect140.i, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %extract157.i = extractelement <16 x i16> %192, i32 0
  %extract158.i = extractelement <16 x i16> %192, i32 1
  %extract159.i = extractelement <16 x i16> %192, i32 2
  %extract160.i = extractelement <16 x i16> %192, i32 3
  %extract161.i = extractelement <16 x i16> %192, i32 4
  %extract162.i = extractelement <16 x i16> %192, i32 5
  %extract163.i = extractelement <16 x i16> %192, i32 6
  %extract164.i = extractelement <16 x i16> %192, i32 7
  %extract165.i = extractelement <16 x i16> %192, i32 8
  %extract166.i = extractelement <16 x i16> %192, i32 9
  %extract167.i = extractelement <16 x i16> %192, i32 10
  %extract168.i = extractelement <16 x i16> %192, i32 11
  %extract169.i = extractelement <16 x i16> %192, i32 12
  %extract170.i = extractelement <16 x i16> %192, i32 13
  %extract171.i = extractelement <16 x i16> %192, i32 14
  %extract172.i = extractelement <16 x i16> %192, i32 15
  store i16 %extract157.i, i16 addrspace(3)* %128, align 2
  store i16 %extract158.i, i16 addrspace(3)* %129, align 2
  store i16 %extract159.i, i16 addrspace(3)* %130, align 2
  store i16 %extract160.i, i16 addrspace(3)* %131, align 2
  store i16 %extract161.i, i16 addrspace(3)* %132, align 2
  store i16 %extract162.i, i16 addrspace(3)* %133, align 2
  store i16 %extract163.i, i16 addrspace(3)* %134, align 2
  store i16 %extract164.i, i16 addrspace(3)* %135, align 2
  store i16 %extract165.i, i16 addrspace(3)* %136, align 2
  store i16 %extract166.i, i16 addrspace(3)* %137, align 2
  store i16 %extract167.i, i16 addrspace(3)* %138, align 2
  store i16 %extract168.i, i16 addrspace(3)* %139, align 2
  store i16 %extract169.i, i16 addrspace(3)* %140, align 2
  store i16 %extract170.i, i16 addrspace(3)* %141, align 2
  store i16 %extract171.i, i16 addrspace(3)* %142, align 2
  store i16 %extract172.i, i16 addrspace(3)* %143, align 2
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, 256
  br i1 %exitcond.i, label %._crit_edge.i, label %91

._crit_edge.i:                                    ; preds = %91
  %check.WI.iter194.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter194.i, label %thenBB191.i, label %____Vectorized_.permute_separated_args.exit

thenBB191.i:                                      ; preds = %._crit_edge.i
  %"CurrWI++195.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride197.i" = add nuw i64 %CurrSBIndex..1.i, 640
  br label %SyncBB.i

____Vectorized_.permute_separated_args.exit:      ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.histogram(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(1)**
  %4 = load i32 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i16 addrspace(3)**
  %10 = load i16 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to i64**
  %13 = load i64** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  br label %SyncBB199.i

SyncBB199.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %26 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %27, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %28 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i64>*
  store <16 x i64> %28, <16 x i64>* %CastToValueType.i, align 128
  %29 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = add i64 %30, %32
  %broadcast11.i = insertelement <16 x i64> undef, i64 %33, i32 0
  %broadcast22.i = shufflevector <16 x i64> %broadcast11.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %34 = add <16 x i64> %broadcast22.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)187.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset188.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)187.i"
  %CastToValueType189.i = bitcast i8* %"&pSB[currWI].offset188.i" to <16 x i64>*
  store <16 x i64> %34, <16 x i64>* %CastToValueType189.i, align 128
  %35 = load i64* %13, align 8
  %tmp213.i = shl <16 x i64> %28, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  br label %36

; <label>:36                                      ; preds = %36, %SyncBB199.i
  %indvar18.i = phi i64 [ 0, %SyncBB199.i ], [ %indvar.next19.i, %36 ]
  %temp.i = insertelement <16 x i64> undef, i64 %indvar18.i, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp224.i = add <16 x i64> %tmp213.i, %vector.i
  %extract.i = extractelement <16 x i64> %tmp224.i, i32 0
  %extract5.i = extractelement <16 x i64> %tmp224.i, i32 1
  %extract6.i = extractelement <16 x i64> %tmp224.i, i32 2
  %extract7.i = extractelement <16 x i64> %tmp224.i, i32 3
  %extract8.i = extractelement <16 x i64> %tmp224.i, i32 4
  %extract9.i = extractelement <16 x i64> %tmp224.i, i32 5
  %extract10.i = extractelement <16 x i64> %tmp224.i, i32 6
  %extract11.i = extractelement <16 x i64> %tmp224.i, i32 7
  %extract12.i = extractelement <16 x i64> %tmp224.i, i32 8
  %extract13.i = extractelement <16 x i64> %tmp224.i, i32 9
  %extract14.i = extractelement <16 x i64> %tmp224.i, i32 10
  %extract15.i = extractelement <16 x i64> %tmp224.i, i32 11
  %extract16.i = extractelement <16 x i64> %tmp224.i, i32 12
  %extract17.i = extractelement <16 x i64> %tmp224.i, i32 13
  %extract18.i = extractelement <16 x i64> %tmp224.i, i32 14
  %extract19.i = extractelement <16 x i64> %tmp224.i, i32 15
  %37 = getelementptr i16 addrspace(3)* %10, i64 %extract.i
  %38 = getelementptr i16 addrspace(3)* %10, i64 %extract5.i
  %39 = getelementptr i16 addrspace(3)* %10, i64 %extract6.i
  %40 = getelementptr i16 addrspace(3)* %10, i64 %extract7.i
  %41 = getelementptr i16 addrspace(3)* %10, i64 %extract8.i
  %42 = getelementptr i16 addrspace(3)* %10, i64 %extract9.i
  %43 = getelementptr i16 addrspace(3)* %10, i64 %extract10.i
  %44 = getelementptr i16 addrspace(3)* %10, i64 %extract11.i
  %45 = getelementptr i16 addrspace(3)* %10, i64 %extract12.i
  %46 = getelementptr i16 addrspace(3)* %10, i64 %extract13.i
  %47 = getelementptr i16 addrspace(3)* %10, i64 %extract14.i
  %48 = getelementptr i16 addrspace(3)* %10, i64 %extract15.i
  %49 = getelementptr i16 addrspace(3)* %10, i64 %extract16.i
  %50 = getelementptr i16 addrspace(3)* %10, i64 %extract17.i
  %51 = getelementptr i16 addrspace(3)* %10, i64 %extract18.i
  %52 = getelementptr i16 addrspace(3)* %10, i64 %extract19.i
  store i16 0, i16 addrspace(3)* %37, align 2
  store i16 0, i16 addrspace(3)* %38, align 2
  store i16 0, i16 addrspace(3)* %39, align 2
  store i16 0, i16 addrspace(3)* %40, align 2
  store i16 0, i16 addrspace(3)* %41, align 2
  store i16 0, i16 addrspace(3)* %42, align 2
  store i16 0, i16 addrspace(3)* %43, align 2
  store i16 0, i16 addrspace(3)* %44, align 2
  store i16 0, i16 addrspace(3)* %45, align 2
  store i16 0, i16 addrspace(3)* %46, align 2
  store i16 0, i16 addrspace(3)* %47, align 2
  store i16 0, i16 addrspace(3)* %48, align 2
  store i16 0, i16 addrspace(3)* %49, align 2
  store i16 0, i16 addrspace(3)* %50, align 2
  store i16 0, i16 addrspace(3)* %51, align 2
  store i16 0, i16 addrspace(3)* %52, align 2
  %indvar.next19.i = add i64 %indvar18.i, 1
  %exitcond20.i = icmp eq i64 %indvar.next19.i, 256
  br i1 %exitcond20.i, label %bb.nph3.i, label %36

bb.nph3.i:                                        ; preds = %36
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %bb.nph3.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 640
  br label %SyncBB199.i

elseBB.i:                                         ; preds = %bb.nph3.i
  %temp55.i = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector56.i = shufflevector <16 x i32> %temp55.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB202.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride208.i", %thenBB202.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++206.i", %thenBB202.i ]
  %"&(pSB[currWI].offset)182.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset183.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)182.i"
  %CastToValueType184.i = bitcast i8* %"&pSB[currWI].offset183.i" to <16 x i64>*
  %loadedValue185.i = load <16 x i64>* %CastToValueType184.i, align 128
  %53 = shl <16 x i64> %loadedValue185.i, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %"&(pSB[currWI].offset)191.i" = add nuw i64 %CurrSBIndex..1.i, 256
  %"&pSB[currWI].offset192.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)191.i"
  %CastToValueType193.i = bitcast i8* %"&pSB[currWI].offset192.i" to <16 x i64>*
  %loadedValue194.i = load <16 x i64>* %CastToValueType193.i, align 128
  %tmp1520.i = shl <16 x i64> %loadedValue194.i, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  br label %54

; <label>:54                                      ; preds = %54, %SyncBB.i
  %indvar12.i = phi i64 [ 0, %SyncBB.i ], [ %indvar.next13.i, %54 ]
  %temp21.i = insertelement <16 x i64> undef, i64 %indvar12.i, i32 0
  %vector22.i = shufflevector <16 x i64> %temp21.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp1623.i = add <16 x i64> %tmp1520.i, %vector22.i
  %extract24.i = extractelement <16 x i64> %tmp1623.i, i32 0
  %extract25.i = extractelement <16 x i64> %tmp1623.i, i32 1
  %extract26.i = extractelement <16 x i64> %tmp1623.i, i32 2
  %extract27.i = extractelement <16 x i64> %tmp1623.i, i32 3
  %extract28.i = extractelement <16 x i64> %tmp1623.i, i32 4
  %extract29.i = extractelement <16 x i64> %tmp1623.i, i32 5
  %extract30.i = extractelement <16 x i64> %tmp1623.i, i32 6
  %extract31.i = extractelement <16 x i64> %tmp1623.i, i32 7
  %extract32.i = extractelement <16 x i64> %tmp1623.i, i32 8
  %extract33.i = extractelement <16 x i64> %tmp1623.i, i32 9
  %extract34.i = extractelement <16 x i64> %tmp1623.i, i32 10
  %extract35.i = extractelement <16 x i64> %tmp1623.i, i32 11
  %extract36.i = extractelement <16 x i64> %tmp1623.i, i32 12
  %extract37.i = extractelement <16 x i64> %tmp1623.i, i32 13
  %extract38.i = extractelement <16 x i64> %tmp1623.i, i32 14
  %extract39.i = extractelement <16 x i64> %tmp1623.i, i32 15
  %55 = getelementptr i32 addrspace(1)* %1, i64 %extract24.i
  %56 = getelementptr i32 addrspace(1)* %1, i64 %extract25.i
  %57 = getelementptr i32 addrspace(1)* %1, i64 %extract26.i
  %58 = getelementptr i32 addrspace(1)* %1, i64 %extract27.i
  %59 = getelementptr i32 addrspace(1)* %1, i64 %extract28.i
  %60 = getelementptr i32 addrspace(1)* %1, i64 %extract29.i
  %61 = getelementptr i32 addrspace(1)* %1, i64 %extract30.i
  %62 = getelementptr i32 addrspace(1)* %1, i64 %extract31.i
  %63 = getelementptr i32 addrspace(1)* %1, i64 %extract32.i
  %64 = getelementptr i32 addrspace(1)* %1, i64 %extract33.i
  %65 = getelementptr i32 addrspace(1)* %1, i64 %extract34.i
  %66 = getelementptr i32 addrspace(1)* %1, i64 %extract35.i
  %67 = getelementptr i32 addrspace(1)* %1, i64 %extract36.i
  %68 = getelementptr i32 addrspace(1)* %1, i64 %extract37.i
  %69 = getelementptr i32 addrspace(1)* %1, i64 %extract38.i
  %70 = getelementptr i32 addrspace(1)* %1, i64 %extract39.i
  %71 = load i32 addrspace(1)* %55, align 4
  %72 = load i32 addrspace(1)* %56, align 4
  %73 = load i32 addrspace(1)* %57, align 4
  %74 = load i32 addrspace(1)* %58, align 4
  %75 = load i32 addrspace(1)* %59, align 4
  %76 = load i32 addrspace(1)* %60, align 4
  %77 = load i32 addrspace(1)* %61, align 4
  %78 = load i32 addrspace(1)* %62, align 4
  %79 = load i32 addrspace(1)* %63, align 4
  %80 = load i32 addrspace(1)* %64, align 4
  %81 = load i32 addrspace(1)* %65, align 4
  %82 = load i32 addrspace(1)* %66, align 4
  %83 = load i32 addrspace(1)* %67, align 4
  %84 = load i32 addrspace(1)* %68, align 4
  %85 = load i32 addrspace(1)* %69, align 4
  %86 = load i32 addrspace(1)* %70, align 4
  %temp.vect.i = insertelement <16 x i32> undef, i32 %71, i32 0
  %temp.vect40.i = insertelement <16 x i32> %temp.vect.i, i32 %72, i32 1
  %temp.vect41.i = insertelement <16 x i32> %temp.vect40.i, i32 %73, i32 2
  %temp.vect42.i = insertelement <16 x i32> %temp.vect41.i, i32 %74, i32 3
  %temp.vect43.i = insertelement <16 x i32> %temp.vect42.i, i32 %75, i32 4
  %temp.vect44.i = insertelement <16 x i32> %temp.vect43.i, i32 %76, i32 5
  %temp.vect45.i = insertelement <16 x i32> %temp.vect44.i, i32 %77, i32 6
  %temp.vect46.i = insertelement <16 x i32> %temp.vect45.i, i32 %78, i32 7
  %temp.vect47.i = insertelement <16 x i32> %temp.vect46.i, i32 %79, i32 8
  %temp.vect48.i = insertelement <16 x i32> %temp.vect47.i, i32 %80, i32 9
  %temp.vect49.i = insertelement <16 x i32> %temp.vect48.i, i32 %81, i32 10
  %temp.vect50.i = insertelement <16 x i32> %temp.vect49.i, i32 %82, i32 11
  %temp.vect51.i = insertelement <16 x i32> %temp.vect50.i, i32 %83, i32 12
  %temp.vect52.i = insertelement <16 x i32> %temp.vect51.i, i32 %84, i32 13
  %temp.vect53.i = insertelement <16 x i32> %temp.vect52.i, i32 %85, i32 14
  %temp.vect54.i = insertelement <16 x i32> %temp.vect53.i, i32 %86, i32 15
  %87 = lshr <16 x i32> %temp.vect54.i, %vector56.i
  %88 = and <16 x i32> %87, <i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255>
  %89 = zext <16 x i32> %88 to <16 x i64>
  %90 = or <16 x i64> %89, %53
  %extract57.i = extractelement <16 x i64> %90, i32 0
  %extract58.i = extractelement <16 x i64> %90, i32 1
  %extract59.i = extractelement <16 x i64> %90, i32 2
  %extract60.i = extractelement <16 x i64> %90, i32 3
  %extract61.i = extractelement <16 x i64> %90, i32 4
  %extract62.i = extractelement <16 x i64> %90, i32 5
  %extract63.i = extractelement <16 x i64> %90, i32 6
  %extract64.i = extractelement <16 x i64> %90, i32 7
  %extract65.i = extractelement <16 x i64> %90, i32 8
  %extract66.i = extractelement <16 x i64> %90, i32 9
  %extract67.i = extractelement <16 x i64> %90, i32 10
  %extract68.i = extractelement <16 x i64> %90, i32 11
  %extract69.i = extractelement <16 x i64> %90, i32 12
  %extract70.i = extractelement <16 x i64> %90, i32 13
  %extract71.i = extractelement <16 x i64> %90, i32 14
  %extract72.i = extractelement <16 x i64> %90, i32 15
  %91 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract57.i
  %92 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract58.i
  %93 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract59.i
  %94 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract60.i
  %95 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract61.i
  %96 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract62.i
  %97 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract63.i
  %98 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract64.i
  %99 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract65.i
  %100 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract66.i
  %101 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract67.i
  %102 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract68.i
  %103 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract69.i
  %104 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract70.i
  %105 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract71.i
  %106 = getelementptr inbounds i16 addrspace(3)* %10, i64 %extract72.i
  %107 = load i16 addrspace(3)* %91, align 2
  %108 = load i16 addrspace(3)* %92, align 2
  %109 = load i16 addrspace(3)* %93, align 2
  %110 = load i16 addrspace(3)* %94, align 2
  %111 = load i16 addrspace(3)* %95, align 2
  %112 = load i16 addrspace(3)* %96, align 2
  %113 = load i16 addrspace(3)* %97, align 2
  %114 = load i16 addrspace(3)* %98, align 2
  %115 = load i16 addrspace(3)* %99, align 2
  %116 = load i16 addrspace(3)* %100, align 2
  %117 = load i16 addrspace(3)* %101, align 2
  %118 = load i16 addrspace(3)* %102, align 2
  %119 = load i16 addrspace(3)* %103, align 2
  %120 = load i16 addrspace(3)* %104, align 2
  %121 = load i16 addrspace(3)* %105, align 2
  %122 = load i16 addrspace(3)* %106, align 2
  %temp.vect73.i = insertelement <16 x i16> undef, i16 %107, i32 0
  %temp.vect74.i = insertelement <16 x i16> %temp.vect73.i, i16 %108, i32 1
  %temp.vect75.i = insertelement <16 x i16> %temp.vect74.i, i16 %109, i32 2
  %temp.vect76.i = insertelement <16 x i16> %temp.vect75.i, i16 %110, i32 3
  %temp.vect77.i = insertelement <16 x i16> %temp.vect76.i, i16 %111, i32 4
  %temp.vect78.i = insertelement <16 x i16> %temp.vect77.i, i16 %112, i32 5
  %temp.vect79.i = insertelement <16 x i16> %temp.vect78.i, i16 %113, i32 6
  %temp.vect80.i = insertelement <16 x i16> %temp.vect79.i, i16 %114, i32 7
  %temp.vect81.i = insertelement <16 x i16> %temp.vect80.i, i16 %115, i32 8
  %temp.vect82.i = insertelement <16 x i16> %temp.vect81.i, i16 %116, i32 9
  %temp.vect83.i = insertelement <16 x i16> %temp.vect82.i, i16 %117, i32 10
  %temp.vect84.i = insertelement <16 x i16> %temp.vect83.i, i16 %118, i32 11
  %temp.vect85.i = insertelement <16 x i16> %temp.vect84.i, i16 %119, i32 12
  %temp.vect86.i = insertelement <16 x i16> %temp.vect85.i, i16 %120, i32 13
  %temp.vect87.i = insertelement <16 x i16> %temp.vect86.i, i16 %121, i32 14
  %temp.vect88.i = insertelement <16 x i16> %temp.vect87.i, i16 %122, i32 15
  %123 = add <16 x i16> %temp.vect88.i, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %extract89.i = extractelement <16 x i16> %123, i32 0
  %extract90.i = extractelement <16 x i16> %123, i32 1
  %extract91.i = extractelement <16 x i16> %123, i32 2
  %extract92.i = extractelement <16 x i16> %123, i32 3
  %extract93.i = extractelement <16 x i16> %123, i32 4
  %extract94.i = extractelement <16 x i16> %123, i32 5
  %extract95.i = extractelement <16 x i16> %123, i32 6
  %extract96.i = extractelement <16 x i16> %123, i32 7
  %extract97.i = extractelement <16 x i16> %123, i32 8
  %extract98.i = extractelement <16 x i16> %123, i32 9
  %extract99.i = extractelement <16 x i16> %123, i32 10
  %extract100.i = extractelement <16 x i16> %123, i32 11
  %extract101.i = extractelement <16 x i16> %123, i32 12
  %extract102.i = extractelement <16 x i16> %123, i32 13
  %extract103.i = extractelement <16 x i16> %123, i32 14
  %extract104.i = extractelement <16 x i16> %123, i32 15
  store i16 %extract89.i, i16 addrspace(3)* %91, align 2
  store i16 %extract90.i, i16 addrspace(3)* %92, align 2
  store i16 %extract91.i, i16 addrspace(3)* %93, align 2
  store i16 %extract92.i, i16 addrspace(3)* %94, align 2
  store i16 %extract93.i, i16 addrspace(3)* %95, align 2
  store i16 %extract94.i, i16 addrspace(3)* %96, align 2
  store i16 %extract95.i, i16 addrspace(3)* %97, align 2
  store i16 %extract96.i, i16 addrspace(3)* %98, align 2
  store i16 %extract97.i, i16 addrspace(3)* %99, align 2
  store i16 %extract98.i, i16 addrspace(3)* %100, align 2
  store i16 %extract99.i, i16 addrspace(3)* %101, align 2
  store i16 %extract100.i, i16 addrspace(3)* %102, align 2
  store i16 %extract101.i, i16 addrspace(3)* %103, align 2
  store i16 %extract102.i, i16 addrspace(3)* %104, align 2
  store i16 %extract103.i, i16 addrspace(3)* %105, align 2
  store i16 %extract104.i, i16 addrspace(3)* %106, align 2
  %indvar.next13.i = add i64 %indvar12.i, 1
  %exitcond14.i = icmp eq i64 %indvar.next13.i, 256
  br i1 %exitcond14.i, label %bb.nph.i, label %54

bb.nph.i:                                         ; preds = %54
  %check.WI.iter205.i = icmp ult i64 %CurrWI..1.i, %22
  br i1 %check.WI.iter205.i, label %thenBB202.i, label %elseBB203.i

thenBB202.i:                                      ; preds = %bb.nph.i
  %"CurrWI++206.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride208.i" = add nuw i64 %CurrSBIndex..1.i, 640
  br label %SyncBB.i

elseBB203.i:                                      ; preds = %bb.nph.i
  %tmp9.i = shl i64 %35, 12
  %temp106.i = insertelement <16 x i64> undef, i64 %tmp9.i, i32 0
  %vector107.i = shufflevector <16 x i64> %temp106.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB198.i

SyncBB198.i:                                      ; preds = %thenBB209.i, %elseBB203.i
  %CurrSBIndex..2.i = phi i64 [ 0, %elseBB203.i ], [ %"loadedCurrSB+Stride215.i", %thenBB209.i ]
  %CurrWI..2.i = phi i64 [ 0, %elseBB203.i ], [ %"CurrWI++213.i", %thenBB209.i ]
  %"&(pSB[currWI].offset)178.i" = add nuw i64 %CurrSBIndex..2.i, 128
  %"&pSB[currWI].offset179.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)178.i"
  %CastToValueType180.i = bitcast i8* %"&pSB[currWI].offset179.i" to <16 x i64>*
  %loadedValue.i = load <16 x i64>* %CastToValueType180.i, align 128
  %tmp105.i = shl <16 x i64> %loadedValue.i, <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
  %tmp10108.i = add <16 x i64> %vector107.i, %tmp105.i
  br label %124

; <label>:124                                     ; preds = %124, %SyncBB198.i
  %indvar.i = phi i64 [ 0, %SyncBB198.i ], [ %indvar.next.i, %124 ]
  %temp109.i = insertelement <16 x i64> undef, i64 %indvar.i, i32 0
  %vector110.i = shufflevector <16 x i64> %temp109.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp8111.i = add <16 x i64> %tmp105.i, %vector110.i
  %extract112.i = extractelement <16 x i64> %tmp8111.i, i32 0
  %extract113.i = extractelement <16 x i64> %tmp8111.i, i32 1
  %extract114.i = extractelement <16 x i64> %tmp8111.i, i32 2
  %extract115.i = extractelement <16 x i64> %tmp8111.i, i32 3
  %extract116.i = extractelement <16 x i64> %tmp8111.i, i32 4
  %extract117.i = extractelement <16 x i64> %tmp8111.i, i32 5
  %extract118.i = extractelement <16 x i64> %tmp8111.i, i32 6
  %extract119.i = extractelement <16 x i64> %tmp8111.i, i32 7
  %extract120.i = extractelement <16 x i64> %tmp8111.i, i32 8
  %extract121.i = extractelement <16 x i64> %tmp8111.i, i32 9
  %extract122.i = extractelement <16 x i64> %tmp8111.i, i32 10
  %extract123.i = extractelement <16 x i64> %tmp8111.i, i32 11
  %extract124.i = extractelement <16 x i64> %tmp8111.i, i32 12
  %extract125.i = extractelement <16 x i64> %tmp8111.i, i32 13
  %extract126.i = extractelement <16 x i64> %tmp8111.i, i32 14
  %extract127.i = extractelement <16 x i64> %tmp8111.i, i32 15
  %125 = getelementptr i16 addrspace(3)* %10, i64 %extract112.i
  %126 = getelementptr i16 addrspace(3)* %10, i64 %extract113.i
  %127 = getelementptr i16 addrspace(3)* %10, i64 %extract114.i
  %128 = getelementptr i16 addrspace(3)* %10, i64 %extract115.i
  %129 = getelementptr i16 addrspace(3)* %10, i64 %extract116.i
  %130 = getelementptr i16 addrspace(3)* %10, i64 %extract117.i
  %131 = getelementptr i16 addrspace(3)* %10, i64 %extract118.i
  %132 = getelementptr i16 addrspace(3)* %10, i64 %extract119.i
  %133 = getelementptr i16 addrspace(3)* %10, i64 %extract120.i
  %134 = getelementptr i16 addrspace(3)* %10, i64 %extract121.i
  %135 = getelementptr i16 addrspace(3)* %10, i64 %extract122.i
  %136 = getelementptr i16 addrspace(3)* %10, i64 %extract123.i
  %137 = getelementptr i16 addrspace(3)* %10, i64 %extract124.i
  %138 = getelementptr i16 addrspace(3)* %10, i64 %extract125.i
  %139 = getelementptr i16 addrspace(3)* %10, i64 %extract126.i
  %140 = getelementptr i16 addrspace(3)* %10, i64 %extract127.i
  %tmp11128.i = add <16 x i64> %tmp10108.i, %vector110.i
  %141 = load i16 addrspace(3)* %125, align 2
  %142 = load i16 addrspace(3)* %126, align 2
  %143 = load i16 addrspace(3)* %127, align 2
  %144 = load i16 addrspace(3)* %128, align 2
  %145 = load i16 addrspace(3)* %129, align 2
  %146 = load i16 addrspace(3)* %130, align 2
  %147 = load i16 addrspace(3)* %131, align 2
  %148 = load i16 addrspace(3)* %132, align 2
  %149 = load i16 addrspace(3)* %133, align 2
  %150 = load i16 addrspace(3)* %134, align 2
  %151 = load i16 addrspace(3)* %135, align 2
  %152 = load i16 addrspace(3)* %136, align 2
  %153 = load i16 addrspace(3)* %137, align 2
  %154 = load i16 addrspace(3)* %138, align 2
  %155 = load i16 addrspace(3)* %139, align 2
  %156 = load i16 addrspace(3)* %140, align 2
  %temp.vect129.i = insertelement <16 x i16> undef, i16 %141, i32 0
  %temp.vect130.i = insertelement <16 x i16> %temp.vect129.i, i16 %142, i32 1
  %temp.vect131.i = insertelement <16 x i16> %temp.vect130.i, i16 %143, i32 2
  %temp.vect132.i = insertelement <16 x i16> %temp.vect131.i, i16 %144, i32 3
  %temp.vect133.i = insertelement <16 x i16> %temp.vect132.i, i16 %145, i32 4
  %temp.vect134.i = insertelement <16 x i16> %temp.vect133.i, i16 %146, i32 5
  %temp.vect135.i = insertelement <16 x i16> %temp.vect134.i, i16 %147, i32 6
  %temp.vect136.i = insertelement <16 x i16> %temp.vect135.i, i16 %148, i32 7
  %temp.vect137.i = insertelement <16 x i16> %temp.vect136.i, i16 %149, i32 8
  %temp.vect138.i = insertelement <16 x i16> %temp.vect137.i, i16 %150, i32 9
  %temp.vect139.i = insertelement <16 x i16> %temp.vect138.i, i16 %151, i32 10
  %temp.vect140.i = insertelement <16 x i16> %temp.vect139.i, i16 %152, i32 11
  %temp.vect141.i = insertelement <16 x i16> %temp.vect140.i, i16 %153, i32 12
  %temp.vect142.i = insertelement <16 x i16> %temp.vect141.i, i16 %154, i32 13
  %temp.vect143.i = insertelement <16 x i16> %temp.vect142.i, i16 %155, i32 14
  %temp.vect144.i = insertelement <16 x i16> %temp.vect143.i, i16 %156, i32 15
  %157 = zext <16 x i16> %temp.vect144.i to <16 x i32>
  %extract161.i = extractelement <16 x i32> %157, i32 0
  %extract162.i = extractelement <16 x i32> %157, i32 1
  %extract163.i = extractelement <16 x i32> %157, i32 2
  %extract164.i = extractelement <16 x i32> %157, i32 3
  %extract165.i = extractelement <16 x i32> %157, i32 4
  %extract166.i = extractelement <16 x i32> %157, i32 5
  %extract167.i = extractelement <16 x i32> %157, i32 6
  %extract168.i = extractelement <16 x i32> %157, i32 7
  %extract169.i = extractelement <16 x i32> %157, i32 8
  %extract170.i = extractelement <16 x i32> %157, i32 9
  %extract171.i = extractelement <16 x i32> %157, i32 10
  %extract172.i = extractelement <16 x i32> %157, i32 11
  %extract173.i = extractelement <16 x i32> %157, i32 12
  %extract174.i = extractelement <16 x i32> %157, i32 13
  %extract175.i = extractelement <16 x i32> %157, i32 14
  %extract176.i = extractelement <16 x i32> %157, i32 15
  %158 = and <16 x i64> %tmp11128.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract145.i = extractelement <16 x i64> %158, i32 0
  %extract146.i = extractelement <16 x i64> %158, i32 1
  %extract147.i = extractelement <16 x i64> %158, i32 2
  %extract148.i = extractelement <16 x i64> %158, i32 3
  %extract149.i = extractelement <16 x i64> %158, i32 4
  %extract150.i = extractelement <16 x i64> %158, i32 5
  %extract151.i = extractelement <16 x i64> %158, i32 6
  %extract152.i = extractelement <16 x i64> %158, i32 7
  %extract153.i = extractelement <16 x i64> %158, i32 8
  %extract154.i = extractelement <16 x i64> %158, i32 9
  %extract155.i = extractelement <16 x i64> %158, i32 10
  %extract156.i = extractelement <16 x i64> %158, i32 11
  %extract157.i = extractelement <16 x i64> %158, i32 12
  %extract158.i = extractelement <16 x i64> %158, i32 13
  %extract159.i = extractelement <16 x i64> %158, i32 14
  %extract160.i = extractelement <16 x i64> %158, i32 15
  %159 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract145.i
  %160 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract146.i
  %161 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract147.i
  %162 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract148.i
  %163 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract149.i
  %164 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract150.i
  %165 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract151.i
  %166 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract152.i
  %167 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract153.i
  %168 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract154.i
  %169 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract155.i
  %170 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract156.i
  %171 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract157.i
  %172 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract158.i
  %173 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract159.i
  %174 = getelementptr inbounds i32 addrspace(1)* %4, i64 %extract160.i
  store i32 %extract161.i, i32 addrspace(1)* %159, align 4
  store i32 %extract162.i, i32 addrspace(1)* %160, align 4
  store i32 %extract163.i, i32 addrspace(1)* %161, align 4
  store i32 %extract164.i, i32 addrspace(1)* %162, align 4
  store i32 %extract165.i, i32 addrspace(1)* %163, align 4
  store i32 %extract166.i, i32 addrspace(1)* %164, align 4
  store i32 %extract167.i, i32 addrspace(1)* %165, align 4
  store i32 %extract168.i, i32 addrspace(1)* %166, align 4
  store i32 %extract169.i, i32 addrspace(1)* %167, align 4
  store i32 %extract170.i, i32 addrspace(1)* %168, align 4
  store i32 %extract171.i, i32 addrspace(1)* %169, align 4
  store i32 %extract172.i, i32 addrspace(1)* %170, align 4
  store i32 %extract173.i, i32 addrspace(1)* %171, align 4
  store i32 %extract174.i, i32 addrspace(1)* %172, align 4
  store i32 %extract175.i, i32 addrspace(1)* %173, align 4
  store i32 %extract176.i, i32 addrspace(1)* %174, align 4
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, 256
  br i1 %exitcond.i, label %._crit_edge.i, label %124

._crit_edge.i:                                    ; preds = %124
  %check.WI.iter212.i = icmp ult i64 %CurrWI..2.i, %22
  br i1 %check.WI.iter212.i, label %thenBB209.i, label %____Vectorized_.histogram_separated_args.exit

thenBB209.i:                                      ; preds = %._crit_edge.i
  %"CurrWI++213.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride215.i" = add nuw i64 %CurrSBIndex..2.i, 640
  br label %SyncBB198.i

____Vectorized_.histogram_separated_args.exit:    ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i16 addrspace(3)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__histogram_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint const __attribute__((address_space(1))) *, uint __attribute__((address_space(1))) *, uint, ushort __attribute__((address_space(3))) *", metadata !"opencl_histogram_locals_anchor", void (i8*)* @histogram}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i16 addrspace(3)*, i32 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__permute_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uint const __attribute__((address_space(1))) *, uint const __attribute__((address_space(1))) *, uint, ushort __attribute__((address_space(3))) *, uint __attribute__((address_space(1))) *", metadata !"opencl_permute_locals_anchor", void (i8*)* @permute}
