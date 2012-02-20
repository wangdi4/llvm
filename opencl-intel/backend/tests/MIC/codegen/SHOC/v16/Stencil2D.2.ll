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

@opencl_StencilKernel_local_sh = internal addrspace(3) global [18 x [18 x double]] zeroinitializer, align 16

declare void @__CopyRect_original(double addrspace(1)* nocapture, i32, i32, double addrspace(1)* nocapture, i32, i32, i32, i32) nounwind

declare i64 @get_group_id(i32)

declare i64 @get_local_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_local_size(i32)

declare void @__StencilKernel_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double, double, double) nounwind

declare i64 @get_num_groups(i32)

declare void @barrier(i64)

declare void @____Vectorized_.CopyRect_original(double addrspace(1)* nocapture, i32, i32, double addrspace(1)* nocapture, i32, i32, i32, i32) nounwind

declare void @____Vectorized_.StencilKernel_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double, double, double) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

declare double @masked_load0(i1, double addrspace(1)*)

declare void @masked_store0(i1, double, double addrspace(1)*)

define i1 @allZero_v16(<16 x i1> %t) {
entry:
  %ipred = bitcast <16 x i1> %t to i16
  %val = call i32 @llvm.x86.mic.kortestz(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

declare double @masked_load2(i1, double addrspace(1)*)

declare void @masked_store2(i1, double, double addrspace(3)*)

declare double @masked_load3(i1, double addrspace(1)*)

declare void @masked_store3(i1, double, double addrspace(3)*)

declare double @masked_load4(i1, double addrspace(1)*)

declare void @masked_store4(i1, double, double addrspace(3)*)

declare double @masked_load5(i1, double addrspace(1)*)

declare void @masked_store5(i1, double, double addrspace(3)*)

declare double @masked_load6(i1, double addrspace(1)*)

declare void @masked_store6(i1, double, double addrspace(3)*)

declare double @masked_load7(i1, double addrspace(1)*)

declare void @masked_store7(i1, double, double addrspace(3)*)

declare double @masked_load8(i1, double addrspace(1)*)

declare void @masked_store8(i1, double, double addrspace(3)*)

declare double @masked_load9(i1, double addrspace(1)*)

declare void @masked_store9(i1, double, double addrspace(3)*)

declare void @masked_store10(i1, <16 x double>, <16 x double> addrspace(3)*)

declare void @masked_store11(i1, <16 x double>, <16 x double> addrspace(3)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

define void @__CopyRect_separated_args(double addrspace(1)* nocapture %dest, i32 %doffset, i32 %dpitch, double addrspace(1)* nocapture %src, i32 %soffset, i32 %spitch, i32 %width, i32 %height, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %width, 0
  %1 = sext i32 %soffset to i64
  %2 = sext i32 %doffset to i64
  br label %SyncBB3

SyncBB3:                                          ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %3 = load i64* %pWGId, align 8
  %4 = trunc i64 %3 to i32
  %5 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
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

bb.nph:                                           ; preds = %SyncBB3
  %14 = mul nsw i32 %12, %spitch
  %15 = mul nsw i32 %12, %dpitch
  br label %16

; <label>:16                                      ; preds = %16, %bb.nph
  %17 = phi i32 [ 0, %bb.nph ], [ %25, %16 ]
  %18 = add nsw i32 %14, %17
  %19 = sext i32 %18 to i64
  %.sum = add i64 %19, %1
  %20 = getelementptr inbounds double addrspace(1)* %src, i64 %.sum
  %21 = load double addrspace(1)* %20, align 8
  %22 = add nsw i32 %15, %17
  %23 = sext i32 %22 to i64
  %.sum1 = add i64 %23, %2
  %24 = getelementptr inbounds double addrspace(1)* %dest, i64 %.sum1
  store double %21, double addrspace(1)* %24, align 8
  %25 = add nsw i32 %17, 1
  %exitcond = icmp eq i32 %25, %width
  br i1 %exitcond, label %.loopexit, label %16

.loopexit:                                        ; preds = %16, %SyncBB3
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %.loopexit
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB3

SyncBB:                                           ; preds = %.loopexit
  ret void
}

define void @__StencilKernel_separated_args(double addrspace(1)* nocapture %data, double addrspace(1)* nocapture %newData, double %wCenter, double %wCardinal, double %wDiagonal, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to [18 x [18 x double]] addrspace(3)*
  %2 = bitcast i8 addrspace(3)* %pLocalMem to double addrspace(3)*
  %3 = getelementptr i8 addrspace(3)* %pLocalMem, i64 2448
  %4 = bitcast i8 addrspace(3)* %3 to double addrspace(3)*
  %5 = getelementptr i8 addrspace(3)* %pLocalMem, i64 136
  %6 = bitcast i8 addrspace(3)* %5 to double addrspace(3)*
  %7 = getelementptr i8 addrspace(3)* %pLocalMem, i64 2584
  %8 = bitcast i8 addrspace(3)* %7 to double addrspace(3)*
  br label %SyncBB95

SyncBB95:                                         ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %9 = getelementptr i64* %pWGId, i64 1
  %10 = load i64* %9, align 8
  %11 = trunc i64 %10 to i32
  %12 = load i64* %pWGId, align 8
  %13 = trunc i64 %12 to i32
  %14 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 0
  %15 = load i64* %14, align 8
  %16 = trunc i64 %15 to i32
  %17 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %18 = load i64* %17, align 8
  %19 = trunc i64 %18 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %19, i32* %CastToValueType, align 4
  %20 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = trunc i64 %21 to i32
  %"&(pSB[currWI].offset)111" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset12" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)111"
  %CastToValueType13 = bitcast i8* %"&pSB[currWI].offset12" to i32*
  store i32 %22, i32* %CastToValueType13, align 4
  %23 = shl i32 %11, 4
  %24 = add nsw i32 %23, %19
  %25 = shl i32 %13, 4
  %26 = add nsw i32 %25, %22
  %27 = shl i32 %16, 4
  %28 = add nsw i32 %24, 1
  %29 = or i32 %27, 2
  %30 = mul nsw i32 %29, %28
  %31 = add nsw i32 %26, 1
  %32 = add nsw i32 %31, %30
  %33 = sext i32 %32 to i64
  %"&(pSB[currWI].offset)302" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset31" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)302"
  %CastToValueType32 = bitcast i8* %"&pSB[currWI].offset31" to i64*
  store i64 %33, i64* %CastToValueType32, align 8
  %34 = getelementptr inbounds double addrspace(1)* %data, i64 %33
  %35 = load double addrspace(1)* %34, align 8
  %36 = add nsw i32 %22, 1
  %37 = sext i32 %36 to i64
  %"&(pSB[currWI].offset)393" = or i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset40" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)393"
  %CastToValueType41 = bitcast i8* %"&pSB[currWI].offset40" to i64*
  store i64 %37, i64* %CastToValueType41, align 8
  %38 = add nsw i32 %19, 1
  %39 = sext i32 %38 to i64
  %"&(pSB[currWI].offset)634" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset64" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)634"
  %CastToValueType65 = bitcast i8* %"&pSB[currWI].offset64" to i64*
  store i64 %39, i64* %CastToValueType65, align 8
  %40 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %39, i64 %37
  %"&(pSB[currWI].offset)875" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset88" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)875"
  %CastToValueType89 = bitcast i8* %"&pSB[currWI].offset88" to double addrspace(3)**
  store double addrspace(3)* %40, double addrspace(3)** %CastToValueType89, align 8
  store double %35, double addrspace(3)* %40, align 8
  %41 = icmp eq i32 %19, 0
  br i1 %41, label %42, label %60

; <label>:42                                      ; preds = %SyncBB95
  %43 = or i32 %27, 2
  %44 = mul nsw i32 %43, %24
  %45 = add nsw i32 %26, 1
  %46 = add nsw i32 %45, %44
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds double addrspace(1)* %data, i64 %47
  %49 = load double addrspace(1)* %48, align 8
  %"&(pSB[currWI].offset)5817" = or i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset59" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5817"
  %CastToValueType60 = bitcast i8* %"&pSB[currWI].offset59" to i64*
  %loadedValue61 = load i64* %CastToValueType60, align 8
  %50 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 0, i64 %loadedValue61
  store double %49, double addrspace(3)* %50, align 8
  %51 = add nsw i32 %24, 17
  %52 = or i32 %27, 2
  %53 = mul nsw i32 %52, %51
  %54 = add nsw i32 %26, 1
  %55 = add nsw i32 %54, %53
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds double addrspace(1)* %data, i64 %56
  %58 = load double addrspace(1)* %57, align 8
  %"&(pSB[currWI].offset)5318" = or i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset54" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5318"
  %CastToValueType55 = bitcast i8* %"&pSB[currWI].offset54" to i64*
  %loadedValue56 = load i64* %CastToValueType55, align 8
  %59 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 17, i64 %loadedValue56
  store double %58, double addrspace(3)* %59, align 8
  br label %60

; <label>:60                                      ; preds = %42, %SyncBB95
  %"&(pSB[currWI].offset)206" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset21" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)206"
  %CastToValueType22 = bitcast i8* %"&pSB[currWI].offset21" to i32*
  %loadedValue23 = load i32* %CastToValueType22, align 4
  %61 = icmp eq i32 %loadedValue23, 0
  br i1 %61, label %62, label %80

; <label>:62                                      ; preds = %60
  %63 = add nsw i32 %24, 1
  %64 = or i32 %27, 2
  %65 = mul nsw i32 %64, %63
  %66 = add nsw i32 %26, %65
  %67 = sext i32 %66 to i64
  %68 = getelementptr inbounds double addrspace(1)* %data, i64 %67
  %69 = load double addrspace(1)* %68, align 8
  %"&(pSB[currWI].offset)8215" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset83" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)8215"
  %CastToValueType84 = bitcast i8* %"&pSB[currWI].offset83" to i64*
  %loadedValue85 = load i64* %CastToValueType84, align 8
  %70 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue85, i64 0
  store double %69, double addrspace(3)* %70, align 16
  %71 = add nsw i32 %24, 1
  %72 = or i32 %27, 2
  %73 = mul nsw i32 %72, %71
  %74 = add nsw i32 %26, 17
  %75 = add nsw i32 %74, %73
  %76 = sext i32 %75 to i64
  %77 = getelementptr inbounds double addrspace(1)* %data, i64 %76
  %78 = load double addrspace(1)* %77, align 8
  %"&(pSB[currWI].offset)7716" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset78" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)7716"
  %CastToValueType79 = bitcast i8* %"&pSB[currWI].offset78" to i64*
  %loadedValue80 = load i64* %CastToValueType79, align 8
  %79 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue80, i64 17
  store double %78, double addrspace(3)* %79, align 8
  br label %80

; <label>:80                                      ; preds = %62, %60
  %81 = or i64 %21, %18
  %82 = trunc i64 %81 to i32
  %83 = icmp eq i32 %82, 0
  br i1 %83, label %84, label %113

; <label>:84                                      ; preds = %80
  %85 = or i32 %27, 2
  %86 = mul nsw i32 %85, %24
  %87 = add nsw i32 %26, %86
  %88 = sext i32 %87 to i64
  %89 = getelementptr inbounds double addrspace(1)* %data, i64 %88
  %90 = load double addrspace(1)* %89, align 8
  store double %90, double addrspace(3)* %2, align 16
  %91 = add nsw i32 %24, 17
  %92 = or i32 %27, 2
  %93 = mul nsw i32 %92, %91
  %94 = add nsw i32 %26, %93
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds double addrspace(1)* %data, i64 %95
  %97 = load double addrspace(1)* %96, align 8
  store double %97, double addrspace(3)* %4, align 16
  %98 = or i32 %27, 2
  %99 = mul nsw i32 %98, %24
  %100 = add nsw i32 %26, 17
  %101 = add nsw i32 %100, %99
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds double addrspace(1)* %data, i64 %102
  %104 = load double addrspace(1)* %103, align 8
  store double %104, double addrspace(3)* %6, align 8
  %105 = add nsw i32 %24, 17
  %106 = or i32 %27, 2
  %107 = mul nsw i32 %106, %105
  %108 = add nsw i32 %26, 17
  %109 = add nsw i32 %108, %107
  %110 = sext i32 %109 to i64
  %111 = getelementptr inbounds double addrspace(1)* %data, i64 %110
  %112 = load double addrspace(1)* %111, align 8
  store double %112, double addrspace(3)* %8, align 8
  br label %113

; <label>:113                                     ; preds = %84, %80
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %113
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 320
  br label %SyncBB95

SyncBB:                                           ; preds = %113, %thenBB98
  %CurrWI..1 = phi i64 [ %"CurrWI++102", %thenBB98 ], [ 0, %113 ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride104", %thenBB98 ], [ 0, %113 ]
  %"&(pSB[currWI].offset)917" = or i64 %CurrSBIndex..1, 32
  %"&pSB[currWI].offset92" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)917"
  %CastToValueType93 = bitcast i8* %"&pSB[currWI].offset92" to double addrspace(3)**
  %loadedValue94 = load double addrspace(3)** %CastToValueType93, align 8
  %114 = load double addrspace(3)* %loadedValue94, align 8
  %"&pSB[currWI].offset7" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType8 = bitcast i8* %"&pSB[currWI].offset7" to i32*
  %loadedValue9 = load i32* %CastToValueType8, align 4
  %115 = sext i32 %loadedValue9 to i64
  %"&(pSB[currWI].offset)488" = or i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset49" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)488"
  %CastToValueType50 = bitcast i8* %"&pSB[currWI].offset49" to i64*
  %loadedValue51 = load i64* %CastToValueType50, align 8
  %116 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %115, i64 %loadedValue51
  %117 = load double addrspace(3)* %116, align 8
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i32*
  %loadedValue = load i32* %CastToValueType4, align 4
  %118 = add nsw i32 %loadedValue, 2
  %119 = sext i32 %118 to i64
  %"&(pSB[currWI].offset)439" = or i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset44" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)439"
  %CastToValueType45 = bitcast i8* %"&pSB[currWI].offset44" to i64*
  %loadedValue46 = load i64* %CastToValueType45, align 8
  %120 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %119, i64 %loadedValue46
  %121 = load double addrspace(3)* %120, align 8
  %122 = fadd double %117, %121
  %"&(pSB[currWI].offset)2510" = or i64 %CurrSBIndex..1, 4
  %"&pSB[currWI].offset26" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2510"
  %CastToValueType27 = bitcast i8* %"&pSB[currWI].offset26" to i32*
  %loadedValue28 = load i32* %CastToValueType27, align 4
  %123 = sext i32 %loadedValue28 to i64
  %"&(pSB[currWI].offset)7211" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset73" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)7211"
  %CastToValueType74 = bitcast i8* %"&pSB[currWI].offset73" to i64*
  %loadedValue75 = load i64* %CastToValueType74, align 8
  %124 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue75, i64 %123
  %125 = load double addrspace(3)* %124, align 8
  %126 = fadd double %122, %125
  %"&(pSB[currWI].offset)1512" = or i64 %CurrSBIndex..1, 4
  %"&pSB[currWI].offset16" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1512"
  %CastToValueType17 = bitcast i8* %"&pSB[currWI].offset16" to i32*
  %loadedValue18 = load i32* %CastToValueType17, align 4
  %127 = add nsw i32 %loadedValue18, 2
  %128 = sext i32 %127 to i64
  %"&(pSB[currWI].offset)6713" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset68" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)6713"
  %CastToValueType69 = bitcast i8* %"&pSB[currWI].offset68" to i64*
  %loadedValue70 = load i64* %CastToValueType69, align 8
  %129 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue70, i64 %128
  %130 = load double addrspace(3)* %129, align 8
  %131 = fadd double %126, %130
  %132 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %115, i64 %123
  %133 = load double addrspace(3)* %132, align 8
  %134 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %115, i64 %128
  %135 = load double addrspace(3)* %134, align 8
  %136 = fadd double %133, %135
  %137 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %119, i64 %123
  %138 = load double addrspace(3)* %137, align 8
  %139 = fadd double %136, %138
  %140 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %119, i64 %128
  %141 = load double addrspace(3)* %140, align 8
  %142 = fadd double %139, %141
  %143 = fmul double %114, %wCenter
  %144 = fmul double %131, %wCardinal
  %145 = fadd double %143, %144
  %146 = fmul double %142, %wDiagonal
  %147 = fadd double %145, %146
  %"&(pSB[currWI].offset)3414" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset35" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3414"
  %CastToValueType36 = bitcast i8* %"&pSB[currWI].offset35" to i64*
  %loadedValue37 = load i64* %CastToValueType36, align 8
  %148 = getelementptr inbounds double addrspace(1)* %newData, i64 %loadedValue37
  store double %147, double addrspace(1)* %148, align 8
  %check.WI.iter101 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter101, label %thenBB98, label %SyncBB96

thenBB98:                                         ; preds = %SyncBB
  %"CurrWI++102" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride104" = add nuw i64 %CurrSBIndex..1, 320
  br label %SyncBB

SyncBB96:                                         ; preds = %SyncBB
  ret void
}

define void @____Vectorized_.CopyRect_separated_args(double addrspace(1)* nocapture %dest, i32 %doffset, i32 %dpitch, double addrspace(1)* nocapture %src, i32 %soffset, i32 %spitch, i32 %width, i32 %height, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph:
  %temp54 = insertelement <16 x i32> undef, i32 %dpitch, i32 0
  %vector55 = shufflevector <16 x i32> %temp54, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp16 = insertelement <16 x i32> undef, i32 %spitch, i32 0
  %vector17 = shufflevector <16 x i32> %temp16, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp7 = insertelement <16 x i32> undef, i32 %height, i32 0
  %vector8 = shufflevector <16 x i32> %temp7, <16 x i32> undef, <16 x i32> zeroinitializer
  %0 = icmp sgt i32 %width, 0
  %temp9 = insertelement <16 x i1> undef, i1 %0, i32 0
  %vector10 = shufflevector <16 x i1> %temp9, <16 x i1> undef, <16 x i32> zeroinitializer
  %1 = sext i32 %soffset to i64
  %temp20 = insertelement <16 x i64> undef, i64 %1, i32 0
  %vector21 = shufflevector <16 x i64> %temp20, <16 x i64> undef, <16 x i32> zeroinitializer
  %2 = sext i32 %doffset to i64
  %temp56 = insertelement <16 x i64> undef, i64 %2, i32 0
  %vector57 = shufflevector <16 x i64> %temp56, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB176

SyncBB176:                                        ; preds = %thenBB, %bb.nph
  %CurrWI..0 = phi i64 [ 0, %bb.nph ], [ %"CurrWI++", %thenBB ]
  %3 = load i64* %pWGId, align 8
  %4 = trunc i64 %3 to i32
  %5 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %broadcast1 = insertelement <16 x i64> undef, i64 %6, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %7 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %8 = trunc <16 x i64> %7 to <16 x i32>
  %9 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %10 = load i64* %9, align 8
  %11 = trunc i64 %10 to i32
  %12 = mul nsw i32 %11, %4
  %temp = insertelement <16 x i32> undef, i32 %12, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %13 = add nsw <16 x i32> %vector, %8
  %14 = icmp slt <16 x i32> %13, %vector8
  %or.cond11 = and <16 x i1> %14, %vector10
  %ipred.i = bitcast <16 x i1> %or.cond11 to i16
  %val.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i, i16 %ipred.i) nounwind
  %tmp.i = and i32 %val.i, 1
  %res.i = icmp eq i32 %tmp.i, 0
  br i1 %res.i, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %SyncBB176
  %negIncomingLoopMask13 = xor <16 x i1> %or.cond11, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %15 = mul nsw <16 x i32> %13, %vector17
  %16 = mul nsw <16 x i32> %13, %vector55
  br label %17

; <label>:17                                      ; preds = %postload175, %.preheader
  %vectorPHI = phi <16 x i1> [ %loop_mask379, %postload175 ], [ %negIncomingLoopMask13, %.preheader ]
  %vectorPHI15 = phi <16 x i1> [ %local_edge83, %postload175 ], [ %or.cond11, %.preheader ]
  %18 = phi i32 [ %55, %postload175 ], [ 0, %.preheader ]
  %extract38 = extractelement <16 x i1> %vectorPHI15, i32 0
  %extract39 = extractelement <16 x i1> %vectorPHI15, i32 1
  %extract40 = extractelement <16 x i1> %vectorPHI15, i32 2
  %extract41 = extractelement <16 x i1> %vectorPHI15, i32 3
  %extract42 = extractelement <16 x i1> %vectorPHI15, i32 4
  %extract43 = extractelement <16 x i1> %vectorPHI15, i32 5
  %extract44 = extractelement <16 x i1> %vectorPHI15, i32 6
  %extract45 = extractelement <16 x i1> %vectorPHI15, i32 7
  %extract46 = extractelement <16 x i1> %vectorPHI15, i32 8
  %extract47 = extractelement <16 x i1> %vectorPHI15, i32 9
  %extract48 = extractelement <16 x i1> %vectorPHI15, i32 10
  %extract49 = extractelement <16 x i1> %vectorPHI15, i32 11
  %extract50 = extractelement <16 x i1> %vectorPHI15, i32 12
  %extract51 = extractelement <16 x i1> %vectorPHI15, i32 13
  %extract52 = extractelement <16 x i1> %vectorPHI15, i32 14
  %extract53 = extractelement <16 x i1> %vectorPHI15, i32 15
  %temp18 = insertelement <16 x i32> undef, i32 %18, i32 0
  %vector19 = shufflevector <16 x i32> %temp18, <16 x i32> undef, <16 x i32> zeroinitializer
  %19 = add nsw <16 x i32> %15, %vector19
  %20 = sext <16 x i32> %19 to <16 x i64>
  %.sum22 = add <16 x i64> %20, %vector21
  %extract23 = extractelement <16 x i64> %.sum22, i32 1
  %extract24 = extractelement <16 x i64> %.sum22, i32 2
  %extract25 = extractelement <16 x i64> %.sum22, i32 3
  %extract26 = extractelement <16 x i64> %.sum22, i32 4
  %extract27 = extractelement <16 x i64> %.sum22, i32 5
  %extract28 = extractelement <16 x i64> %.sum22, i32 6
  %extract29 = extractelement <16 x i64> %.sum22, i32 7
  %extract30 = extractelement <16 x i64> %.sum22, i32 8
  %extract31 = extractelement <16 x i64> %.sum22, i32 9
  %extract32 = extractelement <16 x i64> %.sum22, i32 10
  %extract33 = extractelement <16 x i64> %.sum22, i32 11
  %extract34 = extractelement <16 x i64> %.sum22, i32 12
  %extract35 = extractelement <16 x i64> %.sum22, i32 13
  %extract36 = extractelement <16 x i64> %.sum22, i32 14
  %extract37 = extractelement <16 x i64> %.sum22, i32 15
  %21 = getelementptr inbounds double addrspace(1)* %src, i64 %extract23
  %22 = getelementptr inbounds double addrspace(1)* %src, i64 %extract24
  %23 = getelementptr inbounds double addrspace(1)* %src, i64 %extract25
  %24 = getelementptr inbounds double addrspace(1)* %src, i64 %extract26
  %25 = getelementptr inbounds double addrspace(1)* %src, i64 %extract27
  %26 = getelementptr inbounds double addrspace(1)* %src, i64 %extract28
  %27 = getelementptr inbounds double addrspace(1)* %src, i64 %extract29
  %28 = getelementptr inbounds double addrspace(1)* %src, i64 %extract30
  %29 = getelementptr inbounds double addrspace(1)* %src, i64 %extract31
  %30 = getelementptr inbounds double addrspace(1)* %src, i64 %extract32
  %31 = getelementptr inbounds double addrspace(1)* %src, i64 %extract33
  %32 = getelementptr inbounds double addrspace(1)* %src, i64 %extract34
  %33 = getelementptr inbounds double addrspace(1)* %src, i64 %extract35
  %34 = getelementptr inbounds double addrspace(1)* %src, i64 %extract36
  %35 = getelementptr inbounds double addrspace(1)* %src, i64 %extract37
  br i1 %extract38, label %preload, label %postload

preload:                                          ; preds = %17
  %extract = extractelement <16 x i64> %.sum22, i32 0
  %36 = getelementptr inbounds double addrspace(1)* %src, i64 %extract
  %masked_load = load double addrspace(1)* %36, align 8
  br label %postload

postload:                                         ; preds = %preload, %17
  %phi = phi double [ undef, %17 ], [ %masked_load, %preload ]
  br i1 %extract39, label %preload101, label %postload102

preload101:                                       ; preds = %postload
  %masked_load84 = load double addrspace(1)* %21, align 8
  br label %postload102

postload102:                                      ; preds = %preload101, %postload
  %phi103 = phi double [ undef, %postload ], [ %masked_load84, %preload101 ]
  br i1 %extract40, label %preload106, label %postload107

preload106:                                       ; preds = %postload102
  %masked_load85 = load double addrspace(1)* %22, align 8
  br label %postload107

postload107:                                      ; preds = %preload106, %postload102
  %phi108 = phi double [ undef, %postload102 ], [ %masked_load85, %preload106 ]
  br i1 %extract41, label %preload111, label %postload112

preload111:                                       ; preds = %postload107
  %masked_load86 = load double addrspace(1)* %23, align 8
  br label %postload112

postload112:                                      ; preds = %preload111, %postload107
  %phi113 = phi double [ undef, %postload107 ], [ %masked_load86, %preload111 ]
  br i1 %extract42, label %preload116, label %postload117

preload116:                                       ; preds = %postload112
  %masked_load87 = load double addrspace(1)* %24, align 8
  br label %postload117

postload117:                                      ; preds = %preload116, %postload112
  %phi118 = phi double [ undef, %postload112 ], [ %masked_load87, %preload116 ]
  br i1 %extract43, label %preload121, label %postload122

preload121:                                       ; preds = %postload117
  %masked_load88 = load double addrspace(1)* %25, align 8
  br label %postload122

postload122:                                      ; preds = %preload121, %postload117
  %phi123 = phi double [ undef, %postload117 ], [ %masked_load88, %preload121 ]
  br i1 %extract44, label %preload126, label %postload127

preload126:                                       ; preds = %postload122
  %masked_load89 = load double addrspace(1)* %26, align 8
  br label %postload127

postload127:                                      ; preds = %preload126, %postload122
  %phi128 = phi double [ undef, %postload122 ], [ %masked_load89, %preload126 ]
  br i1 %extract45, label %preload131, label %postload132

preload131:                                       ; preds = %postload127
  %masked_load90 = load double addrspace(1)* %27, align 8
  br label %postload132

postload132:                                      ; preds = %preload131, %postload127
  %phi133 = phi double [ undef, %postload127 ], [ %masked_load90, %preload131 ]
  br i1 %extract46, label %preload136, label %postload137

preload136:                                       ; preds = %postload132
  %masked_load91 = load double addrspace(1)* %28, align 8
  br label %postload137

postload137:                                      ; preds = %preload136, %postload132
  %phi138 = phi double [ undef, %postload132 ], [ %masked_load91, %preload136 ]
  br i1 %extract47, label %preload141, label %postload142

preload141:                                       ; preds = %postload137
  %masked_load92 = load double addrspace(1)* %29, align 8
  br label %postload142

postload142:                                      ; preds = %preload141, %postload137
  %phi143 = phi double [ undef, %postload137 ], [ %masked_load92, %preload141 ]
  br i1 %extract48, label %preload146, label %postload147

preload146:                                       ; preds = %postload142
  %masked_load93 = load double addrspace(1)* %30, align 8
  br label %postload147

postload147:                                      ; preds = %preload146, %postload142
  %phi148 = phi double [ undef, %postload142 ], [ %masked_load93, %preload146 ]
  br i1 %extract49, label %preload151, label %postload152

preload151:                                       ; preds = %postload147
  %masked_load94 = load double addrspace(1)* %31, align 8
  br label %postload152

postload152:                                      ; preds = %preload151, %postload147
  %phi153 = phi double [ undef, %postload147 ], [ %masked_load94, %preload151 ]
  br i1 %extract50, label %preload156, label %postload157

preload156:                                       ; preds = %postload152
  %masked_load95 = load double addrspace(1)* %32, align 8
  br label %postload157

postload157:                                      ; preds = %preload156, %postload152
  %phi158 = phi double [ undef, %postload152 ], [ %masked_load95, %preload156 ]
  br i1 %extract51, label %preload161, label %postload162

preload161:                                       ; preds = %postload157
  %masked_load96 = load double addrspace(1)* %33, align 8
  br label %postload162

postload162:                                      ; preds = %preload161, %postload157
  %phi163 = phi double [ undef, %postload157 ], [ %masked_load96, %preload161 ]
  br i1 %extract52, label %preload166, label %postload167

preload166:                                       ; preds = %postload162
  %masked_load97 = load double addrspace(1)* %34, align 8
  br label %postload167

postload167:                                      ; preds = %preload166, %postload162
  %phi168 = phi double [ undef, %postload162 ], [ %masked_load97, %preload166 ]
  br i1 %extract53, label %preload171, label %postload172

preload171:                                       ; preds = %postload167
  %masked_load98 = load double addrspace(1)* %35, align 8
  br label %postload172

postload172:                                      ; preds = %preload171, %postload167
  %phi173 = phi double [ undef, %postload167 ], [ %masked_load98, %preload171 ]
  %37 = add nsw <16 x i32> %16, %vector19
  %38 = sext <16 x i32> %37 to <16 x i64>
  %.sum158 = add <16 x i64> %38, %vector57
  %extract60 = extractelement <16 x i64> %.sum158, i32 1
  %extract61 = extractelement <16 x i64> %.sum158, i32 2
  %extract62 = extractelement <16 x i64> %.sum158, i32 3
  %extract63 = extractelement <16 x i64> %.sum158, i32 4
  %extract64 = extractelement <16 x i64> %.sum158, i32 5
  %extract65 = extractelement <16 x i64> %.sum158, i32 6
  %extract66 = extractelement <16 x i64> %.sum158, i32 7
  %extract67 = extractelement <16 x i64> %.sum158, i32 8
  %extract68 = extractelement <16 x i64> %.sum158, i32 9
  %extract69 = extractelement <16 x i64> %.sum158, i32 10
  %extract70 = extractelement <16 x i64> %.sum158, i32 11
  %extract71 = extractelement <16 x i64> %.sum158, i32 12
  %extract72 = extractelement <16 x i64> %.sum158, i32 13
  %extract73 = extractelement <16 x i64> %.sum158, i32 14
  %extract74 = extractelement <16 x i64> %.sum158, i32 15
  %39 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract60
  %40 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract61
  %41 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract62
  %42 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract63
  %43 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract64
  %44 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract65
  %45 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract66
  %46 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract67
  %47 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract68
  %48 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract69
  %49 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract70
  %50 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract71
  %51 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract72
  %52 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract73
  %53 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract74
  br i1 %extract38, label %preload99, label %postload100

preload99:                                        ; preds = %postload172
  %extract59 = extractelement <16 x i64> %.sum158, i32 0
  %54 = getelementptr inbounds double addrspace(1)* %dest, i64 %extract59
  store double %phi, double addrspace(1)* %54, align 8
  br label %postload100

postload100:                                      ; preds = %preload99, %postload172
  br i1 %extract39, label %preload104, label %postload105

preload104:                                       ; preds = %postload100
  store double %phi103, double addrspace(1)* %39, align 8
  br label %postload105

postload105:                                      ; preds = %preload104, %postload100
  br i1 %extract40, label %preload109, label %postload110

preload109:                                       ; preds = %postload105
  store double %phi108, double addrspace(1)* %40, align 8
  br label %postload110

postload110:                                      ; preds = %preload109, %postload105
  br i1 %extract41, label %preload114, label %postload115

preload114:                                       ; preds = %postload110
  store double %phi113, double addrspace(1)* %41, align 8
  br label %postload115

postload115:                                      ; preds = %preload114, %postload110
  br i1 %extract42, label %preload119, label %postload120

preload119:                                       ; preds = %postload115
  store double %phi118, double addrspace(1)* %42, align 8
  br label %postload120

postload120:                                      ; preds = %preload119, %postload115
  br i1 %extract43, label %preload124, label %postload125

preload124:                                       ; preds = %postload120
  store double %phi123, double addrspace(1)* %43, align 8
  br label %postload125

postload125:                                      ; preds = %preload124, %postload120
  br i1 %extract44, label %preload129, label %postload130

preload129:                                       ; preds = %postload125
  store double %phi128, double addrspace(1)* %44, align 8
  br label %postload130

postload130:                                      ; preds = %preload129, %postload125
  br i1 %extract45, label %preload134, label %postload135

preload134:                                       ; preds = %postload130
  store double %phi133, double addrspace(1)* %45, align 8
  br label %postload135

postload135:                                      ; preds = %preload134, %postload130
  br i1 %extract46, label %preload139, label %postload140

preload139:                                       ; preds = %postload135
  store double %phi138, double addrspace(1)* %46, align 8
  br label %postload140

postload140:                                      ; preds = %preload139, %postload135
  br i1 %extract47, label %preload144, label %postload145

preload144:                                       ; preds = %postload140
  store double %phi143, double addrspace(1)* %47, align 8
  br label %postload145

postload145:                                      ; preds = %preload144, %postload140
  br i1 %extract48, label %preload149, label %postload150

preload149:                                       ; preds = %postload145
  store double %phi148, double addrspace(1)* %48, align 8
  br label %postload150

postload150:                                      ; preds = %preload149, %postload145
  br i1 %extract49, label %preload154, label %postload155

preload154:                                       ; preds = %postload150
  store double %phi153, double addrspace(1)* %49, align 8
  br label %postload155

postload155:                                      ; preds = %preload154, %postload150
  br i1 %extract50, label %preload159, label %postload160

preload159:                                       ; preds = %postload155
  store double %phi158, double addrspace(1)* %50, align 8
  br label %postload160

postload160:                                      ; preds = %preload159, %postload155
  br i1 %extract51, label %preload164, label %postload165

preload164:                                       ; preds = %postload160
  store double %phi163, double addrspace(1)* %51, align 8
  br label %postload165

postload165:                                      ; preds = %preload164, %postload160
  br i1 %extract52, label %preload169, label %postload170

preload169:                                       ; preds = %postload165
  store double %phi168, double addrspace(1)* %52, align 8
  br label %postload170

postload170:                                      ; preds = %preload169, %postload165
  br i1 %extract53, label %preload174, label %postload175

preload174:                                       ; preds = %postload170
  store double %phi173, double addrspace(1)* %53, align 8
  br label %postload175

postload175:                                      ; preds = %preload174, %postload170
  %55 = add nsw i32 %18, 1
  %exitcond = icmp eq i32 %55, %width
  %temp75 = insertelement <16 x i1> undef, i1 %exitcond, i32 0
  %vector76 = shufflevector <16 x i1> %temp75, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond = xor i1 %exitcond, true
  %temp81 = insertelement <16 x i1> undef, i1 %notCond, i32 0
  %vector82 = shufflevector <16 x i1> %temp81, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr77 = and <16 x i1> %vectorPHI15, %vector76
  %loop_mask379 = or <16 x i1> %vectorPHI, %who_left_tr77
  %curr_loop_mask80 = or <16 x i1> %loop_mask379, %who_left_tr77
  %ipred.i1 = bitcast <16 x i1> %curr_loop_mask80 to i16
  %val.i2 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1, i16 %ipred.i1) nounwind
  %tmp.i3 = and i32 %val.i2, 1
  %res.i4 = icmp eq i32 %tmp.i3, 0
  %local_edge83 = and <16 x i1> %vectorPHI15, %vector82
  br i1 %res.i4, label %17, label %.loopexit

.loopexit:                                        ; preds = %postload175, %SyncBB176
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %.loopexit
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB176

SyncBB:                                           ; preds = %.loopexit
  ret void
}

define void @____Vectorized_.StencilKernel_separated_args(double addrspace(1)* nocapture %data, double addrspace(1)* nocapture %newData, double %wCenter, double %wCardinal, double %wDiagonal, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to [18 x [18 x double]] addrspace(3)*
  %2 = bitcast i8 addrspace(3)* %pLocalMem to double addrspace(3)*
  %3 = getelementptr i8 addrspace(3)* %pLocalMem, i64 2448
  %4 = bitcast i8 addrspace(3)* %3 to double addrspace(3)*
  %5 = getelementptr i8 addrspace(3)* %pLocalMem, i64 136
  %6 = bitcast i8 addrspace(3)* %5 to double addrspace(3)*
  %7 = getelementptr i8 addrspace(3)* %pLocalMem, i64 2584
  %8 = bitcast i8 addrspace(3)* %7 to double addrspace(3)*
  br label %SyncBB1220

SyncBB1220:                                       ; preds = %0, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %9 = getelementptr i64* %pWGId, i64 1
  %10 = load i64* %9, align 8
  %11 = trunc i64 %10 to i32
  %12 = load i64* %pWGId, align 8
  %13 = trunc i64 %12 to i32
  %14 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 0
  %15 = load i64* %14, align 8
  %temp.vect = insertelement <16 x i64> undef, i64 %15, i32 0
  %temp.vect53 = insertelement <16 x i64> %temp.vect, i64 %15, i32 1
  %temp.vect54 = insertelement <16 x i64> %temp.vect53, i64 %15, i32 2
  %temp.vect55 = insertelement <16 x i64> %temp.vect54, i64 %15, i32 3
  %temp.vect56 = insertelement <16 x i64> %temp.vect55, i64 %15, i32 4
  %temp.vect57 = insertelement <16 x i64> %temp.vect56, i64 %15, i32 5
  %temp.vect58 = insertelement <16 x i64> %temp.vect57, i64 %15, i32 6
  %temp.vect59 = insertelement <16 x i64> %temp.vect58, i64 %15, i32 7
  %temp.vect60 = insertelement <16 x i64> %temp.vect59, i64 %15, i32 8
  %temp.vect61 = insertelement <16 x i64> %temp.vect60, i64 %15, i32 9
  %temp.vect62 = insertelement <16 x i64> %temp.vect61, i64 %15, i32 10
  %temp.vect63 = insertelement <16 x i64> %temp.vect62, i64 %15, i32 11
  %temp.vect64 = insertelement <16 x i64> %temp.vect63, i64 %15, i32 12
  %temp.vect65 = insertelement <16 x i64> %temp.vect64, i64 %15, i32 13
  %temp.vect66 = insertelement <16 x i64> %temp.vect65, i64 %15, i32 14
  %temp.vect67 = insertelement <16 x i64> %temp.vect66, i64 %15, i32 15
  %16 = trunc <16 x i64> %temp.vect67 to <16 x i32>
  %17 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %18 = load i64* %17, align 8
  %temp240 = insertelement <16 x i64> undef, i64 %18, i32 0
  %vector241 = shufflevector <16 x i64> %temp240, <16 x i64> undef, <16 x i32> zeroinitializer
  %19 = trunc i64 %18 to i32
  %"&(pSB[currWI].offset)1" = or i64 %CurrSBIndex..0, 40
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %19, i32* %CastToValueType, align 4
  %20 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %21 = load i64* %20, align 8
  %broadcast1 = insertelement <16 x i64> undef, i64 %21, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %22 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %23 = trunc <16 x i64> %22 to <16 x i32>
  %"&(pSB[currWI].offset)1044" = add nuw i64 %CurrSBIndex..0, 64
  %"&pSB[currWI].offset1045" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1044"
  %CastToValueType1046 = bitcast i8* %"&pSB[currWI].offset1045" to <16 x i32>*
  store <16 x i32> %23, <16 x i32>* %CastToValueType1046, align 64
  %24 = shl i32 %11, 4
  %25 = add nsw i32 %24, %19
  %temp117 = insertelement <16 x i32> undef, i32 %25, i32 0
  %vector118 = shufflevector <16 x i32> %temp117, <16 x i32> undef, <16 x i32> zeroinitializer
  %26 = shl i32 %13, 4
  %temp = insertelement <16 x i32> undef, i32 %26, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %27 = add nsw <16 x i32> %vector, %23
  %28 = shl <16 x i32> %16, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %29 = add nsw i32 %25, 1
  %temp68 = insertelement <16 x i32> undef, i32 %29, i32 0
  %vector69 = shufflevector <16 x i32> %temp68, <16 x i32> undef, <16 x i32> zeroinitializer
  %30 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %31 = mul nsw <16 x i32> %30, %vector69
  %32 = add nsw <16 x i32> %27, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %33 = add nsw <16 x i32> %32, %31
  %"&(pSB[currWI].offset)1073" = add nuw i64 %CurrSBIndex..0, 128
  %"&pSB[currWI].offset1074" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1073"
  %CastToValueType1075 = bitcast i8* %"&pSB[currWI].offset1074" to <16 x i32>*
  store <16 x i32> %33, <16 x i32>* %CastToValueType1075, align 64
  %34 = extractelement <16 x i32> %33, i32 0
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds double addrspace(1)* %data, i64 %35
  %37 = extractelement <16 x i32> %33, i32 1
  %38 = sext i32 %37 to i64
  %39 = getelementptr inbounds double addrspace(1)* %data, i64 %38
  %40 = extractelement <16 x i32> %33, i32 2
  %41 = sext i32 %40 to i64
  %42 = getelementptr inbounds double addrspace(1)* %data, i64 %41
  %43 = extractelement <16 x i32> %33, i32 3
  %44 = sext i32 %43 to i64
  %45 = getelementptr inbounds double addrspace(1)* %data, i64 %44
  %46 = extractelement <16 x i32> %33, i32 4
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds double addrspace(1)* %data, i64 %47
  %49 = extractelement <16 x i32> %33, i32 5
  %50 = sext i32 %49 to i64
  %51 = getelementptr inbounds double addrspace(1)* %data, i64 %50
  %52 = extractelement <16 x i32> %33, i32 6
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds double addrspace(1)* %data, i64 %53
  %55 = extractelement <16 x i32> %33, i32 7
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds double addrspace(1)* %data, i64 %56
  %58 = extractelement <16 x i32> %33, i32 8
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds double addrspace(1)* %data, i64 %59
  %61 = extractelement <16 x i32> %33, i32 9
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds double addrspace(1)* %data, i64 %62
  %64 = extractelement <16 x i32> %33, i32 10
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds double addrspace(1)* %data, i64 %65
  %67 = extractelement <16 x i32> %33, i32 11
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds double addrspace(1)* %data, i64 %68
  %70 = extractelement <16 x i32> %33, i32 12
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds double addrspace(1)* %data, i64 %71
  %73 = extractelement <16 x i32> %33, i32 13
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds double addrspace(1)* %data, i64 %74
  %76 = extractelement <16 x i32> %33, i32 14
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds double addrspace(1)* %data, i64 %77
  %79 = extractelement <16 x i32> %33, i32 15
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds double addrspace(1)* %data, i64 %80
  %82 = load double addrspace(1)* %36, align 8
  %83 = load double addrspace(1)* %39, align 8
  %84 = load double addrspace(1)* %42, align 8
  %85 = load double addrspace(1)* %45, align 8
  %86 = load double addrspace(1)* %48, align 8
  %87 = load double addrspace(1)* %51, align 8
  %88 = load double addrspace(1)* %54, align 8
  %89 = load double addrspace(1)* %57, align 8
  %90 = load double addrspace(1)* %60, align 8
  %91 = load double addrspace(1)* %63, align 8
  %92 = load double addrspace(1)* %66, align 8
  %93 = load double addrspace(1)* %69, align 8
  %94 = load double addrspace(1)* %72, align 8
  %95 = load double addrspace(1)* %75, align 8
  %96 = load double addrspace(1)* %78, align 8
  %97 = load double addrspace(1)* %81, align 8
  %temp.vect101 = insertelement <16 x double> undef, double %82, i32 0
  %temp.vect102 = insertelement <16 x double> %temp.vect101, double %83, i32 1
  %temp.vect103 = insertelement <16 x double> %temp.vect102, double %84, i32 2
  %temp.vect104 = insertelement <16 x double> %temp.vect103, double %85, i32 3
  %temp.vect105 = insertelement <16 x double> %temp.vect104, double %86, i32 4
  %temp.vect106 = insertelement <16 x double> %temp.vect105, double %87, i32 5
  %temp.vect107 = insertelement <16 x double> %temp.vect106, double %88, i32 6
  %temp.vect108 = insertelement <16 x double> %temp.vect107, double %89, i32 7
  %temp.vect109 = insertelement <16 x double> %temp.vect108, double %90, i32 8
  %temp.vect110 = insertelement <16 x double> %temp.vect109, double %91, i32 9
  %temp.vect111 = insertelement <16 x double> %temp.vect110, double %92, i32 10
  %temp.vect112 = insertelement <16 x double> %temp.vect111, double %93, i32 11
  %temp.vect113 = insertelement <16 x double> %temp.vect112, double %94, i32 12
  %temp.vect114 = insertelement <16 x double> %temp.vect113, double %95, i32 13
  %temp.vect115 = insertelement <16 x double> %temp.vect114, double %96, i32 14
  %temp.vect116 = insertelement <16 x double> %temp.vect115, double %97, i32 15
  %98 = add nsw <16 x i32> %23, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %"&(pSB[currWI].offset)1157" = add nuw i64 %CurrSBIndex..0, 192
  %"&pSB[currWI].offset1158" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1157"
  %CastToValueType1159 = bitcast i8* %"&pSB[currWI].offset1158" to <16 x i32>*
  store <16 x i32> %98, <16 x i32>* %CastToValueType1159, align 64
  %99 = add nsw i32 %19, 1
  %100 = sext i32 %99 to i64
  %"&(pSB[currWI].offset)1181" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1182" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1181"
  %CastToValueType1183 = bitcast i8* %"&pSB[currWI].offset1182" to i64*
  store i64 %100, i64* %CastToValueType1183, align 8
  %101 = extractelement <16 x i32> %98, i32 0
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %100, i64 %102
  %"&(pSB[currWI].offset)1205" = add nuw i64 %CurrSBIndex..0, 264
  %"&pSB[currWI].offset1206" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1205"
  %CastToValueType1207 = bitcast i8* %"&pSB[currWI].offset1206" to double addrspace(3)**
  store double addrspace(3)* %103, double addrspace(3)** %CastToValueType1207, align 8
  %ptrTypeCast = bitcast double addrspace(3)* %103 to <16 x double> addrspace(3)*
  store <16 x double> %temp.vect116, <16 x double> addrspace(3)* %ptrTypeCast, align 8
  %104 = icmp eq i32 %19, 0
  %105 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %106 = mul nsw <16 x i32> %105, %vector118
  %107 = add nsw <16 x i32> %27, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %108 = add nsw <16 x i32> %107, %106
  br i1 %104, label %preload, label %postload

preload:                                          ; preds = %SyncBB1220
  %109 = extractelement <16 x i32> %108, i32 15
  %110 = sext i32 %109 to i64
  %111 = getelementptr inbounds double addrspace(1)* %data, i64 %110
  %112 = extractelement <16 x i32> %108, i32 14
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds double addrspace(1)* %data, i64 %113
  %115 = extractelement <16 x i32> %108, i32 13
  %116 = sext i32 %115 to i64
  %117 = getelementptr inbounds double addrspace(1)* %data, i64 %116
  %118 = extractelement <16 x i32> %108, i32 12
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds double addrspace(1)* %data, i64 %119
  %121 = extractelement <16 x i32> %108, i32 11
  %122 = sext i32 %121 to i64
  %123 = getelementptr inbounds double addrspace(1)* %data, i64 %122
  %124 = extractelement <16 x i32> %108, i32 10
  %125 = sext i32 %124 to i64
  %126 = getelementptr inbounds double addrspace(1)* %data, i64 %125
  %127 = extractelement <16 x i32> %108, i32 9
  %128 = sext i32 %127 to i64
  %129 = getelementptr inbounds double addrspace(1)* %data, i64 %128
  %130 = extractelement <16 x i32> %108, i32 8
  %131 = sext i32 %130 to i64
  %132 = getelementptr inbounds double addrspace(1)* %data, i64 %131
  %133 = extractelement <16 x i32> %108, i32 7
  %134 = sext i32 %133 to i64
  %135 = getelementptr inbounds double addrspace(1)* %data, i64 %134
  %136 = extractelement <16 x i32> %108, i32 6
  %137 = sext i32 %136 to i64
  %138 = getelementptr inbounds double addrspace(1)* %data, i64 %137
  %139 = extractelement <16 x i32> %108, i32 5
  %140 = sext i32 %139 to i64
  %141 = getelementptr inbounds double addrspace(1)* %data, i64 %140
  %142 = extractelement <16 x i32> %108, i32 4
  %143 = sext i32 %142 to i64
  %144 = getelementptr inbounds double addrspace(1)* %data, i64 %143
  %145 = extractelement <16 x i32> %108, i32 3
  %146 = sext i32 %145 to i64
  %147 = getelementptr inbounds double addrspace(1)* %data, i64 %146
  %148 = extractelement <16 x i32> %108, i32 2
  %149 = sext i32 %148 to i64
  %150 = getelementptr inbounds double addrspace(1)* %data, i64 %149
  %151 = extractelement <16 x i32> %108, i32 1
  %152 = sext i32 %151 to i64
  %153 = getelementptr inbounds double addrspace(1)* %data, i64 %152
  %154 = extractelement <16 x i32> %108, i32 0
  %155 = sext i32 %154 to i64
  %156 = getelementptr inbounds double addrspace(1)* %data, i64 %155
  %masked_load = load double addrspace(1)* %156, align 8
  %masked_load390 = load double addrspace(1)* %153, align 8
  %masked_load391 = load double addrspace(1)* %150, align 8
  %masked_load392 = load double addrspace(1)* %147, align 8
  %masked_load393 = load double addrspace(1)* %144, align 8
  %masked_load394 = load double addrspace(1)* %141, align 8
  %masked_load395 = load double addrspace(1)* %138, align 8
  %masked_load396 = load double addrspace(1)* %135, align 8
  %masked_load397 = load double addrspace(1)* %132, align 8
  %masked_load398 = load double addrspace(1)* %129, align 8
  %masked_load399 = load double addrspace(1)* %126, align 8
  %masked_load400 = load double addrspace(1)* %123, align 8
  %masked_load401 = load double addrspace(1)* %120, align 8
  %masked_load402 = load double addrspace(1)* %117, align 8
  %masked_load403 = load double addrspace(1)* %114, align 8
  %masked_load404 = load double addrspace(1)* %111, align 8
  br label %postload

postload:                                         ; preds = %preload, %SyncBB1220
  %phi = phi double [ undef, %SyncBB1220 ], [ %masked_load, %preload ]
  %phi517 = phi double [ undef, %SyncBB1220 ], [ %masked_load390, %preload ]
  %phi518 = phi double [ undef, %SyncBB1220 ], [ %masked_load391, %preload ]
  %phi519 = phi double [ undef, %SyncBB1220 ], [ %masked_load392, %preload ]
  %phi520 = phi double [ undef, %SyncBB1220 ], [ %masked_load393, %preload ]
  %phi521 = phi double [ undef, %SyncBB1220 ], [ %masked_load394, %preload ]
  %phi522 = phi double [ undef, %SyncBB1220 ], [ %masked_load395, %preload ]
  %phi523 = phi double [ undef, %SyncBB1220 ], [ %masked_load396, %preload ]
  %phi524 = phi double [ undef, %SyncBB1220 ], [ %masked_load397, %preload ]
  %phi525 = phi double [ undef, %SyncBB1220 ], [ %masked_load398, %preload ]
  %phi526 = phi double [ undef, %SyncBB1220 ], [ %masked_load399, %preload ]
  %phi527 = phi double [ undef, %SyncBB1220 ], [ %masked_load400, %preload ]
  %phi528 = phi double [ undef, %SyncBB1220 ], [ %masked_load401, %preload ]
  %phi529 = phi double [ undef, %SyncBB1220 ], [ %masked_load402, %preload ]
  %phi530 = phi double [ undef, %SyncBB1220 ], [ %masked_load403, %preload ]
  %phi531 = phi double [ undef, %SyncBB1220 ], [ %masked_load404, %preload ]
  br i1 %104, label %preload532, label %postload533

preload532:                                       ; preds = %postload
  %temp.vect136 = insertelement <16 x double> undef, double %phi, i32 0
  %temp.vect137 = insertelement <16 x double> %temp.vect136, double %phi517, i32 1
  %temp.vect138 = insertelement <16 x double> %temp.vect137, double %phi518, i32 2
  %temp.vect139 = insertelement <16 x double> %temp.vect138, double %phi519, i32 3
  %temp.vect140 = insertelement <16 x double> %temp.vect139, double %phi520, i32 4
  %temp.vect141 = insertelement <16 x double> %temp.vect140, double %phi521, i32 5
  %temp.vect142 = insertelement <16 x double> %temp.vect141, double %phi522, i32 6
  %temp.vect143 = insertelement <16 x double> %temp.vect142, double %phi523, i32 7
  %temp.vect144 = insertelement <16 x double> %temp.vect143, double %phi524, i32 8
  %temp.vect145 = insertelement <16 x double> %temp.vect144, double %phi525, i32 9
  %temp.vect146 = insertelement <16 x double> %temp.vect145, double %phi526, i32 10
  %temp.vect147 = insertelement <16 x double> %temp.vect146, double %phi527, i32 11
  %"&(pSB[currWI].offset)1176" = add nuw i64 %CurrSBIndex..0, 192
  %"&pSB[currWI].offset1177" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1176"
  %CastToValueType1178 = bitcast i8* %"&pSB[currWI].offset1177" to <16 x i32>*
  %loadedValue1179 = load <16 x i32>* %CastToValueType1178, align 64
  %157 = extractelement <16 x i32> %loadedValue1179, i32 0
  %temp.vect148 = insertelement <16 x double> %temp.vect147, double %phi528, i32 12
  %158 = sext i32 %157 to i64
  %temp.vect149 = insertelement <16 x double> %temp.vect148, double %phi529, i32 13
  %159 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 0, i64 %158
  %temp.vect150 = insertelement <16 x double> %temp.vect149, double %phi530, i32 14
  %ptrTypeCast135 = bitcast double addrspace(3)* %159 to <16 x double> addrspace(3)*
  %temp.vect151 = insertelement <16 x double> %temp.vect150, double %phi531, i32 15
  store <16 x double> %temp.vect151, <16 x double> addrspace(3)* %ptrTypeCast135, align 128
  br label %postload533

postload533:                                      ; preds = %preload532, %postload
  %160 = add nsw i32 %25, 17
  %temp152 = insertelement <16 x i32> undef, i32 %160, i32 0
  %vector153 = shufflevector <16 x i32> %temp152, <16 x i32> undef, <16 x i32> zeroinitializer
  %161 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %162 = mul nsw <16 x i32> %161, %vector153
  %163 = add nsw <16 x i32> %27, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %164 = add nsw <16 x i32> %163, %162
  br i1 %104, label %preload534, label %postload535

preload534:                                       ; preds = %postload533
  %165 = extractelement <16 x i32> %164, i32 15
  %166 = sext i32 %165 to i64
  %167 = getelementptr inbounds double addrspace(1)* %data, i64 %166
  %168 = extractelement <16 x i32> %164, i32 14
  %169 = sext i32 %168 to i64
  %170 = getelementptr inbounds double addrspace(1)* %data, i64 %169
  %171 = extractelement <16 x i32> %164, i32 13
  %172 = sext i32 %171 to i64
  %173 = getelementptr inbounds double addrspace(1)* %data, i64 %172
  %174 = extractelement <16 x i32> %164, i32 12
  %175 = sext i32 %174 to i64
  %176 = getelementptr inbounds double addrspace(1)* %data, i64 %175
  %177 = extractelement <16 x i32> %164, i32 11
  %178 = sext i32 %177 to i64
  %179 = getelementptr inbounds double addrspace(1)* %data, i64 %178
  %180 = extractelement <16 x i32> %164, i32 10
  %181 = sext i32 %180 to i64
  %182 = getelementptr inbounds double addrspace(1)* %data, i64 %181
  %183 = extractelement <16 x i32> %164, i32 9
  %184 = sext i32 %183 to i64
  %185 = getelementptr inbounds double addrspace(1)* %data, i64 %184
  %186 = extractelement <16 x i32> %164, i32 8
  %187 = sext i32 %186 to i64
  %188 = getelementptr inbounds double addrspace(1)* %data, i64 %187
  %189 = extractelement <16 x i32> %164, i32 7
  %190 = sext i32 %189 to i64
  %191 = getelementptr inbounds double addrspace(1)* %data, i64 %190
  %192 = extractelement <16 x i32> %164, i32 6
  %193 = sext i32 %192 to i64
  %194 = getelementptr inbounds double addrspace(1)* %data, i64 %193
  %195 = extractelement <16 x i32> %164, i32 5
  %196 = sext i32 %195 to i64
  %197 = getelementptr inbounds double addrspace(1)* %data, i64 %196
  %198 = extractelement <16 x i32> %164, i32 4
  %199 = sext i32 %198 to i64
  %200 = getelementptr inbounds double addrspace(1)* %data, i64 %199
  %201 = extractelement <16 x i32> %164, i32 3
  %202 = sext i32 %201 to i64
  %203 = getelementptr inbounds double addrspace(1)* %data, i64 %202
  %204 = extractelement <16 x i32> %164, i32 2
  %205 = sext i32 %204 to i64
  %206 = getelementptr inbounds double addrspace(1)* %data, i64 %205
  %207 = extractelement <16 x i32> %164, i32 1
  %208 = sext i32 %207 to i64
  %209 = getelementptr inbounds double addrspace(1)* %data, i64 %208
  %210 = extractelement <16 x i32> %164, i32 0
  %211 = sext i32 %210 to i64
  %212 = getelementptr inbounds double addrspace(1)* %data, i64 %211
  %masked_load405 = load double addrspace(1)* %212, align 8
  %masked_load406 = load double addrspace(1)* %209, align 8
  %masked_load407 = load double addrspace(1)* %206, align 8
  %masked_load408 = load double addrspace(1)* %203, align 8
  %masked_load409 = load double addrspace(1)* %200, align 8
  %masked_load410 = load double addrspace(1)* %197, align 8
  %masked_load411 = load double addrspace(1)* %194, align 8
  %masked_load412 = load double addrspace(1)* %191, align 8
  %masked_load413 = load double addrspace(1)* %188, align 8
  %masked_load414 = load double addrspace(1)* %185, align 8
  %masked_load415 = load double addrspace(1)* %182, align 8
  %masked_load416 = load double addrspace(1)* %179, align 8
  %masked_load417 = load double addrspace(1)* %176, align 8
  %masked_load418 = load double addrspace(1)* %173, align 8
  %masked_load419 = load double addrspace(1)* %170, align 8
  %masked_load420 = load double addrspace(1)* %167, align 8
  br label %postload535

postload535:                                      ; preds = %preload534, %postload533
  %phi536 = phi double [ undef, %postload533 ], [ %masked_load405, %preload534 ]
  %phi537 = phi double [ undef, %postload533 ], [ %masked_load406, %preload534 ]
  %phi538 = phi double [ undef, %postload533 ], [ %masked_load407, %preload534 ]
  %phi539 = phi double [ undef, %postload533 ], [ %masked_load408, %preload534 ]
  %phi540 = phi double [ undef, %postload533 ], [ %masked_load409, %preload534 ]
  %phi541 = phi double [ undef, %postload533 ], [ %masked_load410, %preload534 ]
  %phi542 = phi double [ undef, %postload533 ], [ %masked_load411, %preload534 ]
  %phi543 = phi double [ undef, %postload533 ], [ %masked_load412, %preload534 ]
  %phi544 = phi double [ undef, %postload533 ], [ %masked_load413, %preload534 ]
  %phi545 = phi double [ undef, %postload533 ], [ %masked_load414, %preload534 ]
  %phi546 = phi double [ undef, %postload533 ], [ %masked_load415, %preload534 ]
  %phi547 = phi double [ undef, %postload533 ], [ %masked_load416, %preload534 ]
  %phi548 = phi double [ undef, %postload533 ], [ %masked_load417, %preload534 ]
  %phi549 = phi double [ undef, %postload533 ], [ %masked_load418, %preload534 ]
  %phi550 = phi double [ undef, %postload533 ], [ %masked_load419, %preload534 ]
  %phi551 = phi double [ undef, %postload533 ], [ %masked_load420, %preload534 ]
  br i1 %104, label %preload552, label %postload553

preload552:                                       ; preds = %postload535
  %temp.vect171 = insertelement <16 x double> undef, double %phi536, i32 0
  %temp.vect172 = insertelement <16 x double> %temp.vect171, double %phi537, i32 1
  %temp.vect173 = insertelement <16 x double> %temp.vect172, double %phi538, i32 2
  %temp.vect174 = insertelement <16 x double> %temp.vect173, double %phi539, i32 3
  %temp.vect175 = insertelement <16 x double> %temp.vect174, double %phi540, i32 4
  %temp.vect176 = insertelement <16 x double> %temp.vect175, double %phi541, i32 5
  %temp.vect177 = insertelement <16 x double> %temp.vect176, double %phi542, i32 6
  %temp.vect178 = insertelement <16 x double> %temp.vect177, double %phi543, i32 7
  %temp.vect179 = insertelement <16 x double> %temp.vect178, double %phi544, i32 8
  %temp.vect180 = insertelement <16 x double> %temp.vect179, double %phi545, i32 9
  %temp.vect181 = insertelement <16 x double> %temp.vect180, double %phi546, i32 10
  %temp.vect182 = insertelement <16 x double> %temp.vect181, double %phi547, i32 11
  %"&(pSB[currWI].offset)1171" = add nuw i64 %CurrSBIndex..0, 192
  %"&pSB[currWI].offset1172" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1171"
  %CastToValueType1173 = bitcast i8* %"&pSB[currWI].offset1172" to <16 x i32>*
  %loadedValue1174 = load <16 x i32>* %CastToValueType1173, align 64
  %213 = extractelement <16 x i32> %loadedValue1174, i32 0
  %temp.vect183 = insertelement <16 x double> %temp.vect182, double %phi548, i32 12
  %214 = sext i32 %213 to i64
  %temp.vect184 = insertelement <16 x double> %temp.vect183, double %phi549, i32 13
  %215 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 17, i64 %214
  %temp.vect185 = insertelement <16 x double> %temp.vect184, double %phi550, i32 14
  %ptrTypeCast170 = bitcast double addrspace(3)* %215 to <16 x double> addrspace(3)*
  %temp.vect186 = insertelement <16 x double> %temp.vect185, double %phi551, i32 15
  store <16 x double> %temp.vect186, <16 x double> addrspace(3)* %ptrTypeCast170, align 128
  br label %postload553

postload553:                                      ; preds = %preload552, %postload535
  %"&(pSB[currWI].offset)1068" = add nuw i64 %CurrSBIndex..0, 64
  %"&pSB[currWI].offset1069" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1068"
  %CastToValueType1070 = bitcast i8* %"&pSB[currWI].offset1069" to <16 x i32>*
  %loadedValue1071 = load <16 x i32>* %CastToValueType1070, align 64
  %216 = icmp eq <16 x i32> %loadedValue1071, zeroinitializer
  %extract206 = extractelement <16 x i1> %216, i32 0
  %extract207 = extractelement <16 x i1> %216, i32 1
  %extract208 = extractelement <16 x i1> %216, i32 2
  %extract209 = extractelement <16 x i1> %216, i32 3
  %extract210 = extractelement <16 x i1> %216, i32 4
  %extract211 = extractelement <16 x i1> %216, i32 5
  %extract212 = extractelement <16 x i1> %216, i32 6
  %extract213 = extractelement <16 x i1> %216, i32 7
  %extract214 = extractelement <16 x i1> %216, i32 8
  %extract215 = extractelement <16 x i1> %216, i32 9
  %extract216 = extractelement <16 x i1> %216, i32 10
  %extract217 = extractelement <16 x i1> %216, i32 11
  %extract218 = extractelement <16 x i1> %216, i32 12
  %extract219 = extractelement <16 x i1> %216, i32 13
  %extract220 = extractelement <16 x i1> %216, i32 14
  %extract221 = extractelement <16 x i1> %216, i32 15
  %217 = add nsw i32 %25, 1
  %temp188 = insertelement <16 x i32> undef, i32 %217, i32 0
  %vector189 = shufflevector <16 x i32> %temp188, <16 x i32> undef, <16 x i32> zeroinitializer
  %218 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %219 = mul nsw <16 x i32> %218, %vector189
  %220 = add nsw <16 x i32> %27, %219
  %221 = extractelement <16 x i32> %220, i32 1
  %222 = sext i32 %221 to i64
  %223 = getelementptr inbounds double addrspace(1)* %data, i64 %222
  %224 = extractelement <16 x i32> %220, i32 2
  %225 = sext i32 %224 to i64
  %226 = getelementptr inbounds double addrspace(1)* %data, i64 %225
  %227 = extractelement <16 x i32> %220, i32 3
  %228 = sext i32 %227 to i64
  %229 = getelementptr inbounds double addrspace(1)* %data, i64 %228
  %230 = extractelement <16 x i32> %220, i32 4
  %231 = sext i32 %230 to i64
  %232 = getelementptr inbounds double addrspace(1)* %data, i64 %231
  %233 = extractelement <16 x i32> %220, i32 5
  %234 = sext i32 %233 to i64
  %235 = getelementptr inbounds double addrspace(1)* %data, i64 %234
  %236 = extractelement <16 x i32> %220, i32 6
  %237 = sext i32 %236 to i64
  %238 = getelementptr inbounds double addrspace(1)* %data, i64 %237
  %239 = extractelement <16 x i32> %220, i32 7
  %240 = sext i32 %239 to i64
  %241 = getelementptr inbounds double addrspace(1)* %data, i64 %240
  %242 = extractelement <16 x i32> %220, i32 8
  %243 = sext i32 %242 to i64
  %244 = getelementptr inbounds double addrspace(1)* %data, i64 %243
  %245 = extractelement <16 x i32> %220, i32 9
  %246 = sext i32 %245 to i64
  %247 = getelementptr inbounds double addrspace(1)* %data, i64 %246
  %248 = extractelement <16 x i32> %220, i32 10
  %249 = sext i32 %248 to i64
  %250 = getelementptr inbounds double addrspace(1)* %data, i64 %249
  %251 = extractelement <16 x i32> %220, i32 11
  %252 = sext i32 %251 to i64
  %253 = getelementptr inbounds double addrspace(1)* %data, i64 %252
  %254 = extractelement <16 x i32> %220, i32 12
  %255 = sext i32 %254 to i64
  %256 = getelementptr inbounds double addrspace(1)* %data, i64 %255
  %257 = extractelement <16 x i32> %220, i32 13
  %258 = sext i32 %257 to i64
  %259 = getelementptr inbounds double addrspace(1)* %data, i64 %258
  %260 = extractelement <16 x i32> %220, i32 14
  %261 = sext i32 %260 to i64
  %262 = getelementptr inbounds double addrspace(1)* %data, i64 %261
  %263 = extractelement <16 x i32> %220, i32 15
  %264 = sext i32 %263 to i64
  %265 = getelementptr inbounds double addrspace(1)* %data, i64 %264
  br i1 %extract206, label %preload554, label %postload555

preload554:                                       ; preds = %postload553
  %266 = extractelement <16 x i32> %220, i32 0
  %267 = sext i32 %266 to i64
  %268 = getelementptr inbounds double addrspace(1)* %data, i64 %267
  %masked_load421 = load double addrspace(1)* %268, align 8
  br label %postload555

postload555:                                      ; preds = %preload554, %postload553
  %phi556 = phi double [ undef, %postload553 ], [ %masked_load421, %preload554 ]
  br i1 %extract207, label %preload564, label %postload565

preload564:                                       ; preds = %postload555
  %masked_load422 = load double addrspace(1)* %223, align 8
  br label %postload565

postload565:                                      ; preds = %preload564, %postload555
  %phi566 = phi double [ undef, %postload555 ], [ %masked_load422, %preload564 ]
  br i1 %extract208, label %preload574, label %postload575

preload574:                                       ; preds = %postload565
  %masked_load423 = load double addrspace(1)* %226, align 8
  br label %postload575

postload575:                                      ; preds = %preload574, %postload565
  %phi576 = phi double [ undef, %postload565 ], [ %masked_load423, %preload574 ]
  br i1 %extract209, label %preload584, label %postload585

preload584:                                       ; preds = %postload575
  %masked_load424 = load double addrspace(1)* %229, align 8
  br label %postload585

postload585:                                      ; preds = %preload584, %postload575
  %phi586 = phi double [ undef, %postload575 ], [ %masked_load424, %preload584 ]
  br i1 %extract210, label %preload594, label %postload595

preload594:                                       ; preds = %postload585
  %masked_load425 = load double addrspace(1)* %232, align 8
  br label %postload595

postload595:                                      ; preds = %preload594, %postload585
  %phi596 = phi double [ undef, %postload585 ], [ %masked_load425, %preload594 ]
  br i1 %extract211, label %preload604, label %postload605

preload604:                                       ; preds = %postload595
  %masked_load426 = load double addrspace(1)* %235, align 8
  br label %postload605

postload605:                                      ; preds = %preload604, %postload595
  %phi606 = phi double [ undef, %postload595 ], [ %masked_load426, %preload604 ]
  br i1 %extract212, label %preload614, label %postload615

preload614:                                       ; preds = %postload605
  %masked_load427 = load double addrspace(1)* %238, align 8
  br label %postload615

postload615:                                      ; preds = %preload614, %postload605
  %phi616 = phi double [ undef, %postload605 ], [ %masked_load427, %preload614 ]
  br i1 %extract213, label %preload624, label %postload625

preload624:                                       ; preds = %postload615
  %masked_load428 = load double addrspace(1)* %241, align 8
  br label %postload625

postload625:                                      ; preds = %preload624, %postload615
  %phi626 = phi double [ undef, %postload615 ], [ %masked_load428, %preload624 ]
  br i1 %extract214, label %preload634, label %postload635

preload634:                                       ; preds = %postload625
  %masked_load429 = load double addrspace(1)* %244, align 8
  br label %postload635

postload635:                                      ; preds = %preload634, %postload625
  %phi636 = phi double [ undef, %postload625 ], [ %masked_load429, %preload634 ]
  br i1 %extract215, label %preload644, label %postload645

preload644:                                       ; preds = %postload635
  %masked_load430 = load double addrspace(1)* %247, align 8
  br label %postload645

postload645:                                      ; preds = %preload644, %postload635
  %phi646 = phi double [ undef, %postload635 ], [ %masked_load430, %preload644 ]
  br i1 %extract216, label %preload654, label %postload655

preload654:                                       ; preds = %postload645
  %masked_load431 = load double addrspace(1)* %250, align 8
  br label %postload655

postload655:                                      ; preds = %preload654, %postload645
  %phi656 = phi double [ undef, %postload645 ], [ %masked_load431, %preload654 ]
  br i1 %extract217, label %preload664, label %postload665

preload664:                                       ; preds = %postload655
  %masked_load432 = load double addrspace(1)* %253, align 8
  br label %postload665

postload665:                                      ; preds = %preload664, %postload655
  %phi666 = phi double [ undef, %postload655 ], [ %masked_load432, %preload664 ]
  br i1 %extract218, label %preload674, label %postload675

preload674:                                       ; preds = %postload665
  %masked_load433 = load double addrspace(1)* %256, align 8
  br label %postload675

postload675:                                      ; preds = %preload674, %postload665
  %phi676 = phi double [ undef, %postload665 ], [ %masked_load433, %preload674 ]
  br i1 %extract219, label %preload684, label %postload685

preload684:                                       ; preds = %postload675
  %masked_load434 = load double addrspace(1)* %259, align 8
  br label %postload685

postload685:                                      ; preds = %preload684, %postload675
  %phi686 = phi double [ undef, %postload675 ], [ %masked_load434, %preload684 ]
  br i1 %extract220, label %preload694, label %postload695

preload694:                                       ; preds = %postload685
  %masked_load435 = load double addrspace(1)* %262, align 8
  br label %postload695

postload695:                                      ; preds = %preload694, %postload685
  %phi696 = phi double [ undef, %postload685 ], [ %masked_load435, %preload694 ]
  br i1 %extract221, label %preload704, label %postload705

preload704:                                       ; preds = %postload695
  %masked_load436 = load double addrspace(1)* %265, align 8
  br label %postload705

postload705:                                      ; preds = %preload704, %postload695
  %phi706 = phi double [ undef, %postload695 ], [ %masked_load436, %preload704 ]
  %"&(pSB[currWI].offset)1200" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1201" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1200"
  %CastToValueType1202 = bitcast i8* %"&pSB[currWI].offset1201" to i64*
  %loadedValue1203 = load i64* %CastToValueType1202, align 8
  %269 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue1203, i64 0
  br i1 %extract206, label %preload557, label %postload558

preload557:                                       ; preds = %postload705
  store double %phi556, double addrspace(3)* %269, align 8
  br label %postload558

postload558:                                      ; preds = %preload557, %postload705
  br i1 %extract207, label %preload567, label %postload568

preload567:                                       ; preds = %postload558
  store double %phi566, double addrspace(3)* %269, align 8
  br label %postload568

postload568:                                      ; preds = %preload567, %postload558
  br i1 %extract208, label %preload577, label %postload578

preload577:                                       ; preds = %postload568
  store double %phi576, double addrspace(3)* %269, align 8
  br label %postload578

postload578:                                      ; preds = %preload577, %postload568
  br i1 %extract209, label %preload587, label %postload588

preload587:                                       ; preds = %postload578
  store double %phi586, double addrspace(3)* %269, align 8
  br label %postload588

postload588:                                      ; preds = %preload587, %postload578
  br i1 %extract210, label %preload597, label %postload598

preload597:                                       ; preds = %postload588
  store double %phi596, double addrspace(3)* %269, align 8
  br label %postload598

postload598:                                      ; preds = %preload597, %postload588
  br i1 %extract211, label %preload607, label %postload608

preload607:                                       ; preds = %postload598
  store double %phi606, double addrspace(3)* %269, align 8
  br label %postload608

postload608:                                      ; preds = %preload607, %postload598
  br i1 %extract212, label %preload617, label %postload618

preload617:                                       ; preds = %postload608
  store double %phi616, double addrspace(3)* %269, align 8
  br label %postload618

postload618:                                      ; preds = %preload617, %postload608
  br i1 %extract213, label %preload627, label %postload628

preload627:                                       ; preds = %postload618
  store double %phi626, double addrspace(3)* %269, align 8
  br label %postload628

postload628:                                      ; preds = %preload627, %postload618
  br i1 %extract214, label %preload637, label %postload638

preload637:                                       ; preds = %postload628
  store double %phi636, double addrspace(3)* %269, align 8
  br label %postload638

postload638:                                      ; preds = %preload637, %postload628
  br i1 %extract215, label %preload647, label %postload648

preload647:                                       ; preds = %postload638
  store double %phi646, double addrspace(3)* %269, align 8
  br label %postload648

postload648:                                      ; preds = %preload647, %postload638
  br i1 %extract216, label %preload657, label %postload658

preload657:                                       ; preds = %postload648
  store double %phi656, double addrspace(3)* %269, align 8
  br label %postload658

postload658:                                      ; preds = %preload657, %postload648
  br i1 %extract217, label %preload667, label %postload668

preload667:                                       ; preds = %postload658
  store double %phi666, double addrspace(3)* %269, align 8
  br label %postload668

postload668:                                      ; preds = %preload667, %postload658
  br i1 %extract218, label %preload677, label %postload678

preload677:                                       ; preds = %postload668
  store double %phi676, double addrspace(3)* %269, align 8
  br label %postload678

postload678:                                      ; preds = %preload677, %postload668
  br i1 %extract219, label %preload687, label %postload688

preload687:                                       ; preds = %postload678
  store double %phi686, double addrspace(3)* %269, align 8
  br label %postload688

postload688:                                      ; preds = %preload687, %postload678
  br i1 %extract220, label %preload697, label %postload698

preload697:                                       ; preds = %postload688
  store double %phi696, double addrspace(3)* %269, align 8
  br label %postload698

postload698:                                      ; preds = %preload697, %postload688
  br i1 %extract221, label %preload707, label %postload708

preload707:                                       ; preds = %postload698
  store double %phi706, double addrspace(3)* %269, align 8
  br label %postload708

postload708:                                      ; preds = %preload707, %postload698
  %270 = add nsw i32 %25, 1
  %temp222 = insertelement <16 x i32> undef, i32 %270, i32 0
  %vector223 = shufflevector <16 x i32> %temp222, <16 x i32> undef, <16 x i32> zeroinitializer
  %271 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %272 = mul nsw <16 x i32> %271, %vector223
  %273 = add nsw <16 x i32> %27, <i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17>
  %274 = add nsw <16 x i32> %273, %272
  %275 = extractelement <16 x i32> %274, i32 1
  %276 = sext i32 %275 to i64
  %277 = getelementptr inbounds double addrspace(1)* %data, i64 %276
  %278 = extractelement <16 x i32> %274, i32 2
  %279 = sext i32 %278 to i64
  %280 = getelementptr inbounds double addrspace(1)* %data, i64 %279
  %281 = extractelement <16 x i32> %274, i32 3
  %282 = sext i32 %281 to i64
  %283 = getelementptr inbounds double addrspace(1)* %data, i64 %282
  %284 = extractelement <16 x i32> %274, i32 4
  %285 = sext i32 %284 to i64
  %286 = getelementptr inbounds double addrspace(1)* %data, i64 %285
  %287 = extractelement <16 x i32> %274, i32 5
  %288 = sext i32 %287 to i64
  %289 = getelementptr inbounds double addrspace(1)* %data, i64 %288
  %290 = extractelement <16 x i32> %274, i32 6
  %291 = sext i32 %290 to i64
  %292 = getelementptr inbounds double addrspace(1)* %data, i64 %291
  %293 = extractelement <16 x i32> %274, i32 7
  %294 = sext i32 %293 to i64
  %295 = getelementptr inbounds double addrspace(1)* %data, i64 %294
  %296 = extractelement <16 x i32> %274, i32 8
  %297 = sext i32 %296 to i64
  %298 = getelementptr inbounds double addrspace(1)* %data, i64 %297
  %299 = extractelement <16 x i32> %274, i32 9
  %300 = sext i32 %299 to i64
  %301 = getelementptr inbounds double addrspace(1)* %data, i64 %300
  %302 = extractelement <16 x i32> %274, i32 10
  %303 = sext i32 %302 to i64
  %304 = getelementptr inbounds double addrspace(1)* %data, i64 %303
  %305 = extractelement <16 x i32> %274, i32 11
  %306 = sext i32 %305 to i64
  %307 = getelementptr inbounds double addrspace(1)* %data, i64 %306
  %308 = extractelement <16 x i32> %274, i32 12
  %309 = sext i32 %308 to i64
  %310 = getelementptr inbounds double addrspace(1)* %data, i64 %309
  %311 = extractelement <16 x i32> %274, i32 13
  %312 = sext i32 %311 to i64
  %313 = getelementptr inbounds double addrspace(1)* %data, i64 %312
  %314 = extractelement <16 x i32> %274, i32 14
  %315 = sext i32 %314 to i64
  %316 = getelementptr inbounds double addrspace(1)* %data, i64 %315
  %317 = extractelement <16 x i32> %274, i32 15
  %318 = sext i32 %317 to i64
  %319 = getelementptr inbounds double addrspace(1)* %data, i64 %318
  br i1 %extract206, label %preload559, label %postload560

preload559:                                       ; preds = %postload708
  %320 = extractelement <16 x i32> %274, i32 0
  %321 = sext i32 %320 to i64
  %322 = getelementptr inbounds double addrspace(1)* %data, i64 %321
  %masked_load437 = load double addrspace(1)* %322, align 8
  br label %postload560

postload560:                                      ; preds = %preload559, %postload708
  %phi561 = phi double [ undef, %postload708 ], [ %masked_load437, %preload559 ]
  br i1 %extract207, label %preload569, label %postload570

preload569:                                       ; preds = %postload560
  %masked_load438 = load double addrspace(1)* %277, align 8
  br label %postload570

postload570:                                      ; preds = %preload569, %postload560
  %phi571 = phi double [ undef, %postload560 ], [ %masked_load438, %preload569 ]
  br i1 %extract208, label %preload579, label %postload580

preload579:                                       ; preds = %postload570
  %masked_load439 = load double addrspace(1)* %280, align 8
  br label %postload580

postload580:                                      ; preds = %preload579, %postload570
  %phi581 = phi double [ undef, %postload570 ], [ %masked_load439, %preload579 ]
  br i1 %extract209, label %preload589, label %postload590

preload589:                                       ; preds = %postload580
  %masked_load440 = load double addrspace(1)* %283, align 8
  br label %postload590

postload590:                                      ; preds = %preload589, %postload580
  %phi591 = phi double [ undef, %postload580 ], [ %masked_load440, %preload589 ]
  br i1 %extract210, label %preload599, label %postload600

preload599:                                       ; preds = %postload590
  %masked_load441 = load double addrspace(1)* %286, align 8
  br label %postload600

postload600:                                      ; preds = %preload599, %postload590
  %phi601 = phi double [ undef, %postload590 ], [ %masked_load441, %preload599 ]
  br i1 %extract211, label %preload609, label %postload610

preload609:                                       ; preds = %postload600
  %masked_load442 = load double addrspace(1)* %289, align 8
  br label %postload610

postload610:                                      ; preds = %preload609, %postload600
  %phi611 = phi double [ undef, %postload600 ], [ %masked_load442, %preload609 ]
  br i1 %extract212, label %preload619, label %postload620

preload619:                                       ; preds = %postload610
  %masked_load443 = load double addrspace(1)* %292, align 8
  br label %postload620

postload620:                                      ; preds = %preload619, %postload610
  %phi621 = phi double [ undef, %postload610 ], [ %masked_load443, %preload619 ]
  br i1 %extract213, label %preload629, label %postload630

preload629:                                       ; preds = %postload620
  %masked_load444 = load double addrspace(1)* %295, align 8
  br label %postload630

postload630:                                      ; preds = %preload629, %postload620
  %phi631 = phi double [ undef, %postload620 ], [ %masked_load444, %preload629 ]
  br i1 %extract214, label %preload639, label %postload640

preload639:                                       ; preds = %postload630
  %masked_load445 = load double addrspace(1)* %298, align 8
  br label %postload640

postload640:                                      ; preds = %preload639, %postload630
  %phi641 = phi double [ undef, %postload630 ], [ %masked_load445, %preload639 ]
  br i1 %extract215, label %preload649, label %postload650

preload649:                                       ; preds = %postload640
  %masked_load446 = load double addrspace(1)* %301, align 8
  br label %postload650

postload650:                                      ; preds = %preload649, %postload640
  %phi651 = phi double [ undef, %postload640 ], [ %masked_load446, %preload649 ]
  br i1 %extract216, label %preload659, label %postload660

preload659:                                       ; preds = %postload650
  %masked_load447 = load double addrspace(1)* %304, align 8
  br label %postload660

postload660:                                      ; preds = %preload659, %postload650
  %phi661 = phi double [ undef, %postload650 ], [ %masked_load447, %preload659 ]
  br i1 %extract217, label %preload669, label %postload670

preload669:                                       ; preds = %postload660
  %masked_load448 = load double addrspace(1)* %307, align 8
  br label %postload670

postload670:                                      ; preds = %preload669, %postload660
  %phi671 = phi double [ undef, %postload660 ], [ %masked_load448, %preload669 ]
  br i1 %extract218, label %preload679, label %postload680

preload679:                                       ; preds = %postload670
  %masked_load449 = load double addrspace(1)* %310, align 8
  br label %postload680

postload680:                                      ; preds = %preload679, %postload670
  %phi681 = phi double [ undef, %postload670 ], [ %masked_load449, %preload679 ]
  br i1 %extract219, label %preload689, label %postload690

preload689:                                       ; preds = %postload680
  %masked_load450 = load double addrspace(1)* %313, align 8
  br label %postload690

postload690:                                      ; preds = %preload689, %postload680
  %phi691 = phi double [ undef, %postload680 ], [ %masked_load450, %preload689 ]
  br i1 %extract220, label %preload699, label %postload700

preload699:                                       ; preds = %postload690
  %masked_load451 = load double addrspace(1)* %316, align 8
  br label %postload700

postload700:                                      ; preds = %preload699, %postload690
  %phi701 = phi double [ undef, %postload690 ], [ %masked_load451, %preload699 ]
  br i1 %extract221, label %preload709, label %postload710

preload709:                                       ; preds = %postload700
  %masked_load452 = load double addrspace(1)* %319, align 8
  br label %postload710

postload710:                                      ; preds = %preload709, %postload700
  %phi711 = phi double [ undef, %postload700 ], [ %masked_load452, %preload709 ]
  %"&(pSB[currWI].offset)1195" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1196" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1195"
  %CastToValueType1197 = bitcast i8* %"&pSB[currWI].offset1196" to i64*
  %loadedValue1198 = load i64* %CastToValueType1197, align 8
  %323 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue1198, i64 17
  br i1 %extract206, label %preload562, label %postload563

preload562:                                       ; preds = %postload710
  store double %phi561, double addrspace(3)* %323, align 8
  br label %postload563

postload563:                                      ; preds = %preload562, %postload710
  br i1 %extract207, label %preload572, label %postload573

preload572:                                       ; preds = %postload563
  store double %phi571, double addrspace(3)* %323, align 8
  br label %postload573

postload573:                                      ; preds = %preload572, %postload563
  br i1 %extract208, label %preload582, label %postload583

preload582:                                       ; preds = %postload573
  store double %phi581, double addrspace(3)* %323, align 8
  br label %postload583

postload583:                                      ; preds = %preload582, %postload573
  br i1 %extract209, label %preload592, label %postload593

preload592:                                       ; preds = %postload583
  store double %phi591, double addrspace(3)* %323, align 8
  br label %postload593

postload593:                                      ; preds = %preload592, %postload583
  br i1 %extract210, label %preload602, label %postload603

preload602:                                       ; preds = %postload593
  store double %phi601, double addrspace(3)* %323, align 8
  br label %postload603

postload603:                                      ; preds = %preload602, %postload593
  br i1 %extract211, label %preload612, label %postload613

preload612:                                       ; preds = %postload603
  store double %phi611, double addrspace(3)* %323, align 8
  br label %postload613

postload613:                                      ; preds = %preload612, %postload603
  br i1 %extract212, label %preload622, label %postload623

preload622:                                       ; preds = %postload613
  store double %phi621, double addrspace(3)* %323, align 8
  br label %postload623

postload623:                                      ; preds = %preload622, %postload613
  br i1 %extract213, label %preload632, label %postload633

preload632:                                       ; preds = %postload623
  store double %phi631, double addrspace(3)* %323, align 8
  br label %postload633

postload633:                                      ; preds = %preload632, %postload623
  br i1 %extract214, label %preload642, label %postload643

preload642:                                       ; preds = %postload633
  store double %phi641, double addrspace(3)* %323, align 8
  br label %postload643

postload643:                                      ; preds = %preload642, %postload633
  br i1 %extract215, label %preload652, label %postload653

preload652:                                       ; preds = %postload643
  store double %phi651, double addrspace(3)* %323, align 8
  br label %postload653

postload653:                                      ; preds = %preload652, %postload643
  br i1 %extract216, label %preload662, label %postload663

preload662:                                       ; preds = %postload653
  store double %phi661, double addrspace(3)* %323, align 8
  br label %postload663

postload663:                                      ; preds = %preload662, %postload653
  br i1 %extract217, label %preload672, label %postload673

preload672:                                       ; preds = %postload663
  store double %phi671, double addrspace(3)* %323, align 8
  br label %postload673

postload673:                                      ; preds = %preload672, %postload663
  br i1 %extract218, label %preload682, label %postload683

preload682:                                       ; preds = %postload673
  store double %phi681, double addrspace(3)* %323, align 8
  br label %postload683

postload683:                                      ; preds = %preload682, %postload673
  br i1 %extract219, label %preload692, label %postload693

preload692:                                       ; preds = %postload683
  store double %phi691, double addrspace(3)* %323, align 8
  br label %postload693

postload693:                                      ; preds = %preload692, %postload683
  br i1 %extract220, label %preload702, label %postload703

preload702:                                       ; preds = %postload693
  store double %phi701, double addrspace(3)* %323, align 8
  br label %postload703

postload703:                                      ; preds = %preload702, %postload693
  br i1 %extract221, label %preload712, label %postload713

preload712:                                       ; preds = %postload703
  store double %phi711, double addrspace(3)* %323, align 8
  br label %postload713

postload713:                                      ; preds = %preload712, %postload703
  %324 = or <16 x i64> %22, %vector241
  %325 = trunc <16 x i64> %324 to <16 x i32>
  %326 = icmp eq <16 x i32> %325, zeroinitializer
  %extract259 = extractelement <16 x i1> %326, i32 0
  %extract260 = extractelement <16 x i1> %326, i32 1
  %extract261 = extractelement <16 x i1> %326, i32 2
  %extract262 = extractelement <16 x i1> %326, i32 3
  %extract263 = extractelement <16 x i1> %326, i32 4
  %extract264 = extractelement <16 x i1> %326, i32 5
  %extract265 = extractelement <16 x i1> %326, i32 6
  %extract266 = extractelement <16 x i1> %326, i32 7
  %extract267 = extractelement <16 x i1> %326, i32 8
  %extract268 = extractelement <16 x i1> %326, i32 9
  %extract269 = extractelement <16 x i1> %326, i32 10
  %extract270 = extractelement <16 x i1> %326, i32 11
  %extract271 = extractelement <16 x i1> %326, i32 12
  %extract272 = extractelement <16 x i1> %326, i32 13
  %extract273 = extractelement <16 x i1> %326, i32 14
  %extract274 = extractelement <16 x i1> %326, i32 15
  %327 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %328 = mul nsw <16 x i32> %327, %vector118
  %329 = add nsw <16 x i32> %27, %328
  %330 = extractelement <16 x i32> %329, i32 1
  %331 = sext i32 %330 to i64
  %332 = getelementptr inbounds double addrspace(1)* %data, i64 %331
  %333 = extractelement <16 x i32> %329, i32 2
  %334 = sext i32 %333 to i64
  %335 = getelementptr inbounds double addrspace(1)* %data, i64 %334
  %336 = extractelement <16 x i32> %329, i32 3
  %337 = sext i32 %336 to i64
  %338 = getelementptr inbounds double addrspace(1)* %data, i64 %337
  %339 = extractelement <16 x i32> %329, i32 4
  %340 = sext i32 %339 to i64
  %341 = getelementptr inbounds double addrspace(1)* %data, i64 %340
  %342 = extractelement <16 x i32> %329, i32 5
  %343 = sext i32 %342 to i64
  %344 = getelementptr inbounds double addrspace(1)* %data, i64 %343
  %345 = extractelement <16 x i32> %329, i32 6
  %346 = sext i32 %345 to i64
  %347 = getelementptr inbounds double addrspace(1)* %data, i64 %346
  %348 = extractelement <16 x i32> %329, i32 7
  %349 = sext i32 %348 to i64
  %350 = getelementptr inbounds double addrspace(1)* %data, i64 %349
  %351 = extractelement <16 x i32> %329, i32 8
  %352 = sext i32 %351 to i64
  %353 = getelementptr inbounds double addrspace(1)* %data, i64 %352
  %354 = extractelement <16 x i32> %329, i32 9
  %355 = sext i32 %354 to i64
  %356 = getelementptr inbounds double addrspace(1)* %data, i64 %355
  %357 = extractelement <16 x i32> %329, i32 10
  %358 = sext i32 %357 to i64
  %359 = getelementptr inbounds double addrspace(1)* %data, i64 %358
  %360 = extractelement <16 x i32> %329, i32 11
  %361 = sext i32 %360 to i64
  %362 = getelementptr inbounds double addrspace(1)* %data, i64 %361
  %363 = extractelement <16 x i32> %329, i32 12
  %364 = sext i32 %363 to i64
  %365 = getelementptr inbounds double addrspace(1)* %data, i64 %364
  %366 = extractelement <16 x i32> %329, i32 13
  %367 = sext i32 %366 to i64
  %368 = getelementptr inbounds double addrspace(1)* %data, i64 %367
  %369 = extractelement <16 x i32> %329, i32 14
  %370 = sext i32 %369 to i64
  %371 = getelementptr inbounds double addrspace(1)* %data, i64 %370
  %372 = extractelement <16 x i32> %329, i32 15
  %373 = sext i32 %372 to i64
  %374 = getelementptr inbounds double addrspace(1)* %data, i64 %373
  br i1 %extract259, label %preload714, label %postload715

preload714:                                       ; preds = %postload713
  %375 = extractelement <16 x i32> %329, i32 0
  %376 = sext i32 %375 to i64
  %377 = getelementptr inbounds double addrspace(1)* %data, i64 %376
  %masked_load453 = load double addrspace(1)* %377, align 8
  br label %postload715

postload715:                                      ; preds = %preload714, %postload713
  %phi716 = phi double [ undef, %postload713 ], [ %masked_load453, %preload714 ]
  br i1 %extract260, label %preload734, label %postload735

preload734:                                       ; preds = %postload715
  %masked_load454 = load double addrspace(1)* %332, align 8
  br label %postload735

postload735:                                      ; preds = %preload734, %postload715
  %phi736 = phi double [ undef, %postload715 ], [ %masked_load454, %preload734 ]
  br i1 %extract261, label %preload754, label %postload755

preload754:                                       ; preds = %postload735
  %masked_load455 = load double addrspace(1)* %335, align 8
  br label %postload755

postload755:                                      ; preds = %preload754, %postload735
  %phi756 = phi double [ undef, %postload735 ], [ %masked_load455, %preload754 ]
  br i1 %extract262, label %preload774, label %postload775

preload774:                                       ; preds = %postload755
  %masked_load456 = load double addrspace(1)* %338, align 8
  br label %postload775

postload775:                                      ; preds = %preload774, %postload755
  %phi776 = phi double [ undef, %postload755 ], [ %masked_load456, %preload774 ]
  br i1 %extract263, label %preload794, label %postload795

preload794:                                       ; preds = %postload775
  %masked_load457 = load double addrspace(1)* %341, align 8
  br label %postload795

postload795:                                      ; preds = %preload794, %postload775
  %phi796 = phi double [ undef, %postload775 ], [ %masked_load457, %preload794 ]
  br i1 %extract264, label %preload814, label %postload815

preload814:                                       ; preds = %postload795
  %masked_load458 = load double addrspace(1)* %344, align 8
  br label %postload815

postload815:                                      ; preds = %preload814, %postload795
  %phi816 = phi double [ undef, %postload795 ], [ %masked_load458, %preload814 ]
  br i1 %extract265, label %preload834, label %postload835

preload834:                                       ; preds = %postload815
  %masked_load459 = load double addrspace(1)* %347, align 8
  br label %postload835

postload835:                                      ; preds = %preload834, %postload815
  %phi836 = phi double [ undef, %postload815 ], [ %masked_load459, %preload834 ]
  br i1 %extract266, label %preload854, label %postload855

preload854:                                       ; preds = %postload835
  %masked_load460 = load double addrspace(1)* %350, align 8
  br label %postload855

postload855:                                      ; preds = %preload854, %postload835
  %phi856 = phi double [ undef, %postload835 ], [ %masked_load460, %preload854 ]
  br i1 %extract267, label %preload874, label %postload875

preload874:                                       ; preds = %postload855
  %masked_load461 = load double addrspace(1)* %353, align 8
  br label %postload875

postload875:                                      ; preds = %preload874, %postload855
  %phi876 = phi double [ undef, %postload855 ], [ %masked_load461, %preload874 ]
  br i1 %extract268, label %preload894, label %postload895

preload894:                                       ; preds = %postload875
  %masked_load462 = load double addrspace(1)* %356, align 8
  br label %postload895

postload895:                                      ; preds = %preload894, %postload875
  %phi896 = phi double [ undef, %postload875 ], [ %masked_load462, %preload894 ]
  br i1 %extract269, label %preload914, label %postload915

preload914:                                       ; preds = %postload895
  %masked_load463 = load double addrspace(1)* %359, align 8
  br label %postload915

postload915:                                      ; preds = %preload914, %postload895
  %phi916 = phi double [ undef, %postload895 ], [ %masked_load463, %preload914 ]
  br i1 %extract270, label %preload934, label %postload935

preload934:                                       ; preds = %postload915
  %masked_load464 = load double addrspace(1)* %362, align 8
  br label %postload935

postload935:                                      ; preds = %preload934, %postload915
  %phi936 = phi double [ undef, %postload915 ], [ %masked_load464, %preload934 ]
  br i1 %extract271, label %preload954, label %postload955

preload954:                                       ; preds = %postload935
  %masked_load465 = load double addrspace(1)* %365, align 8
  br label %postload955

postload955:                                      ; preds = %preload954, %postload935
  %phi956 = phi double [ undef, %postload935 ], [ %masked_load465, %preload954 ]
  br i1 %extract272, label %preload974, label %postload975

preload974:                                       ; preds = %postload955
  %masked_load466 = load double addrspace(1)* %368, align 8
  br label %postload975

postload975:                                      ; preds = %preload974, %postload955
  %phi976 = phi double [ undef, %postload955 ], [ %masked_load466, %preload974 ]
  br i1 %extract273, label %preload994, label %postload995

preload994:                                       ; preds = %postload975
  %masked_load467 = load double addrspace(1)* %371, align 8
  br label %postload995

postload995:                                      ; preds = %preload994, %postload975
  %phi996 = phi double [ undef, %postload975 ], [ %masked_load467, %preload994 ]
  br i1 %extract274, label %preload1014, label %postload1015

preload1014:                                      ; preds = %postload995
  %masked_load468 = load double addrspace(1)* %374, align 8
  br label %postload1015

postload1015:                                     ; preds = %preload1014, %postload995
  %phi1016 = phi double [ undef, %postload995 ], [ %masked_load468, %preload1014 ]
  br i1 %extract259, label %preload717, label %postload718

preload717:                                       ; preds = %postload1015
  store double %phi716, double addrspace(3)* %2, align 8
  br label %postload718

postload718:                                      ; preds = %preload717, %postload1015
  br i1 %extract260, label %preload737, label %postload738

preload737:                                       ; preds = %postload718
  store double %phi736, double addrspace(3)* %2, align 8
  br label %postload738

postload738:                                      ; preds = %preload737, %postload718
  br i1 %extract261, label %preload757, label %postload758

preload757:                                       ; preds = %postload738
  store double %phi756, double addrspace(3)* %2, align 8
  br label %postload758

postload758:                                      ; preds = %preload757, %postload738
  br i1 %extract262, label %preload777, label %postload778

preload777:                                       ; preds = %postload758
  store double %phi776, double addrspace(3)* %2, align 8
  br label %postload778

postload778:                                      ; preds = %preload777, %postload758
  br i1 %extract263, label %preload797, label %postload798

preload797:                                       ; preds = %postload778
  store double %phi796, double addrspace(3)* %2, align 8
  br label %postload798

postload798:                                      ; preds = %preload797, %postload778
  br i1 %extract264, label %preload817, label %postload818

preload817:                                       ; preds = %postload798
  store double %phi816, double addrspace(3)* %2, align 8
  br label %postload818

postload818:                                      ; preds = %preload817, %postload798
  br i1 %extract265, label %preload837, label %postload838

preload837:                                       ; preds = %postload818
  store double %phi836, double addrspace(3)* %2, align 8
  br label %postload838

postload838:                                      ; preds = %preload837, %postload818
  br i1 %extract266, label %preload857, label %postload858

preload857:                                       ; preds = %postload838
  store double %phi856, double addrspace(3)* %2, align 8
  br label %postload858

postload858:                                      ; preds = %preload857, %postload838
  br i1 %extract267, label %preload877, label %postload878

preload877:                                       ; preds = %postload858
  store double %phi876, double addrspace(3)* %2, align 8
  br label %postload878

postload878:                                      ; preds = %preload877, %postload858
  br i1 %extract268, label %preload897, label %postload898

preload897:                                       ; preds = %postload878
  store double %phi896, double addrspace(3)* %2, align 8
  br label %postload898

postload898:                                      ; preds = %preload897, %postload878
  br i1 %extract269, label %preload917, label %postload918

preload917:                                       ; preds = %postload898
  store double %phi916, double addrspace(3)* %2, align 8
  br label %postload918

postload918:                                      ; preds = %preload917, %postload898
  br i1 %extract270, label %preload937, label %postload938

preload937:                                       ; preds = %postload918
  store double %phi936, double addrspace(3)* %2, align 8
  br label %postload938

postload938:                                      ; preds = %preload937, %postload918
  br i1 %extract271, label %preload957, label %postload958

preload957:                                       ; preds = %postload938
  store double %phi956, double addrspace(3)* %2, align 8
  br label %postload958

postload958:                                      ; preds = %preload957, %postload938
  br i1 %extract272, label %preload977, label %postload978

preload977:                                       ; preds = %postload958
  store double %phi976, double addrspace(3)* %2, align 8
  br label %postload978

postload978:                                      ; preds = %preload977, %postload958
  br i1 %extract273, label %preload997, label %postload998

preload997:                                       ; preds = %postload978
  store double %phi996, double addrspace(3)* %2, align 8
  br label %postload998

postload998:                                      ; preds = %preload997, %postload978
  br i1 %extract274, label %preload1017, label %postload1018

preload1017:                                      ; preds = %postload998
  store double %phi1016, double addrspace(3)* %2, align 8
  br label %postload1018

postload1018:                                     ; preds = %preload1017, %postload998
  %378 = add nsw i32 %25, 17
  %temp275 = insertelement <16 x i32> undef, i32 %378, i32 0
  %vector276 = shufflevector <16 x i32> %temp275, <16 x i32> undef, <16 x i32> zeroinitializer
  %379 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %380 = mul nsw <16 x i32> %379, %vector276
  %381 = add nsw <16 x i32> %27, %380
  %382 = extractelement <16 x i32> %381, i32 1
  %383 = sext i32 %382 to i64
  %384 = getelementptr inbounds double addrspace(1)* %data, i64 %383
  %385 = extractelement <16 x i32> %381, i32 2
  %386 = sext i32 %385 to i64
  %387 = getelementptr inbounds double addrspace(1)* %data, i64 %386
  %388 = extractelement <16 x i32> %381, i32 3
  %389 = sext i32 %388 to i64
  %390 = getelementptr inbounds double addrspace(1)* %data, i64 %389
  %391 = extractelement <16 x i32> %381, i32 4
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds double addrspace(1)* %data, i64 %392
  %394 = extractelement <16 x i32> %381, i32 5
  %395 = sext i32 %394 to i64
  %396 = getelementptr inbounds double addrspace(1)* %data, i64 %395
  %397 = extractelement <16 x i32> %381, i32 6
  %398 = sext i32 %397 to i64
  %399 = getelementptr inbounds double addrspace(1)* %data, i64 %398
  %400 = extractelement <16 x i32> %381, i32 7
  %401 = sext i32 %400 to i64
  %402 = getelementptr inbounds double addrspace(1)* %data, i64 %401
  %403 = extractelement <16 x i32> %381, i32 8
  %404 = sext i32 %403 to i64
  %405 = getelementptr inbounds double addrspace(1)* %data, i64 %404
  %406 = extractelement <16 x i32> %381, i32 9
  %407 = sext i32 %406 to i64
  %408 = getelementptr inbounds double addrspace(1)* %data, i64 %407
  %409 = extractelement <16 x i32> %381, i32 10
  %410 = sext i32 %409 to i64
  %411 = getelementptr inbounds double addrspace(1)* %data, i64 %410
  %412 = extractelement <16 x i32> %381, i32 11
  %413 = sext i32 %412 to i64
  %414 = getelementptr inbounds double addrspace(1)* %data, i64 %413
  %415 = extractelement <16 x i32> %381, i32 12
  %416 = sext i32 %415 to i64
  %417 = getelementptr inbounds double addrspace(1)* %data, i64 %416
  %418 = extractelement <16 x i32> %381, i32 13
  %419 = sext i32 %418 to i64
  %420 = getelementptr inbounds double addrspace(1)* %data, i64 %419
  %421 = extractelement <16 x i32> %381, i32 14
  %422 = sext i32 %421 to i64
  %423 = getelementptr inbounds double addrspace(1)* %data, i64 %422
  %424 = extractelement <16 x i32> %381, i32 15
  %425 = sext i32 %424 to i64
  %426 = getelementptr inbounds double addrspace(1)* %data, i64 %425
  br i1 %extract259, label %preload719, label %postload720

preload719:                                       ; preds = %postload1018
  %427 = extractelement <16 x i32> %381, i32 0
  %428 = sext i32 %427 to i64
  %429 = getelementptr inbounds double addrspace(1)* %data, i64 %428
  %masked_load469 = load double addrspace(1)* %429, align 8
  br label %postload720

postload720:                                      ; preds = %preload719, %postload1018
  %phi721 = phi double [ undef, %postload1018 ], [ %masked_load469, %preload719 ]
  br i1 %extract260, label %preload739, label %postload740

preload739:                                       ; preds = %postload720
  %masked_load470 = load double addrspace(1)* %384, align 8
  br label %postload740

postload740:                                      ; preds = %preload739, %postload720
  %phi741 = phi double [ undef, %postload720 ], [ %masked_load470, %preload739 ]
  br i1 %extract261, label %preload759, label %postload760

preload759:                                       ; preds = %postload740
  %masked_load471 = load double addrspace(1)* %387, align 8
  br label %postload760

postload760:                                      ; preds = %preload759, %postload740
  %phi761 = phi double [ undef, %postload740 ], [ %masked_load471, %preload759 ]
  br i1 %extract262, label %preload779, label %postload780

preload779:                                       ; preds = %postload760
  %masked_load472 = load double addrspace(1)* %390, align 8
  br label %postload780

postload780:                                      ; preds = %preload779, %postload760
  %phi781 = phi double [ undef, %postload760 ], [ %masked_load472, %preload779 ]
  br i1 %extract263, label %preload799, label %postload800

preload799:                                       ; preds = %postload780
  %masked_load473 = load double addrspace(1)* %393, align 8
  br label %postload800

postload800:                                      ; preds = %preload799, %postload780
  %phi801 = phi double [ undef, %postload780 ], [ %masked_load473, %preload799 ]
  br i1 %extract264, label %preload819, label %postload820

preload819:                                       ; preds = %postload800
  %masked_load474 = load double addrspace(1)* %396, align 8
  br label %postload820

postload820:                                      ; preds = %preload819, %postload800
  %phi821 = phi double [ undef, %postload800 ], [ %masked_load474, %preload819 ]
  br i1 %extract265, label %preload839, label %postload840

preload839:                                       ; preds = %postload820
  %masked_load475 = load double addrspace(1)* %399, align 8
  br label %postload840

postload840:                                      ; preds = %preload839, %postload820
  %phi841 = phi double [ undef, %postload820 ], [ %masked_load475, %preload839 ]
  br i1 %extract266, label %preload859, label %postload860

preload859:                                       ; preds = %postload840
  %masked_load476 = load double addrspace(1)* %402, align 8
  br label %postload860

postload860:                                      ; preds = %preload859, %postload840
  %phi861 = phi double [ undef, %postload840 ], [ %masked_load476, %preload859 ]
  br i1 %extract267, label %preload879, label %postload880

preload879:                                       ; preds = %postload860
  %masked_load477 = load double addrspace(1)* %405, align 8
  br label %postload880

postload880:                                      ; preds = %preload879, %postload860
  %phi881 = phi double [ undef, %postload860 ], [ %masked_load477, %preload879 ]
  br i1 %extract268, label %preload899, label %postload900

preload899:                                       ; preds = %postload880
  %masked_load478 = load double addrspace(1)* %408, align 8
  br label %postload900

postload900:                                      ; preds = %preload899, %postload880
  %phi901 = phi double [ undef, %postload880 ], [ %masked_load478, %preload899 ]
  br i1 %extract269, label %preload919, label %postload920

preload919:                                       ; preds = %postload900
  %masked_load479 = load double addrspace(1)* %411, align 8
  br label %postload920

postload920:                                      ; preds = %preload919, %postload900
  %phi921 = phi double [ undef, %postload900 ], [ %masked_load479, %preload919 ]
  br i1 %extract270, label %preload939, label %postload940

preload939:                                       ; preds = %postload920
  %masked_load480 = load double addrspace(1)* %414, align 8
  br label %postload940

postload940:                                      ; preds = %preload939, %postload920
  %phi941 = phi double [ undef, %postload920 ], [ %masked_load480, %preload939 ]
  br i1 %extract271, label %preload959, label %postload960

preload959:                                       ; preds = %postload940
  %masked_load481 = load double addrspace(1)* %417, align 8
  br label %postload960

postload960:                                      ; preds = %preload959, %postload940
  %phi961 = phi double [ undef, %postload940 ], [ %masked_load481, %preload959 ]
  br i1 %extract272, label %preload979, label %postload980

preload979:                                       ; preds = %postload960
  %masked_load482 = load double addrspace(1)* %420, align 8
  br label %postload980

postload980:                                      ; preds = %preload979, %postload960
  %phi981 = phi double [ undef, %postload960 ], [ %masked_load482, %preload979 ]
  br i1 %extract273, label %preload999, label %postload1000

preload999:                                       ; preds = %postload980
  %masked_load483 = load double addrspace(1)* %423, align 8
  br label %postload1000

postload1000:                                     ; preds = %preload999, %postload980
  %phi1001 = phi double [ undef, %postload980 ], [ %masked_load483, %preload999 ]
  br i1 %extract274, label %preload1019, label %postload1020

preload1019:                                      ; preds = %postload1000
  %masked_load484 = load double addrspace(1)* %426, align 8
  br label %postload1020

postload1020:                                     ; preds = %preload1019, %postload1000
  %phi1021 = phi double [ undef, %postload1000 ], [ %masked_load484, %preload1019 ]
  br i1 %extract259, label %preload722, label %postload723

preload722:                                       ; preds = %postload1020
  store double %phi721, double addrspace(3)* %4, align 8
  br label %postload723

postload723:                                      ; preds = %preload722, %postload1020
  br i1 %extract260, label %preload742, label %postload743

preload742:                                       ; preds = %postload723
  store double %phi741, double addrspace(3)* %4, align 8
  br label %postload743

postload743:                                      ; preds = %preload742, %postload723
  br i1 %extract261, label %preload762, label %postload763

preload762:                                       ; preds = %postload743
  store double %phi761, double addrspace(3)* %4, align 8
  br label %postload763

postload763:                                      ; preds = %preload762, %postload743
  br i1 %extract262, label %preload782, label %postload783

preload782:                                       ; preds = %postload763
  store double %phi781, double addrspace(3)* %4, align 8
  br label %postload783

postload783:                                      ; preds = %preload782, %postload763
  br i1 %extract263, label %preload802, label %postload803

preload802:                                       ; preds = %postload783
  store double %phi801, double addrspace(3)* %4, align 8
  br label %postload803

postload803:                                      ; preds = %preload802, %postload783
  br i1 %extract264, label %preload822, label %postload823

preload822:                                       ; preds = %postload803
  store double %phi821, double addrspace(3)* %4, align 8
  br label %postload823

postload823:                                      ; preds = %preload822, %postload803
  br i1 %extract265, label %preload842, label %postload843

preload842:                                       ; preds = %postload823
  store double %phi841, double addrspace(3)* %4, align 8
  br label %postload843

postload843:                                      ; preds = %preload842, %postload823
  br i1 %extract266, label %preload862, label %postload863

preload862:                                       ; preds = %postload843
  store double %phi861, double addrspace(3)* %4, align 8
  br label %postload863

postload863:                                      ; preds = %preload862, %postload843
  br i1 %extract267, label %preload882, label %postload883

preload882:                                       ; preds = %postload863
  store double %phi881, double addrspace(3)* %4, align 8
  br label %postload883

postload883:                                      ; preds = %preload882, %postload863
  br i1 %extract268, label %preload902, label %postload903

preload902:                                       ; preds = %postload883
  store double %phi901, double addrspace(3)* %4, align 8
  br label %postload903

postload903:                                      ; preds = %preload902, %postload883
  br i1 %extract269, label %preload922, label %postload923

preload922:                                       ; preds = %postload903
  store double %phi921, double addrspace(3)* %4, align 8
  br label %postload923

postload923:                                      ; preds = %preload922, %postload903
  br i1 %extract270, label %preload942, label %postload943

preload942:                                       ; preds = %postload923
  store double %phi941, double addrspace(3)* %4, align 8
  br label %postload943

postload943:                                      ; preds = %preload942, %postload923
  br i1 %extract271, label %preload962, label %postload963

preload962:                                       ; preds = %postload943
  store double %phi961, double addrspace(3)* %4, align 8
  br label %postload963

postload963:                                      ; preds = %preload962, %postload943
  br i1 %extract272, label %preload982, label %postload983

preload982:                                       ; preds = %postload963
  store double %phi981, double addrspace(3)* %4, align 8
  br label %postload983

postload983:                                      ; preds = %preload982, %postload963
  br i1 %extract273, label %preload1002, label %postload1003

preload1002:                                      ; preds = %postload983
  store double %phi1001, double addrspace(3)* %4, align 8
  br label %postload1003

postload1003:                                     ; preds = %preload1002, %postload983
  br i1 %extract274, label %preload1022, label %postload1023

preload1022:                                      ; preds = %postload1003
  store double %phi1021, double addrspace(3)* %4, align 8
  br label %postload1023

postload1023:                                     ; preds = %preload1022, %postload1003
  %430 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %431 = mul nsw <16 x i32> %430, %vector118
  %432 = add nsw <16 x i32> %27, <i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17>
  %433 = add nsw <16 x i32> %432, %431
  %434 = extractelement <16 x i32> %433, i32 1
  %435 = sext i32 %434 to i64
  %436 = getelementptr inbounds double addrspace(1)* %data, i64 %435
  %437 = extractelement <16 x i32> %433, i32 2
  %438 = sext i32 %437 to i64
  %439 = getelementptr inbounds double addrspace(1)* %data, i64 %438
  %440 = extractelement <16 x i32> %433, i32 3
  %441 = sext i32 %440 to i64
  %442 = getelementptr inbounds double addrspace(1)* %data, i64 %441
  %443 = extractelement <16 x i32> %433, i32 4
  %444 = sext i32 %443 to i64
  %445 = getelementptr inbounds double addrspace(1)* %data, i64 %444
  %446 = extractelement <16 x i32> %433, i32 5
  %447 = sext i32 %446 to i64
  %448 = getelementptr inbounds double addrspace(1)* %data, i64 %447
  %449 = extractelement <16 x i32> %433, i32 6
  %450 = sext i32 %449 to i64
  %451 = getelementptr inbounds double addrspace(1)* %data, i64 %450
  %452 = extractelement <16 x i32> %433, i32 7
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds double addrspace(1)* %data, i64 %453
  %455 = extractelement <16 x i32> %433, i32 8
  %456 = sext i32 %455 to i64
  %457 = getelementptr inbounds double addrspace(1)* %data, i64 %456
  %458 = extractelement <16 x i32> %433, i32 9
  %459 = sext i32 %458 to i64
  %460 = getelementptr inbounds double addrspace(1)* %data, i64 %459
  %461 = extractelement <16 x i32> %433, i32 10
  %462 = sext i32 %461 to i64
  %463 = getelementptr inbounds double addrspace(1)* %data, i64 %462
  %464 = extractelement <16 x i32> %433, i32 11
  %465 = sext i32 %464 to i64
  %466 = getelementptr inbounds double addrspace(1)* %data, i64 %465
  %467 = extractelement <16 x i32> %433, i32 12
  %468 = sext i32 %467 to i64
  %469 = getelementptr inbounds double addrspace(1)* %data, i64 %468
  %470 = extractelement <16 x i32> %433, i32 13
  %471 = sext i32 %470 to i64
  %472 = getelementptr inbounds double addrspace(1)* %data, i64 %471
  %473 = extractelement <16 x i32> %433, i32 14
  %474 = sext i32 %473 to i64
  %475 = getelementptr inbounds double addrspace(1)* %data, i64 %474
  %476 = extractelement <16 x i32> %433, i32 15
  %477 = sext i32 %476 to i64
  %478 = getelementptr inbounds double addrspace(1)* %data, i64 %477
  br i1 %extract259, label %preload724, label %postload725

preload724:                                       ; preds = %postload1023
  %479 = extractelement <16 x i32> %433, i32 0
  %480 = sext i32 %479 to i64
  %481 = getelementptr inbounds double addrspace(1)* %data, i64 %480
  %masked_load485 = load double addrspace(1)* %481, align 8
  br label %postload725

postload725:                                      ; preds = %preload724, %postload1023
  %phi726 = phi double [ undef, %postload1023 ], [ %masked_load485, %preload724 ]
  br i1 %extract260, label %preload744, label %postload745

preload744:                                       ; preds = %postload725
  %masked_load486 = load double addrspace(1)* %436, align 8
  br label %postload745

postload745:                                      ; preds = %preload744, %postload725
  %phi746 = phi double [ undef, %postload725 ], [ %masked_load486, %preload744 ]
  br i1 %extract261, label %preload764, label %postload765

preload764:                                       ; preds = %postload745
  %masked_load487 = load double addrspace(1)* %439, align 8
  br label %postload765

postload765:                                      ; preds = %preload764, %postload745
  %phi766 = phi double [ undef, %postload745 ], [ %masked_load487, %preload764 ]
  br i1 %extract262, label %preload784, label %postload785

preload784:                                       ; preds = %postload765
  %masked_load488 = load double addrspace(1)* %442, align 8
  br label %postload785

postload785:                                      ; preds = %preload784, %postload765
  %phi786 = phi double [ undef, %postload765 ], [ %masked_load488, %preload784 ]
  br i1 %extract263, label %preload804, label %postload805

preload804:                                       ; preds = %postload785
  %masked_load489 = load double addrspace(1)* %445, align 8
  br label %postload805

postload805:                                      ; preds = %preload804, %postload785
  %phi806 = phi double [ undef, %postload785 ], [ %masked_load489, %preload804 ]
  br i1 %extract264, label %preload824, label %postload825

preload824:                                       ; preds = %postload805
  %masked_load490 = load double addrspace(1)* %448, align 8
  br label %postload825

postload825:                                      ; preds = %preload824, %postload805
  %phi826 = phi double [ undef, %postload805 ], [ %masked_load490, %preload824 ]
  br i1 %extract265, label %preload844, label %postload845

preload844:                                       ; preds = %postload825
  %masked_load491 = load double addrspace(1)* %451, align 8
  br label %postload845

postload845:                                      ; preds = %preload844, %postload825
  %phi846 = phi double [ undef, %postload825 ], [ %masked_load491, %preload844 ]
  br i1 %extract266, label %preload864, label %postload865

preload864:                                       ; preds = %postload845
  %masked_load492 = load double addrspace(1)* %454, align 8
  br label %postload865

postload865:                                      ; preds = %preload864, %postload845
  %phi866 = phi double [ undef, %postload845 ], [ %masked_load492, %preload864 ]
  br i1 %extract267, label %preload884, label %postload885

preload884:                                       ; preds = %postload865
  %masked_load493 = load double addrspace(1)* %457, align 8
  br label %postload885

postload885:                                      ; preds = %preload884, %postload865
  %phi886 = phi double [ undef, %postload865 ], [ %masked_load493, %preload884 ]
  br i1 %extract268, label %preload904, label %postload905

preload904:                                       ; preds = %postload885
  %masked_load494 = load double addrspace(1)* %460, align 8
  br label %postload905

postload905:                                      ; preds = %preload904, %postload885
  %phi906 = phi double [ undef, %postload885 ], [ %masked_load494, %preload904 ]
  br i1 %extract269, label %preload924, label %postload925

preload924:                                       ; preds = %postload905
  %masked_load495 = load double addrspace(1)* %463, align 8
  br label %postload925

postload925:                                      ; preds = %preload924, %postload905
  %phi926 = phi double [ undef, %postload905 ], [ %masked_load495, %preload924 ]
  br i1 %extract270, label %preload944, label %postload945

preload944:                                       ; preds = %postload925
  %masked_load496 = load double addrspace(1)* %466, align 8
  br label %postload945

postload945:                                      ; preds = %preload944, %postload925
  %phi946 = phi double [ undef, %postload925 ], [ %masked_load496, %preload944 ]
  br i1 %extract271, label %preload964, label %postload965

preload964:                                       ; preds = %postload945
  %masked_load497 = load double addrspace(1)* %469, align 8
  br label %postload965

postload965:                                      ; preds = %preload964, %postload945
  %phi966 = phi double [ undef, %postload945 ], [ %masked_load497, %preload964 ]
  br i1 %extract272, label %preload984, label %postload985

preload984:                                       ; preds = %postload965
  %masked_load498 = load double addrspace(1)* %472, align 8
  br label %postload985

postload985:                                      ; preds = %preload984, %postload965
  %phi986 = phi double [ undef, %postload965 ], [ %masked_load498, %preload984 ]
  br i1 %extract273, label %preload1004, label %postload1005

preload1004:                                      ; preds = %postload985
  %masked_load499 = load double addrspace(1)* %475, align 8
  br label %postload1005

postload1005:                                     ; preds = %preload1004, %postload985
  %phi1006 = phi double [ undef, %postload985 ], [ %masked_load499, %preload1004 ]
  br i1 %extract274, label %preload1024, label %postload1025

preload1024:                                      ; preds = %postload1005
  %masked_load500 = load double addrspace(1)* %478, align 8
  br label %postload1025

postload1025:                                     ; preds = %preload1024, %postload1005
  %phi1026 = phi double [ undef, %postload1005 ], [ %masked_load500, %preload1024 ]
  br i1 %extract259, label %preload727, label %postload728

preload727:                                       ; preds = %postload1025
  store double %phi726, double addrspace(3)* %6, align 8
  br label %postload728

postload728:                                      ; preds = %preload727, %postload1025
  br i1 %extract260, label %preload747, label %postload748

preload747:                                       ; preds = %postload728
  store double %phi746, double addrspace(3)* %6, align 8
  br label %postload748

postload748:                                      ; preds = %preload747, %postload728
  br i1 %extract261, label %preload767, label %postload768

preload767:                                       ; preds = %postload748
  store double %phi766, double addrspace(3)* %6, align 8
  br label %postload768

postload768:                                      ; preds = %preload767, %postload748
  br i1 %extract262, label %preload787, label %postload788

preload787:                                       ; preds = %postload768
  store double %phi786, double addrspace(3)* %6, align 8
  br label %postload788

postload788:                                      ; preds = %preload787, %postload768
  br i1 %extract263, label %preload807, label %postload808

preload807:                                       ; preds = %postload788
  store double %phi806, double addrspace(3)* %6, align 8
  br label %postload808

postload808:                                      ; preds = %preload807, %postload788
  br i1 %extract264, label %preload827, label %postload828

preload827:                                       ; preds = %postload808
  store double %phi826, double addrspace(3)* %6, align 8
  br label %postload828

postload828:                                      ; preds = %preload827, %postload808
  br i1 %extract265, label %preload847, label %postload848

preload847:                                       ; preds = %postload828
  store double %phi846, double addrspace(3)* %6, align 8
  br label %postload848

postload848:                                      ; preds = %preload847, %postload828
  br i1 %extract266, label %preload867, label %postload868

preload867:                                       ; preds = %postload848
  store double %phi866, double addrspace(3)* %6, align 8
  br label %postload868

postload868:                                      ; preds = %preload867, %postload848
  br i1 %extract267, label %preload887, label %postload888

preload887:                                       ; preds = %postload868
  store double %phi886, double addrspace(3)* %6, align 8
  br label %postload888

postload888:                                      ; preds = %preload887, %postload868
  br i1 %extract268, label %preload907, label %postload908

preload907:                                       ; preds = %postload888
  store double %phi906, double addrspace(3)* %6, align 8
  br label %postload908

postload908:                                      ; preds = %preload907, %postload888
  br i1 %extract269, label %preload927, label %postload928

preload927:                                       ; preds = %postload908
  store double %phi926, double addrspace(3)* %6, align 8
  br label %postload928

postload928:                                      ; preds = %preload927, %postload908
  br i1 %extract270, label %preload947, label %postload948

preload947:                                       ; preds = %postload928
  store double %phi946, double addrspace(3)* %6, align 8
  br label %postload948

postload948:                                      ; preds = %preload947, %postload928
  br i1 %extract271, label %preload967, label %postload968

preload967:                                       ; preds = %postload948
  store double %phi966, double addrspace(3)* %6, align 8
  br label %postload968

postload968:                                      ; preds = %preload967, %postload948
  br i1 %extract272, label %preload987, label %postload988

preload987:                                       ; preds = %postload968
  store double %phi986, double addrspace(3)* %6, align 8
  br label %postload988

postload988:                                      ; preds = %preload987, %postload968
  br i1 %extract273, label %preload1007, label %postload1008

preload1007:                                      ; preds = %postload988
  store double %phi1006, double addrspace(3)* %6, align 8
  br label %postload1008

postload1008:                                     ; preds = %preload1007, %postload988
  br i1 %extract274, label %preload1027, label %postload1028

preload1027:                                      ; preds = %postload1008
  store double %phi1026, double addrspace(3)* %6, align 8
  br label %postload1028

postload1028:                                     ; preds = %preload1027, %postload1008
  %482 = add nsw i32 %25, 17
  %temp309 = insertelement <16 x i32> undef, i32 %482, i32 0
  %vector310 = shufflevector <16 x i32> %temp309, <16 x i32> undef, <16 x i32> zeroinitializer
  %483 = or <16 x i32> %28, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %484 = mul nsw <16 x i32> %483, %vector310
  %485 = add nsw <16 x i32> %27, <i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17>
  %486 = add nsw <16 x i32> %485, %484
  %487 = extractelement <16 x i32> %486, i32 1
  %488 = sext i32 %487 to i64
  %489 = getelementptr inbounds double addrspace(1)* %data, i64 %488
  %490 = extractelement <16 x i32> %486, i32 2
  %491 = sext i32 %490 to i64
  %492 = getelementptr inbounds double addrspace(1)* %data, i64 %491
  %493 = extractelement <16 x i32> %486, i32 3
  %494 = sext i32 %493 to i64
  %495 = getelementptr inbounds double addrspace(1)* %data, i64 %494
  %496 = extractelement <16 x i32> %486, i32 4
  %497 = sext i32 %496 to i64
  %498 = getelementptr inbounds double addrspace(1)* %data, i64 %497
  %499 = extractelement <16 x i32> %486, i32 5
  %500 = sext i32 %499 to i64
  %501 = getelementptr inbounds double addrspace(1)* %data, i64 %500
  %502 = extractelement <16 x i32> %486, i32 6
  %503 = sext i32 %502 to i64
  %504 = getelementptr inbounds double addrspace(1)* %data, i64 %503
  %505 = extractelement <16 x i32> %486, i32 7
  %506 = sext i32 %505 to i64
  %507 = getelementptr inbounds double addrspace(1)* %data, i64 %506
  %508 = extractelement <16 x i32> %486, i32 8
  %509 = sext i32 %508 to i64
  %510 = getelementptr inbounds double addrspace(1)* %data, i64 %509
  %511 = extractelement <16 x i32> %486, i32 9
  %512 = sext i32 %511 to i64
  %513 = getelementptr inbounds double addrspace(1)* %data, i64 %512
  %514 = extractelement <16 x i32> %486, i32 10
  %515 = sext i32 %514 to i64
  %516 = getelementptr inbounds double addrspace(1)* %data, i64 %515
  %517 = extractelement <16 x i32> %486, i32 11
  %518 = sext i32 %517 to i64
  %519 = getelementptr inbounds double addrspace(1)* %data, i64 %518
  %520 = extractelement <16 x i32> %486, i32 12
  %521 = sext i32 %520 to i64
  %522 = getelementptr inbounds double addrspace(1)* %data, i64 %521
  %523 = extractelement <16 x i32> %486, i32 13
  %524 = sext i32 %523 to i64
  %525 = getelementptr inbounds double addrspace(1)* %data, i64 %524
  %526 = extractelement <16 x i32> %486, i32 14
  %527 = sext i32 %526 to i64
  %528 = getelementptr inbounds double addrspace(1)* %data, i64 %527
  %529 = extractelement <16 x i32> %486, i32 15
  %530 = sext i32 %529 to i64
  %531 = getelementptr inbounds double addrspace(1)* %data, i64 %530
  br i1 %extract259, label %preload729, label %postload730

preload729:                                       ; preds = %postload1028
  %532 = extractelement <16 x i32> %486, i32 0
  %533 = sext i32 %532 to i64
  %534 = getelementptr inbounds double addrspace(1)* %data, i64 %533
  %masked_load501 = load double addrspace(1)* %534, align 8
  br label %postload730

postload730:                                      ; preds = %preload729, %postload1028
  %phi731 = phi double [ undef, %postload1028 ], [ %masked_load501, %preload729 ]
  br i1 %extract260, label %preload749, label %postload750

preload749:                                       ; preds = %postload730
  %masked_load502 = load double addrspace(1)* %489, align 8
  br label %postload750

postload750:                                      ; preds = %preload749, %postload730
  %phi751 = phi double [ undef, %postload730 ], [ %masked_load502, %preload749 ]
  br i1 %extract261, label %preload769, label %postload770

preload769:                                       ; preds = %postload750
  %masked_load503 = load double addrspace(1)* %492, align 8
  br label %postload770

postload770:                                      ; preds = %preload769, %postload750
  %phi771 = phi double [ undef, %postload750 ], [ %masked_load503, %preload769 ]
  br i1 %extract262, label %preload789, label %postload790

preload789:                                       ; preds = %postload770
  %masked_load504 = load double addrspace(1)* %495, align 8
  br label %postload790

postload790:                                      ; preds = %preload789, %postload770
  %phi791 = phi double [ undef, %postload770 ], [ %masked_load504, %preload789 ]
  br i1 %extract263, label %preload809, label %postload810

preload809:                                       ; preds = %postload790
  %masked_load505 = load double addrspace(1)* %498, align 8
  br label %postload810

postload810:                                      ; preds = %preload809, %postload790
  %phi811 = phi double [ undef, %postload790 ], [ %masked_load505, %preload809 ]
  br i1 %extract264, label %preload829, label %postload830

preload829:                                       ; preds = %postload810
  %masked_load506 = load double addrspace(1)* %501, align 8
  br label %postload830

postload830:                                      ; preds = %preload829, %postload810
  %phi831 = phi double [ undef, %postload810 ], [ %masked_load506, %preload829 ]
  br i1 %extract265, label %preload849, label %postload850

preload849:                                       ; preds = %postload830
  %masked_load507 = load double addrspace(1)* %504, align 8
  br label %postload850

postload850:                                      ; preds = %preload849, %postload830
  %phi851 = phi double [ undef, %postload830 ], [ %masked_load507, %preload849 ]
  br i1 %extract266, label %preload869, label %postload870

preload869:                                       ; preds = %postload850
  %masked_load508 = load double addrspace(1)* %507, align 8
  br label %postload870

postload870:                                      ; preds = %preload869, %postload850
  %phi871 = phi double [ undef, %postload850 ], [ %masked_load508, %preload869 ]
  br i1 %extract267, label %preload889, label %postload890

preload889:                                       ; preds = %postload870
  %masked_load509 = load double addrspace(1)* %510, align 8
  br label %postload890

postload890:                                      ; preds = %preload889, %postload870
  %phi891 = phi double [ undef, %postload870 ], [ %masked_load509, %preload889 ]
  br i1 %extract268, label %preload909, label %postload910

preload909:                                       ; preds = %postload890
  %masked_load510 = load double addrspace(1)* %513, align 8
  br label %postload910

postload910:                                      ; preds = %preload909, %postload890
  %phi911 = phi double [ undef, %postload890 ], [ %masked_load510, %preload909 ]
  br i1 %extract269, label %preload929, label %postload930

preload929:                                       ; preds = %postload910
  %masked_load511 = load double addrspace(1)* %516, align 8
  br label %postload930

postload930:                                      ; preds = %preload929, %postload910
  %phi931 = phi double [ undef, %postload910 ], [ %masked_load511, %preload929 ]
  br i1 %extract270, label %preload949, label %postload950

preload949:                                       ; preds = %postload930
  %masked_load512 = load double addrspace(1)* %519, align 8
  br label %postload950

postload950:                                      ; preds = %preload949, %postload930
  %phi951 = phi double [ undef, %postload930 ], [ %masked_load512, %preload949 ]
  br i1 %extract271, label %preload969, label %postload970

preload969:                                       ; preds = %postload950
  %masked_load513 = load double addrspace(1)* %522, align 8
  br label %postload970

postload970:                                      ; preds = %preload969, %postload950
  %phi971 = phi double [ undef, %postload950 ], [ %masked_load513, %preload969 ]
  br i1 %extract272, label %preload989, label %postload990

preload989:                                       ; preds = %postload970
  %masked_load514 = load double addrspace(1)* %525, align 8
  br label %postload990

postload990:                                      ; preds = %preload989, %postload970
  %phi991 = phi double [ undef, %postload970 ], [ %masked_load514, %preload989 ]
  br i1 %extract273, label %preload1009, label %postload1010

preload1009:                                      ; preds = %postload990
  %masked_load515 = load double addrspace(1)* %528, align 8
  br label %postload1010

postload1010:                                     ; preds = %preload1009, %postload990
  %phi1011 = phi double [ undef, %postload990 ], [ %masked_load515, %preload1009 ]
  br i1 %extract274, label %preload1029, label %postload1030

preload1029:                                      ; preds = %postload1010
  %masked_load516 = load double addrspace(1)* %531, align 8
  br label %postload1030

postload1030:                                     ; preds = %preload1029, %postload1010
  %phi1031 = phi double [ undef, %postload1010 ], [ %masked_load516, %preload1029 ]
  br i1 %extract259, label %preload732, label %postload733

preload732:                                       ; preds = %postload1030
  store double %phi731, double addrspace(3)* %8, align 8
  br label %postload733

postload733:                                      ; preds = %preload732, %postload1030
  br i1 %extract260, label %preload752, label %postload753

preload752:                                       ; preds = %postload733
  store double %phi751, double addrspace(3)* %8, align 8
  br label %postload753

postload753:                                      ; preds = %preload752, %postload733
  br i1 %extract261, label %preload772, label %postload773

preload772:                                       ; preds = %postload753
  store double %phi771, double addrspace(3)* %8, align 8
  br label %postload773

postload773:                                      ; preds = %preload772, %postload753
  br i1 %extract262, label %preload792, label %postload793

preload792:                                       ; preds = %postload773
  store double %phi791, double addrspace(3)* %8, align 8
  br label %postload793

postload793:                                      ; preds = %preload792, %postload773
  br i1 %extract263, label %preload812, label %postload813

preload812:                                       ; preds = %postload793
  store double %phi811, double addrspace(3)* %8, align 8
  br label %postload813

postload813:                                      ; preds = %preload812, %postload793
  br i1 %extract264, label %preload832, label %postload833

preload832:                                       ; preds = %postload813
  store double %phi831, double addrspace(3)* %8, align 8
  br label %postload833

postload833:                                      ; preds = %preload832, %postload813
  br i1 %extract265, label %preload852, label %postload853

preload852:                                       ; preds = %postload833
  store double %phi851, double addrspace(3)* %8, align 8
  br label %postload853

postload853:                                      ; preds = %preload852, %postload833
  br i1 %extract266, label %preload872, label %postload873

preload872:                                       ; preds = %postload853
  store double %phi871, double addrspace(3)* %8, align 8
  br label %postload873

postload873:                                      ; preds = %preload872, %postload853
  br i1 %extract267, label %preload892, label %postload893

preload892:                                       ; preds = %postload873
  store double %phi891, double addrspace(3)* %8, align 8
  br label %postload893

postload893:                                      ; preds = %preload892, %postload873
  br i1 %extract268, label %preload912, label %postload913

preload912:                                       ; preds = %postload893
  store double %phi911, double addrspace(3)* %8, align 8
  br label %postload913

postload913:                                      ; preds = %preload912, %postload893
  br i1 %extract269, label %preload932, label %postload933

preload932:                                       ; preds = %postload913
  store double %phi931, double addrspace(3)* %8, align 8
  br label %postload933

postload933:                                      ; preds = %preload932, %postload913
  br i1 %extract270, label %preload952, label %postload953

preload952:                                       ; preds = %postload933
  store double %phi951, double addrspace(3)* %8, align 8
  br label %postload953

postload953:                                      ; preds = %preload952, %postload933
  br i1 %extract271, label %preload972, label %postload973

preload972:                                       ; preds = %postload953
  store double %phi971, double addrspace(3)* %8, align 8
  br label %postload973

postload973:                                      ; preds = %preload972, %postload953
  br i1 %extract272, label %preload992, label %postload993

preload992:                                       ; preds = %postload973
  store double %phi991, double addrspace(3)* %8, align 8
  br label %postload993

postload993:                                      ; preds = %preload992, %postload973
  br i1 %extract273, label %preload1012, label %postload1013

preload1012:                                      ; preds = %postload993
  store double %phi1011, double addrspace(3)* %8, align 8
  br label %postload1013

postload1013:                                     ; preds = %preload1012, %postload993
  br i1 %extract274, label %preload1032, label %postload1033

preload1032:                                      ; preds = %postload1013
  store double %phi1031, double addrspace(3)* %8, align 8
  br label %postload1033

postload1033:                                     ; preds = %preload1032, %postload1013
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %postload1033
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 320
  br label %SyncBB1220

elseBB:                                           ; preds = %postload1033
  %temp372 = insertelement <16 x double> undef, double %wDiagonal, i32 0
  %vector373 = shufflevector <16 x double> %temp372, <16 x double> undef, <16 x i32> zeroinitializer
  %temp370 = insertelement <16 x double> undef, double %wCardinal, i32 0
  %vector371 = shufflevector <16 x double> %temp370, <16 x double> undef, <16 x i32> zeroinitializer
  %temp368 = insertelement <16 x double> undef, double %wCenter, i32 0
  %vector369 = shufflevector <16 x double> %temp368, <16 x double> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB1222, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride1228", %thenBB1222 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++1226", %thenBB1222 ]
  %"&(pSB[currWI].offset)1209" = add nuw i64 %CurrSBIndex..1, 264
  %"&pSB[currWI].offset1210" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1209"
  %CastToValueType1211 = bitcast i8* %"&pSB[currWI].offset1210" to double addrspace(3)**
  %loadedValue1212 = load double addrspace(3)** %CastToValueType1211, align 8
  %ptrTypeCast327 = bitcast double addrspace(3)* %loadedValue1212 to <16 x double> addrspace(3)*
  %535 = load <16 x double> addrspace(3)* %ptrTypeCast327, align 8
  %"&(pSB[currWI].offset)10392" = or i64 %CurrSBIndex..1, 40
  %"&pSB[currWI].offset1040" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10392"
  %CastToValueType1041 = bitcast i8* %"&pSB[currWI].offset1040" to i32*
  %loadedValue1042 = load i32* %CastToValueType1041, align 4
  %536 = sext i32 %loadedValue1042 to i64
  %"&(pSB[currWI].offset)1166" = add nuw i64 %CurrSBIndex..1, 192
  %"&pSB[currWI].offset1167" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1166"
  %CastToValueType1168 = bitcast i8* %"&pSB[currWI].offset1167" to <16 x i32>*
  %loadedValue1169 = load <16 x i32>* %CastToValueType1168, align 64
  %537 = extractelement <16 x i32> %loadedValue1169, i32 0
  %538 = sext i32 %537 to i64
  %539 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %536, i64 %538
  %ptrTypeCast328 = bitcast double addrspace(3)* %539 to <16 x double> addrspace(3)*
  %540 = load <16 x double> addrspace(3)* %ptrTypeCast328, align 8
  %"&(pSB[currWI].offset)10353" = or i64 %CurrSBIndex..1, 40
  %"&pSB[currWI].offset1036" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10353"
  %CastToValueType1037 = bitcast i8* %"&pSB[currWI].offset1036" to i32*
  %loadedValue = load i32* %CastToValueType1037, align 4
  %541 = add nsw i32 %loadedValue, 2
  %542 = sext i32 %541 to i64
  %"&(pSB[currWI].offset)1161" = add nuw i64 %CurrSBIndex..1, 192
  %"&pSB[currWI].offset1162" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1161"
  %CastToValueType1163 = bitcast i8* %"&pSB[currWI].offset1162" to <16 x i32>*
  %loadedValue1164 = load <16 x i32>* %CastToValueType1163, align 64
  %543 = extractelement <16 x i32> %loadedValue1164, i32 0
  %544 = sext i32 %543 to i64
  %545 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %542, i64 %544
  %ptrTypeCast329 = bitcast double addrspace(3)* %545 to <16 x double> addrspace(3)*
  %546 = load <16 x double> addrspace(3)* %ptrTypeCast329, align 8
  %547 = fadd <16 x double> %540, %546
  %"&(pSB[currWI].offset)1058" = add nuw i64 %CurrSBIndex..1, 64
  %"&pSB[currWI].offset1059" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1058"
  %CastToValueType1060 = bitcast i8* %"&pSB[currWI].offset1059" to <16 x i32>*
  %loadedValue1061 = load <16 x i32>* %CastToValueType1060, align 64
  %548 = extractelement <16 x i32> %loadedValue1061, i32 0
  %549 = sext i32 %548 to i64
  %"&(pSB[currWI].offset)1190" = add nuw i64 %CurrSBIndex..1, 256
  %"&pSB[currWI].offset1191" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1190"
  %CastToValueType1192 = bitcast i8* %"&pSB[currWI].offset1191" to i64*
  %loadedValue1193 = load i64* %CastToValueType1192, align 8
  %550 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue1193, i64 %549
  %ptrTypeCast346 = bitcast double addrspace(3)* %550 to <16 x double> addrspace(3)*
  %551 = load <16 x double> addrspace(3)* %ptrTypeCast346, align 8
  %552 = fadd <16 x double> %547, %551
  %"&(pSB[currWI].offset)1063" = add nuw i64 %CurrSBIndex..1, 64
  %"&pSB[currWI].offset1064" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1063"
  %CastToValueType1065 = bitcast i8* %"&pSB[currWI].offset1064" to <16 x i32>*
  %loadedValue1066 = load <16 x i32>* %CastToValueType1065, align 64
  %553 = add nsw <16 x i32> %loadedValue1066, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %554 = extractelement <16 x i32> %553, i32 0
  %555 = sext i32 %554 to i64
  %"&(pSB[currWI].offset)1185" = add nuw i64 %CurrSBIndex..1, 256
  %"&pSB[currWI].offset1186" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1185"
  %CastToValueType1187 = bitcast i8* %"&pSB[currWI].offset1186" to i64*
  %loadedValue1188 = load i64* %CastToValueType1187, align 8
  %556 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %loadedValue1188, i64 %555
  %ptrTypeCast363 = bitcast double addrspace(3)* %556 to <16 x double> addrspace(3)*
  %557 = load <16 x double> addrspace(3)* %ptrTypeCast363, align 8
  %558 = fadd <16 x double> %552, %557
  %"&(pSB[currWI].offset)1053" = add nuw i64 %CurrSBIndex..1, 64
  %"&pSB[currWI].offset1054" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1053"
  %CastToValueType1055 = bitcast i8* %"&pSB[currWI].offset1054" to <16 x i32>*
  %loadedValue1056 = load <16 x i32>* %CastToValueType1055, align 64
  %559 = extractelement <16 x i32> %loadedValue1056, i32 0
  %560 = sext i32 %559 to i64
  %561 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %536, i64 %560
  %ptrTypeCast364 = bitcast double addrspace(3)* %561 to <16 x double> addrspace(3)*
  %562 = load <16 x double> addrspace(3)* %ptrTypeCast364, align 8
  %563 = extractelement <16 x i32> %553, i32 0
  %564 = sext i32 %563 to i64
  %565 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %536, i64 %564
  %ptrTypeCast365 = bitcast double addrspace(3)* %565 to <16 x double> addrspace(3)*
  %566 = load <16 x double> addrspace(3)* %ptrTypeCast365, align 8
  %567 = fadd <16 x double> %562, %566
  %"&(pSB[currWI].offset)1048" = add nuw i64 %CurrSBIndex..1, 64
  %"&pSB[currWI].offset1049" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1048"
  %CastToValueType1050 = bitcast i8* %"&pSB[currWI].offset1049" to <16 x i32>*
  %loadedValue1051 = load <16 x i32>* %CastToValueType1050, align 64
  %568 = extractelement <16 x i32> %loadedValue1051, i32 0
  %569 = sext i32 %568 to i64
  %570 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %542, i64 %569
  %ptrTypeCast366 = bitcast double addrspace(3)* %570 to <16 x double> addrspace(3)*
  %571 = load <16 x double> addrspace(3)* %ptrTypeCast366, align 8
  %572 = fadd <16 x double> %567, %571
  %573 = extractelement <16 x i32> %553, i32 0
  %574 = sext i32 %573 to i64
  %575 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %1, i64 0, i64 %542, i64 %574
  %ptrTypeCast367 = bitcast double addrspace(3)* %575 to <16 x double> addrspace(3)*
  %576 = load <16 x double> addrspace(3)* %ptrTypeCast367, align 8
  %577 = fadd <16 x double> %572, %576
  %578 = fmul <16 x double> %535, %vector369
  %579 = fmul <16 x double> %558, %vector371
  %580 = fadd <16 x double> %578, %579
  %581 = fmul <16 x double> %577, %vector373
  %582 = fadd <16 x double> %580, %581
  %extract374 = extractelement <16 x double> %582, i32 0
  %extract375 = extractelement <16 x double> %582, i32 1
  %extract376 = extractelement <16 x double> %582, i32 2
  %extract377 = extractelement <16 x double> %582, i32 3
  %extract378 = extractelement <16 x double> %582, i32 4
  %extract379 = extractelement <16 x double> %582, i32 5
  %extract380 = extractelement <16 x double> %582, i32 6
  %extract381 = extractelement <16 x double> %582, i32 7
  %extract382 = extractelement <16 x double> %582, i32 8
  %extract383 = extractelement <16 x double> %582, i32 9
  %extract384 = extractelement <16 x double> %582, i32 10
  %extract385 = extractelement <16 x double> %582, i32 11
  %extract386 = extractelement <16 x double> %582, i32 12
  %extract387 = extractelement <16 x double> %582, i32 13
  %extract388 = extractelement <16 x double> %582, i32 14
  %extract389 = extractelement <16 x double> %582, i32 15
  %"&(pSB[currWI].offset)1152" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1153" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1152"
  %CastToValueType1154 = bitcast i8* %"&pSB[currWI].offset1153" to <16 x i32>*
  %loadedValue1155 = load <16 x i32>* %CastToValueType1154, align 64
  %583 = extractelement <16 x i32> %loadedValue1155, i32 0
  %584 = sext i32 %583 to i64
  %585 = getelementptr inbounds double addrspace(1)* %newData, i64 %584
  %"&(pSB[currWI].offset)1147" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1148" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1147"
  %CastToValueType1149 = bitcast i8* %"&pSB[currWI].offset1148" to <16 x i32>*
  %loadedValue1150 = load <16 x i32>* %CastToValueType1149, align 64
  %586 = extractelement <16 x i32> %loadedValue1150, i32 1
  %587 = sext i32 %586 to i64
  %588 = getelementptr inbounds double addrspace(1)* %newData, i64 %587
  %"&(pSB[currWI].offset)1142" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1143" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1142"
  %CastToValueType1144 = bitcast i8* %"&pSB[currWI].offset1143" to <16 x i32>*
  %loadedValue1145 = load <16 x i32>* %CastToValueType1144, align 64
  %589 = extractelement <16 x i32> %loadedValue1145, i32 2
  %590 = sext i32 %589 to i64
  %591 = getelementptr inbounds double addrspace(1)* %newData, i64 %590
  %"&(pSB[currWI].offset)1137" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1138" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1137"
  %CastToValueType1139 = bitcast i8* %"&pSB[currWI].offset1138" to <16 x i32>*
  %loadedValue1140 = load <16 x i32>* %CastToValueType1139, align 64
  %592 = extractelement <16 x i32> %loadedValue1140, i32 3
  %593 = sext i32 %592 to i64
  %594 = getelementptr inbounds double addrspace(1)* %newData, i64 %593
  %"&(pSB[currWI].offset)1132" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1133" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1132"
  %CastToValueType1134 = bitcast i8* %"&pSB[currWI].offset1133" to <16 x i32>*
  %loadedValue1135 = load <16 x i32>* %CastToValueType1134, align 64
  %595 = extractelement <16 x i32> %loadedValue1135, i32 4
  %596 = sext i32 %595 to i64
  %597 = getelementptr inbounds double addrspace(1)* %newData, i64 %596
  %"&(pSB[currWI].offset)1127" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1128" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1127"
  %CastToValueType1129 = bitcast i8* %"&pSB[currWI].offset1128" to <16 x i32>*
  %loadedValue1130 = load <16 x i32>* %CastToValueType1129, align 64
  %598 = extractelement <16 x i32> %loadedValue1130, i32 5
  %599 = sext i32 %598 to i64
  %600 = getelementptr inbounds double addrspace(1)* %newData, i64 %599
  %"&(pSB[currWI].offset)1122" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1123" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1122"
  %CastToValueType1124 = bitcast i8* %"&pSB[currWI].offset1123" to <16 x i32>*
  %loadedValue1125 = load <16 x i32>* %CastToValueType1124, align 64
  %601 = extractelement <16 x i32> %loadedValue1125, i32 6
  %602 = sext i32 %601 to i64
  %603 = getelementptr inbounds double addrspace(1)* %newData, i64 %602
  %"&(pSB[currWI].offset)1117" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1118" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1117"
  %CastToValueType1119 = bitcast i8* %"&pSB[currWI].offset1118" to <16 x i32>*
  %loadedValue1120 = load <16 x i32>* %CastToValueType1119, align 64
  %604 = extractelement <16 x i32> %loadedValue1120, i32 7
  %605 = sext i32 %604 to i64
  %606 = getelementptr inbounds double addrspace(1)* %newData, i64 %605
  %"&(pSB[currWI].offset)1112" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1113" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1112"
  %CastToValueType1114 = bitcast i8* %"&pSB[currWI].offset1113" to <16 x i32>*
  %loadedValue1115 = load <16 x i32>* %CastToValueType1114, align 64
  %607 = extractelement <16 x i32> %loadedValue1115, i32 8
  %608 = sext i32 %607 to i64
  %609 = getelementptr inbounds double addrspace(1)* %newData, i64 %608
  %"&(pSB[currWI].offset)1107" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1108" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1107"
  %CastToValueType1109 = bitcast i8* %"&pSB[currWI].offset1108" to <16 x i32>*
  %loadedValue1110 = load <16 x i32>* %CastToValueType1109, align 64
  %610 = extractelement <16 x i32> %loadedValue1110, i32 9
  %611 = sext i32 %610 to i64
  %612 = getelementptr inbounds double addrspace(1)* %newData, i64 %611
  %"&(pSB[currWI].offset)1102" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1103" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1102"
  %CastToValueType1104 = bitcast i8* %"&pSB[currWI].offset1103" to <16 x i32>*
  %loadedValue1105 = load <16 x i32>* %CastToValueType1104, align 64
  %613 = extractelement <16 x i32> %loadedValue1105, i32 10
  %614 = sext i32 %613 to i64
  %615 = getelementptr inbounds double addrspace(1)* %newData, i64 %614
  %"&(pSB[currWI].offset)1097" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1098" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1097"
  %CastToValueType1099 = bitcast i8* %"&pSB[currWI].offset1098" to <16 x i32>*
  %loadedValue1100 = load <16 x i32>* %CastToValueType1099, align 64
  %616 = extractelement <16 x i32> %loadedValue1100, i32 11
  %617 = sext i32 %616 to i64
  %618 = getelementptr inbounds double addrspace(1)* %newData, i64 %617
  %"&(pSB[currWI].offset)1092" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1093" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1092"
  %CastToValueType1094 = bitcast i8* %"&pSB[currWI].offset1093" to <16 x i32>*
  %loadedValue1095 = load <16 x i32>* %CastToValueType1094, align 64
  %619 = extractelement <16 x i32> %loadedValue1095, i32 12
  %620 = sext i32 %619 to i64
  %621 = getelementptr inbounds double addrspace(1)* %newData, i64 %620
  %"&(pSB[currWI].offset)1087" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1088" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1087"
  %CastToValueType1089 = bitcast i8* %"&pSB[currWI].offset1088" to <16 x i32>*
  %loadedValue1090 = load <16 x i32>* %CastToValueType1089, align 64
  %622 = extractelement <16 x i32> %loadedValue1090, i32 13
  %623 = sext i32 %622 to i64
  %624 = getelementptr inbounds double addrspace(1)* %newData, i64 %623
  %"&(pSB[currWI].offset)1082" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1083" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1082"
  %CastToValueType1084 = bitcast i8* %"&pSB[currWI].offset1083" to <16 x i32>*
  %loadedValue1085 = load <16 x i32>* %CastToValueType1084, align 64
  %625 = extractelement <16 x i32> %loadedValue1085, i32 14
  %626 = sext i32 %625 to i64
  %627 = getelementptr inbounds double addrspace(1)* %newData, i64 %626
  %"&(pSB[currWI].offset)1077" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset1078" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1077"
  %CastToValueType1079 = bitcast i8* %"&pSB[currWI].offset1078" to <16 x i32>*
  %loadedValue1080 = load <16 x i32>* %CastToValueType1079, align 64
  %628 = extractelement <16 x i32> %loadedValue1080, i32 15
  %629 = sext i32 %628 to i64
  %630 = getelementptr inbounds double addrspace(1)* %newData, i64 %629
  store double %extract374, double addrspace(1)* %585, align 8
  store double %extract375, double addrspace(1)* %588, align 8
  store double %extract376, double addrspace(1)* %591, align 8
  store double %extract377, double addrspace(1)* %594, align 8
  store double %extract378, double addrspace(1)* %597, align 8
  store double %extract379, double addrspace(1)* %600, align 8
  store double %extract380, double addrspace(1)* %603, align 8
  store double %extract381, double addrspace(1)* %606, align 8
  store double %extract382, double addrspace(1)* %609, align 8
  store double %extract383, double addrspace(1)* %612, align 8
  store double %extract384, double addrspace(1)* %615, align 8
  store double %extract385, double addrspace(1)* %618, align 8
  store double %extract386, double addrspace(1)* %621, align 8
  store double %extract387, double addrspace(1)* %624, align 8
  store double %extract388, double addrspace(1)* %627, align 8
  store double %extract389, double addrspace(1)* %630, align 8
  %check.WI.iter1225 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter1225, label %thenBB1222, label %SyncBB1219

thenBB1222:                                       ; preds = %SyncBB
  %"CurrWI++1226" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride1228" = add nuw i64 %CurrSBIndex..1, 320
  br label %SyncBB

SyncBB1219:                                       ; preds = %SyncBB
  ret void
}

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @StencilKernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double*
  %7 = load double* %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double*
  %10 = load double* %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i8 addrspace(3)**
  %16 = load i8 addrspace(3)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to %struct.WorkDim**
  %19 = load %struct.WorkDim** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to i64**
  %22 = load i64** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i8**
  %31 = load i8** %30, align 8
  %32 = bitcast i8 addrspace(3)* %16 to [18 x [18 x double]] addrspace(3)*
  %33 = bitcast i8 addrspace(3)* %16 to double addrspace(3)*
  %34 = getelementptr i8 addrspace(3)* %16, i64 2448
  %35 = bitcast i8 addrspace(3)* %34 to double addrspace(3)*
  %36 = getelementptr i8 addrspace(3)* %16, i64 136
  %37 = bitcast i8 addrspace(3)* %36 to double addrspace(3)*
  %38 = getelementptr i8 addrspace(3)* %16, i64 2584
  %39 = bitcast i8 addrspace(3)* %38 to double addrspace(3)*
  br label %SyncBB95.i

SyncBB95.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %40 = getelementptr i64* %22, i64 1
  %41 = load i64* %40, align 8
  %42 = trunc i64 %41 to i32
  %43 = load i64* %22, align 8
  %44 = trunc i64 %43 to i32
  %45 = getelementptr %struct.WorkDim* %19, i64 0, i32 4, i64 0
  %46 = load i64* %45, align 8
  %47 = trunc i64 %46 to i32
  %48 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 1
  %49 = load i64* %48, align 8
  %50 = trunc i64 %49 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %50, i32* %CastToValueType.i, align 4
  %51 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %52 = load i64* %51, align 8
  %53 = trunc i64 %52 to i32
  %"&(pSB[currWI].offset)111.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset12.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)111.i"
  %CastToValueType13.i = bitcast i8* %"&pSB[currWI].offset12.i" to i32*
  store i32 %53, i32* %CastToValueType13.i, align 4
  %54 = shl i32 %42, 4
  %55 = add nsw i32 %54, %50
  %56 = shl i32 %44, 4
  %57 = add nsw i32 %56, %53
  %58 = shl i32 %47, 4
  %59 = add nsw i32 %55, 1
  %60 = or i32 %58, 2
  %61 = mul nsw i32 %60, %59
  %62 = add nsw i32 %57, 1
  %63 = add nsw i32 %62, %61
  %64 = sext i32 %63 to i64
  %"&(pSB[currWI].offset)302.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset31.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)302.i"
  %CastToValueType32.i = bitcast i8* %"&pSB[currWI].offset31.i" to i64*
  store i64 %64, i64* %CastToValueType32.i, align 8
  %65 = getelementptr inbounds double addrspace(1)* %1, i64 %64
  %66 = load double addrspace(1)* %65, align 8
  %67 = add nsw i32 %53, 1
  %68 = sext i32 %67 to i64
  %"&(pSB[currWI].offset)393.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset40.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)393.i"
  %CastToValueType41.i = bitcast i8* %"&pSB[currWI].offset40.i" to i64*
  store i64 %68, i64* %CastToValueType41.i, align 8
  %69 = add nsw i32 %50, 1
  %70 = sext i32 %69 to i64
  %"&(pSB[currWI].offset)634.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset64.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)634.i"
  %CastToValueType65.i = bitcast i8* %"&pSB[currWI].offset64.i" to i64*
  store i64 %70, i64* %CastToValueType65.i, align 8
  %71 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %70, i64 %68
  %"&(pSB[currWI].offset)875.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset88.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)875.i"
  %CastToValueType89.i = bitcast i8* %"&pSB[currWI].offset88.i" to double addrspace(3)**
  store double addrspace(3)* %71, double addrspace(3)** %CastToValueType89.i, align 8
  store double %66, double addrspace(3)* %71, align 8
  %72 = icmp eq i32 %50, 0
  br i1 %72, label %73, label %91

; <label>:73                                      ; preds = %SyncBB95.i
  %74 = or i32 %58, 2
  %75 = mul nsw i32 %74, %55
  %76 = add nsw i32 %57, 1
  %77 = add nsw i32 %76, %75
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds double addrspace(1)* %1, i64 %78
  %80 = load double addrspace(1)* %79, align 8
  %"&(pSB[currWI].offset)5817.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset59.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)5817.i"
  %CastToValueType60.i = bitcast i8* %"&pSB[currWI].offset59.i" to i64*
  %loadedValue61.i = load i64* %CastToValueType60.i, align 8
  %81 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 0, i64 %loadedValue61.i
  store double %80, double addrspace(3)* %81, align 8
  %82 = add nsw i32 %55, 17
  %83 = or i32 %58, 2
  %84 = mul nsw i32 %83, %82
  %85 = add nsw i32 %57, 1
  %86 = add nsw i32 %85, %84
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds double addrspace(1)* %1, i64 %87
  %89 = load double addrspace(1)* %88, align 8
  %"&(pSB[currWI].offset)5318.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset54.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)5318.i"
  %CastToValueType55.i = bitcast i8* %"&pSB[currWI].offset54.i" to i64*
  %loadedValue56.i = load i64* %CastToValueType55.i, align 8
  %90 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 17, i64 %loadedValue56.i
  store double %89, double addrspace(3)* %90, align 8
  br label %91

; <label>:91                                      ; preds = %73, %SyncBB95.i
  %"&(pSB[currWI].offset)206.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)206.i"
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i32*
  %loadedValue23.i = load i32* %CastToValueType22.i, align 4
  %92 = icmp eq i32 %loadedValue23.i, 0
  br i1 %92, label %93, label %111

; <label>:93                                      ; preds = %91
  %94 = add nsw i32 %55, 1
  %95 = or i32 %58, 2
  %96 = mul nsw i32 %95, %94
  %97 = add nsw i32 %57, %96
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds double addrspace(1)* %1, i64 %98
  %100 = load double addrspace(1)* %99, align 8
  %"&(pSB[currWI].offset)8215.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset83.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)8215.i"
  %CastToValueType84.i = bitcast i8* %"&pSB[currWI].offset83.i" to i64*
  %loadedValue85.i = load i64* %CastToValueType84.i, align 8
  %101 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue85.i, i64 0
  store double %100, double addrspace(3)* %101, align 16
  %102 = add nsw i32 %55, 1
  %103 = or i32 %58, 2
  %104 = mul nsw i32 %103, %102
  %105 = add nsw i32 %57, 17
  %106 = add nsw i32 %105, %104
  %107 = sext i32 %106 to i64
  %108 = getelementptr inbounds double addrspace(1)* %1, i64 %107
  %109 = load double addrspace(1)* %108, align 8
  %"&(pSB[currWI].offset)7716.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset78.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)7716.i"
  %CastToValueType79.i = bitcast i8* %"&pSB[currWI].offset78.i" to i64*
  %loadedValue80.i = load i64* %CastToValueType79.i, align 8
  %110 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue80.i, i64 17
  store double %109, double addrspace(3)* %110, align 8
  br label %111

; <label>:111                                     ; preds = %93, %91
  %112 = or i64 %52, %49
  %113 = trunc i64 %112 to i32
  %114 = icmp eq i32 %113, 0
  br i1 %114, label %115, label %144

; <label>:115                                     ; preds = %111
  %116 = or i32 %58, 2
  %117 = mul nsw i32 %116, %55
  %118 = add nsw i32 %57, %117
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds double addrspace(1)* %1, i64 %119
  %121 = load double addrspace(1)* %120, align 8
  store double %121, double addrspace(3)* %33, align 16
  %122 = add nsw i32 %55, 17
  %123 = or i32 %58, 2
  %124 = mul nsw i32 %123, %122
  %125 = add nsw i32 %57, %124
  %126 = sext i32 %125 to i64
  %127 = getelementptr inbounds double addrspace(1)* %1, i64 %126
  %128 = load double addrspace(1)* %127, align 8
  store double %128, double addrspace(3)* %35, align 16
  %129 = or i32 %58, 2
  %130 = mul nsw i32 %129, %55
  %131 = add nsw i32 %57, 17
  %132 = add nsw i32 %131, %130
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds double addrspace(1)* %1, i64 %133
  %135 = load double addrspace(1)* %134, align 8
  store double %135, double addrspace(3)* %37, align 8
  %136 = add nsw i32 %55, 17
  %137 = or i32 %58, 2
  %138 = mul nsw i32 %137, %136
  %139 = add nsw i32 %57, 17
  %140 = add nsw i32 %139, %138
  %141 = sext i32 %140 to i64
  %142 = getelementptr inbounds double addrspace(1)* %1, i64 %141
  %143 = load double addrspace(1)* %142, align 8
  store double %143, double addrspace(3)* %39, align 8
  br label %144

; <label>:144                                     ; preds = %115, %111
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %144
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 320
  br label %SyncBB95.i

SyncBB.i:                                         ; preds = %thenBB98.i, %144
  %CurrWI..1.i = phi i64 [ %"CurrWI++102.i", %thenBB98.i ], [ 0, %144 ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride104.i", %thenBB98.i ], [ 0, %144 ]
  %"&(pSB[currWI].offset)917.i" = or i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset92.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)917.i"
  %CastToValueType93.i = bitcast i8* %"&pSB[currWI].offset92.i" to double addrspace(3)**
  %loadedValue94.i = load double addrspace(3)** %CastToValueType93.i, align 8
  %145 = load double addrspace(3)* %loadedValue94.i, align 8
  %"&pSB[currWI].offset7.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType8.i = bitcast i8* %"&pSB[currWI].offset7.i" to i32*
  %loadedValue9.i = load i32* %CastToValueType8.i, align 4
  %146 = sext i32 %loadedValue9.i to i64
  %"&(pSB[currWI].offset)488.i" = or i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)488.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to i64*
  %loadedValue51.i = load i64* %CastToValueType50.i, align 8
  %147 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %146, i64 %loadedValue51.i
  %148 = load double addrspace(3)* %147, align 8
  %"&pSB[currWI].offset3.i" = getelementptr inbounds i8* %31, i64 %CurrSBIndex..1.i
  %CastToValueType4.i = bitcast i8* %"&pSB[currWI].offset3.i" to i32*
  %loadedValue.i = load i32* %CastToValueType4.i, align 4
  %149 = add nsw i32 %loadedValue.i, 2
  %150 = sext i32 %149 to i64
  %"&(pSB[currWI].offset)439.i" = or i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset44.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)439.i"
  %CastToValueType45.i = bitcast i8* %"&pSB[currWI].offset44.i" to i64*
  %loadedValue46.i = load i64* %CastToValueType45.i, align 8
  %151 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %150, i64 %loadedValue46.i
  %152 = load double addrspace(3)* %151, align 8
  %153 = fadd double %148, %152
  %"&(pSB[currWI].offset)2510.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset26.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)2510.i"
  %CastToValueType27.i = bitcast i8* %"&pSB[currWI].offset26.i" to i32*
  %loadedValue28.i = load i32* %CastToValueType27.i, align 4
  %154 = sext i32 %loadedValue28.i to i64
  %"&(pSB[currWI].offset)7211.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)7211.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to i64*
  %loadedValue75.i = load i64* %CastToValueType74.i, align 8
  %155 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue75.i, i64 %154
  %156 = load double addrspace(3)* %155, align 8
  %157 = fadd double %153, %156
  %"&(pSB[currWI].offset)1512.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset16.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1512.i"
  %CastToValueType17.i = bitcast i8* %"&pSB[currWI].offset16.i" to i32*
  %loadedValue18.i = load i32* %CastToValueType17.i, align 4
  %158 = add nsw i32 %loadedValue18.i, 2
  %159 = sext i32 %158 to i64
  %"&(pSB[currWI].offset)6713.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset68.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)6713.i"
  %CastToValueType69.i = bitcast i8* %"&pSB[currWI].offset68.i" to i64*
  %loadedValue70.i = load i64* %CastToValueType69.i, align 8
  %160 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue70.i, i64 %159
  %161 = load double addrspace(3)* %160, align 8
  %162 = fadd double %157, %161
  %163 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %146, i64 %154
  %164 = load double addrspace(3)* %163, align 8
  %165 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %146, i64 %159
  %166 = load double addrspace(3)* %165, align 8
  %167 = fadd double %164, %166
  %168 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %150, i64 %154
  %169 = load double addrspace(3)* %168, align 8
  %170 = fadd double %167, %169
  %171 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %150, i64 %159
  %172 = load double addrspace(3)* %171, align 8
  %173 = fadd double %170, %172
  %174 = fmul double %145, %7
  %175 = fmul double %162, %10
  %176 = fadd double %174, %175
  %177 = fmul double %173, %13
  %178 = fadd double %176, %177
  %"&(pSB[currWI].offset)3414.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset35.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)3414.i"
  %CastToValueType36.i = bitcast i8* %"&pSB[currWI].offset35.i" to i64*
  %loadedValue37.i = load i64* %CastToValueType36.i, align 8
  %179 = getelementptr inbounds double addrspace(1)* %4, i64 %loadedValue37.i
  store double %178, double addrspace(1)* %179, align 8
  %check.WI.iter101.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter101.i, label %thenBB98.i, label %__StencilKernel_separated_args.exit

thenBB98.i:                                       ; preds = %SyncBB.i
  %"CurrWI++102.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride104.i" = add nuw i64 %CurrSBIndex..1.i, 320
  br label %SyncBB.i

__StencilKernel_separated_args.exit:              ; preds = %SyncBB.i
  ret void
}

define void @CopyRect(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 12
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 16
  %9 = bitcast i8* %8 to double addrspace(1)**
  %10 = load double addrspace(1)** %9, align 8
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
  %35 = icmp sgt i32 %19, 0
  %36 = sext i32 %13 to i64
  %37 = sext i32 %4 to i64
  br label %SyncBB3.i

SyncBB3.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %38 = load i64* %28, align 8
  %39 = trunc i64 %38 to i32
  %40 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..0.i, i32 0, i64 0
  %41 = load i64* %40, align 8
  %42 = trunc i64 %41 to i32
  %43 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %44 = load i64* %43, align 8
  %45 = trunc i64 %44 to i32
  %46 = mul nsw i32 %45, %39
  %47 = add nsw i32 %46, %42
  %48 = icmp slt i32 %47, %22
  %or.cond.i = and i1 %48, %35
  br i1 %or.cond.i, label %bb.nph.i, label %.loopexit.i

bb.nph.i:                                         ; preds = %SyncBB3.i
  %49 = mul nsw i32 %47, %16
  %50 = mul nsw i32 %47, %7
  br label %51

; <label>:51                                      ; preds = %51, %bb.nph.i
  %52 = phi i32 [ 0, %bb.nph.i ], [ %60, %51 ]
  %53 = add nsw i32 %49, %52
  %54 = sext i32 %53 to i64
  %.sum.i = add i64 %54, %36
  %55 = getelementptr inbounds double addrspace(1)* %10, i64 %.sum.i
  %56 = load double addrspace(1)* %55, align 8
  %57 = add nsw i32 %50, %52
  %58 = sext i32 %57 to i64
  %.sum1.i = add i64 %58, %37
  %59 = getelementptr inbounds double addrspace(1)* %1, i64 %.sum1.i
  store double %56, double addrspace(1)* %59, align 8
  %60 = add nsw i32 %52, 1
  %exitcond.i = icmp eq i32 %60, %19
  br i1 %exitcond.i, label %.loopexit.i, label %51

.loopexit.i:                                      ; preds = %51, %SyncBB3.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %__CopyRect_separated_args.exit

thenBB.i:                                         ; preds = %.loopexit.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB3.i

__CopyRect_separated_args.exit:                   ; preds = %.loopexit.i
  ret void
}

define void @__Vectorized_.StencilKernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double*
  %7 = load double* %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to double*
  %10 = load double* %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to double*
  %13 = load double* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i8 addrspace(3)**
  %16 = load i8 addrspace(3)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to %struct.WorkDim**
  %19 = load %struct.WorkDim** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to i64**
  %22 = load i64** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to %struct.PaddedDimId**
  %25 = load %struct.PaddedDimId** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 88
  %27 = bitcast i8* %26 to i64*
  %28 = load i64* %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i8**
  %31 = load i8** %30, align 8
  %32 = bitcast i8 addrspace(3)* %16 to [18 x [18 x double]] addrspace(3)*
  %33 = bitcast i8 addrspace(3)* %16 to double addrspace(3)*
  %34 = getelementptr i8 addrspace(3)* %16, i64 2448
  %35 = bitcast i8 addrspace(3)* %34 to double addrspace(3)*
  %36 = getelementptr i8 addrspace(3)* %16, i64 136
  %37 = bitcast i8 addrspace(3)* %36 to double addrspace(3)*
  %38 = getelementptr i8 addrspace(3)* %16, i64 2584
  %39 = bitcast i8 addrspace(3)* %38 to double addrspace(3)*
  br label %SyncBB1220.i

SyncBB1220.i:                                     ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %40 = getelementptr i64* %22, i64 1
  %41 = load i64* %40, align 8
  %42 = trunc i64 %41 to i32
  %43 = load i64* %22, align 8
  %44 = trunc i64 %43 to i32
  %45 = getelementptr %struct.WorkDim* %19, i64 0, i32 4, i64 0
  %46 = load i64* %45, align 8
  %temp.vect.i = insertelement <16 x i64> undef, i64 %46, i32 0
  %temp.vect53.i = insertelement <16 x i64> %temp.vect.i, i64 %46, i32 1
  %temp.vect54.i = insertelement <16 x i64> %temp.vect53.i, i64 %46, i32 2
  %temp.vect55.i = insertelement <16 x i64> %temp.vect54.i, i64 %46, i32 3
  %temp.vect56.i = insertelement <16 x i64> %temp.vect55.i, i64 %46, i32 4
  %temp.vect57.i = insertelement <16 x i64> %temp.vect56.i, i64 %46, i32 5
  %temp.vect58.i = insertelement <16 x i64> %temp.vect57.i, i64 %46, i32 6
  %temp.vect59.i = insertelement <16 x i64> %temp.vect58.i, i64 %46, i32 7
  %temp.vect60.i = insertelement <16 x i64> %temp.vect59.i, i64 %46, i32 8
  %temp.vect61.i = insertelement <16 x i64> %temp.vect60.i, i64 %46, i32 9
  %temp.vect62.i = insertelement <16 x i64> %temp.vect61.i, i64 %46, i32 10
  %temp.vect63.i = insertelement <16 x i64> %temp.vect62.i, i64 %46, i32 11
  %temp.vect64.i = insertelement <16 x i64> %temp.vect63.i, i64 %46, i32 12
  %temp.vect65.i = insertelement <16 x i64> %temp.vect64.i, i64 %46, i32 13
  %temp.vect66.i = insertelement <16 x i64> %temp.vect65.i, i64 %46, i32 14
  %temp.vect67.i = insertelement <16 x i64> %temp.vect66.i, i64 %46, i32 15
  %47 = trunc <16 x i64> %temp.vect67.i to <16 x i32>
  %48 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 1
  %49 = load i64* %48, align 8
  %temp240.i = insertelement <16 x i64> undef, i64 %49, i32 0
  %vector241.i = shufflevector <16 x i64> %temp240.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %50 = trunc i64 %49 to i32
  %"&(pSB[currWI].offset)1.i" = or i64 %CurrSBIndex..0.i, 40
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1.i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %50, i32* %CastToValueType.i, align 4
  %51 = getelementptr %struct.PaddedDimId* %25, i64 %CurrWI..0.i, i32 0, i64 0
  %52 = load i64* %51, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %52, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %53 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %54 = trunc <16 x i64> %53 to <16 x i32>
  %"&(pSB[currWI].offset)1044.i" = add nuw i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset1045.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1044.i"
  %CastToValueType1046.i = bitcast i8* %"&pSB[currWI].offset1045.i" to <16 x i32>*
  store <16 x i32> %54, <16 x i32>* %CastToValueType1046.i, align 64
  %55 = shl i32 %42, 4
  %56 = add nsw i32 %55, %50
  %temp117.i = insertelement <16 x i32> undef, i32 %56, i32 0
  %vector118.i = shufflevector <16 x i32> %temp117.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %57 = shl i32 %44, 4
  %temp.i = insertelement <16 x i32> undef, i32 %57, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %58 = add nsw <16 x i32> %vector.i, %54
  %59 = shl <16 x i32> %47, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %60 = add nsw i32 %56, 1
  %temp68.i = insertelement <16 x i32> undef, i32 %60, i32 0
  %vector69.i = shufflevector <16 x i32> %temp68.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %61 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %62 = mul nsw <16 x i32> %61, %vector69.i
  %63 = add nsw <16 x i32> %58, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %64 = add nsw <16 x i32> %63, %62
  %"&(pSB[currWI].offset)1073.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset1074.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1073.i"
  %CastToValueType1075.i = bitcast i8* %"&pSB[currWI].offset1074.i" to <16 x i32>*
  store <16 x i32> %64, <16 x i32>* %CastToValueType1075.i, align 64
  %65 = extractelement <16 x i32> %64, i32 0
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds double addrspace(1)* %1, i64 %66
  %68 = extractelement <16 x i32> %64, i32 1
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds double addrspace(1)* %1, i64 %69
  %71 = extractelement <16 x i32> %64, i32 2
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds double addrspace(1)* %1, i64 %72
  %74 = extractelement <16 x i32> %64, i32 3
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds double addrspace(1)* %1, i64 %75
  %77 = extractelement <16 x i32> %64, i32 4
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds double addrspace(1)* %1, i64 %78
  %80 = extractelement <16 x i32> %64, i32 5
  %81 = sext i32 %80 to i64
  %82 = getelementptr inbounds double addrspace(1)* %1, i64 %81
  %83 = extractelement <16 x i32> %64, i32 6
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds double addrspace(1)* %1, i64 %84
  %86 = extractelement <16 x i32> %64, i32 7
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds double addrspace(1)* %1, i64 %87
  %89 = extractelement <16 x i32> %64, i32 8
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds double addrspace(1)* %1, i64 %90
  %92 = extractelement <16 x i32> %64, i32 9
  %93 = sext i32 %92 to i64
  %94 = getelementptr inbounds double addrspace(1)* %1, i64 %93
  %95 = extractelement <16 x i32> %64, i32 10
  %96 = sext i32 %95 to i64
  %97 = getelementptr inbounds double addrspace(1)* %1, i64 %96
  %98 = extractelement <16 x i32> %64, i32 11
  %99 = sext i32 %98 to i64
  %100 = getelementptr inbounds double addrspace(1)* %1, i64 %99
  %101 = extractelement <16 x i32> %64, i32 12
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds double addrspace(1)* %1, i64 %102
  %104 = extractelement <16 x i32> %64, i32 13
  %105 = sext i32 %104 to i64
  %106 = getelementptr inbounds double addrspace(1)* %1, i64 %105
  %107 = extractelement <16 x i32> %64, i32 14
  %108 = sext i32 %107 to i64
  %109 = getelementptr inbounds double addrspace(1)* %1, i64 %108
  %110 = extractelement <16 x i32> %64, i32 15
  %111 = sext i32 %110 to i64
  %112 = getelementptr inbounds double addrspace(1)* %1, i64 %111
  %113 = load double addrspace(1)* %67, align 8
  %114 = load double addrspace(1)* %70, align 8
  %115 = load double addrspace(1)* %73, align 8
  %116 = load double addrspace(1)* %76, align 8
  %117 = load double addrspace(1)* %79, align 8
  %118 = load double addrspace(1)* %82, align 8
  %119 = load double addrspace(1)* %85, align 8
  %120 = load double addrspace(1)* %88, align 8
  %121 = load double addrspace(1)* %91, align 8
  %122 = load double addrspace(1)* %94, align 8
  %123 = load double addrspace(1)* %97, align 8
  %124 = load double addrspace(1)* %100, align 8
  %125 = load double addrspace(1)* %103, align 8
  %126 = load double addrspace(1)* %106, align 8
  %127 = load double addrspace(1)* %109, align 8
  %128 = load double addrspace(1)* %112, align 8
  %temp.vect101.i = insertelement <16 x double> undef, double %113, i32 0
  %temp.vect102.i = insertelement <16 x double> %temp.vect101.i, double %114, i32 1
  %temp.vect103.i = insertelement <16 x double> %temp.vect102.i, double %115, i32 2
  %temp.vect104.i = insertelement <16 x double> %temp.vect103.i, double %116, i32 3
  %temp.vect105.i = insertelement <16 x double> %temp.vect104.i, double %117, i32 4
  %temp.vect106.i = insertelement <16 x double> %temp.vect105.i, double %118, i32 5
  %temp.vect107.i = insertelement <16 x double> %temp.vect106.i, double %119, i32 6
  %temp.vect108.i = insertelement <16 x double> %temp.vect107.i, double %120, i32 7
  %temp.vect109.i = insertelement <16 x double> %temp.vect108.i, double %121, i32 8
  %temp.vect110.i = insertelement <16 x double> %temp.vect109.i, double %122, i32 9
  %temp.vect111.i = insertelement <16 x double> %temp.vect110.i, double %123, i32 10
  %temp.vect112.i = insertelement <16 x double> %temp.vect111.i, double %124, i32 11
  %temp.vect113.i = insertelement <16 x double> %temp.vect112.i, double %125, i32 12
  %temp.vect114.i = insertelement <16 x double> %temp.vect113.i, double %126, i32 13
  %temp.vect115.i = insertelement <16 x double> %temp.vect114.i, double %127, i32 14
  %temp.vect116.i = insertelement <16 x double> %temp.vect115.i, double %128, i32 15
  %129 = add nsw <16 x i32> %54, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %"&(pSB[currWI].offset)1157.i" = add nuw i64 %CurrSBIndex..0.i, 192
  %"&pSB[currWI].offset1158.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1157.i"
  %CastToValueType1159.i = bitcast i8* %"&pSB[currWI].offset1158.i" to <16 x i32>*
  store <16 x i32> %129, <16 x i32>* %CastToValueType1159.i, align 64
  %130 = add nsw i32 %50, 1
  %131 = sext i32 %130 to i64
  %"&(pSB[currWI].offset)1181.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1182.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1181.i"
  %CastToValueType1183.i = bitcast i8* %"&pSB[currWI].offset1182.i" to i64*
  store i64 %131, i64* %CastToValueType1183.i, align 8
  %132 = extractelement <16 x i32> %129, i32 0
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %131, i64 %133
  %"&(pSB[currWI].offset)1205.i" = add nuw i64 %CurrSBIndex..0.i, 264
  %"&pSB[currWI].offset1206.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1205.i"
  %CastToValueType1207.i = bitcast i8* %"&pSB[currWI].offset1206.i" to double addrspace(3)**
  store double addrspace(3)* %134, double addrspace(3)** %CastToValueType1207.i, align 8
  %ptrTypeCast.i = bitcast double addrspace(3)* %134 to <16 x double> addrspace(3)*
  store <16 x double> %temp.vect116.i, <16 x double> addrspace(3)* %ptrTypeCast.i, align 8
  %135 = icmp eq i32 %50, 0
  %136 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %137 = mul nsw <16 x i32> %136, %vector118.i
  %138 = add nsw <16 x i32> %58, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %139 = add nsw <16 x i32> %138, %137
  br i1 %135, label %preload.i, label %postload.i

preload.i:                                        ; preds = %SyncBB1220.i
  %140 = extractelement <16 x i32> %139, i32 15
  %141 = sext i32 %140 to i64
  %142 = getelementptr inbounds double addrspace(1)* %1, i64 %141
  %143 = extractelement <16 x i32> %139, i32 14
  %144 = sext i32 %143 to i64
  %145 = getelementptr inbounds double addrspace(1)* %1, i64 %144
  %146 = extractelement <16 x i32> %139, i32 13
  %147 = sext i32 %146 to i64
  %148 = getelementptr inbounds double addrspace(1)* %1, i64 %147
  %149 = extractelement <16 x i32> %139, i32 12
  %150 = sext i32 %149 to i64
  %151 = getelementptr inbounds double addrspace(1)* %1, i64 %150
  %152 = extractelement <16 x i32> %139, i32 11
  %153 = sext i32 %152 to i64
  %154 = getelementptr inbounds double addrspace(1)* %1, i64 %153
  %155 = extractelement <16 x i32> %139, i32 10
  %156 = sext i32 %155 to i64
  %157 = getelementptr inbounds double addrspace(1)* %1, i64 %156
  %158 = extractelement <16 x i32> %139, i32 9
  %159 = sext i32 %158 to i64
  %160 = getelementptr inbounds double addrspace(1)* %1, i64 %159
  %161 = extractelement <16 x i32> %139, i32 8
  %162 = sext i32 %161 to i64
  %163 = getelementptr inbounds double addrspace(1)* %1, i64 %162
  %164 = extractelement <16 x i32> %139, i32 7
  %165 = sext i32 %164 to i64
  %166 = getelementptr inbounds double addrspace(1)* %1, i64 %165
  %167 = extractelement <16 x i32> %139, i32 6
  %168 = sext i32 %167 to i64
  %169 = getelementptr inbounds double addrspace(1)* %1, i64 %168
  %170 = extractelement <16 x i32> %139, i32 5
  %171 = sext i32 %170 to i64
  %172 = getelementptr inbounds double addrspace(1)* %1, i64 %171
  %173 = extractelement <16 x i32> %139, i32 4
  %174 = sext i32 %173 to i64
  %175 = getelementptr inbounds double addrspace(1)* %1, i64 %174
  %176 = extractelement <16 x i32> %139, i32 3
  %177 = sext i32 %176 to i64
  %178 = getelementptr inbounds double addrspace(1)* %1, i64 %177
  %179 = extractelement <16 x i32> %139, i32 2
  %180 = sext i32 %179 to i64
  %181 = getelementptr inbounds double addrspace(1)* %1, i64 %180
  %182 = extractelement <16 x i32> %139, i32 1
  %183 = sext i32 %182 to i64
  %184 = getelementptr inbounds double addrspace(1)* %1, i64 %183
  %185 = extractelement <16 x i32> %139, i32 0
  %186 = sext i32 %185 to i64
  %187 = getelementptr inbounds double addrspace(1)* %1, i64 %186
  %masked_load.i = load double addrspace(1)* %187, align 8
  %masked_load390.i = load double addrspace(1)* %184, align 8
  %masked_load391.i = load double addrspace(1)* %181, align 8
  %masked_load392.i = load double addrspace(1)* %178, align 8
  %masked_load393.i = load double addrspace(1)* %175, align 8
  %masked_load394.i = load double addrspace(1)* %172, align 8
  %masked_load395.i = load double addrspace(1)* %169, align 8
  %masked_load396.i = load double addrspace(1)* %166, align 8
  %masked_load397.i = load double addrspace(1)* %163, align 8
  %masked_load398.i = load double addrspace(1)* %160, align 8
  %masked_load399.i = load double addrspace(1)* %157, align 8
  %masked_load400.i = load double addrspace(1)* %154, align 8
  %masked_load401.i = load double addrspace(1)* %151, align 8
  %masked_load402.i = load double addrspace(1)* %148, align 8
  %masked_load403.i = load double addrspace(1)* %145, align 8
  %masked_load404.i = load double addrspace(1)* %142, align 8
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %SyncBB1220.i
  %phi.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load.i, %preload.i ]
  %phi517.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load390.i, %preload.i ]
  %phi518.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load391.i, %preload.i ]
  %phi519.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load392.i, %preload.i ]
  %phi520.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load393.i, %preload.i ]
  %phi521.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load394.i, %preload.i ]
  %phi522.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load395.i, %preload.i ]
  %phi523.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load396.i, %preload.i ]
  %phi524.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load397.i, %preload.i ]
  %phi525.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load398.i, %preload.i ]
  %phi526.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load399.i, %preload.i ]
  %phi527.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load400.i, %preload.i ]
  %phi528.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load401.i, %preload.i ]
  %phi529.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load402.i, %preload.i ]
  %phi530.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load403.i, %preload.i ]
  %phi531.i = phi double [ undef, %SyncBB1220.i ], [ %masked_load404.i, %preload.i ]
  br i1 %135, label %preload532.i, label %postload533.i

preload532.i:                                     ; preds = %postload.i
  %temp.vect136.i = insertelement <16 x double> undef, double %phi.i, i32 0
  %temp.vect137.i = insertelement <16 x double> %temp.vect136.i, double %phi517.i, i32 1
  %temp.vect138.i = insertelement <16 x double> %temp.vect137.i, double %phi518.i, i32 2
  %temp.vect139.i = insertelement <16 x double> %temp.vect138.i, double %phi519.i, i32 3
  %temp.vect140.i = insertelement <16 x double> %temp.vect139.i, double %phi520.i, i32 4
  %temp.vect141.i = insertelement <16 x double> %temp.vect140.i, double %phi521.i, i32 5
  %temp.vect142.i = insertelement <16 x double> %temp.vect141.i, double %phi522.i, i32 6
  %temp.vect143.i = insertelement <16 x double> %temp.vect142.i, double %phi523.i, i32 7
  %temp.vect144.i = insertelement <16 x double> %temp.vect143.i, double %phi524.i, i32 8
  %temp.vect145.i = insertelement <16 x double> %temp.vect144.i, double %phi525.i, i32 9
  %temp.vect146.i = insertelement <16 x double> %temp.vect145.i, double %phi526.i, i32 10
  %temp.vect147.i = insertelement <16 x double> %temp.vect146.i, double %phi527.i, i32 11
  %"&(pSB[currWI].offset)1176.i" = add nuw i64 %CurrSBIndex..0.i, 192
  %"&pSB[currWI].offset1177.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1176.i"
  %CastToValueType1178.i = bitcast i8* %"&pSB[currWI].offset1177.i" to <16 x i32>*
  %loadedValue1179.i = load <16 x i32>* %CastToValueType1178.i, align 64
  %188 = extractelement <16 x i32> %loadedValue1179.i, i32 0
  %temp.vect148.i = insertelement <16 x double> %temp.vect147.i, double %phi528.i, i32 12
  %189 = sext i32 %188 to i64
  %temp.vect149.i = insertelement <16 x double> %temp.vect148.i, double %phi529.i, i32 13
  %190 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 0, i64 %189
  %temp.vect150.i = insertelement <16 x double> %temp.vect149.i, double %phi530.i, i32 14
  %ptrTypeCast135.i = bitcast double addrspace(3)* %190 to <16 x double> addrspace(3)*
  %temp.vect151.i = insertelement <16 x double> %temp.vect150.i, double %phi531.i, i32 15
  store <16 x double> %temp.vect151.i, <16 x double> addrspace(3)* %ptrTypeCast135.i, align 128
  br label %postload533.i

postload533.i:                                    ; preds = %preload532.i, %postload.i
  %191 = add nsw i32 %56, 17
  %temp152.i = insertelement <16 x i32> undef, i32 %191, i32 0
  %vector153.i = shufflevector <16 x i32> %temp152.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %192 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %193 = mul nsw <16 x i32> %192, %vector153.i
  %194 = add nsw <16 x i32> %58, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %195 = add nsw <16 x i32> %194, %193
  br i1 %135, label %preload534.i, label %postload535.i

preload534.i:                                     ; preds = %postload533.i
  %196 = extractelement <16 x i32> %195, i32 15
  %197 = sext i32 %196 to i64
  %198 = getelementptr inbounds double addrspace(1)* %1, i64 %197
  %199 = extractelement <16 x i32> %195, i32 14
  %200 = sext i32 %199 to i64
  %201 = getelementptr inbounds double addrspace(1)* %1, i64 %200
  %202 = extractelement <16 x i32> %195, i32 13
  %203 = sext i32 %202 to i64
  %204 = getelementptr inbounds double addrspace(1)* %1, i64 %203
  %205 = extractelement <16 x i32> %195, i32 12
  %206 = sext i32 %205 to i64
  %207 = getelementptr inbounds double addrspace(1)* %1, i64 %206
  %208 = extractelement <16 x i32> %195, i32 11
  %209 = sext i32 %208 to i64
  %210 = getelementptr inbounds double addrspace(1)* %1, i64 %209
  %211 = extractelement <16 x i32> %195, i32 10
  %212 = sext i32 %211 to i64
  %213 = getelementptr inbounds double addrspace(1)* %1, i64 %212
  %214 = extractelement <16 x i32> %195, i32 9
  %215 = sext i32 %214 to i64
  %216 = getelementptr inbounds double addrspace(1)* %1, i64 %215
  %217 = extractelement <16 x i32> %195, i32 8
  %218 = sext i32 %217 to i64
  %219 = getelementptr inbounds double addrspace(1)* %1, i64 %218
  %220 = extractelement <16 x i32> %195, i32 7
  %221 = sext i32 %220 to i64
  %222 = getelementptr inbounds double addrspace(1)* %1, i64 %221
  %223 = extractelement <16 x i32> %195, i32 6
  %224 = sext i32 %223 to i64
  %225 = getelementptr inbounds double addrspace(1)* %1, i64 %224
  %226 = extractelement <16 x i32> %195, i32 5
  %227 = sext i32 %226 to i64
  %228 = getelementptr inbounds double addrspace(1)* %1, i64 %227
  %229 = extractelement <16 x i32> %195, i32 4
  %230 = sext i32 %229 to i64
  %231 = getelementptr inbounds double addrspace(1)* %1, i64 %230
  %232 = extractelement <16 x i32> %195, i32 3
  %233 = sext i32 %232 to i64
  %234 = getelementptr inbounds double addrspace(1)* %1, i64 %233
  %235 = extractelement <16 x i32> %195, i32 2
  %236 = sext i32 %235 to i64
  %237 = getelementptr inbounds double addrspace(1)* %1, i64 %236
  %238 = extractelement <16 x i32> %195, i32 1
  %239 = sext i32 %238 to i64
  %240 = getelementptr inbounds double addrspace(1)* %1, i64 %239
  %241 = extractelement <16 x i32> %195, i32 0
  %242 = sext i32 %241 to i64
  %243 = getelementptr inbounds double addrspace(1)* %1, i64 %242
  %masked_load405.i = load double addrspace(1)* %243, align 8
  %masked_load406.i = load double addrspace(1)* %240, align 8
  %masked_load407.i = load double addrspace(1)* %237, align 8
  %masked_load408.i = load double addrspace(1)* %234, align 8
  %masked_load409.i = load double addrspace(1)* %231, align 8
  %masked_load410.i = load double addrspace(1)* %228, align 8
  %masked_load411.i = load double addrspace(1)* %225, align 8
  %masked_load412.i = load double addrspace(1)* %222, align 8
  %masked_load413.i = load double addrspace(1)* %219, align 8
  %masked_load414.i = load double addrspace(1)* %216, align 8
  %masked_load415.i = load double addrspace(1)* %213, align 8
  %masked_load416.i = load double addrspace(1)* %210, align 8
  %masked_load417.i = load double addrspace(1)* %207, align 8
  %masked_load418.i = load double addrspace(1)* %204, align 8
  %masked_load419.i = load double addrspace(1)* %201, align 8
  %masked_load420.i = load double addrspace(1)* %198, align 8
  br label %postload535.i

postload535.i:                                    ; preds = %preload534.i, %postload533.i
  %phi536.i = phi double [ undef, %postload533.i ], [ %masked_load405.i, %preload534.i ]
  %phi537.i = phi double [ undef, %postload533.i ], [ %masked_load406.i, %preload534.i ]
  %phi538.i = phi double [ undef, %postload533.i ], [ %masked_load407.i, %preload534.i ]
  %phi539.i = phi double [ undef, %postload533.i ], [ %masked_load408.i, %preload534.i ]
  %phi540.i = phi double [ undef, %postload533.i ], [ %masked_load409.i, %preload534.i ]
  %phi541.i = phi double [ undef, %postload533.i ], [ %masked_load410.i, %preload534.i ]
  %phi542.i = phi double [ undef, %postload533.i ], [ %masked_load411.i, %preload534.i ]
  %phi543.i = phi double [ undef, %postload533.i ], [ %masked_load412.i, %preload534.i ]
  %phi544.i = phi double [ undef, %postload533.i ], [ %masked_load413.i, %preload534.i ]
  %phi545.i = phi double [ undef, %postload533.i ], [ %masked_load414.i, %preload534.i ]
  %phi546.i = phi double [ undef, %postload533.i ], [ %masked_load415.i, %preload534.i ]
  %phi547.i = phi double [ undef, %postload533.i ], [ %masked_load416.i, %preload534.i ]
  %phi548.i = phi double [ undef, %postload533.i ], [ %masked_load417.i, %preload534.i ]
  %phi549.i = phi double [ undef, %postload533.i ], [ %masked_load418.i, %preload534.i ]
  %phi550.i = phi double [ undef, %postload533.i ], [ %masked_load419.i, %preload534.i ]
  %phi551.i = phi double [ undef, %postload533.i ], [ %masked_load420.i, %preload534.i ]
  br i1 %135, label %preload552.i, label %postload553.i

preload552.i:                                     ; preds = %postload535.i
  %temp.vect171.i = insertelement <16 x double> undef, double %phi536.i, i32 0
  %temp.vect172.i = insertelement <16 x double> %temp.vect171.i, double %phi537.i, i32 1
  %temp.vect173.i = insertelement <16 x double> %temp.vect172.i, double %phi538.i, i32 2
  %temp.vect174.i = insertelement <16 x double> %temp.vect173.i, double %phi539.i, i32 3
  %temp.vect175.i = insertelement <16 x double> %temp.vect174.i, double %phi540.i, i32 4
  %temp.vect176.i = insertelement <16 x double> %temp.vect175.i, double %phi541.i, i32 5
  %temp.vect177.i = insertelement <16 x double> %temp.vect176.i, double %phi542.i, i32 6
  %temp.vect178.i = insertelement <16 x double> %temp.vect177.i, double %phi543.i, i32 7
  %temp.vect179.i = insertelement <16 x double> %temp.vect178.i, double %phi544.i, i32 8
  %temp.vect180.i = insertelement <16 x double> %temp.vect179.i, double %phi545.i, i32 9
  %temp.vect181.i = insertelement <16 x double> %temp.vect180.i, double %phi546.i, i32 10
  %temp.vect182.i = insertelement <16 x double> %temp.vect181.i, double %phi547.i, i32 11
  %"&(pSB[currWI].offset)1171.i" = add nuw i64 %CurrSBIndex..0.i, 192
  %"&pSB[currWI].offset1172.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1171.i"
  %CastToValueType1173.i = bitcast i8* %"&pSB[currWI].offset1172.i" to <16 x i32>*
  %loadedValue1174.i = load <16 x i32>* %CastToValueType1173.i, align 64
  %244 = extractelement <16 x i32> %loadedValue1174.i, i32 0
  %temp.vect183.i = insertelement <16 x double> %temp.vect182.i, double %phi548.i, i32 12
  %245 = sext i32 %244 to i64
  %temp.vect184.i = insertelement <16 x double> %temp.vect183.i, double %phi549.i, i32 13
  %246 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 17, i64 %245
  %temp.vect185.i = insertelement <16 x double> %temp.vect184.i, double %phi550.i, i32 14
  %ptrTypeCast170.i = bitcast double addrspace(3)* %246 to <16 x double> addrspace(3)*
  %temp.vect186.i = insertelement <16 x double> %temp.vect185.i, double %phi551.i, i32 15
  store <16 x double> %temp.vect186.i, <16 x double> addrspace(3)* %ptrTypeCast170.i, align 128
  br label %postload553.i

postload553.i:                                    ; preds = %preload552.i, %postload535.i
  %"&(pSB[currWI].offset)1068.i" = add nuw i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset1069.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1068.i"
  %CastToValueType1070.i = bitcast i8* %"&pSB[currWI].offset1069.i" to <16 x i32>*
  %loadedValue1071.i = load <16 x i32>* %CastToValueType1070.i, align 64
  %247 = icmp eq <16 x i32> %loadedValue1071.i, zeroinitializer
  %extract206.i = extractelement <16 x i1> %247, i32 0
  %extract207.i = extractelement <16 x i1> %247, i32 1
  %extract208.i = extractelement <16 x i1> %247, i32 2
  %extract209.i = extractelement <16 x i1> %247, i32 3
  %extract210.i = extractelement <16 x i1> %247, i32 4
  %extract211.i = extractelement <16 x i1> %247, i32 5
  %extract212.i = extractelement <16 x i1> %247, i32 6
  %extract213.i = extractelement <16 x i1> %247, i32 7
  %extract214.i = extractelement <16 x i1> %247, i32 8
  %extract215.i = extractelement <16 x i1> %247, i32 9
  %extract216.i = extractelement <16 x i1> %247, i32 10
  %extract217.i = extractelement <16 x i1> %247, i32 11
  %extract218.i = extractelement <16 x i1> %247, i32 12
  %extract219.i = extractelement <16 x i1> %247, i32 13
  %extract220.i = extractelement <16 x i1> %247, i32 14
  %extract221.i = extractelement <16 x i1> %247, i32 15
  %248 = add nsw i32 %56, 1
  %temp188.i = insertelement <16 x i32> undef, i32 %248, i32 0
  %vector189.i = shufflevector <16 x i32> %temp188.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %249 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %250 = mul nsw <16 x i32> %249, %vector189.i
  %251 = add nsw <16 x i32> %58, %250
  %252 = extractelement <16 x i32> %251, i32 1
  %253 = sext i32 %252 to i64
  %254 = getelementptr inbounds double addrspace(1)* %1, i64 %253
  %255 = extractelement <16 x i32> %251, i32 2
  %256 = sext i32 %255 to i64
  %257 = getelementptr inbounds double addrspace(1)* %1, i64 %256
  %258 = extractelement <16 x i32> %251, i32 3
  %259 = sext i32 %258 to i64
  %260 = getelementptr inbounds double addrspace(1)* %1, i64 %259
  %261 = extractelement <16 x i32> %251, i32 4
  %262 = sext i32 %261 to i64
  %263 = getelementptr inbounds double addrspace(1)* %1, i64 %262
  %264 = extractelement <16 x i32> %251, i32 5
  %265 = sext i32 %264 to i64
  %266 = getelementptr inbounds double addrspace(1)* %1, i64 %265
  %267 = extractelement <16 x i32> %251, i32 6
  %268 = sext i32 %267 to i64
  %269 = getelementptr inbounds double addrspace(1)* %1, i64 %268
  %270 = extractelement <16 x i32> %251, i32 7
  %271 = sext i32 %270 to i64
  %272 = getelementptr inbounds double addrspace(1)* %1, i64 %271
  %273 = extractelement <16 x i32> %251, i32 8
  %274 = sext i32 %273 to i64
  %275 = getelementptr inbounds double addrspace(1)* %1, i64 %274
  %276 = extractelement <16 x i32> %251, i32 9
  %277 = sext i32 %276 to i64
  %278 = getelementptr inbounds double addrspace(1)* %1, i64 %277
  %279 = extractelement <16 x i32> %251, i32 10
  %280 = sext i32 %279 to i64
  %281 = getelementptr inbounds double addrspace(1)* %1, i64 %280
  %282 = extractelement <16 x i32> %251, i32 11
  %283 = sext i32 %282 to i64
  %284 = getelementptr inbounds double addrspace(1)* %1, i64 %283
  %285 = extractelement <16 x i32> %251, i32 12
  %286 = sext i32 %285 to i64
  %287 = getelementptr inbounds double addrspace(1)* %1, i64 %286
  %288 = extractelement <16 x i32> %251, i32 13
  %289 = sext i32 %288 to i64
  %290 = getelementptr inbounds double addrspace(1)* %1, i64 %289
  %291 = extractelement <16 x i32> %251, i32 14
  %292 = sext i32 %291 to i64
  %293 = getelementptr inbounds double addrspace(1)* %1, i64 %292
  %294 = extractelement <16 x i32> %251, i32 15
  %295 = sext i32 %294 to i64
  %296 = getelementptr inbounds double addrspace(1)* %1, i64 %295
  br i1 %extract206.i, label %preload554.i, label %postload555.i

preload554.i:                                     ; preds = %postload553.i
  %297 = extractelement <16 x i32> %251, i32 0
  %298 = sext i32 %297 to i64
  %299 = getelementptr inbounds double addrspace(1)* %1, i64 %298
  %masked_load421.i = load double addrspace(1)* %299, align 8
  br label %postload555.i

postload555.i:                                    ; preds = %preload554.i, %postload553.i
  %phi556.i = phi double [ undef, %postload553.i ], [ %masked_load421.i, %preload554.i ]
  br i1 %extract207.i, label %preload564.i, label %postload565.i

preload564.i:                                     ; preds = %postload555.i
  %masked_load422.i = load double addrspace(1)* %254, align 8
  br label %postload565.i

postload565.i:                                    ; preds = %preload564.i, %postload555.i
  %phi566.i = phi double [ undef, %postload555.i ], [ %masked_load422.i, %preload564.i ]
  br i1 %extract208.i, label %preload574.i, label %postload575.i

preload574.i:                                     ; preds = %postload565.i
  %masked_load423.i = load double addrspace(1)* %257, align 8
  br label %postload575.i

postload575.i:                                    ; preds = %preload574.i, %postload565.i
  %phi576.i = phi double [ undef, %postload565.i ], [ %masked_load423.i, %preload574.i ]
  br i1 %extract209.i, label %preload584.i, label %postload585.i

preload584.i:                                     ; preds = %postload575.i
  %masked_load424.i = load double addrspace(1)* %260, align 8
  br label %postload585.i

postload585.i:                                    ; preds = %preload584.i, %postload575.i
  %phi586.i = phi double [ undef, %postload575.i ], [ %masked_load424.i, %preload584.i ]
  br i1 %extract210.i, label %preload594.i, label %postload595.i

preload594.i:                                     ; preds = %postload585.i
  %masked_load425.i = load double addrspace(1)* %263, align 8
  br label %postload595.i

postload595.i:                                    ; preds = %preload594.i, %postload585.i
  %phi596.i = phi double [ undef, %postload585.i ], [ %masked_load425.i, %preload594.i ]
  br i1 %extract211.i, label %preload604.i, label %postload605.i

preload604.i:                                     ; preds = %postload595.i
  %masked_load426.i = load double addrspace(1)* %266, align 8
  br label %postload605.i

postload605.i:                                    ; preds = %preload604.i, %postload595.i
  %phi606.i = phi double [ undef, %postload595.i ], [ %masked_load426.i, %preload604.i ]
  br i1 %extract212.i, label %preload614.i, label %postload615.i

preload614.i:                                     ; preds = %postload605.i
  %masked_load427.i = load double addrspace(1)* %269, align 8
  br label %postload615.i

postload615.i:                                    ; preds = %preload614.i, %postload605.i
  %phi616.i = phi double [ undef, %postload605.i ], [ %masked_load427.i, %preload614.i ]
  br i1 %extract213.i, label %preload624.i, label %postload625.i

preload624.i:                                     ; preds = %postload615.i
  %masked_load428.i = load double addrspace(1)* %272, align 8
  br label %postload625.i

postload625.i:                                    ; preds = %preload624.i, %postload615.i
  %phi626.i = phi double [ undef, %postload615.i ], [ %masked_load428.i, %preload624.i ]
  br i1 %extract214.i, label %preload634.i, label %postload635.i

preload634.i:                                     ; preds = %postload625.i
  %masked_load429.i = load double addrspace(1)* %275, align 8
  br label %postload635.i

postload635.i:                                    ; preds = %preload634.i, %postload625.i
  %phi636.i = phi double [ undef, %postload625.i ], [ %masked_load429.i, %preload634.i ]
  br i1 %extract215.i, label %preload644.i, label %postload645.i

preload644.i:                                     ; preds = %postload635.i
  %masked_load430.i = load double addrspace(1)* %278, align 8
  br label %postload645.i

postload645.i:                                    ; preds = %preload644.i, %postload635.i
  %phi646.i = phi double [ undef, %postload635.i ], [ %masked_load430.i, %preload644.i ]
  br i1 %extract216.i, label %preload654.i, label %postload655.i

preload654.i:                                     ; preds = %postload645.i
  %masked_load431.i = load double addrspace(1)* %281, align 8
  br label %postload655.i

postload655.i:                                    ; preds = %preload654.i, %postload645.i
  %phi656.i = phi double [ undef, %postload645.i ], [ %masked_load431.i, %preload654.i ]
  br i1 %extract217.i, label %preload664.i, label %postload665.i

preload664.i:                                     ; preds = %postload655.i
  %masked_load432.i = load double addrspace(1)* %284, align 8
  br label %postload665.i

postload665.i:                                    ; preds = %preload664.i, %postload655.i
  %phi666.i = phi double [ undef, %postload655.i ], [ %masked_load432.i, %preload664.i ]
  br i1 %extract218.i, label %preload674.i, label %postload675.i

preload674.i:                                     ; preds = %postload665.i
  %masked_load433.i = load double addrspace(1)* %287, align 8
  br label %postload675.i

postload675.i:                                    ; preds = %preload674.i, %postload665.i
  %phi676.i = phi double [ undef, %postload665.i ], [ %masked_load433.i, %preload674.i ]
  br i1 %extract219.i, label %preload684.i, label %postload685.i

preload684.i:                                     ; preds = %postload675.i
  %masked_load434.i = load double addrspace(1)* %290, align 8
  br label %postload685.i

postload685.i:                                    ; preds = %preload684.i, %postload675.i
  %phi686.i = phi double [ undef, %postload675.i ], [ %masked_load434.i, %preload684.i ]
  br i1 %extract220.i, label %preload694.i, label %postload695.i

preload694.i:                                     ; preds = %postload685.i
  %masked_load435.i = load double addrspace(1)* %293, align 8
  br label %postload695.i

postload695.i:                                    ; preds = %preload694.i, %postload685.i
  %phi696.i = phi double [ undef, %postload685.i ], [ %masked_load435.i, %preload694.i ]
  br i1 %extract221.i, label %preload704.i, label %postload705.i

preload704.i:                                     ; preds = %postload695.i
  %masked_load436.i = load double addrspace(1)* %296, align 8
  br label %postload705.i

postload705.i:                                    ; preds = %preload704.i, %postload695.i
  %phi706.i = phi double [ undef, %postload695.i ], [ %masked_load436.i, %preload704.i ]
  %"&(pSB[currWI].offset)1200.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1201.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1200.i"
  %CastToValueType1202.i = bitcast i8* %"&pSB[currWI].offset1201.i" to i64*
  %loadedValue1203.i = load i64* %CastToValueType1202.i, align 8
  %300 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1203.i, i64 0
  br i1 %extract206.i, label %preload557.i, label %postload558.i

preload557.i:                                     ; preds = %postload705.i
  store double %phi556.i, double addrspace(3)* %300, align 8
  br label %postload558.i

postload558.i:                                    ; preds = %preload557.i, %postload705.i
  br i1 %extract207.i, label %preload567.i, label %postload568.i

preload567.i:                                     ; preds = %postload558.i
  store double %phi566.i, double addrspace(3)* %300, align 8
  br label %postload568.i

postload568.i:                                    ; preds = %preload567.i, %postload558.i
  br i1 %extract208.i, label %preload577.i, label %postload578.i

preload577.i:                                     ; preds = %postload568.i
  store double %phi576.i, double addrspace(3)* %300, align 8
  br label %postload578.i

postload578.i:                                    ; preds = %preload577.i, %postload568.i
  br i1 %extract209.i, label %preload587.i, label %postload588.i

preload587.i:                                     ; preds = %postload578.i
  store double %phi586.i, double addrspace(3)* %300, align 8
  br label %postload588.i

postload588.i:                                    ; preds = %preload587.i, %postload578.i
  br i1 %extract210.i, label %preload597.i, label %postload598.i

preload597.i:                                     ; preds = %postload588.i
  store double %phi596.i, double addrspace(3)* %300, align 8
  br label %postload598.i

postload598.i:                                    ; preds = %preload597.i, %postload588.i
  br i1 %extract211.i, label %preload607.i, label %postload608.i

preload607.i:                                     ; preds = %postload598.i
  store double %phi606.i, double addrspace(3)* %300, align 8
  br label %postload608.i

postload608.i:                                    ; preds = %preload607.i, %postload598.i
  br i1 %extract212.i, label %preload617.i, label %postload618.i

preload617.i:                                     ; preds = %postload608.i
  store double %phi616.i, double addrspace(3)* %300, align 8
  br label %postload618.i

postload618.i:                                    ; preds = %preload617.i, %postload608.i
  br i1 %extract213.i, label %preload627.i, label %postload628.i

preload627.i:                                     ; preds = %postload618.i
  store double %phi626.i, double addrspace(3)* %300, align 8
  br label %postload628.i

postload628.i:                                    ; preds = %preload627.i, %postload618.i
  br i1 %extract214.i, label %preload637.i, label %postload638.i

preload637.i:                                     ; preds = %postload628.i
  store double %phi636.i, double addrspace(3)* %300, align 8
  br label %postload638.i

postload638.i:                                    ; preds = %preload637.i, %postload628.i
  br i1 %extract215.i, label %preload647.i, label %postload648.i

preload647.i:                                     ; preds = %postload638.i
  store double %phi646.i, double addrspace(3)* %300, align 8
  br label %postload648.i

postload648.i:                                    ; preds = %preload647.i, %postload638.i
  br i1 %extract216.i, label %preload657.i, label %postload658.i

preload657.i:                                     ; preds = %postload648.i
  store double %phi656.i, double addrspace(3)* %300, align 8
  br label %postload658.i

postload658.i:                                    ; preds = %preload657.i, %postload648.i
  br i1 %extract217.i, label %preload667.i, label %postload668.i

preload667.i:                                     ; preds = %postload658.i
  store double %phi666.i, double addrspace(3)* %300, align 8
  br label %postload668.i

postload668.i:                                    ; preds = %preload667.i, %postload658.i
  br i1 %extract218.i, label %preload677.i, label %postload678.i

preload677.i:                                     ; preds = %postload668.i
  store double %phi676.i, double addrspace(3)* %300, align 8
  br label %postload678.i

postload678.i:                                    ; preds = %preload677.i, %postload668.i
  br i1 %extract219.i, label %preload687.i, label %postload688.i

preload687.i:                                     ; preds = %postload678.i
  store double %phi686.i, double addrspace(3)* %300, align 8
  br label %postload688.i

postload688.i:                                    ; preds = %preload687.i, %postload678.i
  br i1 %extract220.i, label %preload697.i, label %postload698.i

preload697.i:                                     ; preds = %postload688.i
  store double %phi696.i, double addrspace(3)* %300, align 8
  br label %postload698.i

postload698.i:                                    ; preds = %preload697.i, %postload688.i
  br i1 %extract221.i, label %preload707.i, label %postload708.i

preload707.i:                                     ; preds = %postload698.i
  store double %phi706.i, double addrspace(3)* %300, align 8
  br label %postload708.i

postload708.i:                                    ; preds = %preload707.i, %postload698.i
  %301 = add nsw i32 %56, 1
  %temp222.i = insertelement <16 x i32> undef, i32 %301, i32 0
  %vector223.i = shufflevector <16 x i32> %temp222.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %302 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %303 = mul nsw <16 x i32> %302, %vector223.i
  %304 = add nsw <16 x i32> %58, <i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17>
  %305 = add nsw <16 x i32> %304, %303
  %306 = extractelement <16 x i32> %305, i32 1
  %307 = sext i32 %306 to i64
  %308 = getelementptr inbounds double addrspace(1)* %1, i64 %307
  %309 = extractelement <16 x i32> %305, i32 2
  %310 = sext i32 %309 to i64
  %311 = getelementptr inbounds double addrspace(1)* %1, i64 %310
  %312 = extractelement <16 x i32> %305, i32 3
  %313 = sext i32 %312 to i64
  %314 = getelementptr inbounds double addrspace(1)* %1, i64 %313
  %315 = extractelement <16 x i32> %305, i32 4
  %316 = sext i32 %315 to i64
  %317 = getelementptr inbounds double addrspace(1)* %1, i64 %316
  %318 = extractelement <16 x i32> %305, i32 5
  %319 = sext i32 %318 to i64
  %320 = getelementptr inbounds double addrspace(1)* %1, i64 %319
  %321 = extractelement <16 x i32> %305, i32 6
  %322 = sext i32 %321 to i64
  %323 = getelementptr inbounds double addrspace(1)* %1, i64 %322
  %324 = extractelement <16 x i32> %305, i32 7
  %325 = sext i32 %324 to i64
  %326 = getelementptr inbounds double addrspace(1)* %1, i64 %325
  %327 = extractelement <16 x i32> %305, i32 8
  %328 = sext i32 %327 to i64
  %329 = getelementptr inbounds double addrspace(1)* %1, i64 %328
  %330 = extractelement <16 x i32> %305, i32 9
  %331 = sext i32 %330 to i64
  %332 = getelementptr inbounds double addrspace(1)* %1, i64 %331
  %333 = extractelement <16 x i32> %305, i32 10
  %334 = sext i32 %333 to i64
  %335 = getelementptr inbounds double addrspace(1)* %1, i64 %334
  %336 = extractelement <16 x i32> %305, i32 11
  %337 = sext i32 %336 to i64
  %338 = getelementptr inbounds double addrspace(1)* %1, i64 %337
  %339 = extractelement <16 x i32> %305, i32 12
  %340 = sext i32 %339 to i64
  %341 = getelementptr inbounds double addrspace(1)* %1, i64 %340
  %342 = extractelement <16 x i32> %305, i32 13
  %343 = sext i32 %342 to i64
  %344 = getelementptr inbounds double addrspace(1)* %1, i64 %343
  %345 = extractelement <16 x i32> %305, i32 14
  %346 = sext i32 %345 to i64
  %347 = getelementptr inbounds double addrspace(1)* %1, i64 %346
  %348 = extractelement <16 x i32> %305, i32 15
  %349 = sext i32 %348 to i64
  %350 = getelementptr inbounds double addrspace(1)* %1, i64 %349
  br i1 %extract206.i, label %preload559.i, label %postload560.i

preload559.i:                                     ; preds = %postload708.i
  %351 = extractelement <16 x i32> %305, i32 0
  %352 = sext i32 %351 to i64
  %353 = getelementptr inbounds double addrspace(1)* %1, i64 %352
  %masked_load437.i = load double addrspace(1)* %353, align 8
  br label %postload560.i

postload560.i:                                    ; preds = %preload559.i, %postload708.i
  %phi561.i = phi double [ undef, %postload708.i ], [ %masked_load437.i, %preload559.i ]
  br i1 %extract207.i, label %preload569.i, label %postload570.i

preload569.i:                                     ; preds = %postload560.i
  %masked_load438.i = load double addrspace(1)* %308, align 8
  br label %postload570.i

postload570.i:                                    ; preds = %preload569.i, %postload560.i
  %phi571.i = phi double [ undef, %postload560.i ], [ %masked_load438.i, %preload569.i ]
  br i1 %extract208.i, label %preload579.i, label %postload580.i

preload579.i:                                     ; preds = %postload570.i
  %masked_load439.i = load double addrspace(1)* %311, align 8
  br label %postload580.i

postload580.i:                                    ; preds = %preload579.i, %postload570.i
  %phi581.i = phi double [ undef, %postload570.i ], [ %masked_load439.i, %preload579.i ]
  br i1 %extract209.i, label %preload589.i, label %postload590.i

preload589.i:                                     ; preds = %postload580.i
  %masked_load440.i = load double addrspace(1)* %314, align 8
  br label %postload590.i

postload590.i:                                    ; preds = %preload589.i, %postload580.i
  %phi591.i = phi double [ undef, %postload580.i ], [ %masked_load440.i, %preload589.i ]
  br i1 %extract210.i, label %preload599.i, label %postload600.i

preload599.i:                                     ; preds = %postload590.i
  %masked_load441.i = load double addrspace(1)* %317, align 8
  br label %postload600.i

postload600.i:                                    ; preds = %preload599.i, %postload590.i
  %phi601.i = phi double [ undef, %postload590.i ], [ %masked_load441.i, %preload599.i ]
  br i1 %extract211.i, label %preload609.i, label %postload610.i

preload609.i:                                     ; preds = %postload600.i
  %masked_load442.i = load double addrspace(1)* %320, align 8
  br label %postload610.i

postload610.i:                                    ; preds = %preload609.i, %postload600.i
  %phi611.i = phi double [ undef, %postload600.i ], [ %masked_load442.i, %preload609.i ]
  br i1 %extract212.i, label %preload619.i, label %postload620.i

preload619.i:                                     ; preds = %postload610.i
  %masked_load443.i = load double addrspace(1)* %323, align 8
  br label %postload620.i

postload620.i:                                    ; preds = %preload619.i, %postload610.i
  %phi621.i = phi double [ undef, %postload610.i ], [ %masked_load443.i, %preload619.i ]
  br i1 %extract213.i, label %preload629.i, label %postload630.i

preload629.i:                                     ; preds = %postload620.i
  %masked_load444.i = load double addrspace(1)* %326, align 8
  br label %postload630.i

postload630.i:                                    ; preds = %preload629.i, %postload620.i
  %phi631.i = phi double [ undef, %postload620.i ], [ %masked_load444.i, %preload629.i ]
  br i1 %extract214.i, label %preload639.i, label %postload640.i

preload639.i:                                     ; preds = %postload630.i
  %masked_load445.i = load double addrspace(1)* %329, align 8
  br label %postload640.i

postload640.i:                                    ; preds = %preload639.i, %postload630.i
  %phi641.i = phi double [ undef, %postload630.i ], [ %masked_load445.i, %preload639.i ]
  br i1 %extract215.i, label %preload649.i, label %postload650.i

preload649.i:                                     ; preds = %postload640.i
  %masked_load446.i = load double addrspace(1)* %332, align 8
  br label %postload650.i

postload650.i:                                    ; preds = %preload649.i, %postload640.i
  %phi651.i = phi double [ undef, %postload640.i ], [ %masked_load446.i, %preload649.i ]
  br i1 %extract216.i, label %preload659.i, label %postload660.i

preload659.i:                                     ; preds = %postload650.i
  %masked_load447.i = load double addrspace(1)* %335, align 8
  br label %postload660.i

postload660.i:                                    ; preds = %preload659.i, %postload650.i
  %phi661.i = phi double [ undef, %postload650.i ], [ %masked_load447.i, %preload659.i ]
  br i1 %extract217.i, label %preload669.i, label %postload670.i

preload669.i:                                     ; preds = %postload660.i
  %masked_load448.i = load double addrspace(1)* %338, align 8
  br label %postload670.i

postload670.i:                                    ; preds = %preload669.i, %postload660.i
  %phi671.i = phi double [ undef, %postload660.i ], [ %masked_load448.i, %preload669.i ]
  br i1 %extract218.i, label %preload679.i, label %postload680.i

preload679.i:                                     ; preds = %postload670.i
  %masked_load449.i = load double addrspace(1)* %341, align 8
  br label %postload680.i

postload680.i:                                    ; preds = %preload679.i, %postload670.i
  %phi681.i = phi double [ undef, %postload670.i ], [ %masked_load449.i, %preload679.i ]
  br i1 %extract219.i, label %preload689.i, label %postload690.i

preload689.i:                                     ; preds = %postload680.i
  %masked_load450.i = load double addrspace(1)* %344, align 8
  br label %postload690.i

postload690.i:                                    ; preds = %preload689.i, %postload680.i
  %phi691.i = phi double [ undef, %postload680.i ], [ %masked_load450.i, %preload689.i ]
  br i1 %extract220.i, label %preload699.i, label %postload700.i

preload699.i:                                     ; preds = %postload690.i
  %masked_load451.i = load double addrspace(1)* %347, align 8
  br label %postload700.i

postload700.i:                                    ; preds = %preload699.i, %postload690.i
  %phi701.i = phi double [ undef, %postload690.i ], [ %masked_load451.i, %preload699.i ]
  br i1 %extract221.i, label %preload709.i, label %postload710.i

preload709.i:                                     ; preds = %postload700.i
  %masked_load452.i = load double addrspace(1)* %350, align 8
  br label %postload710.i

postload710.i:                                    ; preds = %preload709.i, %postload700.i
  %phi711.i = phi double [ undef, %postload700.i ], [ %masked_load452.i, %preload709.i ]
  %"&(pSB[currWI].offset)1195.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1196.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1195.i"
  %CastToValueType1197.i = bitcast i8* %"&pSB[currWI].offset1196.i" to i64*
  %loadedValue1198.i = load i64* %CastToValueType1197.i, align 8
  %354 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1198.i, i64 17
  br i1 %extract206.i, label %preload562.i, label %postload563.i

preload562.i:                                     ; preds = %postload710.i
  store double %phi561.i, double addrspace(3)* %354, align 8
  br label %postload563.i

postload563.i:                                    ; preds = %preload562.i, %postload710.i
  br i1 %extract207.i, label %preload572.i, label %postload573.i

preload572.i:                                     ; preds = %postload563.i
  store double %phi571.i, double addrspace(3)* %354, align 8
  br label %postload573.i

postload573.i:                                    ; preds = %preload572.i, %postload563.i
  br i1 %extract208.i, label %preload582.i, label %postload583.i

preload582.i:                                     ; preds = %postload573.i
  store double %phi581.i, double addrspace(3)* %354, align 8
  br label %postload583.i

postload583.i:                                    ; preds = %preload582.i, %postload573.i
  br i1 %extract209.i, label %preload592.i, label %postload593.i

preload592.i:                                     ; preds = %postload583.i
  store double %phi591.i, double addrspace(3)* %354, align 8
  br label %postload593.i

postload593.i:                                    ; preds = %preload592.i, %postload583.i
  br i1 %extract210.i, label %preload602.i, label %postload603.i

preload602.i:                                     ; preds = %postload593.i
  store double %phi601.i, double addrspace(3)* %354, align 8
  br label %postload603.i

postload603.i:                                    ; preds = %preload602.i, %postload593.i
  br i1 %extract211.i, label %preload612.i, label %postload613.i

preload612.i:                                     ; preds = %postload603.i
  store double %phi611.i, double addrspace(3)* %354, align 8
  br label %postload613.i

postload613.i:                                    ; preds = %preload612.i, %postload603.i
  br i1 %extract212.i, label %preload622.i, label %postload623.i

preload622.i:                                     ; preds = %postload613.i
  store double %phi621.i, double addrspace(3)* %354, align 8
  br label %postload623.i

postload623.i:                                    ; preds = %preload622.i, %postload613.i
  br i1 %extract213.i, label %preload632.i, label %postload633.i

preload632.i:                                     ; preds = %postload623.i
  store double %phi631.i, double addrspace(3)* %354, align 8
  br label %postload633.i

postload633.i:                                    ; preds = %preload632.i, %postload623.i
  br i1 %extract214.i, label %preload642.i, label %postload643.i

preload642.i:                                     ; preds = %postload633.i
  store double %phi641.i, double addrspace(3)* %354, align 8
  br label %postload643.i

postload643.i:                                    ; preds = %preload642.i, %postload633.i
  br i1 %extract215.i, label %preload652.i, label %postload653.i

preload652.i:                                     ; preds = %postload643.i
  store double %phi651.i, double addrspace(3)* %354, align 8
  br label %postload653.i

postload653.i:                                    ; preds = %preload652.i, %postload643.i
  br i1 %extract216.i, label %preload662.i, label %postload663.i

preload662.i:                                     ; preds = %postload653.i
  store double %phi661.i, double addrspace(3)* %354, align 8
  br label %postload663.i

postload663.i:                                    ; preds = %preload662.i, %postload653.i
  br i1 %extract217.i, label %preload672.i, label %postload673.i

preload672.i:                                     ; preds = %postload663.i
  store double %phi671.i, double addrspace(3)* %354, align 8
  br label %postload673.i

postload673.i:                                    ; preds = %preload672.i, %postload663.i
  br i1 %extract218.i, label %preload682.i, label %postload683.i

preload682.i:                                     ; preds = %postload673.i
  store double %phi681.i, double addrspace(3)* %354, align 8
  br label %postload683.i

postload683.i:                                    ; preds = %preload682.i, %postload673.i
  br i1 %extract219.i, label %preload692.i, label %postload693.i

preload692.i:                                     ; preds = %postload683.i
  store double %phi691.i, double addrspace(3)* %354, align 8
  br label %postload693.i

postload693.i:                                    ; preds = %preload692.i, %postload683.i
  br i1 %extract220.i, label %preload702.i, label %postload703.i

preload702.i:                                     ; preds = %postload693.i
  store double %phi701.i, double addrspace(3)* %354, align 8
  br label %postload703.i

postload703.i:                                    ; preds = %preload702.i, %postload693.i
  br i1 %extract221.i, label %preload712.i, label %postload713.i

preload712.i:                                     ; preds = %postload703.i
  store double %phi711.i, double addrspace(3)* %354, align 8
  br label %postload713.i

postload713.i:                                    ; preds = %preload712.i, %postload703.i
  %355 = or <16 x i64> %53, %vector241.i
  %356 = trunc <16 x i64> %355 to <16 x i32>
  %357 = icmp eq <16 x i32> %356, zeroinitializer
  %extract259.i = extractelement <16 x i1> %357, i32 0
  %extract260.i = extractelement <16 x i1> %357, i32 1
  %extract261.i = extractelement <16 x i1> %357, i32 2
  %extract262.i = extractelement <16 x i1> %357, i32 3
  %extract263.i = extractelement <16 x i1> %357, i32 4
  %extract264.i = extractelement <16 x i1> %357, i32 5
  %extract265.i = extractelement <16 x i1> %357, i32 6
  %extract266.i = extractelement <16 x i1> %357, i32 7
  %extract267.i = extractelement <16 x i1> %357, i32 8
  %extract268.i = extractelement <16 x i1> %357, i32 9
  %extract269.i = extractelement <16 x i1> %357, i32 10
  %extract270.i = extractelement <16 x i1> %357, i32 11
  %extract271.i = extractelement <16 x i1> %357, i32 12
  %extract272.i = extractelement <16 x i1> %357, i32 13
  %extract273.i = extractelement <16 x i1> %357, i32 14
  %extract274.i = extractelement <16 x i1> %357, i32 15
  %358 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %359 = mul nsw <16 x i32> %358, %vector118.i
  %360 = add nsw <16 x i32> %58, %359
  %361 = extractelement <16 x i32> %360, i32 1
  %362 = sext i32 %361 to i64
  %363 = getelementptr inbounds double addrspace(1)* %1, i64 %362
  %364 = extractelement <16 x i32> %360, i32 2
  %365 = sext i32 %364 to i64
  %366 = getelementptr inbounds double addrspace(1)* %1, i64 %365
  %367 = extractelement <16 x i32> %360, i32 3
  %368 = sext i32 %367 to i64
  %369 = getelementptr inbounds double addrspace(1)* %1, i64 %368
  %370 = extractelement <16 x i32> %360, i32 4
  %371 = sext i32 %370 to i64
  %372 = getelementptr inbounds double addrspace(1)* %1, i64 %371
  %373 = extractelement <16 x i32> %360, i32 5
  %374 = sext i32 %373 to i64
  %375 = getelementptr inbounds double addrspace(1)* %1, i64 %374
  %376 = extractelement <16 x i32> %360, i32 6
  %377 = sext i32 %376 to i64
  %378 = getelementptr inbounds double addrspace(1)* %1, i64 %377
  %379 = extractelement <16 x i32> %360, i32 7
  %380 = sext i32 %379 to i64
  %381 = getelementptr inbounds double addrspace(1)* %1, i64 %380
  %382 = extractelement <16 x i32> %360, i32 8
  %383 = sext i32 %382 to i64
  %384 = getelementptr inbounds double addrspace(1)* %1, i64 %383
  %385 = extractelement <16 x i32> %360, i32 9
  %386 = sext i32 %385 to i64
  %387 = getelementptr inbounds double addrspace(1)* %1, i64 %386
  %388 = extractelement <16 x i32> %360, i32 10
  %389 = sext i32 %388 to i64
  %390 = getelementptr inbounds double addrspace(1)* %1, i64 %389
  %391 = extractelement <16 x i32> %360, i32 11
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds double addrspace(1)* %1, i64 %392
  %394 = extractelement <16 x i32> %360, i32 12
  %395 = sext i32 %394 to i64
  %396 = getelementptr inbounds double addrspace(1)* %1, i64 %395
  %397 = extractelement <16 x i32> %360, i32 13
  %398 = sext i32 %397 to i64
  %399 = getelementptr inbounds double addrspace(1)* %1, i64 %398
  %400 = extractelement <16 x i32> %360, i32 14
  %401 = sext i32 %400 to i64
  %402 = getelementptr inbounds double addrspace(1)* %1, i64 %401
  %403 = extractelement <16 x i32> %360, i32 15
  %404 = sext i32 %403 to i64
  %405 = getelementptr inbounds double addrspace(1)* %1, i64 %404
  br i1 %extract259.i, label %preload714.i, label %postload715.i

preload714.i:                                     ; preds = %postload713.i
  %406 = extractelement <16 x i32> %360, i32 0
  %407 = sext i32 %406 to i64
  %408 = getelementptr inbounds double addrspace(1)* %1, i64 %407
  %masked_load453.i = load double addrspace(1)* %408, align 8
  br label %postload715.i

postload715.i:                                    ; preds = %preload714.i, %postload713.i
  %phi716.i = phi double [ undef, %postload713.i ], [ %masked_load453.i, %preload714.i ]
  br i1 %extract260.i, label %preload734.i, label %postload735.i

preload734.i:                                     ; preds = %postload715.i
  %masked_load454.i = load double addrspace(1)* %363, align 8
  br label %postload735.i

postload735.i:                                    ; preds = %preload734.i, %postload715.i
  %phi736.i = phi double [ undef, %postload715.i ], [ %masked_load454.i, %preload734.i ]
  br i1 %extract261.i, label %preload754.i, label %postload755.i

preload754.i:                                     ; preds = %postload735.i
  %masked_load455.i = load double addrspace(1)* %366, align 8
  br label %postload755.i

postload755.i:                                    ; preds = %preload754.i, %postload735.i
  %phi756.i = phi double [ undef, %postload735.i ], [ %masked_load455.i, %preload754.i ]
  br i1 %extract262.i, label %preload774.i, label %postload775.i

preload774.i:                                     ; preds = %postload755.i
  %masked_load456.i = load double addrspace(1)* %369, align 8
  br label %postload775.i

postload775.i:                                    ; preds = %preload774.i, %postload755.i
  %phi776.i = phi double [ undef, %postload755.i ], [ %masked_load456.i, %preload774.i ]
  br i1 %extract263.i, label %preload794.i, label %postload795.i

preload794.i:                                     ; preds = %postload775.i
  %masked_load457.i = load double addrspace(1)* %372, align 8
  br label %postload795.i

postload795.i:                                    ; preds = %preload794.i, %postload775.i
  %phi796.i = phi double [ undef, %postload775.i ], [ %masked_load457.i, %preload794.i ]
  br i1 %extract264.i, label %preload814.i, label %postload815.i

preload814.i:                                     ; preds = %postload795.i
  %masked_load458.i = load double addrspace(1)* %375, align 8
  br label %postload815.i

postload815.i:                                    ; preds = %preload814.i, %postload795.i
  %phi816.i = phi double [ undef, %postload795.i ], [ %masked_load458.i, %preload814.i ]
  br i1 %extract265.i, label %preload834.i, label %postload835.i

preload834.i:                                     ; preds = %postload815.i
  %masked_load459.i = load double addrspace(1)* %378, align 8
  br label %postload835.i

postload835.i:                                    ; preds = %preload834.i, %postload815.i
  %phi836.i = phi double [ undef, %postload815.i ], [ %masked_load459.i, %preload834.i ]
  br i1 %extract266.i, label %preload854.i, label %postload855.i

preload854.i:                                     ; preds = %postload835.i
  %masked_load460.i = load double addrspace(1)* %381, align 8
  br label %postload855.i

postload855.i:                                    ; preds = %preload854.i, %postload835.i
  %phi856.i = phi double [ undef, %postload835.i ], [ %masked_load460.i, %preload854.i ]
  br i1 %extract267.i, label %preload874.i, label %postload875.i

preload874.i:                                     ; preds = %postload855.i
  %masked_load461.i = load double addrspace(1)* %384, align 8
  br label %postload875.i

postload875.i:                                    ; preds = %preload874.i, %postload855.i
  %phi876.i = phi double [ undef, %postload855.i ], [ %masked_load461.i, %preload874.i ]
  br i1 %extract268.i, label %preload894.i, label %postload895.i

preload894.i:                                     ; preds = %postload875.i
  %masked_load462.i = load double addrspace(1)* %387, align 8
  br label %postload895.i

postload895.i:                                    ; preds = %preload894.i, %postload875.i
  %phi896.i = phi double [ undef, %postload875.i ], [ %masked_load462.i, %preload894.i ]
  br i1 %extract269.i, label %preload914.i, label %postload915.i

preload914.i:                                     ; preds = %postload895.i
  %masked_load463.i = load double addrspace(1)* %390, align 8
  br label %postload915.i

postload915.i:                                    ; preds = %preload914.i, %postload895.i
  %phi916.i = phi double [ undef, %postload895.i ], [ %masked_load463.i, %preload914.i ]
  br i1 %extract270.i, label %preload934.i, label %postload935.i

preload934.i:                                     ; preds = %postload915.i
  %masked_load464.i = load double addrspace(1)* %393, align 8
  br label %postload935.i

postload935.i:                                    ; preds = %preload934.i, %postload915.i
  %phi936.i = phi double [ undef, %postload915.i ], [ %masked_load464.i, %preload934.i ]
  br i1 %extract271.i, label %preload954.i, label %postload955.i

preload954.i:                                     ; preds = %postload935.i
  %masked_load465.i = load double addrspace(1)* %396, align 8
  br label %postload955.i

postload955.i:                                    ; preds = %preload954.i, %postload935.i
  %phi956.i = phi double [ undef, %postload935.i ], [ %masked_load465.i, %preload954.i ]
  br i1 %extract272.i, label %preload974.i, label %postload975.i

preload974.i:                                     ; preds = %postload955.i
  %masked_load466.i = load double addrspace(1)* %399, align 8
  br label %postload975.i

postload975.i:                                    ; preds = %preload974.i, %postload955.i
  %phi976.i = phi double [ undef, %postload955.i ], [ %masked_load466.i, %preload974.i ]
  br i1 %extract273.i, label %preload994.i, label %postload995.i

preload994.i:                                     ; preds = %postload975.i
  %masked_load467.i = load double addrspace(1)* %402, align 8
  br label %postload995.i

postload995.i:                                    ; preds = %preload994.i, %postload975.i
  %phi996.i = phi double [ undef, %postload975.i ], [ %masked_load467.i, %preload994.i ]
  br i1 %extract274.i, label %preload1014.i, label %postload1015.i

preload1014.i:                                    ; preds = %postload995.i
  %masked_load468.i = load double addrspace(1)* %405, align 8
  br label %postload1015.i

postload1015.i:                                   ; preds = %preload1014.i, %postload995.i
  %phi1016.i = phi double [ undef, %postload995.i ], [ %masked_load468.i, %preload1014.i ]
  br i1 %extract259.i, label %preload717.i, label %postload718.i

preload717.i:                                     ; preds = %postload1015.i
  store double %phi716.i, double addrspace(3)* %33, align 8
  br label %postload718.i

postload718.i:                                    ; preds = %preload717.i, %postload1015.i
  br i1 %extract260.i, label %preload737.i, label %postload738.i

preload737.i:                                     ; preds = %postload718.i
  store double %phi736.i, double addrspace(3)* %33, align 8
  br label %postload738.i

postload738.i:                                    ; preds = %preload737.i, %postload718.i
  br i1 %extract261.i, label %preload757.i, label %postload758.i

preload757.i:                                     ; preds = %postload738.i
  store double %phi756.i, double addrspace(3)* %33, align 8
  br label %postload758.i

postload758.i:                                    ; preds = %preload757.i, %postload738.i
  br i1 %extract262.i, label %preload777.i, label %postload778.i

preload777.i:                                     ; preds = %postload758.i
  store double %phi776.i, double addrspace(3)* %33, align 8
  br label %postload778.i

postload778.i:                                    ; preds = %preload777.i, %postload758.i
  br i1 %extract263.i, label %preload797.i, label %postload798.i

preload797.i:                                     ; preds = %postload778.i
  store double %phi796.i, double addrspace(3)* %33, align 8
  br label %postload798.i

postload798.i:                                    ; preds = %preload797.i, %postload778.i
  br i1 %extract264.i, label %preload817.i, label %postload818.i

preload817.i:                                     ; preds = %postload798.i
  store double %phi816.i, double addrspace(3)* %33, align 8
  br label %postload818.i

postload818.i:                                    ; preds = %preload817.i, %postload798.i
  br i1 %extract265.i, label %preload837.i, label %postload838.i

preload837.i:                                     ; preds = %postload818.i
  store double %phi836.i, double addrspace(3)* %33, align 8
  br label %postload838.i

postload838.i:                                    ; preds = %preload837.i, %postload818.i
  br i1 %extract266.i, label %preload857.i, label %postload858.i

preload857.i:                                     ; preds = %postload838.i
  store double %phi856.i, double addrspace(3)* %33, align 8
  br label %postload858.i

postload858.i:                                    ; preds = %preload857.i, %postload838.i
  br i1 %extract267.i, label %preload877.i, label %postload878.i

preload877.i:                                     ; preds = %postload858.i
  store double %phi876.i, double addrspace(3)* %33, align 8
  br label %postload878.i

postload878.i:                                    ; preds = %preload877.i, %postload858.i
  br i1 %extract268.i, label %preload897.i, label %postload898.i

preload897.i:                                     ; preds = %postload878.i
  store double %phi896.i, double addrspace(3)* %33, align 8
  br label %postload898.i

postload898.i:                                    ; preds = %preload897.i, %postload878.i
  br i1 %extract269.i, label %preload917.i, label %postload918.i

preload917.i:                                     ; preds = %postload898.i
  store double %phi916.i, double addrspace(3)* %33, align 8
  br label %postload918.i

postload918.i:                                    ; preds = %preload917.i, %postload898.i
  br i1 %extract270.i, label %preload937.i, label %postload938.i

preload937.i:                                     ; preds = %postload918.i
  store double %phi936.i, double addrspace(3)* %33, align 8
  br label %postload938.i

postload938.i:                                    ; preds = %preload937.i, %postload918.i
  br i1 %extract271.i, label %preload957.i, label %postload958.i

preload957.i:                                     ; preds = %postload938.i
  store double %phi956.i, double addrspace(3)* %33, align 8
  br label %postload958.i

postload958.i:                                    ; preds = %preload957.i, %postload938.i
  br i1 %extract272.i, label %preload977.i, label %postload978.i

preload977.i:                                     ; preds = %postload958.i
  store double %phi976.i, double addrspace(3)* %33, align 8
  br label %postload978.i

postload978.i:                                    ; preds = %preload977.i, %postload958.i
  br i1 %extract273.i, label %preload997.i, label %postload998.i

preload997.i:                                     ; preds = %postload978.i
  store double %phi996.i, double addrspace(3)* %33, align 8
  br label %postload998.i

postload998.i:                                    ; preds = %preload997.i, %postload978.i
  br i1 %extract274.i, label %preload1017.i, label %postload1018.i

preload1017.i:                                    ; preds = %postload998.i
  store double %phi1016.i, double addrspace(3)* %33, align 8
  br label %postload1018.i

postload1018.i:                                   ; preds = %preload1017.i, %postload998.i
  %409 = add nsw i32 %56, 17
  %temp275.i = insertelement <16 x i32> undef, i32 %409, i32 0
  %vector276.i = shufflevector <16 x i32> %temp275.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %410 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %411 = mul nsw <16 x i32> %410, %vector276.i
  %412 = add nsw <16 x i32> %58, %411
  %413 = extractelement <16 x i32> %412, i32 1
  %414 = sext i32 %413 to i64
  %415 = getelementptr inbounds double addrspace(1)* %1, i64 %414
  %416 = extractelement <16 x i32> %412, i32 2
  %417 = sext i32 %416 to i64
  %418 = getelementptr inbounds double addrspace(1)* %1, i64 %417
  %419 = extractelement <16 x i32> %412, i32 3
  %420 = sext i32 %419 to i64
  %421 = getelementptr inbounds double addrspace(1)* %1, i64 %420
  %422 = extractelement <16 x i32> %412, i32 4
  %423 = sext i32 %422 to i64
  %424 = getelementptr inbounds double addrspace(1)* %1, i64 %423
  %425 = extractelement <16 x i32> %412, i32 5
  %426 = sext i32 %425 to i64
  %427 = getelementptr inbounds double addrspace(1)* %1, i64 %426
  %428 = extractelement <16 x i32> %412, i32 6
  %429 = sext i32 %428 to i64
  %430 = getelementptr inbounds double addrspace(1)* %1, i64 %429
  %431 = extractelement <16 x i32> %412, i32 7
  %432 = sext i32 %431 to i64
  %433 = getelementptr inbounds double addrspace(1)* %1, i64 %432
  %434 = extractelement <16 x i32> %412, i32 8
  %435 = sext i32 %434 to i64
  %436 = getelementptr inbounds double addrspace(1)* %1, i64 %435
  %437 = extractelement <16 x i32> %412, i32 9
  %438 = sext i32 %437 to i64
  %439 = getelementptr inbounds double addrspace(1)* %1, i64 %438
  %440 = extractelement <16 x i32> %412, i32 10
  %441 = sext i32 %440 to i64
  %442 = getelementptr inbounds double addrspace(1)* %1, i64 %441
  %443 = extractelement <16 x i32> %412, i32 11
  %444 = sext i32 %443 to i64
  %445 = getelementptr inbounds double addrspace(1)* %1, i64 %444
  %446 = extractelement <16 x i32> %412, i32 12
  %447 = sext i32 %446 to i64
  %448 = getelementptr inbounds double addrspace(1)* %1, i64 %447
  %449 = extractelement <16 x i32> %412, i32 13
  %450 = sext i32 %449 to i64
  %451 = getelementptr inbounds double addrspace(1)* %1, i64 %450
  %452 = extractelement <16 x i32> %412, i32 14
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds double addrspace(1)* %1, i64 %453
  %455 = extractelement <16 x i32> %412, i32 15
  %456 = sext i32 %455 to i64
  %457 = getelementptr inbounds double addrspace(1)* %1, i64 %456
  br i1 %extract259.i, label %preload719.i, label %postload720.i

preload719.i:                                     ; preds = %postload1018.i
  %458 = extractelement <16 x i32> %412, i32 0
  %459 = sext i32 %458 to i64
  %460 = getelementptr inbounds double addrspace(1)* %1, i64 %459
  %masked_load469.i = load double addrspace(1)* %460, align 8
  br label %postload720.i

postload720.i:                                    ; preds = %preload719.i, %postload1018.i
  %phi721.i = phi double [ undef, %postload1018.i ], [ %masked_load469.i, %preload719.i ]
  br i1 %extract260.i, label %preload739.i, label %postload740.i

preload739.i:                                     ; preds = %postload720.i
  %masked_load470.i = load double addrspace(1)* %415, align 8
  br label %postload740.i

postload740.i:                                    ; preds = %preload739.i, %postload720.i
  %phi741.i = phi double [ undef, %postload720.i ], [ %masked_load470.i, %preload739.i ]
  br i1 %extract261.i, label %preload759.i, label %postload760.i

preload759.i:                                     ; preds = %postload740.i
  %masked_load471.i = load double addrspace(1)* %418, align 8
  br label %postload760.i

postload760.i:                                    ; preds = %preload759.i, %postload740.i
  %phi761.i = phi double [ undef, %postload740.i ], [ %masked_load471.i, %preload759.i ]
  br i1 %extract262.i, label %preload779.i, label %postload780.i

preload779.i:                                     ; preds = %postload760.i
  %masked_load472.i = load double addrspace(1)* %421, align 8
  br label %postload780.i

postload780.i:                                    ; preds = %preload779.i, %postload760.i
  %phi781.i = phi double [ undef, %postload760.i ], [ %masked_load472.i, %preload779.i ]
  br i1 %extract263.i, label %preload799.i, label %postload800.i

preload799.i:                                     ; preds = %postload780.i
  %masked_load473.i = load double addrspace(1)* %424, align 8
  br label %postload800.i

postload800.i:                                    ; preds = %preload799.i, %postload780.i
  %phi801.i = phi double [ undef, %postload780.i ], [ %masked_load473.i, %preload799.i ]
  br i1 %extract264.i, label %preload819.i, label %postload820.i

preload819.i:                                     ; preds = %postload800.i
  %masked_load474.i = load double addrspace(1)* %427, align 8
  br label %postload820.i

postload820.i:                                    ; preds = %preload819.i, %postload800.i
  %phi821.i = phi double [ undef, %postload800.i ], [ %masked_load474.i, %preload819.i ]
  br i1 %extract265.i, label %preload839.i, label %postload840.i

preload839.i:                                     ; preds = %postload820.i
  %masked_load475.i = load double addrspace(1)* %430, align 8
  br label %postload840.i

postload840.i:                                    ; preds = %preload839.i, %postload820.i
  %phi841.i = phi double [ undef, %postload820.i ], [ %masked_load475.i, %preload839.i ]
  br i1 %extract266.i, label %preload859.i, label %postload860.i

preload859.i:                                     ; preds = %postload840.i
  %masked_load476.i = load double addrspace(1)* %433, align 8
  br label %postload860.i

postload860.i:                                    ; preds = %preload859.i, %postload840.i
  %phi861.i = phi double [ undef, %postload840.i ], [ %masked_load476.i, %preload859.i ]
  br i1 %extract267.i, label %preload879.i, label %postload880.i

preload879.i:                                     ; preds = %postload860.i
  %masked_load477.i = load double addrspace(1)* %436, align 8
  br label %postload880.i

postload880.i:                                    ; preds = %preload879.i, %postload860.i
  %phi881.i = phi double [ undef, %postload860.i ], [ %masked_load477.i, %preload879.i ]
  br i1 %extract268.i, label %preload899.i, label %postload900.i

preload899.i:                                     ; preds = %postload880.i
  %masked_load478.i = load double addrspace(1)* %439, align 8
  br label %postload900.i

postload900.i:                                    ; preds = %preload899.i, %postload880.i
  %phi901.i = phi double [ undef, %postload880.i ], [ %masked_load478.i, %preload899.i ]
  br i1 %extract269.i, label %preload919.i, label %postload920.i

preload919.i:                                     ; preds = %postload900.i
  %masked_load479.i = load double addrspace(1)* %442, align 8
  br label %postload920.i

postload920.i:                                    ; preds = %preload919.i, %postload900.i
  %phi921.i = phi double [ undef, %postload900.i ], [ %masked_load479.i, %preload919.i ]
  br i1 %extract270.i, label %preload939.i, label %postload940.i

preload939.i:                                     ; preds = %postload920.i
  %masked_load480.i = load double addrspace(1)* %445, align 8
  br label %postload940.i

postload940.i:                                    ; preds = %preload939.i, %postload920.i
  %phi941.i = phi double [ undef, %postload920.i ], [ %masked_load480.i, %preload939.i ]
  br i1 %extract271.i, label %preload959.i, label %postload960.i

preload959.i:                                     ; preds = %postload940.i
  %masked_load481.i = load double addrspace(1)* %448, align 8
  br label %postload960.i

postload960.i:                                    ; preds = %preload959.i, %postload940.i
  %phi961.i = phi double [ undef, %postload940.i ], [ %masked_load481.i, %preload959.i ]
  br i1 %extract272.i, label %preload979.i, label %postload980.i

preload979.i:                                     ; preds = %postload960.i
  %masked_load482.i = load double addrspace(1)* %451, align 8
  br label %postload980.i

postload980.i:                                    ; preds = %preload979.i, %postload960.i
  %phi981.i = phi double [ undef, %postload960.i ], [ %masked_load482.i, %preload979.i ]
  br i1 %extract273.i, label %preload999.i, label %postload1000.i

preload999.i:                                     ; preds = %postload980.i
  %masked_load483.i = load double addrspace(1)* %454, align 8
  br label %postload1000.i

postload1000.i:                                   ; preds = %preload999.i, %postload980.i
  %phi1001.i = phi double [ undef, %postload980.i ], [ %masked_load483.i, %preload999.i ]
  br i1 %extract274.i, label %preload1019.i, label %postload1020.i

preload1019.i:                                    ; preds = %postload1000.i
  %masked_load484.i = load double addrspace(1)* %457, align 8
  br label %postload1020.i

postload1020.i:                                   ; preds = %preload1019.i, %postload1000.i
  %phi1021.i = phi double [ undef, %postload1000.i ], [ %masked_load484.i, %preload1019.i ]
  br i1 %extract259.i, label %preload722.i, label %postload723.i

preload722.i:                                     ; preds = %postload1020.i
  store double %phi721.i, double addrspace(3)* %35, align 8
  br label %postload723.i

postload723.i:                                    ; preds = %preload722.i, %postload1020.i
  br i1 %extract260.i, label %preload742.i, label %postload743.i

preload742.i:                                     ; preds = %postload723.i
  store double %phi741.i, double addrspace(3)* %35, align 8
  br label %postload743.i

postload743.i:                                    ; preds = %preload742.i, %postload723.i
  br i1 %extract261.i, label %preload762.i, label %postload763.i

preload762.i:                                     ; preds = %postload743.i
  store double %phi761.i, double addrspace(3)* %35, align 8
  br label %postload763.i

postload763.i:                                    ; preds = %preload762.i, %postload743.i
  br i1 %extract262.i, label %preload782.i, label %postload783.i

preload782.i:                                     ; preds = %postload763.i
  store double %phi781.i, double addrspace(3)* %35, align 8
  br label %postload783.i

postload783.i:                                    ; preds = %preload782.i, %postload763.i
  br i1 %extract263.i, label %preload802.i, label %postload803.i

preload802.i:                                     ; preds = %postload783.i
  store double %phi801.i, double addrspace(3)* %35, align 8
  br label %postload803.i

postload803.i:                                    ; preds = %preload802.i, %postload783.i
  br i1 %extract264.i, label %preload822.i, label %postload823.i

preload822.i:                                     ; preds = %postload803.i
  store double %phi821.i, double addrspace(3)* %35, align 8
  br label %postload823.i

postload823.i:                                    ; preds = %preload822.i, %postload803.i
  br i1 %extract265.i, label %preload842.i, label %postload843.i

preload842.i:                                     ; preds = %postload823.i
  store double %phi841.i, double addrspace(3)* %35, align 8
  br label %postload843.i

postload843.i:                                    ; preds = %preload842.i, %postload823.i
  br i1 %extract266.i, label %preload862.i, label %postload863.i

preload862.i:                                     ; preds = %postload843.i
  store double %phi861.i, double addrspace(3)* %35, align 8
  br label %postload863.i

postload863.i:                                    ; preds = %preload862.i, %postload843.i
  br i1 %extract267.i, label %preload882.i, label %postload883.i

preload882.i:                                     ; preds = %postload863.i
  store double %phi881.i, double addrspace(3)* %35, align 8
  br label %postload883.i

postload883.i:                                    ; preds = %preload882.i, %postload863.i
  br i1 %extract268.i, label %preload902.i, label %postload903.i

preload902.i:                                     ; preds = %postload883.i
  store double %phi901.i, double addrspace(3)* %35, align 8
  br label %postload903.i

postload903.i:                                    ; preds = %preload902.i, %postload883.i
  br i1 %extract269.i, label %preload922.i, label %postload923.i

preload922.i:                                     ; preds = %postload903.i
  store double %phi921.i, double addrspace(3)* %35, align 8
  br label %postload923.i

postload923.i:                                    ; preds = %preload922.i, %postload903.i
  br i1 %extract270.i, label %preload942.i, label %postload943.i

preload942.i:                                     ; preds = %postload923.i
  store double %phi941.i, double addrspace(3)* %35, align 8
  br label %postload943.i

postload943.i:                                    ; preds = %preload942.i, %postload923.i
  br i1 %extract271.i, label %preload962.i, label %postload963.i

preload962.i:                                     ; preds = %postload943.i
  store double %phi961.i, double addrspace(3)* %35, align 8
  br label %postload963.i

postload963.i:                                    ; preds = %preload962.i, %postload943.i
  br i1 %extract272.i, label %preload982.i, label %postload983.i

preload982.i:                                     ; preds = %postload963.i
  store double %phi981.i, double addrspace(3)* %35, align 8
  br label %postload983.i

postload983.i:                                    ; preds = %preload982.i, %postload963.i
  br i1 %extract273.i, label %preload1002.i, label %postload1003.i

preload1002.i:                                    ; preds = %postload983.i
  store double %phi1001.i, double addrspace(3)* %35, align 8
  br label %postload1003.i

postload1003.i:                                   ; preds = %preload1002.i, %postload983.i
  br i1 %extract274.i, label %preload1022.i, label %postload1023.i

preload1022.i:                                    ; preds = %postload1003.i
  store double %phi1021.i, double addrspace(3)* %35, align 8
  br label %postload1023.i

postload1023.i:                                   ; preds = %preload1022.i, %postload1003.i
  %461 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %462 = mul nsw <16 x i32> %461, %vector118.i
  %463 = add nsw <16 x i32> %58, <i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17>
  %464 = add nsw <16 x i32> %463, %462
  %465 = extractelement <16 x i32> %464, i32 1
  %466 = sext i32 %465 to i64
  %467 = getelementptr inbounds double addrspace(1)* %1, i64 %466
  %468 = extractelement <16 x i32> %464, i32 2
  %469 = sext i32 %468 to i64
  %470 = getelementptr inbounds double addrspace(1)* %1, i64 %469
  %471 = extractelement <16 x i32> %464, i32 3
  %472 = sext i32 %471 to i64
  %473 = getelementptr inbounds double addrspace(1)* %1, i64 %472
  %474 = extractelement <16 x i32> %464, i32 4
  %475 = sext i32 %474 to i64
  %476 = getelementptr inbounds double addrspace(1)* %1, i64 %475
  %477 = extractelement <16 x i32> %464, i32 5
  %478 = sext i32 %477 to i64
  %479 = getelementptr inbounds double addrspace(1)* %1, i64 %478
  %480 = extractelement <16 x i32> %464, i32 6
  %481 = sext i32 %480 to i64
  %482 = getelementptr inbounds double addrspace(1)* %1, i64 %481
  %483 = extractelement <16 x i32> %464, i32 7
  %484 = sext i32 %483 to i64
  %485 = getelementptr inbounds double addrspace(1)* %1, i64 %484
  %486 = extractelement <16 x i32> %464, i32 8
  %487 = sext i32 %486 to i64
  %488 = getelementptr inbounds double addrspace(1)* %1, i64 %487
  %489 = extractelement <16 x i32> %464, i32 9
  %490 = sext i32 %489 to i64
  %491 = getelementptr inbounds double addrspace(1)* %1, i64 %490
  %492 = extractelement <16 x i32> %464, i32 10
  %493 = sext i32 %492 to i64
  %494 = getelementptr inbounds double addrspace(1)* %1, i64 %493
  %495 = extractelement <16 x i32> %464, i32 11
  %496 = sext i32 %495 to i64
  %497 = getelementptr inbounds double addrspace(1)* %1, i64 %496
  %498 = extractelement <16 x i32> %464, i32 12
  %499 = sext i32 %498 to i64
  %500 = getelementptr inbounds double addrspace(1)* %1, i64 %499
  %501 = extractelement <16 x i32> %464, i32 13
  %502 = sext i32 %501 to i64
  %503 = getelementptr inbounds double addrspace(1)* %1, i64 %502
  %504 = extractelement <16 x i32> %464, i32 14
  %505 = sext i32 %504 to i64
  %506 = getelementptr inbounds double addrspace(1)* %1, i64 %505
  %507 = extractelement <16 x i32> %464, i32 15
  %508 = sext i32 %507 to i64
  %509 = getelementptr inbounds double addrspace(1)* %1, i64 %508
  br i1 %extract259.i, label %preload724.i, label %postload725.i

preload724.i:                                     ; preds = %postload1023.i
  %510 = extractelement <16 x i32> %464, i32 0
  %511 = sext i32 %510 to i64
  %512 = getelementptr inbounds double addrspace(1)* %1, i64 %511
  %masked_load485.i = load double addrspace(1)* %512, align 8
  br label %postload725.i

postload725.i:                                    ; preds = %preload724.i, %postload1023.i
  %phi726.i = phi double [ undef, %postload1023.i ], [ %masked_load485.i, %preload724.i ]
  br i1 %extract260.i, label %preload744.i, label %postload745.i

preload744.i:                                     ; preds = %postload725.i
  %masked_load486.i = load double addrspace(1)* %467, align 8
  br label %postload745.i

postload745.i:                                    ; preds = %preload744.i, %postload725.i
  %phi746.i = phi double [ undef, %postload725.i ], [ %masked_load486.i, %preload744.i ]
  br i1 %extract261.i, label %preload764.i, label %postload765.i

preload764.i:                                     ; preds = %postload745.i
  %masked_load487.i = load double addrspace(1)* %470, align 8
  br label %postload765.i

postload765.i:                                    ; preds = %preload764.i, %postload745.i
  %phi766.i = phi double [ undef, %postload745.i ], [ %masked_load487.i, %preload764.i ]
  br i1 %extract262.i, label %preload784.i, label %postload785.i

preload784.i:                                     ; preds = %postload765.i
  %masked_load488.i = load double addrspace(1)* %473, align 8
  br label %postload785.i

postload785.i:                                    ; preds = %preload784.i, %postload765.i
  %phi786.i = phi double [ undef, %postload765.i ], [ %masked_load488.i, %preload784.i ]
  br i1 %extract263.i, label %preload804.i, label %postload805.i

preload804.i:                                     ; preds = %postload785.i
  %masked_load489.i = load double addrspace(1)* %476, align 8
  br label %postload805.i

postload805.i:                                    ; preds = %preload804.i, %postload785.i
  %phi806.i = phi double [ undef, %postload785.i ], [ %masked_load489.i, %preload804.i ]
  br i1 %extract264.i, label %preload824.i, label %postload825.i

preload824.i:                                     ; preds = %postload805.i
  %masked_load490.i = load double addrspace(1)* %479, align 8
  br label %postload825.i

postload825.i:                                    ; preds = %preload824.i, %postload805.i
  %phi826.i = phi double [ undef, %postload805.i ], [ %masked_load490.i, %preload824.i ]
  br i1 %extract265.i, label %preload844.i, label %postload845.i

preload844.i:                                     ; preds = %postload825.i
  %masked_load491.i = load double addrspace(1)* %482, align 8
  br label %postload845.i

postload845.i:                                    ; preds = %preload844.i, %postload825.i
  %phi846.i = phi double [ undef, %postload825.i ], [ %masked_load491.i, %preload844.i ]
  br i1 %extract266.i, label %preload864.i, label %postload865.i

preload864.i:                                     ; preds = %postload845.i
  %masked_load492.i = load double addrspace(1)* %485, align 8
  br label %postload865.i

postload865.i:                                    ; preds = %preload864.i, %postload845.i
  %phi866.i = phi double [ undef, %postload845.i ], [ %masked_load492.i, %preload864.i ]
  br i1 %extract267.i, label %preload884.i, label %postload885.i

preload884.i:                                     ; preds = %postload865.i
  %masked_load493.i = load double addrspace(1)* %488, align 8
  br label %postload885.i

postload885.i:                                    ; preds = %preload884.i, %postload865.i
  %phi886.i = phi double [ undef, %postload865.i ], [ %masked_load493.i, %preload884.i ]
  br i1 %extract268.i, label %preload904.i, label %postload905.i

preload904.i:                                     ; preds = %postload885.i
  %masked_load494.i = load double addrspace(1)* %491, align 8
  br label %postload905.i

postload905.i:                                    ; preds = %preload904.i, %postload885.i
  %phi906.i = phi double [ undef, %postload885.i ], [ %masked_load494.i, %preload904.i ]
  br i1 %extract269.i, label %preload924.i, label %postload925.i

preload924.i:                                     ; preds = %postload905.i
  %masked_load495.i = load double addrspace(1)* %494, align 8
  br label %postload925.i

postload925.i:                                    ; preds = %preload924.i, %postload905.i
  %phi926.i = phi double [ undef, %postload905.i ], [ %masked_load495.i, %preload924.i ]
  br i1 %extract270.i, label %preload944.i, label %postload945.i

preload944.i:                                     ; preds = %postload925.i
  %masked_load496.i = load double addrspace(1)* %497, align 8
  br label %postload945.i

postload945.i:                                    ; preds = %preload944.i, %postload925.i
  %phi946.i = phi double [ undef, %postload925.i ], [ %masked_load496.i, %preload944.i ]
  br i1 %extract271.i, label %preload964.i, label %postload965.i

preload964.i:                                     ; preds = %postload945.i
  %masked_load497.i = load double addrspace(1)* %500, align 8
  br label %postload965.i

postload965.i:                                    ; preds = %preload964.i, %postload945.i
  %phi966.i = phi double [ undef, %postload945.i ], [ %masked_load497.i, %preload964.i ]
  br i1 %extract272.i, label %preload984.i, label %postload985.i

preload984.i:                                     ; preds = %postload965.i
  %masked_load498.i = load double addrspace(1)* %503, align 8
  br label %postload985.i

postload985.i:                                    ; preds = %preload984.i, %postload965.i
  %phi986.i = phi double [ undef, %postload965.i ], [ %masked_load498.i, %preload984.i ]
  br i1 %extract273.i, label %preload1004.i, label %postload1005.i

preload1004.i:                                    ; preds = %postload985.i
  %masked_load499.i = load double addrspace(1)* %506, align 8
  br label %postload1005.i

postload1005.i:                                   ; preds = %preload1004.i, %postload985.i
  %phi1006.i = phi double [ undef, %postload985.i ], [ %masked_load499.i, %preload1004.i ]
  br i1 %extract274.i, label %preload1024.i, label %postload1025.i

preload1024.i:                                    ; preds = %postload1005.i
  %masked_load500.i = load double addrspace(1)* %509, align 8
  br label %postload1025.i

postload1025.i:                                   ; preds = %preload1024.i, %postload1005.i
  %phi1026.i = phi double [ undef, %postload1005.i ], [ %masked_load500.i, %preload1024.i ]
  br i1 %extract259.i, label %preload727.i, label %postload728.i

preload727.i:                                     ; preds = %postload1025.i
  store double %phi726.i, double addrspace(3)* %37, align 8
  br label %postload728.i

postload728.i:                                    ; preds = %preload727.i, %postload1025.i
  br i1 %extract260.i, label %preload747.i, label %postload748.i

preload747.i:                                     ; preds = %postload728.i
  store double %phi746.i, double addrspace(3)* %37, align 8
  br label %postload748.i

postload748.i:                                    ; preds = %preload747.i, %postload728.i
  br i1 %extract261.i, label %preload767.i, label %postload768.i

preload767.i:                                     ; preds = %postload748.i
  store double %phi766.i, double addrspace(3)* %37, align 8
  br label %postload768.i

postload768.i:                                    ; preds = %preload767.i, %postload748.i
  br i1 %extract262.i, label %preload787.i, label %postload788.i

preload787.i:                                     ; preds = %postload768.i
  store double %phi786.i, double addrspace(3)* %37, align 8
  br label %postload788.i

postload788.i:                                    ; preds = %preload787.i, %postload768.i
  br i1 %extract263.i, label %preload807.i, label %postload808.i

preload807.i:                                     ; preds = %postload788.i
  store double %phi806.i, double addrspace(3)* %37, align 8
  br label %postload808.i

postload808.i:                                    ; preds = %preload807.i, %postload788.i
  br i1 %extract264.i, label %preload827.i, label %postload828.i

preload827.i:                                     ; preds = %postload808.i
  store double %phi826.i, double addrspace(3)* %37, align 8
  br label %postload828.i

postload828.i:                                    ; preds = %preload827.i, %postload808.i
  br i1 %extract265.i, label %preload847.i, label %postload848.i

preload847.i:                                     ; preds = %postload828.i
  store double %phi846.i, double addrspace(3)* %37, align 8
  br label %postload848.i

postload848.i:                                    ; preds = %preload847.i, %postload828.i
  br i1 %extract266.i, label %preload867.i, label %postload868.i

preload867.i:                                     ; preds = %postload848.i
  store double %phi866.i, double addrspace(3)* %37, align 8
  br label %postload868.i

postload868.i:                                    ; preds = %preload867.i, %postload848.i
  br i1 %extract267.i, label %preload887.i, label %postload888.i

preload887.i:                                     ; preds = %postload868.i
  store double %phi886.i, double addrspace(3)* %37, align 8
  br label %postload888.i

postload888.i:                                    ; preds = %preload887.i, %postload868.i
  br i1 %extract268.i, label %preload907.i, label %postload908.i

preload907.i:                                     ; preds = %postload888.i
  store double %phi906.i, double addrspace(3)* %37, align 8
  br label %postload908.i

postload908.i:                                    ; preds = %preload907.i, %postload888.i
  br i1 %extract269.i, label %preload927.i, label %postload928.i

preload927.i:                                     ; preds = %postload908.i
  store double %phi926.i, double addrspace(3)* %37, align 8
  br label %postload928.i

postload928.i:                                    ; preds = %preload927.i, %postload908.i
  br i1 %extract270.i, label %preload947.i, label %postload948.i

preload947.i:                                     ; preds = %postload928.i
  store double %phi946.i, double addrspace(3)* %37, align 8
  br label %postload948.i

postload948.i:                                    ; preds = %preload947.i, %postload928.i
  br i1 %extract271.i, label %preload967.i, label %postload968.i

preload967.i:                                     ; preds = %postload948.i
  store double %phi966.i, double addrspace(3)* %37, align 8
  br label %postload968.i

postload968.i:                                    ; preds = %preload967.i, %postload948.i
  br i1 %extract272.i, label %preload987.i, label %postload988.i

preload987.i:                                     ; preds = %postload968.i
  store double %phi986.i, double addrspace(3)* %37, align 8
  br label %postload988.i

postload988.i:                                    ; preds = %preload987.i, %postload968.i
  br i1 %extract273.i, label %preload1007.i, label %postload1008.i

preload1007.i:                                    ; preds = %postload988.i
  store double %phi1006.i, double addrspace(3)* %37, align 8
  br label %postload1008.i

postload1008.i:                                   ; preds = %preload1007.i, %postload988.i
  br i1 %extract274.i, label %preload1027.i, label %postload1028.i

preload1027.i:                                    ; preds = %postload1008.i
  store double %phi1026.i, double addrspace(3)* %37, align 8
  br label %postload1028.i

postload1028.i:                                   ; preds = %preload1027.i, %postload1008.i
  %513 = add nsw i32 %56, 17
  %temp309.i = insertelement <16 x i32> undef, i32 %513, i32 0
  %vector310.i = shufflevector <16 x i32> %temp309.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %514 = or <16 x i32> %59, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %515 = mul nsw <16 x i32> %514, %vector310.i
  %516 = add nsw <16 x i32> %58, <i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17, i32 17>
  %517 = add nsw <16 x i32> %516, %515
  %518 = extractelement <16 x i32> %517, i32 1
  %519 = sext i32 %518 to i64
  %520 = getelementptr inbounds double addrspace(1)* %1, i64 %519
  %521 = extractelement <16 x i32> %517, i32 2
  %522 = sext i32 %521 to i64
  %523 = getelementptr inbounds double addrspace(1)* %1, i64 %522
  %524 = extractelement <16 x i32> %517, i32 3
  %525 = sext i32 %524 to i64
  %526 = getelementptr inbounds double addrspace(1)* %1, i64 %525
  %527 = extractelement <16 x i32> %517, i32 4
  %528 = sext i32 %527 to i64
  %529 = getelementptr inbounds double addrspace(1)* %1, i64 %528
  %530 = extractelement <16 x i32> %517, i32 5
  %531 = sext i32 %530 to i64
  %532 = getelementptr inbounds double addrspace(1)* %1, i64 %531
  %533 = extractelement <16 x i32> %517, i32 6
  %534 = sext i32 %533 to i64
  %535 = getelementptr inbounds double addrspace(1)* %1, i64 %534
  %536 = extractelement <16 x i32> %517, i32 7
  %537 = sext i32 %536 to i64
  %538 = getelementptr inbounds double addrspace(1)* %1, i64 %537
  %539 = extractelement <16 x i32> %517, i32 8
  %540 = sext i32 %539 to i64
  %541 = getelementptr inbounds double addrspace(1)* %1, i64 %540
  %542 = extractelement <16 x i32> %517, i32 9
  %543 = sext i32 %542 to i64
  %544 = getelementptr inbounds double addrspace(1)* %1, i64 %543
  %545 = extractelement <16 x i32> %517, i32 10
  %546 = sext i32 %545 to i64
  %547 = getelementptr inbounds double addrspace(1)* %1, i64 %546
  %548 = extractelement <16 x i32> %517, i32 11
  %549 = sext i32 %548 to i64
  %550 = getelementptr inbounds double addrspace(1)* %1, i64 %549
  %551 = extractelement <16 x i32> %517, i32 12
  %552 = sext i32 %551 to i64
  %553 = getelementptr inbounds double addrspace(1)* %1, i64 %552
  %554 = extractelement <16 x i32> %517, i32 13
  %555 = sext i32 %554 to i64
  %556 = getelementptr inbounds double addrspace(1)* %1, i64 %555
  %557 = extractelement <16 x i32> %517, i32 14
  %558 = sext i32 %557 to i64
  %559 = getelementptr inbounds double addrspace(1)* %1, i64 %558
  %560 = extractelement <16 x i32> %517, i32 15
  %561 = sext i32 %560 to i64
  %562 = getelementptr inbounds double addrspace(1)* %1, i64 %561
  br i1 %extract259.i, label %preload729.i, label %postload730.i

preload729.i:                                     ; preds = %postload1028.i
  %563 = extractelement <16 x i32> %517, i32 0
  %564 = sext i32 %563 to i64
  %565 = getelementptr inbounds double addrspace(1)* %1, i64 %564
  %masked_load501.i = load double addrspace(1)* %565, align 8
  br label %postload730.i

postload730.i:                                    ; preds = %preload729.i, %postload1028.i
  %phi731.i = phi double [ undef, %postload1028.i ], [ %masked_load501.i, %preload729.i ]
  br i1 %extract260.i, label %preload749.i, label %postload750.i

preload749.i:                                     ; preds = %postload730.i
  %masked_load502.i = load double addrspace(1)* %520, align 8
  br label %postload750.i

postload750.i:                                    ; preds = %preload749.i, %postload730.i
  %phi751.i = phi double [ undef, %postload730.i ], [ %masked_load502.i, %preload749.i ]
  br i1 %extract261.i, label %preload769.i, label %postload770.i

preload769.i:                                     ; preds = %postload750.i
  %masked_load503.i = load double addrspace(1)* %523, align 8
  br label %postload770.i

postload770.i:                                    ; preds = %preload769.i, %postload750.i
  %phi771.i = phi double [ undef, %postload750.i ], [ %masked_load503.i, %preload769.i ]
  br i1 %extract262.i, label %preload789.i, label %postload790.i

preload789.i:                                     ; preds = %postload770.i
  %masked_load504.i = load double addrspace(1)* %526, align 8
  br label %postload790.i

postload790.i:                                    ; preds = %preload789.i, %postload770.i
  %phi791.i = phi double [ undef, %postload770.i ], [ %masked_load504.i, %preload789.i ]
  br i1 %extract263.i, label %preload809.i, label %postload810.i

preload809.i:                                     ; preds = %postload790.i
  %masked_load505.i = load double addrspace(1)* %529, align 8
  br label %postload810.i

postload810.i:                                    ; preds = %preload809.i, %postload790.i
  %phi811.i = phi double [ undef, %postload790.i ], [ %masked_load505.i, %preload809.i ]
  br i1 %extract264.i, label %preload829.i, label %postload830.i

preload829.i:                                     ; preds = %postload810.i
  %masked_load506.i = load double addrspace(1)* %532, align 8
  br label %postload830.i

postload830.i:                                    ; preds = %preload829.i, %postload810.i
  %phi831.i = phi double [ undef, %postload810.i ], [ %masked_load506.i, %preload829.i ]
  br i1 %extract265.i, label %preload849.i, label %postload850.i

preload849.i:                                     ; preds = %postload830.i
  %masked_load507.i = load double addrspace(1)* %535, align 8
  br label %postload850.i

postload850.i:                                    ; preds = %preload849.i, %postload830.i
  %phi851.i = phi double [ undef, %postload830.i ], [ %masked_load507.i, %preload849.i ]
  br i1 %extract266.i, label %preload869.i, label %postload870.i

preload869.i:                                     ; preds = %postload850.i
  %masked_load508.i = load double addrspace(1)* %538, align 8
  br label %postload870.i

postload870.i:                                    ; preds = %preload869.i, %postload850.i
  %phi871.i = phi double [ undef, %postload850.i ], [ %masked_load508.i, %preload869.i ]
  br i1 %extract267.i, label %preload889.i, label %postload890.i

preload889.i:                                     ; preds = %postload870.i
  %masked_load509.i = load double addrspace(1)* %541, align 8
  br label %postload890.i

postload890.i:                                    ; preds = %preload889.i, %postload870.i
  %phi891.i = phi double [ undef, %postload870.i ], [ %masked_load509.i, %preload889.i ]
  br i1 %extract268.i, label %preload909.i, label %postload910.i

preload909.i:                                     ; preds = %postload890.i
  %masked_load510.i = load double addrspace(1)* %544, align 8
  br label %postload910.i

postload910.i:                                    ; preds = %preload909.i, %postload890.i
  %phi911.i = phi double [ undef, %postload890.i ], [ %masked_load510.i, %preload909.i ]
  br i1 %extract269.i, label %preload929.i, label %postload930.i

preload929.i:                                     ; preds = %postload910.i
  %masked_load511.i = load double addrspace(1)* %547, align 8
  br label %postload930.i

postload930.i:                                    ; preds = %preload929.i, %postload910.i
  %phi931.i = phi double [ undef, %postload910.i ], [ %masked_load511.i, %preload929.i ]
  br i1 %extract270.i, label %preload949.i, label %postload950.i

preload949.i:                                     ; preds = %postload930.i
  %masked_load512.i = load double addrspace(1)* %550, align 8
  br label %postload950.i

postload950.i:                                    ; preds = %preload949.i, %postload930.i
  %phi951.i = phi double [ undef, %postload930.i ], [ %masked_load512.i, %preload949.i ]
  br i1 %extract271.i, label %preload969.i, label %postload970.i

preload969.i:                                     ; preds = %postload950.i
  %masked_load513.i = load double addrspace(1)* %553, align 8
  br label %postload970.i

postload970.i:                                    ; preds = %preload969.i, %postload950.i
  %phi971.i = phi double [ undef, %postload950.i ], [ %masked_load513.i, %preload969.i ]
  br i1 %extract272.i, label %preload989.i, label %postload990.i

preload989.i:                                     ; preds = %postload970.i
  %masked_load514.i = load double addrspace(1)* %556, align 8
  br label %postload990.i

postload990.i:                                    ; preds = %preload989.i, %postload970.i
  %phi991.i = phi double [ undef, %postload970.i ], [ %masked_load514.i, %preload989.i ]
  br i1 %extract273.i, label %preload1009.i, label %postload1010.i

preload1009.i:                                    ; preds = %postload990.i
  %masked_load515.i = load double addrspace(1)* %559, align 8
  br label %postload1010.i

postload1010.i:                                   ; preds = %preload1009.i, %postload990.i
  %phi1011.i = phi double [ undef, %postload990.i ], [ %masked_load515.i, %preload1009.i ]
  br i1 %extract274.i, label %preload1029.i, label %postload1030.i

preload1029.i:                                    ; preds = %postload1010.i
  %masked_load516.i = load double addrspace(1)* %562, align 8
  br label %postload1030.i

postload1030.i:                                   ; preds = %preload1029.i, %postload1010.i
  %phi1031.i = phi double [ undef, %postload1010.i ], [ %masked_load516.i, %preload1029.i ]
  br i1 %extract259.i, label %preload732.i, label %postload733.i

preload732.i:                                     ; preds = %postload1030.i
  store double %phi731.i, double addrspace(3)* %39, align 8
  br label %postload733.i

postload733.i:                                    ; preds = %preload732.i, %postload1030.i
  br i1 %extract260.i, label %preload752.i, label %postload753.i

preload752.i:                                     ; preds = %postload733.i
  store double %phi751.i, double addrspace(3)* %39, align 8
  br label %postload753.i

postload753.i:                                    ; preds = %preload752.i, %postload733.i
  br i1 %extract261.i, label %preload772.i, label %postload773.i

preload772.i:                                     ; preds = %postload753.i
  store double %phi771.i, double addrspace(3)* %39, align 8
  br label %postload773.i

postload773.i:                                    ; preds = %preload772.i, %postload753.i
  br i1 %extract262.i, label %preload792.i, label %postload793.i

preload792.i:                                     ; preds = %postload773.i
  store double %phi791.i, double addrspace(3)* %39, align 8
  br label %postload793.i

postload793.i:                                    ; preds = %preload792.i, %postload773.i
  br i1 %extract263.i, label %preload812.i, label %postload813.i

preload812.i:                                     ; preds = %postload793.i
  store double %phi811.i, double addrspace(3)* %39, align 8
  br label %postload813.i

postload813.i:                                    ; preds = %preload812.i, %postload793.i
  br i1 %extract264.i, label %preload832.i, label %postload833.i

preload832.i:                                     ; preds = %postload813.i
  store double %phi831.i, double addrspace(3)* %39, align 8
  br label %postload833.i

postload833.i:                                    ; preds = %preload832.i, %postload813.i
  br i1 %extract265.i, label %preload852.i, label %postload853.i

preload852.i:                                     ; preds = %postload833.i
  store double %phi851.i, double addrspace(3)* %39, align 8
  br label %postload853.i

postload853.i:                                    ; preds = %preload852.i, %postload833.i
  br i1 %extract266.i, label %preload872.i, label %postload873.i

preload872.i:                                     ; preds = %postload853.i
  store double %phi871.i, double addrspace(3)* %39, align 8
  br label %postload873.i

postload873.i:                                    ; preds = %preload872.i, %postload853.i
  br i1 %extract267.i, label %preload892.i, label %postload893.i

preload892.i:                                     ; preds = %postload873.i
  store double %phi891.i, double addrspace(3)* %39, align 8
  br label %postload893.i

postload893.i:                                    ; preds = %preload892.i, %postload873.i
  br i1 %extract268.i, label %preload912.i, label %postload913.i

preload912.i:                                     ; preds = %postload893.i
  store double %phi911.i, double addrspace(3)* %39, align 8
  br label %postload913.i

postload913.i:                                    ; preds = %preload912.i, %postload893.i
  br i1 %extract269.i, label %preload932.i, label %postload933.i

preload932.i:                                     ; preds = %postload913.i
  store double %phi931.i, double addrspace(3)* %39, align 8
  br label %postload933.i

postload933.i:                                    ; preds = %preload932.i, %postload913.i
  br i1 %extract270.i, label %preload952.i, label %postload953.i

preload952.i:                                     ; preds = %postload933.i
  store double %phi951.i, double addrspace(3)* %39, align 8
  br label %postload953.i

postload953.i:                                    ; preds = %preload952.i, %postload933.i
  br i1 %extract271.i, label %preload972.i, label %postload973.i

preload972.i:                                     ; preds = %postload953.i
  store double %phi971.i, double addrspace(3)* %39, align 8
  br label %postload973.i

postload973.i:                                    ; preds = %preload972.i, %postload953.i
  br i1 %extract272.i, label %preload992.i, label %postload993.i

preload992.i:                                     ; preds = %postload973.i
  store double %phi991.i, double addrspace(3)* %39, align 8
  br label %postload993.i

postload993.i:                                    ; preds = %preload992.i, %postload973.i
  br i1 %extract273.i, label %preload1012.i, label %postload1013.i

preload1012.i:                                    ; preds = %postload993.i
  store double %phi1011.i, double addrspace(3)* %39, align 8
  br label %postload1013.i

postload1013.i:                                   ; preds = %preload1012.i, %postload993.i
  br i1 %extract274.i, label %preload1032.i, label %postload1033.i

preload1032.i:                                    ; preds = %postload1013.i
  store double %phi1031.i, double addrspace(3)* %39, align 8
  br label %postload1033.i

postload1033.i:                                   ; preds = %preload1032.i, %postload1013.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %28
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %postload1033.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 320
  br label %SyncBB1220.i

elseBB.i:                                         ; preds = %postload1033.i
  %temp372.i = insertelement <16 x double> undef, double %13, i32 0
  %vector373.i = shufflevector <16 x double> %temp372.i, <16 x double> undef, <16 x i32> zeroinitializer
  %temp370.i = insertelement <16 x double> undef, double %10, i32 0
  %vector371.i = shufflevector <16 x double> %temp370.i, <16 x double> undef, <16 x i32> zeroinitializer
  %temp368.i = insertelement <16 x double> undef, double %7, i32 0
  %vector369.i = shufflevector <16 x double> %temp368.i, <16 x double> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB1222.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride1228.i", %thenBB1222.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++1226.i", %thenBB1222.i ]
  %"&(pSB[currWI].offset)1209.i" = add nuw i64 %CurrSBIndex..1.i, 264
  %"&pSB[currWI].offset1210.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1209.i"
  %CastToValueType1211.i = bitcast i8* %"&pSB[currWI].offset1210.i" to double addrspace(3)**
  %loadedValue1212.i = load double addrspace(3)** %CastToValueType1211.i, align 8
  %ptrTypeCast327.i = bitcast double addrspace(3)* %loadedValue1212.i to <16 x double> addrspace(3)*
  %566 = load <16 x double> addrspace(3)* %ptrTypeCast327.i, align 8
  %"&(pSB[currWI].offset)10392.i" = or i64 %CurrSBIndex..1.i, 40
  %"&pSB[currWI].offset1040.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)10392.i"
  %CastToValueType1041.i = bitcast i8* %"&pSB[currWI].offset1040.i" to i32*
  %loadedValue1042.i = load i32* %CastToValueType1041.i, align 4
  %567 = sext i32 %loadedValue1042.i to i64
  %"&(pSB[currWI].offset)1166.i" = add nuw i64 %CurrSBIndex..1.i, 192
  %"&pSB[currWI].offset1167.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1166.i"
  %CastToValueType1168.i = bitcast i8* %"&pSB[currWI].offset1167.i" to <16 x i32>*
  %loadedValue1169.i = load <16 x i32>* %CastToValueType1168.i, align 64
  %568 = extractelement <16 x i32> %loadedValue1169.i, i32 0
  %569 = sext i32 %568 to i64
  %570 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %567, i64 %569
  %ptrTypeCast328.i = bitcast double addrspace(3)* %570 to <16 x double> addrspace(3)*
  %571 = load <16 x double> addrspace(3)* %ptrTypeCast328.i, align 8
  %"&(pSB[currWI].offset)10353.i" = or i64 %CurrSBIndex..1.i, 40
  %"&pSB[currWI].offset1036.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)10353.i"
  %CastToValueType1037.i = bitcast i8* %"&pSB[currWI].offset1036.i" to i32*
  %loadedValue.i = load i32* %CastToValueType1037.i, align 4
  %572 = add nsw i32 %loadedValue.i, 2
  %573 = sext i32 %572 to i64
  %"&(pSB[currWI].offset)1161.i" = add nuw i64 %CurrSBIndex..1.i, 192
  %"&pSB[currWI].offset1162.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1161.i"
  %CastToValueType1163.i = bitcast i8* %"&pSB[currWI].offset1162.i" to <16 x i32>*
  %loadedValue1164.i = load <16 x i32>* %CastToValueType1163.i, align 64
  %574 = extractelement <16 x i32> %loadedValue1164.i, i32 0
  %575 = sext i32 %574 to i64
  %576 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %573, i64 %575
  %ptrTypeCast329.i = bitcast double addrspace(3)* %576 to <16 x double> addrspace(3)*
  %577 = load <16 x double> addrspace(3)* %ptrTypeCast329.i, align 8
  %578 = fadd <16 x double> %571, %577
  %"&(pSB[currWI].offset)1058.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset1059.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1058.i"
  %CastToValueType1060.i = bitcast i8* %"&pSB[currWI].offset1059.i" to <16 x i32>*
  %loadedValue1061.i = load <16 x i32>* %CastToValueType1060.i, align 64
  %579 = extractelement <16 x i32> %loadedValue1061.i, i32 0
  %580 = sext i32 %579 to i64
  %"&(pSB[currWI].offset)1190.i" = add nuw i64 %CurrSBIndex..1.i, 256
  %"&pSB[currWI].offset1191.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1190.i"
  %CastToValueType1192.i = bitcast i8* %"&pSB[currWI].offset1191.i" to i64*
  %loadedValue1193.i = load i64* %CastToValueType1192.i, align 8
  %581 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1193.i, i64 %580
  %ptrTypeCast346.i = bitcast double addrspace(3)* %581 to <16 x double> addrspace(3)*
  %582 = load <16 x double> addrspace(3)* %ptrTypeCast346.i, align 8
  %583 = fadd <16 x double> %578, %582
  %"&(pSB[currWI].offset)1063.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset1064.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1063.i"
  %CastToValueType1065.i = bitcast i8* %"&pSB[currWI].offset1064.i" to <16 x i32>*
  %loadedValue1066.i = load <16 x i32>* %CastToValueType1065.i, align 64
  %584 = add nsw <16 x i32> %loadedValue1066.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %585 = extractelement <16 x i32> %584, i32 0
  %586 = sext i32 %585 to i64
  %"&(pSB[currWI].offset)1185.i" = add nuw i64 %CurrSBIndex..1.i, 256
  %"&pSB[currWI].offset1186.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1185.i"
  %CastToValueType1187.i = bitcast i8* %"&pSB[currWI].offset1186.i" to i64*
  %loadedValue1188.i = load i64* %CastToValueType1187.i, align 8
  %587 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %loadedValue1188.i, i64 %586
  %ptrTypeCast363.i = bitcast double addrspace(3)* %587 to <16 x double> addrspace(3)*
  %588 = load <16 x double> addrspace(3)* %ptrTypeCast363.i, align 8
  %589 = fadd <16 x double> %583, %588
  %"&(pSB[currWI].offset)1053.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset1054.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1053.i"
  %CastToValueType1055.i = bitcast i8* %"&pSB[currWI].offset1054.i" to <16 x i32>*
  %loadedValue1056.i = load <16 x i32>* %CastToValueType1055.i, align 64
  %590 = extractelement <16 x i32> %loadedValue1056.i, i32 0
  %591 = sext i32 %590 to i64
  %592 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %567, i64 %591
  %ptrTypeCast364.i = bitcast double addrspace(3)* %592 to <16 x double> addrspace(3)*
  %593 = load <16 x double> addrspace(3)* %ptrTypeCast364.i, align 8
  %594 = extractelement <16 x i32> %584, i32 0
  %595 = sext i32 %594 to i64
  %596 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %567, i64 %595
  %ptrTypeCast365.i = bitcast double addrspace(3)* %596 to <16 x double> addrspace(3)*
  %597 = load <16 x double> addrspace(3)* %ptrTypeCast365.i, align 8
  %598 = fadd <16 x double> %593, %597
  %"&(pSB[currWI].offset)1048.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset1049.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1048.i"
  %CastToValueType1050.i = bitcast i8* %"&pSB[currWI].offset1049.i" to <16 x i32>*
  %loadedValue1051.i = load <16 x i32>* %CastToValueType1050.i, align 64
  %599 = extractelement <16 x i32> %loadedValue1051.i, i32 0
  %600 = sext i32 %599 to i64
  %601 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %573, i64 %600
  %ptrTypeCast366.i = bitcast double addrspace(3)* %601 to <16 x double> addrspace(3)*
  %602 = load <16 x double> addrspace(3)* %ptrTypeCast366.i, align 8
  %603 = fadd <16 x double> %598, %602
  %604 = extractelement <16 x i32> %584, i32 0
  %605 = sext i32 %604 to i64
  %606 = getelementptr inbounds [18 x [18 x double]] addrspace(3)* %32, i64 0, i64 %573, i64 %605
  %ptrTypeCast367.i = bitcast double addrspace(3)* %606 to <16 x double> addrspace(3)*
  %607 = load <16 x double> addrspace(3)* %ptrTypeCast367.i, align 8
  %608 = fadd <16 x double> %603, %607
  %609 = fmul <16 x double> %566, %vector369.i
  %610 = fmul <16 x double> %589, %vector371.i
  %611 = fadd <16 x double> %609, %610
  %612 = fmul <16 x double> %608, %vector373.i
  %613 = fadd <16 x double> %611, %612
  %extract374.i = extractelement <16 x double> %613, i32 0
  %extract375.i = extractelement <16 x double> %613, i32 1
  %extract376.i = extractelement <16 x double> %613, i32 2
  %extract377.i = extractelement <16 x double> %613, i32 3
  %extract378.i = extractelement <16 x double> %613, i32 4
  %extract379.i = extractelement <16 x double> %613, i32 5
  %extract380.i = extractelement <16 x double> %613, i32 6
  %extract381.i = extractelement <16 x double> %613, i32 7
  %extract382.i = extractelement <16 x double> %613, i32 8
  %extract383.i = extractelement <16 x double> %613, i32 9
  %extract384.i = extractelement <16 x double> %613, i32 10
  %extract385.i = extractelement <16 x double> %613, i32 11
  %extract386.i = extractelement <16 x double> %613, i32 12
  %extract387.i = extractelement <16 x double> %613, i32 13
  %extract388.i = extractelement <16 x double> %613, i32 14
  %extract389.i = extractelement <16 x double> %613, i32 15
  %"&(pSB[currWI].offset)1152.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1153.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1152.i"
  %CastToValueType1154.i = bitcast i8* %"&pSB[currWI].offset1153.i" to <16 x i32>*
  %loadedValue1155.i = load <16 x i32>* %CastToValueType1154.i, align 64
  %614 = extractelement <16 x i32> %loadedValue1155.i, i32 0
  %615 = sext i32 %614 to i64
  %616 = getelementptr inbounds double addrspace(1)* %4, i64 %615
  %"&(pSB[currWI].offset)1147.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1148.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1147.i"
  %CastToValueType1149.i = bitcast i8* %"&pSB[currWI].offset1148.i" to <16 x i32>*
  %loadedValue1150.i = load <16 x i32>* %CastToValueType1149.i, align 64
  %617 = extractelement <16 x i32> %loadedValue1150.i, i32 1
  %618 = sext i32 %617 to i64
  %619 = getelementptr inbounds double addrspace(1)* %4, i64 %618
  %"&(pSB[currWI].offset)1142.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1143.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1142.i"
  %CastToValueType1144.i = bitcast i8* %"&pSB[currWI].offset1143.i" to <16 x i32>*
  %loadedValue1145.i = load <16 x i32>* %CastToValueType1144.i, align 64
  %620 = extractelement <16 x i32> %loadedValue1145.i, i32 2
  %621 = sext i32 %620 to i64
  %622 = getelementptr inbounds double addrspace(1)* %4, i64 %621
  %"&(pSB[currWI].offset)1137.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1138.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1137.i"
  %CastToValueType1139.i = bitcast i8* %"&pSB[currWI].offset1138.i" to <16 x i32>*
  %loadedValue1140.i = load <16 x i32>* %CastToValueType1139.i, align 64
  %623 = extractelement <16 x i32> %loadedValue1140.i, i32 3
  %624 = sext i32 %623 to i64
  %625 = getelementptr inbounds double addrspace(1)* %4, i64 %624
  %"&(pSB[currWI].offset)1132.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1133.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1132.i"
  %CastToValueType1134.i = bitcast i8* %"&pSB[currWI].offset1133.i" to <16 x i32>*
  %loadedValue1135.i = load <16 x i32>* %CastToValueType1134.i, align 64
  %626 = extractelement <16 x i32> %loadedValue1135.i, i32 4
  %627 = sext i32 %626 to i64
  %628 = getelementptr inbounds double addrspace(1)* %4, i64 %627
  %"&(pSB[currWI].offset)1127.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1128.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1127.i"
  %CastToValueType1129.i = bitcast i8* %"&pSB[currWI].offset1128.i" to <16 x i32>*
  %loadedValue1130.i = load <16 x i32>* %CastToValueType1129.i, align 64
  %629 = extractelement <16 x i32> %loadedValue1130.i, i32 5
  %630 = sext i32 %629 to i64
  %631 = getelementptr inbounds double addrspace(1)* %4, i64 %630
  %"&(pSB[currWI].offset)1122.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1123.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1122.i"
  %CastToValueType1124.i = bitcast i8* %"&pSB[currWI].offset1123.i" to <16 x i32>*
  %loadedValue1125.i = load <16 x i32>* %CastToValueType1124.i, align 64
  %632 = extractelement <16 x i32> %loadedValue1125.i, i32 6
  %633 = sext i32 %632 to i64
  %634 = getelementptr inbounds double addrspace(1)* %4, i64 %633
  %"&(pSB[currWI].offset)1117.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1118.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1117.i"
  %CastToValueType1119.i = bitcast i8* %"&pSB[currWI].offset1118.i" to <16 x i32>*
  %loadedValue1120.i = load <16 x i32>* %CastToValueType1119.i, align 64
  %635 = extractelement <16 x i32> %loadedValue1120.i, i32 7
  %636 = sext i32 %635 to i64
  %637 = getelementptr inbounds double addrspace(1)* %4, i64 %636
  %"&(pSB[currWI].offset)1112.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1113.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1112.i"
  %CastToValueType1114.i = bitcast i8* %"&pSB[currWI].offset1113.i" to <16 x i32>*
  %loadedValue1115.i = load <16 x i32>* %CastToValueType1114.i, align 64
  %638 = extractelement <16 x i32> %loadedValue1115.i, i32 8
  %639 = sext i32 %638 to i64
  %640 = getelementptr inbounds double addrspace(1)* %4, i64 %639
  %"&(pSB[currWI].offset)1107.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1108.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1107.i"
  %CastToValueType1109.i = bitcast i8* %"&pSB[currWI].offset1108.i" to <16 x i32>*
  %loadedValue1110.i = load <16 x i32>* %CastToValueType1109.i, align 64
  %641 = extractelement <16 x i32> %loadedValue1110.i, i32 9
  %642 = sext i32 %641 to i64
  %643 = getelementptr inbounds double addrspace(1)* %4, i64 %642
  %"&(pSB[currWI].offset)1102.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1103.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1102.i"
  %CastToValueType1104.i = bitcast i8* %"&pSB[currWI].offset1103.i" to <16 x i32>*
  %loadedValue1105.i = load <16 x i32>* %CastToValueType1104.i, align 64
  %644 = extractelement <16 x i32> %loadedValue1105.i, i32 10
  %645 = sext i32 %644 to i64
  %646 = getelementptr inbounds double addrspace(1)* %4, i64 %645
  %"&(pSB[currWI].offset)1097.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1098.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1097.i"
  %CastToValueType1099.i = bitcast i8* %"&pSB[currWI].offset1098.i" to <16 x i32>*
  %loadedValue1100.i = load <16 x i32>* %CastToValueType1099.i, align 64
  %647 = extractelement <16 x i32> %loadedValue1100.i, i32 11
  %648 = sext i32 %647 to i64
  %649 = getelementptr inbounds double addrspace(1)* %4, i64 %648
  %"&(pSB[currWI].offset)1092.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1093.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1092.i"
  %CastToValueType1094.i = bitcast i8* %"&pSB[currWI].offset1093.i" to <16 x i32>*
  %loadedValue1095.i = load <16 x i32>* %CastToValueType1094.i, align 64
  %650 = extractelement <16 x i32> %loadedValue1095.i, i32 12
  %651 = sext i32 %650 to i64
  %652 = getelementptr inbounds double addrspace(1)* %4, i64 %651
  %"&(pSB[currWI].offset)1087.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1088.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1087.i"
  %CastToValueType1089.i = bitcast i8* %"&pSB[currWI].offset1088.i" to <16 x i32>*
  %loadedValue1090.i = load <16 x i32>* %CastToValueType1089.i, align 64
  %653 = extractelement <16 x i32> %loadedValue1090.i, i32 13
  %654 = sext i32 %653 to i64
  %655 = getelementptr inbounds double addrspace(1)* %4, i64 %654
  %"&(pSB[currWI].offset)1082.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1083.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1082.i"
  %CastToValueType1084.i = bitcast i8* %"&pSB[currWI].offset1083.i" to <16 x i32>*
  %loadedValue1085.i = load <16 x i32>* %CastToValueType1084.i, align 64
  %656 = extractelement <16 x i32> %loadedValue1085.i, i32 14
  %657 = sext i32 %656 to i64
  %658 = getelementptr inbounds double addrspace(1)* %4, i64 %657
  %"&(pSB[currWI].offset)1077.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset1078.i" = getelementptr inbounds i8* %31, i64 %"&(pSB[currWI].offset)1077.i"
  %CastToValueType1079.i = bitcast i8* %"&pSB[currWI].offset1078.i" to <16 x i32>*
  %loadedValue1080.i = load <16 x i32>* %CastToValueType1079.i, align 64
  %659 = extractelement <16 x i32> %loadedValue1080.i, i32 15
  %660 = sext i32 %659 to i64
  %661 = getelementptr inbounds double addrspace(1)* %4, i64 %660
  store double %extract374.i, double addrspace(1)* %616, align 8
  store double %extract375.i, double addrspace(1)* %619, align 8
  store double %extract376.i, double addrspace(1)* %622, align 8
  store double %extract377.i, double addrspace(1)* %625, align 8
  store double %extract378.i, double addrspace(1)* %628, align 8
  store double %extract379.i, double addrspace(1)* %631, align 8
  store double %extract380.i, double addrspace(1)* %634, align 8
  store double %extract381.i, double addrspace(1)* %637, align 8
  store double %extract382.i, double addrspace(1)* %640, align 8
  store double %extract383.i, double addrspace(1)* %643, align 8
  store double %extract384.i, double addrspace(1)* %646, align 8
  store double %extract385.i, double addrspace(1)* %649, align 8
  store double %extract386.i, double addrspace(1)* %652, align 8
  store double %extract387.i, double addrspace(1)* %655, align 8
  store double %extract388.i, double addrspace(1)* %658, align 8
  store double %extract389.i, double addrspace(1)* %661, align 8
  %check.WI.iter1225.i = icmp ult i64 %CurrWI..1.i, %28
  br i1 %check.WI.iter1225.i, label %thenBB1222.i, label %____Vectorized_.StencilKernel_separated_args.exit

thenBB1222.i:                                     ; preds = %SyncBB.i
  %"CurrWI++1226.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride1228.i" = add nuw i64 %CurrSBIndex..1.i, 320
  br label %SyncBB.i

____Vectorized_.StencilKernel_separated_args.exit: ; preds = %SyncBB.i
  ret void
}

define void @__Vectorized_.CopyRect(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 12
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 16
  %9 = bitcast i8* %8 to double addrspace(1)**
  %10 = load double addrspace(1)** %9, align 8
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
  %temp54.i = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector55.i = shufflevector <16 x i32> %temp54.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp16.i = insertelement <16 x i32> undef, i32 %16, i32 0
  %vector17.i = shufflevector <16 x i32> %temp16.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp7.i = insertelement <16 x i32> undef, i32 %22, i32 0
  %vector8.i = shufflevector <16 x i32> %temp7.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %35 = icmp sgt i32 %19, 0
  %temp9.i = insertelement <16 x i1> undef, i1 %35, i32 0
  %vector10.i = shufflevector <16 x i1> %temp9.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %36 = sext i32 %13 to i64
  %temp20.i = insertelement <16 x i64> undef, i64 %36, i32 0
  %vector21.i = shufflevector <16 x i64> %temp20.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %37 = sext i32 %4 to i64
  %temp56.i = insertelement <16 x i64> undef, i64 %37, i32 0
  %vector57.i = shufflevector <16 x i64> %temp56.i, <16 x i64> undef, <16 x i32> zeroinitializer
  br label %SyncBB176.i

SyncBB176.i:                                      ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %38 = load i64* %28, align 8
  %39 = trunc i64 %38 to i32
  %40 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..0.i, i32 0, i64 0
  %41 = load i64* %40, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %41, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %42 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %43 = trunc <16 x i64> %42 to <16 x i32>
  %44 = getelementptr %struct.WorkDim* %25, i64 0, i32 3, i64 0
  %45 = load i64* %44, align 8
  %46 = trunc i64 %45 to i32
  %47 = mul nsw i32 %46, %39
  %temp.i = insertelement <16 x i32> undef, i32 %47, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %48 = add nsw <16 x i32> %vector.i, %43
  %49 = icmp slt <16 x i32> %48, %vector8.i
  %or.cond11.i = and <16 x i1> %49, %vector10.i
  %ipred.i.i = bitcast <16 x i1> %or.cond11.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %tmp.i.i = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %tmp.i.i, 0
  br i1 %res.i.i, label %.preheader.i, label %.loopexit.i

.preheader.i:                                     ; preds = %SyncBB176.i
  %negIncomingLoopMask13.i = xor <16 x i1> %or.cond11.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %50 = mul nsw <16 x i32> %48, %vector17.i
  %51 = mul nsw <16 x i32> %48, %vector55.i
  br label %52

; <label>:52                                      ; preds = %postload175.i, %.preheader.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask379.i, %postload175.i ], [ %negIncomingLoopMask13.i, %.preheader.i ]
  %vectorPHI15.i = phi <16 x i1> [ %local_edge83.i, %postload175.i ], [ %or.cond11.i, %.preheader.i ]
  %53 = phi i32 [ %90, %postload175.i ], [ 0, %.preheader.i ]
  %extract38.i = extractelement <16 x i1> %vectorPHI15.i, i32 0
  %extract39.i = extractelement <16 x i1> %vectorPHI15.i, i32 1
  %extract40.i = extractelement <16 x i1> %vectorPHI15.i, i32 2
  %extract41.i = extractelement <16 x i1> %vectorPHI15.i, i32 3
  %extract42.i = extractelement <16 x i1> %vectorPHI15.i, i32 4
  %extract43.i = extractelement <16 x i1> %vectorPHI15.i, i32 5
  %extract44.i = extractelement <16 x i1> %vectorPHI15.i, i32 6
  %extract45.i = extractelement <16 x i1> %vectorPHI15.i, i32 7
  %extract46.i = extractelement <16 x i1> %vectorPHI15.i, i32 8
  %extract47.i = extractelement <16 x i1> %vectorPHI15.i, i32 9
  %extract48.i = extractelement <16 x i1> %vectorPHI15.i, i32 10
  %extract49.i = extractelement <16 x i1> %vectorPHI15.i, i32 11
  %extract50.i = extractelement <16 x i1> %vectorPHI15.i, i32 12
  %extract51.i = extractelement <16 x i1> %vectorPHI15.i, i32 13
  %extract52.i = extractelement <16 x i1> %vectorPHI15.i, i32 14
  %extract53.i = extractelement <16 x i1> %vectorPHI15.i, i32 15
  %temp18.i = insertelement <16 x i32> undef, i32 %53, i32 0
  %vector19.i = shufflevector <16 x i32> %temp18.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %54 = add nsw <16 x i32> %50, %vector19.i
  %55 = sext <16 x i32> %54 to <16 x i64>
  %.sum22.i = add <16 x i64> %55, %vector21.i
  %extract23.i = extractelement <16 x i64> %.sum22.i, i32 1
  %extract24.i = extractelement <16 x i64> %.sum22.i, i32 2
  %extract25.i = extractelement <16 x i64> %.sum22.i, i32 3
  %extract26.i = extractelement <16 x i64> %.sum22.i, i32 4
  %extract27.i = extractelement <16 x i64> %.sum22.i, i32 5
  %extract28.i = extractelement <16 x i64> %.sum22.i, i32 6
  %extract29.i = extractelement <16 x i64> %.sum22.i, i32 7
  %extract30.i = extractelement <16 x i64> %.sum22.i, i32 8
  %extract31.i = extractelement <16 x i64> %.sum22.i, i32 9
  %extract32.i = extractelement <16 x i64> %.sum22.i, i32 10
  %extract33.i = extractelement <16 x i64> %.sum22.i, i32 11
  %extract34.i = extractelement <16 x i64> %.sum22.i, i32 12
  %extract35.i = extractelement <16 x i64> %.sum22.i, i32 13
  %extract36.i = extractelement <16 x i64> %.sum22.i, i32 14
  %extract37.i = extractelement <16 x i64> %.sum22.i, i32 15
  %56 = getelementptr inbounds double addrspace(1)* %10, i64 %extract23.i
  %57 = getelementptr inbounds double addrspace(1)* %10, i64 %extract24.i
  %58 = getelementptr inbounds double addrspace(1)* %10, i64 %extract25.i
  %59 = getelementptr inbounds double addrspace(1)* %10, i64 %extract26.i
  %60 = getelementptr inbounds double addrspace(1)* %10, i64 %extract27.i
  %61 = getelementptr inbounds double addrspace(1)* %10, i64 %extract28.i
  %62 = getelementptr inbounds double addrspace(1)* %10, i64 %extract29.i
  %63 = getelementptr inbounds double addrspace(1)* %10, i64 %extract30.i
  %64 = getelementptr inbounds double addrspace(1)* %10, i64 %extract31.i
  %65 = getelementptr inbounds double addrspace(1)* %10, i64 %extract32.i
  %66 = getelementptr inbounds double addrspace(1)* %10, i64 %extract33.i
  %67 = getelementptr inbounds double addrspace(1)* %10, i64 %extract34.i
  %68 = getelementptr inbounds double addrspace(1)* %10, i64 %extract35.i
  %69 = getelementptr inbounds double addrspace(1)* %10, i64 %extract36.i
  %70 = getelementptr inbounds double addrspace(1)* %10, i64 %extract37.i
  br i1 %extract38.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %52
  %extract.i = extractelement <16 x i64> %.sum22.i, i32 0
  %71 = getelementptr inbounds double addrspace(1)* %10, i64 %extract.i
  %masked_load.i = load double addrspace(1)* %71, align 8
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %52
  %phi.i = phi double [ undef, %52 ], [ %masked_load.i, %preload.i ]
  br i1 %extract39.i, label %preload101.i, label %postload102.i

preload101.i:                                     ; preds = %postload.i
  %masked_load84.i = load double addrspace(1)* %56, align 8
  br label %postload102.i

postload102.i:                                    ; preds = %preload101.i, %postload.i
  %phi103.i = phi double [ undef, %postload.i ], [ %masked_load84.i, %preload101.i ]
  br i1 %extract40.i, label %preload106.i, label %postload107.i

preload106.i:                                     ; preds = %postload102.i
  %masked_load85.i = load double addrspace(1)* %57, align 8
  br label %postload107.i

postload107.i:                                    ; preds = %preload106.i, %postload102.i
  %phi108.i = phi double [ undef, %postload102.i ], [ %masked_load85.i, %preload106.i ]
  br i1 %extract41.i, label %preload111.i, label %postload112.i

preload111.i:                                     ; preds = %postload107.i
  %masked_load86.i = load double addrspace(1)* %58, align 8
  br label %postload112.i

postload112.i:                                    ; preds = %preload111.i, %postload107.i
  %phi113.i = phi double [ undef, %postload107.i ], [ %masked_load86.i, %preload111.i ]
  br i1 %extract42.i, label %preload116.i, label %postload117.i

preload116.i:                                     ; preds = %postload112.i
  %masked_load87.i = load double addrspace(1)* %59, align 8
  br label %postload117.i

postload117.i:                                    ; preds = %preload116.i, %postload112.i
  %phi118.i = phi double [ undef, %postload112.i ], [ %masked_load87.i, %preload116.i ]
  br i1 %extract43.i, label %preload121.i, label %postload122.i

preload121.i:                                     ; preds = %postload117.i
  %masked_load88.i = load double addrspace(1)* %60, align 8
  br label %postload122.i

postload122.i:                                    ; preds = %preload121.i, %postload117.i
  %phi123.i = phi double [ undef, %postload117.i ], [ %masked_load88.i, %preload121.i ]
  br i1 %extract44.i, label %preload126.i, label %postload127.i

preload126.i:                                     ; preds = %postload122.i
  %masked_load89.i = load double addrspace(1)* %61, align 8
  br label %postload127.i

postload127.i:                                    ; preds = %preload126.i, %postload122.i
  %phi128.i = phi double [ undef, %postload122.i ], [ %masked_load89.i, %preload126.i ]
  br i1 %extract45.i, label %preload131.i, label %postload132.i

preload131.i:                                     ; preds = %postload127.i
  %masked_load90.i = load double addrspace(1)* %62, align 8
  br label %postload132.i

postload132.i:                                    ; preds = %preload131.i, %postload127.i
  %phi133.i = phi double [ undef, %postload127.i ], [ %masked_load90.i, %preload131.i ]
  br i1 %extract46.i, label %preload136.i, label %postload137.i

preload136.i:                                     ; preds = %postload132.i
  %masked_load91.i = load double addrspace(1)* %63, align 8
  br label %postload137.i

postload137.i:                                    ; preds = %preload136.i, %postload132.i
  %phi138.i = phi double [ undef, %postload132.i ], [ %masked_load91.i, %preload136.i ]
  br i1 %extract47.i, label %preload141.i, label %postload142.i

preload141.i:                                     ; preds = %postload137.i
  %masked_load92.i = load double addrspace(1)* %64, align 8
  br label %postload142.i

postload142.i:                                    ; preds = %preload141.i, %postload137.i
  %phi143.i = phi double [ undef, %postload137.i ], [ %masked_load92.i, %preload141.i ]
  br i1 %extract48.i, label %preload146.i, label %postload147.i

preload146.i:                                     ; preds = %postload142.i
  %masked_load93.i = load double addrspace(1)* %65, align 8
  br label %postload147.i

postload147.i:                                    ; preds = %preload146.i, %postload142.i
  %phi148.i = phi double [ undef, %postload142.i ], [ %masked_load93.i, %preload146.i ]
  br i1 %extract49.i, label %preload151.i, label %postload152.i

preload151.i:                                     ; preds = %postload147.i
  %masked_load94.i = load double addrspace(1)* %66, align 8
  br label %postload152.i

postload152.i:                                    ; preds = %preload151.i, %postload147.i
  %phi153.i = phi double [ undef, %postload147.i ], [ %masked_load94.i, %preload151.i ]
  br i1 %extract50.i, label %preload156.i, label %postload157.i

preload156.i:                                     ; preds = %postload152.i
  %masked_load95.i = load double addrspace(1)* %67, align 8
  br label %postload157.i

postload157.i:                                    ; preds = %preload156.i, %postload152.i
  %phi158.i = phi double [ undef, %postload152.i ], [ %masked_load95.i, %preload156.i ]
  br i1 %extract51.i, label %preload161.i, label %postload162.i

preload161.i:                                     ; preds = %postload157.i
  %masked_load96.i = load double addrspace(1)* %68, align 8
  br label %postload162.i

postload162.i:                                    ; preds = %preload161.i, %postload157.i
  %phi163.i = phi double [ undef, %postload157.i ], [ %masked_load96.i, %preload161.i ]
  br i1 %extract52.i, label %preload166.i, label %postload167.i

preload166.i:                                     ; preds = %postload162.i
  %masked_load97.i = load double addrspace(1)* %69, align 8
  br label %postload167.i

postload167.i:                                    ; preds = %preload166.i, %postload162.i
  %phi168.i = phi double [ undef, %postload162.i ], [ %masked_load97.i, %preload166.i ]
  br i1 %extract53.i, label %preload171.i, label %postload172.i

preload171.i:                                     ; preds = %postload167.i
  %masked_load98.i = load double addrspace(1)* %70, align 8
  br label %postload172.i

postload172.i:                                    ; preds = %preload171.i, %postload167.i
  %phi173.i = phi double [ undef, %postload167.i ], [ %masked_load98.i, %preload171.i ]
  %72 = add nsw <16 x i32> %51, %vector19.i
  %73 = sext <16 x i32> %72 to <16 x i64>
  %.sum158.i = add <16 x i64> %73, %vector57.i
  %extract60.i = extractelement <16 x i64> %.sum158.i, i32 1
  %extract61.i = extractelement <16 x i64> %.sum158.i, i32 2
  %extract62.i = extractelement <16 x i64> %.sum158.i, i32 3
  %extract63.i = extractelement <16 x i64> %.sum158.i, i32 4
  %extract64.i = extractelement <16 x i64> %.sum158.i, i32 5
  %extract65.i = extractelement <16 x i64> %.sum158.i, i32 6
  %extract66.i = extractelement <16 x i64> %.sum158.i, i32 7
  %extract67.i = extractelement <16 x i64> %.sum158.i, i32 8
  %extract68.i = extractelement <16 x i64> %.sum158.i, i32 9
  %extract69.i = extractelement <16 x i64> %.sum158.i, i32 10
  %extract70.i = extractelement <16 x i64> %.sum158.i, i32 11
  %extract71.i = extractelement <16 x i64> %.sum158.i, i32 12
  %extract72.i = extractelement <16 x i64> %.sum158.i, i32 13
  %extract73.i = extractelement <16 x i64> %.sum158.i, i32 14
  %extract74.i = extractelement <16 x i64> %.sum158.i, i32 15
  %74 = getelementptr inbounds double addrspace(1)* %1, i64 %extract60.i
  %75 = getelementptr inbounds double addrspace(1)* %1, i64 %extract61.i
  %76 = getelementptr inbounds double addrspace(1)* %1, i64 %extract62.i
  %77 = getelementptr inbounds double addrspace(1)* %1, i64 %extract63.i
  %78 = getelementptr inbounds double addrspace(1)* %1, i64 %extract64.i
  %79 = getelementptr inbounds double addrspace(1)* %1, i64 %extract65.i
  %80 = getelementptr inbounds double addrspace(1)* %1, i64 %extract66.i
  %81 = getelementptr inbounds double addrspace(1)* %1, i64 %extract67.i
  %82 = getelementptr inbounds double addrspace(1)* %1, i64 %extract68.i
  %83 = getelementptr inbounds double addrspace(1)* %1, i64 %extract69.i
  %84 = getelementptr inbounds double addrspace(1)* %1, i64 %extract70.i
  %85 = getelementptr inbounds double addrspace(1)* %1, i64 %extract71.i
  %86 = getelementptr inbounds double addrspace(1)* %1, i64 %extract72.i
  %87 = getelementptr inbounds double addrspace(1)* %1, i64 %extract73.i
  %88 = getelementptr inbounds double addrspace(1)* %1, i64 %extract74.i
  br i1 %extract38.i, label %preload99.i, label %postload100.i

preload99.i:                                      ; preds = %postload172.i
  %extract59.i = extractelement <16 x i64> %.sum158.i, i32 0
  %89 = getelementptr inbounds double addrspace(1)* %1, i64 %extract59.i
  store double %phi.i, double addrspace(1)* %89, align 8
  br label %postload100.i

postload100.i:                                    ; preds = %preload99.i, %postload172.i
  br i1 %extract39.i, label %preload104.i, label %postload105.i

preload104.i:                                     ; preds = %postload100.i
  store double %phi103.i, double addrspace(1)* %74, align 8
  br label %postload105.i

postload105.i:                                    ; preds = %preload104.i, %postload100.i
  br i1 %extract40.i, label %preload109.i, label %postload110.i

preload109.i:                                     ; preds = %postload105.i
  store double %phi108.i, double addrspace(1)* %75, align 8
  br label %postload110.i

postload110.i:                                    ; preds = %preload109.i, %postload105.i
  br i1 %extract41.i, label %preload114.i, label %postload115.i

preload114.i:                                     ; preds = %postload110.i
  store double %phi113.i, double addrspace(1)* %76, align 8
  br label %postload115.i

postload115.i:                                    ; preds = %preload114.i, %postload110.i
  br i1 %extract42.i, label %preload119.i, label %postload120.i

preload119.i:                                     ; preds = %postload115.i
  store double %phi118.i, double addrspace(1)* %77, align 8
  br label %postload120.i

postload120.i:                                    ; preds = %preload119.i, %postload115.i
  br i1 %extract43.i, label %preload124.i, label %postload125.i

preload124.i:                                     ; preds = %postload120.i
  store double %phi123.i, double addrspace(1)* %78, align 8
  br label %postload125.i

postload125.i:                                    ; preds = %preload124.i, %postload120.i
  br i1 %extract44.i, label %preload129.i, label %postload130.i

preload129.i:                                     ; preds = %postload125.i
  store double %phi128.i, double addrspace(1)* %79, align 8
  br label %postload130.i

postload130.i:                                    ; preds = %preload129.i, %postload125.i
  br i1 %extract45.i, label %preload134.i, label %postload135.i

preload134.i:                                     ; preds = %postload130.i
  store double %phi133.i, double addrspace(1)* %80, align 8
  br label %postload135.i

postload135.i:                                    ; preds = %preload134.i, %postload130.i
  br i1 %extract46.i, label %preload139.i, label %postload140.i

preload139.i:                                     ; preds = %postload135.i
  store double %phi138.i, double addrspace(1)* %81, align 8
  br label %postload140.i

postload140.i:                                    ; preds = %preload139.i, %postload135.i
  br i1 %extract47.i, label %preload144.i, label %postload145.i

preload144.i:                                     ; preds = %postload140.i
  store double %phi143.i, double addrspace(1)* %82, align 8
  br label %postload145.i

postload145.i:                                    ; preds = %preload144.i, %postload140.i
  br i1 %extract48.i, label %preload149.i, label %postload150.i

preload149.i:                                     ; preds = %postload145.i
  store double %phi148.i, double addrspace(1)* %83, align 8
  br label %postload150.i

postload150.i:                                    ; preds = %preload149.i, %postload145.i
  br i1 %extract49.i, label %preload154.i, label %postload155.i

preload154.i:                                     ; preds = %postload150.i
  store double %phi153.i, double addrspace(1)* %84, align 8
  br label %postload155.i

postload155.i:                                    ; preds = %preload154.i, %postload150.i
  br i1 %extract50.i, label %preload159.i, label %postload160.i

preload159.i:                                     ; preds = %postload155.i
  store double %phi158.i, double addrspace(1)* %85, align 8
  br label %postload160.i

postload160.i:                                    ; preds = %preload159.i, %postload155.i
  br i1 %extract51.i, label %preload164.i, label %postload165.i

preload164.i:                                     ; preds = %postload160.i
  store double %phi163.i, double addrspace(1)* %86, align 8
  br label %postload165.i

postload165.i:                                    ; preds = %preload164.i, %postload160.i
  br i1 %extract52.i, label %preload169.i, label %postload170.i

preload169.i:                                     ; preds = %postload165.i
  store double %phi168.i, double addrspace(1)* %87, align 8
  br label %postload170.i

postload170.i:                                    ; preds = %preload169.i, %postload165.i
  br i1 %extract53.i, label %preload174.i, label %postload175.i

preload174.i:                                     ; preds = %postload170.i
  store double %phi173.i, double addrspace(1)* %88, align 8
  br label %postload175.i

postload175.i:                                    ; preds = %preload174.i, %postload170.i
  %90 = add nsw i32 %53, 1
  %exitcond.i = icmp eq i32 %90, %19
  %temp75.i = insertelement <16 x i1> undef, i1 %exitcond.i, i32 0
  %vector76.i = shufflevector <16 x i1> %temp75.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond.i = xor i1 %exitcond.i, true
  %temp81.i = insertelement <16 x i1> undef, i1 %notCond.i, i32 0
  %vector82.i = shufflevector <16 x i1> %temp81.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr77.i = and <16 x i1> %vectorPHI15.i, %vector76.i
  %loop_mask379.i = or <16 x i1> %vectorPHI.i, %who_left_tr77.i
  %curr_loop_mask80.i = or <16 x i1> %loop_mask379.i, %who_left_tr77.i
  %ipred.i1.i = bitcast <16 x i1> %curr_loop_mask80.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %tmp.i3.i = and i32 %val.i2.i, 1
  %res.i4.i = icmp eq i32 %tmp.i3.i, 0
  %local_edge83.i = and <16 x i1> %vectorPHI15.i, %vector82.i
  br i1 %res.i4.i, label %52, label %.loopexit.i

.loopexit.i:                                      ; preds = %postload175.i, %SyncBB176.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.CopyRect_separated_args.exit

thenBB.i:                                         ; preds = %.loopexit.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB176.i

____Vectorized_.CopyRect_separated_args.exit:     ; preds = %.loopexit.i
  ret void
}

!opencl.kernels = !{!0, !2}
!opencl_StencilKernel_locals_anchor = !{!3}

!0 = metadata !{void (double addrspace(1)*, i32, i32, double addrspace(1)*, i32, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__CopyRect_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int, int, double __attribute__((address_space(1))) *, int, int, int, int", metadata !"opencl_CopyRect_locals_anchor", void (i8*)* @CopyRect}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (double addrspace(1)*, double addrspace(1)*, double, double, double, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__StencilKernel_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, double __attribute__((address_space(1))) *, double, double, double", metadata !"opencl_StencilKernel_locals_anchor", void (i8*)* @StencilKernel}
!3 = metadata !{metadata !"opencl_StencilKernel_local_sh"}
