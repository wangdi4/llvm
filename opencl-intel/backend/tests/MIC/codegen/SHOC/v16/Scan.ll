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

@opencl_addUniform_local_uni.0 = internal addrspace(3) global float 0.000000e+00
@opencl_scan_local_s_data = internal addrspace(3) global [512 x float] zeroinitializer, align 16

declare void @__addUniform_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_local_id(i32)

declare i64 @get_group_id(i32)

declare i64 @get_local_size(i32)

declare void @barrier(i64)

declare void @__scan_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32) nounwind

declare i64 @get_global_id(i32)

declare void @____Vectorized_.addUniform_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare void @____Vectorized_.scan_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32, i32, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

declare i64 @maskedf_0_get_group_id(i1, i32)

declare float @masked_load0(i1, float addrspace(1)*)

declare void @masked_store0(i1, float, float addrspace(3)*)

declare float @masked_load1(i1, float addrspace(3)*)

declare float @masked_load2(i1, float addrspace(1)*)

declare void @masked_store1(i1, float, float addrspace(1)*)

declare i64 @maskedf_1_get_local_size(i1, i32)

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

declare <4 x float> @masked_load6(i1, <4 x float> addrspace(1)*)

declare float @masked_load7(i1, float addrspace(1)*)

declare float @masked_load8(i1, float addrspace(1)*)

declare float @masked_load9(i1, float addrspace(1)*)

declare i64 @maskedf_2_get_local_size(i1, i32)

declare i64 @maskedf_3_get_group_id(i1, i32)

declare void @masked_store4(i1, float, float addrspace(1)*)

declare void @masked_store5(i1, <4 x float>, <4 x float> addrspace(1)*)

declare void @masked_store6(i1, float, float addrspace(1)*)

declare void @masked_store7(i1, float, float addrspace(1)*)

declare void @masked_store8(i1, float, float addrspace(1)*)

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__addUniform_separated_args(float addrspace(1)* nocapture %d_vector, float addrspace(1)* nocapture %d_uniforms, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to float addrspace(3)*
  br label %SyncBB6

SyncBB6:                                          ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = icmp eq i64 %3, 0
  br i1 %4, label %5, label %"Barrier BB"

; <label>:5                                       ; preds = %SyncBB6
  %6 = load i64* %pWGId, align 8
  %7 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %6
  %8 = load float addrspace(1)* %7, align 4
  store float %8, float addrspace(3)* %1, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %5, %SyncBB6
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = load i64* %pWGId, align 8
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %13 = load i64* %12, align 8
  %14 = shl i64 %11, 2
  %15 = mul i64 %14, %13
  %16 = add i64 %15, %10
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %16, i64* %CastToValueType, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %"Barrier BB"
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 1792
  br label %SyncBB6

SyncBB:                                           ; preds = %"Barrier BB", %thenBB8
  %CurrWI..1 = phi i64 [ %"CurrWI++12", %thenBB8 ], [ 0, %"Barrier BB" ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride14", %thenBB8 ], [ 0, %"Barrier BB" ]
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i64*
  %loadedValue = load i64* %CastToValueType4, align 8
  br label %17

; <label>:17                                      ; preds = %22, %SyncBB
  %address.0.in = phi i64 [ %loadedValue, %SyncBB ], [ %30, %22 ]
  %18 = phi i32 [ 0, %SyncBB ], [ %31, %22 ]
  %19 = icmp slt i32 %18, 4
  br i1 %19, label %20, label %.critedge

; <label>:20                                      ; preds = %17
  %address.0 = trunc i64 %address.0.in to i32
  %21 = icmp ult i32 %address.0, %n
  br i1 %21, label %22, label %.critedge

; <label>:22                                      ; preds = %20
  %23 = load float addrspace(3)* %1, align 4
  %24 = and i64 %address.0.in, 4294967295
  %25 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %24
  %26 = load float addrspace(1)* %25, align 4
  %27 = fadd float %26, %23
  store float %27, float addrspace(1)* %25, align 4
  %28 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %29 = load i64* %28, align 8
  %30 = add i64 %29, %24
  %31 = add nsw i32 %18, 1
  br label %17

.critedge:                                        ; preds = %20, %17
  %check.WI.iter11 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter11, label %thenBB8, label %SyncBB5

thenBB8:                                          ; preds = %.critedge
  %"CurrWI++12" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride14" = add nuw i64 %CurrSBIndex..1, 1792
  br label %SyncBB

SyncBB5:                                          ; preds = %.critedge
  ret void
}

define void @__scan_separated_args(float addrspace(1)* nocapture %g_odata, float addrspace(1)* nocapture %g_idata, float addrspace(1)* nocapture %g_blockSums, i32 %n, i32 %fullBlock, i32 %storeSum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [512 x float] addrspace(3)*
  %1 = bitcast float addrspace(1)* %g_idata to <4 x float> addrspace(1)*
  %2 = icmp eq i32 %fullBlock, 0
  br label %SyncBB382

SyncBB382:                                        ; preds = %thenBB402, %FirstBB
  %CurrSBIndex..1 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride408", %thenBB402 ]
  %CurrWI..1 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++406", %thenBB402 ]
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %8 = trunc i64 %7 to i32
  %"&(pSB[currWI].offset)1" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %8, i32* %CastToValueType, align 4
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %10 = load i64* %9, align 8
  %"&(pSB[currWI].offset)322" = or i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset33" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)322"
  %CastToValueType34 = bitcast i8* %"&pSB[currWI].offset33" to i64*
  store i64 %10, i64* %CastToValueType34, align 8
  %11 = shl i32 %8, 2
  %"&(pSB[currWI].offset)413" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset42" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)413"
  %CastToValueType43 = bitcast i8* %"&pSB[currWI].offset42" to i32*
  store i32 %11, i32* %CastToValueType43, align 4
  br i1 %2, label %12, label %15

; <label>:12                                      ; preds = %SyncBB382
  %"&(pSB[currWI].offset)7072" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset71" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)7072"
  %CastToValueType72 = bitcast i8* %"&pSB[currWI].offset71" to i32*
  %loadedValue73 = load i32* %CastToValueType72, align 4
  %13 = or i32 %loadedValue73, 3
  %14 = icmp slt i32 %13, %n
  br i1 %14, label %15, label %29

; <label>:15                                      ; preds = %12, %SyncBB382
  %"&(pSB[currWI].offset)234" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset24" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)234"
  %CastToValueType25 = bitcast i8* %"&pSB[currWI].offset24" to i32*
  %loadedValue = load i32* %CastToValueType25, align 4
  %16 = sext i32 %loadedValue to i64
  %17 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %16
  %18 = load <4 x float> addrspace(1)* %17, align 16
  %19 = extractelement <4 x float> %18, i32 1
  %20 = extractelement <4 x float> %18, i32 0
  %21 = fadd float %19, %20
  %22 = insertelement <4 x float> %18, float %21, i32 1
  %23 = extractelement <4 x float> %18, i32 2
  %24 = fadd float %23, %21
  %25 = insertelement <4 x float> %22, float %24, i32 2
  %26 = extractelement <4 x float> %18, i32 3
  %27 = fadd float %26, %24
  %28 = insertelement <4 x float> %25, float %27, i32 3
  br label %"Barrier BB"

; <label>:29                                      ; preds = %12
  %"&(pSB[currWI].offset)9073" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset91" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)9073"
  %CastToValueType92 = bitcast i8* %"&pSB[currWI].offset91" to i32*
  %loadedValue93 = load i32* %CastToValueType92, align 4
  %30 = icmp slt i32 %loadedValue93, %n
  br i1 %30, label %31, label %35

; <label>:31                                      ; preds = %29
  %"&(pSB[currWI].offset)8576" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset86" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)8576"
  %CastToValueType87 = bitcast i8* %"&pSB[currWI].offset86" to i32*
  %loadedValue88 = load i32* %CastToValueType87, align 4
  %32 = sext i32 %loadedValue88 to i64
  %33 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %32
  %34 = load float addrspace(1)* %33, align 4
  br label %35

; <label>:35                                      ; preds = %31, %29
  %36 = phi float [ %34, %31 ], [ 0.000000e+00, %29 ]
  %37 = insertelement <4 x float> undef, float %36, i32 0
  %"&(pSB[currWI].offset)6574" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset66" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)6574"
  %CastToValueType67 = bitcast i8* %"&pSB[currWI].offset66" to i32*
  %loadedValue68 = load i32* %CastToValueType67, align 4
  %38 = or i32 %loadedValue68, 1
  %39 = icmp slt i32 %38, %n
  br i1 %39, label %40, label %44

; <label>:40                                      ; preds = %35
  %41 = sext i32 %38 to i64
  %42 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %41
  %43 = load float addrspace(1)* %42, align 4
  br label %44

; <label>:44                                      ; preds = %40, %35
  %45 = phi float [ %43, %40 ], [ 0.000000e+00, %35 ]
  %46 = fadd float %45, %36
  %47 = insertelement <4 x float> %37, float %46, i32 1
  %"&(pSB[currWI].offset)6075" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset61" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)6075"
  %CastToValueType62 = bitcast i8* %"&pSB[currWI].offset61" to i32*
  %loadedValue63 = load i32* %CastToValueType62, align 4
  %48 = or i32 %loadedValue63, 2
  %49 = icmp slt i32 %48, %n
  br i1 %49, label %50, label %54

; <label>:50                                      ; preds = %44
  %51 = sext i32 %48 to i64
  %52 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %51
  %53 = load float addrspace(1)* %52, align 4
  br label %54

; <label>:54                                      ; preds = %50, %44
  %55 = phi float [ %53, %50 ], [ 0.000000e+00, %44 ]
  %56 = fadd float %55, %46
  %57 = insertelement <4 x float> %47, float %56, i32 2
  %58 = fadd float %56, 0.000000e+00
  %59 = insertelement <4 x float> %57, float %58, i32 3
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %54, %15
  %threadScanT.0 = phi <4 x float> [ %28, %15 ], [ %59, %54 ]
  %res.0 = phi float [ %27, %15 ], [ %58, %54 ]
  %"&(pSB[currWI].offset)1195" = or i64 %CurrSBIndex..1, 48
  %"&pSB[currWI].offset120" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1195"
  %CastToValueType121 = bitcast i8* %"&pSB[currWI].offset120" to float*
  store float %res.0, float* %CastToValueType121, align 4
  %"&(pSB[currWI].offset)956" = or i64 %CurrSBIndex..1, 32
  %"&pSB[currWI].offset96" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)956"
  %CastToValueType97 = bitcast i8* %"&pSB[currWI].offset96" to <4 x float>*
  store <4 x float> %threadScanT.0, <4 x float>* %CastToValueType97, align 16
  %check.WI.iter405 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter405, label %thenBB402, label %SyncBB380

thenBB402:                                        ; preds = %"Barrier BB"
  %"CurrWI++406" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride408" = add nuw i64 %CurrSBIndex..1, 1792
  br label %SyncBB382

SyncBB380:                                        ; preds = %"Barrier BB", %thenBB416
  %CurrSBIndex..3 = phi i64 [ %"loadedCurrSB+Stride422", %thenBB416 ], [ 0, %"Barrier BB" ]
  %CurrWI..3 = phi i64 [ %"CurrWI++420", %thenBB416 ], [ 0, %"Barrier BB" ]
  %60 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..3, i32 0, i64 0
  %61 = load i64* %60, align 8
  %"&(pSB[currWI].offset)1287" = or i64 %CurrSBIndex..3, 56
  %"&pSB[currWI].offset129" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1287"
  %CastToValueType130 = bitcast i8* %"&pSB[currWI].offset129" to i64*
  store i64 %61, i64* %CastToValueType130, align 8
  %sext.i = shl i64 %61, 32
  %62 = ashr i64 %sext.i, 32
  %63 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %62
  store float 0.000000e+00, float addrspace(3)* %63, align 4
  %check.WI.iter419 = icmp ult i64 %CurrWI..3, %iterCount
  br i1 %check.WI.iter419, label %thenBB416, label %SyncBB383

thenBB416:                                        ; preds = %SyncBB380
  %"CurrWI++420" = add nuw i64 %CurrWI..3, 1
  %"loadedCurrSB+Stride422" = add nuw i64 %CurrSBIndex..3, 1792
  br label %SyncBB380

SyncBB383:                                        ; preds = %SyncBB380, %thenBB423
  %CurrSBIndex..4 = phi i64 [ %"loadedCurrSB+Stride429", %thenBB423 ], [ 0, %SyncBB380 ]
  %CurrWI..4 = phi i64 [ %"CurrWI++427", %thenBB423 ], [ 0, %SyncBB380 ]
  %64 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %65 = load i64* %64, align 8
  %"&(pSB[currWI].offset)1328" = or i64 %CurrSBIndex..4, 56
  %"&pSB[currWI].offset133" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1328"
  %CastToValueType134 = bitcast i8* %"&pSB[currWI].offset133" to i64*
  %loadedValue135 = load i64* %CastToValueType134, align 8
  %66 = add i64 %65, %loadedValue135
  %67 = trunc i64 %66 to i32
  %"&(pSB[currWI].offset)1379" = or i64 %CurrSBIndex..4, 64
  %"&pSB[currWI].offset138" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1379"
  %CastToValueType139 = bitcast i8* %"&pSB[currWI].offset138" to i32*
  store i32 %67, i32* %CastToValueType139, align 4
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %68
  %"&(pSB[currWI].offset)18110" = or i64 %CurrSBIndex..4, 72
  %"&pSB[currWI].offset182" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)18110"
  %CastToValueType183 = bitcast i8* %"&pSB[currWI].offset182" to float addrspace(3)**
  store float addrspace(3)* %69, float addrspace(3)** %CastToValueType183, align 8
  %"&(pSB[currWI].offset)12311" = or i64 %CurrSBIndex..4, 48
  %"&pSB[currWI].offset124" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)12311"
  %CastToValueType125 = bitcast i8* %"&pSB[currWI].offset124" to float*
  %loadedValue126 = load float* %CastToValueType125, align 4
  store float %loadedValue126, float addrspace(3)* %69, align 4
  %check.WI.iter426 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter426, label %thenBB423, label %SyncBB384

thenBB423:                                        ; preds = %SyncBB383
  %"CurrWI++427" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride429" = add nuw i64 %CurrSBIndex..4, 1792
  br label %SyncBB383

SyncBB384:                                        ; preds = %SyncBB383, %thenBB430
  %CurrSBIndex..5 = phi i64 [ %"loadedCurrSB+Stride436", %thenBB430 ], [ 0, %SyncBB383 ]
  %CurrWI..5 = phi i64 [ %"CurrWI++434", %thenBB430 ], [ 0, %SyncBB383 ]
  %"&(pSB[currWI].offset)17612" = or i64 %CurrSBIndex..5, 64
  %"&pSB[currWI].offset177" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)17612"
  %CastToValueType178 = bitcast i8* %"&pSB[currWI].offset177" to i32*
  %loadedValue179 = load i32* %CastToValueType178, align 4
  %70 = add nsw i32 %loadedValue179, -1
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %71
  %"&(pSB[currWI].offset)26513" = or i64 %CurrSBIndex..5, 80
  %"&pSB[currWI].offset266" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)26513"
  %CastToValueType267 = bitcast i8* %"&pSB[currWI].offset266" to float addrspace(3)**
  store float addrspace(3)* %72, float addrspace(3)** %CastToValueType267, align 8
  %73 = load float addrspace(3)* %72, align 4
  %"&(pSB[currWI].offset)27414" = or i64 %CurrSBIndex..5, 88
  %"&pSB[currWI].offset275" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)27414"
  %CastToValueType276 = bitcast i8* %"&pSB[currWI].offset275" to float*
  store float %73, float* %CastToValueType276, align 4
  %check.WI.iter433 = icmp ult i64 %CurrWI..5, %iterCount
  br i1 %check.WI.iter433, label %thenBB430, label %SyncBB385

thenBB430:                                        ; preds = %SyncBB384
  %"CurrWI++434" = add nuw i64 %CurrWI..5, 1
  %"loadedCurrSB+Stride436" = add nuw i64 %CurrSBIndex..5, 1792
  br label %SyncBB384

SyncBB385:                                        ; preds = %SyncBB384, %thenBB472
  %CurrSBIndex..11 = phi i64 [ %"loadedCurrSB+Stride478", %thenBB472 ], [ 0, %SyncBB384 ]
  %CurrWI..11 = phi i64 [ %"CurrWI++476", %thenBB472 ], [ 0, %SyncBB384 ]
  %"&(pSB[currWI].offset)26015" = or i64 %CurrSBIndex..11, 72
  %"&pSB[currWI].offset261" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)26015"
  %CastToValueType262 = bitcast i8* %"&pSB[currWI].offset261" to float addrspace(3)**
  %loadedValue263 = load float addrspace(3)** %CastToValueType262, align 8
  %74 = load float addrspace(3)* %loadedValue263, align 4
  %"&(pSB[currWI].offset)27816" = or i64 %CurrSBIndex..11, 88
  %"&pSB[currWI].offset279" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)27816"
  %CastToValueType280 = bitcast i8* %"&pSB[currWI].offset279" to float*
  %loadedValue281 = load float* %CastToValueType280, align 4
  %75 = fadd float %74, %loadedValue281
  %"&(pSB[currWI].offset)25517" = or i64 %CurrSBIndex..11, 72
  %"&pSB[currWI].offset256" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)25517"
  %CastToValueType257 = bitcast i8* %"&pSB[currWI].offset256" to float addrspace(3)**
  %loadedValue258 = load float addrspace(3)** %CastToValueType257, align 8
  store float %75, float addrspace(3)* %loadedValue258, align 4
  %check.WI.iter475 = icmp ult i64 %CurrWI..11, %iterCount
  br i1 %check.WI.iter475, label %thenBB472, label %SyncBB391

thenBB472:                                        ; preds = %SyncBB385
  %"CurrWI++476" = add nuw i64 %CurrWI..11, 1
  %"loadedCurrSB+Stride478" = add nuw i64 %CurrSBIndex..11, 1792
  br label %SyncBB385

SyncBB391:                                        ; preds = %SyncBB385, %thenBB479
  %CurrSBIndex..12 = phi i64 [ %"loadedCurrSB+Stride485", %thenBB479 ], [ 0, %SyncBB385 ]
  %CurrWI..12 = phi i64 [ %"CurrWI++483", %thenBB479 ], [ 0, %SyncBB385 ]
  %"&(pSB[currWI].offset)17118" = or i64 %CurrSBIndex..12, 64
  %"&pSB[currWI].offset172" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)17118"
  %CastToValueType173 = bitcast i8* %"&pSB[currWI].offset172" to i32*
  %loadedValue174 = load i32* %CastToValueType173, align 4
  %76 = add nsw i32 %loadedValue174, -2
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %77
  %79 = load float addrspace(3)* %78, align 4
  %"&(pSB[currWI].offset)28319" = or i64 %CurrSBIndex..12, 92
  %"&pSB[currWI].offset284" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)28319"
  %CastToValueType285 = bitcast i8* %"&pSB[currWI].offset284" to float*
  store float %79, float* %CastToValueType285, align 4
  %check.WI.iter482 = icmp ult i64 %CurrWI..12, %iterCount
  br i1 %check.WI.iter482, label %thenBB479, label %SyncBB392

thenBB479:                                        ; preds = %SyncBB391
  %"CurrWI++483" = add nuw i64 %CurrWI..12, 1
  %"loadedCurrSB+Stride485" = add nuw i64 %CurrSBIndex..12, 1792
  br label %SyncBB391

SyncBB392:                                        ; preds = %SyncBB391, %thenBB486
  %CurrSBIndex..13 = phi i64 [ %"loadedCurrSB+Stride492", %thenBB486 ], [ 0, %SyncBB391 ]
  %CurrWI..13 = phi i64 [ %"CurrWI++490", %thenBB486 ], [ 0, %SyncBB391 ]
  %"&(pSB[currWI].offset)25020" = or i64 %CurrSBIndex..13, 72
  %"&pSB[currWI].offset251" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)25020"
  %CastToValueType252 = bitcast i8* %"&pSB[currWI].offset251" to float addrspace(3)**
  %loadedValue253 = load float addrspace(3)** %CastToValueType252, align 8
  %80 = load float addrspace(3)* %loadedValue253, align 4
  %"&(pSB[currWI].offset)28721" = or i64 %CurrSBIndex..13, 92
  %"&pSB[currWI].offset288" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)28721"
  %CastToValueType289 = bitcast i8* %"&pSB[currWI].offset288" to float*
  %loadedValue290 = load float* %CastToValueType289, align 4
  %81 = fadd float %80, %loadedValue290
  %"&(pSB[currWI].offset)24522" = or i64 %CurrSBIndex..13, 72
  %"&pSB[currWI].offset246" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)24522"
  %CastToValueType247 = bitcast i8* %"&pSB[currWI].offset246" to float addrspace(3)**
  %loadedValue248 = load float addrspace(3)** %CastToValueType247, align 8
  store float %81, float addrspace(3)* %loadedValue248, align 4
  %check.WI.iter489 = icmp ult i64 %CurrWI..13, %iterCount
  br i1 %check.WI.iter489, label %thenBB486, label %SyncBB393

thenBB486:                                        ; preds = %SyncBB392
  %"CurrWI++490" = add nuw i64 %CurrWI..13, 1
  %"loadedCurrSB+Stride492" = add nuw i64 %CurrSBIndex..13, 1792
  br label %SyncBB392

SyncBB393:                                        ; preds = %SyncBB392, %thenBB493
  %CurrSBIndex..14 = phi i64 [ %"loadedCurrSB+Stride499", %thenBB493 ], [ 0, %SyncBB392 ]
  %CurrWI..14 = phi i64 [ %"CurrWI++497", %thenBB493 ], [ 0, %SyncBB392 ]
  %"&(pSB[currWI].offset)16623" = or i64 %CurrSBIndex..14, 64
  %"&pSB[currWI].offset167" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)16623"
  %CastToValueType168 = bitcast i8* %"&pSB[currWI].offset167" to i32*
  %loadedValue169 = load i32* %CastToValueType168, align 4
  %82 = add nsw i32 %loadedValue169, -4
  %83 = sext i32 %82 to i64
  %84 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %83
  %85 = load float addrspace(3)* %84, align 4
  %"&(pSB[currWI].offset)29224" = or i64 %CurrSBIndex..14, 96
  %"&pSB[currWI].offset293" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)29224"
  %CastToValueType294 = bitcast i8* %"&pSB[currWI].offset293" to float*
  store float %85, float* %CastToValueType294, align 4
  %check.WI.iter496 = icmp ult i64 %CurrWI..14, %iterCount
  br i1 %check.WI.iter496, label %thenBB493, label %SyncBB394

thenBB493:                                        ; preds = %SyncBB393
  %"CurrWI++497" = add nuw i64 %CurrWI..14, 1
  %"loadedCurrSB+Stride499" = add nuw i64 %CurrSBIndex..14, 1792
  br label %SyncBB393

SyncBB394:                                        ; preds = %SyncBB393, %thenBB437
  %CurrSBIndex..6 = phi i64 [ %"loadedCurrSB+Stride443", %thenBB437 ], [ 0, %SyncBB393 ]
  %CurrWI..6 = phi i64 [ %"CurrWI++441", %thenBB437 ], [ 0, %SyncBB393 ]
  %"&(pSB[currWI].offset)24025" = or i64 %CurrSBIndex..6, 72
  %"&pSB[currWI].offset241" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)24025"
  %CastToValueType242 = bitcast i8* %"&pSB[currWI].offset241" to float addrspace(3)**
  %loadedValue243 = load float addrspace(3)** %CastToValueType242, align 8
  %86 = load float addrspace(3)* %loadedValue243, align 4
  %"&(pSB[currWI].offset)29626" = or i64 %CurrSBIndex..6, 96
  %"&pSB[currWI].offset297" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)29626"
  %CastToValueType298 = bitcast i8* %"&pSB[currWI].offset297" to float*
  %loadedValue299 = load float* %CastToValueType298, align 4
  %87 = fadd float %86, %loadedValue299
  %"&(pSB[currWI].offset)23527" = or i64 %CurrSBIndex..6, 72
  %"&pSB[currWI].offset236" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)23527"
  %CastToValueType237 = bitcast i8* %"&pSB[currWI].offset236" to float addrspace(3)**
  %loadedValue238 = load float addrspace(3)** %CastToValueType237, align 8
  store float %87, float addrspace(3)* %loadedValue238, align 4
  %check.WI.iter440 = icmp ult i64 %CurrWI..6, %iterCount
  br i1 %check.WI.iter440, label %thenBB437, label %SyncBB386

thenBB437:                                        ; preds = %SyncBB394
  %"CurrWI++441" = add nuw i64 %CurrWI..6, 1
  %"loadedCurrSB+Stride443" = add nuw i64 %CurrSBIndex..6, 1792
  br label %SyncBB394

SyncBB386:                                        ; preds = %SyncBB394, %thenBB444
  %CurrSBIndex..7 = phi i64 [ %"loadedCurrSB+Stride450", %thenBB444 ], [ 0, %SyncBB394 ]
  %CurrWI..7 = phi i64 [ %"CurrWI++448", %thenBB444 ], [ 0, %SyncBB394 ]
  %"&(pSB[currWI].offset)16128" = or i64 %CurrSBIndex..7, 64
  %"&pSB[currWI].offset162" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)16128"
  %CastToValueType163 = bitcast i8* %"&pSB[currWI].offset162" to i32*
  %loadedValue164 = load i32* %CastToValueType163, align 4
  %88 = add nsw i32 %loadedValue164, -8
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %89
  %91 = load float addrspace(3)* %90, align 4
  %"&(pSB[currWI].offset)30129" = or i64 %CurrSBIndex..7, 100
  %"&pSB[currWI].offset302" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)30129"
  %CastToValueType303 = bitcast i8* %"&pSB[currWI].offset302" to float*
  store float %91, float* %CastToValueType303, align 4
  %check.WI.iter447 = icmp ult i64 %CurrWI..7, %iterCount
  br i1 %check.WI.iter447, label %thenBB444, label %SyncBB387

thenBB444:                                        ; preds = %SyncBB386
  %"CurrWI++448" = add nuw i64 %CurrWI..7, 1
  %"loadedCurrSB+Stride450" = add nuw i64 %CurrSBIndex..7, 1792
  br label %SyncBB386

SyncBB387:                                        ; preds = %SyncBB386, %thenBB451
  %CurrSBIndex..8 = phi i64 [ %"loadedCurrSB+Stride457", %thenBB451 ], [ 0, %SyncBB386 ]
  %CurrWI..8 = phi i64 [ %"CurrWI++455", %thenBB451 ], [ 0, %SyncBB386 ]
  %"&(pSB[currWI].offset)23030" = or i64 %CurrSBIndex..8, 72
  %"&pSB[currWI].offset231" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)23030"
  %CastToValueType232 = bitcast i8* %"&pSB[currWI].offset231" to float addrspace(3)**
  %loadedValue233 = load float addrspace(3)** %CastToValueType232, align 8
  %92 = load float addrspace(3)* %loadedValue233, align 4
  %"&(pSB[currWI].offset)30531" = or i64 %CurrSBIndex..8, 100
  %"&pSB[currWI].offset306" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)30531"
  %CastToValueType307 = bitcast i8* %"&pSB[currWI].offset306" to float*
  %loadedValue308 = load float* %CastToValueType307, align 4
  %93 = fadd float %92, %loadedValue308
  %"&(pSB[currWI].offset)22532" = or i64 %CurrSBIndex..8, 72
  %"&pSB[currWI].offset226" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)22532"
  %CastToValueType227 = bitcast i8* %"&pSB[currWI].offset226" to float addrspace(3)**
  %loadedValue228 = load float addrspace(3)** %CastToValueType227, align 8
  store float %93, float addrspace(3)* %loadedValue228, align 4
  %check.WI.iter454 = icmp ult i64 %CurrWI..8, %iterCount
  br i1 %check.WI.iter454, label %thenBB451, label %SyncBB388

thenBB451:                                        ; preds = %SyncBB387
  %"CurrWI++455" = add nuw i64 %CurrWI..8, 1
  %"loadedCurrSB+Stride457" = add nuw i64 %CurrSBIndex..8, 1792
  br label %SyncBB387

SyncBB388:                                        ; preds = %SyncBB387, %thenBB458
  %CurrSBIndex..9 = phi i64 [ %"loadedCurrSB+Stride464", %thenBB458 ], [ 0, %SyncBB387 ]
  %CurrWI..9 = phi i64 [ %"CurrWI++462", %thenBB458 ], [ 0, %SyncBB387 ]
  %"&(pSB[currWI].offset)15633" = or i64 %CurrSBIndex..9, 64
  %"&pSB[currWI].offset157" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)15633"
  %CastToValueType158 = bitcast i8* %"&pSB[currWI].offset157" to i32*
  %loadedValue159 = load i32* %CastToValueType158, align 4
  %94 = add nsw i32 %loadedValue159, -16
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %95
  %97 = load float addrspace(3)* %96, align 4
  %"&(pSB[currWI].offset)31034" = or i64 %CurrSBIndex..9, 104
  %"&pSB[currWI].offset311" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)31034"
  %CastToValueType312 = bitcast i8* %"&pSB[currWI].offset311" to float*
  store float %97, float* %CastToValueType312, align 4
  %check.WI.iter461 = icmp ult i64 %CurrWI..9, %iterCount
  br i1 %check.WI.iter461, label %thenBB458, label %SyncBB389

thenBB458:                                        ; preds = %SyncBB388
  %"CurrWI++462" = add nuw i64 %CurrWI..9, 1
  %"loadedCurrSB+Stride464" = add nuw i64 %CurrSBIndex..9, 1792
  br label %SyncBB388

SyncBB389:                                        ; preds = %SyncBB388, %thenBB465
  %CurrSBIndex..10 = phi i64 [ %"loadedCurrSB+Stride471", %thenBB465 ], [ 0, %SyncBB388 ]
  %CurrWI..10 = phi i64 [ %"CurrWI++469", %thenBB465 ], [ 0, %SyncBB388 ]
  %"&(pSB[currWI].offset)22035" = or i64 %CurrSBIndex..10, 72
  %"&pSB[currWI].offset221" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)22035"
  %CastToValueType222 = bitcast i8* %"&pSB[currWI].offset221" to float addrspace(3)**
  %loadedValue223 = load float addrspace(3)** %CastToValueType222, align 8
  %98 = load float addrspace(3)* %loadedValue223, align 4
  %"&(pSB[currWI].offset)31436" = or i64 %CurrSBIndex..10, 104
  %"&pSB[currWI].offset315" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)31436"
  %CastToValueType316 = bitcast i8* %"&pSB[currWI].offset315" to float*
  %loadedValue317 = load float* %CastToValueType316, align 4
  %99 = fadd float %98, %loadedValue317
  %"&(pSB[currWI].offset)21537" = or i64 %CurrSBIndex..10, 72
  %"&pSB[currWI].offset216" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)21537"
  %CastToValueType217 = bitcast i8* %"&pSB[currWI].offset216" to float addrspace(3)**
  %loadedValue218 = load float addrspace(3)** %CastToValueType217, align 8
  store float %99, float addrspace(3)* %loadedValue218, align 4
  %check.WI.iter468 = icmp ult i64 %CurrWI..10, %iterCount
  br i1 %check.WI.iter468, label %thenBB465, label %SyncBB390

thenBB465:                                        ; preds = %SyncBB389
  %"CurrWI++469" = add nuw i64 %CurrWI..10, 1
  %"loadedCurrSB+Stride471" = add nuw i64 %CurrSBIndex..10, 1792
  br label %SyncBB389

SyncBB390:                                        ; preds = %SyncBB389, %thenBB500
  %CurrSBIndex..15 = phi i64 [ %"loadedCurrSB+Stride506", %thenBB500 ], [ 0, %SyncBB389 ]
  %CurrWI..15 = phi i64 [ %"CurrWI++504", %thenBB500 ], [ 0, %SyncBB389 ]
  %"&(pSB[currWI].offset)15138" = or i64 %CurrSBIndex..15, 64
  %"&pSB[currWI].offset152" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)15138"
  %CastToValueType153 = bitcast i8* %"&pSB[currWI].offset152" to i32*
  %loadedValue154 = load i32* %CastToValueType153, align 4
  %100 = add nsw i32 %loadedValue154, -32
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %101
  %103 = load float addrspace(3)* %102, align 4
  %"&(pSB[currWI].offset)31939" = or i64 %CurrSBIndex..15, 108
  %"&pSB[currWI].offset320" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)31939"
  %CastToValueType321 = bitcast i8* %"&pSB[currWI].offset320" to float*
  store float %103, float* %CastToValueType321, align 4
  %check.WI.iter503 = icmp ult i64 %CurrWI..15, %iterCount
  br i1 %check.WI.iter503, label %thenBB500, label %SyncBB395

thenBB500:                                        ; preds = %SyncBB390
  %"CurrWI++504" = add nuw i64 %CurrWI..15, 1
  %"loadedCurrSB+Stride506" = add nuw i64 %CurrSBIndex..15, 1792
  br label %SyncBB390

SyncBB395:                                        ; preds = %SyncBB390, %thenBB507
  %CurrSBIndex..16 = phi i64 [ %"loadedCurrSB+Stride513", %thenBB507 ], [ 0, %SyncBB390 ]
  %CurrWI..16 = phi i64 [ %"CurrWI++511", %thenBB507 ], [ 0, %SyncBB390 ]
  %"&(pSB[currWI].offset)21040" = or i64 %CurrSBIndex..16, 72
  %"&pSB[currWI].offset211" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)21040"
  %CastToValueType212 = bitcast i8* %"&pSB[currWI].offset211" to float addrspace(3)**
  %loadedValue213 = load float addrspace(3)** %CastToValueType212, align 8
  %104 = load float addrspace(3)* %loadedValue213, align 4
  %"&(pSB[currWI].offset)32341" = or i64 %CurrSBIndex..16, 108
  %"&pSB[currWI].offset324" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)32341"
  %CastToValueType325 = bitcast i8* %"&pSB[currWI].offset324" to float*
  %loadedValue326 = load float* %CastToValueType325, align 4
  %105 = fadd float %104, %loadedValue326
  %"&(pSB[currWI].offset)20542" = or i64 %CurrSBIndex..16, 72
  %"&pSB[currWI].offset206" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)20542"
  %CastToValueType207 = bitcast i8* %"&pSB[currWI].offset206" to float addrspace(3)**
  %loadedValue208 = load float addrspace(3)** %CastToValueType207, align 8
  store float %105, float addrspace(3)* %loadedValue208, align 4
  %check.WI.iter510 = icmp ult i64 %CurrWI..16, %iterCount
  br i1 %check.WI.iter510, label %thenBB507, label %SyncBB396

thenBB507:                                        ; preds = %SyncBB395
  %"CurrWI++511" = add nuw i64 %CurrWI..16, 1
  %"loadedCurrSB+Stride513" = add nuw i64 %CurrSBIndex..16, 1792
  br label %SyncBB395

SyncBB396:                                        ; preds = %SyncBB395, %thenBB514
  %CurrSBIndex..17 = phi i64 [ %"loadedCurrSB+Stride520", %thenBB514 ], [ 0, %SyncBB395 ]
  %CurrWI..17 = phi i64 [ %"CurrWI++518", %thenBB514 ], [ 0, %SyncBB395 ]
  %"&(pSB[currWI].offset)14643" = or i64 %CurrSBIndex..17, 64
  %"&pSB[currWI].offset147" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)14643"
  %CastToValueType148 = bitcast i8* %"&pSB[currWI].offset147" to i32*
  %loadedValue149 = load i32* %CastToValueType148, align 4
  %106 = add nsw i32 %loadedValue149, -64
  %107 = sext i32 %106 to i64
  %108 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %107
  %109 = load float addrspace(3)* %108, align 4
  %"&(pSB[currWI].offset)32844" = or i64 %CurrSBIndex..17, 112
  %"&pSB[currWI].offset329" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)32844"
  %CastToValueType330 = bitcast i8* %"&pSB[currWI].offset329" to float*
  store float %109, float* %CastToValueType330, align 4
  %check.WI.iter517 = icmp ult i64 %CurrWI..17, %iterCount
  br i1 %check.WI.iter517, label %thenBB514, label %SyncBB397

thenBB514:                                        ; preds = %SyncBB396
  %"CurrWI++518" = add nuw i64 %CurrWI..17, 1
  %"loadedCurrSB+Stride520" = add nuw i64 %CurrSBIndex..17, 1792
  br label %SyncBB396

SyncBB397:                                        ; preds = %SyncBB396, %thenBB521
  %CurrSBIndex..18 = phi i64 [ %"loadedCurrSB+Stride527", %thenBB521 ], [ 0, %SyncBB396 ]
  %CurrWI..18 = phi i64 [ %"CurrWI++525", %thenBB521 ], [ 0, %SyncBB396 ]
  %"&(pSB[currWI].offset)20045" = or i64 %CurrSBIndex..18, 72
  %"&pSB[currWI].offset201" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)20045"
  %CastToValueType202 = bitcast i8* %"&pSB[currWI].offset201" to float addrspace(3)**
  %loadedValue203 = load float addrspace(3)** %CastToValueType202, align 8
  %110 = load float addrspace(3)* %loadedValue203, align 4
  %"&(pSB[currWI].offset)33246" = or i64 %CurrSBIndex..18, 112
  %"&pSB[currWI].offset333" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)33246"
  %CastToValueType334 = bitcast i8* %"&pSB[currWI].offset333" to float*
  %loadedValue335 = load float* %CastToValueType334, align 4
  %111 = fadd float %110, %loadedValue335
  %"&(pSB[currWI].offset)19547" = or i64 %CurrSBIndex..18, 72
  %"&pSB[currWI].offset196" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)19547"
  %CastToValueType197 = bitcast i8* %"&pSB[currWI].offset196" to float addrspace(3)**
  %loadedValue198 = load float addrspace(3)** %CastToValueType197, align 8
  store float %111, float addrspace(3)* %loadedValue198, align 4
  %check.WI.iter524 = icmp ult i64 %CurrWI..18, %iterCount
  br i1 %check.WI.iter524, label %thenBB521, label %SyncBB398

thenBB521:                                        ; preds = %SyncBB397
  %"CurrWI++525" = add nuw i64 %CurrWI..18, 1
  %"loadedCurrSB+Stride527" = add nuw i64 %CurrSBIndex..18, 1792
  br label %SyncBB397

SyncBB398:                                        ; preds = %SyncBB397, %thenBB528
  %CurrSBIndex..19 = phi i64 [ %"loadedCurrSB+Stride534", %thenBB528 ], [ 0, %SyncBB397 ]
  %CurrWI..19 = phi i64 [ %"CurrWI++532", %thenBB528 ], [ 0, %SyncBB397 ]
  %"&(pSB[currWI].offset)14148" = or i64 %CurrSBIndex..19, 64
  %"&pSB[currWI].offset142" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)14148"
  %CastToValueType143 = bitcast i8* %"&pSB[currWI].offset142" to i32*
  %loadedValue144 = load i32* %CastToValueType143, align 4
  %112 = add nsw i32 %loadedValue144, -128
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %113
  %115 = load float addrspace(3)* %114, align 4
  %"&(pSB[currWI].offset)33749" = or i64 %CurrSBIndex..19, 116
  %"&pSB[currWI].offset338" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)33749"
  %CastToValueType339 = bitcast i8* %"&pSB[currWI].offset338" to float*
  store float %115, float* %CastToValueType339, align 4
  %check.WI.iter531 = icmp ult i64 %CurrWI..19, %iterCount
  br i1 %check.WI.iter531, label %thenBB528, label %SyncBB399

thenBB528:                                        ; preds = %SyncBB398
  %"CurrWI++532" = add nuw i64 %CurrWI..19, 1
  %"loadedCurrSB+Stride534" = add nuw i64 %CurrSBIndex..19, 1792
  br label %SyncBB398

SyncBB399:                                        ; preds = %SyncBB398, %thenBB535
  %CurrSBIndex..20 = phi i64 [ %"loadedCurrSB+Stride541", %thenBB535 ], [ 0, %SyncBB398 ]
  %CurrWI..20 = phi i64 [ %"CurrWI++539", %thenBB535 ], [ 0, %SyncBB398 ]
  %"&(pSB[currWI].offset)19050" = or i64 %CurrSBIndex..20, 72
  %"&pSB[currWI].offset191" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)19050"
  %CastToValueType192 = bitcast i8* %"&pSB[currWI].offset191" to float addrspace(3)**
  %loadedValue193 = load float addrspace(3)** %CastToValueType192, align 8
  %116 = load float addrspace(3)* %loadedValue193, align 4
  %"&(pSB[currWI].offset)34151" = or i64 %CurrSBIndex..20, 116
  %"&pSB[currWI].offset342" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)34151"
  %CastToValueType343 = bitcast i8* %"&pSB[currWI].offset342" to float*
  %loadedValue344 = load float* %CastToValueType343, align 4
  %117 = fadd float %116, %loadedValue344
  %"&(pSB[currWI].offset)18552" = or i64 %CurrSBIndex..20, 72
  %"&pSB[currWI].offset186" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)18552"
  %CastToValueType187 = bitcast i8* %"&pSB[currWI].offset186" to float addrspace(3)**
  %loadedValue188 = load float addrspace(3)** %CastToValueType187, align 8
  store float %117, float addrspace(3)* %loadedValue188, align 4
  %check.WI.iter538 = icmp ult i64 %CurrWI..20, %iterCount
  br i1 %check.WI.iter538, label %thenBB535, label %SyncBB400

thenBB535:                                        ; preds = %SyncBB399
  %"CurrWI++539" = add nuw i64 %CurrWI..20, 1
  %"loadedCurrSB+Stride541" = add nuw i64 %CurrSBIndex..20, 1792
  br label %SyncBB399

SyncBB400:                                        ; preds = %SyncBB399, %thenBB409
  %CurrSBIndex..2 = phi i64 [ %"loadedCurrSB+Stride415", %thenBB409 ], [ 0, %SyncBB399 ]
  %CurrWI..2 = phi i64 [ %"CurrWI++413", %thenBB409 ], [ 0, %SyncBB399 ]
  %"&(pSB[currWI].offset)26953" = or i64 %CurrSBIndex..2, 80
  %"&pSB[currWI].offset270" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)26953"
  %CastToValueType271 = bitcast i8* %"&pSB[currWI].offset270" to float addrspace(3)**
  %loadedValue272 = load float addrspace(3)** %CastToValueType271, align 8
  %118 = load float addrspace(3)* %loadedValue272, align 4
  %"&(pSB[currWI].offset)34654" = or i64 %CurrSBIndex..2, 120
  %"&pSB[currWI].offset347" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)34654"
  %CastToValueType348 = bitcast i8* %"&pSB[currWI].offset347" to float*
  store float %118, float* %CastToValueType348, align 4
  %check.WI.iter412 = icmp ult i64 %CurrWI..2, %iterCount
  br i1 %check.WI.iter412, label %thenBB409, label %elseBB410

thenBB409:                                        ; preds = %SyncBB400
  %"CurrWI++413" = add nuw i64 %CurrWI..2, 1
  %"loadedCurrSB+Stride415" = add nuw i64 %CurrSBIndex..2, 1792
  br label %SyncBB400

elseBB410:                                        ; preds = %SyncBB400
  %119 = icmp eq i32 %storeSum, 0
  %120 = bitcast float addrspace(1)* %g_odata to <4 x float> addrspace(1)*
  br label %SyncBB381

SyncBB381:                                        ; preds = %thenBB, %elseBB410
  %CurrSBIndex..0 = phi i64 [ 0, %elseBB410 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %CurrWI..0 = phi i64 [ 0, %elseBB410 ], [ %"CurrWI++", %thenBB ]
  br i1 %119, label %phi-split-bb, label %121

; <label>:121                                     ; preds = %SyncBB381
  %"&(pSB[currWI].offset)3655" = or i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset37" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3655"
  %CastToValueType38 = bitcast i8* %"&pSB[currWI].offset37" to i64*
  %loadedValue39 = load i64* %CastToValueType38, align 8
  %sext = shl i64 %loadedValue39, 32
  %122 = ashr i64 %sext, 32
  %123 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %124 = load i64* %123, align 8
  %125 = add i64 %124, -1
  %126 = icmp eq i64 %122, %125
  br i1 %126, label %127, label %phi-split-bb

; <label>:127                                     ; preds = %121
  %"&(pSB[currWI].offset)11470" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset115" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)11470"
  %CastToValueType116 = bitcast i8* %"&pSB[currWI].offset115" to <4 x float>*
  %loadedValue117 = load <4 x float>* %CastToValueType116, align 16
  %128 = extractelement <4 x float> %loadedValue117, i32 3
  %"&(pSB[currWI].offset)37571" = or i64 %CurrSBIndex..0, 120
  %"&pSB[currWI].offset376" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)37571"
  %CastToValueType377 = bitcast i8* %"&pSB[currWI].offset376" to float*
  %loadedValue378 = load float* %CastToValueType377, align 4
  %129 = fadd float %loadedValue378, %128
  %130 = load i64* %pWGId, align 8
  %131 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %130
  store float %129, float addrspace(1)* %131, align 4
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %SyncBB381, %127, %121
  %"&(pSB[currWI].offset)37056" = or i64 %CurrSBIndex..0, 120
  %"&pSB[currWI].offset371" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)37056"
  %CastToValueType372 = bitcast i8* %"&pSB[currWI].offset371" to float*
  %loadedValue373 = load float* %CastToValueType372, align 4
  %132 = insertelement <4 x float> undef, float %loadedValue373, i32 0
  %"&(pSB[currWI].offset)10957" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset110" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10957"
  %CastToValueType111 = bitcast i8* %"&pSB[currWI].offset110" to <4 x float>*
  %loadedValue112 = load <4 x float>* %CastToValueType111, align 16
  %133 = extractelement <4 x float> %loadedValue112, i32 0
  %"&(pSB[currWI].offset)36558" = or i64 %CurrSBIndex..0, 120
  %"&pSB[currWI].offset366" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)36558"
  %CastToValueType367 = bitcast i8* %"&pSB[currWI].offset366" to float*
  %loadedValue368 = load float* %CastToValueType367, align 4
  %134 = fadd float %loadedValue368, %133
  %135 = insertelement <4 x float> %132, float %134, i32 1
  %"&(pSB[currWI].offset)10459" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset105" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)10459"
  %CastToValueType106 = bitcast i8* %"&pSB[currWI].offset105" to <4 x float>*
  %loadedValue107 = load <4 x float>* %CastToValueType106, align 16
  %136 = extractelement <4 x float> %loadedValue107, i32 1
  %"&(pSB[currWI].offset)36060" = or i64 %CurrSBIndex..0, 120
  %"&pSB[currWI].offset361" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)36060"
  %CastToValueType362 = bitcast i8* %"&pSB[currWI].offset361" to float*
  %loadedValue363 = load float* %CastToValueType362, align 4
  %137 = fadd float %loadedValue363, %136
  %138 = insertelement <4 x float> %135, float %137, i32 2
  %"&(pSB[currWI].offset)9961" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset100" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)9961"
  %CastToValueType101 = bitcast i8* %"&pSB[currWI].offset100" to <4 x float>*
  %loadedValue102 = load <4 x float>* %CastToValueType101, align 16
  %139 = extractelement <4 x float> %loadedValue102, i32 2
  %"&(pSB[currWI].offset)35562" = or i64 %CurrSBIndex..0, 120
  %"&pSB[currWI].offset356" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)35562"
  %CastToValueType357 = bitcast i8* %"&pSB[currWI].offset356" to float*
  %loadedValue358 = load float* %CastToValueType357, align 4
  %140 = fadd float %loadedValue358, %139
  %141 = insertelement <4 x float> %138, float %140, i32 3
  br i1 %2, label %142, label %145

; <label>:142                                     ; preds = %phi-split-bb
  %"&(pSB[currWI].offset)5564" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset56" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5564"
  %CastToValueType57 = bitcast i8* %"&pSB[currWI].offset56" to i32*
  %loadedValue58 = load i32* %CastToValueType57, align 4
  %143 = or i32 %loadedValue58, 3
  %144 = icmp slt i32 %143, %n
  br i1 %144, label %145, label %148

; <label>:145                                     ; preds = %142, %phi-split-bb
  %"&(pSB[currWI].offset)2763" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset28" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2763"
  %CastToValueType29 = bitcast i8* %"&pSB[currWI].offset28" to i32*
  %loadedValue30 = load i32* %CastToValueType29, align 4
  %146 = sext i32 %loadedValue30 to i64
  %147 = getelementptr inbounds <4 x float> addrspace(1)* %120, i64 %146
  store <4 x float> %141, <4 x float> addrspace(1)* %147, align 16
  br label %UnifiedReturnBlock

; <label>:148                                     ; preds = %142
  %"&(pSB[currWI].offset)8065" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset81" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)8065"
  %CastToValueType82 = bitcast i8* %"&pSB[currWI].offset81" to i32*
  %loadedValue83 = load i32* %CastToValueType82, align 4
  %149 = icmp slt i32 %loadedValue83, %n
  br i1 %149, label %150, label %UnifiedReturnBlock

; <label>:150                                     ; preds = %148
  %"&(pSB[currWI].offset)7566" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset76" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)7566"
  %CastToValueType77 = bitcast i8* %"&pSB[currWI].offset76" to i32*
  %loadedValue78 = load i32* %CastToValueType77, align 4
  %151 = sext i32 %loadedValue78 to i64
  %152 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %151
  %"&(pSB[currWI].offset)35067" = or i64 %CurrSBIndex..0, 120
  %"&pSB[currWI].offset351" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)35067"
  %CastToValueType352 = bitcast i8* %"&pSB[currWI].offset351" to float*
  %loadedValue353 = load float* %CastToValueType352, align 4
  store float %loadedValue353, float addrspace(1)* %152, align 4
  %"&(pSB[currWI].offset)5068" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset51" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5068"
  %CastToValueType52 = bitcast i8* %"&pSB[currWI].offset51" to i32*
  %loadedValue53 = load i32* %CastToValueType52, align 4
  %153 = or i32 %loadedValue53, 1
  %154 = icmp slt i32 %153, %n
  br i1 %154, label %155, label %UnifiedReturnBlock

; <label>:155                                     ; preds = %150
  %156 = sext i32 %153 to i64
  %157 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %156
  store float %134, float addrspace(1)* %157, align 4
  %"&(pSB[currWI].offset)4569" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset46" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)4569"
  %CastToValueType47 = bitcast i8* %"&pSB[currWI].offset46" to i32*
  %loadedValue48 = load i32* %CastToValueType47, align 4
  %158 = or i32 %loadedValue48, 2
  %159 = icmp slt i32 %158, %n
  br i1 %159, label %160, label %UnifiedReturnBlock

; <label>:160                                     ; preds = %155
  %161 = sext i32 %158 to i64
  %162 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %161
  store float %137, float addrspace(1)* %162, align 4
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %160, %148, %155, %150, %145
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %UnifiedReturnBlock
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 1792
  br label %SyncBB381

SyncBB:                                           ; preds = %UnifiedReturnBlock
  ret void
}

define void @____Vectorized_.addUniform_separated_args(float addrspace(1)* nocapture %d_vector, float addrspace(1)* nocapture %d_uniforms, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to float addrspace(3)*
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB535
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride541", %thenBB535 ], [ 0, %0 ]
  %CurrWI..1 = phi i64 [ %"CurrWI++539", %thenBB535 ], [ 0, %0 ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %3 = load i64* %2, align 8
  %broadcast1 = insertelement <16 x i64> undef, i64 %3, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %4 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %5 = icmp eq <16 x i64> %4, zeroinitializer
  %extract = extractelement <16 x i1> %5, i32 0
  %extract42 = extractelement <16 x i1> %5, i32 1
  %extract43 = extractelement <16 x i1> %5, i32 2
  %extract44 = extractelement <16 x i1> %5, i32 3
  %extract45 = extractelement <16 x i1> %5, i32 4
  %extract46 = extractelement <16 x i1> %5, i32 5
  %extract47 = extractelement <16 x i1> %5, i32 6
  %extract48 = extractelement <16 x i1> %5, i32 7
  %extract49 = extractelement <16 x i1> %5, i32 8
  %extract50 = extractelement <16 x i1> %5, i32 9
  %extract51 = extractelement <16 x i1> %5, i32 10
  %extract52 = extractelement <16 x i1> %5, i32 11
  %extract53 = extractelement <16 x i1> %5, i32 12
  %extract54 = extractelement <16 x i1> %5, i32 13
  %extract55 = extractelement <16 x i1> %5, i32 14
  %extract56 = extractelement <16 x i1> %5, i32 15
  br i1 %extract, label %preload334, label %postload335

preload334:                                       ; preds = %SyncBB
  %6 = load i64* %pWGId, align 8
  br label %postload335

postload335:                                      ; preds = %preload334, %SyncBB
  %phi336 = phi i64 [ undef, %SyncBB ], [ %6, %preload334 ]
  br i1 %extract42, label %preload326, label %postload327

preload326:                                       ; preds = %postload335
  %7 = load i64* %pWGId, align 8
  br label %postload327

postload327:                                      ; preds = %preload326, %postload335
  %phi328 = phi i64 [ undef, %postload335 ], [ %7, %preload326 ]
  br i1 %extract43, label %preload, label %postload

preload:                                          ; preds = %postload327
  %8 = load i64* %pWGId, align 8
  br label %postload

postload:                                         ; preds = %preload, %postload327
  %phi = phi i64 [ undef, %postload327 ], [ %8, %preload ]
  br i1 %extract44, label %preload342, label %postload343

preload342:                                       ; preds = %postload
  %9 = load i64* %pWGId, align 8
  br label %postload343

postload343:                                      ; preds = %preload342, %postload
  %phi344 = phi i64 [ undef, %postload ], [ %9, %preload342 ]
  br i1 %extract45, label %preload310, label %postload311

preload310:                                       ; preds = %postload343
  %10 = load i64* %pWGId, align 8
  br label %postload311

postload311:                                      ; preds = %preload310, %postload343
  %phi312 = phi i64 [ undef, %postload343 ], [ %10, %preload310 ]
  br i1 %extract46, label %preload318, label %postload319

preload318:                                       ; preds = %postload311
  %11 = load i64* %pWGId, align 8
  br label %postload319

postload319:                                      ; preds = %preload318, %postload311
  %phi320 = phi i64 [ undef, %postload311 ], [ %11, %preload318 ]
  br i1 %extract47, label %preload230, label %postload231

preload230:                                       ; preds = %postload319
  %12 = load i64* %pWGId, align 8
  br label %postload231

postload231:                                      ; preds = %preload230, %postload319
  %phi232 = phi i64 [ undef, %postload319 ], [ %12, %preload230 ]
  br i1 %extract48, label %preload238, label %postload239

preload238:                                       ; preds = %postload231
  %13 = load i64* %pWGId, align 8
  br label %postload239

postload239:                                      ; preds = %preload238, %postload231
  %phi240 = phi i64 [ undef, %postload231 ], [ %13, %preload238 ]
  br i1 %extract49, label %preload246, label %postload247

preload246:                                       ; preds = %postload239
  %14 = load i64* %pWGId, align 8
  br label %postload247

postload247:                                      ; preds = %preload246, %postload239
  %phi248 = phi i64 [ undef, %postload239 ], [ %14, %preload246 ]
  br i1 %extract50, label %preload254, label %postload255

preload254:                                       ; preds = %postload247
  %15 = load i64* %pWGId, align 8
  br label %postload255

postload255:                                      ; preds = %preload254, %postload247
  %phi256 = phi i64 [ undef, %postload247 ], [ %15, %preload254 ]
  br i1 %extract51, label %preload262, label %postload263

preload262:                                       ; preds = %postload255
  %16 = load i64* %pWGId, align 8
  br label %postload263

postload263:                                      ; preds = %preload262, %postload255
  %phi264 = phi i64 [ undef, %postload255 ], [ %16, %preload262 ]
  br i1 %extract52, label %preload270, label %postload271

preload270:                                       ; preds = %postload263
  %17 = load i64* %pWGId, align 8
  br label %postload271

postload271:                                      ; preds = %preload270, %postload263
  %phi272 = phi i64 [ undef, %postload263 ], [ %17, %preload270 ]
  br i1 %extract53, label %preload278, label %postload279

preload278:                                       ; preds = %postload271
  %18 = load i64* %pWGId, align 8
  br label %postload279

postload279:                                      ; preds = %preload278, %postload271
  %phi280 = phi i64 [ undef, %postload271 ], [ %18, %preload278 ]
  br i1 %extract54, label %preload286, label %postload287

preload286:                                       ; preds = %postload279
  %19 = load i64* %pWGId, align 8
  br label %postload287

postload287:                                      ; preds = %preload286, %postload279
  %phi288 = phi i64 [ undef, %postload279 ], [ %19, %preload286 ]
  br i1 %extract55, label %preload294, label %postload295

preload294:                                       ; preds = %postload287
  %20 = load i64* %pWGId, align 8
  br label %postload295

postload295:                                      ; preds = %preload294, %postload287
  %phi296 = phi i64 [ undef, %postload287 ], [ %20, %preload294 ]
  br i1 %extract56, label %preload302, label %postload303

preload302:                                       ; preds = %postload295
  %21 = load i64* %pWGId, align 8
  br label %postload303

postload303:                                      ; preds = %preload302, %postload295
  %phi304 = phi i64 [ undef, %postload295 ], [ %21, %preload302 ]
  %22 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi328
  %23 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi
  %24 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi344
  %25 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi312
  %26 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi320
  %27 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi232
  %28 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi240
  %29 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi248
  %30 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi256
  %31 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi264
  %32 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi272
  %33 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi280
  %34 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi288
  %35 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi296
  %36 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi304
  br i1 %extract, label %preload337, label %postload338

preload337:                                       ; preds = %postload303
  %37 = getelementptr inbounds float addrspace(1)* %d_uniforms, i64 %phi336
  %masked_load = load float addrspace(1)* %37, align 4
  br label %postload338

postload338:                                      ; preds = %preload337, %postload303
  %phi339 = phi float [ undef, %postload303 ], [ %masked_load, %preload337 ]
  br i1 %extract42, label %preload329, label %postload330

preload329:                                       ; preds = %postload338
  %masked_load178 = load float addrspace(1)* %22, align 4
  br label %postload330

postload330:                                      ; preds = %preload329, %postload338
  %phi331 = phi float [ undef, %postload338 ], [ %masked_load178, %preload329 ]
  br i1 %extract43, label %preload225, label %postload226

preload225:                                       ; preds = %postload330
  %masked_load179 = load float addrspace(1)* %23, align 4
  br label %postload226

postload226:                                      ; preds = %preload225, %postload330
  %phi227 = phi float [ undef, %postload330 ], [ %masked_load179, %preload225 ]
  br i1 %extract44, label %preload345, label %postload346

preload345:                                       ; preds = %postload226
  %masked_load180 = load float addrspace(1)* %24, align 4
  br label %postload346

postload346:                                      ; preds = %preload345, %postload226
  %phi347 = phi float [ undef, %postload226 ], [ %masked_load180, %preload345 ]
  br i1 %extract45, label %preload313, label %postload314

preload313:                                       ; preds = %postload346
  %masked_load181 = load float addrspace(1)* %25, align 4
  br label %postload314

postload314:                                      ; preds = %preload313, %postload346
  %phi315 = phi float [ undef, %postload346 ], [ %masked_load181, %preload313 ]
  br i1 %extract46, label %preload321, label %postload322

preload321:                                       ; preds = %postload314
  %masked_load182 = load float addrspace(1)* %26, align 4
  br label %postload322

postload322:                                      ; preds = %preload321, %postload314
  %phi323 = phi float [ undef, %postload314 ], [ %masked_load182, %preload321 ]
  br i1 %extract47, label %preload233, label %postload234

preload233:                                       ; preds = %postload322
  %masked_load183 = load float addrspace(1)* %27, align 4
  br label %postload234

postload234:                                      ; preds = %preload233, %postload322
  %phi235 = phi float [ undef, %postload322 ], [ %masked_load183, %preload233 ]
  br i1 %extract48, label %preload241, label %postload242

preload241:                                       ; preds = %postload234
  %masked_load184 = load float addrspace(1)* %28, align 4
  br label %postload242

postload242:                                      ; preds = %preload241, %postload234
  %phi243 = phi float [ undef, %postload234 ], [ %masked_load184, %preload241 ]
  br i1 %extract49, label %preload249, label %postload250

preload249:                                       ; preds = %postload242
  %masked_load185 = load float addrspace(1)* %29, align 4
  br label %postload250

postload250:                                      ; preds = %preload249, %postload242
  %phi251 = phi float [ undef, %postload242 ], [ %masked_load185, %preload249 ]
  br i1 %extract50, label %preload257, label %postload258

preload257:                                       ; preds = %postload250
  %masked_load186 = load float addrspace(1)* %30, align 4
  br label %postload258

postload258:                                      ; preds = %preload257, %postload250
  %phi259 = phi float [ undef, %postload250 ], [ %masked_load186, %preload257 ]
  br i1 %extract51, label %preload265, label %postload266

preload265:                                       ; preds = %postload258
  %masked_load187 = load float addrspace(1)* %31, align 4
  br label %postload266

postload266:                                      ; preds = %preload265, %postload258
  %phi267 = phi float [ undef, %postload258 ], [ %masked_load187, %preload265 ]
  br i1 %extract52, label %preload273, label %postload274

preload273:                                       ; preds = %postload266
  %masked_load188 = load float addrspace(1)* %32, align 4
  br label %postload274

postload274:                                      ; preds = %preload273, %postload266
  %phi275 = phi float [ undef, %postload266 ], [ %masked_load188, %preload273 ]
  br i1 %extract53, label %preload281, label %postload282

preload281:                                       ; preds = %postload274
  %masked_load189 = load float addrspace(1)* %33, align 4
  br label %postload282

postload282:                                      ; preds = %preload281, %postload274
  %phi283 = phi float [ undef, %postload274 ], [ %masked_load189, %preload281 ]
  br i1 %extract54, label %preload289, label %postload290

preload289:                                       ; preds = %postload282
  %masked_load190 = load float addrspace(1)* %34, align 4
  br label %postload290

postload290:                                      ; preds = %preload289, %postload282
  %phi291 = phi float [ undef, %postload282 ], [ %masked_load190, %preload289 ]
  br i1 %extract55, label %preload297, label %postload298

preload297:                                       ; preds = %postload290
  %masked_load191 = load float addrspace(1)* %35, align 4
  br label %postload298

postload298:                                      ; preds = %preload297, %postload290
  %phi299 = phi float [ undef, %postload290 ], [ %masked_load191, %preload297 ]
  br i1 %extract56, label %preload305, label %postload306

preload305:                                       ; preds = %postload298
  %masked_load192 = load float addrspace(1)* %36, align 4
  br label %postload306

postload306:                                      ; preds = %preload305, %postload298
  %phi307 = phi float [ undef, %postload298 ], [ %masked_load192, %preload305 ]
  br i1 %extract, label %preload340, label %postload341

preload340:                                       ; preds = %postload306
  store float %phi339, float addrspace(3)* %1, align 4
  br label %postload341

postload341:                                      ; preds = %preload340, %postload306
  br i1 %extract42, label %preload332, label %postload333

preload332:                                       ; preds = %postload341
  store float %phi331, float addrspace(3)* %1, align 4
  br label %postload333

postload333:                                      ; preds = %preload332, %postload341
  br i1 %extract43, label %preload228, label %postload229

preload228:                                       ; preds = %postload333
  store float %phi227, float addrspace(3)* %1, align 4
  br label %postload229

postload229:                                      ; preds = %preload228, %postload333
  br i1 %extract44, label %preload348, label %postload349

preload348:                                       ; preds = %postload229
  store float %phi347, float addrspace(3)* %1, align 4
  br label %postload349

postload349:                                      ; preds = %preload348, %postload229
  br i1 %extract45, label %preload316, label %postload317

preload316:                                       ; preds = %postload349
  store float %phi315, float addrspace(3)* %1, align 4
  br label %postload317

postload317:                                      ; preds = %preload316, %postload349
  br i1 %extract46, label %preload324, label %postload325

preload324:                                       ; preds = %postload317
  store float %phi323, float addrspace(3)* %1, align 4
  br label %postload325

postload325:                                      ; preds = %preload324, %postload317
  br i1 %extract47, label %preload236, label %postload237

preload236:                                       ; preds = %postload325
  store float %phi235, float addrspace(3)* %1, align 4
  br label %postload237

postload237:                                      ; preds = %preload236, %postload325
  br i1 %extract48, label %preload244, label %postload245

preload244:                                       ; preds = %postload237
  store float %phi243, float addrspace(3)* %1, align 4
  br label %postload245

postload245:                                      ; preds = %preload244, %postload237
  br i1 %extract49, label %preload252, label %postload253

preload252:                                       ; preds = %postload245
  store float %phi251, float addrspace(3)* %1, align 4
  br label %postload253

postload253:                                      ; preds = %preload252, %postload245
  br i1 %extract50, label %preload260, label %postload261

preload260:                                       ; preds = %postload253
  store float %phi259, float addrspace(3)* %1, align 4
  br label %postload261

postload261:                                      ; preds = %preload260, %postload253
  br i1 %extract51, label %preload268, label %postload269

preload268:                                       ; preds = %postload261
  store float %phi267, float addrspace(3)* %1, align 4
  br label %postload269

postload269:                                      ; preds = %preload268, %postload261
  br i1 %extract52, label %preload276, label %postload277

preload276:                                       ; preds = %postload269
  store float %phi275, float addrspace(3)* %1, align 4
  br label %postload277

postload277:                                      ; preds = %preload276, %postload269
  br i1 %extract53, label %preload284, label %postload285

preload284:                                       ; preds = %postload277
  store float %phi283, float addrspace(3)* %1, align 4
  br label %postload285

postload285:                                      ; preds = %preload284, %postload277
  br i1 %extract54, label %preload292, label %postload293

preload292:                                       ; preds = %postload285
  store float %phi291, float addrspace(3)* %1, align 4
  br label %postload293

postload293:                                      ; preds = %preload292, %postload285
  br i1 %extract55, label %preload300, label %postload301

preload300:                                       ; preds = %postload293
  store float %phi299, float addrspace(3)* %1, align 4
  br label %postload301

postload301:                                      ; preds = %preload300, %postload293
  br i1 %extract56, label %preload308, label %postload309

preload308:                                       ; preds = %postload301
  store float %phi307, float addrspace(3)* %1, align 4
  br label %postload309

postload309:                                      ; preds = %preload308, %postload301
  %38 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %39 = load i64* %38, align 8
  %broadcast157 = insertelement <16 x i64> undef, i64 %39, i32 0
  %broadcast258 = shufflevector <16 x i64> %broadcast157, <16 x i64> undef, <16 x i32> zeroinitializer
  %40 = add <16 x i64> %broadcast258, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %41 = load i64* %pWGId, align 8
  %42 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %43 = load i64* %42, align 8
  %44 = shl i64 %41, 2
  %45 = mul i64 %44, %43
  %temp = insertelement <16 x i64> undef, i64 %45, i32 0
  %vector = shufflevector <16 x i64> %temp, <16 x i64> undef, <16 x i32> zeroinitializer
  %46 = add <16 x i64> %vector, %40
  %"&(pSB[currWI].offset)9" = or i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)9"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to <16 x i64>*
  store <16 x i64> %46, <16 x i64>* %CastToValueType, align 128
  %check.WI.iter538 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter538, label %thenBB535, label %elseBB536

thenBB535:                                        ; preds = %postload309
  %"CurrWI++539" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride541" = add nuw i64 %CurrSBIndex..1, 1792
  br label %SyncBB

elseBB536:                                        ; preds = %postload309
  %temp75 = insertelement <16 x i32> undef, i32 %n, i32 0
  %vector76 = shufflevector <16 x i32> %temp75, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB533

SyncBB533:                                        ; preds = %thenBB, %elseBB536
  %CurrSBIndex..0 = phi i64 [ 0, %elseBB536 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %CurrWI..0 = phi i64 [ 0, %elseBB536 ], [ %"CurrWI++", %thenBB ]
  %"&(pSB[currWI].offset)52710" = or i64 %CurrSBIndex..0, 128
  %"&pSB[currWI].offset528" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)52710"
  %CastToValueType529 = bitcast i8* %"&pSB[currWI].offset528" to <16 x i64>*
  %loadedValue = load <16 x i64>* %CastToValueType529, align 128
  br label %postload351

postload351:                                      ; preds = %postload524, %SyncBB533
  %vectorPHI59 = phi <16 x i1> [ zeroinitializer, %SyncBB533 ], [ %loop_mask2180, %postload524 ]
  %vectorPHI63 = phi <16 x i1> [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %SyncBB533 ], [ %local_edge2782, %postload524 ]
  %vectorPHI64 = phi <16 x i64> [ %loadedValue, %SyncBB533 ], [ %100, %postload524 ]
  %47 = phi i32 [ 0, %SyncBB533 ], [ %101, %postload524 ]
  %48 = icmp slt i32 %47, 4
  %temp71 = insertelement <16 x i1> undef, i1 %48, i32 0
  %vector72 = shufflevector <16 x i1> %temp71, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond = xor i1 %48, true
  %temp65 = insertelement <16 x i1> undef, i1 %notCond, i32 0
  %vector66 = shufflevector <16 x i1> %temp65, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr67 = and <16 x i1> %vectorPHI63, %vector66
  %loop_mask1369 = or <16 x i1> %vectorPHI59, %who_left_tr67
  %local_edge73 = and <16 x i1> %vectorPHI63, %vector72
  %address.074 = trunc <16 x i64> %vectorPHI64 to <16 x i32>
  %49 = icmp ult <16 x i32> %address.074, %vector76
  %notCond1777 = xor <16 x i1> %49, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr1878 = and <16 x i1> %local_edge73, %notCond1777
  %loop_mask2180 = or <16 x i1> %loop_mask1369, %who_left_tr1878
  %local_edge2782 = and <16 x i1> %local_edge73, %49
  %extract83 = extractelement <16 x i1> %local_edge2782, i32 0
  %extract84 = extractelement <16 x i1> %local_edge2782, i32 1
  %extract85 = extractelement <16 x i1> %local_edge2782, i32 2
  %extract86 = extractelement <16 x i1> %local_edge2782, i32 3
  %extract87 = extractelement <16 x i1> %local_edge2782, i32 4
  %extract88 = extractelement <16 x i1> %local_edge2782, i32 5
  %extract89 = extractelement <16 x i1> %local_edge2782, i32 6
  %extract90 = extractelement <16 x i1> %local_edge2782, i32 7
  %extract91 = extractelement <16 x i1> %local_edge2782, i32 8
  %extract92 = extractelement <16 x i1> %local_edge2782, i32 9
  %extract93 = extractelement <16 x i1> %local_edge2782, i32 10
  %extract94 = extractelement <16 x i1> %local_edge2782, i32 11
  %extract95 = extractelement <16 x i1> %local_edge2782, i32 12
  %extract96 = extractelement <16 x i1> %local_edge2782, i32 13
  %extract97 = extractelement <16 x i1> %local_edge2782, i32 14
  %extract98 = extractelement <16 x i1> %local_edge2782, i32 15
  %masked_load193 = load float addrspace(3)* %1, align 4
  %temp.vect130 = insertelement <16 x float> undef, float %masked_load193, i32 0
  %temp.vect131 = insertelement <16 x float> %temp.vect130, float %masked_load193, i32 1
  %temp.vect132 = insertelement <16 x float> %temp.vect131, float %masked_load193, i32 2
  %temp.vect133 = insertelement <16 x float> %temp.vect132, float %masked_load193, i32 3
  %temp.vect134 = insertelement <16 x float> %temp.vect133, float %masked_load193, i32 4
  %temp.vect135 = insertelement <16 x float> %temp.vect134, float %masked_load193, i32 5
  %temp.vect136 = insertelement <16 x float> %temp.vect135, float %masked_load193, i32 6
  %temp.vect137 = insertelement <16 x float> %temp.vect136, float %masked_load193, i32 7
  %temp.vect138 = insertelement <16 x float> %temp.vect137, float %masked_load193, i32 8
  %temp.vect139 = insertelement <16 x float> %temp.vect138, float %masked_load193, i32 9
  %temp.vect140 = insertelement <16 x float> %temp.vect139, float %masked_load193, i32 10
  %temp.vect141 = insertelement <16 x float> %temp.vect140, float %masked_load193, i32 11
  %temp.vect142 = insertelement <16 x float> %temp.vect141, float %masked_load193, i32 12
  %temp.vect143 = insertelement <16 x float> %temp.vect142, float %masked_load193, i32 13
  %temp.vect144 = insertelement <16 x float> %temp.vect143, float %masked_load193, i32 14
  %temp.vect145 = insertelement <16 x float> %temp.vect144, float %masked_load193, i32 15
  %50 = and <16 x i64> %vectorPHI64, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract99 = extractelement <16 x i64> %50, i32 0
  %extract100 = extractelement <16 x i64> %50, i32 1
  %extract101 = extractelement <16 x i64> %50, i32 2
  %extract102 = extractelement <16 x i64> %50, i32 3
  %extract103 = extractelement <16 x i64> %50, i32 4
  %extract104 = extractelement <16 x i64> %50, i32 5
  %extract105 = extractelement <16 x i64> %50, i32 6
  %extract106 = extractelement <16 x i64> %50, i32 7
  %extract107 = extractelement <16 x i64> %50, i32 8
  %extract108 = extractelement <16 x i64> %50, i32 9
  %extract109 = extractelement <16 x i64> %50, i32 10
  %extract110 = extractelement <16 x i64> %50, i32 11
  %extract111 = extractelement <16 x i64> %50, i32 12
  %extract112 = extractelement <16 x i64> %50, i32 13
  %extract113 = extractelement <16 x i64> %50, i32 14
  %extract114 = extractelement <16 x i64> %50, i32 15
  %51 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract99
  %52 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract100
  %53 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract101
  %54 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract102
  %55 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract103
  %56 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract104
  %57 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract105
  %58 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract106
  %59 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract107
  %60 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract108
  %61 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract109
  %62 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract110
  %63 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract111
  %64 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract112
  %65 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract113
  %66 = getelementptr inbounds float addrspace(1)* %d_vector, i64 %extract114
  br i1 %extract83, label %preload353, label %postload354

preload353:                                       ; preds = %postload351
  %masked_load209 = load float addrspace(1)* %51, align 4
  br label %postload354

postload354:                                      ; preds = %preload353, %postload351
  %phi355 = phi float [ undef, %postload351 ], [ %masked_load209, %preload353 ]
  br i1 %extract84, label %preload364, label %postload365

preload364:                                       ; preds = %postload354
  %masked_load210 = load float addrspace(1)* %52, align 4
  br label %postload365

postload365:                                      ; preds = %preload364, %postload354
  %phi366 = phi float [ undef, %postload354 ], [ %masked_load210, %preload364 ]
  br i1 %extract85, label %preload375, label %postload376

preload375:                                       ; preds = %postload365
  %masked_load211 = load float addrspace(1)* %53, align 4
  br label %postload376

postload376:                                      ; preds = %preload375, %postload365
  %phi377 = phi float [ undef, %postload365 ], [ %masked_load211, %preload375 ]
  br i1 %extract86, label %preload386, label %postload387

preload386:                                       ; preds = %postload376
  %masked_load212 = load float addrspace(1)* %54, align 4
  br label %postload387

postload387:                                      ; preds = %preload386, %postload376
  %phi388 = phi float [ undef, %postload376 ], [ %masked_load212, %preload386 ]
  br i1 %extract87, label %preload397, label %postload398

preload397:                                       ; preds = %postload387
  %masked_load213 = load float addrspace(1)* %55, align 4
  br label %postload398

postload398:                                      ; preds = %preload397, %postload387
  %phi399 = phi float [ undef, %postload387 ], [ %masked_load213, %preload397 ]
  br i1 %extract88, label %preload408, label %postload409

preload408:                                       ; preds = %postload398
  %masked_load214 = load float addrspace(1)* %56, align 4
  br label %postload409

postload409:                                      ; preds = %preload408, %postload398
  %phi410 = phi float [ undef, %postload398 ], [ %masked_load214, %preload408 ]
  br i1 %extract89, label %preload419, label %postload420

preload419:                                       ; preds = %postload409
  %masked_load215 = load float addrspace(1)* %57, align 4
  br label %postload420

postload420:                                      ; preds = %preload419, %postload409
  %phi421 = phi float [ undef, %postload409 ], [ %masked_load215, %preload419 ]
  br i1 %extract90, label %preload430, label %postload431

preload430:                                       ; preds = %postload420
  %masked_load216 = load float addrspace(1)* %58, align 4
  br label %postload431

postload431:                                      ; preds = %preload430, %postload420
  %phi432 = phi float [ undef, %postload420 ], [ %masked_load216, %preload430 ]
  br i1 %extract91, label %preload441, label %postload442

preload441:                                       ; preds = %postload431
  %masked_load217 = load float addrspace(1)* %59, align 4
  br label %postload442

postload442:                                      ; preds = %preload441, %postload431
  %phi443 = phi float [ undef, %postload431 ], [ %masked_load217, %preload441 ]
  br i1 %extract92, label %preload452, label %postload453

preload452:                                       ; preds = %postload442
  %masked_load218 = load float addrspace(1)* %60, align 4
  br label %postload453

postload453:                                      ; preds = %preload452, %postload442
  %phi454 = phi float [ undef, %postload442 ], [ %masked_load218, %preload452 ]
  br i1 %extract93, label %preload463, label %postload464

preload463:                                       ; preds = %postload453
  %masked_load219 = load float addrspace(1)* %61, align 4
  br label %postload464

postload464:                                      ; preds = %preload463, %postload453
  %phi465 = phi float [ undef, %postload453 ], [ %masked_load219, %preload463 ]
  br i1 %extract94, label %preload474, label %postload475

preload474:                                       ; preds = %postload464
  %masked_load220 = load float addrspace(1)* %62, align 4
  br label %postload475

postload475:                                      ; preds = %preload474, %postload464
  %phi476 = phi float [ undef, %postload464 ], [ %masked_load220, %preload474 ]
  br i1 %extract95, label %preload485, label %postload486

preload485:                                       ; preds = %postload475
  %masked_load221 = load float addrspace(1)* %63, align 4
  br label %postload486

postload486:                                      ; preds = %preload485, %postload475
  %phi487 = phi float [ undef, %postload475 ], [ %masked_load221, %preload485 ]
  br i1 %extract96, label %preload496, label %postload497

preload496:                                       ; preds = %postload486
  %masked_load222 = load float addrspace(1)* %64, align 4
  br label %postload497

postload497:                                      ; preds = %preload496, %postload486
  %phi498 = phi float [ undef, %postload486 ], [ %masked_load222, %preload496 ]
  br i1 %extract97, label %preload507, label %postload508

preload507:                                       ; preds = %postload497
  %masked_load223 = load float addrspace(1)* %65, align 4
  br label %postload508

postload508:                                      ; preds = %preload507, %postload497
  %phi509 = phi float [ undef, %postload497 ], [ %masked_load223, %preload507 ]
  br i1 %extract98, label %preload518, label %postload519

preload518:                                       ; preds = %postload508
  %masked_load224 = load float addrspace(1)* %66, align 4
  br label %postload519

postload519:                                      ; preds = %preload518, %postload508
  %phi520 = phi float [ undef, %postload508 ], [ %masked_load224, %preload518 ]
  %temp.vect = insertelement <16 x float> undef, float %phi355, i32 0
  %temp.vect115 = insertelement <16 x float> %temp.vect, float %phi366, i32 1
  %temp.vect116 = insertelement <16 x float> %temp.vect115, float %phi377, i32 2
  %temp.vect117 = insertelement <16 x float> %temp.vect116, float %phi388, i32 3
  %temp.vect118 = insertelement <16 x float> %temp.vect117, float %phi399, i32 4
  %temp.vect119 = insertelement <16 x float> %temp.vect118, float %phi410, i32 5
  %temp.vect120 = insertelement <16 x float> %temp.vect119, float %phi421, i32 6
  %temp.vect121 = insertelement <16 x float> %temp.vect120, float %phi432, i32 7
  %temp.vect122 = insertelement <16 x float> %temp.vect121, float %phi443, i32 8
  %temp.vect123 = insertelement <16 x float> %temp.vect122, float %phi454, i32 9
  %temp.vect124 = insertelement <16 x float> %temp.vect123, float %phi465, i32 10
  %temp.vect125 = insertelement <16 x float> %temp.vect124, float %phi476, i32 11
  %temp.vect126 = insertelement <16 x float> %temp.vect125, float %phi487, i32 12
  %temp.vect127 = insertelement <16 x float> %temp.vect126, float %phi498, i32 13
  %temp.vect128 = insertelement <16 x float> %temp.vect127, float %phi509, i32 14
  %temp.vect129 = insertelement <16 x float> %temp.vect128, float %phi520, i32 15
  %67 = fadd <16 x float> %temp.vect129, %temp.vect145
  %extract147 = extractelement <16 x float> %67, i32 1
  %extract148 = extractelement <16 x float> %67, i32 2
  %extract149 = extractelement <16 x float> %67, i32 3
  %extract150 = extractelement <16 x float> %67, i32 4
  %extract151 = extractelement <16 x float> %67, i32 5
  %extract152 = extractelement <16 x float> %67, i32 6
  %extract153 = extractelement <16 x float> %67, i32 7
  %extract154 = extractelement <16 x float> %67, i32 8
  %extract155 = extractelement <16 x float> %67, i32 9
  %extract156 = extractelement <16 x float> %67, i32 10
  %extract157 = extractelement <16 x float> %67, i32 11
  %extract158 = extractelement <16 x float> %67, i32 12
  %extract159 = extractelement <16 x float> %67, i32 13
  %extract160 = extractelement <16 x float> %67, i32 14
  %extract161 = extractelement <16 x float> %67, i32 15
  br i1 %extract83, label %preload356, label %postload357

preload356:                                       ; preds = %postload519
  %extract146 = extractelement <16 x float> %67, i32 0
  store float %extract146, float addrspace(1)* %51, align 4
  br label %postload357

postload357:                                      ; preds = %preload356, %postload519
  br i1 %extract84, label %preload367, label %postload368

preload367:                                       ; preds = %postload357
  store float %extract147, float addrspace(1)* %52, align 4
  br label %postload368

postload368:                                      ; preds = %preload367, %postload357
  br i1 %extract85, label %preload378, label %postload379

preload378:                                       ; preds = %postload368
  store float %extract148, float addrspace(1)* %53, align 4
  br label %postload379

postload379:                                      ; preds = %preload378, %postload368
  br i1 %extract86, label %preload389, label %postload390

preload389:                                       ; preds = %postload379
  store float %extract149, float addrspace(1)* %54, align 4
  br label %postload390

postload390:                                      ; preds = %preload389, %postload379
  br i1 %extract87, label %preload400, label %postload401

preload400:                                       ; preds = %postload390
  store float %extract150, float addrspace(1)* %55, align 4
  br label %postload401

postload401:                                      ; preds = %preload400, %postload390
  br i1 %extract88, label %preload411, label %postload412

preload411:                                       ; preds = %postload401
  store float %extract151, float addrspace(1)* %56, align 4
  br label %postload412

postload412:                                      ; preds = %preload411, %postload401
  br i1 %extract89, label %preload422, label %postload423

preload422:                                       ; preds = %postload412
  store float %extract152, float addrspace(1)* %57, align 4
  br label %postload423

postload423:                                      ; preds = %preload422, %postload412
  br i1 %extract90, label %preload433, label %postload434

preload433:                                       ; preds = %postload423
  store float %extract153, float addrspace(1)* %58, align 4
  br label %postload434

postload434:                                      ; preds = %preload433, %postload423
  br i1 %extract91, label %preload444, label %postload445

preload444:                                       ; preds = %postload434
  store float %extract154, float addrspace(1)* %59, align 4
  br label %postload445

postload445:                                      ; preds = %preload444, %postload434
  br i1 %extract92, label %preload455, label %postload456

preload455:                                       ; preds = %postload445
  store float %extract155, float addrspace(1)* %60, align 4
  br label %postload456

postload456:                                      ; preds = %preload455, %postload445
  br i1 %extract93, label %preload466, label %postload467

preload466:                                       ; preds = %postload456
  store float %extract156, float addrspace(1)* %61, align 4
  br label %postload467

postload467:                                      ; preds = %preload466, %postload456
  br i1 %extract94, label %preload477, label %postload478

preload477:                                       ; preds = %postload467
  store float %extract157, float addrspace(1)* %62, align 4
  br label %postload478

postload478:                                      ; preds = %preload477, %postload467
  br i1 %extract95, label %preload488, label %postload489

preload488:                                       ; preds = %postload478
  store float %extract158, float addrspace(1)* %63, align 4
  br label %postload489

postload489:                                      ; preds = %preload488, %postload478
  br i1 %extract96, label %preload499, label %postload500

preload499:                                       ; preds = %postload489
  store float %extract159, float addrspace(1)* %64, align 4
  br label %postload500

postload500:                                      ; preds = %preload499, %postload489
  br i1 %extract97, label %preload510, label %postload511

preload510:                                       ; preds = %postload500
  store float %extract160, float addrspace(1)* %65, align 4
  br label %postload511

postload511:                                      ; preds = %preload510, %postload500
  br i1 %extract98, label %preload521, label %postload522

preload521:                                       ; preds = %postload511
  store float %extract161, float addrspace(1)* %66, align 4
  br label %postload522

postload522:                                      ; preds = %preload521, %postload511
  br i1 %extract83, label %preload358, label %postload359

preload358:                                       ; preds = %postload522
  %68 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %69 = load i64* %68, align 8
  br label %postload359

postload359:                                      ; preds = %preload358, %postload522
  %phi360 = phi i64 [ undef, %postload522 ], [ %69, %preload358 ]
  br i1 %extract84, label %preload369, label %postload370

preload369:                                       ; preds = %postload359
  %70 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %71 = load i64* %70, align 8
  br label %postload370

postload370:                                      ; preds = %preload369, %postload359
  %phi371 = phi i64 [ undef, %postload359 ], [ %71, %preload369 ]
  br i1 %extract85, label %preload380, label %postload381

preload380:                                       ; preds = %postload370
  %72 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %73 = load i64* %72, align 8
  br label %postload381

postload381:                                      ; preds = %preload380, %postload370
  %phi382 = phi i64 [ undef, %postload370 ], [ %73, %preload380 ]
  br i1 %extract86, label %preload391, label %postload392

preload391:                                       ; preds = %postload381
  %74 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %75 = load i64* %74, align 8
  br label %postload392

postload392:                                      ; preds = %preload391, %postload381
  %phi393 = phi i64 [ undef, %postload381 ], [ %75, %preload391 ]
  br i1 %extract87, label %preload402, label %postload403

preload402:                                       ; preds = %postload392
  %76 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %77 = load i64* %76, align 8
  br label %postload403

postload403:                                      ; preds = %preload402, %postload392
  %phi404 = phi i64 [ undef, %postload392 ], [ %77, %preload402 ]
  br i1 %extract88, label %preload413, label %postload414

preload413:                                       ; preds = %postload403
  %78 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %79 = load i64* %78, align 8
  br label %postload414

postload414:                                      ; preds = %preload413, %postload403
  %phi415 = phi i64 [ undef, %postload403 ], [ %79, %preload413 ]
  br i1 %extract89, label %preload424, label %postload425

preload424:                                       ; preds = %postload414
  %80 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %81 = load i64* %80, align 8
  br label %postload425

postload425:                                      ; preds = %preload424, %postload414
  %phi426 = phi i64 [ undef, %postload414 ], [ %81, %preload424 ]
  br i1 %extract90, label %preload435, label %postload436

preload435:                                       ; preds = %postload425
  %82 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %83 = load i64* %82, align 8
  br label %postload436

postload436:                                      ; preds = %preload435, %postload425
  %phi437 = phi i64 [ undef, %postload425 ], [ %83, %preload435 ]
  br i1 %extract91, label %preload446, label %postload447

preload446:                                       ; preds = %postload436
  %84 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %85 = load i64* %84, align 8
  br label %postload447

postload447:                                      ; preds = %preload446, %postload436
  %phi448 = phi i64 [ undef, %postload436 ], [ %85, %preload446 ]
  br i1 %extract92, label %preload457, label %postload458

preload457:                                       ; preds = %postload447
  %86 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %87 = load i64* %86, align 8
  br label %postload458

postload458:                                      ; preds = %preload457, %postload447
  %phi459 = phi i64 [ undef, %postload447 ], [ %87, %preload457 ]
  br i1 %extract93, label %preload468, label %postload469

preload468:                                       ; preds = %postload458
  %88 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %89 = load i64* %88, align 8
  br label %postload469

postload469:                                      ; preds = %preload468, %postload458
  %phi470 = phi i64 [ undef, %postload458 ], [ %89, %preload468 ]
  br i1 %extract94, label %preload479, label %postload480

preload479:                                       ; preds = %postload469
  %90 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %91 = load i64* %90, align 8
  br label %postload480

postload480:                                      ; preds = %preload479, %postload469
  %phi481 = phi i64 [ undef, %postload469 ], [ %91, %preload479 ]
  br i1 %extract95, label %preload490, label %postload491

preload490:                                       ; preds = %postload480
  %92 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %93 = load i64* %92, align 8
  br label %postload491

postload491:                                      ; preds = %preload490, %postload480
  %phi492 = phi i64 [ undef, %postload480 ], [ %93, %preload490 ]
  br i1 %extract96, label %preload501, label %postload502

preload501:                                       ; preds = %postload491
  %94 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %95 = load i64* %94, align 8
  br label %postload502

postload502:                                      ; preds = %preload501, %postload491
  %phi503 = phi i64 [ undef, %postload491 ], [ %95, %preload501 ]
  br i1 %extract97, label %preload512, label %postload513

preload512:                                       ; preds = %postload502
  %96 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %97 = load i64* %96, align 8
  br label %postload513

postload513:                                      ; preds = %preload512, %postload502
  %phi514 = phi i64 [ undef, %postload502 ], [ %97, %preload512 ]
  br i1 %extract98, label %preload523, label %postload524

preload523:                                       ; preds = %postload513
  %98 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %99 = load i64* %98, align 8
  br label %postload524

postload524:                                      ; preds = %preload523, %postload513
  %phi525 = phi i64 [ undef, %postload513 ], [ %99, %preload523 ]
  %temp.vect162 = insertelement <16 x i64> undef, i64 %phi360, i32 0
  %temp.vect163 = insertelement <16 x i64> %temp.vect162, i64 %phi371, i32 1
  %temp.vect164 = insertelement <16 x i64> %temp.vect163, i64 %phi382, i32 2
  %temp.vect165 = insertelement <16 x i64> %temp.vect164, i64 %phi393, i32 3
  %temp.vect166 = insertelement <16 x i64> %temp.vect165, i64 %phi404, i32 4
  %temp.vect167 = insertelement <16 x i64> %temp.vect166, i64 %phi415, i32 5
  %temp.vect168 = insertelement <16 x i64> %temp.vect167, i64 %phi426, i32 6
  %temp.vect169 = insertelement <16 x i64> %temp.vect168, i64 %phi437, i32 7
  %temp.vect170 = insertelement <16 x i64> %temp.vect169, i64 %phi448, i32 8
  %temp.vect171 = insertelement <16 x i64> %temp.vect170, i64 %phi459, i32 9
  %temp.vect172 = insertelement <16 x i64> %temp.vect171, i64 %phi470, i32 10
  %temp.vect173 = insertelement <16 x i64> %temp.vect172, i64 %phi481, i32 11
  %temp.vect174 = insertelement <16 x i64> %temp.vect173, i64 %phi492, i32 12
  %temp.vect175 = insertelement <16 x i64> %temp.vect174, i64 %phi503, i32 13
  %temp.vect176 = insertelement <16 x i64> %temp.vect175, i64 %phi514, i32 14
  %temp.vect177 = insertelement <16 x i64> %temp.vect176, i64 %phi525, i32 15
  %100 = add <16 x i64> %temp.vect177, %50
  %101 = add nsw i32 %47, 1
  %ipred.i5 = bitcast <16 x i1> %loop_mask2180 to i16
  %val.i6 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i5, i16 %ipred.i5) nounwind
  %tmp.i7 = and i32 %val.i6, 1
  %res.i8 = icmp eq i32 %tmp.i7, 0
  br i1 %res.i8, label %postload351, label %.critedge

.critedge:                                        ; preds = %postload524
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB532

thenBB:                                           ; preds = %.critedge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 1792
  br label %SyncBB533

SyncBB532:                                        ; preds = %.critedge
  ret void
}

define void @____Vectorized_.scan_separated_args(float addrspace(1)* nocapture %g_odata, float addrspace(1)* nocapture %g_idata, float addrspace(1)* nocapture %g_blockSums, i32 %n, i32 %fullBlock, i32 %storeSum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [512 x float] addrspace(3)*
  %temp = insertelement <16 x i32> undef, i32 %n, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %1 = bitcast float addrspace(1)* %g_idata to <4 x float> addrspace(1)*
  %2 = icmp eq i32 %fullBlock, 0
  %Mneg = xor i1 %2, true
  %temp186 = insertelement <16 x i1> undef, i1 %Mneg, i32 0
  %vector187 = shufflevector <16 x i1> %temp186, <16 x i1> undef, <16 x i32> zeroinitializer
  %temp181 = insertelement <16 x i1> undef, i1 %2, i32 0
  %vector182 = shufflevector <16 x i1> %temp181, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB2176

SyncBB2176:                                       ; preds = %thenBB2198, %FirstBB
  %CurrSBIndex..1 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride2204", %thenBB2198 ]
  %CurrWI..1 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++2202", %thenBB2198 ]
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %broadcast1 = insertelement <16 x i64> undef, i64 %7, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %8 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %9 = trunc <16 x i64> %8 to <16 x i32>
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..1, 256
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to <16 x i32>*
  store <16 x i32> %9, <16 x i32>* %CastToValueType, align 64
  %10 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %11 = load i64* %10, align 8
  %broadcast1178 = insertelement <16 x i64> undef, i64 %11, i32 0
  %broadcast2179 = shufflevector <16 x i64> %broadcast1178, <16 x i64> undef, <16 x i32> zeroinitializer
  %12 = add <16 x i64> %broadcast2179, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)1447" = add nuw i64 %CurrSBIndex..1, 384
  %"&pSB[currWI].offset1448" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1447"
  %CastToValueType1449 = bitcast i8* %"&pSB[currWI].offset1448" to <16 x i64>*
  store <16 x i64> %12, <16 x i64>* %CastToValueType1449, align 128
  %13 = shl <16 x i32> %9, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %"&(pSB[currWI].offset)1456" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1457" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1456"
  %CastToValueType1458 = bitcast i8* %"&pSB[currWI].offset1457" to <16 x i32>*
  store <16 x i32> %13, <16 x i32>* %CastToValueType1458, align 64
  %14 = or <16 x i32> %13, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %15 = icmp slt <16 x i32> %14, %vector
  %Mneg56180 = xor <16 x i1> %15, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %_to_59183 = and <16 x i1> %vector182, %Mneg56180
  %_to_60184 = and <16 x i1> %vector182, %15
  %_Min118188 = or <16 x i1> %_to_60184, %vector187
  %extract204 = extractelement <16 x i1> %_Min118188, i32 0
  %extract205 = extractelement <16 x i1> %_Min118188, i32 1
  %extract206 = extractelement <16 x i1> %_Min118188, i32 2
  %extract207 = extractelement <16 x i1> %_Min118188, i32 3
  %extract208 = extractelement <16 x i1> %_Min118188, i32 4
  %extract209 = extractelement <16 x i1> %_Min118188, i32 5
  %extract210 = extractelement <16 x i1> %_Min118188, i32 6
  %extract211 = extractelement <16 x i1> %_Min118188, i32 7
  %extract212 = extractelement <16 x i1> %_Min118188, i32 8
  %extract213 = extractelement <16 x i1> %_Min118188, i32 9
  %extract214 = extractelement <16 x i1> %_Min118188, i32 10
  %extract215 = extractelement <16 x i1> %_Min118188, i32 11
  %extract216 = extractelement <16 x i1> %_Min118188, i32 12
  %extract217 = extractelement <16 x i1> %_Min118188, i32 13
  %extract218 = extractelement <16 x i1> %_Min118188, i32 14
  %extract219 = extractelement <16 x i1> %_Min118188, i32 15
  %16 = extractelement <16 x i32> %9, i32 1
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %17
  %19 = extractelement <16 x i32> %9, i32 2
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %20
  %22 = extractelement <16 x i32> %9, i32 3
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %23
  %25 = extractelement <16 x i32> %9, i32 4
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %26
  %28 = extractelement <16 x i32> %9, i32 5
  %29 = sext i32 %28 to i64
  %30 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %29
  %31 = extractelement <16 x i32> %9, i32 6
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %32
  %34 = extractelement <16 x i32> %9, i32 7
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %35
  %37 = extractelement <16 x i32> %9, i32 8
  %38 = sext i32 %37 to i64
  %39 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %38
  %40 = extractelement <16 x i32> %9, i32 9
  %41 = sext i32 %40 to i64
  %42 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %41
  %43 = extractelement <16 x i32> %9, i32 10
  %44 = sext i32 %43 to i64
  %45 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %44
  %46 = extractelement <16 x i32> %9, i32 11
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %47
  %49 = extractelement <16 x i32> %9, i32 12
  %50 = sext i32 %49 to i64
  %51 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %50
  %52 = extractelement <16 x i32> %9, i32 13
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %53
  %55 = extractelement <16 x i32> %9, i32 14
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %56
  %58 = extractelement <16 x i32> %9, i32 15
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %59
  br i1 %extract204, label %preload944, label %postload945

preload944:                                       ; preds = %SyncBB2176
  %"&(pSB[currWI].offset)1442" = add nuw i64 %CurrSBIndex..1, 256
  %"&pSB[currWI].offset1443" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1442"
  %CastToValueType1444 = bitcast i8* %"&pSB[currWI].offset1443" to <16 x i32>*
  %loadedValue1445 = load <16 x i32>* %CastToValueType1444, align 64
  %61 = extractelement <16 x i32> %loadedValue1445, i32 0
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds <4 x float> addrspace(1)* %1, i64 %62
  %masked_load = load <4 x float> addrspace(1)* %63, align 16
  br label %postload945

postload945:                                      ; preds = %preload944, %SyncBB2176
  %phi946 = phi <4 x float> [ undef, %SyncBB2176 ], [ %masked_load, %preload944 ]
  br i1 %extract205, label %preload947, label %postload948

preload947:                                       ; preds = %postload945
  %masked_load872 = load <4 x float> addrspace(1)* %18, align 16
  br label %postload948

postload948:                                      ; preds = %preload947, %postload945
  %phi949 = phi <4 x float> [ undef, %postload945 ], [ %masked_load872, %preload947 ]
  br i1 %extract206, label %preload950, label %postload951

preload950:                                       ; preds = %postload948
  %masked_load873 = load <4 x float> addrspace(1)* %21, align 16
  br label %postload951

postload951:                                      ; preds = %preload950, %postload948
  %phi952 = phi <4 x float> [ undef, %postload948 ], [ %masked_load873, %preload950 ]
  br i1 %extract207, label %preload953, label %postload954

preload953:                                       ; preds = %postload951
  %masked_load874 = load <4 x float> addrspace(1)* %24, align 16
  br label %postload954

postload954:                                      ; preds = %preload953, %postload951
  %phi955 = phi <4 x float> [ undef, %postload951 ], [ %masked_load874, %preload953 ]
  br i1 %extract208, label %preload956, label %postload957

preload956:                                       ; preds = %postload954
  %masked_load875 = load <4 x float> addrspace(1)* %27, align 16
  br label %postload957

postload957:                                      ; preds = %preload956, %postload954
  %phi958 = phi <4 x float> [ undef, %postload954 ], [ %masked_load875, %preload956 ]
  br i1 %extract209, label %preload959, label %postload960

preload959:                                       ; preds = %postload957
  %masked_load876 = load <4 x float> addrspace(1)* %30, align 16
  br label %postload960

postload960:                                      ; preds = %preload959, %postload957
  %phi961 = phi <4 x float> [ undef, %postload957 ], [ %masked_load876, %preload959 ]
  br i1 %extract210, label %preload962, label %postload963

preload962:                                       ; preds = %postload960
  %masked_load877 = load <4 x float> addrspace(1)* %33, align 16
  br label %postload963

postload963:                                      ; preds = %preload962, %postload960
  %phi964 = phi <4 x float> [ undef, %postload960 ], [ %masked_load877, %preload962 ]
  br i1 %extract211, label %preload965, label %postload966

preload965:                                       ; preds = %postload963
  %masked_load878 = load <4 x float> addrspace(1)* %36, align 16
  br label %postload966

postload966:                                      ; preds = %preload965, %postload963
  %phi967 = phi <4 x float> [ undef, %postload963 ], [ %masked_load878, %preload965 ]
  br i1 %extract212, label %preload968, label %postload969

preload968:                                       ; preds = %postload966
  %masked_load879 = load <4 x float> addrspace(1)* %39, align 16
  br label %postload969

postload969:                                      ; preds = %preload968, %postload966
  %phi970 = phi <4 x float> [ undef, %postload966 ], [ %masked_load879, %preload968 ]
  br i1 %extract213, label %preload971, label %postload972

preload971:                                       ; preds = %postload969
  %masked_load880 = load <4 x float> addrspace(1)* %42, align 16
  br label %postload972

postload972:                                      ; preds = %preload971, %postload969
  %phi973 = phi <4 x float> [ undef, %postload969 ], [ %masked_load880, %preload971 ]
  br i1 %extract214, label %preload, label %postload

preload:                                          ; preds = %postload972
  %masked_load881 = load <4 x float> addrspace(1)* %45, align 16
  br label %postload

postload:                                         ; preds = %preload, %postload972
  %phi = phi <4 x float> [ undef, %postload972 ], [ %masked_load881, %preload ]
  br i1 %extract215, label %preload935, label %postload936

preload935:                                       ; preds = %postload
  %masked_load882 = load <4 x float> addrspace(1)* %48, align 16
  br label %postload936

postload936:                                      ; preds = %preload935, %postload
  %phi937 = phi <4 x float> [ undef, %postload ], [ %masked_load882, %preload935 ]
  br i1 %extract216, label %preload938, label %postload939

preload938:                                       ; preds = %postload936
  %masked_load883 = load <4 x float> addrspace(1)* %51, align 16
  br label %postload939

postload939:                                      ; preds = %preload938, %postload936
  %phi940 = phi <4 x float> [ undef, %postload936 ], [ %masked_load883, %preload938 ]
  br i1 %extract217, label %preload941, label %postload942

preload941:                                       ; preds = %postload939
  %masked_load884 = load <4 x float> addrspace(1)* %54, align 16
  br label %postload942

postload942:                                      ; preds = %preload941, %postload939
  %phi943 = phi <4 x float> [ undef, %postload939 ], [ %masked_load884, %preload941 ]
  br i1 %extract218, label %preload974, label %postload975

preload974:                                       ; preds = %postload942
  %masked_load885 = load <4 x float> addrspace(1)* %57, align 16
  br label %postload975

postload975:                                      ; preds = %preload974, %postload942
  %phi976 = phi <4 x float> [ undef, %postload942 ], [ %masked_load885, %preload974 ]
  br i1 %extract219, label %preload977, label %postload978

preload977:                                       ; preds = %postload975
  %masked_load886 = load <4 x float> addrspace(1)* %60, align 16
  br label %postload978

postload978:                                      ; preds = %preload977, %postload975
  %phi979 = phi <4 x float> [ undef, %postload975 ], [ %masked_load886, %preload977 ]
  %64 = extractelement <4 x float> %phi946, i32 0
  %65 = extractelement <4 x float> %phi949, i32 0
  %66 = extractelement <4 x float> %phi952, i32 0
  %67 = extractelement <4 x float> %phi955, i32 0
  %68 = extractelement <4 x float> %phi958, i32 0
  %69 = extractelement <4 x float> %phi961, i32 0
  %70 = extractelement <4 x float> %phi964, i32 0
  %71 = extractelement <4 x float> %phi967, i32 0
  %72 = extractelement <4 x float> %phi970, i32 0
  %73 = extractelement <4 x float> %phi973, i32 0
  %74 = extractelement <4 x float> %phi, i32 0
  %75 = extractelement <4 x float> %phi937, i32 0
  %76 = extractelement <4 x float> %phi940, i32 0
  %77 = extractelement <4 x float> %phi943, i32 0
  %78 = extractelement <4 x float> %phi976, i32 0
  %79 = extractelement <4 x float> %phi979, i32 0
  %temp.vect236 = insertelement <16 x float> undef, float %64, i32 0
  %temp.vect237 = insertelement <16 x float> %temp.vect236, float %65, i32 1
  %temp.vect238 = insertelement <16 x float> %temp.vect237, float %66, i32 2
  %temp.vect239 = insertelement <16 x float> %temp.vect238, float %67, i32 3
  %temp.vect240 = insertelement <16 x float> %temp.vect239, float %68, i32 4
  %temp.vect241 = insertelement <16 x float> %temp.vect240, float %69, i32 5
  %temp.vect242 = insertelement <16 x float> %temp.vect241, float %70, i32 6
  %temp.vect243 = insertelement <16 x float> %temp.vect242, float %71, i32 7
  %temp.vect244 = insertelement <16 x float> %temp.vect243, float %72, i32 8
  %temp.vect245 = insertelement <16 x float> %temp.vect244, float %73, i32 9
  %temp.vect246 = insertelement <16 x float> %temp.vect245, float %74, i32 10
  %temp.vect247 = insertelement <16 x float> %temp.vect246, float %75, i32 11
  %temp.vect248 = insertelement <16 x float> %temp.vect247, float %76, i32 12
  %temp.vect249 = insertelement <16 x float> %temp.vect248, float %77, i32 13
  %temp.vect250 = insertelement <16 x float> %temp.vect249, float %78, i32 14
  %temp.vect251 = insertelement <16 x float> %temp.vect250, float %79, i32 15
  %80 = extractelement <4 x float> %phi946, i32 1
  %81 = extractelement <4 x float> %phi949, i32 1
  %82 = extractelement <4 x float> %phi952, i32 1
  %83 = extractelement <4 x float> %phi955, i32 1
  %84 = extractelement <4 x float> %phi958, i32 1
  %85 = extractelement <4 x float> %phi961, i32 1
  %86 = extractelement <4 x float> %phi964, i32 1
  %87 = extractelement <4 x float> %phi967, i32 1
  %88 = extractelement <4 x float> %phi970, i32 1
  %89 = extractelement <4 x float> %phi973, i32 1
  %90 = extractelement <4 x float> %phi, i32 1
  %91 = extractelement <4 x float> %phi937, i32 1
  %92 = extractelement <4 x float> %phi940, i32 1
  %93 = extractelement <4 x float> %phi943, i32 1
  %94 = extractelement <4 x float> %phi976, i32 1
  %95 = extractelement <4 x float> %phi979, i32 1
  %temp.vect220 = insertelement <16 x float> undef, float %80, i32 0
  %temp.vect221 = insertelement <16 x float> %temp.vect220, float %81, i32 1
  %temp.vect222 = insertelement <16 x float> %temp.vect221, float %82, i32 2
  %temp.vect223 = insertelement <16 x float> %temp.vect222, float %83, i32 3
  %temp.vect224 = insertelement <16 x float> %temp.vect223, float %84, i32 4
  %temp.vect225 = insertelement <16 x float> %temp.vect224, float %85, i32 5
  %temp.vect226 = insertelement <16 x float> %temp.vect225, float %86, i32 6
  %temp.vect227 = insertelement <16 x float> %temp.vect226, float %87, i32 7
  %temp.vect228 = insertelement <16 x float> %temp.vect227, float %88, i32 8
  %temp.vect229 = insertelement <16 x float> %temp.vect228, float %89, i32 9
  %temp.vect230 = insertelement <16 x float> %temp.vect229, float %90, i32 10
  %temp.vect231 = insertelement <16 x float> %temp.vect230, float %91, i32 11
  %temp.vect232 = insertelement <16 x float> %temp.vect231, float %92, i32 12
  %temp.vect233 = insertelement <16 x float> %temp.vect232, float %93, i32 13
  %temp.vect234 = insertelement <16 x float> %temp.vect233, float %94, i32 14
  %temp.vect235 = insertelement <16 x float> %temp.vect234, float %95, i32 15
  %96 = extractelement <4 x float> %phi946, i32 2
  %97 = extractelement <4 x float> %phi949, i32 2
  %98 = extractelement <4 x float> %phi952, i32 2
  %99 = extractelement <4 x float> %phi955, i32 2
  %100 = extractelement <4 x float> %phi958, i32 2
  %101 = extractelement <4 x float> %phi961, i32 2
  %102 = extractelement <4 x float> %phi964, i32 2
  %103 = extractelement <4 x float> %phi967, i32 2
  %104 = extractelement <4 x float> %phi970, i32 2
  %105 = extractelement <4 x float> %phi973, i32 2
  %106 = extractelement <4 x float> %phi, i32 2
  %107 = extractelement <4 x float> %phi937, i32 2
  %108 = extractelement <4 x float> %phi940, i32 2
  %109 = extractelement <4 x float> %phi943, i32 2
  %110 = extractelement <4 x float> %phi976, i32 2
  %111 = extractelement <4 x float> %phi979, i32 2
  %temp.vect252 = insertelement <16 x float> undef, float %96, i32 0
  %temp.vect253 = insertelement <16 x float> %temp.vect252, float %97, i32 1
  %temp.vect254 = insertelement <16 x float> %temp.vect253, float %98, i32 2
  %temp.vect255 = insertelement <16 x float> %temp.vect254, float %99, i32 3
  %temp.vect256 = insertelement <16 x float> %temp.vect255, float %100, i32 4
  %temp.vect257 = insertelement <16 x float> %temp.vect256, float %101, i32 5
  %temp.vect258 = insertelement <16 x float> %temp.vect257, float %102, i32 6
  %temp.vect259 = insertelement <16 x float> %temp.vect258, float %103, i32 7
  %temp.vect260 = insertelement <16 x float> %temp.vect259, float %104, i32 8
  %temp.vect261 = insertelement <16 x float> %temp.vect260, float %105, i32 9
  %temp.vect262 = insertelement <16 x float> %temp.vect261, float %106, i32 10
  %temp.vect263 = insertelement <16 x float> %temp.vect262, float %107, i32 11
  %temp.vect264 = insertelement <16 x float> %temp.vect263, float %108, i32 12
  %temp.vect265 = insertelement <16 x float> %temp.vect264, float %109, i32 13
  %temp.vect266 = insertelement <16 x float> %temp.vect265, float %110, i32 14
  %temp.vect267 = insertelement <16 x float> %temp.vect266, float %111, i32 15
  %112 = extractelement <4 x float> %phi946, i32 3
  %113 = extractelement <4 x float> %phi949, i32 3
  %114 = extractelement <4 x float> %phi952, i32 3
  %115 = extractelement <4 x float> %phi955, i32 3
  %116 = extractelement <4 x float> %phi958, i32 3
  %117 = extractelement <4 x float> %phi961, i32 3
  %118 = extractelement <4 x float> %phi964, i32 3
  %119 = extractelement <4 x float> %phi967, i32 3
  %120 = extractelement <4 x float> %phi970, i32 3
  %121 = extractelement <4 x float> %phi973, i32 3
  %122 = extractelement <4 x float> %phi, i32 3
  %123 = extractelement <4 x float> %phi937, i32 3
  %124 = extractelement <4 x float> %phi940, i32 3
  %125 = extractelement <4 x float> %phi943, i32 3
  %126 = extractelement <4 x float> %phi976, i32 3
  %127 = extractelement <4 x float> %phi979, i32 3
  %temp.vect268 = insertelement <16 x float> undef, float %112, i32 0
  %temp.vect269 = insertelement <16 x float> %temp.vect268, float %113, i32 1
  %temp.vect270 = insertelement <16 x float> %temp.vect269, float %114, i32 2
  %temp.vect271 = insertelement <16 x float> %temp.vect270, float %115, i32 3
  %temp.vect272 = insertelement <16 x float> %temp.vect271, float %116, i32 4
  %temp.vect273 = insertelement <16 x float> %temp.vect272, float %117, i32 5
  %temp.vect274 = insertelement <16 x float> %temp.vect273, float %118, i32 6
  %temp.vect275 = insertelement <16 x float> %temp.vect274, float %119, i32 7
  %temp.vect276 = insertelement <16 x float> %temp.vect275, float %120, i32 8
  %temp.vect277 = insertelement <16 x float> %temp.vect276, float %121, i32 9
  %temp.vect278 = insertelement <16 x float> %temp.vect277, float %122, i32 10
  %temp.vect279 = insertelement <16 x float> %temp.vect278, float %123, i32 11
  %temp.vect280 = insertelement <16 x float> %temp.vect279, float %124, i32 12
  %temp.vect281 = insertelement <16 x float> %temp.vect280, float %125, i32 13
  %temp.vect282 = insertelement <16 x float> %temp.vect281, float %126, i32 14
  %temp.vect283 = insertelement <16 x float> %temp.vect282, float %127, i32 15
  %128 = fadd <16 x float> %temp.vect235, %temp.vect251
  %129 = fadd <16 x float> %temp.vect267, %128
  %130 = fadd <16 x float> %temp.vect283, %129
  %"&(pSB[currWI].offset)1650" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1651" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1650"
  %CastToValueType1652 = bitcast i8* %"&pSB[currWI].offset1651" to <16 x i32>*
  %loadedValue1653 = load <16 x i32>* %CastToValueType1652, align 64
  %131 = icmp slt <16 x i32> %loadedValue1653, %vector
  %_to_66284 = and <16 x i1> %_to_59183, %131
  %extract302 = extractelement <16 x i1> %_to_66284, i32 1
  %extract303 = extractelement <16 x i1> %_to_66284, i32 2
  %extract304 = extractelement <16 x i1> %_to_66284, i32 3
  %extract305 = extractelement <16 x i1> %_to_66284, i32 4
  %extract306 = extractelement <16 x i1> %_to_66284, i32 5
  %extract307 = extractelement <16 x i1> %_to_66284, i32 6
  %extract308 = extractelement <16 x i1> %_to_66284, i32 7
  %extract309 = extractelement <16 x i1> %_to_66284, i32 8
  %extract310 = extractelement <16 x i1> %_to_66284, i32 9
  %extract311 = extractelement <16 x i1> %_to_66284, i32 10
  %extract312 = extractelement <16 x i1> %_to_66284, i32 11
  %extract313 = extractelement <16 x i1> %_to_66284, i32 12
  %extract314 = extractelement <16 x i1> %_to_66284, i32 13
  %extract315 = extractelement <16 x i1> %_to_66284, i32 14
  %extract316 = extractelement <16 x i1> %_to_66284, i32 15
  %extract301 = extractelement <16 x i1> %_to_66284, i32 0
  %"&(pSB[currWI].offset)1610" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1611" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1610"
  %CastToValueType1612 = bitcast i8* %"&pSB[currWI].offset1611" to <16 x i32>*
  %loadedValue1613 = load <16 x i32>* %CastToValueType1612, align 64
  %132 = extractelement <16 x i32> %loadedValue1613, i32 1
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %133
  %"&(pSB[currWI].offset)1605" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1606" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1605"
  %CastToValueType1607 = bitcast i8* %"&pSB[currWI].offset1606" to <16 x i32>*
  %loadedValue1608 = load <16 x i32>* %CastToValueType1607, align 64
  %135 = extractelement <16 x i32> %loadedValue1608, i32 2
  %136 = sext i32 %135 to i64
  %137 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %136
  %"&(pSB[currWI].offset)1600" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1601" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1600"
  %CastToValueType1602 = bitcast i8* %"&pSB[currWI].offset1601" to <16 x i32>*
  %loadedValue1603 = load <16 x i32>* %CastToValueType1602, align 64
  %138 = extractelement <16 x i32> %loadedValue1603, i32 3
  %139 = sext i32 %138 to i64
  %140 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %139
  %"&(pSB[currWI].offset)1595" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1596" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1595"
  %CastToValueType1597 = bitcast i8* %"&pSB[currWI].offset1596" to <16 x i32>*
  %loadedValue1598 = load <16 x i32>* %CastToValueType1597, align 64
  %141 = extractelement <16 x i32> %loadedValue1598, i32 4
  %142 = sext i32 %141 to i64
  %143 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %142
  %"&(pSB[currWI].offset)1590" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1591" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1590"
  %CastToValueType1592 = bitcast i8* %"&pSB[currWI].offset1591" to <16 x i32>*
  %loadedValue1593 = load <16 x i32>* %CastToValueType1592, align 64
  %144 = extractelement <16 x i32> %loadedValue1593, i32 5
  %145 = sext i32 %144 to i64
  %146 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %145
  %"&(pSB[currWI].offset)1585" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1586" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1585"
  %CastToValueType1587 = bitcast i8* %"&pSB[currWI].offset1586" to <16 x i32>*
  %loadedValue1588 = load <16 x i32>* %CastToValueType1587, align 64
  %147 = extractelement <16 x i32> %loadedValue1588, i32 6
  %148 = sext i32 %147 to i64
  %149 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %148
  %"&(pSB[currWI].offset)1580" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1581" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1580"
  %CastToValueType1582 = bitcast i8* %"&pSB[currWI].offset1581" to <16 x i32>*
  %loadedValue1583 = load <16 x i32>* %CastToValueType1582, align 64
  %150 = extractelement <16 x i32> %loadedValue1583, i32 7
  %151 = sext i32 %150 to i64
  %152 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %151
  %"&(pSB[currWI].offset)1575" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1576" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1575"
  %CastToValueType1577 = bitcast i8* %"&pSB[currWI].offset1576" to <16 x i32>*
  %loadedValue1578 = load <16 x i32>* %CastToValueType1577, align 64
  %153 = extractelement <16 x i32> %loadedValue1578, i32 8
  %154 = sext i32 %153 to i64
  %155 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %154
  %"&(pSB[currWI].offset)1570" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1571" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1570"
  %CastToValueType1572 = bitcast i8* %"&pSB[currWI].offset1571" to <16 x i32>*
  %loadedValue1573 = load <16 x i32>* %CastToValueType1572, align 64
  %156 = extractelement <16 x i32> %loadedValue1573, i32 9
  %157 = sext i32 %156 to i64
  %158 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %157
  %"&(pSB[currWI].offset)1565" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1566" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1565"
  %CastToValueType1567 = bitcast i8* %"&pSB[currWI].offset1566" to <16 x i32>*
  %loadedValue1568 = load <16 x i32>* %CastToValueType1567, align 64
  %159 = extractelement <16 x i32> %loadedValue1568, i32 10
  %160 = sext i32 %159 to i64
  %161 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %160
  %"&(pSB[currWI].offset)1560" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1561" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1560"
  %CastToValueType1562 = bitcast i8* %"&pSB[currWI].offset1561" to <16 x i32>*
  %loadedValue1563 = load <16 x i32>* %CastToValueType1562, align 64
  %162 = extractelement <16 x i32> %loadedValue1563, i32 11
  %163 = sext i32 %162 to i64
  %164 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %163
  %"&(pSB[currWI].offset)1555" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1556" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1555"
  %CastToValueType1557 = bitcast i8* %"&pSB[currWI].offset1556" to <16 x i32>*
  %loadedValue1558 = load <16 x i32>* %CastToValueType1557, align 64
  %165 = extractelement <16 x i32> %loadedValue1558, i32 12
  %166 = sext i32 %165 to i64
  %167 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %166
  %"&(pSB[currWI].offset)1550" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1551" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1550"
  %CastToValueType1552 = bitcast i8* %"&pSB[currWI].offset1551" to <16 x i32>*
  %loadedValue1553 = load <16 x i32>* %CastToValueType1552, align 64
  %168 = extractelement <16 x i32> %loadedValue1553, i32 13
  %169 = sext i32 %168 to i64
  %170 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %169
  %"&(pSB[currWI].offset)1545" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1546" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1545"
  %CastToValueType1547 = bitcast i8* %"&pSB[currWI].offset1546" to <16 x i32>*
  %loadedValue1548 = load <16 x i32>* %CastToValueType1547, align 64
  %171 = extractelement <16 x i32> %loadedValue1548, i32 14
  %172 = sext i32 %171 to i64
  %173 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %172
  %"&(pSB[currWI].offset)1540" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1541" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1540"
  %CastToValueType1542 = bitcast i8* %"&pSB[currWI].offset1541" to <16 x i32>*
  %loadedValue1543 = load <16 x i32>* %CastToValueType1542, align 64
  %174 = extractelement <16 x i32> %loadedValue1543, i32 15
  %175 = sext i32 %174 to i64
  %176 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %175
  br i1 %extract301, label %preload983, label %postload984

preload983:                                       ; preds = %postload978
  %"&(pSB[currWI].offset)1615" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1616" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1615"
  %CastToValueType1617 = bitcast i8* %"&pSB[currWI].offset1616" to <16 x i32>*
  %loadedValue1618 = load <16 x i32>* %CastToValueType1617, align 64
  %177 = extractelement <16 x i32> %loadedValue1618, i32 0
  %178 = sext i32 %177 to i64
  %179 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %178
  %masked_load887 = load float addrspace(1)* %179, align 4
  br label %postload984

postload984:                                      ; preds = %preload983, %postload978
  %phi985 = phi float [ undef, %postload978 ], [ %masked_load887, %preload983 ]
  br i1 %extract302, label %preload986, label %postload987

preload986:                                       ; preds = %postload984
  %masked_load888 = load float addrspace(1)* %134, align 4
  br label %postload987

postload987:                                      ; preds = %preload986, %postload984
  %phi988 = phi float [ undef, %postload984 ], [ %masked_load888, %preload986 ]
  br i1 %extract303, label %preload989, label %postload990

preload989:                                       ; preds = %postload987
  %masked_load889 = load float addrspace(1)* %137, align 4
  br label %postload990

postload990:                                      ; preds = %preload989, %postload987
  %phi991 = phi float [ undef, %postload987 ], [ %masked_load889, %preload989 ]
  br i1 %extract304, label %preload992, label %postload993

preload992:                                       ; preds = %postload990
  %masked_load890 = load float addrspace(1)* %140, align 4
  br label %postload993

postload993:                                      ; preds = %preload992, %postload990
  %phi994 = phi float [ undef, %postload990 ], [ %masked_load890, %preload992 ]
  br i1 %extract305, label %preload995, label %postload996

preload995:                                       ; preds = %postload993
  %masked_load891 = load float addrspace(1)* %143, align 4
  br label %postload996

postload996:                                      ; preds = %preload995, %postload993
  %phi997 = phi float [ undef, %postload993 ], [ %masked_load891, %preload995 ]
  br i1 %extract306, label %preload998, label %postload999

preload998:                                       ; preds = %postload996
  %masked_load892 = load float addrspace(1)* %146, align 4
  br label %postload999

postload999:                                      ; preds = %preload998, %postload996
  %phi1000 = phi float [ undef, %postload996 ], [ %masked_load892, %preload998 ]
  br i1 %extract307, label %preload1001, label %postload1002

preload1001:                                      ; preds = %postload999
  %masked_load893 = load float addrspace(1)* %149, align 4
  br label %postload1002

postload1002:                                     ; preds = %preload1001, %postload999
  %phi1003 = phi float [ undef, %postload999 ], [ %masked_load893, %preload1001 ]
  br i1 %extract308, label %preload1004, label %postload1005

preload1004:                                      ; preds = %postload1002
  %masked_load894 = load float addrspace(1)* %152, align 4
  br label %postload1005

postload1005:                                     ; preds = %preload1004, %postload1002
  %phi1006 = phi float [ undef, %postload1002 ], [ %masked_load894, %preload1004 ]
  br i1 %extract309, label %preload1007, label %postload1008

preload1007:                                      ; preds = %postload1005
  %masked_load895 = load float addrspace(1)* %155, align 4
  br label %postload1008

postload1008:                                     ; preds = %preload1007, %postload1005
  %phi1009 = phi float [ undef, %postload1005 ], [ %masked_load895, %preload1007 ]
  br i1 %extract310, label %preload1010, label %postload1011

preload1010:                                      ; preds = %postload1008
  %masked_load896 = load float addrspace(1)* %158, align 4
  br label %postload1011

postload1011:                                     ; preds = %preload1010, %postload1008
  %phi1012 = phi float [ undef, %postload1008 ], [ %masked_load896, %preload1010 ]
  br i1 %extract311, label %preload1013, label %postload1014

preload1013:                                      ; preds = %postload1011
  %masked_load897 = load float addrspace(1)* %161, align 4
  br label %postload1014

postload1014:                                     ; preds = %preload1013, %postload1011
  %phi1015 = phi float [ undef, %postload1011 ], [ %masked_load897, %preload1013 ]
  br i1 %extract312, label %preload1016, label %postload1017

preload1016:                                      ; preds = %postload1014
  %masked_load898 = load float addrspace(1)* %164, align 4
  br label %postload1017

postload1017:                                     ; preds = %preload1016, %postload1014
  %phi1018 = phi float [ undef, %postload1014 ], [ %masked_load898, %preload1016 ]
  br i1 %extract313, label %preload1019, label %postload1020

preload1019:                                      ; preds = %postload1017
  %masked_load899 = load float addrspace(1)* %167, align 4
  br label %postload1020

postload1020:                                     ; preds = %preload1019, %postload1017
  %phi1021 = phi float [ undef, %postload1017 ], [ %masked_load899, %preload1019 ]
  br i1 %extract314, label %preload1022, label %postload1023

preload1022:                                      ; preds = %postload1020
  %masked_load900 = load float addrspace(1)* %170, align 4
  br label %postload1023

postload1023:                                     ; preds = %preload1022, %postload1020
  %phi1024 = phi float [ undef, %postload1020 ], [ %masked_load900, %preload1022 ]
  br i1 %extract315, label %preload1025, label %postload1026

preload1025:                                      ; preds = %postload1023
  %masked_load901 = load float addrspace(1)* %173, align 4
  br label %postload1026

postload1026:                                     ; preds = %preload1025, %postload1023
  %phi1027 = phi float [ undef, %postload1023 ], [ %masked_load901, %preload1025 ]
  br i1 %extract316, label %preload1028, label %postload1029

preload1028:                                      ; preds = %postload1026
  %masked_load902 = load float addrspace(1)* %176, align 4
  br label %postload1029

postload1029:                                     ; preds = %preload1028, %postload1026
  %phi1030 = phi float [ undef, %postload1026 ], [ %masked_load902, %preload1028 ]
  %temp.vect317 = insertelement <16 x float> undef, float %phi985, i32 0
  %temp.vect318 = insertelement <16 x float> %temp.vect317, float %phi988, i32 1
  %temp.vect319 = insertelement <16 x float> %temp.vect318, float %phi991, i32 2
  %temp.vect320 = insertelement <16 x float> %temp.vect319, float %phi994, i32 3
  %temp.vect321 = insertelement <16 x float> %temp.vect320, float %phi997, i32 4
  %temp.vect322 = insertelement <16 x float> %temp.vect321, float %phi1000, i32 5
  %temp.vect323 = insertelement <16 x float> %temp.vect322, float %phi1003, i32 6
  %temp.vect324 = insertelement <16 x float> %temp.vect323, float %phi1006, i32 7
  %temp.vect325 = insertelement <16 x float> %temp.vect324, float %phi1009, i32 8
  %temp.vect326 = insertelement <16 x float> %temp.vect325, float %phi1012, i32 9
  %temp.vect327 = insertelement <16 x float> %temp.vect326, float %phi1015, i32 10
  %temp.vect328 = insertelement <16 x float> %temp.vect327, float %phi1018, i32 11
  %temp.vect329 = insertelement <16 x float> %temp.vect328, float %phi1021, i32 12
  %temp.vect330 = insertelement <16 x float> %temp.vect329, float %phi1024, i32 13
  %temp.vect331 = insertelement <16 x float> %temp.vect330, float %phi1027, i32 14
  %temp.vect332 = insertelement <16 x float> %temp.vect331, float %phi1030, i32 15
  %merge333 = select <16 x i1> %_to_66284, <16 x float> %temp.vect332, <16 x float> zeroinitializer
  %"&(pSB[currWI].offset)1645" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1646" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1645"
  %CastToValueType1647 = bitcast i8* %"&pSB[currWI].offset1646" to <16 x i32>*
  %loadedValue1648 = load <16 x i32>* %CastToValueType1647, align 64
  %180 = or <16 x i32> %loadedValue1648, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %181 = icmp slt <16 x i32> %180, %vector
  %_to_72334 = and <16 x i1> %_to_59183, %181
  %extract352 = extractelement <16 x i1> %_to_72334, i32 1
  %extract353 = extractelement <16 x i1> %_to_72334, i32 2
  %extract354 = extractelement <16 x i1> %_to_72334, i32 3
  %extract355 = extractelement <16 x i1> %_to_72334, i32 4
  %extract356 = extractelement <16 x i1> %_to_72334, i32 5
  %extract357 = extractelement <16 x i1> %_to_72334, i32 6
  %extract358 = extractelement <16 x i1> %_to_72334, i32 7
  %extract359 = extractelement <16 x i1> %_to_72334, i32 8
  %extract360 = extractelement <16 x i1> %_to_72334, i32 9
  %extract361 = extractelement <16 x i1> %_to_72334, i32 10
  %extract362 = extractelement <16 x i1> %_to_72334, i32 11
  %extract363 = extractelement <16 x i1> %_to_72334, i32 12
  %extract364 = extractelement <16 x i1> %_to_72334, i32 13
  %extract365 = extractelement <16 x i1> %_to_72334, i32 14
  %extract366 = extractelement <16 x i1> %_to_72334, i32 15
  %extract351 = extractelement <16 x i1> %_to_72334, i32 0
  %182 = extractelement <16 x i32> %180, i32 1
  %183 = sext i32 %182 to i64
  %184 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %183
  %185 = extractelement <16 x i32> %180, i32 2
  %186 = sext i32 %185 to i64
  %187 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %186
  %188 = extractelement <16 x i32> %180, i32 3
  %189 = sext i32 %188 to i64
  %190 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %189
  %191 = extractelement <16 x i32> %180, i32 4
  %192 = sext i32 %191 to i64
  %193 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %192
  %194 = extractelement <16 x i32> %180, i32 5
  %195 = sext i32 %194 to i64
  %196 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %195
  %197 = extractelement <16 x i32> %180, i32 6
  %198 = sext i32 %197 to i64
  %199 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %198
  %200 = extractelement <16 x i32> %180, i32 7
  %201 = sext i32 %200 to i64
  %202 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %201
  %203 = extractelement <16 x i32> %180, i32 8
  %204 = sext i32 %203 to i64
  %205 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %204
  %206 = extractelement <16 x i32> %180, i32 9
  %207 = sext i32 %206 to i64
  %208 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %207
  %209 = extractelement <16 x i32> %180, i32 10
  %210 = sext i32 %209 to i64
  %211 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %210
  %212 = extractelement <16 x i32> %180, i32 11
  %213 = sext i32 %212 to i64
  %214 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %213
  %215 = extractelement <16 x i32> %180, i32 12
  %216 = sext i32 %215 to i64
  %217 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %216
  %218 = extractelement <16 x i32> %180, i32 13
  %219 = sext i32 %218 to i64
  %220 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %219
  %221 = extractelement <16 x i32> %180, i32 14
  %222 = sext i32 %221 to i64
  %223 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %222
  %224 = extractelement <16 x i32> %180, i32 15
  %225 = sext i32 %224 to i64
  %226 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %225
  br i1 %extract351, label %preload1031, label %postload1032

preload1031:                                      ; preds = %postload1029
  %227 = extractelement <16 x i32> %180, i32 0
  %228 = sext i32 %227 to i64
  %229 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %228
  %masked_load903 = load float addrspace(1)* %229, align 4
  br label %postload1032

postload1032:                                     ; preds = %preload1031, %postload1029
  %phi1033 = phi float [ undef, %postload1029 ], [ %masked_load903, %preload1031 ]
  br i1 %extract352, label %preload1034, label %postload1035

preload1034:                                      ; preds = %postload1032
  %masked_load904 = load float addrspace(1)* %184, align 4
  br label %postload1035

postload1035:                                     ; preds = %preload1034, %postload1032
  %phi1036 = phi float [ undef, %postload1032 ], [ %masked_load904, %preload1034 ]
  br i1 %extract353, label %preload1037, label %postload1038

preload1037:                                      ; preds = %postload1035
  %masked_load905 = load float addrspace(1)* %187, align 4
  br label %postload1038

postload1038:                                     ; preds = %preload1037, %postload1035
  %phi1039 = phi float [ undef, %postload1035 ], [ %masked_load905, %preload1037 ]
  br i1 %extract354, label %preload1040, label %postload1041

preload1040:                                      ; preds = %postload1038
  %masked_load906 = load float addrspace(1)* %190, align 4
  br label %postload1041

postload1041:                                     ; preds = %preload1040, %postload1038
  %phi1042 = phi float [ undef, %postload1038 ], [ %masked_load906, %preload1040 ]
  br i1 %extract355, label %preload1043, label %postload1044

preload1043:                                      ; preds = %postload1041
  %masked_load907 = load float addrspace(1)* %193, align 4
  br label %postload1044

postload1044:                                     ; preds = %preload1043, %postload1041
  %phi1045 = phi float [ undef, %postload1041 ], [ %masked_load907, %preload1043 ]
  br i1 %extract356, label %preload1046, label %postload1047

preload1046:                                      ; preds = %postload1044
  %masked_load908 = load float addrspace(1)* %196, align 4
  br label %postload1047

postload1047:                                     ; preds = %preload1046, %postload1044
  %phi1048 = phi float [ undef, %postload1044 ], [ %masked_load908, %preload1046 ]
  br i1 %extract357, label %preload1049, label %postload1050

preload1049:                                      ; preds = %postload1047
  %masked_load909 = load float addrspace(1)* %199, align 4
  br label %postload1050

postload1050:                                     ; preds = %preload1049, %postload1047
  %phi1051 = phi float [ undef, %postload1047 ], [ %masked_load909, %preload1049 ]
  br i1 %extract358, label %preload1052, label %postload1053

preload1052:                                      ; preds = %postload1050
  %masked_load910 = load float addrspace(1)* %202, align 4
  br label %postload1053

postload1053:                                     ; preds = %preload1052, %postload1050
  %phi1054 = phi float [ undef, %postload1050 ], [ %masked_load910, %preload1052 ]
  br i1 %extract359, label %preload1055, label %postload1056

preload1055:                                      ; preds = %postload1053
  %masked_load911 = load float addrspace(1)* %205, align 4
  br label %postload1056

postload1056:                                     ; preds = %preload1055, %postload1053
  %phi1057 = phi float [ undef, %postload1053 ], [ %masked_load911, %preload1055 ]
  br i1 %extract360, label %preload1058, label %postload1059

preload1058:                                      ; preds = %postload1056
  %masked_load912 = load float addrspace(1)* %208, align 4
  br label %postload1059

postload1059:                                     ; preds = %preload1058, %postload1056
  %phi1060 = phi float [ undef, %postload1056 ], [ %masked_load912, %preload1058 ]
  br i1 %extract361, label %preload1061, label %postload1062

preload1061:                                      ; preds = %postload1059
  %masked_load913 = load float addrspace(1)* %211, align 4
  br label %postload1062

postload1062:                                     ; preds = %preload1061, %postload1059
  %phi1063 = phi float [ undef, %postload1059 ], [ %masked_load913, %preload1061 ]
  br i1 %extract362, label %preload1064, label %postload1065

preload1064:                                      ; preds = %postload1062
  %masked_load914 = load float addrspace(1)* %214, align 4
  br label %postload1065

postload1065:                                     ; preds = %preload1064, %postload1062
  %phi1066 = phi float [ undef, %postload1062 ], [ %masked_load914, %preload1064 ]
  br i1 %extract363, label %preload1067, label %postload1068

preload1067:                                      ; preds = %postload1065
  %masked_load915 = load float addrspace(1)* %217, align 4
  br label %postload1068

postload1068:                                     ; preds = %preload1067, %postload1065
  %phi1069 = phi float [ undef, %postload1065 ], [ %masked_load915, %preload1067 ]
  br i1 %extract364, label %preload1070, label %postload1071

preload1070:                                      ; preds = %postload1068
  %masked_load916 = load float addrspace(1)* %220, align 4
  br label %postload1071

postload1071:                                     ; preds = %preload1070, %postload1068
  %phi1072 = phi float [ undef, %postload1068 ], [ %masked_load916, %preload1070 ]
  br i1 %extract365, label %preload1073, label %postload1074

preload1073:                                      ; preds = %postload1071
  %masked_load917 = load float addrspace(1)* %223, align 4
  br label %postload1074

postload1074:                                     ; preds = %preload1073, %postload1071
  %phi1075 = phi float [ undef, %postload1071 ], [ %masked_load917, %preload1073 ]
  br i1 %extract366, label %preload1076, label %postload1077

preload1076:                                      ; preds = %postload1074
  %masked_load918 = load float addrspace(1)* %226, align 4
  br label %postload1077

postload1077:                                     ; preds = %preload1076, %postload1074
  %phi1078 = phi float [ undef, %postload1074 ], [ %masked_load918, %preload1076 ]
  %temp.vect367 = insertelement <16 x float> undef, float %phi1033, i32 0
  %temp.vect368 = insertelement <16 x float> %temp.vect367, float %phi1036, i32 1
  %temp.vect369 = insertelement <16 x float> %temp.vect368, float %phi1039, i32 2
  %temp.vect370 = insertelement <16 x float> %temp.vect369, float %phi1042, i32 3
  %temp.vect371 = insertelement <16 x float> %temp.vect370, float %phi1045, i32 4
  %temp.vect372 = insertelement <16 x float> %temp.vect371, float %phi1048, i32 5
  %temp.vect373 = insertelement <16 x float> %temp.vect372, float %phi1051, i32 6
  %temp.vect374 = insertelement <16 x float> %temp.vect373, float %phi1054, i32 7
  %temp.vect375 = insertelement <16 x float> %temp.vect374, float %phi1057, i32 8
  %temp.vect376 = insertelement <16 x float> %temp.vect375, float %phi1060, i32 9
  %temp.vect377 = insertelement <16 x float> %temp.vect376, float %phi1063, i32 10
  %temp.vect378 = insertelement <16 x float> %temp.vect377, float %phi1066, i32 11
  %temp.vect379 = insertelement <16 x float> %temp.vect378, float %phi1069, i32 12
  %temp.vect380 = insertelement <16 x float> %temp.vect379, float %phi1072, i32 13
  %temp.vect381 = insertelement <16 x float> %temp.vect380, float %phi1075, i32 14
  %temp.vect382 = insertelement <16 x float> %temp.vect381, float %phi1078, i32 15
  %merge152383 = select <16 x i1> %_to_72334, <16 x float> %temp.vect382, <16 x float> zeroinitializer
  %230 = fadd <16 x float> %merge152383, %merge333
  %"&(pSB[currWI].offset)1640" = add nuw i64 %CurrSBIndex..1, 512
  %"&pSB[currWI].offset1641" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1640"
  %CastToValueType1642 = bitcast i8* %"&pSB[currWI].offset1641" to <16 x i32>*
  %loadedValue1643 = load <16 x i32>* %CastToValueType1642, align 64
  %231 = or <16 x i32> %loadedValue1643, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %232 = icmp slt <16 x i32> %231, %vector
  %_to_78384 = and <16 x i1> %_to_59183, %232
  %extract402 = extractelement <16 x i1> %_to_78384, i32 1
  %extract403 = extractelement <16 x i1> %_to_78384, i32 2
  %extract404 = extractelement <16 x i1> %_to_78384, i32 3
  %extract405 = extractelement <16 x i1> %_to_78384, i32 4
  %extract406 = extractelement <16 x i1> %_to_78384, i32 5
  %extract407 = extractelement <16 x i1> %_to_78384, i32 6
  %extract408 = extractelement <16 x i1> %_to_78384, i32 7
  %extract409 = extractelement <16 x i1> %_to_78384, i32 8
  %extract410 = extractelement <16 x i1> %_to_78384, i32 9
  %extract411 = extractelement <16 x i1> %_to_78384, i32 10
  %extract412 = extractelement <16 x i1> %_to_78384, i32 11
  %extract413 = extractelement <16 x i1> %_to_78384, i32 12
  %extract414 = extractelement <16 x i1> %_to_78384, i32 13
  %extract415 = extractelement <16 x i1> %_to_78384, i32 14
  %extract416 = extractelement <16 x i1> %_to_78384, i32 15
  %extract401 = extractelement <16 x i1> %_to_78384, i32 0
  %233 = extractelement <16 x i32> %231, i32 1
  %234 = sext i32 %233 to i64
  %235 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %234
  %236 = extractelement <16 x i32> %231, i32 2
  %237 = sext i32 %236 to i64
  %238 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %237
  %239 = extractelement <16 x i32> %231, i32 3
  %240 = sext i32 %239 to i64
  %241 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %240
  %242 = extractelement <16 x i32> %231, i32 4
  %243 = sext i32 %242 to i64
  %244 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %243
  %245 = extractelement <16 x i32> %231, i32 5
  %246 = sext i32 %245 to i64
  %247 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %246
  %248 = extractelement <16 x i32> %231, i32 6
  %249 = sext i32 %248 to i64
  %250 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %249
  %251 = extractelement <16 x i32> %231, i32 7
  %252 = sext i32 %251 to i64
  %253 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %252
  %254 = extractelement <16 x i32> %231, i32 8
  %255 = sext i32 %254 to i64
  %256 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %255
  %257 = extractelement <16 x i32> %231, i32 9
  %258 = sext i32 %257 to i64
  %259 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %258
  %260 = extractelement <16 x i32> %231, i32 10
  %261 = sext i32 %260 to i64
  %262 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %261
  %263 = extractelement <16 x i32> %231, i32 11
  %264 = sext i32 %263 to i64
  %265 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %264
  %266 = extractelement <16 x i32> %231, i32 12
  %267 = sext i32 %266 to i64
  %268 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %267
  %269 = extractelement <16 x i32> %231, i32 13
  %270 = sext i32 %269 to i64
  %271 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %270
  %272 = extractelement <16 x i32> %231, i32 14
  %273 = sext i32 %272 to i64
  %274 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %273
  %275 = extractelement <16 x i32> %231, i32 15
  %276 = sext i32 %275 to i64
  %277 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %276
  br i1 %extract401, label %preload1079, label %postload1080

preload1079:                                      ; preds = %postload1077
  %278 = extractelement <16 x i32> %231, i32 0
  %279 = sext i32 %278 to i64
  %280 = getelementptr inbounds float addrspace(1)* %g_idata, i64 %279
  %masked_load919 = load float addrspace(1)* %280, align 4
  br label %postload1080

postload1080:                                     ; preds = %preload1079, %postload1077
  %phi1081 = phi float [ undef, %postload1077 ], [ %masked_load919, %preload1079 ]
  br i1 %extract402, label %preload1082, label %postload1083

preload1082:                                      ; preds = %postload1080
  %masked_load920 = load float addrspace(1)* %235, align 4
  br label %postload1083

postload1083:                                     ; preds = %preload1082, %postload1080
  %phi1084 = phi float [ undef, %postload1080 ], [ %masked_load920, %preload1082 ]
  br i1 %extract403, label %preload1085, label %postload1086

preload1085:                                      ; preds = %postload1083
  %masked_load921 = load float addrspace(1)* %238, align 4
  br label %postload1086

postload1086:                                     ; preds = %preload1085, %postload1083
  %phi1087 = phi float [ undef, %postload1083 ], [ %masked_load921, %preload1085 ]
  br i1 %extract404, label %preload1088, label %postload1089

preload1088:                                      ; preds = %postload1086
  %masked_load922 = load float addrspace(1)* %241, align 4
  br label %postload1089

postload1089:                                     ; preds = %preload1088, %postload1086
  %phi1090 = phi float [ undef, %postload1086 ], [ %masked_load922, %preload1088 ]
  br i1 %extract405, label %preload1091, label %postload1092

preload1091:                                      ; preds = %postload1089
  %masked_load923 = load float addrspace(1)* %244, align 4
  br label %postload1092

postload1092:                                     ; preds = %preload1091, %postload1089
  %phi1093 = phi float [ undef, %postload1089 ], [ %masked_load923, %preload1091 ]
  br i1 %extract406, label %preload1094, label %postload1095

preload1094:                                      ; preds = %postload1092
  %masked_load924 = load float addrspace(1)* %247, align 4
  br label %postload1095

postload1095:                                     ; preds = %preload1094, %postload1092
  %phi1096 = phi float [ undef, %postload1092 ], [ %masked_load924, %preload1094 ]
  br i1 %extract407, label %preload1097, label %postload1098

preload1097:                                      ; preds = %postload1095
  %masked_load925 = load float addrspace(1)* %250, align 4
  br label %postload1098

postload1098:                                     ; preds = %preload1097, %postload1095
  %phi1099 = phi float [ undef, %postload1095 ], [ %masked_load925, %preload1097 ]
  br i1 %extract408, label %preload1100, label %postload1101

preload1100:                                      ; preds = %postload1098
  %masked_load926 = load float addrspace(1)* %253, align 4
  br label %postload1101

postload1101:                                     ; preds = %preload1100, %postload1098
  %phi1102 = phi float [ undef, %postload1098 ], [ %masked_load926, %preload1100 ]
  br i1 %extract409, label %preload1103, label %postload1104

preload1103:                                      ; preds = %postload1101
  %masked_load927 = load float addrspace(1)* %256, align 4
  br label %postload1104

postload1104:                                     ; preds = %preload1103, %postload1101
  %phi1105 = phi float [ undef, %postload1101 ], [ %masked_load927, %preload1103 ]
  br i1 %extract410, label %preload1106, label %postload1107

preload1106:                                      ; preds = %postload1104
  %masked_load928 = load float addrspace(1)* %259, align 4
  br label %postload1107

postload1107:                                     ; preds = %preload1106, %postload1104
  %phi1108 = phi float [ undef, %postload1104 ], [ %masked_load928, %preload1106 ]
  br i1 %extract411, label %preload1109, label %postload1110

preload1109:                                      ; preds = %postload1107
  %masked_load929 = load float addrspace(1)* %262, align 4
  br label %postload1110

postload1110:                                     ; preds = %preload1109, %postload1107
  %phi1111 = phi float [ undef, %postload1107 ], [ %masked_load929, %preload1109 ]
  br i1 %extract412, label %preload1112, label %postload1113

preload1112:                                      ; preds = %postload1110
  %masked_load930 = load float addrspace(1)* %265, align 4
  br label %postload1113

postload1113:                                     ; preds = %preload1112, %postload1110
  %phi1114 = phi float [ undef, %postload1110 ], [ %masked_load930, %preload1112 ]
  br i1 %extract413, label %preload1115, label %postload1116

preload1115:                                      ; preds = %postload1113
  %masked_load931 = load float addrspace(1)* %268, align 4
  br label %postload1116

postload1116:                                     ; preds = %preload1115, %postload1113
  %phi1117 = phi float [ undef, %postload1113 ], [ %masked_load931, %preload1115 ]
  br i1 %extract414, label %preload1118, label %postload1119

preload1118:                                      ; preds = %postload1116
  %masked_load932 = load float addrspace(1)* %271, align 4
  br label %postload1119

postload1119:                                     ; preds = %preload1118, %postload1116
  %phi1120 = phi float [ undef, %postload1116 ], [ %masked_load932, %preload1118 ]
  br i1 %extract415, label %preload1121, label %postload1122

preload1121:                                      ; preds = %postload1119
  %masked_load933 = load float addrspace(1)* %274, align 4
  br label %postload1122

postload1122:                                     ; preds = %preload1121, %postload1119
  %phi1123 = phi float [ undef, %postload1119 ], [ %masked_load933, %preload1121 ]
  br i1 %extract416, label %preload1124, label %postload1125

preload1124:                                      ; preds = %postload1122
  %masked_load934 = load float addrspace(1)* %277, align 4
  br label %postload1125

postload1125:                                     ; preds = %preload1124, %postload1122
  %phi1126 = phi float [ undef, %postload1122 ], [ %masked_load934, %preload1124 ]
  %temp.vect417 = insertelement <16 x float> undef, float %phi1081, i32 0
  %temp.vect418 = insertelement <16 x float> %temp.vect417, float %phi1084, i32 1
  %temp.vect419 = insertelement <16 x float> %temp.vect418, float %phi1087, i32 2
  %temp.vect420 = insertelement <16 x float> %temp.vect419, float %phi1090, i32 3
  %temp.vect421 = insertelement <16 x float> %temp.vect420, float %phi1093, i32 4
  %temp.vect422 = insertelement <16 x float> %temp.vect421, float %phi1096, i32 5
  %temp.vect423 = insertelement <16 x float> %temp.vect422, float %phi1099, i32 6
  %temp.vect424 = insertelement <16 x float> %temp.vect423, float %phi1102, i32 7
  %temp.vect425 = insertelement <16 x float> %temp.vect424, float %phi1105, i32 8
  %temp.vect426 = insertelement <16 x float> %temp.vect425, float %phi1108, i32 9
  %temp.vect427 = insertelement <16 x float> %temp.vect426, float %phi1111, i32 10
  %temp.vect428 = insertelement <16 x float> %temp.vect427, float %phi1114, i32 11
  %temp.vect429 = insertelement <16 x float> %temp.vect428, float %phi1117, i32 12
  %temp.vect430 = insertelement <16 x float> %temp.vect429, float %phi1120, i32 13
  %temp.vect431 = insertelement <16 x float> %temp.vect430, float %phi1123, i32 14
  %temp.vect432 = insertelement <16 x float> %temp.vect431, float %phi1126, i32 15
  %merge154433 = select <16 x i1> %_to_78384, <16 x float> %temp.vect432, <16 x float> zeroinitializer
  %281 = fadd <16 x float> %merge154433, %230
  %282 = fadd <16 x float> %281, zeroinitializer
  %merge164434 = select <16 x i1> %_Min118188, <16 x float> %130, <16 x float> %282
  %"&(pSB[currWI].offset)1655" = add nuw i64 %CurrSBIndex..1, 576
  %"&pSB[currWI].offset1656" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1655"
  %CastToValueType1657 = bitcast i8* %"&pSB[currWI].offset1656" to <16 x float>*
  store <16 x float> %merge164434, <16 x float>* %CastToValueType1657, align 64
  %merge162435 = select <16 x i1> %_Min118188, <16 x float> %130, <16 x float> %282
  %"&(pSB[currWI].offset)1664" = add nuw i64 %CurrSBIndex..1, 640
  %"&pSB[currWI].offset1665" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1664"
  %CastToValueType1666 = bitcast i8* %"&pSB[currWI].offset1665" to <16 x float>*
  store <16 x float> %merge162435, <16 x float>* %CastToValueType1666, align 64
  %merge160436 = select <16 x i1> %_Min118188, <16 x float> %129, <16 x float> %281
  %"&(pSB[currWI].offset)1673" = add nuw i64 %CurrSBIndex..1, 704
  %"&pSB[currWI].offset1674" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1673"
  %CastToValueType1675 = bitcast i8* %"&pSB[currWI].offset1674" to <16 x float>*
  store <16 x float> %merge160436, <16 x float>* %CastToValueType1675, align 64
  %merge158437 = select <16 x i1> %_Min118188, <16 x float> %128, <16 x float> %230
  %"&(pSB[currWI].offset)1682" = add nuw i64 %CurrSBIndex..1, 768
  %"&pSB[currWI].offset1683" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1682"
  %CastToValueType1684 = bitcast i8* %"&pSB[currWI].offset1683" to <16 x float>*
  store <16 x float> %merge158437, <16 x float>* %CastToValueType1684, align 64
  %merge156438 = select <16 x i1> %_Min118188, <16 x float> %temp.vect251, <16 x float> %merge333
  %"&(pSB[currWI].offset)1691" = add nuw i64 %CurrSBIndex..1, 832
  %"&pSB[currWI].offset1692" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1691"
  %CastToValueType1693 = bitcast i8* %"&pSB[currWI].offset1692" to <16 x float>*
  store <16 x float> %merge156438, <16 x float>* %CastToValueType1693, align 64
  %check.WI.iter2201 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter2201, label %thenBB2198, label %SyncBB2175

thenBB2198:                                       ; preds = %postload1125
  %"CurrWI++2202" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride2204" = add nuw i64 %CurrSBIndex..1, 1792
  br label %SyncBB2176

SyncBB2175:                                       ; preds = %postload1125, %thenBB2261
  %CurrSBIndex..9 = phi i64 [ %"loadedCurrSB+Stride2267", %thenBB2261 ], [ 0, %postload1125 ]
  %CurrWI..9 = phi i64 [ %"CurrWI++2265", %thenBB2261 ], [ 0, %postload1125 ]
  %283 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..9, i32 0, i64 0
  %284 = load i64* %283, align 8
  %broadcast1439 = insertelement <16 x i64> undef, i64 %284, i32 0
  %broadcast2440 = shufflevector <16 x i64> %broadcast1439, <16 x i64> undef, <16 x i32> zeroinitializer
  %285 = add <16 x i64> %broadcast2440, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)1700" = add nuw i64 %CurrSBIndex..9, 896
  %"&pSB[currWI].offset1701" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1700"
  %CastToValueType1702 = bitcast i8* %"&pSB[currWI].offset1701" to <16 x i64>*
  store <16 x i64> %285, <16 x i64>* %CastToValueType1702, align 128
  %extract442.lhs.lhs = extractelement <16 x i64> %285, i32 0
  %extract442.lhs = shl i64 %extract442.lhs.lhs, 32
  %extract442 = ashr i64 %extract442.lhs, 32
  %286 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract442
  %ptrTypeCast = bitcast float addrspace(3)* %286 to <16 x float> addrspace(3)*
  store <16 x float> zeroinitializer, <16 x float> addrspace(3)* %ptrTypeCast, align 4
  %check.WI.iter2264 = icmp ult i64 %CurrWI..9, %iterCount
  br i1 %check.WI.iter2264, label %thenBB2261, label %SyncBB2185

thenBB2261:                                       ; preds = %SyncBB2175
  %"CurrWI++2265" = add nuw i64 %CurrWI..9, 1
  %"loadedCurrSB+Stride2267" = add nuw i64 %CurrSBIndex..9, 1792
  br label %SyncBB2175

SyncBB2185:                                       ; preds = %SyncBB2175, %thenBB2219
  %CurrSBIndex..3 = phi i64 [ %"loadedCurrSB+Stride2225", %thenBB2219 ], [ 0, %SyncBB2175 ]
  %CurrWI..3 = phi i64 [ %"CurrWI++2223", %thenBB2219 ], [ 0, %SyncBB2175 ]
  %287 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %288 = load i64* %287, align 8
  %temp458 = insertelement <16 x i64> undef, i64 %288, i32 0
  %vector459 = shufflevector <16 x i64> %temp458, <16 x i64> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1704" = add nuw i64 %CurrSBIndex..3, 896
  %"&pSB[currWI].offset1705" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1704"
  %CastToValueType1706 = bitcast i8* %"&pSB[currWI].offset1705" to <16 x i64>*
  %loadedValue1707 = load <16 x i64>* %CastToValueType1706, align 128
  %289 = add <16 x i64> %vector459, %loadedValue1707
  %290 = trunc <16 x i64> %289 to <16 x i32>
  %"&(pSB[currWI].offset)1709" = add nuw i64 %CurrSBIndex..3, 1024
  %"&pSB[currWI].offset1710" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1709"
  %CastToValueType1711 = bitcast i8* %"&pSB[currWI].offset1710" to <16 x i32>*
  store <16 x i32> %290, <16 x i32>* %CastToValueType1711, align 64
  %291 = extractelement <16 x i32> %290, i32 0
  %extract460 = sext i32 %291 to i64
  %292 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract460
  %"&(pSB[currWI].offset)1753" = add nuw i64 %CurrSBIndex..3, 1088
  %"&pSB[currWI].offset1754" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1753"
  %CastToValueType1755 = bitcast i8* %"&pSB[currWI].offset1754" to float addrspace(3)**
  store float addrspace(3)* %292, float addrspace(3)** %CastToValueType1755, align 8
  %ptrTypeCast476 = bitcast float addrspace(3)* %292 to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)1659" = add nuw i64 %CurrSBIndex..3, 576
  %"&pSB[currWI].offset1660" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1659"
  %CastToValueType1661 = bitcast i8* %"&pSB[currWI].offset1660" to <16 x float>*
  %loadedValue1662 = load <16 x float>* %CastToValueType1661, align 64
  store <16 x float> %loadedValue1662, <16 x float> addrspace(3)* %ptrTypeCast476, align 4
  %check.WI.iter2222 = icmp ult i64 %CurrWI..3, %iterCount
  br i1 %check.WI.iter2222, label %thenBB2219, label %SyncBB2179

thenBB2219:                                       ; preds = %SyncBB2185
  %"CurrWI++2223" = add nuw i64 %CurrWI..3, 1
  %"loadedCurrSB+Stride2225" = add nuw i64 %CurrSBIndex..3, 1792
  br label %SyncBB2185

SyncBB2179:                                       ; preds = %SyncBB2185, %thenBB2268
  %CurrSBIndex..10 = phi i64 [ %"loadedCurrSB+Stride2274", %thenBB2268 ], [ 0, %SyncBB2185 ]
  %CurrWI..10 = phi i64 [ %"CurrWI++2272", %thenBB2268 ], [ 0, %SyncBB2185 ]
  %"&(pSB[currWI].offset)1748" = add nuw i64 %CurrSBIndex..10, 1024
  %"&pSB[currWI].offset1749" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1748"
  %CastToValueType1750 = bitcast i8* %"&pSB[currWI].offset1749" to <16 x i32>*
  %loadedValue1751 = load <16 x i32>* %CastToValueType1750, align 64
  %.lhs = extractelement <16 x i32> %loadedValue1751, i32 0
  %293 = add i32 %.lhs, -1
  %extract477 = sext i32 %293 to i64
  %294 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract477
  %"&(pSB[currWI].offset)1837" = add nuw i64 %CurrSBIndex..10, 1096
  %"&pSB[currWI].offset1838" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1837"
  %CastToValueType1839 = bitcast i8* %"&pSB[currWI].offset1838" to float addrspace(3)**
  store float addrspace(3)* %294, float addrspace(3)** %CastToValueType1839, align 8
  %ptrTypeCast493 = bitcast float addrspace(3)* %294 to <16 x float> addrspace(3)*
  %295 = load <16 x float> addrspace(3)* %ptrTypeCast493, align 4
  %"&(pSB[currWI].offset)1846" = add nuw i64 %CurrSBIndex..10, 1152
  %"&pSB[currWI].offset1847" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1846"
  %CastToValueType1848 = bitcast i8* %"&pSB[currWI].offset1847" to <16 x float>*
  store <16 x float> %295, <16 x float>* %CastToValueType1848, align 64
  %check.WI.iter2271 = icmp ult i64 %CurrWI..10, %iterCount
  br i1 %check.WI.iter2271, label %thenBB2268, label %SyncBB2186

thenBB2268:                                       ; preds = %SyncBB2179
  %"CurrWI++2272" = add nuw i64 %CurrWI..10, 1
  %"loadedCurrSB+Stride2274" = add nuw i64 %CurrSBIndex..10, 1792
  br label %SyncBB2179

SyncBB2186:                                       ; preds = %SyncBB2179, %thenBB2275
  %CurrSBIndex..11 = phi i64 [ %"loadedCurrSB+Stride2281", %thenBB2275 ], [ 0, %SyncBB2179 ]
  %CurrWI..11 = phi i64 [ %"CurrWI++2279", %thenBB2275 ], [ 0, %SyncBB2179 ]
  %"&(pSB[currWI].offset)1832" = add nuw i64 %CurrSBIndex..11, 1088
  %"&pSB[currWI].offset1833" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1832"
  %CastToValueType1834 = bitcast i8* %"&pSB[currWI].offset1833" to float addrspace(3)**
  %loadedValue1835 = load float addrspace(3)** %CastToValueType1834, align 8
  %ptrTypeCast494 = bitcast float addrspace(3)* %loadedValue1835 to <16 x float> addrspace(3)*
  %296 = load <16 x float> addrspace(3)* %ptrTypeCast494, align 4
  %"&(pSB[currWI].offset)1850" = add nuw i64 %CurrSBIndex..11, 1152
  %"&pSB[currWI].offset1851" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1850"
  %CastToValueType1852 = bitcast i8* %"&pSB[currWI].offset1851" to <16 x float>*
  %loadedValue1853 = load <16 x float>* %CastToValueType1852, align 64
  %297 = fadd <16 x float> %296, %loadedValue1853
  %"&(pSB[currWI].offset)1827" = add nuw i64 %CurrSBIndex..11, 1088
  %"&pSB[currWI].offset1828" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1827"
  %CastToValueType1829 = bitcast i8* %"&pSB[currWI].offset1828" to float addrspace(3)**
  %loadedValue1830 = load float addrspace(3)** %CastToValueType1829, align 8
  %ptrTypeCast495 = bitcast float addrspace(3)* %loadedValue1830 to <16 x float> addrspace(3)*
  store <16 x float> %297, <16 x float> addrspace(3)* %ptrTypeCast495, align 4
  %check.WI.iter2278 = icmp ult i64 %CurrWI..11, %iterCount
  br i1 %check.WI.iter2278, label %thenBB2275, label %SyncBB2187

thenBB2275:                                       ; preds = %SyncBB2186
  %"CurrWI++2279" = add nuw i64 %CurrWI..11, 1
  %"loadedCurrSB+Stride2281" = add nuw i64 %CurrSBIndex..11, 1792
  br label %SyncBB2186

SyncBB2187:                                       ; preds = %SyncBB2186, %thenBB2282
  %CurrSBIndex..12 = phi i64 [ %"loadedCurrSB+Stride2288", %thenBB2282 ], [ 0, %SyncBB2186 ]
  %CurrWI..12 = phi i64 [ %"CurrWI++2286", %thenBB2282 ], [ 0, %SyncBB2186 ]
  %"&(pSB[currWI].offset)1743" = add nuw i64 %CurrSBIndex..12, 1024
  %"&pSB[currWI].offset1744" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1743"
  %CastToValueType1745 = bitcast i8* %"&pSB[currWI].offset1744" to <16 x i32>*
  %loadedValue1746 = load <16 x i32>* %CastToValueType1745, align 64
  %.lhs1335 = extractelement <16 x i32> %loadedValue1746, i32 0
  %298 = add i32 %.lhs1335, -2
  %extract496 = sext i32 %298 to i64
  %299 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract496
  %ptrTypeCast512 = bitcast float addrspace(3)* %299 to <16 x float> addrspace(3)*
  %300 = load <16 x float> addrspace(3)* %ptrTypeCast512, align 4
  %"&(pSB[currWI].offset)1855" = add nuw i64 %CurrSBIndex..12, 1216
  %"&pSB[currWI].offset1856" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1855"
  %CastToValueType1857 = bitcast i8* %"&pSB[currWI].offset1856" to <16 x float>*
  store <16 x float> %300, <16 x float>* %CastToValueType1857, align 64
  %check.WI.iter2285 = icmp ult i64 %CurrWI..12, %iterCount
  br i1 %check.WI.iter2285, label %thenBB2282, label %SyncBB2188

thenBB2282:                                       ; preds = %SyncBB2187
  %"CurrWI++2286" = add nuw i64 %CurrWI..12, 1
  %"loadedCurrSB+Stride2288" = add nuw i64 %CurrSBIndex..12, 1792
  br label %SyncBB2187

SyncBB2188:                                       ; preds = %SyncBB2187, %thenBB2289
  %CurrSBIndex..13 = phi i64 [ %"loadedCurrSB+Stride2295", %thenBB2289 ], [ 0, %SyncBB2187 ]
  %CurrWI..13 = phi i64 [ %"CurrWI++2293", %thenBB2289 ], [ 0, %SyncBB2187 ]
  %"&(pSB[currWI].offset)1822" = add nuw i64 %CurrSBIndex..13, 1088
  %"&pSB[currWI].offset1823" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1822"
  %CastToValueType1824 = bitcast i8* %"&pSB[currWI].offset1823" to float addrspace(3)**
  %loadedValue1825 = load float addrspace(3)** %CastToValueType1824, align 8
  %ptrTypeCast513 = bitcast float addrspace(3)* %loadedValue1825 to <16 x float> addrspace(3)*
  %301 = load <16 x float> addrspace(3)* %ptrTypeCast513, align 4
  %"&(pSB[currWI].offset)1859" = add nuw i64 %CurrSBIndex..13, 1216
  %"&pSB[currWI].offset1860" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1859"
  %CastToValueType1861 = bitcast i8* %"&pSB[currWI].offset1860" to <16 x float>*
  %loadedValue1862 = load <16 x float>* %CastToValueType1861, align 64
  %302 = fadd <16 x float> %301, %loadedValue1862
  %"&(pSB[currWI].offset)1817" = add nuw i64 %CurrSBIndex..13, 1088
  %"&pSB[currWI].offset1818" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1817"
  %CastToValueType1819 = bitcast i8* %"&pSB[currWI].offset1818" to float addrspace(3)**
  %loadedValue1820 = load float addrspace(3)** %CastToValueType1819, align 8
  %ptrTypeCast514 = bitcast float addrspace(3)* %loadedValue1820 to <16 x float> addrspace(3)*
  store <16 x float> %302, <16 x float> addrspace(3)* %ptrTypeCast514, align 4
  %check.WI.iter2292 = icmp ult i64 %CurrWI..13, %iterCount
  br i1 %check.WI.iter2292, label %thenBB2289, label %SyncBB2189

thenBB2289:                                       ; preds = %SyncBB2188
  %"CurrWI++2293" = add nuw i64 %CurrWI..13, 1
  %"loadedCurrSB+Stride2295" = add nuw i64 %CurrSBIndex..13, 1792
  br label %SyncBB2188

SyncBB2189:                                       ; preds = %SyncBB2188, %thenBB2296
  %CurrSBIndex..14 = phi i64 [ %"loadedCurrSB+Stride2302", %thenBB2296 ], [ 0, %SyncBB2188 ]
  %CurrWI..14 = phi i64 [ %"CurrWI++2300", %thenBB2296 ], [ 0, %SyncBB2188 ]
  %"&(pSB[currWI].offset)1738" = add nuw i64 %CurrSBIndex..14, 1024
  %"&pSB[currWI].offset1739" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1738"
  %CastToValueType1740 = bitcast i8* %"&pSB[currWI].offset1739" to <16 x i32>*
  %loadedValue1741 = load <16 x i32>* %CastToValueType1740, align 64
  %.lhs1336 = extractelement <16 x i32> %loadedValue1741, i32 0
  %303 = add i32 %.lhs1336, -4
  %extract515 = sext i32 %303 to i64
  %304 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract515
  %ptrTypeCast531 = bitcast float addrspace(3)* %304 to <16 x float> addrspace(3)*
  %305 = load <16 x float> addrspace(3)* %ptrTypeCast531, align 4
  %"&(pSB[currWI].offset)1864" = add nuw i64 %CurrSBIndex..14, 1280
  %"&pSB[currWI].offset1865" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1864"
  %CastToValueType1866 = bitcast i8* %"&pSB[currWI].offset1865" to <16 x float>*
  store <16 x float> %305, <16 x float>* %CastToValueType1866, align 64
  %check.WI.iter2299 = icmp ult i64 %CurrWI..14, %iterCount
  br i1 %check.WI.iter2299, label %thenBB2296, label %SyncBB2190

thenBB2296:                                       ; preds = %SyncBB2189
  %"CurrWI++2300" = add nuw i64 %CurrWI..14, 1
  %"loadedCurrSB+Stride2302" = add nuw i64 %CurrSBIndex..14, 1792
  br label %SyncBB2189

SyncBB2190:                                       ; preds = %SyncBB2189, %thenBB2226
  %CurrSBIndex..4 = phi i64 [ %"loadedCurrSB+Stride2232", %thenBB2226 ], [ 0, %SyncBB2189 ]
  %CurrWI..4 = phi i64 [ %"CurrWI++2230", %thenBB2226 ], [ 0, %SyncBB2189 ]
  %"&(pSB[currWI].offset)1812" = add nuw i64 %CurrSBIndex..4, 1088
  %"&pSB[currWI].offset1813" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1812"
  %CastToValueType1814 = bitcast i8* %"&pSB[currWI].offset1813" to float addrspace(3)**
  %loadedValue1815 = load float addrspace(3)** %CastToValueType1814, align 8
  %ptrTypeCast532 = bitcast float addrspace(3)* %loadedValue1815 to <16 x float> addrspace(3)*
  %306 = load <16 x float> addrspace(3)* %ptrTypeCast532, align 4
  %"&(pSB[currWI].offset)1868" = add nuw i64 %CurrSBIndex..4, 1280
  %"&pSB[currWI].offset1869" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1868"
  %CastToValueType1870 = bitcast i8* %"&pSB[currWI].offset1869" to <16 x float>*
  %loadedValue1871 = load <16 x float>* %CastToValueType1870, align 64
  %307 = fadd <16 x float> %306, %loadedValue1871
  %"&(pSB[currWI].offset)1807" = add nuw i64 %CurrSBIndex..4, 1088
  %"&pSB[currWI].offset1808" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1807"
  %CastToValueType1809 = bitcast i8* %"&pSB[currWI].offset1808" to float addrspace(3)**
  %loadedValue1810 = load float addrspace(3)** %CastToValueType1809, align 8
  %ptrTypeCast533 = bitcast float addrspace(3)* %loadedValue1810 to <16 x float> addrspace(3)*
  store <16 x float> %307, <16 x float> addrspace(3)* %ptrTypeCast533, align 4
  %check.WI.iter2229 = icmp ult i64 %CurrWI..4, %iterCount
  br i1 %check.WI.iter2229, label %thenBB2226, label %SyncBB2180

thenBB2226:                                       ; preds = %SyncBB2190
  %"CurrWI++2230" = add nuw i64 %CurrWI..4, 1
  %"loadedCurrSB+Stride2232" = add nuw i64 %CurrSBIndex..4, 1792
  br label %SyncBB2190

SyncBB2180:                                       ; preds = %SyncBB2190, %thenBB2233
  %CurrSBIndex..5 = phi i64 [ %"loadedCurrSB+Stride2239", %thenBB2233 ], [ 0, %SyncBB2190 ]
  %CurrWI..5 = phi i64 [ %"CurrWI++2237", %thenBB2233 ], [ 0, %SyncBB2190 ]
  %"&(pSB[currWI].offset)1733" = add nuw i64 %CurrSBIndex..5, 1024
  %"&pSB[currWI].offset1734" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1733"
  %CastToValueType1735 = bitcast i8* %"&pSB[currWI].offset1734" to <16 x i32>*
  %loadedValue1736 = load <16 x i32>* %CastToValueType1735, align 64
  %.lhs1337 = extractelement <16 x i32> %loadedValue1736, i32 0
  %308 = add i32 %.lhs1337, -8
  %extract534 = sext i32 %308 to i64
  %309 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract534
  %ptrTypeCast550 = bitcast float addrspace(3)* %309 to <16 x float> addrspace(3)*
  %310 = load <16 x float> addrspace(3)* %ptrTypeCast550, align 4
  %"&(pSB[currWI].offset)1873" = add nuw i64 %CurrSBIndex..5, 1344
  %"&pSB[currWI].offset1874" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1873"
  %CastToValueType1875 = bitcast i8* %"&pSB[currWI].offset1874" to <16 x float>*
  store <16 x float> %310, <16 x float>* %CastToValueType1875, align 64
  %check.WI.iter2236 = icmp ult i64 %CurrWI..5, %iterCount
  br i1 %check.WI.iter2236, label %thenBB2233, label %SyncBB2181

thenBB2233:                                       ; preds = %SyncBB2180
  %"CurrWI++2237" = add nuw i64 %CurrWI..5, 1
  %"loadedCurrSB+Stride2239" = add nuw i64 %CurrSBIndex..5, 1792
  br label %SyncBB2180

SyncBB2181:                                       ; preds = %SyncBB2180, %thenBB2240
  %CurrSBIndex..6 = phi i64 [ %"loadedCurrSB+Stride2246", %thenBB2240 ], [ 0, %SyncBB2180 ]
  %CurrWI..6 = phi i64 [ %"CurrWI++2244", %thenBB2240 ], [ 0, %SyncBB2180 ]
  %"&(pSB[currWI].offset)1802" = add nuw i64 %CurrSBIndex..6, 1088
  %"&pSB[currWI].offset1803" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1802"
  %CastToValueType1804 = bitcast i8* %"&pSB[currWI].offset1803" to float addrspace(3)**
  %loadedValue1805 = load float addrspace(3)** %CastToValueType1804, align 8
  %ptrTypeCast551 = bitcast float addrspace(3)* %loadedValue1805 to <16 x float> addrspace(3)*
  %311 = load <16 x float> addrspace(3)* %ptrTypeCast551, align 4
  %"&(pSB[currWI].offset)1877" = add nuw i64 %CurrSBIndex..6, 1344
  %"&pSB[currWI].offset1878" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1877"
  %CastToValueType1879 = bitcast i8* %"&pSB[currWI].offset1878" to <16 x float>*
  %loadedValue1880 = load <16 x float>* %CastToValueType1879, align 64
  %312 = fadd <16 x float> %311, %loadedValue1880
  %"&(pSB[currWI].offset)1797" = add nuw i64 %CurrSBIndex..6, 1088
  %"&pSB[currWI].offset1798" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1797"
  %CastToValueType1799 = bitcast i8* %"&pSB[currWI].offset1798" to float addrspace(3)**
  %loadedValue1800 = load float addrspace(3)** %CastToValueType1799, align 8
  %ptrTypeCast552 = bitcast float addrspace(3)* %loadedValue1800 to <16 x float> addrspace(3)*
  store <16 x float> %312, <16 x float> addrspace(3)* %ptrTypeCast552, align 4
  %check.WI.iter2243 = icmp ult i64 %CurrWI..6, %iterCount
  br i1 %check.WI.iter2243, label %thenBB2240, label %SyncBB2182

thenBB2240:                                       ; preds = %SyncBB2181
  %"CurrWI++2244" = add nuw i64 %CurrWI..6, 1
  %"loadedCurrSB+Stride2246" = add nuw i64 %CurrSBIndex..6, 1792
  br label %SyncBB2181

SyncBB2182:                                       ; preds = %SyncBB2181, %thenBB2247
  %CurrSBIndex..7 = phi i64 [ %"loadedCurrSB+Stride2253", %thenBB2247 ], [ 0, %SyncBB2181 ]
  %CurrWI..7 = phi i64 [ %"CurrWI++2251", %thenBB2247 ], [ 0, %SyncBB2181 ]
  %"&(pSB[currWI].offset)1728" = add nuw i64 %CurrSBIndex..7, 1024
  %"&pSB[currWI].offset1729" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1728"
  %CastToValueType1730 = bitcast i8* %"&pSB[currWI].offset1729" to <16 x i32>*
  %loadedValue1731 = load <16 x i32>* %CastToValueType1730, align 64
  %.lhs1338 = extractelement <16 x i32> %loadedValue1731, i32 0
  %313 = add i32 %.lhs1338, -16
  %extract553 = sext i32 %313 to i64
  %314 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract553
  %ptrTypeCast569 = bitcast float addrspace(3)* %314 to <16 x float> addrspace(3)*
  %315 = load <16 x float> addrspace(3)* %ptrTypeCast569, align 4
  %"&(pSB[currWI].offset)1882" = add nuw i64 %CurrSBIndex..7, 1408
  %"&pSB[currWI].offset1883" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1882"
  %CastToValueType1884 = bitcast i8* %"&pSB[currWI].offset1883" to <16 x float>*
  store <16 x float> %315, <16 x float>* %CastToValueType1884, align 64
  %check.WI.iter2250 = icmp ult i64 %CurrWI..7, %iterCount
  br i1 %check.WI.iter2250, label %thenBB2247, label %SyncBB2183

thenBB2247:                                       ; preds = %SyncBB2182
  %"CurrWI++2251" = add nuw i64 %CurrWI..7, 1
  %"loadedCurrSB+Stride2253" = add nuw i64 %CurrSBIndex..7, 1792
  br label %SyncBB2182

SyncBB2183:                                       ; preds = %SyncBB2182, %thenBB2254
  %CurrSBIndex..8 = phi i64 [ %"loadedCurrSB+Stride2260", %thenBB2254 ], [ 0, %SyncBB2182 ]
  %CurrWI..8 = phi i64 [ %"CurrWI++2258", %thenBB2254 ], [ 0, %SyncBB2182 ]
  %"&(pSB[currWI].offset)1792" = add nuw i64 %CurrSBIndex..8, 1088
  %"&pSB[currWI].offset1793" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1792"
  %CastToValueType1794 = bitcast i8* %"&pSB[currWI].offset1793" to float addrspace(3)**
  %loadedValue1795 = load float addrspace(3)** %CastToValueType1794, align 8
  %ptrTypeCast570 = bitcast float addrspace(3)* %loadedValue1795 to <16 x float> addrspace(3)*
  %316 = load <16 x float> addrspace(3)* %ptrTypeCast570, align 4
  %"&(pSB[currWI].offset)1886" = add nuw i64 %CurrSBIndex..8, 1408
  %"&pSB[currWI].offset1887" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1886"
  %CastToValueType1888 = bitcast i8* %"&pSB[currWI].offset1887" to <16 x float>*
  %loadedValue1889 = load <16 x float>* %CastToValueType1888, align 64
  %317 = fadd <16 x float> %316, %loadedValue1889
  %"&(pSB[currWI].offset)1787" = add nuw i64 %CurrSBIndex..8, 1088
  %"&pSB[currWI].offset1788" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1787"
  %CastToValueType1789 = bitcast i8* %"&pSB[currWI].offset1788" to float addrspace(3)**
  %loadedValue1790 = load float addrspace(3)** %CastToValueType1789, align 8
  %ptrTypeCast571 = bitcast float addrspace(3)* %loadedValue1790 to <16 x float> addrspace(3)*
  store <16 x float> %317, <16 x float> addrspace(3)* %ptrTypeCast571, align 4
  %check.WI.iter2257 = icmp ult i64 %CurrWI..8, %iterCount
  br i1 %check.WI.iter2257, label %thenBB2254, label %SyncBB2184

thenBB2254:                                       ; preds = %SyncBB2183
  %"CurrWI++2258" = add nuw i64 %CurrWI..8, 1
  %"loadedCurrSB+Stride2260" = add nuw i64 %CurrSBIndex..8, 1792
  br label %SyncBB2183

SyncBB2184:                                       ; preds = %SyncBB2183, %thenBB2303
  %CurrSBIndex..15 = phi i64 [ %"loadedCurrSB+Stride2309", %thenBB2303 ], [ 0, %SyncBB2183 ]
  %CurrWI..15 = phi i64 [ %"CurrWI++2307", %thenBB2303 ], [ 0, %SyncBB2183 ]
  %"&(pSB[currWI].offset)1723" = add nuw i64 %CurrSBIndex..15, 1024
  %"&pSB[currWI].offset1724" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1723"
  %CastToValueType1725 = bitcast i8* %"&pSB[currWI].offset1724" to <16 x i32>*
  %loadedValue1726 = load <16 x i32>* %CastToValueType1725, align 64
  %.lhs1339 = extractelement <16 x i32> %loadedValue1726, i32 0
  %318 = add i32 %.lhs1339, -32
  %extract572 = sext i32 %318 to i64
  %319 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract572
  %ptrTypeCast588 = bitcast float addrspace(3)* %319 to <16 x float> addrspace(3)*
  %320 = load <16 x float> addrspace(3)* %ptrTypeCast588, align 4
  %"&(pSB[currWI].offset)1891" = add nuw i64 %CurrSBIndex..15, 1472
  %"&pSB[currWI].offset1892" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1891"
  %CastToValueType1893 = bitcast i8* %"&pSB[currWI].offset1892" to <16 x float>*
  store <16 x float> %320, <16 x float>* %CastToValueType1893, align 64
  %check.WI.iter2306 = icmp ult i64 %CurrWI..15, %iterCount
  br i1 %check.WI.iter2306, label %thenBB2303, label %SyncBB2191

thenBB2303:                                       ; preds = %SyncBB2184
  %"CurrWI++2307" = add nuw i64 %CurrWI..15, 1
  %"loadedCurrSB+Stride2309" = add nuw i64 %CurrSBIndex..15, 1792
  br label %SyncBB2184

SyncBB2191:                                       ; preds = %SyncBB2184, %thenBB2310
  %CurrSBIndex..16 = phi i64 [ %"loadedCurrSB+Stride2316", %thenBB2310 ], [ 0, %SyncBB2184 ]
  %CurrWI..16 = phi i64 [ %"CurrWI++2314", %thenBB2310 ], [ 0, %SyncBB2184 ]
  %"&(pSB[currWI].offset)1782" = add nuw i64 %CurrSBIndex..16, 1088
  %"&pSB[currWI].offset1783" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1782"
  %CastToValueType1784 = bitcast i8* %"&pSB[currWI].offset1783" to float addrspace(3)**
  %loadedValue1785 = load float addrspace(3)** %CastToValueType1784, align 8
  %ptrTypeCast589 = bitcast float addrspace(3)* %loadedValue1785 to <16 x float> addrspace(3)*
  %321 = load <16 x float> addrspace(3)* %ptrTypeCast589, align 4
  %"&(pSB[currWI].offset)1895" = add nuw i64 %CurrSBIndex..16, 1472
  %"&pSB[currWI].offset1896" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1895"
  %CastToValueType1897 = bitcast i8* %"&pSB[currWI].offset1896" to <16 x float>*
  %loadedValue1898 = load <16 x float>* %CastToValueType1897, align 64
  %322 = fadd <16 x float> %321, %loadedValue1898
  %"&(pSB[currWI].offset)1777" = add nuw i64 %CurrSBIndex..16, 1088
  %"&pSB[currWI].offset1778" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1777"
  %CastToValueType1779 = bitcast i8* %"&pSB[currWI].offset1778" to float addrspace(3)**
  %loadedValue1780 = load float addrspace(3)** %CastToValueType1779, align 8
  %ptrTypeCast590 = bitcast float addrspace(3)* %loadedValue1780 to <16 x float> addrspace(3)*
  store <16 x float> %322, <16 x float> addrspace(3)* %ptrTypeCast590, align 4
  %check.WI.iter2313 = icmp ult i64 %CurrWI..16, %iterCount
  br i1 %check.WI.iter2313, label %thenBB2310, label %SyncBB2192

thenBB2310:                                       ; preds = %SyncBB2191
  %"CurrWI++2314" = add nuw i64 %CurrWI..16, 1
  %"loadedCurrSB+Stride2316" = add nuw i64 %CurrSBIndex..16, 1792
  br label %SyncBB2191

SyncBB2192:                                       ; preds = %SyncBB2191, %thenBB2317
  %CurrSBIndex..17 = phi i64 [ %"loadedCurrSB+Stride2323", %thenBB2317 ], [ 0, %SyncBB2191 ]
  %CurrWI..17 = phi i64 [ %"CurrWI++2321", %thenBB2317 ], [ 0, %SyncBB2191 ]
  %"&(pSB[currWI].offset)1718" = add nuw i64 %CurrSBIndex..17, 1024
  %"&pSB[currWI].offset1719" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1718"
  %CastToValueType1720 = bitcast i8* %"&pSB[currWI].offset1719" to <16 x i32>*
  %loadedValue1721 = load <16 x i32>* %CastToValueType1720, align 64
  %.lhs1340 = extractelement <16 x i32> %loadedValue1721, i32 0
  %323 = add i32 %.lhs1340, -64
  %extract591 = sext i32 %323 to i64
  %324 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract591
  %ptrTypeCast607 = bitcast float addrspace(3)* %324 to <16 x float> addrspace(3)*
  %325 = load <16 x float> addrspace(3)* %ptrTypeCast607, align 4
  %"&(pSB[currWI].offset)1900" = add nuw i64 %CurrSBIndex..17, 1536
  %"&pSB[currWI].offset1901" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1900"
  %CastToValueType1902 = bitcast i8* %"&pSB[currWI].offset1901" to <16 x float>*
  store <16 x float> %325, <16 x float>* %CastToValueType1902, align 64
  %check.WI.iter2320 = icmp ult i64 %CurrWI..17, %iterCount
  br i1 %check.WI.iter2320, label %thenBB2317, label %SyncBB2193

thenBB2317:                                       ; preds = %SyncBB2192
  %"CurrWI++2321" = add nuw i64 %CurrWI..17, 1
  %"loadedCurrSB+Stride2323" = add nuw i64 %CurrSBIndex..17, 1792
  br label %SyncBB2192

SyncBB2193:                                       ; preds = %SyncBB2192, %thenBB2324
  %CurrSBIndex..18 = phi i64 [ %"loadedCurrSB+Stride2330", %thenBB2324 ], [ 0, %SyncBB2192 ]
  %CurrWI..18 = phi i64 [ %"CurrWI++2328", %thenBB2324 ], [ 0, %SyncBB2192 ]
  %"&(pSB[currWI].offset)1772" = add nuw i64 %CurrSBIndex..18, 1088
  %"&pSB[currWI].offset1773" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1772"
  %CastToValueType1774 = bitcast i8* %"&pSB[currWI].offset1773" to float addrspace(3)**
  %loadedValue1775 = load float addrspace(3)** %CastToValueType1774, align 8
  %ptrTypeCast608 = bitcast float addrspace(3)* %loadedValue1775 to <16 x float> addrspace(3)*
  %326 = load <16 x float> addrspace(3)* %ptrTypeCast608, align 4
  %"&(pSB[currWI].offset)1904" = add nuw i64 %CurrSBIndex..18, 1536
  %"&pSB[currWI].offset1905" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1904"
  %CastToValueType1906 = bitcast i8* %"&pSB[currWI].offset1905" to <16 x float>*
  %loadedValue1907 = load <16 x float>* %CastToValueType1906, align 64
  %327 = fadd <16 x float> %326, %loadedValue1907
  %"&(pSB[currWI].offset)1767" = add nuw i64 %CurrSBIndex..18, 1088
  %"&pSB[currWI].offset1768" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1767"
  %CastToValueType1769 = bitcast i8* %"&pSB[currWI].offset1768" to float addrspace(3)**
  %loadedValue1770 = load float addrspace(3)** %CastToValueType1769, align 8
  %ptrTypeCast609 = bitcast float addrspace(3)* %loadedValue1770 to <16 x float> addrspace(3)*
  store <16 x float> %327, <16 x float> addrspace(3)* %ptrTypeCast609, align 4
  %check.WI.iter2327 = icmp ult i64 %CurrWI..18, %iterCount
  br i1 %check.WI.iter2327, label %thenBB2324, label %SyncBB2194

thenBB2324:                                       ; preds = %SyncBB2193
  %"CurrWI++2328" = add nuw i64 %CurrWI..18, 1
  %"loadedCurrSB+Stride2330" = add nuw i64 %CurrSBIndex..18, 1792
  br label %SyncBB2193

SyncBB2194:                                       ; preds = %SyncBB2193, %thenBB2331
  %CurrSBIndex..19 = phi i64 [ %"loadedCurrSB+Stride2337", %thenBB2331 ], [ 0, %SyncBB2193 ]
  %CurrWI..19 = phi i64 [ %"CurrWI++2335", %thenBB2331 ], [ 0, %SyncBB2193 ]
  %"&(pSB[currWI].offset)1713" = add nuw i64 %CurrSBIndex..19, 1024
  %"&pSB[currWI].offset1714" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1713"
  %CastToValueType1715 = bitcast i8* %"&pSB[currWI].offset1714" to <16 x i32>*
  %loadedValue1716 = load <16 x i32>* %CastToValueType1715, align 64
  %.lhs1341 = extractelement <16 x i32> %loadedValue1716, i32 0
  %328 = add i32 %.lhs1341, -128
  %extract610 = sext i32 %328 to i64
  %329 = getelementptr inbounds [512 x float] addrspace(3)* %0, i64 0, i64 %extract610
  %ptrTypeCast626 = bitcast float addrspace(3)* %329 to <16 x float> addrspace(3)*
  %330 = load <16 x float> addrspace(3)* %ptrTypeCast626, align 4
  %"&(pSB[currWI].offset)1909" = add nuw i64 %CurrSBIndex..19, 1600
  %"&pSB[currWI].offset1910" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1909"
  %CastToValueType1911 = bitcast i8* %"&pSB[currWI].offset1910" to <16 x float>*
  store <16 x float> %330, <16 x float>* %CastToValueType1911, align 64
  %check.WI.iter2334 = icmp ult i64 %CurrWI..19, %iterCount
  br i1 %check.WI.iter2334, label %thenBB2331, label %SyncBB2195

thenBB2331:                                       ; preds = %SyncBB2194
  %"CurrWI++2335" = add nuw i64 %CurrWI..19, 1
  %"loadedCurrSB+Stride2337" = add nuw i64 %CurrSBIndex..19, 1792
  br label %SyncBB2194

SyncBB2195:                                       ; preds = %SyncBB2194, %thenBB2338
  %CurrSBIndex..20 = phi i64 [ %"loadedCurrSB+Stride2344", %thenBB2338 ], [ 0, %SyncBB2194 ]
  %CurrWI..20 = phi i64 [ %"CurrWI++2342", %thenBB2338 ], [ 0, %SyncBB2194 ]
  %"&(pSB[currWI].offset)1762" = add nuw i64 %CurrSBIndex..20, 1088
  %"&pSB[currWI].offset1763" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1762"
  %CastToValueType1764 = bitcast i8* %"&pSB[currWI].offset1763" to float addrspace(3)**
  %loadedValue1765 = load float addrspace(3)** %CastToValueType1764, align 8
  %ptrTypeCast627 = bitcast float addrspace(3)* %loadedValue1765 to <16 x float> addrspace(3)*
  %331 = load <16 x float> addrspace(3)* %ptrTypeCast627, align 4
  %"&(pSB[currWI].offset)1913" = add nuw i64 %CurrSBIndex..20, 1600
  %"&pSB[currWI].offset1914" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1913"
  %CastToValueType1915 = bitcast i8* %"&pSB[currWI].offset1914" to <16 x float>*
  %loadedValue1916 = load <16 x float>* %CastToValueType1915, align 64
  %332 = fadd <16 x float> %331, %loadedValue1916
  %"&(pSB[currWI].offset)1757" = add nuw i64 %CurrSBIndex..20, 1088
  %"&pSB[currWI].offset1758" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1757"
  %CastToValueType1759 = bitcast i8* %"&pSB[currWI].offset1758" to float addrspace(3)**
  %loadedValue1760 = load float addrspace(3)** %CastToValueType1759, align 8
  %ptrTypeCast628 = bitcast float addrspace(3)* %loadedValue1760 to <16 x float> addrspace(3)*
  store <16 x float> %332, <16 x float> addrspace(3)* %ptrTypeCast628, align 4
  %check.WI.iter2341 = icmp ult i64 %CurrWI..20, %iterCount
  br i1 %check.WI.iter2341, label %thenBB2338, label %SyncBB2196

thenBB2338:                                       ; preds = %SyncBB2195
  %"CurrWI++2342" = add nuw i64 %CurrWI..20, 1
  %"loadedCurrSB+Stride2344" = add nuw i64 %CurrSBIndex..20, 1792
  br label %SyncBB2195

SyncBB2196:                                       ; preds = %SyncBB2195, %thenBB2205
  %CurrSBIndex..2 = phi i64 [ %"loadedCurrSB+Stride2211", %thenBB2205 ], [ 0, %SyncBB2195 ]
  %CurrWI..2 = phi i64 [ %"CurrWI++2209", %thenBB2205 ], [ 0, %SyncBB2195 ]
  %"&(pSB[currWI].offset)1841" = add nuw i64 %CurrSBIndex..2, 1096
  %"&pSB[currWI].offset1842" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1841"
  %CastToValueType1843 = bitcast i8* %"&pSB[currWI].offset1842" to float addrspace(3)**
  %loadedValue1844 = load float addrspace(3)** %CastToValueType1843, align 8
  %ptrTypeCast629 = bitcast float addrspace(3)* %loadedValue1844 to <16 x float> addrspace(3)*
  %333 = load <16 x float> addrspace(3)* %ptrTypeCast629, align 4
  %"&(pSB[currWI].offset)1918" = add nuw i64 %CurrSBIndex..2, 1664
  %"&pSB[currWI].offset1919" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1918"
  %CastToValueType1920 = bitcast i8* %"&pSB[currWI].offset1919" to <16 x float>*
  store <16 x float> %333, <16 x float>* %CastToValueType1920, align 64
  %extract668 = extractelement <16 x float> %333, i32 0
  %"&(pSB[currWI].offset)1942" = add nuw i64 %CurrSBIndex..2, 1728
  %"&pSB[currWI].offset1943" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1942"
  %CastToValueType1944 = bitcast i8* %"&pSB[currWI].offset1943" to float*
  store float %extract668, float* %CastToValueType1944, align 4
  %extract669 = extractelement <16 x float> %333, i32 1
  %"&(pSB[currWI].offset)1956" = add nuw i64 %CurrSBIndex..2, 1732
  %"&pSB[currWI].offset1957" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1956"
  %CastToValueType1958 = bitcast i8* %"&pSB[currWI].offset1957" to float*
  store float %extract669, float* %CastToValueType1958, align 4
  %extract670 = extractelement <16 x float> %333, i32 2
  %"&(pSB[currWI].offset)1970" = add nuw i64 %CurrSBIndex..2, 1736
  %"&pSB[currWI].offset1971" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1970"
  %CastToValueType1972 = bitcast i8* %"&pSB[currWI].offset1971" to float*
  store float %extract670, float* %CastToValueType1972, align 4
  %extract671 = extractelement <16 x float> %333, i32 3
  %"&(pSB[currWI].offset)1984" = add nuw i64 %CurrSBIndex..2, 1740
  %"&pSB[currWI].offset1985" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1984"
  %CastToValueType1986 = bitcast i8* %"&pSB[currWI].offset1985" to float*
  store float %extract671, float* %CastToValueType1986, align 4
  %extract672 = extractelement <16 x float> %333, i32 4
  %"&(pSB[currWI].offset)1998" = add nuw i64 %CurrSBIndex..2, 1744
  %"&pSB[currWI].offset1999" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1998"
  %CastToValueType2000 = bitcast i8* %"&pSB[currWI].offset1999" to float*
  store float %extract672, float* %CastToValueType2000, align 4
  %extract673 = extractelement <16 x float> %333, i32 5
  %"&(pSB[currWI].offset)2012" = add nuw i64 %CurrSBIndex..2, 1748
  %"&pSB[currWI].offset2013" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2012"
  %CastToValueType2014 = bitcast i8* %"&pSB[currWI].offset2013" to float*
  store float %extract673, float* %CastToValueType2014, align 4
  %extract674 = extractelement <16 x float> %333, i32 6
  %"&(pSB[currWI].offset)2026" = add nuw i64 %CurrSBIndex..2, 1752
  %"&pSB[currWI].offset2027" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2026"
  %CastToValueType2028 = bitcast i8* %"&pSB[currWI].offset2027" to float*
  store float %extract674, float* %CastToValueType2028, align 4
  %extract675 = extractelement <16 x float> %333, i32 7
  %"&(pSB[currWI].offset)2040" = add nuw i64 %CurrSBIndex..2, 1756
  %"&pSB[currWI].offset2041" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2040"
  %CastToValueType2042 = bitcast i8* %"&pSB[currWI].offset2041" to float*
  store float %extract675, float* %CastToValueType2042, align 4
  %extract676 = extractelement <16 x float> %333, i32 8
  %"&(pSB[currWI].offset)2054" = add nuw i64 %CurrSBIndex..2, 1760
  %"&pSB[currWI].offset2055" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2054"
  %CastToValueType2056 = bitcast i8* %"&pSB[currWI].offset2055" to float*
  store float %extract676, float* %CastToValueType2056, align 4
  %extract677 = extractelement <16 x float> %333, i32 9
  %"&(pSB[currWI].offset)2068" = add nuw i64 %CurrSBIndex..2, 1764
  %"&pSB[currWI].offset2069" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2068"
  %CastToValueType2070 = bitcast i8* %"&pSB[currWI].offset2069" to float*
  store float %extract677, float* %CastToValueType2070, align 4
  %extract678 = extractelement <16 x float> %333, i32 10
  %"&(pSB[currWI].offset)2082" = add nuw i64 %CurrSBIndex..2, 1768
  %"&pSB[currWI].offset2083" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2082"
  %CastToValueType2084 = bitcast i8* %"&pSB[currWI].offset2083" to float*
  store float %extract678, float* %CastToValueType2084, align 4
  %extract679 = extractelement <16 x float> %333, i32 11
  %"&(pSB[currWI].offset)2096" = add nuw i64 %CurrSBIndex..2, 1772
  %"&pSB[currWI].offset2097" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2096"
  %CastToValueType2098 = bitcast i8* %"&pSB[currWI].offset2097" to float*
  store float %extract679, float* %CastToValueType2098, align 4
  %extract680 = extractelement <16 x float> %333, i32 12
  %"&(pSB[currWI].offset)2110" = add nuw i64 %CurrSBIndex..2, 1776
  %"&pSB[currWI].offset2111" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2110"
  %CastToValueType2112 = bitcast i8* %"&pSB[currWI].offset2111" to float*
  store float %extract680, float* %CastToValueType2112, align 4
  %extract681 = extractelement <16 x float> %333, i32 13
  %"&(pSB[currWI].offset)2124" = add nuw i64 %CurrSBIndex..2, 1780
  %"&pSB[currWI].offset2125" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2124"
  %CastToValueType2126 = bitcast i8* %"&pSB[currWI].offset2125" to float*
  store float %extract681, float* %CastToValueType2126, align 4
  %extract682 = extractelement <16 x float> %333, i32 14
  %"&(pSB[currWI].offset)2138" = add nuw i64 %CurrSBIndex..2, 1784
  %"&pSB[currWI].offset2139" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2138"
  %CastToValueType2140 = bitcast i8* %"&pSB[currWI].offset2139" to float*
  store float %extract682, float* %CastToValueType2140, align 4
  %extract683 = extractelement <16 x float> %333, i32 15
  %"&(pSB[currWI].offset)2152" = add nuw i64 %CurrSBIndex..2, 1788
  %"&pSB[currWI].offset2153" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2152"
  %CastToValueType2154 = bitcast i8* %"&pSB[currWI].offset2153" to float*
  store float %extract683, float* %CastToValueType2154, align 4
  %check.WI.iter2208 = icmp ult i64 %CurrWI..2, %iterCount
  br i1 %check.WI.iter2208, label %thenBB2205, label %elseBB2206

thenBB2205:                                       ; preds = %SyncBB2196
  %"CurrWI++2209" = add nuw i64 %CurrWI..2, 1
  %"loadedCurrSB+Stride2211" = add nuw i64 %CurrSBIndex..2, 1792
  br label %SyncBB2196

elseBB2206:                                       ; preds = %SyncBB2196
  %Mneg80 = icmp ne i32 %storeSum, 0
  %temp633 = insertelement <16 x i1> undef, i1 %Mneg80, i32 0
  %vector634 = shufflevector <16 x i1> %temp633, <16 x i1> undef, <16 x i32> zeroinitializer
  %334 = bitcast float addrspace(1)* %g_odata to <4 x float> addrspace(1)*
  %Mneg90 = xor i1 %2, true
  %temp738 = insertelement <16 x i1> undef, i1 %Mneg90, i32 0
  %vector739 = shufflevector <16 x i1> %temp738, <16 x i1> undef, <16 x i32> zeroinitializer
  %temp733 = insertelement <16 x i1> undef, i1 %2, i32 0
  %vector734 = shufflevector <16 x i1> %temp733, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB2177

SyncBB2177:                                       ; preds = %SyncBB2177.backedge, %elseBB2206
  %CurrSBIndex..0 = phi i64 [ 0, %elseBB2206 ], [ %CurrSBIndex..0.be, %SyncBB2177.backedge ]
  %CurrWI..0 = phi i64 [ 0, %elseBB2206 ], [ %CurrWI..0.be, %SyncBB2177.backedge ]
  %"&(pSB[currWI].offset)1451" = add nuw i64 %CurrSBIndex..0, 384
  %"&pSB[currWI].offset1452" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1451"
  %CastToValueType1453 = bitcast i8* %"&pSB[currWI].offset1452" to <16 x i64>*
  %loadedValue1454 = load <16 x i64>* %CastToValueType1453, align 128
  %sext630 = shl <16 x i64> %loadedValue1454, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %335 = ashr <16 x i64> %sext630, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  br i1 %Mneg80, label %preload980, label %postload981

preload980:                                       ; preds = %SyncBB2177
  %336 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %337 = load i64* %336, align 8
  %phitmp = add i64 %337, -1
  br label %postload981

postload981:                                      ; preds = %preload980, %SyncBB2177
  %phi982 = phi i64 [ undef, %SyncBB2177 ], [ %phitmp, %preload980 ]
  %temp631 = insertelement <16 x i64> undef, i64 %phi982, i32 0
  %vector632 = shufflevector <16 x i64> %temp631, <16 x i64> undef, <16 x i32> zeroinitializer
  %338 = icmp eq <16 x i64> %335, %vector632
  %_to_88635 = and <16 x i1> %vector634, %338
  %extract636 = extractelement <16 x i1> %_to_88635, i32 0
  %extract637 = extractelement <16 x i1> %_to_88635, i32 1
  %extract638 = extractelement <16 x i1> %_to_88635, i32 2
  %extract639 = extractelement <16 x i1> %_to_88635, i32 3
  %extract640 = extractelement <16 x i1> %_to_88635, i32 4
  %extract641 = extractelement <16 x i1> %_to_88635, i32 5
  %extract642 = extractelement <16 x i1> %_to_88635, i32 6
  %extract643 = extractelement <16 x i1> %_to_88635, i32 7
  %extract644 = extractelement <16 x i1> %_to_88635, i32 8
  %extract645 = extractelement <16 x i1> %_to_88635, i32 9
  %extract646 = extractelement <16 x i1> %_to_88635, i32 10
  %extract647 = extractelement <16 x i1> %_to_88635, i32 11
  %extract648 = extractelement <16 x i1> %_to_88635, i32 12
  %extract649 = extractelement <16 x i1> %_to_88635, i32 13
  %extract650 = extractelement <16 x i1> %_to_88635, i32 14
  %extract651 = extractelement <16 x i1> %_to_88635, i32 15
  %"&(pSB[currWI].offset)1668" = add nuw i64 %CurrSBIndex..0, 640
  %"&pSB[currWI].offset1669" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1668"
  %CastToValueType1670 = bitcast i8* %"&pSB[currWI].offset1669" to <16 x float>*
  %loadedValue1671 = load <16 x float>* %CastToValueType1670, align 64
  %"&(pSB[currWI].offset)1937" = add nuw i64 %CurrSBIndex..0, 1664
  %"&pSB[currWI].offset1938" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1937"
  %CastToValueType1939 = bitcast i8* %"&pSB[currWI].offset1938" to <16 x float>*
  %loadedValue1940 = load <16 x float>* %CastToValueType1939, align 64
  %339 = fadd <16 x float> %loadedValue1940, %loadedValue1671
  %extract652 = extractelement <16 x float> %339, i32 0
  %extract653 = extractelement <16 x float> %339, i32 1
  %extract654 = extractelement <16 x float> %339, i32 2
  %extract655 = extractelement <16 x float> %339, i32 3
  %extract656 = extractelement <16 x float> %339, i32 4
  %extract657 = extractelement <16 x float> %339, i32 5
  %extract658 = extractelement <16 x float> %339, i32 6
  %extract659 = extractelement <16 x float> %339, i32 7
  %extract660 = extractelement <16 x float> %339, i32 8
  %extract661 = extractelement <16 x float> %339, i32 9
  %extract662 = extractelement <16 x float> %339, i32 10
  %extract663 = extractelement <16 x float> %339, i32 11
  %extract664 = extractelement <16 x float> %339, i32 12
  %extract665 = extractelement <16 x float> %339, i32 13
  %extract666 = extractelement <16 x float> %339, i32 14
  %extract667 = extractelement <16 x float> %339, i32 15
  br i1 %extract636, label %preload1127, label %postload1128

preload1127:                                      ; preds = %postload981
  %340 = load i64* %pWGId, align 8
  br label %postload1128

postload1128:                                     ; preds = %preload1127, %postload981
  %phi1129 = phi i64 [ undef, %postload981 ], [ %340, %preload1127 ]
  br i1 %extract637, label %preload1132, label %postload1133

preload1132:                                      ; preds = %postload1128
  %341 = load i64* %pWGId, align 8
  br label %postload1133

postload1133:                                     ; preds = %preload1132, %postload1128
  %phi1134 = phi i64 [ undef, %postload1128 ], [ %341, %preload1132 ]
  br i1 %extract638, label %preload1137, label %postload1138

preload1137:                                      ; preds = %postload1133
  %342 = load i64* %pWGId, align 8
  br label %postload1138

postload1138:                                     ; preds = %preload1137, %postload1133
  %phi1139 = phi i64 [ undef, %postload1133 ], [ %342, %preload1137 ]
  br i1 %extract639, label %preload1142, label %postload1143

preload1142:                                      ; preds = %postload1138
  %343 = load i64* %pWGId, align 8
  br label %postload1143

postload1143:                                     ; preds = %preload1142, %postload1138
  %phi1144 = phi i64 [ undef, %postload1138 ], [ %343, %preload1142 ]
  br i1 %extract640, label %preload1147, label %postload1148

preload1147:                                      ; preds = %postload1143
  %344 = load i64* %pWGId, align 8
  br label %postload1148

postload1148:                                     ; preds = %preload1147, %postload1143
  %phi1149 = phi i64 [ undef, %postload1143 ], [ %344, %preload1147 ]
  br i1 %extract641, label %preload1152, label %postload1153

preload1152:                                      ; preds = %postload1148
  %345 = load i64* %pWGId, align 8
  br label %postload1153

postload1153:                                     ; preds = %preload1152, %postload1148
  %phi1154 = phi i64 [ undef, %postload1148 ], [ %345, %preload1152 ]
  br i1 %extract642, label %preload1157, label %postload1158

preload1157:                                      ; preds = %postload1153
  %346 = load i64* %pWGId, align 8
  br label %postload1158

postload1158:                                     ; preds = %preload1157, %postload1153
  %phi1159 = phi i64 [ undef, %postload1153 ], [ %346, %preload1157 ]
  br i1 %extract643, label %preload1162, label %postload1163

preload1162:                                      ; preds = %postload1158
  %347 = load i64* %pWGId, align 8
  br label %postload1163

postload1163:                                     ; preds = %preload1162, %postload1158
  %phi1164 = phi i64 [ undef, %postload1158 ], [ %347, %preload1162 ]
  br i1 %extract644, label %preload1167, label %postload1168

preload1167:                                      ; preds = %postload1163
  %348 = load i64* %pWGId, align 8
  br label %postload1168

postload1168:                                     ; preds = %preload1167, %postload1163
  %phi1169 = phi i64 [ undef, %postload1163 ], [ %348, %preload1167 ]
  br i1 %extract645, label %preload1172, label %postload1173

preload1172:                                      ; preds = %postload1168
  %349 = load i64* %pWGId, align 8
  br label %postload1173

postload1173:                                     ; preds = %preload1172, %postload1168
  %phi1174 = phi i64 [ undef, %postload1168 ], [ %349, %preload1172 ]
  br i1 %extract646, label %preload1177, label %postload1178

preload1177:                                      ; preds = %postload1173
  %350 = load i64* %pWGId, align 8
  br label %postload1178

postload1178:                                     ; preds = %preload1177, %postload1173
  %phi1179 = phi i64 [ undef, %postload1173 ], [ %350, %preload1177 ]
  br i1 %extract647, label %preload1182, label %postload1183

preload1182:                                      ; preds = %postload1178
  %351 = load i64* %pWGId, align 8
  br label %postload1183

postload1183:                                     ; preds = %preload1182, %postload1178
  %phi1184 = phi i64 [ undef, %postload1178 ], [ %351, %preload1182 ]
  br i1 %extract648, label %preload1187, label %postload1188

preload1187:                                      ; preds = %postload1183
  %352 = load i64* %pWGId, align 8
  br label %postload1188

postload1188:                                     ; preds = %preload1187, %postload1183
  %phi1189 = phi i64 [ undef, %postload1183 ], [ %352, %preload1187 ]
  br i1 %extract649, label %preload1192, label %postload1193

preload1192:                                      ; preds = %postload1188
  %353 = load i64* %pWGId, align 8
  br label %postload1193

postload1193:                                     ; preds = %preload1192, %postload1188
  %phi1194 = phi i64 [ undef, %postload1188 ], [ %353, %preload1192 ]
  br i1 %extract650, label %preload1197, label %postload1198

preload1197:                                      ; preds = %postload1193
  %354 = load i64* %pWGId, align 8
  br label %postload1198

postload1198:                                     ; preds = %preload1197, %postload1193
  %phi1199 = phi i64 [ undef, %postload1193 ], [ %354, %preload1197 ]
  br i1 %extract651, label %preload1202, label %postload1203

preload1202:                                      ; preds = %postload1198
  %355 = load i64* %pWGId, align 8
  br label %postload1203

postload1203:                                     ; preds = %preload1202, %postload1198
  %phi1204 = phi i64 [ undef, %postload1198 ], [ %355, %preload1202 ]
  %356 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1134
  %357 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1139
  %358 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1144
  %359 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1149
  %360 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1154
  %361 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1159
  %362 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1164
  %363 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1169
  %364 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1174
  %365 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1179
  %366 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1184
  %367 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1189
  %368 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1194
  %369 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1199
  %370 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1204
  br i1 %extract636, label %preload1130, label %postload1131

preload1130:                                      ; preds = %postload1203
  %371 = getelementptr inbounds float addrspace(1)* %g_blockSums, i64 %phi1129
  store float %extract652, float addrspace(1)* %371, align 4
  br label %postload1131

postload1131:                                     ; preds = %preload1130, %postload1203
  br i1 %extract637, label %preload1135, label %postload1136

preload1135:                                      ; preds = %postload1131
  store float %extract653, float addrspace(1)* %356, align 4
  br label %postload1136

postload1136:                                     ; preds = %preload1135, %postload1131
  br i1 %extract638, label %preload1140, label %postload1141

preload1140:                                      ; preds = %postload1136
  store float %extract654, float addrspace(1)* %357, align 4
  br label %postload1141

postload1141:                                     ; preds = %preload1140, %postload1136
  br i1 %extract639, label %preload1145, label %postload1146

preload1145:                                      ; preds = %postload1141
  store float %extract655, float addrspace(1)* %358, align 4
  br label %postload1146

postload1146:                                     ; preds = %preload1145, %postload1141
  br i1 %extract640, label %preload1150, label %postload1151

preload1150:                                      ; preds = %postload1146
  store float %extract656, float addrspace(1)* %359, align 4
  br label %postload1151

postload1151:                                     ; preds = %preload1150, %postload1146
  br i1 %extract641, label %preload1155, label %postload1156

preload1155:                                      ; preds = %postload1151
  store float %extract657, float addrspace(1)* %360, align 4
  br label %postload1156

postload1156:                                     ; preds = %preload1155, %postload1151
  br i1 %extract642, label %preload1160, label %postload1161

preload1160:                                      ; preds = %postload1156
  store float %extract658, float addrspace(1)* %361, align 4
  br label %postload1161

postload1161:                                     ; preds = %preload1160, %postload1156
  br i1 %extract643, label %preload1165, label %postload1166

preload1165:                                      ; preds = %postload1161
  store float %extract659, float addrspace(1)* %362, align 4
  br label %postload1166

postload1166:                                     ; preds = %preload1165, %postload1161
  br i1 %extract644, label %preload1170, label %postload1171

preload1170:                                      ; preds = %postload1166
  store float %extract660, float addrspace(1)* %363, align 4
  br label %postload1171

postload1171:                                     ; preds = %preload1170, %postload1166
  br i1 %extract645, label %preload1175, label %postload1176

preload1175:                                      ; preds = %postload1171
  store float %extract661, float addrspace(1)* %364, align 4
  br label %postload1176

postload1176:                                     ; preds = %preload1175, %postload1171
  br i1 %extract646, label %preload1180, label %postload1181

preload1180:                                      ; preds = %postload1176
  store float %extract662, float addrspace(1)* %365, align 4
  br label %postload1181

postload1181:                                     ; preds = %preload1180, %postload1176
  br i1 %extract647, label %preload1185, label %postload1186

preload1185:                                      ; preds = %postload1181
  store float %extract663, float addrspace(1)* %366, align 4
  br label %postload1186

postload1186:                                     ; preds = %preload1185, %postload1181
  br i1 %extract648, label %preload1190, label %postload1191

preload1190:                                      ; preds = %postload1186
  store float %extract664, float addrspace(1)* %367, align 4
  br label %postload1191

postload1191:                                     ; preds = %preload1190, %postload1186
  br i1 %extract649, label %preload1195, label %postload1196

preload1195:                                      ; preds = %postload1191
  store float %extract665, float addrspace(1)* %368, align 4
  br label %postload1196

postload1196:                                     ; preds = %preload1195, %postload1191
  br i1 %extract650, label %preload1200, label %postload1201

preload1200:                                      ; preds = %postload1196
  store float %extract666, float addrspace(1)* %369, align 4
  br label %postload1201

postload1201:                                     ; preds = %preload1200, %postload1196
  br i1 %extract651, label %preload1205, label %phi-split-bb

preload1205:                                      ; preds = %postload1201
  store float %extract667, float addrspace(1)* %370, align 4
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %preload1205, %postload1201
  %"&(pSB[currWI].offset)1695" = add nuw i64 %CurrSBIndex..0, 832
  %"&pSB[currWI].offset1696" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1695"
  %CastToValueType1697 = bitcast i8* %"&pSB[currWI].offset1696" to <16 x float>*
  %loadedValue1698 = load <16 x float>* %CastToValueType1697, align 64
  %"&(pSB[currWI].offset)1932" = add nuw i64 %CurrSBIndex..0, 1664
  %"&pSB[currWI].offset1933" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1932"
  %CastToValueType1934 = bitcast i8* %"&pSB[currWI].offset1933" to <16 x float>*
  %loadedValue1935 = load <16 x float>* %CastToValueType1934, align 64
  %372 = fadd <16 x float> %loadedValue1935, %loadedValue1698
  %extract684 = extractelement <16 x float> %372, i32 0
  %extract685 = extractelement <16 x float> %372, i32 1
  %extract686 = extractelement <16 x float> %372, i32 2
  %extract687 = extractelement <16 x float> %372, i32 3
  %extract688 = extractelement <16 x float> %372, i32 4
  %extract689 = extractelement <16 x float> %372, i32 5
  %extract690 = extractelement <16 x float> %372, i32 6
  %extract691 = extractelement <16 x float> %372, i32 7
  %extract692 = extractelement <16 x float> %372, i32 8
  %extract693 = extractelement <16 x float> %372, i32 9
  %extract694 = extractelement <16 x float> %372, i32 10
  %extract695 = extractelement <16 x float> %372, i32 11
  %extract696 = extractelement <16 x float> %372, i32 12
  %extract697 = extractelement <16 x float> %372, i32 13
  %extract698 = extractelement <16 x float> %372, i32 14
  %extract699 = extractelement <16 x float> %372, i32 15
  %"&(pSB[currWI].offset)1686" = add nuw i64 %CurrSBIndex..0, 768
  %"&pSB[currWI].offset1687" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1686"
  %CastToValueType1688 = bitcast i8* %"&pSB[currWI].offset1687" to <16 x float>*
  %loadedValue1689 = load <16 x float>* %CastToValueType1688, align 64
  %"&(pSB[currWI].offset)1927" = add nuw i64 %CurrSBIndex..0, 1664
  %"&pSB[currWI].offset1928" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1927"
  %CastToValueType1929 = bitcast i8* %"&pSB[currWI].offset1928" to <16 x float>*
  %loadedValue1930 = load <16 x float>* %CastToValueType1929, align 64
  %373 = fadd <16 x float> %loadedValue1930, %loadedValue1689
  %extract700 = extractelement <16 x float> %373, i32 0
  %extract701 = extractelement <16 x float> %373, i32 1
  %extract702 = extractelement <16 x float> %373, i32 2
  %extract703 = extractelement <16 x float> %373, i32 3
  %extract704 = extractelement <16 x float> %373, i32 4
  %extract705 = extractelement <16 x float> %373, i32 5
  %extract706 = extractelement <16 x float> %373, i32 6
  %extract707 = extractelement <16 x float> %373, i32 7
  %extract708 = extractelement <16 x float> %373, i32 8
  %extract709 = extractelement <16 x float> %373, i32 9
  %extract710 = extractelement <16 x float> %373, i32 10
  %extract711 = extractelement <16 x float> %373, i32 11
  %extract712 = extractelement <16 x float> %373, i32 12
  %extract713 = extractelement <16 x float> %373, i32 13
  %extract714 = extractelement <16 x float> %373, i32 14
  %extract715 = extractelement <16 x float> %373, i32 15
  %"&(pSB[currWI].offset)1677" = add nuw i64 %CurrSBIndex..0, 704
  %"&pSB[currWI].offset1678" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1677"
  %CastToValueType1679 = bitcast i8* %"&pSB[currWI].offset1678" to <16 x float>*
  %loadedValue1680 = load <16 x float>* %CastToValueType1679, align 64
  %"&(pSB[currWI].offset)1922" = add nuw i64 %CurrSBIndex..0, 1664
  %"&pSB[currWI].offset1923" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1922"
  %CastToValueType1924 = bitcast i8* %"&pSB[currWI].offset1923" to <16 x float>*
  %loadedValue1925 = load <16 x float>* %CastToValueType1924, align 64
  %374 = fadd <16 x float> %loadedValue1925, %loadedValue1680
  %extract717 = extractelement <16 x float> %374, i32 1
  %extract718 = extractelement <16 x float> %374, i32 2
  %extract719 = extractelement <16 x float> %374, i32 3
  %extract720 = extractelement <16 x float> %374, i32 4
  %extract721 = extractelement <16 x float> %374, i32 5
  %extract722 = extractelement <16 x float> %374, i32 6
  %extract723 = extractelement <16 x float> %374, i32 7
  %extract724 = extractelement <16 x float> %374, i32 8
  %extract725 = extractelement <16 x float> %374, i32 9
  %extract726 = extractelement <16 x float> %374, i32 10
  %extract727 = extractelement <16 x float> %374, i32 11
  %extract728 = extractelement <16 x float> %374, i32 12
  %extract729 = extractelement <16 x float> %374, i32 13
  %extract730 = extractelement <16 x float> %374, i32 14
  %extract731 = extractelement <16 x float> %374, i32 15
  %"&(pSB[currWI].offset)1965" = add nuw i64 %CurrSBIndex..0, 1732
  %"&pSB[currWI].offset1966" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1965"
  %CastToValueType1967 = bitcast i8* %"&pSB[currWI].offset1966" to float*
  %loadedValue1968 = load float* %CastToValueType1967, align 4
  %375 = insertelement <4 x float> undef, float %loadedValue1968, i32 0
  %"&(pSB[currWI].offset)1979" = add nuw i64 %CurrSBIndex..0, 1736
  %"&pSB[currWI].offset1980" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1979"
  %CastToValueType1981 = bitcast i8* %"&pSB[currWI].offset1980" to float*
  %loadedValue1982 = load float* %CastToValueType1981, align 4
  %376 = insertelement <4 x float> undef, float %loadedValue1982, i32 0
  %"&(pSB[currWI].offset)1993" = add nuw i64 %CurrSBIndex..0, 1740
  %"&pSB[currWI].offset1994" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1993"
  %CastToValueType1995 = bitcast i8* %"&pSB[currWI].offset1994" to float*
  %loadedValue1996 = load float* %CastToValueType1995, align 4
  %377 = insertelement <4 x float> undef, float %loadedValue1996, i32 0
  %"&(pSB[currWI].offset)2007" = add nuw i64 %CurrSBIndex..0, 1744
  %"&pSB[currWI].offset2008" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2007"
  %CastToValueType2009 = bitcast i8* %"&pSB[currWI].offset2008" to float*
  %loadedValue2010 = load float* %CastToValueType2009, align 4
  %378 = insertelement <4 x float> undef, float %loadedValue2010, i32 0
  %"&(pSB[currWI].offset)2021" = add nuw i64 %CurrSBIndex..0, 1748
  %"&pSB[currWI].offset2022" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2021"
  %CastToValueType2023 = bitcast i8* %"&pSB[currWI].offset2022" to float*
  %loadedValue2024 = load float* %CastToValueType2023, align 4
  %379 = insertelement <4 x float> undef, float %loadedValue2024, i32 0
  %"&(pSB[currWI].offset)2035" = add nuw i64 %CurrSBIndex..0, 1752
  %"&pSB[currWI].offset2036" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2035"
  %CastToValueType2037 = bitcast i8* %"&pSB[currWI].offset2036" to float*
  %loadedValue2038 = load float* %CastToValueType2037, align 4
  %380 = insertelement <4 x float> undef, float %loadedValue2038, i32 0
  %"&(pSB[currWI].offset)2049" = add nuw i64 %CurrSBIndex..0, 1756
  %"&pSB[currWI].offset2050" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2049"
  %CastToValueType2051 = bitcast i8* %"&pSB[currWI].offset2050" to float*
  %loadedValue2052 = load float* %CastToValueType2051, align 4
  %381 = insertelement <4 x float> undef, float %loadedValue2052, i32 0
  %"&(pSB[currWI].offset)2063" = add nuw i64 %CurrSBIndex..0, 1760
  %"&pSB[currWI].offset2064" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2063"
  %CastToValueType2065 = bitcast i8* %"&pSB[currWI].offset2064" to float*
  %loadedValue2066 = load float* %CastToValueType2065, align 4
  %382 = insertelement <4 x float> undef, float %loadedValue2066, i32 0
  %"&(pSB[currWI].offset)2077" = add nuw i64 %CurrSBIndex..0, 1764
  %"&pSB[currWI].offset2078" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2077"
  %CastToValueType2079 = bitcast i8* %"&pSB[currWI].offset2078" to float*
  %loadedValue2080 = load float* %CastToValueType2079, align 4
  %383 = insertelement <4 x float> undef, float %loadedValue2080, i32 0
  %"&(pSB[currWI].offset)2091" = add nuw i64 %CurrSBIndex..0, 1768
  %"&pSB[currWI].offset2092" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2091"
  %CastToValueType2093 = bitcast i8* %"&pSB[currWI].offset2092" to float*
  %loadedValue2094 = load float* %CastToValueType2093, align 4
  %384 = insertelement <4 x float> undef, float %loadedValue2094, i32 0
  %"&(pSB[currWI].offset)2105" = add nuw i64 %CurrSBIndex..0, 1772
  %"&pSB[currWI].offset2106" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2105"
  %CastToValueType2107 = bitcast i8* %"&pSB[currWI].offset2106" to float*
  %loadedValue2108 = load float* %CastToValueType2107, align 4
  %385 = insertelement <4 x float> undef, float %loadedValue2108, i32 0
  %"&(pSB[currWI].offset)2119" = add nuw i64 %CurrSBIndex..0, 1776
  %"&pSB[currWI].offset2120" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2119"
  %CastToValueType2121 = bitcast i8* %"&pSB[currWI].offset2120" to float*
  %loadedValue2122 = load float* %CastToValueType2121, align 4
  %386 = insertelement <4 x float> undef, float %loadedValue2122, i32 0
  %"&(pSB[currWI].offset)2133" = add nuw i64 %CurrSBIndex..0, 1780
  %"&pSB[currWI].offset2134" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2133"
  %CastToValueType2135 = bitcast i8* %"&pSB[currWI].offset2134" to float*
  %loadedValue2136 = load float* %CastToValueType2135, align 4
  %387 = insertelement <4 x float> undef, float %loadedValue2136, i32 0
  %"&(pSB[currWI].offset)2147" = add nuw i64 %CurrSBIndex..0, 1784
  %"&pSB[currWI].offset2148" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2147"
  %CastToValueType2149 = bitcast i8* %"&pSB[currWI].offset2148" to float*
  %loadedValue2150 = load float* %CastToValueType2149, align 4
  %388 = insertelement <4 x float> undef, float %loadedValue2150, i32 0
  %"&(pSB[currWI].offset)2161" = add nuw i64 %CurrSBIndex..0, 1788
  %"&pSB[currWI].offset2162" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2161"
  %CastToValueType2163 = bitcast i8* %"&pSB[currWI].offset2162" to float*
  %loadedValue2164 = load float* %CastToValueType2163, align 4
  %389 = insertelement <4 x float> undef, float %loadedValue2164, i32 0
  %390 = insertelement <4 x float> %375, float %extract685, i32 1
  %391 = insertelement <4 x float> %376, float %extract686, i32 1
  %392 = insertelement <4 x float> %377, float %extract687, i32 1
  %393 = insertelement <4 x float> %378, float %extract688, i32 1
  %394 = insertelement <4 x float> %379, float %extract689, i32 1
  %395 = insertelement <4 x float> %380, float %extract690, i32 1
  %396 = insertelement <4 x float> %381, float %extract691, i32 1
  %397 = insertelement <4 x float> %382, float %extract692, i32 1
  %398 = insertelement <4 x float> %383, float %extract693, i32 1
  %399 = insertelement <4 x float> %384, float %extract694, i32 1
  %400 = insertelement <4 x float> %385, float %extract695, i32 1
  %401 = insertelement <4 x float> %386, float %extract696, i32 1
  %402 = insertelement <4 x float> %387, float %extract697, i32 1
  %403 = insertelement <4 x float> %388, float %extract698, i32 1
  %404 = insertelement <4 x float> %389, float %extract699, i32 1
  %405 = insertelement <4 x float> %390, float %extract701, i32 2
  %406 = insertelement <4 x float> %391, float %extract702, i32 2
  %407 = insertelement <4 x float> %392, float %extract703, i32 2
  %408 = insertelement <4 x float> %393, float %extract704, i32 2
  %409 = insertelement <4 x float> %394, float %extract705, i32 2
  %410 = insertelement <4 x float> %395, float %extract706, i32 2
  %411 = insertelement <4 x float> %396, float %extract707, i32 2
  %412 = insertelement <4 x float> %397, float %extract708, i32 2
  %413 = insertelement <4 x float> %398, float %extract709, i32 2
  %414 = insertelement <4 x float> %399, float %extract710, i32 2
  %415 = insertelement <4 x float> %400, float %extract711, i32 2
  %416 = insertelement <4 x float> %401, float %extract712, i32 2
  %417 = insertelement <4 x float> %402, float %extract713, i32 2
  %418 = insertelement <4 x float> %403, float %extract714, i32 2
  %419 = insertelement <4 x float> %404, float %extract715, i32 2
  %420 = insertelement <4 x float> %405, float %extract717, i32 3
  %421 = insertelement <4 x float> %406, float %extract718, i32 3
  %422 = insertelement <4 x float> %407, float %extract719, i32 3
  %423 = insertelement <4 x float> %408, float %extract720, i32 3
  %424 = insertelement <4 x float> %409, float %extract721, i32 3
  %425 = insertelement <4 x float> %410, float %extract722, i32 3
  %426 = insertelement <4 x float> %411, float %extract723, i32 3
  %427 = insertelement <4 x float> %412, float %extract724, i32 3
  %428 = insertelement <4 x float> %413, float %extract725, i32 3
  %429 = insertelement <4 x float> %414, float %extract726, i32 3
  %430 = insertelement <4 x float> %415, float %extract727, i32 3
  %431 = insertelement <4 x float> %416, float %extract728, i32 3
  %432 = insertelement <4 x float> %417, float %extract729, i32 3
  %433 = insertelement <4 x float> %418, float %extract730, i32 3
  %434 = insertelement <4 x float> %419, float %extract731, i32 3
  %"&(pSB[currWI].offset)1635" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1636" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1635"
  %CastToValueType1637 = bitcast i8* %"&pSB[currWI].offset1636" to <16 x i32>*
  %loadedValue1638 = load <16 x i32>* %CastToValueType1637, align 64
  %435 = or <16 x i32> %loadedValue1638, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %436 = icmp slt <16 x i32> %435, %vector
  %Mneg96732 = xor <16 x i1> %436, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %_to_99735 = and <16 x i1> %vector734, %Mneg96732
  %_to_100736 = and <16 x i1> %vector734, %436
  %_Min135740 = or <16 x i1> %_to_100736, %vector739
  %extract757 = extractelement <16 x i1> %_Min135740, i32 0
  %extract758 = extractelement <16 x i1> %_Min135740, i32 1
  %extract759 = extractelement <16 x i1> %_Min135740, i32 2
  %extract760 = extractelement <16 x i1> %_Min135740, i32 3
  %extract761 = extractelement <16 x i1> %_Min135740, i32 4
  %extract762 = extractelement <16 x i1> %_Min135740, i32 5
  %extract763 = extractelement <16 x i1> %_Min135740, i32 6
  %extract764 = extractelement <16 x i1> %_Min135740, i32 7
  %extract765 = extractelement <16 x i1> %_Min135740, i32 8
  %extract766 = extractelement <16 x i1> %_Min135740, i32 9
  %extract767 = extractelement <16 x i1> %_Min135740, i32 10
  %extract768 = extractelement <16 x i1> %_Min135740, i32 11
  %extract769 = extractelement <16 x i1> %_Min135740, i32 12
  %extract770 = extractelement <16 x i1> %_Min135740, i32 13
  %extract771 = extractelement <16 x i1> %_Min135740, i32 14
  %extract772 = extractelement <16 x i1> %_Min135740, i32 15
  %"&(pSB[currWI].offset)1432" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1433" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1432"
  %CastToValueType1434 = bitcast i8* %"&pSB[currWI].offset1433" to <16 x i32>*
  %loadedValue1435 = load <16 x i32>* %CastToValueType1434, align 64
  %437 = extractelement <16 x i32> %loadedValue1435, i32 1
  %438 = sext i32 %437 to i64
  %439 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %438
  %"&(pSB[currWI].offset)1427" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1428" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1427"
  %CastToValueType1429 = bitcast i8* %"&pSB[currWI].offset1428" to <16 x i32>*
  %loadedValue1430 = load <16 x i32>* %CastToValueType1429, align 64
  %440 = extractelement <16 x i32> %loadedValue1430, i32 2
  %441 = sext i32 %440 to i64
  %442 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %441
  %"&(pSB[currWI].offset)1422" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1423" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1422"
  %CastToValueType1424 = bitcast i8* %"&pSB[currWI].offset1423" to <16 x i32>*
  %loadedValue1425 = load <16 x i32>* %CastToValueType1424, align 64
  %443 = extractelement <16 x i32> %loadedValue1425, i32 3
  %444 = sext i32 %443 to i64
  %445 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %444
  %"&(pSB[currWI].offset)1417" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1418" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1417"
  %CastToValueType1419 = bitcast i8* %"&pSB[currWI].offset1418" to <16 x i32>*
  %loadedValue1420 = load <16 x i32>* %CastToValueType1419, align 64
  %446 = extractelement <16 x i32> %loadedValue1420, i32 4
  %447 = sext i32 %446 to i64
  %448 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %447
  %"&(pSB[currWI].offset)1412" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1413" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1412"
  %CastToValueType1414 = bitcast i8* %"&pSB[currWI].offset1413" to <16 x i32>*
  %loadedValue1415 = load <16 x i32>* %CastToValueType1414, align 64
  %449 = extractelement <16 x i32> %loadedValue1415, i32 5
  %450 = sext i32 %449 to i64
  %451 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %450
  %"&(pSB[currWI].offset)1407" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1408" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1407"
  %CastToValueType1409 = bitcast i8* %"&pSB[currWI].offset1408" to <16 x i32>*
  %loadedValue1410 = load <16 x i32>* %CastToValueType1409, align 64
  %452 = extractelement <16 x i32> %loadedValue1410, i32 6
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %453
  %"&(pSB[currWI].offset)1402" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1403" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1402"
  %CastToValueType1404 = bitcast i8* %"&pSB[currWI].offset1403" to <16 x i32>*
  %loadedValue1405 = load <16 x i32>* %CastToValueType1404, align 64
  %455 = extractelement <16 x i32> %loadedValue1405, i32 7
  %456 = sext i32 %455 to i64
  %457 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %456
  %"&(pSB[currWI].offset)1397" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1398" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1397"
  %CastToValueType1399 = bitcast i8* %"&pSB[currWI].offset1398" to <16 x i32>*
  %loadedValue1400 = load <16 x i32>* %CastToValueType1399, align 64
  %458 = extractelement <16 x i32> %loadedValue1400, i32 8
  %459 = sext i32 %458 to i64
  %460 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %459
  %"&(pSB[currWI].offset)1392" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1393" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1392"
  %CastToValueType1394 = bitcast i8* %"&pSB[currWI].offset1393" to <16 x i32>*
  %loadedValue1395 = load <16 x i32>* %CastToValueType1394, align 64
  %461 = extractelement <16 x i32> %loadedValue1395, i32 9
  %462 = sext i32 %461 to i64
  %463 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %462
  %"&(pSB[currWI].offset)1387" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1388" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1387"
  %CastToValueType1389 = bitcast i8* %"&pSB[currWI].offset1388" to <16 x i32>*
  %loadedValue1390 = load <16 x i32>* %CastToValueType1389, align 64
  %464 = extractelement <16 x i32> %loadedValue1390, i32 10
  %465 = sext i32 %464 to i64
  %466 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %465
  %"&(pSB[currWI].offset)1382" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1383" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1382"
  %CastToValueType1384 = bitcast i8* %"&pSB[currWI].offset1383" to <16 x i32>*
  %loadedValue1385 = load <16 x i32>* %CastToValueType1384, align 64
  %467 = extractelement <16 x i32> %loadedValue1385, i32 11
  %468 = sext i32 %467 to i64
  %469 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %468
  %"&(pSB[currWI].offset)1377" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1378" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1377"
  %CastToValueType1379 = bitcast i8* %"&pSB[currWI].offset1378" to <16 x i32>*
  %loadedValue1380 = load <16 x i32>* %CastToValueType1379, align 64
  %470 = extractelement <16 x i32> %loadedValue1380, i32 12
  %471 = sext i32 %470 to i64
  %472 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %471
  %"&(pSB[currWI].offset)1372" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1373" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1372"
  %CastToValueType1374 = bitcast i8* %"&pSB[currWI].offset1373" to <16 x i32>*
  %loadedValue1375 = load <16 x i32>* %CastToValueType1374, align 64
  %473 = extractelement <16 x i32> %loadedValue1375, i32 13
  %474 = sext i32 %473 to i64
  %475 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %474
  %"&(pSB[currWI].offset)1367" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1368" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1367"
  %CastToValueType1369 = bitcast i8* %"&pSB[currWI].offset1368" to <16 x i32>*
  %loadedValue1370 = load <16 x i32>* %CastToValueType1369, align 64
  %476 = extractelement <16 x i32> %loadedValue1370, i32 14
  %477 = sext i32 %476 to i64
  %478 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %477
  %"&(pSB[currWI].offset)1363" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1364" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1363"
  %CastToValueType1365 = bitcast i8* %"&pSB[currWI].offset1364" to <16 x i32>*
  %loadedValue = load <16 x i32>* %CastToValueType1365, align 64
  %479 = extractelement <16 x i32> %loadedValue, i32 15
  %480 = sext i32 %479 to i64
  %481 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %480
  br i1 %extract757, label %preload1207, label %postload1208

preload1207:                                      ; preds = %phi-split-bb
  %"&(pSB[currWI].offset)1951" = add nuw i64 %CurrSBIndex..0, 1728
  %"&pSB[currWI].offset1952" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1951"
  %CastToValueType1953 = bitcast i8* %"&pSB[currWI].offset1952" to float*
  %loadedValue1954 = load float* %CastToValueType1953, align 4
  %482 = insertelement <4 x float> undef, float %loadedValue1954, i32 0
  %483 = insertelement <4 x float> %482, float %extract684, i32 1
  %484 = insertelement <4 x float> %483, float %extract700, i32 2
  %extract716 = extractelement <16 x float> %374, i32 0
  %485 = insertelement <4 x float> %484, float %extract716, i32 3
  %"&(pSB[currWI].offset)1437" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset1438" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1437"
  %CastToValueType1439 = bitcast i8* %"&pSB[currWI].offset1438" to <16 x i32>*
  %loadedValue1440 = load <16 x i32>* %CastToValueType1439, align 64
  %486 = extractelement <16 x i32> %loadedValue1440, i32 0
  %487 = sext i32 %486 to i64
  %488 = getelementptr inbounds <4 x float> addrspace(1)* %334, i64 %487
  store <4 x float> %485, <4 x float> addrspace(1)* %488, align 16
  br label %postload1208

postload1208:                                     ; preds = %preload1207, %phi-split-bb
  br i1 %extract758, label %preload1209, label %postload1210

preload1209:                                      ; preds = %postload1208
  store <4 x float> %420, <4 x float> addrspace(1)* %439, align 16
  br label %postload1210

postload1210:                                     ; preds = %preload1209, %postload1208
  br i1 %extract759, label %preload1211, label %postload1212

preload1211:                                      ; preds = %postload1210
  store <4 x float> %421, <4 x float> addrspace(1)* %442, align 16
  br label %postload1212

postload1212:                                     ; preds = %preload1211, %postload1210
  br i1 %extract760, label %preload1213, label %postload1214

preload1213:                                      ; preds = %postload1212
  store <4 x float> %422, <4 x float> addrspace(1)* %445, align 16
  br label %postload1214

postload1214:                                     ; preds = %preload1213, %postload1212
  br i1 %extract761, label %preload1215, label %postload1216

preload1215:                                      ; preds = %postload1214
  store <4 x float> %423, <4 x float> addrspace(1)* %448, align 16
  br label %postload1216

postload1216:                                     ; preds = %preload1215, %postload1214
  br i1 %extract762, label %preload1217, label %postload1218

preload1217:                                      ; preds = %postload1216
  store <4 x float> %424, <4 x float> addrspace(1)* %451, align 16
  br label %postload1218

postload1218:                                     ; preds = %preload1217, %postload1216
  br i1 %extract763, label %preload1219, label %postload1220

preload1219:                                      ; preds = %postload1218
  store <4 x float> %425, <4 x float> addrspace(1)* %454, align 16
  br label %postload1220

postload1220:                                     ; preds = %preload1219, %postload1218
  br i1 %extract764, label %preload1221, label %postload1222

preload1221:                                      ; preds = %postload1220
  store <4 x float> %426, <4 x float> addrspace(1)* %457, align 16
  br label %postload1222

postload1222:                                     ; preds = %preload1221, %postload1220
  br i1 %extract765, label %preload1223, label %postload1224

preload1223:                                      ; preds = %postload1222
  store <4 x float> %427, <4 x float> addrspace(1)* %460, align 16
  br label %postload1224

postload1224:                                     ; preds = %preload1223, %postload1222
  br i1 %extract766, label %preload1225, label %postload1226

preload1225:                                      ; preds = %postload1224
  store <4 x float> %428, <4 x float> addrspace(1)* %463, align 16
  br label %postload1226

postload1226:                                     ; preds = %preload1225, %postload1224
  br i1 %extract767, label %preload1227, label %postload1228

preload1227:                                      ; preds = %postload1226
  store <4 x float> %429, <4 x float> addrspace(1)* %466, align 16
  br label %postload1228

postload1228:                                     ; preds = %preload1227, %postload1226
  br i1 %extract768, label %preload1229, label %postload1230

preload1229:                                      ; preds = %postload1228
  store <4 x float> %430, <4 x float> addrspace(1)* %469, align 16
  br label %postload1230

postload1230:                                     ; preds = %preload1229, %postload1228
  br i1 %extract769, label %preload1231, label %postload1232

preload1231:                                      ; preds = %postload1230
  store <4 x float> %431, <4 x float> addrspace(1)* %472, align 16
  br label %postload1232

postload1232:                                     ; preds = %preload1231, %postload1230
  br i1 %extract770, label %preload1233, label %postload1234

preload1233:                                      ; preds = %postload1232
  store <4 x float> %432, <4 x float> addrspace(1)* %475, align 16
  br label %postload1234

postload1234:                                     ; preds = %preload1233, %postload1232
  br i1 %extract771, label %preload1235, label %postload1236

preload1235:                                      ; preds = %postload1234
  store <4 x float> %433, <4 x float> addrspace(1)* %478, align 16
  br label %postload1236

postload1236:                                     ; preds = %preload1235, %postload1234
  br i1 %extract772, label %preload1237, label %postload1238

preload1237:                                      ; preds = %postload1236
  store <4 x float> %434, <4 x float> addrspace(1)* %481, align 16
  br label %postload1238

postload1238:                                     ; preds = %preload1237, %postload1236
  %"&(pSB[currWI].offset)1630" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1631" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1630"
  %CastToValueType1632 = bitcast i8* %"&pSB[currWI].offset1631" to <16 x i32>*
  %loadedValue1633 = load <16 x i32>* %CastToValueType1632, align 64
  %489 = icmp slt <16 x i32> %loadedValue1633, %vector
  %_to_106773 = and <16 x i1> %_to_99735, %489
  %extract791 = extractelement <16 x i1> %_to_106773, i32 1
  %extract792 = extractelement <16 x i1> %_to_106773, i32 2
  %extract793 = extractelement <16 x i1> %_to_106773, i32 3
  %extract794 = extractelement <16 x i1> %_to_106773, i32 4
  %extract795 = extractelement <16 x i1> %_to_106773, i32 5
  %extract796 = extractelement <16 x i1> %_to_106773, i32 6
  %extract797 = extractelement <16 x i1> %_to_106773, i32 7
  %extract798 = extractelement <16 x i1> %_to_106773, i32 8
  %extract799 = extractelement <16 x i1> %_to_106773, i32 9
  %extract800 = extractelement <16 x i1> %_to_106773, i32 10
  %extract801 = extractelement <16 x i1> %_to_106773, i32 11
  %extract802 = extractelement <16 x i1> %_to_106773, i32 12
  %extract803 = extractelement <16 x i1> %_to_106773, i32 13
  %extract804 = extractelement <16 x i1> %_to_106773, i32 14
  %extract805 = extractelement <16 x i1> %_to_106773, i32 15
  %extract790 = extractelement <16 x i1> %_to_106773, i32 0
  %"&(pSB[currWI].offset)1530" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1531" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1530"
  %CastToValueType1532 = bitcast i8* %"&pSB[currWI].offset1531" to <16 x i32>*
  %loadedValue1533 = load <16 x i32>* %CastToValueType1532, align 64
  %490 = extractelement <16 x i32> %loadedValue1533, i32 1
  %491 = sext i32 %490 to i64
  %492 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %491
  %"&(pSB[currWI].offset)1525" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1526" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1525"
  %CastToValueType1527 = bitcast i8* %"&pSB[currWI].offset1526" to <16 x i32>*
  %loadedValue1528 = load <16 x i32>* %CastToValueType1527, align 64
  %493 = extractelement <16 x i32> %loadedValue1528, i32 2
  %494 = sext i32 %493 to i64
  %495 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %494
  %"&(pSB[currWI].offset)1520" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1521" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1520"
  %CastToValueType1522 = bitcast i8* %"&pSB[currWI].offset1521" to <16 x i32>*
  %loadedValue1523 = load <16 x i32>* %CastToValueType1522, align 64
  %496 = extractelement <16 x i32> %loadedValue1523, i32 3
  %497 = sext i32 %496 to i64
  %498 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %497
  %"&(pSB[currWI].offset)1515" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1516" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1515"
  %CastToValueType1517 = bitcast i8* %"&pSB[currWI].offset1516" to <16 x i32>*
  %loadedValue1518 = load <16 x i32>* %CastToValueType1517, align 64
  %499 = extractelement <16 x i32> %loadedValue1518, i32 4
  %500 = sext i32 %499 to i64
  %501 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %500
  %"&(pSB[currWI].offset)1510" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1511" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1510"
  %CastToValueType1512 = bitcast i8* %"&pSB[currWI].offset1511" to <16 x i32>*
  %loadedValue1513 = load <16 x i32>* %CastToValueType1512, align 64
  %502 = extractelement <16 x i32> %loadedValue1513, i32 5
  %503 = sext i32 %502 to i64
  %504 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %503
  %"&(pSB[currWI].offset)1505" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1506" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1505"
  %CastToValueType1507 = bitcast i8* %"&pSB[currWI].offset1506" to <16 x i32>*
  %loadedValue1508 = load <16 x i32>* %CastToValueType1507, align 64
  %505 = extractelement <16 x i32> %loadedValue1508, i32 6
  %506 = sext i32 %505 to i64
  %507 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %506
  %"&(pSB[currWI].offset)1500" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1501" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1500"
  %CastToValueType1502 = bitcast i8* %"&pSB[currWI].offset1501" to <16 x i32>*
  %loadedValue1503 = load <16 x i32>* %CastToValueType1502, align 64
  %508 = extractelement <16 x i32> %loadedValue1503, i32 7
  %509 = sext i32 %508 to i64
  %510 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %509
  %"&(pSB[currWI].offset)1495" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1496" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1495"
  %CastToValueType1497 = bitcast i8* %"&pSB[currWI].offset1496" to <16 x i32>*
  %loadedValue1498 = load <16 x i32>* %CastToValueType1497, align 64
  %511 = extractelement <16 x i32> %loadedValue1498, i32 8
  %512 = sext i32 %511 to i64
  %513 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %512
  %"&(pSB[currWI].offset)1490" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1491" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1490"
  %CastToValueType1492 = bitcast i8* %"&pSB[currWI].offset1491" to <16 x i32>*
  %loadedValue1493 = load <16 x i32>* %CastToValueType1492, align 64
  %514 = extractelement <16 x i32> %loadedValue1493, i32 9
  %515 = sext i32 %514 to i64
  %516 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %515
  %"&(pSB[currWI].offset)1485" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1486" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1485"
  %CastToValueType1487 = bitcast i8* %"&pSB[currWI].offset1486" to <16 x i32>*
  %loadedValue1488 = load <16 x i32>* %CastToValueType1487, align 64
  %517 = extractelement <16 x i32> %loadedValue1488, i32 10
  %518 = sext i32 %517 to i64
  %519 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %518
  %"&(pSB[currWI].offset)1480" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1481" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1480"
  %CastToValueType1482 = bitcast i8* %"&pSB[currWI].offset1481" to <16 x i32>*
  %loadedValue1483 = load <16 x i32>* %CastToValueType1482, align 64
  %520 = extractelement <16 x i32> %loadedValue1483, i32 11
  %521 = sext i32 %520 to i64
  %522 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %521
  %"&(pSB[currWI].offset)1475" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1476" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1475"
  %CastToValueType1477 = bitcast i8* %"&pSB[currWI].offset1476" to <16 x i32>*
  %loadedValue1478 = load <16 x i32>* %CastToValueType1477, align 64
  %523 = extractelement <16 x i32> %loadedValue1478, i32 12
  %524 = sext i32 %523 to i64
  %525 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %524
  %"&(pSB[currWI].offset)1470" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1471" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1470"
  %CastToValueType1472 = bitcast i8* %"&pSB[currWI].offset1471" to <16 x i32>*
  %loadedValue1473 = load <16 x i32>* %CastToValueType1472, align 64
  %526 = extractelement <16 x i32> %loadedValue1473, i32 13
  %527 = sext i32 %526 to i64
  %528 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %527
  %"&(pSB[currWI].offset)1465" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1466" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1465"
  %CastToValueType1467 = bitcast i8* %"&pSB[currWI].offset1466" to <16 x i32>*
  %loadedValue1468 = load <16 x i32>* %CastToValueType1467, align 64
  %529 = extractelement <16 x i32> %loadedValue1468, i32 14
  %530 = sext i32 %529 to i64
  %531 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %530
  %"&(pSB[currWI].offset)1460" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1461" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1460"
  %CastToValueType1462 = bitcast i8* %"&pSB[currWI].offset1461" to <16 x i32>*
  %loadedValue1463 = load <16 x i32>* %CastToValueType1462, align 64
  %532 = extractelement <16 x i32> %loadedValue1463, i32 15
  %533 = sext i32 %532 to i64
  %534 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %533
  br i1 %extract790, label %preload1239, label %postload1240

preload1239:                                      ; preds = %postload1238
  %"&(pSB[currWI].offset)1535" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1536" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1535"
  %CastToValueType1537 = bitcast i8* %"&pSB[currWI].offset1536" to <16 x i32>*
  %loadedValue1538 = load <16 x i32>* %CastToValueType1537, align 64
  %535 = extractelement <16 x i32> %loadedValue1538, i32 0
  %536 = sext i32 %535 to i64
  %537 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %536
  %"&(pSB[currWI].offset)1946" = add nuw i64 %CurrSBIndex..0, 1728
  %"&pSB[currWI].offset1947" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1946"
  %CastToValueType1948 = bitcast i8* %"&pSB[currWI].offset1947" to float*
  %loadedValue1949 = load float* %CastToValueType1948, align 4
  store float %loadedValue1949, float addrspace(1)* %537, align 4
  br label %postload1240

postload1240:                                     ; preds = %preload1239, %postload1238
  br i1 %extract791, label %preload1241, label %postload1242

preload1241:                                      ; preds = %postload1240
  %"&(pSB[currWI].offset)1960" = add nuw i64 %CurrSBIndex..0, 1732
  %"&pSB[currWI].offset1961" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1960"
  %CastToValueType1962 = bitcast i8* %"&pSB[currWI].offset1961" to float*
  %loadedValue1963 = load float* %CastToValueType1962, align 4
  store float %loadedValue1963, float addrspace(1)* %492, align 4
  br label %postload1242

postload1242:                                     ; preds = %preload1241, %postload1240
  br i1 %extract792, label %preload1243, label %postload1244

preload1243:                                      ; preds = %postload1242
  %"&(pSB[currWI].offset)1974" = add nuw i64 %CurrSBIndex..0, 1736
  %"&pSB[currWI].offset1975" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1974"
  %CastToValueType1976 = bitcast i8* %"&pSB[currWI].offset1975" to float*
  %loadedValue1977 = load float* %CastToValueType1976, align 4
  store float %loadedValue1977, float addrspace(1)* %495, align 4
  br label %postload1244

postload1244:                                     ; preds = %preload1243, %postload1242
  br i1 %extract793, label %preload1245, label %postload1246

preload1245:                                      ; preds = %postload1244
  %"&(pSB[currWI].offset)1988" = add nuw i64 %CurrSBIndex..0, 1740
  %"&pSB[currWI].offset1989" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1988"
  %CastToValueType1990 = bitcast i8* %"&pSB[currWI].offset1989" to float*
  %loadedValue1991 = load float* %CastToValueType1990, align 4
  store float %loadedValue1991, float addrspace(1)* %498, align 4
  br label %postload1246

postload1246:                                     ; preds = %preload1245, %postload1244
  br i1 %extract794, label %preload1247, label %postload1248

preload1247:                                      ; preds = %postload1246
  %"&(pSB[currWI].offset)2002" = add nuw i64 %CurrSBIndex..0, 1744
  %"&pSB[currWI].offset2003" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2002"
  %CastToValueType2004 = bitcast i8* %"&pSB[currWI].offset2003" to float*
  %loadedValue2005 = load float* %CastToValueType2004, align 4
  store float %loadedValue2005, float addrspace(1)* %501, align 4
  br label %postload1248

postload1248:                                     ; preds = %preload1247, %postload1246
  br i1 %extract795, label %preload1249, label %postload1250

preload1249:                                      ; preds = %postload1248
  %"&(pSB[currWI].offset)2016" = add nuw i64 %CurrSBIndex..0, 1748
  %"&pSB[currWI].offset2017" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2016"
  %CastToValueType2018 = bitcast i8* %"&pSB[currWI].offset2017" to float*
  %loadedValue2019 = load float* %CastToValueType2018, align 4
  store float %loadedValue2019, float addrspace(1)* %504, align 4
  br label %postload1250

postload1250:                                     ; preds = %preload1249, %postload1248
  br i1 %extract796, label %preload1251, label %postload1252

preload1251:                                      ; preds = %postload1250
  %"&(pSB[currWI].offset)2030" = add nuw i64 %CurrSBIndex..0, 1752
  %"&pSB[currWI].offset2031" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2030"
  %CastToValueType2032 = bitcast i8* %"&pSB[currWI].offset2031" to float*
  %loadedValue2033 = load float* %CastToValueType2032, align 4
  store float %loadedValue2033, float addrspace(1)* %507, align 4
  br label %postload1252

postload1252:                                     ; preds = %preload1251, %postload1250
  br i1 %extract797, label %preload1253, label %postload1254

preload1253:                                      ; preds = %postload1252
  %"&(pSB[currWI].offset)2044" = add nuw i64 %CurrSBIndex..0, 1756
  %"&pSB[currWI].offset2045" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2044"
  %CastToValueType2046 = bitcast i8* %"&pSB[currWI].offset2045" to float*
  %loadedValue2047 = load float* %CastToValueType2046, align 4
  store float %loadedValue2047, float addrspace(1)* %510, align 4
  br label %postload1254

postload1254:                                     ; preds = %preload1253, %postload1252
  br i1 %extract798, label %preload1255, label %postload1256

preload1255:                                      ; preds = %postload1254
  %"&(pSB[currWI].offset)2058" = add nuw i64 %CurrSBIndex..0, 1760
  %"&pSB[currWI].offset2059" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2058"
  %CastToValueType2060 = bitcast i8* %"&pSB[currWI].offset2059" to float*
  %loadedValue2061 = load float* %CastToValueType2060, align 4
  store float %loadedValue2061, float addrspace(1)* %513, align 4
  br label %postload1256

postload1256:                                     ; preds = %preload1255, %postload1254
  br i1 %extract799, label %preload1257, label %postload1258

preload1257:                                      ; preds = %postload1256
  %"&(pSB[currWI].offset)2072" = add nuw i64 %CurrSBIndex..0, 1764
  %"&pSB[currWI].offset2073" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2072"
  %CastToValueType2074 = bitcast i8* %"&pSB[currWI].offset2073" to float*
  %loadedValue2075 = load float* %CastToValueType2074, align 4
  store float %loadedValue2075, float addrspace(1)* %516, align 4
  br label %postload1258

postload1258:                                     ; preds = %preload1257, %postload1256
  br i1 %extract800, label %preload1259, label %postload1260

preload1259:                                      ; preds = %postload1258
  %"&(pSB[currWI].offset)2086" = add nuw i64 %CurrSBIndex..0, 1768
  %"&pSB[currWI].offset2087" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2086"
  %CastToValueType2088 = bitcast i8* %"&pSB[currWI].offset2087" to float*
  %loadedValue2089 = load float* %CastToValueType2088, align 4
  store float %loadedValue2089, float addrspace(1)* %519, align 4
  br label %postload1260

postload1260:                                     ; preds = %preload1259, %postload1258
  br i1 %extract801, label %preload1261, label %postload1262

preload1261:                                      ; preds = %postload1260
  %"&(pSB[currWI].offset)2100" = add nuw i64 %CurrSBIndex..0, 1772
  %"&pSB[currWI].offset2101" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2100"
  %CastToValueType2102 = bitcast i8* %"&pSB[currWI].offset2101" to float*
  %loadedValue2103 = load float* %CastToValueType2102, align 4
  store float %loadedValue2103, float addrspace(1)* %522, align 4
  br label %postload1262

postload1262:                                     ; preds = %preload1261, %postload1260
  br i1 %extract802, label %preload1263, label %postload1264

preload1263:                                      ; preds = %postload1262
  %"&(pSB[currWI].offset)2114" = add nuw i64 %CurrSBIndex..0, 1776
  %"&pSB[currWI].offset2115" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2114"
  %CastToValueType2116 = bitcast i8* %"&pSB[currWI].offset2115" to float*
  %loadedValue2117 = load float* %CastToValueType2116, align 4
  store float %loadedValue2117, float addrspace(1)* %525, align 4
  br label %postload1264

postload1264:                                     ; preds = %preload1263, %postload1262
  br i1 %extract803, label %preload1265, label %postload1266

preload1265:                                      ; preds = %postload1264
  %"&(pSB[currWI].offset)2128" = add nuw i64 %CurrSBIndex..0, 1780
  %"&pSB[currWI].offset2129" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2128"
  %CastToValueType2130 = bitcast i8* %"&pSB[currWI].offset2129" to float*
  %loadedValue2131 = load float* %CastToValueType2130, align 4
  store float %loadedValue2131, float addrspace(1)* %528, align 4
  br label %postload1266

postload1266:                                     ; preds = %preload1265, %postload1264
  br i1 %extract804, label %preload1267, label %postload1268

preload1267:                                      ; preds = %postload1266
  %"&(pSB[currWI].offset)2142" = add nuw i64 %CurrSBIndex..0, 1784
  %"&pSB[currWI].offset2143" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2142"
  %CastToValueType2144 = bitcast i8* %"&pSB[currWI].offset2143" to float*
  %loadedValue2145 = load float* %CastToValueType2144, align 4
  store float %loadedValue2145, float addrspace(1)* %531, align 4
  br label %postload1268

postload1268:                                     ; preds = %preload1267, %postload1266
  br i1 %extract805, label %preload1269, label %postload1270

preload1269:                                      ; preds = %postload1268
  %"&(pSB[currWI].offset)2156" = add nuw i64 %CurrSBIndex..0, 1788
  %"&pSB[currWI].offset2157" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2156"
  %CastToValueType2158 = bitcast i8* %"&pSB[currWI].offset2157" to float*
  %loadedValue2159 = load float* %CastToValueType2158, align 4
  store float %loadedValue2159, float addrspace(1)* %534, align 4
  br label %postload1270

postload1270:                                     ; preds = %preload1269, %postload1268
  %"&(pSB[currWI].offset)1625" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1626" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1625"
  %CastToValueType1627 = bitcast i8* %"&pSB[currWI].offset1626" to <16 x i32>*
  %loadedValue1628 = load <16 x i32>* %CastToValueType1627, align 64
  %538 = or <16 x i32> %loadedValue1628, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %539 = icmp slt <16 x i32> %538, %vector
  %_to_110806 = and <16 x i1> %_to_106773, %539
  %extract824 = extractelement <16 x i1> %_to_110806, i32 1
  %extract825 = extractelement <16 x i1> %_to_110806, i32 2
  %extract826 = extractelement <16 x i1> %_to_110806, i32 3
  %extract827 = extractelement <16 x i1> %_to_110806, i32 4
  %extract828 = extractelement <16 x i1> %_to_110806, i32 5
  %extract829 = extractelement <16 x i1> %_to_110806, i32 6
  %extract830 = extractelement <16 x i1> %_to_110806, i32 7
  %extract831 = extractelement <16 x i1> %_to_110806, i32 8
  %extract832 = extractelement <16 x i1> %_to_110806, i32 9
  %extract833 = extractelement <16 x i1> %_to_110806, i32 10
  %extract834 = extractelement <16 x i1> %_to_110806, i32 11
  %extract835 = extractelement <16 x i1> %_to_110806, i32 12
  %extract836 = extractelement <16 x i1> %_to_110806, i32 13
  %extract837 = extractelement <16 x i1> %_to_110806, i32 14
  %extract838 = extractelement <16 x i1> %_to_110806, i32 15
  %extract823 = extractelement <16 x i1> %_to_110806, i32 0
  %540 = extractelement <16 x i32> %538, i32 1
  %541 = sext i32 %540 to i64
  %542 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %541
  %543 = extractelement <16 x i32> %538, i32 2
  %544 = sext i32 %543 to i64
  %545 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %544
  %546 = extractelement <16 x i32> %538, i32 3
  %547 = sext i32 %546 to i64
  %548 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %547
  %549 = extractelement <16 x i32> %538, i32 4
  %550 = sext i32 %549 to i64
  %551 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %550
  %552 = extractelement <16 x i32> %538, i32 5
  %553 = sext i32 %552 to i64
  %554 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %553
  %555 = extractelement <16 x i32> %538, i32 6
  %556 = sext i32 %555 to i64
  %557 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %556
  %558 = extractelement <16 x i32> %538, i32 7
  %559 = sext i32 %558 to i64
  %560 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %559
  %561 = extractelement <16 x i32> %538, i32 8
  %562 = sext i32 %561 to i64
  %563 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %562
  %564 = extractelement <16 x i32> %538, i32 9
  %565 = sext i32 %564 to i64
  %566 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %565
  %567 = extractelement <16 x i32> %538, i32 10
  %568 = sext i32 %567 to i64
  %569 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %568
  %570 = extractelement <16 x i32> %538, i32 11
  %571 = sext i32 %570 to i64
  %572 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %571
  %573 = extractelement <16 x i32> %538, i32 12
  %574 = sext i32 %573 to i64
  %575 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %574
  %576 = extractelement <16 x i32> %538, i32 13
  %577 = sext i32 %576 to i64
  %578 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %577
  %579 = extractelement <16 x i32> %538, i32 14
  %580 = sext i32 %579 to i64
  %581 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %580
  %582 = extractelement <16 x i32> %538, i32 15
  %583 = sext i32 %582 to i64
  %584 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %583
  br i1 %extract823, label %preload1271, label %postload1272

preload1271:                                      ; preds = %postload1270
  %585 = extractelement <16 x i32> %538, i32 0
  %586 = sext i32 %585 to i64
  %587 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %586
  store float %extract684, float addrspace(1)* %587, align 4
  br label %postload1272

postload1272:                                     ; preds = %preload1271, %postload1270
  br i1 %extract824, label %preload1273, label %postload1274

preload1273:                                      ; preds = %postload1272
  store float %extract685, float addrspace(1)* %542, align 4
  br label %postload1274

postload1274:                                     ; preds = %preload1273, %postload1272
  br i1 %extract825, label %preload1275, label %postload1276

preload1275:                                      ; preds = %postload1274
  store float %extract686, float addrspace(1)* %545, align 4
  br label %postload1276

postload1276:                                     ; preds = %preload1275, %postload1274
  br i1 %extract826, label %preload1277, label %postload1278

preload1277:                                      ; preds = %postload1276
  store float %extract687, float addrspace(1)* %548, align 4
  br label %postload1278

postload1278:                                     ; preds = %preload1277, %postload1276
  br i1 %extract827, label %preload1279, label %postload1280

preload1279:                                      ; preds = %postload1278
  store float %extract688, float addrspace(1)* %551, align 4
  br label %postload1280

postload1280:                                     ; preds = %preload1279, %postload1278
  br i1 %extract828, label %preload1281, label %postload1282

preload1281:                                      ; preds = %postload1280
  store float %extract689, float addrspace(1)* %554, align 4
  br label %postload1282

postload1282:                                     ; preds = %preload1281, %postload1280
  br i1 %extract829, label %preload1283, label %postload1284

preload1283:                                      ; preds = %postload1282
  store float %extract690, float addrspace(1)* %557, align 4
  br label %postload1284

postload1284:                                     ; preds = %preload1283, %postload1282
  br i1 %extract830, label %preload1285, label %postload1286

preload1285:                                      ; preds = %postload1284
  store float %extract691, float addrspace(1)* %560, align 4
  br label %postload1286

postload1286:                                     ; preds = %preload1285, %postload1284
  br i1 %extract831, label %preload1287, label %postload1288

preload1287:                                      ; preds = %postload1286
  store float %extract692, float addrspace(1)* %563, align 4
  br label %postload1288

postload1288:                                     ; preds = %preload1287, %postload1286
  br i1 %extract832, label %preload1289, label %postload1290

preload1289:                                      ; preds = %postload1288
  store float %extract693, float addrspace(1)* %566, align 4
  br label %postload1290

postload1290:                                     ; preds = %preload1289, %postload1288
  br i1 %extract833, label %preload1291, label %postload1292

preload1291:                                      ; preds = %postload1290
  store float %extract694, float addrspace(1)* %569, align 4
  br label %postload1292

postload1292:                                     ; preds = %preload1291, %postload1290
  br i1 %extract834, label %preload1293, label %postload1294

preload1293:                                      ; preds = %postload1292
  store float %extract695, float addrspace(1)* %572, align 4
  br label %postload1294

postload1294:                                     ; preds = %preload1293, %postload1292
  br i1 %extract835, label %preload1295, label %postload1296

preload1295:                                      ; preds = %postload1294
  store float %extract696, float addrspace(1)* %575, align 4
  br label %postload1296

postload1296:                                     ; preds = %preload1295, %postload1294
  br i1 %extract836, label %preload1297, label %postload1298

preload1297:                                      ; preds = %postload1296
  store float %extract697, float addrspace(1)* %578, align 4
  br label %postload1298

postload1298:                                     ; preds = %preload1297, %postload1296
  br i1 %extract837, label %preload1299, label %postload1300

preload1299:                                      ; preds = %postload1298
  store float %extract698, float addrspace(1)* %581, align 4
  br label %postload1300

postload1300:                                     ; preds = %preload1299, %postload1298
  br i1 %extract838, label %preload1301, label %postload1302

preload1301:                                      ; preds = %postload1300
  store float %extract699, float addrspace(1)* %584, align 4
  br label %postload1302

postload1302:                                     ; preds = %preload1301, %postload1300
  %"&(pSB[currWI].offset)1620" = add nuw i64 %CurrSBIndex..0, 512
  %"&pSB[currWI].offset1621" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)1620"
  %CastToValueType1622 = bitcast i8* %"&pSB[currWI].offset1621" to <16 x i32>*
  %loadedValue1623 = load <16 x i32>* %CastToValueType1622, align 64
  %588 = or <16 x i32> %loadedValue1623, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %589 = icmp slt <16 x i32> %588, %vector
  %_to_116839 = and <16 x i1> %_to_110806, %589
  %extract857 = extractelement <16 x i1> %_to_116839, i32 1
  %extract858 = extractelement <16 x i1> %_to_116839, i32 2
  %extract859 = extractelement <16 x i1> %_to_116839, i32 3
  %extract860 = extractelement <16 x i1> %_to_116839, i32 4
  %extract861 = extractelement <16 x i1> %_to_116839, i32 5
  %extract862 = extractelement <16 x i1> %_to_116839, i32 6
  %extract863 = extractelement <16 x i1> %_to_116839, i32 7
  %extract864 = extractelement <16 x i1> %_to_116839, i32 8
  %extract865 = extractelement <16 x i1> %_to_116839, i32 9
  %extract866 = extractelement <16 x i1> %_to_116839, i32 10
  %extract867 = extractelement <16 x i1> %_to_116839, i32 11
  %extract868 = extractelement <16 x i1> %_to_116839, i32 12
  %extract869 = extractelement <16 x i1> %_to_116839, i32 13
  %extract870 = extractelement <16 x i1> %_to_116839, i32 14
  %extract871 = extractelement <16 x i1> %_to_116839, i32 15
  %extract856 = extractelement <16 x i1> %_to_116839, i32 0
  %590 = extractelement <16 x i32> %588, i32 1
  %591 = sext i32 %590 to i64
  %592 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %591
  %593 = extractelement <16 x i32> %588, i32 2
  %594 = sext i32 %593 to i64
  %595 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %594
  %596 = extractelement <16 x i32> %588, i32 3
  %597 = sext i32 %596 to i64
  %598 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %597
  %599 = extractelement <16 x i32> %588, i32 4
  %600 = sext i32 %599 to i64
  %601 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %600
  %602 = extractelement <16 x i32> %588, i32 5
  %603 = sext i32 %602 to i64
  %604 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %603
  %605 = extractelement <16 x i32> %588, i32 6
  %606 = sext i32 %605 to i64
  %607 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %606
  %608 = extractelement <16 x i32> %588, i32 7
  %609 = sext i32 %608 to i64
  %610 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %609
  %611 = extractelement <16 x i32> %588, i32 8
  %612 = sext i32 %611 to i64
  %613 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %612
  %614 = extractelement <16 x i32> %588, i32 9
  %615 = sext i32 %614 to i64
  %616 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %615
  %617 = extractelement <16 x i32> %588, i32 10
  %618 = sext i32 %617 to i64
  %619 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %618
  %620 = extractelement <16 x i32> %588, i32 11
  %621 = sext i32 %620 to i64
  %622 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %621
  %623 = extractelement <16 x i32> %588, i32 12
  %624 = sext i32 %623 to i64
  %625 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %624
  %626 = extractelement <16 x i32> %588, i32 13
  %627 = sext i32 %626 to i64
  %628 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %627
  %629 = extractelement <16 x i32> %588, i32 14
  %630 = sext i32 %629 to i64
  %631 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %630
  %632 = extractelement <16 x i32> %588, i32 15
  %633 = sext i32 %632 to i64
  %634 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %633
  br i1 %extract856, label %preload1303, label %postload1304

preload1303:                                      ; preds = %postload1302
  %635 = extractelement <16 x i32> %588, i32 0
  %636 = sext i32 %635 to i64
  %637 = getelementptr inbounds float addrspace(1)* %g_odata, i64 %636
  store float %extract700, float addrspace(1)* %637, align 4
  br label %postload1304

postload1304:                                     ; preds = %preload1303, %postload1302
  br i1 %extract857, label %preload1305, label %postload1306

preload1305:                                      ; preds = %postload1304
  store float %extract701, float addrspace(1)* %592, align 4
  br label %postload1306

postload1306:                                     ; preds = %preload1305, %postload1304
  br i1 %extract858, label %preload1307, label %postload1308

preload1307:                                      ; preds = %postload1306
  store float %extract702, float addrspace(1)* %595, align 4
  br label %postload1308

postload1308:                                     ; preds = %preload1307, %postload1306
  br i1 %extract859, label %preload1309, label %postload1310

preload1309:                                      ; preds = %postload1308
  store float %extract703, float addrspace(1)* %598, align 4
  br label %postload1310

postload1310:                                     ; preds = %preload1309, %postload1308
  br i1 %extract860, label %preload1311, label %postload1312

preload1311:                                      ; preds = %postload1310
  store float %extract704, float addrspace(1)* %601, align 4
  br label %postload1312

postload1312:                                     ; preds = %preload1311, %postload1310
  br i1 %extract861, label %preload1313, label %postload1314

preload1313:                                      ; preds = %postload1312
  store float %extract705, float addrspace(1)* %604, align 4
  br label %postload1314

postload1314:                                     ; preds = %preload1313, %postload1312
  br i1 %extract862, label %preload1315, label %postload1316

preload1315:                                      ; preds = %postload1314
  store float %extract706, float addrspace(1)* %607, align 4
  br label %postload1316

postload1316:                                     ; preds = %preload1315, %postload1314
  br i1 %extract863, label %preload1317, label %postload1318

preload1317:                                      ; preds = %postload1316
  store float %extract707, float addrspace(1)* %610, align 4
  br label %postload1318

postload1318:                                     ; preds = %preload1317, %postload1316
  br i1 %extract864, label %preload1319, label %postload1320

preload1319:                                      ; preds = %postload1318
  store float %extract708, float addrspace(1)* %613, align 4
  br label %postload1320

postload1320:                                     ; preds = %preload1319, %postload1318
  br i1 %extract865, label %preload1321, label %postload1322

preload1321:                                      ; preds = %postload1320
  store float %extract709, float addrspace(1)* %616, align 4
  br label %postload1322

postload1322:                                     ; preds = %preload1321, %postload1320
  br i1 %extract866, label %preload1323, label %postload1324

preload1323:                                      ; preds = %postload1322
  store float %extract710, float addrspace(1)* %619, align 4
  br label %postload1324

postload1324:                                     ; preds = %preload1323, %postload1322
  br i1 %extract867, label %preload1325, label %postload1326

preload1325:                                      ; preds = %postload1324
  store float %extract711, float addrspace(1)* %622, align 4
  br label %postload1326

postload1326:                                     ; preds = %preload1325, %postload1324
  br i1 %extract868, label %preload1327, label %postload1328

preload1327:                                      ; preds = %postload1326
  store float %extract712, float addrspace(1)* %625, align 4
  br label %postload1328

postload1328:                                     ; preds = %preload1327, %postload1326
  br i1 %extract869, label %preload1329, label %postload1330

preload1329:                                      ; preds = %postload1328
  store float %extract713, float addrspace(1)* %628, align 4
  br label %postload1330

postload1330:                                     ; preds = %preload1329, %postload1328
  br i1 %extract870, label %preload1331, label %postload1332

preload1331:                                      ; preds = %postload1330
  store float %extract714, float addrspace(1)* %631, align 4
  br label %postload1332

postload1332:                                     ; preds = %preload1331, %postload1330
  br i1 %extract871, label %preload1333, label %UnifiedReturnBlock

preload1333:                                      ; preds = %postload1332
  store float %extract715, float addrspace(1)* %634, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %SyncBB2177.backedge, label %SyncBB

SyncBB2177.backedge:                              ; preds = %UnifiedReturnBlock, %preload1333
  %CurrWI..0.be = add i64 %CurrWI..0, 1
  %CurrSBIndex..0.be = add i64 %CurrSBIndex..0, 1792
  br label %SyncBB2177

SyncBB:                                           ; preds = %UnifiedReturnBlock, %preload1333
  ret void

UnifiedReturnBlock:                               ; preds = %postload1332
  %check.WI.iter2215 = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter2215, label %SyncBB2177.backedge, label %SyncBB
}

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @addUniform(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i8 addrspace(3)**
  %10 = load i8 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to %struct.WorkDim**
  %13 = load %struct.WorkDim** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 56
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  %26 = bitcast i8 addrspace(3)* %10 to float addrspace(3)*
  br label %SyncBB6.i

SyncBB6.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %27 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = icmp eq i64 %28, 0
  br i1 %29, label %30, label %"Barrier BB.i"

; <label>:30                                      ; preds = %SyncBB6.i
  %31 = load i64* %16, align 8
  %32 = getelementptr inbounds float addrspace(1)* %4, i64 %31
  %33 = load float addrspace(1)* %32, align 4
  store float %33, float addrspace(3)* %26, align 4
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %30, %SyncBB6.i
  %34 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %35 = load i64* %34, align 8
  %36 = load i64* %16, align 8
  %37 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %38 = load i64* %37, align 8
  %39 = shl i64 %36, 2
  %40 = mul i64 %39, %38
  %41 = add i64 %40, %35
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %41, i64* %CastToValueType.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %"Barrier BB.i"
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  br label %SyncBB6.i

SyncBB.i:                                         ; preds = %thenBB8.i, %"Barrier BB.i"
  %CurrWI..1.i = phi i64 [ %"CurrWI++12.i", %thenBB8.i ], [ 0, %"Barrier BB.i" ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride14.i", %thenBB8.i ], [ 0, %"Barrier BB.i" ]
  %"&pSB[currWI].offset3.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..1.i
  %CastToValueType4.i = bitcast i8* %"&pSB[currWI].offset3.i" to i64*
  %loadedValue.i = load i64* %CastToValueType4.i, align 8
  br label %42

; <label>:42                                      ; preds = %47, %SyncBB.i
  %address.0.in.i = phi i64 [ %loadedValue.i, %SyncBB.i ], [ %55, %47 ]
  %43 = phi i32 [ 0, %SyncBB.i ], [ %56, %47 ]
  %44 = icmp slt i32 %43, 4
  br i1 %44, label %45, label %.critedge.i

; <label>:45                                      ; preds = %42
  %address.0.i = trunc i64 %address.0.in.i to i32
  %46 = icmp ult i32 %address.0.i, %7
  br i1 %46, label %47, label %.critedge.i

; <label>:47                                      ; preds = %45
  %48 = load float addrspace(3)* %26, align 4
  %49 = and i64 %address.0.in.i, 4294967295
  %50 = getelementptr inbounds float addrspace(1)* %1, i64 %49
  %51 = load float addrspace(1)* %50, align 4
  %52 = fadd float %51, %48
  store float %52, float addrspace(1)* %50, align 4
  %53 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %54 = load i64* %53, align 8
  %55 = add i64 %54, %49
  %56 = add nsw i32 %43, 1
  br label %42

.critedge.i:                                      ; preds = %45, %42
  %check.WI.iter11.i = icmp ult i64 %CurrWI..1.i, %22
  br i1 %check.WI.iter11.i, label %thenBB8.i, label %__addUniform_separated_args.exit

thenBB8.i:                                        ; preds = %.critedge.i
  %"CurrWI++12.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride14.i" = add nuw i64 %CurrSBIndex..1.i, 1792
  br label %SyncBB.i

__addUniform_separated_args.exit:                 ; preds = %.critedge.i
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
  %17 = getelementptr i8* %pBuffer, i64 40
  %18 = bitcast i8* %17 to i8 addrspace(3)**
  %19 = load i8 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 48
  %21 = bitcast i8* %20 to %struct.WorkDim**
  %22 = load %struct.WorkDim** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to i64**
  %25 = load i64** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 64
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 72
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 88
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 96
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %38 = bitcast i8 addrspace(3)* %19 to [512 x float] addrspace(3)*
  %39 = bitcast float addrspace(1)* %4 to <4 x float> addrspace(1)*
  %40 = icmp eq i32 %13, 0
  br label %SyncBB382.i

SyncBB382.i:                                      ; preds = %thenBB402.i, %entry
  %CurrSBIndex..1.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride408.i", %thenBB402.i ]
  %CurrWI..1.i = phi i64 [ 0, %entry ], [ %"CurrWI++406.i", %thenBB402.i ]
  %41 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..1.i, i32 0, i64 0
  %42 = load i64* %41, align 8
  %43 = getelementptr %struct.PaddedDimId* %28, i64 0, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = add i64 %42, %44
  %46 = trunc i64 %45 to i32
  %"&(pSB[currWI].offset)1.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1.i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %46, i32* %CastToValueType.i, align 4
  %47 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..1.i, i32 0, i64 0
  %48 = load i64* %47, align 8
  %"&(pSB[currWI].offset)322.i" = or i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset33.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)322.i"
  %CastToValueType34.i = bitcast i8* %"&pSB[currWI].offset33.i" to i64*
  store i64 %48, i64* %CastToValueType34.i, align 8
  %49 = shl i32 %46, 2
  %"&(pSB[currWI].offset)413.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset42.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)413.i"
  %CastToValueType43.i = bitcast i8* %"&pSB[currWI].offset42.i" to i32*
  store i32 %49, i32* %CastToValueType43.i, align 4
  br i1 %40, label %50, label %53

; <label>:50                                      ; preds = %SyncBB382.i
  %"&(pSB[currWI].offset)7072.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset71.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)7072.i"
  %CastToValueType72.i = bitcast i8* %"&pSB[currWI].offset71.i" to i32*
  %loadedValue73.i = load i32* %CastToValueType72.i, align 4
  %51 = or i32 %loadedValue73.i, 3
  %52 = icmp slt i32 %51, %10
  br i1 %52, label %53, label %67

; <label>:53                                      ; preds = %50, %SyncBB382.i
  %"&(pSB[currWI].offset)234.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)234.i"
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i32*
  %loadedValue.i = load i32* %CastToValueType25.i, align 4
  %54 = sext i32 %loadedValue.i to i64
  %55 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %54
  %56 = load <4 x float> addrspace(1)* %55, align 16
  %57 = extractelement <4 x float> %56, i32 1
  %58 = extractelement <4 x float> %56, i32 0
  %59 = fadd float %57, %58
  %60 = insertelement <4 x float> %56, float %59, i32 1
  %61 = extractelement <4 x float> %56, i32 2
  %62 = fadd float %61, %59
  %63 = insertelement <4 x float> %60, float %62, i32 2
  %64 = extractelement <4 x float> %56, i32 3
  %65 = fadd float %64, %62
  %66 = insertelement <4 x float> %63, float %65, i32 3
  br label %"Barrier BB.i"

; <label>:67                                      ; preds = %50
  %"&(pSB[currWI].offset)9073.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset91.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)9073.i"
  %CastToValueType92.i = bitcast i8* %"&pSB[currWI].offset91.i" to i32*
  %loadedValue93.i = load i32* %CastToValueType92.i, align 4
  %68 = icmp slt i32 %loadedValue93.i, %10
  br i1 %68, label %69, label %73

; <label>:69                                      ; preds = %67
  %"&(pSB[currWI].offset)8576.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset86.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)8576.i"
  %CastToValueType87.i = bitcast i8* %"&pSB[currWI].offset86.i" to i32*
  %loadedValue88.i = load i32* %CastToValueType87.i, align 4
  %70 = sext i32 %loadedValue88.i to i64
  %71 = getelementptr inbounds float addrspace(1)* %4, i64 %70
  %72 = load float addrspace(1)* %71, align 4
  br label %73

; <label>:73                                      ; preds = %69, %67
  %74 = phi float [ %72, %69 ], [ 0.000000e+00, %67 ]
  %75 = insertelement <4 x float> undef, float %74, i32 0
  %"&(pSB[currWI].offset)6574.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset66.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)6574.i"
  %CastToValueType67.i = bitcast i8* %"&pSB[currWI].offset66.i" to i32*
  %loadedValue68.i = load i32* %CastToValueType67.i, align 4
  %76 = or i32 %loadedValue68.i, 1
  %77 = icmp slt i32 %76, %10
  br i1 %77, label %78, label %82

; <label>:78                                      ; preds = %73
  %79 = sext i32 %76 to i64
  %80 = getelementptr inbounds float addrspace(1)* %4, i64 %79
  %81 = load float addrspace(1)* %80, align 4
  br label %82

; <label>:82                                      ; preds = %78, %73
  %83 = phi float [ %81, %78 ], [ 0.000000e+00, %73 ]
  %84 = fadd float %83, %74
  %85 = insertelement <4 x float> %75, float %84, i32 1
  %"&(pSB[currWI].offset)6075.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset61.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)6075.i"
  %CastToValueType62.i = bitcast i8* %"&pSB[currWI].offset61.i" to i32*
  %loadedValue63.i = load i32* %CastToValueType62.i, align 4
  %86 = or i32 %loadedValue63.i, 2
  %87 = icmp slt i32 %86, %10
  br i1 %87, label %88, label %92

; <label>:88                                      ; preds = %82
  %89 = sext i32 %86 to i64
  %90 = getelementptr inbounds float addrspace(1)* %4, i64 %89
  %91 = load float addrspace(1)* %90, align 4
  br label %92

; <label>:92                                      ; preds = %88, %82
  %93 = phi float [ %91, %88 ], [ 0.000000e+00, %82 ]
  %94 = fadd float %93, %84
  %95 = insertelement <4 x float> %85, float %94, i32 2
  %96 = fadd float %94, 0.000000e+00
  %97 = insertelement <4 x float> %95, float %96, i32 3
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %92, %53
  %threadScanT.0.i = phi <4 x float> [ %66, %53 ], [ %97, %92 ]
  %res.0.i = phi float [ %65, %53 ], [ %96, %92 ]
  %"&(pSB[currWI].offset)1195.i" = or i64 %CurrSBIndex..1.i, 48
  %"&pSB[currWI].offset120.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1195.i"
  %CastToValueType121.i = bitcast i8* %"&pSB[currWI].offset120.i" to float*
  store float %res.0.i, float* %CastToValueType121.i, align 4
  %"&(pSB[currWI].offset)956.i" = or i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset96.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)956.i"
  %CastToValueType97.i = bitcast i8* %"&pSB[currWI].offset96.i" to <4 x float>*
  store <4 x float> %threadScanT.0.i, <4 x float>* %CastToValueType97.i, align 16
  %check.WI.iter405.i = icmp ult i64 %CurrWI..1.i, %34
  br i1 %check.WI.iter405.i, label %thenBB402.i, label %SyncBB380.i

thenBB402.i:                                      ; preds = %"Barrier BB.i"
  %"CurrWI++406.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride408.i" = add nuw i64 %CurrSBIndex..1.i, 1792
  br label %SyncBB382.i

SyncBB380.i:                                      ; preds = %thenBB416.i, %"Barrier BB.i"
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride422.i", %thenBB416.i ], [ 0, %"Barrier BB.i" ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++420.i", %thenBB416.i ], [ 0, %"Barrier BB.i" ]
  %98 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..3.i, i32 0, i64 0
  %99 = load i64* %98, align 8
  %"&(pSB[currWI].offset)1287.i" = or i64 %CurrSBIndex..3.i, 56
  %"&pSB[currWI].offset129.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1287.i"
  %CastToValueType130.i = bitcast i8* %"&pSB[currWI].offset129.i" to i64*
  store i64 %99, i64* %CastToValueType130.i, align 8
  %sext.i.i = shl i64 %99, 32
  %100 = ashr i64 %sext.i.i, 32
  %101 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %100
  store float 0.000000e+00, float addrspace(3)* %101, align 4
  %check.WI.iter419.i = icmp ult i64 %CurrWI..3.i, %34
  br i1 %check.WI.iter419.i, label %thenBB416.i, label %SyncBB383.i

thenBB416.i:                                      ; preds = %SyncBB380.i
  %"CurrWI++420.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride422.i" = add nuw i64 %CurrSBIndex..3.i, 1792
  br label %SyncBB380.i

SyncBB383.i:                                      ; preds = %thenBB423.i, %SyncBB380.i
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride429.i", %thenBB423.i ], [ 0, %SyncBB380.i ]
  %CurrWI..4.i = phi i64 [ %"CurrWI++427.i", %thenBB423.i ], [ 0, %SyncBB380.i ]
  %102 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %103 = load i64* %102, align 8
  %"&(pSB[currWI].offset)1328.i" = or i64 %CurrSBIndex..4.i, 56
  %"&pSB[currWI].offset133.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1328.i"
  %CastToValueType134.i = bitcast i8* %"&pSB[currWI].offset133.i" to i64*
  %loadedValue135.i = load i64* %CastToValueType134.i, align 8
  %104 = add i64 %103, %loadedValue135.i
  %105 = trunc i64 %104 to i32
  %"&(pSB[currWI].offset)1379.i" = or i64 %CurrSBIndex..4.i, 64
  %"&pSB[currWI].offset138.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1379.i"
  %CastToValueType139.i = bitcast i8* %"&pSB[currWI].offset138.i" to i32*
  store i32 %105, i32* %CastToValueType139.i, align 4
  %106 = sext i32 %105 to i64
  %107 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %106
  %"&(pSB[currWI].offset)18110.i" = or i64 %CurrSBIndex..4.i, 72
  %"&pSB[currWI].offset182.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)18110.i"
  %CastToValueType183.i = bitcast i8* %"&pSB[currWI].offset182.i" to float addrspace(3)**
  store float addrspace(3)* %107, float addrspace(3)** %CastToValueType183.i, align 8
  %"&(pSB[currWI].offset)12311.i" = or i64 %CurrSBIndex..4.i, 48
  %"&pSB[currWI].offset124.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)12311.i"
  %CastToValueType125.i = bitcast i8* %"&pSB[currWI].offset124.i" to float*
  %loadedValue126.i = load float* %CastToValueType125.i, align 4
  store float %loadedValue126.i, float addrspace(3)* %107, align 4
  %check.WI.iter426.i = icmp ult i64 %CurrWI..4.i, %34
  br i1 %check.WI.iter426.i, label %thenBB423.i, label %SyncBB384.i

thenBB423.i:                                      ; preds = %SyncBB383.i
  %"CurrWI++427.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride429.i" = add nuw i64 %CurrSBIndex..4.i, 1792
  br label %SyncBB383.i

SyncBB384.i:                                      ; preds = %thenBB430.i, %SyncBB383.i
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride436.i", %thenBB430.i ], [ 0, %SyncBB383.i ]
  %CurrWI..5.i = phi i64 [ %"CurrWI++434.i", %thenBB430.i ], [ 0, %SyncBB383.i ]
  %"&(pSB[currWI].offset)17612.i" = or i64 %CurrSBIndex..5.i, 64
  %"&pSB[currWI].offset177.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)17612.i"
  %CastToValueType178.i = bitcast i8* %"&pSB[currWI].offset177.i" to i32*
  %loadedValue179.i = load i32* %CastToValueType178.i, align 4
  %108 = add nsw i32 %loadedValue179.i, -1
  %109 = sext i32 %108 to i64
  %110 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %109
  %"&(pSB[currWI].offset)26513.i" = or i64 %CurrSBIndex..5.i, 80
  %"&pSB[currWI].offset266.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)26513.i"
  %CastToValueType267.i = bitcast i8* %"&pSB[currWI].offset266.i" to float addrspace(3)**
  store float addrspace(3)* %110, float addrspace(3)** %CastToValueType267.i, align 8
  %111 = load float addrspace(3)* %110, align 4
  %"&(pSB[currWI].offset)27414.i" = or i64 %CurrSBIndex..5.i, 88
  %"&pSB[currWI].offset275.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)27414.i"
  %CastToValueType276.i = bitcast i8* %"&pSB[currWI].offset275.i" to float*
  store float %111, float* %CastToValueType276.i, align 4
  %check.WI.iter433.i = icmp ult i64 %CurrWI..5.i, %34
  br i1 %check.WI.iter433.i, label %thenBB430.i, label %SyncBB385.i

thenBB430.i:                                      ; preds = %SyncBB384.i
  %"CurrWI++434.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride436.i" = add nuw i64 %CurrSBIndex..5.i, 1792
  br label %SyncBB384.i

SyncBB385.i:                                      ; preds = %thenBB472.i, %SyncBB384.i
  %CurrSBIndex..11.i = phi i64 [ %"loadedCurrSB+Stride478.i", %thenBB472.i ], [ 0, %SyncBB384.i ]
  %CurrWI..11.i = phi i64 [ %"CurrWI++476.i", %thenBB472.i ], [ 0, %SyncBB384.i ]
  %"&(pSB[currWI].offset)26015.i" = or i64 %CurrSBIndex..11.i, 72
  %"&pSB[currWI].offset261.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)26015.i"
  %CastToValueType262.i = bitcast i8* %"&pSB[currWI].offset261.i" to float addrspace(3)**
  %loadedValue263.i = load float addrspace(3)** %CastToValueType262.i, align 8
  %112 = load float addrspace(3)* %loadedValue263.i, align 4
  %"&(pSB[currWI].offset)27816.i" = or i64 %CurrSBIndex..11.i, 88
  %"&pSB[currWI].offset279.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)27816.i"
  %CastToValueType280.i = bitcast i8* %"&pSB[currWI].offset279.i" to float*
  %loadedValue281.i = load float* %CastToValueType280.i, align 4
  %113 = fadd float %112, %loadedValue281.i
  %"&(pSB[currWI].offset)25517.i" = or i64 %CurrSBIndex..11.i, 72
  %"&pSB[currWI].offset256.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)25517.i"
  %CastToValueType257.i = bitcast i8* %"&pSB[currWI].offset256.i" to float addrspace(3)**
  %loadedValue258.i = load float addrspace(3)** %CastToValueType257.i, align 8
  store float %113, float addrspace(3)* %loadedValue258.i, align 4
  %check.WI.iter475.i = icmp ult i64 %CurrWI..11.i, %34
  br i1 %check.WI.iter475.i, label %thenBB472.i, label %SyncBB391.i

thenBB472.i:                                      ; preds = %SyncBB385.i
  %"CurrWI++476.i" = add nuw i64 %CurrWI..11.i, 1
  %"loadedCurrSB+Stride478.i" = add nuw i64 %CurrSBIndex..11.i, 1792
  br label %SyncBB385.i

SyncBB391.i:                                      ; preds = %thenBB479.i, %SyncBB385.i
  %CurrSBIndex..12.i = phi i64 [ %"loadedCurrSB+Stride485.i", %thenBB479.i ], [ 0, %SyncBB385.i ]
  %CurrWI..12.i = phi i64 [ %"CurrWI++483.i", %thenBB479.i ], [ 0, %SyncBB385.i ]
  %"&(pSB[currWI].offset)17118.i" = or i64 %CurrSBIndex..12.i, 64
  %"&pSB[currWI].offset172.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)17118.i"
  %CastToValueType173.i = bitcast i8* %"&pSB[currWI].offset172.i" to i32*
  %loadedValue174.i = load i32* %CastToValueType173.i, align 4
  %114 = add nsw i32 %loadedValue174.i, -2
  %115 = sext i32 %114 to i64
  %116 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %115
  %117 = load float addrspace(3)* %116, align 4
  %"&(pSB[currWI].offset)28319.i" = or i64 %CurrSBIndex..12.i, 92
  %"&pSB[currWI].offset284.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)28319.i"
  %CastToValueType285.i = bitcast i8* %"&pSB[currWI].offset284.i" to float*
  store float %117, float* %CastToValueType285.i, align 4
  %check.WI.iter482.i = icmp ult i64 %CurrWI..12.i, %34
  br i1 %check.WI.iter482.i, label %thenBB479.i, label %SyncBB392.i

thenBB479.i:                                      ; preds = %SyncBB391.i
  %"CurrWI++483.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride485.i" = add nuw i64 %CurrSBIndex..12.i, 1792
  br label %SyncBB391.i

SyncBB392.i:                                      ; preds = %thenBB486.i, %SyncBB391.i
  %CurrSBIndex..13.i = phi i64 [ %"loadedCurrSB+Stride492.i", %thenBB486.i ], [ 0, %SyncBB391.i ]
  %CurrWI..13.i = phi i64 [ %"CurrWI++490.i", %thenBB486.i ], [ 0, %SyncBB391.i ]
  %"&(pSB[currWI].offset)25020.i" = or i64 %CurrSBIndex..13.i, 72
  %"&pSB[currWI].offset251.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)25020.i"
  %CastToValueType252.i = bitcast i8* %"&pSB[currWI].offset251.i" to float addrspace(3)**
  %loadedValue253.i = load float addrspace(3)** %CastToValueType252.i, align 8
  %118 = load float addrspace(3)* %loadedValue253.i, align 4
  %"&(pSB[currWI].offset)28721.i" = or i64 %CurrSBIndex..13.i, 92
  %"&pSB[currWI].offset288.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)28721.i"
  %CastToValueType289.i = bitcast i8* %"&pSB[currWI].offset288.i" to float*
  %loadedValue290.i = load float* %CastToValueType289.i, align 4
  %119 = fadd float %118, %loadedValue290.i
  %"&(pSB[currWI].offset)24522.i" = or i64 %CurrSBIndex..13.i, 72
  %"&pSB[currWI].offset246.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)24522.i"
  %CastToValueType247.i = bitcast i8* %"&pSB[currWI].offset246.i" to float addrspace(3)**
  %loadedValue248.i = load float addrspace(3)** %CastToValueType247.i, align 8
  store float %119, float addrspace(3)* %loadedValue248.i, align 4
  %check.WI.iter489.i = icmp ult i64 %CurrWI..13.i, %34
  br i1 %check.WI.iter489.i, label %thenBB486.i, label %SyncBB393.i

thenBB486.i:                                      ; preds = %SyncBB392.i
  %"CurrWI++490.i" = add nuw i64 %CurrWI..13.i, 1
  %"loadedCurrSB+Stride492.i" = add nuw i64 %CurrSBIndex..13.i, 1792
  br label %SyncBB392.i

SyncBB393.i:                                      ; preds = %thenBB493.i, %SyncBB392.i
  %CurrSBIndex..14.i = phi i64 [ %"loadedCurrSB+Stride499.i", %thenBB493.i ], [ 0, %SyncBB392.i ]
  %CurrWI..14.i = phi i64 [ %"CurrWI++497.i", %thenBB493.i ], [ 0, %SyncBB392.i ]
  %"&(pSB[currWI].offset)16623.i" = or i64 %CurrSBIndex..14.i, 64
  %"&pSB[currWI].offset167.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)16623.i"
  %CastToValueType168.i = bitcast i8* %"&pSB[currWI].offset167.i" to i32*
  %loadedValue169.i = load i32* %CastToValueType168.i, align 4
  %120 = add nsw i32 %loadedValue169.i, -4
  %121 = sext i32 %120 to i64
  %122 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %121
  %123 = load float addrspace(3)* %122, align 4
  %"&(pSB[currWI].offset)29224.i" = or i64 %CurrSBIndex..14.i, 96
  %"&pSB[currWI].offset293.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)29224.i"
  %CastToValueType294.i = bitcast i8* %"&pSB[currWI].offset293.i" to float*
  store float %123, float* %CastToValueType294.i, align 4
  %check.WI.iter496.i = icmp ult i64 %CurrWI..14.i, %34
  br i1 %check.WI.iter496.i, label %thenBB493.i, label %SyncBB394.i

thenBB493.i:                                      ; preds = %SyncBB393.i
  %"CurrWI++497.i" = add nuw i64 %CurrWI..14.i, 1
  %"loadedCurrSB+Stride499.i" = add nuw i64 %CurrSBIndex..14.i, 1792
  br label %SyncBB393.i

SyncBB394.i:                                      ; preds = %thenBB437.i, %SyncBB393.i
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride443.i", %thenBB437.i ], [ 0, %SyncBB393.i ]
  %CurrWI..6.i = phi i64 [ %"CurrWI++441.i", %thenBB437.i ], [ 0, %SyncBB393.i ]
  %"&(pSB[currWI].offset)24025.i" = or i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset241.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)24025.i"
  %CastToValueType242.i = bitcast i8* %"&pSB[currWI].offset241.i" to float addrspace(3)**
  %loadedValue243.i = load float addrspace(3)** %CastToValueType242.i, align 8
  %124 = load float addrspace(3)* %loadedValue243.i, align 4
  %"&(pSB[currWI].offset)29626.i" = or i64 %CurrSBIndex..6.i, 96
  %"&pSB[currWI].offset297.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)29626.i"
  %CastToValueType298.i = bitcast i8* %"&pSB[currWI].offset297.i" to float*
  %loadedValue299.i = load float* %CastToValueType298.i, align 4
  %125 = fadd float %124, %loadedValue299.i
  %"&(pSB[currWI].offset)23527.i" = or i64 %CurrSBIndex..6.i, 72
  %"&pSB[currWI].offset236.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)23527.i"
  %CastToValueType237.i = bitcast i8* %"&pSB[currWI].offset236.i" to float addrspace(3)**
  %loadedValue238.i = load float addrspace(3)** %CastToValueType237.i, align 8
  store float %125, float addrspace(3)* %loadedValue238.i, align 4
  %check.WI.iter440.i = icmp ult i64 %CurrWI..6.i, %34
  br i1 %check.WI.iter440.i, label %thenBB437.i, label %SyncBB386.i

thenBB437.i:                                      ; preds = %SyncBB394.i
  %"CurrWI++441.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride443.i" = add nuw i64 %CurrSBIndex..6.i, 1792
  br label %SyncBB394.i

SyncBB386.i:                                      ; preds = %thenBB444.i, %SyncBB394.i
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride450.i", %thenBB444.i ], [ 0, %SyncBB394.i ]
  %CurrWI..7.i = phi i64 [ %"CurrWI++448.i", %thenBB444.i ], [ 0, %SyncBB394.i ]
  %"&(pSB[currWI].offset)16128.i" = or i64 %CurrSBIndex..7.i, 64
  %"&pSB[currWI].offset162.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)16128.i"
  %CastToValueType163.i = bitcast i8* %"&pSB[currWI].offset162.i" to i32*
  %loadedValue164.i = load i32* %CastToValueType163.i, align 4
  %126 = add nsw i32 %loadedValue164.i, -8
  %127 = sext i32 %126 to i64
  %128 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %127
  %129 = load float addrspace(3)* %128, align 4
  %"&(pSB[currWI].offset)30129.i" = or i64 %CurrSBIndex..7.i, 100
  %"&pSB[currWI].offset302.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)30129.i"
  %CastToValueType303.i = bitcast i8* %"&pSB[currWI].offset302.i" to float*
  store float %129, float* %CastToValueType303.i, align 4
  %check.WI.iter447.i = icmp ult i64 %CurrWI..7.i, %34
  br i1 %check.WI.iter447.i, label %thenBB444.i, label %SyncBB387.i

thenBB444.i:                                      ; preds = %SyncBB386.i
  %"CurrWI++448.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride450.i" = add nuw i64 %CurrSBIndex..7.i, 1792
  br label %SyncBB386.i

SyncBB387.i:                                      ; preds = %thenBB451.i, %SyncBB386.i
  %CurrSBIndex..8.i = phi i64 [ %"loadedCurrSB+Stride457.i", %thenBB451.i ], [ 0, %SyncBB386.i ]
  %CurrWI..8.i = phi i64 [ %"CurrWI++455.i", %thenBB451.i ], [ 0, %SyncBB386.i ]
  %"&(pSB[currWI].offset)23030.i" = or i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset231.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)23030.i"
  %CastToValueType232.i = bitcast i8* %"&pSB[currWI].offset231.i" to float addrspace(3)**
  %loadedValue233.i = load float addrspace(3)** %CastToValueType232.i, align 8
  %130 = load float addrspace(3)* %loadedValue233.i, align 4
  %"&(pSB[currWI].offset)30531.i" = or i64 %CurrSBIndex..8.i, 100
  %"&pSB[currWI].offset306.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)30531.i"
  %CastToValueType307.i = bitcast i8* %"&pSB[currWI].offset306.i" to float*
  %loadedValue308.i = load float* %CastToValueType307.i, align 4
  %131 = fadd float %130, %loadedValue308.i
  %"&(pSB[currWI].offset)22532.i" = or i64 %CurrSBIndex..8.i, 72
  %"&pSB[currWI].offset226.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)22532.i"
  %CastToValueType227.i = bitcast i8* %"&pSB[currWI].offset226.i" to float addrspace(3)**
  %loadedValue228.i = load float addrspace(3)** %CastToValueType227.i, align 8
  store float %131, float addrspace(3)* %loadedValue228.i, align 4
  %check.WI.iter454.i = icmp ult i64 %CurrWI..8.i, %34
  br i1 %check.WI.iter454.i, label %thenBB451.i, label %SyncBB388.i

thenBB451.i:                                      ; preds = %SyncBB387.i
  %"CurrWI++455.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride457.i" = add nuw i64 %CurrSBIndex..8.i, 1792
  br label %SyncBB387.i

SyncBB388.i:                                      ; preds = %thenBB458.i, %SyncBB387.i
  %CurrSBIndex..9.i = phi i64 [ %"loadedCurrSB+Stride464.i", %thenBB458.i ], [ 0, %SyncBB387.i ]
  %CurrWI..9.i = phi i64 [ %"CurrWI++462.i", %thenBB458.i ], [ 0, %SyncBB387.i ]
  %"&(pSB[currWI].offset)15633.i" = or i64 %CurrSBIndex..9.i, 64
  %"&pSB[currWI].offset157.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)15633.i"
  %CastToValueType158.i = bitcast i8* %"&pSB[currWI].offset157.i" to i32*
  %loadedValue159.i = load i32* %CastToValueType158.i, align 4
  %132 = add nsw i32 %loadedValue159.i, -16
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %133
  %135 = load float addrspace(3)* %134, align 4
  %"&(pSB[currWI].offset)31034.i" = or i64 %CurrSBIndex..9.i, 104
  %"&pSB[currWI].offset311.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)31034.i"
  %CastToValueType312.i = bitcast i8* %"&pSB[currWI].offset311.i" to float*
  store float %135, float* %CastToValueType312.i, align 4
  %check.WI.iter461.i = icmp ult i64 %CurrWI..9.i, %34
  br i1 %check.WI.iter461.i, label %thenBB458.i, label %SyncBB389.i

thenBB458.i:                                      ; preds = %SyncBB388.i
  %"CurrWI++462.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride464.i" = add nuw i64 %CurrSBIndex..9.i, 1792
  br label %SyncBB388.i

SyncBB389.i:                                      ; preds = %thenBB465.i, %SyncBB388.i
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride471.i", %thenBB465.i ], [ 0, %SyncBB388.i ]
  %CurrWI..10.i = phi i64 [ %"CurrWI++469.i", %thenBB465.i ], [ 0, %SyncBB388.i ]
  %"&(pSB[currWI].offset)22035.i" = or i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset221.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)22035.i"
  %CastToValueType222.i = bitcast i8* %"&pSB[currWI].offset221.i" to float addrspace(3)**
  %loadedValue223.i = load float addrspace(3)** %CastToValueType222.i, align 8
  %136 = load float addrspace(3)* %loadedValue223.i, align 4
  %"&(pSB[currWI].offset)31436.i" = or i64 %CurrSBIndex..10.i, 104
  %"&pSB[currWI].offset315.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)31436.i"
  %CastToValueType316.i = bitcast i8* %"&pSB[currWI].offset315.i" to float*
  %loadedValue317.i = load float* %CastToValueType316.i, align 4
  %137 = fadd float %136, %loadedValue317.i
  %"&(pSB[currWI].offset)21537.i" = or i64 %CurrSBIndex..10.i, 72
  %"&pSB[currWI].offset216.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)21537.i"
  %CastToValueType217.i = bitcast i8* %"&pSB[currWI].offset216.i" to float addrspace(3)**
  %loadedValue218.i = load float addrspace(3)** %CastToValueType217.i, align 8
  store float %137, float addrspace(3)* %loadedValue218.i, align 4
  %check.WI.iter468.i = icmp ult i64 %CurrWI..10.i, %34
  br i1 %check.WI.iter468.i, label %thenBB465.i, label %SyncBB390.i

thenBB465.i:                                      ; preds = %SyncBB389.i
  %"CurrWI++469.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride471.i" = add nuw i64 %CurrSBIndex..10.i, 1792
  br label %SyncBB389.i

SyncBB390.i:                                      ; preds = %thenBB500.i, %SyncBB389.i
  %CurrSBIndex..15.i = phi i64 [ %"loadedCurrSB+Stride506.i", %thenBB500.i ], [ 0, %SyncBB389.i ]
  %CurrWI..15.i = phi i64 [ %"CurrWI++504.i", %thenBB500.i ], [ 0, %SyncBB389.i ]
  %"&(pSB[currWI].offset)15138.i" = or i64 %CurrSBIndex..15.i, 64
  %"&pSB[currWI].offset152.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)15138.i"
  %CastToValueType153.i = bitcast i8* %"&pSB[currWI].offset152.i" to i32*
  %loadedValue154.i = load i32* %CastToValueType153.i, align 4
  %138 = add nsw i32 %loadedValue154.i, -32
  %139 = sext i32 %138 to i64
  %140 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %139
  %141 = load float addrspace(3)* %140, align 4
  %"&(pSB[currWI].offset)31939.i" = or i64 %CurrSBIndex..15.i, 108
  %"&pSB[currWI].offset320.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)31939.i"
  %CastToValueType321.i = bitcast i8* %"&pSB[currWI].offset320.i" to float*
  store float %141, float* %CastToValueType321.i, align 4
  %check.WI.iter503.i = icmp ult i64 %CurrWI..15.i, %34
  br i1 %check.WI.iter503.i, label %thenBB500.i, label %SyncBB395.i

thenBB500.i:                                      ; preds = %SyncBB390.i
  %"CurrWI++504.i" = add nuw i64 %CurrWI..15.i, 1
  %"loadedCurrSB+Stride506.i" = add nuw i64 %CurrSBIndex..15.i, 1792
  br label %SyncBB390.i

SyncBB395.i:                                      ; preds = %thenBB507.i, %SyncBB390.i
  %CurrSBIndex..16.i = phi i64 [ %"loadedCurrSB+Stride513.i", %thenBB507.i ], [ 0, %SyncBB390.i ]
  %CurrWI..16.i = phi i64 [ %"CurrWI++511.i", %thenBB507.i ], [ 0, %SyncBB390.i ]
  %"&(pSB[currWI].offset)21040.i" = or i64 %CurrSBIndex..16.i, 72
  %"&pSB[currWI].offset211.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)21040.i"
  %CastToValueType212.i = bitcast i8* %"&pSB[currWI].offset211.i" to float addrspace(3)**
  %loadedValue213.i = load float addrspace(3)** %CastToValueType212.i, align 8
  %142 = load float addrspace(3)* %loadedValue213.i, align 4
  %"&(pSB[currWI].offset)32341.i" = or i64 %CurrSBIndex..16.i, 108
  %"&pSB[currWI].offset324.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)32341.i"
  %CastToValueType325.i = bitcast i8* %"&pSB[currWI].offset324.i" to float*
  %loadedValue326.i = load float* %CastToValueType325.i, align 4
  %143 = fadd float %142, %loadedValue326.i
  %"&(pSB[currWI].offset)20542.i" = or i64 %CurrSBIndex..16.i, 72
  %"&pSB[currWI].offset206.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)20542.i"
  %CastToValueType207.i = bitcast i8* %"&pSB[currWI].offset206.i" to float addrspace(3)**
  %loadedValue208.i = load float addrspace(3)** %CastToValueType207.i, align 8
  store float %143, float addrspace(3)* %loadedValue208.i, align 4
  %check.WI.iter510.i = icmp ult i64 %CurrWI..16.i, %34
  br i1 %check.WI.iter510.i, label %thenBB507.i, label %SyncBB396.i

thenBB507.i:                                      ; preds = %SyncBB395.i
  %"CurrWI++511.i" = add nuw i64 %CurrWI..16.i, 1
  %"loadedCurrSB+Stride513.i" = add nuw i64 %CurrSBIndex..16.i, 1792
  br label %SyncBB395.i

SyncBB396.i:                                      ; preds = %thenBB514.i, %SyncBB395.i
  %CurrSBIndex..17.i = phi i64 [ %"loadedCurrSB+Stride520.i", %thenBB514.i ], [ 0, %SyncBB395.i ]
  %CurrWI..17.i = phi i64 [ %"CurrWI++518.i", %thenBB514.i ], [ 0, %SyncBB395.i ]
  %"&(pSB[currWI].offset)14643.i" = or i64 %CurrSBIndex..17.i, 64
  %"&pSB[currWI].offset147.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)14643.i"
  %CastToValueType148.i = bitcast i8* %"&pSB[currWI].offset147.i" to i32*
  %loadedValue149.i = load i32* %CastToValueType148.i, align 4
  %144 = add nsw i32 %loadedValue149.i, -64
  %145 = sext i32 %144 to i64
  %146 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %145
  %147 = load float addrspace(3)* %146, align 4
  %"&(pSB[currWI].offset)32844.i" = or i64 %CurrSBIndex..17.i, 112
  %"&pSB[currWI].offset329.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)32844.i"
  %CastToValueType330.i = bitcast i8* %"&pSB[currWI].offset329.i" to float*
  store float %147, float* %CastToValueType330.i, align 4
  %check.WI.iter517.i = icmp ult i64 %CurrWI..17.i, %34
  br i1 %check.WI.iter517.i, label %thenBB514.i, label %SyncBB397.i

thenBB514.i:                                      ; preds = %SyncBB396.i
  %"CurrWI++518.i" = add nuw i64 %CurrWI..17.i, 1
  %"loadedCurrSB+Stride520.i" = add nuw i64 %CurrSBIndex..17.i, 1792
  br label %SyncBB396.i

SyncBB397.i:                                      ; preds = %thenBB521.i, %SyncBB396.i
  %CurrSBIndex..18.i = phi i64 [ %"loadedCurrSB+Stride527.i", %thenBB521.i ], [ 0, %SyncBB396.i ]
  %CurrWI..18.i = phi i64 [ %"CurrWI++525.i", %thenBB521.i ], [ 0, %SyncBB396.i ]
  %"&(pSB[currWI].offset)20045.i" = or i64 %CurrSBIndex..18.i, 72
  %"&pSB[currWI].offset201.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)20045.i"
  %CastToValueType202.i = bitcast i8* %"&pSB[currWI].offset201.i" to float addrspace(3)**
  %loadedValue203.i = load float addrspace(3)** %CastToValueType202.i, align 8
  %148 = load float addrspace(3)* %loadedValue203.i, align 4
  %"&(pSB[currWI].offset)33246.i" = or i64 %CurrSBIndex..18.i, 112
  %"&pSB[currWI].offset333.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)33246.i"
  %CastToValueType334.i = bitcast i8* %"&pSB[currWI].offset333.i" to float*
  %loadedValue335.i = load float* %CastToValueType334.i, align 4
  %149 = fadd float %148, %loadedValue335.i
  %"&(pSB[currWI].offset)19547.i" = or i64 %CurrSBIndex..18.i, 72
  %"&pSB[currWI].offset196.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)19547.i"
  %CastToValueType197.i = bitcast i8* %"&pSB[currWI].offset196.i" to float addrspace(3)**
  %loadedValue198.i = load float addrspace(3)** %CastToValueType197.i, align 8
  store float %149, float addrspace(3)* %loadedValue198.i, align 4
  %check.WI.iter524.i = icmp ult i64 %CurrWI..18.i, %34
  br i1 %check.WI.iter524.i, label %thenBB521.i, label %SyncBB398.i

thenBB521.i:                                      ; preds = %SyncBB397.i
  %"CurrWI++525.i" = add nuw i64 %CurrWI..18.i, 1
  %"loadedCurrSB+Stride527.i" = add nuw i64 %CurrSBIndex..18.i, 1792
  br label %SyncBB397.i

SyncBB398.i:                                      ; preds = %thenBB528.i, %SyncBB397.i
  %CurrSBIndex..19.i = phi i64 [ %"loadedCurrSB+Stride534.i", %thenBB528.i ], [ 0, %SyncBB397.i ]
  %CurrWI..19.i = phi i64 [ %"CurrWI++532.i", %thenBB528.i ], [ 0, %SyncBB397.i ]
  %"&(pSB[currWI].offset)14148.i" = or i64 %CurrSBIndex..19.i, 64
  %"&pSB[currWI].offset142.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)14148.i"
  %CastToValueType143.i = bitcast i8* %"&pSB[currWI].offset142.i" to i32*
  %loadedValue144.i = load i32* %CastToValueType143.i, align 4
  %150 = add nsw i32 %loadedValue144.i, -128
  %151 = sext i32 %150 to i64
  %152 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %151
  %153 = load float addrspace(3)* %152, align 4
  %"&(pSB[currWI].offset)33749.i" = or i64 %CurrSBIndex..19.i, 116
  %"&pSB[currWI].offset338.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)33749.i"
  %CastToValueType339.i = bitcast i8* %"&pSB[currWI].offset338.i" to float*
  store float %153, float* %CastToValueType339.i, align 4
  %check.WI.iter531.i = icmp ult i64 %CurrWI..19.i, %34
  br i1 %check.WI.iter531.i, label %thenBB528.i, label %SyncBB399.i

thenBB528.i:                                      ; preds = %SyncBB398.i
  %"CurrWI++532.i" = add nuw i64 %CurrWI..19.i, 1
  %"loadedCurrSB+Stride534.i" = add nuw i64 %CurrSBIndex..19.i, 1792
  br label %SyncBB398.i

SyncBB399.i:                                      ; preds = %thenBB535.i, %SyncBB398.i
  %CurrSBIndex..20.i = phi i64 [ %"loadedCurrSB+Stride541.i", %thenBB535.i ], [ 0, %SyncBB398.i ]
  %CurrWI..20.i = phi i64 [ %"CurrWI++539.i", %thenBB535.i ], [ 0, %SyncBB398.i ]
  %"&(pSB[currWI].offset)19050.i" = or i64 %CurrSBIndex..20.i, 72
  %"&pSB[currWI].offset191.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)19050.i"
  %CastToValueType192.i = bitcast i8* %"&pSB[currWI].offset191.i" to float addrspace(3)**
  %loadedValue193.i = load float addrspace(3)** %CastToValueType192.i, align 8
  %154 = load float addrspace(3)* %loadedValue193.i, align 4
  %"&(pSB[currWI].offset)34151.i" = or i64 %CurrSBIndex..20.i, 116
  %"&pSB[currWI].offset342.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)34151.i"
  %CastToValueType343.i = bitcast i8* %"&pSB[currWI].offset342.i" to float*
  %loadedValue344.i = load float* %CastToValueType343.i, align 4
  %155 = fadd float %154, %loadedValue344.i
  %"&(pSB[currWI].offset)18552.i" = or i64 %CurrSBIndex..20.i, 72
  %"&pSB[currWI].offset186.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)18552.i"
  %CastToValueType187.i = bitcast i8* %"&pSB[currWI].offset186.i" to float addrspace(3)**
  %loadedValue188.i = load float addrspace(3)** %CastToValueType187.i, align 8
  store float %155, float addrspace(3)* %loadedValue188.i, align 4
  %check.WI.iter538.i = icmp ult i64 %CurrWI..20.i, %34
  br i1 %check.WI.iter538.i, label %thenBB535.i, label %SyncBB400.i

thenBB535.i:                                      ; preds = %SyncBB399.i
  %"CurrWI++539.i" = add nuw i64 %CurrWI..20.i, 1
  %"loadedCurrSB+Stride541.i" = add nuw i64 %CurrSBIndex..20.i, 1792
  br label %SyncBB399.i

SyncBB400.i:                                      ; preds = %thenBB409.i, %SyncBB399.i
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride415.i", %thenBB409.i ], [ 0, %SyncBB399.i ]
  %CurrWI..2.i = phi i64 [ %"CurrWI++413.i", %thenBB409.i ], [ 0, %SyncBB399.i ]
  %"&(pSB[currWI].offset)26953.i" = or i64 %CurrSBIndex..2.i, 80
  %"&pSB[currWI].offset270.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)26953.i"
  %CastToValueType271.i = bitcast i8* %"&pSB[currWI].offset270.i" to float addrspace(3)**
  %loadedValue272.i = load float addrspace(3)** %CastToValueType271.i, align 8
  %156 = load float addrspace(3)* %loadedValue272.i, align 4
  %"&(pSB[currWI].offset)34654.i" = or i64 %CurrSBIndex..2.i, 120
  %"&pSB[currWI].offset347.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)34654.i"
  %CastToValueType348.i = bitcast i8* %"&pSB[currWI].offset347.i" to float*
  store float %156, float* %CastToValueType348.i, align 4
  %check.WI.iter412.i = icmp ult i64 %CurrWI..2.i, %34
  br i1 %check.WI.iter412.i, label %thenBB409.i, label %elseBB410.i

thenBB409.i:                                      ; preds = %SyncBB400.i
  %"CurrWI++413.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride415.i" = add nuw i64 %CurrSBIndex..2.i, 1792
  br label %SyncBB400.i

elseBB410.i:                                      ; preds = %SyncBB400.i
  %157 = icmp eq i32 %16, 0
  %158 = bitcast float addrspace(1)* %1 to <4 x float> addrspace(1)*
  br label %SyncBB381.i

SyncBB381.i:                                      ; preds = %thenBB.i, %elseBB410.i
  %CurrSBIndex..0.i = phi i64 [ 0, %elseBB410.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %elseBB410.i ], [ %"CurrWI++.i", %thenBB.i ]
  br i1 %157, label %phi-split-bb.i, label %159

; <label>:159                                     ; preds = %SyncBB381.i
  %"&(pSB[currWI].offset)3655.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset37.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)3655.i"
  %CastToValueType38.i = bitcast i8* %"&pSB[currWI].offset37.i" to i64*
  %loadedValue39.i = load i64* %CastToValueType38.i, align 8
  %sext.i = shl i64 %loadedValue39.i, 32
  %160 = ashr i64 %sext.i, 32
  %161 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %162 = load i64* %161, align 8
  %163 = add i64 %162, -1
  %164 = icmp eq i64 %160, %163
  br i1 %164, label %165, label %phi-split-bb.i

; <label>:165                                     ; preds = %159
  %"&(pSB[currWI].offset)11470.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset115.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)11470.i"
  %CastToValueType116.i = bitcast i8* %"&pSB[currWI].offset115.i" to <4 x float>*
  %loadedValue117.i = load <4 x float>* %CastToValueType116.i, align 16
  %166 = extractelement <4 x float> %loadedValue117.i, i32 3
  %"&(pSB[currWI].offset)37571.i" = or i64 %CurrSBIndex..0.i, 120
  %"&pSB[currWI].offset376.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)37571.i"
  %CastToValueType377.i = bitcast i8* %"&pSB[currWI].offset376.i" to float*
  %loadedValue378.i = load float* %CastToValueType377.i, align 4
  %167 = fadd float %loadedValue378.i, %166
  %168 = load i64* %25, align 8
  %169 = getelementptr inbounds float addrspace(1)* %7, i64 %168
  store float %167, float addrspace(1)* %169, align 4
  br label %phi-split-bb.i

phi-split-bb.i:                                   ; preds = %165, %159, %SyncBB381.i
  %"&(pSB[currWI].offset)37056.i" = or i64 %CurrSBIndex..0.i, 120
  %"&pSB[currWI].offset371.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)37056.i"
  %CastToValueType372.i = bitcast i8* %"&pSB[currWI].offset371.i" to float*
  %loadedValue373.i = load float* %CastToValueType372.i, align 4
  %170 = insertelement <4 x float> undef, float %loadedValue373.i, i32 0
  %"&(pSB[currWI].offset)10957.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset110.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)10957.i"
  %CastToValueType111.i = bitcast i8* %"&pSB[currWI].offset110.i" to <4 x float>*
  %loadedValue112.i = load <4 x float>* %CastToValueType111.i, align 16
  %171 = extractelement <4 x float> %loadedValue112.i, i32 0
  %"&(pSB[currWI].offset)36558.i" = or i64 %CurrSBIndex..0.i, 120
  %"&pSB[currWI].offset366.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)36558.i"
  %CastToValueType367.i = bitcast i8* %"&pSB[currWI].offset366.i" to float*
  %loadedValue368.i = load float* %CastToValueType367.i, align 4
  %172 = fadd float %loadedValue368.i, %171
  %173 = insertelement <4 x float> %170, float %172, i32 1
  %"&(pSB[currWI].offset)10459.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset105.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)10459.i"
  %CastToValueType106.i = bitcast i8* %"&pSB[currWI].offset105.i" to <4 x float>*
  %loadedValue107.i = load <4 x float>* %CastToValueType106.i, align 16
  %174 = extractelement <4 x float> %loadedValue107.i, i32 1
  %"&(pSB[currWI].offset)36060.i" = or i64 %CurrSBIndex..0.i, 120
  %"&pSB[currWI].offset361.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)36060.i"
  %CastToValueType362.i = bitcast i8* %"&pSB[currWI].offset361.i" to float*
  %loadedValue363.i = load float* %CastToValueType362.i, align 4
  %175 = fadd float %loadedValue363.i, %174
  %176 = insertelement <4 x float> %173, float %175, i32 2
  %"&(pSB[currWI].offset)9961.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset100.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)9961.i"
  %CastToValueType101.i = bitcast i8* %"&pSB[currWI].offset100.i" to <4 x float>*
  %loadedValue102.i = load <4 x float>* %CastToValueType101.i, align 16
  %177 = extractelement <4 x float> %loadedValue102.i, i32 2
  %"&(pSB[currWI].offset)35562.i" = or i64 %CurrSBIndex..0.i, 120
  %"&pSB[currWI].offset356.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)35562.i"
  %CastToValueType357.i = bitcast i8* %"&pSB[currWI].offset356.i" to float*
  %loadedValue358.i = load float* %CastToValueType357.i, align 4
  %178 = fadd float %loadedValue358.i, %177
  %179 = insertelement <4 x float> %176, float %178, i32 3
  br i1 %40, label %180, label %183

; <label>:180                                     ; preds = %phi-split-bb.i
  %"&(pSB[currWI].offset)5564.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset56.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)5564.i"
  %CastToValueType57.i = bitcast i8* %"&pSB[currWI].offset56.i" to i32*
  %loadedValue58.i = load i32* %CastToValueType57.i, align 4
  %181 = or i32 %loadedValue58.i, 3
  %182 = icmp slt i32 %181, %10
  br i1 %182, label %183, label %186

; <label>:183                                     ; preds = %180, %phi-split-bb.i
  %"&(pSB[currWI].offset)2763.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2763.i"
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i32*
  %loadedValue30.i = load i32* %CastToValueType29.i, align 4
  %184 = sext i32 %loadedValue30.i to i64
  %185 = getelementptr inbounds <4 x float> addrspace(1)* %158, i64 %184
  store <4 x float> %179, <4 x float> addrspace(1)* %185, align 16
  br label %UnifiedReturnBlock.i

; <label>:186                                     ; preds = %180
  %"&(pSB[currWI].offset)8065.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset81.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)8065.i"
  %CastToValueType82.i = bitcast i8* %"&pSB[currWI].offset81.i" to i32*
  %loadedValue83.i = load i32* %CastToValueType82.i, align 4
  %187 = icmp slt i32 %loadedValue83.i, %10
  br i1 %187, label %188, label %UnifiedReturnBlock.i

; <label>:188                                     ; preds = %186
  %"&(pSB[currWI].offset)7566.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset76.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)7566.i"
  %CastToValueType77.i = bitcast i8* %"&pSB[currWI].offset76.i" to i32*
  %loadedValue78.i = load i32* %CastToValueType77.i, align 4
  %189 = sext i32 %loadedValue78.i to i64
  %190 = getelementptr inbounds float addrspace(1)* %1, i64 %189
  %"&(pSB[currWI].offset)35067.i" = or i64 %CurrSBIndex..0.i, 120
  %"&pSB[currWI].offset351.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)35067.i"
  %CastToValueType352.i = bitcast i8* %"&pSB[currWI].offset351.i" to float*
  %loadedValue353.i = load float* %CastToValueType352.i, align 4
  store float %loadedValue353.i, float addrspace(1)* %190, align 4
  %"&(pSB[currWI].offset)5068.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset51.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)5068.i"
  %CastToValueType52.i = bitcast i8* %"&pSB[currWI].offset51.i" to i32*
  %loadedValue53.i = load i32* %CastToValueType52.i, align 4
  %191 = or i32 %loadedValue53.i, 1
  %192 = icmp slt i32 %191, %10
  br i1 %192, label %193, label %UnifiedReturnBlock.i

; <label>:193                                     ; preds = %188
  %194 = sext i32 %191 to i64
  %195 = getelementptr inbounds float addrspace(1)* %1, i64 %194
  store float %172, float addrspace(1)* %195, align 4
  %"&(pSB[currWI].offset)4569.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset46.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)4569.i"
  %CastToValueType47.i = bitcast i8* %"&pSB[currWI].offset46.i" to i32*
  %loadedValue48.i = load i32* %CastToValueType47.i, align 4
  %196 = or i32 %loadedValue48.i, 2
  %197 = icmp slt i32 %196, %10
  br i1 %197, label %198, label %UnifiedReturnBlock.i

; <label>:198                                     ; preds = %193
  %199 = sext i32 %196 to i64
  %200 = getelementptr inbounds float addrspace(1)* %1, i64 %199
  store float %175, float addrspace(1)* %200, align 4
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %198, %193, %188, %186, %183
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %__scan_separated_args.exit

thenBB.i:                                         ; preds = %UnifiedReturnBlock.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  br label %SyncBB381.i

__scan_separated_args.exit:                       ; preds = %UnifiedReturnBlock.i
  ret void
}

define void @__Vectorized_.scan(i8* %pBuffer) {
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
  %17 = getelementptr i8* %pBuffer, i64 40
  %18 = bitcast i8* %17 to i8 addrspace(3)**
  %19 = load i8 addrspace(3)** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 48
  %21 = bitcast i8* %20 to %struct.WorkDim**
  %22 = load %struct.WorkDim** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 56
  %24 = bitcast i8* %23 to i64**
  %25 = load i64** %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 64
  %27 = bitcast i8* %26 to %struct.PaddedDimId**
  %28 = load %struct.PaddedDimId** %27, align 8
  %29 = getelementptr i8* %pBuffer, i64 72
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 88
  %33 = bitcast i8* %32 to i64*
  %34 = load i64* %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 96
  %36 = bitcast i8* %35 to i8**
  %37 = load i8** %36, align 8
  %38 = bitcast i8 addrspace(3)* %19 to [512 x float] addrspace(3)*
  %temp.i = insertelement <16 x i32> undef, i32 %10, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %39 = bitcast float addrspace(1)* %4 to <4 x float> addrspace(1)*
  %40 = icmp eq i32 %13, 0
  %Mneg.i = xor i1 %40, true
  %temp186.i = insertelement <16 x i1> undef, i1 %Mneg.i, i32 0
  %vector187.i = shufflevector <16 x i1> %temp186.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %temp181.i = insertelement <16 x i1> undef, i1 %40, i32 0
  %vector182.i = shufflevector <16 x i1> %temp181.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB2176.i

SyncBB2176.i:                                     ; preds = %thenBB2198.i, %entry
  %CurrSBIndex..1.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride2204.i", %thenBB2198.i ]
  %CurrWI..1.i = phi i64 [ 0, %entry ], [ %"CurrWI++2202.i", %thenBB2198.i ]
  %41 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..1.i, i32 0, i64 0
  %42 = load i64* %41, align 8
  %43 = getelementptr %struct.PaddedDimId* %28, i64 0, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = add i64 %42, %44
  %broadcast1.i = insertelement <16 x i64> undef, i64 %45, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %46 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %47 = trunc <16 x i64> %46 to <16 x i32>
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..1.i, 256
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %47, <16 x i32>* %CastToValueType.i, align 64
  %48 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..1.i, i32 0, i64 0
  %49 = load i64* %48, align 8
  %broadcast1178.i = insertelement <16 x i64> undef, i64 %49, i32 0
  %broadcast2179.i = shufflevector <16 x i64> %broadcast1178.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %50 = add <16 x i64> %broadcast2179.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)1447.i" = add nuw i64 %CurrSBIndex..1.i, 384
  %"&pSB[currWI].offset1448.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1447.i"
  %CastToValueType1449.i = bitcast i8* %"&pSB[currWI].offset1448.i" to <16 x i64>*
  store <16 x i64> %50, <16 x i64>* %CastToValueType1449.i, align 128
  %51 = shl <16 x i32> %47, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %"&(pSB[currWI].offset)1456.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1457.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1456.i"
  %CastToValueType1458.i = bitcast i8* %"&pSB[currWI].offset1457.i" to <16 x i32>*
  store <16 x i32> %51, <16 x i32>* %CastToValueType1458.i, align 64
  %52 = or <16 x i32> %51, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %53 = icmp slt <16 x i32> %52, %vector.i
  %Mneg56180.i = xor <16 x i1> %53, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %_to_59183.i = and <16 x i1> %vector182.i, %Mneg56180.i
  %_to_60184.i = and <16 x i1> %vector182.i, %53
  %_Min118188.i = or <16 x i1> %_to_60184.i, %vector187.i
  %extract204.i = extractelement <16 x i1> %_Min118188.i, i32 0
  %extract205.i = extractelement <16 x i1> %_Min118188.i, i32 1
  %extract206.i = extractelement <16 x i1> %_Min118188.i, i32 2
  %extract207.i = extractelement <16 x i1> %_Min118188.i, i32 3
  %extract208.i = extractelement <16 x i1> %_Min118188.i, i32 4
  %extract209.i = extractelement <16 x i1> %_Min118188.i, i32 5
  %extract210.i = extractelement <16 x i1> %_Min118188.i, i32 6
  %extract211.i = extractelement <16 x i1> %_Min118188.i, i32 7
  %extract212.i = extractelement <16 x i1> %_Min118188.i, i32 8
  %extract213.i = extractelement <16 x i1> %_Min118188.i, i32 9
  %extract214.i = extractelement <16 x i1> %_Min118188.i, i32 10
  %extract215.i = extractelement <16 x i1> %_Min118188.i, i32 11
  %extract216.i = extractelement <16 x i1> %_Min118188.i, i32 12
  %extract217.i = extractelement <16 x i1> %_Min118188.i, i32 13
  %extract218.i = extractelement <16 x i1> %_Min118188.i, i32 14
  %extract219.i = extractelement <16 x i1> %_Min118188.i, i32 15
  %54 = extractelement <16 x i32> %47, i32 1
  %55 = sext i32 %54 to i64
  %56 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %55
  %57 = extractelement <16 x i32> %47, i32 2
  %58 = sext i32 %57 to i64
  %59 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %58
  %60 = extractelement <16 x i32> %47, i32 3
  %61 = sext i32 %60 to i64
  %62 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %61
  %63 = extractelement <16 x i32> %47, i32 4
  %64 = sext i32 %63 to i64
  %65 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %64
  %66 = extractelement <16 x i32> %47, i32 5
  %67 = sext i32 %66 to i64
  %68 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %67
  %69 = extractelement <16 x i32> %47, i32 6
  %70 = sext i32 %69 to i64
  %71 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %70
  %72 = extractelement <16 x i32> %47, i32 7
  %73 = sext i32 %72 to i64
  %74 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %73
  %75 = extractelement <16 x i32> %47, i32 8
  %76 = sext i32 %75 to i64
  %77 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %76
  %78 = extractelement <16 x i32> %47, i32 9
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %79
  %81 = extractelement <16 x i32> %47, i32 10
  %82 = sext i32 %81 to i64
  %83 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %82
  %84 = extractelement <16 x i32> %47, i32 11
  %85 = sext i32 %84 to i64
  %86 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %85
  %87 = extractelement <16 x i32> %47, i32 12
  %88 = sext i32 %87 to i64
  %89 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %88
  %90 = extractelement <16 x i32> %47, i32 13
  %91 = sext i32 %90 to i64
  %92 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %91
  %93 = extractelement <16 x i32> %47, i32 14
  %94 = sext i32 %93 to i64
  %95 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %94
  %96 = extractelement <16 x i32> %47, i32 15
  %97 = sext i32 %96 to i64
  %98 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %97
  br i1 %extract204.i, label %preload944.i, label %postload945.i

preload944.i:                                     ; preds = %SyncBB2176.i
  %"&(pSB[currWI].offset)1442.i" = add nuw i64 %CurrSBIndex..1.i, 256
  %"&pSB[currWI].offset1443.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1442.i"
  %CastToValueType1444.i = bitcast i8* %"&pSB[currWI].offset1443.i" to <16 x i32>*
  %loadedValue1445.i = load <16 x i32>* %CastToValueType1444.i, align 64
  %99 = extractelement <16 x i32> %loadedValue1445.i, i32 0
  %100 = sext i32 %99 to i64
  %101 = getelementptr inbounds <4 x float> addrspace(1)* %39, i64 %100
  %masked_load.i = load <4 x float> addrspace(1)* %101, align 16
  br label %postload945.i

postload945.i:                                    ; preds = %preload944.i, %SyncBB2176.i
  %phi946.i = phi <4 x float> [ undef, %SyncBB2176.i ], [ %masked_load.i, %preload944.i ]
  br i1 %extract205.i, label %preload947.i, label %postload948.i

preload947.i:                                     ; preds = %postload945.i
  %masked_load872.i = load <4 x float> addrspace(1)* %56, align 16
  br label %postload948.i

postload948.i:                                    ; preds = %preload947.i, %postload945.i
  %phi949.i = phi <4 x float> [ undef, %postload945.i ], [ %masked_load872.i, %preload947.i ]
  br i1 %extract206.i, label %preload950.i, label %postload951.i

preload950.i:                                     ; preds = %postload948.i
  %masked_load873.i = load <4 x float> addrspace(1)* %59, align 16
  br label %postload951.i

postload951.i:                                    ; preds = %preload950.i, %postload948.i
  %phi952.i = phi <4 x float> [ undef, %postload948.i ], [ %masked_load873.i, %preload950.i ]
  br i1 %extract207.i, label %preload953.i, label %postload954.i

preload953.i:                                     ; preds = %postload951.i
  %masked_load874.i = load <4 x float> addrspace(1)* %62, align 16
  br label %postload954.i

postload954.i:                                    ; preds = %preload953.i, %postload951.i
  %phi955.i = phi <4 x float> [ undef, %postload951.i ], [ %masked_load874.i, %preload953.i ]
  br i1 %extract208.i, label %preload956.i, label %postload957.i

preload956.i:                                     ; preds = %postload954.i
  %masked_load875.i = load <4 x float> addrspace(1)* %65, align 16
  br label %postload957.i

postload957.i:                                    ; preds = %preload956.i, %postload954.i
  %phi958.i = phi <4 x float> [ undef, %postload954.i ], [ %masked_load875.i, %preload956.i ]
  br i1 %extract209.i, label %preload959.i, label %postload960.i

preload959.i:                                     ; preds = %postload957.i
  %masked_load876.i = load <4 x float> addrspace(1)* %68, align 16
  br label %postload960.i

postload960.i:                                    ; preds = %preload959.i, %postload957.i
  %phi961.i = phi <4 x float> [ undef, %postload957.i ], [ %masked_load876.i, %preload959.i ]
  br i1 %extract210.i, label %preload962.i, label %postload963.i

preload962.i:                                     ; preds = %postload960.i
  %masked_load877.i = load <4 x float> addrspace(1)* %71, align 16
  br label %postload963.i

postload963.i:                                    ; preds = %preload962.i, %postload960.i
  %phi964.i = phi <4 x float> [ undef, %postload960.i ], [ %masked_load877.i, %preload962.i ]
  br i1 %extract211.i, label %preload965.i, label %postload966.i

preload965.i:                                     ; preds = %postload963.i
  %masked_load878.i = load <4 x float> addrspace(1)* %74, align 16
  br label %postload966.i

postload966.i:                                    ; preds = %preload965.i, %postload963.i
  %phi967.i = phi <4 x float> [ undef, %postload963.i ], [ %masked_load878.i, %preload965.i ]
  br i1 %extract212.i, label %preload968.i, label %postload969.i

preload968.i:                                     ; preds = %postload966.i
  %masked_load879.i = load <4 x float> addrspace(1)* %77, align 16
  br label %postload969.i

postload969.i:                                    ; preds = %preload968.i, %postload966.i
  %phi970.i = phi <4 x float> [ undef, %postload966.i ], [ %masked_load879.i, %preload968.i ]
  br i1 %extract213.i, label %preload971.i, label %postload972.i

preload971.i:                                     ; preds = %postload969.i
  %masked_load880.i = load <4 x float> addrspace(1)* %80, align 16
  br label %postload972.i

postload972.i:                                    ; preds = %preload971.i, %postload969.i
  %phi973.i = phi <4 x float> [ undef, %postload969.i ], [ %masked_load880.i, %preload971.i ]
  br i1 %extract214.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload972.i
  %masked_load881.i = load <4 x float> addrspace(1)* %83, align 16
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload972.i
  %phi.i = phi <4 x float> [ undef, %postload972.i ], [ %masked_load881.i, %preload.i ]
  br i1 %extract215.i, label %preload935.i, label %postload936.i

preload935.i:                                     ; preds = %postload.i
  %masked_load882.i = load <4 x float> addrspace(1)* %86, align 16
  br label %postload936.i

postload936.i:                                    ; preds = %preload935.i, %postload.i
  %phi937.i = phi <4 x float> [ undef, %postload.i ], [ %masked_load882.i, %preload935.i ]
  br i1 %extract216.i, label %preload938.i, label %postload939.i

preload938.i:                                     ; preds = %postload936.i
  %masked_load883.i = load <4 x float> addrspace(1)* %89, align 16
  br label %postload939.i

postload939.i:                                    ; preds = %preload938.i, %postload936.i
  %phi940.i = phi <4 x float> [ undef, %postload936.i ], [ %masked_load883.i, %preload938.i ]
  br i1 %extract217.i, label %preload941.i, label %postload942.i

preload941.i:                                     ; preds = %postload939.i
  %masked_load884.i = load <4 x float> addrspace(1)* %92, align 16
  br label %postload942.i

postload942.i:                                    ; preds = %preload941.i, %postload939.i
  %phi943.i = phi <4 x float> [ undef, %postload939.i ], [ %masked_load884.i, %preload941.i ]
  br i1 %extract218.i, label %preload974.i, label %postload975.i

preload974.i:                                     ; preds = %postload942.i
  %masked_load885.i = load <4 x float> addrspace(1)* %95, align 16
  br label %postload975.i

postload975.i:                                    ; preds = %preload974.i, %postload942.i
  %phi976.i = phi <4 x float> [ undef, %postload942.i ], [ %masked_load885.i, %preload974.i ]
  br i1 %extract219.i, label %preload977.i, label %postload978.i

preload977.i:                                     ; preds = %postload975.i
  %masked_load886.i = load <4 x float> addrspace(1)* %98, align 16
  br label %postload978.i

postload978.i:                                    ; preds = %preload977.i, %postload975.i
  %phi979.i = phi <4 x float> [ undef, %postload975.i ], [ %masked_load886.i, %preload977.i ]
  %102 = extractelement <4 x float> %phi946.i, i32 0
  %103 = extractelement <4 x float> %phi949.i, i32 0
  %104 = extractelement <4 x float> %phi952.i, i32 0
  %105 = extractelement <4 x float> %phi955.i, i32 0
  %106 = extractelement <4 x float> %phi958.i, i32 0
  %107 = extractelement <4 x float> %phi961.i, i32 0
  %108 = extractelement <4 x float> %phi964.i, i32 0
  %109 = extractelement <4 x float> %phi967.i, i32 0
  %110 = extractelement <4 x float> %phi970.i, i32 0
  %111 = extractelement <4 x float> %phi973.i, i32 0
  %112 = extractelement <4 x float> %phi.i, i32 0
  %113 = extractelement <4 x float> %phi937.i, i32 0
  %114 = extractelement <4 x float> %phi940.i, i32 0
  %115 = extractelement <4 x float> %phi943.i, i32 0
  %116 = extractelement <4 x float> %phi976.i, i32 0
  %117 = extractelement <4 x float> %phi979.i, i32 0
  %temp.vect236.i = insertelement <16 x float> undef, float %102, i32 0
  %temp.vect237.i = insertelement <16 x float> %temp.vect236.i, float %103, i32 1
  %temp.vect238.i = insertelement <16 x float> %temp.vect237.i, float %104, i32 2
  %temp.vect239.i = insertelement <16 x float> %temp.vect238.i, float %105, i32 3
  %temp.vect240.i = insertelement <16 x float> %temp.vect239.i, float %106, i32 4
  %temp.vect241.i = insertelement <16 x float> %temp.vect240.i, float %107, i32 5
  %temp.vect242.i = insertelement <16 x float> %temp.vect241.i, float %108, i32 6
  %temp.vect243.i = insertelement <16 x float> %temp.vect242.i, float %109, i32 7
  %temp.vect244.i = insertelement <16 x float> %temp.vect243.i, float %110, i32 8
  %temp.vect245.i = insertelement <16 x float> %temp.vect244.i, float %111, i32 9
  %temp.vect246.i = insertelement <16 x float> %temp.vect245.i, float %112, i32 10
  %temp.vect247.i = insertelement <16 x float> %temp.vect246.i, float %113, i32 11
  %temp.vect248.i = insertelement <16 x float> %temp.vect247.i, float %114, i32 12
  %temp.vect249.i = insertelement <16 x float> %temp.vect248.i, float %115, i32 13
  %temp.vect250.i = insertelement <16 x float> %temp.vect249.i, float %116, i32 14
  %temp.vect251.i = insertelement <16 x float> %temp.vect250.i, float %117, i32 15
  %118 = extractelement <4 x float> %phi946.i, i32 1
  %119 = extractelement <4 x float> %phi949.i, i32 1
  %120 = extractelement <4 x float> %phi952.i, i32 1
  %121 = extractelement <4 x float> %phi955.i, i32 1
  %122 = extractelement <4 x float> %phi958.i, i32 1
  %123 = extractelement <4 x float> %phi961.i, i32 1
  %124 = extractelement <4 x float> %phi964.i, i32 1
  %125 = extractelement <4 x float> %phi967.i, i32 1
  %126 = extractelement <4 x float> %phi970.i, i32 1
  %127 = extractelement <4 x float> %phi973.i, i32 1
  %128 = extractelement <4 x float> %phi.i, i32 1
  %129 = extractelement <4 x float> %phi937.i, i32 1
  %130 = extractelement <4 x float> %phi940.i, i32 1
  %131 = extractelement <4 x float> %phi943.i, i32 1
  %132 = extractelement <4 x float> %phi976.i, i32 1
  %133 = extractelement <4 x float> %phi979.i, i32 1
  %temp.vect220.i = insertelement <16 x float> undef, float %118, i32 0
  %temp.vect221.i = insertelement <16 x float> %temp.vect220.i, float %119, i32 1
  %temp.vect222.i = insertelement <16 x float> %temp.vect221.i, float %120, i32 2
  %temp.vect223.i = insertelement <16 x float> %temp.vect222.i, float %121, i32 3
  %temp.vect224.i = insertelement <16 x float> %temp.vect223.i, float %122, i32 4
  %temp.vect225.i = insertelement <16 x float> %temp.vect224.i, float %123, i32 5
  %temp.vect226.i = insertelement <16 x float> %temp.vect225.i, float %124, i32 6
  %temp.vect227.i = insertelement <16 x float> %temp.vect226.i, float %125, i32 7
  %temp.vect228.i = insertelement <16 x float> %temp.vect227.i, float %126, i32 8
  %temp.vect229.i = insertelement <16 x float> %temp.vect228.i, float %127, i32 9
  %temp.vect230.i = insertelement <16 x float> %temp.vect229.i, float %128, i32 10
  %temp.vect231.i = insertelement <16 x float> %temp.vect230.i, float %129, i32 11
  %temp.vect232.i = insertelement <16 x float> %temp.vect231.i, float %130, i32 12
  %temp.vect233.i = insertelement <16 x float> %temp.vect232.i, float %131, i32 13
  %temp.vect234.i = insertelement <16 x float> %temp.vect233.i, float %132, i32 14
  %temp.vect235.i = insertelement <16 x float> %temp.vect234.i, float %133, i32 15
  %134 = extractelement <4 x float> %phi946.i, i32 2
  %135 = extractelement <4 x float> %phi949.i, i32 2
  %136 = extractelement <4 x float> %phi952.i, i32 2
  %137 = extractelement <4 x float> %phi955.i, i32 2
  %138 = extractelement <4 x float> %phi958.i, i32 2
  %139 = extractelement <4 x float> %phi961.i, i32 2
  %140 = extractelement <4 x float> %phi964.i, i32 2
  %141 = extractelement <4 x float> %phi967.i, i32 2
  %142 = extractelement <4 x float> %phi970.i, i32 2
  %143 = extractelement <4 x float> %phi973.i, i32 2
  %144 = extractelement <4 x float> %phi.i, i32 2
  %145 = extractelement <4 x float> %phi937.i, i32 2
  %146 = extractelement <4 x float> %phi940.i, i32 2
  %147 = extractelement <4 x float> %phi943.i, i32 2
  %148 = extractelement <4 x float> %phi976.i, i32 2
  %149 = extractelement <4 x float> %phi979.i, i32 2
  %temp.vect252.i = insertelement <16 x float> undef, float %134, i32 0
  %temp.vect253.i = insertelement <16 x float> %temp.vect252.i, float %135, i32 1
  %temp.vect254.i = insertelement <16 x float> %temp.vect253.i, float %136, i32 2
  %temp.vect255.i = insertelement <16 x float> %temp.vect254.i, float %137, i32 3
  %temp.vect256.i = insertelement <16 x float> %temp.vect255.i, float %138, i32 4
  %temp.vect257.i = insertelement <16 x float> %temp.vect256.i, float %139, i32 5
  %temp.vect258.i = insertelement <16 x float> %temp.vect257.i, float %140, i32 6
  %temp.vect259.i = insertelement <16 x float> %temp.vect258.i, float %141, i32 7
  %temp.vect260.i = insertelement <16 x float> %temp.vect259.i, float %142, i32 8
  %temp.vect261.i = insertelement <16 x float> %temp.vect260.i, float %143, i32 9
  %temp.vect262.i = insertelement <16 x float> %temp.vect261.i, float %144, i32 10
  %temp.vect263.i = insertelement <16 x float> %temp.vect262.i, float %145, i32 11
  %temp.vect264.i = insertelement <16 x float> %temp.vect263.i, float %146, i32 12
  %temp.vect265.i = insertelement <16 x float> %temp.vect264.i, float %147, i32 13
  %temp.vect266.i = insertelement <16 x float> %temp.vect265.i, float %148, i32 14
  %temp.vect267.i = insertelement <16 x float> %temp.vect266.i, float %149, i32 15
  %150 = extractelement <4 x float> %phi946.i, i32 3
  %151 = extractelement <4 x float> %phi949.i, i32 3
  %152 = extractelement <4 x float> %phi952.i, i32 3
  %153 = extractelement <4 x float> %phi955.i, i32 3
  %154 = extractelement <4 x float> %phi958.i, i32 3
  %155 = extractelement <4 x float> %phi961.i, i32 3
  %156 = extractelement <4 x float> %phi964.i, i32 3
  %157 = extractelement <4 x float> %phi967.i, i32 3
  %158 = extractelement <4 x float> %phi970.i, i32 3
  %159 = extractelement <4 x float> %phi973.i, i32 3
  %160 = extractelement <4 x float> %phi.i, i32 3
  %161 = extractelement <4 x float> %phi937.i, i32 3
  %162 = extractelement <4 x float> %phi940.i, i32 3
  %163 = extractelement <4 x float> %phi943.i, i32 3
  %164 = extractelement <4 x float> %phi976.i, i32 3
  %165 = extractelement <4 x float> %phi979.i, i32 3
  %temp.vect268.i = insertelement <16 x float> undef, float %150, i32 0
  %temp.vect269.i = insertelement <16 x float> %temp.vect268.i, float %151, i32 1
  %temp.vect270.i = insertelement <16 x float> %temp.vect269.i, float %152, i32 2
  %temp.vect271.i = insertelement <16 x float> %temp.vect270.i, float %153, i32 3
  %temp.vect272.i = insertelement <16 x float> %temp.vect271.i, float %154, i32 4
  %temp.vect273.i = insertelement <16 x float> %temp.vect272.i, float %155, i32 5
  %temp.vect274.i = insertelement <16 x float> %temp.vect273.i, float %156, i32 6
  %temp.vect275.i = insertelement <16 x float> %temp.vect274.i, float %157, i32 7
  %temp.vect276.i = insertelement <16 x float> %temp.vect275.i, float %158, i32 8
  %temp.vect277.i = insertelement <16 x float> %temp.vect276.i, float %159, i32 9
  %temp.vect278.i = insertelement <16 x float> %temp.vect277.i, float %160, i32 10
  %temp.vect279.i = insertelement <16 x float> %temp.vect278.i, float %161, i32 11
  %temp.vect280.i = insertelement <16 x float> %temp.vect279.i, float %162, i32 12
  %temp.vect281.i = insertelement <16 x float> %temp.vect280.i, float %163, i32 13
  %temp.vect282.i = insertelement <16 x float> %temp.vect281.i, float %164, i32 14
  %temp.vect283.i = insertelement <16 x float> %temp.vect282.i, float %165, i32 15
  %166 = fadd <16 x float> %temp.vect235.i, %temp.vect251.i
  %167 = fadd <16 x float> %temp.vect267.i, %166
  %168 = fadd <16 x float> %temp.vect283.i, %167
  %"&(pSB[currWI].offset)1650.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1651.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1650.i"
  %CastToValueType1652.i = bitcast i8* %"&pSB[currWI].offset1651.i" to <16 x i32>*
  %loadedValue1653.i = load <16 x i32>* %CastToValueType1652.i, align 64
  %169 = icmp slt <16 x i32> %loadedValue1653.i, %vector.i
  %_to_66284.i = and <16 x i1> %_to_59183.i, %169
  %extract302.i = extractelement <16 x i1> %_to_66284.i, i32 1
  %extract303.i = extractelement <16 x i1> %_to_66284.i, i32 2
  %extract304.i = extractelement <16 x i1> %_to_66284.i, i32 3
  %extract305.i = extractelement <16 x i1> %_to_66284.i, i32 4
  %extract306.i = extractelement <16 x i1> %_to_66284.i, i32 5
  %extract307.i = extractelement <16 x i1> %_to_66284.i, i32 6
  %extract308.i = extractelement <16 x i1> %_to_66284.i, i32 7
  %extract309.i = extractelement <16 x i1> %_to_66284.i, i32 8
  %extract310.i = extractelement <16 x i1> %_to_66284.i, i32 9
  %extract311.i = extractelement <16 x i1> %_to_66284.i, i32 10
  %extract312.i = extractelement <16 x i1> %_to_66284.i, i32 11
  %extract313.i = extractelement <16 x i1> %_to_66284.i, i32 12
  %extract314.i = extractelement <16 x i1> %_to_66284.i, i32 13
  %extract315.i = extractelement <16 x i1> %_to_66284.i, i32 14
  %extract316.i = extractelement <16 x i1> %_to_66284.i, i32 15
  %extract301.i = extractelement <16 x i1> %_to_66284.i, i32 0
  %"&(pSB[currWI].offset)1610.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1611.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1610.i"
  %CastToValueType1612.i = bitcast i8* %"&pSB[currWI].offset1611.i" to <16 x i32>*
  %loadedValue1613.i = load <16 x i32>* %CastToValueType1612.i, align 64
  %170 = extractelement <16 x i32> %loadedValue1613.i, i32 1
  %171 = sext i32 %170 to i64
  %172 = getelementptr inbounds float addrspace(1)* %4, i64 %171
  %"&(pSB[currWI].offset)1605.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1606.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1605.i"
  %CastToValueType1607.i = bitcast i8* %"&pSB[currWI].offset1606.i" to <16 x i32>*
  %loadedValue1608.i = load <16 x i32>* %CastToValueType1607.i, align 64
  %173 = extractelement <16 x i32> %loadedValue1608.i, i32 2
  %174 = sext i32 %173 to i64
  %175 = getelementptr inbounds float addrspace(1)* %4, i64 %174
  %"&(pSB[currWI].offset)1600.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1601.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1600.i"
  %CastToValueType1602.i = bitcast i8* %"&pSB[currWI].offset1601.i" to <16 x i32>*
  %loadedValue1603.i = load <16 x i32>* %CastToValueType1602.i, align 64
  %176 = extractelement <16 x i32> %loadedValue1603.i, i32 3
  %177 = sext i32 %176 to i64
  %178 = getelementptr inbounds float addrspace(1)* %4, i64 %177
  %"&(pSB[currWI].offset)1595.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1596.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1595.i"
  %CastToValueType1597.i = bitcast i8* %"&pSB[currWI].offset1596.i" to <16 x i32>*
  %loadedValue1598.i = load <16 x i32>* %CastToValueType1597.i, align 64
  %179 = extractelement <16 x i32> %loadedValue1598.i, i32 4
  %180 = sext i32 %179 to i64
  %181 = getelementptr inbounds float addrspace(1)* %4, i64 %180
  %"&(pSB[currWI].offset)1590.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1591.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1590.i"
  %CastToValueType1592.i = bitcast i8* %"&pSB[currWI].offset1591.i" to <16 x i32>*
  %loadedValue1593.i = load <16 x i32>* %CastToValueType1592.i, align 64
  %182 = extractelement <16 x i32> %loadedValue1593.i, i32 5
  %183 = sext i32 %182 to i64
  %184 = getelementptr inbounds float addrspace(1)* %4, i64 %183
  %"&(pSB[currWI].offset)1585.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1586.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1585.i"
  %CastToValueType1587.i = bitcast i8* %"&pSB[currWI].offset1586.i" to <16 x i32>*
  %loadedValue1588.i = load <16 x i32>* %CastToValueType1587.i, align 64
  %185 = extractelement <16 x i32> %loadedValue1588.i, i32 6
  %186 = sext i32 %185 to i64
  %187 = getelementptr inbounds float addrspace(1)* %4, i64 %186
  %"&(pSB[currWI].offset)1580.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1581.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1580.i"
  %CastToValueType1582.i = bitcast i8* %"&pSB[currWI].offset1581.i" to <16 x i32>*
  %loadedValue1583.i = load <16 x i32>* %CastToValueType1582.i, align 64
  %188 = extractelement <16 x i32> %loadedValue1583.i, i32 7
  %189 = sext i32 %188 to i64
  %190 = getelementptr inbounds float addrspace(1)* %4, i64 %189
  %"&(pSB[currWI].offset)1575.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1576.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1575.i"
  %CastToValueType1577.i = bitcast i8* %"&pSB[currWI].offset1576.i" to <16 x i32>*
  %loadedValue1578.i = load <16 x i32>* %CastToValueType1577.i, align 64
  %191 = extractelement <16 x i32> %loadedValue1578.i, i32 8
  %192 = sext i32 %191 to i64
  %193 = getelementptr inbounds float addrspace(1)* %4, i64 %192
  %"&(pSB[currWI].offset)1570.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1571.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1570.i"
  %CastToValueType1572.i = bitcast i8* %"&pSB[currWI].offset1571.i" to <16 x i32>*
  %loadedValue1573.i = load <16 x i32>* %CastToValueType1572.i, align 64
  %194 = extractelement <16 x i32> %loadedValue1573.i, i32 9
  %195 = sext i32 %194 to i64
  %196 = getelementptr inbounds float addrspace(1)* %4, i64 %195
  %"&(pSB[currWI].offset)1565.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1566.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1565.i"
  %CastToValueType1567.i = bitcast i8* %"&pSB[currWI].offset1566.i" to <16 x i32>*
  %loadedValue1568.i = load <16 x i32>* %CastToValueType1567.i, align 64
  %197 = extractelement <16 x i32> %loadedValue1568.i, i32 10
  %198 = sext i32 %197 to i64
  %199 = getelementptr inbounds float addrspace(1)* %4, i64 %198
  %"&(pSB[currWI].offset)1560.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1561.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1560.i"
  %CastToValueType1562.i = bitcast i8* %"&pSB[currWI].offset1561.i" to <16 x i32>*
  %loadedValue1563.i = load <16 x i32>* %CastToValueType1562.i, align 64
  %200 = extractelement <16 x i32> %loadedValue1563.i, i32 11
  %201 = sext i32 %200 to i64
  %202 = getelementptr inbounds float addrspace(1)* %4, i64 %201
  %"&(pSB[currWI].offset)1555.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1556.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1555.i"
  %CastToValueType1557.i = bitcast i8* %"&pSB[currWI].offset1556.i" to <16 x i32>*
  %loadedValue1558.i = load <16 x i32>* %CastToValueType1557.i, align 64
  %203 = extractelement <16 x i32> %loadedValue1558.i, i32 12
  %204 = sext i32 %203 to i64
  %205 = getelementptr inbounds float addrspace(1)* %4, i64 %204
  %"&(pSB[currWI].offset)1550.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1551.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1550.i"
  %CastToValueType1552.i = bitcast i8* %"&pSB[currWI].offset1551.i" to <16 x i32>*
  %loadedValue1553.i = load <16 x i32>* %CastToValueType1552.i, align 64
  %206 = extractelement <16 x i32> %loadedValue1553.i, i32 13
  %207 = sext i32 %206 to i64
  %208 = getelementptr inbounds float addrspace(1)* %4, i64 %207
  %"&(pSB[currWI].offset)1545.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1546.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1545.i"
  %CastToValueType1547.i = bitcast i8* %"&pSB[currWI].offset1546.i" to <16 x i32>*
  %loadedValue1548.i = load <16 x i32>* %CastToValueType1547.i, align 64
  %209 = extractelement <16 x i32> %loadedValue1548.i, i32 14
  %210 = sext i32 %209 to i64
  %211 = getelementptr inbounds float addrspace(1)* %4, i64 %210
  %"&(pSB[currWI].offset)1540.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1541.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1540.i"
  %CastToValueType1542.i = bitcast i8* %"&pSB[currWI].offset1541.i" to <16 x i32>*
  %loadedValue1543.i = load <16 x i32>* %CastToValueType1542.i, align 64
  %212 = extractelement <16 x i32> %loadedValue1543.i, i32 15
  %213 = sext i32 %212 to i64
  %214 = getelementptr inbounds float addrspace(1)* %4, i64 %213
  br i1 %extract301.i, label %preload983.i, label %postload984.i

preload983.i:                                     ; preds = %postload978.i
  %"&(pSB[currWI].offset)1615.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1616.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1615.i"
  %CastToValueType1617.i = bitcast i8* %"&pSB[currWI].offset1616.i" to <16 x i32>*
  %loadedValue1618.i = load <16 x i32>* %CastToValueType1617.i, align 64
  %215 = extractelement <16 x i32> %loadedValue1618.i, i32 0
  %216 = sext i32 %215 to i64
  %217 = getelementptr inbounds float addrspace(1)* %4, i64 %216
  %masked_load887.i = load float addrspace(1)* %217, align 4
  br label %postload984.i

postload984.i:                                    ; preds = %preload983.i, %postload978.i
  %phi985.i = phi float [ undef, %postload978.i ], [ %masked_load887.i, %preload983.i ]
  br i1 %extract302.i, label %preload986.i, label %postload987.i

preload986.i:                                     ; preds = %postload984.i
  %masked_load888.i = load float addrspace(1)* %172, align 4
  br label %postload987.i

postload987.i:                                    ; preds = %preload986.i, %postload984.i
  %phi988.i = phi float [ undef, %postload984.i ], [ %masked_load888.i, %preload986.i ]
  br i1 %extract303.i, label %preload989.i, label %postload990.i

preload989.i:                                     ; preds = %postload987.i
  %masked_load889.i = load float addrspace(1)* %175, align 4
  br label %postload990.i

postload990.i:                                    ; preds = %preload989.i, %postload987.i
  %phi991.i = phi float [ undef, %postload987.i ], [ %masked_load889.i, %preload989.i ]
  br i1 %extract304.i, label %preload992.i, label %postload993.i

preload992.i:                                     ; preds = %postload990.i
  %masked_load890.i = load float addrspace(1)* %178, align 4
  br label %postload993.i

postload993.i:                                    ; preds = %preload992.i, %postload990.i
  %phi994.i = phi float [ undef, %postload990.i ], [ %masked_load890.i, %preload992.i ]
  br i1 %extract305.i, label %preload995.i, label %postload996.i

preload995.i:                                     ; preds = %postload993.i
  %masked_load891.i = load float addrspace(1)* %181, align 4
  br label %postload996.i

postload996.i:                                    ; preds = %preload995.i, %postload993.i
  %phi997.i = phi float [ undef, %postload993.i ], [ %masked_load891.i, %preload995.i ]
  br i1 %extract306.i, label %preload998.i, label %postload999.i

preload998.i:                                     ; preds = %postload996.i
  %masked_load892.i = load float addrspace(1)* %184, align 4
  br label %postload999.i

postload999.i:                                    ; preds = %preload998.i, %postload996.i
  %phi1000.i = phi float [ undef, %postload996.i ], [ %masked_load892.i, %preload998.i ]
  br i1 %extract307.i, label %preload1001.i, label %postload1002.i

preload1001.i:                                    ; preds = %postload999.i
  %masked_load893.i = load float addrspace(1)* %187, align 4
  br label %postload1002.i

postload1002.i:                                   ; preds = %preload1001.i, %postload999.i
  %phi1003.i = phi float [ undef, %postload999.i ], [ %masked_load893.i, %preload1001.i ]
  br i1 %extract308.i, label %preload1004.i, label %postload1005.i

preload1004.i:                                    ; preds = %postload1002.i
  %masked_load894.i = load float addrspace(1)* %190, align 4
  br label %postload1005.i

postload1005.i:                                   ; preds = %preload1004.i, %postload1002.i
  %phi1006.i = phi float [ undef, %postload1002.i ], [ %masked_load894.i, %preload1004.i ]
  br i1 %extract309.i, label %preload1007.i, label %postload1008.i

preload1007.i:                                    ; preds = %postload1005.i
  %masked_load895.i = load float addrspace(1)* %193, align 4
  br label %postload1008.i

postload1008.i:                                   ; preds = %preload1007.i, %postload1005.i
  %phi1009.i = phi float [ undef, %postload1005.i ], [ %masked_load895.i, %preload1007.i ]
  br i1 %extract310.i, label %preload1010.i, label %postload1011.i

preload1010.i:                                    ; preds = %postload1008.i
  %masked_load896.i = load float addrspace(1)* %196, align 4
  br label %postload1011.i

postload1011.i:                                   ; preds = %preload1010.i, %postload1008.i
  %phi1012.i = phi float [ undef, %postload1008.i ], [ %masked_load896.i, %preload1010.i ]
  br i1 %extract311.i, label %preload1013.i, label %postload1014.i

preload1013.i:                                    ; preds = %postload1011.i
  %masked_load897.i = load float addrspace(1)* %199, align 4
  br label %postload1014.i

postload1014.i:                                   ; preds = %preload1013.i, %postload1011.i
  %phi1015.i = phi float [ undef, %postload1011.i ], [ %masked_load897.i, %preload1013.i ]
  br i1 %extract312.i, label %preload1016.i, label %postload1017.i

preload1016.i:                                    ; preds = %postload1014.i
  %masked_load898.i = load float addrspace(1)* %202, align 4
  br label %postload1017.i

postload1017.i:                                   ; preds = %preload1016.i, %postload1014.i
  %phi1018.i = phi float [ undef, %postload1014.i ], [ %masked_load898.i, %preload1016.i ]
  br i1 %extract313.i, label %preload1019.i, label %postload1020.i

preload1019.i:                                    ; preds = %postload1017.i
  %masked_load899.i = load float addrspace(1)* %205, align 4
  br label %postload1020.i

postload1020.i:                                   ; preds = %preload1019.i, %postload1017.i
  %phi1021.i = phi float [ undef, %postload1017.i ], [ %masked_load899.i, %preload1019.i ]
  br i1 %extract314.i, label %preload1022.i, label %postload1023.i

preload1022.i:                                    ; preds = %postload1020.i
  %masked_load900.i = load float addrspace(1)* %208, align 4
  br label %postload1023.i

postload1023.i:                                   ; preds = %preload1022.i, %postload1020.i
  %phi1024.i = phi float [ undef, %postload1020.i ], [ %masked_load900.i, %preload1022.i ]
  br i1 %extract315.i, label %preload1025.i, label %postload1026.i

preload1025.i:                                    ; preds = %postload1023.i
  %masked_load901.i = load float addrspace(1)* %211, align 4
  br label %postload1026.i

postload1026.i:                                   ; preds = %preload1025.i, %postload1023.i
  %phi1027.i = phi float [ undef, %postload1023.i ], [ %masked_load901.i, %preload1025.i ]
  br i1 %extract316.i, label %preload1028.i, label %postload1029.i

preload1028.i:                                    ; preds = %postload1026.i
  %masked_load902.i = load float addrspace(1)* %214, align 4
  br label %postload1029.i

postload1029.i:                                   ; preds = %preload1028.i, %postload1026.i
  %phi1030.i = phi float [ undef, %postload1026.i ], [ %masked_load902.i, %preload1028.i ]
  %temp.vect317.i = insertelement <16 x float> undef, float %phi985.i, i32 0
  %temp.vect318.i = insertelement <16 x float> %temp.vect317.i, float %phi988.i, i32 1
  %temp.vect319.i = insertelement <16 x float> %temp.vect318.i, float %phi991.i, i32 2
  %temp.vect320.i = insertelement <16 x float> %temp.vect319.i, float %phi994.i, i32 3
  %temp.vect321.i = insertelement <16 x float> %temp.vect320.i, float %phi997.i, i32 4
  %temp.vect322.i = insertelement <16 x float> %temp.vect321.i, float %phi1000.i, i32 5
  %temp.vect323.i = insertelement <16 x float> %temp.vect322.i, float %phi1003.i, i32 6
  %temp.vect324.i = insertelement <16 x float> %temp.vect323.i, float %phi1006.i, i32 7
  %temp.vect325.i = insertelement <16 x float> %temp.vect324.i, float %phi1009.i, i32 8
  %temp.vect326.i = insertelement <16 x float> %temp.vect325.i, float %phi1012.i, i32 9
  %temp.vect327.i = insertelement <16 x float> %temp.vect326.i, float %phi1015.i, i32 10
  %temp.vect328.i = insertelement <16 x float> %temp.vect327.i, float %phi1018.i, i32 11
  %temp.vect329.i = insertelement <16 x float> %temp.vect328.i, float %phi1021.i, i32 12
  %temp.vect330.i = insertelement <16 x float> %temp.vect329.i, float %phi1024.i, i32 13
  %temp.vect331.i = insertelement <16 x float> %temp.vect330.i, float %phi1027.i, i32 14
  %temp.vect332.i = insertelement <16 x float> %temp.vect331.i, float %phi1030.i, i32 15
  %merge333.i = select <16 x i1> %_to_66284.i, <16 x float> %temp.vect332.i, <16 x float> zeroinitializer
  %"&(pSB[currWI].offset)1645.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1646.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1645.i"
  %CastToValueType1647.i = bitcast i8* %"&pSB[currWI].offset1646.i" to <16 x i32>*
  %loadedValue1648.i = load <16 x i32>* %CastToValueType1647.i, align 64
  %218 = or <16 x i32> %loadedValue1648.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %219 = icmp slt <16 x i32> %218, %vector.i
  %_to_72334.i = and <16 x i1> %_to_59183.i, %219
  %extract352.i = extractelement <16 x i1> %_to_72334.i, i32 1
  %extract353.i = extractelement <16 x i1> %_to_72334.i, i32 2
  %extract354.i = extractelement <16 x i1> %_to_72334.i, i32 3
  %extract355.i = extractelement <16 x i1> %_to_72334.i, i32 4
  %extract356.i = extractelement <16 x i1> %_to_72334.i, i32 5
  %extract357.i = extractelement <16 x i1> %_to_72334.i, i32 6
  %extract358.i = extractelement <16 x i1> %_to_72334.i, i32 7
  %extract359.i = extractelement <16 x i1> %_to_72334.i, i32 8
  %extract360.i = extractelement <16 x i1> %_to_72334.i, i32 9
  %extract361.i = extractelement <16 x i1> %_to_72334.i, i32 10
  %extract362.i = extractelement <16 x i1> %_to_72334.i, i32 11
  %extract363.i = extractelement <16 x i1> %_to_72334.i, i32 12
  %extract364.i = extractelement <16 x i1> %_to_72334.i, i32 13
  %extract365.i = extractelement <16 x i1> %_to_72334.i, i32 14
  %extract366.i = extractelement <16 x i1> %_to_72334.i, i32 15
  %extract351.i = extractelement <16 x i1> %_to_72334.i, i32 0
  %220 = extractelement <16 x i32> %218, i32 1
  %221 = sext i32 %220 to i64
  %222 = getelementptr inbounds float addrspace(1)* %4, i64 %221
  %223 = extractelement <16 x i32> %218, i32 2
  %224 = sext i32 %223 to i64
  %225 = getelementptr inbounds float addrspace(1)* %4, i64 %224
  %226 = extractelement <16 x i32> %218, i32 3
  %227 = sext i32 %226 to i64
  %228 = getelementptr inbounds float addrspace(1)* %4, i64 %227
  %229 = extractelement <16 x i32> %218, i32 4
  %230 = sext i32 %229 to i64
  %231 = getelementptr inbounds float addrspace(1)* %4, i64 %230
  %232 = extractelement <16 x i32> %218, i32 5
  %233 = sext i32 %232 to i64
  %234 = getelementptr inbounds float addrspace(1)* %4, i64 %233
  %235 = extractelement <16 x i32> %218, i32 6
  %236 = sext i32 %235 to i64
  %237 = getelementptr inbounds float addrspace(1)* %4, i64 %236
  %238 = extractelement <16 x i32> %218, i32 7
  %239 = sext i32 %238 to i64
  %240 = getelementptr inbounds float addrspace(1)* %4, i64 %239
  %241 = extractelement <16 x i32> %218, i32 8
  %242 = sext i32 %241 to i64
  %243 = getelementptr inbounds float addrspace(1)* %4, i64 %242
  %244 = extractelement <16 x i32> %218, i32 9
  %245 = sext i32 %244 to i64
  %246 = getelementptr inbounds float addrspace(1)* %4, i64 %245
  %247 = extractelement <16 x i32> %218, i32 10
  %248 = sext i32 %247 to i64
  %249 = getelementptr inbounds float addrspace(1)* %4, i64 %248
  %250 = extractelement <16 x i32> %218, i32 11
  %251 = sext i32 %250 to i64
  %252 = getelementptr inbounds float addrspace(1)* %4, i64 %251
  %253 = extractelement <16 x i32> %218, i32 12
  %254 = sext i32 %253 to i64
  %255 = getelementptr inbounds float addrspace(1)* %4, i64 %254
  %256 = extractelement <16 x i32> %218, i32 13
  %257 = sext i32 %256 to i64
  %258 = getelementptr inbounds float addrspace(1)* %4, i64 %257
  %259 = extractelement <16 x i32> %218, i32 14
  %260 = sext i32 %259 to i64
  %261 = getelementptr inbounds float addrspace(1)* %4, i64 %260
  %262 = extractelement <16 x i32> %218, i32 15
  %263 = sext i32 %262 to i64
  %264 = getelementptr inbounds float addrspace(1)* %4, i64 %263
  br i1 %extract351.i, label %preload1031.i, label %postload1032.i

preload1031.i:                                    ; preds = %postload1029.i
  %265 = extractelement <16 x i32> %218, i32 0
  %266 = sext i32 %265 to i64
  %267 = getelementptr inbounds float addrspace(1)* %4, i64 %266
  %masked_load903.i = load float addrspace(1)* %267, align 4
  br label %postload1032.i

postload1032.i:                                   ; preds = %preload1031.i, %postload1029.i
  %phi1033.i = phi float [ undef, %postload1029.i ], [ %masked_load903.i, %preload1031.i ]
  br i1 %extract352.i, label %preload1034.i, label %postload1035.i

preload1034.i:                                    ; preds = %postload1032.i
  %masked_load904.i = load float addrspace(1)* %222, align 4
  br label %postload1035.i

postload1035.i:                                   ; preds = %preload1034.i, %postload1032.i
  %phi1036.i = phi float [ undef, %postload1032.i ], [ %masked_load904.i, %preload1034.i ]
  br i1 %extract353.i, label %preload1037.i, label %postload1038.i

preload1037.i:                                    ; preds = %postload1035.i
  %masked_load905.i = load float addrspace(1)* %225, align 4
  br label %postload1038.i

postload1038.i:                                   ; preds = %preload1037.i, %postload1035.i
  %phi1039.i = phi float [ undef, %postload1035.i ], [ %masked_load905.i, %preload1037.i ]
  br i1 %extract354.i, label %preload1040.i, label %postload1041.i

preload1040.i:                                    ; preds = %postload1038.i
  %masked_load906.i = load float addrspace(1)* %228, align 4
  br label %postload1041.i

postload1041.i:                                   ; preds = %preload1040.i, %postload1038.i
  %phi1042.i = phi float [ undef, %postload1038.i ], [ %masked_load906.i, %preload1040.i ]
  br i1 %extract355.i, label %preload1043.i, label %postload1044.i

preload1043.i:                                    ; preds = %postload1041.i
  %masked_load907.i = load float addrspace(1)* %231, align 4
  br label %postload1044.i

postload1044.i:                                   ; preds = %preload1043.i, %postload1041.i
  %phi1045.i = phi float [ undef, %postload1041.i ], [ %masked_load907.i, %preload1043.i ]
  br i1 %extract356.i, label %preload1046.i, label %postload1047.i

preload1046.i:                                    ; preds = %postload1044.i
  %masked_load908.i = load float addrspace(1)* %234, align 4
  br label %postload1047.i

postload1047.i:                                   ; preds = %preload1046.i, %postload1044.i
  %phi1048.i = phi float [ undef, %postload1044.i ], [ %masked_load908.i, %preload1046.i ]
  br i1 %extract357.i, label %preload1049.i, label %postload1050.i

preload1049.i:                                    ; preds = %postload1047.i
  %masked_load909.i = load float addrspace(1)* %237, align 4
  br label %postload1050.i

postload1050.i:                                   ; preds = %preload1049.i, %postload1047.i
  %phi1051.i = phi float [ undef, %postload1047.i ], [ %masked_load909.i, %preload1049.i ]
  br i1 %extract358.i, label %preload1052.i, label %postload1053.i

preload1052.i:                                    ; preds = %postload1050.i
  %masked_load910.i = load float addrspace(1)* %240, align 4
  br label %postload1053.i

postload1053.i:                                   ; preds = %preload1052.i, %postload1050.i
  %phi1054.i = phi float [ undef, %postload1050.i ], [ %masked_load910.i, %preload1052.i ]
  br i1 %extract359.i, label %preload1055.i, label %postload1056.i

preload1055.i:                                    ; preds = %postload1053.i
  %masked_load911.i = load float addrspace(1)* %243, align 4
  br label %postload1056.i

postload1056.i:                                   ; preds = %preload1055.i, %postload1053.i
  %phi1057.i = phi float [ undef, %postload1053.i ], [ %masked_load911.i, %preload1055.i ]
  br i1 %extract360.i, label %preload1058.i, label %postload1059.i

preload1058.i:                                    ; preds = %postload1056.i
  %masked_load912.i = load float addrspace(1)* %246, align 4
  br label %postload1059.i

postload1059.i:                                   ; preds = %preload1058.i, %postload1056.i
  %phi1060.i = phi float [ undef, %postload1056.i ], [ %masked_load912.i, %preload1058.i ]
  br i1 %extract361.i, label %preload1061.i, label %postload1062.i

preload1061.i:                                    ; preds = %postload1059.i
  %masked_load913.i = load float addrspace(1)* %249, align 4
  br label %postload1062.i

postload1062.i:                                   ; preds = %preload1061.i, %postload1059.i
  %phi1063.i = phi float [ undef, %postload1059.i ], [ %masked_load913.i, %preload1061.i ]
  br i1 %extract362.i, label %preload1064.i, label %postload1065.i

preload1064.i:                                    ; preds = %postload1062.i
  %masked_load914.i = load float addrspace(1)* %252, align 4
  br label %postload1065.i

postload1065.i:                                   ; preds = %preload1064.i, %postload1062.i
  %phi1066.i = phi float [ undef, %postload1062.i ], [ %masked_load914.i, %preload1064.i ]
  br i1 %extract363.i, label %preload1067.i, label %postload1068.i

preload1067.i:                                    ; preds = %postload1065.i
  %masked_load915.i = load float addrspace(1)* %255, align 4
  br label %postload1068.i

postload1068.i:                                   ; preds = %preload1067.i, %postload1065.i
  %phi1069.i = phi float [ undef, %postload1065.i ], [ %masked_load915.i, %preload1067.i ]
  br i1 %extract364.i, label %preload1070.i, label %postload1071.i

preload1070.i:                                    ; preds = %postload1068.i
  %masked_load916.i = load float addrspace(1)* %258, align 4
  br label %postload1071.i

postload1071.i:                                   ; preds = %preload1070.i, %postload1068.i
  %phi1072.i = phi float [ undef, %postload1068.i ], [ %masked_load916.i, %preload1070.i ]
  br i1 %extract365.i, label %preload1073.i, label %postload1074.i

preload1073.i:                                    ; preds = %postload1071.i
  %masked_load917.i = load float addrspace(1)* %261, align 4
  br label %postload1074.i

postload1074.i:                                   ; preds = %preload1073.i, %postload1071.i
  %phi1075.i = phi float [ undef, %postload1071.i ], [ %masked_load917.i, %preload1073.i ]
  br i1 %extract366.i, label %preload1076.i, label %postload1077.i

preload1076.i:                                    ; preds = %postload1074.i
  %masked_load918.i = load float addrspace(1)* %264, align 4
  br label %postload1077.i

postload1077.i:                                   ; preds = %preload1076.i, %postload1074.i
  %phi1078.i = phi float [ undef, %postload1074.i ], [ %masked_load918.i, %preload1076.i ]
  %temp.vect367.i = insertelement <16 x float> undef, float %phi1033.i, i32 0
  %temp.vect368.i = insertelement <16 x float> %temp.vect367.i, float %phi1036.i, i32 1
  %temp.vect369.i = insertelement <16 x float> %temp.vect368.i, float %phi1039.i, i32 2
  %temp.vect370.i = insertelement <16 x float> %temp.vect369.i, float %phi1042.i, i32 3
  %temp.vect371.i = insertelement <16 x float> %temp.vect370.i, float %phi1045.i, i32 4
  %temp.vect372.i = insertelement <16 x float> %temp.vect371.i, float %phi1048.i, i32 5
  %temp.vect373.i = insertelement <16 x float> %temp.vect372.i, float %phi1051.i, i32 6
  %temp.vect374.i = insertelement <16 x float> %temp.vect373.i, float %phi1054.i, i32 7
  %temp.vect375.i = insertelement <16 x float> %temp.vect374.i, float %phi1057.i, i32 8
  %temp.vect376.i = insertelement <16 x float> %temp.vect375.i, float %phi1060.i, i32 9
  %temp.vect377.i = insertelement <16 x float> %temp.vect376.i, float %phi1063.i, i32 10
  %temp.vect378.i = insertelement <16 x float> %temp.vect377.i, float %phi1066.i, i32 11
  %temp.vect379.i = insertelement <16 x float> %temp.vect378.i, float %phi1069.i, i32 12
  %temp.vect380.i = insertelement <16 x float> %temp.vect379.i, float %phi1072.i, i32 13
  %temp.vect381.i = insertelement <16 x float> %temp.vect380.i, float %phi1075.i, i32 14
  %temp.vect382.i = insertelement <16 x float> %temp.vect381.i, float %phi1078.i, i32 15
  %merge152383.i = select <16 x i1> %_to_72334.i, <16 x float> %temp.vect382.i, <16 x float> zeroinitializer
  %268 = fadd <16 x float> %merge152383.i, %merge333.i
  %"&(pSB[currWI].offset)1640.i" = add nuw i64 %CurrSBIndex..1.i, 512
  %"&pSB[currWI].offset1641.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1640.i"
  %CastToValueType1642.i = bitcast i8* %"&pSB[currWI].offset1641.i" to <16 x i32>*
  %loadedValue1643.i = load <16 x i32>* %CastToValueType1642.i, align 64
  %269 = or <16 x i32> %loadedValue1643.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %270 = icmp slt <16 x i32> %269, %vector.i
  %_to_78384.i = and <16 x i1> %_to_59183.i, %270
  %extract402.i = extractelement <16 x i1> %_to_78384.i, i32 1
  %extract403.i = extractelement <16 x i1> %_to_78384.i, i32 2
  %extract404.i = extractelement <16 x i1> %_to_78384.i, i32 3
  %extract405.i = extractelement <16 x i1> %_to_78384.i, i32 4
  %extract406.i = extractelement <16 x i1> %_to_78384.i, i32 5
  %extract407.i = extractelement <16 x i1> %_to_78384.i, i32 6
  %extract408.i = extractelement <16 x i1> %_to_78384.i, i32 7
  %extract409.i = extractelement <16 x i1> %_to_78384.i, i32 8
  %extract410.i = extractelement <16 x i1> %_to_78384.i, i32 9
  %extract411.i = extractelement <16 x i1> %_to_78384.i, i32 10
  %extract412.i = extractelement <16 x i1> %_to_78384.i, i32 11
  %extract413.i = extractelement <16 x i1> %_to_78384.i, i32 12
  %extract414.i = extractelement <16 x i1> %_to_78384.i, i32 13
  %extract415.i = extractelement <16 x i1> %_to_78384.i, i32 14
  %extract416.i = extractelement <16 x i1> %_to_78384.i, i32 15
  %extract401.i = extractelement <16 x i1> %_to_78384.i, i32 0
  %271 = extractelement <16 x i32> %269, i32 1
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds float addrspace(1)* %4, i64 %272
  %274 = extractelement <16 x i32> %269, i32 2
  %275 = sext i32 %274 to i64
  %276 = getelementptr inbounds float addrspace(1)* %4, i64 %275
  %277 = extractelement <16 x i32> %269, i32 3
  %278 = sext i32 %277 to i64
  %279 = getelementptr inbounds float addrspace(1)* %4, i64 %278
  %280 = extractelement <16 x i32> %269, i32 4
  %281 = sext i32 %280 to i64
  %282 = getelementptr inbounds float addrspace(1)* %4, i64 %281
  %283 = extractelement <16 x i32> %269, i32 5
  %284 = sext i32 %283 to i64
  %285 = getelementptr inbounds float addrspace(1)* %4, i64 %284
  %286 = extractelement <16 x i32> %269, i32 6
  %287 = sext i32 %286 to i64
  %288 = getelementptr inbounds float addrspace(1)* %4, i64 %287
  %289 = extractelement <16 x i32> %269, i32 7
  %290 = sext i32 %289 to i64
  %291 = getelementptr inbounds float addrspace(1)* %4, i64 %290
  %292 = extractelement <16 x i32> %269, i32 8
  %293 = sext i32 %292 to i64
  %294 = getelementptr inbounds float addrspace(1)* %4, i64 %293
  %295 = extractelement <16 x i32> %269, i32 9
  %296 = sext i32 %295 to i64
  %297 = getelementptr inbounds float addrspace(1)* %4, i64 %296
  %298 = extractelement <16 x i32> %269, i32 10
  %299 = sext i32 %298 to i64
  %300 = getelementptr inbounds float addrspace(1)* %4, i64 %299
  %301 = extractelement <16 x i32> %269, i32 11
  %302 = sext i32 %301 to i64
  %303 = getelementptr inbounds float addrspace(1)* %4, i64 %302
  %304 = extractelement <16 x i32> %269, i32 12
  %305 = sext i32 %304 to i64
  %306 = getelementptr inbounds float addrspace(1)* %4, i64 %305
  %307 = extractelement <16 x i32> %269, i32 13
  %308 = sext i32 %307 to i64
  %309 = getelementptr inbounds float addrspace(1)* %4, i64 %308
  %310 = extractelement <16 x i32> %269, i32 14
  %311 = sext i32 %310 to i64
  %312 = getelementptr inbounds float addrspace(1)* %4, i64 %311
  %313 = extractelement <16 x i32> %269, i32 15
  %314 = sext i32 %313 to i64
  %315 = getelementptr inbounds float addrspace(1)* %4, i64 %314
  br i1 %extract401.i, label %preload1079.i, label %postload1080.i

preload1079.i:                                    ; preds = %postload1077.i
  %316 = extractelement <16 x i32> %269, i32 0
  %317 = sext i32 %316 to i64
  %318 = getelementptr inbounds float addrspace(1)* %4, i64 %317
  %masked_load919.i = load float addrspace(1)* %318, align 4
  br label %postload1080.i

postload1080.i:                                   ; preds = %preload1079.i, %postload1077.i
  %phi1081.i = phi float [ undef, %postload1077.i ], [ %masked_load919.i, %preload1079.i ]
  br i1 %extract402.i, label %preload1082.i, label %postload1083.i

preload1082.i:                                    ; preds = %postload1080.i
  %masked_load920.i = load float addrspace(1)* %273, align 4
  br label %postload1083.i

postload1083.i:                                   ; preds = %preload1082.i, %postload1080.i
  %phi1084.i = phi float [ undef, %postload1080.i ], [ %masked_load920.i, %preload1082.i ]
  br i1 %extract403.i, label %preload1085.i, label %postload1086.i

preload1085.i:                                    ; preds = %postload1083.i
  %masked_load921.i = load float addrspace(1)* %276, align 4
  br label %postload1086.i

postload1086.i:                                   ; preds = %preload1085.i, %postload1083.i
  %phi1087.i = phi float [ undef, %postload1083.i ], [ %masked_load921.i, %preload1085.i ]
  br i1 %extract404.i, label %preload1088.i, label %postload1089.i

preload1088.i:                                    ; preds = %postload1086.i
  %masked_load922.i = load float addrspace(1)* %279, align 4
  br label %postload1089.i

postload1089.i:                                   ; preds = %preload1088.i, %postload1086.i
  %phi1090.i = phi float [ undef, %postload1086.i ], [ %masked_load922.i, %preload1088.i ]
  br i1 %extract405.i, label %preload1091.i, label %postload1092.i

preload1091.i:                                    ; preds = %postload1089.i
  %masked_load923.i = load float addrspace(1)* %282, align 4
  br label %postload1092.i

postload1092.i:                                   ; preds = %preload1091.i, %postload1089.i
  %phi1093.i = phi float [ undef, %postload1089.i ], [ %masked_load923.i, %preload1091.i ]
  br i1 %extract406.i, label %preload1094.i, label %postload1095.i

preload1094.i:                                    ; preds = %postload1092.i
  %masked_load924.i = load float addrspace(1)* %285, align 4
  br label %postload1095.i

postload1095.i:                                   ; preds = %preload1094.i, %postload1092.i
  %phi1096.i = phi float [ undef, %postload1092.i ], [ %masked_load924.i, %preload1094.i ]
  br i1 %extract407.i, label %preload1097.i, label %postload1098.i

preload1097.i:                                    ; preds = %postload1095.i
  %masked_load925.i = load float addrspace(1)* %288, align 4
  br label %postload1098.i

postload1098.i:                                   ; preds = %preload1097.i, %postload1095.i
  %phi1099.i = phi float [ undef, %postload1095.i ], [ %masked_load925.i, %preload1097.i ]
  br i1 %extract408.i, label %preload1100.i, label %postload1101.i

preload1100.i:                                    ; preds = %postload1098.i
  %masked_load926.i = load float addrspace(1)* %291, align 4
  br label %postload1101.i

postload1101.i:                                   ; preds = %preload1100.i, %postload1098.i
  %phi1102.i = phi float [ undef, %postload1098.i ], [ %masked_load926.i, %preload1100.i ]
  br i1 %extract409.i, label %preload1103.i, label %postload1104.i

preload1103.i:                                    ; preds = %postload1101.i
  %masked_load927.i = load float addrspace(1)* %294, align 4
  br label %postload1104.i

postload1104.i:                                   ; preds = %preload1103.i, %postload1101.i
  %phi1105.i = phi float [ undef, %postload1101.i ], [ %masked_load927.i, %preload1103.i ]
  br i1 %extract410.i, label %preload1106.i, label %postload1107.i

preload1106.i:                                    ; preds = %postload1104.i
  %masked_load928.i = load float addrspace(1)* %297, align 4
  br label %postload1107.i

postload1107.i:                                   ; preds = %preload1106.i, %postload1104.i
  %phi1108.i = phi float [ undef, %postload1104.i ], [ %masked_load928.i, %preload1106.i ]
  br i1 %extract411.i, label %preload1109.i, label %postload1110.i

preload1109.i:                                    ; preds = %postload1107.i
  %masked_load929.i = load float addrspace(1)* %300, align 4
  br label %postload1110.i

postload1110.i:                                   ; preds = %preload1109.i, %postload1107.i
  %phi1111.i = phi float [ undef, %postload1107.i ], [ %masked_load929.i, %preload1109.i ]
  br i1 %extract412.i, label %preload1112.i, label %postload1113.i

preload1112.i:                                    ; preds = %postload1110.i
  %masked_load930.i = load float addrspace(1)* %303, align 4
  br label %postload1113.i

postload1113.i:                                   ; preds = %preload1112.i, %postload1110.i
  %phi1114.i = phi float [ undef, %postload1110.i ], [ %masked_load930.i, %preload1112.i ]
  br i1 %extract413.i, label %preload1115.i, label %postload1116.i

preload1115.i:                                    ; preds = %postload1113.i
  %masked_load931.i = load float addrspace(1)* %306, align 4
  br label %postload1116.i

postload1116.i:                                   ; preds = %preload1115.i, %postload1113.i
  %phi1117.i = phi float [ undef, %postload1113.i ], [ %masked_load931.i, %preload1115.i ]
  br i1 %extract414.i, label %preload1118.i, label %postload1119.i

preload1118.i:                                    ; preds = %postload1116.i
  %masked_load932.i = load float addrspace(1)* %309, align 4
  br label %postload1119.i

postload1119.i:                                   ; preds = %preload1118.i, %postload1116.i
  %phi1120.i = phi float [ undef, %postload1116.i ], [ %masked_load932.i, %preload1118.i ]
  br i1 %extract415.i, label %preload1121.i, label %postload1122.i

preload1121.i:                                    ; preds = %postload1119.i
  %masked_load933.i = load float addrspace(1)* %312, align 4
  br label %postload1122.i

postload1122.i:                                   ; preds = %preload1121.i, %postload1119.i
  %phi1123.i = phi float [ undef, %postload1119.i ], [ %masked_load933.i, %preload1121.i ]
  br i1 %extract416.i, label %preload1124.i, label %postload1125.i

preload1124.i:                                    ; preds = %postload1122.i
  %masked_load934.i = load float addrspace(1)* %315, align 4
  br label %postload1125.i

postload1125.i:                                   ; preds = %preload1124.i, %postload1122.i
  %phi1126.i = phi float [ undef, %postload1122.i ], [ %masked_load934.i, %preload1124.i ]
  %temp.vect417.i = insertelement <16 x float> undef, float %phi1081.i, i32 0
  %temp.vect418.i = insertelement <16 x float> %temp.vect417.i, float %phi1084.i, i32 1
  %temp.vect419.i = insertelement <16 x float> %temp.vect418.i, float %phi1087.i, i32 2
  %temp.vect420.i = insertelement <16 x float> %temp.vect419.i, float %phi1090.i, i32 3
  %temp.vect421.i = insertelement <16 x float> %temp.vect420.i, float %phi1093.i, i32 4
  %temp.vect422.i = insertelement <16 x float> %temp.vect421.i, float %phi1096.i, i32 5
  %temp.vect423.i = insertelement <16 x float> %temp.vect422.i, float %phi1099.i, i32 6
  %temp.vect424.i = insertelement <16 x float> %temp.vect423.i, float %phi1102.i, i32 7
  %temp.vect425.i = insertelement <16 x float> %temp.vect424.i, float %phi1105.i, i32 8
  %temp.vect426.i = insertelement <16 x float> %temp.vect425.i, float %phi1108.i, i32 9
  %temp.vect427.i = insertelement <16 x float> %temp.vect426.i, float %phi1111.i, i32 10
  %temp.vect428.i = insertelement <16 x float> %temp.vect427.i, float %phi1114.i, i32 11
  %temp.vect429.i = insertelement <16 x float> %temp.vect428.i, float %phi1117.i, i32 12
  %temp.vect430.i = insertelement <16 x float> %temp.vect429.i, float %phi1120.i, i32 13
  %temp.vect431.i = insertelement <16 x float> %temp.vect430.i, float %phi1123.i, i32 14
  %temp.vect432.i = insertelement <16 x float> %temp.vect431.i, float %phi1126.i, i32 15
  %merge154433.i = select <16 x i1> %_to_78384.i, <16 x float> %temp.vect432.i, <16 x float> zeroinitializer
  %319 = fadd <16 x float> %merge154433.i, %268
  %320 = fadd <16 x float> %319, zeroinitializer
  %merge164434.i = select <16 x i1> %_Min118188.i, <16 x float> %168, <16 x float> %320
  %"&(pSB[currWI].offset)1655.i" = add nuw i64 %CurrSBIndex..1.i, 576
  %"&pSB[currWI].offset1656.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1655.i"
  %CastToValueType1657.i = bitcast i8* %"&pSB[currWI].offset1656.i" to <16 x float>*
  store <16 x float> %merge164434.i, <16 x float>* %CastToValueType1657.i, align 64
  %merge162435.i = select <16 x i1> %_Min118188.i, <16 x float> %168, <16 x float> %320
  %"&(pSB[currWI].offset)1664.i" = add nuw i64 %CurrSBIndex..1.i, 640
  %"&pSB[currWI].offset1665.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1664.i"
  %CastToValueType1666.i = bitcast i8* %"&pSB[currWI].offset1665.i" to <16 x float>*
  store <16 x float> %merge162435.i, <16 x float>* %CastToValueType1666.i, align 64
  %merge160436.i = select <16 x i1> %_Min118188.i, <16 x float> %167, <16 x float> %319
  %"&(pSB[currWI].offset)1673.i" = add nuw i64 %CurrSBIndex..1.i, 704
  %"&pSB[currWI].offset1674.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1673.i"
  %CastToValueType1675.i = bitcast i8* %"&pSB[currWI].offset1674.i" to <16 x float>*
  store <16 x float> %merge160436.i, <16 x float>* %CastToValueType1675.i, align 64
  %merge158437.i = select <16 x i1> %_Min118188.i, <16 x float> %166, <16 x float> %268
  %"&(pSB[currWI].offset)1682.i" = add nuw i64 %CurrSBIndex..1.i, 768
  %"&pSB[currWI].offset1683.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1682.i"
  %CastToValueType1684.i = bitcast i8* %"&pSB[currWI].offset1683.i" to <16 x float>*
  store <16 x float> %merge158437.i, <16 x float>* %CastToValueType1684.i, align 64
  %merge156438.i = select <16 x i1> %_Min118188.i, <16 x float> %temp.vect251.i, <16 x float> %merge333.i
  %"&(pSB[currWI].offset)1691.i" = add nuw i64 %CurrSBIndex..1.i, 832
  %"&pSB[currWI].offset1692.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1691.i"
  %CastToValueType1693.i = bitcast i8* %"&pSB[currWI].offset1692.i" to <16 x float>*
  store <16 x float> %merge156438.i, <16 x float>* %CastToValueType1693.i, align 64
  %check.WI.iter2201.i = icmp ult i64 %CurrWI..1.i, %34
  br i1 %check.WI.iter2201.i, label %thenBB2198.i, label %SyncBB2175.i

thenBB2198.i:                                     ; preds = %postload1125.i
  %"CurrWI++2202.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride2204.i" = add nuw i64 %CurrSBIndex..1.i, 1792
  br label %SyncBB2176.i

SyncBB2175.i:                                     ; preds = %thenBB2261.i, %postload1125.i
  %CurrSBIndex..9.i = phi i64 [ %"loadedCurrSB+Stride2267.i", %thenBB2261.i ], [ 0, %postload1125.i ]
  %CurrWI..9.i = phi i64 [ %"CurrWI++2265.i", %thenBB2261.i ], [ 0, %postload1125.i ]
  %321 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..9.i, i32 0, i64 0
  %322 = load i64* %321, align 8
  %broadcast1439.i = insertelement <16 x i64> undef, i64 %322, i32 0
  %broadcast2440.i = shufflevector <16 x i64> %broadcast1439.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %323 = add <16 x i64> %broadcast2440.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %"&(pSB[currWI].offset)1700.i" = add nuw i64 %CurrSBIndex..9.i, 896
  %"&pSB[currWI].offset1701.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1700.i"
  %CastToValueType1702.i = bitcast i8* %"&pSB[currWI].offset1701.i" to <16 x i64>*
  store <16 x i64> %323, <16 x i64>* %CastToValueType1702.i, align 128
  %extract442.lhs.lhs.i = extractelement <16 x i64> %323, i32 0
  %extract442.lhs.i = shl i64 %extract442.lhs.lhs.i, 32
  %extract442.i = ashr i64 %extract442.lhs.i, 32
  %324 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract442.i
  %ptrTypeCast.i = bitcast float addrspace(3)* %324 to <16 x float> addrspace(3)*
  store <16 x float> zeroinitializer, <16 x float> addrspace(3)* %ptrTypeCast.i, align 4
  %check.WI.iter2264.i = icmp ult i64 %CurrWI..9.i, %34
  br i1 %check.WI.iter2264.i, label %thenBB2261.i, label %SyncBB2185.i

thenBB2261.i:                                     ; preds = %SyncBB2175.i
  %"CurrWI++2265.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride2267.i" = add nuw i64 %CurrSBIndex..9.i, 1792
  br label %SyncBB2175.i

SyncBB2185.i:                                     ; preds = %thenBB2219.i, %SyncBB2175.i
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride2225.i", %thenBB2219.i ], [ 0, %SyncBB2175.i ]
  %CurrWI..3.i = phi i64 [ %"CurrWI++2223.i", %thenBB2219.i ], [ 0, %SyncBB2175.i ]
  %325 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %326 = load i64* %325, align 8
  %temp458.i = insertelement <16 x i64> undef, i64 %326, i32 0
  %vector459.i = shufflevector <16 x i64> %temp458.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)1704.i" = add nuw i64 %CurrSBIndex..3.i, 896
  %"&pSB[currWI].offset1705.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1704.i"
  %CastToValueType1706.i = bitcast i8* %"&pSB[currWI].offset1705.i" to <16 x i64>*
  %loadedValue1707.i = load <16 x i64>* %CastToValueType1706.i, align 128
  %327 = add <16 x i64> %vector459.i, %loadedValue1707.i
  %328 = trunc <16 x i64> %327 to <16 x i32>
  %"&(pSB[currWI].offset)1709.i" = add nuw i64 %CurrSBIndex..3.i, 1024
  %"&pSB[currWI].offset1710.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1709.i"
  %CastToValueType1711.i = bitcast i8* %"&pSB[currWI].offset1710.i" to <16 x i32>*
  store <16 x i32> %328, <16 x i32>* %CastToValueType1711.i, align 64
  %329 = extractelement <16 x i32> %328, i32 0
  %extract460.i = sext i32 %329 to i64
  %330 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract460.i
  %"&(pSB[currWI].offset)1753.i" = add nuw i64 %CurrSBIndex..3.i, 1088
  %"&pSB[currWI].offset1754.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1753.i"
  %CastToValueType1755.i = bitcast i8* %"&pSB[currWI].offset1754.i" to float addrspace(3)**
  store float addrspace(3)* %330, float addrspace(3)** %CastToValueType1755.i, align 8
  %ptrTypeCast476.i = bitcast float addrspace(3)* %330 to <16 x float> addrspace(3)*
  %"&(pSB[currWI].offset)1659.i" = add nuw i64 %CurrSBIndex..3.i, 576
  %"&pSB[currWI].offset1660.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1659.i"
  %CastToValueType1661.i = bitcast i8* %"&pSB[currWI].offset1660.i" to <16 x float>*
  %loadedValue1662.i = load <16 x float>* %CastToValueType1661.i, align 64
  store <16 x float> %loadedValue1662.i, <16 x float> addrspace(3)* %ptrTypeCast476.i, align 4
  %check.WI.iter2222.i = icmp ult i64 %CurrWI..3.i, %34
  br i1 %check.WI.iter2222.i, label %thenBB2219.i, label %SyncBB2179.i

thenBB2219.i:                                     ; preds = %SyncBB2185.i
  %"CurrWI++2223.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride2225.i" = add nuw i64 %CurrSBIndex..3.i, 1792
  br label %SyncBB2185.i

SyncBB2179.i:                                     ; preds = %thenBB2268.i, %SyncBB2185.i
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride2274.i", %thenBB2268.i ], [ 0, %SyncBB2185.i ]
  %CurrWI..10.i = phi i64 [ %"CurrWI++2272.i", %thenBB2268.i ], [ 0, %SyncBB2185.i ]
  %"&(pSB[currWI].offset)1748.i" = add nuw i64 %CurrSBIndex..10.i, 1024
  %"&pSB[currWI].offset1749.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1748.i"
  %CastToValueType1750.i = bitcast i8* %"&pSB[currWI].offset1749.i" to <16 x i32>*
  %loadedValue1751.i = load <16 x i32>* %CastToValueType1750.i, align 64
  %.lhs.i = extractelement <16 x i32> %loadedValue1751.i, i32 0
  %331 = add i32 %.lhs.i, -1
  %extract477.i = sext i32 %331 to i64
  %332 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract477.i
  %"&(pSB[currWI].offset)1837.i" = add nuw i64 %CurrSBIndex..10.i, 1096
  %"&pSB[currWI].offset1838.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1837.i"
  %CastToValueType1839.i = bitcast i8* %"&pSB[currWI].offset1838.i" to float addrspace(3)**
  store float addrspace(3)* %332, float addrspace(3)** %CastToValueType1839.i, align 8
  %ptrTypeCast493.i = bitcast float addrspace(3)* %332 to <16 x float> addrspace(3)*
  %333 = load <16 x float> addrspace(3)* %ptrTypeCast493.i, align 4
  %"&(pSB[currWI].offset)1846.i" = add nuw i64 %CurrSBIndex..10.i, 1152
  %"&pSB[currWI].offset1847.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1846.i"
  %CastToValueType1848.i = bitcast i8* %"&pSB[currWI].offset1847.i" to <16 x float>*
  store <16 x float> %333, <16 x float>* %CastToValueType1848.i, align 64
  %check.WI.iter2271.i = icmp ult i64 %CurrWI..10.i, %34
  br i1 %check.WI.iter2271.i, label %thenBB2268.i, label %SyncBB2186.i

thenBB2268.i:                                     ; preds = %SyncBB2179.i
  %"CurrWI++2272.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride2274.i" = add nuw i64 %CurrSBIndex..10.i, 1792
  br label %SyncBB2179.i

SyncBB2186.i:                                     ; preds = %thenBB2275.i, %SyncBB2179.i
  %CurrSBIndex..11.i = phi i64 [ %"loadedCurrSB+Stride2281.i", %thenBB2275.i ], [ 0, %SyncBB2179.i ]
  %CurrWI..11.i = phi i64 [ %"CurrWI++2279.i", %thenBB2275.i ], [ 0, %SyncBB2179.i ]
  %"&(pSB[currWI].offset)1832.i" = add nuw i64 %CurrSBIndex..11.i, 1088
  %"&pSB[currWI].offset1833.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1832.i"
  %CastToValueType1834.i = bitcast i8* %"&pSB[currWI].offset1833.i" to float addrspace(3)**
  %loadedValue1835.i = load float addrspace(3)** %CastToValueType1834.i, align 8
  %ptrTypeCast494.i = bitcast float addrspace(3)* %loadedValue1835.i to <16 x float> addrspace(3)*
  %334 = load <16 x float> addrspace(3)* %ptrTypeCast494.i, align 4
  %"&(pSB[currWI].offset)1850.i" = add nuw i64 %CurrSBIndex..11.i, 1152
  %"&pSB[currWI].offset1851.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1850.i"
  %CastToValueType1852.i = bitcast i8* %"&pSB[currWI].offset1851.i" to <16 x float>*
  %loadedValue1853.i = load <16 x float>* %CastToValueType1852.i, align 64
  %335 = fadd <16 x float> %334, %loadedValue1853.i
  %"&(pSB[currWI].offset)1827.i" = add nuw i64 %CurrSBIndex..11.i, 1088
  %"&pSB[currWI].offset1828.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1827.i"
  %CastToValueType1829.i = bitcast i8* %"&pSB[currWI].offset1828.i" to float addrspace(3)**
  %loadedValue1830.i = load float addrspace(3)** %CastToValueType1829.i, align 8
  %ptrTypeCast495.i = bitcast float addrspace(3)* %loadedValue1830.i to <16 x float> addrspace(3)*
  store <16 x float> %335, <16 x float> addrspace(3)* %ptrTypeCast495.i, align 4
  %check.WI.iter2278.i = icmp ult i64 %CurrWI..11.i, %34
  br i1 %check.WI.iter2278.i, label %thenBB2275.i, label %SyncBB2187.i

thenBB2275.i:                                     ; preds = %SyncBB2186.i
  %"CurrWI++2279.i" = add nuw i64 %CurrWI..11.i, 1
  %"loadedCurrSB+Stride2281.i" = add nuw i64 %CurrSBIndex..11.i, 1792
  br label %SyncBB2186.i

SyncBB2187.i:                                     ; preds = %thenBB2282.i, %SyncBB2186.i
  %CurrSBIndex..12.i = phi i64 [ %"loadedCurrSB+Stride2288.i", %thenBB2282.i ], [ 0, %SyncBB2186.i ]
  %CurrWI..12.i = phi i64 [ %"CurrWI++2286.i", %thenBB2282.i ], [ 0, %SyncBB2186.i ]
  %"&(pSB[currWI].offset)1743.i" = add nuw i64 %CurrSBIndex..12.i, 1024
  %"&pSB[currWI].offset1744.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1743.i"
  %CastToValueType1745.i = bitcast i8* %"&pSB[currWI].offset1744.i" to <16 x i32>*
  %loadedValue1746.i = load <16 x i32>* %CastToValueType1745.i, align 64
  %.lhs1335.i = extractelement <16 x i32> %loadedValue1746.i, i32 0
  %336 = add i32 %.lhs1335.i, -2
  %extract496.i = sext i32 %336 to i64
  %337 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract496.i
  %ptrTypeCast512.i = bitcast float addrspace(3)* %337 to <16 x float> addrspace(3)*
  %338 = load <16 x float> addrspace(3)* %ptrTypeCast512.i, align 4
  %"&(pSB[currWI].offset)1855.i" = add nuw i64 %CurrSBIndex..12.i, 1216
  %"&pSB[currWI].offset1856.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1855.i"
  %CastToValueType1857.i = bitcast i8* %"&pSB[currWI].offset1856.i" to <16 x float>*
  store <16 x float> %338, <16 x float>* %CastToValueType1857.i, align 64
  %check.WI.iter2285.i = icmp ult i64 %CurrWI..12.i, %34
  br i1 %check.WI.iter2285.i, label %thenBB2282.i, label %SyncBB2188.i

thenBB2282.i:                                     ; preds = %SyncBB2187.i
  %"CurrWI++2286.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride2288.i" = add nuw i64 %CurrSBIndex..12.i, 1792
  br label %SyncBB2187.i

SyncBB2188.i:                                     ; preds = %thenBB2289.i, %SyncBB2187.i
  %CurrSBIndex..13.i = phi i64 [ %"loadedCurrSB+Stride2295.i", %thenBB2289.i ], [ 0, %SyncBB2187.i ]
  %CurrWI..13.i = phi i64 [ %"CurrWI++2293.i", %thenBB2289.i ], [ 0, %SyncBB2187.i ]
  %"&(pSB[currWI].offset)1822.i" = add nuw i64 %CurrSBIndex..13.i, 1088
  %"&pSB[currWI].offset1823.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1822.i"
  %CastToValueType1824.i = bitcast i8* %"&pSB[currWI].offset1823.i" to float addrspace(3)**
  %loadedValue1825.i = load float addrspace(3)** %CastToValueType1824.i, align 8
  %ptrTypeCast513.i = bitcast float addrspace(3)* %loadedValue1825.i to <16 x float> addrspace(3)*
  %339 = load <16 x float> addrspace(3)* %ptrTypeCast513.i, align 4
  %"&(pSB[currWI].offset)1859.i" = add nuw i64 %CurrSBIndex..13.i, 1216
  %"&pSB[currWI].offset1860.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1859.i"
  %CastToValueType1861.i = bitcast i8* %"&pSB[currWI].offset1860.i" to <16 x float>*
  %loadedValue1862.i = load <16 x float>* %CastToValueType1861.i, align 64
  %340 = fadd <16 x float> %339, %loadedValue1862.i
  %"&(pSB[currWI].offset)1817.i" = add nuw i64 %CurrSBIndex..13.i, 1088
  %"&pSB[currWI].offset1818.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1817.i"
  %CastToValueType1819.i = bitcast i8* %"&pSB[currWI].offset1818.i" to float addrspace(3)**
  %loadedValue1820.i = load float addrspace(3)** %CastToValueType1819.i, align 8
  %ptrTypeCast514.i = bitcast float addrspace(3)* %loadedValue1820.i to <16 x float> addrspace(3)*
  store <16 x float> %340, <16 x float> addrspace(3)* %ptrTypeCast514.i, align 4
  %check.WI.iter2292.i = icmp ult i64 %CurrWI..13.i, %34
  br i1 %check.WI.iter2292.i, label %thenBB2289.i, label %SyncBB2189.i

thenBB2289.i:                                     ; preds = %SyncBB2188.i
  %"CurrWI++2293.i" = add nuw i64 %CurrWI..13.i, 1
  %"loadedCurrSB+Stride2295.i" = add nuw i64 %CurrSBIndex..13.i, 1792
  br label %SyncBB2188.i

SyncBB2189.i:                                     ; preds = %thenBB2296.i, %SyncBB2188.i
  %CurrSBIndex..14.i = phi i64 [ %"loadedCurrSB+Stride2302.i", %thenBB2296.i ], [ 0, %SyncBB2188.i ]
  %CurrWI..14.i = phi i64 [ %"CurrWI++2300.i", %thenBB2296.i ], [ 0, %SyncBB2188.i ]
  %"&(pSB[currWI].offset)1738.i" = add nuw i64 %CurrSBIndex..14.i, 1024
  %"&pSB[currWI].offset1739.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1738.i"
  %CastToValueType1740.i = bitcast i8* %"&pSB[currWI].offset1739.i" to <16 x i32>*
  %loadedValue1741.i = load <16 x i32>* %CastToValueType1740.i, align 64
  %.lhs1336.i = extractelement <16 x i32> %loadedValue1741.i, i32 0
  %341 = add i32 %.lhs1336.i, -4
  %extract515.i = sext i32 %341 to i64
  %342 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract515.i
  %ptrTypeCast531.i = bitcast float addrspace(3)* %342 to <16 x float> addrspace(3)*
  %343 = load <16 x float> addrspace(3)* %ptrTypeCast531.i, align 4
  %"&(pSB[currWI].offset)1864.i" = add nuw i64 %CurrSBIndex..14.i, 1280
  %"&pSB[currWI].offset1865.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1864.i"
  %CastToValueType1866.i = bitcast i8* %"&pSB[currWI].offset1865.i" to <16 x float>*
  store <16 x float> %343, <16 x float>* %CastToValueType1866.i, align 64
  %check.WI.iter2299.i = icmp ult i64 %CurrWI..14.i, %34
  br i1 %check.WI.iter2299.i, label %thenBB2296.i, label %SyncBB2190.i

thenBB2296.i:                                     ; preds = %SyncBB2189.i
  %"CurrWI++2300.i" = add nuw i64 %CurrWI..14.i, 1
  %"loadedCurrSB+Stride2302.i" = add nuw i64 %CurrSBIndex..14.i, 1792
  br label %SyncBB2189.i

SyncBB2190.i:                                     ; preds = %thenBB2226.i, %SyncBB2189.i
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride2232.i", %thenBB2226.i ], [ 0, %SyncBB2189.i ]
  %CurrWI..4.i = phi i64 [ %"CurrWI++2230.i", %thenBB2226.i ], [ 0, %SyncBB2189.i ]
  %"&(pSB[currWI].offset)1812.i" = add nuw i64 %CurrSBIndex..4.i, 1088
  %"&pSB[currWI].offset1813.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1812.i"
  %CastToValueType1814.i = bitcast i8* %"&pSB[currWI].offset1813.i" to float addrspace(3)**
  %loadedValue1815.i = load float addrspace(3)** %CastToValueType1814.i, align 8
  %ptrTypeCast532.i = bitcast float addrspace(3)* %loadedValue1815.i to <16 x float> addrspace(3)*
  %344 = load <16 x float> addrspace(3)* %ptrTypeCast532.i, align 4
  %"&(pSB[currWI].offset)1868.i" = add nuw i64 %CurrSBIndex..4.i, 1280
  %"&pSB[currWI].offset1869.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1868.i"
  %CastToValueType1870.i = bitcast i8* %"&pSB[currWI].offset1869.i" to <16 x float>*
  %loadedValue1871.i = load <16 x float>* %CastToValueType1870.i, align 64
  %345 = fadd <16 x float> %344, %loadedValue1871.i
  %"&(pSB[currWI].offset)1807.i" = add nuw i64 %CurrSBIndex..4.i, 1088
  %"&pSB[currWI].offset1808.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1807.i"
  %CastToValueType1809.i = bitcast i8* %"&pSB[currWI].offset1808.i" to float addrspace(3)**
  %loadedValue1810.i = load float addrspace(3)** %CastToValueType1809.i, align 8
  %ptrTypeCast533.i = bitcast float addrspace(3)* %loadedValue1810.i to <16 x float> addrspace(3)*
  store <16 x float> %345, <16 x float> addrspace(3)* %ptrTypeCast533.i, align 4
  %check.WI.iter2229.i = icmp ult i64 %CurrWI..4.i, %34
  br i1 %check.WI.iter2229.i, label %thenBB2226.i, label %SyncBB2180.i

thenBB2226.i:                                     ; preds = %SyncBB2190.i
  %"CurrWI++2230.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride2232.i" = add nuw i64 %CurrSBIndex..4.i, 1792
  br label %SyncBB2190.i

SyncBB2180.i:                                     ; preds = %thenBB2233.i, %SyncBB2190.i
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride2239.i", %thenBB2233.i ], [ 0, %SyncBB2190.i ]
  %CurrWI..5.i = phi i64 [ %"CurrWI++2237.i", %thenBB2233.i ], [ 0, %SyncBB2190.i ]
  %"&(pSB[currWI].offset)1733.i" = add nuw i64 %CurrSBIndex..5.i, 1024
  %"&pSB[currWI].offset1734.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1733.i"
  %CastToValueType1735.i = bitcast i8* %"&pSB[currWI].offset1734.i" to <16 x i32>*
  %loadedValue1736.i = load <16 x i32>* %CastToValueType1735.i, align 64
  %.lhs1337.i = extractelement <16 x i32> %loadedValue1736.i, i32 0
  %346 = add i32 %.lhs1337.i, -8
  %extract534.i = sext i32 %346 to i64
  %347 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract534.i
  %ptrTypeCast550.i = bitcast float addrspace(3)* %347 to <16 x float> addrspace(3)*
  %348 = load <16 x float> addrspace(3)* %ptrTypeCast550.i, align 4
  %"&(pSB[currWI].offset)1873.i" = add nuw i64 %CurrSBIndex..5.i, 1344
  %"&pSB[currWI].offset1874.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1873.i"
  %CastToValueType1875.i = bitcast i8* %"&pSB[currWI].offset1874.i" to <16 x float>*
  store <16 x float> %348, <16 x float>* %CastToValueType1875.i, align 64
  %check.WI.iter2236.i = icmp ult i64 %CurrWI..5.i, %34
  br i1 %check.WI.iter2236.i, label %thenBB2233.i, label %SyncBB2181.i

thenBB2233.i:                                     ; preds = %SyncBB2180.i
  %"CurrWI++2237.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride2239.i" = add nuw i64 %CurrSBIndex..5.i, 1792
  br label %SyncBB2180.i

SyncBB2181.i:                                     ; preds = %thenBB2240.i, %SyncBB2180.i
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride2246.i", %thenBB2240.i ], [ 0, %SyncBB2180.i ]
  %CurrWI..6.i = phi i64 [ %"CurrWI++2244.i", %thenBB2240.i ], [ 0, %SyncBB2180.i ]
  %"&(pSB[currWI].offset)1802.i" = add nuw i64 %CurrSBIndex..6.i, 1088
  %"&pSB[currWI].offset1803.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1802.i"
  %CastToValueType1804.i = bitcast i8* %"&pSB[currWI].offset1803.i" to float addrspace(3)**
  %loadedValue1805.i = load float addrspace(3)** %CastToValueType1804.i, align 8
  %ptrTypeCast551.i = bitcast float addrspace(3)* %loadedValue1805.i to <16 x float> addrspace(3)*
  %349 = load <16 x float> addrspace(3)* %ptrTypeCast551.i, align 4
  %"&(pSB[currWI].offset)1877.i" = add nuw i64 %CurrSBIndex..6.i, 1344
  %"&pSB[currWI].offset1878.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1877.i"
  %CastToValueType1879.i = bitcast i8* %"&pSB[currWI].offset1878.i" to <16 x float>*
  %loadedValue1880.i = load <16 x float>* %CastToValueType1879.i, align 64
  %350 = fadd <16 x float> %349, %loadedValue1880.i
  %"&(pSB[currWI].offset)1797.i" = add nuw i64 %CurrSBIndex..6.i, 1088
  %"&pSB[currWI].offset1798.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1797.i"
  %CastToValueType1799.i = bitcast i8* %"&pSB[currWI].offset1798.i" to float addrspace(3)**
  %loadedValue1800.i = load float addrspace(3)** %CastToValueType1799.i, align 8
  %ptrTypeCast552.i = bitcast float addrspace(3)* %loadedValue1800.i to <16 x float> addrspace(3)*
  store <16 x float> %350, <16 x float> addrspace(3)* %ptrTypeCast552.i, align 4
  %check.WI.iter2243.i = icmp ult i64 %CurrWI..6.i, %34
  br i1 %check.WI.iter2243.i, label %thenBB2240.i, label %SyncBB2182.i

thenBB2240.i:                                     ; preds = %SyncBB2181.i
  %"CurrWI++2244.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride2246.i" = add nuw i64 %CurrSBIndex..6.i, 1792
  br label %SyncBB2181.i

SyncBB2182.i:                                     ; preds = %thenBB2247.i, %SyncBB2181.i
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride2253.i", %thenBB2247.i ], [ 0, %SyncBB2181.i ]
  %CurrWI..7.i = phi i64 [ %"CurrWI++2251.i", %thenBB2247.i ], [ 0, %SyncBB2181.i ]
  %"&(pSB[currWI].offset)1728.i" = add nuw i64 %CurrSBIndex..7.i, 1024
  %"&pSB[currWI].offset1729.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1728.i"
  %CastToValueType1730.i = bitcast i8* %"&pSB[currWI].offset1729.i" to <16 x i32>*
  %loadedValue1731.i = load <16 x i32>* %CastToValueType1730.i, align 64
  %.lhs1338.i = extractelement <16 x i32> %loadedValue1731.i, i32 0
  %351 = add i32 %.lhs1338.i, -16
  %extract553.i = sext i32 %351 to i64
  %352 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract553.i
  %ptrTypeCast569.i = bitcast float addrspace(3)* %352 to <16 x float> addrspace(3)*
  %353 = load <16 x float> addrspace(3)* %ptrTypeCast569.i, align 4
  %"&(pSB[currWI].offset)1882.i" = add nuw i64 %CurrSBIndex..7.i, 1408
  %"&pSB[currWI].offset1883.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1882.i"
  %CastToValueType1884.i = bitcast i8* %"&pSB[currWI].offset1883.i" to <16 x float>*
  store <16 x float> %353, <16 x float>* %CastToValueType1884.i, align 64
  %check.WI.iter2250.i = icmp ult i64 %CurrWI..7.i, %34
  br i1 %check.WI.iter2250.i, label %thenBB2247.i, label %SyncBB2183.i

thenBB2247.i:                                     ; preds = %SyncBB2182.i
  %"CurrWI++2251.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride2253.i" = add nuw i64 %CurrSBIndex..7.i, 1792
  br label %SyncBB2182.i

SyncBB2183.i:                                     ; preds = %thenBB2254.i, %SyncBB2182.i
  %CurrSBIndex..8.i = phi i64 [ %"loadedCurrSB+Stride2260.i", %thenBB2254.i ], [ 0, %SyncBB2182.i ]
  %CurrWI..8.i = phi i64 [ %"CurrWI++2258.i", %thenBB2254.i ], [ 0, %SyncBB2182.i ]
  %"&(pSB[currWI].offset)1792.i" = add nuw i64 %CurrSBIndex..8.i, 1088
  %"&pSB[currWI].offset1793.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1792.i"
  %CastToValueType1794.i = bitcast i8* %"&pSB[currWI].offset1793.i" to float addrspace(3)**
  %loadedValue1795.i = load float addrspace(3)** %CastToValueType1794.i, align 8
  %ptrTypeCast570.i = bitcast float addrspace(3)* %loadedValue1795.i to <16 x float> addrspace(3)*
  %354 = load <16 x float> addrspace(3)* %ptrTypeCast570.i, align 4
  %"&(pSB[currWI].offset)1886.i" = add nuw i64 %CurrSBIndex..8.i, 1408
  %"&pSB[currWI].offset1887.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1886.i"
  %CastToValueType1888.i = bitcast i8* %"&pSB[currWI].offset1887.i" to <16 x float>*
  %loadedValue1889.i = load <16 x float>* %CastToValueType1888.i, align 64
  %355 = fadd <16 x float> %354, %loadedValue1889.i
  %"&(pSB[currWI].offset)1787.i" = add nuw i64 %CurrSBIndex..8.i, 1088
  %"&pSB[currWI].offset1788.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1787.i"
  %CastToValueType1789.i = bitcast i8* %"&pSB[currWI].offset1788.i" to float addrspace(3)**
  %loadedValue1790.i = load float addrspace(3)** %CastToValueType1789.i, align 8
  %ptrTypeCast571.i = bitcast float addrspace(3)* %loadedValue1790.i to <16 x float> addrspace(3)*
  store <16 x float> %355, <16 x float> addrspace(3)* %ptrTypeCast571.i, align 4
  %check.WI.iter2257.i = icmp ult i64 %CurrWI..8.i, %34
  br i1 %check.WI.iter2257.i, label %thenBB2254.i, label %SyncBB2184.i

thenBB2254.i:                                     ; preds = %SyncBB2183.i
  %"CurrWI++2258.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride2260.i" = add nuw i64 %CurrSBIndex..8.i, 1792
  br label %SyncBB2183.i

SyncBB2184.i:                                     ; preds = %thenBB2303.i, %SyncBB2183.i
  %CurrSBIndex..15.i = phi i64 [ %"loadedCurrSB+Stride2309.i", %thenBB2303.i ], [ 0, %SyncBB2183.i ]
  %CurrWI..15.i = phi i64 [ %"CurrWI++2307.i", %thenBB2303.i ], [ 0, %SyncBB2183.i ]
  %"&(pSB[currWI].offset)1723.i" = add nuw i64 %CurrSBIndex..15.i, 1024
  %"&pSB[currWI].offset1724.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1723.i"
  %CastToValueType1725.i = bitcast i8* %"&pSB[currWI].offset1724.i" to <16 x i32>*
  %loadedValue1726.i = load <16 x i32>* %CastToValueType1725.i, align 64
  %.lhs1339.i = extractelement <16 x i32> %loadedValue1726.i, i32 0
  %356 = add i32 %.lhs1339.i, -32
  %extract572.i = sext i32 %356 to i64
  %357 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract572.i
  %ptrTypeCast588.i = bitcast float addrspace(3)* %357 to <16 x float> addrspace(3)*
  %358 = load <16 x float> addrspace(3)* %ptrTypeCast588.i, align 4
  %"&(pSB[currWI].offset)1891.i" = add nuw i64 %CurrSBIndex..15.i, 1472
  %"&pSB[currWI].offset1892.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1891.i"
  %CastToValueType1893.i = bitcast i8* %"&pSB[currWI].offset1892.i" to <16 x float>*
  store <16 x float> %358, <16 x float>* %CastToValueType1893.i, align 64
  %check.WI.iter2306.i = icmp ult i64 %CurrWI..15.i, %34
  br i1 %check.WI.iter2306.i, label %thenBB2303.i, label %SyncBB2191.i

thenBB2303.i:                                     ; preds = %SyncBB2184.i
  %"CurrWI++2307.i" = add nuw i64 %CurrWI..15.i, 1
  %"loadedCurrSB+Stride2309.i" = add nuw i64 %CurrSBIndex..15.i, 1792
  br label %SyncBB2184.i

SyncBB2191.i:                                     ; preds = %thenBB2310.i, %SyncBB2184.i
  %CurrSBIndex..16.i = phi i64 [ %"loadedCurrSB+Stride2316.i", %thenBB2310.i ], [ 0, %SyncBB2184.i ]
  %CurrWI..16.i = phi i64 [ %"CurrWI++2314.i", %thenBB2310.i ], [ 0, %SyncBB2184.i ]
  %"&(pSB[currWI].offset)1782.i" = add nuw i64 %CurrSBIndex..16.i, 1088
  %"&pSB[currWI].offset1783.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1782.i"
  %CastToValueType1784.i = bitcast i8* %"&pSB[currWI].offset1783.i" to float addrspace(3)**
  %loadedValue1785.i = load float addrspace(3)** %CastToValueType1784.i, align 8
  %ptrTypeCast589.i = bitcast float addrspace(3)* %loadedValue1785.i to <16 x float> addrspace(3)*
  %359 = load <16 x float> addrspace(3)* %ptrTypeCast589.i, align 4
  %"&(pSB[currWI].offset)1895.i" = add nuw i64 %CurrSBIndex..16.i, 1472
  %"&pSB[currWI].offset1896.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1895.i"
  %CastToValueType1897.i = bitcast i8* %"&pSB[currWI].offset1896.i" to <16 x float>*
  %loadedValue1898.i = load <16 x float>* %CastToValueType1897.i, align 64
  %360 = fadd <16 x float> %359, %loadedValue1898.i
  %"&(pSB[currWI].offset)1777.i" = add nuw i64 %CurrSBIndex..16.i, 1088
  %"&pSB[currWI].offset1778.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1777.i"
  %CastToValueType1779.i = bitcast i8* %"&pSB[currWI].offset1778.i" to float addrspace(3)**
  %loadedValue1780.i = load float addrspace(3)** %CastToValueType1779.i, align 8
  %ptrTypeCast590.i = bitcast float addrspace(3)* %loadedValue1780.i to <16 x float> addrspace(3)*
  store <16 x float> %360, <16 x float> addrspace(3)* %ptrTypeCast590.i, align 4
  %check.WI.iter2313.i = icmp ult i64 %CurrWI..16.i, %34
  br i1 %check.WI.iter2313.i, label %thenBB2310.i, label %SyncBB2192.i

thenBB2310.i:                                     ; preds = %SyncBB2191.i
  %"CurrWI++2314.i" = add nuw i64 %CurrWI..16.i, 1
  %"loadedCurrSB+Stride2316.i" = add nuw i64 %CurrSBIndex..16.i, 1792
  br label %SyncBB2191.i

SyncBB2192.i:                                     ; preds = %thenBB2317.i, %SyncBB2191.i
  %CurrSBIndex..17.i = phi i64 [ %"loadedCurrSB+Stride2323.i", %thenBB2317.i ], [ 0, %SyncBB2191.i ]
  %CurrWI..17.i = phi i64 [ %"CurrWI++2321.i", %thenBB2317.i ], [ 0, %SyncBB2191.i ]
  %"&(pSB[currWI].offset)1718.i" = add nuw i64 %CurrSBIndex..17.i, 1024
  %"&pSB[currWI].offset1719.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1718.i"
  %CastToValueType1720.i = bitcast i8* %"&pSB[currWI].offset1719.i" to <16 x i32>*
  %loadedValue1721.i = load <16 x i32>* %CastToValueType1720.i, align 64
  %.lhs1340.i = extractelement <16 x i32> %loadedValue1721.i, i32 0
  %361 = add i32 %.lhs1340.i, -64
  %extract591.i = sext i32 %361 to i64
  %362 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract591.i
  %ptrTypeCast607.i = bitcast float addrspace(3)* %362 to <16 x float> addrspace(3)*
  %363 = load <16 x float> addrspace(3)* %ptrTypeCast607.i, align 4
  %"&(pSB[currWI].offset)1900.i" = add nuw i64 %CurrSBIndex..17.i, 1536
  %"&pSB[currWI].offset1901.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1900.i"
  %CastToValueType1902.i = bitcast i8* %"&pSB[currWI].offset1901.i" to <16 x float>*
  store <16 x float> %363, <16 x float>* %CastToValueType1902.i, align 64
  %check.WI.iter2320.i = icmp ult i64 %CurrWI..17.i, %34
  br i1 %check.WI.iter2320.i, label %thenBB2317.i, label %SyncBB2193.i

thenBB2317.i:                                     ; preds = %SyncBB2192.i
  %"CurrWI++2321.i" = add nuw i64 %CurrWI..17.i, 1
  %"loadedCurrSB+Stride2323.i" = add nuw i64 %CurrSBIndex..17.i, 1792
  br label %SyncBB2192.i

SyncBB2193.i:                                     ; preds = %thenBB2324.i, %SyncBB2192.i
  %CurrSBIndex..18.i = phi i64 [ %"loadedCurrSB+Stride2330.i", %thenBB2324.i ], [ 0, %SyncBB2192.i ]
  %CurrWI..18.i = phi i64 [ %"CurrWI++2328.i", %thenBB2324.i ], [ 0, %SyncBB2192.i ]
  %"&(pSB[currWI].offset)1772.i" = add nuw i64 %CurrSBIndex..18.i, 1088
  %"&pSB[currWI].offset1773.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1772.i"
  %CastToValueType1774.i = bitcast i8* %"&pSB[currWI].offset1773.i" to float addrspace(3)**
  %loadedValue1775.i = load float addrspace(3)** %CastToValueType1774.i, align 8
  %ptrTypeCast608.i = bitcast float addrspace(3)* %loadedValue1775.i to <16 x float> addrspace(3)*
  %364 = load <16 x float> addrspace(3)* %ptrTypeCast608.i, align 4
  %"&(pSB[currWI].offset)1904.i" = add nuw i64 %CurrSBIndex..18.i, 1536
  %"&pSB[currWI].offset1905.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1904.i"
  %CastToValueType1906.i = bitcast i8* %"&pSB[currWI].offset1905.i" to <16 x float>*
  %loadedValue1907.i = load <16 x float>* %CastToValueType1906.i, align 64
  %365 = fadd <16 x float> %364, %loadedValue1907.i
  %"&(pSB[currWI].offset)1767.i" = add nuw i64 %CurrSBIndex..18.i, 1088
  %"&pSB[currWI].offset1768.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1767.i"
  %CastToValueType1769.i = bitcast i8* %"&pSB[currWI].offset1768.i" to float addrspace(3)**
  %loadedValue1770.i = load float addrspace(3)** %CastToValueType1769.i, align 8
  %ptrTypeCast609.i = bitcast float addrspace(3)* %loadedValue1770.i to <16 x float> addrspace(3)*
  store <16 x float> %365, <16 x float> addrspace(3)* %ptrTypeCast609.i, align 4
  %check.WI.iter2327.i = icmp ult i64 %CurrWI..18.i, %34
  br i1 %check.WI.iter2327.i, label %thenBB2324.i, label %SyncBB2194.i

thenBB2324.i:                                     ; preds = %SyncBB2193.i
  %"CurrWI++2328.i" = add nuw i64 %CurrWI..18.i, 1
  %"loadedCurrSB+Stride2330.i" = add nuw i64 %CurrSBIndex..18.i, 1792
  br label %SyncBB2193.i

SyncBB2194.i:                                     ; preds = %thenBB2331.i, %SyncBB2193.i
  %CurrSBIndex..19.i = phi i64 [ %"loadedCurrSB+Stride2337.i", %thenBB2331.i ], [ 0, %SyncBB2193.i ]
  %CurrWI..19.i = phi i64 [ %"CurrWI++2335.i", %thenBB2331.i ], [ 0, %SyncBB2193.i ]
  %"&(pSB[currWI].offset)1713.i" = add nuw i64 %CurrSBIndex..19.i, 1024
  %"&pSB[currWI].offset1714.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1713.i"
  %CastToValueType1715.i = bitcast i8* %"&pSB[currWI].offset1714.i" to <16 x i32>*
  %loadedValue1716.i = load <16 x i32>* %CastToValueType1715.i, align 64
  %.lhs1341.i = extractelement <16 x i32> %loadedValue1716.i, i32 0
  %366 = add i32 %.lhs1341.i, -128
  %extract610.i = sext i32 %366 to i64
  %367 = getelementptr inbounds [512 x float] addrspace(3)* %38, i64 0, i64 %extract610.i
  %ptrTypeCast626.i = bitcast float addrspace(3)* %367 to <16 x float> addrspace(3)*
  %368 = load <16 x float> addrspace(3)* %ptrTypeCast626.i, align 4
  %"&(pSB[currWI].offset)1909.i" = add nuw i64 %CurrSBIndex..19.i, 1600
  %"&pSB[currWI].offset1910.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1909.i"
  %CastToValueType1911.i = bitcast i8* %"&pSB[currWI].offset1910.i" to <16 x float>*
  store <16 x float> %368, <16 x float>* %CastToValueType1911.i, align 64
  %check.WI.iter2334.i = icmp ult i64 %CurrWI..19.i, %34
  br i1 %check.WI.iter2334.i, label %thenBB2331.i, label %SyncBB2195.i

thenBB2331.i:                                     ; preds = %SyncBB2194.i
  %"CurrWI++2335.i" = add nuw i64 %CurrWI..19.i, 1
  %"loadedCurrSB+Stride2337.i" = add nuw i64 %CurrSBIndex..19.i, 1792
  br label %SyncBB2194.i

SyncBB2195.i:                                     ; preds = %thenBB2338.i, %SyncBB2194.i
  %CurrSBIndex..20.i = phi i64 [ %"loadedCurrSB+Stride2344.i", %thenBB2338.i ], [ 0, %SyncBB2194.i ]
  %CurrWI..20.i = phi i64 [ %"CurrWI++2342.i", %thenBB2338.i ], [ 0, %SyncBB2194.i ]
  %"&(pSB[currWI].offset)1762.i" = add nuw i64 %CurrSBIndex..20.i, 1088
  %"&pSB[currWI].offset1763.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1762.i"
  %CastToValueType1764.i = bitcast i8* %"&pSB[currWI].offset1763.i" to float addrspace(3)**
  %loadedValue1765.i = load float addrspace(3)** %CastToValueType1764.i, align 8
  %ptrTypeCast627.i = bitcast float addrspace(3)* %loadedValue1765.i to <16 x float> addrspace(3)*
  %369 = load <16 x float> addrspace(3)* %ptrTypeCast627.i, align 4
  %"&(pSB[currWI].offset)1913.i" = add nuw i64 %CurrSBIndex..20.i, 1600
  %"&pSB[currWI].offset1914.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1913.i"
  %CastToValueType1915.i = bitcast i8* %"&pSB[currWI].offset1914.i" to <16 x float>*
  %loadedValue1916.i = load <16 x float>* %CastToValueType1915.i, align 64
  %370 = fadd <16 x float> %369, %loadedValue1916.i
  %"&(pSB[currWI].offset)1757.i" = add nuw i64 %CurrSBIndex..20.i, 1088
  %"&pSB[currWI].offset1758.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1757.i"
  %CastToValueType1759.i = bitcast i8* %"&pSB[currWI].offset1758.i" to float addrspace(3)**
  %loadedValue1760.i = load float addrspace(3)** %CastToValueType1759.i, align 8
  %ptrTypeCast628.i = bitcast float addrspace(3)* %loadedValue1760.i to <16 x float> addrspace(3)*
  store <16 x float> %370, <16 x float> addrspace(3)* %ptrTypeCast628.i, align 4
  %check.WI.iter2341.i = icmp ult i64 %CurrWI..20.i, %34
  br i1 %check.WI.iter2341.i, label %thenBB2338.i, label %SyncBB2196.i

thenBB2338.i:                                     ; preds = %SyncBB2195.i
  %"CurrWI++2342.i" = add nuw i64 %CurrWI..20.i, 1
  %"loadedCurrSB+Stride2344.i" = add nuw i64 %CurrSBIndex..20.i, 1792
  br label %SyncBB2195.i

SyncBB2196.i:                                     ; preds = %thenBB2205.i, %SyncBB2195.i
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride2211.i", %thenBB2205.i ], [ 0, %SyncBB2195.i ]
  %CurrWI..2.i = phi i64 [ %"CurrWI++2209.i", %thenBB2205.i ], [ 0, %SyncBB2195.i ]
  %"&(pSB[currWI].offset)1841.i" = add nuw i64 %CurrSBIndex..2.i, 1096
  %"&pSB[currWI].offset1842.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1841.i"
  %CastToValueType1843.i = bitcast i8* %"&pSB[currWI].offset1842.i" to float addrspace(3)**
  %loadedValue1844.i = load float addrspace(3)** %CastToValueType1843.i, align 8
  %ptrTypeCast629.i = bitcast float addrspace(3)* %loadedValue1844.i to <16 x float> addrspace(3)*
  %371 = load <16 x float> addrspace(3)* %ptrTypeCast629.i, align 4
  %"&(pSB[currWI].offset)1918.i" = add nuw i64 %CurrSBIndex..2.i, 1664
  %"&pSB[currWI].offset1919.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1918.i"
  %CastToValueType1920.i = bitcast i8* %"&pSB[currWI].offset1919.i" to <16 x float>*
  store <16 x float> %371, <16 x float>* %CastToValueType1920.i, align 64
  %extract668.i = extractelement <16 x float> %371, i32 0
  %"&(pSB[currWI].offset)1942.i" = add nuw i64 %CurrSBIndex..2.i, 1728
  %"&pSB[currWI].offset1943.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1942.i"
  %CastToValueType1944.i = bitcast i8* %"&pSB[currWI].offset1943.i" to float*
  store float %extract668.i, float* %CastToValueType1944.i, align 4
  %extract669.i = extractelement <16 x float> %371, i32 1
  %"&(pSB[currWI].offset)1956.i" = add nuw i64 %CurrSBIndex..2.i, 1732
  %"&pSB[currWI].offset1957.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1956.i"
  %CastToValueType1958.i = bitcast i8* %"&pSB[currWI].offset1957.i" to float*
  store float %extract669.i, float* %CastToValueType1958.i, align 4
  %extract670.i = extractelement <16 x float> %371, i32 2
  %"&(pSB[currWI].offset)1970.i" = add nuw i64 %CurrSBIndex..2.i, 1736
  %"&pSB[currWI].offset1971.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1970.i"
  %CastToValueType1972.i = bitcast i8* %"&pSB[currWI].offset1971.i" to float*
  store float %extract670.i, float* %CastToValueType1972.i, align 4
  %extract671.i = extractelement <16 x float> %371, i32 3
  %"&(pSB[currWI].offset)1984.i" = add nuw i64 %CurrSBIndex..2.i, 1740
  %"&pSB[currWI].offset1985.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1984.i"
  %CastToValueType1986.i = bitcast i8* %"&pSB[currWI].offset1985.i" to float*
  store float %extract671.i, float* %CastToValueType1986.i, align 4
  %extract672.i = extractelement <16 x float> %371, i32 4
  %"&(pSB[currWI].offset)1998.i" = add nuw i64 %CurrSBIndex..2.i, 1744
  %"&pSB[currWI].offset1999.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1998.i"
  %CastToValueType2000.i = bitcast i8* %"&pSB[currWI].offset1999.i" to float*
  store float %extract672.i, float* %CastToValueType2000.i, align 4
  %extract673.i = extractelement <16 x float> %371, i32 5
  %"&(pSB[currWI].offset)2012.i" = add nuw i64 %CurrSBIndex..2.i, 1748
  %"&pSB[currWI].offset2013.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2012.i"
  %CastToValueType2014.i = bitcast i8* %"&pSB[currWI].offset2013.i" to float*
  store float %extract673.i, float* %CastToValueType2014.i, align 4
  %extract674.i = extractelement <16 x float> %371, i32 6
  %"&(pSB[currWI].offset)2026.i" = add nuw i64 %CurrSBIndex..2.i, 1752
  %"&pSB[currWI].offset2027.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2026.i"
  %CastToValueType2028.i = bitcast i8* %"&pSB[currWI].offset2027.i" to float*
  store float %extract674.i, float* %CastToValueType2028.i, align 4
  %extract675.i = extractelement <16 x float> %371, i32 7
  %"&(pSB[currWI].offset)2040.i" = add nuw i64 %CurrSBIndex..2.i, 1756
  %"&pSB[currWI].offset2041.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2040.i"
  %CastToValueType2042.i = bitcast i8* %"&pSB[currWI].offset2041.i" to float*
  store float %extract675.i, float* %CastToValueType2042.i, align 4
  %extract676.i = extractelement <16 x float> %371, i32 8
  %"&(pSB[currWI].offset)2054.i" = add nuw i64 %CurrSBIndex..2.i, 1760
  %"&pSB[currWI].offset2055.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2054.i"
  %CastToValueType2056.i = bitcast i8* %"&pSB[currWI].offset2055.i" to float*
  store float %extract676.i, float* %CastToValueType2056.i, align 4
  %extract677.i = extractelement <16 x float> %371, i32 9
  %"&(pSB[currWI].offset)2068.i" = add nuw i64 %CurrSBIndex..2.i, 1764
  %"&pSB[currWI].offset2069.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2068.i"
  %CastToValueType2070.i = bitcast i8* %"&pSB[currWI].offset2069.i" to float*
  store float %extract677.i, float* %CastToValueType2070.i, align 4
  %extract678.i = extractelement <16 x float> %371, i32 10
  %"&(pSB[currWI].offset)2082.i" = add nuw i64 %CurrSBIndex..2.i, 1768
  %"&pSB[currWI].offset2083.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2082.i"
  %CastToValueType2084.i = bitcast i8* %"&pSB[currWI].offset2083.i" to float*
  store float %extract678.i, float* %CastToValueType2084.i, align 4
  %extract679.i = extractelement <16 x float> %371, i32 11
  %"&(pSB[currWI].offset)2096.i" = add nuw i64 %CurrSBIndex..2.i, 1772
  %"&pSB[currWI].offset2097.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2096.i"
  %CastToValueType2098.i = bitcast i8* %"&pSB[currWI].offset2097.i" to float*
  store float %extract679.i, float* %CastToValueType2098.i, align 4
  %extract680.i = extractelement <16 x float> %371, i32 12
  %"&(pSB[currWI].offset)2110.i" = add nuw i64 %CurrSBIndex..2.i, 1776
  %"&pSB[currWI].offset2111.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2110.i"
  %CastToValueType2112.i = bitcast i8* %"&pSB[currWI].offset2111.i" to float*
  store float %extract680.i, float* %CastToValueType2112.i, align 4
  %extract681.i = extractelement <16 x float> %371, i32 13
  %"&(pSB[currWI].offset)2124.i" = add nuw i64 %CurrSBIndex..2.i, 1780
  %"&pSB[currWI].offset2125.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2124.i"
  %CastToValueType2126.i = bitcast i8* %"&pSB[currWI].offset2125.i" to float*
  store float %extract681.i, float* %CastToValueType2126.i, align 4
  %extract682.i = extractelement <16 x float> %371, i32 14
  %"&(pSB[currWI].offset)2138.i" = add nuw i64 %CurrSBIndex..2.i, 1784
  %"&pSB[currWI].offset2139.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2138.i"
  %CastToValueType2140.i = bitcast i8* %"&pSB[currWI].offset2139.i" to float*
  store float %extract682.i, float* %CastToValueType2140.i, align 4
  %extract683.i = extractelement <16 x float> %371, i32 15
  %"&(pSB[currWI].offset)2152.i" = add nuw i64 %CurrSBIndex..2.i, 1788
  %"&pSB[currWI].offset2153.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2152.i"
  %CastToValueType2154.i = bitcast i8* %"&pSB[currWI].offset2153.i" to float*
  store float %extract683.i, float* %CastToValueType2154.i, align 4
  %check.WI.iter2208.i = icmp ult i64 %CurrWI..2.i, %34
  br i1 %check.WI.iter2208.i, label %thenBB2205.i, label %elseBB2206.i

thenBB2205.i:                                     ; preds = %SyncBB2196.i
  %"CurrWI++2209.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride2211.i" = add nuw i64 %CurrSBIndex..2.i, 1792
  br label %SyncBB2196.i

elseBB2206.i:                                     ; preds = %SyncBB2196.i
  %Mneg80.i = icmp ne i32 %16, 0
  %temp633.i = insertelement <16 x i1> undef, i1 %Mneg80.i, i32 0
  %vector634.i = shufflevector <16 x i1> %temp633.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %372 = bitcast float addrspace(1)* %1 to <4 x float> addrspace(1)*
  %Mneg90.i = xor i1 %40, true
  %temp738.i = insertelement <16 x i1> undef, i1 %Mneg90.i, i32 0
  %vector739.i = shufflevector <16 x i1> %temp738.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %temp733.i = insertelement <16 x i1> undef, i1 %40, i32 0
  %vector734.i = shufflevector <16 x i1> %temp733.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %SyncBB2177.i

SyncBB2177.i:                                     ; preds = %SyncBB2177.backedge.i, %elseBB2206.i
  %CurrSBIndex..0.i = phi i64 [ 0, %elseBB2206.i ], [ %CurrSBIndex..0.be.i, %SyncBB2177.backedge.i ]
  %CurrWI..0.i = phi i64 [ 0, %elseBB2206.i ], [ %CurrWI..0.be.i, %SyncBB2177.backedge.i ]
  %"&(pSB[currWI].offset)1451.i" = add nuw i64 %CurrSBIndex..0.i, 384
  %"&pSB[currWI].offset1452.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1451.i"
  %CastToValueType1453.i = bitcast i8* %"&pSB[currWI].offset1452.i" to <16 x i64>*
  %loadedValue1454.i = load <16 x i64>* %CastToValueType1453.i, align 128
  %sext630.i = shl <16 x i64> %loadedValue1454.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %373 = ashr <16 x i64> %sext630.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  br i1 %Mneg80.i, label %preload980.i, label %postload981.i

preload980.i:                                     ; preds = %SyncBB2177.i
  %374 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %375 = load i64* %374, align 8
  %phitmp.i = add i64 %375, -1
  br label %postload981.i

postload981.i:                                    ; preds = %preload980.i, %SyncBB2177.i
  %phi982.i = phi i64 [ undef, %SyncBB2177.i ], [ %phitmp.i, %preload980.i ]
  %temp631.i = insertelement <16 x i64> undef, i64 %phi982.i, i32 0
  %vector632.i = shufflevector <16 x i64> %temp631.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %376 = icmp eq <16 x i64> %373, %vector632.i
  %_to_88635.i = and <16 x i1> %vector634.i, %376
  %extract636.i = extractelement <16 x i1> %_to_88635.i, i32 0
  %extract637.i = extractelement <16 x i1> %_to_88635.i, i32 1
  %extract638.i = extractelement <16 x i1> %_to_88635.i, i32 2
  %extract639.i = extractelement <16 x i1> %_to_88635.i, i32 3
  %extract640.i = extractelement <16 x i1> %_to_88635.i, i32 4
  %extract641.i = extractelement <16 x i1> %_to_88635.i, i32 5
  %extract642.i = extractelement <16 x i1> %_to_88635.i, i32 6
  %extract643.i = extractelement <16 x i1> %_to_88635.i, i32 7
  %extract644.i = extractelement <16 x i1> %_to_88635.i, i32 8
  %extract645.i = extractelement <16 x i1> %_to_88635.i, i32 9
  %extract646.i = extractelement <16 x i1> %_to_88635.i, i32 10
  %extract647.i = extractelement <16 x i1> %_to_88635.i, i32 11
  %extract648.i = extractelement <16 x i1> %_to_88635.i, i32 12
  %extract649.i = extractelement <16 x i1> %_to_88635.i, i32 13
  %extract650.i = extractelement <16 x i1> %_to_88635.i, i32 14
  %extract651.i = extractelement <16 x i1> %_to_88635.i, i32 15
  %"&(pSB[currWI].offset)1668.i" = add nuw i64 %CurrSBIndex..0.i, 640
  %"&pSB[currWI].offset1669.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1668.i"
  %CastToValueType1670.i = bitcast i8* %"&pSB[currWI].offset1669.i" to <16 x float>*
  %loadedValue1671.i = load <16 x float>* %CastToValueType1670.i, align 64
  %"&(pSB[currWI].offset)1937.i" = add nuw i64 %CurrSBIndex..0.i, 1664
  %"&pSB[currWI].offset1938.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1937.i"
  %CastToValueType1939.i = bitcast i8* %"&pSB[currWI].offset1938.i" to <16 x float>*
  %loadedValue1940.i = load <16 x float>* %CastToValueType1939.i, align 64
  %377 = fadd <16 x float> %loadedValue1940.i, %loadedValue1671.i
  %extract652.i = extractelement <16 x float> %377, i32 0
  %extract653.i = extractelement <16 x float> %377, i32 1
  %extract654.i = extractelement <16 x float> %377, i32 2
  %extract655.i = extractelement <16 x float> %377, i32 3
  %extract656.i = extractelement <16 x float> %377, i32 4
  %extract657.i = extractelement <16 x float> %377, i32 5
  %extract658.i = extractelement <16 x float> %377, i32 6
  %extract659.i = extractelement <16 x float> %377, i32 7
  %extract660.i = extractelement <16 x float> %377, i32 8
  %extract661.i = extractelement <16 x float> %377, i32 9
  %extract662.i = extractelement <16 x float> %377, i32 10
  %extract663.i = extractelement <16 x float> %377, i32 11
  %extract664.i = extractelement <16 x float> %377, i32 12
  %extract665.i = extractelement <16 x float> %377, i32 13
  %extract666.i = extractelement <16 x float> %377, i32 14
  %extract667.i = extractelement <16 x float> %377, i32 15
  br i1 %extract636.i, label %preload1127.i, label %postload1128.i

preload1127.i:                                    ; preds = %postload981.i
  %378 = load i64* %25, align 8
  br label %postload1128.i

postload1128.i:                                   ; preds = %preload1127.i, %postload981.i
  %phi1129.i = phi i64 [ undef, %postload981.i ], [ %378, %preload1127.i ]
  br i1 %extract637.i, label %preload1132.i, label %postload1133.i

preload1132.i:                                    ; preds = %postload1128.i
  %379 = load i64* %25, align 8
  br label %postload1133.i

postload1133.i:                                   ; preds = %preload1132.i, %postload1128.i
  %phi1134.i = phi i64 [ undef, %postload1128.i ], [ %379, %preload1132.i ]
  br i1 %extract638.i, label %preload1137.i, label %postload1138.i

preload1137.i:                                    ; preds = %postload1133.i
  %380 = load i64* %25, align 8
  br label %postload1138.i

postload1138.i:                                   ; preds = %preload1137.i, %postload1133.i
  %phi1139.i = phi i64 [ undef, %postload1133.i ], [ %380, %preload1137.i ]
  br i1 %extract639.i, label %preload1142.i, label %postload1143.i

preload1142.i:                                    ; preds = %postload1138.i
  %381 = load i64* %25, align 8
  br label %postload1143.i

postload1143.i:                                   ; preds = %preload1142.i, %postload1138.i
  %phi1144.i = phi i64 [ undef, %postload1138.i ], [ %381, %preload1142.i ]
  br i1 %extract640.i, label %preload1147.i, label %postload1148.i

preload1147.i:                                    ; preds = %postload1143.i
  %382 = load i64* %25, align 8
  br label %postload1148.i

postload1148.i:                                   ; preds = %preload1147.i, %postload1143.i
  %phi1149.i = phi i64 [ undef, %postload1143.i ], [ %382, %preload1147.i ]
  br i1 %extract641.i, label %preload1152.i, label %postload1153.i

preload1152.i:                                    ; preds = %postload1148.i
  %383 = load i64* %25, align 8
  br label %postload1153.i

postload1153.i:                                   ; preds = %preload1152.i, %postload1148.i
  %phi1154.i = phi i64 [ undef, %postload1148.i ], [ %383, %preload1152.i ]
  br i1 %extract642.i, label %preload1157.i, label %postload1158.i

preload1157.i:                                    ; preds = %postload1153.i
  %384 = load i64* %25, align 8
  br label %postload1158.i

postload1158.i:                                   ; preds = %preload1157.i, %postload1153.i
  %phi1159.i = phi i64 [ undef, %postload1153.i ], [ %384, %preload1157.i ]
  br i1 %extract643.i, label %preload1162.i, label %postload1163.i

preload1162.i:                                    ; preds = %postload1158.i
  %385 = load i64* %25, align 8
  br label %postload1163.i

postload1163.i:                                   ; preds = %preload1162.i, %postload1158.i
  %phi1164.i = phi i64 [ undef, %postload1158.i ], [ %385, %preload1162.i ]
  br i1 %extract644.i, label %preload1167.i, label %postload1168.i

preload1167.i:                                    ; preds = %postload1163.i
  %386 = load i64* %25, align 8
  br label %postload1168.i

postload1168.i:                                   ; preds = %preload1167.i, %postload1163.i
  %phi1169.i = phi i64 [ undef, %postload1163.i ], [ %386, %preload1167.i ]
  br i1 %extract645.i, label %preload1172.i, label %postload1173.i

preload1172.i:                                    ; preds = %postload1168.i
  %387 = load i64* %25, align 8
  br label %postload1173.i

postload1173.i:                                   ; preds = %preload1172.i, %postload1168.i
  %phi1174.i = phi i64 [ undef, %postload1168.i ], [ %387, %preload1172.i ]
  br i1 %extract646.i, label %preload1177.i, label %postload1178.i

preload1177.i:                                    ; preds = %postload1173.i
  %388 = load i64* %25, align 8
  br label %postload1178.i

postload1178.i:                                   ; preds = %preload1177.i, %postload1173.i
  %phi1179.i = phi i64 [ undef, %postload1173.i ], [ %388, %preload1177.i ]
  br i1 %extract647.i, label %preload1182.i, label %postload1183.i

preload1182.i:                                    ; preds = %postload1178.i
  %389 = load i64* %25, align 8
  br label %postload1183.i

postload1183.i:                                   ; preds = %preload1182.i, %postload1178.i
  %phi1184.i = phi i64 [ undef, %postload1178.i ], [ %389, %preload1182.i ]
  br i1 %extract648.i, label %preload1187.i, label %postload1188.i

preload1187.i:                                    ; preds = %postload1183.i
  %390 = load i64* %25, align 8
  br label %postload1188.i

postload1188.i:                                   ; preds = %preload1187.i, %postload1183.i
  %phi1189.i = phi i64 [ undef, %postload1183.i ], [ %390, %preload1187.i ]
  br i1 %extract649.i, label %preload1192.i, label %postload1193.i

preload1192.i:                                    ; preds = %postload1188.i
  %391 = load i64* %25, align 8
  br label %postload1193.i

postload1193.i:                                   ; preds = %preload1192.i, %postload1188.i
  %phi1194.i = phi i64 [ undef, %postload1188.i ], [ %391, %preload1192.i ]
  br i1 %extract650.i, label %preload1197.i, label %postload1198.i

preload1197.i:                                    ; preds = %postload1193.i
  %392 = load i64* %25, align 8
  br label %postload1198.i

postload1198.i:                                   ; preds = %preload1197.i, %postload1193.i
  %phi1199.i = phi i64 [ undef, %postload1193.i ], [ %392, %preload1197.i ]
  br i1 %extract651.i, label %preload1202.i, label %postload1203.i

preload1202.i:                                    ; preds = %postload1198.i
  %393 = load i64* %25, align 8
  br label %postload1203.i

postload1203.i:                                   ; preds = %preload1202.i, %postload1198.i
  %phi1204.i = phi i64 [ undef, %postload1198.i ], [ %393, %preload1202.i ]
  %394 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1134.i
  %395 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1139.i
  %396 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1144.i
  %397 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1149.i
  %398 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1154.i
  %399 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1159.i
  %400 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1164.i
  %401 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1169.i
  %402 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1174.i
  %403 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1179.i
  %404 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1184.i
  %405 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1189.i
  %406 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1194.i
  %407 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1199.i
  %408 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1204.i
  br i1 %extract636.i, label %preload1130.i, label %postload1131.i

preload1130.i:                                    ; preds = %postload1203.i
  %409 = getelementptr inbounds float addrspace(1)* %7, i64 %phi1129.i
  store float %extract652.i, float addrspace(1)* %409, align 4
  br label %postload1131.i

postload1131.i:                                   ; preds = %preload1130.i, %postload1203.i
  br i1 %extract637.i, label %preload1135.i, label %postload1136.i

preload1135.i:                                    ; preds = %postload1131.i
  store float %extract653.i, float addrspace(1)* %394, align 4
  br label %postload1136.i

postload1136.i:                                   ; preds = %preload1135.i, %postload1131.i
  br i1 %extract638.i, label %preload1140.i, label %postload1141.i

preload1140.i:                                    ; preds = %postload1136.i
  store float %extract654.i, float addrspace(1)* %395, align 4
  br label %postload1141.i

postload1141.i:                                   ; preds = %preload1140.i, %postload1136.i
  br i1 %extract639.i, label %preload1145.i, label %postload1146.i

preload1145.i:                                    ; preds = %postload1141.i
  store float %extract655.i, float addrspace(1)* %396, align 4
  br label %postload1146.i

postload1146.i:                                   ; preds = %preload1145.i, %postload1141.i
  br i1 %extract640.i, label %preload1150.i, label %postload1151.i

preload1150.i:                                    ; preds = %postload1146.i
  store float %extract656.i, float addrspace(1)* %397, align 4
  br label %postload1151.i

postload1151.i:                                   ; preds = %preload1150.i, %postload1146.i
  br i1 %extract641.i, label %preload1155.i, label %postload1156.i

preload1155.i:                                    ; preds = %postload1151.i
  store float %extract657.i, float addrspace(1)* %398, align 4
  br label %postload1156.i

postload1156.i:                                   ; preds = %preload1155.i, %postload1151.i
  br i1 %extract642.i, label %preload1160.i, label %postload1161.i

preload1160.i:                                    ; preds = %postload1156.i
  store float %extract658.i, float addrspace(1)* %399, align 4
  br label %postload1161.i

postload1161.i:                                   ; preds = %preload1160.i, %postload1156.i
  br i1 %extract643.i, label %preload1165.i, label %postload1166.i

preload1165.i:                                    ; preds = %postload1161.i
  store float %extract659.i, float addrspace(1)* %400, align 4
  br label %postload1166.i

postload1166.i:                                   ; preds = %preload1165.i, %postload1161.i
  br i1 %extract644.i, label %preload1170.i, label %postload1171.i

preload1170.i:                                    ; preds = %postload1166.i
  store float %extract660.i, float addrspace(1)* %401, align 4
  br label %postload1171.i

postload1171.i:                                   ; preds = %preload1170.i, %postload1166.i
  br i1 %extract645.i, label %preload1175.i, label %postload1176.i

preload1175.i:                                    ; preds = %postload1171.i
  store float %extract661.i, float addrspace(1)* %402, align 4
  br label %postload1176.i

postload1176.i:                                   ; preds = %preload1175.i, %postload1171.i
  br i1 %extract646.i, label %preload1180.i, label %postload1181.i

preload1180.i:                                    ; preds = %postload1176.i
  store float %extract662.i, float addrspace(1)* %403, align 4
  br label %postload1181.i

postload1181.i:                                   ; preds = %preload1180.i, %postload1176.i
  br i1 %extract647.i, label %preload1185.i, label %postload1186.i

preload1185.i:                                    ; preds = %postload1181.i
  store float %extract663.i, float addrspace(1)* %404, align 4
  br label %postload1186.i

postload1186.i:                                   ; preds = %preload1185.i, %postload1181.i
  br i1 %extract648.i, label %preload1190.i, label %postload1191.i

preload1190.i:                                    ; preds = %postload1186.i
  store float %extract664.i, float addrspace(1)* %405, align 4
  br label %postload1191.i

postload1191.i:                                   ; preds = %preload1190.i, %postload1186.i
  br i1 %extract649.i, label %preload1195.i, label %postload1196.i

preload1195.i:                                    ; preds = %postload1191.i
  store float %extract665.i, float addrspace(1)* %406, align 4
  br label %postload1196.i

postload1196.i:                                   ; preds = %preload1195.i, %postload1191.i
  br i1 %extract650.i, label %preload1200.i, label %postload1201.i

preload1200.i:                                    ; preds = %postload1196.i
  store float %extract666.i, float addrspace(1)* %407, align 4
  br label %postload1201.i

postload1201.i:                                   ; preds = %preload1200.i, %postload1196.i
  br i1 %extract651.i, label %preload1205.i, label %phi-split-bb.i

preload1205.i:                                    ; preds = %postload1201.i
  store float %extract667.i, float addrspace(1)* %408, align 4
  br label %phi-split-bb.i

phi-split-bb.i:                                   ; preds = %preload1205.i, %postload1201.i
  %"&(pSB[currWI].offset)1695.i" = add nuw i64 %CurrSBIndex..0.i, 832
  %"&pSB[currWI].offset1696.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1695.i"
  %CastToValueType1697.i = bitcast i8* %"&pSB[currWI].offset1696.i" to <16 x float>*
  %loadedValue1698.i = load <16 x float>* %CastToValueType1697.i, align 64
  %"&(pSB[currWI].offset)1932.i" = add nuw i64 %CurrSBIndex..0.i, 1664
  %"&pSB[currWI].offset1933.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1932.i"
  %CastToValueType1934.i = bitcast i8* %"&pSB[currWI].offset1933.i" to <16 x float>*
  %loadedValue1935.i = load <16 x float>* %CastToValueType1934.i, align 64
  %410 = fadd <16 x float> %loadedValue1935.i, %loadedValue1698.i
  %extract684.i = extractelement <16 x float> %410, i32 0
  %extract685.i = extractelement <16 x float> %410, i32 1
  %extract686.i = extractelement <16 x float> %410, i32 2
  %extract687.i = extractelement <16 x float> %410, i32 3
  %extract688.i = extractelement <16 x float> %410, i32 4
  %extract689.i = extractelement <16 x float> %410, i32 5
  %extract690.i = extractelement <16 x float> %410, i32 6
  %extract691.i = extractelement <16 x float> %410, i32 7
  %extract692.i = extractelement <16 x float> %410, i32 8
  %extract693.i = extractelement <16 x float> %410, i32 9
  %extract694.i = extractelement <16 x float> %410, i32 10
  %extract695.i = extractelement <16 x float> %410, i32 11
  %extract696.i = extractelement <16 x float> %410, i32 12
  %extract697.i = extractelement <16 x float> %410, i32 13
  %extract698.i = extractelement <16 x float> %410, i32 14
  %extract699.i = extractelement <16 x float> %410, i32 15
  %"&(pSB[currWI].offset)1686.i" = add nuw i64 %CurrSBIndex..0.i, 768
  %"&pSB[currWI].offset1687.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1686.i"
  %CastToValueType1688.i = bitcast i8* %"&pSB[currWI].offset1687.i" to <16 x float>*
  %loadedValue1689.i = load <16 x float>* %CastToValueType1688.i, align 64
  %"&(pSB[currWI].offset)1927.i" = add nuw i64 %CurrSBIndex..0.i, 1664
  %"&pSB[currWI].offset1928.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1927.i"
  %CastToValueType1929.i = bitcast i8* %"&pSB[currWI].offset1928.i" to <16 x float>*
  %loadedValue1930.i = load <16 x float>* %CastToValueType1929.i, align 64
  %411 = fadd <16 x float> %loadedValue1930.i, %loadedValue1689.i
  %extract700.i = extractelement <16 x float> %411, i32 0
  %extract701.i = extractelement <16 x float> %411, i32 1
  %extract702.i = extractelement <16 x float> %411, i32 2
  %extract703.i = extractelement <16 x float> %411, i32 3
  %extract704.i = extractelement <16 x float> %411, i32 4
  %extract705.i = extractelement <16 x float> %411, i32 5
  %extract706.i = extractelement <16 x float> %411, i32 6
  %extract707.i = extractelement <16 x float> %411, i32 7
  %extract708.i = extractelement <16 x float> %411, i32 8
  %extract709.i = extractelement <16 x float> %411, i32 9
  %extract710.i = extractelement <16 x float> %411, i32 10
  %extract711.i = extractelement <16 x float> %411, i32 11
  %extract712.i = extractelement <16 x float> %411, i32 12
  %extract713.i = extractelement <16 x float> %411, i32 13
  %extract714.i = extractelement <16 x float> %411, i32 14
  %extract715.i = extractelement <16 x float> %411, i32 15
  %"&(pSB[currWI].offset)1677.i" = add nuw i64 %CurrSBIndex..0.i, 704
  %"&pSB[currWI].offset1678.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1677.i"
  %CastToValueType1679.i = bitcast i8* %"&pSB[currWI].offset1678.i" to <16 x float>*
  %loadedValue1680.i = load <16 x float>* %CastToValueType1679.i, align 64
  %"&(pSB[currWI].offset)1922.i" = add nuw i64 %CurrSBIndex..0.i, 1664
  %"&pSB[currWI].offset1923.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1922.i"
  %CastToValueType1924.i = bitcast i8* %"&pSB[currWI].offset1923.i" to <16 x float>*
  %loadedValue1925.i = load <16 x float>* %CastToValueType1924.i, align 64
  %412 = fadd <16 x float> %loadedValue1925.i, %loadedValue1680.i
  %extract717.i = extractelement <16 x float> %412, i32 1
  %extract718.i = extractelement <16 x float> %412, i32 2
  %extract719.i = extractelement <16 x float> %412, i32 3
  %extract720.i = extractelement <16 x float> %412, i32 4
  %extract721.i = extractelement <16 x float> %412, i32 5
  %extract722.i = extractelement <16 x float> %412, i32 6
  %extract723.i = extractelement <16 x float> %412, i32 7
  %extract724.i = extractelement <16 x float> %412, i32 8
  %extract725.i = extractelement <16 x float> %412, i32 9
  %extract726.i = extractelement <16 x float> %412, i32 10
  %extract727.i = extractelement <16 x float> %412, i32 11
  %extract728.i = extractelement <16 x float> %412, i32 12
  %extract729.i = extractelement <16 x float> %412, i32 13
  %extract730.i = extractelement <16 x float> %412, i32 14
  %extract731.i = extractelement <16 x float> %412, i32 15
  %"&(pSB[currWI].offset)1965.i" = add nuw i64 %CurrSBIndex..0.i, 1732
  %"&pSB[currWI].offset1966.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1965.i"
  %CastToValueType1967.i = bitcast i8* %"&pSB[currWI].offset1966.i" to float*
  %loadedValue1968.i = load float* %CastToValueType1967.i, align 4
  %413 = insertelement <4 x float> undef, float %loadedValue1968.i, i32 0
  %"&(pSB[currWI].offset)1979.i" = add nuw i64 %CurrSBIndex..0.i, 1736
  %"&pSB[currWI].offset1980.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1979.i"
  %CastToValueType1981.i = bitcast i8* %"&pSB[currWI].offset1980.i" to float*
  %loadedValue1982.i = load float* %CastToValueType1981.i, align 4
  %414 = insertelement <4 x float> undef, float %loadedValue1982.i, i32 0
  %"&(pSB[currWI].offset)1993.i" = add nuw i64 %CurrSBIndex..0.i, 1740
  %"&pSB[currWI].offset1994.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1993.i"
  %CastToValueType1995.i = bitcast i8* %"&pSB[currWI].offset1994.i" to float*
  %loadedValue1996.i = load float* %CastToValueType1995.i, align 4
  %415 = insertelement <4 x float> undef, float %loadedValue1996.i, i32 0
  %"&(pSB[currWI].offset)2007.i" = add nuw i64 %CurrSBIndex..0.i, 1744
  %"&pSB[currWI].offset2008.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2007.i"
  %CastToValueType2009.i = bitcast i8* %"&pSB[currWI].offset2008.i" to float*
  %loadedValue2010.i = load float* %CastToValueType2009.i, align 4
  %416 = insertelement <4 x float> undef, float %loadedValue2010.i, i32 0
  %"&(pSB[currWI].offset)2021.i" = add nuw i64 %CurrSBIndex..0.i, 1748
  %"&pSB[currWI].offset2022.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2021.i"
  %CastToValueType2023.i = bitcast i8* %"&pSB[currWI].offset2022.i" to float*
  %loadedValue2024.i = load float* %CastToValueType2023.i, align 4
  %417 = insertelement <4 x float> undef, float %loadedValue2024.i, i32 0
  %"&(pSB[currWI].offset)2035.i" = add nuw i64 %CurrSBIndex..0.i, 1752
  %"&pSB[currWI].offset2036.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2035.i"
  %CastToValueType2037.i = bitcast i8* %"&pSB[currWI].offset2036.i" to float*
  %loadedValue2038.i = load float* %CastToValueType2037.i, align 4
  %418 = insertelement <4 x float> undef, float %loadedValue2038.i, i32 0
  %"&(pSB[currWI].offset)2049.i" = add nuw i64 %CurrSBIndex..0.i, 1756
  %"&pSB[currWI].offset2050.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2049.i"
  %CastToValueType2051.i = bitcast i8* %"&pSB[currWI].offset2050.i" to float*
  %loadedValue2052.i = load float* %CastToValueType2051.i, align 4
  %419 = insertelement <4 x float> undef, float %loadedValue2052.i, i32 0
  %"&(pSB[currWI].offset)2063.i" = add nuw i64 %CurrSBIndex..0.i, 1760
  %"&pSB[currWI].offset2064.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2063.i"
  %CastToValueType2065.i = bitcast i8* %"&pSB[currWI].offset2064.i" to float*
  %loadedValue2066.i = load float* %CastToValueType2065.i, align 4
  %420 = insertelement <4 x float> undef, float %loadedValue2066.i, i32 0
  %"&(pSB[currWI].offset)2077.i" = add nuw i64 %CurrSBIndex..0.i, 1764
  %"&pSB[currWI].offset2078.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2077.i"
  %CastToValueType2079.i = bitcast i8* %"&pSB[currWI].offset2078.i" to float*
  %loadedValue2080.i = load float* %CastToValueType2079.i, align 4
  %421 = insertelement <4 x float> undef, float %loadedValue2080.i, i32 0
  %"&(pSB[currWI].offset)2091.i" = add nuw i64 %CurrSBIndex..0.i, 1768
  %"&pSB[currWI].offset2092.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2091.i"
  %CastToValueType2093.i = bitcast i8* %"&pSB[currWI].offset2092.i" to float*
  %loadedValue2094.i = load float* %CastToValueType2093.i, align 4
  %422 = insertelement <4 x float> undef, float %loadedValue2094.i, i32 0
  %"&(pSB[currWI].offset)2105.i" = add nuw i64 %CurrSBIndex..0.i, 1772
  %"&pSB[currWI].offset2106.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2105.i"
  %CastToValueType2107.i = bitcast i8* %"&pSB[currWI].offset2106.i" to float*
  %loadedValue2108.i = load float* %CastToValueType2107.i, align 4
  %423 = insertelement <4 x float> undef, float %loadedValue2108.i, i32 0
  %"&(pSB[currWI].offset)2119.i" = add nuw i64 %CurrSBIndex..0.i, 1776
  %"&pSB[currWI].offset2120.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2119.i"
  %CastToValueType2121.i = bitcast i8* %"&pSB[currWI].offset2120.i" to float*
  %loadedValue2122.i = load float* %CastToValueType2121.i, align 4
  %424 = insertelement <4 x float> undef, float %loadedValue2122.i, i32 0
  %"&(pSB[currWI].offset)2133.i" = add nuw i64 %CurrSBIndex..0.i, 1780
  %"&pSB[currWI].offset2134.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2133.i"
  %CastToValueType2135.i = bitcast i8* %"&pSB[currWI].offset2134.i" to float*
  %loadedValue2136.i = load float* %CastToValueType2135.i, align 4
  %425 = insertelement <4 x float> undef, float %loadedValue2136.i, i32 0
  %"&(pSB[currWI].offset)2147.i" = add nuw i64 %CurrSBIndex..0.i, 1784
  %"&pSB[currWI].offset2148.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2147.i"
  %CastToValueType2149.i = bitcast i8* %"&pSB[currWI].offset2148.i" to float*
  %loadedValue2150.i = load float* %CastToValueType2149.i, align 4
  %426 = insertelement <4 x float> undef, float %loadedValue2150.i, i32 0
  %"&(pSB[currWI].offset)2161.i" = add nuw i64 %CurrSBIndex..0.i, 1788
  %"&pSB[currWI].offset2162.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2161.i"
  %CastToValueType2163.i = bitcast i8* %"&pSB[currWI].offset2162.i" to float*
  %loadedValue2164.i = load float* %CastToValueType2163.i, align 4
  %427 = insertelement <4 x float> undef, float %loadedValue2164.i, i32 0
  %428 = insertelement <4 x float> %413, float %extract685.i, i32 1
  %429 = insertelement <4 x float> %414, float %extract686.i, i32 1
  %430 = insertelement <4 x float> %415, float %extract687.i, i32 1
  %431 = insertelement <4 x float> %416, float %extract688.i, i32 1
  %432 = insertelement <4 x float> %417, float %extract689.i, i32 1
  %433 = insertelement <4 x float> %418, float %extract690.i, i32 1
  %434 = insertelement <4 x float> %419, float %extract691.i, i32 1
  %435 = insertelement <4 x float> %420, float %extract692.i, i32 1
  %436 = insertelement <4 x float> %421, float %extract693.i, i32 1
  %437 = insertelement <4 x float> %422, float %extract694.i, i32 1
  %438 = insertelement <4 x float> %423, float %extract695.i, i32 1
  %439 = insertelement <4 x float> %424, float %extract696.i, i32 1
  %440 = insertelement <4 x float> %425, float %extract697.i, i32 1
  %441 = insertelement <4 x float> %426, float %extract698.i, i32 1
  %442 = insertelement <4 x float> %427, float %extract699.i, i32 1
  %443 = insertelement <4 x float> %428, float %extract701.i, i32 2
  %444 = insertelement <4 x float> %429, float %extract702.i, i32 2
  %445 = insertelement <4 x float> %430, float %extract703.i, i32 2
  %446 = insertelement <4 x float> %431, float %extract704.i, i32 2
  %447 = insertelement <4 x float> %432, float %extract705.i, i32 2
  %448 = insertelement <4 x float> %433, float %extract706.i, i32 2
  %449 = insertelement <4 x float> %434, float %extract707.i, i32 2
  %450 = insertelement <4 x float> %435, float %extract708.i, i32 2
  %451 = insertelement <4 x float> %436, float %extract709.i, i32 2
  %452 = insertelement <4 x float> %437, float %extract710.i, i32 2
  %453 = insertelement <4 x float> %438, float %extract711.i, i32 2
  %454 = insertelement <4 x float> %439, float %extract712.i, i32 2
  %455 = insertelement <4 x float> %440, float %extract713.i, i32 2
  %456 = insertelement <4 x float> %441, float %extract714.i, i32 2
  %457 = insertelement <4 x float> %442, float %extract715.i, i32 2
  %458 = insertelement <4 x float> %443, float %extract717.i, i32 3
  %459 = insertelement <4 x float> %444, float %extract718.i, i32 3
  %460 = insertelement <4 x float> %445, float %extract719.i, i32 3
  %461 = insertelement <4 x float> %446, float %extract720.i, i32 3
  %462 = insertelement <4 x float> %447, float %extract721.i, i32 3
  %463 = insertelement <4 x float> %448, float %extract722.i, i32 3
  %464 = insertelement <4 x float> %449, float %extract723.i, i32 3
  %465 = insertelement <4 x float> %450, float %extract724.i, i32 3
  %466 = insertelement <4 x float> %451, float %extract725.i, i32 3
  %467 = insertelement <4 x float> %452, float %extract726.i, i32 3
  %468 = insertelement <4 x float> %453, float %extract727.i, i32 3
  %469 = insertelement <4 x float> %454, float %extract728.i, i32 3
  %470 = insertelement <4 x float> %455, float %extract729.i, i32 3
  %471 = insertelement <4 x float> %456, float %extract730.i, i32 3
  %472 = insertelement <4 x float> %457, float %extract731.i, i32 3
  %"&(pSB[currWI].offset)1635.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1636.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1635.i"
  %CastToValueType1637.i = bitcast i8* %"&pSB[currWI].offset1636.i" to <16 x i32>*
  %loadedValue1638.i = load <16 x i32>* %CastToValueType1637.i, align 64
  %473 = or <16 x i32> %loadedValue1638.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %474 = icmp slt <16 x i32> %473, %vector.i
  %Mneg96732.i = xor <16 x i1> %474, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %_to_99735.i = and <16 x i1> %vector734.i, %Mneg96732.i
  %_to_100736.i = and <16 x i1> %vector734.i, %474
  %_Min135740.i = or <16 x i1> %_to_100736.i, %vector739.i
  %extract757.i = extractelement <16 x i1> %_Min135740.i, i32 0
  %extract758.i = extractelement <16 x i1> %_Min135740.i, i32 1
  %extract759.i = extractelement <16 x i1> %_Min135740.i, i32 2
  %extract760.i = extractelement <16 x i1> %_Min135740.i, i32 3
  %extract761.i = extractelement <16 x i1> %_Min135740.i, i32 4
  %extract762.i = extractelement <16 x i1> %_Min135740.i, i32 5
  %extract763.i = extractelement <16 x i1> %_Min135740.i, i32 6
  %extract764.i = extractelement <16 x i1> %_Min135740.i, i32 7
  %extract765.i = extractelement <16 x i1> %_Min135740.i, i32 8
  %extract766.i = extractelement <16 x i1> %_Min135740.i, i32 9
  %extract767.i = extractelement <16 x i1> %_Min135740.i, i32 10
  %extract768.i = extractelement <16 x i1> %_Min135740.i, i32 11
  %extract769.i = extractelement <16 x i1> %_Min135740.i, i32 12
  %extract770.i = extractelement <16 x i1> %_Min135740.i, i32 13
  %extract771.i = extractelement <16 x i1> %_Min135740.i, i32 14
  %extract772.i = extractelement <16 x i1> %_Min135740.i, i32 15
  %"&(pSB[currWI].offset)1432.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1433.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1432.i"
  %CastToValueType1434.i = bitcast i8* %"&pSB[currWI].offset1433.i" to <16 x i32>*
  %loadedValue1435.i = load <16 x i32>* %CastToValueType1434.i, align 64
  %475 = extractelement <16 x i32> %loadedValue1435.i, i32 1
  %476 = sext i32 %475 to i64
  %477 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %476
  %"&(pSB[currWI].offset)1427.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1428.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1427.i"
  %CastToValueType1429.i = bitcast i8* %"&pSB[currWI].offset1428.i" to <16 x i32>*
  %loadedValue1430.i = load <16 x i32>* %CastToValueType1429.i, align 64
  %478 = extractelement <16 x i32> %loadedValue1430.i, i32 2
  %479 = sext i32 %478 to i64
  %480 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %479
  %"&(pSB[currWI].offset)1422.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1423.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1422.i"
  %CastToValueType1424.i = bitcast i8* %"&pSB[currWI].offset1423.i" to <16 x i32>*
  %loadedValue1425.i = load <16 x i32>* %CastToValueType1424.i, align 64
  %481 = extractelement <16 x i32> %loadedValue1425.i, i32 3
  %482 = sext i32 %481 to i64
  %483 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %482
  %"&(pSB[currWI].offset)1417.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1418.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1417.i"
  %CastToValueType1419.i = bitcast i8* %"&pSB[currWI].offset1418.i" to <16 x i32>*
  %loadedValue1420.i = load <16 x i32>* %CastToValueType1419.i, align 64
  %484 = extractelement <16 x i32> %loadedValue1420.i, i32 4
  %485 = sext i32 %484 to i64
  %486 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %485
  %"&(pSB[currWI].offset)1412.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1413.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1412.i"
  %CastToValueType1414.i = bitcast i8* %"&pSB[currWI].offset1413.i" to <16 x i32>*
  %loadedValue1415.i = load <16 x i32>* %CastToValueType1414.i, align 64
  %487 = extractelement <16 x i32> %loadedValue1415.i, i32 5
  %488 = sext i32 %487 to i64
  %489 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %488
  %"&(pSB[currWI].offset)1407.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1408.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1407.i"
  %CastToValueType1409.i = bitcast i8* %"&pSB[currWI].offset1408.i" to <16 x i32>*
  %loadedValue1410.i = load <16 x i32>* %CastToValueType1409.i, align 64
  %490 = extractelement <16 x i32> %loadedValue1410.i, i32 6
  %491 = sext i32 %490 to i64
  %492 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %491
  %"&(pSB[currWI].offset)1402.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1403.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1402.i"
  %CastToValueType1404.i = bitcast i8* %"&pSB[currWI].offset1403.i" to <16 x i32>*
  %loadedValue1405.i = load <16 x i32>* %CastToValueType1404.i, align 64
  %493 = extractelement <16 x i32> %loadedValue1405.i, i32 7
  %494 = sext i32 %493 to i64
  %495 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %494
  %"&(pSB[currWI].offset)1397.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1398.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1397.i"
  %CastToValueType1399.i = bitcast i8* %"&pSB[currWI].offset1398.i" to <16 x i32>*
  %loadedValue1400.i = load <16 x i32>* %CastToValueType1399.i, align 64
  %496 = extractelement <16 x i32> %loadedValue1400.i, i32 8
  %497 = sext i32 %496 to i64
  %498 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %497
  %"&(pSB[currWI].offset)1392.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1393.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1392.i"
  %CastToValueType1394.i = bitcast i8* %"&pSB[currWI].offset1393.i" to <16 x i32>*
  %loadedValue1395.i = load <16 x i32>* %CastToValueType1394.i, align 64
  %499 = extractelement <16 x i32> %loadedValue1395.i, i32 9
  %500 = sext i32 %499 to i64
  %501 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %500
  %"&(pSB[currWI].offset)1387.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1388.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1387.i"
  %CastToValueType1389.i = bitcast i8* %"&pSB[currWI].offset1388.i" to <16 x i32>*
  %loadedValue1390.i = load <16 x i32>* %CastToValueType1389.i, align 64
  %502 = extractelement <16 x i32> %loadedValue1390.i, i32 10
  %503 = sext i32 %502 to i64
  %504 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %503
  %"&(pSB[currWI].offset)1382.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1383.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1382.i"
  %CastToValueType1384.i = bitcast i8* %"&pSB[currWI].offset1383.i" to <16 x i32>*
  %loadedValue1385.i = load <16 x i32>* %CastToValueType1384.i, align 64
  %505 = extractelement <16 x i32> %loadedValue1385.i, i32 11
  %506 = sext i32 %505 to i64
  %507 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %506
  %"&(pSB[currWI].offset)1377.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1378.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1377.i"
  %CastToValueType1379.i = bitcast i8* %"&pSB[currWI].offset1378.i" to <16 x i32>*
  %loadedValue1380.i = load <16 x i32>* %CastToValueType1379.i, align 64
  %508 = extractelement <16 x i32> %loadedValue1380.i, i32 12
  %509 = sext i32 %508 to i64
  %510 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %509
  %"&(pSB[currWI].offset)1372.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1373.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1372.i"
  %CastToValueType1374.i = bitcast i8* %"&pSB[currWI].offset1373.i" to <16 x i32>*
  %loadedValue1375.i = load <16 x i32>* %CastToValueType1374.i, align 64
  %511 = extractelement <16 x i32> %loadedValue1375.i, i32 13
  %512 = sext i32 %511 to i64
  %513 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %512
  %"&(pSB[currWI].offset)1367.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1368.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1367.i"
  %CastToValueType1369.i = bitcast i8* %"&pSB[currWI].offset1368.i" to <16 x i32>*
  %loadedValue1370.i = load <16 x i32>* %CastToValueType1369.i, align 64
  %514 = extractelement <16 x i32> %loadedValue1370.i, i32 14
  %515 = sext i32 %514 to i64
  %516 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %515
  %"&(pSB[currWI].offset)1363.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1364.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1363.i"
  %CastToValueType1365.i = bitcast i8* %"&pSB[currWI].offset1364.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType1365.i, align 64
  %517 = extractelement <16 x i32> %loadedValue.i, i32 15
  %518 = sext i32 %517 to i64
  %519 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %518
  br i1 %extract757.i, label %preload1207.i, label %postload1208.i

preload1207.i:                                    ; preds = %phi-split-bb.i
  %"&(pSB[currWI].offset)1951.i" = add nuw i64 %CurrSBIndex..0.i, 1728
  %"&pSB[currWI].offset1952.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1951.i"
  %CastToValueType1953.i = bitcast i8* %"&pSB[currWI].offset1952.i" to float*
  %loadedValue1954.i = load float* %CastToValueType1953.i, align 4
  %520 = insertelement <4 x float> undef, float %loadedValue1954.i, i32 0
  %521 = insertelement <4 x float> %520, float %extract684.i, i32 1
  %522 = insertelement <4 x float> %521, float %extract700.i, i32 2
  %extract716.i = extractelement <16 x float> %412, i32 0
  %523 = insertelement <4 x float> %522, float %extract716.i, i32 3
  %"&(pSB[currWI].offset)1437.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset1438.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1437.i"
  %CastToValueType1439.i = bitcast i8* %"&pSB[currWI].offset1438.i" to <16 x i32>*
  %loadedValue1440.i = load <16 x i32>* %CastToValueType1439.i, align 64
  %524 = extractelement <16 x i32> %loadedValue1440.i, i32 0
  %525 = sext i32 %524 to i64
  %526 = getelementptr inbounds <4 x float> addrspace(1)* %372, i64 %525
  store <4 x float> %523, <4 x float> addrspace(1)* %526, align 16
  br label %postload1208.i

postload1208.i:                                   ; preds = %preload1207.i, %phi-split-bb.i
  br i1 %extract758.i, label %preload1209.i, label %postload1210.i

preload1209.i:                                    ; preds = %postload1208.i
  store <4 x float> %458, <4 x float> addrspace(1)* %477, align 16
  br label %postload1210.i

postload1210.i:                                   ; preds = %preload1209.i, %postload1208.i
  br i1 %extract759.i, label %preload1211.i, label %postload1212.i

preload1211.i:                                    ; preds = %postload1210.i
  store <4 x float> %459, <4 x float> addrspace(1)* %480, align 16
  br label %postload1212.i

postload1212.i:                                   ; preds = %preload1211.i, %postload1210.i
  br i1 %extract760.i, label %preload1213.i, label %postload1214.i

preload1213.i:                                    ; preds = %postload1212.i
  store <4 x float> %460, <4 x float> addrspace(1)* %483, align 16
  br label %postload1214.i

postload1214.i:                                   ; preds = %preload1213.i, %postload1212.i
  br i1 %extract761.i, label %preload1215.i, label %postload1216.i

preload1215.i:                                    ; preds = %postload1214.i
  store <4 x float> %461, <4 x float> addrspace(1)* %486, align 16
  br label %postload1216.i

postload1216.i:                                   ; preds = %preload1215.i, %postload1214.i
  br i1 %extract762.i, label %preload1217.i, label %postload1218.i

preload1217.i:                                    ; preds = %postload1216.i
  store <4 x float> %462, <4 x float> addrspace(1)* %489, align 16
  br label %postload1218.i

postload1218.i:                                   ; preds = %preload1217.i, %postload1216.i
  br i1 %extract763.i, label %preload1219.i, label %postload1220.i

preload1219.i:                                    ; preds = %postload1218.i
  store <4 x float> %463, <4 x float> addrspace(1)* %492, align 16
  br label %postload1220.i

postload1220.i:                                   ; preds = %preload1219.i, %postload1218.i
  br i1 %extract764.i, label %preload1221.i, label %postload1222.i

preload1221.i:                                    ; preds = %postload1220.i
  store <4 x float> %464, <4 x float> addrspace(1)* %495, align 16
  br label %postload1222.i

postload1222.i:                                   ; preds = %preload1221.i, %postload1220.i
  br i1 %extract765.i, label %preload1223.i, label %postload1224.i

preload1223.i:                                    ; preds = %postload1222.i
  store <4 x float> %465, <4 x float> addrspace(1)* %498, align 16
  br label %postload1224.i

postload1224.i:                                   ; preds = %preload1223.i, %postload1222.i
  br i1 %extract766.i, label %preload1225.i, label %postload1226.i

preload1225.i:                                    ; preds = %postload1224.i
  store <4 x float> %466, <4 x float> addrspace(1)* %501, align 16
  br label %postload1226.i

postload1226.i:                                   ; preds = %preload1225.i, %postload1224.i
  br i1 %extract767.i, label %preload1227.i, label %postload1228.i

preload1227.i:                                    ; preds = %postload1226.i
  store <4 x float> %467, <4 x float> addrspace(1)* %504, align 16
  br label %postload1228.i

postload1228.i:                                   ; preds = %preload1227.i, %postload1226.i
  br i1 %extract768.i, label %preload1229.i, label %postload1230.i

preload1229.i:                                    ; preds = %postload1228.i
  store <4 x float> %468, <4 x float> addrspace(1)* %507, align 16
  br label %postload1230.i

postload1230.i:                                   ; preds = %preload1229.i, %postload1228.i
  br i1 %extract769.i, label %preload1231.i, label %postload1232.i

preload1231.i:                                    ; preds = %postload1230.i
  store <4 x float> %469, <4 x float> addrspace(1)* %510, align 16
  br label %postload1232.i

postload1232.i:                                   ; preds = %preload1231.i, %postload1230.i
  br i1 %extract770.i, label %preload1233.i, label %postload1234.i

preload1233.i:                                    ; preds = %postload1232.i
  store <4 x float> %470, <4 x float> addrspace(1)* %513, align 16
  br label %postload1234.i

postload1234.i:                                   ; preds = %preload1233.i, %postload1232.i
  br i1 %extract771.i, label %preload1235.i, label %postload1236.i

preload1235.i:                                    ; preds = %postload1234.i
  store <4 x float> %471, <4 x float> addrspace(1)* %516, align 16
  br label %postload1236.i

postload1236.i:                                   ; preds = %preload1235.i, %postload1234.i
  br i1 %extract772.i, label %preload1237.i, label %postload1238.i

preload1237.i:                                    ; preds = %postload1236.i
  store <4 x float> %472, <4 x float> addrspace(1)* %519, align 16
  br label %postload1238.i

postload1238.i:                                   ; preds = %preload1237.i, %postload1236.i
  %"&(pSB[currWI].offset)1630.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1631.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1630.i"
  %CastToValueType1632.i = bitcast i8* %"&pSB[currWI].offset1631.i" to <16 x i32>*
  %loadedValue1633.i = load <16 x i32>* %CastToValueType1632.i, align 64
  %527 = icmp slt <16 x i32> %loadedValue1633.i, %vector.i
  %_to_106773.i = and <16 x i1> %_to_99735.i, %527
  %extract791.i = extractelement <16 x i1> %_to_106773.i, i32 1
  %extract792.i = extractelement <16 x i1> %_to_106773.i, i32 2
  %extract793.i = extractelement <16 x i1> %_to_106773.i, i32 3
  %extract794.i = extractelement <16 x i1> %_to_106773.i, i32 4
  %extract795.i = extractelement <16 x i1> %_to_106773.i, i32 5
  %extract796.i = extractelement <16 x i1> %_to_106773.i, i32 6
  %extract797.i = extractelement <16 x i1> %_to_106773.i, i32 7
  %extract798.i = extractelement <16 x i1> %_to_106773.i, i32 8
  %extract799.i = extractelement <16 x i1> %_to_106773.i, i32 9
  %extract800.i = extractelement <16 x i1> %_to_106773.i, i32 10
  %extract801.i = extractelement <16 x i1> %_to_106773.i, i32 11
  %extract802.i = extractelement <16 x i1> %_to_106773.i, i32 12
  %extract803.i = extractelement <16 x i1> %_to_106773.i, i32 13
  %extract804.i = extractelement <16 x i1> %_to_106773.i, i32 14
  %extract805.i = extractelement <16 x i1> %_to_106773.i, i32 15
  %extract790.i = extractelement <16 x i1> %_to_106773.i, i32 0
  %"&(pSB[currWI].offset)1530.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1531.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1530.i"
  %CastToValueType1532.i = bitcast i8* %"&pSB[currWI].offset1531.i" to <16 x i32>*
  %loadedValue1533.i = load <16 x i32>* %CastToValueType1532.i, align 64
  %528 = extractelement <16 x i32> %loadedValue1533.i, i32 1
  %529 = sext i32 %528 to i64
  %530 = getelementptr inbounds float addrspace(1)* %1, i64 %529
  %"&(pSB[currWI].offset)1525.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1526.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1525.i"
  %CastToValueType1527.i = bitcast i8* %"&pSB[currWI].offset1526.i" to <16 x i32>*
  %loadedValue1528.i = load <16 x i32>* %CastToValueType1527.i, align 64
  %531 = extractelement <16 x i32> %loadedValue1528.i, i32 2
  %532 = sext i32 %531 to i64
  %533 = getelementptr inbounds float addrspace(1)* %1, i64 %532
  %"&(pSB[currWI].offset)1520.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1521.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1520.i"
  %CastToValueType1522.i = bitcast i8* %"&pSB[currWI].offset1521.i" to <16 x i32>*
  %loadedValue1523.i = load <16 x i32>* %CastToValueType1522.i, align 64
  %534 = extractelement <16 x i32> %loadedValue1523.i, i32 3
  %535 = sext i32 %534 to i64
  %536 = getelementptr inbounds float addrspace(1)* %1, i64 %535
  %"&(pSB[currWI].offset)1515.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1516.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1515.i"
  %CastToValueType1517.i = bitcast i8* %"&pSB[currWI].offset1516.i" to <16 x i32>*
  %loadedValue1518.i = load <16 x i32>* %CastToValueType1517.i, align 64
  %537 = extractelement <16 x i32> %loadedValue1518.i, i32 4
  %538 = sext i32 %537 to i64
  %539 = getelementptr inbounds float addrspace(1)* %1, i64 %538
  %"&(pSB[currWI].offset)1510.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1511.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1510.i"
  %CastToValueType1512.i = bitcast i8* %"&pSB[currWI].offset1511.i" to <16 x i32>*
  %loadedValue1513.i = load <16 x i32>* %CastToValueType1512.i, align 64
  %540 = extractelement <16 x i32> %loadedValue1513.i, i32 5
  %541 = sext i32 %540 to i64
  %542 = getelementptr inbounds float addrspace(1)* %1, i64 %541
  %"&(pSB[currWI].offset)1505.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1506.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1505.i"
  %CastToValueType1507.i = bitcast i8* %"&pSB[currWI].offset1506.i" to <16 x i32>*
  %loadedValue1508.i = load <16 x i32>* %CastToValueType1507.i, align 64
  %543 = extractelement <16 x i32> %loadedValue1508.i, i32 6
  %544 = sext i32 %543 to i64
  %545 = getelementptr inbounds float addrspace(1)* %1, i64 %544
  %"&(pSB[currWI].offset)1500.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1501.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1500.i"
  %CastToValueType1502.i = bitcast i8* %"&pSB[currWI].offset1501.i" to <16 x i32>*
  %loadedValue1503.i = load <16 x i32>* %CastToValueType1502.i, align 64
  %546 = extractelement <16 x i32> %loadedValue1503.i, i32 7
  %547 = sext i32 %546 to i64
  %548 = getelementptr inbounds float addrspace(1)* %1, i64 %547
  %"&(pSB[currWI].offset)1495.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1496.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1495.i"
  %CastToValueType1497.i = bitcast i8* %"&pSB[currWI].offset1496.i" to <16 x i32>*
  %loadedValue1498.i = load <16 x i32>* %CastToValueType1497.i, align 64
  %549 = extractelement <16 x i32> %loadedValue1498.i, i32 8
  %550 = sext i32 %549 to i64
  %551 = getelementptr inbounds float addrspace(1)* %1, i64 %550
  %"&(pSB[currWI].offset)1490.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1491.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1490.i"
  %CastToValueType1492.i = bitcast i8* %"&pSB[currWI].offset1491.i" to <16 x i32>*
  %loadedValue1493.i = load <16 x i32>* %CastToValueType1492.i, align 64
  %552 = extractelement <16 x i32> %loadedValue1493.i, i32 9
  %553 = sext i32 %552 to i64
  %554 = getelementptr inbounds float addrspace(1)* %1, i64 %553
  %"&(pSB[currWI].offset)1485.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1486.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1485.i"
  %CastToValueType1487.i = bitcast i8* %"&pSB[currWI].offset1486.i" to <16 x i32>*
  %loadedValue1488.i = load <16 x i32>* %CastToValueType1487.i, align 64
  %555 = extractelement <16 x i32> %loadedValue1488.i, i32 10
  %556 = sext i32 %555 to i64
  %557 = getelementptr inbounds float addrspace(1)* %1, i64 %556
  %"&(pSB[currWI].offset)1480.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1481.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1480.i"
  %CastToValueType1482.i = bitcast i8* %"&pSB[currWI].offset1481.i" to <16 x i32>*
  %loadedValue1483.i = load <16 x i32>* %CastToValueType1482.i, align 64
  %558 = extractelement <16 x i32> %loadedValue1483.i, i32 11
  %559 = sext i32 %558 to i64
  %560 = getelementptr inbounds float addrspace(1)* %1, i64 %559
  %"&(pSB[currWI].offset)1475.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1476.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1475.i"
  %CastToValueType1477.i = bitcast i8* %"&pSB[currWI].offset1476.i" to <16 x i32>*
  %loadedValue1478.i = load <16 x i32>* %CastToValueType1477.i, align 64
  %561 = extractelement <16 x i32> %loadedValue1478.i, i32 12
  %562 = sext i32 %561 to i64
  %563 = getelementptr inbounds float addrspace(1)* %1, i64 %562
  %"&(pSB[currWI].offset)1470.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1471.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1470.i"
  %CastToValueType1472.i = bitcast i8* %"&pSB[currWI].offset1471.i" to <16 x i32>*
  %loadedValue1473.i = load <16 x i32>* %CastToValueType1472.i, align 64
  %564 = extractelement <16 x i32> %loadedValue1473.i, i32 13
  %565 = sext i32 %564 to i64
  %566 = getelementptr inbounds float addrspace(1)* %1, i64 %565
  %"&(pSB[currWI].offset)1465.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1466.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1465.i"
  %CastToValueType1467.i = bitcast i8* %"&pSB[currWI].offset1466.i" to <16 x i32>*
  %loadedValue1468.i = load <16 x i32>* %CastToValueType1467.i, align 64
  %567 = extractelement <16 x i32> %loadedValue1468.i, i32 14
  %568 = sext i32 %567 to i64
  %569 = getelementptr inbounds float addrspace(1)* %1, i64 %568
  %"&(pSB[currWI].offset)1460.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1461.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1460.i"
  %CastToValueType1462.i = bitcast i8* %"&pSB[currWI].offset1461.i" to <16 x i32>*
  %loadedValue1463.i = load <16 x i32>* %CastToValueType1462.i, align 64
  %570 = extractelement <16 x i32> %loadedValue1463.i, i32 15
  %571 = sext i32 %570 to i64
  %572 = getelementptr inbounds float addrspace(1)* %1, i64 %571
  br i1 %extract790.i, label %preload1239.i, label %postload1240.i

preload1239.i:                                    ; preds = %postload1238.i
  %"&(pSB[currWI].offset)1535.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1536.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1535.i"
  %CastToValueType1537.i = bitcast i8* %"&pSB[currWI].offset1536.i" to <16 x i32>*
  %loadedValue1538.i = load <16 x i32>* %CastToValueType1537.i, align 64
  %573 = extractelement <16 x i32> %loadedValue1538.i, i32 0
  %574 = sext i32 %573 to i64
  %575 = getelementptr inbounds float addrspace(1)* %1, i64 %574
  %"&(pSB[currWI].offset)1946.i" = add nuw i64 %CurrSBIndex..0.i, 1728
  %"&pSB[currWI].offset1947.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1946.i"
  %CastToValueType1948.i = bitcast i8* %"&pSB[currWI].offset1947.i" to float*
  %loadedValue1949.i = load float* %CastToValueType1948.i, align 4
  store float %loadedValue1949.i, float addrspace(1)* %575, align 4
  br label %postload1240.i

postload1240.i:                                   ; preds = %preload1239.i, %postload1238.i
  br i1 %extract791.i, label %preload1241.i, label %postload1242.i

preload1241.i:                                    ; preds = %postload1240.i
  %"&(pSB[currWI].offset)1960.i" = add nuw i64 %CurrSBIndex..0.i, 1732
  %"&pSB[currWI].offset1961.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1960.i"
  %CastToValueType1962.i = bitcast i8* %"&pSB[currWI].offset1961.i" to float*
  %loadedValue1963.i = load float* %CastToValueType1962.i, align 4
  store float %loadedValue1963.i, float addrspace(1)* %530, align 4
  br label %postload1242.i

postload1242.i:                                   ; preds = %preload1241.i, %postload1240.i
  br i1 %extract792.i, label %preload1243.i, label %postload1244.i

preload1243.i:                                    ; preds = %postload1242.i
  %"&(pSB[currWI].offset)1974.i" = add nuw i64 %CurrSBIndex..0.i, 1736
  %"&pSB[currWI].offset1975.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1974.i"
  %CastToValueType1976.i = bitcast i8* %"&pSB[currWI].offset1975.i" to float*
  %loadedValue1977.i = load float* %CastToValueType1976.i, align 4
  store float %loadedValue1977.i, float addrspace(1)* %533, align 4
  br label %postload1244.i

postload1244.i:                                   ; preds = %preload1243.i, %postload1242.i
  br i1 %extract793.i, label %preload1245.i, label %postload1246.i

preload1245.i:                                    ; preds = %postload1244.i
  %"&(pSB[currWI].offset)1988.i" = add nuw i64 %CurrSBIndex..0.i, 1740
  %"&pSB[currWI].offset1989.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1988.i"
  %CastToValueType1990.i = bitcast i8* %"&pSB[currWI].offset1989.i" to float*
  %loadedValue1991.i = load float* %CastToValueType1990.i, align 4
  store float %loadedValue1991.i, float addrspace(1)* %536, align 4
  br label %postload1246.i

postload1246.i:                                   ; preds = %preload1245.i, %postload1244.i
  br i1 %extract794.i, label %preload1247.i, label %postload1248.i

preload1247.i:                                    ; preds = %postload1246.i
  %"&(pSB[currWI].offset)2002.i" = add nuw i64 %CurrSBIndex..0.i, 1744
  %"&pSB[currWI].offset2003.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2002.i"
  %CastToValueType2004.i = bitcast i8* %"&pSB[currWI].offset2003.i" to float*
  %loadedValue2005.i = load float* %CastToValueType2004.i, align 4
  store float %loadedValue2005.i, float addrspace(1)* %539, align 4
  br label %postload1248.i

postload1248.i:                                   ; preds = %preload1247.i, %postload1246.i
  br i1 %extract795.i, label %preload1249.i, label %postload1250.i

preload1249.i:                                    ; preds = %postload1248.i
  %"&(pSB[currWI].offset)2016.i" = add nuw i64 %CurrSBIndex..0.i, 1748
  %"&pSB[currWI].offset2017.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2016.i"
  %CastToValueType2018.i = bitcast i8* %"&pSB[currWI].offset2017.i" to float*
  %loadedValue2019.i = load float* %CastToValueType2018.i, align 4
  store float %loadedValue2019.i, float addrspace(1)* %542, align 4
  br label %postload1250.i

postload1250.i:                                   ; preds = %preload1249.i, %postload1248.i
  br i1 %extract796.i, label %preload1251.i, label %postload1252.i

preload1251.i:                                    ; preds = %postload1250.i
  %"&(pSB[currWI].offset)2030.i" = add nuw i64 %CurrSBIndex..0.i, 1752
  %"&pSB[currWI].offset2031.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2030.i"
  %CastToValueType2032.i = bitcast i8* %"&pSB[currWI].offset2031.i" to float*
  %loadedValue2033.i = load float* %CastToValueType2032.i, align 4
  store float %loadedValue2033.i, float addrspace(1)* %545, align 4
  br label %postload1252.i

postload1252.i:                                   ; preds = %preload1251.i, %postload1250.i
  br i1 %extract797.i, label %preload1253.i, label %postload1254.i

preload1253.i:                                    ; preds = %postload1252.i
  %"&(pSB[currWI].offset)2044.i" = add nuw i64 %CurrSBIndex..0.i, 1756
  %"&pSB[currWI].offset2045.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2044.i"
  %CastToValueType2046.i = bitcast i8* %"&pSB[currWI].offset2045.i" to float*
  %loadedValue2047.i = load float* %CastToValueType2046.i, align 4
  store float %loadedValue2047.i, float addrspace(1)* %548, align 4
  br label %postload1254.i

postload1254.i:                                   ; preds = %preload1253.i, %postload1252.i
  br i1 %extract798.i, label %preload1255.i, label %postload1256.i

preload1255.i:                                    ; preds = %postload1254.i
  %"&(pSB[currWI].offset)2058.i" = add nuw i64 %CurrSBIndex..0.i, 1760
  %"&pSB[currWI].offset2059.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2058.i"
  %CastToValueType2060.i = bitcast i8* %"&pSB[currWI].offset2059.i" to float*
  %loadedValue2061.i = load float* %CastToValueType2060.i, align 4
  store float %loadedValue2061.i, float addrspace(1)* %551, align 4
  br label %postload1256.i

postload1256.i:                                   ; preds = %preload1255.i, %postload1254.i
  br i1 %extract799.i, label %preload1257.i, label %postload1258.i

preload1257.i:                                    ; preds = %postload1256.i
  %"&(pSB[currWI].offset)2072.i" = add nuw i64 %CurrSBIndex..0.i, 1764
  %"&pSB[currWI].offset2073.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2072.i"
  %CastToValueType2074.i = bitcast i8* %"&pSB[currWI].offset2073.i" to float*
  %loadedValue2075.i = load float* %CastToValueType2074.i, align 4
  store float %loadedValue2075.i, float addrspace(1)* %554, align 4
  br label %postload1258.i

postload1258.i:                                   ; preds = %preload1257.i, %postload1256.i
  br i1 %extract800.i, label %preload1259.i, label %postload1260.i

preload1259.i:                                    ; preds = %postload1258.i
  %"&(pSB[currWI].offset)2086.i" = add nuw i64 %CurrSBIndex..0.i, 1768
  %"&pSB[currWI].offset2087.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2086.i"
  %CastToValueType2088.i = bitcast i8* %"&pSB[currWI].offset2087.i" to float*
  %loadedValue2089.i = load float* %CastToValueType2088.i, align 4
  store float %loadedValue2089.i, float addrspace(1)* %557, align 4
  br label %postload1260.i

postload1260.i:                                   ; preds = %preload1259.i, %postload1258.i
  br i1 %extract801.i, label %preload1261.i, label %postload1262.i

preload1261.i:                                    ; preds = %postload1260.i
  %"&(pSB[currWI].offset)2100.i" = add nuw i64 %CurrSBIndex..0.i, 1772
  %"&pSB[currWI].offset2101.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2100.i"
  %CastToValueType2102.i = bitcast i8* %"&pSB[currWI].offset2101.i" to float*
  %loadedValue2103.i = load float* %CastToValueType2102.i, align 4
  store float %loadedValue2103.i, float addrspace(1)* %560, align 4
  br label %postload1262.i

postload1262.i:                                   ; preds = %preload1261.i, %postload1260.i
  br i1 %extract802.i, label %preload1263.i, label %postload1264.i

preload1263.i:                                    ; preds = %postload1262.i
  %"&(pSB[currWI].offset)2114.i" = add nuw i64 %CurrSBIndex..0.i, 1776
  %"&pSB[currWI].offset2115.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2114.i"
  %CastToValueType2116.i = bitcast i8* %"&pSB[currWI].offset2115.i" to float*
  %loadedValue2117.i = load float* %CastToValueType2116.i, align 4
  store float %loadedValue2117.i, float addrspace(1)* %563, align 4
  br label %postload1264.i

postload1264.i:                                   ; preds = %preload1263.i, %postload1262.i
  br i1 %extract803.i, label %preload1265.i, label %postload1266.i

preload1265.i:                                    ; preds = %postload1264.i
  %"&(pSB[currWI].offset)2128.i" = add nuw i64 %CurrSBIndex..0.i, 1780
  %"&pSB[currWI].offset2129.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2128.i"
  %CastToValueType2130.i = bitcast i8* %"&pSB[currWI].offset2129.i" to float*
  %loadedValue2131.i = load float* %CastToValueType2130.i, align 4
  store float %loadedValue2131.i, float addrspace(1)* %566, align 4
  br label %postload1266.i

postload1266.i:                                   ; preds = %preload1265.i, %postload1264.i
  br i1 %extract804.i, label %preload1267.i, label %postload1268.i

preload1267.i:                                    ; preds = %postload1266.i
  %"&(pSB[currWI].offset)2142.i" = add nuw i64 %CurrSBIndex..0.i, 1784
  %"&pSB[currWI].offset2143.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2142.i"
  %CastToValueType2144.i = bitcast i8* %"&pSB[currWI].offset2143.i" to float*
  %loadedValue2145.i = load float* %CastToValueType2144.i, align 4
  store float %loadedValue2145.i, float addrspace(1)* %569, align 4
  br label %postload1268.i

postload1268.i:                                   ; preds = %preload1267.i, %postload1266.i
  br i1 %extract805.i, label %preload1269.i, label %postload1270.i

preload1269.i:                                    ; preds = %postload1268.i
  %"&(pSB[currWI].offset)2156.i" = add nuw i64 %CurrSBIndex..0.i, 1788
  %"&pSB[currWI].offset2157.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2156.i"
  %CastToValueType2158.i = bitcast i8* %"&pSB[currWI].offset2157.i" to float*
  %loadedValue2159.i = load float* %CastToValueType2158.i, align 4
  store float %loadedValue2159.i, float addrspace(1)* %572, align 4
  br label %postload1270.i

postload1270.i:                                   ; preds = %preload1269.i, %postload1268.i
  %"&(pSB[currWI].offset)1625.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1626.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1625.i"
  %CastToValueType1627.i = bitcast i8* %"&pSB[currWI].offset1626.i" to <16 x i32>*
  %loadedValue1628.i = load <16 x i32>* %CastToValueType1627.i, align 64
  %576 = or <16 x i32> %loadedValue1628.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %577 = icmp slt <16 x i32> %576, %vector.i
  %_to_110806.i = and <16 x i1> %_to_106773.i, %577
  %extract824.i = extractelement <16 x i1> %_to_110806.i, i32 1
  %extract825.i = extractelement <16 x i1> %_to_110806.i, i32 2
  %extract826.i = extractelement <16 x i1> %_to_110806.i, i32 3
  %extract827.i = extractelement <16 x i1> %_to_110806.i, i32 4
  %extract828.i = extractelement <16 x i1> %_to_110806.i, i32 5
  %extract829.i = extractelement <16 x i1> %_to_110806.i, i32 6
  %extract830.i = extractelement <16 x i1> %_to_110806.i, i32 7
  %extract831.i = extractelement <16 x i1> %_to_110806.i, i32 8
  %extract832.i = extractelement <16 x i1> %_to_110806.i, i32 9
  %extract833.i = extractelement <16 x i1> %_to_110806.i, i32 10
  %extract834.i = extractelement <16 x i1> %_to_110806.i, i32 11
  %extract835.i = extractelement <16 x i1> %_to_110806.i, i32 12
  %extract836.i = extractelement <16 x i1> %_to_110806.i, i32 13
  %extract837.i = extractelement <16 x i1> %_to_110806.i, i32 14
  %extract838.i = extractelement <16 x i1> %_to_110806.i, i32 15
  %extract823.i = extractelement <16 x i1> %_to_110806.i, i32 0
  %578 = extractelement <16 x i32> %576, i32 1
  %579 = sext i32 %578 to i64
  %580 = getelementptr inbounds float addrspace(1)* %1, i64 %579
  %581 = extractelement <16 x i32> %576, i32 2
  %582 = sext i32 %581 to i64
  %583 = getelementptr inbounds float addrspace(1)* %1, i64 %582
  %584 = extractelement <16 x i32> %576, i32 3
  %585 = sext i32 %584 to i64
  %586 = getelementptr inbounds float addrspace(1)* %1, i64 %585
  %587 = extractelement <16 x i32> %576, i32 4
  %588 = sext i32 %587 to i64
  %589 = getelementptr inbounds float addrspace(1)* %1, i64 %588
  %590 = extractelement <16 x i32> %576, i32 5
  %591 = sext i32 %590 to i64
  %592 = getelementptr inbounds float addrspace(1)* %1, i64 %591
  %593 = extractelement <16 x i32> %576, i32 6
  %594 = sext i32 %593 to i64
  %595 = getelementptr inbounds float addrspace(1)* %1, i64 %594
  %596 = extractelement <16 x i32> %576, i32 7
  %597 = sext i32 %596 to i64
  %598 = getelementptr inbounds float addrspace(1)* %1, i64 %597
  %599 = extractelement <16 x i32> %576, i32 8
  %600 = sext i32 %599 to i64
  %601 = getelementptr inbounds float addrspace(1)* %1, i64 %600
  %602 = extractelement <16 x i32> %576, i32 9
  %603 = sext i32 %602 to i64
  %604 = getelementptr inbounds float addrspace(1)* %1, i64 %603
  %605 = extractelement <16 x i32> %576, i32 10
  %606 = sext i32 %605 to i64
  %607 = getelementptr inbounds float addrspace(1)* %1, i64 %606
  %608 = extractelement <16 x i32> %576, i32 11
  %609 = sext i32 %608 to i64
  %610 = getelementptr inbounds float addrspace(1)* %1, i64 %609
  %611 = extractelement <16 x i32> %576, i32 12
  %612 = sext i32 %611 to i64
  %613 = getelementptr inbounds float addrspace(1)* %1, i64 %612
  %614 = extractelement <16 x i32> %576, i32 13
  %615 = sext i32 %614 to i64
  %616 = getelementptr inbounds float addrspace(1)* %1, i64 %615
  %617 = extractelement <16 x i32> %576, i32 14
  %618 = sext i32 %617 to i64
  %619 = getelementptr inbounds float addrspace(1)* %1, i64 %618
  %620 = extractelement <16 x i32> %576, i32 15
  %621 = sext i32 %620 to i64
  %622 = getelementptr inbounds float addrspace(1)* %1, i64 %621
  br i1 %extract823.i, label %preload1271.i, label %postload1272.i

preload1271.i:                                    ; preds = %postload1270.i
  %623 = extractelement <16 x i32> %576, i32 0
  %624 = sext i32 %623 to i64
  %625 = getelementptr inbounds float addrspace(1)* %1, i64 %624
  store float %extract684.i, float addrspace(1)* %625, align 4
  br label %postload1272.i

postload1272.i:                                   ; preds = %preload1271.i, %postload1270.i
  br i1 %extract824.i, label %preload1273.i, label %postload1274.i

preload1273.i:                                    ; preds = %postload1272.i
  store float %extract685.i, float addrspace(1)* %580, align 4
  br label %postload1274.i

postload1274.i:                                   ; preds = %preload1273.i, %postload1272.i
  br i1 %extract825.i, label %preload1275.i, label %postload1276.i

preload1275.i:                                    ; preds = %postload1274.i
  store float %extract686.i, float addrspace(1)* %583, align 4
  br label %postload1276.i

postload1276.i:                                   ; preds = %preload1275.i, %postload1274.i
  br i1 %extract826.i, label %preload1277.i, label %postload1278.i

preload1277.i:                                    ; preds = %postload1276.i
  store float %extract687.i, float addrspace(1)* %586, align 4
  br label %postload1278.i

postload1278.i:                                   ; preds = %preload1277.i, %postload1276.i
  br i1 %extract827.i, label %preload1279.i, label %postload1280.i

preload1279.i:                                    ; preds = %postload1278.i
  store float %extract688.i, float addrspace(1)* %589, align 4
  br label %postload1280.i

postload1280.i:                                   ; preds = %preload1279.i, %postload1278.i
  br i1 %extract828.i, label %preload1281.i, label %postload1282.i

preload1281.i:                                    ; preds = %postload1280.i
  store float %extract689.i, float addrspace(1)* %592, align 4
  br label %postload1282.i

postload1282.i:                                   ; preds = %preload1281.i, %postload1280.i
  br i1 %extract829.i, label %preload1283.i, label %postload1284.i

preload1283.i:                                    ; preds = %postload1282.i
  store float %extract690.i, float addrspace(1)* %595, align 4
  br label %postload1284.i

postload1284.i:                                   ; preds = %preload1283.i, %postload1282.i
  br i1 %extract830.i, label %preload1285.i, label %postload1286.i

preload1285.i:                                    ; preds = %postload1284.i
  store float %extract691.i, float addrspace(1)* %598, align 4
  br label %postload1286.i

postload1286.i:                                   ; preds = %preload1285.i, %postload1284.i
  br i1 %extract831.i, label %preload1287.i, label %postload1288.i

preload1287.i:                                    ; preds = %postload1286.i
  store float %extract692.i, float addrspace(1)* %601, align 4
  br label %postload1288.i

postload1288.i:                                   ; preds = %preload1287.i, %postload1286.i
  br i1 %extract832.i, label %preload1289.i, label %postload1290.i

preload1289.i:                                    ; preds = %postload1288.i
  store float %extract693.i, float addrspace(1)* %604, align 4
  br label %postload1290.i

postload1290.i:                                   ; preds = %preload1289.i, %postload1288.i
  br i1 %extract833.i, label %preload1291.i, label %postload1292.i

preload1291.i:                                    ; preds = %postload1290.i
  store float %extract694.i, float addrspace(1)* %607, align 4
  br label %postload1292.i

postload1292.i:                                   ; preds = %preload1291.i, %postload1290.i
  br i1 %extract834.i, label %preload1293.i, label %postload1294.i

preload1293.i:                                    ; preds = %postload1292.i
  store float %extract695.i, float addrspace(1)* %610, align 4
  br label %postload1294.i

postload1294.i:                                   ; preds = %preload1293.i, %postload1292.i
  br i1 %extract835.i, label %preload1295.i, label %postload1296.i

preload1295.i:                                    ; preds = %postload1294.i
  store float %extract696.i, float addrspace(1)* %613, align 4
  br label %postload1296.i

postload1296.i:                                   ; preds = %preload1295.i, %postload1294.i
  br i1 %extract836.i, label %preload1297.i, label %postload1298.i

preload1297.i:                                    ; preds = %postload1296.i
  store float %extract697.i, float addrspace(1)* %616, align 4
  br label %postload1298.i

postload1298.i:                                   ; preds = %preload1297.i, %postload1296.i
  br i1 %extract837.i, label %preload1299.i, label %postload1300.i

preload1299.i:                                    ; preds = %postload1298.i
  store float %extract698.i, float addrspace(1)* %619, align 4
  br label %postload1300.i

postload1300.i:                                   ; preds = %preload1299.i, %postload1298.i
  br i1 %extract838.i, label %preload1301.i, label %postload1302.i

preload1301.i:                                    ; preds = %postload1300.i
  store float %extract699.i, float addrspace(1)* %622, align 4
  br label %postload1302.i

postload1302.i:                                   ; preds = %preload1301.i, %postload1300.i
  %"&(pSB[currWI].offset)1620.i" = add nuw i64 %CurrSBIndex..0.i, 512
  %"&pSB[currWI].offset1621.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)1620.i"
  %CastToValueType1622.i = bitcast i8* %"&pSB[currWI].offset1621.i" to <16 x i32>*
  %loadedValue1623.i = load <16 x i32>* %CastToValueType1622.i, align 64
  %626 = or <16 x i32> %loadedValue1623.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %627 = icmp slt <16 x i32> %626, %vector.i
  %_to_116839.i = and <16 x i1> %_to_110806.i, %627
  %extract857.i = extractelement <16 x i1> %_to_116839.i, i32 1
  %extract858.i = extractelement <16 x i1> %_to_116839.i, i32 2
  %extract859.i = extractelement <16 x i1> %_to_116839.i, i32 3
  %extract860.i = extractelement <16 x i1> %_to_116839.i, i32 4
  %extract861.i = extractelement <16 x i1> %_to_116839.i, i32 5
  %extract862.i = extractelement <16 x i1> %_to_116839.i, i32 6
  %extract863.i = extractelement <16 x i1> %_to_116839.i, i32 7
  %extract864.i = extractelement <16 x i1> %_to_116839.i, i32 8
  %extract865.i = extractelement <16 x i1> %_to_116839.i, i32 9
  %extract866.i = extractelement <16 x i1> %_to_116839.i, i32 10
  %extract867.i = extractelement <16 x i1> %_to_116839.i, i32 11
  %extract868.i = extractelement <16 x i1> %_to_116839.i, i32 12
  %extract869.i = extractelement <16 x i1> %_to_116839.i, i32 13
  %extract870.i = extractelement <16 x i1> %_to_116839.i, i32 14
  %extract871.i = extractelement <16 x i1> %_to_116839.i, i32 15
  %extract856.i = extractelement <16 x i1> %_to_116839.i, i32 0
  %628 = extractelement <16 x i32> %626, i32 1
  %629 = sext i32 %628 to i64
  %630 = getelementptr inbounds float addrspace(1)* %1, i64 %629
  %631 = extractelement <16 x i32> %626, i32 2
  %632 = sext i32 %631 to i64
  %633 = getelementptr inbounds float addrspace(1)* %1, i64 %632
  %634 = extractelement <16 x i32> %626, i32 3
  %635 = sext i32 %634 to i64
  %636 = getelementptr inbounds float addrspace(1)* %1, i64 %635
  %637 = extractelement <16 x i32> %626, i32 4
  %638 = sext i32 %637 to i64
  %639 = getelementptr inbounds float addrspace(1)* %1, i64 %638
  %640 = extractelement <16 x i32> %626, i32 5
  %641 = sext i32 %640 to i64
  %642 = getelementptr inbounds float addrspace(1)* %1, i64 %641
  %643 = extractelement <16 x i32> %626, i32 6
  %644 = sext i32 %643 to i64
  %645 = getelementptr inbounds float addrspace(1)* %1, i64 %644
  %646 = extractelement <16 x i32> %626, i32 7
  %647 = sext i32 %646 to i64
  %648 = getelementptr inbounds float addrspace(1)* %1, i64 %647
  %649 = extractelement <16 x i32> %626, i32 8
  %650 = sext i32 %649 to i64
  %651 = getelementptr inbounds float addrspace(1)* %1, i64 %650
  %652 = extractelement <16 x i32> %626, i32 9
  %653 = sext i32 %652 to i64
  %654 = getelementptr inbounds float addrspace(1)* %1, i64 %653
  %655 = extractelement <16 x i32> %626, i32 10
  %656 = sext i32 %655 to i64
  %657 = getelementptr inbounds float addrspace(1)* %1, i64 %656
  %658 = extractelement <16 x i32> %626, i32 11
  %659 = sext i32 %658 to i64
  %660 = getelementptr inbounds float addrspace(1)* %1, i64 %659
  %661 = extractelement <16 x i32> %626, i32 12
  %662 = sext i32 %661 to i64
  %663 = getelementptr inbounds float addrspace(1)* %1, i64 %662
  %664 = extractelement <16 x i32> %626, i32 13
  %665 = sext i32 %664 to i64
  %666 = getelementptr inbounds float addrspace(1)* %1, i64 %665
  %667 = extractelement <16 x i32> %626, i32 14
  %668 = sext i32 %667 to i64
  %669 = getelementptr inbounds float addrspace(1)* %1, i64 %668
  %670 = extractelement <16 x i32> %626, i32 15
  %671 = sext i32 %670 to i64
  %672 = getelementptr inbounds float addrspace(1)* %1, i64 %671
  br i1 %extract856.i, label %preload1303.i, label %postload1304.i

preload1303.i:                                    ; preds = %postload1302.i
  %673 = extractelement <16 x i32> %626, i32 0
  %674 = sext i32 %673 to i64
  %675 = getelementptr inbounds float addrspace(1)* %1, i64 %674
  store float %extract700.i, float addrspace(1)* %675, align 4
  br label %postload1304.i

postload1304.i:                                   ; preds = %preload1303.i, %postload1302.i
  br i1 %extract857.i, label %preload1305.i, label %postload1306.i

preload1305.i:                                    ; preds = %postload1304.i
  store float %extract701.i, float addrspace(1)* %630, align 4
  br label %postload1306.i

postload1306.i:                                   ; preds = %preload1305.i, %postload1304.i
  br i1 %extract858.i, label %preload1307.i, label %postload1308.i

preload1307.i:                                    ; preds = %postload1306.i
  store float %extract702.i, float addrspace(1)* %633, align 4
  br label %postload1308.i

postload1308.i:                                   ; preds = %preload1307.i, %postload1306.i
  br i1 %extract859.i, label %preload1309.i, label %postload1310.i

preload1309.i:                                    ; preds = %postload1308.i
  store float %extract703.i, float addrspace(1)* %636, align 4
  br label %postload1310.i

postload1310.i:                                   ; preds = %preload1309.i, %postload1308.i
  br i1 %extract860.i, label %preload1311.i, label %postload1312.i

preload1311.i:                                    ; preds = %postload1310.i
  store float %extract704.i, float addrspace(1)* %639, align 4
  br label %postload1312.i

postload1312.i:                                   ; preds = %preload1311.i, %postload1310.i
  br i1 %extract861.i, label %preload1313.i, label %postload1314.i

preload1313.i:                                    ; preds = %postload1312.i
  store float %extract705.i, float addrspace(1)* %642, align 4
  br label %postload1314.i

postload1314.i:                                   ; preds = %preload1313.i, %postload1312.i
  br i1 %extract862.i, label %preload1315.i, label %postload1316.i

preload1315.i:                                    ; preds = %postload1314.i
  store float %extract706.i, float addrspace(1)* %645, align 4
  br label %postload1316.i

postload1316.i:                                   ; preds = %preload1315.i, %postload1314.i
  br i1 %extract863.i, label %preload1317.i, label %postload1318.i

preload1317.i:                                    ; preds = %postload1316.i
  store float %extract707.i, float addrspace(1)* %648, align 4
  br label %postload1318.i

postload1318.i:                                   ; preds = %preload1317.i, %postload1316.i
  br i1 %extract864.i, label %preload1319.i, label %postload1320.i

preload1319.i:                                    ; preds = %postload1318.i
  store float %extract708.i, float addrspace(1)* %651, align 4
  br label %postload1320.i

postload1320.i:                                   ; preds = %preload1319.i, %postload1318.i
  br i1 %extract865.i, label %preload1321.i, label %postload1322.i

preload1321.i:                                    ; preds = %postload1320.i
  store float %extract709.i, float addrspace(1)* %654, align 4
  br label %postload1322.i

postload1322.i:                                   ; preds = %preload1321.i, %postload1320.i
  br i1 %extract866.i, label %preload1323.i, label %postload1324.i

preload1323.i:                                    ; preds = %postload1322.i
  store float %extract710.i, float addrspace(1)* %657, align 4
  br label %postload1324.i

postload1324.i:                                   ; preds = %preload1323.i, %postload1322.i
  br i1 %extract867.i, label %preload1325.i, label %postload1326.i

preload1325.i:                                    ; preds = %postload1324.i
  store float %extract711.i, float addrspace(1)* %660, align 4
  br label %postload1326.i

postload1326.i:                                   ; preds = %preload1325.i, %postload1324.i
  br i1 %extract868.i, label %preload1327.i, label %postload1328.i

preload1327.i:                                    ; preds = %postload1326.i
  store float %extract712.i, float addrspace(1)* %663, align 4
  br label %postload1328.i

postload1328.i:                                   ; preds = %preload1327.i, %postload1326.i
  br i1 %extract869.i, label %preload1329.i, label %postload1330.i

preload1329.i:                                    ; preds = %postload1328.i
  store float %extract713.i, float addrspace(1)* %666, align 4
  br label %postload1330.i

postload1330.i:                                   ; preds = %preload1329.i, %postload1328.i
  br i1 %extract870.i, label %preload1331.i, label %postload1332.i

preload1331.i:                                    ; preds = %postload1330.i
  store float %extract714.i, float addrspace(1)* %669, align 4
  br label %postload1332.i

postload1332.i:                                   ; preds = %preload1331.i, %postload1330.i
  br i1 %extract871.i, label %preload1333.i, label %UnifiedReturnBlock.i

preload1333.i:                                    ; preds = %postload1332.i
  store float %extract715.i, float addrspace(1)* %672, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %34
  br i1 %check.WI.iter.i, label %SyncBB2177.backedge.i, label %____Vectorized_.scan_separated_args.exit

SyncBB2177.backedge.i:                            ; preds = %UnifiedReturnBlock.i, %preload1333.i
  %CurrWI..0.be.i = add i64 %CurrWI..0.i, 1
  %CurrSBIndex..0.be.i = add i64 %CurrSBIndex..0.i, 1792
  br label %SyncBB2177.i

UnifiedReturnBlock.i:                             ; preds = %postload1332.i
  %check.WI.iter2215.i = icmp ult i64 %CurrWI..0.i, %34
  br i1 %check.WI.iter2215.i, label %SyncBB2177.backedge.i, label %____Vectorized_.scan_separated_args.exit

____Vectorized_.scan_separated_args.exit:         ; preds = %preload1333.i, %UnifiedReturnBlock.i
  ret void
}

define void @__Vectorized_.addUniform(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i8 addrspace(3)**
  %10 = load i8 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to %struct.WorkDim**
  %13 = load %struct.WorkDim** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 56
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  %26 = bitcast i8 addrspace(3)* %10 to float addrspace(3)*
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB535.i, %entry
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride541.i", %thenBB535.i ], [ 0, %entry ]
  %CurrWI..1.i = phi i64 [ %"CurrWI++539.i", %thenBB535.i ], [ 0, %entry ]
  %27 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..1.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %broadcast1.i = insertelement <16 x i64> undef, i64 %28, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %29 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %30 = icmp eq <16 x i64> %29, zeroinitializer
  %extract.i = extractelement <16 x i1> %30, i32 0
  %extract42.i = extractelement <16 x i1> %30, i32 1
  %extract43.i = extractelement <16 x i1> %30, i32 2
  %extract44.i = extractelement <16 x i1> %30, i32 3
  %extract45.i = extractelement <16 x i1> %30, i32 4
  %extract46.i = extractelement <16 x i1> %30, i32 5
  %extract47.i = extractelement <16 x i1> %30, i32 6
  %extract48.i = extractelement <16 x i1> %30, i32 7
  %extract49.i = extractelement <16 x i1> %30, i32 8
  %extract50.i = extractelement <16 x i1> %30, i32 9
  %extract51.i = extractelement <16 x i1> %30, i32 10
  %extract52.i = extractelement <16 x i1> %30, i32 11
  %extract53.i = extractelement <16 x i1> %30, i32 12
  %extract54.i = extractelement <16 x i1> %30, i32 13
  %extract55.i = extractelement <16 x i1> %30, i32 14
  %extract56.i = extractelement <16 x i1> %30, i32 15
  br i1 %extract.i, label %preload334.i, label %postload335.i

preload334.i:                                     ; preds = %SyncBB.i
  %31 = load i64* %16, align 8
  br label %postload335.i

postload335.i:                                    ; preds = %preload334.i, %SyncBB.i
  %phi336.i = phi i64 [ undef, %SyncBB.i ], [ %31, %preload334.i ]
  br i1 %extract42.i, label %preload326.i, label %postload327.i

preload326.i:                                     ; preds = %postload335.i
  %32 = load i64* %16, align 8
  br label %postload327.i

postload327.i:                                    ; preds = %preload326.i, %postload335.i
  %phi328.i = phi i64 [ undef, %postload335.i ], [ %32, %preload326.i ]
  br i1 %extract43.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload327.i
  %33 = load i64* %16, align 8
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload327.i
  %phi.i = phi i64 [ undef, %postload327.i ], [ %33, %preload.i ]
  br i1 %extract44.i, label %preload342.i, label %postload343.i

preload342.i:                                     ; preds = %postload.i
  %34 = load i64* %16, align 8
  br label %postload343.i

postload343.i:                                    ; preds = %preload342.i, %postload.i
  %phi344.i = phi i64 [ undef, %postload.i ], [ %34, %preload342.i ]
  br i1 %extract45.i, label %preload310.i, label %postload311.i

preload310.i:                                     ; preds = %postload343.i
  %35 = load i64* %16, align 8
  br label %postload311.i

postload311.i:                                    ; preds = %preload310.i, %postload343.i
  %phi312.i = phi i64 [ undef, %postload343.i ], [ %35, %preload310.i ]
  br i1 %extract46.i, label %preload318.i, label %postload319.i

preload318.i:                                     ; preds = %postload311.i
  %36 = load i64* %16, align 8
  br label %postload319.i

postload319.i:                                    ; preds = %preload318.i, %postload311.i
  %phi320.i = phi i64 [ undef, %postload311.i ], [ %36, %preload318.i ]
  br i1 %extract47.i, label %preload230.i, label %postload231.i

preload230.i:                                     ; preds = %postload319.i
  %37 = load i64* %16, align 8
  br label %postload231.i

postload231.i:                                    ; preds = %preload230.i, %postload319.i
  %phi232.i = phi i64 [ undef, %postload319.i ], [ %37, %preload230.i ]
  br i1 %extract48.i, label %preload238.i, label %postload239.i

preload238.i:                                     ; preds = %postload231.i
  %38 = load i64* %16, align 8
  br label %postload239.i

postload239.i:                                    ; preds = %preload238.i, %postload231.i
  %phi240.i = phi i64 [ undef, %postload231.i ], [ %38, %preload238.i ]
  br i1 %extract49.i, label %preload246.i, label %postload247.i

preload246.i:                                     ; preds = %postload239.i
  %39 = load i64* %16, align 8
  br label %postload247.i

postload247.i:                                    ; preds = %preload246.i, %postload239.i
  %phi248.i = phi i64 [ undef, %postload239.i ], [ %39, %preload246.i ]
  br i1 %extract50.i, label %preload254.i, label %postload255.i

preload254.i:                                     ; preds = %postload247.i
  %40 = load i64* %16, align 8
  br label %postload255.i

postload255.i:                                    ; preds = %preload254.i, %postload247.i
  %phi256.i = phi i64 [ undef, %postload247.i ], [ %40, %preload254.i ]
  br i1 %extract51.i, label %preload262.i, label %postload263.i

preload262.i:                                     ; preds = %postload255.i
  %41 = load i64* %16, align 8
  br label %postload263.i

postload263.i:                                    ; preds = %preload262.i, %postload255.i
  %phi264.i = phi i64 [ undef, %postload255.i ], [ %41, %preload262.i ]
  br i1 %extract52.i, label %preload270.i, label %postload271.i

preload270.i:                                     ; preds = %postload263.i
  %42 = load i64* %16, align 8
  br label %postload271.i

postload271.i:                                    ; preds = %preload270.i, %postload263.i
  %phi272.i = phi i64 [ undef, %postload263.i ], [ %42, %preload270.i ]
  br i1 %extract53.i, label %preload278.i, label %postload279.i

preload278.i:                                     ; preds = %postload271.i
  %43 = load i64* %16, align 8
  br label %postload279.i

postload279.i:                                    ; preds = %preload278.i, %postload271.i
  %phi280.i = phi i64 [ undef, %postload271.i ], [ %43, %preload278.i ]
  br i1 %extract54.i, label %preload286.i, label %postload287.i

preload286.i:                                     ; preds = %postload279.i
  %44 = load i64* %16, align 8
  br label %postload287.i

postload287.i:                                    ; preds = %preload286.i, %postload279.i
  %phi288.i = phi i64 [ undef, %postload279.i ], [ %44, %preload286.i ]
  br i1 %extract55.i, label %preload294.i, label %postload295.i

preload294.i:                                     ; preds = %postload287.i
  %45 = load i64* %16, align 8
  br label %postload295.i

postload295.i:                                    ; preds = %preload294.i, %postload287.i
  %phi296.i = phi i64 [ undef, %postload287.i ], [ %45, %preload294.i ]
  br i1 %extract56.i, label %preload302.i, label %postload303.i

preload302.i:                                     ; preds = %postload295.i
  %46 = load i64* %16, align 8
  br label %postload303.i

postload303.i:                                    ; preds = %preload302.i, %postload295.i
  %phi304.i = phi i64 [ undef, %postload295.i ], [ %46, %preload302.i ]
  %47 = getelementptr inbounds float addrspace(1)* %4, i64 %phi328.i
  %48 = getelementptr inbounds float addrspace(1)* %4, i64 %phi.i
  %49 = getelementptr inbounds float addrspace(1)* %4, i64 %phi344.i
  %50 = getelementptr inbounds float addrspace(1)* %4, i64 %phi312.i
  %51 = getelementptr inbounds float addrspace(1)* %4, i64 %phi320.i
  %52 = getelementptr inbounds float addrspace(1)* %4, i64 %phi232.i
  %53 = getelementptr inbounds float addrspace(1)* %4, i64 %phi240.i
  %54 = getelementptr inbounds float addrspace(1)* %4, i64 %phi248.i
  %55 = getelementptr inbounds float addrspace(1)* %4, i64 %phi256.i
  %56 = getelementptr inbounds float addrspace(1)* %4, i64 %phi264.i
  %57 = getelementptr inbounds float addrspace(1)* %4, i64 %phi272.i
  %58 = getelementptr inbounds float addrspace(1)* %4, i64 %phi280.i
  %59 = getelementptr inbounds float addrspace(1)* %4, i64 %phi288.i
  %60 = getelementptr inbounds float addrspace(1)* %4, i64 %phi296.i
  %61 = getelementptr inbounds float addrspace(1)* %4, i64 %phi304.i
  br i1 %extract.i, label %preload337.i, label %postload338.i

preload337.i:                                     ; preds = %postload303.i
  %62 = getelementptr inbounds float addrspace(1)* %4, i64 %phi336.i
  %masked_load.i = load float addrspace(1)* %62, align 4
  br label %postload338.i

postload338.i:                                    ; preds = %preload337.i, %postload303.i
  %phi339.i = phi float [ undef, %postload303.i ], [ %masked_load.i, %preload337.i ]
  br i1 %extract42.i, label %preload329.i, label %postload330.i

preload329.i:                                     ; preds = %postload338.i
  %masked_load178.i = load float addrspace(1)* %47, align 4
  br label %postload330.i

postload330.i:                                    ; preds = %preload329.i, %postload338.i
  %phi331.i = phi float [ undef, %postload338.i ], [ %masked_load178.i, %preload329.i ]
  br i1 %extract43.i, label %preload225.i, label %postload226.i

preload225.i:                                     ; preds = %postload330.i
  %masked_load179.i = load float addrspace(1)* %48, align 4
  br label %postload226.i

postload226.i:                                    ; preds = %preload225.i, %postload330.i
  %phi227.i = phi float [ undef, %postload330.i ], [ %masked_load179.i, %preload225.i ]
  br i1 %extract44.i, label %preload345.i, label %postload346.i

preload345.i:                                     ; preds = %postload226.i
  %masked_load180.i = load float addrspace(1)* %49, align 4
  br label %postload346.i

postload346.i:                                    ; preds = %preload345.i, %postload226.i
  %phi347.i = phi float [ undef, %postload226.i ], [ %masked_load180.i, %preload345.i ]
  br i1 %extract45.i, label %preload313.i, label %postload314.i

preload313.i:                                     ; preds = %postload346.i
  %masked_load181.i = load float addrspace(1)* %50, align 4
  br label %postload314.i

postload314.i:                                    ; preds = %preload313.i, %postload346.i
  %phi315.i = phi float [ undef, %postload346.i ], [ %masked_load181.i, %preload313.i ]
  br i1 %extract46.i, label %preload321.i, label %postload322.i

preload321.i:                                     ; preds = %postload314.i
  %masked_load182.i = load float addrspace(1)* %51, align 4
  br label %postload322.i

postload322.i:                                    ; preds = %preload321.i, %postload314.i
  %phi323.i = phi float [ undef, %postload314.i ], [ %masked_load182.i, %preload321.i ]
  br i1 %extract47.i, label %preload233.i, label %postload234.i

preload233.i:                                     ; preds = %postload322.i
  %masked_load183.i = load float addrspace(1)* %52, align 4
  br label %postload234.i

postload234.i:                                    ; preds = %preload233.i, %postload322.i
  %phi235.i = phi float [ undef, %postload322.i ], [ %masked_load183.i, %preload233.i ]
  br i1 %extract48.i, label %preload241.i, label %postload242.i

preload241.i:                                     ; preds = %postload234.i
  %masked_load184.i = load float addrspace(1)* %53, align 4
  br label %postload242.i

postload242.i:                                    ; preds = %preload241.i, %postload234.i
  %phi243.i = phi float [ undef, %postload234.i ], [ %masked_load184.i, %preload241.i ]
  br i1 %extract49.i, label %preload249.i, label %postload250.i

preload249.i:                                     ; preds = %postload242.i
  %masked_load185.i = load float addrspace(1)* %54, align 4
  br label %postload250.i

postload250.i:                                    ; preds = %preload249.i, %postload242.i
  %phi251.i = phi float [ undef, %postload242.i ], [ %masked_load185.i, %preload249.i ]
  br i1 %extract50.i, label %preload257.i, label %postload258.i

preload257.i:                                     ; preds = %postload250.i
  %masked_load186.i = load float addrspace(1)* %55, align 4
  br label %postload258.i

postload258.i:                                    ; preds = %preload257.i, %postload250.i
  %phi259.i = phi float [ undef, %postload250.i ], [ %masked_load186.i, %preload257.i ]
  br i1 %extract51.i, label %preload265.i, label %postload266.i

preload265.i:                                     ; preds = %postload258.i
  %masked_load187.i = load float addrspace(1)* %56, align 4
  br label %postload266.i

postload266.i:                                    ; preds = %preload265.i, %postload258.i
  %phi267.i = phi float [ undef, %postload258.i ], [ %masked_load187.i, %preload265.i ]
  br i1 %extract52.i, label %preload273.i, label %postload274.i

preload273.i:                                     ; preds = %postload266.i
  %masked_load188.i = load float addrspace(1)* %57, align 4
  br label %postload274.i

postload274.i:                                    ; preds = %preload273.i, %postload266.i
  %phi275.i = phi float [ undef, %postload266.i ], [ %masked_load188.i, %preload273.i ]
  br i1 %extract53.i, label %preload281.i, label %postload282.i

preload281.i:                                     ; preds = %postload274.i
  %masked_load189.i = load float addrspace(1)* %58, align 4
  br label %postload282.i

postload282.i:                                    ; preds = %preload281.i, %postload274.i
  %phi283.i = phi float [ undef, %postload274.i ], [ %masked_load189.i, %preload281.i ]
  br i1 %extract54.i, label %preload289.i, label %postload290.i

preload289.i:                                     ; preds = %postload282.i
  %masked_load190.i = load float addrspace(1)* %59, align 4
  br label %postload290.i

postload290.i:                                    ; preds = %preload289.i, %postload282.i
  %phi291.i = phi float [ undef, %postload282.i ], [ %masked_load190.i, %preload289.i ]
  br i1 %extract55.i, label %preload297.i, label %postload298.i

preload297.i:                                     ; preds = %postload290.i
  %masked_load191.i = load float addrspace(1)* %60, align 4
  br label %postload298.i

postload298.i:                                    ; preds = %preload297.i, %postload290.i
  %phi299.i = phi float [ undef, %postload290.i ], [ %masked_load191.i, %preload297.i ]
  br i1 %extract56.i, label %preload305.i, label %postload306.i

preload305.i:                                     ; preds = %postload298.i
  %masked_load192.i = load float addrspace(1)* %61, align 4
  br label %postload306.i

postload306.i:                                    ; preds = %preload305.i, %postload298.i
  %phi307.i = phi float [ undef, %postload298.i ], [ %masked_load192.i, %preload305.i ]
  br i1 %extract.i, label %preload340.i, label %postload341.i

preload340.i:                                     ; preds = %postload306.i
  store float %phi339.i, float addrspace(3)* %26, align 4
  br label %postload341.i

postload341.i:                                    ; preds = %preload340.i, %postload306.i
  br i1 %extract42.i, label %preload332.i, label %postload333.i

preload332.i:                                     ; preds = %postload341.i
  store float %phi331.i, float addrspace(3)* %26, align 4
  br label %postload333.i

postload333.i:                                    ; preds = %preload332.i, %postload341.i
  br i1 %extract43.i, label %preload228.i, label %postload229.i

preload228.i:                                     ; preds = %postload333.i
  store float %phi227.i, float addrspace(3)* %26, align 4
  br label %postload229.i

postload229.i:                                    ; preds = %preload228.i, %postload333.i
  br i1 %extract44.i, label %preload348.i, label %postload349.i

preload348.i:                                     ; preds = %postload229.i
  store float %phi347.i, float addrspace(3)* %26, align 4
  br label %postload349.i

postload349.i:                                    ; preds = %preload348.i, %postload229.i
  br i1 %extract45.i, label %preload316.i, label %postload317.i

preload316.i:                                     ; preds = %postload349.i
  store float %phi315.i, float addrspace(3)* %26, align 4
  br label %postload317.i

postload317.i:                                    ; preds = %preload316.i, %postload349.i
  br i1 %extract46.i, label %preload324.i, label %postload325.i

preload324.i:                                     ; preds = %postload317.i
  store float %phi323.i, float addrspace(3)* %26, align 4
  br label %postload325.i

postload325.i:                                    ; preds = %preload324.i, %postload317.i
  br i1 %extract47.i, label %preload236.i, label %postload237.i

preload236.i:                                     ; preds = %postload325.i
  store float %phi235.i, float addrspace(3)* %26, align 4
  br label %postload237.i

postload237.i:                                    ; preds = %preload236.i, %postload325.i
  br i1 %extract48.i, label %preload244.i, label %postload245.i

preload244.i:                                     ; preds = %postload237.i
  store float %phi243.i, float addrspace(3)* %26, align 4
  br label %postload245.i

postload245.i:                                    ; preds = %preload244.i, %postload237.i
  br i1 %extract49.i, label %preload252.i, label %postload253.i

preload252.i:                                     ; preds = %postload245.i
  store float %phi251.i, float addrspace(3)* %26, align 4
  br label %postload253.i

postload253.i:                                    ; preds = %preload252.i, %postload245.i
  br i1 %extract50.i, label %preload260.i, label %postload261.i

preload260.i:                                     ; preds = %postload253.i
  store float %phi259.i, float addrspace(3)* %26, align 4
  br label %postload261.i

postload261.i:                                    ; preds = %preload260.i, %postload253.i
  br i1 %extract51.i, label %preload268.i, label %postload269.i

preload268.i:                                     ; preds = %postload261.i
  store float %phi267.i, float addrspace(3)* %26, align 4
  br label %postload269.i

postload269.i:                                    ; preds = %preload268.i, %postload261.i
  br i1 %extract52.i, label %preload276.i, label %postload277.i

preload276.i:                                     ; preds = %postload269.i
  store float %phi275.i, float addrspace(3)* %26, align 4
  br label %postload277.i

postload277.i:                                    ; preds = %preload276.i, %postload269.i
  br i1 %extract53.i, label %preload284.i, label %postload285.i

preload284.i:                                     ; preds = %postload277.i
  store float %phi283.i, float addrspace(3)* %26, align 4
  br label %postload285.i

postload285.i:                                    ; preds = %preload284.i, %postload277.i
  br i1 %extract54.i, label %preload292.i, label %postload293.i

preload292.i:                                     ; preds = %postload285.i
  store float %phi291.i, float addrspace(3)* %26, align 4
  br label %postload293.i

postload293.i:                                    ; preds = %preload292.i, %postload285.i
  br i1 %extract55.i, label %preload300.i, label %postload301.i

preload300.i:                                     ; preds = %postload293.i
  store float %phi299.i, float addrspace(3)* %26, align 4
  br label %postload301.i

postload301.i:                                    ; preds = %preload300.i, %postload293.i
  br i1 %extract56.i, label %preload308.i, label %postload309.i

preload308.i:                                     ; preds = %postload301.i
  store float %phi307.i, float addrspace(3)* %26, align 4
  br label %postload309.i

postload309.i:                                    ; preds = %preload308.i, %postload301.i
  %63 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..1.i, i32 0, i64 0
  %64 = load i64* %63, align 8
  %broadcast157.i = insertelement <16 x i64> undef, i64 %64, i32 0
  %broadcast258.i = shufflevector <16 x i64> %broadcast157.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %65 = add <16 x i64> %broadcast258.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %66 = load i64* %16, align 8
  %67 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %68 = load i64* %67, align 8
  %69 = shl i64 %66, 2
  %70 = mul i64 %69, %68
  %temp.i = insertelement <16 x i64> undef, i64 %70, i32 0
  %vector.i = shufflevector <16 x i64> %temp.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %71 = add <16 x i64> %vector.i, %65
  %"&(pSB[currWI].offset)9.i" = or i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)9.i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i64>*
  store <16 x i64> %71, <16 x i64>* %CastToValueType.i, align 128
  %check.WI.iter538.i = icmp ult i64 %CurrWI..1.i, %22
  br i1 %check.WI.iter538.i, label %thenBB535.i, label %elseBB536.i

thenBB535.i:                                      ; preds = %postload309.i
  %"CurrWI++539.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride541.i" = add nuw i64 %CurrSBIndex..1.i, 1792
  br label %SyncBB.i

elseBB536.i:                                      ; preds = %postload309.i
  %temp75.i = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector76.i = shufflevector <16 x i32> %temp75.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB533.i

SyncBB533.i:                                      ; preds = %thenBB.i, %elseBB536.i
  %CurrSBIndex..0.i = phi i64 [ 0, %elseBB536.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.i = phi i64 [ 0, %elseBB536.i ], [ %"CurrWI++.i", %thenBB.i ]
  %"&(pSB[currWI].offset)52710.i" = or i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset528.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)52710.i"
  %CastToValueType529.i = bitcast i8* %"&pSB[currWI].offset528.i" to <16 x i64>*
  %loadedValue.i = load <16 x i64>* %CastToValueType529.i, align 128
  br label %postload351.i

postload351.i:                                    ; preds = %postload524.i, %SyncBB533.i
  %vectorPHI59.i = phi <16 x i1> [ zeroinitializer, %SyncBB533.i ], [ %loop_mask2180.i, %postload524.i ]
  %vectorPHI63.i = phi <16 x i1> [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %SyncBB533.i ], [ %local_edge2782.i, %postload524.i ]
  %vectorPHI64.i = phi <16 x i64> [ %loadedValue.i, %SyncBB533.i ], [ %125, %postload524.i ]
  %72 = phi i32 [ 0, %SyncBB533.i ], [ %126, %postload524.i ]
  %73 = icmp slt i32 %72, 4
  %temp71.i = insertelement <16 x i1> undef, i1 %73, i32 0
  %vector72.i = shufflevector <16 x i1> %temp71.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond.i = xor i1 %73, true
  %temp65.i = insertelement <16 x i1> undef, i1 %notCond.i, i32 0
  %vector66.i = shufflevector <16 x i1> %temp65.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr67.i = and <16 x i1> %vectorPHI63.i, %vector66.i
  %loop_mask1369.i = or <16 x i1> %vectorPHI59.i, %who_left_tr67.i
  %local_edge73.i = and <16 x i1> %vectorPHI63.i, %vector72.i
  %address.074.i = trunc <16 x i64> %vectorPHI64.i to <16 x i32>
  %74 = icmp ult <16 x i32> %address.074.i, %vector76.i
  %notCond1777.i = xor <16 x i1> %74, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr1878.i = and <16 x i1> %local_edge73.i, %notCond1777.i
  %loop_mask2180.i = or <16 x i1> %loop_mask1369.i, %who_left_tr1878.i
  %local_edge2782.i = and <16 x i1> %local_edge73.i, %74
  %extract83.i = extractelement <16 x i1> %local_edge2782.i, i32 0
  %extract84.i = extractelement <16 x i1> %local_edge2782.i, i32 1
  %extract85.i = extractelement <16 x i1> %local_edge2782.i, i32 2
  %extract86.i = extractelement <16 x i1> %local_edge2782.i, i32 3
  %extract87.i = extractelement <16 x i1> %local_edge2782.i, i32 4
  %extract88.i = extractelement <16 x i1> %local_edge2782.i, i32 5
  %extract89.i = extractelement <16 x i1> %local_edge2782.i, i32 6
  %extract90.i = extractelement <16 x i1> %local_edge2782.i, i32 7
  %extract91.i = extractelement <16 x i1> %local_edge2782.i, i32 8
  %extract92.i = extractelement <16 x i1> %local_edge2782.i, i32 9
  %extract93.i = extractelement <16 x i1> %local_edge2782.i, i32 10
  %extract94.i = extractelement <16 x i1> %local_edge2782.i, i32 11
  %extract95.i = extractelement <16 x i1> %local_edge2782.i, i32 12
  %extract96.i = extractelement <16 x i1> %local_edge2782.i, i32 13
  %extract97.i = extractelement <16 x i1> %local_edge2782.i, i32 14
  %extract98.i = extractelement <16 x i1> %local_edge2782.i, i32 15
  %masked_load193.i = load float addrspace(3)* %26, align 4
  %temp.vect130.i = insertelement <16 x float> undef, float %masked_load193.i, i32 0
  %temp.vect131.i = insertelement <16 x float> %temp.vect130.i, float %masked_load193.i, i32 1
  %temp.vect132.i = insertelement <16 x float> %temp.vect131.i, float %masked_load193.i, i32 2
  %temp.vect133.i = insertelement <16 x float> %temp.vect132.i, float %masked_load193.i, i32 3
  %temp.vect134.i = insertelement <16 x float> %temp.vect133.i, float %masked_load193.i, i32 4
  %temp.vect135.i = insertelement <16 x float> %temp.vect134.i, float %masked_load193.i, i32 5
  %temp.vect136.i = insertelement <16 x float> %temp.vect135.i, float %masked_load193.i, i32 6
  %temp.vect137.i = insertelement <16 x float> %temp.vect136.i, float %masked_load193.i, i32 7
  %temp.vect138.i = insertelement <16 x float> %temp.vect137.i, float %masked_load193.i, i32 8
  %temp.vect139.i = insertelement <16 x float> %temp.vect138.i, float %masked_load193.i, i32 9
  %temp.vect140.i = insertelement <16 x float> %temp.vect139.i, float %masked_load193.i, i32 10
  %temp.vect141.i = insertelement <16 x float> %temp.vect140.i, float %masked_load193.i, i32 11
  %temp.vect142.i = insertelement <16 x float> %temp.vect141.i, float %masked_load193.i, i32 12
  %temp.vect143.i = insertelement <16 x float> %temp.vect142.i, float %masked_load193.i, i32 13
  %temp.vect144.i = insertelement <16 x float> %temp.vect143.i, float %masked_load193.i, i32 14
  %temp.vect145.i = insertelement <16 x float> %temp.vect144.i, float %masked_load193.i, i32 15
  %75 = and <16 x i64> %vectorPHI64.i, <i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295, i64 4294967295>
  %extract99.i = extractelement <16 x i64> %75, i32 0
  %extract100.i = extractelement <16 x i64> %75, i32 1
  %extract101.i = extractelement <16 x i64> %75, i32 2
  %extract102.i = extractelement <16 x i64> %75, i32 3
  %extract103.i = extractelement <16 x i64> %75, i32 4
  %extract104.i = extractelement <16 x i64> %75, i32 5
  %extract105.i = extractelement <16 x i64> %75, i32 6
  %extract106.i = extractelement <16 x i64> %75, i32 7
  %extract107.i = extractelement <16 x i64> %75, i32 8
  %extract108.i = extractelement <16 x i64> %75, i32 9
  %extract109.i = extractelement <16 x i64> %75, i32 10
  %extract110.i = extractelement <16 x i64> %75, i32 11
  %extract111.i = extractelement <16 x i64> %75, i32 12
  %extract112.i = extractelement <16 x i64> %75, i32 13
  %extract113.i = extractelement <16 x i64> %75, i32 14
  %extract114.i = extractelement <16 x i64> %75, i32 15
  %76 = getelementptr inbounds float addrspace(1)* %1, i64 %extract99.i
  %77 = getelementptr inbounds float addrspace(1)* %1, i64 %extract100.i
  %78 = getelementptr inbounds float addrspace(1)* %1, i64 %extract101.i
  %79 = getelementptr inbounds float addrspace(1)* %1, i64 %extract102.i
  %80 = getelementptr inbounds float addrspace(1)* %1, i64 %extract103.i
  %81 = getelementptr inbounds float addrspace(1)* %1, i64 %extract104.i
  %82 = getelementptr inbounds float addrspace(1)* %1, i64 %extract105.i
  %83 = getelementptr inbounds float addrspace(1)* %1, i64 %extract106.i
  %84 = getelementptr inbounds float addrspace(1)* %1, i64 %extract107.i
  %85 = getelementptr inbounds float addrspace(1)* %1, i64 %extract108.i
  %86 = getelementptr inbounds float addrspace(1)* %1, i64 %extract109.i
  %87 = getelementptr inbounds float addrspace(1)* %1, i64 %extract110.i
  %88 = getelementptr inbounds float addrspace(1)* %1, i64 %extract111.i
  %89 = getelementptr inbounds float addrspace(1)* %1, i64 %extract112.i
  %90 = getelementptr inbounds float addrspace(1)* %1, i64 %extract113.i
  %91 = getelementptr inbounds float addrspace(1)* %1, i64 %extract114.i
  br i1 %extract83.i, label %preload353.i, label %postload354.i

preload353.i:                                     ; preds = %postload351.i
  %masked_load209.i = load float addrspace(1)* %76, align 4
  br label %postload354.i

postload354.i:                                    ; preds = %preload353.i, %postload351.i
  %phi355.i = phi float [ undef, %postload351.i ], [ %masked_load209.i, %preload353.i ]
  br i1 %extract84.i, label %preload364.i, label %postload365.i

preload364.i:                                     ; preds = %postload354.i
  %masked_load210.i = load float addrspace(1)* %77, align 4
  br label %postload365.i

postload365.i:                                    ; preds = %preload364.i, %postload354.i
  %phi366.i = phi float [ undef, %postload354.i ], [ %masked_load210.i, %preload364.i ]
  br i1 %extract85.i, label %preload375.i, label %postload376.i

preload375.i:                                     ; preds = %postload365.i
  %masked_load211.i = load float addrspace(1)* %78, align 4
  br label %postload376.i

postload376.i:                                    ; preds = %preload375.i, %postload365.i
  %phi377.i = phi float [ undef, %postload365.i ], [ %masked_load211.i, %preload375.i ]
  br i1 %extract86.i, label %preload386.i, label %postload387.i

preload386.i:                                     ; preds = %postload376.i
  %masked_load212.i = load float addrspace(1)* %79, align 4
  br label %postload387.i

postload387.i:                                    ; preds = %preload386.i, %postload376.i
  %phi388.i = phi float [ undef, %postload376.i ], [ %masked_load212.i, %preload386.i ]
  br i1 %extract87.i, label %preload397.i, label %postload398.i

preload397.i:                                     ; preds = %postload387.i
  %masked_load213.i = load float addrspace(1)* %80, align 4
  br label %postload398.i

postload398.i:                                    ; preds = %preload397.i, %postload387.i
  %phi399.i = phi float [ undef, %postload387.i ], [ %masked_load213.i, %preload397.i ]
  br i1 %extract88.i, label %preload408.i, label %postload409.i

preload408.i:                                     ; preds = %postload398.i
  %masked_load214.i = load float addrspace(1)* %81, align 4
  br label %postload409.i

postload409.i:                                    ; preds = %preload408.i, %postload398.i
  %phi410.i = phi float [ undef, %postload398.i ], [ %masked_load214.i, %preload408.i ]
  br i1 %extract89.i, label %preload419.i, label %postload420.i

preload419.i:                                     ; preds = %postload409.i
  %masked_load215.i = load float addrspace(1)* %82, align 4
  br label %postload420.i

postload420.i:                                    ; preds = %preload419.i, %postload409.i
  %phi421.i = phi float [ undef, %postload409.i ], [ %masked_load215.i, %preload419.i ]
  br i1 %extract90.i, label %preload430.i, label %postload431.i

preload430.i:                                     ; preds = %postload420.i
  %masked_load216.i = load float addrspace(1)* %83, align 4
  br label %postload431.i

postload431.i:                                    ; preds = %preload430.i, %postload420.i
  %phi432.i = phi float [ undef, %postload420.i ], [ %masked_load216.i, %preload430.i ]
  br i1 %extract91.i, label %preload441.i, label %postload442.i

preload441.i:                                     ; preds = %postload431.i
  %masked_load217.i = load float addrspace(1)* %84, align 4
  br label %postload442.i

postload442.i:                                    ; preds = %preload441.i, %postload431.i
  %phi443.i = phi float [ undef, %postload431.i ], [ %masked_load217.i, %preload441.i ]
  br i1 %extract92.i, label %preload452.i, label %postload453.i

preload452.i:                                     ; preds = %postload442.i
  %masked_load218.i = load float addrspace(1)* %85, align 4
  br label %postload453.i

postload453.i:                                    ; preds = %preload452.i, %postload442.i
  %phi454.i = phi float [ undef, %postload442.i ], [ %masked_load218.i, %preload452.i ]
  br i1 %extract93.i, label %preload463.i, label %postload464.i

preload463.i:                                     ; preds = %postload453.i
  %masked_load219.i = load float addrspace(1)* %86, align 4
  br label %postload464.i

postload464.i:                                    ; preds = %preload463.i, %postload453.i
  %phi465.i = phi float [ undef, %postload453.i ], [ %masked_load219.i, %preload463.i ]
  br i1 %extract94.i, label %preload474.i, label %postload475.i

preload474.i:                                     ; preds = %postload464.i
  %masked_load220.i = load float addrspace(1)* %87, align 4
  br label %postload475.i

postload475.i:                                    ; preds = %preload474.i, %postload464.i
  %phi476.i = phi float [ undef, %postload464.i ], [ %masked_load220.i, %preload474.i ]
  br i1 %extract95.i, label %preload485.i, label %postload486.i

preload485.i:                                     ; preds = %postload475.i
  %masked_load221.i = load float addrspace(1)* %88, align 4
  br label %postload486.i

postload486.i:                                    ; preds = %preload485.i, %postload475.i
  %phi487.i = phi float [ undef, %postload475.i ], [ %masked_load221.i, %preload485.i ]
  br i1 %extract96.i, label %preload496.i, label %postload497.i

preload496.i:                                     ; preds = %postload486.i
  %masked_load222.i = load float addrspace(1)* %89, align 4
  br label %postload497.i

postload497.i:                                    ; preds = %preload496.i, %postload486.i
  %phi498.i = phi float [ undef, %postload486.i ], [ %masked_load222.i, %preload496.i ]
  br i1 %extract97.i, label %preload507.i, label %postload508.i

preload507.i:                                     ; preds = %postload497.i
  %masked_load223.i = load float addrspace(1)* %90, align 4
  br label %postload508.i

postload508.i:                                    ; preds = %preload507.i, %postload497.i
  %phi509.i = phi float [ undef, %postload497.i ], [ %masked_load223.i, %preload507.i ]
  br i1 %extract98.i, label %preload518.i, label %postload519.i

preload518.i:                                     ; preds = %postload508.i
  %masked_load224.i = load float addrspace(1)* %91, align 4
  br label %postload519.i

postload519.i:                                    ; preds = %preload518.i, %postload508.i
  %phi520.i = phi float [ undef, %postload508.i ], [ %masked_load224.i, %preload518.i ]
  %temp.vect.i = insertelement <16 x float> undef, float %phi355.i, i32 0
  %temp.vect115.i = insertelement <16 x float> %temp.vect.i, float %phi366.i, i32 1
  %temp.vect116.i = insertelement <16 x float> %temp.vect115.i, float %phi377.i, i32 2
  %temp.vect117.i = insertelement <16 x float> %temp.vect116.i, float %phi388.i, i32 3
  %temp.vect118.i = insertelement <16 x float> %temp.vect117.i, float %phi399.i, i32 4
  %temp.vect119.i = insertelement <16 x float> %temp.vect118.i, float %phi410.i, i32 5
  %temp.vect120.i = insertelement <16 x float> %temp.vect119.i, float %phi421.i, i32 6
  %temp.vect121.i = insertelement <16 x float> %temp.vect120.i, float %phi432.i, i32 7
  %temp.vect122.i = insertelement <16 x float> %temp.vect121.i, float %phi443.i, i32 8
  %temp.vect123.i = insertelement <16 x float> %temp.vect122.i, float %phi454.i, i32 9
  %temp.vect124.i = insertelement <16 x float> %temp.vect123.i, float %phi465.i, i32 10
  %temp.vect125.i = insertelement <16 x float> %temp.vect124.i, float %phi476.i, i32 11
  %temp.vect126.i = insertelement <16 x float> %temp.vect125.i, float %phi487.i, i32 12
  %temp.vect127.i = insertelement <16 x float> %temp.vect126.i, float %phi498.i, i32 13
  %temp.vect128.i = insertelement <16 x float> %temp.vect127.i, float %phi509.i, i32 14
  %temp.vect129.i = insertelement <16 x float> %temp.vect128.i, float %phi520.i, i32 15
  %92 = fadd <16 x float> %temp.vect129.i, %temp.vect145.i
  %extract147.i = extractelement <16 x float> %92, i32 1
  %extract148.i = extractelement <16 x float> %92, i32 2
  %extract149.i = extractelement <16 x float> %92, i32 3
  %extract150.i = extractelement <16 x float> %92, i32 4
  %extract151.i = extractelement <16 x float> %92, i32 5
  %extract152.i = extractelement <16 x float> %92, i32 6
  %extract153.i = extractelement <16 x float> %92, i32 7
  %extract154.i = extractelement <16 x float> %92, i32 8
  %extract155.i = extractelement <16 x float> %92, i32 9
  %extract156.i = extractelement <16 x float> %92, i32 10
  %extract157.i = extractelement <16 x float> %92, i32 11
  %extract158.i = extractelement <16 x float> %92, i32 12
  %extract159.i = extractelement <16 x float> %92, i32 13
  %extract160.i = extractelement <16 x float> %92, i32 14
  %extract161.i = extractelement <16 x float> %92, i32 15
  br i1 %extract83.i, label %preload356.i, label %postload357.i

preload356.i:                                     ; preds = %postload519.i
  %extract146.i = extractelement <16 x float> %92, i32 0
  store float %extract146.i, float addrspace(1)* %76, align 4
  br label %postload357.i

postload357.i:                                    ; preds = %preload356.i, %postload519.i
  br i1 %extract84.i, label %preload367.i, label %postload368.i

preload367.i:                                     ; preds = %postload357.i
  store float %extract147.i, float addrspace(1)* %77, align 4
  br label %postload368.i

postload368.i:                                    ; preds = %preload367.i, %postload357.i
  br i1 %extract85.i, label %preload378.i, label %postload379.i

preload378.i:                                     ; preds = %postload368.i
  store float %extract148.i, float addrspace(1)* %78, align 4
  br label %postload379.i

postload379.i:                                    ; preds = %preload378.i, %postload368.i
  br i1 %extract86.i, label %preload389.i, label %postload390.i

preload389.i:                                     ; preds = %postload379.i
  store float %extract149.i, float addrspace(1)* %79, align 4
  br label %postload390.i

postload390.i:                                    ; preds = %preload389.i, %postload379.i
  br i1 %extract87.i, label %preload400.i, label %postload401.i

preload400.i:                                     ; preds = %postload390.i
  store float %extract150.i, float addrspace(1)* %80, align 4
  br label %postload401.i

postload401.i:                                    ; preds = %preload400.i, %postload390.i
  br i1 %extract88.i, label %preload411.i, label %postload412.i

preload411.i:                                     ; preds = %postload401.i
  store float %extract151.i, float addrspace(1)* %81, align 4
  br label %postload412.i

postload412.i:                                    ; preds = %preload411.i, %postload401.i
  br i1 %extract89.i, label %preload422.i, label %postload423.i

preload422.i:                                     ; preds = %postload412.i
  store float %extract152.i, float addrspace(1)* %82, align 4
  br label %postload423.i

postload423.i:                                    ; preds = %preload422.i, %postload412.i
  br i1 %extract90.i, label %preload433.i, label %postload434.i

preload433.i:                                     ; preds = %postload423.i
  store float %extract153.i, float addrspace(1)* %83, align 4
  br label %postload434.i

postload434.i:                                    ; preds = %preload433.i, %postload423.i
  br i1 %extract91.i, label %preload444.i, label %postload445.i

preload444.i:                                     ; preds = %postload434.i
  store float %extract154.i, float addrspace(1)* %84, align 4
  br label %postload445.i

postload445.i:                                    ; preds = %preload444.i, %postload434.i
  br i1 %extract92.i, label %preload455.i, label %postload456.i

preload455.i:                                     ; preds = %postload445.i
  store float %extract155.i, float addrspace(1)* %85, align 4
  br label %postload456.i

postload456.i:                                    ; preds = %preload455.i, %postload445.i
  br i1 %extract93.i, label %preload466.i, label %postload467.i

preload466.i:                                     ; preds = %postload456.i
  store float %extract156.i, float addrspace(1)* %86, align 4
  br label %postload467.i

postload467.i:                                    ; preds = %preload466.i, %postload456.i
  br i1 %extract94.i, label %preload477.i, label %postload478.i

preload477.i:                                     ; preds = %postload467.i
  store float %extract157.i, float addrspace(1)* %87, align 4
  br label %postload478.i

postload478.i:                                    ; preds = %preload477.i, %postload467.i
  br i1 %extract95.i, label %preload488.i, label %postload489.i

preload488.i:                                     ; preds = %postload478.i
  store float %extract158.i, float addrspace(1)* %88, align 4
  br label %postload489.i

postload489.i:                                    ; preds = %preload488.i, %postload478.i
  br i1 %extract96.i, label %preload499.i, label %postload500.i

preload499.i:                                     ; preds = %postload489.i
  store float %extract159.i, float addrspace(1)* %89, align 4
  br label %postload500.i

postload500.i:                                    ; preds = %preload499.i, %postload489.i
  br i1 %extract97.i, label %preload510.i, label %postload511.i

preload510.i:                                     ; preds = %postload500.i
  store float %extract160.i, float addrspace(1)* %90, align 4
  br label %postload511.i

postload511.i:                                    ; preds = %preload510.i, %postload500.i
  br i1 %extract98.i, label %preload521.i, label %postload522.i

preload521.i:                                     ; preds = %postload511.i
  store float %extract161.i, float addrspace(1)* %91, align 4
  br label %postload522.i

postload522.i:                                    ; preds = %preload521.i, %postload511.i
  br i1 %extract83.i, label %preload358.i, label %postload359.i

preload358.i:                                     ; preds = %postload522.i
  %93 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %94 = load i64* %93, align 8
  br label %postload359.i

postload359.i:                                    ; preds = %preload358.i, %postload522.i
  %phi360.i = phi i64 [ undef, %postload522.i ], [ %94, %preload358.i ]
  br i1 %extract84.i, label %preload369.i, label %postload370.i

preload369.i:                                     ; preds = %postload359.i
  %95 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %96 = load i64* %95, align 8
  br label %postload370.i

postload370.i:                                    ; preds = %preload369.i, %postload359.i
  %phi371.i = phi i64 [ undef, %postload359.i ], [ %96, %preload369.i ]
  br i1 %extract85.i, label %preload380.i, label %postload381.i

preload380.i:                                     ; preds = %postload370.i
  %97 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %98 = load i64* %97, align 8
  br label %postload381.i

postload381.i:                                    ; preds = %preload380.i, %postload370.i
  %phi382.i = phi i64 [ undef, %postload370.i ], [ %98, %preload380.i ]
  br i1 %extract86.i, label %preload391.i, label %postload392.i

preload391.i:                                     ; preds = %postload381.i
  %99 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %100 = load i64* %99, align 8
  br label %postload392.i

postload392.i:                                    ; preds = %preload391.i, %postload381.i
  %phi393.i = phi i64 [ undef, %postload381.i ], [ %100, %preload391.i ]
  br i1 %extract87.i, label %preload402.i, label %postload403.i

preload402.i:                                     ; preds = %postload392.i
  %101 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %102 = load i64* %101, align 8
  br label %postload403.i

postload403.i:                                    ; preds = %preload402.i, %postload392.i
  %phi404.i = phi i64 [ undef, %postload392.i ], [ %102, %preload402.i ]
  br i1 %extract88.i, label %preload413.i, label %postload414.i

preload413.i:                                     ; preds = %postload403.i
  %103 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %104 = load i64* %103, align 8
  br label %postload414.i

postload414.i:                                    ; preds = %preload413.i, %postload403.i
  %phi415.i = phi i64 [ undef, %postload403.i ], [ %104, %preload413.i ]
  br i1 %extract89.i, label %preload424.i, label %postload425.i

preload424.i:                                     ; preds = %postload414.i
  %105 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %106 = load i64* %105, align 8
  br label %postload425.i

postload425.i:                                    ; preds = %preload424.i, %postload414.i
  %phi426.i = phi i64 [ undef, %postload414.i ], [ %106, %preload424.i ]
  br i1 %extract90.i, label %preload435.i, label %postload436.i

preload435.i:                                     ; preds = %postload425.i
  %107 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %108 = load i64* %107, align 8
  br label %postload436.i

postload436.i:                                    ; preds = %preload435.i, %postload425.i
  %phi437.i = phi i64 [ undef, %postload425.i ], [ %108, %preload435.i ]
  br i1 %extract91.i, label %preload446.i, label %postload447.i

preload446.i:                                     ; preds = %postload436.i
  %109 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %110 = load i64* %109, align 8
  br label %postload447.i

postload447.i:                                    ; preds = %preload446.i, %postload436.i
  %phi448.i = phi i64 [ undef, %postload436.i ], [ %110, %preload446.i ]
  br i1 %extract92.i, label %preload457.i, label %postload458.i

preload457.i:                                     ; preds = %postload447.i
  %111 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %112 = load i64* %111, align 8
  br label %postload458.i

postload458.i:                                    ; preds = %preload457.i, %postload447.i
  %phi459.i = phi i64 [ undef, %postload447.i ], [ %112, %preload457.i ]
  br i1 %extract93.i, label %preload468.i, label %postload469.i

preload468.i:                                     ; preds = %postload458.i
  %113 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %114 = load i64* %113, align 8
  br label %postload469.i

postload469.i:                                    ; preds = %preload468.i, %postload458.i
  %phi470.i = phi i64 [ undef, %postload458.i ], [ %114, %preload468.i ]
  br i1 %extract94.i, label %preload479.i, label %postload480.i

preload479.i:                                     ; preds = %postload469.i
  %115 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %116 = load i64* %115, align 8
  br label %postload480.i

postload480.i:                                    ; preds = %preload479.i, %postload469.i
  %phi481.i = phi i64 [ undef, %postload469.i ], [ %116, %preload479.i ]
  br i1 %extract95.i, label %preload490.i, label %postload491.i

preload490.i:                                     ; preds = %postload480.i
  %117 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %118 = load i64* %117, align 8
  br label %postload491.i

postload491.i:                                    ; preds = %preload490.i, %postload480.i
  %phi492.i = phi i64 [ undef, %postload480.i ], [ %118, %preload490.i ]
  br i1 %extract96.i, label %preload501.i, label %postload502.i

preload501.i:                                     ; preds = %postload491.i
  %119 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %120 = load i64* %119, align 8
  br label %postload502.i

postload502.i:                                    ; preds = %preload501.i, %postload491.i
  %phi503.i = phi i64 [ undef, %postload491.i ], [ %120, %preload501.i ]
  br i1 %extract97.i, label %preload512.i, label %postload513.i

preload512.i:                                     ; preds = %postload502.i
  %121 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %122 = load i64* %121, align 8
  br label %postload513.i

postload513.i:                                    ; preds = %preload512.i, %postload502.i
  %phi514.i = phi i64 [ undef, %postload502.i ], [ %122, %preload512.i ]
  br i1 %extract98.i, label %preload523.i, label %postload524.i

preload523.i:                                     ; preds = %postload513.i
  %123 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %124 = load i64* %123, align 8
  br label %postload524.i

postload524.i:                                    ; preds = %preload523.i, %postload513.i
  %phi525.i = phi i64 [ undef, %postload513.i ], [ %124, %preload523.i ]
  %temp.vect162.i = insertelement <16 x i64> undef, i64 %phi360.i, i32 0
  %temp.vect163.i = insertelement <16 x i64> %temp.vect162.i, i64 %phi371.i, i32 1
  %temp.vect164.i = insertelement <16 x i64> %temp.vect163.i, i64 %phi382.i, i32 2
  %temp.vect165.i = insertelement <16 x i64> %temp.vect164.i, i64 %phi393.i, i32 3
  %temp.vect166.i = insertelement <16 x i64> %temp.vect165.i, i64 %phi404.i, i32 4
  %temp.vect167.i = insertelement <16 x i64> %temp.vect166.i, i64 %phi415.i, i32 5
  %temp.vect168.i = insertelement <16 x i64> %temp.vect167.i, i64 %phi426.i, i32 6
  %temp.vect169.i = insertelement <16 x i64> %temp.vect168.i, i64 %phi437.i, i32 7
  %temp.vect170.i = insertelement <16 x i64> %temp.vect169.i, i64 %phi448.i, i32 8
  %temp.vect171.i = insertelement <16 x i64> %temp.vect170.i, i64 %phi459.i, i32 9
  %temp.vect172.i = insertelement <16 x i64> %temp.vect171.i, i64 %phi470.i, i32 10
  %temp.vect173.i = insertelement <16 x i64> %temp.vect172.i, i64 %phi481.i, i32 11
  %temp.vect174.i = insertelement <16 x i64> %temp.vect173.i, i64 %phi492.i, i32 12
  %temp.vect175.i = insertelement <16 x i64> %temp.vect174.i, i64 %phi503.i, i32 13
  %temp.vect176.i = insertelement <16 x i64> %temp.vect175.i, i64 %phi514.i, i32 14
  %temp.vect177.i = insertelement <16 x i64> %temp.vect176.i, i64 %phi525.i, i32 15
  %125 = add <16 x i64> %temp.vect177.i, %75
  %126 = add nsw i32 %72, 1
  %ipred.i5.i = bitcast <16 x i1> %loop_mask2180.i to i16
  %val.i6.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i5.i, i16 %ipred.i5.i) nounwind
  %tmp.i7.i = and i32 %val.i6.i, 1
  %res.i8.i = icmp eq i32 %tmp.i7.i, 0
  br i1 %res.i8.i, label %postload351.i, label %.critedge.i

.critedge.i:                                      ; preds = %postload524.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.addUniform_separated_args.exit

thenBB.i:                                         ; preds = %.critedge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 1792
  br label %SyncBB533.i

____Vectorized_.addUniform_separated_args.exit:   ; preds = %.critedge.i
  ret void
}

!opencl.kernels = !{!0, !2}
!opencl_addUniform_locals_anchor = !{!3}
!opencl_scan_locals_anchor = !{!4}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__addUniform_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, int const", metadata !"opencl_addUniform_locals_anchor", void (i8*)* @addUniform}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__scan_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int const, int const, int const", metadata !"opencl_scan_locals_anchor", void (i8*)* @scan}
!3 = metadata !{metadata !"opencl_addUniform_local_uni"}
!4 = metadata !{metadata !"opencl_scan_local_s_data"}
