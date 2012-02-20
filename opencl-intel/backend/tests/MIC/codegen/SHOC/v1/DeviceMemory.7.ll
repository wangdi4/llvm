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

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__writeLocalMemory_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph4:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [4096 x float] addrspace(3)*
  br label %SyncBB22

SyncBB22:                                         ; preds = %bb.nph4, %thenBB25
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride31", %thenBB25 ], [ 0, %bb.nph4 ]
  %CurrWI..1 = phi i64 [ %"CurrWI++29", %thenBB25 ], [ 0, %bb.nph4 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %6, i32* %CastToValueType, align 4
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %8 = load i64* %7, align 8
  %9 = trunc i64 %8 to i32
  %"&(pSB[currWI].offset)121" = or i64 %CurrSBIndex..1, 4
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
  %check.WI.iter28 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter28, label %thenBB25, label %elseBB26

thenBB25:                                         ; preds = %._crit_edge5
  %"CurrWI++29" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride31" = add nuw i64 %CurrSBIndex..1, 8
  br label %SyncBB22

elseBB26:                                         ; preds = %._crit_edge5
  %80 = trunc i64 %11 to i32
  %81 = icmp eq i32 %80, 0
  %82 = select i1 %81, i32 1, i32 %80
  %83 = sdiv i32 4096, %82
  %84 = icmp sgt i32 %83, 0
  br label %SyncBB23

SyncBB23:                                         ; preds = %thenBB, %elseBB26
  %CurrSBIndex..0 = phi i64 [ 0, %elseBB26 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %CurrWI..0 = phi i64 [ 0, %elseBB26 ], [ %"CurrWI++", %thenBB ]
  br i1 %84, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB23
  %"&(pSB[currWI].offset)162" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset17" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)162"
  %CastToValueType18 = bitcast i8* %"&pSB[currWI].offset17" to i32*
  %loadedValue19 = load i32* %CastToValueType18, align 4
  %85 = sext i32 %loadedValue19 to i64
  %86 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %85
  %"&pSB[currWI].offset9" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
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

._crit_edge:                                      ; preds = %89, %SyncBB23
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 8
  br label %SyncBB23

SyncBB:                                           ; preds = %._crit_edge
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

SyncBB22.i:                                       ; preds = %thenBB25.i, %entry
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride31.i", %thenBB25.i ], [ 0, %entry ]
  %CurrWI..1.i = phi i64 [ %"CurrWI++29.i", %thenBB25.i ], [ 0, %entry ]
  %21 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..1.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = add i64 %22, %24
  %26 = trunc i64 %25 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..1.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %26, i32* %CastToValueType.i, align 4
  %27 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..1.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = trunc i64 %28 to i32
  %"&(pSB[currWI].offset)121.i" = or i64 %CurrSBIndex..1.i, 4
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
  %check.WI.iter28.i = icmp ult i64 %CurrWI..1.i, %16
  br i1 %check.WI.iter28.i, label %thenBB25.i, label %elseBB26.i

thenBB25.i:                                       ; preds = %._crit_edge5.i
  %"CurrWI++29.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride31.i" = add nuw i64 %CurrSBIndex..1.i, 8
  br label %SyncBB22.i

elseBB26.i:                                       ; preds = %._crit_edge5.i
  %100 = trunc i64 %31 to i32
  %101 = icmp eq i32 %100, 0
  %102 = select i1 %101, i32 1, i32 %100
  %103 = sdiv i32 4096, %102
  %104 = icmp sgt i32 %103, 0
  br label %SyncBB23.i

SyncBB23.i:                                       ; preds = %thenBB.i, %elseBB26.i
  %CurrSBIndex..0.i = phi i64 [ 0, %elseBB26.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %elseBB26.i ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %104, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %SyncBB23.i
  %"&(pSB[currWI].offset)162.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset17.i" = getelementptr inbounds i8* %19, i64 %"&(pSB[currWI].offset)162.i"
  %CastToValueType18.i = bitcast i8* %"&pSB[currWI].offset17.i" to i32*
  %loadedValue19.i = load i32* %CastToValueType18.i, align 4
  %105 = sext i32 %loadedValue19.i to i64
  %106 = getelementptr inbounds [4096 x float] addrspace(3)* %20, i64 0, i64 %105
  %"&pSB[currWI].offset9.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
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

._crit_edge.i:                                    ; preds = %109, %SyncBB23.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__writeLocalMemory_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 8
  br label %SyncBB23.i

__writeLocalMemory_separated_args.exit:           ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}
!opencl_writeLocalMemory_locals_anchor = !{!2}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__writeLocalMemory_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int", metadata !"opencl_writeLocalMemory_locals_anchor", void (i8*)* @writeLocalMemory}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"opencl_writeLocalMemory_local_lbuf"}
