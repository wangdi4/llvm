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

declare void @__reduce_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(3)* nocapture, i32) nounwind

declare i64 @get_local_id(i32)

declare i64 @get_group_id(i32)

declare i64 @get_local_size(i32)

declare i64 @get_num_groups(i32)

declare void @barrier(i64)

declare void @__reduceNoLocal_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare void @____Vectorized_.reduce_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(3)* nocapture, i32) nounwind

declare void @____Vectorized_.reduceNoLocal_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

declare float @masked_load0(i1, float addrspace(1)*)

declare float @masked_load1(i1, float addrspace(1)*)

declare void @masked_store0(i1, float, float addrspace(3)*)

declare float @masked_load2(i1, float addrspace(3)*)

declare float @masked_load3(i1, float addrspace(3)*)

declare void @masked_store1(i1, float, float addrspace(3)*)

declare void @maskedf_0_barrier(i1, i64)

declare float @masked_load4(i1, float addrspace(3)*)

declare i64 @maskedf_1_get_group_id(i1, i32)

declare void @masked_store2(i1, float, float addrspace(1)*)

define i1 @allZero_v16(<16 x i1> %t) {
entry:
  %ipred = bitcast <16 x i1> %t to i16
  %val = call i32 @llvm.x86.mic.kortestz(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

declare void @masked_store3(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

declare <16 x float> @masked_load8(<16 x i1>, <16 x float> addrspace(3)*)

declare void @masked_store4(<16 x i1>, <16 x float>, <16 x float> addrspace(3)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

define void @__reduce_separated_args(float addrspace(1)* nocapture %g_idata, float addrspace(1)* nocapture %g_odata, float addrspace(3)* nocapture %sdata, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB73

SyncBB73:                                         ; preds = %0, %thenBB
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
  %17 = getelementptr inbounds float addrspace(3)* %sdata, i64 %16
  %"&(pSB[currWI].offset)341" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset35" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)341"
  %CastToValueType36 = bitcast i8* %"&pSB[currWI].offset35" to float addrspace(3)**
  store float addrspace(3)* %17, float addrspace(3)** %CastToValueType36, align 8
  store float 0.000000e+00, float addrspace(3)* %17, align 4
  %18 = icmp ult i32 %10, %n
  br i1 %18, label %bb.nph4, label %._crit_edge5

bb.nph4:                                          ; preds = %SyncBB73
  %tmp = mul i64 %6, %12
  %tmp6 = shl i64 %tmp, 1
  %tmp7 = trunc i64 %tmp6 to i32
  %tmp9 = mul i64 %4, %6
  %tmp10 = shl i64 %tmp9, 1
  %tmp11 = add i64 %2, %tmp10
  %tmp12 = trunc i64 %tmp11 to i32
  %tmp15 = add i32 %15, %tmp12
  %tmp17 = add i32 %tmp7, %tmp12
  %"&(pSB[currWI].offset)482" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset49" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)482"
  %CastToValueType50 = bitcast i8* %"&pSB[currWI].offset49" to float addrspace(3)**
  br label %19

; <label>:19                                      ; preds = %19, %bb.nph4
  %20 = phi float [ 0.000000e+00, %bb.nph4 ], [ %28, %19 ]
  %indvar = phi i32 [ 0, %bb.nph4 ], [ %indvar.next, %19 ]
  %tmp8 = mul i32 %tmp7, %indvar
  %i.03 = add i32 %tmp12, %tmp8
  %tmp16 = add i32 %tmp15, %tmp8
  %21 = zext i32 %i.03 to i64
  %22 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %21
  %23 = load float addrspace(1)* %22, align 4
  %24 = zext i32 %tmp16 to i64
  %25 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %24
  %26 = load float addrspace(1)* %25, align 4
  %27 = fadd float %23, %26
  %28 = fadd float %20, %27
  %loadedValue51 = load float addrspace(3)** %CastToValueType50, align 8
  store float %28, float addrspace(3)* %loadedValue51, align 4
  %tmp18 = add i32 %tmp17, %tmp8
  %29 = icmp ult i32 %tmp18, %n
  %indvar.next = add i32 %indvar, 1
  br i1 %29, label %19, label %._crit_edge5

._crit_edge5:                                     ; preds = %19, %SyncBB73
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %._crit_edge5
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 2752
  br label %SyncBB73

elseBB:                                           ; preds = %._crit_edge5
  %s.01 = lshr i32 %15, 1
  %30 = icmp eq i32 %s.01, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB83, %thenBB76, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride89", %thenBB83 ], [ %"loadedCurrSB+Stride82", %thenBB76 ]
  %currBarrier.3 = phi i32 [ 0, %elseBB ], [ %currBarrier.2, %thenBB83 ], [ %currBarrier.1, %thenBB76 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++87", %thenBB83 ], [ %"CurrWI++80", %thenBB76 ]
  br i1 %30, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %SyncBB, %SyncBB72
  %CurrSBIndex..3 = phi i64 [ %CurrSBIndex..2, %SyncBB72 ], [ %CurrSBIndex..1, %SyncBB ]
  %currBarrier.1 = phi i32 [ %currBarrier.0, %SyncBB72 ], [ %currBarrier.3, %SyncBB ]
  %CurrWI..3 = phi i64 [ %CurrWI..2, %SyncBB72 ], [ %CurrWI..1, %SyncBB ]
  %s.02 = phi i32 [ %s.0, %SyncBB72 ], [ %s.01, %SyncBB ]
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
  %35 = getelementptr inbounds float addrspace(3)* %sdata, i64 %34
  %36 = load float addrspace(3)* %35, align 4
  %"&(pSB[currWI].offset)38" = add nuw i64 %CurrSBIndex..3, 8
  %"&pSB[currWI].offset39" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)38"
  %CastToValueType40 = bitcast i8* %"&pSB[currWI].offset39" to float addrspace(3)**
  %loadedValue41 = load float addrspace(3)** %CastToValueType40, align 8
  %37 = load float addrspace(3)* %loadedValue41, align 4
  %38 = fadd float %37, %36
  %"&(pSB[currWI].offset)43" = add nuw i64 %CurrSBIndex..3, 8
  %"&pSB[currWI].offset44" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)43"
  %CastToValueType45 = bitcast i8* %"&pSB[currWI].offset44" to float addrspace(3)**
  %loadedValue46 = load float addrspace(3)** %CastToValueType45, align 8
  store float %38, float addrspace(3)* %loadedValue46, align 4
  br label %39

; <label>:39                                      ; preds = %32, %bb.nph
  %check.WI.iter79 = icmp ult i64 %CurrWI..3, %iterCount
  br i1 %check.WI.iter79, label %thenBB76, label %SyncBB72

thenBB76:                                         ; preds = %39
  %"CurrWI++80" = add nuw i64 %CurrWI..3, 1
  %"loadedCurrSB+Stride82" = add nuw i64 %CurrSBIndex..3, 2752
  %cond3 = icmp eq i32 %currBarrier.1, 0
  br i1 %cond3, label %SyncBB, label %SyncBB72

SyncBB72:                                         ; preds = %39, %thenBB76, %thenBB83
  %CurrSBIndex..2 = phi i64 [ %"loadedCurrSB+Stride89", %thenBB83 ], [ %"loadedCurrSB+Stride82", %thenBB76 ], [ 0, %39 ]
  %currBarrier.0 = phi i32 [ %currBarrier.2, %thenBB83 ], [ %currBarrier.1, %thenBB76 ], [ 1, %39 ]
  %CurrWI..2 = phi i64 [ %"CurrWI++87", %thenBB83 ], [ %"CurrWI++80", %thenBB76 ], [ 0, %39 ]
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

._crit_edge:                                      ; preds = %SyncBB72, %SyncBB
  %CurrSBIndex..4 = phi i64 [ %CurrSBIndex..1, %SyncBB ], [ %CurrSBIndex..2, %SyncBB72 ]
  %currBarrier.2 = phi i32 [ %currBarrier.3, %SyncBB ], [ %currBarrier.0, %SyncBB72 ]
  %CurrWI..4 = phi i64 [ %CurrWI..1, %SyncBB ], [ %CurrWI..2, %SyncBB72 ]
  %"&pSB[currWI].offset30" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..4
  %CastToValueType31 = bitcast i8* %"&pSB[currWI].offset30" to i32*
  %loadedValue32 = load i32* %CastToValueType31, align 4
  %41 = icmp eq i32 %loadedValue32, 0
  br i1 %41, label %42, label %UnifiedReturnBlock

; <label>:42                                      ; preds = %._crit_edge
  %43 = load float addrspace(3)* %sdata, align 4
  %44 = load i64* %pWGId, align 8
  %45 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %44
  store float %43, float addrspace(1)* %45, align 4
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %._crit_edge, %42
  %check.WI.iter86 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter86, label %thenBB83, label %SyncBB74

thenBB83:                                         ; preds = %UnifiedReturnBlock
  %"CurrWI++87" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride89" = add nuw i64 %CurrSBIndex..4, 2752
  %cond = icmp eq i32 %currBarrier.2, 1
  br i1 %cond, label %SyncBB72, label %SyncBB

SyncBB74:                                         ; preds = %UnifiedReturnBlock
  ret void
}

define void @__reduceNoLocal_separated_args(float addrspace(1)* nocapture %g_idata, float addrspace(1)* nocapture %g_odata, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp eq i32 %n, 0
  %tmp = zext i32 %n to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  br i1 %0, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %indvar = phi i64 [ %indvar.next, %bb.nph ], [ 0, %SyncBB ]
  %sum.01 = phi float [ %2, %bb.nph ], [ 0.000000e+00, %SyncBB ]
  %scevgep = getelementptr float addrspace(1)* %g_idata, i64 %indvar
  %1 = load float addrspace(1)* %scevgep, align 4
  %2 = fadd float %sum.01, %1
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %sum.0.lcssa = phi float [ 0.000000e+00, %SyncBB ], [ %2, %bb.nph ]
  store float %sum.0.lcssa, float addrspace(1)* %g_odata, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB3

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB3:                                          ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.reduce_separated_args(float addrspace(1)* nocapture %g_idata, float addrspace(1)* nocapture %g_odata, float addrspace(3)* nocapture %sdata, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph4:
  %temp81 = insertelement <16 x i32> undef, i32 %n, i32 0
  %vector82 = shufflevector <16 x i32> %temp81, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB2172

SyncBB2172:                                       ; preds = %thenBB, %bb.nph4
  %CurrSBIndex..0 = phi i64 [ 0, %bb.nph4 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %CurrWI..0 = phi i64 [ 0, %bb.nph4 ], [ %"CurrWI++", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %broadcast1 = insertelement <16 x i64> undef, i64 %1, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %2 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %3 = trunc <16 x i64> %2 to <16 x i32>
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 64
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to <16 x i32>*
  store <16 x i32> %3, <16 x i32>* %CastToValueType, align 64
  %4 = load i64* %pWGId, align 8
  %5 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %6 = load i64* %5, align 8
  %7 = shl i64 %4, 1
  %8 = mul i64 %7, %6
  %temp = insertelement <16 x i64> undef, i64 %8, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %9 = add <16 x i64> %vector, %2
  %10 = trunc <16 x i64> %9 to <16 x i32>
  %11 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %12 = load i64* %11, align 8
  %13 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 0
  %14 = load i64* %13, align 8
  %15 = trunc i64 %12 to i32
  %extract.lhs = extractelement <16 x i64> %2, i32 0
  %extract = and i64 %extract.lhs, 4294967295
  %16 = getelementptr inbounds float addrspace(3)* %sdata, i64 %extract
  %"&(pSB[currWI].offset)999" = add nuw i64 %CurrSBIndex..0, 128
  %"&pSB[currWI].offset1000" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)999"
  %CastToValueType1001 = bitcast i8* %"&pSB[currWI].offset1000" to float addrspace(3)**
  store float addrspace(3)* %16, float addrspace(3)** %CastToValueType1001, align 8
  %ptrTypeCast = bitcast float addrspace(3)* %16 to <16 x float> addrspace(3)*
  store <16 x float> zeroinitializer, <16 x float> addrspace(3)* %ptrTypeCast, align 4
  %17 = icmp ult <16 x i32> %10, %vector82
  %temp.vect = insertelement <16 x i64> undef, i64 %14, i32 0
  %temp.vect86 = insertelement <16 x i64> %temp.vect, i64 %14, i32 1
  %temp.vect87 = insertelement <16 x i64> %temp.vect86, i64 %14, i32 2
  %temp.vect88 = insertelement <16 x i64> %temp.vect87, i64 %14, i32 3
  %temp.vect89 = insertelement <16 x i64> %temp.vect88, i64 %14, i32 4
  %temp.vect90 = insertelement <16 x i64> %temp.vect89, i64 %14, i32 5
  %temp.vect91 = insertelement <16 x i64> %temp.vect90, i64 %14, i32 6
  %temp.vect92 = insertelement <16 x i64> %temp.vect91, i64 %14, i32 7
  %temp.vect93 = insertelement <16 x i64> %temp.vect92, i64 %14, i32 8
  %temp.vect94 = insertelement <16 x i64> %temp.vect93, i64 %14, i32 9
  %temp.vect95 = insertelement <16 x i64> %temp.vect94, i64 %14, i32 10
  %temp.vect96 = insertelement <16 x i64> %temp.vect95, i64 %14, i32 11
  %temp.vect97 = insertelement <16 x i64> %temp.vect96, i64 %14, i32 12
  %temp.vect98 = insertelement <16 x i64> %temp.vect97, i64 %14, i32 13
  %temp108 = insertelement <16 x i32> undef, i32 %15, i32 0
  %temp.vect99 = insertelement <16 x i64> %temp.vect98, i64 %14, i32 14
  %temp84 = insertelement <16 x i64> undef, i64 %12, i32 0
  %vector109 = shufflevector <16 x i32> %temp108, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp.vect100 = insertelement <16 x i64> %temp.vect99, i64 %14, i32 15
  %vector85 = shufflevector <16 x i64> %temp84, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp101 = mul <16 x i64> %vector85, %temp.vect100
  %tmp6102 = shl <16 x i64> %tmp101, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %tmp7103 = trunc <16 x i64> %tmp6102 to <16 x i32>
  %tmp9 = mul i64 %4, %6
  %tmp10 = shl i64 %tmp9, 1
  %temp104 = insertelement <16 x i64> undef, i64 %tmp10, i32 0
  %vector105 = shufflevector <16 x i64> %temp104, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp11106 = add <16 x i64> %2, %vector105
  %tmp12107 = trunc <16 x i64> %tmp11106 to <16 x i32>
  %tmp15110 = add <16 x i32> %vector109, %tmp12107
  %tmp17111 = add <16 x i32> %tmp7103, %tmp12107
  %ipred.i = bitcast <16 x i1> %17 to i16
  %val.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i, i16 %ipred.i) nounwind
  %tmp.i = and i32 %val.i, 1
  %res.i = icmp eq i32 %tmp.i, 0
  br i1 %res.i, label %.preheader, label %._crit_edge5

.preheader:                                       ; preds = %SyncBB2172
  %negIncomingLoopMask112 = xor <16 x i1> %17, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %"&(pSB[currWI].offset)1013" = add nuw i64 %CurrSBIndex..0, 128
  %"&pSB[currWI].offset1014" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1013"
  %CastToValueType1015 = bitcast i8* %"&pSB[currWI].offset1014" to float addrspace(3)**
  br label %18

; <label>:18                                      ; preds = %postload579, %.preheader
  %vectorPHI = phi <16 x i1> [ %loop_mask11206, %postload579 ], [ %negIncomingLoopMask112, %.preheader ]
  %vectorPHI114 = phi <16 x i1> [ %local_edge225, %postload579 ], [ %17, %.preheader ]
  %vectorPHI115 = phi <16 x float> [ %116, %postload579 ], [ zeroinitializer, %.preheader ]
  %indvar = phi i32 [ %indvar.next, %postload579 ], [ 0, %.preheader ]
  %extract137 = extractelement <16 x i1> %vectorPHI114, i32 0
  %extract138 = extractelement <16 x i1> %vectorPHI114, i32 1
  %extract139 = extractelement <16 x i1> %vectorPHI114, i32 2
  %extract140 = extractelement <16 x i1> %vectorPHI114, i32 3
  %extract141 = extractelement <16 x i1> %vectorPHI114, i32 4
  %extract142 = extractelement <16 x i1> %vectorPHI114, i32 5
  %extract143 = extractelement <16 x i1> %vectorPHI114, i32 6
  %extract144 = extractelement <16 x i1> %vectorPHI114, i32 7
  %extract145 = extractelement <16 x i1> %vectorPHI114, i32 8
  %extract146 = extractelement <16 x i1> %vectorPHI114, i32 9
  %extract147 = extractelement <16 x i1> %vectorPHI114, i32 10
  %extract148 = extractelement <16 x i1> %vectorPHI114, i32 11
  %extract149 = extractelement <16 x i1> %vectorPHI114, i32 12
  %extract150 = extractelement <16 x i1> %vectorPHI114, i32 13
  %extract151 = extractelement <16 x i1> %vectorPHI114, i32 14
  %extract152 = extractelement <16 x i1> %vectorPHI114, i32 15
  %temp116 = insertelement <16 x i32> undef, i32 %indvar, i32 0
  %vector117 = shufflevector <16 x i32> %temp116, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp8118 = mul <16 x i32> %tmp7103, %vector117
  %i.03119 = add <16 x i32> %tmp12107, %tmp8118
  %tmp16120 = add <16 x i32> %tmp15110, %tmp8118
  %19 = extractelement <16 x i32> %i.03119, i32 1
  %20 = zext i32 %19 to i64
  %21 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %20
  %22 = extractelement <16 x i32> %i.03119, i32 2
  %23 = zext i32 %22 to i64
  %24 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %23
  %25 = extractelement <16 x i32> %i.03119, i32 3
  %26 = zext i32 %25 to i64
  %27 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %26
  %28 = extractelement <16 x i32> %i.03119, i32 4
  %29 = zext i32 %28 to i64
  %30 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %29
  %31 = extractelement <16 x i32> %i.03119, i32 5
  %32 = zext i32 %31 to i64
  %33 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %32
  %34 = extractelement <16 x i32> %i.03119, i32 6
  %35 = zext i32 %34 to i64
  %36 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %35
  %37 = extractelement <16 x i32> %i.03119, i32 7
  %38 = zext i32 %37 to i64
  %39 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %38
  %40 = extractelement <16 x i32> %i.03119, i32 8
  %41 = zext i32 %40 to i64
  %42 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %41
  %43 = extractelement <16 x i32> %i.03119, i32 9
  %44 = zext i32 %43 to i64
  %45 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %44
  %46 = extractelement <16 x i32> %i.03119, i32 10
  %47 = zext i32 %46 to i64
  %48 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %47
  %49 = extractelement <16 x i32> %i.03119, i32 11
  %50 = zext i32 %49 to i64
  %51 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %50
  %52 = extractelement <16 x i32> %i.03119, i32 12
  %53 = zext i32 %52 to i64
  %54 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %53
  %55 = extractelement <16 x i32> %i.03119, i32 13
  %56 = zext i32 %55 to i64
  %57 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %56
  %58 = extractelement <16 x i32> %i.03119, i32 14
  %59 = zext i32 %58 to i64
  %60 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %59
  %61 = extractelement <16 x i32> %i.03119, i32 15
  %62 = zext i32 %61 to i64
  %63 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %62
  br i1 %extract137, label %preload605, label %postload606

preload605:                                       ; preds = %18
  %64 = extractelement <16 x i32> %i.03119, i32 0
  %65 = zext i32 %64 to i64
  %66 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %65
  %masked_load = load float addrspace(1)* %66, align 4
  br label %postload606

postload606:                                      ; preds = %preload605, %18
  %phi607 = phi float [ undef, %18 ], [ %masked_load, %preload605 ]
  br i1 %extract138, label %preload614, label %postload615

preload614:                                       ; preds = %postload606
  %masked_load348 = load float addrspace(1)* %21, align 4
  br label %postload615

postload615:                                      ; preds = %preload614, %postload606
  %phi616 = phi float [ undef, %postload606 ], [ %masked_load348, %preload614 ]
  br i1 %extract139, label %preload620, label %postload621

preload620:                                       ; preds = %postload615
  %masked_load349 = load float addrspace(1)* %24, align 4
  br label %postload621

postload621:                                      ; preds = %preload620, %postload615
  %phi622 = phi float [ undef, %postload615 ], [ %masked_load349, %preload620 ]
  br i1 %extract140, label %preload626, label %postload627

preload626:                                       ; preds = %postload621
  %masked_load350 = load float addrspace(1)* %27, align 4
  br label %postload627

postload627:                                      ; preds = %preload626, %postload621
  %phi628 = phi float [ undef, %postload621 ], [ %masked_load350, %preload626 ]
  br i1 %extract141, label %preload632, label %postload633

preload632:                                       ; preds = %postload627
  %masked_load351 = load float addrspace(1)* %30, align 4
  br label %postload633

postload633:                                      ; preds = %preload632, %postload627
  %phi634 = phi float [ undef, %postload627 ], [ %masked_load351, %preload632 ]
  br i1 %extract142, label %preload638, label %postload639

preload638:                                       ; preds = %postload633
  %masked_load352 = load float addrspace(1)* %33, align 4
  br label %postload639

postload639:                                      ; preds = %preload638, %postload633
  %phi640 = phi float [ undef, %postload633 ], [ %masked_load352, %preload638 ]
  br i1 %extract143, label %preload644, label %postload645

preload644:                                       ; preds = %postload639
  %masked_load353 = load float addrspace(1)* %36, align 4
  br label %postload645

postload645:                                      ; preds = %preload644, %postload639
  %phi646 = phi float [ undef, %postload639 ], [ %masked_load353, %preload644 ]
  br i1 %extract144, label %preload650, label %postload651

preload650:                                       ; preds = %postload645
  %masked_load354 = load float addrspace(1)* %39, align 4
  br label %postload651

postload651:                                      ; preds = %preload650, %postload645
  %phi652 = phi float [ undef, %postload645 ], [ %masked_load354, %preload650 ]
  br i1 %extract145, label %preload656, label %postload657

preload656:                                       ; preds = %postload651
  %masked_load355 = load float addrspace(1)* %42, align 4
  br label %postload657

postload657:                                      ; preds = %preload656, %postload651
  %phi658 = phi float [ undef, %postload651 ], [ %masked_load355, %preload656 ]
  br i1 %extract146, label %preload662, label %postload663

preload662:                                       ; preds = %postload657
  %masked_load356 = load float addrspace(1)* %45, align 4
  br label %postload663

postload663:                                      ; preds = %preload662, %postload657
  %phi664 = phi float [ undef, %postload657 ], [ %masked_load356, %preload662 ]
  br i1 %extract147, label %preload668, label %postload669

preload668:                                       ; preds = %postload663
  %masked_load357 = load float addrspace(1)* %48, align 4
  br label %postload669

postload669:                                      ; preds = %preload668, %postload663
  %phi670 = phi float [ undef, %postload663 ], [ %masked_load357, %preload668 ]
  br i1 %extract148, label %preload674, label %postload675

preload674:                                       ; preds = %postload669
  %masked_load358 = load float addrspace(1)* %51, align 4
  br label %postload675

postload675:                                      ; preds = %preload674, %postload669
  %phi676 = phi float [ undef, %postload669 ], [ %masked_load358, %preload674 ]
  br i1 %extract149, label %preload680, label %postload681

preload680:                                       ; preds = %postload675
  %masked_load359 = load float addrspace(1)* %54, align 4
  br label %postload681

postload681:                                      ; preds = %preload680, %postload675
  %phi682 = phi float [ undef, %postload675 ], [ %masked_load359, %preload680 ]
  br i1 %extract150, label %preload686, label %postload687

preload686:                                       ; preds = %postload681
  %masked_load360 = load float addrspace(1)* %57, align 4
  br label %postload687

postload687:                                      ; preds = %preload686, %postload681
  %phi688 = phi float [ undef, %postload681 ], [ %masked_load360, %preload686 ]
  br i1 %extract151, label %preload692, label %postload693

preload692:                                       ; preds = %postload687
  %masked_load361 = load float addrspace(1)* %60, align 4
  br label %postload693

postload693:                                      ; preds = %preload692, %postload687
  %phi694 = phi float [ undef, %postload687 ], [ %masked_load361, %preload692 ]
  br i1 %extract152, label %preload698, label %postload699

preload698:                                       ; preds = %postload693
  %masked_load362 = load float addrspace(1)* %63, align 4
  br label %postload699

postload699:                                      ; preds = %preload698, %postload693
  %phi700 = phi float [ undef, %postload693 ], [ %masked_load362, %preload698 ]
  %temp.vect169 = insertelement <16 x float> undef, float %phi607, i32 0
  %temp.vect170 = insertelement <16 x float> %temp.vect169, float %phi616, i32 1
  %temp.vect171 = insertelement <16 x float> %temp.vect170, float %phi622, i32 2
  %temp.vect172 = insertelement <16 x float> %temp.vect171, float %phi628, i32 3
  %temp.vect173 = insertelement <16 x float> %temp.vect172, float %phi634, i32 4
  %temp.vect174 = insertelement <16 x float> %temp.vect173, float %phi640, i32 5
  %temp.vect175 = insertelement <16 x float> %temp.vect174, float %phi646, i32 6
  %temp.vect176 = insertelement <16 x float> %temp.vect175, float %phi652, i32 7
  %temp.vect177 = insertelement <16 x float> %temp.vect176, float %phi658, i32 8
  %temp.vect178 = insertelement <16 x float> %temp.vect177, float %phi664, i32 9
  %temp.vect179 = insertelement <16 x float> %temp.vect178, float %phi670, i32 10
  %temp.vect180 = insertelement <16 x float> %temp.vect179, float %phi676, i32 11
  %temp.vect181 = insertelement <16 x float> %temp.vect180, float %phi682, i32 12
  %temp.vect182 = insertelement <16 x float> %temp.vect181, float %phi688, i32 13
  %temp.vect183 = insertelement <16 x float> %temp.vect182, float %phi694, i32 14
  %temp.vect184 = insertelement <16 x float> %temp.vect183, float %phi700, i32 15
  %67 = extractelement <16 x i32> %tmp16120, i32 1
  %68 = zext i32 %67 to i64
  %69 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %68
  %70 = extractelement <16 x i32> %tmp16120, i32 2
  %71 = zext i32 %70 to i64
  %72 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %71
  %73 = extractelement <16 x i32> %tmp16120, i32 3
  %74 = zext i32 %73 to i64
  %75 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %74
  %76 = extractelement <16 x i32> %tmp16120, i32 4
  %77 = zext i32 %76 to i64
  %78 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %77
  %79 = extractelement <16 x i32> %tmp16120, i32 5
  %80 = zext i32 %79 to i64
  %81 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %80
  %82 = extractelement <16 x i32> %tmp16120, i32 6
  %83 = zext i32 %82 to i64
  %84 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %83
  %85 = extractelement <16 x i32> %tmp16120, i32 7
  %86 = zext i32 %85 to i64
  %87 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %86
  %88 = extractelement <16 x i32> %tmp16120, i32 8
  %89 = zext i32 %88 to i64
  %90 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %89
  %91 = extractelement <16 x i32> %tmp16120, i32 9
  %92 = zext i32 %91 to i64
  %93 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %92
  %94 = extractelement <16 x i32> %tmp16120, i32 10
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %95
  %97 = extractelement <16 x i32> %tmp16120, i32 11
  %98 = zext i32 %97 to i64
  %99 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %98
  %100 = extractelement <16 x i32> %tmp16120, i32 12
  %101 = zext i32 %100 to i64
  %102 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %101
  %103 = extractelement <16 x i32> %tmp16120, i32 13
  %104 = zext i32 %103 to i64
  %105 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %104
  %106 = extractelement <16 x i32> %tmp16120, i32 14
  %107 = zext i32 %106 to i64
  %108 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %107
  %109 = extractelement <16 x i32> %tmp16120, i32 15
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %110
  br i1 %extract137, label %preload608, label %postload609

preload608:                                       ; preds = %postload699
  %112 = extractelement <16 x i32> %tmp16120, i32 0
  %113 = zext i32 %112 to i64
  %114 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %113
  %masked_load363 = load float addrspace(1)* %114, align 4
  br label %postload609

postload609:                                      ; preds = %preload608, %postload699
  %phi610 = phi float [ undef, %postload699 ], [ %masked_load363, %preload608 ]
  br i1 %extract138, label %preload617, label %postload618

preload617:                                       ; preds = %postload609
  %masked_load364 = load float addrspace(1)* %69, align 4
  br label %postload618

postload618:                                      ; preds = %preload617, %postload609
  %phi619 = phi float [ undef, %postload609 ], [ %masked_load364, %preload617 ]
  br i1 %extract139, label %preload623, label %postload624

preload623:                                       ; preds = %postload618
  %masked_load365 = load float addrspace(1)* %72, align 4
  br label %postload624

postload624:                                      ; preds = %preload623, %postload618
  %phi625 = phi float [ undef, %postload618 ], [ %masked_load365, %preload623 ]
  br i1 %extract140, label %preload629, label %postload630

preload629:                                       ; preds = %postload624
  %masked_load366 = load float addrspace(1)* %75, align 4
  br label %postload630

postload630:                                      ; preds = %preload629, %postload624
  %phi631 = phi float [ undef, %postload624 ], [ %masked_load366, %preload629 ]
  br i1 %extract141, label %preload635, label %postload636

preload635:                                       ; preds = %postload630
  %masked_load367 = load float addrspace(1)* %78, align 4
  br label %postload636

postload636:                                      ; preds = %preload635, %postload630
  %phi637 = phi float [ undef, %postload630 ], [ %masked_load367, %preload635 ]
  br i1 %extract142, label %preload641, label %postload642

preload641:                                       ; preds = %postload636
  %masked_load368 = load float addrspace(1)* %81, align 4
  br label %postload642

postload642:                                      ; preds = %preload641, %postload636
  %phi643 = phi float [ undef, %postload636 ], [ %masked_load368, %preload641 ]
  br i1 %extract143, label %preload647, label %postload648

preload647:                                       ; preds = %postload642
  %masked_load369 = load float addrspace(1)* %84, align 4
  br label %postload648

postload648:                                      ; preds = %preload647, %postload642
  %phi649 = phi float [ undef, %postload642 ], [ %masked_load369, %preload647 ]
  br i1 %extract144, label %preload653, label %postload654

preload653:                                       ; preds = %postload648
  %masked_load370 = load float addrspace(1)* %87, align 4
  br label %postload654

postload654:                                      ; preds = %preload653, %postload648
  %phi655 = phi float [ undef, %postload648 ], [ %masked_load370, %preload653 ]
  br i1 %extract145, label %preload659, label %postload660

preload659:                                       ; preds = %postload654
  %masked_load371 = load float addrspace(1)* %90, align 4
  br label %postload660

postload660:                                      ; preds = %preload659, %postload654
  %phi661 = phi float [ undef, %postload654 ], [ %masked_load371, %preload659 ]
  br i1 %extract146, label %preload665, label %postload666

preload665:                                       ; preds = %postload660
  %masked_load372 = load float addrspace(1)* %93, align 4
  br label %postload666

postload666:                                      ; preds = %preload665, %postload660
  %phi667 = phi float [ undef, %postload660 ], [ %masked_load372, %preload665 ]
  br i1 %extract147, label %preload671, label %postload672

preload671:                                       ; preds = %postload666
  %masked_load373 = load float addrspace(1)* %96, align 4
  br label %postload672

postload672:                                      ; preds = %preload671, %postload666
  %phi673 = phi float [ undef, %postload666 ], [ %masked_load373, %preload671 ]
  br i1 %extract148, label %preload677, label %postload678

preload677:                                       ; preds = %postload672
  %masked_load374 = load float addrspace(1)* %99, align 4
  br label %postload678

postload678:                                      ; preds = %preload677, %postload672
  %phi679 = phi float [ undef, %postload672 ], [ %masked_load374, %preload677 ]
  br i1 %extract149, label %preload683, label %postload684

preload683:                                       ; preds = %postload678
  %masked_load375 = load float addrspace(1)* %102, align 4
  br label %postload684

postload684:                                      ; preds = %preload683, %postload678
  %phi685 = phi float [ undef, %postload678 ], [ %masked_load375, %preload683 ]
  br i1 %extract150, label %preload689, label %postload690

preload689:                                       ; preds = %postload684
  %masked_load376 = load float addrspace(1)* %105, align 4
  br label %postload690

postload690:                                      ; preds = %preload689, %postload684
  %phi691 = phi float [ undef, %postload684 ], [ %masked_load376, %preload689 ]
  br i1 %extract151, label %preload695, label %postload696

preload695:                                       ; preds = %postload690
  %masked_load377 = load float addrspace(1)* %108, align 4
  br label %postload696

postload696:                                      ; preds = %preload695, %postload690
  %phi697 = phi float [ undef, %postload690 ], [ %masked_load377, %preload695 ]
  br i1 %extract152, label %preload701, label %postload702

preload701:                                       ; preds = %postload696
  %masked_load378 = load float addrspace(1)* %111, align 4
  br label %postload702

postload702:                                      ; preds = %preload701, %postload696
  %phi703 = phi float [ undef, %postload696 ], [ %masked_load378, %preload701 ]
  %temp.vect185 = insertelement <16 x float> undef, float %phi610, i32 0
  %temp.vect186 = insertelement <16 x float> %temp.vect185, float %phi619, i32 1
  %temp.vect187 = insertelement <16 x float> %temp.vect186, float %phi625, i32 2
  %temp.vect188 = insertelement <16 x float> %temp.vect187, float %phi631, i32 3
  %temp.vect189 = insertelement <16 x float> %temp.vect188, float %phi637, i32 4
  %temp.vect190 = insertelement <16 x float> %temp.vect189, float %phi643, i32 5
  %temp.vect191 = insertelement <16 x float> %temp.vect190, float %phi649, i32 6
  %temp.vect192 = insertelement <16 x float> %temp.vect191, float %phi655, i32 7
  %temp.vect193 = insertelement <16 x float> %temp.vect192, float %phi661, i32 8
  %temp.vect194 = insertelement <16 x float> %temp.vect193, float %phi667, i32 9
  %temp.vect195 = insertelement <16 x float> %temp.vect194, float %phi673, i32 10
  %temp.vect196 = insertelement <16 x float> %temp.vect195, float %phi679, i32 11
  %temp.vect197 = insertelement <16 x float> %temp.vect196, float %phi685, i32 12
  %temp.vect198 = insertelement <16 x float> %temp.vect197, float %phi691, i32 13
  %temp.vect199 = insertelement <16 x float> %temp.vect198, float %phi697, i32 14
  %temp.vect200 = insertelement <16 x float> %temp.vect199, float %phi703, i32 15
  %115 = fadd <16 x float> %temp.vect184, %temp.vect200
  %116 = fadd <16 x float> %vectorPHI115, %115
  %loadedValue1016 = load float addrspace(3)** %CastToValueType1015, align 8
  %exmask = extractelement <16 x i1> %vectorPHI114, i32 0
  br i1 %exmask, label %preload575, label %postload576

preload575:                                       ; preds = %postload702
  %exData = extractelement <16 x float> %116, i32 0
  store float %exData, float addrspace(3)* %loadedValue1016, align 4
  br label %postload576

postload576:                                      ; preds = %preload575, %postload702
  %exmask381 = extractelement <16 x i1> %vectorPHI114, i32 1
  br i1 %exmask381, label %preload599, label %postload600

preload599:                                       ; preds = %postload576
  %117 = getelementptr float addrspace(3)* %loadedValue1016, i64 1
  %exData382 = extractelement <16 x float> %116, i32 1
  store float %exData382, float addrspace(3)* %117, align 4
  br label %postload600

postload600:                                      ; preds = %preload599, %postload576
  %exmask384 = extractelement <16 x i1> %vectorPHI114, i32 2
  br i1 %exmask384, label %preload596, label %postload597

preload596:                                       ; preds = %postload600
  %118 = getelementptr float addrspace(3)* %loadedValue1016, i64 2
  %exData385 = extractelement <16 x float> %116, i32 2
  store float %exData385, float addrspace(3)* %118, align 4
  br label %postload597

postload597:                                      ; preds = %preload596, %postload600
  %exmask387 = extractelement <16 x i1> %vectorPHI114, i32 3
  br i1 %exmask387, label %preload572, label %postload573

preload572:                                       ; preds = %postload597
  %119 = getelementptr float addrspace(3)* %loadedValue1016, i64 3
  %exData388 = extractelement <16 x float> %116, i32 3
  store float %exData388, float addrspace(3)* %119, align 4
  br label %postload573

postload573:                                      ; preds = %preload572, %postload597
  %exmask390 = extractelement <16 x i1> %vectorPHI114, i32 4
  br i1 %exmask390, label %preload707, label %postload708

preload707:                                       ; preds = %postload573
  %120 = getelementptr float addrspace(3)* %loadedValue1016, i64 4
  %exData391 = extractelement <16 x float> %116, i32 4
  store float %exData391, float addrspace(3)* %120, align 4
  br label %postload708

postload708:                                      ; preds = %preload707, %postload573
  %exmask393 = extractelement <16 x i1> %vectorPHI114, i32 5
  br i1 %exmask393, label %preload, label %postload

preload:                                          ; preds = %postload708
  %121 = getelementptr float addrspace(3)* %loadedValue1016, i64 5
  %exData394 = extractelement <16 x float> %116, i32 5
  store float %exData394, float addrspace(3)* %121, align 4
  br label %postload

postload:                                         ; preds = %preload, %postload708
  %exmask396 = extractelement <16 x i1> %vectorPHI114, i32 6
  br i1 %exmask396, label %preload602, label %postload603

preload602:                                       ; preds = %postload
  %122 = getelementptr float addrspace(3)* %loadedValue1016, i64 6
  %exData397 = extractelement <16 x float> %116, i32 6
  store float %exData397, float addrspace(3)* %122, align 4
  br label %postload603

postload603:                                      ; preds = %preload602, %postload
  %exmask399 = extractelement <16 x i1> %vectorPHI114, i32 7
  br i1 %exmask399, label %preload569, label %postload570

preload569:                                       ; preds = %postload603
  %123 = getelementptr float addrspace(3)* %loadedValue1016, i64 7
  %exData400 = extractelement <16 x float> %116, i32 7
  store float %exData400, float addrspace(3)* %123, align 4
  br label %postload570

postload570:                                      ; preds = %preload569, %postload603
  %exmask402 = extractelement <16 x i1> %vectorPHI114, i32 8
  br i1 %exmask402, label %preload593, label %postload594

preload593:                                       ; preds = %postload570
  %124 = getelementptr float addrspace(3)* %loadedValue1016, i64 8
  %exData403 = extractelement <16 x float> %116, i32 8
  store float %exData403, float addrspace(3)* %124, align 4
  br label %postload594

postload594:                                      ; preds = %preload593, %postload570
  %exmask405 = extractelement <16 x i1> %vectorPHI114, i32 9
  br i1 %exmask405, label %preload704, label %postload705

preload704:                                       ; preds = %postload594
  %125 = getelementptr float addrspace(3)* %loadedValue1016, i64 9
  %exData406 = extractelement <16 x float> %116, i32 9
  store float %exData406, float addrspace(3)* %125, align 4
  br label %postload705

postload705:                                      ; preds = %preload704, %postload594
  %exmask408 = extractelement <16 x i1> %vectorPHI114, i32 10
  br i1 %exmask408, label %preload584, label %postload585

preload584:                                       ; preds = %postload705
  %126 = getelementptr float addrspace(3)* %loadedValue1016, i64 10
  %exData409 = extractelement <16 x float> %116, i32 10
  store float %exData409, float addrspace(3)* %126, align 4
  br label %postload585

postload585:                                      ; preds = %preload584, %postload705
  %exmask411 = extractelement <16 x i1> %vectorPHI114, i32 11
  br i1 %exmask411, label %preload710, label %postload711

preload710:                                       ; preds = %postload585
  %127 = getelementptr float addrspace(3)* %loadedValue1016, i64 11
  %exData412 = extractelement <16 x float> %116, i32 11
  store float %exData412, float addrspace(3)* %127, align 4
  br label %postload711

postload711:                                      ; preds = %preload710, %postload585
  %exmask414 = extractelement <16 x i1> %vectorPHI114, i32 12
  br i1 %exmask414, label %preload590, label %postload591

preload590:                                       ; preds = %postload711
  %128 = getelementptr float addrspace(3)* %loadedValue1016, i64 12
  %exData415 = extractelement <16 x float> %116, i32 12
  store float %exData415, float addrspace(3)* %128, align 4
  br label %postload591

postload591:                                      ; preds = %preload590, %postload711
  %exmask417 = extractelement <16 x i1> %vectorPHI114, i32 13
  br i1 %exmask417, label %preload581, label %postload582

preload581:                                       ; preds = %postload591
  %129 = getelementptr float addrspace(3)* %loadedValue1016, i64 13
  %exData418 = extractelement <16 x float> %116, i32 13
  store float %exData418, float addrspace(3)* %129, align 4
  br label %postload582

postload582:                                      ; preds = %preload581, %postload591
  %exmask420 = extractelement <16 x i1> %vectorPHI114, i32 14
  br i1 %exmask420, label %preload587, label %postload588

preload587:                                       ; preds = %postload582
  %130 = getelementptr float addrspace(3)* %loadedValue1016, i64 14
  %exData421 = extractelement <16 x float> %116, i32 14
  store float %exData421, float addrspace(3)* %130, align 4
  br label %postload588

postload588:                                      ; preds = %preload587, %postload582
  %exmask423 = extractelement <16 x i1> %vectorPHI114, i32 15
  br i1 %exmask423, label %preload578, label %postload579

preload578:                                       ; preds = %postload588
  %131 = getelementptr float addrspace(3)* %loadedValue1016, i64 15
  %exData424 = extractelement <16 x float> %116, i32 15
  store float %exData424, float addrspace(3)* %131, align 4
  br label %postload579

postload579:                                      ; preds = %preload578, %postload588
  %tmp18202 = add <16 x i32> %tmp17111, %tmp8118
  %132 = icmp ult <16 x i32> %tmp18202, %vector82
  %indvar.next = add i32 %indvar, 1
  %notCond203 = xor <16 x i1> %132, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr204 = and <16 x i1> %vectorPHI114, %notCond203
  %loop_mask11206 = or <16 x i1> %vectorPHI, %who_left_tr204
  %curr_loop_mask207 = or <16 x i1> %loop_mask11206, %who_left_tr204
  %ipred.i1 = bitcast <16 x i1> %curr_loop_mask207 to i16
  %val.i2 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1, i16 %ipred.i1) nounwind
  %tmp.i3 = and i32 %val.i2, 1
  %res.i4 = icmp eq i32 %tmp.i3, 0
  %local_edge225 = and <16 x i1> %vectorPHI114, %132
  br i1 %res.i4, label %18, label %._crit_edge5

._crit_edge5:                                     ; preds = %postload579, %SyncBB2172
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %._crit_edge5
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 2752
  br label %SyncBB2172

elseBB:                                           ; preds = %._crit_edge5
  %s.01 = lshr i32 %15, 1
  %Mneg13 = icmp ne i32 %s.01, 0
  %temp247 = insertelement <16 x i1> undef, i1 %Mneg13, i32 0
  %vector248 = shufflevector <16 x i1> %temp247, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMask40 = xor i1 %Mneg13, true
  %temp244 = insertelement <16 x i1> undef, i1 %negIncomingLoopMask40, i32 0
  %vector245 = shufflevector <16 x i1> %temp244, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB2184, %thenBB2177, %thenBB2192, %elseBB
  %currBarrier.4 = phi i32 [ 2, %elseBB ], [ %currBarrier.3, %thenBB2177 ], [ %currBarrier.2, %thenBB2192 ], [ %currBarrier.3, %thenBB2184 ]
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride2183", %thenBB2177 ], [ %"loadedCurrSB+Stride2198", %thenBB2192 ], [ %"loadedCurrSB+Stride2190", %thenBB2184 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++2181", %thenBB2177 ], [ %"CurrWI++2196", %thenBB2192 ], [ %"CurrWI++2188", %thenBB2184 ]
  br i1 %Mneg13, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %postload798, %SyncBB
  %currBarrier.2 = phi i32 [ %currBarrier.1, %postload798 ], [ %currBarrier.4, %SyncBB ]
  %CurrSBIndex..4 = phi i64 [ %CurrSBIndex..3, %postload798 ], [ %CurrSBIndex..1, %SyncBB ]
  %CurrWI..4 = phi i64 [ %CurrWI..3, %postload798 ], [ %CurrWI..1, %SyncBB ]
  %vectorPHI243 = phi <16 x i1> [ %loop_mask24326, %postload798 ], [ %vector245, %SyncBB ]
  %vectorPHI246 = phi <16 x i1> [ %local_edge29330, %postload798 ], [ %vector248, %SyncBB ]
  %s.02 = phi i32 [ %s.0, %postload798 ], [ %s.01, %SyncBB ]
  %"&(pSB[currWI].offset)1046" = add nuw i64 %CurrSBIndex..4, 164
  %"&pSB[currWI].offset1047" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1046"
  %CastToValueType1048 = bitcast i8* %"&pSB[currWI].offset1047" to i32*
  store i32 %s.02, i32* %CastToValueType1048, align 4
  %"&(pSB[currWI].offset)1027" = add nuw i64 %CurrSBIndex..4, 160
  %"&pSB[currWI].offset1028" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1027"
  %CastToValueType1029 = bitcast i8* %"&pSB[currWI].offset1028" to <16 x i1>*
  store <16 x i1> %vectorPHI246, <16 x i1>* %CastToValueType1029, align 16
  %"&(pSB[currWI].offset)1018" = add nuw i64 %CurrSBIndex..4, 144
  %"&pSB[currWI].offset1019" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1018"
  %CastToValueType1020 = bitcast i8* %"&pSB[currWI].offset1019" to <16 x i1>*
  store <16 x i1> %vectorPHI243, <16 x i1>* %CastToValueType1020, align 16
  %temp249 = insertelement <16 x i32> undef, i32 %s.02, i32 0
  %vector250 = shufflevector <16 x i32> %temp249, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)994" = add nuw i64 %CurrSBIndex..4, 64
  %"&pSB[currWI].offset995" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)994"
  %CastToValueType996 = bitcast i8* %"&pSB[currWI].offset995" to <16 x i32>*
  %loadedValue997 = load <16 x i32>* %CastToValueType996, align 64
  %133 = icmp ult <16 x i32> %loadedValue997, %vector250
  %bb.nph_to_17253 = and <16 x i1> %vectorPHI246, %133
  %"&(pSB[currWI].offset)1055" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1056" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1055"
  %CastToValueType1057 = bitcast i8* %"&pSB[currWI].offset1056" to <16 x i1>*
  store <16 x i1> %bb.nph_to_17253, <16 x i1>* %CastToValueType1057, align 16
  %extract271 = extractelement <16 x i1> %bb.nph_to_17253, i32 1
  %"&(pSB[currWI].offset)1219" = add nuw i64 %CurrSBIndex..4, 178
  %"&pSB[currWI].offset1220" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1219"
  %CastToValueType1221 = bitcast i8* %"&pSB[currWI].offset1220" to i1*
  store i1 %extract271, i1* %CastToValueType1221, align 1
  %extract272 = extractelement <16 x i1> %bb.nph_to_17253, i32 2
  %"&(pSB[currWI].offset)1228" = add nuw i64 %CurrSBIndex..4, 179
  %"&pSB[currWI].offset1229" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1228"
  %CastToValueType1230 = bitcast i8* %"&pSB[currWI].offset1229" to i1*
  store i1 %extract272, i1* %CastToValueType1230, align 1
  %extract273 = extractelement <16 x i1> %bb.nph_to_17253, i32 3
  %"&(pSB[currWI].offset)1237" = add nuw i64 %CurrSBIndex..4, 180
  %"&pSB[currWI].offset1238" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1237"
  %CastToValueType1239 = bitcast i8* %"&pSB[currWI].offset1238" to i1*
  store i1 %extract273, i1* %CastToValueType1239, align 1
  %extract274 = extractelement <16 x i1> %bb.nph_to_17253, i32 4
  %"&(pSB[currWI].offset)1246" = add nuw i64 %CurrSBIndex..4, 181
  %"&pSB[currWI].offset1247" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1246"
  %CastToValueType1248 = bitcast i8* %"&pSB[currWI].offset1247" to i1*
  store i1 %extract274, i1* %CastToValueType1248, align 1
  %extract275 = extractelement <16 x i1> %bb.nph_to_17253, i32 5
  %"&(pSB[currWI].offset)1255" = add nuw i64 %CurrSBIndex..4, 182
  %"&pSB[currWI].offset1256" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1255"
  %CastToValueType1257 = bitcast i8* %"&pSB[currWI].offset1256" to i1*
  store i1 %extract275, i1* %CastToValueType1257, align 1
  %extract276 = extractelement <16 x i1> %bb.nph_to_17253, i32 6
  %"&(pSB[currWI].offset)1264" = add nuw i64 %CurrSBIndex..4, 183
  %"&pSB[currWI].offset1265" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1264"
  %CastToValueType1266 = bitcast i8* %"&pSB[currWI].offset1265" to i1*
  store i1 %extract276, i1* %CastToValueType1266, align 1
  %extract277 = extractelement <16 x i1> %bb.nph_to_17253, i32 7
  %"&(pSB[currWI].offset)1273" = add nuw i64 %CurrSBIndex..4, 184
  %"&pSB[currWI].offset1274" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1273"
  %CastToValueType1275 = bitcast i8* %"&pSB[currWI].offset1274" to i1*
  store i1 %extract277, i1* %CastToValueType1275, align 1
  %extract278 = extractelement <16 x i1> %bb.nph_to_17253, i32 8
  %"&(pSB[currWI].offset)1282" = add nuw i64 %CurrSBIndex..4, 185
  %"&pSB[currWI].offset1283" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1282"
  %CastToValueType1284 = bitcast i8* %"&pSB[currWI].offset1283" to i1*
  store i1 %extract278, i1* %CastToValueType1284, align 1
  %extract279 = extractelement <16 x i1> %bb.nph_to_17253, i32 9
  %"&(pSB[currWI].offset)1291" = add nuw i64 %CurrSBIndex..4, 186
  %"&pSB[currWI].offset1292" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1291"
  %CastToValueType1293 = bitcast i8* %"&pSB[currWI].offset1292" to i1*
  store i1 %extract279, i1* %CastToValueType1293, align 1
  %extract280 = extractelement <16 x i1> %bb.nph_to_17253, i32 10
  %"&(pSB[currWI].offset)1300" = add nuw i64 %CurrSBIndex..4, 187
  %"&pSB[currWI].offset1301" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1300"
  %CastToValueType1302 = bitcast i8* %"&pSB[currWI].offset1301" to i1*
  store i1 %extract280, i1* %CastToValueType1302, align 1
  %extract281 = extractelement <16 x i1> %bb.nph_to_17253, i32 11
  %"&(pSB[currWI].offset)1309" = add nuw i64 %CurrSBIndex..4, 188
  %"&pSB[currWI].offset1310" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1309"
  %CastToValueType1311 = bitcast i8* %"&pSB[currWI].offset1310" to i1*
  store i1 %extract281, i1* %CastToValueType1311, align 1
  %extract282 = extractelement <16 x i1> %bb.nph_to_17253, i32 12
  %"&(pSB[currWI].offset)1318" = add nuw i64 %CurrSBIndex..4, 189
  %"&pSB[currWI].offset1319" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1318"
  %CastToValueType1320 = bitcast i8* %"&pSB[currWI].offset1319" to i1*
  store i1 %extract282, i1* %CastToValueType1320, align 1
  %extract283 = extractelement <16 x i1> %bb.nph_to_17253, i32 13
  %"&(pSB[currWI].offset)1327" = add nuw i64 %CurrSBIndex..4, 190
  %"&pSB[currWI].offset1328" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1327"
  %CastToValueType1329 = bitcast i8* %"&pSB[currWI].offset1328" to i1*
  store i1 %extract283, i1* %CastToValueType1329, align 1
  %extract284 = extractelement <16 x i1> %bb.nph_to_17253, i32 14
  %"&(pSB[currWI].offset)1336" = add nuw i64 %CurrSBIndex..4, 191
  %"&pSB[currWI].offset1337" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1336"
  %CastToValueType1338 = bitcast i8* %"&pSB[currWI].offset1337" to i1*
  store i1 %extract284, i1* %CastToValueType1338, align 1
  %extract285 = extractelement <16 x i1> %bb.nph_to_17253, i32 15
  %"&(pSB[currWI].offset)1345" = add nuw i64 %CurrSBIndex..4, 192
  %"&pSB[currWI].offset1346" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1345"
  %CastToValueType1347 = bitcast i8* %"&pSB[currWI].offset1346" to i1*
  store i1 %extract285, i1* %CastToValueType1347, align 1
  %extract270 = extractelement <16 x i1> %bb.nph_to_17253, i32 0
  %"&(pSB[currWI].offset)989" = add nuw i64 %CurrSBIndex..4, 64
  %"&pSB[currWI].offset990" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)989"
  %CastToValueType991 = bitcast i8* %"&pSB[currWI].offset990" to <16 x i32>*
  %loadedValue992 = load <16 x i32>* %CastToValueType991, align 64
  %134 = add <16 x i32> %vector250, %loadedValue992
  %"&(pSB[currWI].offset)1354" = add nuw i64 %CurrSBIndex..4, 256
  %"&pSB[currWI].offset1355" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1354"
  %CastToValueType1356 = bitcast i8* %"&pSB[currWI].offset1355" to <16 x i32>*
  store <16 x i32> %134, <16 x i32>* %CastToValueType1356, align 64
  %135 = extractelement <16 x i32> %134, i32 1
  %136 = zext i32 %135 to i64
  %137 = getelementptr inbounds float addrspace(3)* %sdata, i64 %136
  %"&(pSB[currWI].offset)1363" = add nuw i64 %CurrSBIndex..4, 320
  %"&pSB[currWI].offset1364" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1363"
  %CastToValueType1365 = bitcast i8* %"&pSB[currWI].offset1364" to float addrspace(3)**
  store float addrspace(3)* %137, float addrspace(3)** %CastToValueType1365, align 8
  %138 = extractelement <16 x i32> %134, i32 2
  %139 = zext i32 %138 to i64
  %140 = getelementptr inbounds float addrspace(3)* %sdata, i64 %139
  %"&(pSB[currWI].offset)1372" = add nuw i64 %CurrSBIndex..4, 328
  %"&pSB[currWI].offset1373" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1372"
  %CastToValueType1374 = bitcast i8* %"&pSB[currWI].offset1373" to float addrspace(3)**
  store float addrspace(3)* %140, float addrspace(3)** %CastToValueType1374, align 8
  %141 = extractelement <16 x i32> %134, i32 3
  %142 = zext i32 %141 to i64
  %143 = getelementptr inbounds float addrspace(3)* %sdata, i64 %142
  %"&(pSB[currWI].offset)1381" = add nuw i64 %CurrSBIndex..4, 336
  %"&pSB[currWI].offset1382" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1381"
  %CastToValueType1383 = bitcast i8* %"&pSB[currWI].offset1382" to float addrspace(3)**
  store float addrspace(3)* %143, float addrspace(3)** %CastToValueType1383, align 8
  %144 = extractelement <16 x i32> %134, i32 4
  %145 = zext i32 %144 to i64
  %146 = getelementptr inbounds float addrspace(3)* %sdata, i64 %145
  %"&(pSB[currWI].offset)1390" = add nuw i64 %CurrSBIndex..4, 344
  %"&pSB[currWI].offset1391" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1390"
  %CastToValueType1392 = bitcast i8* %"&pSB[currWI].offset1391" to float addrspace(3)**
  store float addrspace(3)* %146, float addrspace(3)** %CastToValueType1392, align 8
  %147 = extractelement <16 x i32> %134, i32 5
  %148 = zext i32 %147 to i64
  %149 = getelementptr inbounds float addrspace(3)* %sdata, i64 %148
  %"&(pSB[currWI].offset)1399" = add nuw i64 %CurrSBIndex..4, 352
  %"&pSB[currWI].offset1400" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1399"
  %CastToValueType1401 = bitcast i8* %"&pSB[currWI].offset1400" to float addrspace(3)**
  store float addrspace(3)* %149, float addrspace(3)** %CastToValueType1401, align 8
  %150 = extractelement <16 x i32> %134, i32 6
  %151 = zext i32 %150 to i64
  %152 = getelementptr inbounds float addrspace(3)* %sdata, i64 %151
  %"&(pSB[currWI].offset)1408" = add nuw i64 %CurrSBIndex..4, 360
  %"&pSB[currWI].offset1409" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1408"
  %CastToValueType1410 = bitcast i8* %"&pSB[currWI].offset1409" to float addrspace(3)**
  store float addrspace(3)* %152, float addrspace(3)** %CastToValueType1410, align 8
  %153 = extractelement <16 x i32> %134, i32 7
  %154 = zext i32 %153 to i64
  %155 = getelementptr inbounds float addrspace(3)* %sdata, i64 %154
  %"&(pSB[currWI].offset)1417" = add nuw i64 %CurrSBIndex..4, 368
  %"&pSB[currWI].offset1418" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1417"
  %CastToValueType1419 = bitcast i8* %"&pSB[currWI].offset1418" to float addrspace(3)**
  store float addrspace(3)* %155, float addrspace(3)** %CastToValueType1419, align 8
  %156 = extractelement <16 x i32> %134, i32 8
  %157 = zext i32 %156 to i64
  %158 = getelementptr inbounds float addrspace(3)* %sdata, i64 %157
  %"&(pSB[currWI].offset)1426" = add nuw i64 %CurrSBIndex..4, 376
  %"&pSB[currWI].offset1427" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1426"
  %CastToValueType1428 = bitcast i8* %"&pSB[currWI].offset1427" to float addrspace(3)**
  store float addrspace(3)* %158, float addrspace(3)** %CastToValueType1428, align 8
  %159 = extractelement <16 x i32> %134, i32 9
  %160 = zext i32 %159 to i64
  %161 = getelementptr inbounds float addrspace(3)* %sdata, i64 %160
  %"&(pSB[currWI].offset)1435" = add nuw i64 %CurrSBIndex..4, 384
  %"&pSB[currWI].offset1436" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1435"
  %CastToValueType1437 = bitcast i8* %"&pSB[currWI].offset1436" to float addrspace(3)**
  store float addrspace(3)* %161, float addrspace(3)** %CastToValueType1437, align 8
  %162 = extractelement <16 x i32> %134, i32 10
  %163 = zext i32 %162 to i64
  %164 = getelementptr inbounds float addrspace(3)* %sdata, i64 %163
  %"&(pSB[currWI].offset)1444" = add nuw i64 %CurrSBIndex..4, 392
  %"&pSB[currWI].offset1445" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1444"
  %CastToValueType1446 = bitcast i8* %"&pSB[currWI].offset1445" to float addrspace(3)**
  store float addrspace(3)* %164, float addrspace(3)** %CastToValueType1446, align 8
  %165 = extractelement <16 x i32> %134, i32 11
  %166 = zext i32 %165 to i64
  %167 = getelementptr inbounds float addrspace(3)* %sdata, i64 %166
  %"&(pSB[currWI].offset)1453" = add nuw i64 %CurrSBIndex..4, 400
  %"&pSB[currWI].offset1454" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1453"
  %CastToValueType1455 = bitcast i8* %"&pSB[currWI].offset1454" to float addrspace(3)**
  store float addrspace(3)* %167, float addrspace(3)** %CastToValueType1455, align 8
  %168 = extractelement <16 x i32> %134, i32 12
  %169 = zext i32 %168 to i64
  %170 = getelementptr inbounds float addrspace(3)* %sdata, i64 %169
  %"&(pSB[currWI].offset)1462" = add nuw i64 %CurrSBIndex..4, 408
  %"&pSB[currWI].offset1463" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1462"
  %CastToValueType1464 = bitcast i8* %"&pSB[currWI].offset1463" to float addrspace(3)**
  store float addrspace(3)* %170, float addrspace(3)** %CastToValueType1464, align 8
  %171 = extractelement <16 x i32> %134, i32 13
  %172 = zext i32 %171 to i64
  %173 = getelementptr inbounds float addrspace(3)* %sdata, i64 %172
  %"&(pSB[currWI].offset)1471" = add nuw i64 %CurrSBIndex..4, 416
  %"&pSB[currWI].offset1472" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1471"
  %CastToValueType1473 = bitcast i8* %"&pSB[currWI].offset1472" to float addrspace(3)**
  store float addrspace(3)* %173, float addrspace(3)** %CastToValueType1473, align 8
  %174 = extractelement <16 x i32> %134, i32 14
  %175 = zext i32 %174 to i64
  %176 = getelementptr inbounds float addrspace(3)* %sdata, i64 %175
  %"&(pSB[currWI].offset)1480" = add nuw i64 %CurrSBIndex..4, 424
  %"&pSB[currWI].offset1481" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1480"
  %CastToValueType1482 = bitcast i8* %"&pSB[currWI].offset1481" to float addrspace(3)**
  store float addrspace(3)* %176, float addrspace(3)** %CastToValueType1482, align 8
  %177 = extractelement <16 x i32> %134, i32 15
  %178 = zext i32 %177 to i64
  %179 = getelementptr inbounds float addrspace(3)* %sdata, i64 %178
  %"&(pSB[currWI].offset)1489" = add nuw i64 %CurrSBIndex..4, 432
  %"&pSB[currWI].offset1490" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1489"
  %CastToValueType1491 = bitcast i8* %"&pSB[currWI].offset1490" to float addrspace(3)**
  store float addrspace(3)* %179, float addrspace(3)** %CastToValueType1491, align 8
  br i1 %extract270, label %preload749, label %postload750

preload749:                                       ; preds = %bb.nph
  %"&(pSB[currWI].offset)1358" = add nuw i64 %CurrSBIndex..4, 256
  %"&pSB[currWI].offset1359" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1358"
  %CastToValueType1360 = bitcast i8* %"&pSB[currWI].offset1359" to <16 x i32>*
  %loadedValue1361 = load <16 x i32>* %CastToValueType1360, align 64
  %180 = extractelement <16 x i32> %loadedValue1361, i32 0
  %181 = zext i32 %180 to i64
  %182 = getelementptr inbounds float addrspace(3)* %sdata, i64 %181
  %masked_load425 = load float addrspace(3)* %182, align 4
  %"&(pSB[currWI].offset)1498" = add nuw i64 %CurrSBIndex..4, 440
  %"&pSB[currWI].offset1499" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1498"
  %CastToValueType1500 = bitcast i8* %"&pSB[currWI].offset1499" to float*
  store float %masked_load425, float* %CastToValueType1500, align 4
  br label %postload750

postload750:                                      ; preds = %preload749, %bb.nph
  %phi751 = phi float [ undef, %bb.nph ], [ %masked_load425, %preload749 ]
  %"&(pSB[currWI].offset)1502" = add nuw i64 %CurrSBIndex..4, 444
  %"&pSB[currWI].offset1503" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1502"
  %CastToValueType1504 = bitcast i8* %"&pSB[currWI].offset1503" to float*
  store float %phi751, float* %CastToValueType1504, align 4
  %"&(pSB[currWI].offset)1223" = add nuw i64 %CurrSBIndex..4, 178
  %"&pSB[currWI].offset1224" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1223"
  %CastToValueType1225 = bitcast i8* %"&pSB[currWI].offset1224" to i1*
  %loadedValue1226 = load i1* %CastToValueType1225, align 1
  br i1 %loadedValue1226, label %preload752, label %postload753

preload752:                                       ; preds = %postload750
  %"&(pSB[currWI].offset)1367" = add nuw i64 %CurrSBIndex..4, 320
  %"&pSB[currWI].offset1368" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1367"
  %CastToValueType1369 = bitcast i8* %"&pSB[currWI].offset1368" to float addrspace(3)**
  %loadedValue1370 = load float addrspace(3)** %CastToValueType1369, align 8
  %masked_load426 = load float addrspace(3)* %loadedValue1370, align 4
  %"&(pSB[currWI].offset)1511" = add nuw i64 %CurrSBIndex..4, 448
  %"&pSB[currWI].offset1512" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1511"
  %CastToValueType1513 = bitcast i8* %"&pSB[currWI].offset1512" to float*
  store float %masked_load426, float* %CastToValueType1513, align 4
  br label %postload753

postload753:                                      ; preds = %preload752, %postload750
  %phi754 = phi float [ undef, %postload750 ], [ %masked_load426, %preload752 ]
  %"&(pSB[currWI].offset)1515" = add nuw i64 %CurrSBIndex..4, 452
  %"&pSB[currWI].offset1516" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1515"
  %CastToValueType1517 = bitcast i8* %"&pSB[currWI].offset1516" to float*
  store float %phi754, float* %CastToValueType1517, align 4
  %"&(pSB[currWI].offset)1232" = add nuw i64 %CurrSBIndex..4, 179
  %"&pSB[currWI].offset1233" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1232"
  %CastToValueType1234 = bitcast i8* %"&pSB[currWI].offset1233" to i1*
  %loadedValue1235 = load i1* %CastToValueType1234, align 1
  br i1 %loadedValue1235, label %preload755, label %postload756

preload755:                                       ; preds = %postload753
  %"&(pSB[currWI].offset)1376" = add nuw i64 %CurrSBIndex..4, 328
  %"&pSB[currWI].offset1377" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1376"
  %CastToValueType1378 = bitcast i8* %"&pSB[currWI].offset1377" to float addrspace(3)**
  %loadedValue1379 = load float addrspace(3)** %CastToValueType1378, align 8
  %masked_load427 = load float addrspace(3)* %loadedValue1379, align 4
  %"&(pSB[currWI].offset)1524" = add nuw i64 %CurrSBIndex..4, 456
  %"&pSB[currWI].offset1525" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1524"
  %CastToValueType1526 = bitcast i8* %"&pSB[currWI].offset1525" to float*
  store float %masked_load427, float* %CastToValueType1526, align 4
  br label %postload756

postload756:                                      ; preds = %preload755, %postload753
  %phi757 = phi float [ undef, %postload753 ], [ %masked_load427, %preload755 ]
  %"&(pSB[currWI].offset)1528" = add nuw i64 %CurrSBIndex..4, 460
  %"&pSB[currWI].offset1529" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1528"
  %CastToValueType1530 = bitcast i8* %"&pSB[currWI].offset1529" to float*
  store float %phi757, float* %CastToValueType1530, align 4
  %"&(pSB[currWI].offset)1241" = add nuw i64 %CurrSBIndex..4, 180
  %"&pSB[currWI].offset1242" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1241"
  %CastToValueType1243 = bitcast i8* %"&pSB[currWI].offset1242" to i1*
  %loadedValue1244 = load i1* %CastToValueType1243, align 1
  br i1 %loadedValue1244, label %preload758, label %postload759

preload758:                                       ; preds = %postload756
  %"&(pSB[currWI].offset)1385" = add nuw i64 %CurrSBIndex..4, 336
  %"&pSB[currWI].offset1386" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1385"
  %CastToValueType1387 = bitcast i8* %"&pSB[currWI].offset1386" to float addrspace(3)**
  %loadedValue1388 = load float addrspace(3)** %CastToValueType1387, align 8
  %masked_load428 = load float addrspace(3)* %loadedValue1388, align 4
  %"&(pSB[currWI].offset)1537" = add nuw i64 %CurrSBIndex..4, 464
  %"&pSB[currWI].offset1538" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1537"
  %CastToValueType1539 = bitcast i8* %"&pSB[currWI].offset1538" to float*
  store float %masked_load428, float* %CastToValueType1539, align 4
  br label %postload759

postload759:                                      ; preds = %preload758, %postload756
  %phi760 = phi float [ undef, %postload756 ], [ %masked_load428, %preload758 ]
  %"&(pSB[currWI].offset)1541" = add nuw i64 %CurrSBIndex..4, 468
  %"&pSB[currWI].offset1542" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1541"
  %CastToValueType1543 = bitcast i8* %"&pSB[currWI].offset1542" to float*
  store float %phi760, float* %CastToValueType1543, align 4
  %"&(pSB[currWI].offset)1250" = add nuw i64 %CurrSBIndex..4, 181
  %"&pSB[currWI].offset1251" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1250"
  %CastToValueType1252 = bitcast i8* %"&pSB[currWI].offset1251" to i1*
  %loadedValue1253 = load i1* %CastToValueType1252, align 1
  br i1 %loadedValue1253, label %preload761, label %postload762

preload761:                                       ; preds = %postload759
  %"&(pSB[currWI].offset)1394" = add nuw i64 %CurrSBIndex..4, 344
  %"&pSB[currWI].offset1395" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1394"
  %CastToValueType1396 = bitcast i8* %"&pSB[currWI].offset1395" to float addrspace(3)**
  %loadedValue1397 = load float addrspace(3)** %CastToValueType1396, align 8
  %masked_load429 = load float addrspace(3)* %loadedValue1397, align 4
  %"&(pSB[currWI].offset)1550" = add nuw i64 %CurrSBIndex..4, 472
  %"&pSB[currWI].offset1551" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1550"
  %CastToValueType1552 = bitcast i8* %"&pSB[currWI].offset1551" to float*
  store float %masked_load429, float* %CastToValueType1552, align 4
  br label %postload762

postload762:                                      ; preds = %preload761, %postload759
  %phi763 = phi float [ undef, %postload759 ], [ %masked_load429, %preload761 ]
  %"&(pSB[currWI].offset)1554" = add nuw i64 %CurrSBIndex..4, 476
  %"&pSB[currWI].offset1555" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1554"
  %CastToValueType1556 = bitcast i8* %"&pSB[currWI].offset1555" to float*
  store float %phi763, float* %CastToValueType1556, align 4
  %"&(pSB[currWI].offset)1259" = add nuw i64 %CurrSBIndex..4, 182
  %"&pSB[currWI].offset1260" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1259"
  %CastToValueType1261 = bitcast i8* %"&pSB[currWI].offset1260" to i1*
  %loadedValue1262 = load i1* %CastToValueType1261, align 1
  br i1 %loadedValue1262, label %preload764, label %postload765

preload764:                                       ; preds = %postload762
  %"&(pSB[currWI].offset)1403" = add nuw i64 %CurrSBIndex..4, 352
  %"&pSB[currWI].offset1404" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1403"
  %CastToValueType1405 = bitcast i8* %"&pSB[currWI].offset1404" to float addrspace(3)**
  %loadedValue1406 = load float addrspace(3)** %CastToValueType1405, align 8
  %masked_load430 = load float addrspace(3)* %loadedValue1406, align 4
  %"&(pSB[currWI].offset)1563" = add nuw i64 %CurrSBIndex..4, 480
  %"&pSB[currWI].offset1564" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1563"
  %CastToValueType1565 = bitcast i8* %"&pSB[currWI].offset1564" to float*
  store float %masked_load430, float* %CastToValueType1565, align 4
  br label %postload765

postload765:                                      ; preds = %preload764, %postload762
  %phi766 = phi float [ undef, %postload762 ], [ %masked_load430, %preload764 ]
  %"&(pSB[currWI].offset)1567" = add nuw i64 %CurrSBIndex..4, 484
  %"&pSB[currWI].offset1568" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1567"
  %CastToValueType1569 = bitcast i8* %"&pSB[currWI].offset1568" to float*
  store float %phi766, float* %CastToValueType1569, align 4
  %"&(pSB[currWI].offset)1268" = add nuw i64 %CurrSBIndex..4, 183
  %"&pSB[currWI].offset1269" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1268"
  %CastToValueType1270 = bitcast i8* %"&pSB[currWI].offset1269" to i1*
  %loadedValue1271 = load i1* %CastToValueType1270, align 1
  br i1 %loadedValue1271, label %preload767, label %postload768

preload767:                                       ; preds = %postload765
  %"&(pSB[currWI].offset)1412" = add nuw i64 %CurrSBIndex..4, 360
  %"&pSB[currWI].offset1413" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1412"
  %CastToValueType1414 = bitcast i8* %"&pSB[currWI].offset1413" to float addrspace(3)**
  %loadedValue1415 = load float addrspace(3)** %CastToValueType1414, align 8
  %masked_load431 = load float addrspace(3)* %loadedValue1415, align 4
  %"&(pSB[currWI].offset)1576" = add nuw i64 %CurrSBIndex..4, 488
  %"&pSB[currWI].offset1577" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1576"
  %CastToValueType1578 = bitcast i8* %"&pSB[currWI].offset1577" to float*
  store float %masked_load431, float* %CastToValueType1578, align 4
  br label %postload768

postload768:                                      ; preds = %preload767, %postload765
  %phi769 = phi float [ undef, %postload765 ], [ %masked_load431, %preload767 ]
  %"&(pSB[currWI].offset)1580" = add nuw i64 %CurrSBIndex..4, 492
  %"&pSB[currWI].offset1581" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1580"
  %CastToValueType1582 = bitcast i8* %"&pSB[currWI].offset1581" to float*
  store float %phi769, float* %CastToValueType1582, align 4
  %"&(pSB[currWI].offset)1277" = add nuw i64 %CurrSBIndex..4, 184
  %"&pSB[currWI].offset1278" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1277"
  %CastToValueType1279 = bitcast i8* %"&pSB[currWI].offset1278" to i1*
  %loadedValue1280 = load i1* %CastToValueType1279, align 1
  br i1 %loadedValue1280, label %preload770, label %postload771

preload770:                                       ; preds = %postload768
  %"&(pSB[currWI].offset)1421" = add nuw i64 %CurrSBIndex..4, 368
  %"&pSB[currWI].offset1422" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1421"
  %CastToValueType1423 = bitcast i8* %"&pSB[currWI].offset1422" to float addrspace(3)**
  %loadedValue1424 = load float addrspace(3)** %CastToValueType1423, align 8
  %masked_load432 = load float addrspace(3)* %loadedValue1424, align 4
  %"&(pSB[currWI].offset)1589" = add nuw i64 %CurrSBIndex..4, 496
  %"&pSB[currWI].offset1590" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1589"
  %CastToValueType1591 = bitcast i8* %"&pSB[currWI].offset1590" to float*
  store float %masked_load432, float* %CastToValueType1591, align 4
  br label %postload771

postload771:                                      ; preds = %preload770, %postload768
  %phi772 = phi float [ undef, %postload768 ], [ %masked_load432, %preload770 ]
  %"&(pSB[currWI].offset)1593" = add nuw i64 %CurrSBIndex..4, 500
  %"&pSB[currWI].offset1594" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1593"
  %CastToValueType1595 = bitcast i8* %"&pSB[currWI].offset1594" to float*
  store float %phi772, float* %CastToValueType1595, align 4
  %"&(pSB[currWI].offset)1286" = add nuw i64 %CurrSBIndex..4, 185
  %"&pSB[currWI].offset1287" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1286"
  %CastToValueType1288 = bitcast i8* %"&pSB[currWI].offset1287" to i1*
  %loadedValue1289 = load i1* %CastToValueType1288, align 1
  br i1 %loadedValue1289, label %preload773, label %postload774

preload773:                                       ; preds = %postload771
  %"&(pSB[currWI].offset)1430" = add nuw i64 %CurrSBIndex..4, 376
  %"&pSB[currWI].offset1431" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1430"
  %CastToValueType1432 = bitcast i8* %"&pSB[currWI].offset1431" to float addrspace(3)**
  %loadedValue1433 = load float addrspace(3)** %CastToValueType1432, align 8
  %masked_load433 = load float addrspace(3)* %loadedValue1433, align 4
  %"&(pSB[currWI].offset)1602" = add nuw i64 %CurrSBIndex..4, 504
  %"&pSB[currWI].offset1603" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1602"
  %CastToValueType1604 = bitcast i8* %"&pSB[currWI].offset1603" to float*
  store float %masked_load433, float* %CastToValueType1604, align 4
  br label %postload774

postload774:                                      ; preds = %preload773, %postload771
  %phi775 = phi float [ undef, %postload771 ], [ %masked_load433, %preload773 ]
  %"&(pSB[currWI].offset)1606" = add nuw i64 %CurrSBIndex..4, 508
  %"&pSB[currWI].offset1607" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1606"
  %CastToValueType1608 = bitcast i8* %"&pSB[currWI].offset1607" to float*
  store float %phi775, float* %CastToValueType1608, align 4
  %"&(pSB[currWI].offset)1295" = add nuw i64 %CurrSBIndex..4, 186
  %"&pSB[currWI].offset1296" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1295"
  %CastToValueType1297 = bitcast i8* %"&pSB[currWI].offset1296" to i1*
  %loadedValue1298 = load i1* %CastToValueType1297, align 1
  br i1 %loadedValue1298, label %preload776, label %postload777

preload776:                                       ; preds = %postload774
  %"&(pSB[currWI].offset)1439" = add nuw i64 %CurrSBIndex..4, 384
  %"&pSB[currWI].offset1440" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1439"
  %CastToValueType1441 = bitcast i8* %"&pSB[currWI].offset1440" to float addrspace(3)**
  %loadedValue1442 = load float addrspace(3)** %CastToValueType1441, align 8
  %masked_load434 = load float addrspace(3)* %loadedValue1442, align 4
  %"&(pSB[currWI].offset)1615" = add nuw i64 %CurrSBIndex..4, 512
  %"&pSB[currWI].offset1616" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1615"
  %CastToValueType1617 = bitcast i8* %"&pSB[currWI].offset1616" to float*
  store float %masked_load434, float* %CastToValueType1617, align 4
  br label %postload777

postload777:                                      ; preds = %preload776, %postload774
  %phi778 = phi float [ undef, %postload774 ], [ %masked_load434, %preload776 ]
  %"&(pSB[currWI].offset)1619" = add nuw i64 %CurrSBIndex..4, 516
  %"&pSB[currWI].offset1620" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1619"
  %CastToValueType1621 = bitcast i8* %"&pSB[currWI].offset1620" to float*
  store float %phi778, float* %CastToValueType1621, align 4
  %"&(pSB[currWI].offset)1304" = add nuw i64 %CurrSBIndex..4, 187
  %"&pSB[currWI].offset1305" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1304"
  %CastToValueType1306 = bitcast i8* %"&pSB[currWI].offset1305" to i1*
  %loadedValue1307 = load i1* %CastToValueType1306, align 1
  br i1 %loadedValue1307, label %preload779, label %postload780

preload779:                                       ; preds = %postload777
  %"&(pSB[currWI].offset)1448" = add nuw i64 %CurrSBIndex..4, 392
  %"&pSB[currWI].offset1449" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1448"
  %CastToValueType1450 = bitcast i8* %"&pSB[currWI].offset1449" to float addrspace(3)**
  %loadedValue1451 = load float addrspace(3)** %CastToValueType1450, align 8
  %masked_load435 = load float addrspace(3)* %loadedValue1451, align 4
  %"&(pSB[currWI].offset)1628" = add nuw i64 %CurrSBIndex..4, 520
  %"&pSB[currWI].offset1629" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1628"
  %CastToValueType1630 = bitcast i8* %"&pSB[currWI].offset1629" to float*
  store float %masked_load435, float* %CastToValueType1630, align 4
  br label %postload780

postload780:                                      ; preds = %preload779, %postload777
  %phi781 = phi float [ undef, %postload777 ], [ %masked_load435, %preload779 ]
  %"&(pSB[currWI].offset)1632" = add nuw i64 %CurrSBIndex..4, 524
  %"&pSB[currWI].offset1633" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1632"
  %CastToValueType1634 = bitcast i8* %"&pSB[currWI].offset1633" to float*
  store float %phi781, float* %CastToValueType1634, align 4
  %"&(pSB[currWI].offset)1313" = add nuw i64 %CurrSBIndex..4, 188
  %"&pSB[currWI].offset1314" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1313"
  %CastToValueType1315 = bitcast i8* %"&pSB[currWI].offset1314" to i1*
  %loadedValue1316 = load i1* %CastToValueType1315, align 1
  br i1 %loadedValue1316, label %preload782, label %postload783

preload782:                                       ; preds = %postload780
  %"&(pSB[currWI].offset)1457" = add nuw i64 %CurrSBIndex..4, 400
  %"&pSB[currWI].offset1458" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1457"
  %CastToValueType1459 = bitcast i8* %"&pSB[currWI].offset1458" to float addrspace(3)**
  %loadedValue1460 = load float addrspace(3)** %CastToValueType1459, align 8
  %masked_load436 = load float addrspace(3)* %loadedValue1460, align 4
  %"&(pSB[currWI].offset)1641" = add nuw i64 %CurrSBIndex..4, 528
  %"&pSB[currWI].offset1642" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1641"
  %CastToValueType1643 = bitcast i8* %"&pSB[currWI].offset1642" to float*
  store float %masked_load436, float* %CastToValueType1643, align 4
  br label %postload783

postload783:                                      ; preds = %preload782, %postload780
  %phi784 = phi float [ undef, %postload780 ], [ %masked_load436, %preload782 ]
  %"&(pSB[currWI].offset)1645" = add nuw i64 %CurrSBIndex..4, 532
  %"&pSB[currWI].offset1646" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1645"
  %CastToValueType1647 = bitcast i8* %"&pSB[currWI].offset1646" to float*
  store float %phi784, float* %CastToValueType1647, align 4
  %"&(pSB[currWI].offset)1322" = add nuw i64 %CurrSBIndex..4, 189
  %"&pSB[currWI].offset1323" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1322"
  %CastToValueType1324 = bitcast i8* %"&pSB[currWI].offset1323" to i1*
  %loadedValue1325 = load i1* %CastToValueType1324, align 1
  br i1 %loadedValue1325, label %preload785, label %postload786

preload785:                                       ; preds = %postload783
  %"&(pSB[currWI].offset)1466" = add nuw i64 %CurrSBIndex..4, 408
  %"&pSB[currWI].offset1467" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1466"
  %CastToValueType1468 = bitcast i8* %"&pSB[currWI].offset1467" to float addrspace(3)**
  %loadedValue1469 = load float addrspace(3)** %CastToValueType1468, align 8
  %masked_load437 = load float addrspace(3)* %loadedValue1469, align 4
  %"&(pSB[currWI].offset)1654" = add nuw i64 %CurrSBIndex..4, 536
  %"&pSB[currWI].offset1655" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1654"
  %CastToValueType1656 = bitcast i8* %"&pSB[currWI].offset1655" to float*
  store float %masked_load437, float* %CastToValueType1656, align 4
  br label %postload786

postload786:                                      ; preds = %preload785, %postload783
  %phi787 = phi float [ undef, %postload783 ], [ %masked_load437, %preload785 ]
  %"&(pSB[currWI].offset)1658" = add nuw i64 %CurrSBIndex..4, 540
  %"&pSB[currWI].offset1659" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1658"
  %CastToValueType1660 = bitcast i8* %"&pSB[currWI].offset1659" to float*
  store float %phi787, float* %CastToValueType1660, align 4
  %"&(pSB[currWI].offset)1331" = add nuw i64 %CurrSBIndex..4, 190
  %"&pSB[currWI].offset1332" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1331"
  %CastToValueType1333 = bitcast i8* %"&pSB[currWI].offset1332" to i1*
  %loadedValue1334 = load i1* %CastToValueType1333, align 1
  br i1 %loadedValue1334, label %preload788, label %postload789

preload788:                                       ; preds = %postload786
  %"&(pSB[currWI].offset)1475" = add nuw i64 %CurrSBIndex..4, 416
  %"&pSB[currWI].offset1476" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1475"
  %CastToValueType1477 = bitcast i8* %"&pSB[currWI].offset1476" to float addrspace(3)**
  %loadedValue1478 = load float addrspace(3)** %CastToValueType1477, align 8
  %masked_load438 = load float addrspace(3)* %loadedValue1478, align 4
  %"&(pSB[currWI].offset)1667" = add nuw i64 %CurrSBIndex..4, 544
  %"&pSB[currWI].offset1668" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1667"
  %CastToValueType1669 = bitcast i8* %"&pSB[currWI].offset1668" to float*
  store float %masked_load438, float* %CastToValueType1669, align 4
  br label %postload789

postload789:                                      ; preds = %preload788, %postload786
  %phi790 = phi float [ undef, %postload786 ], [ %masked_load438, %preload788 ]
  %"&(pSB[currWI].offset)1671" = add nuw i64 %CurrSBIndex..4, 548
  %"&pSB[currWI].offset1672" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1671"
  %CastToValueType1673 = bitcast i8* %"&pSB[currWI].offset1672" to float*
  store float %phi790, float* %CastToValueType1673, align 4
  %"&(pSB[currWI].offset)1340" = add nuw i64 %CurrSBIndex..4, 191
  %"&pSB[currWI].offset1341" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1340"
  %CastToValueType1342 = bitcast i8* %"&pSB[currWI].offset1341" to i1*
  %loadedValue1343 = load i1* %CastToValueType1342, align 1
  br i1 %loadedValue1343, label %preload791, label %postload792

preload791:                                       ; preds = %postload789
  %"&(pSB[currWI].offset)1484" = add nuw i64 %CurrSBIndex..4, 424
  %"&pSB[currWI].offset1485" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1484"
  %CastToValueType1486 = bitcast i8* %"&pSB[currWI].offset1485" to float addrspace(3)**
  %loadedValue1487 = load float addrspace(3)** %CastToValueType1486, align 8
  %masked_load439 = load float addrspace(3)* %loadedValue1487, align 4
  %"&(pSB[currWI].offset)1680" = add nuw i64 %CurrSBIndex..4, 552
  %"&pSB[currWI].offset1681" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1680"
  %CastToValueType1682 = bitcast i8* %"&pSB[currWI].offset1681" to float*
  store float %masked_load439, float* %CastToValueType1682, align 4
  br label %postload792

postload792:                                      ; preds = %preload791, %postload789
  %phi793 = phi float [ undef, %postload789 ], [ %masked_load439, %preload791 ]
  %"&(pSB[currWI].offset)1684" = add nuw i64 %CurrSBIndex..4, 556
  %"&pSB[currWI].offset1685" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1684"
  %CastToValueType1686 = bitcast i8* %"&pSB[currWI].offset1685" to float*
  store float %phi793, float* %CastToValueType1686, align 4
  %"&(pSB[currWI].offset)1349" = add nuw i64 %CurrSBIndex..4, 192
  %"&pSB[currWI].offset1350" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1349"
  %CastToValueType1351 = bitcast i8* %"&pSB[currWI].offset1350" to i1*
  %loadedValue1352 = load i1* %CastToValueType1351, align 1
  br i1 %loadedValue1352, label %preload794, label %postload795

preload794:                                       ; preds = %postload792
  %"&(pSB[currWI].offset)1493" = add nuw i64 %CurrSBIndex..4, 432
  %"&pSB[currWI].offset1494" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1493"
  %CastToValueType1495 = bitcast i8* %"&pSB[currWI].offset1494" to float addrspace(3)**
  %loadedValue1496 = load float addrspace(3)** %CastToValueType1495, align 8
  %masked_load440 = load float addrspace(3)* %loadedValue1496, align 4
  %"&(pSB[currWI].offset)1693" = add nuw i64 %CurrSBIndex..4, 560
  %"&pSB[currWI].offset1694" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1693"
  %CastToValueType1695 = bitcast i8* %"&pSB[currWI].offset1694" to float*
  store float %masked_load440, float* %CastToValueType1695, align 4
  br label %postload795

postload795:                                      ; preds = %preload794, %postload792
  %phi796 = phi float [ undef, %postload792 ], [ %masked_load440, %preload794 ]
  %"&(pSB[currWI].offset)1506" = add nuw i64 %CurrSBIndex..4, 444
  %"&pSB[currWI].offset1507" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1506"
  %CastToValueType1508 = bitcast i8* %"&pSB[currWI].offset1507" to float*
  %loadedValue1509 = load float* %CastToValueType1508, align 4
  %temp.vect287 = insertelement <16 x float> undef, float %loadedValue1509, i32 0
  %"&(pSB[currWI].offset)1519" = add nuw i64 %CurrSBIndex..4, 452
  %"&pSB[currWI].offset1520" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1519"
  %CastToValueType1521 = bitcast i8* %"&pSB[currWI].offset1520" to float*
  %loadedValue1522 = load float* %CastToValueType1521, align 4
  %temp.vect288 = insertelement <16 x float> %temp.vect287, float %loadedValue1522, i32 1
  %"&(pSB[currWI].offset)1532" = add nuw i64 %CurrSBIndex..4, 460
  %"&pSB[currWI].offset1533" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1532"
  %CastToValueType1534 = bitcast i8* %"&pSB[currWI].offset1533" to float*
  %loadedValue1535 = load float* %CastToValueType1534, align 4
  %temp.vect289 = insertelement <16 x float> %temp.vect288, float %loadedValue1535, i32 2
  %"&(pSB[currWI].offset)1545" = add nuw i64 %CurrSBIndex..4, 468
  %"&pSB[currWI].offset1546" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1545"
  %CastToValueType1547 = bitcast i8* %"&pSB[currWI].offset1546" to float*
  %loadedValue1548 = load float* %CastToValueType1547, align 4
  %temp.vect290 = insertelement <16 x float> %temp.vect289, float %loadedValue1548, i32 3
  %"&(pSB[currWI].offset)1558" = add nuw i64 %CurrSBIndex..4, 476
  %"&pSB[currWI].offset1559" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1558"
  %CastToValueType1560 = bitcast i8* %"&pSB[currWI].offset1559" to float*
  %loadedValue1561 = load float* %CastToValueType1560, align 4
  %temp.vect291 = insertelement <16 x float> %temp.vect290, float %loadedValue1561, i32 4
  %"&(pSB[currWI].offset)1571" = add nuw i64 %CurrSBIndex..4, 484
  %"&pSB[currWI].offset1572" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1571"
  %CastToValueType1573 = bitcast i8* %"&pSB[currWI].offset1572" to float*
  %loadedValue1574 = load float* %CastToValueType1573, align 4
  %temp.vect292 = insertelement <16 x float> %temp.vect291, float %loadedValue1574, i32 5
  %"&(pSB[currWI].offset)1584" = add nuw i64 %CurrSBIndex..4, 492
  %"&pSB[currWI].offset1585" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1584"
  %CastToValueType1586 = bitcast i8* %"&pSB[currWI].offset1585" to float*
  %loadedValue1587 = load float* %CastToValueType1586, align 4
  %temp.vect293 = insertelement <16 x float> %temp.vect292, float %loadedValue1587, i32 6
  %"&(pSB[currWI].offset)1597" = add nuw i64 %CurrSBIndex..4, 500
  %"&pSB[currWI].offset1598" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1597"
  %CastToValueType1599 = bitcast i8* %"&pSB[currWI].offset1598" to float*
  %loadedValue1600 = load float* %CastToValueType1599, align 4
  %temp.vect294 = insertelement <16 x float> %temp.vect293, float %loadedValue1600, i32 7
  %"&(pSB[currWI].offset)1610" = add nuw i64 %CurrSBIndex..4, 508
  %"&pSB[currWI].offset1611" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1610"
  %CastToValueType1612 = bitcast i8* %"&pSB[currWI].offset1611" to float*
  %loadedValue1613 = load float* %CastToValueType1612, align 4
  %temp.vect295 = insertelement <16 x float> %temp.vect294, float %loadedValue1613, i32 8
  %"&(pSB[currWI].offset)1623" = add nuw i64 %CurrSBIndex..4, 516
  %"&pSB[currWI].offset1624" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1623"
  %CastToValueType1625 = bitcast i8* %"&pSB[currWI].offset1624" to float*
  %loadedValue1626 = load float* %CastToValueType1625, align 4
  %temp.vect296 = insertelement <16 x float> %temp.vect295, float %loadedValue1626, i32 9
  %"&(pSB[currWI].offset)1636" = add nuw i64 %CurrSBIndex..4, 524
  %"&pSB[currWI].offset1637" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1636"
  %CastToValueType1638 = bitcast i8* %"&pSB[currWI].offset1637" to float*
  %loadedValue1639 = load float* %CastToValueType1638, align 4
  %temp.vect297 = insertelement <16 x float> %temp.vect296, float %loadedValue1639, i32 10
  %"&(pSB[currWI].offset)1649" = add nuw i64 %CurrSBIndex..4, 532
  %"&pSB[currWI].offset1650" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1649"
  %CastToValueType1651 = bitcast i8* %"&pSB[currWI].offset1650" to float*
  %loadedValue1652 = load float* %CastToValueType1651, align 4
  %temp.vect298 = insertelement <16 x float> %temp.vect297, float %loadedValue1652, i32 11
  %"&(pSB[currWI].offset)1662" = add nuw i64 %CurrSBIndex..4, 540
  %"&pSB[currWI].offset1663" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1662"
  %CastToValueType1664 = bitcast i8* %"&pSB[currWI].offset1663" to float*
  %loadedValue1665 = load float* %CastToValueType1664, align 4
  %temp.vect299 = insertelement <16 x float> %temp.vect298, float %loadedValue1665, i32 12
  %"&(pSB[currWI].offset)1675" = add nuw i64 %CurrSBIndex..4, 548
  %"&pSB[currWI].offset1676" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1675"
  %CastToValueType1677 = bitcast i8* %"&pSB[currWI].offset1676" to float*
  %loadedValue1678 = load float* %CastToValueType1677, align 4
  %temp.vect300 = insertelement <16 x float> %temp.vect299, float %loadedValue1678, i32 13
  %"&(pSB[currWI].offset)1688" = add nuw i64 %CurrSBIndex..4, 556
  %"&pSB[currWI].offset1689" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1688"
  %CastToValueType1690 = bitcast i8* %"&pSB[currWI].offset1689" to float*
  %loadedValue1691 = load float* %CastToValueType1690, align 4
  %temp.vect301 = insertelement <16 x float> %temp.vect300, float %loadedValue1691, i32 14
  %temp.vect302 = insertelement <16 x float> %temp.vect301, float %phi796, i32 15
  %"&(pSB[currWI].offset)1697" = add nuw i64 %CurrSBIndex..4, 576
  %"&pSB[currWI].offset1698" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1697"
  %CastToValueType1699 = bitcast i8* %"&pSB[currWI].offset1698" to <16 x float>*
  store <16 x float> %temp.vect302, <16 x float>* %CastToValueType1699, align 64
  %"&(pSB[currWI].offset)1008" = add nuw i64 %CurrSBIndex..4, 128
  %"&pSB[currWI].offset1009" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1008"
  %CastToValueType1010 = bitcast i8* %"&pSB[currWI].offset1009" to float addrspace(3)**
  %loadedValue1011 = load float addrspace(3)** %CastToValueType1010, align 8
  %ptrTypeCast286 = bitcast float addrspace(3)* %loadedValue1011 to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)1706" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1707" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1706"
  %CastToValueType1708 = bitcast i8* %"&pSB[currWI].offset1707" to <16 x float> addrspace(3)**
  store <16 x float> addrspace(3)* %ptrTypeCast286, <16 x float> addrspace(3)** %CastToValueType1708, align 8
  %"&(pSB[currWI].offset)1214" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1215" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1214"
  %CastToValueType1216 = bitcast i8* %"&pSB[currWI].offset1215" to <16 x i1>*
  %loadedValue1217 = load <16 x i1>* %CastToValueType1216, align 16
  %exmask442 = extractelement <16 x i1> %loadedValue1217, i32 0
  br i1 %exmask442, label %preload611, label %postload612

preload611:                                       ; preds = %postload795
  %"&(pSB[currWI].offset)1785" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1786" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1785"
  %CastToValueType1787 = bitcast i8* %"&pSB[currWI].offset1786" to <16 x float> addrspace(3)**
  %loadedValue1788 = load <16 x float> addrspace(3)** %CastToValueType1787, align 8
  %ptrTypeCast441 = getelementptr inbounds <16 x float> addrspace(3)* %loadedValue1788, i64 0, i64 0
  %vload443 = load float addrspace(3)* %ptrTypeCast441, align 4
  %"&(pSB[currWI].offset)1790" = add nuw i64 %CurrSBIndex..4, 648
  %"&pSB[currWI].offset1791" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1790"
  %CastToValueType1792 = bitcast i8* %"&pSB[currWI].offset1791" to float*
  store float %vload443, float* %CastToValueType1792, align 4
  br label %postload612

postload612:                                      ; preds = %preload611, %postload795
  %phi613 = phi float [ undef, %postload795 ], [ %vload443, %preload611 ]
  %vpack = insertelement <16 x float> undef, float %phi613, i32 0
  %"&(pSB[currWI].offset)1794" = add nuw i64 %CurrSBIndex..4, 704
  %"&pSB[currWI].offset1795" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1794"
  %CastToValueType1796 = bitcast i8* %"&pSB[currWI].offset1795" to <16 x float>*
  store <16 x float> %vpack, <16 x float>* %CastToValueType1796, align 64
  %"&(pSB[currWI].offset)1209" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1210" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1209"
  %CastToValueType1211 = bitcast i8* %"&pSB[currWI].offset1210" to <16 x i1>*
  %loadedValue1212 = load <16 x i1>* %CastToValueType1211, align 16
  %exmask445 = extractelement <16 x i1> %loadedValue1212, i32 1
  br i1 %exmask445, label %preload799, label %postload800

preload799:                                       ; preds = %postload612
  %"&(pSB[currWI].offset)1780" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1781" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1780"
  %CastToValueType1782 = bitcast i8* %"&pSB[currWI].offset1781" to <16 x float> addrspace(3)**
  %loadedValue1783 = load <16 x float> addrspace(3)** %CastToValueType1782, align 8
  %vload444 = getelementptr <16 x float> addrspace(3)* %loadedValue1783, i64 0, i64 1
  %vload446 = load float addrspace(3)* %vload444, align 4
  %"&(pSB[currWI].offset)1803" = add nuw i64 %CurrSBIndex..4, 768
  %"&pSB[currWI].offset1804" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1803"
  %CastToValueType1805 = bitcast i8* %"&pSB[currWI].offset1804" to float*
  store float %vload446, float* %CastToValueType1805, align 4
  br label %postload800

postload800:                                      ; preds = %preload799, %postload612
  %phi801 = phi float [ undef, %postload612 ], [ %vload446, %preload799 ]
  %"&(pSB[currWI].offset)1798" = add nuw i64 %CurrSBIndex..4, 704
  %"&pSB[currWI].offset1799" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1798"
  %CastToValueType1800 = bitcast i8* %"&pSB[currWI].offset1799" to <16 x float>*
  %loadedValue1801 = load <16 x float>* %CastToValueType1800, align 64
  %vpack447 = insertelement <16 x float> %loadedValue1801, float %phi801, i32 1
  %"&(pSB[currWI].offset)1807" = add nuw i64 %CurrSBIndex..4, 832
  %"&pSB[currWI].offset1808" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1807"
  %CastToValueType1809 = bitcast i8* %"&pSB[currWI].offset1808" to <16 x float>*
  store <16 x float> %vpack447, <16 x float>* %CastToValueType1809, align 64
  %"&(pSB[currWI].offset)1204" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1205" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1204"
  %CastToValueType1206 = bitcast i8* %"&pSB[currWI].offset1205" to <16 x i1>*
  %loadedValue1207 = load <16 x i1>* %CastToValueType1206, align 16
  %exmask449 = extractelement <16 x i1> %loadedValue1207, i32 2
  br i1 %exmask449, label %preload802, label %postload803

preload802:                                       ; preds = %postload800
  %"&(pSB[currWI].offset)1775" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1776" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1775"
  %CastToValueType1777 = bitcast i8* %"&pSB[currWI].offset1776" to <16 x float> addrspace(3)**
  %loadedValue1778 = load <16 x float> addrspace(3)** %CastToValueType1777, align 8
  %vload448 = getelementptr <16 x float> addrspace(3)* %loadedValue1778, i64 0, i64 2
  %vload450 = load float addrspace(3)* %vload448, align 4
  %"&(pSB[currWI].offset)1816" = add nuw i64 %CurrSBIndex..4, 896
  %"&pSB[currWI].offset1817" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1816"
  %CastToValueType1818 = bitcast i8* %"&pSB[currWI].offset1817" to float*
  store float %vload450, float* %CastToValueType1818, align 4
  br label %postload803

postload803:                                      ; preds = %preload802, %postload800
  %phi804 = phi float [ undef, %postload800 ], [ %vload450, %preload802 ]
  %"&(pSB[currWI].offset)1811" = add nuw i64 %CurrSBIndex..4, 832
  %"&pSB[currWI].offset1812" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1811"
  %CastToValueType1813 = bitcast i8* %"&pSB[currWI].offset1812" to <16 x float>*
  %loadedValue1814 = load <16 x float>* %CastToValueType1813, align 64
  %vpack451 = insertelement <16 x float> %loadedValue1814, float %phi804, i32 2
  %"&(pSB[currWI].offset)1820" = add nuw i64 %CurrSBIndex..4, 960
  %"&pSB[currWI].offset1821" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1820"
  %CastToValueType1822 = bitcast i8* %"&pSB[currWI].offset1821" to <16 x float>*
  store <16 x float> %vpack451, <16 x float>* %CastToValueType1822, align 64
  %"&(pSB[currWI].offset)1199" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1200" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1199"
  %CastToValueType1201 = bitcast i8* %"&pSB[currWI].offset1200" to <16 x i1>*
  %loadedValue1202 = load <16 x i1>* %CastToValueType1201, align 16
  %exmask453 = extractelement <16 x i1> %loadedValue1202, i32 3
  br i1 %exmask453, label %preload805, label %postload806

preload805:                                       ; preds = %postload803
  %"&(pSB[currWI].offset)1770" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1771" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1770"
  %CastToValueType1772 = bitcast i8* %"&pSB[currWI].offset1771" to <16 x float> addrspace(3)**
  %loadedValue1773 = load <16 x float> addrspace(3)** %CastToValueType1772, align 8
  %vload452 = getelementptr <16 x float> addrspace(3)* %loadedValue1773, i64 0, i64 3
  %vload454 = load float addrspace(3)* %vload452, align 4
  %"&(pSB[currWI].offset)1829" = add nuw i64 %CurrSBIndex..4, 1024
  %"&pSB[currWI].offset1830" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1829"
  %CastToValueType1831 = bitcast i8* %"&pSB[currWI].offset1830" to float*
  store float %vload454, float* %CastToValueType1831, align 4
  br label %postload806

postload806:                                      ; preds = %preload805, %postload803
  %phi807 = phi float [ undef, %postload803 ], [ %vload454, %preload805 ]
  %"&(pSB[currWI].offset)1824" = add nuw i64 %CurrSBIndex..4, 960
  %"&pSB[currWI].offset1825" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1824"
  %CastToValueType1826 = bitcast i8* %"&pSB[currWI].offset1825" to <16 x float>*
  %loadedValue1827 = load <16 x float>* %CastToValueType1826, align 64
  %vpack455 = insertelement <16 x float> %loadedValue1827, float %phi807, i32 3
  %"&(pSB[currWI].offset)1833" = add nuw i64 %CurrSBIndex..4, 1088
  %"&pSB[currWI].offset1834" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1833"
  %CastToValueType1835 = bitcast i8* %"&pSB[currWI].offset1834" to <16 x float>*
  store <16 x float> %vpack455, <16 x float>* %CastToValueType1835, align 64
  %"&(pSB[currWI].offset)1194" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1195" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1194"
  %CastToValueType1196 = bitcast i8* %"&pSB[currWI].offset1195" to <16 x i1>*
  %loadedValue1197 = load <16 x i1>* %CastToValueType1196, align 16
  %exmask457 = extractelement <16 x i1> %loadedValue1197, i32 4
  br i1 %exmask457, label %preload740, label %postload741

preload740:                                       ; preds = %postload806
  %"&(pSB[currWI].offset)1765" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1766" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1765"
  %CastToValueType1767 = bitcast i8* %"&pSB[currWI].offset1766" to <16 x float> addrspace(3)**
  %loadedValue1768 = load <16 x float> addrspace(3)** %CastToValueType1767, align 8
  %vload456 = getelementptr <16 x float> addrspace(3)* %loadedValue1768, i64 0, i64 4
  %vload458 = load float addrspace(3)* %vload456, align 4
  %"&(pSB[currWI].offset)1842" = add nuw i64 %CurrSBIndex..4, 1152
  %"&pSB[currWI].offset1843" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1842"
  %CastToValueType1844 = bitcast i8* %"&pSB[currWI].offset1843" to float*
  store float %vload458, float* %CastToValueType1844, align 4
  br label %postload741

postload741:                                      ; preds = %preload740, %postload806
  %phi742 = phi float [ undef, %postload806 ], [ %vload458, %preload740 ]
  %"&(pSB[currWI].offset)1837" = add nuw i64 %CurrSBIndex..4, 1088
  %"&pSB[currWI].offset1838" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1837"
  %CastToValueType1839 = bitcast i8* %"&pSB[currWI].offset1838" to <16 x float>*
  %loadedValue1840 = load <16 x float>* %CastToValueType1839, align 64
  %vpack459 = insertelement <16 x float> %loadedValue1840, float %phi742, i32 4
  %"&(pSB[currWI].offset)1846" = add nuw i64 %CurrSBIndex..4, 1216
  %"&pSB[currWI].offset1847" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1846"
  %CastToValueType1848 = bitcast i8* %"&pSB[currWI].offset1847" to <16 x float>*
  store <16 x float> %vpack459, <16 x float>* %CastToValueType1848, align 64
  %"&(pSB[currWI].offset)1189" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1190" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1189"
  %CastToValueType1191 = bitcast i8* %"&pSB[currWI].offset1190" to <16 x i1>*
  %loadedValue1192 = load <16 x i1>* %CastToValueType1191, align 16
  %exmask461 = extractelement <16 x i1> %loadedValue1192, i32 5
  br i1 %exmask461, label %preload743, label %postload744

preload743:                                       ; preds = %postload741
  %"&(pSB[currWI].offset)1760" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1761" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1760"
  %CastToValueType1762 = bitcast i8* %"&pSB[currWI].offset1761" to <16 x float> addrspace(3)**
  %loadedValue1763 = load <16 x float> addrspace(3)** %CastToValueType1762, align 8
  %vload460 = getelementptr <16 x float> addrspace(3)* %loadedValue1763, i64 0, i64 5
  %vload462 = load float addrspace(3)* %vload460, align 4
  %"&(pSB[currWI].offset)1855" = add nuw i64 %CurrSBIndex..4, 1280
  %"&pSB[currWI].offset1856" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1855"
  %CastToValueType1857 = bitcast i8* %"&pSB[currWI].offset1856" to float*
  store float %vload462, float* %CastToValueType1857, align 4
  br label %postload744

postload744:                                      ; preds = %preload743, %postload741
  %phi745 = phi float [ undef, %postload741 ], [ %vload462, %preload743 ]
  %"&(pSB[currWI].offset)1850" = add nuw i64 %CurrSBIndex..4, 1216
  %"&pSB[currWI].offset1851" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1850"
  %CastToValueType1852 = bitcast i8* %"&pSB[currWI].offset1851" to <16 x float>*
  %loadedValue1853 = load <16 x float>* %CastToValueType1852, align 64
  %vpack463 = insertelement <16 x float> %loadedValue1853, float %phi745, i32 5
  %"&(pSB[currWI].offset)1859" = add nuw i64 %CurrSBIndex..4, 1344
  %"&pSB[currWI].offset1860" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1859"
  %CastToValueType1861 = bitcast i8* %"&pSB[currWI].offset1860" to <16 x float>*
  store <16 x float> %vpack463, <16 x float>* %CastToValueType1861, align 64
  %"&(pSB[currWI].offset)1184" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1185" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1184"
  %CastToValueType1186 = bitcast i8* %"&pSB[currWI].offset1185" to <16 x i1>*
  %loadedValue1187 = load <16 x i1>* %CastToValueType1186, align 16
  %exmask465 = extractelement <16 x i1> %loadedValue1187, i32 6
  br i1 %exmask465, label %preload746, label %postload747

preload746:                                       ; preds = %postload744
  %"&(pSB[currWI].offset)1755" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1756" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1755"
  %CastToValueType1757 = bitcast i8* %"&pSB[currWI].offset1756" to <16 x float> addrspace(3)**
  %loadedValue1758 = load <16 x float> addrspace(3)** %CastToValueType1757, align 8
  %vload464 = getelementptr <16 x float> addrspace(3)* %loadedValue1758, i64 0, i64 6
  %vload466 = load float addrspace(3)* %vload464, align 4
  %"&(pSB[currWI].offset)1868" = add nuw i64 %CurrSBIndex..4, 1408
  %"&pSB[currWI].offset1869" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1868"
  %CastToValueType1870 = bitcast i8* %"&pSB[currWI].offset1869" to float*
  store float %vload466, float* %CastToValueType1870, align 4
  br label %postload747

postload747:                                      ; preds = %preload746, %postload744
  %phi748 = phi float [ undef, %postload744 ], [ %vload466, %preload746 ]
  %"&(pSB[currWI].offset)1863" = add nuw i64 %CurrSBIndex..4, 1344
  %"&pSB[currWI].offset1864" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1863"
  %CastToValueType1865 = bitcast i8* %"&pSB[currWI].offset1864" to <16 x float>*
  %loadedValue1866 = load <16 x float>* %CastToValueType1865, align 64
  %vpack467 = insertelement <16 x float> %loadedValue1866, float %phi748, i32 6
  %"&(pSB[currWI].offset)1872" = add nuw i64 %CurrSBIndex..4, 1472
  %"&pSB[currWI].offset1873" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1872"
  %CastToValueType1874 = bitcast i8* %"&pSB[currWI].offset1873" to <16 x float>*
  store <16 x float> %vpack467, <16 x float>* %CastToValueType1874, align 64
  %"&(pSB[currWI].offset)1179" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1180" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1179"
  %CastToValueType1181 = bitcast i8* %"&pSB[currWI].offset1180" to <16 x i1>*
  %loadedValue1182 = load <16 x i1>* %CastToValueType1181, align 16
  %exmask469 = extractelement <16 x i1> %loadedValue1182, i32 7
  br i1 %exmask469, label %preload713, label %postload714

preload713:                                       ; preds = %postload747
  %"&(pSB[currWI].offset)1750" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1751" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1750"
  %CastToValueType1752 = bitcast i8* %"&pSB[currWI].offset1751" to <16 x float> addrspace(3)**
  %loadedValue1753 = load <16 x float> addrspace(3)** %CastToValueType1752, align 8
  %vload468 = getelementptr <16 x float> addrspace(3)* %loadedValue1753, i64 0, i64 7
  %vload470 = load float addrspace(3)* %vload468, align 4
  %"&(pSB[currWI].offset)1881" = add nuw i64 %CurrSBIndex..4, 1536
  %"&pSB[currWI].offset1882" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1881"
  %CastToValueType1883 = bitcast i8* %"&pSB[currWI].offset1882" to float*
  store float %vload470, float* %CastToValueType1883, align 4
  br label %postload714

postload714:                                      ; preds = %preload713, %postload747
  %phi715 = phi float [ undef, %postload747 ], [ %vload470, %preload713 ]
  %"&(pSB[currWI].offset)1876" = add nuw i64 %CurrSBIndex..4, 1472
  %"&pSB[currWI].offset1877" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1876"
  %CastToValueType1878 = bitcast i8* %"&pSB[currWI].offset1877" to <16 x float>*
  %loadedValue1879 = load <16 x float>* %CastToValueType1878, align 64
  %vpack471 = insertelement <16 x float> %loadedValue1879, float %phi715, i32 7
  %"&(pSB[currWI].offset)1885" = add nuw i64 %CurrSBIndex..4, 1600
  %"&pSB[currWI].offset1886" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1885"
  %CastToValueType1887 = bitcast i8* %"&pSB[currWI].offset1886" to <16 x float>*
  store <16 x float> %vpack471, <16 x float>* %CastToValueType1887, align 64
  %"&(pSB[currWI].offset)1174" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1175" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1174"
  %CastToValueType1176 = bitcast i8* %"&pSB[currWI].offset1175" to <16 x i1>*
  %loadedValue1177 = load <16 x i1>* %CastToValueType1176, align 16
  %exmask473 = extractelement <16 x i1> %loadedValue1177, i32 8
  br i1 %exmask473, label %preload716, label %postload717

preload716:                                       ; preds = %postload714
  %"&(pSB[currWI].offset)1745" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1746" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1745"
  %CastToValueType1747 = bitcast i8* %"&pSB[currWI].offset1746" to <16 x float> addrspace(3)**
  %loadedValue1748 = load <16 x float> addrspace(3)** %CastToValueType1747, align 8
  %vload472 = getelementptr <16 x float> addrspace(3)* %loadedValue1748, i64 0, i64 8
  %vload474 = load float addrspace(3)* %vload472, align 4
  %"&(pSB[currWI].offset)1894" = add nuw i64 %CurrSBIndex..4, 1664
  %"&pSB[currWI].offset1895" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1894"
  %CastToValueType1896 = bitcast i8* %"&pSB[currWI].offset1895" to float*
  store float %vload474, float* %CastToValueType1896, align 4
  br label %postload717

postload717:                                      ; preds = %preload716, %postload714
  %phi718 = phi float [ undef, %postload714 ], [ %vload474, %preload716 ]
  %"&(pSB[currWI].offset)1889" = add nuw i64 %CurrSBIndex..4, 1600
  %"&pSB[currWI].offset1890" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1889"
  %CastToValueType1891 = bitcast i8* %"&pSB[currWI].offset1890" to <16 x float>*
  %loadedValue1892 = load <16 x float>* %CastToValueType1891, align 64
  %vpack475 = insertelement <16 x float> %loadedValue1892, float %phi718, i32 8
  %"&(pSB[currWI].offset)1898" = add nuw i64 %CurrSBIndex..4, 1728
  %"&pSB[currWI].offset1899" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1898"
  %CastToValueType1900 = bitcast i8* %"&pSB[currWI].offset1899" to <16 x float>*
  store <16 x float> %vpack475, <16 x float>* %CastToValueType1900, align 64
  %"&(pSB[currWI].offset)1169" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1170" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1169"
  %CastToValueType1171 = bitcast i8* %"&pSB[currWI].offset1170" to <16 x i1>*
  %loadedValue1172 = load <16 x i1>* %CastToValueType1171, align 16
  %exmask477 = extractelement <16 x i1> %loadedValue1172, i32 9
  br i1 %exmask477, label %preload719, label %postload720

preload719:                                       ; preds = %postload717
  %"&(pSB[currWI].offset)1740" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1741" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1740"
  %CastToValueType1742 = bitcast i8* %"&pSB[currWI].offset1741" to <16 x float> addrspace(3)**
  %loadedValue1743 = load <16 x float> addrspace(3)** %CastToValueType1742, align 8
  %vload476 = getelementptr <16 x float> addrspace(3)* %loadedValue1743, i64 0, i64 9
  %vload478 = load float addrspace(3)* %vload476, align 4
  %"&(pSB[currWI].offset)1907" = add nuw i64 %CurrSBIndex..4, 1792
  %"&pSB[currWI].offset1908" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1907"
  %CastToValueType1909 = bitcast i8* %"&pSB[currWI].offset1908" to float*
  store float %vload478, float* %CastToValueType1909, align 4
  br label %postload720

postload720:                                      ; preds = %preload719, %postload717
  %phi721 = phi float [ undef, %postload717 ], [ %vload478, %preload719 ]
  %"&(pSB[currWI].offset)1902" = add nuw i64 %CurrSBIndex..4, 1728
  %"&pSB[currWI].offset1903" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1902"
  %CastToValueType1904 = bitcast i8* %"&pSB[currWI].offset1903" to <16 x float>*
  %loadedValue1905 = load <16 x float>* %CastToValueType1904, align 64
  %vpack479 = insertelement <16 x float> %loadedValue1905, float %phi721, i32 9
  %"&(pSB[currWI].offset)1911" = add nuw i64 %CurrSBIndex..4, 1856
  %"&pSB[currWI].offset1912" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1911"
  %CastToValueType1913 = bitcast i8* %"&pSB[currWI].offset1912" to <16 x float>*
  store <16 x float> %vpack479, <16 x float>* %CastToValueType1913, align 64
  %"&(pSB[currWI].offset)1164" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1165" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1164"
  %CastToValueType1166 = bitcast i8* %"&pSB[currWI].offset1165" to <16 x i1>*
  %loadedValue1167 = load <16 x i1>* %CastToValueType1166, align 16
  %exmask481 = extractelement <16 x i1> %loadedValue1167, i32 10
  br i1 %exmask481, label %preload722, label %postload723

preload722:                                       ; preds = %postload720
  %"&(pSB[currWI].offset)1735" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1736" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1735"
  %CastToValueType1737 = bitcast i8* %"&pSB[currWI].offset1736" to <16 x float> addrspace(3)**
  %loadedValue1738 = load <16 x float> addrspace(3)** %CastToValueType1737, align 8
  %vload480 = getelementptr <16 x float> addrspace(3)* %loadedValue1738, i64 0, i64 10
  %vload482 = load float addrspace(3)* %vload480, align 4
  %"&(pSB[currWI].offset)1920" = add nuw i64 %CurrSBIndex..4, 1920
  %"&pSB[currWI].offset1921" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1920"
  %CastToValueType1922 = bitcast i8* %"&pSB[currWI].offset1921" to float*
  store float %vload482, float* %CastToValueType1922, align 4
  br label %postload723

postload723:                                      ; preds = %preload722, %postload720
  %phi724 = phi float [ undef, %postload720 ], [ %vload482, %preload722 ]
  %"&(pSB[currWI].offset)1915" = add nuw i64 %CurrSBIndex..4, 1856
  %"&pSB[currWI].offset1916" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1915"
  %CastToValueType1917 = bitcast i8* %"&pSB[currWI].offset1916" to <16 x float>*
  %loadedValue1918 = load <16 x float>* %CastToValueType1917, align 64
  %vpack483 = insertelement <16 x float> %loadedValue1918, float %phi724, i32 10
  %"&(pSB[currWI].offset)1924" = add nuw i64 %CurrSBIndex..4, 1984
  %"&pSB[currWI].offset1925" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1924"
  %CastToValueType1926 = bitcast i8* %"&pSB[currWI].offset1925" to <16 x float>*
  store <16 x float> %vpack483, <16 x float>* %CastToValueType1926, align 64
  %"&(pSB[currWI].offset)1159" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1160" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1159"
  %CastToValueType1161 = bitcast i8* %"&pSB[currWI].offset1160" to <16 x i1>*
  %loadedValue1162 = load <16 x i1>* %CastToValueType1161, align 16
  %exmask485 = extractelement <16 x i1> %loadedValue1162, i32 11
  br i1 %exmask485, label %preload725, label %postload726

preload725:                                       ; preds = %postload723
  %"&(pSB[currWI].offset)1730" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1731" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1730"
  %CastToValueType1732 = bitcast i8* %"&pSB[currWI].offset1731" to <16 x float> addrspace(3)**
  %loadedValue1733 = load <16 x float> addrspace(3)** %CastToValueType1732, align 8
  %vload484 = getelementptr <16 x float> addrspace(3)* %loadedValue1733, i64 0, i64 11
  %vload486 = load float addrspace(3)* %vload484, align 4
  %"&(pSB[currWI].offset)1933" = add nuw i64 %CurrSBIndex..4, 2048
  %"&pSB[currWI].offset1934" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1933"
  %CastToValueType1935 = bitcast i8* %"&pSB[currWI].offset1934" to float*
  store float %vload486, float* %CastToValueType1935, align 4
  br label %postload726

postload726:                                      ; preds = %preload725, %postload723
  %phi727 = phi float [ undef, %postload723 ], [ %vload486, %preload725 ]
  %"&(pSB[currWI].offset)1928" = add nuw i64 %CurrSBIndex..4, 1984
  %"&pSB[currWI].offset1929" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1928"
  %CastToValueType1930 = bitcast i8* %"&pSB[currWI].offset1929" to <16 x float>*
  %loadedValue1931 = load <16 x float>* %CastToValueType1930, align 64
  %vpack487 = insertelement <16 x float> %loadedValue1931, float %phi727, i32 11
  %"&(pSB[currWI].offset)1937" = add nuw i64 %CurrSBIndex..4, 2112
  %"&pSB[currWI].offset1938" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1937"
  %CastToValueType1939 = bitcast i8* %"&pSB[currWI].offset1938" to <16 x float>*
  store <16 x float> %vpack487, <16 x float>* %CastToValueType1939, align 64
  %"&(pSB[currWI].offset)1154" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1155" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1154"
  %CastToValueType1156 = bitcast i8* %"&pSB[currWI].offset1155" to <16 x i1>*
  %loadedValue1157 = load <16 x i1>* %CastToValueType1156, align 16
  %exmask489 = extractelement <16 x i1> %loadedValue1157, i32 12
  br i1 %exmask489, label %preload728, label %postload729

preload728:                                       ; preds = %postload726
  %"&(pSB[currWI].offset)1725" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1726" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1725"
  %CastToValueType1727 = bitcast i8* %"&pSB[currWI].offset1726" to <16 x float> addrspace(3)**
  %loadedValue1728 = load <16 x float> addrspace(3)** %CastToValueType1727, align 8
  %vload488 = getelementptr <16 x float> addrspace(3)* %loadedValue1728, i64 0, i64 12
  %vload490 = load float addrspace(3)* %vload488, align 4
  %"&(pSB[currWI].offset)1946" = add nuw i64 %CurrSBIndex..4, 2176
  %"&pSB[currWI].offset1947" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1946"
  %CastToValueType1948 = bitcast i8* %"&pSB[currWI].offset1947" to float*
  store float %vload490, float* %CastToValueType1948, align 4
  br label %postload729

postload729:                                      ; preds = %preload728, %postload726
  %phi730 = phi float [ undef, %postload726 ], [ %vload490, %preload728 ]
  %"&(pSB[currWI].offset)1941" = add nuw i64 %CurrSBIndex..4, 2112
  %"&pSB[currWI].offset1942" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1941"
  %CastToValueType1943 = bitcast i8* %"&pSB[currWI].offset1942" to <16 x float>*
  %loadedValue1944 = load <16 x float>* %CastToValueType1943, align 64
  %vpack491 = insertelement <16 x float> %loadedValue1944, float %phi730, i32 12
  %"&(pSB[currWI].offset)1950" = add nuw i64 %CurrSBIndex..4, 2240
  %"&pSB[currWI].offset1951" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1950"
  %CastToValueType1952 = bitcast i8* %"&pSB[currWI].offset1951" to <16 x float>*
  store <16 x float> %vpack491, <16 x float>* %CastToValueType1952, align 64
  %"&(pSB[currWI].offset)1149" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1150" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1149"
  %CastToValueType1151 = bitcast i8* %"&pSB[currWI].offset1150" to <16 x i1>*
  %loadedValue1152 = load <16 x i1>* %CastToValueType1151, align 16
  %exmask493 = extractelement <16 x i1> %loadedValue1152, i32 13
  br i1 %exmask493, label %preload731, label %postload732

preload731:                                       ; preds = %postload729
  %"&(pSB[currWI].offset)1720" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1721" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1720"
  %CastToValueType1722 = bitcast i8* %"&pSB[currWI].offset1721" to <16 x float> addrspace(3)**
  %loadedValue1723 = load <16 x float> addrspace(3)** %CastToValueType1722, align 8
  %vload492 = getelementptr <16 x float> addrspace(3)* %loadedValue1723, i64 0, i64 13
  %vload494 = load float addrspace(3)* %vload492, align 4
  %"&(pSB[currWI].offset)1959" = add nuw i64 %CurrSBIndex..4, 2304
  %"&pSB[currWI].offset1960" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1959"
  %CastToValueType1961 = bitcast i8* %"&pSB[currWI].offset1960" to float*
  store float %vload494, float* %CastToValueType1961, align 4
  br label %postload732

postload732:                                      ; preds = %preload731, %postload729
  %phi733 = phi float [ undef, %postload729 ], [ %vload494, %preload731 ]
  %"&(pSB[currWI].offset)1954" = add nuw i64 %CurrSBIndex..4, 2240
  %"&pSB[currWI].offset1955" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1954"
  %CastToValueType1956 = bitcast i8* %"&pSB[currWI].offset1955" to <16 x float>*
  %loadedValue1957 = load <16 x float>* %CastToValueType1956, align 64
  %vpack495 = insertelement <16 x float> %loadedValue1957, float %phi733, i32 13
  %"&(pSB[currWI].offset)1963" = add nuw i64 %CurrSBIndex..4, 2368
  %"&pSB[currWI].offset1964" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1963"
  %CastToValueType1965 = bitcast i8* %"&pSB[currWI].offset1964" to <16 x float>*
  store <16 x float> %vpack495, <16 x float>* %CastToValueType1965, align 64
  %"&(pSB[currWI].offset)1144" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1145" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1144"
  %CastToValueType1146 = bitcast i8* %"&pSB[currWI].offset1145" to <16 x i1>*
  %loadedValue1147 = load <16 x i1>* %CastToValueType1146, align 16
  %exmask497 = extractelement <16 x i1> %loadedValue1147, i32 14
  br i1 %exmask497, label %preload734, label %postload735

preload734:                                       ; preds = %postload732
  %"&(pSB[currWI].offset)1715" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1716" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1715"
  %CastToValueType1717 = bitcast i8* %"&pSB[currWI].offset1716" to <16 x float> addrspace(3)**
  %loadedValue1718 = load <16 x float> addrspace(3)** %CastToValueType1717, align 8
  %vload496 = getelementptr <16 x float> addrspace(3)* %loadedValue1718, i64 0, i64 14
  %vload498 = load float addrspace(3)* %vload496, align 4
  %"&(pSB[currWI].offset)1972" = add nuw i64 %CurrSBIndex..4, 2432
  %"&pSB[currWI].offset1973" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1972"
  %CastToValueType1974 = bitcast i8* %"&pSB[currWI].offset1973" to float*
  store float %vload498, float* %CastToValueType1974, align 4
  br label %postload735

postload735:                                      ; preds = %preload734, %postload732
  %phi736 = phi float [ undef, %postload732 ], [ %vload498, %preload734 ]
  %"&(pSB[currWI].offset)1967" = add nuw i64 %CurrSBIndex..4, 2368
  %"&pSB[currWI].offset1968" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1967"
  %CastToValueType1969 = bitcast i8* %"&pSB[currWI].offset1968" to <16 x float>*
  %loadedValue1970 = load <16 x float>* %CastToValueType1969, align 64
  %vpack499 = insertelement <16 x float> %loadedValue1970, float %phi736, i32 14
  %"&(pSB[currWI].offset)1976" = add nuw i64 %CurrSBIndex..4, 2496
  %"&pSB[currWI].offset1977" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1976"
  %CastToValueType1978 = bitcast i8* %"&pSB[currWI].offset1977" to <16 x float>*
  store <16 x float> %vpack499, <16 x float>* %CastToValueType1978, align 64
  %"&(pSB[currWI].offset)1139" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1140" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1139"
  %CastToValueType1141 = bitcast i8* %"&pSB[currWI].offset1140" to <16 x i1>*
  %loadedValue1142 = load <16 x i1>* %CastToValueType1141, align 16
  %exmask501 = extractelement <16 x i1> %loadedValue1142, i32 15
  br i1 %exmask501, label %preload737, label %postload738

preload737:                                       ; preds = %postload735
  %"&(pSB[currWI].offset)1710" = add nuw i64 %CurrSBIndex..4, 640
  %"&pSB[currWI].offset1711" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1710"
  %CastToValueType1712 = bitcast i8* %"&pSB[currWI].offset1711" to <16 x float> addrspace(3)**
  %loadedValue1713 = load <16 x float> addrspace(3)** %CastToValueType1712, align 8
  %vload500 = getelementptr <16 x float> addrspace(3)* %loadedValue1713, i64 0, i64 15
  %vload502 = load float addrspace(3)* %vload500, align 4
  %"&(pSB[currWI].offset)1985" = add nuw i64 %CurrSBIndex..4, 2560
  %"&pSB[currWI].offset1986" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1985"
  %CastToValueType1987 = bitcast i8* %"&pSB[currWI].offset1986" to float*
  store float %vload502, float* %CastToValueType1987, align 4
  br label %postload738

postload738:                                      ; preds = %preload737, %postload735
  %phi739 = phi float [ undef, %postload735 ], [ %vload502, %preload737 ]
  %"&(pSB[currWI].offset)1980" = add nuw i64 %CurrSBIndex..4, 2496
  %"&pSB[currWI].offset1981" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1980"
  %CastToValueType1982 = bitcast i8* %"&pSB[currWI].offset1981" to <16 x float>*
  %loadedValue1983 = load <16 x float>* %CastToValueType1982, align 64
  %vpack503 = insertelement <16 x float> %loadedValue1983, float %phi739, i32 15
  %"&(pSB[currWI].offset)1701" = add nuw i64 %CurrSBIndex..4, 576
  %"&pSB[currWI].offset1702" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1701"
  %CastToValueType1703 = bitcast i8* %"&pSB[currWI].offset1702" to <16 x float>*
  %loadedValue1704 = load <16 x float>* %CastToValueType1703, align 64
  %183 = fadd <16 x float> %vpack503, %loadedValue1704
  %"&(pSB[currWI].offset)1989" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset1990" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1989"
  %CastToValueType1991 = bitcast i8* %"&pSB[currWI].offset1990" to <16 x float>*
  store <16 x float> %183, <16 x float>* %CastToValueType1991, align 64
  %"&(pSB[currWI].offset)1003" = add nuw i64 %CurrSBIndex..4, 128
  %"&pSB[currWI].offset1004" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1003"
  %CastToValueType1005 = bitcast i8* %"&pSB[currWI].offset1004" to float addrspace(3)**
  %loadedValue1006 = load float addrspace(3)** %CastToValueType1005, align 8
  %ptrTypeCast303 = bitcast float addrspace(3)* %loadedValue1006 to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)2073" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2074" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2073"
  %CastToValueType2075 = bitcast i8* %"&pSB[currWI].offset2074" to <16 x float> addrspace(3)**
  store <16 x float> addrspace(3)* %ptrTypeCast303, <16 x float> addrspace(3)** %CastToValueType2075, align 8
  %"&(pSB[currWI].offset)1134" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1135" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1134"
  %CastToValueType1136 = bitcast i8* %"&pSB[currWI].offset1135" to <16 x i1>*
  %loadedValue1137 = load <16 x i1>* %CastToValueType1136, align 16
  %exmask506 = extractelement <16 x i1> %loadedValue1137, i32 0
  br i1 %exmask506, label %preload936, label %postload937

preload936:                                       ; preds = %postload738
  %"&(pSB[currWI].offset)2152" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2153" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2152"
  %CastToValueType2154 = bitcast i8* %"&pSB[currWI].offset2153" to <16 x float> addrspace(3)**
  %loadedValue2155 = load <16 x float> addrspace(3)** %CastToValueType2154, align 8
  %ptrTypeCast504 = getelementptr inbounds <16 x float> addrspace(3)* %loadedValue2155, i64 0, i64 0
  %"&(pSB[currWI].offset)2068" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2069" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2068"
  %CastToValueType2070 = bitcast i8* %"&pSB[currWI].offset2069" to <16 x float>*
  %loadedValue2071 = load <16 x float>* %CastToValueType2070, align 64
  %exData507 = extractelement <16 x float> %loadedValue2071, i32 0
  store float %exData507, float addrspace(3)* %ptrTypeCast504, align 4
  br label %postload937

postload937:                                      ; preds = %preload936, %postload738
  %"&(pSB[currWI].offset)1129" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1130" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1129"
  %CastToValueType1131 = bitcast i8* %"&pSB[currWI].offset1130" to <16 x i1>*
  %loadedValue1132 = load <16 x i1>* %CastToValueType1131, align 16
  %exmask509 = extractelement <16 x i1> %loadedValue1132, i32 1
  br i1 %exmask509, label %preload939, label %postload940

preload939:                                       ; preds = %postload937
  %"&(pSB[currWI].offset)2147" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2148" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2147"
  %CastToValueType2149 = bitcast i8* %"&pSB[currWI].offset2148" to <16 x float> addrspace(3)**
  %loadedValue2150 = load <16 x float> addrspace(3)** %CastToValueType2149, align 8
  %vstore508 = getelementptr <16 x float> addrspace(3)* %loadedValue2150, i64 0, i64 1
  %"&(pSB[currWI].offset)2063" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2064" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2063"
  %CastToValueType2065 = bitcast i8* %"&pSB[currWI].offset2064" to <16 x float>*
  %loadedValue2066 = load <16 x float>* %CastToValueType2065, align 64
  %exData510 = extractelement <16 x float> %loadedValue2066, i32 1
  store float %exData510, float addrspace(3)* %vstore508, align 4
  br label %postload940

postload940:                                      ; preds = %preload939, %postload937
  %"&(pSB[currWI].offset)1124" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1125" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1124"
  %CastToValueType1126 = bitcast i8* %"&pSB[currWI].offset1125" to <16 x i1>*
  %loadedValue1127 = load <16 x i1>* %CastToValueType1126, align 16
  %exmask512 = extractelement <16 x i1> %loadedValue1127, i32 2
  br i1 %exmask512, label %preload942, label %postload943

preload942:                                       ; preds = %postload940
  %"&(pSB[currWI].offset)2142" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2143" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2142"
  %CastToValueType2144 = bitcast i8* %"&pSB[currWI].offset2143" to <16 x float> addrspace(3)**
  %loadedValue2145 = load <16 x float> addrspace(3)** %CastToValueType2144, align 8
  %vstore511 = getelementptr <16 x float> addrspace(3)* %loadedValue2145, i64 0, i64 2
  %"&(pSB[currWI].offset)2058" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2059" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2058"
  %CastToValueType2060 = bitcast i8* %"&pSB[currWI].offset2059" to <16 x float>*
  %loadedValue2061 = load <16 x float>* %CastToValueType2060, align 64
  %exData513 = extractelement <16 x float> %loadedValue2061, i32 2
  store float %exData513, float addrspace(3)* %vstore511, align 4
  br label %postload943

postload943:                                      ; preds = %preload942, %postload940
  %"&(pSB[currWI].offset)1119" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1120" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1119"
  %CastToValueType1121 = bitcast i8* %"&pSB[currWI].offset1120" to <16 x i1>*
  %loadedValue1122 = load <16 x i1>* %CastToValueType1121, align 16
  %exmask515 = extractelement <16 x i1> %loadedValue1122, i32 3
  br i1 %exmask515, label %preload945, label %postload946

preload945:                                       ; preds = %postload943
  %"&(pSB[currWI].offset)2137" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2138" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2137"
  %CastToValueType2139 = bitcast i8* %"&pSB[currWI].offset2138" to <16 x float> addrspace(3)**
  %loadedValue2140 = load <16 x float> addrspace(3)** %CastToValueType2139, align 8
  %vstore514 = getelementptr <16 x float> addrspace(3)* %loadedValue2140, i64 0, i64 3
  %"&(pSB[currWI].offset)2053" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2054" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2053"
  %CastToValueType2055 = bitcast i8* %"&pSB[currWI].offset2054" to <16 x float>*
  %loadedValue2056 = load <16 x float>* %CastToValueType2055, align 64
  %exData516 = extractelement <16 x float> %loadedValue2056, i32 3
  store float %exData516, float addrspace(3)* %vstore514, align 4
  br label %postload946

postload946:                                      ; preds = %preload945, %postload943
  %"&(pSB[currWI].offset)1114" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1115" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1114"
  %CastToValueType1116 = bitcast i8* %"&pSB[currWI].offset1115" to <16 x i1>*
  %loadedValue1117 = load <16 x i1>* %CastToValueType1116, align 16
  %exmask518 = extractelement <16 x i1> %loadedValue1117, i32 4
  br i1 %exmask518, label %preload948, label %postload949

preload948:                                       ; preds = %postload946
  %"&(pSB[currWI].offset)2132" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2133" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2132"
  %CastToValueType2134 = bitcast i8* %"&pSB[currWI].offset2133" to <16 x float> addrspace(3)**
  %loadedValue2135 = load <16 x float> addrspace(3)** %CastToValueType2134, align 8
  %vstore517 = getelementptr <16 x float> addrspace(3)* %loadedValue2135, i64 0, i64 4
  %"&(pSB[currWI].offset)2048" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2049" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2048"
  %CastToValueType2050 = bitcast i8* %"&pSB[currWI].offset2049" to <16 x float>*
  %loadedValue2051 = load <16 x float>* %CastToValueType2050, align 64
  %exData519 = extractelement <16 x float> %loadedValue2051, i32 4
  store float %exData519, float addrspace(3)* %vstore517, align 4
  br label %postload949

postload949:                                      ; preds = %preload948, %postload946
  %"&(pSB[currWI].offset)1109" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1110" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1109"
  %CastToValueType1111 = bitcast i8* %"&pSB[currWI].offset1110" to <16 x i1>*
  %loadedValue1112 = load <16 x i1>* %CastToValueType1111, align 16
  %exmask521 = extractelement <16 x i1> %loadedValue1112, i32 5
  br i1 %exmask521, label %preload951, label %postload952

preload951:                                       ; preds = %postload949
  %"&(pSB[currWI].offset)2127" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2128" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2127"
  %CastToValueType2129 = bitcast i8* %"&pSB[currWI].offset2128" to <16 x float> addrspace(3)**
  %loadedValue2130 = load <16 x float> addrspace(3)** %CastToValueType2129, align 8
  %vstore520 = getelementptr <16 x float> addrspace(3)* %loadedValue2130, i64 0, i64 5
  %"&(pSB[currWI].offset)2043" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2044" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2043"
  %CastToValueType2045 = bitcast i8* %"&pSB[currWI].offset2044" to <16 x float>*
  %loadedValue2046 = load <16 x float>* %CastToValueType2045, align 64
  %exData522 = extractelement <16 x float> %loadedValue2046, i32 5
  store float %exData522, float addrspace(3)* %vstore520, align 4
  br label %postload952

postload952:                                      ; preds = %preload951, %postload949
  %"&(pSB[currWI].offset)1104" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1105" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1104"
  %CastToValueType1106 = bitcast i8* %"&pSB[currWI].offset1105" to <16 x i1>*
  %loadedValue1107 = load <16 x i1>* %CastToValueType1106, align 16
  %exmask524 = extractelement <16 x i1> %loadedValue1107, i32 6
  br i1 %exmask524, label %preload954, label %postload955

preload954:                                       ; preds = %postload952
  %"&(pSB[currWI].offset)2122" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2123" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2122"
  %CastToValueType2124 = bitcast i8* %"&pSB[currWI].offset2123" to <16 x float> addrspace(3)**
  %loadedValue2125 = load <16 x float> addrspace(3)** %CastToValueType2124, align 8
  %vstore523 = getelementptr <16 x float> addrspace(3)* %loadedValue2125, i64 0, i64 6
  %"&(pSB[currWI].offset)2038" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2039" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2038"
  %CastToValueType2040 = bitcast i8* %"&pSB[currWI].offset2039" to <16 x float>*
  %loadedValue2041 = load <16 x float>* %CastToValueType2040, align 64
  %exData525 = extractelement <16 x float> %loadedValue2041, i32 6
  store float %exData525, float addrspace(3)* %vstore523, align 4
  br label %postload955

postload955:                                      ; preds = %preload954, %postload952
  %"&(pSB[currWI].offset)1099" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1100" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1099"
  %CastToValueType1101 = bitcast i8* %"&pSB[currWI].offset1100" to <16 x i1>*
  %loadedValue1102 = load <16 x i1>* %CastToValueType1101, align 16
  %exmask527 = extractelement <16 x i1> %loadedValue1102, i32 7
  br i1 %exmask527, label %preload957, label %postload958

preload957:                                       ; preds = %postload955
  %"&(pSB[currWI].offset)2117" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2118" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2117"
  %CastToValueType2119 = bitcast i8* %"&pSB[currWI].offset2118" to <16 x float> addrspace(3)**
  %loadedValue2120 = load <16 x float> addrspace(3)** %CastToValueType2119, align 8
  %vstore526 = getelementptr <16 x float> addrspace(3)* %loadedValue2120, i64 0, i64 7
  %"&(pSB[currWI].offset)2033" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2034" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2033"
  %CastToValueType2035 = bitcast i8* %"&pSB[currWI].offset2034" to <16 x float>*
  %loadedValue2036 = load <16 x float>* %CastToValueType2035, align 64
  %exData528 = extractelement <16 x float> %loadedValue2036, i32 7
  store float %exData528, float addrspace(3)* %vstore526, align 4
  br label %postload958

postload958:                                      ; preds = %preload957, %postload955
  %"&(pSB[currWI].offset)1094" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1095" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1094"
  %CastToValueType1096 = bitcast i8* %"&pSB[currWI].offset1095" to <16 x i1>*
  %loadedValue1097 = load <16 x i1>* %CastToValueType1096, align 16
  %exmask530 = extractelement <16 x i1> %loadedValue1097, i32 8
  br i1 %exmask530, label %preload960, label %postload961

preload960:                                       ; preds = %postload958
  %"&(pSB[currWI].offset)2112" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2113" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2112"
  %CastToValueType2114 = bitcast i8* %"&pSB[currWI].offset2113" to <16 x float> addrspace(3)**
  %loadedValue2115 = load <16 x float> addrspace(3)** %CastToValueType2114, align 8
  %vstore529 = getelementptr <16 x float> addrspace(3)* %loadedValue2115, i64 0, i64 8
  %"&(pSB[currWI].offset)2028" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2029" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2028"
  %CastToValueType2030 = bitcast i8* %"&pSB[currWI].offset2029" to <16 x float>*
  %loadedValue2031 = load <16 x float>* %CastToValueType2030, align 64
  %exData531 = extractelement <16 x float> %loadedValue2031, i32 8
  store float %exData531, float addrspace(3)* %vstore529, align 4
  br label %postload961

postload961:                                      ; preds = %preload960, %postload958
  %"&(pSB[currWI].offset)1089" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1090" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1089"
  %CastToValueType1091 = bitcast i8* %"&pSB[currWI].offset1090" to <16 x i1>*
  %loadedValue1092 = load <16 x i1>* %CastToValueType1091, align 16
  %exmask533 = extractelement <16 x i1> %loadedValue1092, i32 9
  br i1 %exmask533, label %preload963, label %postload964

preload963:                                       ; preds = %postload961
  %"&(pSB[currWI].offset)2107" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2108" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2107"
  %CastToValueType2109 = bitcast i8* %"&pSB[currWI].offset2108" to <16 x float> addrspace(3)**
  %loadedValue2110 = load <16 x float> addrspace(3)** %CastToValueType2109, align 8
  %vstore532 = getelementptr <16 x float> addrspace(3)* %loadedValue2110, i64 0, i64 9
  %"&(pSB[currWI].offset)2023" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2024" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2023"
  %CastToValueType2025 = bitcast i8* %"&pSB[currWI].offset2024" to <16 x float>*
  %loadedValue2026 = load <16 x float>* %CastToValueType2025, align 64
  %exData534 = extractelement <16 x float> %loadedValue2026, i32 9
  store float %exData534, float addrspace(3)* %vstore532, align 4
  br label %postload964

postload964:                                      ; preds = %preload963, %postload961
  %"&(pSB[currWI].offset)1084" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1085" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1084"
  %CastToValueType1086 = bitcast i8* %"&pSB[currWI].offset1085" to <16 x i1>*
  %loadedValue1087 = load <16 x i1>* %CastToValueType1086, align 16
  %exmask536 = extractelement <16 x i1> %loadedValue1087, i32 10
  br i1 %exmask536, label %preload966, label %postload967

preload966:                                       ; preds = %postload964
  %"&(pSB[currWI].offset)2102" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2103" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2102"
  %CastToValueType2104 = bitcast i8* %"&pSB[currWI].offset2103" to <16 x float> addrspace(3)**
  %loadedValue2105 = load <16 x float> addrspace(3)** %CastToValueType2104, align 8
  %vstore535 = getelementptr <16 x float> addrspace(3)* %loadedValue2105, i64 0, i64 10
  %"&(pSB[currWI].offset)2018" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2019" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2018"
  %CastToValueType2020 = bitcast i8* %"&pSB[currWI].offset2019" to <16 x float>*
  %loadedValue2021 = load <16 x float>* %CastToValueType2020, align 64
  %exData537 = extractelement <16 x float> %loadedValue2021, i32 10
  store float %exData537, float addrspace(3)* %vstore535, align 4
  br label %postload967

postload967:                                      ; preds = %preload966, %postload964
  %"&(pSB[currWI].offset)1079" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1080" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1079"
  %CastToValueType1081 = bitcast i8* %"&pSB[currWI].offset1080" to <16 x i1>*
  %loadedValue1082 = load <16 x i1>* %CastToValueType1081, align 16
  %exmask539 = extractelement <16 x i1> %loadedValue1082, i32 11
  br i1 %exmask539, label %preload969, label %postload970

preload969:                                       ; preds = %postload967
  %"&(pSB[currWI].offset)2097" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2098" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2097"
  %CastToValueType2099 = bitcast i8* %"&pSB[currWI].offset2098" to <16 x float> addrspace(3)**
  %loadedValue2100 = load <16 x float> addrspace(3)** %CastToValueType2099, align 8
  %vstore538 = getelementptr <16 x float> addrspace(3)* %loadedValue2100, i64 0, i64 11
  %"&(pSB[currWI].offset)2013" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2014" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2013"
  %CastToValueType2015 = bitcast i8* %"&pSB[currWI].offset2014" to <16 x float>*
  %loadedValue2016 = load <16 x float>* %CastToValueType2015, align 64
  %exData540 = extractelement <16 x float> %loadedValue2016, i32 11
  store float %exData540, float addrspace(3)* %vstore538, align 4
  br label %postload970

postload970:                                      ; preds = %preload969, %postload967
  %"&(pSB[currWI].offset)1074" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1075" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1074"
  %CastToValueType1076 = bitcast i8* %"&pSB[currWI].offset1075" to <16 x i1>*
  %loadedValue1077 = load <16 x i1>* %CastToValueType1076, align 16
  %exmask542 = extractelement <16 x i1> %loadedValue1077, i32 12
  br i1 %exmask542, label %preload972, label %postload973

preload972:                                       ; preds = %postload970
  %"&(pSB[currWI].offset)2092" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2093" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2092"
  %CastToValueType2094 = bitcast i8* %"&pSB[currWI].offset2093" to <16 x float> addrspace(3)**
  %loadedValue2095 = load <16 x float> addrspace(3)** %CastToValueType2094, align 8
  %vstore541 = getelementptr <16 x float> addrspace(3)* %loadedValue2095, i64 0, i64 12
  %"&(pSB[currWI].offset)2008" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2009" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2008"
  %CastToValueType2010 = bitcast i8* %"&pSB[currWI].offset2009" to <16 x float>*
  %loadedValue2011 = load <16 x float>* %CastToValueType2010, align 64
  %exData543 = extractelement <16 x float> %loadedValue2011, i32 12
  store float %exData543, float addrspace(3)* %vstore541, align 4
  br label %postload973

postload973:                                      ; preds = %preload972, %postload970
  %"&(pSB[currWI].offset)1069" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1070" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1069"
  %CastToValueType1071 = bitcast i8* %"&pSB[currWI].offset1070" to <16 x i1>*
  %loadedValue1072 = load <16 x i1>* %CastToValueType1071, align 16
  %exmask545 = extractelement <16 x i1> %loadedValue1072, i32 13
  br i1 %exmask545, label %preload975, label %postload976

preload975:                                       ; preds = %postload973
  %"&(pSB[currWI].offset)2087" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2088" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2087"
  %CastToValueType2089 = bitcast i8* %"&pSB[currWI].offset2088" to <16 x float> addrspace(3)**
  %loadedValue2090 = load <16 x float> addrspace(3)** %CastToValueType2089, align 8
  %vstore544 = getelementptr <16 x float> addrspace(3)* %loadedValue2090, i64 0, i64 13
  %"&(pSB[currWI].offset)2003" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset2004" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2003"
  %CastToValueType2005 = bitcast i8* %"&pSB[currWI].offset2004" to <16 x float>*
  %loadedValue2006 = load <16 x float>* %CastToValueType2005, align 64
  %exData546 = extractelement <16 x float> %loadedValue2006, i32 13
  store float %exData546, float addrspace(3)* %vstore544, align 4
  br label %postload976

postload976:                                      ; preds = %preload975, %postload973
  %"&(pSB[currWI].offset)1064" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1065" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1064"
  %CastToValueType1066 = bitcast i8* %"&pSB[currWI].offset1065" to <16 x i1>*
  %loadedValue1067 = load <16 x i1>* %CastToValueType1066, align 16
  %exmask548 = extractelement <16 x i1> %loadedValue1067, i32 14
  br i1 %exmask548, label %preload978, label %postload979

preload978:                                       ; preds = %postload976
  %"&(pSB[currWI].offset)2082" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2083" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2082"
  %CastToValueType2084 = bitcast i8* %"&pSB[currWI].offset2083" to <16 x float> addrspace(3)**
  %loadedValue2085 = load <16 x float> addrspace(3)** %CastToValueType2084, align 8
  %vstore547 = getelementptr <16 x float> addrspace(3)* %loadedValue2085, i64 0, i64 14
  %"&(pSB[currWI].offset)1998" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset1999" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1998"
  %CastToValueType2000 = bitcast i8* %"&pSB[currWI].offset1999" to <16 x float>*
  %loadedValue2001 = load <16 x float>* %CastToValueType2000, align 64
  %exData549 = extractelement <16 x float> %loadedValue2001, i32 14
  store float %exData549, float addrspace(3)* %vstore547, align 4
  br label %postload979

postload979:                                      ; preds = %preload978, %postload976
  %"&(pSB[currWI].offset)1059" = add nuw i64 %CurrSBIndex..4, 176
  %"&pSB[currWI].offset1060" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1059"
  %CastToValueType1061 = bitcast i8* %"&pSB[currWI].offset1060" to <16 x i1>*
  %loadedValue1062 = load <16 x i1>* %CastToValueType1061, align 16
  %exmask551 = extractelement <16 x i1> %loadedValue1062, i32 15
  br i1 %exmask551, label %preload981, label %postload982

preload981:                                       ; preds = %postload979
  %"&(pSB[currWI].offset)2077" = add nuw i64 %CurrSBIndex..4, 2688
  %"&pSB[currWI].offset2078" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2077"
  %CastToValueType2079 = bitcast i8* %"&pSB[currWI].offset2078" to <16 x float> addrspace(3)**
  %loadedValue2080 = load <16 x float> addrspace(3)** %CastToValueType2079, align 8
  %vstore550 = getelementptr <16 x float> addrspace(3)* %loadedValue2080, i64 0, i64 15
  %"&(pSB[currWI].offset)1993" = add nuw i64 %CurrSBIndex..4, 2624
  %"&pSB[currWI].offset1994" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1993"
  %CastToValueType1995 = bitcast i8* %"&pSB[currWI].offset1994" to <16 x float>*
  %loadedValue1996 = load <16 x float>* %CastToValueType1995, align 64
  %exData552 = extractelement <16 x float> %loadedValue1996, i32 15
  store float %exData552, float addrspace(3)* %vstore550, align 4
  br label %postload982

postload982:                                      ; preds = %preload981, %postload979
  %"&(pSB[currWI].offset)1041" = add nuw i64 %CurrSBIndex..4, 160
  %"&pSB[currWI].offset1042" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1041"
  %CastToValueType1043 = bitcast i8* %"&pSB[currWI].offset1042" to <16 x i1>*
  %loadedValue1044 = load <16 x i1>* %CastToValueType1043, align 16
  %extract306 = extractelement <16 x i1> %loadedValue1044, i32 0
  br i1 %extract306, label %preload797, label %postload798

preload797:                                       ; preds = %postload982
  %check.WI.iter2195 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter2195, label %thenBB2192, label %postload798

thenBB2192:                                       ; preds = %preload797
  %"CurrWI++2196" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride2198" = add nuw i64 %CurrSBIndex..4, 2752
  %cond10 = icmp eq i32 %currBarrier.2, 8
  br i1 %cond10, label %postload798, label %SyncBB

postload798:                                      ; preds = %thenBB2184, %thenBB2177, %thenBB2192, %preload797, %postload982
  %currBarrier.1 = phi i32 [ %currBarrier.2, %postload982 ], [ %currBarrier.3, %thenBB2177 ], [ %currBarrier.2, %thenBB2192 ], [ %currBarrier.3, %thenBB2184 ], [ 8, %preload797 ]
  %CurrSBIndex..3 = phi i64 [ %CurrSBIndex..4, %postload982 ], [ %"loadedCurrSB+Stride2183", %thenBB2177 ], [ %"loadedCurrSB+Stride2198", %thenBB2192 ], [ %"loadedCurrSB+Stride2190", %thenBB2184 ], [ 0, %preload797 ]
  %CurrWI..3 = phi i64 [ %CurrWI..4, %postload982 ], [ %"CurrWI++2181", %thenBB2177 ], [ %"CurrWI++2196", %thenBB2192 ], [ %"CurrWI++2188", %thenBB2184 ], [ 0, %preload797 ]
  %"&(pSB[currWI].offset)1050" = add nuw i64 %CurrSBIndex..3, 164
  %"&pSB[currWI].offset1051" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1050"
  %CastToValueType1052 = bitcast i8* %"&pSB[currWI].offset1051" to i32*
  %loadedValue1053 = load i32* %CastToValueType1052, align 4
  %s.0 = lshr i32 %loadedValue1053, 1
  %"&(pSB[currWI].offset)2157" = add nuw i64 %CurrSBIndex..3, 2696
  %"&pSB[currWI].offset2158" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2157"
  %CastToValueType2159 = bitcast i8* %"&pSB[currWI].offset2158" to i32*
  store i32 %s.0, i32* %CastToValueType2159, align 4
  %184 = icmp eq i32 %s.0, 0
  %temp322 = insertelement <16 x i1> undef, i1 %184, i32 0
  %vector323 = shufflevector <16 x i1> %temp322, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond20 = xor i1 %184, true
  %temp328 = insertelement <16 x i1> undef, i1 %notCond20, i32 0
  %vector329 = shufflevector <16 x i1> %temp328, <16 x i1> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1036" = add nuw i64 %CurrSBIndex..3, 160
  %"&pSB[currWI].offset1037" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1036"
  %CastToValueType1038 = bitcast i8* %"&pSB[currWI].offset1037" to <16 x i1>*
  %loadedValue1039 = load <16 x i1>* %CastToValueType1038, align 16
  %who_left_tr21324 = and <16 x i1> %loadedValue1039, %vector323
  %"&(pSB[currWI].offset)1022" = add nuw i64 %CurrSBIndex..3, 144
  %"&pSB[currWI].offset1023" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1022"
  %CastToValueType1024 = bitcast i8* %"&pSB[currWI].offset1023" to <16 x i1>*
  %loadedValue1025 = load <16 x i1>* %CastToValueType1024, align 16
  %loop_mask24326 = or <16 x i1> %loadedValue1025, %who_left_tr21324
  %"&(pSB[currWI].offset)2161" = add nuw i64 %CurrSBIndex..3, 2704
  %"&pSB[currWI].offset2162" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2161"
  %CastToValueType2163 = bitcast i8* %"&pSB[currWI].offset2162" to <16 x i1>*
  store <16 x i1> %loop_mask24326, <16 x i1>* %CastToValueType2163, align 16
  %curr_loop_mask26327 = or <16 x i1> %loop_mask24326, %who_left_tr21324
  %ipred.i5 = bitcast <16 x i1> %curr_loop_mask26327 to i16
  %val.i6 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i5, i16 %ipred.i5) nounwind
  %tmp.i7 = and i32 %val.i6, 1
  %res.i8 = icmp eq i32 %tmp.i7, 0
  %"&(pSB[currWI].offset)1031" = add nuw i64 %CurrSBIndex..3, 160
  %"&pSB[currWI].offset1032" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1031"
  %CastToValueType1033 = bitcast i8* %"&pSB[currWI].offset1032" to <16 x i1>*
  %loadedValue1034 = load <16 x i1>* %CastToValueType1033, align 16
  %local_edge29330 = and <16 x i1> %loadedValue1034, %vector329
  %"&(pSB[currWI].offset)2165" = add nuw i64 %CurrSBIndex..3, 2720
  %"&pSB[currWI].offset2166" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2165"
  %CastToValueType2167 = bitcast i8* %"&pSB[currWI].offset2166" to <16 x i1>*
  store <16 x i1> %local_edge29330, <16 x i1>* %CastToValueType2167, align 16
  br i1 %res.i8, label %bb.nph, label %._crit_edge

._crit_edge:                                      ; preds = %postload798, %SyncBB
  %currBarrier.3 = phi i32 [ %currBarrier.4, %SyncBB ], [ %currBarrier.1, %postload798 ]
  %CurrSBIndex..5 = phi i64 [ %CurrSBIndex..1, %SyncBB ], [ %CurrSBIndex..3, %postload798 ]
  %CurrWI..5 = phi i64 [ %CurrWI..1, %SyncBB ], [ %CurrWI..3, %postload798 ]
  %"&(pSB[currWI].offset)985" = add nuw i64 %CurrSBIndex..5, 64
  %"&pSB[currWI].offset986" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)985"
  %CastToValueType987 = bitcast i8* %"&pSB[currWI].offset986" to <16 x i32>*
  %loadedValue = load <16 x i32>* %CastToValueType987, align 64
  %185 = icmp eq <16 x i32> %loadedValue, zeroinitializer
  %extract332 = extractelement <16 x i1> %185, i32 0
  %extract333 = extractelement <16 x i1> %185, i32 1
  %extract334 = extractelement <16 x i1> %185, i32 2
  %extract335 = extractelement <16 x i1> %185, i32 3
  %extract336 = extractelement <16 x i1> %185, i32 4
  %extract337 = extractelement <16 x i1> %185, i32 5
  %extract338 = extractelement <16 x i1> %185, i32 6
  %extract339 = extractelement <16 x i1> %185, i32 7
  %extract340 = extractelement <16 x i1> %185, i32 8
  %extract341 = extractelement <16 x i1> %185, i32 9
  %extract342 = extractelement <16 x i1> %185, i32 10
  %extract343 = extractelement <16 x i1> %185, i32 11
  %extract344 = extractelement <16 x i1> %185, i32 12
  %extract345 = extractelement <16 x i1> %185, i32 13
  %extract346 = extractelement <16 x i1> %185, i32 14
  %extract347 = extractelement <16 x i1> %185, i32 15
  br i1 %extract332, label %preload808, label %postload809

preload808:                                       ; preds = %._crit_edge
  %masked_load553 = load float addrspace(3)* %sdata, align 4
  br label %postload809

postload809:                                      ; preds = %preload808, %._crit_edge
  %phi810 = phi float [ undef, %._crit_edge ], [ %masked_load553, %preload808 ]
  br i1 %extract333, label %preload816, label %postload817

preload816:                                       ; preds = %postload809
  %masked_load554 = load float addrspace(3)* %sdata, align 4
  br label %postload817

postload817:                                      ; preds = %preload816, %postload809
  %phi818 = phi float [ undef, %postload809 ], [ %masked_load554, %preload816 ]
  br i1 %extract334, label %preload824, label %postload825

preload824:                                       ; preds = %postload817
  %masked_load555 = load float addrspace(3)* %sdata, align 4
  br label %postload825

postload825:                                      ; preds = %preload824, %postload817
  %phi826 = phi float [ undef, %postload817 ], [ %masked_load555, %preload824 ]
  br i1 %extract335, label %preload832, label %postload833

preload832:                                       ; preds = %postload825
  %masked_load556 = load float addrspace(3)* %sdata, align 4
  br label %postload833

postload833:                                      ; preds = %preload832, %postload825
  %phi834 = phi float [ undef, %postload825 ], [ %masked_load556, %preload832 ]
  br i1 %extract336, label %preload840, label %postload841

preload840:                                       ; preds = %postload833
  %masked_load557 = load float addrspace(3)* %sdata, align 4
  br label %postload841

postload841:                                      ; preds = %preload840, %postload833
  %phi842 = phi float [ undef, %postload833 ], [ %masked_load557, %preload840 ]
  br i1 %extract337, label %preload848, label %postload849

preload848:                                       ; preds = %postload841
  %masked_load558 = load float addrspace(3)* %sdata, align 4
  br label %postload849

postload849:                                      ; preds = %preload848, %postload841
  %phi850 = phi float [ undef, %postload841 ], [ %masked_load558, %preload848 ]
  br i1 %extract338, label %preload856, label %postload857

preload856:                                       ; preds = %postload849
  %masked_load559 = load float addrspace(3)* %sdata, align 4
  br label %postload857

postload857:                                      ; preds = %preload856, %postload849
  %phi858 = phi float [ undef, %postload849 ], [ %masked_load559, %preload856 ]
  br i1 %extract339, label %preload864, label %postload865

preload864:                                       ; preds = %postload857
  %masked_load560 = load float addrspace(3)* %sdata, align 4
  br label %postload865

postload865:                                      ; preds = %preload864, %postload857
  %phi866 = phi float [ undef, %postload857 ], [ %masked_load560, %preload864 ]
  br i1 %extract340, label %preload872, label %postload873

preload872:                                       ; preds = %postload865
  %masked_load561 = load float addrspace(3)* %sdata, align 4
  br label %postload873

postload873:                                      ; preds = %preload872, %postload865
  %phi874 = phi float [ undef, %postload865 ], [ %masked_load561, %preload872 ]
  br i1 %extract341, label %preload880, label %postload881

preload880:                                       ; preds = %postload873
  %masked_load562 = load float addrspace(3)* %sdata, align 4
  br label %postload881

postload881:                                      ; preds = %preload880, %postload873
  %phi882 = phi float [ undef, %postload873 ], [ %masked_load562, %preload880 ]
  br i1 %extract342, label %preload888, label %postload889

preload888:                                       ; preds = %postload881
  %masked_load563 = load float addrspace(3)* %sdata, align 4
  br label %postload889

postload889:                                      ; preds = %preload888, %postload881
  %phi890 = phi float [ undef, %postload881 ], [ %masked_load563, %preload888 ]
  br i1 %extract343, label %preload896, label %postload897

preload896:                                       ; preds = %postload889
  %masked_load564 = load float addrspace(3)* %sdata, align 4
  br label %postload897

postload897:                                      ; preds = %preload896, %postload889
  %phi898 = phi float [ undef, %postload889 ], [ %masked_load564, %preload896 ]
  br i1 %extract344, label %preload904, label %postload905

preload904:                                       ; preds = %postload897
  %masked_load565 = load float addrspace(3)* %sdata, align 4
  br label %postload905

postload905:                                      ; preds = %preload904, %postload897
  %phi906 = phi float [ undef, %postload897 ], [ %masked_load565, %preload904 ]
  br i1 %extract345, label %preload912, label %postload913

preload912:                                       ; preds = %postload905
  %masked_load566 = load float addrspace(3)* %sdata, align 4
  br label %postload913

postload913:                                      ; preds = %preload912, %postload905
  %phi914 = phi float [ undef, %postload905 ], [ %masked_load566, %preload912 ]
  br i1 %extract346, label %preload920, label %postload921

preload920:                                       ; preds = %postload913
  %masked_load567 = load float addrspace(3)* %sdata, align 4
  br label %postload921

postload921:                                      ; preds = %preload920, %postload913
  %phi922 = phi float [ undef, %postload913 ], [ %masked_load567, %preload920 ]
  br i1 %extract347, label %preload928, label %postload929

preload928:                                       ; preds = %postload921
  %masked_load568 = load float addrspace(3)* %sdata, align 4
  br label %postload929

postload929:                                      ; preds = %preload928, %postload921
  %phi930 = phi float [ undef, %postload921 ], [ %masked_load568, %preload928 ]
  br i1 %extract332, label %preload811, label %postload812

preload811:                                       ; preds = %postload929
  %186 = load i64* %pWGId, align 8
  br label %postload812

postload812:                                      ; preds = %preload811, %postload929
  %phi813 = phi i64 [ undef, %postload929 ], [ %186, %preload811 ]
  br i1 %extract333, label %preload819, label %postload820

preload819:                                       ; preds = %postload812
  %187 = load i64* %pWGId, align 8
  br label %postload820

postload820:                                      ; preds = %preload819, %postload812
  %phi821 = phi i64 [ undef, %postload812 ], [ %187, %preload819 ]
  br i1 %extract334, label %preload827, label %postload828

preload827:                                       ; preds = %postload820
  %188 = load i64* %pWGId, align 8
  br label %postload828

postload828:                                      ; preds = %preload827, %postload820
  %phi829 = phi i64 [ undef, %postload820 ], [ %188, %preload827 ]
  br i1 %extract335, label %preload835, label %postload836

preload835:                                       ; preds = %postload828
  %189 = load i64* %pWGId, align 8
  br label %postload836

postload836:                                      ; preds = %preload835, %postload828
  %phi837 = phi i64 [ undef, %postload828 ], [ %189, %preload835 ]
  br i1 %extract336, label %preload843, label %postload844

preload843:                                       ; preds = %postload836
  %190 = load i64* %pWGId, align 8
  br label %postload844

postload844:                                      ; preds = %preload843, %postload836
  %phi845 = phi i64 [ undef, %postload836 ], [ %190, %preload843 ]
  br i1 %extract337, label %preload851, label %postload852

preload851:                                       ; preds = %postload844
  %191 = load i64* %pWGId, align 8
  br label %postload852

postload852:                                      ; preds = %preload851, %postload844
  %phi853 = phi i64 [ undef, %postload844 ], [ %191, %preload851 ]
  br i1 %extract338, label %preload859, label %postload860

preload859:                                       ; preds = %postload852
  %192 = load i64* %pWGId, align 8
  br label %postload860

postload860:                                      ; preds = %preload859, %postload852
  %phi861 = phi i64 [ undef, %postload852 ], [ %192, %preload859 ]
  br i1 %extract339, label %preload867, label %postload868

preload867:                                       ; preds = %postload860
  %193 = load i64* %pWGId, align 8
  br label %postload868

postload868:                                      ; preds = %preload867, %postload860
  %phi869 = phi i64 [ undef, %postload860 ], [ %193, %preload867 ]
  br i1 %extract340, label %preload875, label %postload876

preload875:                                       ; preds = %postload868
  %194 = load i64* %pWGId, align 8
  br label %postload876

postload876:                                      ; preds = %preload875, %postload868
  %phi877 = phi i64 [ undef, %postload868 ], [ %194, %preload875 ]
  br i1 %extract341, label %preload883, label %postload884

preload883:                                       ; preds = %postload876
  %195 = load i64* %pWGId, align 8
  br label %postload884

postload884:                                      ; preds = %preload883, %postload876
  %phi885 = phi i64 [ undef, %postload876 ], [ %195, %preload883 ]
  br i1 %extract342, label %preload891, label %postload892

preload891:                                       ; preds = %postload884
  %196 = load i64* %pWGId, align 8
  br label %postload892

postload892:                                      ; preds = %preload891, %postload884
  %phi893 = phi i64 [ undef, %postload884 ], [ %196, %preload891 ]
  br i1 %extract343, label %preload899, label %postload900

preload899:                                       ; preds = %postload892
  %197 = load i64* %pWGId, align 8
  br label %postload900

postload900:                                      ; preds = %preload899, %postload892
  %phi901 = phi i64 [ undef, %postload892 ], [ %197, %preload899 ]
  br i1 %extract344, label %preload907, label %postload908

preload907:                                       ; preds = %postload900
  %198 = load i64* %pWGId, align 8
  br label %postload908

postload908:                                      ; preds = %preload907, %postload900
  %phi909 = phi i64 [ undef, %postload900 ], [ %198, %preload907 ]
  br i1 %extract345, label %preload915, label %postload916

preload915:                                       ; preds = %postload908
  %199 = load i64* %pWGId, align 8
  br label %postload916

postload916:                                      ; preds = %preload915, %postload908
  %phi917 = phi i64 [ undef, %postload908 ], [ %199, %preload915 ]
  br i1 %extract346, label %preload923, label %postload924

preload923:                                       ; preds = %postload916
  %200 = load i64* %pWGId, align 8
  br label %postload924

postload924:                                      ; preds = %preload923, %postload916
  %phi925 = phi i64 [ undef, %postload916 ], [ %200, %preload923 ]
  br i1 %extract347, label %preload931, label %postload932

preload931:                                       ; preds = %postload924
  %201 = load i64* %pWGId, align 8
  br label %postload932

postload932:                                      ; preds = %preload931, %postload924
  %phi933 = phi i64 [ undef, %postload924 ], [ %201, %preload931 ]
  %202 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi821
  %203 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi829
  %204 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi837
  %205 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi845
  %206 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi853
  %207 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi861
  %208 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi869
  %209 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi877
  %210 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi885
  %211 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi893
  %212 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi901
  %213 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi909
  %214 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi917
  %215 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi925
  %216 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi933
  br i1 %extract332, label %preload814, label %postload815

preload814:                                       ; preds = %postload932
  %217 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %phi813
  store float %phi810, float addrspace(1)* %217, align 4
  br label %postload815

postload815:                                      ; preds = %preload814, %postload932
  br i1 %extract333, label %preload822, label %postload823

preload822:                                       ; preds = %postload815
  store float %phi818, float addrspace(1)* %202, align 4
  br label %postload823

postload823:                                      ; preds = %preload822, %postload815
  br i1 %extract334, label %preload830, label %postload831

preload830:                                       ; preds = %postload823
  store float %phi826, float addrspace(1)* %203, align 4
  br label %postload831

postload831:                                      ; preds = %preload830, %postload823
  br i1 %extract335, label %preload838, label %postload839

preload838:                                       ; preds = %postload831
  store float %phi834, float addrspace(1)* %204, align 4
  br label %postload839

postload839:                                      ; preds = %preload838, %postload831
  br i1 %extract336, label %preload846, label %postload847

preload846:                                       ; preds = %postload839
  store float %phi842, float addrspace(1)* %205, align 4
  br label %postload847

postload847:                                      ; preds = %preload846, %postload839
  br i1 %extract337, label %preload854, label %postload855

preload854:                                       ; preds = %postload847
  store float %phi850, float addrspace(1)* %206, align 4
  br label %postload855

postload855:                                      ; preds = %preload854, %postload847
  br i1 %extract338, label %preload862, label %postload863

preload862:                                       ; preds = %postload855
  store float %phi858, float addrspace(1)* %207, align 4
  br label %postload863

postload863:                                      ; preds = %preload862, %postload855
  br i1 %extract339, label %preload870, label %postload871

preload870:                                       ; preds = %postload863
  store float %phi866, float addrspace(1)* %208, align 4
  br label %postload871

postload871:                                      ; preds = %preload870, %postload863
  br i1 %extract340, label %preload878, label %postload879

preload878:                                       ; preds = %postload871
  store float %phi874, float addrspace(1)* %209, align 4
  br label %postload879

postload879:                                      ; preds = %preload878, %postload871
  br i1 %extract341, label %preload886, label %postload887

preload886:                                       ; preds = %postload879
  store float %phi882, float addrspace(1)* %210, align 4
  br label %postload887

postload887:                                      ; preds = %preload886, %postload879
  br i1 %extract342, label %preload894, label %postload895

preload894:                                       ; preds = %postload887
  store float %phi890, float addrspace(1)* %211, align 4
  br label %postload895

postload895:                                      ; preds = %preload894, %postload887
  br i1 %extract343, label %preload902, label %postload903

preload902:                                       ; preds = %postload895
  store float %phi898, float addrspace(1)* %212, align 4
  br label %postload903

postload903:                                      ; preds = %preload902, %postload895
  br i1 %extract344, label %preload910, label %postload911

preload910:                                       ; preds = %postload903
  store float %phi906, float addrspace(1)* %213, align 4
  br label %postload911

postload911:                                      ; preds = %preload910, %postload903
  br i1 %extract345, label %preload918, label %postload919

preload918:                                       ; preds = %postload911
  store float %phi914, float addrspace(1)* %214, align 4
  br label %postload919

postload919:                                      ; preds = %preload918, %postload911
  br i1 %extract346, label %preload926, label %postload927

preload926:                                       ; preds = %postload919
  store float %phi922, float addrspace(1)* %215, align 4
  br label %postload927

postload927:                                      ; preds = %preload926, %postload919
  br i1 %extract347, label %preload934, label %UnifiedReturnBlock

preload934:                                       ; preds = %postload927
  store float %phi930, float addrspace(1)* %216, align 4
  %check.WI.iter2180 = icmp ult i64 %CurrWI..5, %iterCount
  br i1 %check.WI.iter2180, label %thenBB2177, label %SyncBB2173

thenBB2177:                                       ; preds = %preload934
  %"CurrWI++2181" = add nuw i64 %CurrWI..5, 1
  %"loadedCurrSB+Stride2183" = add nuw i64 %CurrSBIndex..5, 2752
  %cond9 = icmp eq i32 %currBarrier.3, 8
  br i1 %cond9, label %postload798, label %SyncBB

SyncBB2173:                                       ; preds = %UnifiedReturnBlock, %preload934
  ret void

UnifiedReturnBlock:                               ; preds = %postload927
  %check.WI.iter2187 = icmp ult i64 %CurrWI..5, %iterCount
  br i1 %check.WI.iter2187, label %thenBB2184, label %SyncBB2173

thenBB2184:                                       ; preds = %UnifiedReturnBlock
  %"CurrWI++2188" = add nuw i64 %CurrWI..5, 1
  %"loadedCurrSB+Stride2190" = add nuw i64 %CurrSBIndex..5, 2752
  %cond = icmp eq i32 %currBarrier.3, 8
  br i1 %cond, label %postload798, label %SyncBB
}

define void @____Vectorized_.reduceNoLocal_separated_args(float addrspace(1)* nocapture %g_idata, float addrspace(1)* nocapture %g_odata, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp eq i32 %n, 0
  %tmp = zext i32 %n to i64
  br label %SyncBB1

SyncBB1:                                          ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  br i1 %0, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %SyncBB1, %bb.nph
  %indvar = phi i64 [ %indvar.next, %bb.nph ], [ 0, %SyncBB1 ]
  %sum.01 = phi float [ %2, %bb.nph ], [ 0.000000e+00, %SyncBB1 ]
  %scevgep = getelementptr float addrspace(1)* %g_idata, i64 %indvar
  %1 = load float addrspace(1)* %scevgep, align 4
  %2 = fadd float %sum.01, %1
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB1
  %sum.0.lcssa = phi float [ 0.000000e+00, %SyncBB1 ], [ %2, %bb.nph ]
  store float %sum.0.lcssa, float addrspace(1)* %g_odata, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB1

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @reduceNoLocal(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
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
  %sum.01.i = phi float [ %13, %bb.nph.i ], [ 0.000000e+00, %SyncBB.i ]
  %scevgep.i = getelementptr float addrspace(1)* %1, i64 %indvar.i
  %12 = load float addrspace(1)* %scevgep.i, align 4
  %13 = fadd float %sum.01.i, %12
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp.i
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %sum.0.lcssa.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %13, %bb.nph.i ]
  store float %sum.0.lcssa.i, float addrspace(1)* %4, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %10
  br i1 %check.WI.iter.i, label %thenBB.i, label %__reduceNoLocal_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__reduceNoLocal_separated_args.exit:              ; preds = %._crit_edge.i
  ret void
}

define void @reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(3)**
  %7 = load float addrspace(3)** %6, align 8
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
  br label %SyncBB73.i

SyncBB73.i:                                       ; preds = %thenBB.i, %entry
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
  %42 = getelementptr inbounds float addrspace(3)* %7, i64 %41
  %"&(pSB[currWI].offset)341.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset35.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)341.i"
  %CastToValueType36.i = bitcast i8* %"&pSB[currWI].offset35.i" to float addrspace(3)**
  store float addrspace(3)* %42, float addrspace(3)** %CastToValueType36.i, align 8
  store float 0.000000e+00, float addrspace(3)* %42, align 4
  %43 = icmp ult i32 %35, %10
  br i1 %43, label %bb.nph4.i, label %._crit_edge5.i

bb.nph4.i:                                        ; preds = %SyncBB73.i
  %tmp.i = mul i64 %31, %37
  %tmp6.i = shl i64 %tmp.i, 1
  %tmp7.i = trunc i64 %tmp6.i to i32
  %tmp9.i = mul i64 %29, %31
  %tmp10.i = shl i64 %tmp9.i, 1
  %tmp11.i = add i64 %27, %tmp10.i
  %tmp12.i = trunc i64 %tmp11.i to i32
  %tmp15.i = add i32 %40, %tmp12.i
  %tmp17.i = add i32 %tmp7.i, %tmp12.i
  %"&(pSB[currWI].offset)482.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)482.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to float addrspace(3)**
  br label %44

; <label>:44                                      ; preds = %44, %bb.nph4.i
  %45 = phi float [ 0.000000e+00, %bb.nph4.i ], [ %53, %44 ]
  %indvar.i = phi i32 [ 0, %bb.nph4.i ], [ %indvar.next.i, %44 ]
  %tmp8.i = mul i32 %tmp7.i, %indvar.i
  %i.03.i = add i32 %tmp12.i, %tmp8.i
  %tmp16.i = add i32 %tmp15.i, %tmp8.i
  %46 = zext i32 %i.03.i to i64
  %47 = getelementptr inbounds float addrspace(1)* %1, i64 %46
  %48 = load float addrspace(1)* %47, align 4
  %49 = zext i32 %tmp16.i to i64
  %50 = getelementptr inbounds float addrspace(1)* %1, i64 %49
  %51 = load float addrspace(1)* %50, align 4
  %52 = fadd float %48, %51
  %53 = fadd float %45, %52
  %loadedValue51.i = load float addrspace(3)** %CastToValueType50.i, align 8
  store float %53, float addrspace(3)* %loadedValue51.i, align 4
  %tmp18.i = add i32 %tmp17.i, %tmp8.i
  %54 = icmp ult i32 %tmp18.i, %10
  %indvar.next.i = add i32 %indvar.i, 1
  br i1 %54, label %44, label %._crit_edge5.i

._crit_edge5.i:                                   ; preds = %44, %SyncBB73.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %._crit_edge5.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 2752
  br label %SyncBB73.i

elseBB.i:                                         ; preds = %._crit_edge5.i
  %s.01.i = lshr i32 %40, 1
  %55 = icmp eq i32 %s.01.i, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB83.i, %thenBB76.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride89.i", %thenBB83.i ], [ %"loadedCurrSB+Stride82.i", %thenBB76.i ]
  %currBarrier.3.i = phi i32 [ 0, %elseBB.i ], [ %currBarrier.2.i, %thenBB83.i ], [ %currBarrier.1.i, %thenBB76.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++87.i", %thenBB83.i ], [ %"CurrWI++80.i", %thenBB76.i ]
  br i1 %55, label %._crit_edge.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %SyncBB72.i, %SyncBB.i
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..2.i, %SyncBB72.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.0.i, %SyncBB72.i ], [ %currBarrier.3.i, %SyncBB.i ]
  %CurrWI..3.i = phi i64 [ %CurrWI..2.i, %SyncBB72.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %s.02.i = phi i32 [ %s.0.i, %SyncBB72.i ], [ %s.01.i, %SyncBB.i ]
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
  %60 = getelementptr inbounds float addrspace(3)* %7, i64 %59
  %61 = load float addrspace(3)* %60, align 4
  %"&(pSB[currWI].offset)38.i" = add nuw i64 %CurrSBIndex..3.i, 8
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)38.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to float addrspace(3)**
  %loadedValue41.i = load float addrspace(3)** %CastToValueType40.i, align 8
  %62 = load float addrspace(3)* %loadedValue41.i, align 4
  %63 = fadd float %62, %61
  %"&(pSB[currWI].offset)43.i" = add nuw i64 %CurrSBIndex..3.i, 8
  %"&pSB[currWI].offset44.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)43.i"
  %CastToValueType45.i = bitcast i8* %"&pSB[currWI].offset44.i" to float addrspace(3)**
  %loadedValue46.i = load float addrspace(3)** %CastToValueType45.i, align 8
  store float %63, float addrspace(3)* %loadedValue46.i, align 4
  br label %64

; <label>:64                                      ; preds = %57, %bb.nph.i
  %check.WI.iter79.i = icmp ult i64 %CurrWI..3.i, %22
  br i1 %check.WI.iter79.i, label %thenBB76.i, label %SyncBB72.i

thenBB76.i:                                       ; preds = %64
  %"CurrWI++80.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride82.i" = add nuw i64 %CurrSBIndex..3.i, 2752
  %cond3.i = icmp eq i32 %currBarrier.1.i, 0
  br i1 %cond3.i, label %SyncBB.i, label %SyncBB72.i

SyncBB72.i:                                       ; preds = %thenBB83.i, %thenBB76.i, %64
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride89.i", %thenBB83.i ], [ %"loadedCurrSB+Stride82.i", %thenBB76.i ], [ 0, %64 ]
  %currBarrier.0.i = phi i32 [ %currBarrier.2.i, %thenBB83.i ], [ %currBarrier.1.i, %thenBB76.i ], [ 1, %64 ]
  %CurrWI..2.i = phi i64 [ %"CurrWI++87.i", %thenBB83.i ], [ %"CurrWI++80.i", %thenBB76.i ], [ 0, %64 ]
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

._crit_edge.i:                                    ; preds = %SyncBB72.i, %SyncBB.i
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..2.i, %SyncBB72.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.3.i, %SyncBB.i ], [ %currBarrier.0.i, %SyncBB72.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..2.i, %SyncBB72.i ]
  %"&pSB[currWI].offset30.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..4.i
  %CastToValueType31.i = bitcast i8* %"&pSB[currWI].offset30.i" to i32*
  %loadedValue32.i = load i32* %CastToValueType31.i, align 4
  %66 = icmp eq i32 %loadedValue32.i, 0
  br i1 %66, label %67, label %UnifiedReturnBlock.i

; <label>:67                                      ; preds = %._crit_edge.i
  %68 = load float addrspace(3)* %7, align 4
  %69 = load i64* %16, align 8
  %70 = getelementptr inbounds float addrspace(1)* %4, i64 %69
  store float %68, float addrspace(1)* %70, align 4
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %67, %._crit_edge.i
  %check.WI.iter86.i = icmp ult i64 %CurrWI..4.i, %22
  br i1 %check.WI.iter86.i, label %thenBB83.i, label %__reduce_separated_args.exit

thenBB83.i:                                       ; preds = %UnifiedReturnBlock.i
  %"CurrWI++87.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride89.i" = add nuw i64 %CurrSBIndex..4.i, 2752
  %cond.i = icmp eq i32 %currBarrier.2.i, 1
  br i1 %cond.i, label %SyncBB72.i, label %SyncBB.i

__reduce_separated_args.exit:                     ; preds = %UnifiedReturnBlock.i
  ret void
}

define void @__Vectorized_.reduceNoLocal(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 72
  %9 = bitcast i8* %8 to i64*
  %10 = load i64* %9, align 8
  %11 = icmp eq i32 %7, 0
  %tmp.i = zext i32 %7 to i64
  br label %SyncBB1.i

SyncBB1.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %11, label %._crit_edge.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB1.i
  %indvar.i = phi i64 [ %indvar.next.i, %bb.nph.i ], [ 0, %SyncBB1.i ]
  %sum.01.i = phi float [ %13, %bb.nph.i ], [ 0.000000e+00, %SyncBB1.i ]
  %scevgep.i = getelementptr float addrspace(1)* %1, i64 %indvar.i
  %12 = load float addrspace(1)* %scevgep.i, align 4
  %13 = fadd float %sum.01.i, %12
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp.i
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB1.i
  %sum.0.lcssa.i = phi float [ 0.000000e+00, %SyncBB1.i ], [ %13, %bb.nph.i ]
  store float %sum.0.lcssa.i, float addrspace(1)* %4, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %10
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.reduceNoLocal_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB1.i

____Vectorized_.reduceNoLocal_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.reduce(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(3)**
  %7 = load float addrspace(3)** %6, align 8
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
  %temp81.i = insertelement <16 x i32> undef, i32 %10, i32 0
  %vector82.i = shufflevector <16 x i32> %temp81.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB2172.i

SyncBB2172.i:                                     ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %26 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %27, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %28 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %29 = trunc <16 x i64> %28 to <16 x i32>
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %29, <16 x i32>* %CastToValueType.i, align 64
  %30 = load i64* %16, align 8
  %31 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %32 = load i64* %31, align 8
  %33 = shl i64 %30, 1
  %34 = mul i64 %33, %32
  %temp.i = insertelement <16 x i64> undef, i64 %34, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %35 = add <16 x i64> %vector.i, %28
  %36 = trunc <16 x i64> %35 to <16 x i32>
  %37 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %38 = load i64* %37, align 8
  %39 = getelementptr %struct.WorkDim* %13, i64 0, i32 4, i64 0
  %40 = load i64* %39, align 8
  %41 = trunc i64 %38 to i32
  %extract.lhs.i = extractelement <16 x i64> %28, i32 0
  %extract.i = and i64 %extract.lhs.i, 4294967295
  %42 = getelementptr inbounds float addrspace(3)* %7, i64 %extract.i
  %"&(pSB[currWI].offset)999.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset1000.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)999.i"
  %CastToValueType1001.i = bitcast i8* %"&pSB[currWI].offset1000.i" to float addrspace(3)**
  store float addrspace(3)* %42, float addrspace(3)** %CastToValueType1001.i, align 8
  %ptrTypeCast.i = bitcast float addrspace(3)* %42 to <16 x float> addrspace(3)*
  store <16 x float> zeroinitializer, <16 x float> addrspace(3)* %ptrTypeCast.i, align 4
  %43 = icmp ult <16 x i32> %36, %vector82.i
  %temp.vect.i = insertelement <16 x i64> undef, i64 %40, i32 0
  %temp.vect86.i = insertelement <16 x i64> %temp.vect.i, i64 %40, i32 1
  %temp.vect87.i = insertelement <16 x i64> %temp.vect86.i, i64 %40, i32 2
  %temp.vect88.i = insertelement <16 x i64> %temp.vect87.i, i64 %40, i32 3
  %temp.vect89.i = insertelement <16 x i64> %temp.vect88.i, i64 %40, i32 4
  %temp.vect90.i = insertelement <16 x i64> %temp.vect89.i, i64 %40, i32 5
  %temp.vect91.i = insertelement <16 x i64> %temp.vect90.i, i64 %40, i32 6
  %temp.vect92.i = insertelement <16 x i64> %temp.vect91.i, i64 %40, i32 7
  %temp.vect93.i = insertelement <16 x i64> %temp.vect92.i, i64 %40, i32 8
  %temp.vect94.i = insertelement <16 x i64> %temp.vect93.i, i64 %40, i32 9
  %temp.vect95.i = insertelement <16 x i64> %temp.vect94.i, i64 %40, i32 10
  %temp.vect96.i = insertelement <16 x i64> %temp.vect95.i, i64 %40, i32 11
  %temp.vect97.i = insertelement <16 x i64> %temp.vect96.i, i64 %40, i32 12
  %temp.vect98.i = insertelement <16 x i64> %temp.vect97.i, i64 %40, i32 13
  %temp108.i = insertelement <16 x i32> undef, i32 %41, i32 0
  %temp.vect99.i = insertelement <16 x i64> %temp.vect98.i, i64 %40, i32 14
  %temp84.i = insertelement <16 x i64> undef, i64 %38, i32 0
  %vector109.i = shufflevector <16 x i32> %temp108.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp.vect100.i = insertelement <16 x i64> %temp.vect99.i, i64 %40, i32 15
  %vector85.i = shufflevector <16 x i64> %temp84.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp101.i = mul <16 x i64> %vector85.i, %temp.vect100.i
  %tmp6102.i = shl <16 x i64> %tmp101.i, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %tmp7103.i = trunc <16 x i64> %tmp6102.i to <16 x i32>
  %tmp9.i = mul i64 %30, %32
  %tmp10.i = shl i64 %tmp9.i, 1
  %temp104.i = insertelement <16 x i64> undef, i64 %tmp10.i, i32 0
  %vector105.i = shufflevector <16 x i64> %temp104.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp11106.i = add <16 x i64> %28, %vector105.i
  %tmp12107.i = trunc <16 x i64> %tmp11106.i to <16 x i32>
  %tmp15110.i = add <16 x i32> %vector109.i, %tmp12107.i
  %tmp17111.i = add <16 x i32> %tmp7103.i, %tmp12107.i
  %ipred.i.i = bitcast <16 x i1> %43 to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %tmp.i.i = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %tmp.i.i, 0
  br i1 %res.i.i, label %.preheader.i, label %._crit_edge5.i

.preheader.i:                                     ; preds = %SyncBB2172.i
  %negIncomingLoopMask112.i = xor <16 x i1> %43, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %"&(pSB[currWI].offset)1013.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset1014.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1013.i"
  %CastToValueType1015.i = bitcast i8* %"&pSB[currWI].offset1014.i" to float addrspace(3)**
  br label %44

; <label>:44                                      ; preds = %postload579.i, %.preheader.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask11206.i, %postload579.i ], [ %negIncomingLoopMask112.i, %.preheader.i ]
  %vectorPHI114.i = phi <16 x i1> [ %local_edge225.i, %postload579.i ], [ %43, %.preheader.i ]
  %vectorPHI115.i = phi <16 x float> [ %142, %postload579.i ], [ zeroinitializer, %.preheader.i ]
  %indvar.i = phi i32 [ %indvar.next.i, %postload579.i ], [ 0, %.preheader.i ]
  %extract137.i = extractelement <16 x i1> %vectorPHI114.i, i32 0
  %extract138.i = extractelement <16 x i1> %vectorPHI114.i, i32 1
  %extract139.i = extractelement <16 x i1> %vectorPHI114.i, i32 2
  %extract140.i = extractelement <16 x i1> %vectorPHI114.i, i32 3
  %extract141.i = extractelement <16 x i1> %vectorPHI114.i, i32 4
  %extract142.i = extractelement <16 x i1> %vectorPHI114.i, i32 5
  %extract143.i = extractelement <16 x i1> %vectorPHI114.i, i32 6
  %extract144.i = extractelement <16 x i1> %vectorPHI114.i, i32 7
  %extract145.i = extractelement <16 x i1> %vectorPHI114.i, i32 8
  %extract146.i = extractelement <16 x i1> %vectorPHI114.i, i32 9
  %extract147.i = extractelement <16 x i1> %vectorPHI114.i, i32 10
  %extract148.i = extractelement <16 x i1> %vectorPHI114.i, i32 11
  %extract149.i = extractelement <16 x i1> %vectorPHI114.i, i32 12
  %extract150.i = extractelement <16 x i1> %vectorPHI114.i, i32 13
  %extract151.i = extractelement <16 x i1> %vectorPHI114.i, i32 14
  %extract152.i = extractelement <16 x i1> %vectorPHI114.i, i32 15
  %temp116.i = insertelement <16 x i32> undef, i32 %indvar.i, i32 0
  %vector117.i = shufflevector <16 x i32> %temp116.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp8118.i = mul <16 x i32> %tmp7103.i, %vector117.i
  %i.03119.i = add <16 x i32> %tmp12107.i, %tmp8118.i
  %tmp16120.i = add <16 x i32> %tmp15110.i, %tmp8118.i
  %45 = extractelement <16 x i32> %i.03119.i, i32 1
  %46 = zext i32 %45 to i64
  %47 = getelementptr inbounds float addrspace(1)* %1, i64 %46
  %48 = extractelement <16 x i32> %i.03119.i, i32 2
  %49 = zext i32 %48 to i64
  %50 = getelementptr inbounds float addrspace(1)* %1, i64 %49
  %51 = extractelement <16 x i32> %i.03119.i, i32 3
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds float addrspace(1)* %1, i64 %52
  %54 = extractelement <16 x i32> %i.03119.i, i32 4
  %55 = zext i32 %54 to i64
  %56 = getelementptr inbounds float addrspace(1)* %1, i64 %55
  %57 = extractelement <16 x i32> %i.03119.i, i32 5
  %58 = zext i32 %57 to i64
  %59 = getelementptr inbounds float addrspace(1)* %1, i64 %58
  %60 = extractelement <16 x i32> %i.03119.i, i32 6
  %61 = zext i32 %60 to i64
  %62 = getelementptr inbounds float addrspace(1)* %1, i64 %61
  %63 = extractelement <16 x i32> %i.03119.i, i32 7
  %64 = zext i32 %63 to i64
  %65 = getelementptr inbounds float addrspace(1)* %1, i64 %64
  %66 = extractelement <16 x i32> %i.03119.i, i32 8
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds float addrspace(1)* %1, i64 %67
  %69 = extractelement <16 x i32> %i.03119.i, i32 9
  %70 = zext i32 %69 to i64
  %71 = getelementptr inbounds float addrspace(1)* %1, i64 %70
  %72 = extractelement <16 x i32> %i.03119.i, i32 10
  %73 = zext i32 %72 to i64
  %74 = getelementptr inbounds float addrspace(1)* %1, i64 %73
  %75 = extractelement <16 x i32> %i.03119.i, i32 11
  %76 = zext i32 %75 to i64
  %77 = getelementptr inbounds float addrspace(1)* %1, i64 %76
  %78 = extractelement <16 x i32> %i.03119.i, i32 12
  %79 = zext i32 %78 to i64
  %80 = getelementptr inbounds float addrspace(1)* %1, i64 %79
  %81 = extractelement <16 x i32> %i.03119.i, i32 13
  %82 = zext i32 %81 to i64
  %83 = getelementptr inbounds float addrspace(1)* %1, i64 %82
  %84 = extractelement <16 x i32> %i.03119.i, i32 14
  %85 = zext i32 %84 to i64
  %86 = getelementptr inbounds float addrspace(1)* %1, i64 %85
  %87 = extractelement <16 x i32> %i.03119.i, i32 15
  %88 = zext i32 %87 to i64
  %89 = getelementptr inbounds float addrspace(1)* %1, i64 %88
  br i1 %extract137.i, label %preload605.i, label %postload606.i

preload605.i:                                     ; preds = %44
  %90 = extractelement <16 x i32> %i.03119.i, i32 0
  %91 = zext i32 %90 to i64
  %92 = getelementptr inbounds float addrspace(1)* %1, i64 %91
  %masked_load.i = load float addrspace(1)* %92, align 4
  br label %postload606.i

postload606.i:                                    ; preds = %preload605.i, %44
  %phi607.i = phi float [ undef, %44 ], [ %masked_load.i, %preload605.i ]
  br i1 %extract138.i, label %preload614.i, label %postload615.i

preload614.i:                                     ; preds = %postload606.i
  %masked_load348.i = load float addrspace(1)* %47, align 4
  br label %postload615.i

postload615.i:                                    ; preds = %preload614.i, %postload606.i
  %phi616.i = phi float [ undef, %postload606.i ], [ %masked_load348.i, %preload614.i ]
  br i1 %extract139.i, label %preload620.i, label %postload621.i

preload620.i:                                     ; preds = %postload615.i
  %masked_load349.i = load float addrspace(1)* %50, align 4
  br label %postload621.i

postload621.i:                                    ; preds = %preload620.i, %postload615.i
  %phi622.i = phi float [ undef, %postload615.i ], [ %masked_load349.i, %preload620.i ]
  br i1 %extract140.i, label %preload626.i, label %postload627.i

preload626.i:                                     ; preds = %postload621.i
  %masked_load350.i = load float addrspace(1)* %53, align 4
  br label %postload627.i

postload627.i:                                    ; preds = %preload626.i, %postload621.i
  %phi628.i = phi float [ undef, %postload621.i ], [ %masked_load350.i, %preload626.i ]
  br i1 %extract141.i, label %preload632.i, label %postload633.i

preload632.i:                                     ; preds = %postload627.i
  %masked_load351.i = load float addrspace(1)* %56, align 4
  br label %postload633.i

postload633.i:                                    ; preds = %preload632.i, %postload627.i
  %phi634.i = phi float [ undef, %postload627.i ], [ %masked_load351.i, %preload632.i ]
  br i1 %extract142.i, label %preload638.i, label %postload639.i

preload638.i:                                     ; preds = %postload633.i
  %masked_load352.i = load float addrspace(1)* %59, align 4
  br label %postload639.i

postload639.i:                                    ; preds = %preload638.i, %postload633.i
  %phi640.i = phi float [ undef, %postload633.i ], [ %masked_load352.i, %preload638.i ]
  br i1 %extract143.i, label %preload644.i, label %postload645.i

preload644.i:                                     ; preds = %postload639.i
  %masked_load353.i = load float addrspace(1)* %62, align 4
  br label %postload645.i

postload645.i:                                    ; preds = %preload644.i, %postload639.i
  %phi646.i = phi float [ undef, %postload639.i ], [ %masked_load353.i, %preload644.i ]
  br i1 %extract144.i, label %preload650.i, label %postload651.i

preload650.i:                                     ; preds = %postload645.i
  %masked_load354.i = load float addrspace(1)* %65, align 4
  br label %postload651.i

postload651.i:                                    ; preds = %preload650.i, %postload645.i
  %phi652.i = phi float [ undef, %postload645.i ], [ %masked_load354.i, %preload650.i ]
  br i1 %extract145.i, label %preload656.i, label %postload657.i

preload656.i:                                     ; preds = %postload651.i
  %masked_load355.i = load float addrspace(1)* %68, align 4
  br label %postload657.i

postload657.i:                                    ; preds = %preload656.i, %postload651.i
  %phi658.i = phi float [ undef, %postload651.i ], [ %masked_load355.i, %preload656.i ]
  br i1 %extract146.i, label %preload662.i, label %postload663.i

preload662.i:                                     ; preds = %postload657.i
  %masked_load356.i = load float addrspace(1)* %71, align 4
  br label %postload663.i

postload663.i:                                    ; preds = %preload662.i, %postload657.i
  %phi664.i = phi float [ undef, %postload657.i ], [ %masked_load356.i, %preload662.i ]
  br i1 %extract147.i, label %preload668.i, label %postload669.i

preload668.i:                                     ; preds = %postload663.i
  %masked_load357.i = load float addrspace(1)* %74, align 4
  br label %postload669.i

postload669.i:                                    ; preds = %preload668.i, %postload663.i
  %phi670.i = phi float [ undef, %postload663.i ], [ %masked_load357.i, %preload668.i ]
  br i1 %extract148.i, label %preload674.i, label %postload675.i

preload674.i:                                     ; preds = %postload669.i
  %masked_load358.i = load float addrspace(1)* %77, align 4
  br label %postload675.i

postload675.i:                                    ; preds = %preload674.i, %postload669.i
  %phi676.i = phi float [ undef, %postload669.i ], [ %masked_load358.i, %preload674.i ]
  br i1 %extract149.i, label %preload680.i, label %postload681.i

preload680.i:                                     ; preds = %postload675.i
  %masked_load359.i = load float addrspace(1)* %80, align 4
  br label %postload681.i

postload681.i:                                    ; preds = %preload680.i, %postload675.i
  %phi682.i = phi float [ undef, %postload675.i ], [ %masked_load359.i, %preload680.i ]
  br i1 %extract150.i, label %preload686.i, label %postload687.i

preload686.i:                                     ; preds = %postload681.i
  %masked_load360.i = load float addrspace(1)* %83, align 4
  br label %postload687.i

postload687.i:                                    ; preds = %preload686.i, %postload681.i
  %phi688.i = phi float [ undef, %postload681.i ], [ %masked_load360.i, %preload686.i ]
  br i1 %extract151.i, label %preload692.i, label %postload693.i

preload692.i:                                     ; preds = %postload687.i
  %masked_load361.i = load float addrspace(1)* %86, align 4
  br label %postload693.i

postload693.i:                                    ; preds = %preload692.i, %postload687.i
  %phi694.i = phi float [ undef, %postload687.i ], [ %masked_load361.i, %preload692.i ]
  br i1 %extract152.i, label %preload698.i, label %postload699.i

preload698.i:                                     ; preds = %postload693.i
  %masked_load362.i = load float addrspace(1)* %89, align 4
  br label %postload699.i

postload699.i:                                    ; preds = %preload698.i, %postload693.i
  %phi700.i = phi float [ undef, %postload693.i ], [ %masked_load362.i, %preload698.i ]
  %temp.vect169.i = insertelement <16 x float> undef, float %phi607.i, i32 0
  %temp.vect170.i = insertelement <16 x float> %temp.vect169.i, float %phi616.i, i32 1
  %temp.vect171.i = insertelement <16 x float> %temp.vect170.i, float %phi622.i, i32 2
  %temp.vect172.i = insertelement <16 x float> %temp.vect171.i, float %phi628.i, i32 3
  %temp.vect173.i = insertelement <16 x float> %temp.vect172.i, float %phi634.i, i32 4
  %temp.vect174.i = insertelement <16 x float> %temp.vect173.i, float %phi640.i, i32 5
  %temp.vect175.i = insertelement <16 x float> %temp.vect174.i, float %phi646.i, i32 6
  %temp.vect176.i = insertelement <16 x float> %temp.vect175.i, float %phi652.i, i32 7
  %temp.vect177.i = insertelement <16 x float> %temp.vect176.i, float %phi658.i, i32 8
  %temp.vect178.i = insertelement <16 x float> %temp.vect177.i, float %phi664.i, i32 9
  %temp.vect179.i = insertelement <16 x float> %temp.vect178.i, float %phi670.i, i32 10
  %temp.vect180.i = insertelement <16 x float> %temp.vect179.i, float %phi676.i, i32 11
  %temp.vect181.i = insertelement <16 x float> %temp.vect180.i, float %phi682.i, i32 12
  %temp.vect182.i = insertelement <16 x float> %temp.vect181.i, float %phi688.i, i32 13
  %temp.vect183.i = insertelement <16 x float> %temp.vect182.i, float %phi694.i, i32 14
  %temp.vect184.i = insertelement <16 x float> %temp.vect183.i, float %phi700.i, i32 15
  %93 = extractelement <16 x i32> %tmp16120.i, i32 1
  %94 = zext i32 %93 to i64
  %95 = getelementptr inbounds float addrspace(1)* %1, i64 %94
  %96 = extractelement <16 x i32> %tmp16120.i, i32 2
  %97 = zext i32 %96 to i64
  %98 = getelementptr inbounds float addrspace(1)* %1, i64 %97
  %99 = extractelement <16 x i32> %tmp16120.i, i32 3
  %100 = zext i32 %99 to i64
  %101 = getelementptr inbounds float addrspace(1)* %1, i64 %100
  %102 = extractelement <16 x i32> %tmp16120.i, i32 4
  %103 = zext i32 %102 to i64
  %104 = getelementptr inbounds float addrspace(1)* %1, i64 %103
  %105 = extractelement <16 x i32> %tmp16120.i, i32 5
  %106 = zext i32 %105 to i64
  %107 = getelementptr inbounds float addrspace(1)* %1, i64 %106
  %108 = extractelement <16 x i32> %tmp16120.i, i32 6
  %109 = zext i32 %108 to i64
  %110 = getelementptr inbounds float addrspace(1)* %1, i64 %109
  %111 = extractelement <16 x i32> %tmp16120.i, i32 7
  %112 = zext i32 %111 to i64
  %113 = getelementptr inbounds float addrspace(1)* %1, i64 %112
  %114 = extractelement <16 x i32> %tmp16120.i, i32 8
  %115 = zext i32 %114 to i64
  %116 = getelementptr inbounds float addrspace(1)* %1, i64 %115
  %117 = extractelement <16 x i32> %tmp16120.i, i32 9
  %118 = zext i32 %117 to i64
  %119 = getelementptr inbounds float addrspace(1)* %1, i64 %118
  %120 = extractelement <16 x i32> %tmp16120.i, i32 10
  %121 = zext i32 %120 to i64
  %122 = getelementptr inbounds float addrspace(1)* %1, i64 %121
  %123 = extractelement <16 x i32> %tmp16120.i, i32 11
  %124 = zext i32 %123 to i64
  %125 = getelementptr inbounds float addrspace(1)* %1, i64 %124
  %126 = extractelement <16 x i32> %tmp16120.i, i32 12
  %127 = zext i32 %126 to i64
  %128 = getelementptr inbounds float addrspace(1)* %1, i64 %127
  %129 = extractelement <16 x i32> %tmp16120.i, i32 13
  %130 = zext i32 %129 to i64
  %131 = getelementptr inbounds float addrspace(1)* %1, i64 %130
  %132 = extractelement <16 x i32> %tmp16120.i, i32 14
  %133 = zext i32 %132 to i64
  %134 = getelementptr inbounds float addrspace(1)* %1, i64 %133
  %135 = extractelement <16 x i32> %tmp16120.i, i32 15
  %136 = zext i32 %135 to i64
  %137 = getelementptr inbounds float addrspace(1)* %1, i64 %136
  br i1 %extract137.i, label %preload608.i, label %postload609.i

preload608.i:                                     ; preds = %postload699.i
  %138 = extractelement <16 x i32> %tmp16120.i, i32 0
  %139 = zext i32 %138 to i64
  %140 = getelementptr inbounds float addrspace(1)* %1, i64 %139
  %masked_load363.i = load float addrspace(1)* %140, align 4
  br label %postload609.i

postload609.i:                                    ; preds = %preload608.i, %postload699.i
  %phi610.i = phi float [ undef, %postload699.i ], [ %masked_load363.i, %preload608.i ]
  br i1 %extract138.i, label %preload617.i, label %postload618.i

preload617.i:                                     ; preds = %postload609.i
  %masked_load364.i = load float addrspace(1)* %95, align 4
  br label %postload618.i

postload618.i:                                    ; preds = %preload617.i, %postload609.i
  %phi619.i = phi float [ undef, %postload609.i ], [ %masked_load364.i, %preload617.i ]
  br i1 %extract139.i, label %preload623.i, label %postload624.i

preload623.i:                                     ; preds = %postload618.i
  %masked_load365.i = load float addrspace(1)* %98, align 4
  br label %postload624.i

postload624.i:                                    ; preds = %preload623.i, %postload618.i
  %phi625.i = phi float [ undef, %postload618.i ], [ %masked_load365.i, %preload623.i ]
  br i1 %extract140.i, label %preload629.i, label %postload630.i

preload629.i:                                     ; preds = %postload624.i
  %masked_load366.i = load float addrspace(1)* %101, align 4
  br label %postload630.i

postload630.i:                                    ; preds = %preload629.i, %postload624.i
  %phi631.i = phi float [ undef, %postload624.i ], [ %masked_load366.i, %preload629.i ]
  br i1 %extract141.i, label %preload635.i, label %postload636.i

preload635.i:                                     ; preds = %postload630.i
  %masked_load367.i = load float addrspace(1)* %104, align 4
  br label %postload636.i

postload636.i:                                    ; preds = %preload635.i, %postload630.i
  %phi637.i = phi float [ undef, %postload630.i ], [ %masked_load367.i, %preload635.i ]
  br i1 %extract142.i, label %preload641.i, label %postload642.i

preload641.i:                                     ; preds = %postload636.i
  %masked_load368.i = load float addrspace(1)* %107, align 4
  br label %postload642.i

postload642.i:                                    ; preds = %preload641.i, %postload636.i
  %phi643.i = phi float [ undef, %postload636.i ], [ %masked_load368.i, %preload641.i ]
  br i1 %extract143.i, label %preload647.i, label %postload648.i

preload647.i:                                     ; preds = %postload642.i
  %masked_load369.i = load float addrspace(1)* %110, align 4
  br label %postload648.i

postload648.i:                                    ; preds = %preload647.i, %postload642.i
  %phi649.i = phi float [ undef, %postload642.i ], [ %masked_load369.i, %preload647.i ]
  br i1 %extract144.i, label %preload653.i, label %postload654.i

preload653.i:                                     ; preds = %postload648.i
  %masked_load370.i = load float addrspace(1)* %113, align 4
  br label %postload654.i

postload654.i:                                    ; preds = %preload653.i, %postload648.i
  %phi655.i = phi float [ undef, %postload648.i ], [ %masked_load370.i, %preload653.i ]
  br i1 %extract145.i, label %preload659.i, label %postload660.i

preload659.i:                                     ; preds = %postload654.i
  %masked_load371.i = load float addrspace(1)* %116, align 4
  br label %postload660.i

postload660.i:                                    ; preds = %preload659.i, %postload654.i
  %phi661.i = phi float [ undef, %postload654.i ], [ %masked_load371.i, %preload659.i ]
  br i1 %extract146.i, label %preload665.i, label %postload666.i

preload665.i:                                     ; preds = %postload660.i
  %masked_load372.i = load float addrspace(1)* %119, align 4
  br label %postload666.i

postload666.i:                                    ; preds = %preload665.i, %postload660.i
  %phi667.i = phi float [ undef, %postload660.i ], [ %masked_load372.i, %preload665.i ]
  br i1 %extract147.i, label %preload671.i, label %postload672.i

preload671.i:                                     ; preds = %postload666.i
  %masked_load373.i = load float addrspace(1)* %122, align 4
  br label %postload672.i

postload672.i:                                    ; preds = %preload671.i, %postload666.i
  %phi673.i = phi float [ undef, %postload666.i ], [ %masked_load373.i, %preload671.i ]
  br i1 %extract148.i, label %preload677.i, label %postload678.i

preload677.i:                                     ; preds = %postload672.i
  %masked_load374.i = load float addrspace(1)* %125, align 4
  br label %postload678.i

postload678.i:                                    ; preds = %preload677.i, %postload672.i
  %phi679.i = phi float [ undef, %postload672.i ], [ %masked_load374.i, %preload677.i ]
  br i1 %extract149.i, label %preload683.i, label %postload684.i

preload683.i:                                     ; preds = %postload678.i
  %masked_load375.i = load float addrspace(1)* %128, align 4
  br label %postload684.i

postload684.i:                                    ; preds = %preload683.i, %postload678.i
  %phi685.i = phi float [ undef, %postload678.i ], [ %masked_load375.i, %preload683.i ]
  br i1 %extract150.i, label %preload689.i, label %postload690.i

preload689.i:                                     ; preds = %postload684.i
  %masked_load376.i = load float addrspace(1)* %131, align 4
  br label %postload690.i

postload690.i:                                    ; preds = %preload689.i, %postload684.i
  %phi691.i = phi float [ undef, %postload684.i ], [ %masked_load376.i, %preload689.i ]
  br i1 %extract151.i, label %preload695.i, label %postload696.i

preload695.i:                                     ; preds = %postload690.i
  %masked_load377.i = load float addrspace(1)* %134, align 4
  br label %postload696.i

postload696.i:                                    ; preds = %preload695.i, %postload690.i
  %phi697.i = phi float [ undef, %postload690.i ], [ %masked_load377.i, %preload695.i ]
  br i1 %extract152.i, label %preload701.i, label %postload702.i

preload701.i:                                     ; preds = %postload696.i
  %masked_load378.i = load float addrspace(1)* %137, align 4
  br label %postload702.i

postload702.i:                                    ; preds = %preload701.i, %postload696.i
  %phi703.i = phi float [ undef, %postload696.i ], [ %masked_load378.i, %preload701.i ]
  %temp.vect185.i = insertelement <16 x float> undef, float %phi610.i, i32 0
  %temp.vect186.i = insertelement <16 x float> %temp.vect185.i, float %phi619.i, i32 1
  %temp.vect187.i = insertelement <16 x float> %temp.vect186.i, float %phi625.i, i32 2
  %temp.vect188.i = insertelement <16 x float> %temp.vect187.i, float %phi631.i, i32 3
  %temp.vect189.i = insertelement <16 x float> %temp.vect188.i, float %phi637.i, i32 4
  %temp.vect190.i = insertelement <16 x float> %temp.vect189.i, float %phi643.i, i32 5
  %temp.vect191.i = insertelement <16 x float> %temp.vect190.i, float %phi649.i, i32 6
  %temp.vect192.i = insertelement <16 x float> %temp.vect191.i, float %phi655.i, i32 7
  %temp.vect193.i = insertelement <16 x float> %temp.vect192.i, float %phi661.i, i32 8
  %temp.vect194.i = insertelement <16 x float> %temp.vect193.i, float %phi667.i, i32 9
  %temp.vect195.i = insertelement <16 x float> %temp.vect194.i, float %phi673.i, i32 10
  %temp.vect196.i = insertelement <16 x float> %temp.vect195.i, float %phi679.i, i32 11
  %temp.vect197.i = insertelement <16 x float> %temp.vect196.i, float %phi685.i, i32 12
  %temp.vect198.i = insertelement <16 x float> %temp.vect197.i, float %phi691.i, i32 13
  %temp.vect199.i = insertelement <16 x float> %temp.vect198.i, float %phi697.i, i32 14
  %temp.vect200.i = insertelement <16 x float> %temp.vect199.i, float %phi703.i, i32 15
  %141 = fadd <16 x float> %temp.vect184.i, %temp.vect200.i
  %142 = fadd <16 x float> %vectorPHI115.i, %141
  %loadedValue1016.i = load float addrspace(3)** %CastToValueType1015.i, align 8
  %exmask.i = extractelement <16 x i1> %vectorPHI114.i, i32 0
  br i1 %exmask.i, label %preload575.i, label %postload576.i

preload575.i:                                     ; preds = %postload702.i
  %exData.i = extractelement <16 x float> %142, i32 0
  store float %exData.i, float addrspace(3)* %loadedValue1016.i, align 4
  br label %postload576.i

postload576.i:                                    ; preds = %preload575.i, %postload702.i
  %exmask381.i = extractelement <16 x i1> %vectorPHI114.i, i32 1
  br i1 %exmask381.i, label %preload599.i, label %postload600.i

preload599.i:                                     ; preds = %postload576.i
  %143 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 1
  %exData382.i = extractelement <16 x float> %142, i32 1
  store float %exData382.i, float addrspace(3)* %143, align 4
  br label %postload600.i

postload600.i:                                    ; preds = %preload599.i, %postload576.i
  %exmask384.i = extractelement <16 x i1> %vectorPHI114.i, i32 2
  br i1 %exmask384.i, label %preload596.i, label %postload597.i

preload596.i:                                     ; preds = %postload600.i
  %144 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 2
  %exData385.i = extractelement <16 x float> %142, i32 2
  store float %exData385.i, float addrspace(3)* %144, align 4
  br label %postload597.i

postload597.i:                                    ; preds = %preload596.i, %postload600.i
  %exmask387.i = extractelement <16 x i1> %vectorPHI114.i, i32 3
  br i1 %exmask387.i, label %preload572.i, label %postload573.i

preload572.i:                                     ; preds = %postload597.i
  %145 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 3
  %exData388.i = extractelement <16 x float> %142, i32 3
  store float %exData388.i, float addrspace(3)* %145, align 4
  br label %postload573.i

postload573.i:                                    ; preds = %preload572.i, %postload597.i
  %exmask390.i = extractelement <16 x i1> %vectorPHI114.i, i32 4
  br i1 %exmask390.i, label %preload707.i, label %postload708.i

preload707.i:                                     ; preds = %postload573.i
  %146 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 4
  %exData391.i = extractelement <16 x float> %142, i32 4
  store float %exData391.i, float addrspace(3)* %146, align 4
  br label %postload708.i

postload708.i:                                    ; preds = %preload707.i, %postload573.i
  %exmask393.i = extractelement <16 x i1> %vectorPHI114.i, i32 5
  br i1 %exmask393.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload708.i
  %147 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 5
  %exData394.i = extractelement <16 x float> %142, i32 5
  store float %exData394.i, float addrspace(3)* %147, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload708.i
  %exmask396.i = extractelement <16 x i1> %vectorPHI114.i, i32 6
  br i1 %exmask396.i, label %preload602.i, label %postload603.i

preload602.i:                                     ; preds = %postload.i
  %148 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 6
  %exData397.i = extractelement <16 x float> %142, i32 6
  store float %exData397.i, float addrspace(3)* %148, align 4
  br label %postload603.i

postload603.i:                                    ; preds = %preload602.i, %postload.i
  %exmask399.i = extractelement <16 x i1> %vectorPHI114.i, i32 7
  br i1 %exmask399.i, label %preload569.i, label %postload570.i

preload569.i:                                     ; preds = %postload603.i
  %149 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 7
  %exData400.i = extractelement <16 x float> %142, i32 7
  store float %exData400.i, float addrspace(3)* %149, align 4
  br label %postload570.i

postload570.i:                                    ; preds = %preload569.i, %postload603.i
  %exmask402.i = extractelement <16 x i1> %vectorPHI114.i, i32 8
  br i1 %exmask402.i, label %preload593.i, label %postload594.i

preload593.i:                                     ; preds = %postload570.i
  %150 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 8
  %exData403.i = extractelement <16 x float> %142, i32 8
  store float %exData403.i, float addrspace(3)* %150, align 4
  br label %postload594.i

postload594.i:                                    ; preds = %preload593.i, %postload570.i
  %exmask405.i = extractelement <16 x i1> %vectorPHI114.i, i32 9
  br i1 %exmask405.i, label %preload704.i, label %postload705.i

preload704.i:                                     ; preds = %postload594.i
  %151 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 9
  %exData406.i = extractelement <16 x float> %142, i32 9
  store float %exData406.i, float addrspace(3)* %151, align 4
  br label %postload705.i

postload705.i:                                    ; preds = %preload704.i, %postload594.i
  %exmask408.i = extractelement <16 x i1> %vectorPHI114.i, i32 10
  br i1 %exmask408.i, label %preload584.i, label %postload585.i

preload584.i:                                     ; preds = %postload705.i
  %152 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 10
  %exData409.i = extractelement <16 x float> %142, i32 10
  store float %exData409.i, float addrspace(3)* %152, align 4
  br label %postload585.i

postload585.i:                                    ; preds = %preload584.i, %postload705.i
  %exmask411.i = extractelement <16 x i1> %vectorPHI114.i, i32 11
  br i1 %exmask411.i, label %preload710.i, label %postload711.i

preload710.i:                                     ; preds = %postload585.i
  %153 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 11
  %exData412.i = extractelement <16 x float> %142, i32 11
  store float %exData412.i, float addrspace(3)* %153, align 4
  br label %postload711.i

postload711.i:                                    ; preds = %preload710.i, %postload585.i
  %exmask414.i = extractelement <16 x i1> %vectorPHI114.i, i32 12
  br i1 %exmask414.i, label %preload590.i, label %postload591.i

preload590.i:                                     ; preds = %postload711.i
  %154 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 12
  %exData415.i = extractelement <16 x float> %142, i32 12
  store float %exData415.i, float addrspace(3)* %154, align 4
  br label %postload591.i

postload591.i:                                    ; preds = %preload590.i, %postload711.i
  %exmask417.i = extractelement <16 x i1> %vectorPHI114.i, i32 13
  br i1 %exmask417.i, label %preload581.i, label %postload582.i

preload581.i:                                     ; preds = %postload591.i
  %155 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 13
  %exData418.i = extractelement <16 x float> %142, i32 13
  store float %exData418.i, float addrspace(3)* %155, align 4
  br label %postload582.i

postload582.i:                                    ; preds = %preload581.i, %postload591.i
  %exmask420.i = extractelement <16 x i1> %vectorPHI114.i, i32 14
  br i1 %exmask420.i, label %preload587.i, label %postload588.i

preload587.i:                                     ; preds = %postload582.i
  %156 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 14
  %exData421.i = extractelement <16 x float> %142, i32 14
  store float %exData421.i, float addrspace(3)* %156, align 4
  br label %postload588.i

postload588.i:                                    ; preds = %preload587.i, %postload582.i
  %exmask423.i = extractelement <16 x i1> %vectorPHI114.i, i32 15
  br i1 %exmask423.i, label %preload578.i, label %postload579.i

preload578.i:                                     ; preds = %postload588.i
  %157 = getelementptr float addrspace(3)* %loadedValue1016.i, i64 15
  %exData424.i = extractelement <16 x float> %142, i32 15
  store float %exData424.i, float addrspace(3)* %157, align 4
  br label %postload579.i

postload579.i:                                    ; preds = %preload578.i, %postload588.i
  %tmp18202.i = add <16 x i32> %tmp17111.i, %tmp8118.i
  %158 = icmp ult <16 x i32> %tmp18202.i, %vector82.i
  %indvar.next.i = add i32 %indvar.i, 1
  %notCond203.i = xor <16 x i1> %158, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr204.i = and <16 x i1> %vectorPHI114.i, %notCond203.i
  %loop_mask11206.i = or <16 x i1> %vectorPHI.i, %who_left_tr204.i
  %curr_loop_mask207.i = or <16 x i1> %loop_mask11206.i, %who_left_tr204.i
  %ipred.i1.i = bitcast <16 x i1> %curr_loop_mask207.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %tmp.i3.i = and i32 %val.i2.i, 1
  %res.i4.i = icmp eq i32 %tmp.i3.i, 0
  %local_edge225.i = and <16 x i1> %vectorPHI114.i, %158
  br i1 %res.i4.i, label %44, label %._crit_edge5.i

._crit_edge5.i:                                   ; preds = %postload579.i, %SyncBB2172.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %._crit_edge5.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 2752
  br label %SyncBB2172.i

elseBB.i:                                         ; preds = %._crit_edge5.i
  %s.01.i = lshr i32 %41, 1
  %Mneg13.i = icmp ne i32 %s.01.i, 0
  %temp247.i = insertelement <16 x i1> undef, i1 %Mneg13.i, i32 0
  %vector248.i = shufflevector <16 x i1> %temp247.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %negIncomingLoopMask40.i = xor i1 %Mneg13.i, true
  %temp244.i = insertelement <16 x i1> undef, i1 %negIncomingLoopMask40.i, i32 0
  %vector245.i = shufflevector <16 x i1> %temp244.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB2184.i, %thenBB2177.i, %thenBB2192.i, %elseBB.i
  %currBarrier.4.i = phi i32 [ 2, %elseBB.i ], [ %currBarrier.3.i, %thenBB2177.i ], [ %currBarrier.2.i, %thenBB2192.i ], [ %currBarrier.3.i, %thenBB2184.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride2183.i", %thenBB2177.i ], [ %"loadedCurrSB+Stride2198.i", %thenBB2192.i ], [ %"loadedCurrSB+Stride2190.i", %thenBB2184.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++2181.i", %thenBB2177.i ], [ %"CurrWI++2196.i", %thenBB2192.i ], [ %"CurrWI++2188.i", %thenBB2184.i ]
  br i1 %Mneg13.i, label %._crit_edge.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %postload798.i, %SyncBB.i
  %currBarrier.2.i = phi i32 [ %currBarrier.1.i, %postload798.i ], [ %currBarrier.4.i, %SyncBB.i ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..3.i, %postload798.i ], [ %CurrSBIndex..1.i, %SyncBB.i ]
  %CurrWI..4.i = phi i64 [ %CurrWI..3.i, %postload798.i ], [ %CurrWI..1.i, %SyncBB.i ]
  %vectorPHI243.i = phi <16 x i1> [ %loop_mask24326.i, %postload798.i ], [ %vector245.i, %SyncBB.i ]
  %vectorPHI246.i = phi <16 x i1> [ %local_edge29330.i, %postload798.i ], [ %vector248.i, %SyncBB.i ]
  %s.02.i = phi i32 [ %s.0.i, %postload798.i ], [ %s.01.i, %SyncBB.i ]
  %"&(pSB[currWI].offset)1046.i" = add nuw i64 %CurrSBIndex..4.i, 164
  %"&pSB[currWI].offset1047.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1046.i"
  %CastToValueType1048.i = bitcast i8* %"&pSB[currWI].offset1047.i" to i32*
  store i32 %s.02.i, i32* %CastToValueType1048.i, align 4
  %"&(pSB[currWI].offset)1027.i" = add nuw i64 %CurrSBIndex..4.i, 160
  %"&pSB[currWI].offset1028.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1027.i"
  %CastToValueType1029.i = bitcast i8* %"&pSB[currWI].offset1028.i" to <16 x i1>*
  store <16 x i1> %vectorPHI246.i, <16 x i1>* %CastToValueType1029.i, align 16
  %"&(pSB[currWI].offset)1018.i" = add nuw i64 %CurrSBIndex..4.i, 144
  %"&pSB[currWI].offset1019.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1018.i"
  %CastToValueType1020.i = bitcast i8* %"&pSB[currWI].offset1019.i" to <16 x i1>*
  store <16 x i1> %vectorPHI243.i, <16 x i1>* %CastToValueType1020.i, align 16
  %temp249.i = insertelement <16 x i32> undef, i32 %s.02.i, i32 0
  %vector250.i = shufflevector <16 x i32> %temp249.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)994.i" = add nuw i64 %CurrSBIndex..4.i, 64
  %"&pSB[currWI].offset995.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)994.i"
  %CastToValueType996.i = bitcast i8* %"&pSB[currWI].offset995.i" to <16 x i32>*
  %loadedValue997.i = load <16 x i32>* %CastToValueType996.i, align 64
  %159 = icmp ult <16 x i32> %loadedValue997.i, %vector250.i
  %bb.nph_to_17253.i = and <16 x i1> %vectorPHI246.i, %159
  %"&(pSB[currWI].offset)1055.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1056.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1055.i"
  %CastToValueType1057.i = bitcast i8* %"&pSB[currWI].offset1056.i" to <16 x i1>*
  store <16 x i1> %bb.nph_to_17253.i, <16 x i1>* %CastToValueType1057.i, align 16
  %extract271.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 1
  %"&(pSB[currWI].offset)1219.i" = add nuw i64 %CurrSBIndex..4.i, 178
  %"&pSB[currWI].offset1220.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1219.i"
  %CastToValueType1221.i = bitcast i8* %"&pSB[currWI].offset1220.i" to i1*
  store i1 %extract271.i, i1* %CastToValueType1221.i, align 1
  %extract272.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 2
  %"&(pSB[currWI].offset)1228.i" = add nuw i64 %CurrSBIndex..4.i, 179
  %"&pSB[currWI].offset1229.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1228.i"
  %CastToValueType1230.i = bitcast i8* %"&pSB[currWI].offset1229.i" to i1*
  store i1 %extract272.i, i1* %CastToValueType1230.i, align 1
  %extract273.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 3
  %"&(pSB[currWI].offset)1237.i" = add nuw i64 %CurrSBIndex..4.i, 180
  %"&pSB[currWI].offset1238.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1237.i"
  %CastToValueType1239.i = bitcast i8* %"&pSB[currWI].offset1238.i" to i1*
  store i1 %extract273.i, i1* %CastToValueType1239.i, align 1
  %extract274.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 4
  %"&(pSB[currWI].offset)1246.i" = add nuw i64 %CurrSBIndex..4.i, 181
  %"&pSB[currWI].offset1247.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1246.i"
  %CastToValueType1248.i = bitcast i8* %"&pSB[currWI].offset1247.i" to i1*
  store i1 %extract274.i, i1* %CastToValueType1248.i, align 1
  %extract275.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 5
  %"&(pSB[currWI].offset)1255.i" = add nuw i64 %CurrSBIndex..4.i, 182
  %"&pSB[currWI].offset1256.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1255.i"
  %CastToValueType1257.i = bitcast i8* %"&pSB[currWI].offset1256.i" to i1*
  store i1 %extract275.i, i1* %CastToValueType1257.i, align 1
  %extract276.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 6
  %"&(pSB[currWI].offset)1264.i" = add nuw i64 %CurrSBIndex..4.i, 183
  %"&pSB[currWI].offset1265.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1264.i"
  %CastToValueType1266.i = bitcast i8* %"&pSB[currWI].offset1265.i" to i1*
  store i1 %extract276.i, i1* %CastToValueType1266.i, align 1
  %extract277.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 7
  %"&(pSB[currWI].offset)1273.i" = add nuw i64 %CurrSBIndex..4.i, 184
  %"&pSB[currWI].offset1274.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1273.i"
  %CastToValueType1275.i = bitcast i8* %"&pSB[currWI].offset1274.i" to i1*
  store i1 %extract277.i, i1* %CastToValueType1275.i, align 1
  %extract278.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 8
  %"&(pSB[currWI].offset)1282.i" = add nuw i64 %CurrSBIndex..4.i, 185
  %"&pSB[currWI].offset1283.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1282.i"
  %CastToValueType1284.i = bitcast i8* %"&pSB[currWI].offset1283.i" to i1*
  store i1 %extract278.i, i1* %CastToValueType1284.i, align 1
  %extract279.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 9
  %"&(pSB[currWI].offset)1291.i" = add nuw i64 %CurrSBIndex..4.i, 186
  %"&pSB[currWI].offset1292.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1291.i"
  %CastToValueType1293.i = bitcast i8* %"&pSB[currWI].offset1292.i" to i1*
  store i1 %extract279.i, i1* %CastToValueType1293.i, align 1
  %extract280.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 10
  %"&(pSB[currWI].offset)1300.i" = add nuw i64 %CurrSBIndex..4.i, 187
  %"&pSB[currWI].offset1301.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1300.i"
  %CastToValueType1302.i = bitcast i8* %"&pSB[currWI].offset1301.i" to i1*
  store i1 %extract280.i, i1* %CastToValueType1302.i, align 1
  %extract281.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 11
  %"&(pSB[currWI].offset)1309.i" = add nuw i64 %CurrSBIndex..4.i, 188
  %"&pSB[currWI].offset1310.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1309.i"
  %CastToValueType1311.i = bitcast i8* %"&pSB[currWI].offset1310.i" to i1*
  store i1 %extract281.i, i1* %CastToValueType1311.i, align 1
  %extract282.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 12
  %"&(pSB[currWI].offset)1318.i" = add nuw i64 %CurrSBIndex..4.i, 189
  %"&pSB[currWI].offset1319.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1318.i"
  %CastToValueType1320.i = bitcast i8* %"&pSB[currWI].offset1319.i" to i1*
  store i1 %extract282.i, i1* %CastToValueType1320.i, align 1
  %extract283.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 13
  %"&(pSB[currWI].offset)1327.i" = add nuw i64 %CurrSBIndex..4.i, 190
  %"&pSB[currWI].offset1328.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1327.i"
  %CastToValueType1329.i = bitcast i8* %"&pSB[currWI].offset1328.i" to i1*
  store i1 %extract283.i, i1* %CastToValueType1329.i, align 1
  %extract284.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 14
  %"&(pSB[currWI].offset)1336.i" = add nuw i64 %CurrSBIndex..4.i, 191
  %"&pSB[currWI].offset1337.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1336.i"
  %CastToValueType1338.i = bitcast i8* %"&pSB[currWI].offset1337.i" to i1*
  store i1 %extract284.i, i1* %CastToValueType1338.i, align 1
  %extract285.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 15
  %"&(pSB[currWI].offset)1345.i" = add nuw i64 %CurrSBIndex..4.i, 192
  %"&pSB[currWI].offset1346.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1345.i"
  %CastToValueType1347.i = bitcast i8* %"&pSB[currWI].offset1346.i" to i1*
  store i1 %extract285.i, i1* %CastToValueType1347.i, align 1
  %extract270.i = extractelement <16 x i1> %bb.nph_to_17253.i, i32 0
  %"&(pSB[currWI].offset)989.i" = add nuw i64 %CurrSBIndex..4.i, 64
  %"&pSB[currWI].offset990.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)989.i"
  %CastToValueType991.i = bitcast i8* %"&pSB[currWI].offset990.i" to <16 x i32>*
  %loadedValue992.i = load <16 x i32>* %CastToValueType991.i, align 64
  %160 = add <16 x i32> %vector250.i, %loadedValue992.i
  %"&(pSB[currWI].offset)1354.i" = add nuw i64 %CurrSBIndex..4.i, 256
  %"&pSB[currWI].offset1355.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1354.i"
  %CastToValueType1356.i = bitcast i8* %"&pSB[currWI].offset1355.i" to <16 x i32>*
  store <16 x i32> %160, <16 x i32>* %CastToValueType1356.i, align 64
  %161 = extractelement <16 x i32> %160, i32 1
  %162 = zext i32 %161 to i64
  %163 = getelementptr inbounds float addrspace(3)* %7, i64 %162
  %"&(pSB[currWI].offset)1363.i" = add nuw i64 %CurrSBIndex..4.i, 320
  %"&pSB[currWI].offset1364.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1363.i"
  %CastToValueType1365.i = bitcast i8* %"&pSB[currWI].offset1364.i" to float addrspace(3)**
  store float addrspace(3)* %163, float addrspace(3)** %CastToValueType1365.i, align 8
  %164 = extractelement <16 x i32> %160, i32 2
  %165 = zext i32 %164 to i64
  %166 = getelementptr inbounds float addrspace(3)* %7, i64 %165
  %"&(pSB[currWI].offset)1372.i" = add nuw i64 %CurrSBIndex..4.i, 328
  %"&pSB[currWI].offset1373.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1372.i"
  %CastToValueType1374.i = bitcast i8* %"&pSB[currWI].offset1373.i" to float addrspace(3)**
  store float addrspace(3)* %166, float addrspace(3)** %CastToValueType1374.i, align 8
  %167 = extractelement <16 x i32> %160, i32 3
  %168 = zext i32 %167 to i64
  %169 = getelementptr inbounds float addrspace(3)* %7, i64 %168
  %"&(pSB[currWI].offset)1381.i" = add nuw i64 %CurrSBIndex..4.i, 336
  %"&pSB[currWI].offset1382.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1381.i"
  %CastToValueType1383.i = bitcast i8* %"&pSB[currWI].offset1382.i" to float addrspace(3)**
  store float addrspace(3)* %169, float addrspace(3)** %CastToValueType1383.i, align 8
  %170 = extractelement <16 x i32> %160, i32 4
  %171 = zext i32 %170 to i64
  %172 = getelementptr inbounds float addrspace(3)* %7, i64 %171
  %"&(pSB[currWI].offset)1390.i" = add nuw i64 %CurrSBIndex..4.i, 344
  %"&pSB[currWI].offset1391.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1390.i"
  %CastToValueType1392.i = bitcast i8* %"&pSB[currWI].offset1391.i" to float addrspace(3)**
  store float addrspace(3)* %172, float addrspace(3)** %CastToValueType1392.i, align 8
  %173 = extractelement <16 x i32> %160, i32 5
  %174 = zext i32 %173 to i64
  %175 = getelementptr inbounds float addrspace(3)* %7, i64 %174
  %"&(pSB[currWI].offset)1399.i" = add nuw i64 %CurrSBIndex..4.i, 352
  %"&pSB[currWI].offset1400.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1399.i"
  %CastToValueType1401.i = bitcast i8* %"&pSB[currWI].offset1400.i" to float addrspace(3)**
  store float addrspace(3)* %175, float addrspace(3)** %CastToValueType1401.i, align 8
  %176 = extractelement <16 x i32> %160, i32 6
  %177 = zext i32 %176 to i64
  %178 = getelementptr inbounds float addrspace(3)* %7, i64 %177
  %"&(pSB[currWI].offset)1408.i" = add nuw i64 %CurrSBIndex..4.i, 360
  %"&pSB[currWI].offset1409.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1408.i"
  %CastToValueType1410.i = bitcast i8* %"&pSB[currWI].offset1409.i" to float addrspace(3)**
  store float addrspace(3)* %178, float addrspace(3)** %CastToValueType1410.i, align 8
  %179 = extractelement <16 x i32> %160, i32 7
  %180 = zext i32 %179 to i64
  %181 = getelementptr inbounds float addrspace(3)* %7, i64 %180
  %"&(pSB[currWI].offset)1417.i" = add nuw i64 %CurrSBIndex..4.i, 368
  %"&pSB[currWI].offset1418.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1417.i"
  %CastToValueType1419.i = bitcast i8* %"&pSB[currWI].offset1418.i" to float addrspace(3)**
  store float addrspace(3)* %181, float addrspace(3)** %CastToValueType1419.i, align 8
  %182 = extractelement <16 x i32> %160, i32 8
  %183 = zext i32 %182 to i64
  %184 = getelementptr inbounds float addrspace(3)* %7, i64 %183
  %"&(pSB[currWI].offset)1426.i" = add nuw i64 %CurrSBIndex..4.i, 376
  %"&pSB[currWI].offset1427.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1426.i"
  %CastToValueType1428.i = bitcast i8* %"&pSB[currWI].offset1427.i" to float addrspace(3)**
  store float addrspace(3)* %184, float addrspace(3)** %CastToValueType1428.i, align 8
  %185 = extractelement <16 x i32> %160, i32 9
  %186 = zext i32 %185 to i64
  %187 = getelementptr inbounds float addrspace(3)* %7, i64 %186
  %"&(pSB[currWI].offset)1435.i" = add nuw i64 %CurrSBIndex..4.i, 384
  %"&pSB[currWI].offset1436.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1435.i"
  %CastToValueType1437.i = bitcast i8* %"&pSB[currWI].offset1436.i" to float addrspace(3)**
  store float addrspace(3)* %187, float addrspace(3)** %CastToValueType1437.i, align 8
  %188 = extractelement <16 x i32> %160, i32 10
  %189 = zext i32 %188 to i64
  %190 = getelementptr inbounds float addrspace(3)* %7, i64 %189
  %"&(pSB[currWI].offset)1444.i" = add nuw i64 %CurrSBIndex..4.i, 392
  %"&pSB[currWI].offset1445.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1444.i"
  %CastToValueType1446.i = bitcast i8* %"&pSB[currWI].offset1445.i" to float addrspace(3)**
  store float addrspace(3)* %190, float addrspace(3)** %CastToValueType1446.i, align 8
  %191 = extractelement <16 x i32> %160, i32 11
  %192 = zext i32 %191 to i64
  %193 = getelementptr inbounds float addrspace(3)* %7, i64 %192
  %"&(pSB[currWI].offset)1453.i" = add nuw i64 %CurrSBIndex..4.i, 400
  %"&pSB[currWI].offset1454.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1453.i"
  %CastToValueType1455.i = bitcast i8* %"&pSB[currWI].offset1454.i" to float addrspace(3)**
  store float addrspace(3)* %193, float addrspace(3)** %CastToValueType1455.i, align 8
  %194 = extractelement <16 x i32> %160, i32 12
  %195 = zext i32 %194 to i64
  %196 = getelementptr inbounds float addrspace(3)* %7, i64 %195
  %"&(pSB[currWI].offset)1462.i" = add nuw i64 %CurrSBIndex..4.i, 408
  %"&pSB[currWI].offset1463.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1462.i"
  %CastToValueType1464.i = bitcast i8* %"&pSB[currWI].offset1463.i" to float addrspace(3)**
  store float addrspace(3)* %196, float addrspace(3)** %CastToValueType1464.i, align 8
  %197 = extractelement <16 x i32> %160, i32 13
  %198 = zext i32 %197 to i64
  %199 = getelementptr inbounds float addrspace(3)* %7, i64 %198
  %"&(pSB[currWI].offset)1471.i" = add nuw i64 %CurrSBIndex..4.i, 416
  %"&pSB[currWI].offset1472.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1471.i"
  %CastToValueType1473.i = bitcast i8* %"&pSB[currWI].offset1472.i" to float addrspace(3)**
  store float addrspace(3)* %199, float addrspace(3)** %CastToValueType1473.i, align 8
  %200 = extractelement <16 x i32> %160, i32 14
  %201 = zext i32 %200 to i64
  %202 = getelementptr inbounds float addrspace(3)* %7, i64 %201
  %"&(pSB[currWI].offset)1480.i" = add nuw i64 %CurrSBIndex..4.i, 424
  %"&pSB[currWI].offset1481.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1480.i"
  %CastToValueType1482.i = bitcast i8* %"&pSB[currWI].offset1481.i" to float addrspace(3)**
  store float addrspace(3)* %202, float addrspace(3)** %CastToValueType1482.i, align 8
  %203 = extractelement <16 x i32> %160, i32 15
  %204 = zext i32 %203 to i64
  %205 = getelementptr inbounds float addrspace(3)* %7, i64 %204
  %"&(pSB[currWI].offset)1489.i" = add nuw i64 %CurrSBIndex..4.i, 432
  %"&pSB[currWI].offset1490.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1489.i"
  %CastToValueType1491.i = bitcast i8* %"&pSB[currWI].offset1490.i" to float addrspace(3)**
  store float addrspace(3)* %205, float addrspace(3)** %CastToValueType1491.i, align 8
  br i1 %extract270.i, label %preload749.i, label %postload750.i

preload749.i:                                     ; preds = %bb.nph.i
  %"&(pSB[currWI].offset)1358.i" = add nuw i64 %CurrSBIndex..4.i, 256
  %"&pSB[currWI].offset1359.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1358.i"
  %CastToValueType1360.i = bitcast i8* %"&pSB[currWI].offset1359.i" to <16 x i32>*
  %loadedValue1361.i = load <16 x i32>* %CastToValueType1360.i, align 64
  %206 = extractelement <16 x i32> %loadedValue1361.i, i32 0
  %207 = zext i32 %206 to i64
  %208 = getelementptr inbounds float addrspace(3)* %7, i64 %207
  %masked_load425.i = load float addrspace(3)* %208, align 4
  %"&(pSB[currWI].offset)1498.i" = add nuw i64 %CurrSBIndex..4.i, 440
  %"&pSB[currWI].offset1499.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1498.i"
  %CastToValueType1500.i = bitcast i8* %"&pSB[currWI].offset1499.i" to float*
  store float %masked_load425.i, float* %CastToValueType1500.i, align 4
  br label %postload750.i

postload750.i:                                    ; preds = %preload749.i, %bb.nph.i
  %phi751.i = phi float [ undef, %bb.nph.i ], [ %masked_load425.i, %preload749.i ]
  %"&(pSB[currWI].offset)1502.i" = add nuw i64 %CurrSBIndex..4.i, 444
  %"&pSB[currWI].offset1503.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1502.i"
  %CastToValueType1504.i = bitcast i8* %"&pSB[currWI].offset1503.i" to float*
  store float %phi751.i, float* %CastToValueType1504.i, align 4
  %"&(pSB[currWI].offset)1223.i" = add nuw i64 %CurrSBIndex..4.i, 178
  %"&pSB[currWI].offset1224.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1223.i"
  %CastToValueType1225.i = bitcast i8* %"&pSB[currWI].offset1224.i" to i1*
  %loadedValue1226.i = load i1* %CastToValueType1225.i, align 1
  br i1 %loadedValue1226.i, label %preload752.i, label %postload753.i

preload752.i:                                     ; preds = %postload750.i
  %"&(pSB[currWI].offset)1367.i" = add nuw i64 %CurrSBIndex..4.i, 320
  %"&pSB[currWI].offset1368.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1367.i"
  %CastToValueType1369.i = bitcast i8* %"&pSB[currWI].offset1368.i" to float addrspace(3)**
  %loadedValue1370.i = load float addrspace(3)** %CastToValueType1369.i, align 8
  %masked_load426.i = load float addrspace(3)* %loadedValue1370.i, align 4
  %"&(pSB[currWI].offset)1511.i" = add nuw i64 %CurrSBIndex..4.i, 448
  %"&pSB[currWI].offset1512.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1511.i"
  %CastToValueType1513.i = bitcast i8* %"&pSB[currWI].offset1512.i" to float*
  store float %masked_load426.i, float* %CastToValueType1513.i, align 4
  br label %postload753.i

postload753.i:                                    ; preds = %preload752.i, %postload750.i
  %phi754.i = phi float [ undef, %postload750.i ], [ %masked_load426.i, %preload752.i ]
  %"&(pSB[currWI].offset)1515.i" = add nuw i64 %CurrSBIndex..4.i, 452
  %"&pSB[currWI].offset1516.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1515.i"
  %CastToValueType1517.i = bitcast i8* %"&pSB[currWI].offset1516.i" to float*
  store float %phi754.i, float* %CastToValueType1517.i, align 4
  %"&(pSB[currWI].offset)1232.i" = add nuw i64 %CurrSBIndex..4.i, 179
  %"&pSB[currWI].offset1233.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1232.i"
  %CastToValueType1234.i = bitcast i8* %"&pSB[currWI].offset1233.i" to i1*
  %loadedValue1235.i = load i1* %CastToValueType1234.i, align 1
  br i1 %loadedValue1235.i, label %preload755.i, label %postload756.i

preload755.i:                                     ; preds = %postload753.i
  %"&(pSB[currWI].offset)1376.i" = add nuw i64 %CurrSBIndex..4.i, 328
  %"&pSB[currWI].offset1377.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1376.i"
  %CastToValueType1378.i = bitcast i8* %"&pSB[currWI].offset1377.i" to float addrspace(3)**
  %loadedValue1379.i = load float addrspace(3)** %CastToValueType1378.i, align 8
  %masked_load427.i = load float addrspace(3)* %loadedValue1379.i, align 4
  %"&(pSB[currWI].offset)1524.i" = add nuw i64 %CurrSBIndex..4.i, 456
  %"&pSB[currWI].offset1525.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1524.i"
  %CastToValueType1526.i = bitcast i8* %"&pSB[currWI].offset1525.i" to float*
  store float %masked_load427.i, float* %CastToValueType1526.i, align 4
  br label %postload756.i

postload756.i:                                    ; preds = %preload755.i, %postload753.i
  %phi757.i = phi float [ undef, %postload753.i ], [ %masked_load427.i, %preload755.i ]
  %"&(pSB[currWI].offset)1528.i" = add nuw i64 %CurrSBIndex..4.i, 460
  %"&pSB[currWI].offset1529.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1528.i"
  %CastToValueType1530.i = bitcast i8* %"&pSB[currWI].offset1529.i" to float*
  store float %phi757.i, float* %CastToValueType1530.i, align 4
  %"&(pSB[currWI].offset)1241.i" = add nuw i64 %CurrSBIndex..4.i, 180
  %"&pSB[currWI].offset1242.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1241.i"
  %CastToValueType1243.i = bitcast i8* %"&pSB[currWI].offset1242.i" to i1*
  %loadedValue1244.i = load i1* %CastToValueType1243.i, align 1
  br i1 %loadedValue1244.i, label %preload758.i, label %postload759.i

preload758.i:                                     ; preds = %postload756.i
  %"&(pSB[currWI].offset)1385.i" = add nuw i64 %CurrSBIndex..4.i, 336
  %"&pSB[currWI].offset1386.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1385.i"
  %CastToValueType1387.i = bitcast i8* %"&pSB[currWI].offset1386.i" to float addrspace(3)**
  %loadedValue1388.i = load float addrspace(3)** %CastToValueType1387.i, align 8
  %masked_load428.i = load float addrspace(3)* %loadedValue1388.i, align 4
  %"&(pSB[currWI].offset)1537.i" = add nuw i64 %CurrSBIndex..4.i, 464
  %"&pSB[currWI].offset1538.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1537.i"
  %CastToValueType1539.i = bitcast i8* %"&pSB[currWI].offset1538.i" to float*
  store float %masked_load428.i, float* %CastToValueType1539.i, align 4
  br label %postload759.i

postload759.i:                                    ; preds = %preload758.i, %postload756.i
  %phi760.i = phi float [ undef, %postload756.i ], [ %masked_load428.i, %preload758.i ]
  %"&(pSB[currWI].offset)1541.i" = add nuw i64 %CurrSBIndex..4.i, 468
  %"&pSB[currWI].offset1542.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1541.i"
  %CastToValueType1543.i = bitcast i8* %"&pSB[currWI].offset1542.i" to float*
  store float %phi760.i, float* %CastToValueType1543.i, align 4
  %"&(pSB[currWI].offset)1250.i" = add nuw i64 %CurrSBIndex..4.i, 181
  %"&pSB[currWI].offset1251.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1250.i"
  %CastToValueType1252.i = bitcast i8* %"&pSB[currWI].offset1251.i" to i1*
  %loadedValue1253.i = load i1* %CastToValueType1252.i, align 1
  br i1 %loadedValue1253.i, label %preload761.i, label %postload762.i

preload761.i:                                     ; preds = %postload759.i
  %"&(pSB[currWI].offset)1394.i" = add nuw i64 %CurrSBIndex..4.i, 344
  %"&pSB[currWI].offset1395.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1394.i"
  %CastToValueType1396.i = bitcast i8* %"&pSB[currWI].offset1395.i" to float addrspace(3)**
  %loadedValue1397.i = load float addrspace(3)** %CastToValueType1396.i, align 8
  %masked_load429.i = load float addrspace(3)* %loadedValue1397.i, align 4
  %"&(pSB[currWI].offset)1550.i" = add nuw i64 %CurrSBIndex..4.i, 472
  %"&pSB[currWI].offset1551.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1550.i"
  %CastToValueType1552.i = bitcast i8* %"&pSB[currWI].offset1551.i" to float*
  store float %masked_load429.i, float* %CastToValueType1552.i, align 4
  br label %postload762.i

postload762.i:                                    ; preds = %preload761.i, %postload759.i
  %phi763.i = phi float [ undef, %postload759.i ], [ %masked_load429.i, %preload761.i ]
  %"&(pSB[currWI].offset)1554.i" = add nuw i64 %CurrSBIndex..4.i, 476
  %"&pSB[currWI].offset1555.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1554.i"
  %CastToValueType1556.i = bitcast i8* %"&pSB[currWI].offset1555.i" to float*
  store float %phi763.i, float* %CastToValueType1556.i, align 4
  %"&(pSB[currWI].offset)1259.i" = add nuw i64 %CurrSBIndex..4.i, 182
  %"&pSB[currWI].offset1260.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1259.i"
  %CastToValueType1261.i = bitcast i8* %"&pSB[currWI].offset1260.i" to i1*
  %loadedValue1262.i = load i1* %CastToValueType1261.i, align 1
  br i1 %loadedValue1262.i, label %preload764.i, label %postload765.i

preload764.i:                                     ; preds = %postload762.i
  %"&(pSB[currWI].offset)1403.i" = add nuw i64 %CurrSBIndex..4.i, 352
  %"&pSB[currWI].offset1404.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1403.i"
  %CastToValueType1405.i = bitcast i8* %"&pSB[currWI].offset1404.i" to float addrspace(3)**
  %loadedValue1406.i = load float addrspace(3)** %CastToValueType1405.i, align 8
  %masked_load430.i = load float addrspace(3)* %loadedValue1406.i, align 4
  %"&(pSB[currWI].offset)1563.i" = add nuw i64 %CurrSBIndex..4.i, 480
  %"&pSB[currWI].offset1564.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1563.i"
  %CastToValueType1565.i = bitcast i8* %"&pSB[currWI].offset1564.i" to float*
  store float %masked_load430.i, float* %CastToValueType1565.i, align 4
  br label %postload765.i

postload765.i:                                    ; preds = %preload764.i, %postload762.i
  %phi766.i = phi float [ undef, %postload762.i ], [ %masked_load430.i, %preload764.i ]
  %"&(pSB[currWI].offset)1567.i" = add nuw i64 %CurrSBIndex..4.i, 484
  %"&pSB[currWI].offset1568.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1567.i"
  %CastToValueType1569.i = bitcast i8* %"&pSB[currWI].offset1568.i" to float*
  store float %phi766.i, float* %CastToValueType1569.i, align 4
  %"&(pSB[currWI].offset)1268.i" = add nuw i64 %CurrSBIndex..4.i, 183
  %"&pSB[currWI].offset1269.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1268.i"
  %CastToValueType1270.i = bitcast i8* %"&pSB[currWI].offset1269.i" to i1*
  %loadedValue1271.i = load i1* %CastToValueType1270.i, align 1
  br i1 %loadedValue1271.i, label %preload767.i, label %postload768.i

preload767.i:                                     ; preds = %postload765.i
  %"&(pSB[currWI].offset)1412.i" = add nuw i64 %CurrSBIndex..4.i, 360
  %"&pSB[currWI].offset1413.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1412.i"
  %CastToValueType1414.i = bitcast i8* %"&pSB[currWI].offset1413.i" to float addrspace(3)**
  %loadedValue1415.i = load float addrspace(3)** %CastToValueType1414.i, align 8
  %masked_load431.i = load float addrspace(3)* %loadedValue1415.i, align 4
  %"&(pSB[currWI].offset)1576.i" = add nuw i64 %CurrSBIndex..4.i, 488
  %"&pSB[currWI].offset1577.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1576.i"
  %CastToValueType1578.i = bitcast i8* %"&pSB[currWI].offset1577.i" to float*
  store float %masked_load431.i, float* %CastToValueType1578.i, align 4
  br label %postload768.i

postload768.i:                                    ; preds = %preload767.i, %postload765.i
  %phi769.i = phi float [ undef, %postload765.i ], [ %masked_load431.i, %preload767.i ]
  %"&(pSB[currWI].offset)1580.i" = add nuw i64 %CurrSBIndex..4.i, 492
  %"&pSB[currWI].offset1581.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1580.i"
  %CastToValueType1582.i = bitcast i8* %"&pSB[currWI].offset1581.i" to float*
  store float %phi769.i, float* %CastToValueType1582.i, align 4
  %"&(pSB[currWI].offset)1277.i" = add nuw i64 %CurrSBIndex..4.i, 184
  %"&pSB[currWI].offset1278.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1277.i"
  %CastToValueType1279.i = bitcast i8* %"&pSB[currWI].offset1278.i" to i1*
  %loadedValue1280.i = load i1* %CastToValueType1279.i, align 1
  br i1 %loadedValue1280.i, label %preload770.i, label %postload771.i

preload770.i:                                     ; preds = %postload768.i
  %"&(pSB[currWI].offset)1421.i" = add nuw i64 %CurrSBIndex..4.i, 368
  %"&pSB[currWI].offset1422.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1421.i"
  %CastToValueType1423.i = bitcast i8* %"&pSB[currWI].offset1422.i" to float addrspace(3)**
  %loadedValue1424.i = load float addrspace(3)** %CastToValueType1423.i, align 8
  %masked_load432.i = load float addrspace(3)* %loadedValue1424.i, align 4
  %"&(pSB[currWI].offset)1589.i" = add nuw i64 %CurrSBIndex..4.i, 496
  %"&pSB[currWI].offset1590.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1589.i"
  %CastToValueType1591.i = bitcast i8* %"&pSB[currWI].offset1590.i" to float*
  store float %masked_load432.i, float* %CastToValueType1591.i, align 4
  br label %postload771.i

postload771.i:                                    ; preds = %preload770.i, %postload768.i
  %phi772.i = phi float [ undef, %postload768.i ], [ %masked_load432.i, %preload770.i ]
  %"&(pSB[currWI].offset)1593.i" = add nuw i64 %CurrSBIndex..4.i, 500
  %"&pSB[currWI].offset1594.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1593.i"
  %CastToValueType1595.i = bitcast i8* %"&pSB[currWI].offset1594.i" to float*
  store float %phi772.i, float* %CastToValueType1595.i, align 4
  %"&(pSB[currWI].offset)1286.i" = add nuw i64 %CurrSBIndex..4.i, 185
  %"&pSB[currWI].offset1287.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1286.i"
  %CastToValueType1288.i = bitcast i8* %"&pSB[currWI].offset1287.i" to i1*
  %loadedValue1289.i = load i1* %CastToValueType1288.i, align 1
  br i1 %loadedValue1289.i, label %preload773.i, label %postload774.i

preload773.i:                                     ; preds = %postload771.i
  %"&(pSB[currWI].offset)1430.i" = add nuw i64 %CurrSBIndex..4.i, 376
  %"&pSB[currWI].offset1431.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1430.i"
  %CastToValueType1432.i = bitcast i8* %"&pSB[currWI].offset1431.i" to float addrspace(3)**
  %loadedValue1433.i = load float addrspace(3)** %CastToValueType1432.i, align 8
  %masked_load433.i = load float addrspace(3)* %loadedValue1433.i, align 4
  %"&(pSB[currWI].offset)1602.i" = add nuw i64 %CurrSBIndex..4.i, 504
  %"&pSB[currWI].offset1603.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1602.i"
  %CastToValueType1604.i = bitcast i8* %"&pSB[currWI].offset1603.i" to float*
  store float %masked_load433.i, float* %CastToValueType1604.i, align 4
  br label %postload774.i

postload774.i:                                    ; preds = %preload773.i, %postload771.i
  %phi775.i = phi float [ undef, %postload771.i ], [ %masked_load433.i, %preload773.i ]
  %"&(pSB[currWI].offset)1606.i" = add nuw i64 %CurrSBIndex..4.i, 508
  %"&pSB[currWI].offset1607.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1606.i"
  %CastToValueType1608.i = bitcast i8* %"&pSB[currWI].offset1607.i" to float*
  store float %phi775.i, float* %CastToValueType1608.i, align 4
  %"&(pSB[currWI].offset)1295.i" = add nuw i64 %CurrSBIndex..4.i, 186
  %"&pSB[currWI].offset1296.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1295.i"
  %CastToValueType1297.i = bitcast i8* %"&pSB[currWI].offset1296.i" to i1*
  %loadedValue1298.i = load i1* %CastToValueType1297.i, align 1
  br i1 %loadedValue1298.i, label %preload776.i, label %postload777.i

preload776.i:                                     ; preds = %postload774.i
  %"&(pSB[currWI].offset)1439.i" = add nuw i64 %CurrSBIndex..4.i, 384
  %"&pSB[currWI].offset1440.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1439.i"
  %CastToValueType1441.i = bitcast i8* %"&pSB[currWI].offset1440.i" to float addrspace(3)**
  %loadedValue1442.i = load float addrspace(3)** %CastToValueType1441.i, align 8
  %masked_load434.i = load float addrspace(3)* %loadedValue1442.i, align 4
  %"&(pSB[currWI].offset)1615.i" = add nuw i64 %CurrSBIndex..4.i, 512
  %"&pSB[currWI].offset1616.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1615.i"
  %CastToValueType1617.i = bitcast i8* %"&pSB[currWI].offset1616.i" to float*
  store float %masked_load434.i, float* %CastToValueType1617.i, align 4
  br label %postload777.i

postload777.i:                                    ; preds = %preload776.i, %postload774.i
  %phi778.i = phi float [ undef, %postload774.i ], [ %masked_load434.i, %preload776.i ]
  %"&(pSB[currWI].offset)1619.i" = add nuw i64 %CurrSBIndex..4.i, 516
  %"&pSB[currWI].offset1620.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1619.i"
  %CastToValueType1621.i = bitcast i8* %"&pSB[currWI].offset1620.i" to float*
  store float %phi778.i, float* %CastToValueType1621.i, align 4
  %"&(pSB[currWI].offset)1304.i" = add nuw i64 %CurrSBIndex..4.i, 187
  %"&pSB[currWI].offset1305.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1304.i"
  %CastToValueType1306.i = bitcast i8* %"&pSB[currWI].offset1305.i" to i1*
  %loadedValue1307.i = load i1* %CastToValueType1306.i, align 1
  br i1 %loadedValue1307.i, label %preload779.i, label %postload780.i

preload779.i:                                     ; preds = %postload777.i
  %"&(pSB[currWI].offset)1448.i" = add nuw i64 %CurrSBIndex..4.i, 392
  %"&pSB[currWI].offset1449.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1448.i"
  %CastToValueType1450.i = bitcast i8* %"&pSB[currWI].offset1449.i" to float addrspace(3)**
  %loadedValue1451.i = load float addrspace(3)** %CastToValueType1450.i, align 8
  %masked_load435.i = load float addrspace(3)* %loadedValue1451.i, align 4
  %"&(pSB[currWI].offset)1628.i" = add nuw i64 %CurrSBIndex..4.i, 520
  %"&pSB[currWI].offset1629.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1628.i"
  %CastToValueType1630.i = bitcast i8* %"&pSB[currWI].offset1629.i" to float*
  store float %masked_load435.i, float* %CastToValueType1630.i, align 4
  br label %postload780.i

postload780.i:                                    ; preds = %preload779.i, %postload777.i
  %phi781.i = phi float [ undef, %postload777.i ], [ %masked_load435.i, %preload779.i ]
  %"&(pSB[currWI].offset)1632.i" = add nuw i64 %CurrSBIndex..4.i, 524
  %"&pSB[currWI].offset1633.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1632.i"
  %CastToValueType1634.i = bitcast i8* %"&pSB[currWI].offset1633.i" to float*
  store float %phi781.i, float* %CastToValueType1634.i, align 4
  %"&(pSB[currWI].offset)1313.i" = add nuw i64 %CurrSBIndex..4.i, 188
  %"&pSB[currWI].offset1314.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1313.i"
  %CastToValueType1315.i = bitcast i8* %"&pSB[currWI].offset1314.i" to i1*
  %loadedValue1316.i = load i1* %CastToValueType1315.i, align 1
  br i1 %loadedValue1316.i, label %preload782.i, label %postload783.i

preload782.i:                                     ; preds = %postload780.i
  %"&(pSB[currWI].offset)1457.i" = add nuw i64 %CurrSBIndex..4.i, 400
  %"&pSB[currWI].offset1458.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1457.i"
  %CastToValueType1459.i = bitcast i8* %"&pSB[currWI].offset1458.i" to float addrspace(3)**
  %loadedValue1460.i = load float addrspace(3)** %CastToValueType1459.i, align 8
  %masked_load436.i = load float addrspace(3)* %loadedValue1460.i, align 4
  %"&(pSB[currWI].offset)1641.i" = add nuw i64 %CurrSBIndex..4.i, 528
  %"&pSB[currWI].offset1642.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1641.i"
  %CastToValueType1643.i = bitcast i8* %"&pSB[currWI].offset1642.i" to float*
  store float %masked_load436.i, float* %CastToValueType1643.i, align 4
  br label %postload783.i

postload783.i:                                    ; preds = %preload782.i, %postload780.i
  %phi784.i = phi float [ undef, %postload780.i ], [ %masked_load436.i, %preload782.i ]
  %"&(pSB[currWI].offset)1645.i" = add nuw i64 %CurrSBIndex..4.i, 532
  %"&pSB[currWI].offset1646.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1645.i"
  %CastToValueType1647.i = bitcast i8* %"&pSB[currWI].offset1646.i" to float*
  store float %phi784.i, float* %CastToValueType1647.i, align 4
  %"&(pSB[currWI].offset)1322.i" = add nuw i64 %CurrSBIndex..4.i, 189
  %"&pSB[currWI].offset1323.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1322.i"
  %CastToValueType1324.i = bitcast i8* %"&pSB[currWI].offset1323.i" to i1*
  %loadedValue1325.i = load i1* %CastToValueType1324.i, align 1
  br i1 %loadedValue1325.i, label %preload785.i, label %postload786.i

preload785.i:                                     ; preds = %postload783.i
  %"&(pSB[currWI].offset)1466.i" = add nuw i64 %CurrSBIndex..4.i, 408
  %"&pSB[currWI].offset1467.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1466.i"
  %CastToValueType1468.i = bitcast i8* %"&pSB[currWI].offset1467.i" to float addrspace(3)**
  %loadedValue1469.i = load float addrspace(3)** %CastToValueType1468.i, align 8
  %masked_load437.i = load float addrspace(3)* %loadedValue1469.i, align 4
  %"&(pSB[currWI].offset)1654.i" = add nuw i64 %CurrSBIndex..4.i, 536
  %"&pSB[currWI].offset1655.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1654.i"
  %CastToValueType1656.i = bitcast i8* %"&pSB[currWI].offset1655.i" to float*
  store float %masked_load437.i, float* %CastToValueType1656.i, align 4
  br label %postload786.i

postload786.i:                                    ; preds = %preload785.i, %postload783.i
  %phi787.i = phi float [ undef, %postload783.i ], [ %masked_load437.i, %preload785.i ]
  %"&(pSB[currWI].offset)1658.i" = add nuw i64 %CurrSBIndex..4.i, 540
  %"&pSB[currWI].offset1659.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1658.i"
  %CastToValueType1660.i = bitcast i8* %"&pSB[currWI].offset1659.i" to float*
  store float %phi787.i, float* %CastToValueType1660.i, align 4
  %"&(pSB[currWI].offset)1331.i" = add nuw i64 %CurrSBIndex..4.i, 190
  %"&pSB[currWI].offset1332.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1331.i"
  %CastToValueType1333.i = bitcast i8* %"&pSB[currWI].offset1332.i" to i1*
  %loadedValue1334.i = load i1* %CastToValueType1333.i, align 1
  br i1 %loadedValue1334.i, label %preload788.i, label %postload789.i

preload788.i:                                     ; preds = %postload786.i
  %"&(pSB[currWI].offset)1475.i" = add nuw i64 %CurrSBIndex..4.i, 416
  %"&pSB[currWI].offset1476.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1475.i"
  %CastToValueType1477.i = bitcast i8* %"&pSB[currWI].offset1476.i" to float addrspace(3)**
  %loadedValue1478.i = load float addrspace(3)** %CastToValueType1477.i, align 8
  %masked_load438.i = load float addrspace(3)* %loadedValue1478.i, align 4
  %"&(pSB[currWI].offset)1667.i" = add nuw i64 %CurrSBIndex..4.i, 544
  %"&pSB[currWI].offset1668.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1667.i"
  %CastToValueType1669.i = bitcast i8* %"&pSB[currWI].offset1668.i" to float*
  store float %masked_load438.i, float* %CastToValueType1669.i, align 4
  br label %postload789.i

postload789.i:                                    ; preds = %preload788.i, %postload786.i
  %phi790.i = phi float [ undef, %postload786.i ], [ %masked_load438.i, %preload788.i ]
  %"&(pSB[currWI].offset)1671.i" = add nuw i64 %CurrSBIndex..4.i, 548
  %"&pSB[currWI].offset1672.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1671.i"
  %CastToValueType1673.i = bitcast i8* %"&pSB[currWI].offset1672.i" to float*
  store float %phi790.i, float* %CastToValueType1673.i, align 4
  %"&(pSB[currWI].offset)1340.i" = add nuw i64 %CurrSBIndex..4.i, 191
  %"&pSB[currWI].offset1341.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1340.i"
  %CastToValueType1342.i = bitcast i8* %"&pSB[currWI].offset1341.i" to i1*
  %loadedValue1343.i = load i1* %CastToValueType1342.i, align 1
  br i1 %loadedValue1343.i, label %preload791.i, label %postload792.i

preload791.i:                                     ; preds = %postload789.i
  %"&(pSB[currWI].offset)1484.i" = add nuw i64 %CurrSBIndex..4.i, 424
  %"&pSB[currWI].offset1485.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1484.i"
  %CastToValueType1486.i = bitcast i8* %"&pSB[currWI].offset1485.i" to float addrspace(3)**
  %loadedValue1487.i = load float addrspace(3)** %CastToValueType1486.i, align 8
  %masked_load439.i = load float addrspace(3)* %loadedValue1487.i, align 4
  %"&(pSB[currWI].offset)1680.i" = add nuw i64 %CurrSBIndex..4.i, 552
  %"&pSB[currWI].offset1681.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1680.i"
  %CastToValueType1682.i = bitcast i8* %"&pSB[currWI].offset1681.i" to float*
  store float %masked_load439.i, float* %CastToValueType1682.i, align 4
  br label %postload792.i

postload792.i:                                    ; preds = %preload791.i, %postload789.i
  %phi793.i = phi float [ undef, %postload789.i ], [ %masked_load439.i, %preload791.i ]
  %"&(pSB[currWI].offset)1684.i" = add nuw i64 %CurrSBIndex..4.i, 556
  %"&pSB[currWI].offset1685.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1684.i"
  %CastToValueType1686.i = bitcast i8* %"&pSB[currWI].offset1685.i" to float*
  store float %phi793.i, float* %CastToValueType1686.i, align 4
  %"&(pSB[currWI].offset)1349.i" = add nuw i64 %CurrSBIndex..4.i, 192
  %"&pSB[currWI].offset1350.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1349.i"
  %CastToValueType1351.i = bitcast i8* %"&pSB[currWI].offset1350.i" to i1*
  %loadedValue1352.i = load i1* %CastToValueType1351.i, align 1
  br i1 %loadedValue1352.i, label %preload794.i, label %postload795.i

preload794.i:                                     ; preds = %postload792.i
  %"&(pSB[currWI].offset)1493.i" = add nuw i64 %CurrSBIndex..4.i, 432
  %"&pSB[currWI].offset1494.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1493.i"
  %CastToValueType1495.i = bitcast i8* %"&pSB[currWI].offset1494.i" to float addrspace(3)**
  %loadedValue1496.i = load float addrspace(3)** %CastToValueType1495.i, align 8
  %masked_load440.i = load float addrspace(3)* %loadedValue1496.i, align 4
  %"&(pSB[currWI].offset)1693.i" = add nuw i64 %CurrSBIndex..4.i, 560
  %"&pSB[currWI].offset1694.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1693.i"
  %CastToValueType1695.i = bitcast i8* %"&pSB[currWI].offset1694.i" to float*
  store float %masked_load440.i, float* %CastToValueType1695.i, align 4
  br label %postload795.i

postload795.i:                                    ; preds = %preload794.i, %postload792.i
  %phi796.i = phi float [ undef, %postload792.i ], [ %masked_load440.i, %preload794.i ]
  %"&(pSB[currWI].offset)1506.i" = add nuw i64 %CurrSBIndex..4.i, 444
  %"&pSB[currWI].offset1507.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1506.i"
  %CastToValueType1508.i = bitcast i8* %"&pSB[currWI].offset1507.i" to float*
  %loadedValue1509.i = load float* %CastToValueType1508.i, align 4
  %temp.vect287.i = insertelement <16 x float> undef, float %loadedValue1509.i, i32 0
  %"&(pSB[currWI].offset)1519.i" = add nuw i64 %CurrSBIndex..4.i, 452
  %"&pSB[currWI].offset1520.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1519.i"
  %CastToValueType1521.i = bitcast i8* %"&pSB[currWI].offset1520.i" to float*
  %loadedValue1522.i = load float* %CastToValueType1521.i, align 4
  %temp.vect288.i = insertelement <16 x float> %temp.vect287.i, float %loadedValue1522.i, i32 1
  %"&(pSB[currWI].offset)1532.i" = add nuw i64 %CurrSBIndex..4.i, 460
  %"&pSB[currWI].offset1533.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1532.i"
  %CastToValueType1534.i = bitcast i8* %"&pSB[currWI].offset1533.i" to float*
  %loadedValue1535.i = load float* %CastToValueType1534.i, align 4
  %temp.vect289.i = insertelement <16 x float> %temp.vect288.i, float %loadedValue1535.i, i32 2
  %"&(pSB[currWI].offset)1545.i" = add nuw i64 %CurrSBIndex..4.i, 468
  %"&pSB[currWI].offset1546.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1545.i"
  %CastToValueType1547.i = bitcast i8* %"&pSB[currWI].offset1546.i" to float*
  %loadedValue1548.i = load float* %CastToValueType1547.i, align 4
  %temp.vect290.i = insertelement <16 x float> %temp.vect289.i, float %loadedValue1548.i, i32 3
  %"&(pSB[currWI].offset)1558.i" = add nuw i64 %CurrSBIndex..4.i, 476
  %"&pSB[currWI].offset1559.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1558.i"
  %CastToValueType1560.i = bitcast i8* %"&pSB[currWI].offset1559.i" to float*
  %loadedValue1561.i = load float* %CastToValueType1560.i, align 4
  %temp.vect291.i = insertelement <16 x float> %temp.vect290.i, float %loadedValue1561.i, i32 4
  %"&(pSB[currWI].offset)1571.i" = add nuw i64 %CurrSBIndex..4.i, 484
  %"&pSB[currWI].offset1572.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1571.i"
  %CastToValueType1573.i = bitcast i8* %"&pSB[currWI].offset1572.i" to float*
  %loadedValue1574.i = load float* %CastToValueType1573.i, align 4
  %temp.vect292.i = insertelement <16 x float> %temp.vect291.i, float %loadedValue1574.i, i32 5
  %"&(pSB[currWI].offset)1584.i" = add nuw i64 %CurrSBIndex..4.i, 492
  %"&pSB[currWI].offset1585.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1584.i"
  %CastToValueType1586.i = bitcast i8* %"&pSB[currWI].offset1585.i" to float*
  %loadedValue1587.i = load float* %CastToValueType1586.i, align 4
  %temp.vect293.i = insertelement <16 x float> %temp.vect292.i, float %loadedValue1587.i, i32 6
  %"&(pSB[currWI].offset)1597.i" = add nuw i64 %CurrSBIndex..4.i, 500
  %"&pSB[currWI].offset1598.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1597.i"
  %CastToValueType1599.i = bitcast i8* %"&pSB[currWI].offset1598.i" to float*
  %loadedValue1600.i = load float* %CastToValueType1599.i, align 4
  %temp.vect294.i = insertelement <16 x float> %temp.vect293.i, float %loadedValue1600.i, i32 7
  %"&(pSB[currWI].offset)1610.i" = add nuw i64 %CurrSBIndex..4.i, 508
  %"&pSB[currWI].offset1611.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1610.i"
  %CastToValueType1612.i = bitcast i8* %"&pSB[currWI].offset1611.i" to float*
  %loadedValue1613.i = load float* %CastToValueType1612.i, align 4
  %temp.vect295.i = insertelement <16 x float> %temp.vect294.i, float %loadedValue1613.i, i32 8
  %"&(pSB[currWI].offset)1623.i" = add nuw i64 %CurrSBIndex..4.i, 516
  %"&pSB[currWI].offset1624.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1623.i"
  %CastToValueType1625.i = bitcast i8* %"&pSB[currWI].offset1624.i" to float*
  %loadedValue1626.i = load float* %CastToValueType1625.i, align 4
  %temp.vect296.i = insertelement <16 x float> %temp.vect295.i, float %loadedValue1626.i, i32 9
  %"&(pSB[currWI].offset)1636.i" = add nuw i64 %CurrSBIndex..4.i, 524
  %"&pSB[currWI].offset1637.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1636.i"
  %CastToValueType1638.i = bitcast i8* %"&pSB[currWI].offset1637.i" to float*
  %loadedValue1639.i = load float* %CastToValueType1638.i, align 4
  %temp.vect297.i = insertelement <16 x float> %temp.vect296.i, float %loadedValue1639.i, i32 10
  %"&(pSB[currWI].offset)1649.i" = add nuw i64 %CurrSBIndex..4.i, 532
  %"&pSB[currWI].offset1650.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1649.i"
  %CastToValueType1651.i = bitcast i8* %"&pSB[currWI].offset1650.i" to float*
  %loadedValue1652.i = load float* %CastToValueType1651.i, align 4
  %temp.vect298.i = insertelement <16 x float> %temp.vect297.i, float %loadedValue1652.i, i32 11
  %"&(pSB[currWI].offset)1662.i" = add nuw i64 %CurrSBIndex..4.i, 540
  %"&pSB[currWI].offset1663.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1662.i"
  %CastToValueType1664.i = bitcast i8* %"&pSB[currWI].offset1663.i" to float*
  %loadedValue1665.i = load float* %CastToValueType1664.i, align 4
  %temp.vect299.i = insertelement <16 x float> %temp.vect298.i, float %loadedValue1665.i, i32 12
  %"&(pSB[currWI].offset)1675.i" = add nuw i64 %CurrSBIndex..4.i, 548
  %"&pSB[currWI].offset1676.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1675.i"
  %CastToValueType1677.i = bitcast i8* %"&pSB[currWI].offset1676.i" to float*
  %loadedValue1678.i = load float* %CastToValueType1677.i, align 4
  %temp.vect300.i = insertelement <16 x float> %temp.vect299.i, float %loadedValue1678.i, i32 13
  %"&(pSB[currWI].offset)1688.i" = add nuw i64 %CurrSBIndex..4.i, 556
  %"&pSB[currWI].offset1689.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1688.i"
  %CastToValueType1690.i = bitcast i8* %"&pSB[currWI].offset1689.i" to float*
  %loadedValue1691.i = load float* %CastToValueType1690.i, align 4
  %temp.vect301.i = insertelement <16 x float> %temp.vect300.i, float %loadedValue1691.i, i32 14
  %temp.vect302.i = insertelement <16 x float> %temp.vect301.i, float %phi796.i, i32 15
  %"&(pSB[currWI].offset)1697.i" = add nuw i64 %CurrSBIndex..4.i, 576
  %"&pSB[currWI].offset1698.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1697.i"
  %CastToValueType1699.i = bitcast i8* %"&pSB[currWI].offset1698.i" to <16 x float>*
  store <16 x float> %temp.vect302.i, <16 x float>* %CastToValueType1699.i, align 64
  %"&(pSB[currWI].offset)1008.i" = add nuw i64 %CurrSBIndex..4.i, 128
  %"&pSB[currWI].offset1009.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1008.i"
  %CastToValueType1010.i = bitcast i8* %"&pSB[currWI].offset1009.i" to float addrspace(3)**
  %loadedValue1011.i = load float addrspace(3)** %CastToValueType1010.i, align 8
  %ptrTypeCast286.i = bitcast float addrspace(3)* %loadedValue1011.i to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)1706.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1707.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1706.i"
  %CastToValueType1708.i = bitcast i8* %"&pSB[currWI].offset1707.i" to <16 x float> addrspace(3)**
  store <16 x float> addrspace(3)* %ptrTypeCast286.i, <16 x float> addrspace(3)** %CastToValueType1708.i, align 8
  %"&(pSB[currWI].offset)1214.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1215.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1214.i"
  %CastToValueType1216.i = bitcast i8* %"&pSB[currWI].offset1215.i" to <16 x i1>*
  %loadedValue1217.i = load <16 x i1>* %CastToValueType1216.i, align 16
  %exmask442.i = extractelement <16 x i1> %loadedValue1217.i, i32 0
  br i1 %exmask442.i, label %preload611.i, label %postload612.i

preload611.i:                                     ; preds = %postload795.i
  %"&(pSB[currWI].offset)1785.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1786.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1785.i"
  %CastToValueType1787.i = bitcast i8* %"&pSB[currWI].offset1786.i" to <16 x float> addrspace(3)**
  %loadedValue1788.i = load <16 x float> addrspace(3)** %CastToValueType1787.i, align 8
  %ptrTypeCast441.i = getelementptr inbounds <16 x float> addrspace(3)* %loadedValue1788.i, i64 0, i64 0
  %vload443.i = load float addrspace(3)* %ptrTypeCast441.i, align 4
  %"&(pSB[currWI].offset)1790.i" = add nuw i64 %CurrSBIndex..4.i, 648
  %"&pSB[currWI].offset1791.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1790.i"
  %CastToValueType1792.i = bitcast i8* %"&pSB[currWI].offset1791.i" to float*
  store float %vload443.i, float* %CastToValueType1792.i, align 4
  br label %postload612.i

postload612.i:                                    ; preds = %preload611.i, %postload795.i
  %phi613.i = phi float [ undef, %postload795.i ], [ %vload443.i, %preload611.i ]
  %vpack.i = insertelement <16 x float> undef, float %phi613.i, i32 0
  %"&(pSB[currWI].offset)1794.i" = add nuw i64 %CurrSBIndex..4.i, 704
  %"&pSB[currWI].offset1795.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1794.i"
  %CastToValueType1796.i = bitcast i8* %"&pSB[currWI].offset1795.i" to <16 x float>*
  store <16 x float> %vpack.i, <16 x float>* %CastToValueType1796.i, align 64
  %"&(pSB[currWI].offset)1209.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1210.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1209.i"
  %CastToValueType1211.i = bitcast i8* %"&pSB[currWI].offset1210.i" to <16 x i1>*
  %loadedValue1212.i = load <16 x i1>* %CastToValueType1211.i, align 16
  %exmask445.i = extractelement <16 x i1> %loadedValue1212.i, i32 1
  br i1 %exmask445.i, label %preload799.i, label %postload800.i

preload799.i:                                     ; preds = %postload612.i
  %"&(pSB[currWI].offset)1780.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1781.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1780.i"
  %CastToValueType1782.i = bitcast i8* %"&pSB[currWI].offset1781.i" to <16 x float> addrspace(3)**
  %loadedValue1783.i = load <16 x float> addrspace(3)** %CastToValueType1782.i, align 8
  %vload444.i = getelementptr <16 x float> addrspace(3)* %loadedValue1783.i, i64 0, i64 1
  %vload446.i = load float addrspace(3)* %vload444.i, align 4
  %"&(pSB[currWI].offset)1803.i" = add nuw i64 %CurrSBIndex..4.i, 768
  %"&pSB[currWI].offset1804.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1803.i"
  %CastToValueType1805.i = bitcast i8* %"&pSB[currWI].offset1804.i" to float*
  store float %vload446.i, float* %CastToValueType1805.i, align 4
  br label %postload800.i

postload800.i:                                    ; preds = %preload799.i, %postload612.i
  %phi801.i = phi float [ undef, %postload612.i ], [ %vload446.i, %preload799.i ]
  %"&(pSB[currWI].offset)1798.i" = add nuw i64 %CurrSBIndex..4.i, 704
  %"&pSB[currWI].offset1799.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1798.i"
  %CastToValueType1800.i = bitcast i8* %"&pSB[currWI].offset1799.i" to <16 x float>*
  %loadedValue1801.i = load <16 x float>* %CastToValueType1800.i, align 64
  %vpack447.i = insertelement <16 x float> %loadedValue1801.i, float %phi801.i, i32 1
  %"&(pSB[currWI].offset)1807.i" = add nuw i64 %CurrSBIndex..4.i, 832
  %"&pSB[currWI].offset1808.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1807.i"
  %CastToValueType1809.i = bitcast i8* %"&pSB[currWI].offset1808.i" to <16 x float>*
  store <16 x float> %vpack447.i, <16 x float>* %CastToValueType1809.i, align 64
  %"&(pSB[currWI].offset)1204.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1205.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1204.i"
  %CastToValueType1206.i = bitcast i8* %"&pSB[currWI].offset1205.i" to <16 x i1>*
  %loadedValue1207.i = load <16 x i1>* %CastToValueType1206.i, align 16
  %exmask449.i = extractelement <16 x i1> %loadedValue1207.i, i32 2
  br i1 %exmask449.i, label %preload802.i, label %postload803.i

preload802.i:                                     ; preds = %postload800.i
  %"&(pSB[currWI].offset)1775.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1776.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1775.i"
  %CastToValueType1777.i = bitcast i8* %"&pSB[currWI].offset1776.i" to <16 x float> addrspace(3)**
  %loadedValue1778.i = load <16 x float> addrspace(3)** %CastToValueType1777.i, align 8
  %vload448.i = getelementptr <16 x float> addrspace(3)* %loadedValue1778.i, i64 0, i64 2
  %vload450.i = load float addrspace(3)* %vload448.i, align 4
  %"&(pSB[currWI].offset)1816.i" = add nuw i64 %CurrSBIndex..4.i, 896
  %"&pSB[currWI].offset1817.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1816.i"
  %CastToValueType1818.i = bitcast i8* %"&pSB[currWI].offset1817.i" to float*
  store float %vload450.i, float* %CastToValueType1818.i, align 4
  br label %postload803.i

postload803.i:                                    ; preds = %preload802.i, %postload800.i
  %phi804.i = phi float [ undef, %postload800.i ], [ %vload450.i, %preload802.i ]
  %"&(pSB[currWI].offset)1811.i" = add nuw i64 %CurrSBIndex..4.i, 832
  %"&pSB[currWI].offset1812.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1811.i"
  %CastToValueType1813.i = bitcast i8* %"&pSB[currWI].offset1812.i" to <16 x float>*
  %loadedValue1814.i = load <16 x float>* %CastToValueType1813.i, align 64
  %vpack451.i = insertelement <16 x float> %loadedValue1814.i, float %phi804.i, i32 2
  %"&(pSB[currWI].offset)1820.i" = add nuw i64 %CurrSBIndex..4.i, 960
  %"&pSB[currWI].offset1821.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1820.i"
  %CastToValueType1822.i = bitcast i8* %"&pSB[currWI].offset1821.i" to <16 x float>*
  store <16 x float> %vpack451.i, <16 x float>* %CastToValueType1822.i, align 64
  %"&(pSB[currWI].offset)1199.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1200.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1199.i"
  %CastToValueType1201.i = bitcast i8* %"&pSB[currWI].offset1200.i" to <16 x i1>*
  %loadedValue1202.i = load <16 x i1>* %CastToValueType1201.i, align 16
  %exmask453.i = extractelement <16 x i1> %loadedValue1202.i, i32 3
  br i1 %exmask453.i, label %preload805.i, label %postload806.i

preload805.i:                                     ; preds = %postload803.i
  %"&(pSB[currWI].offset)1770.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1771.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1770.i"
  %CastToValueType1772.i = bitcast i8* %"&pSB[currWI].offset1771.i" to <16 x float> addrspace(3)**
  %loadedValue1773.i = load <16 x float> addrspace(3)** %CastToValueType1772.i, align 8
  %vload452.i = getelementptr <16 x float> addrspace(3)* %loadedValue1773.i, i64 0, i64 3
  %vload454.i = load float addrspace(3)* %vload452.i, align 4
  %"&(pSB[currWI].offset)1829.i" = add nuw i64 %CurrSBIndex..4.i, 1024
  %"&pSB[currWI].offset1830.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1829.i"
  %CastToValueType1831.i = bitcast i8* %"&pSB[currWI].offset1830.i" to float*
  store float %vload454.i, float* %CastToValueType1831.i, align 4
  br label %postload806.i

postload806.i:                                    ; preds = %preload805.i, %postload803.i
  %phi807.i = phi float [ undef, %postload803.i ], [ %vload454.i, %preload805.i ]
  %"&(pSB[currWI].offset)1824.i" = add nuw i64 %CurrSBIndex..4.i, 960
  %"&pSB[currWI].offset1825.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1824.i"
  %CastToValueType1826.i = bitcast i8* %"&pSB[currWI].offset1825.i" to <16 x float>*
  %loadedValue1827.i = load <16 x float>* %CastToValueType1826.i, align 64
  %vpack455.i = insertelement <16 x float> %loadedValue1827.i, float %phi807.i, i32 3
  %"&(pSB[currWI].offset)1833.i" = add nuw i64 %CurrSBIndex..4.i, 1088
  %"&pSB[currWI].offset1834.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1833.i"
  %CastToValueType1835.i = bitcast i8* %"&pSB[currWI].offset1834.i" to <16 x float>*
  store <16 x float> %vpack455.i, <16 x float>* %CastToValueType1835.i, align 64
  %"&(pSB[currWI].offset)1194.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1195.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1194.i"
  %CastToValueType1196.i = bitcast i8* %"&pSB[currWI].offset1195.i" to <16 x i1>*
  %loadedValue1197.i = load <16 x i1>* %CastToValueType1196.i, align 16
  %exmask457.i = extractelement <16 x i1> %loadedValue1197.i, i32 4
  br i1 %exmask457.i, label %preload740.i, label %postload741.i

preload740.i:                                     ; preds = %postload806.i
  %"&(pSB[currWI].offset)1765.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1766.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1765.i"
  %CastToValueType1767.i = bitcast i8* %"&pSB[currWI].offset1766.i" to <16 x float> addrspace(3)**
  %loadedValue1768.i = load <16 x float> addrspace(3)** %CastToValueType1767.i, align 8
  %vload456.i = getelementptr <16 x float> addrspace(3)* %loadedValue1768.i, i64 0, i64 4
  %vload458.i = load float addrspace(3)* %vload456.i, align 4
  %"&(pSB[currWI].offset)1842.i" = add nuw i64 %CurrSBIndex..4.i, 1152
  %"&pSB[currWI].offset1843.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1842.i"
  %CastToValueType1844.i = bitcast i8* %"&pSB[currWI].offset1843.i" to float*
  store float %vload458.i, float* %CastToValueType1844.i, align 4
  br label %postload741.i

postload741.i:                                    ; preds = %preload740.i, %postload806.i
  %phi742.i = phi float [ undef, %postload806.i ], [ %vload458.i, %preload740.i ]
  %"&(pSB[currWI].offset)1837.i" = add nuw i64 %CurrSBIndex..4.i, 1088
  %"&pSB[currWI].offset1838.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1837.i"
  %CastToValueType1839.i = bitcast i8* %"&pSB[currWI].offset1838.i" to <16 x float>*
  %loadedValue1840.i = load <16 x float>* %CastToValueType1839.i, align 64
  %vpack459.i = insertelement <16 x float> %loadedValue1840.i, float %phi742.i, i32 4
  %"&(pSB[currWI].offset)1846.i" = add nuw i64 %CurrSBIndex..4.i, 1216
  %"&pSB[currWI].offset1847.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1846.i"
  %CastToValueType1848.i = bitcast i8* %"&pSB[currWI].offset1847.i" to <16 x float>*
  store <16 x float> %vpack459.i, <16 x float>* %CastToValueType1848.i, align 64
  %"&(pSB[currWI].offset)1189.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1190.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1189.i"
  %CastToValueType1191.i = bitcast i8* %"&pSB[currWI].offset1190.i" to <16 x i1>*
  %loadedValue1192.i = load <16 x i1>* %CastToValueType1191.i, align 16
  %exmask461.i = extractelement <16 x i1> %loadedValue1192.i, i32 5
  br i1 %exmask461.i, label %preload743.i, label %postload744.i

preload743.i:                                     ; preds = %postload741.i
  %"&(pSB[currWI].offset)1760.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1761.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1760.i"
  %CastToValueType1762.i = bitcast i8* %"&pSB[currWI].offset1761.i" to <16 x float> addrspace(3)**
  %loadedValue1763.i = load <16 x float> addrspace(3)** %CastToValueType1762.i, align 8
  %vload460.i = getelementptr <16 x float> addrspace(3)* %loadedValue1763.i, i64 0, i64 5
  %vload462.i = load float addrspace(3)* %vload460.i, align 4
  %"&(pSB[currWI].offset)1855.i" = add nuw i64 %CurrSBIndex..4.i, 1280
  %"&pSB[currWI].offset1856.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1855.i"
  %CastToValueType1857.i = bitcast i8* %"&pSB[currWI].offset1856.i" to float*
  store float %vload462.i, float* %CastToValueType1857.i, align 4
  br label %postload744.i

postload744.i:                                    ; preds = %preload743.i, %postload741.i
  %phi745.i = phi float [ undef, %postload741.i ], [ %vload462.i, %preload743.i ]
  %"&(pSB[currWI].offset)1850.i" = add nuw i64 %CurrSBIndex..4.i, 1216
  %"&pSB[currWI].offset1851.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1850.i"
  %CastToValueType1852.i = bitcast i8* %"&pSB[currWI].offset1851.i" to <16 x float>*
  %loadedValue1853.i = load <16 x float>* %CastToValueType1852.i, align 64
  %vpack463.i = insertelement <16 x float> %loadedValue1853.i, float %phi745.i, i32 5
  %"&(pSB[currWI].offset)1859.i" = add nuw i64 %CurrSBIndex..4.i, 1344
  %"&pSB[currWI].offset1860.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1859.i"
  %CastToValueType1861.i = bitcast i8* %"&pSB[currWI].offset1860.i" to <16 x float>*
  store <16 x float> %vpack463.i, <16 x float>* %CastToValueType1861.i, align 64
  %"&(pSB[currWI].offset)1184.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1185.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1184.i"
  %CastToValueType1186.i = bitcast i8* %"&pSB[currWI].offset1185.i" to <16 x i1>*
  %loadedValue1187.i = load <16 x i1>* %CastToValueType1186.i, align 16
  %exmask465.i = extractelement <16 x i1> %loadedValue1187.i, i32 6
  br i1 %exmask465.i, label %preload746.i, label %postload747.i

preload746.i:                                     ; preds = %postload744.i
  %"&(pSB[currWI].offset)1755.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1756.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1755.i"
  %CastToValueType1757.i = bitcast i8* %"&pSB[currWI].offset1756.i" to <16 x float> addrspace(3)**
  %loadedValue1758.i = load <16 x float> addrspace(3)** %CastToValueType1757.i, align 8
  %vload464.i = getelementptr <16 x float> addrspace(3)* %loadedValue1758.i, i64 0, i64 6
  %vload466.i = load float addrspace(3)* %vload464.i, align 4
  %"&(pSB[currWI].offset)1868.i" = add nuw i64 %CurrSBIndex..4.i, 1408
  %"&pSB[currWI].offset1869.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1868.i"
  %CastToValueType1870.i = bitcast i8* %"&pSB[currWI].offset1869.i" to float*
  store float %vload466.i, float* %CastToValueType1870.i, align 4
  br label %postload747.i

postload747.i:                                    ; preds = %preload746.i, %postload744.i
  %phi748.i = phi float [ undef, %postload744.i ], [ %vload466.i, %preload746.i ]
  %"&(pSB[currWI].offset)1863.i" = add nuw i64 %CurrSBIndex..4.i, 1344
  %"&pSB[currWI].offset1864.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1863.i"
  %CastToValueType1865.i = bitcast i8* %"&pSB[currWI].offset1864.i" to <16 x float>*
  %loadedValue1866.i = load <16 x float>* %CastToValueType1865.i, align 64
  %vpack467.i = insertelement <16 x float> %loadedValue1866.i, float %phi748.i, i32 6
  %"&(pSB[currWI].offset)1872.i" = add nuw i64 %CurrSBIndex..4.i, 1472
  %"&pSB[currWI].offset1873.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1872.i"
  %CastToValueType1874.i = bitcast i8* %"&pSB[currWI].offset1873.i" to <16 x float>*
  store <16 x float> %vpack467.i, <16 x float>* %CastToValueType1874.i, align 64
  %"&(pSB[currWI].offset)1179.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1180.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1179.i"
  %CastToValueType1181.i = bitcast i8* %"&pSB[currWI].offset1180.i" to <16 x i1>*
  %loadedValue1182.i = load <16 x i1>* %CastToValueType1181.i, align 16
  %exmask469.i = extractelement <16 x i1> %loadedValue1182.i, i32 7
  br i1 %exmask469.i, label %preload713.i, label %postload714.i

preload713.i:                                     ; preds = %postload747.i
  %"&(pSB[currWI].offset)1750.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1751.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1750.i"
  %CastToValueType1752.i = bitcast i8* %"&pSB[currWI].offset1751.i" to <16 x float> addrspace(3)**
  %loadedValue1753.i = load <16 x float> addrspace(3)** %CastToValueType1752.i, align 8
  %vload468.i = getelementptr <16 x float> addrspace(3)* %loadedValue1753.i, i64 0, i64 7
  %vload470.i = load float addrspace(3)* %vload468.i, align 4
  %"&(pSB[currWI].offset)1881.i" = add nuw i64 %CurrSBIndex..4.i, 1536
  %"&pSB[currWI].offset1882.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1881.i"
  %CastToValueType1883.i = bitcast i8* %"&pSB[currWI].offset1882.i" to float*
  store float %vload470.i, float* %CastToValueType1883.i, align 4
  br label %postload714.i

postload714.i:                                    ; preds = %preload713.i, %postload747.i
  %phi715.i = phi float [ undef, %postload747.i ], [ %vload470.i, %preload713.i ]
  %"&(pSB[currWI].offset)1876.i" = add nuw i64 %CurrSBIndex..4.i, 1472
  %"&pSB[currWI].offset1877.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1876.i"
  %CastToValueType1878.i = bitcast i8* %"&pSB[currWI].offset1877.i" to <16 x float>*
  %loadedValue1879.i = load <16 x float>* %CastToValueType1878.i, align 64
  %vpack471.i = insertelement <16 x float> %loadedValue1879.i, float %phi715.i, i32 7
  %"&(pSB[currWI].offset)1885.i" = add nuw i64 %CurrSBIndex..4.i, 1600
  %"&pSB[currWI].offset1886.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1885.i"
  %CastToValueType1887.i = bitcast i8* %"&pSB[currWI].offset1886.i" to <16 x float>*
  store <16 x float> %vpack471.i, <16 x float>* %CastToValueType1887.i, align 64
  %"&(pSB[currWI].offset)1174.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1175.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1174.i"
  %CastToValueType1176.i = bitcast i8* %"&pSB[currWI].offset1175.i" to <16 x i1>*
  %loadedValue1177.i = load <16 x i1>* %CastToValueType1176.i, align 16
  %exmask473.i = extractelement <16 x i1> %loadedValue1177.i, i32 8
  br i1 %exmask473.i, label %preload716.i, label %postload717.i

preload716.i:                                     ; preds = %postload714.i
  %"&(pSB[currWI].offset)1745.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1746.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1745.i"
  %CastToValueType1747.i = bitcast i8* %"&pSB[currWI].offset1746.i" to <16 x float> addrspace(3)**
  %loadedValue1748.i = load <16 x float> addrspace(3)** %CastToValueType1747.i, align 8
  %vload472.i = getelementptr <16 x float> addrspace(3)* %loadedValue1748.i, i64 0, i64 8
  %vload474.i = load float addrspace(3)* %vload472.i, align 4
  %"&(pSB[currWI].offset)1894.i" = add nuw i64 %CurrSBIndex..4.i, 1664
  %"&pSB[currWI].offset1895.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1894.i"
  %CastToValueType1896.i = bitcast i8* %"&pSB[currWI].offset1895.i" to float*
  store float %vload474.i, float* %CastToValueType1896.i, align 4
  br label %postload717.i

postload717.i:                                    ; preds = %preload716.i, %postload714.i
  %phi718.i = phi float [ undef, %postload714.i ], [ %vload474.i, %preload716.i ]
  %"&(pSB[currWI].offset)1889.i" = add nuw i64 %CurrSBIndex..4.i, 1600
  %"&pSB[currWI].offset1890.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1889.i"
  %CastToValueType1891.i = bitcast i8* %"&pSB[currWI].offset1890.i" to <16 x float>*
  %loadedValue1892.i = load <16 x float>* %CastToValueType1891.i, align 64
  %vpack475.i = insertelement <16 x float> %loadedValue1892.i, float %phi718.i, i32 8
  %"&(pSB[currWI].offset)1898.i" = add nuw i64 %CurrSBIndex..4.i, 1728
  %"&pSB[currWI].offset1899.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1898.i"
  %CastToValueType1900.i = bitcast i8* %"&pSB[currWI].offset1899.i" to <16 x float>*
  store <16 x float> %vpack475.i, <16 x float>* %CastToValueType1900.i, align 64
  %"&(pSB[currWI].offset)1169.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1170.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1169.i"
  %CastToValueType1171.i = bitcast i8* %"&pSB[currWI].offset1170.i" to <16 x i1>*
  %loadedValue1172.i = load <16 x i1>* %CastToValueType1171.i, align 16
  %exmask477.i = extractelement <16 x i1> %loadedValue1172.i, i32 9
  br i1 %exmask477.i, label %preload719.i, label %postload720.i

preload719.i:                                     ; preds = %postload717.i
  %"&(pSB[currWI].offset)1740.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1741.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1740.i"
  %CastToValueType1742.i = bitcast i8* %"&pSB[currWI].offset1741.i" to <16 x float> addrspace(3)**
  %loadedValue1743.i = load <16 x float> addrspace(3)** %CastToValueType1742.i, align 8
  %vload476.i = getelementptr <16 x float> addrspace(3)* %loadedValue1743.i, i64 0, i64 9
  %vload478.i = load float addrspace(3)* %vload476.i, align 4
  %"&(pSB[currWI].offset)1907.i" = add nuw i64 %CurrSBIndex..4.i, 1792
  %"&pSB[currWI].offset1908.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1907.i"
  %CastToValueType1909.i = bitcast i8* %"&pSB[currWI].offset1908.i" to float*
  store float %vload478.i, float* %CastToValueType1909.i, align 4
  br label %postload720.i

postload720.i:                                    ; preds = %preload719.i, %postload717.i
  %phi721.i = phi float [ undef, %postload717.i ], [ %vload478.i, %preload719.i ]
  %"&(pSB[currWI].offset)1902.i" = add nuw i64 %CurrSBIndex..4.i, 1728
  %"&pSB[currWI].offset1903.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1902.i"
  %CastToValueType1904.i = bitcast i8* %"&pSB[currWI].offset1903.i" to <16 x float>*
  %loadedValue1905.i = load <16 x float>* %CastToValueType1904.i, align 64
  %vpack479.i = insertelement <16 x float> %loadedValue1905.i, float %phi721.i, i32 9
  %"&(pSB[currWI].offset)1911.i" = add nuw i64 %CurrSBIndex..4.i, 1856
  %"&pSB[currWI].offset1912.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1911.i"
  %CastToValueType1913.i = bitcast i8* %"&pSB[currWI].offset1912.i" to <16 x float>*
  store <16 x float> %vpack479.i, <16 x float>* %CastToValueType1913.i, align 64
  %"&(pSB[currWI].offset)1164.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1165.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1164.i"
  %CastToValueType1166.i = bitcast i8* %"&pSB[currWI].offset1165.i" to <16 x i1>*
  %loadedValue1167.i = load <16 x i1>* %CastToValueType1166.i, align 16
  %exmask481.i = extractelement <16 x i1> %loadedValue1167.i, i32 10
  br i1 %exmask481.i, label %preload722.i, label %postload723.i

preload722.i:                                     ; preds = %postload720.i
  %"&(pSB[currWI].offset)1735.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1736.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1735.i"
  %CastToValueType1737.i = bitcast i8* %"&pSB[currWI].offset1736.i" to <16 x float> addrspace(3)**
  %loadedValue1738.i = load <16 x float> addrspace(3)** %CastToValueType1737.i, align 8
  %vload480.i = getelementptr <16 x float> addrspace(3)* %loadedValue1738.i, i64 0, i64 10
  %vload482.i = load float addrspace(3)* %vload480.i, align 4
  %"&(pSB[currWI].offset)1920.i" = add nuw i64 %CurrSBIndex..4.i, 1920
  %"&pSB[currWI].offset1921.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1920.i"
  %CastToValueType1922.i = bitcast i8* %"&pSB[currWI].offset1921.i" to float*
  store float %vload482.i, float* %CastToValueType1922.i, align 4
  br label %postload723.i

postload723.i:                                    ; preds = %preload722.i, %postload720.i
  %phi724.i = phi float [ undef, %postload720.i ], [ %vload482.i, %preload722.i ]
  %"&(pSB[currWI].offset)1915.i" = add nuw i64 %CurrSBIndex..4.i, 1856
  %"&pSB[currWI].offset1916.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1915.i"
  %CastToValueType1917.i = bitcast i8* %"&pSB[currWI].offset1916.i" to <16 x float>*
  %loadedValue1918.i = load <16 x float>* %CastToValueType1917.i, align 64
  %vpack483.i = insertelement <16 x float> %loadedValue1918.i, float %phi724.i, i32 10
  %"&(pSB[currWI].offset)1924.i" = add nuw i64 %CurrSBIndex..4.i, 1984
  %"&pSB[currWI].offset1925.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1924.i"
  %CastToValueType1926.i = bitcast i8* %"&pSB[currWI].offset1925.i" to <16 x float>*
  store <16 x float> %vpack483.i, <16 x float>* %CastToValueType1926.i, align 64
  %"&(pSB[currWI].offset)1159.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1160.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1159.i"
  %CastToValueType1161.i = bitcast i8* %"&pSB[currWI].offset1160.i" to <16 x i1>*
  %loadedValue1162.i = load <16 x i1>* %CastToValueType1161.i, align 16
  %exmask485.i = extractelement <16 x i1> %loadedValue1162.i, i32 11
  br i1 %exmask485.i, label %preload725.i, label %postload726.i

preload725.i:                                     ; preds = %postload723.i
  %"&(pSB[currWI].offset)1730.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1731.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1730.i"
  %CastToValueType1732.i = bitcast i8* %"&pSB[currWI].offset1731.i" to <16 x float> addrspace(3)**
  %loadedValue1733.i = load <16 x float> addrspace(3)** %CastToValueType1732.i, align 8
  %vload484.i = getelementptr <16 x float> addrspace(3)* %loadedValue1733.i, i64 0, i64 11
  %vload486.i = load float addrspace(3)* %vload484.i, align 4
  %"&(pSB[currWI].offset)1933.i" = add nuw i64 %CurrSBIndex..4.i, 2048
  %"&pSB[currWI].offset1934.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1933.i"
  %CastToValueType1935.i = bitcast i8* %"&pSB[currWI].offset1934.i" to float*
  store float %vload486.i, float* %CastToValueType1935.i, align 4
  br label %postload726.i

postload726.i:                                    ; preds = %preload725.i, %postload723.i
  %phi727.i = phi float [ undef, %postload723.i ], [ %vload486.i, %preload725.i ]
  %"&(pSB[currWI].offset)1928.i" = add nuw i64 %CurrSBIndex..4.i, 1984
  %"&pSB[currWI].offset1929.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1928.i"
  %CastToValueType1930.i = bitcast i8* %"&pSB[currWI].offset1929.i" to <16 x float>*
  %loadedValue1931.i = load <16 x float>* %CastToValueType1930.i, align 64
  %vpack487.i = insertelement <16 x float> %loadedValue1931.i, float %phi727.i, i32 11
  %"&(pSB[currWI].offset)1937.i" = add nuw i64 %CurrSBIndex..4.i, 2112
  %"&pSB[currWI].offset1938.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1937.i"
  %CastToValueType1939.i = bitcast i8* %"&pSB[currWI].offset1938.i" to <16 x float>*
  store <16 x float> %vpack487.i, <16 x float>* %CastToValueType1939.i, align 64
  %"&(pSB[currWI].offset)1154.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1155.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1154.i"
  %CastToValueType1156.i = bitcast i8* %"&pSB[currWI].offset1155.i" to <16 x i1>*
  %loadedValue1157.i = load <16 x i1>* %CastToValueType1156.i, align 16
  %exmask489.i = extractelement <16 x i1> %loadedValue1157.i, i32 12
  br i1 %exmask489.i, label %preload728.i, label %postload729.i

preload728.i:                                     ; preds = %postload726.i
  %"&(pSB[currWI].offset)1725.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1726.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1725.i"
  %CastToValueType1727.i = bitcast i8* %"&pSB[currWI].offset1726.i" to <16 x float> addrspace(3)**
  %loadedValue1728.i = load <16 x float> addrspace(3)** %CastToValueType1727.i, align 8
  %vload488.i = getelementptr <16 x float> addrspace(3)* %loadedValue1728.i, i64 0, i64 12
  %vload490.i = load float addrspace(3)* %vload488.i, align 4
  %"&(pSB[currWI].offset)1946.i" = add nuw i64 %CurrSBIndex..4.i, 2176
  %"&pSB[currWI].offset1947.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1946.i"
  %CastToValueType1948.i = bitcast i8* %"&pSB[currWI].offset1947.i" to float*
  store float %vload490.i, float* %CastToValueType1948.i, align 4
  br label %postload729.i

postload729.i:                                    ; preds = %preload728.i, %postload726.i
  %phi730.i = phi float [ undef, %postload726.i ], [ %vload490.i, %preload728.i ]
  %"&(pSB[currWI].offset)1941.i" = add nuw i64 %CurrSBIndex..4.i, 2112
  %"&pSB[currWI].offset1942.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1941.i"
  %CastToValueType1943.i = bitcast i8* %"&pSB[currWI].offset1942.i" to <16 x float>*
  %loadedValue1944.i = load <16 x float>* %CastToValueType1943.i, align 64
  %vpack491.i = insertelement <16 x float> %loadedValue1944.i, float %phi730.i, i32 12
  %"&(pSB[currWI].offset)1950.i" = add nuw i64 %CurrSBIndex..4.i, 2240
  %"&pSB[currWI].offset1951.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1950.i"
  %CastToValueType1952.i = bitcast i8* %"&pSB[currWI].offset1951.i" to <16 x float>*
  store <16 x float> %vpack491.i, <16 x float>* %CastToValueType1952.i, align 64
  %"&(pSB[currWI].offset)1149.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1150.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1149.i"
  %CastToValueType1151.i = bitcast i8* %"&pSB[currWI].offset1150.i" to <16 x i1>*
  %loadedValue1152.i = load <16 x i1>* %CastToValueType1151.i, align 16
  %exmask493.i = extractelement <16 x i1> %loadedValue1152.i, i32 13
  br i1 %exmask493.i, label %preload731.i, label %postload732.i

preload731.i:                                     ; preds = %postload729.i
  %"&(pSB[currWI].offset)1720.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1721.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1720.i"
  %CastToValueType1722.i = bitcast i8* %"&pSB[currWI].offset1721.i" to <16 x float> addrspace(3)**
  %loadedValue1723.i = load <16 x float> addrspace(3)** %CastToValueType1722.i, align 8
  %vload492.i = getelementptr <16 x float> addrspace(3)* %loadedValue1723.i, i64 0, i64 13
  %vload494.i = load float addrspace(3)* %vload492.i, align 4
  %"&(pSB[currWI].offset)1959.i" = add nuw i64 %CurrSBIndex..4.i, 2304
  %"&pSB[currWI].offset1960.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1959.i"
  %CastToValueType1961.i = bitcast i8* %"&pSB[currWI].offset1960.i" to float*
  store float %vload494.i, float* %CastToValueType1961.i, align 4
  br label %postload732.i

postload732.i:                                    ; preds = %preload731.i, %postload729.i
  %phi733.i = phi float [ undef, %postload729.i ], [ %vload494.i, %preload731.i ]
  %"&(pSB[currWI].offset)1954.i" = add nuw i64 %CurrSBIndex..4.i, 2240
  %"&pSB[currWI].offset1955.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1954.i"
  %CastToValueType1956.i = bitcast i8* %"&pSB[currWI].offset1955.i" to <16 x float>*
  %loadedValue1957.i = load <16 x float>* %CastToValueType1956.i, align 64
  %vpack495.i = insertelement <16 x float> %loadedValue1957.i, float %phi733.i, i32 13
  %"&(pSB[currWI].offset)1963.i" = add nuw i64 %CurrSBIndex..4.i, 2368
  %"&pSB[currWI].offset1964.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1963.i"
  %CastToValueType1965.i = bitcast i8* %"&pSB[currWI].offset1964.i" to <16 x float>*
  store <16 x float> %vpack495.i, <16 x float>* %CastToValueType1965.i, align 64
  %"&(pSB[currWI].offset)1144.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1145.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1144.i"
  %CastToValueType1146.i = bitcast i8* %"&pSB[currWI].offset1145.i" to <16 x i1>*
  %loadedValue1147.i = load <16 x i1>* %CastToValueType1146.i, align 16
  %exmask497.i = extractelement <16 x i1> %loadedValue1147.i, i32 14
  br i1 %exmask497.i, label %preload734.i, label %postload735.i

preload734.i:                                     ; preds = %postload732.i
  %"&(pSB[currWI].offset)1715.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1716.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1715.i"
  %CastToValueType1717.i = bitcast i8* %"&pSB[currWI].offset1716.i" to <16 x float> addrspace(3)**
  %loadedValue1718.i = load <16 x float> addrspace(3)** %CastToValueType1717.i, align 8
  %vload496.i = getelementptr <16 x float> addrspace(3)* %loadedValue1718.i, i64 0, i64 14
  %vload498.i = load float addrspace(3)* %vload496.i, align 4
  %"&(pSB[currWI].offset)1972.i" = add nuw i64 %CurrSBIndex..4.i, 2432
  %"&pSB[currWI].offset1973.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1972.i"
  %CastToValueType1974.i = bitcast i8* %"&pSB[currWI].offset1973.i" to float*
  store float %vload498.i, float* %CastToValueType1974.i, align 4
  br label %postload735.i

postload735.i:                                    ; preds = %preload734.i, %postload732.i
  %phi736.i = phi float [ undef, %postload732.i ], [ %vload498.i, %preload734.i ]
  %"&(pSB[currWI].offset)1967.i" = add nuw i64 %CurrSBIndex..4.i, 2368
  %"&pSB[currWI].offset1968.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1967.i"
  %CastToValueType1969.i = bitcast i8* %"&pSB[currWI].offset1968.i" to <16 x float>*
  %loadedValue1970.i = load <16 x float>* %CastToValueType1969.i, align 64
  %vpack499.i = insertelement <16 x float> %loadedValue1970.i, float %phi736.i, i32 14
  %"&(pSB[currWI].offset)1976.i" = add nuw i64 %CurrSBIndex..4.i, 2496
  %"&pSB[currWI].offset1977.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1976.i"
  %CastToValueType1978.i = bitcast i8* %"&pSB[currWI].offset1977.i" to <16 x float>*
  store <16 x float> %vpack499.i, <16 x float>* %CastToValueType1978.i, align 64
  %"&(pSB[currWI].offset)1139.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1140.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1139.i"
  %CastToValueType1141.i = bitcast i8* %"&pSB[currWI].offset1140.i" to <16 x i1>*
  %loadedValue1142.i = load <16 x i1>* %CastToValueType1141.i, align 16
  %exmask501.i = extractelement <16 x i1> %loadedValue1142.i, i32 15
  br i1 %exmask501.i, label %preload737.i, label %postload738.i

preload737.i:                                     ; preds = %postload735.i
  %"&(pSB[currWI].offset)1710.i" = add nuw i64 %CurrSBIndex..4.i, 640
  %"&pSB[currWI].offset1711.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1710.i"
  %CastToValueType1712.i = bitcast i8* %"&pSB[currWI].offset1711.i" to <16 x float> addrspace(3)**
  %loadedValue1713.i = load <16 x float> addrspace(3)** %CastToValueType1712.i, align 8
  %vload500.i = getelementptr <16 x float> addrspace(3)* %loadedValue1713.i, i64 0, i64 15
  %vload502.i = load float addrspace(3)* %vload500.i, align 4
  %"&(pSB[currWI].offset)1985.i" = add nuw i64 %CurrSBIndex..4.i, 2560
  %"&pSB[currWI].offset1986.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1985.i"
  %CastToValueType1987.i = bitcast i8* %"&pSB[currWI].offset1986.i" to float*
  store float %vload502.i, float* %CastToValueType1987.i, align 4
  br label %postload738.i

postload738.i:                                    ; preds = %preload737.i, %postload735.i
  %phi739.i = phi float [ undef, %postload735.i ], [ %vload502.i, %preload737.i ]
  %"&(pSB[currWI].offset)1980.i" = add nuw i64 %CurrSBIndex..4.i, 2496
  %"&pSB[currWI].offset1981.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1980.i"
  %CastToValueType1982.i = bitcast i8* %"&pSB[currWI].offset1981.i" to <16 x float>*
  %loadedValue1983.i = load <16 x float>* %CastToValueType1982.i, align 64
  %vpack503.i = insertelement <16 x float> %loadedValue1983.i, float %phi739.i, i32 15
  %"&(pSB[currWI].offset)1701.i" = add nuw i64 %CurrSBIndex..4.i, 576
  %"&pSB[currWI].offset1702.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1701.i"
  %CastToValueType1703.i = bitcast i8* %"&pSB[currWI].offset1702.i" to <16 x float>*
  %loadedValue1704.i = load <16 x float>* %CastToValueType1703.i, align 64
  %209 = fadd <16 x float> %vpack503.i, %loadedValue1704.i
  %"&(pSB[currWI].offset)1989.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset1990.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1989.i"
  %CastToValueType1991.i = bitcast i8* %"&pSB[currWI].offset1990.i" to <16 x float>*
  store <16 x float> %209, <16 x float>* %CastToValueType1991.i, align 64
  %"&(pSB[currWI].offset)1003.i" = add nuw i64 %CurrSBIndex..4.i, 128
  %"&pSB[currWI].offset1004.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1003.i"
  %CastToValueType1005.i = bitcast i8* %"&pSB[currWI].offset1004.i" to float addrspace(3)**
  %loadedValue1006.i = load float addrspace(3)** %CastToValueType1005.i, align 8
  %ptrTypeCast303.i = bitcast float addrspace(3)* %loadedValue1006.i to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)2073.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2074.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2073.i"
  %CastToValueType2075.i = bitcast i8* %"&pSB[currWI].offset2074.i" to <16 x float> addrspace(3)**
  store <16 x float> addrspace(3)* %ptrTypeCast303.i, <16 x float> addrspace(3)** %CastToValueType2075.i, align 8
  %"&(pSB[currWI].offset)1134.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1135.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1134.i"
  %CastToValueType1136.i = bitcast i8* %"&pSB[currWI].offset1135.i" to <16 x i1>*
  %loadedValue1137.i = load <16 x i1>* %CastToValueType1136.i, align 16
  %exmask506.i = extractelement <16 x i1> %loadedValue1137.i, i32 0
  br i1 %exmask506.i, label %preload936.i, label %postload937.i

preload936.i:                                     ; preds = %postload738.i
  %"&(pSB[currWI].offset)2152.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2153.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2152.i"
  %CastToValueType2154.i = bitcast i8* %"&pSB[currWI].offset2153.i" to <16 x float> addrspace(3)**
  %loadedValue2155.i = load <16 x float> addrspace(3)** %CastToValueType2154.i, align 8
  %ptrTypeCast504.i = getelementptr inbounds <16 x float> addrspace(3)* %loadedValue2155.i, i64 0, i64 0
  %"&(pSB[currWI].offset)2068.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2069.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2068.i"
  %CastToValueType2070.i = bitcast i8* %"&pSB[currWI].offset2069.i" to <16 x float>*
  %loadedValue2071.i = load <16 x float>* %CastToValueType2070.i, align 64
  %exData507.i = extractelement <16 x float> %loadedValue2071.i, i32 0
  store float %exData507.i, float addrspace(3)* %ptrTypeCast504.i, align 4
  br label %postload937.i

postload937.i:                                    ; preds = %preload936.i, %postload738.i
  %"&(pSB[currWI].offset)1129.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1130.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1129.i"
  %CastToValueType1131.i = bitcast i8* %"&pSB[currWI].offset1130.i" to <16 x i1>*
  %loadedValue1132.i = load <16 x i1>* %CastToValueType1131.i, align 16
  %exmask509.i = extractelement <16 x i1> %loadedValue1132.i, i32 1
  br i1 %exmask509.i, label %preload939.i, label %postload940.i

preload939.i:                                     ; preds = %postload937.i
  %"&(pSB[currWI].offset)2147.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2148.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2147.i"
  %CastToValueType2149.i = bitcast i8* %"&pSB[currWI].offset2148.i" to <16 x float> addrspace(3)**
  %loadedValue2150.i = load <16 x float> addrspace(3)** %CastToValueType2149.i, align 8
  %vstore508.i = getelementptr <16 x float> addrspace(3)* %loadedValue2150.i, i64 0, i64 1
  %"&(pSB[currWI].offset)2063.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2064.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2063.i"
  %CastToValueType2065.i = bitcast i8* %"&pSB[currWI].offset2064.i" to <16 x float>*
  %loadedValue2066.i = load <16 x float>* %CastToValueType2065.i, align 64
  %exData510.i = extractelement <16 x float> %loadedValue2066.i, i32 1
  store float %exData510.i, float addrspace(3)* %vstore508.i, align 4
  br label %postload940.i

postload940.i:                                    ; preds = %preload939.i, %postload937.i
  %"&(pSB[currWI].offset)1124.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1125.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1124.i"
  %CastToValueType1126.i = bitcast i8* %"&pSB[currWI].offset1125.i" to <16 x i1>*
  %loadedValue1127.i = load <16 x i1>* %CastToValueType1126.i, align 16
  %exmask512.i = extractelement <16 x i1> %loadedValue1127.i, i32 2
  br i1 %exmask512.i, label %preload942.i, label %postload943.i

preload942.i:                                     ; preds = %postload940.i
  %"&(pSB[currWI].offset)2142.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2143.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2142.i"
  %CastToValueType2144.i = bitcast i8* %"&pSB[currWI].offset2143.i" to <16 x float> addrspace(3)**
  %loadedValue2145.i = load <16 x float> addrspace(3)** %CastToValueType2144.i, align 8
  %vstore511.i = getelementptr <16 x float> addrspace(3)* %loadedValue2145.i, i64 0, i64 2
  %"&(pSB[currWI].offset)2058.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2059.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2058.i"
  %CastToValueType2060.i = bitcast i8* %"&pSB[currWI].offset2059.i" to <16 x float>*
  %loadedValue2061.i = load <16 x float>* %CastToValueType2060.i, align 64
  %exData513.i = extractelement <16 x float> %loadedValue2061.i, i32 2
  store float %exData513.i, float addrspace(3)* %vstore511.i, align 4
  br label %postload943.i

postload943.i:                                    ; preds = %preload942.i, %postload940.i
  %"&(pSB[currWI].offset)1119.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1120.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1119.i"
  %CastToValueType1121.i = bitcast i8* %"&pSB[currWI].offset1120.i" to <16 x i1>*
  %loadedValue1122.i = load <16 x i1>* %CastToValueType1121.i, align 16
  %exmask515.i = extractelement <16 x i1> %loadedValue1122.i, i32 3
  br i1 %exmask515.i, label %preload945.i, label %postload946.i

preload945.i:                                     ; preds = %postload943.i
  %"&(pSB[currWI].offset)2137.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2138.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2137.i"
  %CastToValueType2139.i = bitcast i8* %"&pSB[currWI].offset2138.i" to <16 x float> addrspace(3)**
  %loadedValue2140.i = load <16 x float> addrspace(3)** %CastToValueType2139.i, align 8
  %vstore514.i = getelementptr <16 x float> addrspace(3)* %loadedValue2140.i, i64 0, i64 3
  %"&(pSB[currWI].offset)2053.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2054.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2053.i"
  %CastToValueType2055.i = bitcast i8* %"&pSB[currWI].offset2054.i" to <16 x float>*
  %loadedValue2056.i = load <16 x float>* %CastToValueType2055.i, align 64
  %exData516.i = extractelement <16 x float> %loadedValue2056.i, i32 3
  store float %exData516.i, float addrspace(3)* %vstore514.i, align 4
  br label %postload946.i

postload946.i:                                    ; preds = %preload945.i, %postload943.i
  %"&(pSB[currWI].offset)1114.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1115.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1114.i"
  %CastToValueType1116.i = bitcast i8* %"&pSB[currWI].offset1115.i" to <16 x i1>*
  %loadedValue1117.i = load <16 x i1>* %CastToValueType1116.i, align 16
  %exmask518.i = extractelement <16 x i1> %loadedValue1117.i, i32 4
  br i1 %exmask518.i, label %preload948.i, label %postload949.i

preload948.i:                                     ; preds = %postload946.i
  %"&(pSB[currWI].offset)2132.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2133.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2132.i"
  %CastToValueType2134.i = bitcast i8* %"&pSB[currWI].offset2133.i" to <16 x float> addrspace(3)**
  %loadedValue2135.i = load <16 x float> addrspace(3)** %CastToValueType2134.i, align 8
  %vstore517.i = getelementptr <16 x float> addrspace(3)* %loadedValue2135.i, i64 0, i64 4
  %"&(pSB[currWI].offset)2048.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2049.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2048.i"
  %CastToValueType2050.i = bitcast i8* %"&pSB[currWI].offset2049.i" to <16 x float>*
  %loadedValue2051.i = load <16 x float>* %CastToValueType2050.i, align 64
  %exData519.i = extractelement <16 x float> %loadedValue2051.i, i32 4
  store float %exData519.i, float addrspace(3)* %vstore517.i, align 4
  br label %postload949.i

postload949.i:                                    ; preds = %preload948.i, %postload946.i
  %"&(pSB[currWI].offset)1109.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1110.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1109.i"
  %CastToValueType1111.i = bitcast i8* %"&pSB[currWI].offset1110.i" to <16 x i1>*
  %loadedValue1112.i = load <16 x i1>* %CastToValueType1111.i, align 16
  %exmask521.i = extractelement <16 x i1> %loadedValue1112.i, i32 5
  br i1 %exmask521.i, label %preload951.i, label %postload952.i

preload951.i:                                     ; preds = %postload949.i
  %"&(pSB[currWI].offset)2127.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2128.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2127.i"
  %CastToValueType2129.i = bitcast i8* %"&pSB[currWI].offset2128.i" to <16 x float> addrspace(3)**
  %loadedValue2130.i = load <16 x float> addrspace(3)** %CastToValueType2129.i, align 8
  %vstore520.i = getelementptr <16 x float> addrspace(3)* %loadedValue2130.i, i64 0, i64 5
  %"&(pSB[currWI].offset)2043.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2044.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2043.i"
  %CastToValueType2045.i = bitcast i8* %"&pSB[currWI].offset2044.i" to <16 x float>*
  %loadedValue2046.i = load <16 x float>* %CastToValueType2045.i, align 64
  %exData522.i = extractelement <16 x float> %loadedValue2046.i, i32 5
  store float %exData522.i, float addrspace(3)* %vstore520.i, align 4
  br label %postload952.i

postload952.i:                                    ; preds = %preload951.i, %postload949.i
  %"&(pSB[currWI].offset)1104.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1105.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1104.i"
  %CastToValueType1106.i = bitcast i8* %"&pSB[currWI].offset1105.i" to <16 x i1>*
  %loadedValue1107.i = load <16 x i1>* %CastToValueType1106.i, align 16
  %exmask524.i = extractelement <16 x i1> %loadedValue1107.i, i32 6
  br i1 %exmask524.i, label %preload954.i, label %postload955.i

preload954.i:                                     ; preds = %postload952.i
  %"&(pSB[currWI].offset)2122.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2123.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2122.i"
  %CastToValueType2124.i = bitcast i8* %"&pSB[currWI].offset2123.i" to <16 x float> addrspace(3)**
  %loadedValue2125.i = load <16 x float> addrspace(3)** %CastToValueType2124.i, align 8
  %vstore523.i = getelementptr <16 x float> addrspace(3)* %loadedValue2125.i, i64 0, i64 6
  %"&(pSB[currWI].offset)2038.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2039.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2038.i"
  %CastToValueType2040.i = bitcast i8* %"&pSB[currWI].offset2039.i" to <16 x float>*
  %loadedValue2041.i = load <16 x float>* %CastToValueType2040.i, align 64
  %exData525.i = extractelement <16 x float> %loadedValue2041.i, i32 6
  store float %exData525.i, float addrspace(3)* %vstore523.i, align 4
  br label %postload955.i

postload955.i:                                    ; preds = %preload954.i, %postload952.i
  %"&(pSB[currWI].offset)1099.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1100.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1099.i"
  %CastToValueType1101.i = bitcast i8* %"&pSB[currWI].offset1100.i" to <16 x i1>*
  %loadedValue1102.i = load <16 x i1>* %CastToValueType1101.i, align 16
  %exmask527.i = extractelement <16 x i1> %loadedValue1102.i, i32 7
  br i1 %exmask527.i, label %preload957.i, label %postload958.i

preload957.i:                                     ; preds = %postload955.i
  %"&(pSB[currWI].offset)2117.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2118.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2117.i"
  %CastToValueType2119.i = bitcast i8* %"&pSB[currWI].offset2118.i" to <16 x float> addrspace(3)**
  %loadedValue2120.i = load <16 x float> addrspace(3)** %CastToValueType2119.i, align 8
  %vstore526.i = getelementptr <16 x float> addrspace(3)* %loadedValue2120.i, i64 0, i64 7
  %"&(pSB[currWI].offset)2033.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2034.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2033.i"
  %CastToValueType2035.i = bitcast i8* %"&pSB[currWI].offset2034.i" to <16 x float>*
  %loadedValue2036.i = load <16 x float>* %CastToValueType2035.i, align 64
  %exData528.i = extractelement <16 x float> %loadedValue2036.i, i32 7
  store float %exData528.i, float addrspace(3)* %vstore526.i, align 4
  br label %postload958.i

postload958.i:                                    ; preds = %preload957.i, %postload955.i
  %"&(pSB[currWI].offset)1094.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1095.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1094.i"
  %CastToValueType1096.i = bitcast i8* %"&pSB[currWI].offset1095.i" to <16 x i1>*
  %loadedValue1097.i = load <16 x i1>* %CastToValueType1096.i, align 16
  %exmask530.i = extractelement <16 x i1> %loadedValue1097.i, i32 8
  br i1 %exmask530.i, label %preload960.i, label %postload961.i

preload960.i:                                     ; preds = %postload958.i
  %"&(pSB[currWI].offset)2112.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2113.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2112.i"
  %CastToValueType2114.i = bitcast i8* %"&pSB[currWI].offset2113.i" to <16 x float> addrspace(3)**
  %loadedValue2115.i = load <16 x float> addrspace(3)** %CastToValueType2114.i, align 8
  %vstore529.i = getelementptr <16 x float> addrspace(3)* %loadedValue2115.i, i64 0, i64 8
  %"&(pSB[currWI].offset)2028.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2029.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2028.i"
  %CastToValueType2030.i = bitcast i8* %"&pSB[currWI].offset2029.i" to <16 x float>*
  %loadedValue2031.i = load <16 x float>* %CastToValueType2030.i, align 64
  %exData531.i = extractelement <16 x float> %loadedValue2031.i, i32 8
  store float %exData531.i, float addrspace(3)* %vstore529.i, align 4
  br label %postload961.i

postload961.i:                                    ; preds = %preload960.i, %postload958.i
  %"&(pSB[currWI].offset)1089.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1090.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1089.i"
  %CastToValueType1091.i = bitcast i8* %"&pSB[currWI].offset1090.i" to <16 x i1>*
  %loadedValue1092.i = load <16 x i1>* %CastToValueType1091.i, align 16
  %exmask533.i = extractelement <16 x i1> %loadedValue1092.i, i32 9
  br i1 %exmask533.i, label %preload963.i, label %postload964.i

preload963.i:                                     ; preds = %postload961.i
  %"&(pSB[currWI].offset)2107.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2108.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2107.i"
  %CastToValueType2109.i = bitcast i8* %"&pSB[currWI].offset2108.i" to <16 x float> addrspace(3)**
  %loadedValue2110.i = load <16 x float> addrspace(3)** %CastToValueType2109.i, align 8
  %vstore532.i = getelementptr <16 x float> addrspace(3)* %loadedValue2110.i, i64 0, i64 9
  %"&(pSB[currWI].offset)2023.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2024.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2023.i"
  %CastToValueType2025.i = bitcast i8* %"&pSB[currWI].offset2024.i" to <16 x float>*
  %loadedValue2026.i = load <16 x float>* %CastToValueType2025.i, align 64
  %exData534.i = extractelement <16 x float> %loadedValue2026.i, i32 9
  store float %exData534.i, float addrspace(3)* %vstore532.i, align 4
  br label %postload964.i

postload964.i:                                    ; preds = %preload963.i, %postload961.i
  %"&(pSB[currWI].offset)1084.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1085.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1084.i"
  %CastToValueType1086.i = bitcast i8* %"&pSB[currWI].offset1085.i" to <16 x i1>*
  %loadedValue1087.i = load <16 x i1>* %CastToValueType1086.i, align 16
  %exmask536.i = extractelement <16 x i1> %loadedValue1087.i, i32 10
  br i1 %exmask536.i, label %preload966.i, label %postload967.i

preload966.i:                                     ; preds = %postload964.i
  %"&(pSB[currWI].offset)2102.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2103.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2102.i"
  %CastToValueType2104.i = bitcast i8* %"&pSB[currWI].offset2103.i" to <16 x float> addrspace(3)**
  %loadedValue2105.i = load <16 x float> addrspace(3)** %CastToValueType2104.i, align 8
  %vstore535.i = getelementptr <16 x float> addrspace(3)* %loadedValue2105.i, i64 0, i64 10
  %"&(pSB[currWI].offset)2018.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2019.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2018.i"
  %CastToValueType2020.i = bitcast i8* %"&pSB[currWI].offset2019.i" to <16 x float>*
  %loadedValue2021.i = load <16 x float>* %CastToValueType2020.i, align 64
  %exData537.i = extractelement <16 x float> %loadedValue2021.i, i32 10
  store float %exData537.i, float addrspace(3)* %vstore535.i, align 4
  br label %postload967.i

postload967.i:                                    ; preds = %preload966.i, %postload964.i
  %"&(pSB[currWI].offset)1079.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1080.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1079.i"
  %CastToValueType1081.i = bitcast i8* %"&pSB[currWI].offset1080.i" to <16 x i1>*
  %loadedValue1082.i = load <16 x i1>* %CastToValueType1081.i, align 16
  %exmask539.i = extractelement <16 x i1> %loadedValue1082.i, i32 11
  br i1 %exmask539.i, label %preload969.i, label %postload970.i

preload969.i:                                     ; preds = %postload967.i
  %"&(pSB[currWI].offset)2097.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2098.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2097.i"
  %CastToValueType2099.i = bitcast i8* %"&pSB[currWI].offset2098.i" to <16 x float> addrspace(3)**
  %loadedValue2100.i = load <16 x float> addrspace(3)** %CastToValueType2099.i, align 8
  %vstore538.i = getelementptr <16 x float> addrspace(3)* %loadedValue2100.i, i64 0, i64 11
  %"&(pSB[currWI].offset)2013.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2014.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2013.i"
  %CastToValueType2015.i = bitcast i8* %"&pSB[currWI].offset2014.i" to <16 x float>*
  %loadedValue2016.i = load <16 x float>* %CastToValueType2015.i, align 64
  %exData540.i = extractelement <16 x float> %loadedValue2016.i, i32 11
  store float %exData540.i, float addrspace(3)* %vstore538.i, align 4
  br label %postload970.i

postload970.i:                                    ; preds = %preload969.i, %postload967.i
  %"&(pSB[currWI].offset)1074.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1075.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1074.i"
  %CastToValueType1076.i = bitcast i8* %"&pSB[currWI].offset1075.i" to <16 x i1>*
  %loadedValue1077.i = load <16 x i1>* %CastToValueType1076.i, align 16
  %exmask542.i = extractelement <16 x i1> %loadedValue1077.i, i32 12
  br i1 %exmask542.i, label %preload972.i, label %postload973.i

preload972.i:                                     ; preds = %postload970.i
  %"&(pSB[currWI].offset)2092.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2093.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2092.i"
  %CastToValueType2094.i = bitcast i8* %"&pSB[currWI].offset2093.i" to <16 x float> addrspace(3)**
  %loadedValue2095.i = load <16 x float> addrspace(3)** %CastToValueType2094.i, align 8
  %vstore541.i = getelementptr <16 x float> addrspace(3)* %loadedValue2095.i, i64 0, i64 12
  %"&(pSB[currWI].offset)2008.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2009.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2008.i"
  %CastToValueType2010.i = bitcast i8* %"&pSB[currWI].offset2009.i" to <16 x float>*
  %loadedValue2011.i = load <16 x float>* %CastToValueType2010.i, align 64
  %exData543.i = extractelement <16 x float> %loadedValue2011.i, i32 12
  store float %exData543.i, float addrspace(3)* %vstore541.i, align 4
  br label %postload973.i

postload973.i:                                    ; preds = %preload972.i, %postload970.i
  %"&(pSB[currWI].offset)1069.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1070.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1069.i"
  %CastToValueType1071.i = bitcast i8* %"&pSB[currWI].offset1070.i" to <16 x i1>*
  %loadedValue1072.i = load <16 x i1>* %CastToValueType1071.i, align 16
  %exmask545.i = extractelement <16 x i1> %loadedValue1072.i, i32 13
  br i1 %exmask545.i, label %preload975.i, label %postload976.i

preload975.i:                                     ; preds = %postload973.i
  %"&(pSB[currWI].offset)2087.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2088.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2087.i"
  %CastToValueType2089.i = bitcast i8* %"&pSB[currWI].offset2088.i" to <16 x float> addrspace(3)**
  %loadedValue2090.i = load <16 x float> addrspace(3)** %CastToValueType2089.i, align 8
  %vstore544.i = getelementptr <16 x float> addrspace(3)* %loadedValue2090.i, i64 0, i64 13
  %"&(pSB[currWI].offset)2003.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset2004.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2003.i"
  %CastToValueType2005.i = bitcast i8* %"&pSB[currWI].offset2004.i" to <16 x float>*
  %loadedValue2006.i = load <16 x float>* %CastToValueType2005.i, align 64
  %exData546.i = extractelement <16 x float> %loadedValue2006.i, i32 13
  store float %exData546.i, float addrspace(3)* %vstore544.i, align 4
  br label %postload976.i

postload976.i:                                    ; preds = %preload975.i, %postload973.i
  %"&(pSB[currWI].offset)1064.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1065.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1064.i"
  %CastToValueType1066.i = bitcast i8* %"&pSB[currWI].offset1065.i" to <16 x i1>*
  %loadedValue1067.i = load <16 x i1>* %CastToValueType1066.i, align 16
  %exmask548.i = extractelement <16 x i1> %loadedValue1067.i, i32 14
  br i1 %exmask548.i, label %preload978.i, label %postload979.i

preload978.i:                                     ; preds = %postload976.i
  %"&(pSB[currWI].offset)2082.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2083.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2082.i"
  %CastToValueType2084.i = bitcast i8* %"&pSB[currWI].offset2083.i" to <16 x float> addrspace(3)**
  %loadedValue2085.i = load <16 x float> addrspace(3)** %CastToValueType2084.i, align 8
  %vstore547.i = getelementptr <16 x float> addrspace(3)* %loadedValue2085.i, i64 0, i64 14
  %"&(pSB[currWI].offset)1998.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset1999.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1998.i"
  %CastToValueType2000.i = bitcast i8* %"&pSB[currWI].offset1999.i" to <16 x float>*
  %loadedValue2001.i = load <16 x float>* %CastToValueType2000.i, align 64
  %exData549.i = extractelement <16 x float> %loadedValue2001.i, i32 14
  store float %exData549.i, float addrspace(3)* %vstore547.i, align 4
  br label %postload979.i

postload979.i:                                    ; preds = %preload978.i, %postload976.i
  %"&(pSB[currWI].offset)1059.i" = add nuw i64 %CurrSBIndex..4.i, 176
  %"&pSB[currWI].offset1060.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1059.i"
  %CastToValueType1061.i = bitcast i8* %"&pSB[currWI].offset1060.i" to <16 x i1>*
  %loadedValue1062.i = load <16 x i1>* %CastToValueType1061.i, align 16
  %exmask551.i = extractelement <16 x i1> %loadedValue1062.i, i32 15
  br i1 %exmask551.i, label %preload981.i, label %postload982.i

preload981.i:                                     ; preds = %postload979.i
  %"&(pSB[currWI].offset)2077.i" = add nuw i64 %CurrSBIndex..4.i, 2688
  %"&pSB[currWI].offset2078.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2077.i"
  %CastToValueType2079.i = bitcast i8* %"&pSB[currWI].offset2078.i" to <16 x float> addrspace(3)**
  %loadedValue2080.i = load <16 x float> addrspace(3)** %CastToValueType2079.i, align 8
  %vstore550.i = getelementptr <16 x float> addrspace(3)* %loadedValue2080.i, i64 0, i64 15
  %"&(pSB[currWI].offset)1993.i" = add nuw i64 %CurrSBIndex..4.i, 2624
  %"&pSB[currWI].offset1994.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1993.i"
  %CastToValueType1995.i = bitcast i8* %"&pSB[currWI].offset1994.i" to <16 x float>*
  %loadedValue1996.i = load <16 x float>* %CastToValueType1995.i, align 64
  %exData552.i = extractelement <16 x float> %loadedValue1996.i, i32 15
  store float %exData552.i, float addrspace(3)* %vstore550.i, align 4
  br label %postload982.i

postload982.i:                                    ; preds = %preload981.i, %postload979.i
  %"&(pSB[currWI].offset)1041.i" = add nuw i64 %CurrSBIndex..4.i, 160
  %"&pSB[currWI].offset1042.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1041.i"
  %CastToValueType1043.i = bitcast i8* %"&pSB[currWI].offset1042.i" to <16 x i1>*
  %loadedValue1044.i = load <16 x i1>* %CastToValueType1043.i, align 16
  %extract306.i = extractelement <16 x i1> %loadedValue1044.i, i32 0
  br i1 %extract306.i, label %preload797.i, label %postload798.i

preload797.i:                                     ; preds = %postload982.i
  %check.WI.iter2195.i = icmp ult i64 %CurrWI..4.i, %22
  br i1 %check.WI.iter2195.i, label %thenBB2192.i, label %postload798.i

thenBB2192.i:                                     ; preds = %preload797.i
  %"CurrWI++2196.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride2198.i" = add nuw i64 %CurrSBIndex..4.i, 2752
  %cond10.i = icmp eq i32 %currBarrier.2.i, 8
  br i1 %cond10.i, label %postload798.i, label %SyncBB.i

postload798.i:                                    ; preds = %thenBB2184.i, %thenBB2177.i, %thenBB2192.i, %preload797.i, %postload982.i
  %currBarrier.1.i = phi i32 [ %currBarrier.2.i, %postload982.i ], [ %currBarrier.3.i, %thenBB2177.i ], [ %currBarrier.2.i, %thenBB2192.i ], [ %currBarrier.3.i, %thenBB2184.i ], [ 8, %preload797.i ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..4.i, %postload982.i ], [ %"loadedCurrSB+Stride2183.i", %thenBB2177.i ], [ %"loadedCurrSB+Stride2198.i", %thenBB2192.i ], [ %"loadedCurrSB+Stride2190.i", %thenBB2184.i ], [ 0, %preload797.i ]
  %CurrWI..3.i = phi i64 [ %CurrWI..4.i, %postload982.i ], [ %"CurrWI++2181.i", %thenBB2177.i ], [ %"CurrWI++2196.i", %thenBB2192.i ], [ %"CurrWI++2188.i", %thenBB2184.i ], [ 0, %preload797.i ]
  %"&(pSB[currWI].offset)1050.i" = add nuw i64 %CurrSBIndex..3.i, 164
  %"&pSB[currWI].offset1051.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1050.i"
  %CastToValueType1052.i = bitcast i8* %"&pSB[currWI].offset1051.i" to i32*
  %loadedValue1053.i = load i32* %CastToValueType1052.i, align 4
  %s.0.i = lshr i32 %loadedValue1053.i, 1
  %"&(pSB[currWI].offset)2157.i" = add nuw i64 %CurrSBIndex..3.i, 2696
  %"&pSB[currWI].offset2158.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2157.i"
  %CastToValueType2159.i = bitcast i8* %"&pSB[currWI].offset2158.i" to i32*
  store i32 %s.0.i, i32* %CastToValueType2159.i, align 4
  %210 = icmp eq i32 %s.0.i, 0
  %temp322.i = insertelement <16 x i1> undef, i1 %210, i32 0
  %vector323.i = shufflevector <16 x i1> %temp322.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond20.i = xor i1 %210, true
  %temp328.i = insertelement <16 x i1> undef, i1 %notCond20.i, i32 0
  %vector329.i = shufflevector <16 x i1> %temp328.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1036.i" = add nuw i64 %CurrSBIndex..3.i, 160
  %"&pSB[currWI].offset1037.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1036.i"
  %CastToValueType1038.i = bitcast i8* %"&pSB[currWI].offset1037.i" to <16 x i1>*
  %loadedValue1039.i = load <16 x i1>* %CastToValueType1038.i, align 16
  %who_left_tr21324.i = and <16 x i1> %loadedValue1039.i, %vector323.i
  %"&(pSB[currWI].offset)1022.i" = add nuw i64 %CurrSBIndex..3.i, 144
  %"&pSB[currWI].offset1023.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1022.i"
  %CastToValueType1024.i = bitcast i8* %"&pSB[currWI].offset1023.i" to <16 x i1>*
  %loadedValue1025.i = load <16 x i1>* %CastToValueType1024.i, align 16
  %loop_mask24326.i = or <16 x i1> %loadedValue1025.i, %who_left_tr21324.i
  %"&(pSB[currWI].offset)2161.i" = add nuw i64 %CurrSBIndex..3.i, 2704
  %"&pSB[currWI].offset2162.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2161.i"
  %CastToValueType2163.i = bitcast i8* %"&pSB[currWI].offset2162.i" to <16 x i1>*
  store <16 x i1> %loop_mask24326.i, <16 x i1>* %CastToValueType2163.i, align 16
  %curr_loop_mask26327.i = or <16 x i1> %loop_mask24326.i, %who_left_tr21324.i
  %ipred.i5.i = bitcast <16 x i1> %curr_loop_mask26327.i to i16
  %val.i6.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i5.i, i16 %ipred.i5.i) nounwind
  %tmp.i7.i = and i32 %val.i6.i, 1
  %res.i8.i = icmp eq i32 %tmp.i7.i, 0
  %"&(pSB[currWI].offset)1031.i" = add nuw i64 %CurrSBIndex..3.i, 160
  %"&pSB[currWI].offset1032.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)1031.i"
  %CastToValueType1033.i = bitcast i8* %"&pSB[currWI].offset1032.i" to <16 x i1>*
  %loadedValue1034.i = load <16 x i1>* %CastToValueType1033.i, align 16
  %local_edge29330.i = and <16 x i1> %loadedValue1034.i, %vector329.i
  %"&(pSB[currWI].offset)2165.i" = add nuw i64 %CurrSBIndex..3.i, 2720
  %"&pSB[currWI].offset2166.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)2165.i"
  %CastToValueType2167.i = bitcast i8* %"&pSB[currWI].offset2166.i" to <16 x i1>*
  store <16 x i1> %local_edge29330.i, <16 x i1>* %CastToValueType2167.i, align 16
  br i1 %res.i8.i, label %bb.nph.i, label %._crit_edge.i

._crit_edge.i:                                    ; preds = %postload798.i, %SyncBB.i
  %currBarrier.3.i = phi i32 [ %currBarrier.4.i, %SyncBB.i ], [ %currBarrier.1.i, %postload798.i ]
  %CurrSBIndex..5.i = phi i64 [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..3.i, %postload798.i ]
  %CurrWI..5.i = phi i64 [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..3.i, %postload798.i ]
  %"&(pSB[currWI].offset)985.i" = add nuw i64 %CurrSBIndex..5.i, 64
  %"&pSB[currWI].offset986.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)985.i"
  %CastToValueType987.i = bitcast i8* %"&pSB[currWI].offset986.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType987.i, align 64
  %211 = icmp eq <16 x i32> %loadedValue.i, zeroinitializer
  %extract332.i = extractelement <16 x i1> %211, i32 0
  %extract333.i = extractelement <16 x i1> %211, i32 1
  %extract334.i = extractelement <16 x i1> %211, i32 2
  %extract335.i = extractelement <16 x i1> %211, i32 3
  %extract336.i = extractelement <16 x i1> %211, i32 4
  %extract337.i = extractelement <16 x i1> %211, i32 5
  %extract338.i = extractelement <16 x i1> %211, i32 6
  %extract339.i = extractelement <16 x i1> %211, i32 7
  %extract340.i = extractelement <16 x i1> %211, i32 8
  %extract341.i = extractelement <16 x i1> %211, i32 9
  %extract342.i = extractelement <16 x i1> %211, i32 10
  %extract343.i = extractelement <16 x i1> %211, i32 11
  %extract344.i = extractelement <16 x i1> %211, i32 12
  %extract345.i = extractelement <16 x i1> %211, i32 13
  %extract346.i = extractelement <16 x i1> %211, i32 14
  %extract347.i = extractelement <16 x i1> %211, i32 15
  br i1 %extract332.i, label %preload808.i, label %postload809.i

preload808.i:                                     ; preds = %._crit_edge.i
  %masked_load553.i = load float addrspace(3)* %7, align 4
  br label %postload809.i

postload809.i:                                    ; preds = %preload808.i, %._crit_edge.i
  %phi810.i = phi float [ undef, %._crit_edge.i ], [ %masked_load553.i, %preload808.i ]
  br i1 %extract333.i, label %preload816.i, label %postload817.i

preload816.i:                                     ; preds = %postload809.i
  %masked_load554.i = load float addrspace(3)* %7, align 4
  br label %postload817.i

postload817.i:                                    ; preds = %preload816.i, %postload809.i
  %phi818.i = phi float [ undef, %postload809.i ], [ %masked_load554.i, %preload816.i ]
  br i1 %extract334.i, label %preload824.i, label %postload825.i

preload824.i:                                     ; preds = %postload817.i
  %masked_load555.i = load float addrspace(3)* %7, align 4
  br label %postload825.i

postload825.i:                                    ; preds = %preload824.i, %postload817.i
  %phi826.i = phi float [ undef, %postload817.i ], [ %masked_load555.i, %preload824.i ]
  br i1 %extract335.i, label %preload832.i, label %postload833.i

preload832.i:                                     ; preds = %postload825.i
  %masked_load556.i = load float addrspace(3)* %7, align 4
  br label %postload833.i

postload833.i:                                    ; preds = %preload832.i, %postload825.i
  %phi834.i = phi float [ undef, %postload825.i ], [ %masked_load556.i, %preload832.i ]
  br i1 %extract336.i, label %preload840.i, label %postload841.i

preload840.i:                                     ; preds = %postload833.i
  %masked_load557.i = load float addrspace(3)* %7, align 4
  br label %postload841.i

postload841.i:                                    ; preds = %preload840.i, %postload833.i
  %phi842.i = phi float [ undef, %postload833.i ], [ %masked_load557.i, %preload840.i ]
  br i1 %extract337.i, label %preload848.i, label %postload849.i

preload848.i:                                     ; preds = %postload841.i
  %masked_load558.i = load float addrspace(3)* %7, align 4
  br label %postload849.i

postload849.i:                                    ; preds = %preload848.i, %postload841.i
  %phi850.i = phi float [ undef, %postload841.i ], [ %masked_load558.i, %preload848.i ]
  br i1 %extract338.i, label %preload856.i, label %postload857.i

preload856.i:                                     ; preds = %postload849.i
  %masked_load559.i = load float addrspace(3)* %7, align 4
  br label %postload857.i

postload857.i:                                    ; preds = %preload856.i, %postload849.i
  %phi858.i = phi float [ undef, %postload849.i ], [ %masked_load559.i, %preload856.i ]
  br i1 %extract339.i, label %preload864.i, label %postload865.i

preload864.i:                                     ; preds = %postload857.i
  %masked_load560.i = load float addrspace(3)* %7, align 4
  br label %postload865.i

postload865.i:                                    ; preds = %preload864.i, %postload857.i
  %phi866.i = phi float [ undef, %postload857.i ], [ %masked_load560.i, %preload864.i ]
  br i1 %extract340.i, label %preload872.i, label %postload873.i

preload872.i:                                     ; preds = %postload865.i
  %masked_load561.i = load float addrspace(3)* %7, align 4
  br label %postload873.i

postload873.i:                                    ; preds = %preload872.i, %postload865.i
  %phi874.i = phi float [ undef, %postload865.i ], [ %masked_load561.i, %preload872.i ]
  br i1 %extract341.i, label %preload880.i, label %postload881.i

preload880.i:                                     ; preds = %postload873.i
  %masked_load562.i = load float addrspace(3)* %7, align 4
  br label %postload881.i

postload881.i:                                    ; preds = %preload880.i, %postload873.i
  %phi882.i = phi float [ undef, %postload873.i ], [ %masked_load562.i, %preload880.i ]
  br i1 %extract342.i, label %preload888.i, label %postload889.i

preload888.i:                                     ; preds = %postload881.i
  %masked_load563.i = load float addrspace(3)* %7, align 4
  br label %postload889.i

postload889.i:                                    ; preds = %preload888.i, %postload881.i
  %phi890.i = phi float [ undef, %postload881.i ], [ %masked_load563.i, %preload888.i ]
  br i1 %extract343.i, label %preload896.i, label %postload897.i

preload896.i:                                     ; preds = %postload889.i
  %masked_load564.i = load float addrspace(3)* %7, align 4
  br label %postload897.i

postload897.i:                                    ; preds = %preload896.i, %postload889.i
  %phi898.i = phi float [ undef, %postload889.i ], [ %masked_load564.i, %preload896.i ]
  br i1 %extract344.i, label %preload904.i, label %postload905.i

preload904.i:                                     ; preds = %postload897.i
  %masked_load565.i = load float addrspace(3)* %7, align 4
  br label %postload905.i

postload905.i:                                    ; preds = %preload904.i, %postload897.i
  %phi906.i = phi float [ undef, %postload897.i ], [ %masked_load565.i, %preload904.i ]
  br i1 %extract345.i, label %preload912.i, label %postload913.i

preload912.i:                                     ; preds = %postload905.i
  %masked_load566.i = load float addrspace(3)* %7, align 4
  br label %postload913.i

postload913.i:                                    ; preds = %preload912.i, %postload905.i
  %phi914.i = phi float [ undef, %postload905.i ], [ %masked_load566.i, %preload912.i ]
  br i1 %extract346.i, label %preload920.i, label %postload921.i

preload920.i:                                     ; preds = %postload913.i
  %masked_load567.i = load float addrspace(3)* %7, align 4
  br label %postload921.i

postload921.i:                                    ; preds = %preload920.i, %postload913.i
  %phi922.i = phi float [ undef, %postload913.i ], [ %masked_load567.i, %preload920.i ]
  br i1 %extract347.i, label %preload928.i, label %postload929.i

preload928.i:                                     ; preds = %postload921.i
  %masked_load568.i = load float addrspace(3)* %7, align 4
  br label %postload929.i

postload929.i:                                    ; preds = %preload928.i, %postload921.i
  %phi930.i = phi float [ undef, %postload921.i ], [ %masked_load568.i, %preload928.i ]
  br i1 %extract332.i, label %preload811.i, label %postload812.i

preload811.i:                                     ; preds = %postload929.i
  %212 = load i64* %16, align 8
  br label %postload812.i

postload812.i:                                    ; preds = %preload811.i, %postload929.i
  %phi813.i = phi i64 [ undef, %postload929.i ], [ %212, %preload811.i ]
  br i1 %extract333.i, label %preload819.i, label %postload820.i

preload819.i:                                     ; preds = %postload812.i
  %213 = load i64* %16, align 8
  br label %postload820.i

postload820.i:                                    ; preds = %preload819.i, %postload812.i
  %phi821.i = phi i64 [ undef, %postload812.i ], [ %213, %preload819.i ]
  br i1 %extract334.i, label %preload827.i, label %postload828.i

preload827.i:                                     ; preds = %postload820.i
  %214 = load i64* %16, align 8
  br label %postload828.i

postload828.i:                                    ; preds = %preload827.i, %postload820.i
  %phi829.i = phi i64 [ undef, %postload820.i ], [ %214, %preload827.i ]
  br i1 %extract335.i, label %preload835.i, label %postload836.i

preload835.i:                                     ; preds = %postload828.i
  %215 = load i64* %16, align 8
  br label %postload836.i

postload836.i:                                    ; preds = %preload835.i, %postload828.i
  %phi837.i = phi i64 [ undef, %postload828.i ], [ %215, %preload835.i ]
  br i1 %extract336.i, label %preload843.i, label %postload844.i

preload843.i:                                     ; preds = %postload836.i
  %216 = load i64* %16, align 8
  br label %postload844.i

postload844.i:                                    ; preds = %preload843.i, %postload836.i
  %phi845.i = phi i64 [ undef, %postload836.i ], [ %216, %preload843.i ]
  br i1 %extract337.i, label %preload851.i, label %postload852.i

preload851.i:                                     ; preds = %postload844.i
  %217 = load i64* %16, align 8
  br label %postload852.i

postload852.i:                                    ; preds = %preload851.i, %postload844.i
  %phi853.i = phi i64 [ undef, %postload844.i ], [ %217, %preload851.i ]
  br i1 %extract338.i, label %preload859.i, label %postload860.i

preload859.i:                                     ; preds = %postload852.i
  %218 = load i64* %16, align 8
  br label %postload860.i

postload860.i:                                    ; preds = %preload859.i, %postload852.i
  %phi861.i = phi i64 [ undef, %postload852.i ], [ %218, %preload859.i ]
  br i1 %extract339.i, label %preload867.i, label %postload868.i

preload867.i:                                     ; preds = %postload860.i
  %219 = load i64* %16, align 8
  br label %postload868.i

postload868.i:                                    ; preds = %preload867.i, %postload860.i
  %phi869.i = phi i64 [ undef, %postload860.i ], [ %219, %preload867.i ]
  br i1 %extract340.i, label %preload875.i, label %postload876.i

preload875.i:                                     ; preds = %postload868.i
  %220 = load i64* %16, align 8
  br label %postload876.i

postload876.i:                                    ; preds = %preload875.i, %postload868.i
  %phi877.i = phi i64 [ undef, %postload868.i ], [ %220, %preload875.i ]
  br i1 %extract341.i, label %preload883.i, label %postload884.i

preload883.i:                                     ; preds = %postload876.i
  %221 = load i64* %16, align 8
  br label %postload884.i

postload884.i:                                    ; preds = %preload883.i, %postload876.i
  %phi885.i = phi i64 [ undef, %postload876.i ], [ %221, %preload883.i ]
  br i1 %extract342.i, label %preload891.i, label %postload892.i

preload891.i:                                     ; preds = %postload884.i
  %222 = load i64* %16, align 8
  br label %postload892.i

postload892.i:                                    ; preds = %preload891.i, %postload884.i
  %phi893.i = phi i64 [ undef, %postload884.i ], [ %222, %preload891.i ]
  br i1 %extract343.i, label %preload899.i, label %postload900.i

preload899.i:                                     ; preds = %postload892.i
  %223 = load i64* %16, align 8
  br label %postload900.i

postload900.i:                                    ; preds = %preload899.i, %postload892.i
  %phi901.i = phi i64 [ undef, %postload892.i ], [ %223, %preload899.i ]
  br i1 %extract344.i, label %preload907.i, label %postload908.i

preload907.i:                                     ; preds = %postload900.i
  %224 = load i64* %16, align 8
  br label %postload908.i

postload908.i:                                    ; preds = %preload907.i, %postload900.i
  %phi909.i = phi i64 [ undef, %postload900.i ], [ %224, %preload907.i ]
  br i1 %extract345.i, label %preload915.i, label %postload916.i

preload915.i:                                     ; preds = %postload908.i
  %225 = load i64* %16, align 8
  br label %postload916.i

postload916.i:                                    ; preds = %preload915.i, %postload908.i
  %phi917.i = phi i64 [ undef, %postload908.i ], [ %225, %preload915.i ]
  br i1 %extract346.i, label %preload923.i, label %postload924.i

preload923.i:                                     ; preds = %postload916.i
  %226 = load i64* %16, align 8
  br label %postload924.i

postload924.i:                                    ; preds = %preload923.i, %postload916.i
  %phi925.i = phi i64 [ undef, %postload916.i ], [ %226, %preload923.i ]
  br i1 %extract347.i, label %preload931.i, label %postload932.i

preload931.i:                                     ; preds = %postload924.i
  %227 = load i64* %16, align 8
  br label %postload932.i

postload932.i:                                    ; preds = %preload931.i, %postload924.i
  %phi933.i = phi i64 [ undef, %postload924.i ], [ %227, %preload931.i ]
  %228 = getelementptr inbounds float addrspace(1)* %4, i64 %phi821.i
  %229 = getelementptr inbounds float addrspace(1)* %4, i64 %phi829.i
  %230 = getelementptr inbounds float addrspace(1)* %4, i64 %phi837.i
  %231 = getelementptr inbounds float addrspace(1)* %4, i64 %phi845.i
  %232 = getelementptr inbounds float addrspace(1)* %4, i64 %phi853.i
  %233 = getelementptr inbounds float addrspace(1)* %4, i64 %phi861.i
  %234 = getelementptr inbounds float addrspace(1)* %4, i64 %phi869.i
  %235 = getelementptr inbounds float addrspace(1)* %4, i64 %phi877.i
  %236 = getelementptr inbounds float addrspace(1)* %4, i64 %phi885.i
  %237 = getelementptr inbounds float addrspace(1)* %4, i64 %phi893.i
  %238 = getelementptr inbounds float addrspace(1)* %4, i64 %phi901.i
  %239 = getelementptr inbounds float addrspace(1)* %4, i64 %phi909.i
  %240 = getelementptr inbounds float addrspace(1)* %4, i64 %phi917.i
  %241 = getelementptr inbounds float addrspace(1)* %4, i64 %phi925.i
  %242 = getelementptr inbounds float addrspace(1)* %4, i64 %phi933.i
  br i1 %extract332.i, label %preload814.i, label %postload815.i

preload814.i:                                     ; preds = %postload932.i
  %243 = getelementptr inbounds float addrspace(1)* %4, i64 %phi813.i
  store float %phi810.i, float addrspace(1)* %243, align 4
  br label %postload815.i

postload815.i:                                    ; preds = %preload814.i, %postload932.i
  br i1 %extract333.i, label %preload822.i, label %postload823.i

preload822.i:                                     ; preds = %postload815.i
  store float %phi818.i, float addrspace(1)* %228, align 4
  br label %postload823.i

postload823.i:                                    ; preds = %preload822.i, %postload815.i
  br i1 %extract334.i, label %preload830.i, label %postload831.i

preload830.i:                                     ; preds = %postload823.i
  store float %phi826.i, float addrspace(1)* %229, align 4
  br label %postload831.i

postload831.i:                                    ; preds = %preload830.i, %postload823.i
  br i1 %extract335.i, label %preload838.i, label %postload839.i

preload838.i:                                     ; preds = %postload831.i
  store float %phi834.i, float addrspace(1)* %230, align 4
  br label %postload839.i

postload839.i:                                    ; preds = %preload838.i, %postload831.i
  br i1 %extract336.i, label %preload846.i, label %postload847.i

preload846.i:                                     ; preds = %postload839.i
  store float %phi842.i, float addrspace(1)* %231, align 4
  br label %postload847.i

postload847.i:                                    ; preds = %preload846.i, %postload839.i
  br i1 %extract337.i, label %preload854.i, label %postload855.i

preload854.i:                                     ; preds = %postload847.i
  store float %phi850.i, float addrspace(1)* %232, align 4
  br label %postload855.i

postload855.i:                                    ; preds = %preload854.i, %postload847.i
  br i1 %extract338.i, label %preload862.i, label %postload863.i

preload862.i:                                     ; preds = %postload855.i
  store float %phi858.i, float addrspace(1)* %233, align 4
  br label %postload863.i

postload863.i:                                    ; preds = %preload862.i, %postload855.i
  br i1 %extract339.i, label %preload870.i, label %postload871.i

preload870.i:                                     ; preds = %postload863.i
  store float %phi866.i, float addrspace(1)* %234, align 4
  br label %postload871.i

postload871.i:                                    ; preds = %preload870.i, %postload863.i
  br i1 %extract340.i, label %preload878.i, label %postload879.i

preload878.i:                                     ; preds = %postload871.i
  store float %phi874.i, float addrspace(1)* %235, align 4
  br label %postload879.i

postload879.i:                                    ; preds = %preload878.i, %postload871.i
  br i1 %extract341.i, label %preload886.i, label %postload887.i

preload886.i:                                     ; preds = %postload879.i
  store float %phi882.i, float addrspace(1)* %236, align 4
  br label %postload887.i

postload887.i:                                    ; preds = %preload886.i, %postload879.i
  br i1 %extract342.i, label %preload894.i, label %postload895.i

preload894.i:                                     ; preds = %postload887.i
  store float %phi890.i, float addrspace(1)* %237, align 4
  br label %postload895.i

postload895.i:                                    ; preds = %preload894.i, %postload887.i
  br i1 %extract343.i, label %preload902.i, label %postload903.i

preload902.i:                                     ; preds = %postload895.i
  store float %phi898.i, float addrspace(1)* %238, align 4
  br label %postload903.i

postload903.i:                                    ; preds = %preload902.i, %postload895.i
  br i1 %extract344.i, label %preload910.i, label %postload911.i

preload910.i:                                     ; preds = %postload903.i
  store float %phi906.i, float addrspace(1)* %239, align 4
  br label %postload911.i

postload911.i:                                    ; preds = %preload910.i, %postload903.i
  br i1 %extract345.i, label %preload918.i, label %postload919.i

preload918.i:                                     ; preds = %postload911.i
  store float %phi914.i, float addrspace(1)* %240, align 4
  br label %postload919.i

postload919.i:                                    ; preds = %preload918.i, %postload911.i
  br i1 %extract346.i, label %preload926.i, label %postload927.i

preload926.i:                                     ; preds = %postload919.i
  store float %phi922.i, float addrspace(1)* %241, align 4
  br label %postload927.i

postload927.i:                                    ; preds = %preload926.i, %postload919.i
  br i1 %extract347.i, label %preload934.i, label %UnifiedReturnBlock.i

preload934.i:                                     ; preds = %postload927.i
  store float %phi930.i, float addrspace(1)* %242, align 4
  %check.WI.iter2180.i = icmp ult i64 %CurrWI..5.i, %22
  br i1 %check.WI.iter2180.i, label %thenBB2177.i, label %____Vectorized_.reduce_separated_args.exit

thenBB2177.i:                                     ; preds = %preload934.i
  %"CurrWI++2181.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride2183.i" = add nuw i64 %CurrSBIndex..5.i, 2752
  %cond9.i = icmp eq i32 %currBarrier.3.i, 8
  br i1 %cond9.i, label %postload798.i, label %SyncBB.i

UnifiedReturnBlock.i:                             ; preds = %postload927.i
  %check.WI.iter2187.i = icmp ult i64 %CurrWI..5.i, %22
  br i1 %check.WI.iter2187.i, label %thenBB2184.i, label %____Vectorized_.reduce_separated_args.exit

thenBB2184.i:                                     ; preds = %UnifiedReturnBlock.i
  %"CurrWI++2188.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride2190.i" = add nuw i64 %CurrSBIndex..5.i, 2752
  %cond.i = icmp eq i32 %currBarrier.3.i, 8
  br i1 %cond.i, label %postload798.i, label %SyncBB.i

____Vectorized_.reduce_separated_args.exit:       ; preds = %preload934.i, %UnifiedReturnBlock.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(3)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__reduce_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(3))) *, unsigned int const", metadata !"opencl_reduce_locals_anchor", void (i8*)* @reduce}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__reduceNoLocal_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, unsigned int", metadata !"opencl_reduceNoLocal_locals_anchor", void (i8*)* @reduceNoLocal}
