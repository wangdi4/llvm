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

@opencl_spmv_csr_vector_kernel_local_partialSums = internal addrspace(3) global [128 x double] zeroinitializer, align 16

declare void @__spmv_csr_scalar_kernel_original(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture) nounwind

declare i64 @get_global_id(i32)

declare void @__spmv_csr_vector_kernel_original(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture) nounwind

declare i64 @get_local_id(i32)

declare i64 @get_local_size(i32)

declare i64 @get_group_id(i32)

declare void @barrier(i64)

declare void @__spmv_ellpackr_kernel_original(double addrspace(1)* noalias nocapture, double addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32 addrspace(1)* noalias nocapture, i32, double addrspace(1)* noalias nocapture) nounwind

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__spmv_csr_scalar_kernel_separated_args(double addrspace(1)* noalias nocapture %val, double addrspace(1)* noalias nocapture %vec, i32 addrspace(1)* noalias nocapture %cols, i32 addrspace(1)* noalias nocapture %rowDelimiters, i32 %dim, double addrspace(1)* noalias nocapture %out, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %7 = icmp slt i32 %6, %dim
  br i1 %7, label %8, label %UnifiedReturnBlock

; <label>:8                                       ; preds = %SyncBB
  %9 = sext i32 %6 to i64
  %10 = getelementptr inbounds i32 addrspace(1)* %rowDelimiters, i64 %9
  %11 = load i32 addrspace(1)* %10, align 4
  %12 = add nsw i32 %6, 1
  %13 = sext i32 %12 to i64
  %14 = getelementptr inbounds i32 addrspace(1)* %rowDelimiters, i64 %13
  %15 = load i32 addrspace(1)* %14, align 4
  %16 = icmp slt i32 %11, %15
  br i1 %16, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %8
  %tmp = add i32 %15, -1
  %tmp3 = sub i32 %tmp, %11
  %tmp4 = zext i32 %tmp3 to i64
  %tmp5 = add i64 %tmp4, 1
  %tmp6 = sext i32 %11 to i64
  br label %17

; <label>:17                                      ; preds = %17, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %17 ]
  %t.01 = phi double [ 0.000000e+00, %bb.nph ], [ %24, %17 ]
  %tmp7 = add i64 %tmp6, %indvar
  %scevgep = getelementptr i32 addrspace(1)* %cols, i64 %tmp7
  %scevgep8 = getelementptr double addrspace(1)* %val, i64 %tmp7
  %18 = load i32 addrspace(1)* %scevgep, align 4
  %19 = load double addrspace(1)* %scevgep8, align 8
  %20 = sext i32 %18 to i64
  %21 = getelementptr inbounds double addrspace(1)* %vec, i64 %20
  %22 = load double addrspace(1)* %21, align 8
  %23 = fmul double %19, %22
  %24 = fadd double %t.01, %23
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %tmp5
  br i1 %exitcond, label %._crit_edge, label %17

._crit_edge:                                      ; preds = %17, %8
  %t.0.lcssa = phi double [ 0.000000e+00, %8 ], [ %24, %17 ]
  %25 = getelementptr inbounds double addrspace(1)* %out, i64 %9
  store double %t.0.lcssa, double addrspace(1)* %25, align 8
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %SyncBB, %._crit_edge
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB9

thenBB:                                           ; preds = %UnifiedReturnBlock
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB9:                                          ; preds = %UnifiedReturnBlock
  ret void
}

define void @__spmv_csr_vector_kernel_separated_args(double addrspace(1)* noalias nocapture %val, double addrspace(1)* noalias nocapture %vec, i32 addrspace(1)* noalias nocapture %cols, i32 addrspace(1)* noalias nocapture %rowDelimiters, i32 %dim, double addrspace(1)* noalias nocapture %out, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to [128 x double] addrspace(3)*
  br label %SyncBB159.outer

SyncBB159.outer:                                  ; preds = %0, %thenBB196
  %CurrWI..4.ph = phi i64 [ %"CurrWI++200", %thenBB196 ], [ 0, %0 ]
  %CurrSBIndex..4.ph = phi i64 [ %"loadedCurrSB+Stride202", %thenBB196 ], [ 0, %0 ]
  %currBarrier.3.ph = phi i32 [ %currBarrier.2, %thenBB196 ], [ 11, %0 ]
  br label %SyncBB159

SyncBB159:                                        ; preds = %thenBB168, %SyncBB159.outer
  %CurrWI..4 = phi i64 [ %"CurrWI++172", %thenBB168 ], [ %CurrWI..4.ph, %SyncBB159.outer ]
  %CurrSBIndex..4 = phi i64 [ %"loadedCurrSB+Stride174", %thenBB168 ], [ %CurrSBIndex..4.ph, %SyncBB159.outer ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..4, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = trunc i64 %3 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..4
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %4, i32* %CastToValueType, align 4
  %5 = and i32 %4, 31
  %"&(pSB[currWI].offset)38" = add nuw i64 %CurrSBIndex..4, 4
  %"&pSB[currWI].offset39" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)38"
  %CastToValueType40 = bitcast i8* %"&pSB[currWI].offset39" to i32*
  store i32 %5, i32* %CastToValueType40, align 4
  %6 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %7 = load i64* %6, align 8
  %8 = load i64* %pWGId, align 8
  %9 = shl i64 %7, 27
  %10 = ashr i64 %9, 32
  %11 = mul i64 %10, %8
  %12 = sdiv i32 %4, 32
  %13 = zext i32 %12 to i64
  %14 = add i64 %11, %13
  %15 = trunc i64 %14 to i32
  %16 = sext i32 %4 to i64
  %17 = getelementptr inbounds [128 x double] addrspace(3)* %1, i64 0, i64 %16
  %"&(pSB[currWI].offset)72" = add nuw i64 %CurrSBIndex..4, 8
  %"&pSB[currWI].offset73" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)72"
  %CastToValueType74 = bitcast i8* %"&pSB[currWI].offset73" to double addrspace(3)**
  store double addrspace(3)* %17, double addrspace(3)** %CastToValueType74, align 8
  volatile store double 0.000000e+00, double addrspace(3)* %17, align 8
  %18 = icmp slt i32 %15, %dim
  br i1 %18, label %19, label %UnifiedReturnBlock

; <label>:19                                      ; preds = %SyncBB159
  %20 = sext i32 %15 to i64
  %"&(pSB[currWI].offset)136" = add nuw i64 %CurrSBIndex..4, 16
  %"&pSB[currWI].offset137" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)136"
  %CastToValueType138 = bitcast i8* %"&pSB[currWI].offset137" to i64*
  store i64 %20, i64* %CastToValueType138, align 8
  %21 = getelementptr inbounds i32 addrspace(1)* %rowDelimiters, i64 %20
  %22 = load i32 addrspace(1)* %21, align 4
  %23 = add nsw i32 %15, 1
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds i32 addrspace(1)* %rowDelimiters, i64 %24
  %26 = load i32 addrspace(1)* %25, align 4
  %"&(pSB[currWI].offset)42" = add nuw i64 %CurrSBIndex..4, 4
  %"&pSB[currWI].offset43" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)42"
  %CastToValueType44 = bitcast i8* %"&pSB[currWI].offset43" to i32*
  %loadedValue45 = load i32* %CastToValueType44, align 4
  %27 = add nsw i32 %22, %loadedValue45
  %28 = icmp slt i32 %27, %26
  br i1 %28, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %19
  %tmp6 = sext i32 %27 to i64
  %tmp9 = add i32 %27, 32
  %tmp10 = zext i32 %tmp9 to i64
  br label %29

; <label>:29                                      ; preds = %29, %bb.nph
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %29 ]
  %mySum.01 = phi double [ 0.000000e+00, %bb.nph ], [ %36, %29 ]
  %tmp = shl i64 %indvar, 5
  %tmp7 = add i64 %tmp6, %tmp
  %scevgep = getelementptr i32 addrspace(1)* %cols, i64 %tmp7
  %scevgep8 = getelementptr double addrspace(1)* %val, i64 %tmp7
  %30 = load i32 addrspace(1)* %scevgep, align 4
  %31 = load double addrspace(1)* %scevgep8, align 8
  %32 = sext i32 %30 to i64
  %33 = getelementptr inbounds double addrspace(1)* %vec, i64 %32
  %34 = load double addrspace(1)* %33, align 8
  %35 = fmul double %31, %34
  %36 = fadd double %mySum.01, %35
  %tmp11 = add i64 %tmp10, %tmp
  %tmp12 = trunc i64 %tmp11 to i32
  %37 = icmp slt i32 %tmp12, %26
  %indvar.next = add i64 %indvar, 1
  br i1 %37, label %29, label %._crit_edge

._crit_edge:                                      ; preds = %29, %19
  %mySum.0.lcssa = phi double [ 0.000000e+00, %19 ], [ %36, %29 ]
  %"&(pSB[currWI].offset)131" = add nuw i64 %CurrSBIndex..4, 8
  %"&pSB[currWI].offset132" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)131"
  %CastToValueType133 = bitcast i8* %"&pSB[currWI].offset132" to double addrspace(3)**
  %loadedValue134 = load double addrspace(3)** %CastToValueType133, align 8
  volatile store double %mySum.0.lcssa, double addrspace(3)* %loadedValue134, align 8
  %check.WI.iter171 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter171, label %thenBB168, label %SyncBB154

thenBB168:                                        ; preds = %._crit_edge
  %"CurrWI++172" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride174" = add nuw i64 %CurrSBIndex..4, 32
  br label %SyncBB159

SyncBB154:                                        ; preds = %._crit_edge, %thenBB175
  %CurrWI..6 = phi i64 [ %"CurrWI++179", %thenBB175 ], [ 0, %._crit_edge ]
  %CurrSBIndex..6 = phi i64 [ %"loadedCurrSB+Stride181", %thenBB175 ], [ 0, %._crit_edge ]
  %"&(pSB[currWI].offset)671" = or i64 %CurrSBIndex..6, 4
  %"&pSB[currWI].offset68" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)671"
  %CastToValueType69 = bitcast i8* %"&pSB[currWI].offset68" to i32*
  %loadedValue70 = load i32* %CastToValueType69, align 4
  %38 = icmp ult i32 %loadedValue70, 16
  br i1 %38, label %39, label %46

; <label>:39                                      ; preds = %SyncBB154
  %"&pSB[currWI].offset34" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..6
  %CastToValueType35 = bitcast i8* %"&pSB[currWI].offset34" to i32*
  %loadedValue36 = load i32* %CastToValueType35, align 4
  %40 = add nsw i32 %loadedValue36, 16
  %41 = sext i32 %40 to i64
  %42 = getelementptr inbounds [128 x double] addrspace(3)* %1, i64 0, i64 %41
  %43 = volatile load double addrspace(3)* %42, align 8
  %"&(pSB[currWI].offset)12115" = or i64 %CurrSBIndex..6, 8
  %"&pSB[currWI].offset122" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)12115"
  %CastToValueType123 = bitcast i8* %"&pSB[currWI].offset122" to double addrspace(3)**
  %loadedValue124 = load double addrspace(3)** %CastToValueType123, align 8
  %44 = volatile load double addrspace(3)* %loadedValue124, align 8
  %45 = fadd double %44, %43
  %"&(pSB[currWI].offset)12616" = or i64 %CurrSBIndex..6, 8
  %"&pSB[currWI].offset127" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)12616"
  %CastToValueType128 = bitcast i8* %"&pSB[currWI].offset127" to double addrspace(3)**
  %loadedValue129 = load double addrspace(3)** %CastToValueType128, align 8
  volatile store double %45, double addrspace(3)* %loadedValue129, align 8
  br label %46

; <label>:46                                      ; preds = %39, %SyncBB154
  %check.WI.iter178 = icmp ult i64 %CurrWI..6, %iterCount
  br i1 %check.WI.iter178, label %thenBB175, label %SyncBB155

thenBB175:                                        ; preds = %46
  %"CurrWI++179" = add nuw i64 %CurrWI..6, 1
  %"loadedCurrSB+Stride181" = add nuw i64 %CurrSBIndex..6, 32
  br label %SyncBB154

SyncBB155:                                        ; preds = %46, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %46 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %46 ]
  %"&(pSB[currWI].offset)622" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset63" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)622"
  %CastToValueType64 = bitcast i8* %"&pSB[currWI].offset63" to i32*
  %loadedValue65 = load i32* %CastToValueType64, align 4
  %47 = icmp ult i32 %loadedValue65, 8
  br i1 %47, label %48, label %55

; <label>:48                                      ; preds = %SyncBB155
  %"&pSB[currWI].offset29" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType30 = bitcast i8* %"&pSB[currWI].offset29" to i32*
  %loadedValue31 = load i32* %CastToValueType30, align 4
  %49 = add nsw i32 %loadedValue31, 8
  %50 = sext i32 %49 to i64
  %51 = getelementptr inbounds [128 x double] addrspace(3)* %1, i64 0, i64 %50
  %52 = volatile load double addrspace(3)* %51, align 8
  %"&(pSB[currWI].offset)11113" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset112" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)11113"
  %CastToValueType113 = bitcast i8* %"&pSB[currWI].offset112" to double addrspace(3)**
  %loadedValue114 = load double addrspace(3)** %CastToValueType113, align 8
  %53 = volatile load double addrspace(3)* %loadedValue114, align 8
  %54 = fadd double %53, %52
  %"&(pSB[currWI].offset)11614" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset117" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)11614"
  %CastToValueType118 = bitcast i8* %"&pSB[currWI].offset117" to double addrspace(3)**
  %loadedValue119 = load double addrspace(3)** %CastToValueType118, align 8
  volatile store double %54, double addrspace(3)* %loadedValue119, align 8
  br label %55

; <label>:55                                      ; preds = %48, %SyncBB155
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %55
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 32
  br label %SyncBB155

SyncBB:                                           ; preds = %55, %thenBB161
  %CurrWI..5 = phi i64 [ %"CurrWI++165", %thenBB161 ], [ 0, %55 ]
  %CurrSBIndex..5 = phi i64 [ %"loadedCurrSB+Stride167", %thenBB161 ], [ 0, %55 ]
  %"&(pSB[currWI].offset)573" = or i64 %CurrSBIndex..5, 4
  %"&pSB[currWI].offset58" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)573"
  %CastToValueType59 = bitcast i8* %"&pSB[currWI].offset58" to i32*
  %loadedValue60 = load i32* %CastToValueType59, align 4
  %56 = icmp ult i32 %loadedValue60, 4
  br i1 %56, label %57, label %64

; <label>:57                                      ; preds = %SyncBB
  %"&pSB[currWI].offset24" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..5
  %CastToValueType25 = bitcast i8* %"&pSB[currWI].offset24" to i32*
  %loadedValue26 = load i32* %CastToValueType25, align 4
  %58 = add nsw i32 %loadedValue26, 4
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds [128 x double] addrspace(3)* %1, i64 0, i64 %59
  %61 = volatile load double addrspace(3)* %60, align 8
  %"&(pSB[currWI].offset)10111" = or i64 %CurrSBIndex..5, 8
  %"&pSB[currWI].offset102" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10111"
  %CastToValueType103 = bitcast i8* %"&pSB[currWI].offset102" to double addrspace(3)**
  %loadedValue104 = load double addrspace(3)** %CastToValueType103, align 8
  %62 = volatile load double addrspace(3)* %loadedValue104, align 8
  %63 = fadd double %62, %61
  %"&(pSB[currWI].offset)10612" = or i64 %CurrSBIndex..5, 8
  %"&pSB[currWI].offset107" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10612"
  %CastToValueType108 = bitcast i8* %"&pSB[currWI].offset107" to double addrspace(3)**
  %loadedValue109 = load double addrspace(3)** %CastToValueType108, align 8
  volatile store double %63, double addrspace(3)* %loadedValue109, align 8
  br label %64

; <label>:64                                      ; preds = %57, %SyncBB
  %check.WI.iter164 = icmp ult i64 %CurrWI..5, %iterCount
  br i1 %check.WI.iter164, label %thenBB161, label %SyncBB153

thenBB161:                                        ; preds = %64
  %"CurrWI++165" = add nuw i64 %CurrWI..5, 1
  %"loadedCurrSB+Stride167" = add nuw i64 %CurrSBIndex..5, 32
  br label %SyncBB

SyncBB153:                                        ; preds = %64, %thenBB182
  %CurrWI..7 = phi i64 [ %"CurrWI++186", %thenBB182 ], [ 0, %64 ]
  %CurrSBIndex..7 = phi i64 [ %"loadedCurrSB+Stride188", %thenBB182 ], [ 0, %64 ]
  %"&(pSB[currWI].offset)524" = or i64 %CurrSBIndex..7, 4
  %"&pSB[currWI].offset53" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)524"
  %CastToValueType54 = bitcast i8* %"&pSB[currWI].offset53" to i32*
  %loadedValue55 = load i32* %CastToValueType54, align 4
  %65 = icmp ult i32 %loadedValue55, 2
  br i1 %65, label %66, label %73

; <label>:66                                      ; preds = %SyncBB153
  %"&pSB[currWI].offset19" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..7
  %CastToValueType20 = bitcast i8* %"&pSB[currWI].offset19" to i32*
  %loadedValue21 = load i32* %CastToValueType20, align 4
  %67 = add nsw i32 %loadedValue21, 2
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds [128 x double] addrspace(3)* %1, i64 0, i64 %68
  %70 = volatile load double addrspace(3)* %69, align 8
  %"&(pSB[currWI].offset)919" = or i64 %CurrSBIndex..7, 8
  %"&pSB[currWI].offset92" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)919"
  %CastToValueType93 = bitcast i8* %"&pSB[currWI].offset92" to double addrspace(3)**
  %loadedValue94 = load double addrspace(3)** %CastToValueType93, align 8
  %71 = volatile load double addrspace(3)* %loadedValue94, align 8
  %72 = fadd double %71, %70
  %"&(pSB[currWI].offset)9610" = or i64 %CurrSBIndex..7, 8
  %"&pSB[currWI].offset97" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)9610"
  %CastToValueType98 = bitcast i8* %"&pSB[currWI].offset97" to double addrspace(3)**
  %loadedValue99 = load double addrspace(3)** %CastToValueType98, align 8
  volatile store double %72, double addrspace(3)* %loadedValue99, align 8
  br label %73

; <label>:73                                      ; preds = %66, %SyncBB153
  %check.WI.iter185 = icmp ult i64 %CurrWI..7, %iterCount
  br i1 %check.WI.iter185, label %thenBB182, label %SyncBB156

thenBB182:                                        ; preds = %73
  %"CurrWI++186" = add nuw i64 %CurrWI..7, 1
  %"loadedCurrSB+Stride188" = add nuw i64 %CurrSBIndex..7, 32
  br label %SyncBB153

SyncBB156:                                        ; preds = %73, %thenBB189
  %CurrWI..8 = phi i64 [ %"CurrWI++193", %thenBB189 ], [ 0, %73 ]
  %CurrSBIndex..8 = phi i64 [ %"loadedCurrSB+Stride195", %thenBB189 ], [ 0, %73 ]
  %"&(pSB[currWI].offset)475" = or i64 %CurrSBIndex..8, 4
  %"&pSB[currWI].offset48" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)475"
  %CastToValueType49 = bitcast i8* %"&pSB[currWI].offset48" to i32*
  %loadedValue50 = load i32* %CastToValueType49, align 4
  %74 = icmp eq i32 %loadedValue50, 0
  %"&(pSB[currWI].offset)1456" = or i64 %CurrSBIndex..8, 24
  %"&pSB[currWI].offset146" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1456"
  %CastToValueType147 = bitcast i8* %"&pSB[currWI].offset146" to i1*
  store i1 %74, i1* %CastToValueType147, align 1
  br i1 %74, label %75, label %82

; <label>:75                                      ; preds = %SyncBB156
  %"&pSB[currWI].offset15" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..8
  %CastToValueType16 = bitcast i8* %"&pSB[currWI].offset15" to i32*
  %loadedValue = load i32* %CastToValueType16, align 4
  %76 = add nsw i32 %loadedValue, 1
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds [128 x double] addrspace(3)* %1, i64 0, i64 %77
  %79 = volatile load double addrspace(3)* %78, align 8
  %"&(pSB[currWI].offset)817" = or i64 %CurrSBIndex..8, 8
  %"&pSB[currWI].offset82" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)817"
  %CastToValueType83 = bitcast i8* %"&pSB[currWI].offset82" to double addrspace(3)**
  %loadedValue84 = load double addrspace(3)** %CastToValueType83, align 8
  %80 = volatile load double addrspace(3)* %loadedValue84, align 8
  %81 = fadd double %80, %79
  %"&(pSB[currWI].offset)868" = or i64 %CurrSBIndex..8, 8
  %"&pSB[currWI].offset87" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)868"
  %CastToValueType88 = bitcast i8* %"&pSB[currWI].offset87" to double addrspace(3)**
  %loadedValue89 = load double addrspace(3)** %CastToValueType88, align 8
  volatile store double %81, double addrspace(3)* %loadedValue89, align 8
  br label %82

; <label>:82                                      ; preds = %75, %SyncBB156
  %check.WI.iter192 = icmp ult i64 %CurrWI..8, %iterCount
  br i1 %check.WI.iter192, label %thenBB189, label %SyncBB157

thenBB189:                                        ; preds = %82
  %"CurrWI++193" = add nuw i64 %CurrWI..8, 1
  %"loadedCurrSB+Stride195" = add nuw i64 %CurrSBIndex..8, 32
  br label %SyncBB156

SyncBB157:                                        ; preds = %82, %thenBB196
  %CurrWI..1 = phi i64 [ %"CurrWI++200", %thenBB196 ], [ 0, %82 ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride202", %thenBB196 ], [ 0, %82 ]
  %currBarrier.0 = phi i32 [ %currBarrier.2, %thenBB196 ], [ 5, %82 ]
  %"&(pSB[currWI].offset)149" = add nuw i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset150" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)149"
  %CastToValueType151 = bitcast i8* %"&pSB[currWI].offset150" to i1*
  %loadedValue152 = load i1* %CastToValueType151, align 1
  br i1 %loadedValue152, label %83, label %UnifiedReturnBlock

; <label>:83                                      ; preds = %SyncBB157
  %"&(pSB[currWI].offset)76" = add nuw i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset77" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)76"
  %CastToValueType78 = bitcast i8* %"&pSB[currWI].offset77" to double addrspace(3)**
  %loadedValue79 = load double addrspace(3)** %CastToValueType78, align 8
  %84 = volatile load double addrspace(3)* %loadedValue79, align 8
  %"&(pSB[currWI].offset)140" = add nuw i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset141" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)140"
  %CastToValueType142 = bitcast i8* %"&pSB[currWI].offset141" to i64*
  %loadedValue143 = load i64* %CastToValueType142, align 8
  %85 = getelementptr inbounds double addrspace(1)* %out, i64 %loadedValue143
  store double %84, double addrspace(1)* %85, align 8
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %SyncBB157, %SyncBB159, %83
  %CurrWI..3 = phi i64 [ %CurrWI..1, %83 ], [ %CurrWI..1, %SyncBB157 ], [ %CurrWI..4, %SyncBB159 ]
  %CurrSBIndex..3 = phi i64 [ %CurrSBIndex..1, %83 ], [ %CurrSBIndex..1, %SyncBB157 ], [ %CurrSBIndex..4, %SyncBB159 ]
  %currBarrier.2 = phi i32 [ %currBarrier.0, %83 ], [ %currBarrier.0, %SyncBB157 ], [ %currBarrier.3.ph, %SyncBB159 ]
  %check.WI.iter199 = icmp ult i64 %CurrWI..3, %iterCount
  br i1 %check.WI.iter199, label %thenBB196, label %SyncBB158

thenBB196:                                        ; preds = %UnifiedReturnBlock
  %"CurrWI++200" = add nuw i64 %CurrWI..3, 1
  %"loadedCurrSB+Stride202" = add nuw i64 %CurrSBIndex..3, 32
  %cond = icmp eq i32 %currBarrier.2, 11
  br i1 %cond, label %SyncBB159.outer, label %SyncBB157

SyncBB158:                                        ; preds = %UnifiedReturnBlock
  ret void
}

define void @__spmv_ellpackr_kernel_separated_args(double addrspace(1)* noalias nocapture %val, double addrspace(1)* noalias nocapture %vec, i32 addrspace(1)* noalias nocapture %cols, i32 addrspace(1)* noalias nocapture %rowLengths, i32 %dim, double addrspace(1)* noalias nocapture %out, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %7 = icmp slt i32 %6, %dim
  br i1 %7, label %8, label %UnifiedReturnBlock

; <label>:8                                       ; preds = %SyncBB
  %9 = sext i32 %6 to i64
  %10 = getelementptr inbounds i32 addrspace(1)* %rowLengths, i64 %9
  %11 = load i32 addrspace(1)* %10, align 4
  %12 = icmp sgt i32 %11, 0
  br i1 %12, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %8, %bb.nph
  %i.02 = phi i32 [ %23, %bb.nph ], [ 0, %8 ]
  %result.01 = phi double [ %22, %bb.nph ], [ 0.000000e+00, %8 ]
  %tmp = mul i32 %i.02, %dim
  %tmp4 = add i32 %6, %tmp
  %13 = sext i32 %tmp4 to i64
  %14 = getelementptr inbounds double addrspace(1)* %val, i64 %13
  %15 = load double addrspace(1)* %14, align 8
  %16 = getelementptr inbounds i32 addrspace(1)* %cols, i64 %13
  %17 = load i32 addrspace(1)* %16, align 4
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds double addrspace(1)* %vec, i64 %18
  %20 = load double addrspace(1)* %19, align 8
  %21 = fmul double %15, %20
  %22 = fadd double %result.01, %21
  %23 = add nsw i32 %i.02, 1
  %exitcond = icmp eq i32 %23, %11
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %8
  %result.0.lcssa = phi double [ 0.000000e+00, %8 ], [ %22, %bb.nph ]
  %24 = getelementptr inbounds double addrspace(1)* %out, i64 %9
  store double %result.0.lcssa, double addrspace(1)* %24, align 8
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %SyncBB, %._crit_edge
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB5

thenBB:                                           ; preds = %UnifiedReturnBlock
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB5:                                          ; preds = %UnifiedReturnBlock
  ret void
}

define void @spmv_csr_vector_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to double addrspace(1)**
  %16 = load double addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to i8 addrspace(3)**
  %19 = load i8 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to %struct.WorkDim**
  %22 = load %struct.WorkDim** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 64
  %24 = bitcast i8* %23 to i64**
  %25 = load i64** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 80
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 96
  %30 = bitcast i8* %29 to i64*
  %31 = load i64* %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 104
  %33 = bitcast i8* %32 to i8**
  %34 = load i8** %33, align 8
  %35 = bitcast i8 addrspace(3)* %19 to [128 x double] addrspace(3)*
  br label %SyncBB159.outer.i

SyncBB159.outer.i:                                ; preds = %thenBB196.i, %entry
  %CurrWI..4.ph.i = phi i64 [ %"CurrWI++200.i", %thenBB196.i ], [ 0, %entry ]
  %CurrSBIndex..4.ph.i = phi i64 [ %"loadedCurrSB+Stride202.i", %thenBB196.i ], [ 0, %entry ]
  %currBarrier.3.ph.i = phi i32 [ %currBarrier.2.i, %thenBB196.i ], [ 11, %entry ]
  br label %SyncBB159.i

SyncBB159.i:                                      ; preds = %thenBB168.i, %SyncBB159.outer.i
  %CurrWI..4.i = phi i64 [ %"CurrWI++172.i", %thenBB168.i ], [ %CurrWI..4.ph.i, %SyncBB159.outer.i ]
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride174.i", %thenBB168.i ], [ %CurrSBIndex..4.ph.i, %SyncBB159.outer.i ]
  %36 = getelementptr %struct.PaddedDimId* %28, i64 %CurrWI..4.i, i32 0, i64 0
  %37 = load i64* %36, align 8
  %38 = trunc i64 %37 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..4.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %38, i32* %CastToValueType.i, align 4
  %39 = and i32 %38, 31
  %"&(pSB[currWI].offset)38.i" = add nuw i64 %CurrSBIndex..4.i, 4
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)38.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to i32*
  store i32 %39, i32* %CastToValueType40.i, align 4
  %40 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %41 = load i64* %40, align 8
  %42 = load i64* %25, align 8
  %43 = shl i64 %41, 27
  %44 = ashr i64 %43, 32
  %45 = mul i64 %44, %42
  %46 = sdiv i32 %38, 32
  %47 = zext i32 %46 to i64
  %48 = add i64 %45, %47
  %49 = trunc i64 %48 to i32
  %50 = sext i32 %38 to i64
  %51 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %50
  %"&(pSB[currWI].offset)72.i" = add nuw i64 %CurrSBIndex..4.i, 8
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)72.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to double addrspace(3)**
  store double addrspace(3)* %51, double addrspace(3)** %CastToValueType74.i, align 8
  volatile store double 0.000000e+00, double addrspace(3)* %51, align 8
  %52 = icmp slt i32 %49, %13
  br i1 %52, label %53, label %UnifiedReturnBlock.i

; <label>:53                                      ; preds = %SyncBB159.i
  %54 = sext i32 %49 to i64
  %"&(pSB[currWI].offset)136.i" = add nuw i64 %CurrSBIndex..4.i, 16
  %"&pSB[currWI].offset137.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)136.i"
  %CastToValueType138.i = bitcast i8* %"&pSB[currWI].offset137.i" to i64*
  store i64 %54, i64* %CastToValueType138.i, align 8
  %55 = getelementptr inbounds i32 addrspace(1)* %10, i64 %54
  %56 = load i32 addrspace(1)* %55, align 4
  %57 = add nsw i32 %49, 1
  %58 = sext i32 %57 to i64
  %59 = getelementptr inbounds i32 addrspace(1)* %10, i64 %58
  %60 = load i32 addrspace(1)* %59, align 4
  %"&(pSB[currWI].offset)42.i" = add nuw i64 %CurrSBIndex..4.i, 4
  %"&pSB[currWI].offset43.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)42.i"
  %CastToValueType44.i = bitcast i8* %"&pSB[currWI].offset43.i" to i32*
  %loadedValue45.i = load i32* %CastToValueType44.i, align 4
  %61 = add nsw i32 %56, %loadedValue45.i
  %62 = icmp slt i32 %61, %60
  br i1 %62, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %53
  %tmp6.i = sext i32 %61 to i64
  %tmp9.i = add i32 %61, 32
  %tmp10.i = zext i32 %tmp9.i to i64
  br label %63

; <label>:63                                      ; preds = %63, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %63 ]
  %mySum.01.i = phi double [ 0.000000e+00, %bb.nph.i ], [ %70, %63 ]
  %tmp.i = shl i64 %indvar.i, 5
  %tmp7.i = add i64 %tmp6.i, %tmp.i
  %scevgep.i = getelementptr i32 addrspace(1)* %7, i64 %tmp7.i
  %scevgep8.i = getelementptr double addrspace(1)* %1, i64 %tmp7.i
  %64 = load i32 addrspace(1)* %scevgep.i, align 4
  %65 = load double addrspace(1)* %scevgep8.i, align 8
  %66 = sext i32 %64 to i64
  %67 = getelementptr inbounds double addrspace(1)* %4, i64 %66
  %68 = load double addrspace(1)* %67, align 8
  %69 = fmul double %65, %68
  %70 = fadd double %mySum.01.i, %69
  %tmp11.i = add i64 %tmp10.i, %tmp.i
  %tmp12.i = trunc i64 %tmp11.i to i32
  %71 = icmp slt i32 %tmp12.i, %60
  %indvar.next.i = add i64 %indvar.i, 1
  br i1 %71, label %63, label %._crit_edge.i

._crit_edge.i:                                    ; preds = %63, %53
  %mySum.0.lcssa.i = phi double [ 0.000000e+00, %53 ], [ %70, %63 ]
  %"&(pSB[currWI].offset)131.i" = add nuw i64 %CurrSBIndex..4.i, 8
  %"&pSB[currWI].offset132.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)131.i"
  %CastToValueType133.i = bitcast i8* %"&pSB[currWI].offset132.i" to double addrspace(3)**
  %loadedValue134.i = load double addrspace(3)** %CastToValueType133.i, align 8
  volatile store double %mySum.0.lcssa.i, double addrspace(3)* %loadedValue134.i, align 8
  %check.WI.iter171.i = icmp ult i64 %CurrWI..4.i, %31
  br i1 %check.WI.iter171.i, label %thenBB168.i, label %SyncBB154.i

thenBB168.i:                                      ; preds = %._crit_edge.i
  %"CurrWI++172.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride174.i" = add nuw i64 %CurrSBIndex..4.i, 32
  br label %SyncBB159.i

SyncBB154.i:                                      ; preds = %thenBB175.i, %._crit_edge.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++179.i", %thenBB175.i ], [ 0, %._crit_edge.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride181.i", %thenBB175.i ], [ 0, %._crit_edge.i ]
  %"&(pSB[currWI].offset)671.i" = or i64 %CurrSBIndex..6.i, 4
  %"&pSB[currWI].offset68.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)671.i"
  %CastToValueType69.i = bitcast i8* %"&pSB[currWI].offset68.i" to i32*
  %loadedValue70.i = load i32* %CastToValueType69.i, align 4
  %72 = icmp ult i32 %loadedValue70.i, 16
  br i1 %72, label %73, label %80

; <label>:73                                      ; preds = %SyncBB154.i
  %"&pSB[currWI].offset34.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..6.i
  %CastToValueType35.i = bitcast i8* %"&pSB[currWI].offset34.i" to i32*
  %loadedValue36.i = load i32* %CastToValueType35.i, align 4
  %74 = add nsw i32 %loadedValue36.i, 16
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %75
  %77 = volatile load double addrspace(3)* %76, align 8
  %"&(pSB[currWI].offset)12115.i" = or i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset122.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)12115.i"
  %CastToValueType123.i = bitcast i8* %"&pSB[currWI].offset122.i" to double addrspace(3)**
  %loadedValue124.i = load double addrspace(3)** %CastToValueType123.i, align 8
  %78 = volatile load double addrspace(3)* %loadedValue124.i, align 8
  %79 = fadd double %78, %77
  %"&(pSB[currWI].offset)12616.i" = or i64 %CurrSBIndex..6.i, 8
  %"&pSB[currWI].offset127.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)12616.i"
  %CastToValueType128.i = bitcast i8* %"&pSB[currWI].offset127.i" to double addrspace(3)**
  %loadedValue129.i = load double addrspace(3)** %CastToValueType128.i, align 8
  volatile store double %79, double addrspace(3)* %loadedValue129.i, align 8
  br label %80

; <label>:80                                      ; preds = %73, %SyncBB154.i
  %check.WI.iter178.i = icmp ult i64 %CurrWI..6.i, %31
  br i1 %check.WI.iter178.i, label %thenBB175.i, label %SyncBB155.i

thenBB175.i:                                      ; preds = %80
  %"CurrWI++179.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride181.i" = add nuw i64 %CurrSBIndex..6.i, 32
  br label %SyncBB154.i

SyncBB155.i:                                      ; preds = %thenBB.i, %80
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %80 ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %80 ]
  %"&(pSB[currWI].offset)622.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset63.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)622.i"
  %CastToValueType64.i = bitcast i8* %"&pSB[currWI].offset63.i" to i32*
  %loadedValue65.i = load i32* %CastToValueType64.i, align 4
  %81 = icmp ult i32 %loadedValue65.i, 8
  br i1 %81, label %82, label %89

; <label>:82                                      ; preds = %SyncBB155.i
  %"&pSB[currWI].offset29.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..0.i
  %CastToValueType30.i = bitcast i8* %"&pSB[currWI].offset29.i" to i32*
  %loadedValue31.i = load i32* %CastToValueType30.i, align 4
  %83 = add nsw i32 %loadedValue31.i, 8
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %84
  %86 = volatile load double addrspace(3)* %85, align 8
  %"&(pSB[currWI].offset)11113.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset112.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)11113.i"
  %CastToValueType113.i = bitcast i8* %"&pSB[currWI].offset112.i" to double addrspace(3)**
  %loadedValue114.i = load double addrspace(3)** %CastToValueType113.i, align 8
  %87 = volatile load double addrspace(3)* %loadedValue114.i, align 8
  %88 = fadd double %87, %86
  %"&(pSB[currWI].offset)11614.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset117.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)11614.i"
  %CastToValueType118.i = bitcast i8* %"&pSB[currWI].offset117.i" to double addrspace(3)**
  %loadedValue119.i = load double addrspace(3)** %CastToValueType118.i, align 8
  volatile store double %88, double addrspace(3)* %loadedValue119.i, align 8
  br label %89

; <label>:89                                      ; preds = %82, %SyncBB155.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %31
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %89
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 32
  br label %SyncBB155.i

SyncBB.i:                                         ; preds = %thenBB161.i, %89
  %CurrWI..5.i = phi i64 [ %"CurrWI++165.i", %thenBB161.i ], [ 0, %89 ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride167.i", %thenBB161.i ], [ 0, %89 ]
  %"&(pSB[currWI].offset)573.i" = or i64 %CurrSBIndex..5.i, 4
  %"&pSB[currWI].offset58.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)573.i"
  %CastToValueType59.i = bitcast i8* %"&pSB[currWI].offset58.i" to i32*
  %loadedValue60.i = load i32* %CastToValueType59.i, align 4
  %90 = icmp ult i32 %loadedValue60.i, 4
  br i1 %90, label %91, label %98

; <label>:91                                      ; preds = %SyncBB.i
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..5.i
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i32*
  %loadedValue26.i = load i32* %CastToValueType25.i, align 4
  %92 = add nsw i32 %loadedValue26.i, 4
  %93 = sext i32 %92 to i64
  %94 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %93
  %95 = volatile load double addrspace(3)* %94, align 8
  %"&(pSB[currWI].offset)10111.i" = or i64 %CurrSBIndex..5.i, 8
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)10111.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to double addrspace(3)**
  %loadedValue104.i = load double addrspace(3)** %CastToValueType103.i, align 8
  %96 = volatile load double addrspace(3)* %loadedValue104.i, align 8
  %97 = fadd double %96, %95
  %"&(pSB[currWI].offset)10612.i" = or i64 %CurrSBIndex..5.i, 8
  %"&pSB[currWI].offset107.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)10612.i"
  %CastToValueType108.i = bitcast i8* %"&pSB[currWI].offset107.i" to double addrspace(3)**
  %loadedValue109.i = load double addrspace(3)** %CastToValueType108.i, align 8
  volatile store double %97, double addrspace(3)* %loadedValue109.i, align 8
  br label %98

; <label>:98                                      ; preds = %91, %SyncBB.i
  %check.WI.iter164.i = icmp ult i64 %CurrWI..5.i, %31
  br i1 %check.WI.iter164.i, label %thenBB161.i, label %SyncBB153.i

thenBB161.i:                                      ; preds = %98
  %"CurrWI++165.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride167.i" = add nuw i64 %CurrSBIndex..5.i, 32
  br label %SyncBB.i

SyncBB153.i:                                      ; preds = %thenBB182.i, %98
  %CurrWI..7.i = phi i64 [ %"CurrWI++186.i", %thenBB182.i ], [ 0, %98 ]
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride188.i", %thenBB182.i ], [ 0, %98 ]
  %"&(pSB[currWI].offset)524.i" = or i64 %CurrSBIndex..7.i, 4
  %"&pSB[currWI].offset53.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)524.i"
  %CastToValueType54.i = bitcast i8* %"&pSB[currWI].offset53.i" to i32*
  %loadedValue55.i = load i32* %CastToValueType54.i, align 4
  %99 = icmp ult i32 %loadedValue55.i, 2
  br i1 %99, label %100, label %107

; <label>:100                                     ; preds = %SyncBB153.i
  %"&pSB[currWI].offset19.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..7.i
  %CastToValueType20.i = bitcast i8* %"&pSB[currWI].offset19.i" to i32*
  %loadedValue21.i = load i32* %CastToValueType20.i, align 4
  %101 = add nsw i32 %loadedValue21.i, 2
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %102
  %104 = volatile load double addrspace(3)* %103, align 8
  %"&(pSB[currWI].offset)919.i" = or i64 %CurrSBIndex..7.i, 8
  %"&pSB[currWI].offset92.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)919.i"
  %CastToValueType93.i = bitcast i8* %"&pSB[currWI].offset92.i" to double addrspace(3)**
  %loadedValue94.i = load double addrspace(3)** %CastToValueType93.i, align 8
  %105 = volatile load double addrspace(3)* %loadedValue94.i, align 8
  %106 = fadd double %105, %104
  %"&(pSB[currWI].offset)9610.i" = or i64 %CurrSBIndex..7.i, 8
  %"&pSB[currWI].offset97.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)9610.i"
  %CastToValueType98.i = bitcast i8* %"&pSB[currWI].offset97.i" to double addrspace(3)**
  %loadedValue99.i = load double addrspace(3)** %CastToValueType98.i, align 8
  volatile store double %106, double addrspace(3)* %loadedValue99.i, align 8
  br label %107

; <label>:107                                     ; preds = %100, %SyncBB153.i
  %check.WI.iter185.i = icmp ult i64 %CurrWI..7.i, %31
  br i1 %check.WI.iter185.i, label %thenBB182.i, label %SyncBB156.i

thenBB182.i:                                      ; preds = %107
  %"CurrWI++186.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride188.i" = add nuw i64 %CurrSBIndex..7.i, 32
  br label %SyncBB153.i

SyncBB156.i:                                      ; preds = %thenBB189.i, %107
  %CurrWI..8.i = phi i64 [ %"CurrWI++193.i", %thenBB189.i ], [ 0, %107 ]
  %CurrSBIndex..8.i = phi i64 [ %"loadedCurrSB+Stride195.i", %thenBB189.i ], [ 0, %107 ]
  %"&(pSB[currWI].offset)475.i" = or i64 %CurrSBIndex..8.i, 4
  %"&pSB[currWI].offset48.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)475.i"
  %CastToValueType49.i = bitcast i8* %"&pSB[currWI].offset48.i" to i32*
  %loadedValue50.i = load i32* %CastToValueType49.i, align 4
  %108 = icmp eq i32 %loadedValue50.i, 0
  %"&(pSB[currWI].offset)1456.i" = or i64 %CurrSBIndex..8.i, 24
  %"&pSB[currWI].offset146.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)1456.i"
  %CastToValueType147.i = bitcast i8* %"&pSB[currWI].offset146.i" to i1*
  store i1 %108, i1* %CastToValueType147.i, align 1
  br i1 %108, label %109, label %116

; <label>:109                                     ; preds = %SyncBB156.i
  %"&pSB[currWI].offset15.i" = getelementptr inbounds i8* %34, i64 %CurrSBIndex..8.i
  %CastToValueType16.i = bitcast i8* %"&pSB[currWI].offset15.i" to i32*
  %loadedValue.i = load i32* %CastToValueType16.i, align 4
  %110 = add nsw i32 %loadedValue.i, 1
  %111 = sext i32 %110 to i64
  %112 = getelementptr inbounds [128 x double] addrspace(3)* %35, i64 0, i64 %111
  %113 = volatile load double addrspace(3)* %112, align 8
  %"&(pSB[currWI].offset)817.i" = or i64 %CurrSBIndex..8.i, 8
  %"&pSB[currWI].offset82.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)817.i"
  %CastToValueType83.i = bitcast i8* %"&pSB[currWI].offset82.i" to double addrspace(3)**
  %loadedValue84.i = load double addrspace(3)** %CastToValueType83.i, align 8
  %114 = volatile load double addrspace(3)* %loadedValue84.i, align 8
  %115 = fadd double %114, %113
  %"&(pSB[currWI].offset)868.i" = or i64 %CurrSBIndex..8.i, 8
  %"&pSB[currWI].offset87.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)868.i"
  %CastToValueType88.i = bitcast i8* %"&pSB[currWI].offset87.i" to double addrspace(3)**
  %loadedValue89.i = load double addrspace(3)** %CastToValueType88.i, align 8
  volatile store double %115, double addrspace(3)* %loadedValue89.i, align 8
  br label %116

; <label>:116                                     ; preds = %109, %SyncBB156.i
  %check.WI.iter192.i = icmp ult i64 %CurrWI..8.i, %31
  br i1 %check.WI.iter192.i, label %thenBB189.i, label %SyncBB157.i

thenBB189.i:                                      ; preds = %116
  %"CurrWI++193.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride195.i" = add nuw i64 %CurrSBIndex..8.i, 32
  br label %SyncBB156.i

SyncBB157.i:                                      ; preds = %thenBB196.i, %116
  %CurrWI..1.i = phi i64 [ %"CurrWI++200.i", %thenBB196.i ], [ 0, %116 ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride202.i", %thenBB196.i ], [ 0, %116 ]
  %currBarrier.0.i = phi i32 [ %currBarrier.2.i, %thenBB196.i ], [ 5, %116 ]
  %"&(pSB[currWI].offset)149.i" = add nuw i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset150.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)149.i"
  %CastToValueType151.i = bitcast i8* %"&pSB[currWI].offset150.i" to i1*
  %loadedValue152.i = load i1* %CastToValueType151.i, align 1
  br i1 %loadedValue152.i, label %117, label %UnifiedReturnBlock.i

; <label>:117                                     ; preds = %SyncBB157.i
  %"&(pSB[currWI].offset)76.i" = add nuw i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset77.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)76.i"
  %CastToValueType78.i = bitcast i8* %"&pSB[currWI].offset77.i" to double addrspace(3)**
  %loadedValue79.i = load double addrspace(3)** %CastToValueType78.i, align 8
  %118 = volatile load double addrspace(3)* %loadedValue79.i, align 8
  %"&(pSB[currWI].offset)140.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset141.i" = getelementptr inbounds i8* %34, i64 %"&(pSB[currWI].offset)140.i"
  %CastToValueType142.i = bitcast i8* %"&pSB[currWI].offset141.i" to i64*
  %loadedValue143.i = load i64* %CastToValueType142.i, align 8
  %119 = getelementptr inbounds double addrspace(1)* %16, i64 %loadedValue143.i
  store double %118, double addrspace(1)* %119, align 8
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %117, %SyncBB157.i, %SyncBB159.i
  %CurrWI..3.i = phi i64 [ %CurrWI..1.i, %117 ], [ %CurrWI..1.i, %SyncBB157.i ], [ %CurrWI..4.i, %SyncBB159.i ]
  %CurrSBIndex..3.i = phi i64 [ %CurrSBIndex..1.i, %117 ], [ %CurrSBIndex..1.i, %SyncBB157.i ], [ %CurrSBIndex..4.i, %SyncBB159.i ]
  %currBarrier.2.i = phi i32 [ %currBarrier.0.i, %117 ], [ %currBarrier.0.i, %SyncBB157.i ], [ %currBarrier.3.ph.i, %SyncBB159.i ]
  %check.WI.iter199.i = icmp ult i64 %CurrWI..3.i, %31
  br i1 %check.WI.iter199.i, label %thenBB196.i, label %__spmv_csr_vector_kernel_separated_args.exit

thenBB196.i:                                      ; preds = %UnifiedReturnBlock.i
  %"CurrWI++200.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride202.i" = add nuw i64 %CurrSBIndex..3.i, 32
  %cond.i = icmp eq i32 %currBarrier.2.i, 11
  br i1 %cond.i, label %SyncBB159.outer.i, label %SyncBB157.i

__spmv_csr_vector_kernel_separated_args.exit:     ; preds = %UnifiedReturnBlock.i
  ret void
}

define void @spmv_csr_scalar_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to double addrspace(1)**
  %16 = load double addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 96
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %26 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %29 = load i64* %28, align 8
  %30 = add i64 %27, %29
  %31 = trunc i64 %30 to i32
  %32 = icmp slt i32 %31, %13
  br i1 %32, label %33, label %UnifiedReturnBlock.i

; <label>:33                                      ; preds = %SyncBB.i
  %34 = sext i32 %31 to i64
  %35 = getelementptr inbounds i32 addrspace(1)* %10, i64 %34
  %36 = load i32 addrspace(1)* %35, align 4
  %37 = add nsw i32 %31, 1
  %38 = sext i32 %37 to i64
  %39 = getelementptr inbounds i32 addrspace(1)* %10, i64 %38
  %40 = load i32 addrspace(1)* %39, align 4
  %41 = icmp slt i32 %36, %40
  br i1 %41, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %33
  %tmp.i = add i32 %40, -1
  %tmp3.i = sub i32 %tmp.i, %36
  %tmp4.i = zext i32 %tmp3.i to i64
  %tmp5.i = add i64 %tmp4.i, 1
  %tmp6.i = sext i32 %36 to i64
  br label %42

; <label>:42                                      ; preds = %42, %bb.nph.i
  %indvar.i = phi i64 [ 0, %bb.nph.i ], [ %indvar.next.i, %42 ]
  %t.01.i = phi double [ 0.000000e+00, %bb.nph.i ], [ %49, %42 ]
  %tmp7.i = add i64 %tmp6.i, %indvar.i
  %scevgep.i = getelementptr i32 addrspace(1)* %7, i64 %tmp7.i
  %scevgep8.i = getelementptr double addrspace(1)* %1, i64 %tmp7.i
  %43 = load i32 addrspace(1)* %scevgep.i, align 4
  %44 = load double addrspace(1)* %scevgep8.i, align 8
  %45 = sext i32 %43 to i64
  %46 = getelementptr inbounds double addrspace(1)* %4, i64 %45
  %47 = load double addrspace(1)* %46, align 8
  %48 = fmul double %44, %47
  %49 = fadd double %t.01.i, %48
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond.i = icmp eq i64 %indvar.next.i, %tmp5.i
  br i1 %exitcond.i, label %._crit_edge.i, label %42

._crit_edge.i:                                    ; preds = %42, %33
  %t.0.lcssa.i = phi double [ 0.000000e+00, %33 ], [ %49, %42 ]
  %50 = getelementptr inbounds double addrspace(1)* %16, i64 %34
  store double %t.0.lcssa.i, double addrspace(1)* %50, align 8
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %._crit_edge.i, %SyncBB.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %__spmv_csr_scalar_kernel_separated_args.exit

thenBB.i:                                         ; preds = %UnifiedReturnBlock.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__spmv_csr_scalar_kernel_separated_args.exit:     ; preds = %UnifiedReturnBlock.i
  ret void
}

define void @spmv_ellpackr_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32 addrspace(1)**
  %7 = load i32 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32 addrspace(1)**
  %10 = load i32 addrspace(1)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to double addrspace(1)**
  %16 = load double addrspace(1)** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 80
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 96
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %26 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %27 = load i64* %26, align 8
  %28 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %29 = load i64* %28, align 8
  %30 = add i64 %27, %29
  %31 = trunc i64 %30 to i32
  %32 = icmp slt i32 %31, %13
  br i1 %32, label %33, label %UnifiedReturnBlock.i

; <label>:33                                      ; preds = %SyncBB.i
  %34 = sext i32 %31 to i64
  %35 = getelementptr inbounds i32 addrspace(1)* %10, i64 %34
  %36 = load i32 addrspace(1)* %35, align 4
  %37 = icmp sgt i32 %36, 0
  br i1 %37, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %33
  %i.02.i = phi i32 [ %48, %bb.nph.i ], [ 0, %33 ]
  %result.01.i = phi double [ %47, %bb.nph.i ], [ 0.000000e+00, %33 ]
  %tmp.i = mul i32 %i.02.i, %13
  %tmp4.i = add i32 %31, %tmp.i
  %38 = sext i32 %tmp4.i to i64
  %39 = getelementptr inbounds double addrspace(1)* %1, i64 %38
  %40 = load double addrspace(1)* %39, align 8
  %41 = getelementptr inbounds i32 addrspace(1)* %7, i64 %38
  %42 = load i32 addrspace(1)* %41, align 4
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds double addrspace(1)* %4, i64 %43
  %45 = load double addrspace(1)* %44, align 8
  %46 = fmul double %40, %45
  %47 = fadd double %result.01.i, %46
  %48 = add nsw i32 %i.02.i, 1
  %exitcond.i = icmp eq i32 %48, %36
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %33
  %result.0.lcssa.i = phi double [ 0.000000e+00, %33 ], [ %47, %bb.nph.i ]
  %49 = getelementptr inbounds double addrspace(1)* %16, i64 %34
  store double %result.0.lcssa.i, double addrspace(1)* %49, align 8
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %._crit_edge.i, %SyncBB.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %__spmv_ellpackr_kernel_separated_args.exit

thenBB.i:                                         ; preds = %UnifiedReturnBlock.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__spmv_ellpackr_kernel_separated_args.exit:       ; preds = %UnifiedReturnBlock.i
  ret void
}

!opencl.kernels = !{!0, !2, !3}
!opencl_spmv_csr_vector_kernel_locals_anchor = !{!4}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__spmv_csr_scalar_kernel_separated_args, metadata !1, metadata !1, metadata !"float4", metadata !"double const __attribute__((address_space(1))) *restrict, double const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const, double __attribute__((address_space(1))) *restrict", metadata !"opencl_spmv_csr_scalar_kernel_locals_anchor", void (i8*)* @spmv_csr_scalar_kernel}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__spmv_csr_vector_kernel_separated_args, metadata !1, metadata !1, metadata !"float4", metadata !"double const __attribute__((address_space(1))) *restrict, double const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const, double __attribute__((address_space(1))) *restrict", metadata !"opencl_spmv_csr_vector_kernel_locals_anchor", void (i8*)* @spmv_csr_vector_kernel}
!3 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32, double addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__spmv_ellpackr_kernel_separated_args, metadata !1, metadata !1, metadata !"float4", metadata !"double const __attribute__((address_space(1))) *restrict, double const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const __attribute__((address_space(1))) *restrict, int const, double __attribute__((address_space(1))) *restrict", metadata !"opencl_spmv_ellpackr_kernel_locals_anchor", void (i8*)* @spmv_ellpackr_kernel}
!4 = metadata !{metadata !"opencl_spmv_csr_vector_kernel_local_partialSums"}
