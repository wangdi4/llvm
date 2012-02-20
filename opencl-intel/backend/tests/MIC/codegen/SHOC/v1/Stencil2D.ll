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

@opencl_StencilKernel_local_sh = internal addrspace(3) global [18 x [18 x float]] zeroinitializer, align 16

declare void @__CopyRect_original(float addrspace(1)* nocapture, i32, i32, float addrspace(1)* nocapture, i32, i32, i32, i32) nounwind

declare i64 @get_group_id(i32)

declare i64 @get_local_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_local_size(i32)

declare fastcc i32 @__ToFlatIdx_original(i32, i32, i32) nounwind readnone inlinehint

declare void @__StencilKernel_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float, float, float) nounwind

declare i64 @get_num_groups(i32)

declare fastcc i32 @__ToGlobalRow_original(i32, i32) nounwind readnone inlinehint

declare fastcc i32 @__ToGlobalCol_original(i32, i32) nounwind readnone inlinehint

declare fastcc i32 @__ToFlatHaloedIdx_original(i32, i32, i32) nounwind readnone inlinehint

declare void @barrier(i64)

declare void @dummybarrier.()

declare i64* @get_curr_wi.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

define void @__CopyRect_separated_args(float addrspace(1)* nocapture %dest, i32 %doffset, i32 %dpitch, float addrspace(1)* nocapture %src, i32 %soffset, i32 %spitch, i32 %width, i32 %height, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  store i64 0, i64* %pCurrWI, align 8
  %0 = icmp sgt i32 %width, 0
  %1 = sext i32 %soffset to i64
  %2 = sext i32 %doffset to i64
  br label %SyncBB3

SyncBB3:                                          ; preds = %thenBB, %FirstBB
  %3 = load i64* %pWGId, align 8
  %4 = trunc i64 %3 to i32
  %currWI = load i64* %pCurrWI, align 8
  %5 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = trunc i64 %6 to i32
  %8 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %9 = load i64* %8, align 8
  %10 = trunc i64 %9 to i32
  %11 = mul nsw i32 %10, %4
  %12 = add nsw i32 %11, %7
  %13 = icmp slt i32 %12, %height
  %or.cond = and i1 %13, %0
  br i1 %or.cond, label %bb.nph, label %.loopexit

bb.nph:                                           ; preds = %SyncBB3, %bb.nph
  %14 = phi i32 [ %24, %bb.nph ], [ 0, %SyncBB3 ]
  %15 = mul nsw i32 %12, %spitch
  %16 = add nsw i32 %15, %14
  %17 = sext i32 %16 to i64
  %.sum = add i64 %17, %1
  %18 = getelementptr inbounds float addrspace(1)* %src, i64 %.sum
  %19 = load float addrspace(1)* %18, align 4
  %20 = mul nsw i32 %12, %dpitch
  %21 = add nsw i32 %20, %14
  %22 = sext i32 %21 to i64
  %.sum1 = add i64 %22, %2
  %23 = getelementptr inbounds float addrspace(1)* %dest, i64 %.sum1
  store float %19, float addrspace(1)* %23, align 4
  %24 = add nsw i32 %14, 1
  %exitcond = icmp eq i32 %24, %width
  br i1 %exitcond, label %.loopexit, label %bb.nph

.loopexit:                                        ; preds = %bb.nph, %SyncBB3
  %loadedCurrWI = load i64* %pCurrWI, align 8
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %.loopexit
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %pCurrWI, align 8
  br label %SyncBB3

elseBB:                                           ; preds = %.loopexit
  store i64 0, i64* %pCurrWI, align 8
  ret void
}

define void @__StencilKernel_separated_args(float addrspace(1)* nocapture %data, float addrspace(1)* nocapture %newData, float %wCenter, float %wCardinal, float %wDiagonal, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [18 x [18 x float]] addrspace(3)*
  %1 = bitcast i8 addrspace(3)* %pLocalMem to float addrspace(3)*
  %2 = getelementptr i8 addrspace(3)* %pLocalMem, i64 1224
  %3 = bitcast i8 addrspace(3)* %2 to float addrspace(3)*
  %4 = getelementptr i8 addrspace(3)* %pLocalMem, i64 68
  %5 = bitcast i8 addrspace(3)* %4 to float addrspace(3)*
  %6 = getelementptr i8 addrspace(3)* %pLocalMem, i64 1292
  %7 = bitcast i8 addrspace(3)* %6 to float addrspace(3)*
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB96

SyncBB96:                                         ; preds = %thenBB, %FirstBB
  %CurrSBIndex..0 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride", %thenBB ]
  %8 = getelementptr i64* %pWGId, i64 1
  %9 = load i64* %8, align 8
  %10 = trunc i64 %9 to i32
  %11 = load i64* %pWGId, align 8
  %12 = trunc i64 %11 to i32
  %13 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 0
  %14 = load i64* %13, align 8
  %15 = trunc i64 %14 to i32
  %currWI105 = load i64* %pCurrWI, align 8
  %16 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI105, i32 0, i64 1
  %17 = load i64* %16, align 8
  %18 = trunc i64 %17 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %18, i32* %CastToValueType, align 4
  %currWI = load i64* %pCurrWI, align 8
  %19 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI, i32 0, i64 0
  %20 = load i64* %19, align 8
  %21 = trunc i64 %20 to i32
  %"&(pSB[currWI].offset)112" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset12" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)112"
  %CastToValueType13 = bitcast i8* %"&pSB[currWI].offset12" to i32*
  store i32 %21, i32* %CastToValueType13, align 4
  %22 = shl i32 %10, 4
  %23 = add nsw i32 %22, %18
  %24 = shl i32 %12, 4
  %25 = add nsw i32 %24, %21
  %26 = shl i32 %15, 4
  %27 = add nsw i32 %23, 1
  %28 = or i32 %26, 2
  %29 = mul nsw i32 %28, %27
  %30 = add nsw i32 %25, 1
  %31 = add nsw i32 %30, %29
  %32 = sext i32 %31 to i64
  %"&(pSB[currWI].offset)30" = add nuw i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset31" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)30"
  %CastToValueType32 = bitcast i8* %"&pSB[currWI].offset31" to i64*
  store i64 %32, i64* %CastToValueType32, align 8
  %33 = getelementptr inbounds float addrspace(1)* %data, i64 %32
  %34 = load float addrspace(1)* %33, align 4
  %35 = add nsw i32 %21, 1
  %36 = sext i32 %35 to i64
  %"&(pSB[currWI].offset)39" = add nuw i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset40" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)39"
  %CastToValueType41 = bitcast i8* %"&pSB[currWI].offset40" to i64*
  store i64 %36, i64* %CastToValueType41, align 8
  %37 = add nsw i32 %18, 1
  %38 = sext i32 %37 to i64
  %"&(pSB[currWI].offset)63" = add nuw i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset64" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)63"
  %CastToValueType65 = bitcast i8* %"&pSB[currWI].offset64" to i64*
  store i64 %38, i64* %CastToValueType65, align 8
  %39 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %38, i64 %36
  %"&(pSB[currWI].offset)87" = add nuw i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset88" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)87"
  %CastToValueType89 = bitcast i8* %"&pSB[currWI].offset88" to float addrspace(3)**
  store float addrspace(3)* %39, float addrspace(3)** %CastToValueType89, align 8
  store float %34, float addrspace(3)* %39, align 4
  %40 = icmp eq i32 %18, 0
  br i1 %40, label %41, label %59

; <label>:41                                      ; preds = %SyncBB96
  %42 = or i32 %26, 2
  %43 = mul nsw i32 %42, %23
  %44 = add nsw i32 %25, 1
  %45 = add nsw i32 %44, %43
  %46 = sext i32 %45 to i64
  %47 = getelementptr inbounds float addrspace(1)* %data, i64 %46
  %48 = load float addrspace(1)* %47, align 4
  %"&(pSB[currWI].offset)58" = add nuw i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset59" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)58"
  %CastToValueType60 = bitcast i8* %"&pSB[currWI].offset59" to i64*
  %loadedValue61 = load i64* %CastToValueType60, align 8
  %49 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 0, i64 %loadedValue61
  store float %48, float addrspace(3)* %49, align 4
  %50 = add nsw i32 %23, 17
  %51 = or i32 %26, 2
  %52 = mul nsw i32 %51, %50
  %53 = add nsw i32 %25, 1
  %54 = add nsw i32 %53, %52
  %55 = sext i32 %54 to i64
  %56 = getelementptr inbounds float addrspace(1)* %data, i64 %55
  %57 = load float addrspace(1)* %56, align 4
  %"&(pSB[currWI].offset)53" = add nuw i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset54" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)53"
  %CastToValueType55 = bitcast i8* %"&pSB[currWI].offset54" to i64*
  %loadedValue56 = load i64* %CastToValueType55, align 8
  %58 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 17, i64 %loadedValue56
  store float %57, float addrspace(3)* %58, align 4
  br label %59

; <label>:59                                      ; preds = %41, %SyncBB96
  %"&(pSB[currWI].offset)203" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset21" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)203"
  %CastToValueType22 = bitcast i8* %"&pSB[currWI].offset21" to i32*
  %loadedValue23 = load i32* %CastToValueType22, align 4
  %60 = icmp eq i32 %loadedValue23, 0
  br i1 %60, label %61, label %79

; <label>:61                                      ; preds = %59
  %62 = add nsw i32 %23, 1
  %63 = or i32 %26, 2
  %64 = mul nsw i32 %63, %62
  %65 = add nsw i32 %25, %64
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds float addrspace(1)* %data, i64 %66
  %68 = load float addrspace(1)* %67, align 4
  %"&(pSB[currWI].offset)82" = add nuw i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset83" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)82"
  %CastToValueType84 = bitcast i8* %"&pSB[currWI].offset83" to i64*
  %loadedValue85 = load i64* %CastToValueType84, align 8
  %69 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %loadedValue85, i64 0
  store float %68, float addrspace(3)* %69, align 8
  %70 = add nsw i32 %23, 1
  %71 = or i32 %26, 2
  %72 = mul nsw i32 %71, %70
  %73 = add nsw i32 %25, 17
  %74 = add nsw i32 %73, %72
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds float addrspace(1)* %data, i64 %75
  %77 = load float addrspace(1)* %76, align 4
  %"&(pSB[currWI].offset)77" = add nuw i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset78" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)77"
  %CastToValueType79 = bitcast i8* %"&pSB[currWI].offset78" to i64*
  %loadedValue80 = load i64* %CastToValueType79, align 8
  %78 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %loadedValue80, i64 17
  store float %77, float addrspace(3)* %78, align 4
  br label %79

; <label>:79                                      ; preds = %61, %59
  %80 = or i64 %20, %17
  %81 = trunc i64 %80 to i32
  %82 = icmp eq i32 %81, 0
  br i1 %82, label %83, label %112

; <label>:83                                      ; preds = %79
  %84 = or i32 %26, 2
  %85 = mul nsw i32 %84, %23
  %86 = add nsw i32 %25, %85
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds float addrspace(1)* %data, i64 %87
  %89 = load float addrspace(1)* %88, align 4
  store float %89, float addrspace(3)* %1, align 16
  %90 = add nsw i32 %23, 17
  %91 = or i32 %26, 2
  %92 = mul nsw i32 %91, %90
  %93 = add nsw i32 %25, %92
  %94 = sext i32 %93 to i64
  %95 = getelementptr inbounds float addrspace(1)* %data, i64 %94
  %96 = load float addrspace(1)* %95, align 4
  store float %96, float addrspace(3)* %3, align 8
  %97 = or i32 %26, 2
  %98 = mul nsw i32 %97, %23
  %99 = add nsw i32 %25, 17
  %100 = add nsw i32 %99, %98
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds float addrspace(1)* %data, i64 %101
  %103 = load float addrspace(1)* %102, align 4
  store float %103, float addrspace(3)* %5, align 4
  %104 = add nsw i32 %23, 17
  %105 = or i32 %26, 2
  %106 = mul nsw i32 %105, %104
  %107 = add nsw i32 %25, 17
  %108 = add nsw i32 %107, %106
  %109 = sext i32 %108 to i64
  %110 = getelementptr inbounds float addrspace(1)* %data, i64 %109
  %111 = load float addrspace(1)* %110, align 4
  store float %111, float addrspace(3)* %7, align 4
  br label %112

; <label>:112                                     ; preds = %83, %79
  %loadedCurrWI = load i64* %pCurrWI, align 8
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %112
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 40
  br label %SyncBB96

elseBB:                                           ; preds = %112
  store i64 0, i64* %pCurrWI, align 8
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB98, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride104", %thenBB98 ]
  %"&(pSB[currWI].offset)91" = add nuw i64 %CurrSBIndex..1, 32
  %"&pSB[currWI].offset92" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)91"
  %CastToValueType93 = bitcast i8* %"&pSB[currWI].offset92" to float addrspace(3)**
  %loadedValue94 = load float addrspace(3)** %CastToValueType93, align 8
  %113 = load float addrspace(3)* %loadedValue94, align 4
  %"&pSB[currWI].offset7" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType8 = bitcast i8* %"&pSB[currWI].offset7" to i32*
  %loadedValue9 = load i32* %CastToValueType8, align 4
  %114 = sext i32 %loadedValue9 to i64
  %"&(pSB[currWI].offset)48" = add nuw i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset49" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)48"
  %CastToValueType50 = bitcast i8* %"&pSB[currWI].offset49" to i64*
  %loadedValue51 = load i64* %CastToValueType50, align 8
  %115 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %114, i64 %loadedValue51
  %116 = load float addrspace(3)* %115, align 4
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i32*
  %loadedValue = load i32* %CastToValueType4, align 4
  %117 = add nsw i32 %loadedValue, 2
  %118 = sext i32 %117 to i64
  %"&(pSB[currWI].offset)43" = add nuw i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset44" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)43"
  %CastToValueType45 = bitcast i8* %"&pSB[currWI].offset44" to i64*
  %loadedValue46 = load i64* %CastToValueType45, align 8
  %119 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %118, i64 %loadedValue46
  %120 = load float addrspace(3)* %119, align 4
  %121 = fadd float %116, %120
  %"&(pSB[currWI].offset)254" = or i64 %CurrSBIndex..1, 4
  %"&pSB[currWI].offset26" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)254"
  %CastToValueType27 = bitcast i8* %"&pSB[currWI].offset26" to i32*
  %loadedValue28 = load i32* %CastToValueType27, align 4
  %122 = sext i32 %loadedValue28 to i64
  %"&(pSB[currWI].offset)72" = add nuw i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset73" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)72"
  %CastToValueType74 = bitcast i8* %"&pSB[currWI].offset73" to i64*
  %loadedValue75 = load i64* %CastToValueType74, align 8
  %123 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %loadedValue75, i64 %122
  %124 = load float addrspace(3)* %123, align 4
  %125 = fadd float %121, %124
  %"&(pSB[currWI].offset)155" = or i64 %CurrSBIndex..1, 4
  %"&pSB[currWI].offset16" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)155"
  %CastToValueType17 = bitcast i8* %"&pSB[currWI].offset16" to i32*
  %loadedValue18 = load i32* %CastToValueType17, align 4
  %126 = add nsw i32 %loadedValue18, 2
  %127 = sext i32 %126 to i64
  %"&(pSB[currWI].offset)67" = add nuw i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset68" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)67"
  %CastToValueType69 = bitcast i8* %"&pSB[currWI].offset68" to i64*
  %loadedValue70 = load i64* %CastToValueType69, align 8
  %128 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %loadedValue70, i64 %127
  %129 = load float addrspace(3)* %128, align 4
  %130 = fadd float %125, %129
  %131 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %114, i64 %122
  %132 = load float addrspace(3)* %131, align 4
  %133 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %114, i64 %127
  %134 = load float addrspace(3)* %133, align 4
  %135 = fadd float %132, %134
  %136 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %118, i64 %122
  %137 = load float addrspace(3)* %136, align 4
  %138 = fadd float %135, %137
  %139 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %0, i64 0, i64 %118, i64 %127
  %140 = load float addrspace(3)* %139, align 4
  %141 = fadd float %138, %140
  %142 = fmul float %113, %wCenter
  %143 = fmul float %130, %wCardinal
  %144 = fadd float %142, %143
  %145 = fmul float %141, %wDiagonal
  %146 = fadd float %144, %145
  %"&(pSB[currWI].offset)34" = add nuw i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset35" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)34"
  %CastToValueType36 = bitcast i8* %"&pSB[currWI].offset35" to i64*
  %loadedValue37 = load i64* %CastToValueType36, align 8
  %147 = getelementptr inbounds float addrspace(1)* %newData, i64 %loadedValue37
  store float %146, float addrspace(1)* %147, align 4
  %loadedCurrWI100 = load i64* %pCurrWI, align 8
  %check.WI.iter101 = icmp ult i64 %loadedCurrWI100, %iterCount
  br i1 %check.WI.iter101, label %thenBB98, label %elseBB99

thenBB98:                                         ; preds = %SyncBB
  %"CurrWI++102" = add nuw i64 %loadedCurrWI100, 1
  store i64 %"CurrWI++102", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride104" = add nuw i64 %CurrSBIndex..1, 40
  br label %SyncBB

elseBB99:                                         ; preds = %SyncBB
  store i64 0, i64* %pCurrWI, align 8
  ret void
}

define void @StencilKernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float*
  %7 = load float* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to float*
  %10 = load float* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to float*
  %13 = load float* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i8 addrspace(3)**
  %16 = load i8 addrspace(3)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 40
  %18 = bitcast i8* %17 to %struct.WorkDim**
  %19 = load %struct.WorkDim** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 48
  %21 = bitcast i8* %20 to i64**
  %22 = load i64** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 64
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 80
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 88
  %30 = bitcast i8* %29 to i8**
  %31 = load i8** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 96
  %33 = bitcast i8* %32 to i64**
  %34 = load i64** %33, align 8
  %35 = bitcast i8 addrspace(3)* %16 to [18 x [18 x float]] addrspace(3)*
  %36 = bitcast i8 addrspace(3)* %16 to float addrspace(3)*
  %37 = getelementptr i8 addrspace(3)* %16, i64 1224
  %38 = bitcast i8 addrspace(3)* %37 to float addrspace(3)*
  %39 = getelementptr i8 addrspace(3)* %16, i64 68
  %40 = bitcast i8 addrspace(3)* %39 to float addrspace(3)*
  %41 = getelementptr i8 addrspace(3)* %16, i64 1292
  %42 = bitcast i8 addrspace(3)* %41 to float addrspace(3)*
  store i64 0, i64* %34, align 8
  br label %SyncBB96.i

SyncBB96.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %43 = getelementptr i64* %22, i64 1
  %44 = load i64* %43, align 8
  %45 = trunc i64 %44 to i32
  %46 = load i64* %22, align 8
  %47 = trunc i64 %46 to i32
  %48 = getelementptr %struct.WorkDim* %19, i64 0, i32 4, i64 0
  %49 = load i64* %48, align 8
  %50 = trunc i64 %49 to i32
  %currWI105.i = load i64* %34, align 8
  %51 = getelementptr %struct.PaddedDimId* %25, i64 %currWI105.i, i32 0, i64 1
  %52 = load i64* %51, align 8
  %53 = trunc i64 %52 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %53, i32* %CastToValueType.i, align 4
  %currWI.i = load i64* %34, align 8
  %54 = getelementptr %struct.PaddedDimId* %25, i64 %currWI.i, i32 0, i64 0
  %55 = load i64* %54, align 8
  %56 = trunc i64 %55 to i32
  %"&(pSB[currWI].offset)112.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset12.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)112.i"
  %CastToValueType13.i = bitcast i8* %"&pSB[currWI].offset12.i" to i32*
  store i32 %56, i32* %CastToValueType13.i, align 4
  %57 = shl i32 %45, 4
  %58 = add nsw i32 %57, %53
  %59 = shl i32 %47, 4
  %60 = add nsw i32 %59, %56
  %61 = shl i32 %50, 4
  %62 = add nsw i32 %58, 1
  %63 = or i32 %61, 2
  %64 = mul nsw i32 %63, %62
  %65 = add nsw i32 %60, 1
  %66 = add nsw i32 %65, %64
  %67 = sext i32 %66 to i64
  %"&(pSB[currWI].offset)30.i" = add nuw i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset31.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)30.i"
  %CastToValueType32.i = bitcast i8* %"&pSB[currWI].offset31.i" to i64*
  store i64 %67, i64* %CastToValueType32.i, align 8
  %68 = getelementptr inbounds float addrspace(1)* %1, i64 %67
  %69 = load float addrspace(1)* %68, align 4
  %70 = add nsw i32 %56, 1
  %71 = sext i32 %70 to i64
  %"&(pSB[currWI].offset)39.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset40.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)39.i"
  %CastToValueType41.i = bitcast i8* %"&pSB[currWI].offset40.i" to i64*
  store i64 %71, i64* %CastToValueType41.i, align 8
  %72 = add nsw i32 %53, 1
  %73 = sext i32 %72 to i64
  %"&(pSB[currWI].offset)63.i" = add nuw i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset64.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)63.i"
  %CastToValueType65.i = bitcast i8* %"&pSB[currWI].offset64.i" to i64*
  store i64 %73, i64* %CastToValueType65.i, align 8
  %74 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %73, i64 %71
  %"&(pSB[currWI].offset)87.i" = add nuw i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset88.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)87.i"
  %CastToValueType89.i = bitcast i8* %"&pSB[currWI].offset88.i" to float addrspace(3)**
  store float addrspace(3)* %74, float addrspace(3)** %CastToValueType89.i, align 8
  store float %69, float addrspace(3)* %74, align 4
  %75 = icmp eq i32 %53, 0
  br i1 %75, label %76, label %94

; <label>:76                                      ; preds = %SyncBB96.i
  %77 = or i32 %61, 2
  %78 = mul nsw i32 %77, %58
  %79 = add nsw i32 %60, 1
  %80 = add nsw i32 %79, %78
  %81 = sext i32 %80 to i64
  %82 = getelementptr inbounds float addrspace(1)* %1, i64 %81
  %83 = load float addrspace(1)* %82, align 4
  %"&(pSB[currWI].offset)58.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset59.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)58.i"
  %CastToValueType60.i = bitcast i8* %"&pSB[currWI].offset59.i" to i64*
  %loadedValue61.i = load i64* %CastToValueType60.i, align 8
  %84 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 0, i64 %loadedValue61.i
  store float %83, float addrspace(3)* %84, align 4
  %85 = add nsw i32 %58, 17
  %86 = or i32 %61, 2
  %87 = mul nsw i32 %86, %85
  %88 = add nsw i32 %60, 1
  %89 = add nsw i32 %88, %87
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds float addrspace(1)* %1, i64 %90
  %92 = load float addrspace(1)* %91, align 4
  %"&(pSB[currWI].offset)53.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset54.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)53.i"
  %CastToValueType55.i = bitcast i8* %"&pSB[currWI].offset54.i" to i64*
  %loadedValue56.i = load i64* %CastToValueType55.i, align 8
  %93 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 17, i64 %loadedValue56.i
  store float %92, float addrspace(3)* %93, align 4
  br label %94

; <label>:94                                      ; preds = %76, %SyncBB96.i
  %"&(pSB[currWI].offset)203.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)203.i"
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i32*
  %loadedValue23.i = load i32* %CastToValueType22.i, align 4
  %95 = icmp eq i32 %loadedValue23.i, 0
  br i1 %95, label %96, label %114

; <label>:96                                      ; preds = %94
  %97 = add nsw i32 %58, 1
  %98 = or i32 %61, 2
  %99 = mul nsw i32 %98, %97
  %100 = add nsw i32 %60, %99
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds float addrspace(1)* %1, i64 %101
  %103 = load float addrspace(1)* %102, align 4
  %"&(pSB[currWI].offset)82.i" = add nuw i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset83.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)82.i"
  %CastToValueType84.i = bitcast i8* %"&pSB[currWI].offset83.i" to i64*
  %loadedValue85.i = load i64* %CastToValueType84.i, align 8
  %104 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %loadedValue85.i, i64 0
  store float %103, float addrspace(3)* %104, align 8
  %105 = add nsw i32 %58, 1
  %106 = or i32 %61, 2
  %107 = mul nsw i32 %106, %105
  %108 = add nsw i32 %60, 17
  %109 = add nsw i32 %108, %107
  %110 = sext i32 %109 to i64
  %111 = getelementptr inbounds float addrspace(1)* %1, i64 %110
  %112 = load float addrspace(1)* %111, align 4
  %"&(pSB[currWI].offset)77.i" = add nuw i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset78.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)77.i"
  %CastToValueType79.i = bitcast i8* %"&pSB[currWI].offset78.i" to i64*
  %loadedValue80.i = load i64* %CastToValueType79.i, align 8
  %113 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %loadedValue80.i, i64 17
  store float %112, float addrspace(3)* %113, align 4
  br label %114

; <label>:114                                     ; preds = %96, %94
  %115 = or i64 %55, %52
  %116 = trunc i64 %115 to i32
  %117 = icmp eq i32 %116, 0
  br i1 %117, label %118, label %147

; <label>:118                                     ; preds = %114
  %119 = or i32 %61, 2
  %120 = mul nsw i32 %119, %58
  %121 = add nsw i32 %60, %120
  %122 = sext i32 %121 to i64
  %123 = getelementptr inbounds float addrspace(1)* %1, i64 %122
  %124 = load float addrspace(1)* %123, align 4
  store float %124, float addrspace(3)* %36, align 16
  %125 = add nsw i32 %58, 17
  %126 = or i32 %61, 2
  %127 = mul nsw i32 %126, %125
  %128 = add nsw i32 %60, %127
  %129 = sext i32 %128 to i64
  %130 = getelementptr inbounds float addrspace(1)* %1, i64 %129
  %131 = load float addrspace(1)* %130, align 4
  store float %131, float addrspace(3)* %38, align 8
  %132 = or i32 %61, 2
  %133 = mul nsw i32 %132, %58
  %134 = add nsw i32 %60, 17
  %135 = add nsw i32 %134, %133
  %136 = sext i32 %135 to i64
  %137 = getelementptr inbounds float addrspace(1)* %1, i64 %136
  %138 = load float addrspace(1)* %137, align 4
  store float %138, float addrspace(3)* %40, align 4
  %139 = add nsw i32 %58, 17
  %140 = or i32 %61, 2
  %141 = mul nsw i32 %140, %139
  %142 = add nsw i32 %60, 17
  %143 = add nsw i32 %142, %141
  %144 = sext i32 %143 to i64
  %145 = getelementptr inbounds float addrspace(1)* %1, i64 %144
  %146 = load float addrspace(1)* %145, align 4
  store float %146, float addrspace(3)* %42, align 4
  br label %147

; <label>:147                                     ; preds = %118, %114
  %loadedCurrWI.i = load i64* %34, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %147
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %34, align 8
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 40
  br label %SyncBB96.i

elseBB.i:                                         ; preds = %147
  store i64 0, i64* %34, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB98.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride104.i", %thenBB98.i ]
  %"&(pSB[currWI].offset)91.i" = add nuw i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset92.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)91.i"
  %CastToValueType93.i = bitcast i8* %"&pSB[currWI].offset92.i" to float addrspace(3)**
  %loadedValue94.i = load float addrspace(3)** %CastToValueType93.i, align 8
  %148 = load float addrspace(3)* %loadedValue94.i, align 4
  %"&pSB[currWI].offset7.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType8.i = bitcast i8* %"&pSB[currWI].offset7.i" to i32*
  %loadedValue9.i = load i32* %CastToValueType8.i, align 4
  %149 = sext i32 %loadedValue9.i to i64
  %"&(pSB[currWI].offset)48.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)48.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to i64*
  %loadedValue51.i = load i64* %CastToValueType50.i, align 8
  %150 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %149, i64 %loadedValue51.i
  %151 = load float addrspace(3)* %150, align 4
  %"&pSB[currWI].offset3.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType4.i = bitcast i8* %"&pSB[currWI].offset3.i" to i32*
  %loadedValue.i = load i32* %CastToValueType4.i, align 4
  %152 = add nsw i32 %loadedValue.i, 2
  %153 = sext i32 %152 to i64
  %"&(pSB[currWI].offset)43.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset44.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)43.i"
  %CastToValueType45.i = bitcast i8* %"&pSB[currWI].offset44.i" to i64*
  %loadedValue46.i = load i64* %CastToValueType45.i, align 8
  %154 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %153, i64 %loadedValue46.i
  %155 = load float addrspace(3)* %154, align 4
  %156 = fadd float %151, %155
  %"&(pSB[currWI].offset)254.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)254.i"
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to i32*
  %loadedValue28.i = load i32* %CastToValueType27.i, align 4
  %157 = sext i32 %loadedValue28.i to i64
  %"&(pSB[currWI].offset)72.i" = add nuw i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)72.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to i64*
  %loadedValue75.i = load i64* %CastToValueType74.i, align 8
  %158 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %loadedValue75.i, i64 %157
  %159 = load float addrspace(3)* %158, align 4
  %160 = fadd float %156, %159
  %"&(pSB[currWI].offset)155.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset16.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)155.i"
  %CastToValueType17.i = bitcast i8* %"&pSB[currWI].offset16.i" to i32*
  %loadedValue18.i = load i32* %CastToValueType17.i, align 4
  %161 = add nsw i32 %loadedValue18.i, 2
  %162 = sext i32 %161 to i64
  %"&(pSB[currWI].offset)67.i" = add nuw i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset68.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)67.i"
  %CastToValueType69.i = bitcast i8* %"&pSB[currWI].offset68.i" to i64*
  %loadedValue70.i = load i64* %CastToValueType69.i, align 8
  %163 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %loadedValue70.i, i64 %162
  %164 = load float addrspace(3)* %163, align 4
  %165 = fadd float %160, %164
  %166 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %149, i64 %157
  %167 = load float addrspace(3)* %166, align 4
  %168 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %149, i64 %162
  %169 = load float addrspace(3)* %168, align 4
  %170 = fadd float %167, %169
  %171 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %153, i64 %157
  %172 = load float addrspace(3)* %171, align 4
  %173 = fadd float %170, %172
  %174 = getelementptr inbounds [18 x [18 x float]] addrspace(3)* %35, i64 0, i64 %153, i64 %162
  %175 = load float addrspace(3)* %174, align 4
  %176 = fadd float %173, %175
  %177 = fmul float %148, %7
  %178 = fmul float %165, %10
  %179 = fadd float %177, %178
  %180 = fmul float %176, %13
  %181 = fadd float %179, %180
  %"&(pSB[currWI].offset)34.i" = add nuw i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset35.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)34.i"
  %CastToValueType36.i = bitcast i8* %"&pSB[currWI].offset35.i" to i64*
  %loadedValue37.i = load i64* %CastToValueType36.i, align 8
  %182 = getelementptr inbounds float addrspace(1)* %4, i64 %loadedValue37.i
  store float %181, float addrspace(1)* %182, align 4
  %loadedCurrWI100.i = load i64* %34, align 8
  %check.WI.iter101.i = icmp ult i64 %loadedCurrWI100.i, %28
  br i1 %check.WI.iter101.i, label %thenBB98.i, label %__StencilKernel_separated_args.exit

thenBB98.i:                                       ; preds = %SyncBB.i
  %"CurrWI++102.i" = add nuw i64 %loadedCurrWI100.i, 1
  store i64 %"CurrWI++102.i", i64* %34, align 8
  %"loadedCurrSB+Stride104.i" = add nuw i64 %CurrSBIndex..1.i, 40
  br label %SyncBB.i

__StencilKernel_separated_args.exit:              ; preds = %SyncBB.i
  store i64 0, i64* %34, align 8
  ret void
}

define void @CopyRect(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 12
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 16
  %9 = bitcast i8* %8 to float addrspace(1)**
  %10 = load float addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 28
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 32
  %18 = bitcast i8* %17 to i32*
  %19 = load i32* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 36
  %21 = bitcast i8* %20 to i32*
  %22 = load i32* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 48
  %24 = bitcast i8* %23 to %struct.WorkDim**
  %25 = load %struct.WorkDim** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 56
  %27 = bitcast i8* %26 to i64**
  %28 = load i64** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 72
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 88
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i64**
  %37 = load i64** %36, align 8
  store i64 0, i64* %37, align 8
  %38 = icmp sgt i32 %19, 0
  %39 = sext i32 %13 to i64
  %40 = sext i32 %4 to i64
  br label %SyncBB3.i

SyncBB3.i:                                        ; preds = %thenBB.i, %entry
  %41 = load i64* %28, align 8
  %42 = trunc i64 %41 to i32
  %currWI.i = load i64* %37, align 8
  %43 = getelementptr %struct.PaddedDimId* %31, i64 %currWI.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = trunc i64 %44 to i32
  %46 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %47 = load i64* %46, align 8
  %48 = trunc i64 %47 to i32
  %49 = mul nsw i32 %48, %42
  %50 = add nsw i32 %49, %45
  %51 = icmp slt i32 %50, %22
  %or.cond.i = and i1 %51, %38
  br i1 %or.cond.i, label %bb.nph.i, label %.loopexit.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB3.i
  %52 = phi i32 [ %62, %bb.nph.i ], [ 0, %SyncBB3.i ]
  %53 = mul nsw i32 %50, %16
  %54 = add nsw i32 %53, %52
  %55 = sext i32 %54 to i64
  %.sum.i = add i64 %55, %39
  %56 = getelementptr inbounds float addrspace(1)* %10, i64 %.sum.i
  %57 = load float addrspace(1)* %56, align 4
  %58 = mul nsw i32 %50, %7
  %59 = add nsw i32 %58, %52
  %60 = sext i32 %59 to i64
  %.sum1.i = add i64 %60, %40
  %61 = getelementptr inbounds float addrspace(1)* %1, i64 %.sum1.i
  store float %57, float addrspace(1)* %61, align 4
  %62 = add nsw i32 %52, 1
  %exitcond.i = icmp eq i32 %62, %19
  br i1 %exitcond.i, label %.loopexit.i, label %bb.nph.i

.loopexit.i:                                      ; preds = %bb.nph.i, %SyncBB3.i
  %loadedCurrWI.i = load i64* %37, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %__CopyRect_separated_args.exit

thenBB.i:                                         ; preds = %.loopexit.i
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %37, align 8
  br label %SyncBB3.i

__CopyRect_separated_args.exit:                   ; preds = %.loopexit.i
  store i64 0, i64* %37, align 8
  ret void
}

!opencl.kernels = !{!0, !2}
!opencl_StencilKernel_locals_anchor = !{!3}

!0 = metadata !{void (float addrspace(1)*, i32, i32, float addrspace(1)*, i32, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__CopyRect_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int, int, float __attribute__((address_space(1))) *, int, int, int, int", metadata !"opencl_CopyRect_locals_anchor", void (i8*)* @CopyRect}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float, float, float, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__StencilKernel_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float, float, float", metadata !"opencl_StencilKernel_locals_anchor", void (i8*)* @StencilKernel}
!3 = metadata !{metadata !"opencl_StencilKernel_local_sh"}


