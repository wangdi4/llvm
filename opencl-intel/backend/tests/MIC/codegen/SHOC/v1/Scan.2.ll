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

@opencl_addUniform_local_uni.0 = internal addrspace(3) global double 0.000000e+00
@opencl_scan_local_s_data = internal addrspace(3) global [512 x double] zeroinitializer, align 16

declare void @__addUniform_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_local_id(i32)

declare i64 @get_group_id(i32)

declare i64 @get_local_size(i32)

declare void @barrier(i64)

declare void @__scan_original(double addrspace(1)* nocapture, double addrspace(1)* nocapture, double addrspace(1)* nocapture, i32, i32, i32) nounwind

declare i64 @get_global_id(i32)

declare fastcc double @__scanLocalMem_original(double) nounwind inlinehint

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64* @get_curr_wi.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

declare fastcc double @__scanLocalMem_New_original(double, i64, i64) nounwind inlinehint

define void @__addUniform_separated_args(double addrspace(1)* nocapture %d_vector, double addrspace(1)* nocapture %d_uniforms, i32 %n, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to double addrspace(3)*
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = icmp eq i64 %3, 0
  br i1 %4, label %5, label %"Barrier BB"

; <label>:5                                       ; preds = %SyncBB
  %6 = load i64* %pWGId, align 8
  %7 = getelementptr inbounds double addrspace(1)* %d_uniforms, i64 %6
  %8 = load double addrspace(1)* %7, align 8
  store double %8, double addrspace(3)* %1, align 8
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %5, %SyncBB
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
  br i1 %check.WI.iter, label %thenBB, label %SyncBB5

thenBB:                                           ; preds = %"Barrier BB"
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 192
  br label %SyncBB

SyncBB5:                                          ; preds = %"Barrier BB", %thenBB8
  %CurrWI..1 = phi i64 [ %"CurrWI++12", %thenBB8 ], [ 0, %"Barrier BB" ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride14", %thenBB8 ], [ 0, %"Barrier BB" ]
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i64*
  %loadedValue = load i64* %CastToValueType4, align 8
  br label %17

; <label>:17                                      ; preds = %22, %SyncBB5
  %address.0.in = phi i64 [ %loadedValue, %SyncBB5 ], [ %30, %22 ]
  %18 = phi i32 [ 0, %SyncBB5 ], [ %31, %22 ]
  %19 = icmp slt i32 %18, 4
  br i1 %19, label %20, label %.critedge

; <label>:20                                      ; preds = %17
  %address.0 = trunc i64 %address.0.in to i32
  %21 = icmp ult i32 %address.0, %n
  br i1 %21, label %22, label %.critedge

; <label>:22                                      ; preds = %20
  %23 = load double addrspace(3)* %1, align 8
  %24 = and i64 %address.0.in, 4294967295
  %25 = getelementptr inbounds double addrspace(1)* %d_vector, i64 %24
  %26 = load double addrspace(1)* %25, align 8
  %27 = fadd double %26, %23
  store double %27, double addrspace(1)* %25, align 8
  %28 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %29 = load i64* %28, align 8
  %30 = add i64 %29, %24
  %31 = add nsw i32 %18, 1
  br label %17

.critedge:                                        ; preds = %20, %17
  %check.WI.iter11 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter11, label %thenBB8, label %SyncBB6

thenBB8:                                          ; preds = %.critedge
  %"CurrWI++12" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride14" = add nuw i64 %CurrSBIndex..1, 192
  br label %SyncBB5

SyncBB6:                                          ; preds = %.critedge
  ret void
}

define void @__scan_separated_args(double addrspace(1)* nocapture %g_odata, double addrspace(1)* nocapture %g_idata, double addrspace(1)* nocapture %g_blockSums, i32 %n, i32 %fullBlock, i32 %storeSum, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  store i64 0, i64* %pCurrWI, align 8
  %0 = bitcast double addrspace(1)* %g_idata to <4 x double> addrspace(1)*
  %1 = icmp eq i32 %fullBlock, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB144, %FirstBB
  %CurrSBIndex..1 = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride150", %thenBB144 ]
  %currWI151 = load i64* %pCurrWI, align 8
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI151, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %7 = trunc i64 %6 to i32
  %"&(pSB[currWI].offset)2" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %7, i32* %CastToValueType, align 4
  %currWI = load i64* %pCurrWI, align 8
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %currWI, i32 0, i64 0
  %9 = load i64* %8, align 8
  %"&(pSB[currWI].offset)143" = or i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset15" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)143"
  %CastToValueType16 = bitcast i8* %"&pSB[currWI].offset15" to i64*
  store i64 %9, i64* %CastToValueType16, align 8
  %10 = shl i32 %7, 2
  %"&(pSB[currWI].offset)234" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset24" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)234"
  %CastToValueType25 = bitcast i8* %"&pSB[currWI].offset24" to i32*
  store i32 %10, i32* %CastToValueType25, align 4
  br i1 %1, label %11, label %14

; <label>:11                                      ; preds = %SyncBB
  %"&(pSB[currWI].offset)5218" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset53" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5218"
  %CastToValueType54 = bitcast i8* %"&pSB[currWI].offset53" to i32*
  %loadedValue55 = load i32* %CastToValueType54, align 4
  %12 = or i32 %loadedValue55, 3
  %13 = icmp slt i32 %12, %n
  br i1 %13, label %14, label %28

; <label>:14                                      ; preds = %11, %SyncBB
  %"&(pSB[currWI].offset)55" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset6" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)55"
  %CastToValueType7 = bitcast i8* %"&pSB[currWI].offset6" to i32*
  %loadedValue = load i32* %CastToValueType7, align 4
  %15 = sext i32 %loadedValue to i64
  %16 = getelementptr inbounds <4 x double> addrspace(1)* %0, i64 %15
  %17 = load <4 x double> addrspace(1)* %16, align 32
  %18 = extractelement <4 x double> %17, i32 1
  %19 = extractelement <4 x double> %17, i32 0
  %20 = fadd double %18, %19
  %21 = insertelement <4 x double> %17, double %20, i32 1
  %22 = extractelement <4 x double> %17, i32 2
  %23 = fadd double %22, %20
  %24 = insertelement <4 x double> %21, double %23, i32 2
  %25 = extractelement <4 x double> %17, i32 3
  %26 = fadd double %25, %23
  %27 = insertelement <4 x double> %24, double %26, i32 3
  br label %"Barrier BB"

; <label>:28                                      ; preds = %11
  %"&(pSB[currWI].offset)7219" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset73" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)7219"
  %CastToValueType74 = bitcast i8* %"&pSB[currWI].offset73" to i32*
  %loadedValue75 = load i32* %CastToValueType74, align 4
  %29 = icmp slt i32 %loadedValue75, %n
  br i1 %29, label %30, label %34

; <label>:30                                      ; preds = %28
  %"&(pSB[currWI].offset)6722" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset68" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)6722"
  %CastToValueType69 = bitcast i8* %"&pSB[currWI].offset68" to i32*
  %loadedValue70 = load i32* %CastToValueType69, align 4
  %31 = sext i32 %loadedValue70 to i64
  %32 = getelementptr inbounds double addrspace(1)* %g_idata, i64 %31
  %33 = load double addrspace(1)* %32, align 8
  br label %34

; <label>:34                                      ; preds = %30, %28
  %35 = phi double [ %33, %30 ], [ 0.000000e+00, %28 ]
  %36 = insertelement <4 x double> undef, double %35, i32 0
  %"&(pSB[currWI].offset)4720" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset48" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)4720"
  %CastToValueType49 = bitcast i8* %"&pSB[currWI].offset48" to i32*
  %loadedValue50 = load i32* %CastToValueType49, align 4
  %37 = or i32 %loadedValue50, 1
  %38 = icmp slt i32 %37, %n
  br i1 %38, label %39, label %43

; <label>:39                                      ; preds = %34
  %40 = sext i32 %37 to i64
  %41 = getelementptr inbounds double addrspace(1)* %g_idata, i64 %40
  %42 = load double addrspace(1)* %41, align 8
  br label %43

; <label>:43                                      ; preds = %39, %34
  %44 = phi double [ %42, %39 ], [ 0.000000e+00, %34 ]
  %45 = fadd double %44, %35
  %46 = insertelement <4 x double> %36, double %45, i32 1
  %"&(pSB[currWI].offset)4221" = or i64 %CurrSBIndex..1, 24
  %"&pSB[currWI].offset43" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)4221"
  %CastToValueType44 = bitcast i8* %"&pSB[currWI].offset43" to i32*
  %loadedValue45 = load i32* %CastToValueType44, align 4
  %47 = or i32 %loadedValue45, 2
  %48 = icmp slt i32 %47, %n
  br i1 %48, label %49, label %53

; <label>:49                                      ; preds = %43
  %50 = sext i32 %47 to i64
  %51 = getelementptr inbounds double addrspace(1)* %g_idata, i64 %50
  %52 = load double addrspace(1)* %51, align 8
  br label %53

; <label>:53                                      ; preds = %49, %43
  %54 = phi double [ %52, %49 ], [ 0.000000e+00, %43 ]
  %55 = fadd double %54, %45
  %56 = insertelement <4 x double> %46, double %55, i32 2
  %57 = fadd double %55, 0.000000e+00
  %58 = insertelement <4 x double> %56, double %57, i32 3
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %53, %14
  %threadScanT.0 = phi <4 x double> [ %27, %14 ], [ %58, %53 ]
  %res.0 = phi double [ %26, %14 ], [ %57, %53 ]
  %"&(pSB[currWI].offset)101" = add nuw i64 %CurrSBIndex..1, 64
  %"&pSB[currWI].offset102" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)101"
  %CastToValueType103 = bitcast i8* %"&pSB[currWI].offset102" to double*
  store double %res.0, double* %CastToValueType103, align 8
  %"&(pSB[currWI].offset)776" = or i64 %CurrSBIndex..1, 32
  %"&pSB[currWI].offset78" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)776"
  %CastToValueType79 = bitcast i8* %"&pSB[currWI].offset78" to <4 x double>*
  store <4 x double> %threadScanT.0, <4 x double>* %CastToValueType79, align 32
  %loadedCurrWI146 = load i64* %pCurrWI, align 8
  %check.WI.iter147 = icmp ult i64 %loadedCurrWI146, %iterCount
  br i1 %check.WI.iter147, label %thenBB144, label %elseBB145

thenBB144:                                        ; preds = %"Barrier BB"
  %"CurrWI++148" = add nuw i64 %loadedCurrWI146, 1
  store i64 %"CurrWI++148", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride150" = add nuw i64 %CurrSBIndex..1, 192
  br label %SyncBB

elseBB145:                                        ; preds = %"Barrier BB"
  store i64 0, i64* %pCurrWI, align 8
  %59 = bitcast i8 addrspace(3)* %pLocalMem to [512 x double] addrspace(3)*
  br label %SyncBB239.i

SyncBB239.i:                                      ; preds = %thenBB.i, %elseBB145
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %elseBB145 ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %elseBB145 ]
  %60 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0.i, i32 0, i64 0
  %61 = load i64* %60, align 8
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 80
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %61, i64* %CastToValueType.i, align 8
  %sext.i = shl i64 %61, 32
  %62 = ashr i64 %sext.i, 32
  %63 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %62
  store double 0.000000e+00, double addrspace(3)* %63, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %iterCount
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %SyncBB239.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 192
  br label %SyncBB239.i

SyncBB.i:                                         ; preds = %thenBB252.i, %SyncBB239.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++256.i", %thenBB252.i ], [ 0, %SyncBB239.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride258.i", %thenBB252.i ], [ 0, %SyncBB239.i ]
  %64 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %65 = load i64* %64, align 8
  %"&(pSB[currWI].offset)20.i" = add nuw i64 %CurrSBIndex..1.i, 80
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)20.i"
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to i64*
  %loadedValue.i = load i64* %CastToValueType22.i, align 8
  %66 = add i64 %65, %loadedValue.i
  %67 = trunc i64 %66 to i32
  %"&(pSB[currWI].offset)24.i" = add nuw i64 %CurrSBIndex..1.i, 88
  %"&pSB[currWI].offset25.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)24.i"
  %CastToValueType26.i = bitcast i8* %"&pSB[currWI].offset25.i" to i32*
  store i32 %67, i32* %CastToValueType26.i, align 4
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %68
  %"&(pSB[currWI].offset)68.i" = add nuw i64 %CurrSBIndex..1.i, 96
  %"&pSB[currWI].offset69.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)68.i"
  %CastToValueType70.i = bitcast i8* %"&pSB[currWI].offset69.i" to double addrspace(3)**
  store double addrspace(3)* %69, double addrspace(3)** %CastToValueType70.i, align 8
  %"&(pSB[currWI].offset)4.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset5.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)4.i"
  %CastToArgType.i = bitcast i8* %"&pSB[currWI].offset5.i" to double*
  %loadedValue6.i = load double* %CastToArgType.i, align 8
  store double %loadedValue6.i, double addrspace(3)* %69, align 8
  %check.WI.iter255.i = icmp ult i64 %CurrWI..1.i, %iterCount
  br i1 %check.WI.iter255.i, label %thenBB252.i, label %SyncBB232.i

thenBB252.i:                                      ; preds = %SyncBB.i
  %"CurrWI++256.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride258.i" = add nuw i64 %CurrSBIndex..1.i, 192
  br label %SyncBB.i

SyncBB232.i:                                      ; preds = %thenBB259.i, %SyncBB.i
  %CurrWI..2.i = phi i64 [ %"CurrWI++263.i", %thenBB259.i ], [ 0, %SyncBB.i ]
  %CurrSBIndex..2.i = phi i64 [ %"loadedCurrSB+Stride265.i", %thenBB259.i ], [ 0, %SyncBB.i ]
  %"&(pSB[currWI].offset)63.i" = add nuw i64 %CurrSBIndex..2.i, 88
  %"&pSB[currWI].offset64.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)63.i"
  %CastToValueType65.i = bitcast i8* %"&pSB[currWI].offset64.i" to i32*
  %loadedValue66.i = load i32* %CastToValueType65.i, align 4
  %70 = add nsw i32 %loadedValue66.i, -1
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %71
  %"&(pSB[currWI].offset)152.i" = add nuw i64 %CurrSBIndex..2.i, 104
  %"&pSB[currWI].offset153.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)152.i"
  %CastToValueType154.i = bitcast i8* %"&pSB[currWI].offset153.i" to double addrspace(3)**
  store double addrspace(3)* %72, double addrspace(3)** %CastToValueType154.i, align 8
  %73 = load double addrspace(3)* %72, align 8
  %"&(pSB[currWI].offset)161.i" = add nuw i64 %CurrSBIndex..2.i, 112
  %"&pSB[currWI].offset162.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)161.i"
  %CastToValueType163.i = bitcast i8* %"&pSB[currWI].offset162.i" to double*
  store double %73, double* %CastToValueType163.i, align 8
  %check.WI.iter262.i = icmp ult i64 %CurrWI..2.i, %iterCount
  br i1 %check.WI.iter262.i, label %thenBB259.i, label %SyncBB233.i

thenBB259.i:                                      ; preds = %SyncBB232.i
  %"CurrWI++263.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride265.i" = add nuw i64 %CurrSBIndex..2.i, 192
  br label %SyncBB232.i

SyncBB233.i:                                      ; preds = %thenBB266.i, %SyncBB232.i
  %CurrWI..3.i = phi i64 [ %"CurrWI++270.i", %thenBB266.i ], [ 0, %SyncBB232.i ]
  %CurrSBIndex..3.i = phi i64 [ %"loadedCurrSB+Stride272.i", %thenBB266.i ], [ 0, %SyncBB232.i ]
  %"&(pSB[currWI].offset)147.i" = add nuw i64 %CurrSBIndex..3.i, 96
  %"&pSB[currWI].offset148.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)147.i"
  %CastToValueType149.i = bitcast i8* %"&pSB[currWI].offset148.i" to double addrspace(3)**
  %loadedValue150.i = load double addrspace(3)** %CastToValueType149.i, align 8
  %74 = load double addrspace(3)* %loadedValue150.i, align 8
  %"&(pSB[currWI].offset)165.i" = add nuw i64 %CurrSBIndex..3.i, 112
  %"&pSB[currWI].offset166.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)165.i"
  %CastToValueType167.i = bitcast i8* %"&pSB[currWI].offset166.i" to double*
  %loadedValue168.i = load double* %CastToValueType167.i, align 8
  %75 = fadd double %74, %loadedValue168.i
  %"&(pSB[currWI].offset)142.i" = add nuw i64 %CurrSBIndex..3.i, 96
  %"&pSB[currWI].offset143.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)142.i"
  %CastToValueType144.i = bitcast i8* %"&pSB[currWI].offset143.i" to double addrspace(3)**
  %loadedValue145.i = load double addrspace(3)** %CastToValueType144.i, align 8
  store double %75, double addrspace(3)* %loadedValue145.i, align 8
  %check.WI.iter269.i = icmp ult i64 %CurrWI..3.i, %iterCount
  br i1 %check.WI.iter269.i, label %thenBB266.i, label %SyncBB234.i

thenBB266.i:                                      ; preds = %SyncBB233.i
  %"CurrWI++270.i" = add nuw i64 %CurrWI..3.i, 1
  %"loadedCurrSB+Stride272.i" = add nuw i64 %CurrSBIndex..3.i, 192
  br label %SyncBB233.i

SyncBB234.i:                                      ; preds = %thenBB273.i, %SyncBB233.i
  %CurrWI..4.i = phi i64 [ %"CurrWI++277.i", %thenBB273.i ], [ 0, %SyncBB233.i ]
  %CurrSBIndex..4.i = phi i64 [ %"loadedCurrSB+Stride279.i", %thenBB273.i ], [ 0, %SyncBB233.i ]
  %"&(pSB[currWI].offset)58.i" = add nuw i64 %CurrSBIndex..4.i, 88
  %"&pSB[currWI].offset59.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)58.i"
  %CastToValueType60.i = bitcast i8* %"&pSB[currWI].offset59.i" to i32*
  %loadedValue61.i = load i32* %CastToValueType60.i, align 4
  %76 = add nsw i32 %loadedValue61.i, -2
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %77
  %79 = load double addrspace(3)* %78, align 8
  %"&(pSB[currWI].offset)170.i" = add nuw i64 %CurrSBIndex..4.i, 120
  %"&pSB[currWI].offset171.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)170.i"
  %CastToValueType172.i = bitcast i8* %"&pSB[currWI].offset171.i" to double*
  store double %79, double* %CastToValueType172.i, align 8
  %check.WI.iter276.i = icmp ult i64 %CurrWI..4.i, %iterCount
  br i1 %check.WI.iter276.i, label %thenBB273.i, label %SyncBB235.i

thenBB273.i:                                      ; preds = %SyncBB234.i
  %"CurrWI++277.i" = add nuw i64 %CurrWI..4.i, 1
  %"loadedCurrSB+Stride279.i" = add nuw i64 %CurrSBIndex..4.i, 192
  br label %SyncBB234.i

SyncBB235.i:                                      ; preds = %thenBB280.i, %SyncBB234.i
  %CurrWI..5.i = phi i64 [ %"CurrWI++284.i", %thenBB280.i ], [ 0, %SyncBB234.i ]
  %CurrSBIndex..5.i = phi i64 [ %"loadedCurrSB+Stride286.i", %thenBB280.i ], [ 0, %SyncBB234.i ]
  %"&(pSB[currWI].offset)137.i" = add nuw i64 %CurrSBIndex..5.i, 96
  %"&pSB[currWI].offset138.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)137.i"
  %CastToValueType139.i = bitcast i8* %"&pSB[currWI].offset138.i" to double addrspace(3)**
  %loadedValue140.i = load double addrspace(3)** %CastToValueType139.i, align 8
  %80 = load double addrspace(3)* %loadedValue140.i, align 8
  %"&(pSB[currWI].offset)174.i" = add nuw i64 %CurrSBIndex..5.i, 120
  %"&pSB[currWI].offset175.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)174.i"
  %CastToValueType176.i = bitcast i8* %"&pSB[currWI].offset175.i" to double*
  %loadedValue177.i = load double* %CastToValueType176.i, align 8
  %81 = fadd double %80, %loadedValue177.i
  %"&(pSB[currWI].offset)132.i" = add nuw i64 %CurrSBIndex..5.i, 96
  %"&pSB[currWI].offset133.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)132.i"
  %CastToValueType134.i = bitcast i8* %"&pSB[currWI].offset133.i" to double addrspace(3)**
  %loadedValue135.i = load double addrspace(3)** %CastToValueType134.i, align 8
  store double %81, double addrspace(3)* %loadedValue135.i, align 8
  %check.WI.iter283.i = icmp ult i64 %CurrWI..5.i, %iterCount
  br i1 %check.WI.iter283.i, label %thenBB280.i, label %SyncBB236.i

thenBB280.i:                                      ; preds = %SyncBB235.i
  %"CurrWI++284.i" = add nuw i64 %CurrWI..5.i, 1
  %"loadedCurrSB+Stride286.i" = add nuw i64 %CurrSBIndex..5.i, 192
  br label %SyncBB235.i

SyncBB236.i:                                      ; preds = %thenBB287.i, %SyncBB235.i
  %CurrWI..6.i = phi i64 [ %"CurrWI++291.i", %thenBB287.i ], [ 0, %SyncBB235.i ]
  %CurrSBIndex..6.i = phi i64 [ %"loadedCurrSB+Stride293.i", %thenBB287.i ], [ 0, %SyncBB235.i ]
  %"&(pSB[currWI].offset)53.i" = add nuw i64 %CurrSBIndex..6.i, 88
  %"&pSB[currWI].offset54.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)53.i"
  %CastToValueType55.i = bitcast i8* %"&pSB[currWI].offset54.i" to i32*
  %loadedValue56.i = load i32* %CastToValueType55.i, align 4
  %82 = add nsw i32 %loadedValue56.i, -4
  %83 = sext i32 %82 to i64
  %84 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %83
  %85 = load double addrspace(3)* %84, align 8
  %"&(pSB[currWI].offset)179.i" = add nuw i64 %CurrSBIndex..6.i, 128
  %"&pSB[currWI].offset180.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)179.i"
  %CastToValueType181.i = bitcast i8* %"&pSB[currWI].offset180.i" to double*
  store double %85, double* %CastToValueType181.i, align 8
  %check.WI.iter290.i = icmp ult i64 %CurrWI..6.i, %iterCount
  br i1 %check.WI.iter290.i, label %thenBB287.i, label %SyncBB237.i

thenBB287.i:                                      ; preds = %SyncBB236.i
  %"CurrWI++291.i" = add nuw i64 %CurrWI..6.i, 1
  %"loadedCurrSB+Stride293.i" = add nuw i64 %CurrSBIndex..6.i, 192
  br label %SyncBB236.i

SyncBB237.i:                                      ; preds = %thenBB294.i, %SyncBB236.i
  %CurrWI..7.i = phi i64 [ %"CurrWI++298.i", %thenBB294.i ], [ 0, %SyncBB236.i ]
  %CurrSBIndex..7.i = phi i64 [ %"loadedCurrSB+Stride300.i", %thenBB294.i ], [ 0, %SyncBB236.i ]
  %"&(pSB[currWI].offset)127.i" = add nuw i64 %CurrSBIndex..7.i, 96
  %"&pSB[currWI].offset128.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)127.i"
  %CastToValueType129.i = bitcast i8* %"&pSB[currWI].offset128.i" to double addrspace(3)**
  %loadedValue130.i = load double addrspace(3)** %CastToValueType129.i, align 8
  %86 = load double addrspace(3)* %loadedValue130.i, align 8
  %"&(pSB[currWI].offset)183.i" = add nuw i64 %CurrSBIndex..7.i, 128
  %"&pSB[currWI].offset184.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)183.i"
  %CastToValueType185.i = bitcast i8* %"&pSB[currWI].offset184.i" to double*
  %loadedValue186.i = load double* %CastToValueType185.i, align 8
  %87 = fadd double %86, %loadedValue186.i
  %"&(pSB[currWI].offset)122.i" = add nuw i64 %CurrSBIndex..7.i, 96
  %"&pSB[currWI].offset123.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)122.i"
  %CastToValueType124.i = bitcast i8* %"&pSB[currWI].offset123.i" to double addrspace(3)**
  %loadedValue125.i = load double addrspace(3)** %CastToValueType124.i, align 8
  store double %87, double addrspace(3)* %loadedValue125.i, align 8
  %check.WI.iter297.i = icmp ult i64 %CurrWI..7.i, %iterCount
  br i1 %check.WI.iter297.i, label %thenBB294.i, label %SyncBB238.i

thenBB294.i:                                      ; preds = %SyncBB237.i
  %"CurrWI++298.i" = add nuw i64 %CurrWI..7.i, 1
  %"loadedCurrSB+Stride300.i" = add nuw i64 %CurrSBIndex..7.i, 192
  br label %SyncBB237.i

SyncBB238.i:                                      ; preds = %thenBB301.i, %SyncBB237.i
  %CurrWI..8.i = phi i64 [ %"CurrWI++305.i", %thenBB301.i ], [ 0, %SyncBB237.i ]
  %CurrSBIndex..8.i = phi i64 [ %"loadedCurrSB+Stride307.i", %thenBB301.i ], [ 0, %SyncBB237.i ]
  %"&(pSB[currWI].offset)48.i" = add nuw i64 %CurrSBIndex..8.i, 88
  %"&pSB[currWI].offset49.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)48.i"
  %CastToValueType50.i = bitcast i8* %"&pSB[currWI].offset49.i" to i32*
  %loadedValue51.i = load i32* %CastToValueType50.i, align 4
  %88 = add nsw i32 %loadedValue51.i, -8
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %89
  %91 = load double addrspace(3)* %90, align 8
  %"&(pSB[currWI].offset)188.i" = add nuw i64 %CurrSBIndex..8.i, 136
  %"&pSB[currWI].offset189.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)188.i"
  %CastToValueType190.i = bitcast i8* %"&pSB[currWI].offset189.i" to double*
  store double %91, double* %CastToValueType190.i, align 8
  %check.WI.iter304.i = icmp ult i64 %CurrWI..8.i, %iterCount
  br i1 %check.WI.iter304.i, label %thenBB301.i, label %SyncBB240.i

thenBB301.i:                                      ; preds = %SyncBB238.i
  %"CurrWI++305.i" = add nuw i64 %CurrWI..8.i, 1
  %"loadedCurrSB+Stride307.i" = add nuw i64 %CurrSBIndex..8.i, 192
  br label %SyncBB238.i

SyncBB240.i:                                      ; preds = %thenBB308.i, %SyncBB238.i
  %CurrWI..9.i = phi i64 [ %"CurrWI++312.i", %thenBB308.i ], [ 0, %SyncBB238.i ]
  %CurrSBIndex..9.i = phi i64 [ %"loadedCurrSB+Stride314.i", %thenBB308.i ], [ 0, %SyncBB238.i ]
  %"&(pSB[currWI].offset)117.i" = add nuw i64 %CurrSBIndex..9.i, 96
  %"&pSB[currWI].offset118.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)117.i"
  %CastToValueType119.i = bitcast i8* %"&pSB[currWI].offset118.i" to double addrspace(3)**
  %loadedValue120.i = load double addrspace(3)** %CastToValueType119.i, align 8
  %92 = load double addrspace(3)* %loadedValue120.i, align 8
  %"&(pSB[currWI].offset)192.i" = add nuw i64 %CurrSBIndex..9.i, 136
  %"&pSB[currWI].offset193.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)192.i"
  %CastToValueType194.i = bitcast i8* %"&pSB[currWI].offset193.i" to double*
  %loadedValue195.i = load double* %CastToValueType194.i, align 8
  %93 = fadd double %92, %loadedValue195.i
  %"&(pSB[currWI].offset)112.i" = add nuw i64 %CurrSBIndex..9.i, 96
  %"&pSB[currWI].offset113.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)112.i"
  %CastToValueType114.i = bitcast i8* %"&pSB[currWI].offset113.i" to double addrspace(3)**
  %loadedValue115.i = load double addrspace(3)** %CastToValueType114.i, align 8
  store double %93, double addrspace(3)* %loadedValue115.i, align 8
  %check.WI.iter311.i = icmp ult i64 %CurrWI..9.i, %iterCount
  br i1 %check.WI.iter311.i, label %thenBB308.i, label %SyncBB241.i

thenBB308.i:                                      ; preds = %SyncBB240.i
  %"CurrWI++312.i" = add nuw i64 %CurrWI..9.i, 1
  %"loadedCurrSB+Stride314.i" = add nuw i64 %CurrSBIndex..9.i, 192
  br label %SyncBB240.i

SyncBB241.i:                                      ; preds = %thenBB315.i, %SyncBB240.i
  %CurrWI..10.i = phi i64 [ %"CurrWI++319.i", %thenBB315.i ], [ 0, %SyncBB240.i ]
  %CurrSBIndex..10.i = phi i64 [ %"loadedCurrSB+Stride321.i", %thenBB315.i ], [ 0, %SyncBB240.i ]
  %"&(pSB[currWI].offset)43.i" = add nuw i64 %CurrSBIndex..10.i, 88
  %"&pSB[currWI].offset44.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)43.i"
  %CastToValueType45.i = bitcast i8* %"&pSB[currWI].offset44.i" to i32*
  %loadedValue46.i = load i32* %CastToValueType45.i, align 4
  %94 = add nsw i32 %loadedValue46.i, -16
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %95
  %97 = load double addrspace(3)* %96, align 8
  %"&(pSB[currWI].offset)197.i" = add nuw i64 %CurrSBIndex..10.i, 144
  %"&pSB[currWI].offset198.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)197.i"
  %CastToValueType199.i = bitcast i8* %"&pSB[currWI].offset198.i" to double*
  store double %97, double* %CastToValueType199.i, align 8
  %check.WI.iter318.i = icmp ult i64 %CurrWI..10.i, %iterCount
  br i1 %check.WI.iter318.i, label %thenBB315.i, label %SyncBB242.i

thenBB315.i:                                      ; preds = %SyncBB241.i
  %"CurrWI++319.i" = add nuw i64 %CurrWI..10.i, 1
  %"loadedCurrSB+Stride321.i" = add nuw i64 %CurrSBIndex..10.i, 192
  br label %SyncBB241.i

SyncBB242.i:                                      ; preds = %thenBB322.i, %SyncBB241.i
  %CurrWI..11.i = phi i64 [ %"CurrWI++326.i", %thenBB322.i ], [ 0, %SyncBB241.i ]
  %CurrSBIndex..11.i = phi i64 [ %"loadedCurrSB+Stride328.i", %thenBB322.i ], [ 0, %SyncBB241.i ]
  %"&(pSB[currWI].offset)107.i" = add nuw i64 %CurrSBIndex..11.i, 96
  %"&pSB[currWI].offset108.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)107.i"
  %CastToValueType109.i = bitcast i8* %"&pSB[currWI].offset108.i" to double addrspace(3)**
  %loadedValue110.i = load double addrspace(3)** %CastToValueType109.i, align 8
  %98 = load double addrspace(3)* %loadedValue110.i, align 8
  %"&(pSB[currWI].offset)201.i" = add nuw i64 %CurrSBIndex..11.i, 144
  %"&pSB[currWI].offset202.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)201.i"
  %CastToValueType203.i = bitcast i8* %"&pSB[currWI].offset202.i" to double*
  %loadedValue204.i = load double* %CastToValueType203.i, align 8
  %99 = fadd double %98, %loadedValue204.i
  %"&(pSB[currWI].offset)102.i" = add nuw i64 %CurrSBIndex..11.i, 96
  %"&pSB[currWI].offset103.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)102.i"
  %CastToValueType104.i = bitcast i8* %"&pSB[currWI].offset103.i" to double addrspace(3)**
  %loadedValue105.i = load double addrspace(3)** %CastToValueType104.i, align 8
  store double %99, double addrspace(3)* %loadedValue105.i, align 8
  %check.WI.iter325.i = icmp ult i64 %CurrWI..11.i, %iterCount
  br i1 %check.WI.iter325.i, label %thenBB322.i, label %SyncBB243.i

thenBB322.i:                                      ; preds = %SyncBB242.i
  %"CurrWI++326.i" = add nuw i64 %CurrWI..11.i, 1
  %"loadedCurrSB+Stride328.i" = add nuw i64 %CurrSBIndex..11.i, 192
  br label %SyncBB242.i

SyncBB243.i:                                      ; preds = %thenBB329.i, %SyncBB242.i
  %CurrWI..12.i = phi i64 [ %"CurrWI++333.i", %thenBB329.i ], [ 0, %SyncBB242.i ]
  %CurrSBIndex..12.i = phi i64 [ %"loadedCurrSB+Stride335.i", %thenBB329.i ], [ 0, %SyncBB242.i ]
  %"&(pSB[currWI].offset)38.i" = add nuw i64 %CurrSBIndex..12.i, 88
  %"&pSB[currWI].offset39.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)38.i"
  %CastToValueType40.i = bitcast i8* %"&pSB[currWI].offset39.i" to i32*
  %loadedValue41.i = load i32* %CastToValueType40.i, align 4
  %100 = add nsw i32 %loadedValue41.i, -32
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %101
  %103 = load double addrspace(3)* %102, align 8
  %"&(pSB[currWI].offset)206.i" = add nuw i64 %CurrSBIndex..12.i, 152
  %"&pSB[currWI].offset207.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)206.i"
  %CastToValueType208.i = bitcast i8* %"&pSB[currWI].offset207.i" to double*
  store double %103, double* %CastToValueType208.i, align 8
  %check.WI.iter332.i = icmp ult i64 %CurrWI..12.i, %iterCount
  br i1 %check.WI.iter332.i, label %thenBB329.i, label %SyncBB244.i

thenBB329.i:                                      ; preds = %SyncBB243.i
  %"CurrWI++333.i" = add nuw i64 %CurrWI..12.i, 1
  %"loadedCurrSB+Stride335.i" = add nuw i64 %CurrSBIndex..12.i, 192
  br label %SyncBB243.i

SyncBB244.i:                                      ; preds = %thenBB336.i, %SyncBB243.i
  %CurrWI..13.i = phi i64 [ %"CurrWI++340.i", %thenBB336.i ], [ 0, %SyncBB243.i ]
  %CurrSBIndex..13.i = phi i64 [ %"loadedCurrSB+Stride342.i", %thenBB336.i ], [ 0, %SyncBB243.i ]
  %"&(pSB[currWI].offset)97.i" = add nuw i64 %CurrSBIndex..13.i, 96
  %"&pSB[currWI].offset98.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)97.i"
  %CastToValueType99.i = bitcast i8* %"&pSB[currWI].offset98.i" to double addrspace(3)**
  %loadedValue100.i = load double addrspace(3)** %CastToValueType99.i, align 8
  %104 = load double addrspace(3)* %loadedValue100.i, align 8
  %"&(pSB[currWI].offset)210.i" = add nuw i64 %CurrSBIndex..13.i, 152
  %"&pSB[currWI].offset211.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)210.i"
  %CastToValueType212.i = bitcast i8* %"&pSB[currWI].offset211.i" to double*
  %loadedValue213.i = load double* %CastToValueType212.i, align 8
  %105 = fadd double %104, %loadedValue213.i
  %"&(pSB[currWI].offset)92.i" = add nuw i64 %CurrSBIndex..13.i, 96
  %"&pSB[currWI].offset93.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)92.i"
  %CastToValueType94.i = bitcast i8* %"&pSB[currWI].offset93.i" to double addrspace(3)**
  %loadedValue95.i = load double addrspace(3)** %CastToValueType94.i, align 8
  store double %105, double addrspace(3)* %loadedValue95.i, align 8
  %check.WI.iter339.i = icmp ult i64 %CurrWI..13.i, %iterCount
  br i1 %check.WI.iter339.i, label %thenBB336.i, label %SyncBB245.i

thenBB336.i:                                      ; preds = %SyncBB244.i
  %"CurrWI++340.i" = add nuw i64 %CurrWI..13.i, 1
  %"loadedCurrSB+Stride342.i" = add nuw i64 %CurrSBIndex..13.i, 192
  br label %SyncBB244.i

SyncBB245.i:                                      ; preds = %thenBB343.i, %SyncBB244.i
  %CurrWI..14.i = phi i64 [ %"CurrWI++347.i", %thenBB343.i ], [ 0, %SyncBB244.i ]
  %CurrSBIndex..14.i = phi i64 [ %"loadedCurrSB+Stride349.i", %thenBB343.i ], [ 0, %SyncBB244.i ]
  %"&(pSB[currWI].offset)33.i" = add nuw i64 %CurrSBIndex..14.i, 88
  %"&pSB[currWI].offset34.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)33.i"
  %CastToValueType35.i = bitcast i8* %"&pSB[currWI].offset34.i" to i32*
  %loadedValue36.i = load i32* %CastToValueType35.i, align 4
  %106 = add nsw i32 %loadedValue36.i, -64
  %107 = sext i32 %106 to i64
  %108 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %107
  %109 = load double addrspace(3)* %108, align 8
  %"&(pSB[currWI].offset)215.i" = add nuw i64 %CurrSBIndex..14.i, 160
  %"&pSB[currWI].offset216.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)215.i"
  %CastToValueType217.i = bitcast i8* %"&pSB[currWI].offset216.i" to double*
  store double %109, double* %CastToValueType217.i, align 8
  %check.WI.iter346.i = icmp ult i64 %CurrWI..14.i, %iterCount
  br i1 %check.WI.iter346.i, label %thenBB343.i, label %SyncBB246.i

thenBB343.i:                                      ; preds = %SyncBB245.i
  %"CurrWI++347.i" = add nuw i64 %CurrWI..14.i, 1
  %"loadedCurrSB+Stride349.i" = add nuw i64 %CurrSBIndex..14.i, 192
  br label %SyncBB245.i

SyncBB246.i:                                      ; preds = %thenBB350.i, %SyncBB245.i
  %CurrWI..15.i = phi i64 [ %"CurrWI++354.i", %thenBB350.i ], [ 0, %SyncBB245.i ]
  %CurrSBIndex..15.i = phi i64 [ %"loadedCurrSB+Stride356.i", %thenBB350.i ], [ 0, %SyncBB245.i ]
  %"&(pSB[currWI].offset)87.i" = add nuw i64 %CurrSBIndex..15.i, 96
  %"&pSB[currWI].offset88.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)87.i"
  %CastToValueType89.i = bitcast i8* %"&pSB[currWI].offset88.i" to double addrspace(3)**
  %loadedValue90.i = load double addrspace(3)** %CastToValueType89.i, align 8
  %110 = load double addrspace(3)* %loadedValue90.i, align 8
  %"&(pSB[currWI].offset)219.i" = add nuw i64 %CurrSBIndex..15.i, 160
  %"&pSB[currWI].offset220.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)219.i"
  %CastToValueType221.i = bitcast i8* %"&pSB[currWI].offset220.i" to double*
  %loadedValue222.i = load double* %CastToValueType221.i, align 8
  %111 = fadd double %110, %loadedValue222.i
  %"&(pSB[currWI].offset)82.i" = add nuw i64 %CurrSBIndex..15.i, 96
  %"&pSB[currWI].offset83.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)82.i"
  %CastToValueType84.i = bitcast i8* %"&pSB[currWI].offset83.i" to double addrspace(3)**
  %loadedValue85.i = load double addrspace(3)** %CastToValueType84.i, align 8
  store double %111, double addrspace(3)* %loadedValue85.i, align 8
  %check.WI.iter353.i = icmp ult i64 %CurrWI..15.i, %iterCount
  br i1 %check.WI.iter353.i, label %thenBB350.i, label %SyncBB247.i

thenBB350.i:                                      ; preds = %SyncBB246.i
  %"CurrWI++354.i" = add nuw i64 %CurrWI..15.i, 1
  %"loadedCurrSB+Stride356.i" = add nuw i64 %CurrSBIndex..15.i, 192
  br label %SyncBB246.i

SyncBB247.i:                                      ; preds = %thenBB364.i, %SyncBB246.i
  %CurrWI..16.i = phi i64 [ %"CurrWI++368.i", %thenBB364.i ], [ 0, %SyncBB246.i ]
  %CurrSBIndex..16.i = phi i64 [ %"loadedCurrSB+Stride370.i", %thenBB364.i ], [ 0, %SyncBB246.i ]
  %"&(pSB[currWI].offset)28.i" = add nuw i64 %CurrSBIndex..16.i, 88
  %"&pSB[currWI].offset29.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)28.i"
  %CastToValueType30.i = bitcast i8* %"&pSB[currWI].offset29.i" to i32*
  %loadedValue31.i = load i32* %CastToValueType30.i, align 4
  %112 = add nsw i32 %loadedValue31.i, -128
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds [512 x double] addrspace(3)* %59, i64 0, i64 %113
  %115 = load double addrspace(3)* %114, align 8
  %"&(pSB[currWI].offset)224.i" = add nuw i64 %CurrSBIndex..16.i, 168
  %"&pSB[currWI].offset225.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)224.i"
  %CastToValueType226.i = bitcast i8* %"&pSB[currWI].offset225.i" to double*
  store double %115, double* %CastToValueType226.i, align 8
  %check.WI.iter367.i = icmp ult i64 %CurrWI..16.i, %iterCount
  br i1 %check.WI.iter367.i, label %thenBB364.i, label %SyncBB249.i

thenBB364.i:                                      ; preds = %SyncBB247.i
  %"CurrWI++368.i" = add nuw i64 %CurrWI..16.i, 1
  %"loadedCurrSB+Stride370.i" = add nuw i64 %CurrSBIndex..16.i, 192
  br label %SyncBB247.i

SyncBB249.i:                                      ; preds = %thenBB371.i, %SyncBB247.i
  %CurrWI..17.i = phi i64 [ %"CurrWI++375.i", %thenBB371.i ], [ 0, %SyncBB247.i ]
  %CurrSBIndex..17.i = phi i64 [ %"loadedCurrSB+Stride377.i", %thenBB371.i ], [ 0, %SyncBB247.i ]
  %"&(pSB[currWI].offset)77.i" = add nuw i64 %CurrSBIndex..17.i, 96
  %"&pSB[currWI].offset78.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)77.i"
  %CastToValueType79.i = bitcast i8* %"&pSB[currWI].offset78.i" to double addrspace(3)**
  %loadedValue80.i = load double addrspace(3)** %CastToValueType79.i, align 8
  %116 = load double addrspace(3)* %loadedValue80.i, align 8
  %"&(pSB[currWI].offset)228.i" = add nuw i64 %CurrSBIndex..17.i, 168
  %"&pSB[currWI].offset229.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)228.i"
  %CastToValueType230.i = bitcast i8* %"&pSB[currWI].offset229.i" to double*
  %loadedValue231.i = load double* %CastToValueType230.i, align 8
  %117 = fadd double %116, %loadedValue231.i
  %"&(pSB[currWI].offset)72.i" = add nuw i64 %CurrSBIndex..17.i, 96
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)72.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to double addrspace(3)**
  %loadedValue75.i = load double addrspace(3)** %CastToValueType74.i, align 8
  store double %117, double addrspace(3)* %loadedValue75.i, align 8
  %check.WI.iter374.i = icmp ult i64 %CurrWI..17.i, %iterCount
  br i1 %check.WI.iter374.i, label %thenBB371.i, label %SyncBB250.i

thenBB371.i:                                      ; preds = %SyncBB249.i
  %"CurrWI++375.i" = add nuw i64 %CurrWI..17.i, 1
  %"loadedCurrSB+Stride377.i" = add nuw i64 %CurrSBIndex..17.i, 192
  br label %SyncBB249.i

SyncBB250.i:                                      ; preds = %thenBB357.i, %SyncBB249.i
  %CurrWI..18.i = phi i64 [ %"CurrWI++361.i", %thenBB357.i ], [ 0, %SyncBB249.i ]
  %CurrSBIndex..18.i = phi i64 [ %"loadedCurrSB+Stride363.i", %thenBB357.i ], [ 0, %SyncBB249.i ]
  %"&(pSB[currWI].offset)156.i" = add nuw i64 %CurrSBIndex..18.i, 104
  %"&pSB[currWI].offset157.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)156.i"
  %CastToValueType158.i = bitcast i8* %"&pSB[currWI].offset157.i" to double addrspace(3)**
  %loadedValue159.i = load double addrspace(3)** %CastToValueType158.i, align 8
  %118 = load double addrspace(3)* %loadedValue159.i, align 8
  %"&(pSB[currWI].offset)8.i" = add nuw i64 %CurrSBIndex..18.i, 72
  %"&pSB[currWI].offset9.i" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)8.i"
  %CastToArgType10.i = bitcast i8* %"&pSB[currWI].offset9.i" to double*
  store double %118, double* %CastToArgType10.i, align 8
  %check.WI.iter360.i = icmp ult i64 %CurrWI..18.i, %iterCount
  br i1 %check.WI.iter360.i, label %thenBB357.i, label %"Barrier BB3"

thenBB357.i:                                      ; preds = %SyncBB250.i
  %"CurrWI++361.i" = add nuw i64 %CurrWI..18.i, 1
  %"loadedCurrSB+Stride363.i" = add nuw i64 %CurrSBIndex..18.i, 192
  br label %SyncBB250.i

"Barrier BB3":                                    ; preds = %SyncBB250.i
  store i64 0, i64* %pCurrWI, align 8
  %119 = icmp eq i32 %storeSum, 0
  %120 = bitcast double addrspace(1)* %g_odata to <4 x double> addrspace(1)*
  br label %SyncBB142

SyncBB142:                                        ; preds = %thenBB, %"Barrier BB3"
  %CurrSBIndex..0 = phi i64 [ 0, %"Barrier BB3" ], [ %"loadedCurrSB+Stride", %thenBB ]
  br i1 %119, label %phi-split-bb, label %121

; <label>:121                                     ; preds = %SyncBB142
  %"&(pSB[currWI].offset)187" = or i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset19" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)187"
  %CastToValueType20 = bitcast i8* %"&pSB[currWI].offset19" to i64*
  %loadedValue21 = load i64* %CastToValueType20, align 8
  %sext = shl i64 %loadedValue21, 32
  %122 = ashr i64 %sext, 32
  %123 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %124 = load i64* %123, align 8
  %125 = add i64 %124, -1
  %126 = icmp eq i64 %122, %125
  br i1 %126, label %127, label %phi-split-bb

; <label>:127                                     ; preds = %121
  %"&(pSB[currWI].offset)9617" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset97" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)9617"
  %CastToValueType98 = bitcast i8* %"&pSB[currWI].offset97" to <4 x double>*
  %loadedValue99 = load <4 x double>* %CastToValueType98, align 32
  %128 = extractelement <4 x double> %loadedValue99, i32 3
  %"&(pSB[currWI].offset)110" = add nuw i64 %CurrSBIndex..0, 72
  %"&pSB[currWI].offset111" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)110"
  %CastToValueType112 = bitcast i8* %"&pSB[currWI].offset111" to double*
  %loadedValue113 = load double* %CastToValueType112, align 8
  %129 = fadd double %loadedValue113, %128
  %130 = load i64* %pWGId, align 8
  %131 = getelementptr inbounds double addrspace(1)* %g_blockSums, i64 %130
  store double %129, double addrspace(1)* %131, align 8
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %SyncBB142, %127, %121
  %"&(pSB[currWI].offset)115" = add nuw i64 %CurrSBIndex..0, 72
  %"&pSB[currWI].offset116" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)115"
  %CastToValueType117 = bitcast i8* %"&pSB[currWI].offset116" to double*
  %loadedValue118 = load double* %CastToValueType117, align 8
  %132 = insertelement <4 x double> undef, double %loadedValue118, i32 0
  %"&(pSB[currWI].offset)918" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset92" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)918"
  %CastToValueType93 = bitcast i8* %"&pSB[currWI].offset92" to <4 x double>*
  %loadedValue94 = load <4 x double>* %CastToValueType93, align 32
  %133 = extractelement <4 x double> %loadedValue94, i32 0
  %"&(pSB[currWI].offset)120" = add nuw i64 %CurrSBIndex..0, 72
  %"&pSB[currWI].offset121" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)120"
  %CastToValueType122 = bitcast i8* %"&pSB[currWI].offset121" to double*
  %loadedValue123 = load double* %CastToValueType122, align 8
  %134 = fadd double %loadedValue123, %133
  %135 = insertelement <4 x double> %132, double %134, i32 1
  %"&(pSB[currWI].offset)869" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset87" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)869"
  %CastToValueType88 = bitcast i8* %"&pSB[currWI].offset87" to <4 x double>*
  %loadedValue89 = load <4 x double>* %CastToValueType88, align 32
  %136 = extractelement <4 x double> %loadedValue89, i32 1
  %"&(pSB[currWI].offset)125" = add nuw i64 %CurrSBIndex..0, 72
  %"&pSB[currWI].offset126" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)125"
  %CastToValueType127 = bitcast i8* %"&pSB[currWI].offset126" to double*
  %loadedValue128 = load double* %CastToValueType127, align 8
  %137 = fadd double %loadedValue128, %136
  %138 = insertelement <4 x double> %135, double %137, i32 2
  %"&(pSB[currWI].offset)8110" = or i64 %CurrSBIndex..0, 32
  %"&pSB[currWI].offset82" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)8110"
  %CastToValueType83 = bitcast i8* %"&pSB[currWI].offset82" to <4 x double>*
  %loadedValue84 = load <4 x double>* %CastToValueType83, align 32
  %139 = extractelement <4 x double> %loadedValue84, i32 2
  %"&(pSB[currWI].offset)130" = add nuw i64 %CurrSBIndex..0, 72
  %"&pSB[currWI].offset131" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)130"
  %CastToValueType132 = bitcast i8* %"&pSB[currWI].offset131" to double*
  %loadedValue133 = load double* %CastToValueType132, align 8
  %140 = fadd double %loadedValue133, %139
  %141 = insertelement <4 x double> %138, double %140, i32 3
  br i1 %1, label %142, label %145

; <label>:142                                     ; preds = %phi-split-bb
  %"&(pSB[currWI].offset)3712" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset38" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3712"
  %CastToValueType39 = bitcast i8* %"&pSB[currWI].offset38" to i32*
  %loadedValue40 = load i32* %CastToValueType39, align 4
  %143 = or i32 %loadedValue40, 3
  %144 = icmp slt i32 %143, %n
  br i1 %144, label %145, label %148

; <label>:145                                     ; preds = %142, %phi-split-bb
  %"&(pSB[currWI].offset)911" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset10" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)911"
  %CastToValueType11 = bitcast i8* %"&pSB[currWI].offset10" to i32*
  %loadedValue12 = load i32* %CastToValueType11, align 4
  %146 = sext i32 %loadedValue12 to i64
  %147 = getelementptr inbounds <4 x double> addrspace(1)* %120, i64 %146
  store <4 x double> %141, <4 x double> addrspace(1)* %147, align 32
  br label %UnifiedReturnBlock

; <label>:148                                     ; preds = %142
  %"&(pSB[currWI].offset)6213" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset63" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)6213"
  %CastToValueType64 = bitcast i8* %"&pSB[currWI].offset63" to i32*
  %loadedValue65 = load i32* %CastToValueType64, align 4
  %149 = icmp slt i32 %loadedValue65, %n
  br i1 %149, label %150, label %UnifiedReturnBlock

; <label>:150                                     ; preds = %148
  %"&(pSB[currWI].offset)5714" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset58" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)5714"
  %CastToValueType59 = bitcast i8* %"&pSB[currWI].offset58" to i32*
  %loadedValue60 = load i32* %CastToValueType59, align 4
  %151 = sext i32 %loadedValue60 to i64
  %152 = getelementptr inbounds double addrspace(1)* %g_odata, i64 %151
  %"&(pSB[currWI].offset)135" = add nuw i64 %CurrSBIndex..0, 72
  %"&pSB[currWI].offset136" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)135"
  %CastToValueType137 = bitcast i8* %"&pSB[currWI].offset136" to double*
  %loadedValue138 = load double* %CastToValueType137, align 8
  store double %loadedValue138, double addrspace(1)* %152, align 8
  %"&(pSB[currWI].offset)3215" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset33" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)3215"
  %CastToValueType34 = bitcast i8* %"&pSB[currWI].offset33" to i32*
  %loadedValue35 = load i32* %CastToValueType34, align 4
  %153 = or i32 %loadedValue35, 1
  %154 = icmp slt i32 %153, %n
  br i1 %154, label %155, label %UnifiedReturnBlock

; <label>:155                                     ; preds = %150
  %156 = sext i32 %153 to i64
  %157 = getelementptr inbounds double addrspace(1)* %g_odata, i64 %156
  store double %134, double addrspace(1)* %157, align 8
  %"&(pSB[currWI].offset)2716" = or i64 %CurrSBIndex..0, 24
  %"&pSB[currWI].offset28" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)2716"
  %CastToValueType29 = bitcast i8* %"&pSB[currWI].offset28" to i32*
  %loadedValue30 = load i32* %CastToValueType29, align 4
  %158 = or i32 %loadedValue30, 2
  %159 = icmp slt i32 %158, %n
  br i1 %159, label %160, label %UnifiedReturnBlock

; <label>:160                                     ; preds = %155
  %161 = sext i32 %158 to i64
  %162 = getelementptr inbounds double addrspace(1)* %g_odata, i64 %161
  store double %137, double addrspace(1)* %162, align 8
  br label %UnifiedReturnBlock

UnifiedReturnBlock:                               ; preds = %160, %148, %155, %150, %145
  %loadedCurrWI = load i64* %pCurrWI, align 8
  %check.WI.iter = icmp ult i64 %loadedCurrWI, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %UnifiedReturnBlock
  %"CurrWI++" = add nuw i64 %loadedCurrWI, 1
  store i64 %"CurrWI++", i64* %pCurrWI, align 8
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 192
  br label %SyncBB142

elseBB:                                           ; preds = %UnifiedReturnBlock
  store i64 0, i64* %pCurrWI, align 8
  ret void
}

define void @scan(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(1)**
  %7 = load double addrspace(1)** %6, align 8
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
  %38 = getelementptr i8* %pBuffer, i64 104
  %39 = bitcast i8* %38 to i64**
  %40 = load i64** %39, align 8
  store i64 0, i64* %40, align 8
  %41 = bitcast double addrspace(1)* %4 to <4 x double> addrspace(1)*
  %42 = icmp eq i32 %13, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB144.i, %entry
  %CurrSBIndex..1.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride150.i", %thenBB144.i ]
  %currWI151.i = load i64* %40, align 8
  %43 = getelementptr %struct.PaddedDimId* %31, i64 %currWI151.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = getelementptr %struct.PaddedDimId* %28, i64 0, i32 0, i64 0
  %46 = load i64* %45, align 8
  %47 = add i64 %44, %46
  %48 = trunc i64 %47 to i32
  %"&(pSB[currWI].offset)2.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2.i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %48, i32* %CastToValueType.i, align 4
  %currWI.i = load i64* %40, align 8
  %49 = getelementptr %struct.PaddedDimId* %31, i64 %currWI.i, i32 0, i64 0
  %50 = load i64* %49, align 8
  %"&(pSB[currWI].offset)143.i" = or i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset15.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)143.i"
  %CastToValueType16.i = bitcast i8* %"&pSB[currWI].offset15.i" to i64*
  store i64 %50, i64* %CastToValueType16.i, align 8
  %51 = shl i32 %48, 2
  %"&(pSB[currWI].offset)234.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)234.i"
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i32*
  store i32 %51, i32* %CastToValueType25.i, align 4
  br i1 %42, label %52, label %55

; <label>:52                                      ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)5218.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset53.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)5218.i"
  %CastToValueType54.i = bitcast i8* %"&pSB[currWI].offset53.i" to i32*
  %loadedValue55.i = load i32* %CastToValueType54.i, align 4
  %53 = or i32 %loadedValue55.i, 3
  %54 = icmp slt i32 %53, %10
  br i1 %54, label %55, label %69

; <label>:55                                      ; preds = %52, %SyncBB.i
  %"&(pSB[currWI].offset)55.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset6.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)55.i"
  %CastToValueType7.i = bitcast i8* %"&pSB[currWI].offset6.i" to i32*
  %loadedValue.i = load i32* %CastToValueType7.i, align 4
  %56 = sext i32 %loadedValue.i to i64
  %57 = getelementptr inbounds <4 x double> addrspace(1)* %41, i64 %56
  %58 = load <4 x double> addrspace(1)* %57, align 32
  %59 = extractelement <4 x double> %58, i32 1
  %60 = extractelement <4 x double> %58, i32 0
  %61 = fadd double %59, %60
  %62 = insertelement <4 x double> %58, double %61, i32 1
  %63 = extractelement <4 x double> %58, i32 2
  %64 = fadd double %63, %61
  %65 = insertelement <4 x double> %62, double %64, i32 2
  %66 = extractelement <4 x double> %58, i32 3
  %67 = fadd double %66, %64
  %68 = insertelement <4 x double> %65, double %67, i32 3
  br label %"Barrier BB.i"

; <label>:69                                      ; preds = %52
  %"&(pSB[currWI].offset)7219.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset73.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)7219.i"
  %CastToValueType74.i = bitcast i8* %"&pSB[currWI].offset73.i" to i32*
  %loadedValue75.i = load i32* %CastToValueType74.i, align 4
  %70 = icmp slt i32 %loadedValue75.i, %10
  br i1 %70, label %71, label %75

; <label>:71                                      ; preds = %69
  %"&(pSB[currWI].offset)6722.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset68.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)6722.i"
  %CastToValueType69.i = bitcast i8* %"&pSB[currWI].offset68.i" to i32*
  %loadedValue70.i = load i32* %CastToValueType69.i, align 4
  %72 = sext i32 %loadedValue70.i to i64
  %73 = getelementptr inbounds double addrspace(1)* %4, i64 %72
  %74 = load double addrspace(1)* %73, align 8
  br label %75

; <label>:75                                      ; preds = %71, %69
  %76 = phi double [ %74, %71 ], [ 0.000000e+00, %69 ]
  %77 = insertelement <4 x double> undef, double %76, i32 0
  %"&(pSB[currWI].offset)4720.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset48.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)4720.i"
  %CastToValueType49.i = bitcast i8* %"&pSB[currWI].offset48.i" to i32*
  %loadedValue50.i = load i32* %CastToValueType49.i, align 4
  %78 = or i32 %loadedValue50.i, 1
  %79 = icmp slt i32 %78, %10
  br i1 %79, label %80, label %84

; <label>:80                                      ; preds = %75
  %81 = sext i32 %78 to i64
  %82 = getelementptr inbounds double addrspace(1)* %4, i64 %81
  %83 = load double addrspace(1)* %82, align 8
  br label %84

; <label>:84                                      ; preds = %80, %75
  %85 = phi double [ %83, %80 ], [ 0.000000e+00, %75 ]
  %86 = fadd double %85, %76
  %87 = insertelement <4 x double> %77, double %86, i32 1
  %"&(pSB[currWI].offset)4221.i" = or i64 %CurrSBIndex..1.i, 24
  %"&pSB[currWI].offset43.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)4221.i"
  %CastToValueType44.i = bitcast i8* %"&pSB[currWI].offset43.i" to i32*
  %loadedValue45.i = load i32* %CastToValueType44.i, align 4
  %88 = or i32 %loadedValue45.i, 2
  %89 = icmp slt i32 %88, %10
  br i1 %89, label %90, label %94

; <label>:90                                      ; preds = %84
  %91 = sext i32 %88 to i64
  %92 = getelementptr inbounds double addrspace(1)* %4, i64 %91
  %93 = load double addrspace(1)* %92, align 8
  br label %94

; <label>:94                                      ; preds = %90, %84
  %95 = phi double [ %93, %90 ], [ 0.000000e+00, %84 ]
  %96 = fadd double %95, %86
  %97 = insertelement <4 x double> %87, double %96, i32 2
  %98 = fadd double %96, 0.000000e+00
  %99 = insertelement <4 x double> %97, double %98, i32 3
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %94, %55
  %threadScanT.0.i = phi <4 x double> [ %68, %55 ], [ %99, %94 ]
  %res.0.i = phi double [ %67, %55 ], [ %98, %94 ]
  %"&(pSB[currWI].offset)101.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset102.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)101.i"
  %CastToValueType103.i = bitcast i8* %"&pSB[currWI].offset102.i" to double*
  store double %res.0.i, double* %CastToValueType103.i, align 8
  %"&(pSB[currWI].offset)776.i" = or i64 %CurrSBIndex..1.i, 32
  %"&pSB[currWI].offset78.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)776.i"
  %CastToValueType79.i = bitcast i8* %"&pSB[currWI].offset78.i" to <4 x double>*
  store <4 x double> %threadScanT.0.i, <4 x double>* %CastToValueType79.i, align 32
  %loadedCurrWI146.i = load i64* %40, align 8
  %check.WI.iter147.i = icmp ult i64 %loadedCurrWI146.i, %34
  br i1 %check.WI.iter147.i, label %thenBB144.i, label %elseBB145.i

thenBB144.i:                                      ; preds = %"Barrier BB.i"
  %"CurrWI++148.i" = add nuw i64 %loadedCurrWI146.i, 1
  store i64 %"CurrWI++148.i", i64* %40, align 8
  %"loadedCurrSB+Stride150.i" = add nuw i64 %CurrSBIndex..1.i, 192
  br label %SyncBB.i

elseBB145.i:                                      ; preds = %"Barrier BB.i"
  store i64 0, i64* %40, align 8
  %100 = bitcast i8 addrspace(3)* %19 to [512 x double] addrspace(3)*
  br label %SyncBB239.i.i

SyncBB239.i.i:                                    ; preds = %thenBB.i.i, %elseBB145.i
  %CurrWI..0.i.i = phi i64 [ %"CurrWI++.i.i", %thenBB.i.i ], [ 0, %elseBB145.i ]
  %CurrSBIndex..0.i.i = phi i64 [ %"loadedCurrSB+Stride.i.i", %thenBB.i.i ], [ 0, %elseBB145.i ]
  %101 = getelementptr %struct.PaddedDimId* %31, i64 %CurrWI..0.i.i, i32 0, i64 0
  %102 = load i64* %101, align 8
  %"&(pSB[currWI].offset).i.i" = add nuw i64 %CurrSBIndex..0.i.i, 80
  %"&pSB[currWI].offset.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset).i.i"
  %CastToValueType.i.i = bitcast i8* %"&pSB[currWI].offset.i.i" to i64*
  store i64 %102, i64* %CastToValueType.i.i, align 8
  %sext.i.i = shl i64 %102, 32
  %103 = ashr i64 %sext.i.i, 32
  %104 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %103
  store double 0.000000e+00, double addrspace(3)* %104, align 8
  %check.WI.iter.i.i = icmp ult i64 %CurrWI..0.i.i, %34
  br i1 %check.WI.iter.i.i, label %thenBB.i.i, label %SyncBB.i.i

thenBB.i.i:                                       ; preds = %SyncBB239.i.i
  %"CurrWI++.i.i" = add nuw i64 %CurrWI..0.i.i, 1
  %"loadedCurrSB+Stride.i.i" = add nuw i64 %CurrSBIndex..0.i.i, 192
  br label %SyncBB239.i.i

SyncBB.i.i:                                       ; preds = %thenBB252.i.i, %SyncBB239.i.i
  %CurrWI..1.i.i = phi i64 [ %"CurrWI++256.i.i", %thenBB252.i.i ], [ 0, %SyncBB239.i.i ]
  %CurrSBIndex..1.i.i = phi i64 [ %"loadedCurrSB+Stride258.i.i", %thenBB252.i.i ], [ 0, %SyncBB239.i.i ]
  %105 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %106 = load i64* %105, align 8
  %"&(pSB[currWI].offset)20.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 80
  %"&pSB[currWI].offset21.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)20.i.i"
  %CastToValueType22.i.i = bitcast i8* %"&pSB[currWI].offset21.i.i" to i64*
  %loadedValue.i.i = load i64* %CastToValueType22.i.i, align 8
  %107 = add i64 %106, %loadedValue.i.i
  %108 = trunc i64 %107 to i32
  %"&(pSB[currWI].offset)24.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 88
  %"&pSB[currWI].offset25.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)24.i.i"
  %CastToValueType26.i.i = bitcast i8* %"&pSB[currWI].offset25.i.i" to i32*
  store i32 %108, i32* %CastToValueType26.i.i, align 4
  %109 = sext i32 %108 to i64
  %110 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %109
  %"&(pSB[currWI].offset)68.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 96
  %"&pSB[currWI].offset69.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)68.i.i"
  %CastToValueType70.i.i = bitcast i8* %"&pSB[currWI].offset69.i.i" to double addrspace(3)**
  store double addrspace(3)* %110, double addrspace(3)** %CastToValueType70.i.i, align 8
  %"&(pSB[currWI].offset)4.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 64
  %"&pSB[currWI].offset5.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)4.i.i"
  %CastToArgType.i.i = bitcast i8* %"&pSB[currWI].offset5.i.i" to double*
  %loadedValue6.i.i = load double* %CastToArgType.i.i, align 8
  store double %loadedValue6.i.i, double addrspace(3)* %110, align 8
  %check.WI.iter255.i.i = icmp ult i64 %CurrWI..1.i.i, %34
  br i1 %check.WI.iter255.i.i, label %thenBB252.i.i, label %SyncBB232.i.i

thenBB252.i.i:                                    ; preds = %SyncBB.i.i
  %"CurrWI++256.i.i" = add nuw i64 %CurrWI..1.i.i, 1
  %"loadedCurrSB+Stride258.i.i" = add nuw i64 %CurrSBIndex..1.i.i, 192
  br label %SyncBB.i.i

SyncBB232.i.i:                                    ; preds = %thenBB259.i.i, %SyncBB.i.i
  %CurrWI..2.i.i = phi i64 [ %"CurrWI++263.i.i", %thenBB259.i.i ], [ 0, %SyncBB.i.i ]
  %CurrSBIndex..2.i.i = phi i64 [ %"loadedCurrSB+Stride265.i.i", %thenBB259.i.i ], [ 0, %SyncBB.i.i ]
  %"&(pSB[currWI].offset)63.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 88
  %"&pSB[currWI].offset64.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)63.i.i"
  %CastToValueType65.i.i = bitcast i8* %"&pSB[currWI].offset64.i.i" to i32*
  %loadedValue66.i.i = load i32* %CastToValueType65.i.i, align 4
  %111 = add nsw i32 %loadedValue66.i.i, -1
  %112 = sext i32 %111 to i64
  %113 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %112
  %"&(pSB[currWI].offset)152.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 104
  %"&pSB[currWI].offset153.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)152.i.i"
  %CastToValueType154.i.i = bitcast i8* %"&pSB[currWI].offset153.i.i" to double addrspace(3)**
  store double addrspace(3)* %113, double addrspace(3)** %CastToValueType154.i.i, align 8
  %114 = load double addrspace(3)* %113, align 8
  %"&(pSB[currWI].offset)161.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 112
  %"&pSB[currWI].offset162.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)161.i.i"
  %CastToValueType163.i.i = bitcast i8* %"&pSB[currWI].offset162.i.i" to double*
  store double %114, double* %CastToValueType163.i.i, align 8
  %check.WI.iter262.i.i = icmp ult i64 %CurrWI..2.i.i, %34
  br i1 %check.WI.iter262.i.i, label %thenBB259.i.i, label %SyncBB233.i.i

thenBB259.i.i:                                    ; preds = %SyncBB232.i.i
  %"CurrWI++263.i.i" = add nuw i64 %CurrWI..2.i.i, 1
  %"loadedCurrSB+Stride265.i.i" = add nuw i64 %CurrSBIndex..2.i.i, 192
  br label %SyncBB232.i.i

SyncBB233.i.i:                                    ; preds = %thenBB266.i.i, %SyncBB232.i.i
  %CurrWI..3.i.i = phi i64 [ %"CurrWI++270.i.i", %thenBB266.i.i ], [ 0, %SyncBB232.i.i ]
  %CurrSBIndex..3.i.i = phi i64 [ %"loadedCurrSB+Stride272.i.i", %thenBB266.i.i ], [ 0, %SyncBB232.i.i ]
  %"&(pSB[currWI].offset)147.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 96
  %"&pSB[currWI].offset148.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)147.i.i"
  %CastToValueType149.i.i = bitcast i8* %"&pSB[currWI].offset148.i.i" to double addrspace(3)**
  %loadedValue150.i.i = load double addrspace(3)** %CastToValueType149.i.i, align 8
  %115 = load double addrspace(3)* %loadedValue150.i.i, align 8
  %"&(pSB[currWI].offset)165.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 112
  %"&pSB[currWI].offset166.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)165.i.i"
  %CastToValueType167.i.i = bitcast i8* %"&pSB[currWI].offset166.i.i" to double*
  %loadedValue168.i.i = load double* %CastToValueType167.i.i, align 8
  %116 = fadd double %115, %loadedValue168.i.i
  %"&(pSB[currWI].offset)142.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 96
  %"&pSB[currWI].offset143.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)142.i.i"
  %CastToValueType144.i.i = bitcast i8* %"&pSB[currWI].offset143.i.i" to double addrspace(3)**
  %loadedValue145.i.i = load double addrspace(3)** %CastToValueType144.i.i, align 8
  store double %116, double addrspace(3)* %loadedValue145.i.i, align 8
  %check.WI.iter269.i.i = icmp ult i64 %CurrWI..3.i.i, %34
  br i1 %check.WI.iter269.i.i, label %thenBB266.i.i, label %SyncBB234.i.i

thenBB266.i.i:                                    ; preds = %SyncBB233.i.i
  %"CurrWI++270.i.i" = add nuw i64 %CurrWI..3.i.i, 1
  %"loadedCurrSB+Stride272.i.i" = add nuw i64 %CurrSBIndex..3.i.i, 192
  br label %SyncBB233.i.i

SyncBB234.i.i:                                    ; preds = %thenBB273.i.i, %SyncBB233.i.i
  %CurrWI..4.i.i = phi i64 [ %"CurrWI++277.i.i", %thenBB273.i.i ], [ 0, %SyncBB233.i.i ]
  %CurrSBIndex..4.i.i = phi i64 [ %"loadedCurrSB+Stride279.i.i", %thenBB273.i.i ], [ 0, %SyncBB233.i.i ]
  %"&(pSB[currWI].offset)58.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 88
  %"&pSB[currWI].offset59.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)58.i.i"
  %CastToValueType60.i.i = bitcast i8* %"&pSB[currWI].offset59.i.i" to i32*
  %loadedValue61.i.i = load i32* %CastToValueType60.i.i, align 4
  %117 = add nsw i32 %loadedValue61.i.i, -2
  %118 = sext i32 %117 to i64
  %119 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %118
  %120 = load double addrspace(3)* %119, align 8
  %"&(pSB[currWI].offset)170.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 120
  %"&pSB[currWI].offset171.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)170.i.i"
  %CastToValueType172.i.i = bitcast i8* %"&pSB[currWI].offset171.i.i" to double*
  store double %120, double* %CastToValueType172.i.i, align 8
  %check.WI.iter276.i.i = icmp ult i64 %CurrWI..4.i.i, %34
  br i1 %check.WI.iter276.i.i, label %thenBB273.i.i, label %SyncBB235.i.i

thenBB273.i.i:                                    ; preds = %SyncBB234.i.i
  %"CurrWI++277.i.i" = add nuw i64 %CurrWI..4.i.i, 1
  %"loadedCurrSB+Stride279.i.i" = add nuw i64 %CurrSBIndex..4.i.i, 192
  br label %SyncBB234.i.i

SyncBB235.i.i:                                    ; preds = %thenBB280.i.i, %SyncBB234.i.i
  %CurrWI..5.i.i = phi i64 [ %"CurrWI++284.i.i", %thenBB280.i.i ], [ 0, %SyncBB234.i.i ]
  %CurrSBIndex..5.i.i = phi i64 [ %"loadedCurrSB+Stride286.i.i", %thenBB280.i.i ], [ 0, %SyncBB234.i.i ]
  %"&(pSB[currWI].offset)137.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 96
  %"&pSB[currWI].offset138.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)137.i.i"
  %CastToValueType139.i.i = bitcast i8* %"&pSB[currWI].offset138.i.i" to double addrspace(3)**
  %loadedValue140.i.i = load double addrspace(3)** %CastToValueType139.i.i, align 8
  %121 = load double addrspace(3)* %loadedValue140.i.i, align 8
  %"&(pSB[currWI].offset)174.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 120
  %"&pSB[currWI].offset175.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)174.i.i"
  %CastToValueType176.i.i = bitcast i8* %"&pSB[currWI].offset175.i.i" to double*
  %loadedValue177.i.i = load double* %CastToValueType176.i.i, align 8
  %122 = fadd double %121, %loadedValue177.i.i
  %"&(pSB[currWI].offset)132.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 96
  %"&pSB[currWI].offset133.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)132.i.i"
  %CastToValueType134.i.i = bitcast i8* %"&pSB[currWI].offset133.i.i" to double addrspace(3)**
  %loadedValue135.i.i = load double addrspace(3)** %CastToValueType134.i.i, align 8
  store double %122, double addrspace(3)* %loadedValue135.i.i, align 8
  %check.WI.iter283.i.i = icmp ult i64 %CurrWI..5.i.i, %34
  br i1 %check.WI.iter283.i.i, label %thenBB280.i.i, label %SyncBB236.i.i

thenBB280.i.i:                                    ; preds = %SyncBB235.i.i
  %"CurrWI++284.i.i" = add nuw i64 %CurrWI..5.i.i, 1
  %"loadedCurrSB+Stride286.i.i" = add nuw i64 %CurrSBIndex..5.i.i, 192
  br label %SyncBB235.i.i

SyncBB236.i.i:                                    ; preds = %thenBB287.i.i, %SyncBB235.i.i
  %CurrWI..6.i.i = phi i64 [ %"CurrWI++291.i.i", %thenBB287.i.i ], [ 0, %SyncBB235.i.i ]
  %CurrSBIndex..6.i.i = phi i64 [ %"loadedCurrSB+Stride293.i.i", %thenBB287.i.i ], [ 0, %SyncBB235.i.i ]
  %"&(pSB[currWI].offset)53.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 88
  %"&pSB[currWI].offset54.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)53.i.i"
  %CastToValueType55.i.i = bitcast i8* %"&pSB[currWI].offset54.i.i" to i32*
  %loadedValue56.i.i = load i32* %CastToValueType55.i.i, align 4
  %123 = add nsw i32 %loadedValue56.i.i, -4
  %124 = sext i32 %123 to i64
  %125 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %124
  %126 = load double addrspace(3)* %125, align 8
  %"&(pSB[currWI].offset)179.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 128
  %"&pSB[currWI].offset180.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)179.i.i"
  %CastToValueType181.i.i = bitcast i8* %"&pSB[currWI].offset180.i.i" to double*
  store double %126, double* %CastToValueType181.i.i, align 8
  %check.WI.iter290.i.i = icmp ult i64 %CurrWI..6.i.i, %34
  br i1 %check.WI.iter290.i.i, label %thenBB287.i.i, label %SyncBB237.i.i

thenBB287.i.i:                                    ; preds = %SyncBB236.i.i
  %"CurrWI++291.i.i" = add nuw i64 %CurrWI..6.i.i, 1
  %"loadedCurrSB+Stride293.i.i" = add nuw i64 %CurrSBIndex..6.i.i, 192
  br label %SyncBB236.i.i

SyncBB237.i.i:                                    ; preds = %thenBB294.i.i, %SyncBB236.i.i
  %CurrWI..7.i.i = phi i64 [ %"CurrWI++298.i.i", %thenBB294.i.i ], [ 0, %SyncBB236.i.i ]
  %CurrSBIndex..7.i.i = phi i64 [ %"loadedCurrSB+Stride300.i.i", %thenBB294.i.i ], [ 0, %SyncBB236.i.i ]
  %"&(pSB[currWI].offset)127.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 96
  %"&pSB[currWI].offset128.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)127.i.i"
  %CastToValueType129.i.i = bitcast i8* %"&pSB[currWI].offset128.i.i" to double addrspace(3)**
  %loadedValue130.i.i = load double addrspace(3)** %CastToValueType129.i.i, align 8
  %127 = load double addrspace(3)* %loadedValue130.i.i, align 8
  %"&(pSB[currWI].offset)183.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 128
  %"&pSB[currWI].offset184.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)183.i.i"
  %CastToValueType185.i.i = bitcast i8* %"&pSB[currWI].offset184.i.i" to double*
  %loadedValue186.i.i = load double* %CastToValueType185.i.i, align 8
  %128 = fadd double %127, %loadedValue186.i.i
  %"&(pSB[currWI].offset)122.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 96
  %"&pSB[currWI].offset123.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)122.i.i"
  %CastToValueType124.i.i = bitcast i8* %"&pSB[currWI].offset123.i.i" to double addrspace(3)**
  %loadedValue125.i.i = load double addrspace(3)** %CastToValueType124.i.i, align 8
  store double %128, double addrspace(3)* %loadedValue125.i.i, align 8
  %check.WI.iter297.i.i = icmp ult i64 %CurrWI..7.i.i, %34
  br i1 %check.WI.iter297.i.i, label %thenBB294.i.i, label %SyncBB238.i.i

thenBB294.i.i:                                    ; preds = %SyncBB237.i.i
  %"CurrWI++298.i.i" = add nuw i64 %CurrWI..7.i.i, 1
  %"loadedCurrSB+Stride300.i.i" = add nuw i64 %CurrSBIndex..7.i.i, 192
  br label %SyncBB237.i.i

SyncBB238.i.i:                                    ; preds = %thenBB301.i.i, %SyncBB237.i.i
  %CurrWI..8.i.i = phi i64 [ %"CurrWI++305.i.i", %thenBB301.i.i ], [ 0, %SyncBB237.i.i ]
  %CurrSBIndex..8.i.i = phi i64 [ %"loadedCurrSB+Stride307.i.i", %thenBB301.i.i ], [ 0, %SyncBB237.i.i ]
  %"&(pSB[currWI].offset)48.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 88
  %"&pSB[currWI].offset49.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)48.i.i"
  %CastToValueType50.i.i = bitcast i8* %"&pSB[currWI].offset49.i.i" to i32*
  %loadedValue51.i.i = load i32* %CastToValueType50.i.i, align 4
  %129 = add nsw i32 %loadedValue51.i.i, -8
  %130 = sext i32 %129 to i64
  %131 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %130
  %132 = load double addrspace(3)* %131, align 8
  %"&(pSB[currWI].offset)188.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 136
  %"&pSB[currWI].offset189.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)188.i.i"
  %CastToValueType190.i.i = bitcast i8* %"&pSB[currWI].offset189.i.i" to double*
  store double %132, double* %CastToValueType190.i.i, align 8
  %check.WI.iter304.i.i = icmp ult i64 %CurrWI..8.i.i, %34
  br i1 %check.WI.iter304.i.i, label %thenBB301.i.i, label %SyncBB240.i.i

thenBB301.i.i:                                    ; preds = %SyncBB238.i.i
  %"CurrWI++305.i.i" = add nuw i64 %CurrWI..8.i.i, 1
  %"loadedCurrSB+Stride307.i.i" = add nuw i64 %CurrSBIndex..8.i.i, 192
  br label %SyncBB238.i.i

SyncBB240.i.i:                                    ; preds = %thenBB308.i.i, %SyncBB238.i.i
  %CurrWI..9.i.i = phi i64 [ %"CurrWI++312.i.i", %thenBB308.i.i ], [ 0, %SyncBB238.i.i ]
  %CurrSBIndex..9.i.i = phi i64 [ %"loadedCurrSB+Stride314.i.i", %thenBB308.i.i ], [ 0, %SyncBB238.i.i ]
  %"&(pSB[currWI].offset)117.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 96
  %"&pSB[currWI].offset118.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)117.i.i"
  %CastToValueType119.i.i = bitcast i8* %"&pSB[currWI].offset118.i.i" to double addrspace(3)**
  %loadedValue120.i.i = load double addrspace(3)** %CastToValueType119.i.i, align 8
  %133 = load double addrspace(3)* %loadedValue120.i.i, align 8
  %"&(pSB[currWI].offset)192.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 136
  %"&pSB[currWI].offset193.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)192.i.i"
  %CastToValueType194.i.i = bitcast i8* %"&pSB[currWI].offset193.i.i" to double*
  %loadedValue195.i.i = load double* %CastToValueType194.i.i, align 8
  %134 = fadd double %133, %loadedValue195.i.i
  %"&(pSB[currWI].offset)112.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 96
  %"&pSB[currWI].offset113.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)112.i.i"
  %CastToValueType114.i.i = bitcast i8* %"&pSB[currWI].offset113.i.i" to double addrspace(3)**
  %loadedValue115.i.i = load double addrspace(3)** %CastToValueType114.i.i, align 8
  store double %134, double addrspace(3)* %loadedValue115.i.i, align 8
  %check.WI.iter311.i.i = icmp ult i64 %CurrWI..9.i.i, %34
  br i1 %check.WI.iter311.i.i, label %thenBB308.i.i, label %SyncBB241.i.i

thenBB308.i.i:                                    ; preds = %SyncBB240.i.i
  %"CurrWI++312.i.i" = add nuw i64 %CurrWI..9.i.i, 1
  %"loadedCurrSB+Stride314.i.i" = add nuw i64 %CurrSBIndex..9.i.i, 192
  br label %SyncBB240.i.i

SyncBB241.i.i:                                    ; preds = %thenBB315.i.i, %SyncBB240.i.i
  %CurrWI..10.i.i = phi i64 [ %"CurrWI++319.i.i", %thenBB315.i.i ], [ 0, %SyncBB240.i.i ]
  %CurrSBIndex..10.i.i = phi i64 [ %"loadedCurrSB+Stride321.i.i", %thenBB315.i.i ], [ 0, %SyncBB240.i.i ]
  %"&(pSB[currWI].offset)43.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 88
  %"&pSB[currWI].offset44.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)43.i.i"
  %CastToValueType45.i.i = bitcast i8* %"&pSB[currWI].offset44.i.i" to i32*
  %loadedValue46.i.i = load i32* %CastToValueType45.i.i, align 4
  %135 = add nsw i32 %loadedValue46.i.i, -16
  %136 = sext i32 %135 to i64
  %137 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %136
  %138 = load double addrspace(3)* %137, align 8
  %"&(pSB[currWI].offset)197.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 144
  %"&pSB[currWI].offset198.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)197.i.i"
  %CastToValueType199.i.i = bitcast i8* %"&pSB[currWI].offset198.i.i" to double*
  store double %138, double* %CastToValueType199.i.i, align 8
  %check.WI.iter318.i.i = icmp ult i64 %CurrWI..10.i.i, %34
  br i1 %check.WI.iter318.i.i, label %thenBB315.i.i, label %SyncBB242.i.i

thenBB315.i.i:                                    ; preds = %SyncBB241.i.i
  %"CurrWI++319.i.i" = add nuw i64 %CurrWI..10.i.i, 1
  %"loadedCurrSB+Stride321.i.i" = add nuw i64 %CurrSBIndex..10.i.i, 192
  br label %SyncBB241.i.i

SyncBB242.i.i:                                    ; preds = %thenBB322.i.i, %SyncBB241.i.i
  %CurrWI..11.i.i = phi i64 [ %"CurrWI++326.i.i", %thenBB322.i.i ], [ 0, %SyncBB241.i.i ]
  %CurrSBIndex..11.i.i = phi i64 [ %"loadedCurrSB+Stride328.i.i", %thenBB322.i.i ], [ 0, %SyncBB241.i.i ]
  %"&(pSB[currWI].offset)107.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 96
  %"&pSB[currWI].offset108.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)107.i.i"
  %CastToValueType109.i.i = bitcast i8* %"&pSB[currWI].offset108.i.i" to double addrspace(3)**
  %loadedValue110.i.i = load double addrspace(3)** %CastToValueType109.i.i, align 8
  %139 = load double addrspace(3)* %loadedValue110.i.i, align 8
  %"&(pSB[currWI].offset)201.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 144
  %"&pSB[currWI].offset202.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)201.i.i"
  %CastToValueType203.i.i = bitcast i8* %"&pSB[currWI].offset202.i.i" to double*
  %loadedValue204.i.i = load double* %CastToValueType203.i.i, align 8
  %140 = fadd double %139, %loadedValue204.i.i
  %"&(pSB[currWI].offset)102.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 96
  %"&pSB[currWI].offset103.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)102.i.i"
  %CastToValueType104.i.i = bitcast i8* %"&pSB[currWI].offset103.i.i" to double addrspace(3)**
  %loadedValue105.i.i = load double addrspace(3)** %CastToValueType104.i.i, align 8
  store double %140, double addrspace(3)* %loadedValue105.i.i, align 8
  %check.WI.iter325.i.i = icmp ult i64 %CurrWI..11.i.i, %34
  br i1 %check.WI.iter325.i.i, label %thenBB322.i.i, label %SyncBB243.i.i

thenBB322.i.i:                                    ; preds = %SyncBB242.i.i
  %"CurrWI++326.i.i" = add nuw i64 %CurrWI..11.i.i, 1
  %"loadedCurrSB+Stride328.i.i" = add nuw i64 %CurrSBIndex..11.i.i, 192
  br label %SyncBB242.i.i

SyncBB243.i.i:                                    ; preds = %thenBB329.i.i, %SyncBB242.i.i
  %CurrWI..12.i.i = phi i64 [ %"CurrWI++333.i.i", %thenBB329.i.i ], [ 0, %SyncBB242.i.i ]
  %CurrSBIndex..12.i.i = phi i64 [ %"loadedCurrSB+Stride335.i.i", %thenBB329.i.i ], [ 0, %SyncBB242.i.i ]
  %"&(pSB[currWI].offset)38.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 88
  %"&pSB[currWI].offset39.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)38.i.i"
  %CastToValueType40.i.i = bitcast i8* %"&pSB[currWI].offset39.i.i" to i32*
  %loadedValue41.i.i = load i32* %CastToValueType40.i.i, align 4
  %141 = add nsw i32 %loadedValue41.i.i, -32
  %142 = sext i32 %141 to i64
  %143 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %142
  %144 = load double addrspace(3)* %143, align 8
  %"&(pSB[currWI].offset)206.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 152
  %"&pSB[currWI].offset207.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)206.i.i"
  %CastToValueType208.i.i = bitcast i8* %"&pSB[currWI].offset207.i.i" to double*
  store double %144, double* %CastToValueType208.i.i, align 8
  %check.WI.iter332.i.i = icmp ult i64 %CurrWI..12.i.i, %34
  br i1 %check.WI.iter332.i.i, label %thenBB329.i.i, label %SyncBB244.i.i

thenBB329.i.i:                                    ; preds = %SyncBB243.i.i
  %"CurrWI++333.i.i" = add nuw i64 %CurrWI..12.i.i, 1
  %"loadedCurrSB+Stride335.i.i" = add nuw i64 %CurrSBIndex..12.i.i, 192
  br label %SyncBB243.i.i

SyncBB244.i.i:                                    ; preds = %thenBB336.i.i, %SyncBB243.i.i
  %CurrWI..13.i.i = phi i64 [ %"CurrWI++340.i.i", %thenBB336.i.i ], [ 0, %SyncBB243.i.i ]
  %CurrSBIndex..13.i.i = phi i64 [ %"loadedCurrSB+Stride342.i.i", %thenBB336.i.i ], [ 0, %SyncBB243.i.i ]
  %"&(pSB[currWI].offset)97.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 96
  %"&pSB[currWI].offset98.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)97.i.i"
  %CastToValueType99.i.i = bitcast i8* %"&pSB[currWI].offset98.i.i" to double addrspace(3)**
  %loadedValue100.i.i = load double addrspace(3)** %CastToValueType99.i.i, align 8
  %145 = load double addrspace(3)* %loadedValue100.i.i, align 8
  %"&(pSB[currWI].offset)210.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 152
  %"&pSB[currWI].offset211.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)210.i.i"
  %CastToValueType212.i.i = bitcast i8* %"&pSB[currWI].offset211.i.i" to double*
  %loadedValue213.i.i = load double* %CastToValueType212.i.i, align 8
  %146 = fadd double %145, %loadedValue213.i.i
  %"&(pSB[currWI].offset)92.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 96
  %"&pSB[currWI].offset93.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)92.i.i"
  %CastToValueType94.i.i = bitcast i8* %"&pSB[currWI].offset93.i.i" to double addrspace(3)**
  %loadedValue95.i.i = load double addrspace(3)** %CastToValueType94.i.i, align 8
  store double %146, double addrspace(3)* %loadedValue95.i.i, align 8
  %check.WI.iter339.i.i = icmp ult i64 %CurrWI..13.i.i, %34
  br i1 %check.WI.iter339.i.i, label %thenBB336.i.i, label %SyncBB245.i.i

thenBB336.i.i:                                    ; preds = %SyncBB244.i.i
  %"CurrWI++340.i.i" = add nuw i64 %CurrWI..13.i.i, 1
  %"loadedCurrSB+Stride342.i.i" = add nuw i64 %CurrSBIndex..13.i.i, 192
  br label %SyncBB244.i.i

SyncBB245.i.i:                                    ; preds = %thenBB343.i.i, %SyncBB244.i.i
  %CurrWI..14.i.i = phi i64 [ %"CurrWI++347.i.i", %thenBB343.i.i ], [ 0, %SyncBB244.i.i ]
  %CurrSBIndex..14.i.i = phi i64 [ %"loadedCurrSB+Stride349.i.i", %thenBB343.i.i ], [ 0, %SyncBB244.i.i ]
  %"&(pSB[currWI].offset)33.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 88
  %"&pSB[currWI].offset34.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)33.i.i"
  %CastToValueType35.i.i = bitcast i8* %"&pSB[currWI].offset34.i.i" to i32*
  %loadedValue36.i.i = load i32* %CastToValueType35.i.i, align 4
  %147 = add nsw i32 %loadedValue36.i.i, -64
  %148 = sext i32 %147 to i64
  %149 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %148
  %150 = load double addrspace(3)* %149, align 8
  %"&(pSB[currWI].offset)215.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 160
  %"&pSB[currWI].offset216.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)215.i.i"
  %CastToValueType217.i.i = bitcast i8* %"&pSB[currWI].offset216.i.i" to double*
  store double %150, double* %CastToValueType217.i.i, align 8
  %check.WI.iter346.i.i = icmp ult i64 %CurrWI..14.i.i, %34
  br i1 %check.WI.iter346.i.i, label %thenBB343.i.i, label %SyncBB246.i.i

thenBB343.i.i:                                    ; preds = %SyncBB245.i.i
  %"CurrWI++347.i.i" = add nuw i64 %CurrWI..14.i.i, 1
  %"loadedCurrSB+Stride349.i.i" = add nuw i64 %CurrSBIndex..14.i.i, 192
  br label %SyncBB245.i.i

SyncBB246.i.i:                                    ; preds = %thenBB350.i.i, %SyncBB245.i.i
  %CurrWI..15.i.i = phi i64 [ %"CurrWI++354.i.i", %thenBB350.i.i ], [ 0, %SyncBB245.i.i ]
  %CurrSBIndex..15.i.i = phi i64 [ %"loadedCurrSB+Stride356.i.i", %thenBB350.i.i ], [ 0, %SyncBB245.i.i ]
  %"&(pSB[currWI].offset)87.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 96
  %"&pSB[currWI].offset88.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)87.i.i"
  %CastToValueType89.i.i = bitcast i8* %"&pSB[currWI].offset88.i.i" to double addrspace(3)**
  %loadedValue90.i.i = load double addrspace(3)** %CastToValueType89.i.i, align 8
  %151 = load double addrspace(3)* %loadedValue90.i.i, align 8
  %"&(pSB[currWI].offset)219.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 160
  %"&pSB[currWI].offset220.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)219.i.i"
  %CastToValueType221.i.i = bitcast i8* %"&pSB[currWI].offset220.i.i" to double*
  %loadedValue222.i.i = load double* %CastToValueType221.i.i, align 8
  %152 = fadd double %151, %loadedValue222.i.i
  %"&(pSB[currWI].offset)82.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 96
  %"&pSB[currWI].offset83.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)82.i.i"
  %CastToValueType84.i.i = bitcast i8* %"&pSB[currWI].offset83.i.i" to double addrspace(3)**
  %loadedValue85.i.i = load double addrspace(3)** %CastToValueType84.i.i, align 8
  store double %152, double addrspace(3)* %loadedValue85.i.i, align 8
  %check.WI.iter353.i.i = icmp ult i64 %CurrWI..15.i.i, %34
  br i1 %check.WI.iter353.i.i, label %thenBB350.i.i, label %SyncBB247.i.i

thenBB350.i.i:                                    ; preds = %SyncBB246.i.i
  %"CurrWI++354.i.i" = add nuw i64 %CurrWI..15.i.i, 1
  %"loadedCurrSB+Stride356.i.i" = add nuw i64 %CurrSBIndex..15.i.i, 192
  br label %SyncBB246.i.i

SyncBB247.i.i:                                    ; preds = %thenBB364.i.i, %SyncBB246.i.i
  %CurrWI..16.i.i = phi i64 [ %"CurrWI++368.i.i", %thenBB364.i.i ], [ 0, %SyncBB246.i.i ]
  %CurrSBIndex..16.i.i = phi i64 [ %"loadedCurrSB+Stride370.i.i", %thenBB364.i.i ], [ 0, %SyncBB246.i.i ]
  %"&(pSB[currWI].offset)28.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 88
  %"&pSB[currWI].offset29.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)28.i.i"
  %CastToValueType30.i.i = bitcast i8* %"&pSB[currWI].offset29.i.i" to i32*
  %loadedValue31.i.i = load i32* %CastToValueType30.i.i, align 4
  %153 = add nsw i32 %loadedValue31.i.i, -128
  %154 = sext i32 %153 to i64
  %155 = getelementptr inbounds [512 x double] addrspace(3)* %100, i64 0, i64 %154
  %156 = load double addrspace(3)* %155, align 8
  %"&(pSB[currWI].offset)224.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 168
  %"&pSB[currWI].offset225.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)224.i.i"
  %CastToValueType226.i.i = bitcast i8* %"&pSB[currWI].offset225.i.i" to double*
  store double %156, double* %CastToValueType226.i.i, align 8
  %check.WI.iter367.i.i = icmp ult i64 %CurrWI..16.i.i, %34
  br i1 %check.WI.iter367.i.i, label %thenBB364.i.i, label %SyncBB249.i.i

thenBB364.i.i:                                    ; preds = %SyncBB247.i.i
  %"CurrWI++368.i.i" = add nuw i64 %CurrWI..16.i.i, 1
  %"loadedCurrSB+Stride370.i.i" = add nuw i64 %CurrSBIndex..16.i.i, 192
  br label %SyncBB247.i.i

SyncBB249.i.i:                                    ; preds = %thenBB371.i.i, %SyncBB247.i.i
  %CurrWI..17.i.i = phi i64 [ %"CurrWI++375.i.i", %thenBB371.i.i ], [ 0, %SyncBB247.i.i ]
  %CurrSBIndex..17.i.i = phi i64 [ %"loadedCurrSB+Stride377.i.i", %thenBB371.i.i ], [ 0, %SyncBB247.i.i ]
  %"&(pSB[currWI].offset)77.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 96
  %"&pSB[currWI].offset78.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)77.i.i"
  %CastToValueType79.i.i = bitcast i8* %"&pSB[currWI].offset78.i.i" to double addrspace(3)**
  %loadedValue80.i.i = load double addrspace(3)** %CastToValueType79.i.i, align 8
  %157 = load double addrspace(3)* %loadedValue80.i.i, align 8
  %"&(pSB[currWI].offset)228.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 168
  %"&pSB[currWI].offset229.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)228.i.i"
  %CastToValueType230.i.i = bitcast i8* %"&pSB[currWI].offset229.i.i" to double*
  %loadedValue231.i.i = load double* %CastToValueType230.i.i, align 8
  %158 = fadd double %157, %loadedValue231.i.i
  %"&(pSB[currWI].offset)72.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 96
  %"&pSB[currWI].offset73.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)72.i.i"
  %CastToValueType74.i.i = bitcast i8* %"&pSB[currWI].offset73.i.i" to double addrspace(3)**
  %loadedValue75.i.i = load double addrspace(3)** %CastToValueType74.i.i, align 8
  store double %158, double addrspace(3)* %loadedValue75.i.i, align 8
  %check.WI.iter374.i.i = icmp ult i64 %CurrWI..17.i.i, %34
  br i1 %check.WI.iter374.i.i, label %thenBB371.i.i, label %SyncBB250.i.i

thenBB371.i.i:                                    ; preds = %SyncBB249.i.i
  %"CurrWI++375.i.i" = add nuw i64 %CurrWI..17.i.i, 1
  %"loadedCurrSB+Stride377.i.i" = add nuw i64 %CurrSBIndex..17.i.i, 192
  br label %SyncBB249.i.i

SyncBB250.i.i:                                    ; preds = %thenBB357.i.i, %SyncBB249.i.i
  %CurrWI..18.i.i = phi i64 [ %"CurrWI++361.i.i", %thenBB357.i.i ], [ 0, %SyncBB249.i.i ]
  %CurrSBIndex..18.i.i = phi i64 [ %"loadedCurrSB+Stride363.i.i", %thenBB357.i.i ], [ 0, %SyncBB249.i.i ]
  %"&(pSB[currWI].offset)156.i.i" = add nuw i64 %CurrSBIndex..18.i.i, 104
  %"&pSB[currWI].offset157.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)156.i.i"
  %CastToValueType158.i.i = bitcast i8* %"&pSB[currWI].offset157.i.i" to double addrspace(3)**
  %loadedValue159.i.i = load double addrspace(3)** %CastToValueType158.i.i, align 8
  %159 = load double addrspace(3)* %loadedValue159.i.i, align 8
  %"&(pSB[currWI].offset)8.i.i" = add nuw i64 %CurrSBIndex..18.i.i, 72
  %"&pSB[currWI].offset9.i.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)8.i.i"
  %CastToArgType10.i.i = bitcast i8* %"&pSB[currWI].offset9.i.i" to double*
  store double %159, double* %CastToArgType10.i.i, align 8
  %check.WI.iter360.i.i = icmp ult i64 %CurrWI..18.i.i, %34
  br i1 %check.WI.iter360.i.i, label %thenBB357.i.i, label %"Barrier BB3.i"

thenBB357.i.i:                                    ; preds = %SyncBB250.i.i
  %"CurrWI++361.i.i" = add nuw i64 %CurrWI..18.i.i, 1
  %"loadedCurrSB+Stride363.i.i" = add nuw i64 %CurrSBIndex..18.i.i, 192
  br label %SyncBB250.i.i

"Barrier BB3.i":                                  ; preds = %SyncBB250.i.i
  store i64 0, i64* %40, align 8
  %160 = icmp eq i32 %16, 0
  %161 = bitcast double addrspace(1)* %1 to <4 x double> addrspace(1)*
  br label %SyncBB142.i

SyncBB142.i:                                      ; preds = %thenBB.i, %"Barrier BB3.i"
  %CurrSBIndex..0.i = phi i64 [ 0, %"Barrier BB3.i" ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  br i1 %160, label %phi-split-bb.i, label %162

; <label>:162                                     ; preds = %SyncBB142.i
  %"&(pSB[currWI].offset)187.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset19.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)187.i"
  %CastToValueType20.i = bitcast i8* %"&pSB[currWI].offset19.i" to i64*
  %loadedValue21.i = load i64* %CastToValueType20.i, align 8
  %sext.i = shl i64 %loadedValue21.i, 32
  %163 = ashr i64 %sext.i, 32
  %164 = getelementptr %struct.WorkDim* %22, i64 0, i32 3, i64 0
  %165 = load i64* %164, align 8
  %166 = add i64 %165, -1
  %167 = icmp eq i64 %163, %166
  br i1 %167, label %168, label %phi-split-bb.i

; <label>:168                                     ; preds = %162
  %"&(pSB[currWI].offset)9617.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset97.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)9617.i"
  %CastToValueType98.i = bitcast i8* %"&pSB[currWI].offset97.i" to <4 x double>*
  %loadedValue99.i = load <4 x double>* %CastToValueType98.i, align 32
  %169 = extractelement <4 x double> %loadedValue99.i, i32 3
  %"&(pSB[currWI].offset)110.i" = add nuw i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset111.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)110.i"
  %CastToValueType112.i = bitcast i8* %"&pSB[currWI].offset111.i" to double*
  %loadedValue113.i = load double* %CastToValueType112.i, align 8
  %170 = fadd double %loadedValue113.i, %169
  %171 = load i64* %25, align 8
  %172 = getelementptr inbounds double addrspace(1)* %7, i64 %171
  store double %170, double addrspace(1)* %172, align 8
  br label %phi-split-bb.i

phi-split-bb.i:                                   ; preds = %168, %162, %SyncBB142.i
  %"&(pSB[currWI].offset)115.i" = add nuw i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset116.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)115.i"
  %CastToValueType117.i = bitcast i8* %"&pSB[currWI].offset116.i" to double*
  %loadedValue118.i = load double* %CastToValueType117.i, align 8
  %173 = insertelement <4 x double> undef, double %loadedValue118.i, i32 0
  %"&(pSB[currWI].offset)918.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset92.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)918.i"
  %CastToValueType93.i = bitcast i8* %"&pSB[currWI].offset92.i" to <4 x double>*
  %loadedValue94.i = load <4 x double>* %CastToValueType93.i, align 32
  %174 = extractelement <4 x double> %loadedValue94.i, i32 0
  %"&(pSB[currWI].offset)120.i" = add nuw i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset121.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)120.i"
  %CastToValueType122.i = bitcast i8* %"&pSB[currWI].offset121.i" to double*
  %loadedValue123.i = load double* %CastToValueType122.i, align 8
  %175 = fadd double %loadedValue123.i, %174
  %176 = insertelement <4 x double> %173, double %175, i32 1
  %"&(pSB[currWI].offset)869.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset87.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)869.i"
  %CastToValueType88.i = bitcast i8* %"&pSB[currWI].offset87.i" to <4 x double>*
  %loadedValue89.i = load <4 x double>* %CastToValueType88.i, align 32
  %177 = extractelement <4 x double> %loadedValue89.i, i32 1
  %"&(pSB[currWI].offset)125.i" = add nuw i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset126.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)125.i"
  %CastToValueType127.i = bitcast i8* %"&pSB[currWI].offset126.i" to double*
  %loadedValue128.i = load double* %CastToValueType127.i, align 8
  %178 = fadd double %loadedValue128.i, %177
  %179 = insertelement <4 x double> %176, double %178, i32 2
  %"&(pSB[currWI].offset)8110.i" = or i64 %CurrSBIndex..0.i, 32
  %"&pSB[currWI].offset82.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)8110.i"
  %CastToValueType83.i = bitcast i8* %"&pSB[currWI].offset82.i" to <4 x double>*
  %loadedValue84.i = load <4 x double>* %CastToValueType83.i, align 32
  %180 = extractelement <4 x double> %loadedValue84.i, i32 2
  %"&(pSB[currWI].offset)130.i" = add nuw i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset131.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)130.i"
  %CastToValueType132.i = bitcast i8* %"&pSB[currWI].offset131.i" to double*
  %loadedValue133.i = load double* %CastToValueType132.i, align 8
  %181 = fadd double %loadedValue133.i, %180
  %182 = insertelement <4 x double> %179, double %181, i32 3
  br i1 %42, label %183, label %186

; <label>:183                                     ; preds = %phi-split-bb.i
  %"&(pSB[currWI].offset)3712.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset38.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)3712.i"
  %CastToValueType39.i = bitcast i8* %"&pSB[currWI].offset38.i" to i32*
  %loadedValue40.i = load i32* %CastToValueType39.i, align 4
  %184 = or i32 %loadedValue40.i, 3
  %185 = icmp slt i32 %184, %10
  br i1 %185, label %186, label %189

; <label>:186                                     ; preds = %183, %phi-split-bb.i
  %"&(pSB[currWI].offset)911.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset10.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)911.i"
  %CastToValueType11.i = bitcast i8* %"&pSB[currWI].offset10.i" to i32*
  %loadedValue12.i = load i32* %CastToValueType11.i, align 4
  %187 = sext i32 %loadedValue12.i to i64
  %188 = getelementptr inbounds <4 x double> addrspace(1)* %161, i64 %187
  store <4 x double> %182, <4 x double> addrspace(1)* %188, align 32
  br label %UnifiedReturnBlock.i

; <label>:189                                     ; preds = %183
  %"&(pSB[currWI].offset)6213.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset63.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)6213.i"
  %CastToValueType64.i = bitcast i8* %"&pSB[currWI].offset63.i" to i32*
  %loadedValue65.i = load i32* %CastToValueType64.i, align 4
  %190 = icmp slt i32 %loadedValue65.i, %10
  br i1 %190, label %191, label %UnifiedReturnBlock.i

; <label>:191                                     ; preds = %189
  %"&(pSB[currWI].offset)5714.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset58.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)5714.i"
  %CastToValueType59.i = bitcast i8* %"&pSB[currWI].offset58.i" to i32*
  %loadedValue60.i = load i32* %CastToValueType59.i, align 4
  %192 = sext i32 %loadedValue60.i to i64
  %193 = getelementptr inbounds double addrspace(1)* %1, i64 %192
  %"&(pSB[currWI].offset)135.i" = add nuw i64 %CurrSBIndex..0.i, 72
  %"&pSB[currWI].offset136.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)135.i"
  %CastToValueType137.i = bitcast i8* %"&pSB[currWI].offset136.i" to double*
  %loadedValue138.i = load double* %CastToValueType137.i, align 8
  store double %loadedValue138.i, double addrspace(1)* %193, align 8
  %"&(pSB[currWI].offset)3215.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset33.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)3215.i"
  %CastToValueType34.i = bitcast i8* %"&pSB[currWI].offset33.i" to i32*
  %loadedValue35.i = load i32* %CastToValueType34.i, align 4
  %194 = or i32 %loadedValue35.i, 1
  %195 = icmp slt i32 %194, %10
  br i1 %195, label %196, label %UnifiedReturnBlock.i

; <label>:196                                     ; preds = %191
  %197 = sext i32 %194 to i64
  %198 = getelementptr inbounds double addrspace(1)* %1, i64 %197
  store double %175, double addrspace(1)* %198, align 8
  %"&(pSB[currWI].offset)2716.i" = or i64 %CurrSBIndex..0.i, 24
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %37, i64 %"&(pSB[currWI].offset)2716.i"
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i32*
  %loadedValue30.i = load i32* %CastToValueType29.i, align 4
  %199 = or i32 %loadedValue30.i, 2
  %200 = icmp slt i32 %199, %10
  br i1 %200, label %201, label %UnifiedReturnBlock.i

; <label>:201                                     ; preds = %196
  %202 = sext i32 %199 to i64
  %203 = getelementptr inbounds double addrspace(1)* %1, i64 %202
  store double %178, double addrspace(1)* %203, align 8
  br label %UnifiedReturnBlock.i

UnifiedReturnBlock.i:                             ; preds = %201, %196, %191, %189, %186
  %loadedCurrWI.i = load i64* %40, align 8
  %check.WI.iter.i = icmp ult i64 %loadedCurrWI.i, %34
  br i1 %check.WI.iter.i, label %thenBB.i, label %__scan_separated_args.exit

thenBB.i:                                         ; preds = %UnifiedReturnBlock.i
  %"CurrWI++.i" = add nuw i64 %loadedCurrWI.i, 1
  store i64 %"CurrWI++.i", i64* %40, align 8
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 192
  br label %SyncBB142.i

__scan_separated_args.exit:                       ; preds = %UnifiedReturnBlock.i
  store i64 0, i64* %40, align 8
  ret void
}

define void @addUniform(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to double addrspace(1)**
  %4 = load double addrspace(1)** %3, align 8
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
  %26 = bitcast i8 addrspace(3)* %10 to double addrspace(3)*
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %27 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = icmp eq i64 %28, 0
  br i1 %29, label %30, label %"Barrier BB.i"

; <label>:30                                      ; preds = %SyncBB.i
  %31 = load i64* %16, align 8
  %32 = getelementptr inbounds double addrspace(1)* %4, i64 %31
  %33 = load double addrspace(1)* %32, align 8
  store double %33, double addrspace(3)* %26, align 8
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %30, %SyncBB.i
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
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB5.i

thenBB.i:                                         ; preds = %"Barrier BB.i"
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 192
  br label %SyncBB.i

SyncBB5.i:                                        ; preds = %thenBB8.i, %"Barrier BB.i"
  %CurrWI..1.i = phi i64 [ %"CurrWI++12.i", %thenBB8.i ], [ 0, %"Barrier BB.i" ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride14.i", %thenBB8.i ], [ 0, %"Barrier BB.i" ]
  %"&pSB[currWI].offset3.i" = getelementptr inbounds i8* %25, i64 %CurrSBIndex..1.i
  %CastToValueType4.i = bitcast i8* %"&pSB[currWI].offset3.i" to i64*
  %loadedValue.i = load i64* %CastToValueType4.i, align 8
  br label %42

; <label>:42                                      ; preds = %47, %SyncBB5.i
  %address.0.in.i = phi i64 [ %loadedValue.i, %SyncBB5.i ], [ %55, %47 ]
  %43 = phi i32 [ 0, %SyncBB5.i ], [ %56, %47 ]
  %44 = icmp slt i32 %43, 4
  br i1 %44, label %45, label %.critedge.i

; <label>:45                                      ; preds = %42
  %address.0.i = trunc i64 %address.0.in.i to i32
  %46 = icmp ult i32 %address.0.i, %7
  br i1 %46, label %47, label %.critedge.i

; <label>:47                                      ; preds = %45
  %48 = load double addrspace(3)* %26, align 8
  %49 = and i64 %address.0.in.i, 4294967295
  %50 = getelementptr inbounds double addrspace(1)* %1, i64 %49
  %51 = load double addrspace(1)* %50, align 8
  %52 = fadd double %51, %48
  store double %52, double addrspace(1)* %50, align 8
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
  %"loadedCurrSB+Stride14.i" = add nuw i64 %CurrSBIndex..1.i, 192
  br label %SyncBB5.i

__addUniform_separated_args.exit:                 ; preds = %.critedge.i
  ret void
}

!opencl.kernels = !{!0, !2}
!opencl_addUniform_locals_anchor = !{!3}
!opencl_scan_locals_anchor = !{!4}

!0 = metadata !{void (double addrspace(1)*, double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__addUniform_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, double const __attribute__((address_space(1))) *, int const", metadata !"opencl_addUniform_locals_anchor", void (i8*)* @addUniform}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (double addrspace(1)*, double addrspace(1)*, double addrspace(1)*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__scan_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, double __attribute__((address_space(1))) *, double __attribute__((address_space(1))) *, int const, int const, int const", metadata !"opencl_scan_locals_anchor", void (i8*)* @scan}
!3 = metadata !{metadata !"opencl_addUniform_local_uni"}
!4 = metadata !{metadata !"opencl_scan_local_s_data"}


