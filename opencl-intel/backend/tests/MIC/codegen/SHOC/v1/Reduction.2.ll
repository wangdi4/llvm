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

declare void @__reduce_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double addrspace(3)* nocapture, i32) nounwind

declare i64 @get_local_id(i32)

declare i64 @get_group_id(i32)

declare i64 @get_local_size(i32)

declare i64 @get_num_groups(i32)

declare void @barrier(i64)

declare void @__reduceNoLocal_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32) nounwind

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

define void @__reduce_separated_args(double addrspace(1)* nocapture %g_idata, double addrspace(1)* nocapture %g_odata, double addrspace(3)* nocapture %sdata, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB74

SyncBB74:                                         ; preds = %0, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = trunc i64 %2 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %3, i32* %CastToValueType, align 4
  %4 = load i64* %pWGId, align 8
  %5 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %6 = load i64* %5, align 8
  %7 = shl i64 %4, 1
  %8 = mul i64 %7, %6
  %9 = add i64 %8, %2
  %10 = trunc i64 %9 to i32
  %11 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 0
  %12 = load i64* %11, align 8
  %13 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %14 = load i64* %13, align 8
  %15 = trunc i64 %14 to i32
  %16 = and i64 %2, 4294967295
  %17 = getelementptr inbounds double addrspace(3)* %sdata, i64 %16
  %"&(pSB[currWI].offset)34" = add nuw i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset35" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)34"
  %CastToValueType36 = bitcast i8* %"&pSB[currWI].offset35" to double addrspace(3)**
  store double addrspace(3)* %17, double addrspace(3)** %CastToValueType36, align 8
  store double 0.000000e+00, double addrspace(3)* %17, align 8
  %18 = icmp ult i32 %10, %n
  br i1 %18, label %bb.nph4, label %._crit_edge5

bb.nph4:                                          ; preds = %SyncBB74
  %tmp = mul i64 %6, %12
  %tmp6 = shl i64 %tmp, 1
  %tmp7 = trunc i64 %tmp6 to i32
  %tmp9 = mul i64 %4, %6
  %tmp10 = shl i64 %tmp9, 1
  %tmp11 = add i64 %2, %tmp10
  %tmp12 = trunc i64 %tmp11 to i32
  %tmp15 = add i32 %15, %tmp12
  %tmp17 = add i32 %tmp7, %tmp12
  %"&(pSB[currWI].offset)48" = add nuw i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset49" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)48"
  %CastToValueType50 = bitcast i8* %"&pSB[currWI].offset49" to double addrspace(3)**
  br label %19

; <label>:19                                      ; preds = %19, %bb.nph4
  %20 = phi double [ 0.000000e+00, %bb.nph4 ], [ %28, %19 ]
  %indvar = phi i32 [ 0, %bb.nph4 ], [ %indvar.next, %19 ]
  %tmp8 = mul i32 %tmp7, %indvar
  %i.03 = add i32 %tmp12, %tmp8
  %tmp16 = add i32 %tmp15, %tmp8
  %21 = zext i32 %i.03 to i64
  %22 = getelementptr inbounds double addrspace(1)* %g_idata, i64 %21
  %23 = load double addrspace(1)* %22, align 8
  %24 = zext i32 %tmp16 to i64
  %25 = getelementptr inbounds double addrspace(1)* %g_idata, i64 %24
  %26 = load double addrspace(1)* %25, align 8
  %27 = fadd double %23, %26
  %28 = fadd double %20, %27
  %loadedValue51 = load double addrspace(3)** %CastToValueType50, align 8
  store double %28, double addrspace(3)* %loadedValue51, align 8
  %tmp18 = add i32 %tmp17, %tmp8
  %29 = icmp ult i32 %tmp18, %n
  %indvar.next = add i32 %indvar, 1
  br i1 %29, label %19, label %._crit_edge5

._crit_edge5:                                     ; preds = %19, %SyncBB74
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %._crit_edge5
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 24
  br label %SyncBB74

elseBB:                                           ; preds = %._crit_edge5
  %s.01 = lshr i32 %15, 1
  %30 = icmp eq i32 %s.01, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB84, %thenBB77, %elseBB
  %currBarrier.3 = phi i32 [ 0, %elseBB ], [ %currBarrier.1, %thenBB77 ], [ %currBarrier.2, %thenBB84 ]
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride83", %thenBB77 ], [ %"loadedCurrSB+Stride90", %thenBB84 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++81", %thenBB77 ], [ %"CurrWI++88", %thenBB84 ]
  br i1 %30, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %SyncBB73, %SyncBB
  %currBarrier.1 = phi i32 [ %currBarrier.0, %SyncBB73 ], [ %currBarrier.3, %SyncBB ]
  %CurrSBIndex..3 = phi i64 [ %CurrSBIndex..2, %SyncBB73 ], [ %CurrSBIndex..1, %SyncBB ]
  %CurrWI..3 = phi i64 [ %CurrWI..2, %SyncBB73 ], [ %CurrWI..1, %SyncBB ]
  %s.02 = phi i32 [ %s.0, %SyncBB73 ], [ %s.01, %SyncBB ]
  %"&(pSB[currWI].offset)53" = add nuw i64 %CurrSBIndex..3, 16
  %"&pSB[currWI].offset54" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)53"
  %CastToValueType55 = bitcast i8* %"&pSB[currWI].offset54" to i32*
  store i32 %s.02, i32* %CastToValueType55, align 4
  %"&pSB[currWI].offset25" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..3
  %CastToValueType26 = bitcast i8* %"&pSB[currWI].offset25" to i32*
  %loadedValue27 = load i32* %CastToValueType26, align 4
  %31 = icmp ult i32 %loadedValue27, %s.02
  br i1 %31, label %32, label %39

; <label>:32                                      ; preds = %bb.nph
  %"&pSB[currWI].offset21" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..3
  %CastToValueType22 = bitcast i8* %"&pSB[currWI].offset21" to i32*
  %loadedValue = load i32* %CastToValueType22, align 4
  %"&(pSB[currWI].offset)62" = add nuw i64 %CurrSBIndex..3, 16
  %"&pSB[currWI].offset63" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)62"
  %CastToValueType64 = bitcast i8* %"&pSB[currWI].offset63" to i32*
  %loadedValue65 = load i32* %CastToValueType64, align 4
  %33 = add i32 %loadedValue65, %loadedValue
  %34 = zext i32 %33 to i64
  %35 = getelementptr inbounds double addrspace(3)* %sdata, i64 %34
  %36 = load double addrspace(3)* %35, align 8
  %"&(pSB[currWI].offset)38" = add nuw i64 %CurrSBIndex..3, 8
  %"&pSB[currWI].offset39" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)38"
  %CastToValueType40 = bitcast i8* %"&pSB[currWI].offset39" to double addrspace(3)**
  %loadedValue41 = load double addrspace(3)** %CastToValueType40, align 8
  %37 = load double addrspace(3)* %loadedValue41, align 8
  %38 = fadd double %37, %36
  %"&(pSB[currWI].offset)43" = add nuw i64 %CurrSBIndex..3, 8
  %"&pSB[currWI].offset44" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)43"
  %CastToValueType45 = bitcast i8* %"&pSB[currWI].offset44" to double addrspace(3)**
  %loadedValue46 = load double addrspace(3)** %CastToValueType45, align 8
  store double %38, double addrspace(3)* %loadedValue46, align 8
  br label %39

; <label>:39                                      ; preds = %32, %bb.nph
  %check.WI.iter80 = icmp ult i64 %CurrWI..3, %iterCount
  br i1 %check.WI.iter80, label %thenBB77, label %SyncBB73

thenBB77:                                         ; preds = %39
  %"CurrWI++81" = add nuw i64 %CurrWI..3, 1
  %"loadedCurrSB+Stride83" = add nuw i64 %CurrSBIndex..3, 24
  %cond1 = icmp eq i32 %currBarrier.1, 1
  br i1 %cond1, label %SyncBB73, label %SyncBB

SyncBB73:                                         ; preds = %39, %thenBB77, %thenBB84
  %currBarrier.0 = phi i32 [ %currBarrier.2, %thenBB84 ], [ %currBarrier.1, %thenBB77 ], [ 1, %39 ]
  %CurrSBIndex..2 = phi i64 [ %"loadedCurrSB+Stride90", %thenBB84 ], [ %"loadedCurrSB+Stride83", %thenBB77 ], [ 0, %39 ]
  %CurrWI..2 = phi i64 [ %"CurrWI++88", %thenBB84 ], [ %"CurrWI++81", %thenBB77 ], [ 0, %39 ]
  %"&(pSB[currWI].offset)57" = add nuw i64 %CurrSBIndex..2, 16
  %"&pSB[currWI].offset58" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)57"
  %CastToValueType59 = bitcast i8* %"&pSB[currWI].offset58" to i32*
  %loadedValue60 = load i32* %CastToValueType59, align 4
  %s.0 = lshr i32 %loadedValue60, 1
  %"&(pSB[currWI].offset)67" = add nuw i64 %CurrSBIndex..2, 20
  %"&pSB[currWI].offset68" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)67"
  %CastToValueType69 = bitcast i8* %"&pSB[currWI].offset68" to i32*
  store i32 %s.0, i32* %CastToValueType69, align 4
  %40 = icmp eq i32 %s.0, 0
  br i1 %40, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %SyncBB73, %SyncBB
  %currBarrier.2 = phi i32 [ %currBarrier.3, %SyncBB ], [ %currBarrier.0, %SyncBB73 ]
  %CurrSBIndex..4 = phi i64 [ %CurrSBIndex..1, %SyncBB ], [ %CurrSBIndex..2, %SyncBB73 ]
  %CurrWI..4 = phi i64 [ %CurrWI..1, %SyncBB ], [ %CurrWI..2, %SyncBB73 ]
  %"&pSB[currWI].offset30" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..4
  %CastToValueType31 = bitcast i8* %"&pSB[currWI].offset30" to i32*
  %loadedValue32 = load i32* %CastToValueType31, align 4
  %41 = icmp eq i32 %loadedValue32, 0
  br i1 %41, label %42, label %UnifiedReturnBlock

; <label>:42                                      ; preds = %._crit_edge
  %43 = load double addrspace(3)* %sdata, align 8
  %44 = load i64* %pWGId, align 8
  %45 = getelementptr inbounds double addrspace(1)* %g_odata, i64 %44
  store double %43, double addrspace(1)* %45, align 8
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %._crit_edge, %42
  %check.WI.iter87 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter87, label %thenBB84, label %SyncBB75

thenBB84:                                         ; preds = %UnifiedReturnBlock
  %"CurrWI++88" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride90" = add nuw i64 %CurrSBIndex..4, 24
  %cond = icmp eq i32 %currBarrier.2, 0
  br i1 %cond, label %SyncBB, label %SyncBB73

SyncBB75:                                         ; preds = %UnifiedReturnBlock
  ret void
}

define void @__reduceNoLocal_separated_args(double addrspace(1)* nocapture %g_idata, double addrspace(1)* nocapture %g_odata, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp eq i32 %n, 0
  %tmp = zext i32 %n to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  br i1 %0, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %indvar = phi i64 [ %indvar.next, %bb.nph ], [ 0, %SyncBB ]
  %sum.01 = phi double [ %2, %bb.nph ], [ 0.000000e+00, %SyncBB ]
  %scevgep = getelementptr double addrspace(1)* %g_idata, i64 %indvar
  %1 = load double addrspace(1)* %scevgep, align 8
  %2 = fadd double %sum.01, %1
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %sum.0.lcssa = phi double [ 0.000000e+00, %SyncBB ], [ %2, %bb.nph ]
  store double %sum.0.lcssa, double addrspace(1)* %g_odata, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB3

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB3:                                          ; preds = %._crit_edge
  ret void
}

define void @reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(3)**
  %7 = load double addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to %struct.WorkDim**
  %13 = load %struct.WorkDim** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 48
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  br label %SyncBB74.i

SyncBB74.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %26 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = trunc i64 %27 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %28, i32* %CastToValueType.i, align 4
  %29 = load i64* %16, align 8
  %30 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %31 = load i64* %30, align 8
  %32 = shl i64 %29, 1
  %33 = mul i64 %32, %31
  %34 = add i64 %33, %27
  %35 = trunc i64 %34 to i32
  %36 = getelementptr %struct.WorkDim* %13, i64 0, i32 4, i64 0
  %37 = load i64* %36, align 8
  %38 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %39 = load i64* %38, align 8
  %40 = trunc i64 %39 to i32
  %41 = and i64 %27, 4294967295
  %42 = getelementptr inbounds double addrspace(3)* %7, i64 %41
  %"&(pSB[currWI].offset)34.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset35.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)34.i"
  %CastToValueType36.i = bitcast i8* %"&pSB[currWI].offset35.i" to double addrspace(3)**
  store double addrspace(3)* %42, double addrspace(3)** %CastToValueType36.i, align 8
  store double 0.000000e+00, double addrspace(3)* %42, align 8
  %43 = icmp ult i32 %35, %10
  br i1 %43, label %bb.nph4.i, label %._crit_edge5.i

bb.nph4.i:                                        ; preds = %SyncBB74.i
  %tmp.i = mul i64 %31, %37
  %tmp6.i = shl i64 %tmp.i, 1
  %tmp7.i = trunc i64 %tmp6.i to i32
  %tmp9.i = mul i64 %29, %31
  %tmp10.i = shl i64 %tmp9.i, 1
  %tmp11.i = add i64 %27, %tmp10.i
  %tmp12.i = trunc i64 %tmp11.i to i32
  %tmp15.i = add i32 %40, %tmp12.i
  %tmp17.i = add i32 %tmp7.i, %tmp12.i
  %"&(pSB[currWI].offset)48.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)48.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to double addrspace(3)**
  br label %44

; <label>:44                                      ; preds = %44, %bb.nph4.i
  %45 = phi double [ 0.000000e+00, %bb.nph4.i ], [ %53, %44 ]
  %indvar.i = phi i32 [ 0, %bb.nph4.i ], [ %indvar.next.i, %44 ]
  %tmp8.i = mul i32 %tmp7.i, %indvar.i
  %i.03.i = add i32 %tmp12.i, %tmp8.i
  %tmp16.i = add i32 %tmp15.i, %tmp8.i
  %46 = zext i32 %i.03.i to i64
  %47 = getelementptr inbounds double addrspace(1)* %1, i64 %46
  %48 = load double addrspace(1)* %47, align 8
  %49 = zext i32 %tmp16.i to i64
  %50 = getelementptr inbounds double addrspace(1)* %1, i64 %49
  %51 = load double addrspace(1)* %50, align 8
  %52 = fadd double %48, %51
  %53 = fadd double %45, %52
  %loadedValue51.i = load double addrspace(3)** %CastToValueType50.i, align 8
  store double %53, double addrspace(3)* %loadedValue51.i, align 8
  %tmp18.i = add i32 %tmp17.i, %tmp8.i
  %54 = icmp ult i32 %tmp18.i, %10
  %indvar.next.i = add i32 %indvar.i, 1
  br i1 %54, label %44, label %._crit_edge5.i

._crit_edge5.i:                                   ; preds = %44, %SyncBB74.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %._crit_edge5.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 24
  br label %SyncBB74.i

elseBB.i:                                         ; preds = %._crit_edge5.i
  %s.01.i = lshr i32 %40, 1
  %55 = icmp eq i32 %s.01.i, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB84.i, %thenBB77.i, %elseBB.i
  %currBarrier.3.i = phi i32 [ 0, %elseBB.i ], [ %currBarrier.1.i, %thenBB77.i ], [ %currBarrier.2.i, %thenBB84.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride83.i", %thenBB77.i ], [ %"loadedCurrSB+Stride90.i", %thenBB84.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++81.i", %thenBB77.i ], [ %"CurrWI++88.i", %thenBB84.i ]
  br i1 %55, label %._crit_edge.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %SyncBB73.i, %SyncBB.i
  %currBarrier.1.i = phi i32 [ %currBarrier.0.i, %SyncBB73.i ], [ %currBarrier.3.i, %SyncBB.i ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..2.i, %SyncBB73.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..3.i = phi i64 [ %CurrWI..2.i, %SyncBB73.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %s.02.i = phi i32 [ %s.0.i, %SyncBB73.i ], [ %s.01.i, %SyncBB.i ]
  %"&(pSB[currWI].offset)53.i" = add nuw i64 %CurrSBIndex..3.i, 16
  %"&pSB[currWI].offset54.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)53.i"
  %CastToValueType55.i = bitcast i8* %"&pSB[currWI].offset54.i" to i32*
  store i32 %s.02.i, i32* %CastToValueType55.i, align 4
  %"&pSB[currWI].offset25.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..3.i
  %CastToValueType26.i = bitcast i8* %"&pSB[currWI].offset25.i" to i32*
  %loadedValue27.i = load i32* %CastToValueType26.i, align 4
  %56 = icmp ult i32 %loadedValue27.i, %s.02.i
  br i1 %56, label %57, label %64

; <label>:57                                      ; preds = %bb.nph.i
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..3.i
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i32*
  %loadedValue.i = load i32* %CastToValueType22.i, align 4
  %"&(pSB[currWI].offset)62.i" = add nuw i64 %CurrSBIndex..3.i, 16
  %"&pSB[currWI].offset63.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)62.i"
  %CastToValueType64.i = bitcast i8* %"&pSB[currWI].offset63.i" to i32*
  %loadedValue65.i = load i32* %CastToValueType64.i, align 4
  %58 = add i32 %loadedValue65.i, %loadedValue.i
  %59 = zext i32 %58 to i64
  %60 = getelementptr inbounds double addrspace(3)* %7, i64 %59
  %61 = load double addrspace(3)* %60, align 8
  %"&(pSB[currWI].offset)38.i" = add nuw i64 %CurrSBIndex..3.i, 8
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)38.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to double addrspace(3)**
  %loadedValue41.i = load double addrspace(3)** %CastToValueType40.i, align 8
  %62 = load double addrspace(3)* %loadedValue41.i, align 8
  %63 = fadd double %62, %61
  %"&(pSB[currWI].offset)43.i" = add nuw i64 %CurrSBIndex..3.i, 8
  %"&pSB[currWI].offset44.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)43.i"
  %CastToValueType45.i = bitcast i8* %"&pSB[currWI].offset44.i" to double addrspace(3)**
  %loadedValue46.i = load double addrspace(3)** %CastToValueType45.i, align 8
  store double %63, double addrspace(3)* %loadedValue46.i, align 8
  br label %64

; <label>:64                                      ; preds = %57, %bb.nph.i
  %check.WI.iter80.i = icmp ult i64 %CurrWI..3.i, %22
  br i1 %check.WI.iter80.i, label %thenBB77.i, label %SyncBB73.i

thenBB77.i:                                       ; preds = %64
  %"CurrWI++81.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride83.i" = add nuw i64 %CurrSBIndex..3.i, 24
  %cond1.i = icmp eq i32 %currBarrier.1.i, 1
  br i1 %cond1.i, label %SyncBB73.i, label %SyncBB.i

SyncBB73.i:                                       ; preds = %thenBB84.i, %thenBB77.i, %64
  %currBarrier.0.i = phi i32 [ %currBarrier.2.i, %thenBB84.i ], [ %currBarrier.1.i, %thenBB77.i ], [ 1, %64 ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride90.i", %thenBB84.i ], [ %"loadedCurrSB+Stride83.i", %thenBB77.i ], [ 0, %64 ]
  %CurrWI..2.i = phi i64 [ %"CurrWI++88.i", %thenBB84.i ], [ %"CurrWI++81.i", %thenBB77.i ], [ 0, %64 ]
  %"&(pSB[currWI].offset)57.i" = add nuw i64 %CurrSBIndex..2.i, 16
  %"&pSB[currWI].offset58.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)57.i"
  %CastToValueType59.i = bitcast i8* %"&pSB[currWI].offset58.i" to i32*
  %loadedValue60.i = load i32* %CastToValueType59.i, align 4
  %s.0.i = lshr i32 %loadedValue60.i, 1
  %"&(pSB[currWI].offset)67.i" = add nuw i64 %CurrSBIndex..2.i, 20
  %"&pSB[currWI].offset68.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)67.i"
  %CastToValueType69.i = bitcast i8* %"&pSB[currWI].offset68.i" to i32*
  store i32 %s.0.i, i32* %CastToValueType69.i, align 4
  %65 = icmp eq i32 %s.0.i, 0
  br i1 %65, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %SyncBB73.i, %SyncBB.i
  %currBarrier.2.i = phi i32 [ %currBarrier.3.i, %SyncBB.i ], [ %currBarrier.0.i, %SyncBB73.i ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..2.i, %SyncBB73.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..2.i, %SyncBB73.i ]
  %"&pSB[currWI].offset30.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..4.i
  %CastToValueType31.i = bitcast i8* %"&pSB[currWI].offset30.i" to i32*
  %loadedValue32.i = load i32* %CastToValueType31.i, align 4
  %66 = icmp eq i32 %loadedValue32.i, 0
  br i1 %66, label %67, label %UnifiedReturnBlock.i

; <label>:67                                      ; preds = %._crit_edge.i
  %68 = load double addrspace(3)* %7, align 8
  %69 = load i64* %16, align 8
  %70 = getelementptr inbounds double addrspace(1)* %4, i64 %69
  store double %68, double addrspace(1)* %70, align 8
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %67, %._crit_edge.i
  %check.WI.iter87.i = icmp ult i64 %CurrWI..4.i, %22
  br i1 %check.WI.iter87.i, label %thenBB84.i, label %__reduce_separated_args.exit

thenBB84.i:                                       ; preds = %UnifiedReturnBlock.i
  %"CurrWI++88.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride90.i" = add nuw i64 %CurrSBIndex..4.i, 24
  %cond.i = icmp eq i32 %currBarrier.2.i, 0
  br i1 %cond.i, label %SyncBB.i, label %SyncBB73.i

__reduce_separated_args.exit:                     ; preds = %UnifiedReturnBlock.i
  ret void
}

define void @reduceNoLocal(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 72
  %9 = bitcast i8* %8 to i64*
  %10 = load i64* %9, align 8
  %11 = icmp eq i32 %7, 0
  %tmp.i = zext i32 %7 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %11, label %._crit_edge.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %indvar.i = phi i64 [ %indvar.next.i, %bb.nph.i ], [ 0, %SyncBB.i ]
  %sum.01.i = phi double [ %13, %bb.nph.i ], [ 0.000000e+00, %SyncBB.i ]
  %scevgep.i = getelementptr double addrspace(1)* %1, i64 %indvar.i
  %12 = load double addrspace(1)* %scevgep.i, align 8
  %13 = fadd double %sum.01.i, %12
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp.i
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %sum.0.lcssa.i = phi double [ 0.000000e+00, %SyncBB.i ], [ %13, %bb.nph.i ]
  store double %sum.0.lcssa.i, double addrspace(1)* %4, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %10
  br i1 %check.WI.iter.i, label %thenBB.i, label %__reduceNoLocal_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__reduceNoLocal_separated_args.exit:              ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, double addrspace(3)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__reduce_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double const __attribute__((address_space(1))) *, double __attribute__((address_space(1))) *, double __attribute__((address_space(3))) *, unsigned int const", metadata !"opencl_reduce_locals_anchor", void (i8*)* @reduce}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__reduceNoLocal_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, double __attribute__((address_space(1))) *, unsigned int", metadata !"opencl_reduceNoLocal_locals_anchor", void (i8*)* @reduceNoLocal}


