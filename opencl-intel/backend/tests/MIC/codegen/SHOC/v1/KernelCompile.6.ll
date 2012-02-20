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

declare void @__uniformAdd_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32) nounwind

declare i64 @get_group_id(i32)

declare i64 @get_local_size(i32)

declare i64 @get_local_id(i32)

declare void @barrier(i64)

declare void @__scan_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32, i32, float addrspace(3)* nocapture) nounwind

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

define void @__uniformAdd_separated_args(float addrspace(1)* nocapture %g_data, float addrspace(1)* nocapture %uniforms, i32 %n, i32 %blockOffset, i32 %baseIndex, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = sext i32 %blockOffset to i64
  %1 = zext i32 %baseIndex to i64
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrSBIndex..0 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride", %thenBB ]
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %2 = load i64* %pWGId, align 8
  %3 = add i64 %2, %0
  %4 = getelementptr inbounds float addrspace(1)* %uniforms, i64 %3
  %5 = load float addrspace(1)* %4, align 4
  %6 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %7 = load i64* %6, align 8
  %8 = shl i64 %2, 1
  %9 = mul i64 %8, %7
  %10 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %11 = load i64* %10, align 8
  %12 = add i64 %11, %1
  %13 = add i64 %12, %9
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %13, i64* %CastToValueType, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 88
  br label %SyncBB

elseBB:                                           ; preds = %SyncBB
  %14 = sext i32 %n to i64
  br label %SyncBB7

SyncBB7:                                          ; preds = %thenBB10, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride16", %thenBB10 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++14", %thenBB10 ]
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i64*
  %loadedValue = load i64* %CastToValueType4, align 8
  %15 = and i64 %loadedValue, 4294967295
  %16 = getelementptr inbounds float addrspace(1)* %g_data, i64 %15
  %17 = load float addrspace(1)* %16, align 4
  %18 = fadd float %17, %5
  store float %18, float addrspace(1)* %16, align 4
  %19 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %20 = load i64* %19, align 8
  %21 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %22 = load i64* %21, align 8
  %23 = add i64 %22, %20
  %24 = icmp ult i64 %23, %14
  br i1 %24, label %25, label %UnifiedReturnBlock

; <label>:25                                      ; preds = %SyncBB7
  %26 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %27 = load i64* %26, align 8
  %28 = add i64 %27, %15
  %29 = getelementptr inbounds float addrspace(1)* %g_data, i64 %28
  %30 = load float addrspace(1)* %29, align 4
  %31 = fadd float %30, %5
  store float %31, float addrspace(1)* %29, align 4
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %SyncBB7, %25
  %check.WI.iter13 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter13, label %thenBB10, label %SyncBB8

thenBB10:                                         ; preds = %UnifiedReturnBlock
  %"CurrWI++14" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride16" = add nuw i64 %CurrSBIndex..1, 88
  br label %SyncBB7

SyncBB8:                                          ; preds = %UnifiedReturnBlock
  ret void
}

define void @__scan_separated_args(float addrspace(1)* nocapture %g_odata, float addrspace(1)* nocapture %g_idata, float addrspace(1)* nocapture %g_blockSums, i32 %n, i32 %blockIndex, i32 %baseIndex, i32 %storeSum, float addrspace(3)* nocapture %s_data, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp eq i32 %baseIndex, 0
  %1 = icmp eq i32 %blockIndex, 0
  %2 = icmp eq i32 %storeSum, 1
  %3 = sext i32 %blockIndex to i64
  br label %SyncBB202

SyncBB202:                                        ; preds = %thenBB211, %thenBB204, %FirstBB
  %CurrWI..1 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++215", %thenBB211 ], [ %"CurrWI++208", %thenBB204 ]
  %CurrSBIndex..1 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride217", %thenBB211 ], [ %"loadedCurrSB+Stride210", %thenBB204 ]
  %currBarrier.3 = phi i32 [ 8, %FirstBB ], [ %currBarrier.2, %thenBB211 ], [ %currBarrier.1, %thenBB204 ]
  br i1 %0, label %4, label %11

; <label>:4                                       ; preds = %SyncBB202
  %5 = load i64* %pWGId, align 8
  %6 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %7 = load i64* %6, align 8
  %8 = shl i64 %5, 1
  %9 = mul i64 %8, %7
  %10 = trunc i64 %9 to i32
  br label %11

; <label>:11                                      ; preds = %4, %SyncBB202
  %bIndex.0 = phi i32 [ %10, %4 ], [ %baseIndex, %SyncBB202 ]
  %12 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %13 = load i64* %12, align 8
  %14 = trunc i64 %13 to i32
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %14, i32* %CastToValueType, align 4
  %15 = add nsw i32 %14, %bIndex.0
  %16 = zext i32 %15 to i64
  %17 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %20 = trunc i64 %19 to i32
  %"&(pSB[currWI].offset)29" = add nuw i64 %CurrSBIndex..1, 12
  %"&pSB[currWI].offset30" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)29"
  %CastToValueType31 = bitcast i8* %"&pSB[currWI].offset30" to i32*
  store i32 %20, i32* %CastToValueType31, align 4
  %21 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %22 = load i64* %21, align 8
  %23 = add i64 %22, %13
  %24 = trunc i64 %23 to i32
  %"&(pSB[currWI].offset)43" = add nuw i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset44" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)43"
  %CastToValueType45 = bitcast i8* %"&pSB[currWI].offset44" to i32*
  store i32 %24, i32* %CastToValueType45, align 4
  %25 = sext i32 %15 to i64
  %"&(pSB[currWI].offset)62" = add nuw i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset63" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)62"
  %CastToValueType64 = bitcast i8* %"&pSB[currWI].offset63" to i64*
  store i64 %25, i64* %CastToValueType64, align 8
  %26 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %25
  %27 = load float addrspace(1)* %26, align 4
  %28 = sext i32 %14 to i64
  %29 = getelementptr inbounds float addrspace(3)* %s_data, i64 %28
  %"&(pSB[currWI].offset)71" = add nuw i64 %CurrSBIndex..1, 32
  %"&pSB[currWI].offset72" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)71"
  %CastToValueType73 = bitcast i8* %"&pSB[currWI].offset72" to float addrspace(3)**
  store float addrspace(3)* %29, float addrspace(3)** %CastToValueType73, align 8
  store float %27, float addrspace(3)* %29, align 4
  %30 = icmp slt i32 %24, %n
  %"&(pSB[currWI].offset)80" = add nuw i64 %CurrSBIndex..1, 40
  %"&pSB[currWI].offset81" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)80"
  %CastToValueType82 = bitcast i8* %"&pSB[currWI].offset81" to i1*
  store i1 %30, i1* %CastToValueType82, align 1
  br i1 %30, label %31, label %37

; <label>:31                                      ; preds = %11
  %"&(pSB[currWI].offset)33" = add nuw i64 %CurrSBIndex..1, 12
  %"&pSB[currWI].offset34" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)33"
  %CastToValueType35 = bitcast i8* %"&pSB[currWI].offset34" to i32*
  %loadedValue36 = load i32* %CastToValueType35, align 4
  %32 = sext i32 %loadedValue36 to i64
  %33 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %32
  %34 = load float addrspace(1)* %33, align 4
  %"&(pSB[currWI].offset)47" = add nuw i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset48" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)47"
  %CastToValueType49 = bitcast i8* %"&pSB[currWI].offset48" to i32*
  %loadedValue50 = load i32* %CastToValueType49, align 4
  %35 = sext i32 %loadedValue50 to i64
  %36 = getelementptr inbounds float addrspace(3)* %s_data, i64 %35
  store float %34, float addrspace(3)* %36, align 4
  br label %40

; <label>:37                                      ; preds = %11
  %"&(pSB[currWI].offset)52" = add nuw i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset53" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)52"
  %CastToValueType54 = bitcast i8* %"&pSB[currWI].offset53" to i32*
  %loadedValue55 = load i32* %CastToValueType54, align 4
  %38 = sext i32 %loadedValue55 to i64
  %39 = getelementptr inbounds float addrspace(3)* %s_data, i64 %38
  store float 0.000000e+00, float addrspace(3)* %39, align 4
  br label %40

; <label>:40                                      ; preds = %37, %31
  %41 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %42 = load i64* %41, align 8
  %43 = trunc i64 %42 to i32
  %44 = icmp sgt i32 %43, 0
  br i1 %44, label %bb.nph5, label %._crit_edge6

bb.nph5:                                          ; preds = %40
  %"&(pSB[currWI].offset)10" = add nuw i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset11" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10"
  %CastToValueType12 = bitcast i8* %"&pSB[currWI].offset11" to i32*
  %loadedValue = load i32* %CastToValueType12, align 4
  %45 = shl i32 %loadedValue, 1
  %"&(pSB[currWI].offset)89" = add nuw i64 %CurrSBIndex..1, 44
  %"&pSB[currWI].offset90" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)89"
  %CastToValueType91 = bitcast i8* %"&pSB[currWI].offset90" to i32*
  store i32 %45, i32* %CastToValueType91, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %59, %bb.nph5
  %CurrWI..3 = phi i64 [ %CurrWI..1, %bb.nph5 ], [ %CurrWI..2, %59 ]
  %CurrSBIndex..3 = phi i64 [ %CurrSBIndex..1, %bb.nph5 ], [ %CurrSBIndex..2, %59 ]
  %currBarrier.1 = phi i32 [ %currBarrier.3, %bb.nph5 ], [ %currBarrier.0, %59 ]
  %d.04 = phi i32 [ %43, %bb.nph5 ], [ %61, %59 ]
  %stride.03 = phi i32 [ 1, %bb.nph5 ], [ %60, %59 ]
  %"&(pSB[currWI].offset)112" = add nuw i64 %CurrSBIndex..3, 52
  %"&pSB[currWI].offset113" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)112"
  %CastToValueType114 = bitcast i8* %"&pSB[currWI].offset113" to i32*
  store i32 %stride.03, i32* %CastToValueType114, align 4
  %"&(pSB[currWI].offset)98" = add nuw i64 %CurrSBIndex..3, 48
  %"&pSB[currWI].offset99" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)98"
  %CastToValueType100 = bitcast i8* %"&pSB[currWI].offset99" to i32*
  store i32 %d.04, i32* %CastToValueType100, align 4
  %check.WI.iter207 = icmp ult i64 %CurrWI..3, %iterCount
  br i1 %check.WI.iter207, label %thenBB204, label %SyncBB198

thenBB204:                                        ; preds = %"Barrier BB"
  %"CurrWI++208" = add nuw i64 %CurrWI..3, 1
  %"loadedCurrSB+Stride210" = add nuw i64 %CurrSBIndex..3, 88
  %cond3 = icmp eq i32 %currBarrier.1, 8
  br i1 %cond3, label %SyncBB202, label %SyncBB198

SyncBB198:                                        ; preds = %"Barrier BB", %thenBB204, %thenBB211
  %CurrWI..2 = phi i64 [ %"CurrWI++208", %thenBB204 ], [ %"CurrWI++215", %thenBB211 ], [ 0, %"Barrier BB" ]
  %CurrSBIndex..2 = phi i64 [ %"loadedCurrSB+Stride210", %thenBB204 ], [ %"loadedCurrSB+Stride217", %thenBB211 ], [ 0, %"Barrier BB" ]
  %currBarrier.0 = phi i32 [ %currBarrier.1, %thenBB204 ], [ %currBarrier.2, %thenBB211 ], [ 2, %"Barrier BB" ]
  %"&(pSB[currWI].offset)19" = add nuw i64 %CurrSBIndex..2, 8
  %"&pSB[currWI].offset20" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)19"
  %CastToValueType21 = bitcast i8* %"&pSB[currWI].offset20" to i32*
  %loadedValue22 = load i32* %CastToValueType21, align 4
  %"&(pSB[currWI].offset)102" = add nuw i64 %CurrSBIndex..2, 48
  %"&pSB[currWI].offset103" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)102"
  %CastToValueType104 = bitcast i8* %"&pSB[currWI].offset103" to i32*
  %loadedValue105 = load i32* %CastToValueType104, align 4
  %46 = icmp slt i32 %loadedValue22, %loadedValue105
  br i1 %46, label %47, label %59

; <label>:47                                      ; preds = %SyncBB198
  %"&(pSB[currWI].offset)93" = add nuw i64 %CurrSBIndex..2, 44
  %"&pSB[currWI].offset94" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)93"
  %CastToValueType95 = bitcast i8* %"&pSB[currWI].offset94" to i32*
  %loadedValue96 = load i32* %CastToValueType95, align 4
  %"&(pSB[currWI].offset)121" = add nuw i64 %CurrSBIndex..2, 52
  %"&pSB[currWI].offset122" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)121"
  %CastToValueType123 = bitcast i8* %"&pSB[currWI].offset122" to i32*
  %loadedValue124 = load i32* %CastToValueType123, align 4
  %48 = mul i32 %loadedValue96, %loadedValue124
  %"&(pSB[currWI].offset)126" = add nuw i64 %CurrSBIndex..2, 52
  %"&pSB[currWI].offset127" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)126"
  %CastToValueType128 = bitcast i8* %"&pSB[currWI].offset127" to i32*
  %loadedValue129 = load i32* %CastToValueType128, align 4
  %49 = add i32 %loadedValue129, -1
  %50 = add i32 %49, %48
  %"&(pSB[currWI].offset)131" = add nuw i64 %CurrSBIndex..2, 52
  %"&pSB[currWI].offset132" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)131"
  %CastToValueType133 = bitcast i8* %"&pSB[currWI].offset132" to i32*
  %loadedValue134 = load i32* %CastToValueType133, align 4
  %51 = add i32 %50, %loadedValue134
  %52 = sext i32 %50 to i64
  %53 = getelementptr inbounds float addrspace(3)* %s_data, i64 %52
  %54 = load float addrspace(3)* %53, align 4
  %55 = sext i32 %51 to i64
  %56 = getelementptr inbounds float addrspace(3)* %s_data, i64 %55
  %57 = load float addrspace(3)* %56, align 4
  %58 = fadd float %57, %54
  store float %58, float addrspace(3)* %56, align 4
  br label %59

; <label>:59                                      ; preds = %47, %SyncBB198
  %"&(pSB[currWI].offset)116" = add nuw i64 %CurrSBIndex..2, 52
  %"&pSB[currWI].offset117" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)116"
  %CastToValueType118 = bitcast i8* %"&pSB[currWI].offset117" to i32*
  %loadedValue119 = load i32* %CastToValueType118, align 4
  %60 = shl i32 %loadedValue119, 1
  %"&(pSB[currWI].offset)136" = add nuw i64 %CurrSBIndex..2, 56
  %"&pSB[currWI].offset137" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)136"
  %CastToValueType138 = bitcast i8* %"&pSB[currWI].offset137" to i32*
  store i32 %60, i32* %CastToValueType138, align 4
  %"&(pSB[currWI].offset)107" = add nuw i64 %CurrSBIndex..2, 48
  %"&pSB[currWI].offset108" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)107"
  %CastToValueType109 = bitcast i8* %"&pSB[currWI].offset108" to i32*
  %loadedValue110 = load i32* %CastToValueType109, align 4
  %61 = ashr i32 %loadedValue110, 1
  %"&(pSB[currWI].offset)140" = add nuw i64 %CurrSBIndex..2, 60
  %"&pSB[currWI].offset141" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)140"
  %CastToValueType142 = bitcast i8* %"&pSB[currWI].offset141" to i32*
  store i32 %61, i32* %CastToValueType142, align 4
  %62 = icmp sgt i32 %61, 0
  br i1 %62, label %"Barrier BB", label %._crit_edge6

._crit_edge6:                                     ; preds = %59, %40
  %CurrWI..4 = phi i64 [ %CurrWI..2, %59 ], [ %CurrWI..1, %40 ]
  %CurrSBIndex..4 = phi i64 [ %CurrSBIndex..2, %59 ], [ %CurrSBIndex..1, %40 ]
  %currBarrier.2 = phi i32 [ %currBarrier.0, %59 ], [ %currBarrier.3, %40 ]
  %stride.0.lcssa = phi i32 [ %60, %59 ], [ 1, %40 ]
  %"&(pSB[currWI].offset)144" = add nuw i64 %CurrSBIndex..4, 64
  %"&pSB[currWI].offset145" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)144"
  %CastToValueType146 = bitcast i8* %"&pSB[currWI].offset145" to i32*
  store i32 %stride.0.lcssa, i32* %CastToValueType146, align 4
  br i1 %1, label %63, label %65

; <label>:63                                      ; preds = %._crit_edge6
  %64 = load i64* %pWGId, align 8
  br label %65

; <label>:65                                      ; preds = %._crit_edge6, %63
  %66 = phi i64 [ %64, %63 ], [ %3, %._crit_edge6 ]
  %67 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..4, i32 0, i64 0
  %68 = load i64* %67, align 8
  %69 = icmp eq i64 %68, 0
  br i1 %69, label %70, label %84

; <label>:70                                      ; preds = %65
  %71 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %72 = load i64* %71, align 8
  %73 = shl i64 %72, 1
  %74 = add i64 %73, 4294967295
  %75 = trunc i64 %74 to i32
  br i1 %2, label %76, label %._crit_edge7

; <label>:76                                      ; preds = %70
  %77 = sext i32 %75 to i64
  %78 = getelementptr inbounds float addrspace(3)* %s_data, i64 %77
  %79 = load float addrspace(3)* %78, align 4
  %sext = shl i64 %66, 32
  %80 = ashr i64 %sext, 32
  %81 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %80
  store float %79, float addrspace(1)* %81, align 4
  br label %._crit_edge7

._crit_edge7:                                     ; preds = %76, %70
  %82 = sext i32 %75 to i64
  %83 = getelementptr inbounds float addrspace(3)* %s_data, i64 %82
  store float 0.000000e+00, float addrspace(3)* %83, align 4
  br label %84

; <label>:84                                      ; preds = %._crit_edge7, %65
  %check.WI.iter214 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter214, label %thenBB211, label %SyncBB199

thenBB211:                                        ; preds = %84
  %"CurrWI++215" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride217" = add nuw i64 %CurrSBIndex..4, 88
  %cond2 = icmp eq i32 %currBarrier.2, 2
  br i1 %cond2, label %SyncBB198, label %SyncBB202

SyncBB199:                                        ; preds = %thenBB227, %thenBB219, %84
  %CurrWI..5 = phi i64 [ 0, %84 ], [ %"CurrWI++231", %thenBB227 ], [ %"CurrWI++223", %thenBB219 ]
  %CurrSBIndex..5 = phi i64 [ 0, %84 ], [ %"loadedCurrSB+Stride233", %thenBB227 ], [ %"loadedCurrSB+Stride225", %thenBB219 ]
  %currBarrier.7 = phi i32 [ 3, %84 ], [ %currBarrier.6, %thenBB227 ], [ %currBarrier.5, %thenBB219 ]
  %85 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %86 = load i64* %85, align 8
  %87 = icmp eq i64 %86, 0
  %"&(pSB[currWI].offset)148" = add nuw i64 %CurrSBIndex..5, 64
  br i1 %87, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %104, %SyncBB199
  %CurrWI..7 = phi i64 [ %CurrWI..6, %104 ], [ %CurrWI..5, %SyncBB199 ]
  %CurrSBIndex..7 = phi i64 [ %CurrSBIndex..6, %104 ], [ %CurrSBIndex..5, %SyncBB199 ]
  %currBarrier.5 = phi i32 [ %currBarrier.4, %104 ], [ %currBarrier.7, %SyncBB199 ]
  %d1.02 = phi i32 [ %105, %104 ], [ 1, %SyncBB199 ]
  %"&(pSB[currWI].offset)180.pn" = phi i64 [ %"&(pSB[currWI].offset)180", %104 ], [ %"&(pSB[currWI].offset)148", %SyncBB199 ]
  %stride.11.in.in = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)180.pn"
  %stride.11.in = bitcast i8* %stride.11.in.in to i32*
  %stride.11 = load i32* %stride.11.in, align 4
  %"&(pSB[currWI].offset)167" = add nuw i64 %CurrSBIndex..7, 72
  %"&pSB[currWI].offset168" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)167"
  %CastToValueType169 = bitcast i8* %"&pSB[currWI].offset168" to i32*
  store i32 %stride.11, i32* %CastToValueType169, align 4
  %"&(pSB[currWI].offset)153" = add nuw i64 %CurrSBIndex..7, 68
  %"&pSB[currWI].offset154" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)153"
  %CastToValueType155 = bitcast i8* %"&pSB[currWI].offset154" to i32*
  store i32 %d1.02, i32* %CastToValueType155, align 4
  %88 = lshr i32 %stride.11, 1
  %"&(pSB[currWI].offset)176" = add nuw i64 %CurrSBIndex..7, 76
  %"&pSB[currWI].offset177" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)176"
  %CastToValueType178 = bitcast i8* %"&pSB[currWI].offset177" to i32*
  store i32 %88, i32* %CastToValueType178, align 4
  %check.WI.iter222 = icmp ult i64 %CurrWI..7, %iterCount
  br i1 %check.WI.iter222, label %thenBB219, label %SyncBB200

thenBB219:                                        ; preds = %bb.nph
  %"CurrWI++223" = add nuw i64 %CurrWI..7, 1
  %"loadedCurrSB+Stride225" = add nuw i64 %CurrSBIndex..7, 88
  %cond1 = icmp eq i32 %currBarrier.5, 4
  br i1 %cond1, label %SyncBB200, label %SyncBB199

SyncBB200:                                        ; preds = %bb.nph, %thenBB219, %thenBB227
  %CurrWI..6 = phi i64 [ %"CurrWI++231", %thenBB227 ], [ %"CurrWI++223", %thenBB219 ], [ 0, %bb.nph ]
  %CurrSBIndex..6 = phi i64 [ %"loadedCurrSB+Stride233", %thenBB227 ], [ %"loadedCurrSB+Stride225", %thenBB219 ], [ 0, %bb.nph ]
  %currBarrier.4 = phi i32 [ %currBarrier.6, %thenBB227 ], [ %currBarrier.5, %thenBB219 ], [ 4, %bb.nph ]
  %"&(pSB[currWI].offset)24" = add nuw i64 %CurrSBIndex..6, 8
  %"&pSB[currWI].offset25" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)24"
  %CastToValueType26 = bitcast i8* %"&pSB[currWI].offset25" to i32*
  %loadedValue27 = load i32* %CastToValueType26, align 4
  %"&(pSB[currWI].offset)157" = add nuw i64 %CurrSBIndex..6, 68
  %"&pSB[currWI].offset158" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)157"
  %CastToValueType159 = bitcast i8* %"&pSB[currWI].offset158" to i32*
  %loadedValue160 = load i32* %CastToValueType159, align 4
  %89 = icmp slt i32 %loadedValue27, %loadedValue160
  br i1 %89, label %90, label %104

; <label>:90                                      ; preds = %SyncBB200
  %"&(pSB[currWI].offset)171" = add nuw i64 %CurrSBIndex..6, 72
  %"&pSB[currWI].offset172" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)171"
  %CastToValueType173 = bitcast i8* %"&pSB[currWI].offset172" to i32*
  %loadedValue174 = load i32* %CastToValueType173, align 4
  %91 = and i32 %loadedValue174, -2
  %"&(pSB[currWI].offset)14" = add nuw i64 %CurrSBIndex..6, 8
  %"&pSB[currWI].offset15" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)14"
  %CastToValueType16 = bitcast i8* %"&pSB[currWI].offset15" to i32*
  %loadedValue17 = load i32* %CastToValueType16, align 4
  %92 = mul i32 %91, %loadedValue17
  %"&(pSB[currWI].offset)190" = add nuw i64 %CurrSBIndex..6, 76
  %"&pSB[currWI].offset191" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)190"
  %CastToValueType192 = bitcast i8* %"&pSB[currWI].offset191" to i32*
  %loadedValue193 = load i32* %CastToValueType192, align 4
  %93 = add i32 %loadedValue193, -1
  %94 = add i32 %93, %92
  %"&(pSB[currWI].offset)185" = add nuw i64 %CurrSBIndex..6, 76
  %"&pSB[currWI].offset186" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)185"
  %CastToValueType187 = bitcast i8* %"&pSB[currWI].offset186" to i32*
  %loadedValue188 = load i32* %CastToValueType187, align 4
  %95 = add i32 %94, %loadedValue188
  %96 = sext i32 %94 to i64
  %97 = getelementptr inbounds float addrspace(3)* %s_data, i64 %96
  %98 = load float addrspace(3)* %97, align 4
  %99 = sext i32 %95 to i64
  %100 = getelementptr inbounds float addrspace(3)* %s_data, i64 %99
  %101 = load float addrspace(3)* %100, align 4
  store float %101, float addrspace(3)* %97, align 4
  %102 = load float addrspace(3)* %100, align 4
  %103 = fadd float %102, %98
  store float %103, float addrspace(3)* %100, align 4
  br label %104

; <label>:104                                     ; preds = %90, %SyncBB200
  %"&(pSB[currWI].offset)162" = add nuw i64 %CurrSBIndex..6, 68
  %"&pSB[currWI].offset163" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)162"
  %CastToValueType164 = bitcast i8* %"&pSB[currWI].offset163" to i32*
  %loadedValue165 = load i32* %CastToValueType164, align 4
  %105 = shl i32 %loadedValue165, 1
  %"&(pSB[currWI].offset)195" = add nuw i64 %CurrSBIndex..6, 80
  %"&pSB[currWI].offset196" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)195"
  %CastToValueType197 = bitcast i8* %"&pSB[currWI].offset196" to i32*
  store i32 %105, i32* %CastToValueType197, align 4
  %106 = sext i32 %105 to i64
  %107 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %108 = load i64* %107, align 8
  %109 = icmp ugt i64 %106, %108
  %"&(pSB[currWI].offset)180" = add nuw i64 %CurrSBIndex..6, 76
  br i1 %109, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %104, %SyncBB199
  %CurrWI..8 = phi i64 [ %CurrWI..5, %SyncBB199 ], [ %CurrWI..6, %104 ]
  %CurrSBIndex..8 = phi i64 [ %CurrSBIndex..5, %SyncBB199 ], [ %CurrSBIndex..6, %104 ]
  %currBarrier.6 = phi i32 [ %currBarrier.7, %SyncBB199 ], [ %currBarrier.4, %104 ]
  %check.WI.iter230 = icmp ult i64 %CurrWI..8, %iterCount
  br i1 %check.WI.iter230, label %thenBB227, label %SyncBB201

thenBB227:                                        ; preds = %._crit_edge
  %"CurrWI++231" = add nuw i64 %CurrWI..8, 1
  %"loadedCurrSB+Stride233" = add nuw i64 %CurrSBIndex..8, 88
  %cond = icmp eq i32 %currBarrier.6, 4
  br i1 %cond, label %SyncBB200, label %SyncBB199

SyncBB201:                                        ; preds = %._crit_edge, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %._crit_edge ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %._crit_edge ]
  %"&(pSB[currWI].offset)75" = add nuw i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset76" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)75"
  %CastToValueType77 = bitcast i8* %"&pSB[currWI].offset76" to float addrspace(3)**
  %loadedValue78 = load float addrspace(3)** %CastToValueType77, align 8
  %110 = load float addrspace(3)* %loadedValue78, align 4
  %"&(pSB[currWI].offset)66" = add nuw i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset67" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)66"
  %CastToValueType68 = bitcast i8* %"&pSB[currWI].offset67" to i64*
  %loadedValue69 = load i64* %CastToValueType68, align 8
  %111 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %loadedValue69
  store float %110, float addrspace(1)* %111, align 4
  %"&(pSB[currWI].offset)84" = add nuw i64 %CurrSBIndex..0, 40
  %"&pSB[currWI].offset85" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)84"
  %CastToValueType86 = bitcast i8* %"&pSB[currWI].offset85" to i1*
  %loadedValue87 = load i1* %CastToValueType86, align 1
  br i1 %loadedValue87, label %112, label %UnifiedReturnBlock

; <label>:112                                     ; preds = %SyncBB201
  %"&(pSB[currWI].offset)57" = add nuw i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset58" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)57"
  %CastToValueType59 = bitcast i8* %"&pSB[currWI].offset58" to i32*
  %loadedValue60 = load i32* %CastToValueType59, align 4
  %113 = sext i32 %loadedValue60 to i64
  %114 = getelementptr inbounds float addrspace(3)* %s_data, i64 %113
  %115 = load float addrspace(3)* %114, align 4
  %"&(pSB[currWI].offset)38" = add nuw i64 %CurrSBIndex..0, 12
  %"&pSB[currWI].offset39" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)38"
  %CastToValueType40 = bitcast i8* %"&pSB[currWI].offset39" to i32*
  %loadedValue41 = load i32* %CastToValueType40, align 4
  %116 = sext i32 %loadedValue41 to i64
  %117 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %116
  store float %115, float addrspace(1)* %117, align 4
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %SyncBB201, %112
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %UnifiedReturnBlock
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 88
  br label %SyncBB201

SyncBB:                                           ; preds = %UnifiedReturnBlock
  ret void
}

define void @uniformAdd(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to %struct.WorkDim**
  %16 = load %struct.WorkDim** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i64**
  %19 = load i64** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 64
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  %29 = sext i32 %10 to i64
  %30 = zext i32 %13 to i64
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %31 = load i64* %19, align 8
  %32 = add i64 %31, %29
  %33 = getelementptr inbounds float addrspace(1)* %4, i64 %32
  %34 = load float addrspace(1)* %33, align 4
  %35 = getelementptr %struct.WorkDim* %16, i64 0, i32 3, i64 0
  %36 = load i64* %35, align 8
  %37 = shl i64 %31, 1
  %38 = mul i64 %37, %36
  %39 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %40 = load i64* %39, align 8
  %41 = add i64 %40, %30
  %42 = add i64 %41, %38
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %42, i64* %CastToValueType.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 88
  br label %SyncBB.i

elseBB.i:                                         ; preds = %SyncBB.i
  %43 = sext i32 %7 to i64
  br label %SyncBB7.i

SyncBB7.i:                                        ; preds = %thenBB10.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride16.i", %thenBB10.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++14.i", %thenBB10.i ]
  %"&pSB[currWI].offset3.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..1.i
  %CastToValueType4.i = bitcast i8* %"&pSB[currWI].offset3.i" to i64*
  %loadedValue.i = load i64* %CastToValueType4.i, align 8
  %44 = and i64 %loadedValue.i, 4294967295
  %45 = getelementptr inbounds float addrspace(1)* %1, i64 %44
  %46 = load float addrspace(1)* %45, align 4
  %47 = fadd float %46, %34
  store float %47, float addrspace(1)* %45, align 4
  %48 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..1.i, i32 0, i64 0
  %49 = load i64* %48, align 8
  %50 = getelementptr %struct.WorkDim* %16, i64 0, i32 3, i64 0
  %51 = load i64* %50, align 8
  %52 = add i64 %51, %49
  %53 = icmp ult i64 %52, %43
  br i1 %53, label %54, label %UnifiedReturnBlock.i

; <label>:54                                      ; preds = %SyncBB7.i
  %55 = getelementptr %struct.WorkDim* %16, i64 0, i32 3, i64 0
  %56 = load i64* %55, align 8
  %57 = add i64 %56, %44
  %58 = getelementptr inbounds float addrspace(1)* %1, i64 %57
  %59 = load float addrspace(1)* %58, align 4
  %60 = fadd float %59, %34
  store float %60, float addrspace(1)* %58, align 4
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %54, %SyncBB7.i
  %check.WI.iter13.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter13.i, label %thenBB10.i, label %__uniformAdd_separated_args.exit

thenBB10.i:                                       ; preds = %UnifiedReturnBlock.i
  %"CurrWI++14.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride16.i" = add nuw i64 %CurrSBIndex..1.i, 88
  br label %SyncBB7.i

__uniformAdd_separated_args.exit:                 ; preds = %UnifiedReturnBlock.i
  ret void
}

define void @scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 36
  %18 = bitcast i8* %17 to i32*
  %19 = load i32* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 40
  %21 = bitcast i8* %20 to float addrspace(3)**
  %22 = load float addrspace(3)** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to %struct.WorkDim**
  %25 = load %struct.WorkDim** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 64
  %27 = bitcast i8* %26 to i64**
  %28 = load i64** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 80
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 96
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %38 = icmp eq i32 %16, 0
  %39 = icmp eq i32 %13, 0
  %40 = icmp eq i32 %19, 1
  %41 = sext i32 %13 to i64
  br label %SyncBB202.i

SyncBB202.i:                                      ; preds = %thenBB211.i, %thenBB204.i, %entry
  %CurrWI..1.i = phi i64 [ 0, %entry ], [ %"CurrWI++215.i", %thenBB211.i ], [ %"CurrWI++208.i", %thenBB204.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride217.i", %thenBB211.i ], [ %"loadedCurrSB+Stride210.i", %thenBB204.i ]
  %currBarrier.3.i = phi i32 [ 8, %entry ], [ %currBarrier.2.i, %thenBB211.i ], [ %currBarrier.1.i, %thenBB204.i ]
  br i1 %38, label %42, label %49

; <label>:42                                      ; preds = %SyncBB202.i
  %43 = load i64* %28, align 8
  %44 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %45 = load i64* %44, align 8
  %46 = shl i64 %43, 1
  %47 = mul i64 %46, %45
  %48 = trunc i64 %47 to i32
  br label %49

; <label>:49                                      ; preds = %42, %SyncBB202.i
  %bIndex.0.i = phi i32 [ %48, %42 ], [ %16, %SyncBB202.i ]
  %50 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..1.i, i32 0, i64 0
  %51 = load i64* %50, align 8
  %52 = trunc i64 %51 to i32
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %52, i32* %CastToValueType.i, align 4
  %53 = add nsw i32 %52, %bIndex.0.i
  %54 = zext i32 %53 to i64
  %55 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %56 = load i64* %55, align 8
  %57 = add i64 %54, %56
  %58 = trunc i64 %57 to i32
  %"&(pSB[currWI].offset)29.i" = add nuw i64 %CurrSBIndex..1.i, 12
  %"&pSB[currWI].offset30.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)29.i"
  %CastToValueType31.i = bitcast i8* %"&pSB[currWI].offset30.i" to i32*
  store i32 %58, i32* %CastToValueType31.i, align 4
  %59 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %60 = load i64* %59, align 8
  %61 = add i64 %60, %51
  %62 = trunc i64 %61 to i32
  %"&(pSB[currWI].offset)43.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset44.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)43.i"
  %CastToValueType45.i = bitcast i8* %"&pSB[currWI].offset44.i" to i32*
  store i32 %62, i32* %CastToValueType45.i, align 4
  %63 = sext i32 %53 to i64
  %"&(pSB[currWI].offset)62.i" = add nuw i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset63.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)62.i"
  %CastToValueType64.i = bitcast i8* %"&pSB[currWI].offset63.i" to i64*
  store i64 %63, i64* %CastToValueType64.i, align 8
  %64 = getelementptr inbounds float addrspace(1)* %4, i64 %63
  %65 = load float addrspace(1)* %64, align 4
  %66 = sext i32 %52 to i64
  %67 = getelementptr inbounds float addrspace(3)* %22, i64 %66
  %"&(pSB[currWI].offset)71.i" = add nuw i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset72.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)71.i"
  %CastToValueType73.i = bitcast i8* %"&pSB[currWI].offset72.i" to float addrspace(3)**
  store float addrspace(3)* %67, float addrspace(3)** %CastToValueType73.i, align 8
  store float %65, float addrspace(3)* %67, align 4
  %68 = icmp slt i32 %62, %10
  %"&(pSB[currWI].offset)80.i" = add nuw i64 %CurrSBIndex..1.i, 40
  %"&pSB[currWI].offset81.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)80.i"
  %CastToValueType82.i = bitcast i8* %"&pSB[currWI].offset81.i" to i1*
  store i1 %68, i1* %CastToValueType82.i, align 1
  br i1 %68, label %69, label %75

; <label>:69                                      ; preds = %49
  %"&(pSB[currWI].offset)33.i" = add nuw i64 %CurrSBIndex..1.i, 12
  %"&pSB[currWI].offset34.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)33.i"
  %CastToValueType35.i = bitcast i8* %"&pSB[currWI].offset34.i" to i32*
  %loadedValue36.i = load i32* %CastToValueType35.i, align 4
  %70 = sext i32 %loadedValue36.i to i64
  %71 = getelementptr inbounds float addrspace(1)* %4, i64 %70
  %72 = load float addrspace(1)* %71, align 4
  %"&(pSB[currWI].offset)47.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset48.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)47.i"
  %CastToValueType49.i = bitcast i8* %"&pSB[currWI].offset48.i" to i32*
  %loadedValue50.i = load i32* %CastToValueType49.i, align 4
  %73 = sext i32 %loadedValue50.i to i64
  %74 = getelementptr inbounds float addrspace(3)* %22, i64 %73
  store float %72, float addrspace(3)* %74, align 4
  br label %78

; <label>:75                                      ; preds = %49
  %"&(pSB[currWI].offset)52.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset53.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)52.i"
  %CastToValueType54.i = bitcast i8* %"&pSB[currWI].offset53.i" to i32*
  %loadedValue55.i = load i32* %CastToValueType54.i, align 4
  %76 = sext i32 %loadedValue55.i to i64
  %77 = getelementptr inbounds float addrspace(3)* %22, i64 %76
  store float 0.000000e+00, float addrspace(3)* %77, align 4
  br label %78

; <label>:78                                      ; preds = %75, %69
  %79 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %80 = load i64* %79, align 8
  %81 = trunc i64 %80 to i32
  %82 = icmp sgt i32 %81, 0
  br i1 %82, label %bb.nph5.i, label %._crit_edge6.i

bb.nph5.i:                                        ; preds = %78
  %"&(pSB[currWI].offset)10.i" = add nuw i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset11.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)10.i"
  %CastToValueType12.i = bitcast i8* %"&pSB[currWI].offset11.i" to i32*
  %loadedValue.i = load i32* %CastToValueType12.i, align 4
  %83 = shl i32 %loadedValue.i, 1
  %"&(pSB[currWI].offset)89.i" = add nuw i64 %CurrSBIndex..1.i, 44
  %"&pSB[currWI].offset90.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)89.i"
  %CastToValueType91.i = bitcast i8* %"&pSB[currWI].offset90.i" to i32*
  store i32 %83, i32* %CastToValueType91.i, align 4
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %97, %bb.nph5.i
  %CurrWI..3.i = phi i64 [ %CurrWI..1.i, %bb.nph5.i ], [ %CurrWI..2.i, %97 ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..1.i, %bb.nph5.i ], [ %CurrSBIndex..2.i, %97 ]
  %currBarrier.1.i = phi i32 [ %currBarrier.3.i, %bb.nph5.i ], [ %currBarrier.0.i, %97 ]
  %d.04.i = phi i32 [ %81, %bb.nph5.i ], [ %99, %97 ]
  %stride.03.i = phi i32 [ 1, %bb.nph5.i ], [ %98, %97 ]
  %"&(pSB[currWI].offset)112.i" = add nuw i64 %CurrSBIndex..3.i, 52
  %"&pSB[currWI].offset113.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)112.i"
  %CastToValueType114.i = bitcast i8* %"&pSB[currWI].offset113.i" to i32*
  store i32 %stride.03.i, i32* %CastToValueType114.i, align 4
  %"&(pSB[currWI].offset)98.i" = add nuw i64 %CurrSBIndex..3.i, 48
  %"&pSB[currWI].offset99.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)98.i"
  %CastToValueType100.i = bitcast i8* %"&pSB[currWI].offset99.i" to i32*
  store i32 %d.04.i, i32* %CastToValueType100.i, align 4
  %check.WI.iter207.i = icmp ult i64 %CurrWI..3.i, %34
  br i1 %check.WI.iter207.i, label %thenBB204.i, label %SyncBB198.i

thenBB204.i:                                      ; preds = %"Barrier BB.i"
  %"CurrWI++208.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride210.i" = add nuw i64 %CurrSBIndex..3.i, 88
  %cond3.i = icmp eq i32 %currBarrier.1.i, 8
  br i1 %cond3.i, label %SyncBB202.i, label %SyncBB198.i

SyncBB198.i:                                      ; preds = %thenBB211.i, %thenBB204.i, %"Barrier BB.i"
  %CurrWI..2.i = phi i64 [ %"CurrWI++208.i", %thenBB204.i ], [ %"CurrWI++215.i", %thenBB211.i ], [ 0, %"Barrier BB.i" ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride210.i", %thenBB204.i ], [ %"loadedCurrSB+Stride217.i", %thenBB211.i ], [ 0, %"Barrier BB.i" ]
  %currBarrier.0.i = phi i32 [ %currBarrier.1.i, %thenBB204.i ], [ %currBarrier.2.i, %thenBB211.i ], [ 2, %"Barrier BB.i" ]
  %"&(pSB[currWI].offset)19.i" = add nuw i64 %CurrSBIndex..2.i, 8
  %"&pSB[currWI].offset20.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)19.i"
  %CastToValueType21.i = bitcast i8* %"&pSB[currWI].offset20.i" to i32*
  %loadedValue22.i = load i32* %CastToValueType21.i, align 4
  %"&(pSB[currWI].offset)102.i" = add nuw i64 %CurrSBIndex..2.i, 48
  %"&pSB[currWI].offset103.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)102.i"
  %CastToValueType104.i = bitcast i8* %"&pSB[currWI].offset103.i" to i32*
  %loadedValue105.i = load i32* %CastToValueType104.i, align 4
  %84 = icmp slt i32 %loadedValue22.i, %loadedValue105.i
  br i1 %84, label %85, label %97

; <label>:85                                      ; preds = %SyncBB198.i
  %"&(pSB[currWI].offset)93.i" = add nuw i64 %CurrSBIndex..2.i, 44
  %"&pSB[currWI].offset94.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)93.i"
  %CastToValueType95.i = bitcast i8* %"&pSB[currWI].offset94.i" to i32*
  %loadedValue96.i = load i32* %CastToValueType95.i, align 4
  %"&(pSB[currWI].offset)121.i" = add nuw i64 %CurrSBIndex..2.i, 52
  %"&pSB[currWI].offset122.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)121.i"
  %CastToValueType123.i = bitcast i8* %"&pSB[currWI].offset122.i" to i32*
  %loadedValue124.i = load i32* %CastToValueType123.i, align 4
  %86 = mul i32 %loadedValue96.i, %loadedValue124.i
  %"&(pSB[currWI].offset)126.i" = add nuw i64 %CurrSBIndex..2.i, 52
  %"&pSB[currWI].offset127.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)126.i"
  %CastToValueType128.i = bitcast i8* %"&pSB[currWI].offset127.i" to i32*
  %loadedValue129.i = load i32* %CastToValueType128.i, align 4
  %87 = add i32 %loadedValue129.i, -1
  %88 = add i32 %87, %86
  %"&(pSB[currWI].offset)131.i" = add nuw i64 %CurrSBIndex..2.i, 52
  %"&pSB[currWI].offset132.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)131.i"
  %CastToValueType133.i = bitcast i8* %"&pSB[currWI].offset132.i" to i32*
  %loadedValue134.i = load i32* %CastToValueType133.i, align 4
  %89 = add i32 %88, %loadedValue134.i
  %90 = sext i32 %88 to i64
  %91 = getelementptr inbounds float addrspace(3)* %22, i64 %90
  %92 = load float addrspace(3)* %91, align 4
  %93 = sext i32 %89 to i64
  %94 = getelementptr inbounds float addrspace(3)* %22, i64 %93
  %95 = load float addrspace(3)* %94, align 4
  %96 = fadd float %95, %92
  store float %96, float addrspace(3)* %94, align 4
  br label %97

; <label>:97                                      ; preds = %85, %SyncBB198.i
  %"&(pSB[currWI].offset)116.i" = add nuw i64 %CurrSBIndex..2.i, 52
  %"&pSB[currWI].offset117.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)116.i"
  %CastToValueType118.i = bitcast i8* %"&pSB[currWI].offset117.i" to i32*
  %loadedValue119.i = load i32* %CastToValueType118.i, align 4
  %98 = shl i32 %loadedValue119.i, 1
  %"&(pSB[currWI].offset)136.i" = add nuw i64 %CurrSBIndex..2.i, 56
  %"&pSB[currWI].offset137.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)136.i"
  %CastToValueType138.i = bitcast i8* %"&pSB[currWI].offset137.i" to i32*
  store i32 %98, i32* %CastToValueType138.i, align 4
  %"&(pSB[currWI].offset)107.i" = add nuw i64 %CurrSBIndex..2.i, 48
  %"&pSB[currWI].offset108.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)107.i"
  %CastToValueType109.i = bitcast i8* %"&pSB[currWI].offset108.i" to i32*
  %loadedValue110.i = load i32* %CastToValueType109.i, align 4
  %99 = ashr i32 %loadedValue110.i, 1
  %"&(pSB[currWI].offset)140.i" = add nuw i64 %CurrSBIndex..2.i, 60
  %"&pSB[currWI].offset141.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)140.i"
  %CastToValueType142.i = bitcast i8* %"&pSB[currWI].offset141.i" to i32*
  store i32 %99, i32* %CastToValueType142.i, align 4
  %100 = icmp sgt i32 %99, 0
  br i1 %100, label %"Barrier BB.i", label %._crit_edge6.i

._crit_edge6.i:                                   ; preds = %97, %78
  %CurrWI..4.i = phi i64 [ %CurrWI..2.i, %97 ], [ %CurrWI..1.i, %78 ]
  %CurrSBIndex..4.i = phi i64 [ %CurrSBIndex..2.i, %97 ], [ %CurrSBIndex..1.i, %78 ]
  %currBarrier.2.i = phi i32 [ %currBarrier.0.i, %97 ], [ %currBarrier.3.i, %78 ]
  %stride.0.lcssa.i = phi i32 [ %98, %97 ], [ 1, %78 ]
  %"&(pSB[currWI].offset)144.i" = add nuw i64 %CurrSBIndex..4.i, 64
  %"&pSB[currWI].offset145.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)144.i"
  %CastToValueType146.i = bitcast i8* %"&pSB[currWI].offset145.i" to i32*
  store i32 %stride.0.lcssa.i, i32* %CastToValueType146.i, align 4
  br i1 %39, label %101, label %103

; <label>:101                                     ; preds = %._crit_edge6.i
  %102 = load i64* %28, align 8
  br label %103

; <label>:103                                     ; preds = %101, %._crit_edge6.i
  %104 = phi i64 [ %102, %101 ], [ %41, %._crit_edge6.i ]
  %105 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..4.i, i32 0, i64 0
  %106 = load i64* %105, align 8
  %107 = icmp eq i64 %106, 0
  br i1 %107, label %108, label %122

; <label>:108                                     ; preds = %103
  %109 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %110 = load i64* %109, align 8
  %111 = shl i64 %110, 1
  %112 = add i64 %111, 4294967295
  %113 = trunc i64 %112 to i32
  br i1 %40, label %114, label %._crit_edge7.i

; <label>:114                                     ; preds = %108
  %115 = sext i32 %113 to i64
  %116 = getelementptr inbounds float addrspace(3)* %22, i64 %115
  %117 = load float addrspace(3)* %116, align 4
  %sext.i = shl i64 %104, 32
  %118 = ashr i64 %sext.i, 32
  %119 = getelementptr inbounds float addrspace(1)* %7, i64 %118
  store float %117, float addrspace(1)* %119, align 4
  br label %._crit_edge7.i

._crit_edge7.i:                                   ; preds = %114, %108
  %120 = sext i32 %113 to i64
  %121 = getelementptr inbounds float addrspace(3)* %22, i64 %120
  store float 0.000000e+00, float addrspace(3)* %121, align 4
  br label %122

; <label>:122                                     ; preds = %._crit_edge7.i, %103
  %check.WI.iter214.i = icmp ult i64 %CurrWI..4.i, %34
  br i1 %check.WI.iter214.i, label %thenBB211.i, label %SyncBB199.i

thenBB211.i:                                      ; preds = %122
  %"CurrWI++215.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride217.i" = add nuw i64 %CurrSBIndex..4.i, 88
  %cond2.i = icmp eq i32 %currBarrier.2.i, 2
  br i1 %cond2.i, label %SyncBB198.i, label %SyncBB202.i

SyncBB199.i:                                      ; preds = %thenBB227.i, %thenBB219.i, %122
  %CurrWI..5.i = phi i64 [ 0, %122 ], [ %"CurrWI++231.i", %thenBB227.i ], [ %"CurrWI++223.i", %thenBB219.i ]
  %CurrSBIndex..5.i = phi i64 [ 0, %122 ], [ %"loadedCurrSB+Stride233.i", %thenBB227.i ], [ %"loadedCurrSB+Stride225.i", %thenBB219.i ]
  %currBarrier.7.i = phi i32 [ 3, %122 ], [ %currBarrier.6.i, %thenBB227.i ], [ %currBarrier.5.i, %thenBB219.i ]
  %123 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %124 = load i64* %123, align 8
  %125 = icmp eq i64 %124, 0
  %"&(pSB[currWI].offset)148.i" = add nuw i64 %CurrSBIndex..5.i, 64
  br i1 %125, label %._crit_edge.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %142, %SyncBB199.i
  %CurrWI..7.i = phi i64 [ %CurrWI..6.i, %142 ], [ %CurrWI..5.i, %SyncBB199.i ]
  %CurrSBIndex..7.i = phi i64 [ %CurrSBIndex..6.i, %142 ], [ %CurrSBIndex..5.i, %SyncBB199.i ]
  %currBarrier.5.i = phi i32 [ %currBarrier.4.i, %142 ], [ %currBarrier.7.i, %SyncBB199.i ]
  %d1.02.i = phi i32 [ %143, %142 ], [ 1, %SyncBB199.i ]
  %"&(pSB[currWI].offset)180.pn.i" = phi i64 [ %"&(pSB[currWI].offset)180.i", %142 ], [ %"&(pSB[currWI].offset)148.i", %SyncBB199.i ]
  %stride.11.in.in.i = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)180.pn.i"
  %stride.11.in.i = bitcast i8* %stride.11.in.in.i to i32*
  %stride.11.i = load i32* %stride.11.in.i, align 4
  %"&(pSB[currWI].offset)167.i" = add nuw i64 %CurrSBIndex..7.i, 72
  %"&pSB[currWI].offset168.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)167.i"
  %CastToValueType169.i = bitcast i8* %"&pSB[currWI].offset168.i" to i32*
  store i32 %stride.11.i, i32* %CastToValueType169.i, align 4
  %"&(pSB[currWI].offset)153.i" = add nuw i64 %CurrSBIndex..7.i, 68
  %"&pSB[currWI].offset154.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)153.i"
  %CastToValueType155.i = bitcast i8* %"&pSB[currWI].offset154.i" to i32*
  store i32 %d1.02.i, i32* %CastToValueType155.i, align 4
  %126 = lshr i32 %stride.11.i, 1
  %"&(pSB[currWI].offset)176.i" = add nuw i64 %CurrSBIndex..7.i, 76
  %"&pSB[currWI].offset177.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)176.i"
  %CastToValueType178.i = bitcast i8* %"&pSB[currWI].offset177.i" to i32*
  store i32 %126, i32* %CastToValueType178.i, align 4
  %check.WI.iter222.i = icmp ult i64 %CurrWI..7.i, %34
  br i1 %check.WI.iter222.i, label %thenBB219.i, label %SyncBB200.i

thenBB219.i:                                      ; preds = %bb.nph.i
  %"CurrWI++223.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride225.i" = add nuw i64 %CurrSBIndex..7.i, 88
  %cond1.i = icmp eq i32 %currBarrier.5.i, 4
  br i1 %cond1.i, label %SyncBB200.i, label %SyncBB199.i

SyncBB200.i:                                      ; preds = %thenBB227.i, %thenBB219.i, %bb.nph.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++231.i", %thenBB227.i ], [ %"CurrWI++223.i", %thenBB219.i ], [ 0, %bb.nph.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride233.i", %thenBB227.i ], [ %"loadedCurrSB+Stride225.i", %thenBB219.i ], [ 0, %bb.nph.i ]
  %currBarrier.4.i = phi i32 [ %currBarrier.6.i, %thenBB227.i ], [ %currBarrier.5.i, %thenBB219.i ], [ 4, %bb.nph.i ]
  %"&(pSB[currWI].offset)24.i" = add nuw i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset25.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)24.i"
  %CastToValueType26.i = bitcast i8* %"&pSB[currWI].offset25.i" to i32*
  %loadedValue27.i = load i32* %CastToValueType26.i, align 4
  %"&(pSB[currWI].offset)157.i" = add nuw i64 %CurrSBIndex..6.i, 68
  %"&pSB[currWI].offset158.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)157.i"
  %CastToValueType159.i = bitcast i8* %"&pSB[currWI].offset158.i" to i32*
  %loadedValue160.i = load i32* %CastToValueType159.i, align 4
  %127 = icmp slt i32 %loadedValue27.i, %loadedValue160.i
  br i1 %127, label %128, label %142

; <label>:128                                     ; preds = %SyncBB200.i
  %"&(pSB[currWI].offset)171.i" = add nuw i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset172.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)171.i"
  %CastToValueType173.i = bitcast i8* %"&pSB[currWI].offset172.i" to i32*
  %loadedValue174.i = load i32* %CastToValueType173.i, align 4
  %129 = and i32 %loadedValue174.i, -2
  %"&(pSB[currWI].offset)14.i" = add nuw i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset15.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)14.i"
  %CastToValueType16.i = bitcast i8* %"&pSB[currWI].offset15.i" to i32*
  %loadedValue17.i = load i32* %CastToValueType16.i, align 4
  %130 = mul i32 %129, %loadedValue17.i
  %"&(pSB[currWI].offset)190.i" = add nuw i64 %CurrSBIndex..6.i, 76
  %"&pSB[currWI].offset191.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)190.i"
  %CastToValueType192.i = bitcast i8* %"&pSB[currWI].offset191.i" to i32*
  %loadedValue193.i = load i32* %CastToValueType192.i, align 4
  %131 = add i32 %loadedValue193.i, -1
  %132 = add i32 %131, %130
  %"&(pSB[currWI].offset)185.i" = add nuw i64 %CurrSBIndex..6.i, 76
  %"&pSB[currWI].offset186.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)185.i"
  %CastToValueType187.i = bitcast i8* %"&pSB[currWI].offset186.i" to i32*
  %loadedValue188.i = load i32* %CastToValueType187.i, align 4
  %133 = add i32 %132, %loadedValue188.i
  %134 = sext i32 %132 to i64
  %135 = getelementptr inbounds float addrspace(3)* %22, i64 %134
  %136 = load float addrspace(3)* %135, align 4
  %137 = sext i32 %133 to i64
  %138 = getelementptr inbounds float addrspace(3)* %22, i64 %137
  %139 = load float addrspace(3)* %138, align 4
  store float %139, float addrspace(3)* %135, align 4
  %140 = load float addrspace(3)* %138, align 4
  %141 = fadd float %140, %136
  store float %141, float addrspace(3)* %138, align 4
  br label %142

; <label>:142                                     ; preds = %128, %SyncBB200.i
  %"&(pSB[currWI].offset)162.i" = add nuw i64 %CurrSBIndex..6.i, 68
  %"&pSB[currWI].offset163.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)162.i"
  %CastToValueType164.i = bitcast i8* %"&pSB[currWI].offset163.i" to i32*
  %loadedValue165.i = load i32* %CastToValueType164.i, align 4
  %143 = shl i32 %loadedValue165.i, 1
  %"&(pSB[currWI].offset)195.i" = add nuw i64 %CurrSBIndex..6.i, 80
  %"&pSB[currWI].offset196.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)195.i"
  %CastToValueType197.i = bitcast i8* %"&pSB[currWI].offset196.i" to i32*
  store i32 %143, i32* %CastToValueType197.i, align 4
  %144 = sext i32 %143 to i64
  %145 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %146 = load i64* %145, align 8
  %147 = icmp ugt i64 %144, %146
  %"&(pSB[currWI].offset)180.i" = add nuw i64 %CurrSBIndex..6.i, 76
  br i1 %147, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %142, %SyncBB199.i
  %CurrWI..8.i = phi i64 [ %CurrWI..5.i, %SyncBB199.i ], [ %CurrWI..6.i, %142 ]
  %CurrSBIndex..8.i = phi i64 [ %CurrSBIndex..5.i, %SyncBB199.i ], [ %CurrSBIndex..6.i, %142 ]
  %currBarrier.6.i = phi i32 [ %currBarrier.7.i, %SyncBB199.i ], [ %currBarrier.4.i, %142 ]
  %check.WI.iter230.i = icmp ult i64 %CurrWI..8.i, %34
  br i1 %check.WI.iter230.i, label %thenBB227.i, label %SyncBB201.i

thenBB227.i:                                      ; preds = %._crit_edge.i
  %"CurrWI++231.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride233.i" = add nuw i64 %CurrSBIndex..8.i, 88
  %cond.i = icmp eq i32 %currBarrier.6.i, 4
  br i1 %cond.i, label %SyncBB200.i, label %SyncBB199.i

SyncBB201.i:                                      ; preds = %thenBB.i, %._crit_edge.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %._crit_edge.i ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %._crit_edge.i ]
  %"&(pSB[currWI].offset)75.i" = add nuw i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset76.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)75.i"
  %CastToValueType77.i = bitcast i8* %"&pSB[currWI].offset76.i" to float addrspace(3)**
  %loadedValue78.i = load float addrspace(3)** %CastToValueType77.i, align 8
  %148 = load float addrspace(3)* %loadedValue78.i, align 4
  %"&(pSB[currWI].offset)66.i" = add nuw i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset67.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)66.i"
  %CastToValueType68.i = bitcast i8* %"&pSB[currWI].offset67.i" to i64*
  %loadedValue69.i = load i64* %CastToValueType68.i, align 8
  %149 = getelementptr inbounds float addrspace(1)* %1, i64 %loadedValue69.i
  store float %148, float addrspace(1)* %149, align 4
  %"&(pSB[currWI].offset)84.i" = add nuw i64 %CurrSBIndex..0.i, 40
  %"&pSB[currWI].offset85.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)84.i"
  %CastToValueType86.i = bitcast i8* %"&pSB[currWI].offset85.i" to i1*
  %loadedValue87.i = load i1* %CastToValueType86.i, align 1
  br i1 %loadedValue87.i, label %150, label %UnifiedReturnBlock.i

; <label>:150                                     ; preds = %SyncBB201.i
  %"&(pSB[currWI].offset)57.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset58.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)57.i"
  %CastToValueType59.i = bitcast i8* %"&pSB[currWI].offset58.i" to i32*
  %loadedValue60.i = load i32* %CastToValueType59.i, align 4
  %151 = sext i32 %loadedValue60.i to i64
  %152 = getelementptr inbounds float addrspace(3)* %22, i64 %151
  %153 = load float addrspace(3)* %152, align 4
  %"&(pSB[currWI].offset)38.i" = add nuw i64 %CurrSBIndex..0.i, 12
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)38.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to i32*
  %loadedValue41.i = load i32* %CastToValueType40.i, align 4
  %154 = sext i32 %loadedValue41.i to i64
  %155 = getelementptr inbounds float addrspace(1)* %1, i64 %154
  store float %153, float addrspace(1)* %155, align 4
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %150, %SyncBB201.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %__scan_separated_args.exit

thenBB.i:                                         ; preds = %UnifiedReturnBlock.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 88
  br label %SyncBB201.i

__scan_separated_args.exit:                       ; preds = %UnifiedReturnBlock.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__uniformAdd_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int, int, int", metadata !"opencl_uniformAdd_locals_anchor", void (i8*)* @uniformAdd}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i32, i32, i32, float addrspace(3)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__scan_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int, int, int, int, float __attribute__((address_space(3))) *", metadata !"opencl_scan_locals_anchor", void (i8*)* @scan}
