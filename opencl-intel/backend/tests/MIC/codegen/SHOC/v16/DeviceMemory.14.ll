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

@opencl_writeLocalMemory_local_lbuf = internal addrspace(3) global [4096 x float] zeroinitializer, align 16

declare void @__writeLocalMemory_original(float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_group_id(i32)

declare i64 @get_local_id(i32)

declare i64 @get_local_size(i32)

declare void @barrier(i64)

declare void @____Vectorized_.writeLocalMemory_original(float addrspace(1)* nocapture, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__writeLocalMemory_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph4:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [4096 x float] addrspace(3)*
  br label %SyncBB22

SyncBB22:                                         ; preds = %bb.nph4, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %bb.nph4 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph4 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %6, i32* %CastToValueType, align 4
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %8 = load i64* %7, align 8
  %9 = trunc i64 %8 to i32
  %"&(pSB[currWI].offset)121" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset13" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)121"
  %CastToValueType14 = bitcast i8* %"&pSB[currWI].offset13" to i32*
  store i32 %9, i32* %CastToValueType14, align 4
  %10 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %11 = load i64* %10, align 8
  %12 = sitofp i32 %6 to float
  br label %13

; <label>:13                                      ; preds = %13, %SyncBB22
  %s.03 = phi i32 [ %9, %SyncBB22 ], [ %79, %13 ]
  %j.02 = phi i32 [ 0, %SyncBB22 ], [ %77, %13 ]
  %14 = and i32 %s.03, 4095
  %15 = zext i32 %14 to i64
  %16 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %15
  store float %12, float addrspace(3)* %16, align 4
  %17 = add nsw i32 %s.03, 1
  %18 = and i32 %17, 4095
  %19 = zext i32 %18 to i64
  %20 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %19
  store float %12, float addrspace(3)* %20, align 4
  %21 = add nsw i32 %s.03, 2
  %22 = and i32 %21, 4095
  %23 = zext i32 %22 to i64
  %24 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %23
  store float %12, float addrspace(3)* %24, align 4
  %25 = add nsw i32 %s.03, 3
  %26 = and i32 %25, 4095
  %27 = zext i32 %26 to i64
  %28 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %27
  store float %12, float addrspace(3)* %28, align 4
  %29 = add nsw i32 %s.03, 4
  %30 = and i32 %29, 4095
  %31 = zext i32 %30 to i64
  %32 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %31
  store float %12, float addrspace(3)* %32, align 4
  %33 = add nsw i32 %s.03, 5
  %34 = and i32 %33, 4095
  %35 = zext i32 %34 to i64
  %36 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %35
  store float %12, float addrspace(3)* %36, align 4
  %37 = add nsw i32 %s.03, 6
  %38 = and i32 %37, 4095
  %39 = zext i32 %38 to i64
  %40 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %39
  store float %12, float addrspace(3)* %40, align 4
  %41 = add nsw i32 %s.03, 7
  %42 = and i32 %41, 4095
  %43 = zext i32 %42 to i64
  %44 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %43
  store float %12, float addrspace(3)* %44, align 4
  %45 = add nsw i32 %s.03, 8
  %46 = and i32 %45, 4095
  %47 = zext i32 %46 to i64
  %48 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %47
  store float %12, float addrspace(3)* %48, align 4
  %49 = add nsw i32 %s.03, 9
  %50 = and i32 %49, 4095
  %51 = zext i32 %50 to i64
  %52 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %51
  store float %12, float addrspace(3)* %52, align 4
  %53 = add nsw i32 %s.03, 10
  %54 = and i32 %53, 4095
  %55 = zext i32 %54 to i64
  %56 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %55
  store float %12, float addrspace(3)* %56, align 4
  %57 = add nsw i32 %s.03, 11
  %58 = and i32 %57, 4095
  %59 = zext i32 %58 to i64
  %60 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %59
  store float %12, float addrspace(3)* %60, align 4
  %61 = add nsw i32 %s.03, 12
  %62 = and i32 %61, 4095
  %63 = zext i32 %62 to i64
  %64 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %63
  store float %12, float addrspace(3)* %64, align 4
  %65 = add nsw i32 %s.03, 13
  %66 = and i32 %65, 4095
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %67
  store float %12, float addrspace(3)* %68, align 4
  %69 = add nsw i32 %s.03, 14
  %70 = and i32 %69, 4095
  %71 = zext i32 %70 to i64
  %72 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %71
  store float %12, float addrspace(3)* %72, align 4
  %73 = add nsw i32 %s.03, 15
  %74 = and i32 %73, 4095
  %75 = zext i32 %74 to i64
  %76 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %75
  store float %12, float addrspace(3)* %76, align 4
  %77 = add nsw i32 %j.02, 1
  %78 = add nsw i32 %s.03, 16
  %79 = and i32 %78, 4095
  %exitcond6 = icmp eq i32 %77, 3000
  br i1 %exitcond6, label %._crit_edge5, label %13

._crit_edge5:                                     ; preds = %13
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %._crit_edge5
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 192
  br label %SyncBB22

elseBB:                                           ; preds = %._crit_edge5
  %80 = trunc i64 %11 to i32
  %81 = icmp eq i32 %80, 0
  %82 = select i1 %81, i32 1, i32 %80
  %83 = sdiv i32 4096, %82
  %84 = icmp sgt i32 %83, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB25, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride31", %thenBB25 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++29", %thenBB25 ]
  br i1 %84, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %"&(pSB[currWI].offset)162" = or i64 %CurrSBIndex..1, 4
  %"&pSB[currWI].offset17" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)162"
  %CastToValueType18 = bitcast i8* %"&pSB[currWI].offset17" to i32*
  %loadedValue19 = load i32* %CastToValueType18, align 4
  %85 = sext i32 %loadedValue19 to i64
  %86 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %85
  %"&pSB[currWI].offset9" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType10 = bitcast i8* %"&pSB[currWI].offset9" to i32*
  %loadedValue = load i32* %CastToValueType10, align 4
  %87 = sext i32 %loadedValue to i64
  %88 = getelementptr inbounds float addrspace(1)* %output, i64 %87
  br label %89

; <label>:89                                      ; preds = %89, %bb.nph
  %j.11 = phi i32 [ 0, %bb.nph ], [ %91, %89 ]
  %90 = load float addrspace(3)* %86, align 4
  store float %90, float addrspace(1)* %88, align 4
  %91 = add nsw i32 %j.11, 1
  %exitcond = icmp eq i32 %91, %83
  br i1 %exitcond, label %._crit_edge, label %89

._crit_edge:                                      ; preds = %89, %SyncBB
  %check.WI.iter28 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter28, label %thenBB25, label %SyncBB23

thenBB25:                                         ; preds = %._crit_edge
  %"CurrWI++29" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride31" = add nuw i64 %CurrSBIndex..1, 192
  br label %SyncBB

SyncBB23:                                         ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.writeLocalMemory_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph4:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [4096 x float] addrspace(3)*
  br label %SyncBB323

SyncBB323:                                        ; preds = %bb.nph4, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %bb.nph4 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph4 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %broadcast1 = insertelement <16 x i64> undef, i64 %5, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %6 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %7 = trunc <16 x i64> %6 to <16 x i32>
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 64
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to <16 x i32>*
  store <16 x i32> %7, <16 x i32>* %CastToValueType, align 64
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %9 = load i64* %8, align 8
  %broadcast11 = insertelement <16 x i64> undef, i64 %9, i32 0
  %broadcast22 = shufflevector <16 x i64> %broadcast11, <16 x i64> undef, <16 x i32> zeroinitializer
  %10 = add <16 x i64> %broadcast22, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %11 = trunc <16 x i64> %10 to <16 x i32>
  %"&(pSB[currWI].offset)312" = add nuw i64 %CurrSBIndex..0, 128
  %"&pSB[currWI].offset313" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)312"
  %CastToValueType314 = bitcast i8* %"&pSB[currWI].offset313" to <16 x i32>*
  store <16 x i32> %11, <16 x i32>* %CastToValueType314, align 64
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %13 = load i64* %12, align 8
  %14 = sitofp <16 x i32> %7 to <16 x float>
  %extract18 = extractelement <16 x float> %14, i32 0
  %extract19 = extractelement <16 x float> %14, i32 1
  %extract20 = extractelement <16 x float> %14, i32 2
  %extract21 = extractelement <16 x float> %14, i32 3
  %extract22 = extractelement <16 x float> %14, i32 4
  %extract23 = extractelement <16 x float> %14, i32 5
  %extract24 = extractelement <16 x float> %14, i32 6
  %extract25 = extractelement <16 x float> %14, i32 7
  %extract26 = extractelement <16 x float> %14, i32 8
  %extract27 = extractelement <16 x float> %14, i32 9
  %extract28 = extractelement <16 x float> %14, i32 10
  %extract29 = extractelement <16 x float> %14, i32 11
  %extract30 = extractelement <16 x float> %14, i32 12
  %extract31 = extractelement <16 x float> %14, i32 13
  %extract32 = extractelement <16 x float> %14, i32 14
  %extract33 = extractelement <16 x float> %14, i32 15
  br label %15

; <label>:15                                      ; preds = %15, %SyncBB323
  %vectorPHI = phi <16 x i32> [ %11, %SyncBB323 ], [ %817, %15 ]
  %j.02 = phi i32 [ 0, %SyncBB323 ], [ %815, %15 ]
  %16 = and <16 x i32> %vectorPHI, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %17 = extractelement <16 x i32> %16, i32 0
  %18 = zext i32 %17 to i64
  %19 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %18
  %20 = extractelement <16 x i32> %16, i32 1
  %21 = zext i32 %20 to i64
  %22 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %21
  %23 = extractelement <16 x i32> %16, i32 2
  %24 = zext i32 %23 to i64
  %25 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %24
  %26 = extractelement <16 x i32> %16, i32 3
  %27 = zext i32 %26 to i64
  %28 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %27
  %29 = extractelement <16 x i32> %16, i32 4
  %30 = zext i32 %29 to i64
  %31 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %30
  %32 = extractelement <16 x i32> %16, i32 5
  %33 = zext i32 %32 to i64
  %34 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %33
  %35 = extractelement <16 x i32> %16, i32 6
  %36 = zext i32 %35 to i64
  %37 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %36
  %38 = extractelement <16 x i32> %16, i32 7
  %39 = zext i32 %38 to i64
  %40 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %39
  %41 = extractelement <16 x i32> %16, i32 8
  %42 = zext i32 %41 to i64
  %43 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %42
  %44 = extractelement <16 x i32> %16, i32 9
  %45 = zext i32 %44 to i64
  %46 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %45
  %47 = extractelement <16 x i32> %16, i32 10
  %48 = zext i32 %47 to i64
  %49 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %48
  %50 = extractelement <16 x i32> %16, i32 11
  %51 = zext i32 %50 to i64
  %52 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %51
  %53 = extractelement <16 x i32> %16, i32 12
  %54 = zext i32 %53 to i64
  %55 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %54
  %56 = extractelement <16 x i32> %16, i32 13
  %57 = zext i32 %56 to i64
  %58 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %57
  %59 = extractelement <16 x i32> %16, i32 14
  %60 = zext i32 %59 to i64
  %61 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %60
  %62 = extractelement <16 x i32> %16, i32 15
  %63 = zext i32 %62 to i64
  %64 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %63
  store float %extract18, float addrspace(3)* %19, align 4
  store float %extract19, float addrspace(3)* %22, align 4
  store float %extract20, float addrspace(3)* %25, align 4
  store float %extract21, float addrspace(3)* %28, align 4
  store float %extract22, float addrspace(3)* %31, align 4
  store float %extract23, float addrspace(3)* %34, align 4
  store float %extract24, float addrspace(3)* %37, align 4
  store float %extract25, float addrspace(3)* %40, align 4
  store float %extract26, float addrspace(3)* %43, align 4
  store float %extract27, float addrspace(3)* %46, align 4
  store float %extract28, float addrspace(3)* %49, align 4
  store float %extract29, float addrspace(3)* %52, align 4
  store float %extract30, float addrspace(3)* %55, align 4
  store float %extract31, float addrspace(3)* %58, align 4
  store float %extract32, float addrspace(3)* %61, align 4
  store float %extract33, float addrspace(3)* %64, align 4
  %65 = add nsw <16 x i32> %vectorPHI, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %66 = and <16 x i32> %65, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %67 = extractelement <16 x i32> %66, i32 0
  %68 = zext i32 %67 to i64
  %69 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %68
  %70 = extractelement <16 x i32> %66, i32 1
  %71 = zext i32 %70 to i64
  %72 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %71
  %73 = extractelement <16 x i32> %66, i32 2
  %74 = zext i32 %73 to i64
  %75 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %74
  %76 = extractelement <16 x i32> %66, i32 3
  %77 = zext i32 %76 to i64
  %78 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %77
  %79 = extractelement <16 x i32> %66, i32 4
  %80 = zext i32 %79 to i64
  %81 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %80
  %82 = extractelement <16 x i32> %66, i32 5
  %83 = zext i32 %82 to i64
  %84 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %83
  %85 = extractelement <16 x i32> %66, i32 6
  %86 = zext i32 %85 to i64
  %87 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %86
  %88 = extractelement <16 x i32> %66, i32 7
  %89 = zext i32 %88 to i64
  %90 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %89
  %91 = extractelement <16 x i32> %66, i32 8
  %92 = zext i32 %91 to i64
  %93 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %92
  %94 = extractelement <16 x i32> %66, i32 9
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %95
  %97 = extractelement <16 x i32> %66, i32 10
  %98 = zext i32 %97 to i64
  %99 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %98
  %100 = extractelement <16 x i32> %66, i32 11
  %101 = zext i32 %100 to i64
  %102 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %101
  %103 = extractelement <16 x i32> %66, i32 12
  %104 = zext i32 %103 to i64
  %105 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %104
  %106 = extractelement <16 x i32> %66, i32 13
  %107 = zext i32 %106 to i64
  %108 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %107
  %109 = extractelement <16 x i32> %66, i32 14
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %110
  %112 = extractelement <16 x i32> %66, i32 15
  %113 = zext i32 %112 to i64
  %114 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %113
  store float %extract18, float addrspace(3)* %69, align 4
  store float %extract19, float addrspace(3)* %72, align 4
  store float %extract20, float addrspace(3)* %75, align 4
  store float %extract21, float addrspace(3)* %78, align 4
  store float %extract22, float addrspace(3)* %81, align 4
  store float %extract23, float addrspace(3)* %84, align 4
  store float %extract24, float addrspace(3)* %87, align 4
  store float %extract25, float addrspace(3)* %90, align 4
  store float %extract26, float addrspace(3)* %93, align 4
  store float %extract27, float addrspace(3)* %96, align 4
  store float %extract28, float addrspace(3)* %99, align 4
  store float %extract29, float addrspace(3)* %102, align 4
  store float %extract30, float addrspace(3)* %105, align 4
  store float %extract31, float addrspace(3)* %108, align 4
  store float %extract32, float addrspace(3)* %111, align 4
  store float %extract33, float addrspace(3)* %114, align 4
  %115 = add nsw <16 x i32> %vectorPHI, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %116 = and <16 x i32> %115, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %117 = extractelement <16 x i32> %116, i32 0
  %118 = zext i32 %117 to i64
  %119 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %118
  %120 = extractelement <16 x i32> %116, i32 1
  %121 = zext i32 %120 to i64
  %122 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %121
  %123 = extractelement <16 x i32> %116, i32 2
  %124 = zext i32 %123 to i64
  %125 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %124
  %126 = extractelement <16 x i32> %116, i32 3
  %127 = zext i32 %126 to i64
  %128 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %127
  %129 = extractelement <16 x i32> %116, i32 4
  %130 = zext i32 %129 to i64
  %131 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %130
  %132 = extractelement <16 x i32> %116, i32 5
  %133 = zext i32 %132 to i64
  %134 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %133
  %135 = extractelement <16 x i32> %116, i32 6
  %136 = zext i32 %135 to i64
  %137 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %136
  %138 = extractelement <16 x i32> %116, i32 7
  %139 = zext i32 %138 to i64
  %140 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %139
  %141 = extractelement <16 x i32> %116, i32 8
  %142 = zext i32 %141 to i64
  %143 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %142
  %144 = extractelement <16 x i32> %116, i32 9
  %145 = zext i32 %144 to i64
  %146 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %145
  %147 = extractelement <16 x i32> %116, i32 10
  %148 = zext i32 %147 to i64
  %149 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %148
  %150 = extractelement <16 x i32> %116, i32 11
  %151 = zext i32 %150 to i64
  %152 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %151
  %153 = extractelement <16 x i32> %116, i32 12
  %154 = zext i32 %153 to i64
  %155 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %154
  %156 = extractelement <16 x i32> %116, i32 13
  %157 = zext i32 %156 to i64
  %158 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %157
  %159 = extractelement <16 x i32> %116, i32 14
  %160 = zext i32 %159 to i64
  %161 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %160
  %162 = extractelement <16 x i32> %116, i32 15
  %163 = zext i32 %162 to i64
  %164 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %163
  store float %extract18, float addrspace(3)* %119, align 4
  store float %extract19, float addrspace(3)* %122, align 4
  store float %extract20, float addrspace(3)* %125, align 4
  store float %extract21, float addrspace(3)* %128, align 4
  store float %extract22, float addrspace(3)* %131, align 4
  store float %extract23, float addrspace(3)* %134, align 4
  store float %extract24, float addrspace(3)* %137, align 4
  store float %extract25, float addrspace(3)* %140, align 4
  store float %extract26, float addrspace(3)* %143, align 4
  store float %extract27, float addrspace(3)* %146, align 4
  store float %extract28, float addrspace(3)* %149, align 4
  store float %extract29, float addrspace(3)* %152, align 4
  store float %extract30, float addrspace(3)* %155, align 4
  store float %extract31, float addrspace(3)* %158, align 4
  store float %extract32, float addrspace(3)* %161, align 4
  store float %extract33, float addrspace(3)* %164, align 4
  %165 = add nsw <16 x i32> %vectorPHI, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %166 = and <16 x i32> %165, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %167 = extractelement <16 x i32> %166, i32 0
  %168 = zext i32 %167 to i64
  %169 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %168
  %170 = extractelement <16 x i32> %166, i32 1
  %171 = zext i32 %170 to i64
  %172 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %171
  %173 = extractelement <16 x i32> %166, i32 2
  %174 = zext i32 %173 to i64
  %175 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %174
  %176 = extractelement <16 x i32> %166, i32 3
  %177 = zext i32 %176 to i64
  %178 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %177
  %179 = extractelement <16 x i32> %166, i32 4
  %180 = zext i32 %179 to i64
  %181 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %180
  %182 = extractelement <16 x i32> %166, i32 5
  %183 = zext i32 %182 to i64
  %184 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %183
  %185 = extractelement <16 x i32> %166, i32 6
  %186 = zext i32 %185 to i64
  %187 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %186
  %188 = extractelement <16 x i32> %166, i32 7
  %189 = zext i32 %188 to i64
  %190 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %189
  %191 = extractelement <16 x i32> %166, i32 8
  %192 = zext i32 %191 to i64
  %193 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %192
  %194 = extractelement <16 x i32> %166, i32 9
  %195 = zext i32 %194 to i64
  %196 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %195
  %197 = extractelement <16 x i32> %166, i32 10
  %198 = zext i32 %197 to i64
  %199 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %198
  %200 = extractelement <16 x i32> %166, i32 11
  %201 = zext i32 %200 to i64
  %202 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %201
  %203 = extractelement <16 x i32> %166, i32 12
  %204 = zext i32 %203 to i64
  %205 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %204
  %206 = extractelement <16 x i32> %166, i32 13
  %207 = zext i32 %206 to i64
  %208 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %207
  %209 = extractelement <16 x i32> %166, i32 14
  %210 = zext i32 %209 to i64
  %211 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %210
  %212 = extractelement <16 x i32> %166, i32 15
  %213 = zext i32 %212 to i64
  %214 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %213
  store float %extract18, float addrspace(3)* %169, align 4
  store float %extract19, float addrspace(3)* %172, align 4
  store float %extract20, float addrspace(3)* %175, align 4
  store float %extract21, float addrspace(3)* %178, align 4
  store float %extract22, float addrspace(3)* %181, align 4
  store float %extract23, float addrspace(3)* %184, align 4
  store float %extract24, float addrspace(3)* %187, align 4
  store float %extract25, float addrspace(3)* %190, align 4
  store float %extract26, float addrspace(3)* %193, align 4
  store float %extract27, float addrspace(3)* %196, align 4
  store float %extract28, float addrspace(3)* %199, align 4
  store float %extract29, float addrspace(3)* %202, align 4
  store float %extract30, float addrspace(3)* %205, align 4
  store float %extract31, float addrspace(3)* %208, align 4
  store float %extract32, float addrspace(3)* %211, align 4
  store float %extract33, float addrspace(3)* %214, align 4
  %215 = add nsw <16 x i32> %vectorPHI, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %216 = and <16 x i32> %215, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %217 = extractelement <16 x i32> %216, i32 0
  %218 = zext i32 %217 to i64
  %219 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %218
  %220 = extractelement <16 x i32> %216, i32 1
  %221 = zext i32 %220 to i64
  %222 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %221
  %223 = extractelement <16 x i32> %216, i32 2
  %224 = zext i32 %223 to i64
  %225 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %224
  %226 = extractelement <16 x i32> %216, i32 3
  %227 = zext i32 %226 to i64
  %228 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %227
  %229 = extractelement <16 x i32> %216, i32 4
  %230 = zext i32 %229 to i64
  %231 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %230
  %232 = extractelement <16 x i32> %216, i32 5
  %233 = zext i32 %232 to i64
  %234 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %233
  %235 = extractelement <16 x i32> %216, i32 6
  %236 = zext i32 %235 to i64
  %237 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %236
  %238 = extractelement <16 x i32> %216, i32 7
  %239 = zext i32 %238 to i64
  %240 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %239
  %241 = extractelement <16 x i32> %216, i32 8
  %242 = zext i32 %241 to i64
  %243 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %242
  %244 = extractelement <16 x i32> %216, i32 9
  %245 = zext i32 %244 to i64
  %246 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %245
  %247 = extractelement <16 x i32> %216, i32 10
  %248 = zext i32 %247 to i64
  %249 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %248
  %250 = extractelement <16 x i32> %216, i32 11
  %251 = zext i32 %250 to i64
  %252 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %251
  %253 = extractelement <16 x i32> %216, i32 12
  %254 = zext i32 %253 to i64
  %255 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %254
  %256 = extractelement <16 x i32> %216, i32 13
  %257 = zext i32 %256 to i64
  %258 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %257
  %259 = extractelement <16 x i32> %216, i32 14
  %260 = zext i32 %259 to i64
  %261 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %260
  %262 = extractelement <16 x i32> %216, i32 15
  %263 = zext i32 %262 to i64
  %264 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %263
  store float %extract18, float addrspace(3)* %219, align 4
  store float %extract19, float addrspace(3)* %222, align 4
  store float %extract20, float addrspace(3)* %225, align 4
  store float %extract21, float addrspace(3)* %228, align 4
  store float %extract22, float addrspace(3)* %231, align 4
  store float %extract23, float addrspace(3)* %234, align 4
  store float %extract24, float addrspace(3)* %237, align 4
  store float %extract25, float addrspace(3)* %240, align 4
  store float %extract26, float addrspace(3)* %243, align 4
  store float %extract27, float addrspace(3)* %246, align 4
  store float %extract28, float addrspace(3)* %249, align 4
  store float %extract29, float addrspace(3)* %252, align 4
  store float %extract30, float addrspace(3)* %255, align 4
  store float %extract31, float addrspace(3)* %258, align 4
  store float %extract32, float addrspace(3)* %261, align 4
  store float %extract33, float addrspace(3)* %264, align 4
  %265 = add nsw <16 x i32> %vectorPHI, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  %266 = and <16 x i32> %265, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %267 = extractelement <16 x i32> %266, i32 0
  %268 = zext i32 %267 to i64
  %269 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %268
  %270 = extractelement <16 x i32> %266, i32 1
  %271 = zext i32 %270 to i64
  %272 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %271
  %273 = extractelement <16 x i32> %266, i32 2
  %274 = zext i32 %273 to i64
  %275 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %274
  %276 = extractelement <16 x i32> %266, i32 3
  %277 = zext i32 %276 to i64
  %278 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %277
  %279 = extractelement <16 x i32> %266, i32 4
  %280 = zext i32 %279 to i64
  %281 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %280
  %282 = extractelement <16 x i32> %266, i32 5
  %283 = zext i32 %282 to i64
  %284 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %283
  %285 = extractelement <16 x i32> %266, i32 6
  %286 = zext i32 %285 to i64
  %287 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %286
  %288 = extractelement <16 x i32> %266, i32 7
  %289 = zext i32 %288 to i64
  %290 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %289
  %291 = extractelement <16 x i32> %266, i32 8
  %292 = zext i32 %291 to i64
  %293 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %292
  %294 = extractelement <16 x i32> %266, i32 9
  %295 = zext i32 %294 to i64
  %296 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %295
  %297 = extractelement <16 x i32> %266, i32 10
  %298 = zext i32 %297 to i64
  %299 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %298
  %300 = extractelement <16 x i32> %266, i32 11
  %301 = zext i32 %300 to i64
  %302 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %301
  %303 = extractelement <16 x i32> %266, i32 12
  %304 = zext i32 %303 to i64
  %305 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %304
  %306 = extractelement <16 x i32> %266, i32 13
  %307 = zext i32 %306 to i64
  %308 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %307
  %309 = extractelement <16 x i32> %266, i32 14
  %310 = zext i32 %309 to i64
  %311 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %310
  %312 = extractelement <16 x i32> %266, i32 15
  %313 = zext i32 %312 to i64
  %314 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %313
  store float %extract18, float addrspace(3)* %269, align 4
  store float %extract19, float addrspace(3)* %272, align 4
  store float %extract20, float addrspace(3)* %275, align 4
  store float %extract21, float addrspace(3)* %278, align 4
  store float %extract22, float addrspace(3)* %281, align 4
  store float %extract23, float addrspace(3)* %284, align 4
  store float %extract24, float addrspace(3)* %287, align 4
  store float %extract25, float addrspace(3)* %290, align 4
  store float %extract26, float addrspace(3)* %293, align 4
  store float %extract27, float addrspace(3)* %296, align 4
  store float %extract28, float addrspace(3)* %299, align 4
  store float %extract29, float addrspace(3)* %302, align 4
  store float %extract30, float addrspace(3)* %305, align 4
  store float %extract31, float addrspace(3)* %308, align 4
  store float %extract32, float addrspace(3)* %311, align 4
  store float %extract33, float addrspace(3)* %314, align 4
  %315 = add nsw <16 x i32> %vectorPHI, <i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6>
  %316 = and <16 x i32> %315, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %317 = extractelement <16 x i32> %316, i32 0
  %318 = zext i32 %317 to i64
  %319 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %318
  %320 = extractelement <16 x i32> %316, i32 1
  %321 = zext i32 %320 to i64
  %322 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %321
  %323 = extractelement <16 x i32> %316, i32 2
  %324 = zext i32 %323 to i64
  %325 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %324
  %326 = extractelement <16 x i32> %316, i32 3
  %327 = zext i32 %326 to i64
  %328 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %327
  %329 = extractelement <16 x i32> %316, i32 4
  %330 = zext i32 %329 to i64
  %331 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %330
  %332 = extractelement <16 x i32> %316, i32 5
  %333 = zext i32 %332 to i64
  %334 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %333
  %335 = extractelement <16 x i32> %316, i32 6
  %336 = zext i32 %335 to i64
  %337 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %336
  %338 = extractelement <16 x i32> %316, i32 7
  %339 = zext i32 %338 to i64
  %340 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %339
  %341 = extractelement <16 x i32> %316, i32 8
  %342 = zext i32 %341 to i64
  %343 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %342
  %344 = extractelement <16 x i32> %316, i32 9
  %345 = zext i32 %344 to i64
  %346 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %345
  %347 = extractelement <16 x i32> %316, i32 10
  %348 = zext i32 %347 to i64
  %349 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %348
  %350 = extractelement <16 x i32> %316, i32 11
  %351 = zext i32 %350 to i64
  %352 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %351
  %353 = extractelement <16 x i32> %316, i32 12
  %354 = zext i32 %353 to i64
  %355 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %354
  %356 = extractelement <16 x i32> %316, i32 13
  %357 = zext i32 %356 to i64
  %358 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %357
  %359 = extractelement <16 x i32> %316, i32 14
  %360 = zext i32 %359 to i64
  %361 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %360
  %362 = extractelement <16 x i32> %316, i32 15
  %363 = zext i32 %362 to i64
  %364 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %363
  store float %extract18, float addrspace(3)* %319, align 4
  store float %extract19, float addrspace(3)* %322, align 4
  store float %extract20, float addrspace(3)* %325, align 4
  store float %extract21, float addrspace(3)* %328, align 4
  store float %extract22, float addrspace(3)* %331, align 4
  store float %extract23, float addrspace(3)* %334, align 4
  store float %extract24, float addrspace(3)* %337, align 4
  store float %extract25, float addrspace(3)* %340, align 4
  store float %extract26, float addrspace(3)* %343, align 4
  store float %extract27, float addrspace(3)* %346, align 4
  store float %extract28, float addrspace(3)* %349, align 4
  store float %extract29, float addrspace(3)* %352, align 4
  store float %extract30, float addrspace(3)* %355, align 4
  store float %extract31, float addrspace(3)* %358, align 4
  store float %extract32, float addrspace(3)* %361, align 4
  store float %extract33, float addrspace(3)* %364, align 4
  %365 = add nsw <16 x i32> %vectorPHI, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %366 = and <16 x i32> %365, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %367 = extractelement <16 x i32> %366, i32 0
  %368 = zext i32 %367 to i64
  %369 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %368
  %370 = extractelement <16 x i32> %366, i32 1
  %371 = zext i32 %370 to i64
  %372 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %371
  %373 = extractelement <16 x i32> %366, i32 2
  %374 = zext i32 %373 to i64
  %375 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %374
  %376 = extractelement <16 x i32> %366, i32 3
  %377 = zext i32 %376 to i64
  %378 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %377
  %379 = extractelement <16 x i32> %366, i32 4
  %380 = zext i32 %379 to i64
  %381 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %380
  %382 = extractelement <16 x i32> %366, i32 5
  %383 = zext i32 %382 to i64
  %384 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %383
  %385 = extractelement <16 x i32> %366, i32 6
  %386 = zext i32 %385 to i64
  %387 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %386
  %388 = extractelement <16 x i32> %366, i32 7
  %389 = zext i32 %388 to i64
  %390 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %389
  %391 = extractelement <16 x i32> %366, i32 8
  %392 = zext i32 %391 to i64
  %393 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %392
  %394 = extractelement <16 x i32> %366, i32 9
  %395 = zext i32 %394 to i64
  %396 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %395
  %397 = extractelement <16 x i32> %366, i32 10
  %398 = zext i32 %397 to i64
  %399 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %398
  %400 = extractelement <16 x i32> %366, i32 11
  %401 = zext i32 %400 to i64
  %402 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %401
  %403 = extractelement <16 x i32> %366, i32 12
  %404 = zext i32 %403 to i64
  %405 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %404
  %406 = extractelement <16 x i32> %366, i32 13
  %407 = zext i32 %406 to i64
  %408 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %407
  %409 = extractelement <16 x i32> %366, i32 14
  %410 = zext i32 %409 to i64
  %411 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %410
  %412 = extractelement <16 x i32> %366, i32 15
  %413 = zext i32 %412 to i64
  %414 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %413
  store float %extract18, float addrspace(3)* %369, align 4
  store float %extract19, float addrspace(3)* %372, align 4
  store float %extract20, float addrspace(3)* %375, align 4
  store float %extract21, float addrspace(3)* %378, align 4
  store float %extract22, float addrspace(3)* %381, align 4
  store float %extract23, float addrspace(3)* %384, align 4
  store float %extract24, float addrspace(3)* %387, align 4
  store float %extract25, float addrspace(3)* %390, align 4
  store float %extract26, float addrspace(3)* %393, align 4
  store float %extract27, float addrspace(3)* %396, align 4
  store float %extract28, float addrspace(3)* %399, align 4
  store float %extract29, float addrspace(3)* %402, align 4
  store float %extract30, float addrspace(3)* %405, align 4
  store float %extract31, float addrspace(3)* %408, align 4
  store float %extract32, float addrspace(3)* %411, align 4
  store float %extract33, float addrspace(3)* %414, align 4
  %415 = add nsw <16 x i32> %vectorPHI, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %416 = and <16 x i32> %415, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %417 = extractelement <16 x i32> %416, i32 0
  %418 = zext i32 %417 to i64
  %419 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %418
  %420 = extractelement <16 x i32> %416, i32 1
  %421 = zext i32 %420 to i64
  %422 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %421
  %423 = extractelement <16 x i32> %416, i32 2
  %424 = zext i32 %423 to i64
  %425 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %424
  %426 = extractelement <16 x i32> %416, i32 3
  %427 = zext i32 %426 to i64
  %428 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %427
  %429 = extractelement <16 x i32> %416, i32 4
  %430 = zext i32 %429 to i64
  %431 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %430
  %432 = extractelement <16 x i32> %416, i32 5
  %433 = zext i32 %432 to i64
  %434 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %433
  %435 = extractelement <16 x i32> %416, i32 6
  %436 = zext i32 %435 to i64
  %437 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %436
  %438 = extractelement <16 x i32> %416, i32 7
  %439 = zext i32 %438 to i64
  %440 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %439
  %441 = extractelement <16 x i32> %416, i32 8
  %442 = zext i32 %441 to i64
  %443 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %442
  %444 = extractelement <16 x i32> %416, i32 9
  %445 = zext i32 %444 to i64
  %446 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %445
  %447 = extractelement <16 x i32> %416, i32 10
  %448 = zext i32 %447 to i64
  %449 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %448
  %450 = extractelement <16 x i32> %416, i32 11
  %451 = zext i32 %450 to i64
  %452 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %451
  %453 = extractelement <16 x i32> %416, i32 12
  %454 = zext i32 %453 to i64
  %455 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %454
  %456 = extractelement <16 x i32> %416, i32 13
  %457 = zext i32 %456 to i64
  %458 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %457
  %459 = extractelement <16 x i32> %416, i32 14
  %460 = zext i32 %459 to i64
  %461 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %460
  %462 = extractelement <16 x i32> %416, i32 15
  %463 = zext i32 %462 to i64
  %464 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %463
  store float %extract18, float addrspace(3)* %419, align 4
  store float %extract19, float addrspace(3)* %422, align 4
  store float %extract20, float addrspace(3)* %425, align 4
  store float %extract21, float addrspace(3)* %428, align 4
  store float %extract22, float addrspace(3)* %431, align 4
  store float %extract23, float addrspace(3)* %434, align 4
  store float %extract24, float addrspace(3)* %437, align 4
  store float %extract25, float addrspace(3)* %440, align 4
  store float %extract26, float addrspace(3)* %443, align 4
  store float %extract27, float addrspace(3)* %446, align 4
  store float %extract28, float addrspace(3)* %449, align 4
  store float %extract29, float addrspace(3)* %452, align 4
  store float %extract30, float addrspace(3)* %455, align 4
  store float %extract31, float addrspace(3)* %458, align 4
  store float %extract32, float addrspace(3)* %461, align 4
  store float %extract33, float addrspace(3)* %464, align 4
  %465 = add nsw <16 x i32> %vectorPHI, <i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9>
  %466 = and <16 x i32> %465, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %467 = extractelement <16 x i32> %466, i32 0
  %468 = zext i32 %467 to i64
  %469 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %468
  %470 = extractelement <16 x i32> %466, i32 1
  %471 = zext i32 %470 to i64
  %472 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %471
  %473 = extractelement <16 x i32> %466, i32 2
  %474 = zext i32 %473 to i64
  %475 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %474
  %476 = extractelement <16 x i32> %466, i32 3
  %477 = zext i32 %476 to i64
  %478 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %477
  %479 = extractelement <16 x i32> %466, i32 4
  %480 = zext i32 %479 to i64
  %481 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %480
  %482 = extractelement <16 x i32> %466, i32 5
  %483 = zext i32 %482 to i64
  %484 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %483
  %485 = extractelement <16 x i32> %466, i32 6
  %486 = zext i32 %485 to i64
  %487 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %486
  %488 = extractelement <16 x i32> %466, i32 7
  %489 = zext i32 %488 to i64
  %490 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %489
  %491 = extractelement <16 x i32> %466, i32 8
  %492 = zext i32 %491 to i64
  %493 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %492
  %494 = extractelement <16 x i32> %466, i32 9
  %495 = zext i32 %494 to i64
  %496 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %495
  %497 = extractelement <16 x i32> %466, i32 10
  %498 = zext i32 %497 to i64
  %499 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %498
  %500 = extractelement <16 x i32> %466, i32 11
  %501 = zext i32 %500 to i64
  %502 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %501
  %503 = extractelement <16 x i32> %466, i32 12
  %504 = zext i32 %503 to i64
  %505 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %504
  %506 = extractelement <16 x i32> %466, i32 13
  %507 = zext i32 %506 to i64
  %508 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %507
  %509 = extractelement <16 x i32> %466, i32 14
  %510 = zext i32 %509 to i64
  %511 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %510
  %512 = extractelement <16 x i32> %466, i32 15
  %513 = zext i32 %512 to i64
  %514 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %513
  store float %extract18, float addrspace(3)* %469, align 4
  store float %extract19, float addrspace(3)* %472, align 4
  store float %extract20, float addrspace(3)* %475, align 4
  store float %extract21, float addrspace(3)* %478, align 4
  store float %extract22, float addrspace(3)* %481, align 4
  store float %extract23, float addrspace(3)* %484, align 4
  store float %extract24, float addrspace(3)* %487, align 4
  store float %extract25, float addrspace(3)* %490, align 4
  store float %extract26, float addrspace(3)* %493, align 4
  store float %extract27, float addrspace(3)* %496, align 4
  store float %extract28, float addrspace(3)* %499, align 4
  store float %extract29, float addrspace(3)* %502, align 4
  store float %extract30, float addrspace(3)* %505, align 4
  store float %extract31, float addrspace(3)* %508, align 4
  store float %extract32, float addrspace(3)* %511, align 4
  store float %extract33, float addrspace(3)* %514, align 4
  %515 = add nsw <16 x i32> %vectorPHI, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %516 = and <16 x i32> %515, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %517 = extractelement <16 x i32> %516, i32 0
  %518 = zext i32 %517 to i64
  %519 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %518
  %520 = extractelement <16 x i32> %516, i32 1
  %521 = zext i32 %520 to i64
  %522 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %521
  %523 = extractelement <16 x i32> %516, i32 2
  %524 = zext i32 %523 to i64
  %525 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %524
  %526 = extractelement <16 x i32> %516, i32 3
  %527 = zext i32 %526 to i64
  %528 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %527
  %529 = extractelement <16 x i32> %516, i32 4
  %530 = zext i32 %529 to i64
  %531 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %530
  %532 = extractelement <16 x i32> %516, i32 5
  %533 = zext i32 %532 to i64
  %534 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %533
  %535 = extractelement <16 x i32> %516, i32 6
  %536 = zext i32 %535 to i64
  %537 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %536
  %538 = extractelement <16 x i32> %516, i32 7
  %539 = zext i32 %538 to i64
  %540 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %539
  %541 = extractelement <16 x i32> %516, i32 8
  %542 = zext i32 %541 to i64
  %543 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %542
  %544 = extractelement <16 x i32> %516, i32 9
  %545 = zext i32 %544 to i64
  %546 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %545
  %547 = extractelement <16 x i32> %516, i32 10
  %548 = zext i32 %547 to i64
  %549 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %548
  %550 = extractelement <16 x i32> %516, i32 11
  %551 = zext i32 %550 to i64
  %552 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %551
  %553 = extractelement <16 x i32> %516, i32 12
  %554 = zext i32 %553 to i64
  %555 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %554
  %556 = extractelement <16 x i32> %516, i32 13
  %557 = zext i32 %556 to i64
  %558 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %557
  %559 = extractelement <16 x i32> %516, i32 14
  %560 = zext i32 %559 to i64
  %561 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %560
  %562 = extractelement <16 x i32> %516, i32 15
  %563 = zext i32 %562 to i64
  %564 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %563
  store float %extract18, float addrspace(3)* %519, align 4
  store float %extract19, float addrspace(3)* %522, align 4
  store float %extract20, float addrspace(3)* %525, align 4
  store float %extract21, float addrspace(3)* %528, align 4
  store float %extract22, float addrspace(3)* %531, align 4
  store float %extract23, float addrspace(3)* %534, align 4
  store float %extract24, float addrspace(3)* %537, align 4
  store float %extract25, float addrspace(3)* %540, align 4
  store float %extract26, float addrspace(3)* %543, align 4
  store float %extract27, float addrspace(3)* %546, align 4
  store float %extract28, float addrspace(3)* %549, align 4
  store float %extract29, float addrspace(3)* %552, align 4
  store float %extract30, float addrspace(3)* %555, align 4
  store float %extract31, float addrspace(3)* %558, align 4
  store float %extract32, float addrspace(3)* %561, align 4
  store float %extract33, float addrspace(3)* %564, align 4
  %565 = add nsw <16 x i32> %vectorPHI, <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
  %566 = and <16 x i32> %565, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %567 = extractelement <16 x i32> %566, i32 0
  %568 = zext i32 %567 to i64
  %569 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %568
  %570 = extractelement <16 x i32> %566, i32 1
  %571 = zext i32 %570 to i64
  %572 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %571
  %573 = extractelement <16 x i32> %566, i32 2
  %574 = zext i32 %573 to i64
  %575 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %574
  %576 = extractelement <16 x i32> %566, i32 3
  %577 = zext i32 %576 to i64
  %578 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %577
  %579 = extractelement <16 x i32> %566, i32 4
  %580 = zext i32 %579 to i64
  %581 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %580
  %582 = extractelement <16 x i32> %566, i32 5
  %583 = zext i32 %582 to i64
  %584 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %583
  %585 = extractelement <16 x i32> %566, i32 6
  %586 = zext i32 %585 to i64
  %587 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %586
  %588 = extractelement <16 x i32> %566, i32 7
  %589 = zext i32 %588 to i64
  %590 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %589
  %591 = extractelement <16 x i32> %566, i32 8
  %592 = zext i32 %591 to i64
  %593 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %592
  %594 = extractelement <16 x i32> %566, i32 9
  %595 = zext i32 %594 to i64
  %596 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %595
  %597 = extractelement <16 x i32> %566, i32 10
  %598 = zext i32 %597 to i64
  %599 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %598
  %600 = extractelement <16 x i32> %566, i32 11
  %601 = zext i32 %600 to i64
  %602 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %601
  %603 = extractelement <16 x i32> %566, i32 12
  %604 = zext i32 %603 to i64
  %605 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %604
  %606 = extractelement <16 x i32> %566, i32 13
  %607 = zext i32 %606 to i64
  %608 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %607
  %609 = extractelement <16 x i32> %566, i32 14
  %610 = zext i32 %609 to i64
  %611 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %610
  %612 = extractelement <16 x i32> %566, i32 15
  %613 = zext i32 %612 to i64
  %614 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %613
  store float %extract18, float addrspace(3)* %569, align 4
  store float %extract19, float addrspace(3)* %572, align 4
  store float %extract20, float addrspace(3)* %575, align 4
  store float %extract21, float addrspace(3)* %578, align 4
  store float %extract22, float addrspace(3)* %581, align 4
  store float %extract23, float addrspace(3)* %584, align 4
  store float %extract24, float addrspace(3)* %587, align 4
  store float %extract25, float addrspace(3)* %590, align 4
  store float %extract26, float addrspace(3)* %593, align 4
  store float %extract27, float addrspace(3)* %596, align 4
  store float %extract28, float addrspace(3)* %599, align 4
  store float %extract29, float addrspace(3)* %602, align 4
  store float %extract30, float addrspace(3)* %605, align 4
  store float %extract31, float addrspace(3)* %608, align 4
  store float %extract32, float addrspace(3)* %611, align 4
  store float %extract33, float addrspace(3)* %614, align 4
  %615 = add nsw <16 x i32> %vectorPHI, <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
  %616 = and <16 x i32> %615, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %617 = extractelement <16 x i32> %616, i32 0
  %618 = zext i32 %617 to i64
  %619 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %618
  %620 = extractelement <16 x i32> %616, i32 1
  %621 = zext i32 %620 to i64
  %622 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %621
  %623 = extractelement <16 x i32> %616, i32 2
  %624 = zext i32 %623 to i64
  %625 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %624
  %626 = extractelement <16 x i32> %616, i32 3
  %627 = zext i32 %626 to i64
  %628 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %627
  %629 = extractelement <16 x i32> %616, i32 4
  %630 = zext i32 %629 to i64
  %631 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %630
  %632 = extractelement <16 x i32> %616, i32 5
  %633 = zext i32 %632 to i64
  %634 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %633
  %635 = extractelement <16 x i32> %616, i32 6
  %636 = zext i32 %635 to i64
  %637 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %636
  %638 = extractelement <16 x i32> %616, i32 7
  %639 = zext i32 %638 to i64
  %640 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %639
  %641 = extractelement <16 x i32> %616, i32 8
  %642 = zext i32 %641 to i64
  %643 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %642
  %644 = extractelement <16 x i32> %616, i32 9
  %645 = zext i32 %644 to i64
  %646 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %645
  %647 = extractelement <16 x i32> %616, i32 10
  %648 = zext i32 %647 to i64
  %649 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %648
  %650 = extractelement <16 x i32> %616, i32 11
  %651 = zext i32 %650 to i64
  %652 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %651
  %653 = extractelement <16 x i32> %616, i32 12
  %654 = zext i32 %653 to i64
  %655 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %654
  %656 = extractelement <16 x i32> %616, i32 13
  %657 = zext i32 %656 to i64
  %658 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %657
  %659 = extractelement <16 x i32> %616, i32 14
  %660 = zext i32 %659 to i64
  %661 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %660
  %662 = extractelement <16 x i32> %616, i32 15
  %663 = zext i32 %662 to i64
  %664 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %663
  store float %extract18, float addrspace(3)* %619, align 4
  store float %extract19, float addrspace(3)* %622, align 4
  store float %extract20, float addrspace(3)* %625, align 4
  store float %extract21, float addrspace(3)* %628, align 4
  store float %extract22, float addrspace(3)* %631, align 4
  store float %extract23, float addrspace(3)* %634, align 4
  store float %extract24, float addrspace(3)* %637, align 4
  store float %extract25, float addrspace(3)* %640, align 4
  store float %extract26, float addrspace(3)* %643, align 4
  store float %extract27, float addrspace(3)* %646, align 4
  store float %extract28, float addrspace(3)* %649, align 4
  store float %extract29, float addrspace(3)* %652, align 4
  store float %extract30, float addrspace(3)* %655, align 4
  store float %extract31, float addrspace(3)* %658, align 4
  store float %extract32, float addrspace(3)* %661, align 4
  store float %extract33, float addrspace(3)* %664, align 4
  %665 = add nsw <16 x i32> %vectorPHI, <i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13>
  %666 = and <16 x i32> %665, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %667 = extractelement <16 x i32> %666, i32 0
  %668 = zext i32 %667 to i64
  %669 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %668
  %670 = extractelement <16 x i32> %666, i32 1
  %671 = zext i32 %670 to i64
  %672 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %671
  %673 = extractelement <16 x i32> %666, i32 2
  %674 = zext i32 %673 to i64
  %675 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %674
  %676 = extractelement <16 x i32> %666, i32 3
  %677 = zext i32 %676 to i64
  %678 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %677
  %679 = extractelement <16 x i32> %666, i32 4
  %680 = zext i32 %679 to i64
  %681 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %680
  %682 = extractelement <16 x i32> %666, i32 5
  %683 = zext i32 %682 to i64
  %684 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %683
  %685 = extractelement <16 x i32> %666, i32 6
  %686 = zext i32 %685 to i64
  %687 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %686
  %688 = extractelement <16 x i32> %666, i32 7
  %689 = zext i32 %688 to i64
  %690 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %689
  %691 = extractelement <16 x i32> %666, i32 8
  %692 = zext i32 %691 to i64
  %693 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %692
  %694 = extractelement <16 x i32> %666, i32 9
  %695 = zext i32 %694 to i64
  %696 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %695
  %697 = extractelement <16 x i32> %666, i32 10
  %698 = zext i32 %697 to i64
  %699 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %698
  %700 = extractelement <16 x i32> %666, i32 11
  %701 = zext i32 %700 to i64
  %702 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %701
  %703 = extractelement <16 x i32> %666, i32 12
  %704 = zext i32 %703 to i64
  %705 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %704
  %706 = extractelement <16 x i32> %666, i32 13
  %707 = zext i32 %706 to i64
  %708 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %707
  %709 = extractelement <16 x i32> %666, i32 14
  %710 = zext i32 %709 to i64
  %711 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %710
  %712 = extractelement <16 x i32> %666, i32 15
  %713 = zext i32 %712 to i64
  %714 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %713
  store float %extract18, float addrspace(3)* %669, align 4
  store float %extract19, float addrspace(3)* %672, align 4
  store float %extract20, float addrspace(3)* %675, align 4
  store float %extract21, float addrspace(3)* %678, align 4
  store float %extract22, float addrspace(3)* %681, align 4
  store float %extract23, float addrspace(3)* %684, align 4
  store float %extract24, float addrspace(3)* %687, align 4
  store float %extract25, float addrspace(3)* %690, align 4
  store float %extract26, float addrspace(3)* %693, align 4
  store float %extract27, float addrspace(3)* %696, align 4
  store float %extract28, float addrspace(3)* %699, align 4
  store float %extract29, float addrspace(3)* %702, align 4
  store float %extract30, float addrspace(3)* %705, align 4
  store float %extract31, float addrspace(3)* %708, align 4
  store float %extract32, float addrspace(3)* %711, align 4
  store float %extract33, float addrspace(3)* %714, align 4
  %715 = add nsw <16 x i32> %vectorPHI, <i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14>
  %716 = and <16 x i32> %715, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %717 = extractelement <16 x i32> %716, i32 0
  %718 = zext i32 %717 to i64
  %719 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %718
  %720 = extractelement <16 x i32> %716, i32 1
  %721 = zext i32 %720 to i64
  %722 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %721
  %723 = extractelement <16 x i32> %716, i32 2
  %724 = zext i32 %723 to i64
  %725 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %724
  %726 = extractelement <16 x i32> %716, i32 3
  %727 = zext i32 %726 to i64
  %728 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %727
  %729 = extractelement <16 x i32> %716, i32 4
  %730 = zext i32 %729 to i64
  %731 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %730
  %732 = extractelement <16 x i32> %716, i32 5
  %733 = zext i32 %732 to i64
  %734 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %733
  %735 = extractelement <16 x i32> %716, i32 6
  %736 = zext i32 %735 to i64
  %737 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %736
  %738 = extractelement <16 x i32> %716, i32 7
  %739 = zext i32 %738 to i64
  %740 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %739
  %741 = extractelement <16 x i32> %716, i32 8
  %742 = zext i32 %741 to i64
  %743 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %742
  %744 = extractelement <16 x i32> %716, i32 9
  %745 = zext i32 %744 to i64
  %746 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %745
  %747 = extractelement <16 x i32> %716, i32 10
  %748 = zext i32 %747 to i64
  %749 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %748
  %750 = extractelement <16 x i32> %716, i32 11
  %751 = zext i32 %750 to i64
  %752 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %751
  %753 = extractelement <16 x i32> %716, i32 12
  %754 = zext i32 %753 to i64
  %755 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %754
  %756 = extractelement <16 x i32> %716, i32 13
  %757 = zext i32 %756 to i64
  %758 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %757
  %759 = extractelement <16 x i32> %716, i32 14
  %760 = zext i32 %759 to i64
  %761 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %760
  %762 = extractelement <16 x i32> %716, i32 15
  %763 = zext i32 %762 to i64
  %764 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %763
  store float %extract18, float addrspace(3)* %719, align 4
  store float %extract19, float addrspace(3)* %722, align 4
  store float %extract20, float addrspace(3)* %725, align 4
  store float %extract21, float addrspace(3)* %728, align 4
  store float %extract22, float addrspace(3)* %731, align 4
  store float %extract23, float addrspace(3)* %734, align 4
  store float %extract24, float addrspace(3)* %737, align 4
  store float %extract25, float addrspace(3)* %740, align 4
  store float %extract26, float addrspace(3)* %743, align 4
  store float %extract27, float addrspace(3)* %746, align 4
  store float %extract28, float addrspace(3)* %749, align 4
  store float %extract29, float addrspace(3)* %752, align 4
  store float %extract30, float addrspace(3)* %755, align 4
  store float %extract31, float addrspace(3)* %758, align 4
  store float %extract32, float addrspace(3)* %761, align 4
  store float %extract33, float addrspace(3)* %764, align 4
  %765 = add nsw <16 x i32> %vectorPHI, <i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15>
  %766 = and <16 x i32> %765, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %767 = extractelement <16 x i32> %766, i32 0
  %768 = zext i32 %767 to i64
  %769 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %768
  %770 = extractelement <16 x i32> %766, i32 1
  %771 = zext i32 %770 to i64
  %772 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %771
  %773 = extractelement <16 x i32> %766, i32 2
  %774 = zext i32 %773 to i64
  %775 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %774
  %776 = extractelement <16 x i32> %766, i32 3
  %777 = zext i32 %776 to i64
  %778 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %777
  %779 = extractelement <16 x i32> %766, i32 4
  %780 = zext i32 %779 to i64
  %781 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %780
  %782 = extractelement <16 x i32> %766, i32 5
  %783 = zext i32 %782 to i64
  %784 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %783
  %785 = extractelement <16 x i32> %766, i32 6
  %786 = zext i32 %785 to i64
  %787 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %786
  %788 = extractelement <16 x i32> %766, i32 7
  %789 = zext i32 %788 to i64
  %790 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %789
  %791 = extractelement <16 x i32> %766, i32 8
  %792 = zext i32 %791 to i64
  %793 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %792
  %794 = extractelement <16 x i32> %766, i32 9
  %795 = zext i32 %794 to i64
  %796 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %795
  %797 = extractelement <16 x i32> %766, i32 10
  %798 = zext i32 %797 to i64
  %799 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %798
  %800 = extractelement <16 x i32> %766, i32 11
  %801 = zext i32 %800 to i64
  %802 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %801
  %803 = extractelement <16 x i32> %766, i32 12
  %804 = zext i32 %803 to i64
  %805 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %804
  %806 = extractelement <16 x i32> %766, i32 13
  %807 = zext i32 %806 to i64
  %808 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %807
  %809 = extractelement <16 x i32> %766, i32 14
  %810 = zext i32 %809 to i64
  %811 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %810
  %812 = extractelement <16 x i32> %766, i32 15
  %813 = zext i32 %812 to i64
  %814 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %813
  store float %extract18, float addrspace(3)* %769, align 4
  store float %extract19, float addrspace(3)* %772, align 4
  store float %extract20, float addrspace(3)* %775, align 4
  store float %extract21, float addrspace(3)* %778, align 4
  store float %extract22, float addrspace(3)* %781, align 4
  store float %extract23, float addrspace(3)* %784, align 4
  store float %extract24, float addrspace(3)* %787, align 4
  store float %extract25, float addrspace(3)* %790, align 4
  store float %extract26, float addrspace(3)* %793, align 4
  store float %extract27, float addrspace(3)* %796, align 4
  store float %extract28, float addrspace(3)* %799, align 4
  store float %extract29, float addrspace(3)* %802, align 4
  store float %extract30, float addrspace(3)* %805, align 4
  store float %extract31, float addrspace(3)* %808, align 4
  store float %extract32, float addrspace(3)* %811, align 4
  store float %extract33, float addrspace(3)* %814, align 4
  %815 = add nsw i32 %j.02, 1
  %816 = add nsw <16 x i32> %vectorPHI, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %817 = and <16 x i32> %816, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %exitcond6 = icmp eq i32 %815, 3000
  br i1 %exitcond6, label %._crit_edge5, label %15

._crit_edge5:                                     ; preds = %15
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %._crit_edge5
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 192
  br label %SyncBB323

elseBB:                                           ; preds = %._crit_edge5
  %818 = trunc i64 %13 to i32
  %819 = icmp eq i32 %818, 0
  %820 = select i1 %819, i32 1, i32 %818
  %821 = sdiv i32 4096, %820
  %822 = icmp sgt i32 %821, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB325, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride331", %thenBB325 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++329", %thenBB325 ]
  br i1 %822, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB
  %"&(pSB[currWI].offset)316" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset317" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)316"
  %CastToValueType318 = bitcast i8* %"&pSB[currWI].offset317" to <16 x i32>*
  %loadedValue319 = load <16 x i32>* %CastToValueType318, align 64
  %823 = extractelement <16 x i32> %loadedValue319, i32 0
  %extract274 = sext i32 %823 to i64
  %824 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract274
  %"&(pSB[currWI].offset)308" = add nuw i64 %CurrSBIndex..1, 64
  %"&pSB[currWI].offset309" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)308"
  %CastToValueType310 = bitcast i8* %"&pSB[currWI].offset309" to <16 x i32>*
  %loadedValue = load <16 x i32>* %CastToValueType310, align 64
  %825 = extractelement <16 x i32> %loadedValue, i32 0
  %extract290 = sext i32 %825 to i64
  %826 = getelementptr inbounds float addrspace(1)* %output, i64 %extract290
  %ptrTypeCast = bitcast float addrspace(3)* %824 to <16 x float> addrspace(3)*
  %ptrTypeCast306 = bitcast float addrspace(1)* %826 to <16 x float> addrspace(1)*
  br label %827

; <label>:827                                     ; preds = %827, %bb.nph
  %j.11 = phi i32 [ 0, %bb.nph ], [ %829, %827 ]
  %828 = load <16 x float> addrspace(3)* %ptrTypeCast, align 4
  store <16 x float> %828, <16 x float> addrspace(1)* %ptrTypeCast306, align 4
  %829 = add nsw i32 %j.11, 1
  %exitcond = icmp eq i32 %829, %821
  br i1 %exitcond, label %._crit_edge, label %827

._crit_edge:                                      ; preds = %827, %SyncBB
  %check.WI.iter328 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter328, label %thenBB325, label %SyncBB322

thenBB325:                                        ; preds = %._crit_edge
  %"CurrWI++329" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride331" = add nuw i64 %CurrSBIndex..1, 192
  br label %SyncBB

SyncBB322:                                        ; preds = %._crit_edge
  ret void
}

define void @writeLocalMemory(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to i8 addrspace(3)**
  %4 = load i8 addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to %struct.WorkDim**
  %7 = load %struct.WorkDim** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %20 = bitcast i8 addrspace(3)* %4 to [4096 x float] addrspace(3)*
  br label %SyncBB22.i

SyncBB22.i:                                       ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %21 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = add i64 %22, %24
  %26 = trunc i64 %25 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %26, i32* %CastToValueType.i, align 4
  %27 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = trunc i64 %28 to i32
  %"&(pSB[currWI].offset)121.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset13.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)121.i"
  %CastToValueType14.i = bitcast i8* %"&pSB[currWI].offset13.i" to i32*
  store i32 %29, i32* %CastToValueType14.i, align 4
  %30 = getelementptr %struct.WorkDim* %7, i64 0, i32 3, i64 0
  %31 = load i64* %30, align 8
  %32 = sitofp i32 %26 to float
  br label %33

; <label>:33                                      ; preds = %33, %SyncBB22.i
  %s.03.i = phi i32 [ %29, %SyncBB22.i ], [ %99, %33 ]
  %j.02.i = phi i32 [ 0, %SyncBB22.i ], [ %97, %33 ]
  %34 = and i32 %s.03.i, 4095
  %35 = zext i32 %34 to i64
  %36 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %35
  store float %32, float addrspace(3)* %36, align 4
  %37 = add nsw i32 %s.03.i, 1
  %38 = and i32 %37, 4095
  %39 = zext i32 %38 to i64
  %40 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %39
  store float %32, float addrspace(3)* %40, align 4
  %41 = add nsw i32 %s.03.i, 2
  %42 = and i32 %41, 4095
  %43 = zext i32 %42 to i64
  %44 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %43
  store float %32, float addrspace(3)* %44, align 4
  %45 = add nsw i32 %s.03.i, 3
  %46 = and i32 %45, 4095
  %47 = zext i32 %46 to i64
  %48 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %47
  store float %32, float addrspace(3)* %48, align 4
  %49 = add nsw i32 %s.03.i, 4
  %50 = and i32 %49, 4095
  %51 = zext i32 %50 to i64
  %52 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %51
  store float %32, float addrspace(3)* %52, align 4
  %53 = add nsw i32 %s.03.i, 5
  %54 = and i32 %53, 4095
  %55 = zext i32 %54 to i64
  %56 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %55
  store float %32, float addrspace(3)* %56, align 4
  %57 = add nsw i32 %s.03.i, 6
  %58 = and i32 %57, 4095
  %59 = zext i32 %58 to i64
  %60 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %59
  store float %32, float addrspace(3)* %60, align 4
  %61 = add nsw i32 %s.03.i, 7
  %62 = and i32 %61, 4095
  %63 = zext i32 %62 to i64
  %64 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %63
  store float %32, float addrspace(3)* %64, align 4
  %65 = add nsw i32 %s.03.i, 8
  %66 = and i32 %65, 4095
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %67
  store float %32, float addrspace(3)* %68, align 4
  %69 = add nsw i32 %s.03.i, 9
  %70 = and i32 %69, 4095
  %71 = zext i32 %70 to i64
  %72 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %71
  store float %32, float addrspace(3)* %72, align 4
  %73 = add nsw i32 %s.03.i, 10
  %74 = and i32 %73, 4095
  %75 = zext i32 %74 to i64
  %76 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %75
  store float %32, float addrspace(3)* %76, align 4
  %77 = add nsw i32 %s.03.i, 11
  %78 = and i32 %77, 4095
  %79 = zext i32 %78 to i64
  %80 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %79
  store float %32, float addrspace(3)* %80, align 4
  %81 = add nsw i32 %s.03.i, 12
  %82 = and i32 %81, 4095
  %83 = zext i32 %82 to i64
  %84 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %83
  store float %32, float addrspace(3)* %84, align 4
  %85 = add nsw i32 %s.03.i, 13
  %86 = and i32 %85, 4095
  %87 = zext i32 %86 to i64
  %88 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %87
  store float %32, float addrspace(3)* %88, align 4
  %89 = add nsw i32 %s.03.i, 14
  %90 = and i32 %89, 4095
  %91 = zext i32 %90 to i64
  %92 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %91
  store float %32, float addrspace(3)* %92, align 4
  %93 = add nsw i32 %s.03.i, 15
  %94 = and i32 %93, 4095
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %95
  store float %32, float addrspace(3)* %96, align 4
  %97 = add nsw i32 %j.02.i, 1
  %98 = add nsw i32 %s.03.i, 16
  %99 = and i32 %98, 4095
  %exitcond6.i = icmp eq i32 %97, 3000
  br i1 %exitcond6.i, label %._crit_edge5.i, label %33

._crit_edge5.i:                                   ; preds = %33
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %._crit_edge5.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 192
  br label %SyncBB22.i

elseBB.i:                                         ; preds = %._crit_edge5.i
  %100 = trunc i64 %31 to i32
  %101 = icmp eq i32 %100, 0
  %102 = select i1 %101, i32 1, i32 %100
  %103 = sdiv i32 4096, %102
  %104 = icmp sgt i32 %103, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB25.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride31.i", %thenBB25.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++29.i", %thenBB25.i ]
  br i1 %104, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)162.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset17.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)162.i"
  %CastToValueType18.i = bitcast i8* %"&pSB[currWI].offset17.i" to i32*
  %loadedValue19.i = load i32* %CastToValueType18.i, align 4
  %105 = sext i32 %loadedValue19.i to i64
  %106 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %105
  %"&pSB[currWI].offset9.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..1.i
  %CastToValueType10.i = bitcast i8* %"&pSB[currWI].offset9.i" to i32*
  %loadedValue.i = load i32* %CastToValueType10.i, align 4
  %107 = sext i32 %loadedValue.i to i64
  %108 = getelementptr inbounds float addrspace(1)* %1, i64 %107
  br label %109

; <label>:109                                     ; preds = %109, %bb.nph.i
  %j.11.i = phi i32 [ 0, %bb.nph.i ], [ %111, %109 ]
  %110 = load float addrspace(3)* %106, align 4
  store float %110, float addrspace(1)* %108, align 4
  %111 = add nsw i32 %j.11.i, 1
  %exitcond.i = icmp eq i32 %111, %103
  br i1 %exitcond.i, label %._crit_edge.i, label %109

._crit_edge.i:                                    ; preds = %109, %SyncBB.i
  %check.WI.iter28.i = icmp ult i64 %CurrWI..1.i, %16
  br i1 %check.WI.iter28.i, label %thenBB25.i, label %__writeLocalMemory_separated_args.exit

thenBB25.i:                                       ; preds = %._crit_edge.i
  %"CurrWI++29.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride31.i" = add nuw i64 %CurrSBIndex..1.i, 192
  br label %SyncBB.i

__writeLocalMemory_separated_args.exit:           ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.writeLocalMemory(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to i8 addrspace(3)**
  %4 = load i8 addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to %struct.WorkDim**
  %7 = load %struct.WorkDim** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %20 = bitcast i8 addrspace(3)* %4 to [4096 x float] addrspace(3)*
  br label %SyncBB323.i

SyncBB323.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %21 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = add i64 %22, %24
  %broadcast1.i = insertelement <16 x i64> undef, i64 %25, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %26 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %27 = trunc <16 x i64> %26 to <16 x i32>
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %27, <16 x i32>* %CastToValueType.i, align 64
  %28 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %29 = load i64* %28, align 8
  %broadcast11.i = insertelement <16 x i64> undef, i64 %29, i32 0
  %broadcast22.i = shufflevector <16 x i64> %broadcast11.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %30 = add <16 x i64> %broadcast22.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %31 = trunc <16 x i64> %30 to <16 x i32>
  %"&(pSB[currWI].offset)312.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset313.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)312.i"
  %CastToValueType314.i = bitcast i8* %"&pSB[currWI].offset313.i" to <16 x i32>*
  store <16 x i32> %31, <16 x i32>* %CastToValueType314.i, align 64
  %32 = getelementptr %struct.WorkDim* %7, i64 0, i32 3, i64 0
  %33 = load i64* %32, align 8
  %34 = sitofp <16 x i32> %27 to <16 x float>
  %extract18.i = extractelement <16 x float> %34, i32 0
  %extract19.i = extractelement <16 x float> %34, i32 1
  %extract20.i = extractelement <16 x float> %34, i32 2
  %extract21.i = extractelement <16 x float> %34, i32 3
  %extract22.i = extractelement <16 x float> %34, i32 4
  %extract23.i = extractelement <16 x float> %34, i32 5
  %extract24.i = extractelement <16 x float> %34, i32 6
  %extract25.i = extractelement <16 x float> %34, i32 7
  %extract26.i = extractelement <16 x float> %34, i32 8
  %extract27.i = extractelement <16 x float> %34, i32 9
  %extract28.i = extractelement <16 x float> %34, i32 10
  %extract29.i = extractelement <16 x float> %34, i32 11
  %extract30.i = extractelement <16 x float> %34, i32 12
  %extract31.i = extractelement <16 x float> %34, i32 13
  %extract32.i = extractelement <16 x float> %34, i32 14
  %extract33.i = extractelement <16 x float> %34, i32 15
  br label %35

; <label>:35                                      ; preds = %35, %SyncBB323.i
  %vectorPHI.i = phi <16 x i32> [ %31, %SyncBB323.i ], [ %837, %35 ]
  %j.02.i = phi i32 [ 0, %SyncBB323.i ], [ %835, %35 ]
  %36 = and <16 x i32> %vectorPHI.i, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %37 = extractelement <16 x i32> %36, i32 0
  %38 = zext i32 %37 to i64
  %39 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %38
  %40 = extractelement <16 x i32> %36, i32 1
  %41 = zext i32 %40 to i64
  %42 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %41
  %43 = extractelement <16 x i32> %36, i32 2
  %44 = zext i32 %43 to i64
  %45 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %44
  %46 = extractelement <16 x i32> %36, i32 3
  %47 = zext i32 %46 to i64
  %48 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %47
  %49 = extractelement <16 x i32> %36, i32 4
  %50 = zext i32 %49 to i64
  %51 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %50
  %52 = extractelement <16 x i32> %36, i32 5
  %53 = zext i32 %52 to i64
  %54 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %53
  %55 = extractelement <16 x i32> %36, i32 6
  %56 = zext i32 %55 to i64
  %57 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %56
  %58 = extractelement <16 x i32> %36, i32 7
  %59 = zext i32 %58 to i64
  %60 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %59
  %61 = extractelement <16 x i32> %36, i32 8
  %62 = zext i32 %61 to i64
  %63 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %62
  %64 = extractelement <16 x i32> %36, i32 9
  %65 = zext i32 %64 to i64
  %66 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %65
  %67 = extractelement <16 x i32> %36, i32 10
  %68 = zext i32 %67 to i64
  %69 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %68
  %70 = extractelement <16 x i32> %36, i32 11
  %71 = zext i32 %70 to i64
  %72 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %71
  %73 = extractelement <16 x i32> %36, i32 12
  %74 = zext i32 %73 to i64
  %75 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %74
  %76 = extractelement <16 x i32> %36, i32 13
  %77 = zext i32 %76 to i64
  %78 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %77
  %79 = extractelement <16 x i32> %36, i32 14
  %80 = zext i32 %79 to i64
  %81 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %80
  %82 = extractelement <16 x i32> %36, i32 15
  %83 = zext i32 %82 to i64
  %84 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %83
  store float %extract18.i, float addrspace(3)* %39, align 4
  store float %extract19.i, float addrspace(3)* %42, align 4
  store float %extract20.i, float addrspace(3)* %45, align 4
  store float %extract21.i, float addrspace(3)* %48, align 4
  store float %extract22.i, float addrspace(3)* %51, align 4
  store float %extract23.i, float addrspace(3)* %54, align 4
  store float %extract24.i, float addrspace(3)* %57, align 4
  store float %extract25.i, float addrspace(3)* %60, align 4
  store float %extract26.i, float addrspace(3)* %63, align 4
  store float %extract27.i, float addrspace(3)* %66, align 4
  store float %extract28.i, float addrspace(3)* %69, align 4
  store float %extract29.i, float addrspace(3)* %72, align 4
  store float %extract30.i, float addrspace(3)* %75, align 4
  store float %extract31.i, float addrspace(3)* %78, align 4
  store float %extract32.i, float addrspace(3)* %81, align 4
  store float %extract33.i, float addrspace(3)* %84, align 4
  %85 = add nsw <16 x i32> %vectorPHI.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %86 = and <16 x i32> %85, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %87 = extractelement <16 x i32> %86, i32 0
  %88 = zext i32 %87 to i64
  %89 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %88
  %90 = extractelement <16 x i32> %86, i32 1
  %91 = zext i32 %90 to i64
  %92 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %91
  %93 = extractelement <16 x i32> %86, i32 2
  %94 = zext i32 %93 to i64
  %95 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %94
  %96 = extractelement <16 x i32> %86, i32 3
  %97 = zext i32 %96 to i64
  %98 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %97
  %99 = extractelement <16 x i32> %86, i32 4
  %100 = zext i32 %99 to i64
  %101 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %100
  %102 = extractelement <16 x i32> %86, i32 5
  %103 = zext i32 %102 to i64
  %104 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %103
  %105 = extractelement <16 x i32> %86, i32 6
  %106 = zext i32 %105 to i64
  %107 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %106
  %108 = extractelement <16 x i32> %86, i32 7
  %109 = zext i32 %108 to i64
  %110 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %109
  %111 = extractelement <16 x i32> %86, i32 8
  %112 = zext i32 %111 to i64
  %113 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %112
  %114 = extractelement <16 x i32> %86, i32 9
  %115 = zext i32 %114 to i64
  %116 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %115
  %117 = extractelement <16 x i32> %86, i32 10
  %118 = zext i32 %117 to i64
  %119 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %118
  %120 = extractelement <16 x i32> %86, i32 11
  %121 = zext i32 %120 to i64
  %122 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %121
  %123 = extractelement <16 x i32> %86, i32 12
  %124 = zext i32 %123 to i64
  %125 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %124
  %126 = extractelement <16 x i32> %86, i32 13
  %127 = zext i32 %126 to i64
  %128 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %127
  %129 = extractelement <16 x i32> %86, i32 14
  %130 = zext i32 %129 to i64
  %131 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %130
  %132 = extractelement <16 x i32> %86, i32 15
  %133 = zext i32 %132 to i64
  %134 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %133
  store float %extract18.i, float addrspace(3)* %89, align 4
  store float %extract19.i, float addrspace(3)* %92, align 4
  store float %extract20.i, float addrspace(3)* %95, align 4
  store float %extract21.i, float addrspace(3)* %98, align 4
  store float %extract22.i, float addrspace(3)* %101, align 4
  store float %extract23.i, float addrspace(3)* %104, align 4
  store float %extract24.i, float addrspace(3)* %107, align 4
  store float %extract25.i, float addrspace(3)* %110, align 4
  store float %extract26.i, float addrspace(3)* %113, align 4
  store float %extract27.i, float addrspace(3)* %116, align 4
  store float %extract28.i, float addrspace(3)* %119, align 4
  store float %extract29.i, float addrspace(3)* %122, align 4
  store float %extract30.i, float addrspace(3)* %125, align 4
  store float %extract31.i, float addrspace(3)* %128, align 4
  store float %extract32.i, float addrspace(3)* %131, align 4
  store float %extract33.i, float addrspace(3)* %134, align 4
  %135 = add nsw <16 x i32> %vectorPHI.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %136 = and <16 x i32> %135, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %137 = extractelement <16 x i32> %136, i32 0
  %138 = zext i32 %137 to i64
  %139 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %138
  %140 = extractelement <16 x i32> %136, i32 1
  %141 = zext i32 %140 to i64
  %142 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %141
  %143 = extractelement <16 x i32> %136, i32 2
  %144 = zext i32 %143 to i64
  %145 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %144
  %146 = extractelement <16 x i32> %136, i32 3
  %147 = zext i32 %146 to i64
  %148 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %147
  %149 = extractelement <16 x i32> %136, i32 4
  %150 = zext i32 %149 to i64
  %151 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %150
  %152 = extractelement <16 x i32> %136, i32 5
  %153 = zext i32 %152 to i64
  %154 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %153
  %155 = extractelement <16 x i32> %136, i32 6
  %156 = zext i32 %155 to i64
  %157 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %156
  %158 = extractelement <16 x i32> %136, i32 7
  %159 = zext i32 %158 to i64
  %160 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %159
  %161 = extractelement <16 x i32> %136, i32 8
  %162 = zext i32 %161 to i64
  %163 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %162
  %164 = extractelement <16 x i32> %136, i32 9
  %165 = zext i32 %164 to i64
  %166 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %165
  %167 = extractelement <16 x i32> %136, i32 10
  %168 = zext i32 %167 to i64
  %169 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %168
  %170 = extractelement <16 x i32> %136, i32 11
  %171 = zext i32 %170 to i64
  %172 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %171
  %173 = extractelement <16 x i32> %136, i32 12
  %174 = zext i32 %173 to i64
  %175 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %174
  %176 = extractelement <16 x i32> %136, i32 13
  %177 = zext i32 %176 to i64
  %178 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %177
  %179 = extractelement <16 x i32> %136, i32 14
  %180 = zext i32 %179 to i64
  %181 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %180
  %182 = extractelement <16 x i32> %136, i32 15
  %183 = zext i32 %182 to i64
  %184 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %183
  store float %extract18.i, float addrspace(3)* %139, align 4
  store float %extract19.i, float addrspace(3)* %142, align 4
  store float %extract20.i, float addrspace(3)* %145, align 4
  store float %extract21.i, float addrspace(3)* %148, align 4
  store float %extract22.i, float addrspace(3)* %151, align 4
  store float %extract23.i, float addrspace(3)* %154, align 4
  store float %extract24.i, float addrspace(3)* %157, align 4
  store float %extract25.i, float addrspace(3)* %160, align 4
  store float %extract26.i, float addrspace(3)* %163, align 4
  store float %extract27.i, float addrspace(3)* %166, align 4
  store float %extract28.i, float addrspace(3)* %169, align 4
  store float %extract29.i, float addrspace(3)* %172, align 4
  store float %extract30.i, float addrspace(3)* %175, align 4
  store float %extract31.i, float addrspace(3)* %178, align 4
  store float %extract32.i, float addrspace(3)* %181, align 4
  store float %extract33.i, float addrspace(3)* %184, align 4
  %185 = add nsw <16 x i32> %vectorPHI.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %186 = and <16 x i32> %185, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %187 = extractelement <16 x i32> %186, i32 0
  %188 = zext i32 %187 to i64
  %189 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %188
  %190 = extractelement <16 x i32> %186, i32 1
  %191 = zext i32 %190 to i64
  %192 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %191
  %193 = extractelement <16 x i32> %186, i32 2
  %194 = zext i32 %193 to i64
  %195 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %194
  %196 = extractelement <16 x i32> %186, i32 3
  %197 = zext i32 %196 to i64
  %198 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %197
  %199 = extractelement <16 x i32> %186, i32 4
  %200 = zext i32 %199 to i64
  %201 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %200
  %202 = extractelement <16 x i32> %186, i32 5
  %203 = zext i32 %202 to i64
  %204 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %203
  %205 = extractelement <16 x i32> %186, i32 6
  %206 = zext i32 %205 to i64
  %207 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %206
  %208 = extractelement <16 x i32> %186, i32 7
  %209 = zext i32 %208 to i64
  %210 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %209
  %211 = extractelement <16 x i32> %186, i32 8
  %212 = zext i32 %211 to i64
  %213 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %212
  %214 = extractelement <16 x i32> %186, i32 9
  %215 = zext i32 %214 to i64
  %216 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %215
  %217 = extractelement <16 x i32> %186, i32 10
  %218 = zext i32 %217 to i64
  %219 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %218
  %220 = extractelement <16 x i32> %186, i32 11
  %221 = zext i32 %220 to i64
  %222 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %221
  %223 = extractelement <16 x i32> %186, i32 12
  %224 = zext i32 %223 to i64
  %225 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %224
  %226 = extractelement <16 x i32> %186, i32 13
  %227 = zext i32 %226 to i64
  %228 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %227
  %229 = extractelement <16 x i32> %186, i32 14
  %230 = zext i32 %229 to i64
  %231 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %230
  %232 = extractelement <16 x i32> %186, i32 15
  %233 = zext i32 %232 to i64
  %234 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %233
  store float %extract18.i, float addrspace(3)* %189, align 4
  store float %extract19.i, float addrspace(3)* %192, align 4
  store float %extract20.i, float addrspace(3)* %195, align 4
  store float %extract21.i, float addrspace(3)* %198, align 4
  store float %extract22.i, float addrspace(3)* %201, align 4
  store float %extract23.i, float addrspace(3)* %204, align 4
  store float %extract24.i, float addrspace(3)* %207, align 4
  store float %extract25.i, float addrspace(3)* %210, align 4
  store float %extract26.i, float addrspace(3)* %213, align 4
  store float %extract27.i, float addrspace(3)* %216, align 4
  store float %extract28.i, float addrspace(3)* %219, align 4
  store float %extract29.i, float addrspace(3)* %222, align 4
  store float %extract30.i, float addrspace(3)* %225, align 4
  store float %extract31.i, float addrspace(3)* %228, align 4
  store float %extract32.i, float addrspace(3)* %231, align 4
  store float %extract33.i, float addrspace(3)* %234, align 4
  %235 = add nsw <16 x i32> %vectorPHI.i, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %236 = and <16 x i32> %235, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %237 = extractelement <16 x i32> %236, i32 0
  %238 = zext i32 %237 to i64
  %239 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %238
  %240 = extractelement <16 x i32> %236, i32 1
  %241 = zext i32 %240 to i64
  %242 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %241
  %243 = extractelement <16 x i32> %236, i32 2
  %244 = zext i32 %243 to i64
  %245 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %244
  %246 = extractelement <16 x i32> %236, i32 3
  %247 = zext i32 %246 to i64
  %248 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %247
  %249 = extractelement <16 x i32> %236, i32 4
  %250 = zext i32 %249 to i64
  %251 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %250
  %252 = extractelement <16 x i32> %236, i32 5
  %253 = zext i32 %252 to i64
  %254 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %253
  %255 = extractelement <16 x i32> %236, i32 6
  %256 = zext i32 %255 to i64
  %257 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %256
  %258 = extractelement <16 x i32> %236, i32 7
  %259 = zext i32 %258 to i64
  %260 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %259
  %261 = extractelement <16 x i32> %236, i32 8
  %262 = zext i32 %261 to i64
  %263 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %262
  %264 = extractelement <16 x i32> %236, i32 9
  %265 = zext i32 %264 to i64
  %266 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %265
  %267 = extractelement <16 x i32> %236, i32 10
  %268 = zext i32 %267 to i64
  %269 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %268
  %270 = extractelement <16 x i32> %236, i32 11
  %271 = zext i32 %270 to i64
  %272 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %271
  %273 = extractelement <16 x i32> %236, i32 12
  %274 = zext i32 %273 to i64
  %275 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %274
  %276 = extractelement <16 x i32> %236, i32 13
  %277 = zext i32 %276 to i64
  %278 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %277
  %279 = extractelement <16 x i32> %236, i32 14
  %280 = zext i32 %279 to i64
  %281 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %280
  %282 = extractelement <16 x i32> %236, i32 15
  %283 = zext i32 %282 to i64
  %284 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %283
  store float %extract18.i, float addrspace(3)* %239, align 4
  store float %extract19.i, float addrspace(3)* %242, align 4
  store float %extract20.i, float addrspace(3)* %245, align 4
  store float %extract21.i, float addrspace(3)* %248, align 4
  store float %extract22.i, float addrspace(3)* %251, align 4
  store float %extract23.i, float addrspace(3)* %254, align 4
  store float %extract24.i, float addrspace(3)* %257, align 4
  store float %extract25.i, float addrspace(3)* %260, align 4
  store float %extract26.i, float addrspace(3)* %263, align 4
  store float %extract27.i, float addrspace(3)* %266, align 4
  store float %extract28.i, float addrspace(3)* %269, align 4
  store float %extract29.i, float addrspace(3)* %272, align 4
  store float %extract30.i, float addrspace(3)* %275, align 4
  store float %extract31.i, float addrspace(3)* %278, align 4
  store float %extract32.i, float addrspace(3)* %281, align 4
  store float %extract33.i, float addrspace(3)* %284, align 4
  %285 = add nsw <16 x i32> %vectorPHI.i, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  %286 = and <16 x i32> %285, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %287 = extractelement <16 x i32> %286, i32 0
  %288 = zext i32 %287 to i64
  %289 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %288
  %290 = extractelement <16 x i32> %286, i32 1
  %291 = zext i32 %290 to i64
  %292 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %291
  %293 = extractelement <16 x i32> %286, i32 2
  %294 = zext i32 %293 to i64
  %295 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %294
  %296 = extractelement <16 x i32> %286, i32 3
  %297 = zext i32 %296 to i64
  %298 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %297
  %299 = extractelement <16 x i32> %286, i32 4
  %300 = zext i32 %299 to i64
  %301 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %300
  %302 = extractelement <16 x i32> %286, i32 5
  %303 = zext i32 %302 to i64
  %304 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %303
  %305 = extractelement <16 x i32> %286, i32 6
  %306 = zext i32 %305 to i64
  %307 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %306
  %308 = extractelement <16 x i32> %286, i32 7
  %309 = zext i32 %308 to i64
  %310 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %309
  %311 = extractelement <16 x i32> %286, i32 8
  %312 = zext i32 %311 to i64
  %313 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %312
  %314 = extractelement <16 x i32> %286, i32 9
  %315 = zext i32 %314 to i64
  %316 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %315
  %317 = extractelement <16 x i32> %286, i32 10
  %318 = zext i32 %317 to i64
  %319 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %318
  %320 = extractelement <16 x i32> %286, i32 11
  %321 = zext i32 %320 to i64
  %322 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %321
  %323 = extractelement <16 x i32> %286, i32 12
  %324 = zext i32 %323 to i64
  %325 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %324
  %326 = extractelement <16 x i32> %286, i32 13
  %327 = zext i32 %326 to i64
  %328 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %327
  %329 = extractelement <16 x i32> %286, i32 14
  %330 = zext i32 %329 to i64
  %331 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %330
  %332 = extractelement <16 x i32> %286, i32 15
  %333 = zext i32 %332 to i64
  %334 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %333
  store float %extract18.i, float addrspace(3)* %289, align 4
  store float %extract19.i, float addrspace(3)* %292, align 4
  store float %extract20.i, float addrspace(3)* %295, align 4
  store float %extract21.i, float addrspace(3)* %298, align 4
  store float %extract22.i, float addrspace(3)* %301, align 4
  store float %extract23.i, float addrspace(3)* %304, align 4
  store float %extract24.i, float addrspace(3)* %307, align 4
  store float %extract25.i, float addrspace(3)* %310, align 4
  store float %extract26.i, float addrspace(3)* %313, align 4
  store float %extract27.i, float addrspace(3)* %316, align 4
  store float %extract28.i, float addrspace(3)* %319, align 4
  store float %extract29.i, float addrspace(3)* %322, align 4
  store float %extract30.i, float addrspace(3)* %325, align 4
  store float %extract31.i, float addrspace(3)* %328, align 4
  store float %extract32.i, float addrspace(3)* %331, align 4
  store float %extract33.i, float addrspace(3)* %334, align 4
  %335 = add nsw <16 x i32> %vectorPHI.i, <i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6>
  %336 = and <16 x i32> %335, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %337 = extractelement <16 x i32> %336, i32 0
  %338 = zext i32 %337 to i64
  %339 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %338
  %340 = extractelement <16 x i32> %336, i32 1
  %341 = zext i32 %340 to i64
  %342 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %341
  %343 = extractelement <16 x i32> %336, i32 2
  %344 = zext i32 %343 to i64
  %345 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %344
  %346 = extractelement <16 x i32> %336, i32 3
  %347 = zext i32 %346 to i64
  %348 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %347
  %349 = extractelement <16 x i32> %336, i32 4
  %350 = zext i32 %349 to i64
  %351 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %350
  %352 = extractelement <16 x i32> %336, i32 5
  %353 = zext i32 %352 to i64
  %354 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %353
  %355 = extractelement <16 x i32> %336, i32 6
  %356 = zext i32 %355 to i64
  %357 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %356
  %358 = extractelement <16 x i32> %336, i32 7
  %359 = zext i32 %358 to i64
  %360 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %359
  %361 = extractelement <16 x i32> %336, i32 8
  %362 = zext i32 %361 to i64
  %363 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %362
  %364 = extractelement <16 x i32> %336, i32 9
  %365 = zext i32 %364 to i64
  %366 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %365
  %367 = extractelement <16 x i32> %336, i32 10
  %368 = zext i32 %367 to i64
  %369 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %368
  %370 = extractelement <16 x i32> %336, i32 11
  %371 = zext i32 %370 to i64
  %372 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %371
  %373 = extractelement <16 x i32> %336, i32 12
  %374 = zext i32 %373 to i64
  %375 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %374
  %376 = extractelement <16 x i32> %336, i32 13
  %377 = zext i32 %376 to i64
  %378 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %377
  %379 = extractelement <16 x i32> %336, i32 14
  %380 = zext i32 %379 to i64
  %381 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %380
  %382 = extractelement <16 x i32> %336, i32 15
  %383 = zext i32 %382 to i64
  %384 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %383
  store float %extract18.i, float addrspace(3)* %339, align 4
  store float %extract19.i, float addrspace(3)* %342, align 4
  store float %extract20.i, float addrspace(3)* %345, align 4
  store float %extract21.i, float addrspace(3)* %348, align 4
  store float %extract22.i, float addrspace(3)* %351, align 4
  store float %extract23.i, float addrspace(3)* %354, align 4
  store float %extract24.i, float addrspace(3)* %357, align 4
  store float %extract25.i, float addrspace(3)* %360, align 4
  store float %extract26.i, float addrspace(3)* %363, align 4
  store float %extract27.i, float addrspace(3)* %366, align 4
  store float %extract28.i, float addrspace(3)* %369, align 4
  store float %extract29.i, float addrspace(3)* %372, align 4
  store float %extract30.i, float addrspace(3)* %375, align 4
  store float %extract31.i, float addrspace(3)* %378, align 4
  store float %extract32.i, float addrspace(3)* %381, align 4
  store float %extract33.i, float addrspace(3)* %384, align 4
  %385 = add nsw <16 x i32> %vectorPHI.i, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %386 = and <16 x i32> %385, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %387 = extractelement <16 x i32> %386, i32 0
  %388 = zext i32 %387 to i64
  %389 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %388
  %390 = extractelement <16 x i32> %386, i32 1
  %391 = zext i32 %390 to i64
  %392 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %391
  %393 = extractelement <16 x i32> %386, i32 2
  %394 = zext i32 %393 to i64
  %395 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %394
  %396 = extractelement <16 x i32> %386, i32 3
  %397 = zext i32 %396 to i64
  %398 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %397
  %399 = extractelement <16 x i32> %386, i32 4
  %400 = zext i32 %399 to i64
  %401 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %400
  %402 = extractelement <16 x i32> %386, i32 5
  %403 = zext i32 %402 to i64
  %404 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %403
  %405 = extractelement <16 x i32> %386, i32 6
  %406 = zext i32 %405 to i64
  %407 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %406
  %408 = extractelement <16 x i32> %386, i32 7
  %409 = zext i32 %408 to i64
  %410 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %409
  %411 = extractelement <16 x i32> %386, i32 8
  %412 = zext i32 %411 to i64
  %413 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %412
  %414 = extractelement <16 x i32> %386, i32 9
  %415 = zext i32 %414 to i64
  %416 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %415
  %417 = extractelement <16 x i32> %386, i32 10
  %418 = zext i32 %417 to i64
  %419 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %418
  %420 = extractelement <16 x i32> %386, i32 11
  %421 = zext i32 %420 to i64
  %422 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %421
  %423 = extractelement <16 x i32> %386, i32 12
  %424 = zext i32 %423 to i64
  %425 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %424
  %426 = extractelement <16 x i32> %386, i32 13
  %427 = zext i32 %426 to i64
  %428 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %427
  %429 = extractelement <16 x i32> %386, i32 14
  %430 = zext i32 %429 to i64
  %431 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %430
  %432 = extractelement <16 x i32> %386, i32 15
  %433 = zext i32 %432 to i64
  %434 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %433
  store float %extract18.i, float addrspace(3)* %389, align 4
  store float %extract19.i, float addrspace(3)* %392, align 4
  store float %extract20.i, float addrspace(3)* %395, align 4
  store float %extract21.i, float addrspace(3)* %398, align 4
  store float %extract22.i, float addrspace(3)* %401, align 4
  store float %extract23.i, float addrspace(3)* %404, align 4
  store float %extract24.i, float addrspace(3)* %407, align 4
  store float %extract25.i, float addrspace(3)* %410, align 4
  store float %extract26.i, float addrspace(3)* %413, align 4
  store float %extract27.i, float addrspace(3)* %416, align 4
  store float %extract28.i, float addrspace(3)* %419, align 4
  store float %extract29.i, float addrspace(3)* %422, align 4
  store float %extract30.i, float addrspace(3)* %425, align 4
  store float %extract31.i, float addrspace(3)* %428, align 4
  store float %extract32.i, float addrspace(3)* %431, align 4
  store float %extract33.i, float addrspace(3)* %434, align 4
  %435 = add nsw <16 x i32> %vectorPHI.i, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %436 = and <16 x i32> %435, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %437 = extractelement <16 x i32> %436, i32 0
  %438 = zext i32 %437 to i64
  %439 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %438
  %440 = extractelement <16 x i32> %436, i32 1
  %441 = zext i32 %440 to i64
  %442 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %441
  %443 = extractelement <16 x i32> %436, i32 2
  %444 = zext i32 %443 to i64
  %445 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %444
  %446 = extractelement <16 x i32> %436, i32 3
  %447 = zext i32 %446 to i64
  %448 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %447
  %449 = extractelement <16 x i32> %436, i32 4
  %450 = zext i32 %449 to i64
  %451 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %450
  %452 = extractelement <16 x i32> %436, i32 5
  %453 = zext i32 %452 to i64
  %454 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %453
  %455 = extractelement <16 x i32> %436, i32 6
  %456 = zext i32 %455 to i64
  %457 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %456
  %458 = extractelement <16 x i32> %436, i32 7
  %459 = zext i32 %458 to i64
  %460 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %459
  %461 = extractelement <16 x i32> %436, i32 8
  %462 = zext i32 %461 to i64
  %463 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %462
  %464 = extractelement <16 x i32> %436, i32 9
  %465 = zext i32 %464 to i64
  %466 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %465
  %467 = extractelement <16 x i32> %436, i32 10
  %468 = zext i32 %467 to i64
  %469 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %468
  %470 = extractelement <16 x i32> %436, i32 11
  %471 = zext i32 %470 to i64
  %472 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %471
  %473 = extractelement <16 x i32> %436, i32 12
  %474 = zext i32 %473 to i64
  %475 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %474
  %476 = extractelement <16 x i32> %436, i32 13
  %477 = zext i32 %476 to i64
  %478 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %477
  %479 = extractelement <16 x i32> %436, i32 14
  %480 = zext i32 %479 to i64
  %481 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %480
  %482 = extractelement <16 x i32> %436, i32 15
  %483 = zext i32 %482 to i64
  %484 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %483
  store float %extract18.i, float addrspace(3)* %439, align 4
  store float %extract19.i, float addrspace(3)* %442, align 4
  store float %extract20.i, float addrspace(3)* %445, align 4
  store float %extract21.i, float addrspace(3)* %448, align 4
  store float %extract22.i, float addrspace(3)* %451, align 4
  store float %extract23.i, float addrspace(3)* %454, align 4
  store float %extract24.i, float addrspace(3)* %457, align 4
  store float %extract25.i, float addrspace(3)* %460, align 4
  store float %extract26.i, float addrspace(3)* %463, align 4
  store float %extract27.i, float addrspace(3)* %466, align 4
  store float %extract28.i, float addrspace(3)* %469, align 4
  store float %extract29.i, float addrspace(3)* %472, align 4
  store float %extract30.i, float addrspace(3)* %475, align 4
  store float %extract31.i, float addrspace(3)* %478, align 4
  store float %extract32.i, float addrspace(3)* %481, align 4
  store float %extract33.i, float addrspace(3)* %484, align 4
  %485 = add nsw <16 x i32> %vectorPHI.i, <i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9>
  %486 = and <16 x i32> %485, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %487 = extractelement <16 x i32> %486, i32 0
  %488 = zext i32 %487 to i64
  %489 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %488
  %490 = extractelement <16 x i32> %486, i32 1
  %491 = zext i32 %490 to i64
  %492 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %491
  %493 = extractelement <16 x i32> %486, i32 2
  %494 = zext i32 %493 to i64
  %495 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %494
  %496 = extractelement <16 x i32> %486, i32 3
  %497 = zext i32 %496 to i64
  %498 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %497
  %499 = extractelement <16 x i32> %486, i32 4
  %500 = zext i32 %499 to i64
  %501 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %500
  %502 = extractelement <16 x i32> %486, i32 5
  %503 = zext i32 %502 to i64
  %504 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %503
  %505 = extractelement <16 x i32> %486, i32 6
  %506 = zext i32 %505 to i64
  %507 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %506
  %508 = extractelement <16 x i32> %486, i32 7
  %509 = zext i32 %508 to i64
  %510 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %509
  %511 = extractelement <16 x i32> %486, i32 8
  %512 = zext i32 %511 to i64
  %513 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %512
  %514 = extractelement <16 x i32> %486, i32 9
  %515 = zext i32 %514 to i64
  %516 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %515
  %517 = extractelement <16 x i32> %486, i32 10
  %518 = zext i32 %517 to i64
  %519 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %518
  %520 = extractelement <16 x i32> %486, i32 11
  %521 = zext i32 %520 to i64
  %522 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %521
  %523 = extractelement <16 x i32> %486, i32 12
  %524 = zext i32 %523 to i64
  %525 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %524
  %526 = extractelement <16 x i32> %486, i32 13
  %527 = zext i32 %526 to i64
  %528 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %527
  %529 = extractelement <16 x i32> %486, i32 14
  %530 = zext i32 %529 to i64
  %531 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %530
  %532 = extractelement <16 x i32> %486, i32 15
  %533 = zext i32 %532 to i64
  %534 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %533
  store float %extract18.i, float addrspace(3)* %489, align 4
  store float %extract19.i, float addrspace(3)* %492, align 4
  store float %extract20.i, float addrspace(3)* %495, align 4
  store float %extract21.i, float addrspace(3)* %498, align 4
  store float %extract22.i, float addrspace(3)* %501, align 4
  store float %extract23.i, float addrspace(3)* %504, align 4
  store float %extract24.i, float addrspace(3)* %507, align 4
  store float %extract25.i, float addrspace(3)* %510, align 4
  store float %extract26.i, float addrspace(3)* %513, align 4
  store float %extract27.i, float addrspace(3)* %516, align 4
  store float %extract28.i, float addrspace(3)* %519, align 4
  store float %extract29.i, float addrspace(3)* %522, align 4
  store float %extract30.i, float addrspace(3)* %525, align 4
  store float %extract31.i, float addrspace(3)* %528, align 4
  store float %extract32.i, float addrspace(3)* %531, align 4
  store float %extract33.i, float addrspace(3)* %534, align 4
  %535 = add nsw <16 x i32> %vectorPHI.i, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %536 = and <16 x i32> %535, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %537 = extractelement <16 x i32> %536, i32 0
  %538 = zext i32 %537 to i64
  %539 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %538
  %540 = extractelement <16 x i32> %536, i32 1
  %541 = zext i32 %540 to i64
  %542 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %541
  %543 = extractelement <16 x i32> %536, i32 2
  %544 = zext i32 %543 to i64
  %545 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %544
  %546 = extractelement <16 x i32> %536, i32 3
  %547 = zext i32 %546 to i64
  %548 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %547
  %549 = extractelement <16 x i32> %536, i32 4
  %550 = zext i32 %549 to i64
  %551 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %550
  %552 = extractelement <16 x i32> %536, i32 5
  %553 = zext i32 %552 to i64
  %554 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %553
  %555 = extractelement <16 x i32> %536, i32 6
  %556 = zext i32 %555 to i64
  %557 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %556
  %558 = extractelement <16 x i32> %536, i32 7
  %559 = zext i32 %558 to i64
  %560 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %559
  %561 = extractelement <16 x i32> %536, i32 8
  %562 = zext i32 %561 to i64
  %563 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %562
  %564 = extractelement <16 x i32> %536, i32 9
  %565 = zext i32 %564 to i64
  %566 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %565
  %567 = extractelement <16 x i32> %536, i32 10
  %568 = zext i32 %567 to i64
  %569 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %568
  %570 = extractelement <16 x i32> %536, i32 11
  %571 = zext i32 %570 to i64
  %572 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %571
  %573 = extractelement <16 x i32> %536, i32 12
  %574 = zext i32 %573 to i64
  %575 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %574
  %576 = extractelement <16 x i32> %536, i32 13
  %577 = zext i32 %576 to i64
  %578 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %577
  %579 = extractelement <16 x i32> %536, i32 14
  %580 = zext i32 %579 to i64
  %581 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %580
  %582 = extractelement <16 x i32> %536, i32 15
  %583 = zext i32 %582 to i64
  %584 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %583
  store float %extract18.i, float addrspace(3)* %539, align 4
  store float %extract19.i, float addrspace(3)* %542, align 4
  store float %extract20.i, float addrspace(3)* %545, align 4
  store float %extract21.i, float addrspace(3)* %548, align 4
  store float %extract22.i, float addrspace(3)* %551, align 4
  store float %extract23.i, float addrspace(3)* %554, align 4
  store float %extract24.i, float addrspace(3)* %557, align 4
  store float %extract25.i, float addrspace(3)* %560, align 4
  store float %extract26.i, float addrspace(3)* %563, align 4
  store float %extract27.i, float addrspace(3)* %566, align 4
  store float %extract28.i, float addrspace(3)* %569, align 4
  store float %extract29.i, float addrspace(3)* %572, align 4
  store float %extract30.i, float addrspace(3)* %575, align 4
  store float %extract31.i, float addrspace(3)* %578, align 4
  store float %extract32.i, float addrspace(3)* %581, align 4
  store float %extract33.i, float addrspace(3)* %584, align 4
  %585 = add nsw <16 x i32> %vectorPHI.i, <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
  %586 = and <16 x i32> %585, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %587 = extractelement <16 x i32> %586, i32 0
  %588 = zext i32 %587 to i64
  %589 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %588
  %590 = extractelement <16 x i32> %586, i32 1
  %591 = zext i32 %590 to i64
  %592 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %591
  %593 = extractelement <16 x i32> %586, i32 2
  %594 = zext i32 %593 to i64
  %595 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %594
  %596 = extractelement <16 x i32> %586, i32 3
  %597 = zext i32 %596 to i64
  %598 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %597
  %599 = extractelement <16 x i32> %586, i32 4
  %600 = zext i32 %599 to i64
  %601 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %600
  %602 = extractelement <16 x i32> %586, i32 5
  %603 = zext i32 %602 to i64
  %604 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %603
  %605 = extractelement <16 x i32> %586, i32 6
  %606 = zext i32 %605 to i64
  %607 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %606
  %608 = extractelement <16 x i32> %586, i32 7
  %609 = zext i32 %608 to i64
  %610 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %609
  %611 = extractelement <16 x i32> %586, i32 8
  %612 = zext i32 %611 to i64
  %613 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %612
  %614 = extractelement <16 x i32> %586, i32 9
  %615 = zext i32 %614 to i64
  %616 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %615
  %617 = extractelement <16 x i32> %586, i32 10
  %618 = zext i32 %617 to i64
  %619 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %618
  %620 = extractelement <16 x i32> %586, i32 11
  %621 = zext i32 %620 to i64
  %622 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %621
  %623 = extractelement <16 x i32> %586, i32 12
  %624 = zext i32 %623 to i64
  %625 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %624
  %626 = extractelement <16 x i32> %586, i32 13
  %627 = zext i32 %626 to i64
  %628 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %627
  %629 = extractelement <16 x i32> %586, i32 14
  %630 = zext i32 %629 to i64
  %631 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %630
  %632 = extractelement <16 x i32> %586, i32 15
  %633 = zext i32 %632 to i64
  %634 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %633
  store float %extract18.i, float addrspace(3)* %589, align 4
  store float %extract19.i, float addrspace(3)* %592, align 4
  store float %extract20.i, float addrspace(3)* %595, align 4
  store float %extract21.i, float addrspace(3)* %598, align 4
  store float %extract22.i, float addrspace(3)* %601, align 4
  store float %extract23.i, float addrspace(3)* %604, align 4
  store float %extract24.i, float addrspace(3)* %607, align 4
  store float %extract25.i, float addrspace(3)* %610, align 4
  store float %extract26.i, float addrspace(3)* %613, align 4
  store float %extract27.i, float addrspace(3)* %616, align 4
  store float %extract28.i, float addrspace(3)* %619, align 4
  store float %extract29.i, float addrspace(3)* %622, align 4
  store float %extract30.i, float addrspace(3)* %625, align 4
  store float %extract31.i, float addrspace(3)* %628, align 4
  store float %extract32.i, float addrspace(3)* %631, align 4
  store float %extract33.i, float addrspace(3)* %634, align 4
  %635 = add nsw <16 x i32> %vectorPHI.i, <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
  %636 = and <16 x i32> %635, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %637 = extractelement <16 x i32> %636, i32 0
  %638 = zext i32 %637 to i64
  %639 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %638
  %640 = extractelement <16 x i32> %636, i32 1
  %641 = zext i32 %640 to i64
  %642 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %641
  %643 = extractelement <16 x i32> %636, i32 2
  %644 = zext i32 %643 to i64
  %645 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %644
  %646 = extractelement <16 x i32> %636, i32 3
  %647 = zext i32 %646 to i64
  %648 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %647
  %649 = extractelement <16 x i32> %636, i32 4
  %650 = zext i32 %649 to i64
  %651 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %650
  %652 = extractelement <16 x i32> %636, i32 5
  %653 = zext i32 %652 to i64
  %654 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %653
  %655 = extractelement <16 x i32> %636, i32 6
  %656 = zext i32 %655 to i64
  %657 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %656
  %658 = extractelement <16 x i32> %636, i32 7
  %659 = zext i32 %658 to i64
  %660 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %659
  %661 = extractelement <16 x i32> %636, i32 8
  %662 = zext i32 %661 to i64
  %663 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %662
  %664 = extractelement <16 x i32> %636, i32 9
  %665 = zext i32 %664 to i64
  %666 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %665
  %667 = extractelement <16 x i32> %636, i32 10
  %668 = zext i32 %667 to i64
  %669 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %668
  %670 = extractelement <16 x i32> %636, i32 11
  %671 = zext i32 %670 to i64
  %672 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %671
  %673 = extractelement <16 x i32> %636, i32 12
  %674 = zext i32 %673 to i64
  %675 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %674
  %676 = extractelement <16 x i32> %636, i32 13
  %677 = zext i32 %676 to i64
  %678 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %677
  %679 = extractelement <16 x i32> %636, i32 14
  %680 = zext i32 %679 to i64
  %681 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %680
  %682 = extractelement <16 x i32> %636, i32 15
  %683 = zext i32 %682 to i64
  %684 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %683
  store float %extract18.i, float addrspace(3)* %639, align 4
  store float %extract19.i, float addrspace(3)* %642, align 4
  store float %extract20.i, float addrspace(3)* %645, align 4
  store float %extract21.i, float addrspace(3)* %648, align 4
  store float %extract22.i, float addrspace(3)* %651, align 4
  store float %extract23.i, float addrspace(3)* %654, align 4
  store float %extract24.i, float addrspace(3)* %657, align 4
  store float %extract25.i, float addrspace(3)* %660, align 4
  store float %extract26.i, float addrspace(3)* %663, align 4
  store float %extract27.i, float addrspace(3)* %666, align 4
  store float %extract28.i, float addrspace(3)* %669, align 4
  store float %extract29.i, float addrspace(3)* %672, align 4
  store float %extract30.i, float addrspace(3)* %675, align 4
  store float %extract31.i, float addrspace(3)* %678, align 4
  store float %extract32.i, float addrspace(3)* %681, align 4
  store float %extract33.i, float addrspace(3)* %684, align 4
  %685 = add nsw <16 x i32> %vectorPHI.i, <i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13>
  %686 = and <16 x i32> %685, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %687 = extractelement <16 x i32> %686, i32 0
  %688 = zext i32 %687 to i64
  %689 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %688
  %690 = extractelement <16 x i32> %686, i32 1
  %691 = zext i32 %690 to i64
  %692 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %691
  %693 = extractelement <16 x i32> %686, i32 2
  %694 = zext i32 %693 to i64
  %695 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %694
  %696 = extractelement <16 x i32> %686, i32 3
  %697 = zext i32 %696 to i64
  %698 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %697
  %699 = extractelement <16 x i32> %686, i32 4
  %700 = zext i32 %699 to i64
  %701 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %700
  %702 = extractelement <16 x i32> %686, i32 5
  %703 = zext i32 %702 to i64
  %704 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %703
  %705 = extractelement <16 x i32> %686, i32 6
  %706 = zext i32 %705 to i64
  %707 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %706
  %708 = extractelement <16 x i32> %686, i32 7
  %709 = zext i32 %708 to i64
  %710 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %709
  %711 = extractelement <16 x i32> %686, i32 8
  %712 = zext i32 %711 to i64
  %713 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %712
  %714 = extractelement <16 x i32> %686, i32 9
  %715 = zext i32 %714 to i64
  %716 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %715
  %717 = extractelement <16 x i32> %686, i32 10
  %718 = zext i32 %717 to i64
  %719 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %718
  %720 = extractelement <16 x i32> %686, i32 11
  %721 = zext i32 %720 to i64
  %722 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %721
  %723 = extractelement <16 x i32> %686, i32 12
  %724 = zext i32 %723 to i64
  %725 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %724
  %726 = extractelement <16 x i32> %686, i32 13
  %727 = zext i32 %726 to i64
  %728 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %727
  %729 = extractelement <16 x i32> %686, i32 14
  %730 = zext i32 %729 to i64
  %731 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %730
  %732 = extractelement <16 x i32> %686, i32 15
  %733 = zext i32 %732 to i64
  %734 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %733
  store float %extract18.i, float addrspace(3)* %689, align 4
  store float %extract19.i, float addrspace(3)* %692, align 4
  store float %extract20.i, float addrspace(3)* %695, align 4
  store float %extract21.i, float addrspace(3)* %698, align 4
  store float %extract22.i, float addrspace(3)* %701, align 4
  store float %extract23.i, float addrspace(3)* %704, align 4
  store float %extract24.i, float addrspace(3)* %707, align 4
  store float %extract25.i, float addrspace(3)* %710, align 4
  store float %extract26.i, float addrspace(3)* %713, align 4
  store float %extract27.i, float addrspace(3)* %716, align 4
  store float %extract28.i, float addrspace(3)* %719, align 4
  store float %extract29.i, float addrspace(3)* %722, align 4
  store float %extract30.i, float addrspace(3)* %725, align 4
  store float %extract31.i, float addrspace(3)* %728, align 4
  store float %extract32.i, float addrspace(3)* %731, align 4
  store float %extract33.i, float addrspace(3)* %734, align 4
  %735 = add nsw <16 x i32> %vectorPHI.i, <i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14>
  %736 = and <16 x i32> %735, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %737 = extractelement <16 x i32> %736, i32 0
  %738 = zext i32 %737 to i64
  %739 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %738
  %740 = extractelement <16 x i32> %736, i32 1
  %741 = zext i32 %740 to i64
  %742 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %741
  %743 = extractelement <16 x i32> %736, i32 2
  %744 = zext i32 %743 to i64
  %745 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %744
  %746 = extractelement <16 x i32> %736, i32 3
  %747 = zext i32 %746 to i64
  %748 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %747
  %749 = extractelement <16 x i32> %736, i32 4
  %750 = zext i32 %749 to i64
  %751 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %750
  %752 = extractelement <16 x i32> %736, i32 5
  %753 = zext i32 %752 to i64
  %754 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %753
  %755 = extractelement <16 x i32> %736, i32 6
  %756 = zext i32 %755 to i64
  %757 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %756
  %758 = extractelement <16 x i32> %736, i32 7
  %759 = zext i32 %758 to i64
  %760 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %759
  %761 = extractelement <16 x i32> %736, i32 8
  %762 = zext i32 %761 to i64
  %763 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %762
  %764 = extractelement <16 x i32> %736, i32 9
  %765 = zext i32 %764 to i64
  %766 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %765
  %767 = extractelement <16 x i32> %736, i32 10
  %768 = zext i32 %767 to i64
  %769 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %768
  %770 = extractelement <16 x i32> %736, i32 11
  %771 = zext i32 %770 to i64
  %772 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %771
  %773 = extractelement <16 x i32> %736, i32 12
  %774 = zext i32 %773 to i64
  %775 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %774
  %776 = extractelement <16 x i32> %736, i32 13
  %777 = zext i32 %776 to i64
  %778 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %777
  %779 = extractelement <16 x i32> %736, i32 14
  %780 = zext i32 %779 to i64
  %781 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %780
  %782 = extractelement <16 x i32> %736, i32 15
  %783 = zext i32 %782 to i64
  %784 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %783
  store float %extract18.i, float addrspace(3)* %739, align 4
  store float %extract19.i, float addrspace(3)* %742, align 4
  store float %extract20.i, float addrspace(3)* %745, align 4
  store float %extract21.i, float addrspace(3)* %748, align 4
  store float %extract22.i, float addrspace(3)* %751, align 4
  store float %extract23.i, float addrspace(3)* %754, align 4
  store float %extract24.i, float addrspace(3)* %757, align 4
  store float %extract25.i, float addrspace(3)* %760, align 4
  store float %extract26.i, float addrspace(3)* %763, align 4
  store float %extract27.i, float addrspace(3)* %766, align 4
  store float %extract28.i, float addrspace(3)* %769, align 4
  store float %extract29.i, float addrspace(3)* %772, align 4
  store float %extract30.i, float addrspace(3)* %775, align 4
  store float %extract31.i, float addrspace(3)* %778, align 4
  store float %extract32.i, float addrspace(3)* %781, align 4
  store float %extract33.i, float addrspace(3)* %784, align 4
  %785 = add nsw <16 x i32> %vectorPHI.i, <i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15>
  %786 = and <16 x i32> %785, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %787 = extractelement <16 x i32> %786, i32 0
  %788 = zext i32 %787 to i64
  %789 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %788
  %790 = extractelement <16 x i32> %786, i32 1
  %791 = zext i32 %790 to i64
  %792 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %791
  %793 = extractelement <16 x i32> %786, i32 2
  %794 = zext i32 %793 to i64
  %795 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %794
  %796 = extractelement <16 x i32> %786, i32 3
  %797 = zext i32 %796 to i64
  %798 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %797
  %799 = extractelement <16 x i32> %786, i32 4
  %800 = zext i32 %799 to i64
  %801 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %800
  %802 = extractelement <16 x i32> %786, i32 5
  %803 = zext i32 %802 to i64
  %804 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %803
  %805 = extractelement <16 x i32> %786, i32 6
  %806 = zext i32 %805 to i64
  %807 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %806
  %808 = extractelement <16 x i32> %786, i32 7
  %809 = zext i32 %808 to i64
  %810 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %809
  %811 = extractelement <16 x i32> %786, i32 8
  %812 = zext i32 %811 to i64
  %813 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %812
  %814 = extractelement <16 x i32> %786, i32 9
  %815 = zext i32 %814 to i64
  %816 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %815
  %817 = extractelement <16 x i32> %786, i32 10
  %818 = zext i32 %817 to i64
  %819 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %818
  %820 = extractelement <16 x i32> %786, i32 11
  %821 = zext i32 %820 to i64
  %822 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %821
  %823 = extractelement <16 x i32> %786, i32 12
  %824 = zext i32 %823 to i64
  %825 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %824
  %826 = extractelement <16 x i32> %786, i32 13
  %827 = zext i32 %826 to i64
  %828 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %827
  %829 = extractelement <16 x i32> %786, i32 14
  %830 = zext i32 %829 to i64
  %831 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %830
  %832 = extractelement <16 x i32> %786, i32 15
  %833 = zext i32 %832 to i64
  %834 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %833
  store float %extract18.i, float addrspace(3)* %789, align 4
  store float %extract19.i, float addrspace(3)* %792, align 4
  store float %extract20.i, float addrspace(3)* %795, align 4
  store float %extract21.i, float addrspace(3)* %798, align 4
  store float %extract22.i, float addrspace(3)* %801, align 4
  store float %extract23.i, float addrspace(3)* %804, align 4
  store float %extract24.i, float addrspace(3)* %807, align 4
  store float %extract25.i, float addrspace(3)* %810, align 4
  store float %extract26.i, float addrspace(3)* %813, align 4
  store float %extract27.i, float addrspace(3)* %816, align 4
  store float %extract28.i, float addrspace(3)* %819, align 4
  store float %extract29.i, float addrspace(3)* %822, align 4
  store float %extract30.i, float addrspace(3)* %825, align 4
  store float %extract31.i, float addrspace(3)* %828, align 4
  store float %extract32.i, float addrspace(3)* %831, align 4
  store float %extract33.i, float addrspace(3)* %834, align 4
  %835 = add nsw i32 %j.02.i, 1
  %836 = add nsw <16 x i32> %vectorPHI.i, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %837 = and <16 x i32> %836, <i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095, i32 4095>
  %exitcond6.i = icmp eq i32 %835, 3000
  br i1 %exitcond6.i, label %._crit_edge5.i, label %35

._crit_edge5.i:                                   ; preds = %35
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %._crit_edge5.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 192
  br label %SyncBB323.i

elseBB.i:                                         ; preds = %._crit_edge5.i
  %838 = trunc i64 %33 to i32
  %839 = icmp eq i32 %838, 0
  %840 = select i1 %839, i32 1, i32 %838
  %841 = sdiv i32 4096, %840
  %842 = icmp sgt i32 %841, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB325.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride331.i", %thenBB325.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++329.i", %thenBB325.i ]
  br i1 %842, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)316.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset317.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)316.i"
  %CastToValueType318.i = bitcast i8* %"&pSB[currWI].offset317.i" to <16 x i32>*
  %loadedValue319.i = load <16 x i32>* %CastToValueType318.i, align 64
  %843 = extractelement <16 x i32> %loadedValue319.i, i32 0
  %extract274.i = sext i32 %843 to i64
  %844 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %extract274.i
  %"&(pSB[currWI].offset)308.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset309.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)308.i"
  %CastToValueType310.i = bitcast i8* %"&pSB[currWI].offset309.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType310.i, align 64
  %845 = extractelement <16 x i32> %loadedValue.i, i32 0
  %extract290.i = sext i32 %845 to i64
  %846 = getelementptr inbounds float addrspace(1)* %1, i64 %extract290.i
  %ptrTypeCast.i = bitcast float addrspace(3)* %844 to <16 x float> addrspace(3)*
  %ptrTypeCast306.i = bitcast float addrspace(1)* %846 to <16 x float> addrspace(1)*
  br label %847

; <label>:847                                     ; preds = %847, %bb.nph.i
  %j.11.i = phi i32 [ 0, %bb.nph.i ], [ %849, %847 ]
  %848 = load <16 x float> addrspace(3)* %ptrTypeCast.i, align 4
  store <16 x float> %848, <16 x float> addrspace(1)* %ptrTypeCast306.i, align 4
  %849 = add nsw i32 %j.11.i, 1
  %exitcond.i = icmp eq i32 %849, %841
  br i1 %exitcond.i, label %._crit_edge.i, label %847

._crit_edge.i:                                    ; preds = %847, %SyncBB.i
  %check.WI.iter328.i = icmp ult i64 %CurrWI..1.i, %16
  br i1 %check.WI.iter328.i, label %thenBB325.i, label %____Vectorized_.writeLocalMemory_separated_args.exit

thenBB325.i:                                      ; preds = %._crit_edge.i
  %"CurrWI++329.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride331.i" = add nuw i64 %CurrSBIndex..1.i, 192
  br label %SyncBB.i

____Vectorized_.writeLocalMemory_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}
!opencl_writeLocalMemory_locals_anchor = !{!2}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__writeLocalMemory_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int", metadata !"opencl_writeLocalMemory_locals_anchor", void (i8*)* @writeLocalMemory}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"opencl_writeLocalMemory_local_lbuf"}
