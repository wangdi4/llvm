; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__transpose_kernel_original(<4 x i8> addrspace(1)* nocapture, <4 x i8> addrspace(1)* nocapture, <4 x i8> addrspace(3)* nocapture, i32, i32, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_local_id(i32)

declare void @barrier(i64)

declare void @__RecursiveGaussian_kernel_original(<4 x i8> addrspace(1)* nocapture, <4 x i8> addrspace(1)* nocapture, i32, i32, float, float, float, float, float, float, float, float) nounwind

declare void @____Vectorized_.transpose_kernel_original(<4 x i8> addrspace(1)* nocapture, <4 x i8> addrspace(1)* nocapture, <4 x i8> addrspace(3)* nocapture, i32, i32, i32) nounwind

declare void @____Vectorized_.RecursiveGaussian_kernel_original(<4 x i8> addrspace(1)* nocapture, <4 x i8> addrspace(1)* nocapture, i32, i32, float, float, float, float, float, float, float, float) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

declare <4 x i8> @masked_load_align4_0(i1, <4 x i8> addrspace(1)*)

declare void @masked_store_align4_0(i1, <4 x i8>, <4 x i8> addrspace(1)*)

declare void @maskedf_0_barrier(i1, i64)

declare <4 x i8> @masked_load_align4_1(i1, <4 x i8> addrspace(1)*)

declare <4 x i8> @masked_load_align4_2(i1, <4 x i8> addrspace(1)*)

declare void @masked_store_align4_1(i1, <4 x i8>, <4 x i8> addrspace(1)*)

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

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare void @llvm.x86.sse2.mfence() nounwind

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__transpose_kernel_separated_args(<4 x i8> addrspace(1)* nocapture %output, <4 x i8> addrspace(1)* nocapture %input, <4 x i8> addrspace(3)* nocapture %block, i32 %width, i32 %height, i32 %blockSize, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB24

SyncBB24:                                         ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %6, i32* %CastToValueType, align 4
  %7 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %8 = load i64* %7, align 8
  %9 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %10 = load i64* %9, align 8
  %11 = add i64 %8, %10
  %12 = trunc i64 %11 to i32
  %"&(pSB[currWI].offset)71" = or i64 %CurrSBIndex..0, 4
  %"&pSB[currWI].offset8" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)71"
  %CastToValueType9 = bitcast i8* %"&pSB[currWI].offset8" to i32*
  store i32 %12, i32* %CastToValueType9, align 4
  %13 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %14 = load i64* %13, align 8
  %15 = trunc i64 %14 to i32
  %16 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %17 = load i64* %16, align 8
  %18 = trunc i64 %17 to i32
  %19 = mul i32 %12, %width
  %20 = add i32 %19, %6
  %21 = zext i32 %20 to i64
  %22 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %21
  %23 = load <4 x i8> addrspace(1)* %22, align 4
  %24 = mul i32 %18, %blockSize
  %25 = add i32 %24, %15
  %26 = zext i32 %25 to i64
  %27 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %26
  %"&(pSB[currWI].offset)162" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset17" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)162"
  %CastToValueType18 = bitcast i8* %"&pSB[currWI].offset17" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %27, <4 x i8> addrspace(3)** %CastToValueType18, align 8
  store <4 x i8> %23, <4 x i8> addrspace(3)* %27, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %SyncBB24
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 448
  br label %SyncBB24

SyncBB:                                           ; preds = %SyncBB24, %thenBB27
  %CurrWI..1 = phi i64 [ %"CurrWI++31", %thenBB27 ], [ 0, %SyncBB24 ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride33", %thenBB27 ], [ 0, %SyncBB24 ]
  %"&pSB[currWI].offset4" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType5 = bitcast i8* %"&pSB[currWI].offset4" to i32*
  %loadedValue = load i32* %CastToValueType5, align 4
  %28 = mul i32 %loadedValue, %height
  %"&(pSB[currWI].offset)113" = or i64 %CurrSBIndex..1, 4
  %"&pSB[currWI].offset12" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)113"
  %CastToValueType13 = bitcast i8* %"&pSB[currWI].offset12" to i32*
  %loadedValue14 = load i32* %CastToValueType13, align 4
  %29 = add i32 %loadedValue14, %28
  %"&(pSB[currWI].offset)204" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset21" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)204"
  %CastToValueType22 = bitcast i8* %"&pSB[currWI].offset21" to <4 x i8> addrspace(3)**
  %loadedValue23 = load <4 x i8> addrspace(3)** %CastToValueType22, align 8
  %30 = load <4 x i8> addrspace(3)* %loadedValue23, align 4
  %31 = zext i32 %29 to i64
  %32 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %31
  store <4 x i8> %30, <4 x i8> addrspace(1)* %32, align 4
  %check.WI.iter30 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter30, label %thenBB27, label %SyncBB25

thenBB27:                                         ; preds = %SyncBB
  %"CurrWI++31" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride33" = add nuw i64 %CurrSBIndex..1, 448
  br label %SyncBB

SyncBB25:                                         ; preds = %SyncBB
  ret void
}

define void @__RecursiveGaussian_kernel_separated_args(<4 x i8> addrspace(1)* nocapture %input, <4 x i8> addrspace(1)* nocapture %output, i32 %width, i32 %height, float %a0, float %a1, float %a2, float %a3, float %b1, float %b2, float %coefp, float %coefn, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %height, 0
  %1 = insertelement <4 x float> undef, float %a0, i32 0
  %2 = shufflevector <4 x float> %1, <4 x float> undef, <4 x i32> zeroinitializer
  %3 = insertelement <4 x float> undef, float %a1, i32 0
  %4 = shufflevector <4 x float> %3, <4 x float> undef, <4 x i32> zeroinitializer
  %5 = insertelement <4 x float> undef, float %b1, i32 0
  %6 = shufflevector <4 x float> %5, <4 x float> undef, <4 x i32> zeroinitializer
  %7 = insertelement <4 x float> undef, float %b2, i32 0
  %8 = shufflevector <4 x float> %7, <4 x float> undef, <4 x i32> zeroinitializer
  %y1.01 = add i32 %height, -1
  %9 = icmp sgt i32 %y1.01, -1
  %10 = insertelement <4 x float> undef, float %a2, i32 0
  %11 = shufflevector <4 x float> %10, <4 x float> undef, <4 x i32> zeroinitializer
  %12 = insertelement <4 x float> undef, float %a3, i32 0
  %13 = shufflevector <4 x float> %12, <4 x float> undef, <4 x i32> zeroinitializer
  %14 = insertelement <4 x float> undef, float %b1, i32 0
  %15 = shufflevector <4 x float> %14, <4 x float> undef, <4 x i32> zeroinitializer
  %16 = insertelement <4 x float> undef, float %b2, i32 0
  %17 = shufflevector <4 x float> %16, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp = sub i32 0, %width
  %tmp14 = mul i32 %y1.01, %width
  br label %SyncBB31.outer

SyncBB31.outer:                                   ; preds = %thenBB34, %FirstBB
  %CurrWI..0.ph = phi i64 [ 0, %FirstBB ], [ %"CurrWI++38", %thenBB34 ]
  %CurrSBIndex..0.ph = phi i64 [ 0, %FirstBB ], [ %"loadedCurrSB+Stride40", %thenBB34 ]
  %currBarrier.2.ph = phi i32 [ 9, %FirstBB ], [ %currBarrier.1, %thenBB34 ]
  br label %SyncBB31

SyncBB31:                                         ; preds = %thenBB, %SyncBB31.outer
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ %CurrWI..0.ph, %SyncBB31.outer ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ %CurrSBIndex..0.ph, %SyncBB31.outer ]
  %18 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = add i64 %19, %21
  %23 = trunc i64 %22 to i32
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %23, i32* %CastToValueType, align 4
  %24 = icmp ult i32 %23, %width
  br i1 %24, label %.preheader, label %.loopexit

.preheader:                                       ; preds = %SyncBB31
  br i1 %0, label %bb.nph11, label %._crit_edge

bb.nph11:                                         ; preds = %.preheader
  %"&(pSB[currWI].offset)27" = add nuw i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset28" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)27"
  %CastToValueType29 = bitcast i8* %"&pSB[currWI].offset28" to i32*
  br label %25

; <label>:25                                      ; preds = %25, %bb.nph11
  %y.010 = phi i32 [ 0, %bb.nph11 ], [ %61, %25 ]
  %yb.09 = phi <4 x float> [ zeroinitializer, %bb.nph11 ], [ %yp.08, %25 ]
  %yp.08 = phi <4 x float> [ zeroinitializer, %bb.nph11 ], [ %47, %25 ]
  %xp.07 = phi <4 x float> [ zeroinitializer, %bb.nph11 ], [ %40, %25 ]
  %tmp19 = mul i32 %y.010, %width
  %loadedValue30 = load i32* %CastToValueType29, align 4
  %tmp21 = add i32 %loadedValue30, %tmp19
  %26 = sext i32 %tmp21 to i64
  %27 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %26
  %28 = load <4 x i8> addrspace(1)* %27, align 4
  %29 = extractelement <4 x i8> %28, i32 0
  %30 = uitofp i8 %29 to float
  %31 = insertelement <4 x float> undef, float %30, i32 0
  %32 = extractelement <4 x i8> %28, i32 1
  %33 = uitofp i8 %32 to float
  %34 = insertelement <4 x float> %31, float %33, i32 1
  %35 = extractelement <4 x i8> %28, i32 2
  %36 = uitofp i8 %35 to float
  %37 = insertelement <4 x float> %34, float %36, i32 2
  %38 = extractelement <4 x i8> %28, i32 3
  %39 = uitofp i8 %38 to float
  %40 = insertelement <4 x float> %37, float %39, i32 3
  %41 = fmul <4 x float> %2, %40
  %42 = fmul <4 x float> %4, %xp.07
  %43 = fadd <4 x float> %41, %42
  %44 = fmul <4 x float> %6, %yp.08
  %45 = fsub <4 x float> %43, %44
  %46 = fmul <4 x float> %8, %yb.09
  %47 = fsub <4 x float> %45, %46
  %48 = extractelement <4 x float> %47, i32 0
  %49 = fptoui float %48 to i8
  %50 = insertelement <4 x i8> undef, i8 %49, i32 0
  %51 = extractelement <4 x float> %47, i32 1
  %52 = fptoui float %51 to i8
  %53 = insertelement <4 x i8> %50, i8 %52, i32 1
  %54 = extractelement <4 x float> %47, i32 2
  %55 = fptoui float %54 to i8
  %56 = insertelement <4 x i8> %53, i8 %55, i32 2
  %57 = extractelement <4 x float> %47, i32 3
  %58 = fptoui float %57 to i8
  %59 = insertelement <4 x i8> %56, i8 %58, i32 3
  %60 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %26
  store <4 x i8> %59, <4 x i8> addrspace(1)* %60, align 4
  %61 = add nsw i32 %y.010, 1
  %exitcond18 = icmp eq i32 %61, %height
  br i1 %exitcond18, label %._crit_edge, label %25

._crit_edge:                                      ; preds = %25, %.preheader
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 448
  br label %SyncBB31

elseBB:                                           ; preds = %._crit_edge
  call void @llvm.x86.sse2.mfence()
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB34, %elseBB
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++38", %thenBB34 ]
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride40", %thenBB34 ]
  %currBarrier.0 = phi i32 [ 2, %elseBB ], [ %currBarrier.1, %thenBB34 ]
  br i1 %9, label %bb.nph, label %.loopexit

bb.nph:                                           ; preds = %SyncBB
  %"&(pSB[currWI].offset)23" = add nuw i64 %CurrSBIndex..1, 16
  %"&pSB[currWI].offset24" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)23"
  %CastToValueType25 = bitcast i8* %"&pSB[currWI].offset24" to i32*
  %loadedValue = load i32* %CastToValueType25, align 4
  %tmp16 = add i32 %tmp14, %loadedValue
  br label %62

; <label>:62                                      ; preds = %62, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %indvar.next, %62 ]
  %ya.05 = phi <4 x float> [ zeroinitializer, %bb.nph ], [ %yn.04, %62 ]
  %yn.04 = phi <4 x float> [ zeroinitializer, %bb.nph ], [ %72, %62 ]
  %xa.03 = phi <4 x float> [ zeroinitializer, %bb.nph ], [ %xn.02, %62 ]
  %xn.02 = phi <4 x float> [ zeroinitializer, %bb.nph ], [ %111, %62 ]
  %tmp12 = mul i32 %indvar, %tmp
  %tmp17 = add i32 %tmp16, %tmp12
  %63 = sext i32 %tmp17 to i64
  %64 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %63
  %65 = load <4 x i8> addrspace(1)* %64, align 4
  %66 = fmul <4 x float> %11, %xn.02
  %67 = fmul <4 x float> %13, %xa.03
  %68 = fadd <4 x float> %66, %67
  %69 = fmul <4 x float> %15, %yn.04
  %70 = fsub <4 x float> %68, %69
  %71 = fmul <4 x float> %17, %ya.05
  %72 = fsub <4 x float> %70, %71
  %73 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %63
  %74 = load <4 x i8> addrspace(1)* %73, align 4
  %75 = extractelement <4 x i8> %74, i32 0
  %76 = uitofp i8 %75 to float
  %77 = insertelement <4 x float> undef, float %76, i32 0
  %78 = extractelement <4 x i8> %74, i32 1
  %79 = uitofp i8 %78 to float
  %80 = insertelement <4 x float> %77, float %79, i32 1
  %81 = extractelement <4 x i8> %74, i32 2
  %82 = uitofp i8 %81 to float
  %83 = insertelement <4 x float> %80, float %82, i32 2
  %84 = extractelement <4 x i8> %74, i32 3
  %85 = uitofp i8 %84 to float
  %86 = insertelement <4 x float> %83, float %85, i32 3
  %87 = fadd <4 x float> %86, %72
  %88 = extractelement <4 x float> %87, i32 0
  %89 = fptoui float %88 to i8
  %90 = insertelement <4 x i8> undef, i8 %89, i32 0
  %91 = extractelement <4 x float> %87, i32 1
  %92 = fptoui float %91 to i8
  %93 = insertelement <4 x i8> %90, i8 %92, i32 1
  %94 = extractelement <4 x float> %87, i32 2
  %95 = fptoui float %94 to i8
  %96 = insertelement <4 x i8> %93, i8 %95, i32 2
  %97 = extractelement <4 x float> %87, i32 3
  %98 = fptoui float %97 to i8
  %99 = insertelement <4 x i8> %96, i8 %98, i32 3
  store <4 x i8> %99, <4 x i8> addrspace(1)* %73, align 4
  %100 = extractelement <4 x i8> %65, i32 0
  %101 = extractelement <4 x i8> %65, i32 1
  %102 = uitofp i8 %100 to float
  %103 = extractelement <4 x i8> %65, i32 2
  %104 = uitofp i8 %101 to float
  %105 = insertelement <4 x float> undef, float %102, i32 0
  %106 = extractelement <4 x i8> %65, i32 3
  %107 = uitofp i8 %103 to float
  %108 = insertelement <4 x float> %105, float %104, i32 1
  %109 = uitofp i8 %106 to float
  %110 = insertelement <4 x float> %108, float %107, i32 2
  %111 = insertelement <4 x float> %110, float %109, i32 3
  %indvar.next = add i32 %indvar, 1
  %exitcond = icmp eq i32 %indvar.next, %height
  br i1 %exitcond, label %.loopexit, label %62

.loopexit:                                        ; preds = %SyncBB31, %SyncBB, %62
  %CurrWI..2 = phi i64 [ %CurrWI..1, %62 ], [ %CurrWI..1, %SyncBB ], [ %CurrWI..0, %SyncBB31 ]
  %CurrSBIndex..2 = phi i64 [ %CurrSBIndex..1, %62 ], [ %CurrSBIndex..1, %SyncBB ], [ %CurrSBIndex..0, %SyncBB31 ]
  %currBarrier.1 = phi i32 [ %currBarrier.0, %62 ], [ %currBarrier.0, %SyncBB ], [ %currBarrier.2.ph, %SyncBB31 ]
  %check.WI.iter37 = icmp ult i64 %CurrWI..2, %iterCount
  br i1 %check.WI.iter37, label %thenBB34, label %SyncBB32

thenBB34:                                         ; preds = %.loopexit
  %"CurrWI++38" = add nuw i64 %CurrWI..2, 1
  %"loadedCurrSB+Stride40" = add nuw i64 %CurrSBIndex..2, 448
  %cond = icmp eq i32 %currBarrier.1, 2
  br i1 %cond, label %SyncBB, label %SyncBB31.outer

SyncBB32:                                         ; preds = %.loopexit
  ret void
}

define void @____Vectorized_.transpose_kernel_separated_args(<4 x i8> addrspace(1)* nocapture %output, <4 x i8> addrspace(1)* nocapture %input, <4 x i8> addrspace(3)* nocapture %block, i32 %width, i32 %height, i32 %blockSize, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB216

SyncBB216:                                        ; preds = %0, %thenBB
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
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
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %9 = load i64* %8, align 8
  %10 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 1
  %11 = load i64* %10, align 8
  %12 = add i64 %9, %11
  %13 = trunc i64 %12 to i32
  %temp38 = insertelement <16 x i32> undef, i32 %13, i32 0
  %vector39 = shufflevector <16 x i32> %temp38, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)62" = add nuw i64 %CurrSBIndex..0, 128
  %"&pSB[currWI].offset63" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)62"
  %CastToValueType64 = bitcast i8* %"&pSB[currWI].offset63" to <16 x i32>*
  store <16 x i32> %vector39, <16 x i32>* %CastToValueType64, align 64
  %14 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %15 = load i64* %14, align 8
  %broadcast11 = insertelement <16 x i64> undef, i64 %15, i32 0
  %broadcast22 = shufflevector <16 x i64> %broadcast11, <16 x i64> undef, <16 x i32> zeroinitializer
  %16 = add <16 x i64> %broadcast22, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %17 = trunc <16 x i64> %16 to <16 x i32>
  %18 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 1
  %19 = load i64* %18, align 8
  %20 = trunc i64 %19 to i32
  %21 = mul i32 %13, %width
  %temp = insertelement <16 x i32> undef, i32 %21, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %22 = add <16 x i32> %vector, %7
  %23 = extractelement <16 x i32> %22, i32 0
  %24 = zext i32 %23 to i64
  %25 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %24
  %26 = extractelement <16 x i32> %22, i32 1
  %27 = zext i32 %26 to i64
  %28 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %27
  %29 = extractelement <16 x i32> %22, i32 2
  %30 = zext i32 %29 to i64
  %31 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %30
  %32 = extractelement <16 x i32> %22, i32 3
  %33 = zext i32 %32 to i64
  %34 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %33
  %35 = extractelement <16 x i32> %22, i32 4
  %36 = zext i32 %35 to i64
  %37 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %36
  %38 = extractelement <16 x i32> %22, i32 5
  %39 = zext i32 %38 to i64
  %40 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %39
  %41 = extractelement <16 x i32> %22, i32 6
  %42 = zext i32 %41 to i64
  %43 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %42
  %44 = extractelement <16 x i32> %22, i32 7
  %45 = zext i32 %44 to i64
  %46 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %45
  %47 = extractelement <16 x i32> %22, i32 8
  %48 = zext i32 %47 to i64
  %49 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %48
  %50 = extractelement <16 x i32> %22, i32 9
  %51 = zext i32 %50 to i64
  %52 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %51
  %53 = extractelement <16 x i32> %22, i32 10
  %54 = zext i32 %53 to i64
  %55 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %54
  %56 = extractelement <16 x i32> %22, i32 11
  %57 = zext i32 %56 to i64
  %58 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %57
  %59 = extractelement <16 x i32> %22, i32 12
  %60 = zext i32 %59 to i64
  %61 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %60
  %62 = extractelement <16 x i32> %22, i32 13
  %63 = zext i32 %62 to i64
  %64 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %63
  %65 = extractelement <16 x i32> %22, i32 14
  %66 = zext i32 %65 to i64
  %67 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %66
  %68 = extractelement <16 x i32> %22, i32 15
  %69 = zext i32 %68 to i64
  %70 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %69
  %71 = load <4 x i8> addrspace(1)* %25, align 4
  %72 = load <4 x i8> addrspace(1)* %28, align 4
  %73 = load <4 x i8> addrspace(1)* %31, align 4
  %74 = load <4 x i8> addrspace(1)* %34, align 4
  %75 = load <4 x i8> addrspace(1)* %37, align 4
  %76 = load <4 x i8> addrspace(1)* %40, align 4
  %77 = load <4 x i8> addrspace(1)* %43, align 4
  %78 = load <4 x i8> addrspace(1)* %46, align 4
  %79 = load <4 x i8> addrspace(1)* %49, align 4
  %80 = load <4 x i8> addrspace(1)* %52, align 4
  %81 = load <4 x i8> addrspace(1)* %55, align 4
  %82 = load <4 x i8> addrspace(1)* %58, align 4
  %83 = load <4 x i8> addrspace(1)* %61, align 4
  %84 = load <4 x i8> addrspace(1)* %64, align 4
  %85 = load <4 x i8> addrspace(1)* %67, align 4
  %86 = load <4 x i8> addrspace(1)* %70, align 4
  %87 = mul i32 %20, %blockSize
  %temp18 = insertelement <16 x i32> undef, i32 %87, i32 0
  %vector19 = shufflevector <16 x i32> %temp18, <16 x i32> undef, <16 x i32> zeroinitializer
  %88 = add <16 x i32> %vector19, %17
  %89 = extractelement <16 x i32> %88, i32 0
  %90 = zext i32 %89 to i64
  %91 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %90
  %"&(pSB[currWI].offset)71" = add nuw i64 %CurrSBIndex..0, 192
  %"&pSB[currWI].offset72" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)71"
  %CastToValueType73 = bitcast i8* %"&pSB[currWI].offset72" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %91, <4 x i8> addrspace(3)** %CastToValueType73, align 8
  %92 = extractelement <16 x i32> %88, i32 1
  %93 = zext i32 %92 to i64
  %94 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %93
  %"&(pSB[currWI].offset)80" = add nuw i64 %CurrSBIndex..0, 200
  %"&pSB[currWI].offset81" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)80"
  %CastToValueType82 = bitcast i8* %"&pSB[currWI].offset81" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %94, <4 x i8> addrspace(3)** %CastToValueType82, align 8
  %95 = extractelement <16 x i32> %88, i32 2
  %96 = zext i32 %95 to i64
  %97 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %96
  %"&(pSB[currWI].offset)89" = add nuw i64 %CurrSBIndex..0, 208
  %"&pSB[currWI].offset90" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)89"
  %CastToValueType91 = bitcast i8* %"&pSB[currWI].offset90" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %97, <4 x i8> addrspace(3)** %CastToValueType91, align 8
  %98 = extractelement <16 x i32> %88, i32 3
  %99 = zext i32 %98 to i64
  %100 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %99
  %"&(pSB[currWI].offset)98" = add nuw i64 %CurrSBIndex..0, 216
  %"&pSB[currWI].offset99" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)98"
  %CastToValueType100 = bitcast i8* %"&pSB[currWI].offset99" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %100, <4 x i8> addrspace(3)** %CastToValueType100, align 8
  %101 = extractelement <16 x i32> %88, i32 4
  %102 = zext i32 %101 to i64
  %103 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %102
  %"&(pSB[currWI].offset)107" = add nuw i64 %CurrSBIndex..0, 224
  %"&pSB[currWI].offset108" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)107"
  %CastToValueType109 = bitcast i8* %"&pSB[currWI].offset108" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %103, <4 x i8> addrspace(3)** %CastToValueType109, align 8
  %104 = extractelement <16 x i32> %88, i32 5
  %105 = zext i32 %104 to i64
  %106 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %105
  %"&(pSB[currWI].offset)116" = add nuw i64 %CurrSBIndex..0, 232
  %"&pSB[currWI].offset117" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)116"
  %CastToValueType118 = bitcast i8* %"&pSB[currWI].offset117" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %106, <4 x i8> addrspace(3)** %CastToValueType118, align 8
  %107 = extractelement <16 x i32> %88, i32 6
  %108 = zext i32 %107 to i64
  %109 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %108
  %"&(pSB[currWI].offset)125" = add nuw i64 %CurrSBIndex..0, 240
  %"&pSB[currWI].offset126" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)125"
  %CastToValueType127 = bitcast i8* %"&pSB[currWI].offset126" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %109, <4 x i8> addrspace(3)** %CastToValueType127, align 8
  %110 = extractelement <16 x i32> %88, i32 7
  %111 = zext i32 %110 to i64
  %112 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %111
  %"&(pSB[currWI].offset)134" = add nuw i64 %CurrSBIndex..0, 248
  %"&pSB[currWI].offset135" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)134"
  %CastToValueType136 = bitcast i8* %"&pSB[currWI].offset135" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %112, <4 x i8> addrspace(3)** %CastToValueType136, align 8
  %113 = extractelement <16 x i32> %88, i32 8
  %114 = zext i32 %113 to i64
  %115 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %114
  %"&(pSB[currWI].offset)143" = add nuw i64 %CurrSBIndex..0, 256
  %"&pSB[currWI].offset144" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)143"
  %CastToValueType145 = bitcast i8* %"&pSB[currWI].offset144" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %115, <4 x i8> addrspace(3)** %CastToValueType145, align 8
  %116 = extractelement <16 x i32> %88, i32 9
  %117 = zext i32 %116 to i64
  %118 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %117
  %"&(pSB[currWI].offset)152" = add nuw i64 %CurrSBIndex..0, 264
  %"&pSB[currWI].offset153" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)152"
  %CastToValueType154 = bitcast i8* %"&pSB[currWI].offset153" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %118, <4 x i8> addrspace(3)** %CastToValueType154, align 8
  %119 = extractelement <16 x i32> %88, i32 10
  %120 = zext i32 %119 to i64
  %121 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %120
  %"&(pSB[currWI].offset)161" = add nuw i64 %CurrSBIndex..0, 272
  %"&pSB[currWI].offset162" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)161"
  %CastToValueType163 = bitcast i8* %"&pSB[currWI].offset162" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %121, <4 x i8> addrspace(3)** %CastToValueType163, align 8
  %122 = extractelement <16 x i32> %88, i32 11
  %123 = zext i32 %122 to i64
  %124 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %123
  %"&(pSB[currWI].offset)170" = add nuw i64 %CurrSBIndex..0, 280
  %"&pSB[currWI].offset171" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)170"
  %CastToValueType172 = bitcast i8* %"&pSB[currWI].offset171" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %124, <4 x i8> addrspace(3)** %CastToValueType172, align 8
  %125 = extractelement <16 x i32> %88, i32 12
  %126 = zext i32 %125 to i64
  %127 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %126
  %"&(pSB[currWI].offset)179" = add nuw i64 %CurrSBIndex..0, 288
  %"&pSB[currWI].offset180" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)179"
  %CastToValueType181 = bitcast i8* %"&pSB[currWI].offset180" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %127, <4 x i8> addrspace(3)** %CastToValueType181, align 8
  %128 = extractelement <16 x i32> %88, i32 13
  %129 = zext i32 %128 to i64
  %130 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %129
  %"&(pSB[currWI].offset)188" = add nuw i64 %CurrSBIndex..0, 296
  %"&pSB[currWI].offset189" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)188"
  %CastToValueType190 = bitcast i8* %"&pSB[currWI].offset189" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %130, <4 x i8> addrspace(3)** %CastToValueType190, align 8
  %131 = extractelement <16 x i32> %88, i32 14
  %132 = zext i32 %131 to i64
  %133 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %132
  %"&(pSB[currWI].offset)197" = add nuw i64 %CurrSBIndex..0, 304
  %"&pSB[currWI].offset198" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)197"
  %CastToValueType199 = bitcast i8* %"&pSB[currWI].offset198" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %133, <4 x i8> addrspace(3)** %CastToValueType199, align 8
  %134 = extractelement <16 x i32> %88, i32 15
  %135 = zext i32 %134 to i64
  %136 = getelementptr inbounds <4 x i8> addrspace(3)* %block, i64 %135
  %"&(pSB[currWI].offset)206" = add nuw i64 %CurrSBIndex..0, 312
  %"&pSB[currWI].offset207" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)206"
  %CastToValueType208 = bitcast i8* %"&pSB[currWI].offset207" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %136, <4 x i8> addrspace(3)** %CastToValueType208, align 8
  store <4 x i8> %71, <4 x i8> addrspace(3)* %91, align 4
  store <4 x i8> %72, <4 x i8> addrspace(3)* %94, align 4
  store <4 x i8> %73, <4 x i8> addrspace(3)* %97, align 4
  store <4 x i8> %74, <4 x i8> addrspace(3)* %100, align 4
  store <4 x i8> %75, <4 x i8> addrspace(3)* %103, align 4
  store <4 x i8> %76, <4 x i8> addrspace(3)* %106, align 4
  store <4 x i8> %77, <4 x i8> addrspace(3)* %109, align 4
  store <4 x i8> %78, <4 x i8> addrspace(3)* %112, align 4
  store <4 x i8> %79, <4 x i8> addrspace(3)* %115, align 4
  store <4 x i8> %80, <4 x i8> addrspace(3)* %118, align 4
  store <4 x i8> %81, <4 x i8> addrspace(3)* %121, align 4
  store <4 x i8> %82, <4 x i8> addrspace(3)* %124, align 4
  store <4 x i8> %83, <4 x i8> addrspace(3)* %127, align 4
  store <4 x i8> %84, <4 x i8> addrspace(3)* %130, align 4
  store <4 x i8> %85, <4 x i8> addrspace(3)* %133, align 4
  store <4 x i8> %86, <4 x i8> addrspace(3)* %136, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %SyncBB216
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 448
  br label %SyncBB216

elseBB:                                           ; preds = %SyncBB216
  %temp36 = insertelement <16 x i32> undef, i32 %height, i32 0
  %vector37 = shufflevector <16 x i32> %temp36, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB219, %elseBB
  %CurrSBIndex..1 = phi i64 [ 0, %elseBB ], [ %"loadedCurrSB+Stride225", %thenBB219 ]
  %CurrWI..1 = phi i64 [ 0, %elseBB ], [ %"CurrWI++223", %thenBB219 ]
  %"&(pSB[currWI].offset)58" = add nuw i64 %CurrSBIndex..1, 64
  %"&pSB[currWI].offset59" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)58"
  %CastToValueType60 = bitcast i8* %"&pSB[currWI].offset59" to <16 x i32>*
  %loadedValue = load <16 x i32>* %CastToValueType60, align 64
  %137 = mul <16 x i32> %loadedValue, %vector37
  %"&(pSB[currWI].offset)66" = add nuw i64 %CurrSBIndex..1, 128
  %"&pSB[currWI].offset67" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)66"
  %CastToValueType68 = bitcast i8* %"&pSB[currWI].offset67" to <16 x i32>*
  %loadedValue69 = load <16 x i32>* %CastToValueType68, align 64
  %138 = add <16 x i32> %loadedValue69, %137
  %"&(pSB[currWI].offset)75" = add nuw i64 %CurrSBIndex..1, 192
  %"&pSB[currWI].offset76" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)75"
  %CastToValueType77 = bitcast i8* %"&pSB[currWI].offset76" to <4 x i8> addrspace(3)**
  %loadedValue78 = load <4 x i8> addrspace(3)** %CastToValueType77, align 8
  %139 = load <4 x i8> addrspace(3)* %loadedValue78, align 4
  %"&(pSB[currWI].offset)84" = add nuw i64 %CurrSBIndex..1, 200
  %"&pSB[currWI].offset85" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)84"
  %CastToValueType86 = bitcast i8* %"&pSB[currWI].offset85" to <4 x i8> addrspace(3)**
  %loadedValue87 = load <4 x i8> addrspace(3)** %CastToValueType86, align 8
  %140 = load <4 x i8> addrspace(3)* %loadedValue87, align 4
  %"&(pSB[currWI].offset)93" = add nuw i64 %CurrSBIndex..1, 208
  %"&pSB[currWI].offset94" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)93"
  %CastToValueType95 = bitcast i8* %"&pSB[currWI].offset94" to <4 x i8> addrspace(3)**
  %loadedValue96 = load <4 x i8> addrspace(3)** %CastToValueType95, align 8
  %141 = load <4 x i8> addrspace(3)* %loadedValue96, align 4
  %"&(pSB[currWI].offset)102" = add nuw i64 %CurrSBIndex..1, 216
  %"&pSB[currWI].offset103" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)102"
  %CastToValueType104 = bitcast i8* %"&pSB[currWI].offset103" to <4 x i8> addrspace(3)**
  %loadedValue105 = load <4 x i8> addrspace(3)** %CastToValueType104, align 8
  %142 = load <4 x i8> addrspace(3)* %loadedValue105, align 4
  %"&(pSB[currWI].offset)111" = add nuw i64 %CurrSBIndex..1, 224
  %"&pSB[currWI].offset112" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)111"
  %CastToValueType113 = bitcast i8* %"&pSB[currWI].offset112" to <4 x i8> addrspace(3)**
  %loadedValue114 = load <4 x i8> addrspace(3)** %CastToValueType113, align 8
  %143 = load <4 x i8> addrspace(3)* %loadedValue114, align 4
  %"&(pSB[currWI].offset)120" = add nuw i64 %CurrSBIndex..1, 232
  %"&pSB[currWI].offset121" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)120"
  %CastToValueType122 = bitcast i8* %"&pSB[currWI].offset121" to <4 x i8> addrspace(3)**
  %loadedValue123 = load <4 x i8> addrspace(3)** %CastToValueType122, align 8
  %144 = load <4 x i8> addrspace(3)* %loadedValue123, align 4
  %"&(pSB[currWI].offset)129" = add nuw i64 %CurrSBIndex..1, 240
  %"&pSB[currWI].offset130" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)129"
  %CastToValueType131 = bitcast i8* %"&pSB[currWI].offset130" to <4 x i8> addrspace(3)**
  %loadedValue132 = load <4 x i8> addrspace(3)** %CastToValueType131, align 8
  %145 = load <4 x i8> addrspace(3)* %loadedValue132, align 4
  %"&(pSB[currWI].offset)138" = add nuw i64 %CurrSBIndex..1, 248
  %"&pSB[currWI].offset139" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)138"
  %CastToValueType140 = bitcast i8* %"&pSB[currWI].offset139" to <4 x i8> addrspace(3)**
  %loadedValue141 = load <4 x i8> addrspace(3)** %CastToValueType140, align 8
  %146 = load <4 x i8> addrspace(3)* %loadedValue141, align 4
  %"&(pSB[currWI].offset)147" = add nuw i64 %CurrSBIndex..1, 256
  %"&pSB[currWI].offset148" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)147"
  %CastToValueType149 = bitcast i8* %"&pSB[currWI].offset148" to <4 x i8> addrspace(3)**
  %loadedValue150 = load <4 x i8> addrspace(3)** %CastToValueType149, align 8
  %147 = load <4 x i8> addrspace(3)* %loadedValue150, align 4
  %"&(pSB[currWI].offset)156" = add nuw i64 %CurrSBIndex..1, 264
  %"&pSB[currWI].offset157" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)156"
  %CastToValueType158 = bitcast i8* %"&pSB[currWI].offset157" to <4 x i8> addrspace(3)**
  %loadedValue159 = load <4 x i8> addrspace(3)** %CastToValueType158, align 8
  %148 = load <4 x i8> addrspace(3)* %loadedValue159, align 4
  %"&(pSB[currWI].offset)165" = add nuw i64 %CurrSBIndex..1, 272
  %"&pSB[currWI].offset166" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)165"
  %CastToValueType167 = bitcast i8* %"&pSB[currWI].offset166" to <4 x i8> addrspace(3)**
  %loadedValue168 = load <4 x i8> addrspace(3)** %CastToValueType167, align 8
  %149 = load <4 x i8> addrspace(3)* %loadedValue168, align 4
  %"&(pSB[currWI].offset)174" = add nuw i64 %CurrSBIndex..1, 280
  %"&pSB[currWI].offset175" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)174"
  %CastToValueType176 = bitcast i8* %"&pSB[currWI].offset175" to <4 x i8> addrspace(3)**
  %loadedValue177 = load <4 x i8> addrspace(3)** %CastToValueType176, align 8
  %150 = load <4 x i8> addrspace(3)* %loadedValue177, align 4
  %"&(pSB[currWI].offset)183" = add nuw i64 %CurrSBIndex..1, 288
  %"&pSB[currWI].offset184" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)183"
  %CastToValueType185 = bitcast i8* %"&pSB[currWI].offset184" to <4 x i8> addrspace(3)**
  %loadedValue186 = load <4 x i8> addrspace(3)** %CastToValueType185, align 8
  %151 = load <4 x i8> addrspace(3)* %loadedValue186, align 4
  %"&(pSB[currWI].offset)192" = add nuw i64 %CurrSBIndex..1, 296
  %"&pSB[currWI].offset193" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)192"
  %CastToValueType194 = bitcast i8* %"&pSB[currWI].offset193" to <4 x i8> addrspace(3)**
  %loadedValue195 = load <4 x i8> addrspace(3)** %CastToValueType194, align 8
  %152 = load <4 x i8> addrspace(3)* %loadedValue195, align 4
  %"&(pSB[currWI].offset)201" = add nuw i64 %CurrSBIndex..1, 304
  %"&pSB[currWI].offset202" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)201"
  %CastToValueType203 = bitcast i8* %"&pSB[currWI].offset202" to <4 x i8> addrspace(3)**
  %loadedValue204 = load <4 x i8> addrspace(3)** %CastToValueType203, align 8
  %153 = load <4 x i8> addrspace(3)* %loadedValue204, align 4
  %"&(pSB[currWI].offset)210" = add nuw i64 %CurrSBIndex..1, 312
  %"&pSB[currWI].offset211" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)210"
  %CastToValueType212 = bitcast i8* %"&pSB[currWI].offset211" to <4 x i8> addrspace(3)**
  %loadedValue213 = load <4 x i8> addrspace(3)** %CastToValueType212, align 8
  %154 = load <4 x i8> addrspace(3)* %loadedValue213, align 4
  %155 = extractelement <16 x i32> %138, i32 0
  %156 = zext i32 %155 to i64
  %157 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %156
  %158 = extractelement <16 x i32> %138, i32 1
  %159 = zext i32 %158 to i64
  %160 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %159
  %161 = extractelement <16 x i32> %138, i32 2
  %162 = zext i32 %161 to i64
  %163 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %162
  %164 = extractelement <16 x i32> %138, i32 3
  %165 = zext i32 %164 to i64
  %166 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %165
  %167 = extractelement <16 x i32> %138, i32 4
  %168 = zext i32 %167 to i64
  %169 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %168
  %170 = extractelement <16 x i32> %138, i32 5
  %171 = zext i32 %170 to i64
  %172 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %171
  %173 = extractelement <16 x i32> %138, i32 6
  %174 = zext i32 %173 to i64
  %175 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %174
  %176 = extractelement <16 x i32> %138, i32 7
  %177 = zext i32 %176 to i64
  %178 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %177
  %179 = extractelement <16 x i32> %138, i32 8
  %180 = zext i32 %179 to i64
  %181 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %180
  %182 = extractelement <16 x i32> %138, i32 9
  %183 = zext i32 %182 to i64
  %184 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %183
  %185 = extractelement <16 x i32> %138, i32 10
  %186 = zext i32 %185 to i64
  %187 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %186
  %188 = extractelement <16 x i32> %138, i32 11
  %189 = zext i32 %188 to i64
  %190 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %189
  %191 = extractelement <16 x i32> %138, i32 12
  %192 = zext i32 %191 to i64
  %193 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %192
  %194 = extractelement <16 x i32> %138, i32 13
  %195 = zext i32 %194 to i64
  %196 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %195
  %197 = extractelement <16 x i32> %138, i32 14
  %198 = zext i32 %197 to i64
  %199 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %198
  %200 = extractelement <16 x i32> %138, i32 15
  %201 = zext i32 %200 to i64
  %202 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %201
  store <4 x i8> %139, <4 x i8> addrspace(1)* %157, align 4
  store <4 x i8> %140, <4 x i8> addrspace(1)* %160, align 4
  store <4 x i8> %141, <4 x i8> addrspace(1)* %163, align 4
  store <4 x i8> %142, <4 x i8> addrspace(1)* %166, align 4
  store <4 x i8> %143, <4 x i8> addrspace(1)* %169, align 4
  store <4 x i8> %144, <4 x i8> addrspace(1)* %172, align 4
  store <4 x i8> %145, <4 x i8> addrspace(1)* %175, align 4
  store <4 x i8> %146, <4 x i8> addrspace(1)* %178, align 4
  store <4 x i8> %147, <4 x i8> addrspace(1)* %181, align 4
  store <4 x i8> %148, <4 x i8> addrspace(1)* %184, align 4
  store <4 x i8> %149, <4 x i8> addrspace(1)* %187, align 4
  store <4 x i8> %150, <4 x i8> addrspace(1)* %190, align 4
  store <4 x i8> %151, <4 x i8> addrspace(1)* %193, align 4
  store <4 x i8> %152, <4 x i8> addrspace(1)* %196, align 4
  store <4 x i8> %153, <4 x i8> addrspace(1)* %199, align 4
  store <4 x i8> %154, <4 x i8> addrspace(1)* %202, align 4
  %check.WI.iter222 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter222, label %thenBB219, label %SyncBB217

thenBB219:                                        ; preds = %SyncBB
  %"CurrWI++223" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride225" = add nuw i64 %CurrSBIndex..1, 448
  br label %SyncBB

SyncBB217:                                        ; preds = %SyncBB
  ret void
}

define void @____Vectorized_.RecursiveGaussian_kernel_separated_args(<4 x i8> addrspace(1)* nocapture %input, <4 x i8> addrspace(1)* nocapture %output, i32 %width, i32 %height, float %a0, float %a1, float %a2, float %a3, float %b1, float %b2, float %coefp, float %coefn, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
.preheader:
  %temp215 = insertelement <16 x float> undef, float %b2, i32 0
  %vector216 = shufflevector <16 x float> %temp215, <16 x float> undef, <16 x i32> zeroinitializer
  %temp213 = insertelement <16 x float> undef, float %b1, i32 0
  %vector214 = shufflevector <16 x float> %temp213, <16 x float> undef, <16 x i32> zeroinitializer
  %temp211 = insertelement <16 x float> undef, float %a1, i32 0
  %vector212 = shufflevector <16 x float> %temp211, <16 x float> undef, <16 x i32> zeroinitializer
  %temp145 = insertelement <16 x float> undef, float %a0, i32 0
  %vector146 = shufflevector <16 x float> %temp145, <16 x float> undef, <16 x i32> zeroinitializer
  %temp = insertelement <16 x i32> undef, i32 %width, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %0 = icmp sgt i32 %height, 0
  %temp93 = insertelement <16 x i1> undef, i1 %0, i32 0
  %vector94 = shufflevector <16 x i1> %temp93, <16 x i1> undef, <16 x i32> zeroinitializer
  %temp369 = insertelement <16 x float> undef, float %a3, i32 0
  %vector370 = shufflevector <16 x float> %temp369, <16 x float> undef, <16 x i32> zeroinitializer
  %temp367 = insertelement <16 x float> undef, float %a2, i32 0
  %vector368 = shufflevector <16 x float> %temp367, <16 x float> undef, <16 x i32> zeroinitializer
  %y1.01 = add i32 %height, -1
  %1 = icmp sgt i32 %y1.01, -1
  %temp306 = insertelement <16 x i1> undef, i1 %1, i32 0
  %vector307 = shufflevector <16 x i1> %temp306, <16 x i1> undef, <16 x i32> zeroinitializer
  %tmp = sub i32 0, %width
  %tmp14 = mul i32 %y1.01, %width
  %temp309 = insertelement <16 x i32> undef, i32 %tmp14, i32 0
  %vector310 = shufflevector <16 x i32> %temp309, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp369.le = insertelement <16 x float> undef, float %a3, i32 0
  %vector370.le = shufflevector <16 x float> %temp369.le, <16 x float> undef, <16 x i32> zeroinitializer
  %temp367.le = insertelement <16 x float> undef, float %a2, i32 0
  %vector368.le = shufflevector <16 x float> %temp367.le, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB.outer

SyncBB.outer:                                     ; preds = %thenBB, %.preheader
  %currBarrier.2.ph = phi i32 [ 11, %.preheader ], [ %currBarrier.1, %thenBB ]
  %CurrSBIndex..0.ph = phi i64 [ 0, %.preheader ], [ %"loadedCurrSB+Stride", %thenBB ]
  %CurrWI..0.ph = phi i64 [ 0, %.preheader ], [ %"CurrWI++", %thenBB ]
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB875, %SyncBB.outer
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride881", %thenBB875 ], [ %CurrSBIndex..0.ph, %SyncBB.outer ]
  %CurrWI..0 = phi i64 [ %"CurrWI++879", %thenBB875 ], [ %CurrWI..0.ph, %SyncBB.outer ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %broadcast1 = insertelement <16 x i64> undef, i64 %6, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %7 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %8 = trunc <16 x i64> %7 to <16 x i32>
  %"&(pSB[currWI].offset)" = add nuw i64 %CurrSBIndex..0, 320
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to <16 x i32>*
  store <16 x i32> %8, <16 x i32>* %CastToValueType, align 64
  %9 = icmp ult <16 x i32> %8, %vector
  %"&(pSB[currWI].offset)836" = add nuw i64 %CurrSBIndex..0, 384
  %"&pSB[currWI].offset837" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)836"
  %CastToValueType838 = bitcast i8* %"&pSB[currWI].offset837" to <16 x i1>*
  store <16 x i1> %9, <16 x i1>* %CastToValueType838, align 16
  %extract290 = extractelement <16 x i1> %9, i32 0
  %.preheader_to_bb.nph1195 = and <16 x i1> %9, %vector94
  %ipred.i = bitcast <16 x i1> %.preheader_to_bb.nph1195 to i16
  %val.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i, i16 %ipred.i) nounwind
  %tmp.i = and i32 %val.i, 1
  %res.i = icmp eq i32 %tmp.i, 0
  br i1 %res.i, label %.preheader883, label %._crit_edge

.preheader883:                                    ; preds = %SyncBB
  %negIncomingLoopMask96 = xor <16 x i1> %.preheader_to_bb.nph1195, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %"&(pSB[currWI].offset)831" = add nuw i64 %CurrSBIndex..0, 320
  %"&pSB[currWI].offset832" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)831"
  %CastToValueType833 = bitcast i8* %"&pSB[currWI].offset832" to <16 x i32>*
  br label %10

; <label>:10                                      ; preds = %postload695, %.preheader883
  %vectorPHI = phi <16 x i1> [ %loop_mask54285, %postload695 ], [ %negIncomingLoopMask96, %.preheader883 ]
  %vectorPHI98 = phi <16 x i1> [ %local_edge289, %postload695 ], [ %.preheader_to_bb.nph1195, %.preheader883 ]
  %y.010 = phi i32 [ %391, %postload695 ], [ 0, %.preheader883 ]
  %vectorPHI99 = phi <16 x float> [ %vectorPHI103, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI100 = phi <16 x float> [ %vectorPHI104, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI101 = phi <16 x float> [ %vectorPHI105, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI102 = phi <16 x float> [ %vectorPHI106, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI103 = phi <16 x float> [ %211, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI104 = phi <16 x float> [ %212, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI105 = phi <16 x float> [ %213, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI106 = phi <16 x float> [ %214, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI107 = phi <16 x float> [ %temp.vect162, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI108 = phi <16 x float> [ %temp.vect178, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI109 = phi <16 x float> [ %temp.vect194, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %vectorPHI110 = phi <16 x float> [ %temp.vect210, %postload695 ], [ zeroinitializer, %.preheader883 ]
  %extract129 = extractelement <16 x i1> %vectorPHI98, i32 0
  %extract130 = extractelement <16 x i1> %vectorPHI98, i32 1
  %extract131 = extractelement <16 x i1> %vectorPHI98, i32 2
  %extract132 = extractelement <16 x i1> %vectorPHI98, i32 3
  %extract133 = extractelement <16 x i1> %vectorPHI98, i32 4
  %extract134 = extractelement <16 x i1> %vectorPHI98, i32 5
  %extract135 = extractelement <16 x i1> %vectorPHI98, i32 6
  %extract136 = extractelement <16 x i1> %vectorPHI98, i32 7
  %extract137 = extractelement <16 x i1> %vectorPHI98, i32 8
  %extract138 = extractelement <16 x i1> %vectorPHI98, i32 9
  %extract139 = extractelement <16 x i1> %vectorPHI98, i32 10
  %extract140 = extractelement <16 x i1> %vectorPHI98, i32 11
  %extract141 = extractelement <16 x i1> %vectorPHI98, i32 12
  %extract142 = extractelement <16 x i1> %vectorPHI98, i32 13
  %extract143 = extractelement <16 x i1> %vectorPHI98, i32 14
  %extract144 = extractelement <16 x i1> %vectorPHI98, i32 15
  %tmp19 = mul i32 %y.010, %width
  %temp111 = insertelement <16 x i32> undef, i32 %tmp19, i32 0
  %vector112 = shufflevector <16 x i32> %temp111, <16 x i32> undef, <16 x i32> zeroinitializer
  %loadedValue834 = load <16 x i32>* %CastToValueType833, align 64
  %tmp21113 = add <16 x i32> %loadedValue834, %vector112
  %11 = extractelement <16 x i32> %tmp21113, i32 1
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %12
  %14 = extractelement <16 x i32> %tmp21113, i32 2
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %15
  %17 = extractelement <16 x i32> %tmp21113, i32 3
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %18
  %20 = extractelement <16 x i32> %tmp21113, i32 4
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %21
  %23 = extractelement <16 x i32> %tmp21113, i32 5
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %24
  %26 = extractelement <16 x i32> %tmp21113, i32 6
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %27
  %29 = extractelement <16 x i32> %tmp21113, i32 7
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %30
  %32 = extractelement <16 x i32> %tmp21113, i32 8
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %33
  %35 = extractelement <16 x i32> %tmp21113, i32 9
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %36
  %38 = extractelement <16 x i32> %tmp21113, i32 10
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %39
  %41 = extractelement <16 x i32> %tmp21113, i32 11
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %42
  %44 = extractelement <16 x i32> %tmp21113, i32 12
  %45 = sext i32 %44 to i64
  %46 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %45
  %47 = extractelement <16 x i32> %tmp21113, i32 13
  %48 = sext i32 %47 to i64
  %49 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %48
  %50 = extractelement <16 x i32> %tmp21113, i32 14
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %51
  %53 = extractelement <16 x i32> %tmp21113, i32 15
  %54 = sext i32 %53 to i64
  %55 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %54
  br i1 %extract129, label %preload, label %postload

preload:                                          ; preds = %10
  %56 = extractelement <16 x i32> %tmp21113, i32 0
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %57
  %masked_load = load <4 x i8> addrspace(1)* %58, align 4
  br label %postload

postload:                                         ; preds = %preload, %10
  %phi = phi <4 x i8> [ undef, %10 ], [ %masked_load, %preload ]
  br i1 %extract130, label %preload621, label %postload622

preload621:                                       ; preds = %postload
  %masked_load572 = load <4 x i8> addrspace(1)* %13, align 4
  br label %postload622

postload622:                                      ; preds = %preload621, %postload
  %phi623 = phi <4 x i8> [ undef, %postload ], [ %masked_load572, %preload621 ]
  br i1 %extract131, label %preload626, label %postload627

preload626:                                       ; preds = %postload622
  %masked_load573 = load <4 x i8> addrspace(1)* %16, align 4
  br label %postload627

postload627:                                      ; preds = %preload626, %postload622
  %phi628 = phi <4 x i8> [ undef, %postload622 ], [ %masked_load573, %preload626 ]
  br i1 %extract132, label %preload631, label %postload632

preload631:                                       ; preds = %postload627
  %masked_load574 = load <4 x i8> addrspace(1)* %19, align 4
  br label %postload632

postload632:                                      ; preds = %preload631, %postload627
  %phi633 = phi <4 x i8> [ undef, %postload627 ], [ %masked_load574, %preload631 ]
  br i1 %extract133, label %preload636, label %postload637

preload636:                                       ; preds = %postload632
  %masked_load575 = load <4 x i8> addrspace(1)* %22, align 4
  br label %postload637

postload637:                                      ; preds = %preload636, %postload632
  %phi638 = phi <4 x i8> [ undef, %postload632 ], [ %masked_load575, %preload636 ]
  br i1 %extract134, label %preload641, label %postload642

preload641:                                       ; preds = %postload637
  %masked_load576 = load <4 x i8> addrspace(1)* %25, align 4
  br label %postload642

postload642:                                      ; preds = %preload641, %postload637
  %phi643 = phi <4 x i8> [ undef, %postload637 ], [ %masked_load576, %preload641 ]
  br i1 %extract135, label %preload646, label %postload647

preload646:                                       ; preds = %postload642
  %masked_load577 = load <4 x i8> addrspace(1)* %28, align 4
  br label %postload647

postload647:                                      ; preds = %preload646, %postload642
  %phi648 = phi <4 x i8> [ undef, %postload642 ], [ %masked_load577, %preload646 ]
  br i1 %extract136, label %preload651, label %postload652

preload651:                                       ; preds = %postload647
  %masked_load578 = load <4 x i8> addrspace(1)* %31, align 4
  br label %postload652

postload652:                                      ; preds = %preload651, %postload647
  %phi653 = phi <4 x i8> [ undef, %postload647 ], [ %masked_load578, %preload651 ]
  br i1 %extract137, label %preload656, label %postload657

preload656:                                       ; preds = %postload652
  %masked_load579 = load <4 x i8> addrspace(1)* %34, align 4
  br label %postload657

postload657:                                      ; preds = %preload656, %postload652
  %phi658 = phi <4 x i8> [ undef, %postload652 ], [ %masked_load579, %preload656 ]
  br i1 %extract138, label %preload661, label %postload662

preload661:                                       ; preds = %postload657
  %masked_load580 = load <4 x i8> addrspace(1)* %37, align 4
  br label %postload662

postload662:                                      ; preds = %preload661, %postload657
  %phi663 = phi <4 x i8> [ undef, %postload657 ], [ %masked_load580, %preload661 ]
  br i1 %extract139, label %preload666, label %postload667

preload666:                                       ; preds = %postload662
  %masked_load581 = load <4 x i8> addrspace(1)* %40, align 4
  br label %postload667

postload667:                                      ; preds = %preload666, %postload662
  %phi668 = phi <4 x i8> [ undef, %postload662 ], [ %masked_load581, %preload666 ]
  br i1 %extract140, label %preload671, label %postload672

preload671:                                       ; preds = %postload667
  %masked_load582 = load <4 x i8> addrspace(1)* %43, align 4
  br label %postload672

postload672:                                      ; preds = %preload671, %postload667
  %phi673 = phi <4 x i8> [ undef, %postload667 ], [ %masked_load582, %preload671 ]
  br i1 %extract141, label %preload676, label %postload677

preload676:                                       ; preds = %postload672
  %masked_load583 = load <4 x i8> addrspace(1)* %46, align 4
  br label %postload677

postload677:                                      ; preds = %preload676, %postload672
  %phi678 = phi <4 x i8> [ undef, %postload672 ], [ %masked_load583, %preload676 ]
  br i1 %extract142, label %preload681, label %postload682

preload681:                                       ; preds = %postload677
  %masked_load584 = load <4 x i8> addrspace(1)* %49, align 4
  br label %postload682

postload682:                                      ; preds = %preload681, %postload677
  %phi683 = phi <4 x i8> [ undef, %postload677 ], [ %masked_load584, %preload681 ]
  br i1 %extract143, label %preload686, label %postload687

preload686:                                       ; preds = %postload682
  %masked_load585 = load <4 x i8> addrspace(1)* %52, align 4
  br label %postload687

postload687:                                      ; preds = %preload686, %postload682
  %phi688 = phi <4 x i8> [ undef, %postload682 ], [ %masked_load585, %preload686 ]
  br i1 %extract144, label %preload691, label %postload692

preload691:                                       ; preds = %postload687
  %masked_load586 = load <4 x i8> addrspace(1)* %55, align 4
  br label %postload692

postload692:                                      ; preds = %preload691, %postload687
  %phi693 = phi <4 x i8> [ undef, %postload687 ], [ %masked_load586, %preload691 ]
  %59 = extractelement <4 x i8> %phi, i32 0
  %60 = extractelement <4 x i8> %phi623, i32 0
  %61 = extractelement <4 x i8> %phi628, i32 0
  %62 = extractelement <4 x i8> %phi633, i32 0
  %63 = extractelement <4 x i8> %phi638, i32 0
  %64 = extractelement <4 x i8> %phi643, i32 0
  %65 = extractelement <4 x i8> %phi648, i32 0
  %66 = extractelement <4 x i8> %phi653, i32 0
  %67 = extractelement <4 x i8> %phi658, i32 0
  %68 = extractelement <4 x i8> %phi663, i32 0
  %69 = extractelement <4 x i8> %phi668, i32 0
  %70 = extractelement <4 x i8> %phi673, i32 0
  %71 = extractelement <4 x i8> %phi678, i32 0
  %72 = extractelement <4 x i8> %phi683, i32 0
  %73 = extractelement <4 x i8> %phi688, i32 0
  %74 = extractelement <4 x i8> %phi693, i32 0
  %75 = extractelement <4 x i8> %phi, i32 1
  %76 = extractelement <4 x i8> %phi623, i32 1
  %77 = extractelement <4 x i8> %phi628, i32 1
  %78 = extractelement <4 x i8> %phi633, i32 1
  %79 = extractelement <4 x i8> %phi638, i32 1
  %80 = extractelement <4 x i8> %phi643, i32 1
  %81 = extractelement <4 x i8> %phi648, i32 1
  %82 = extractelement <4 x i8> %phi653, i32 1
  %83 = extractelement <4 x i8> %phi658, i32 1
  %84 = extractelement <4 x i8> %phi663, i32 1
  %85 = extractelement <4 x i8> %phi668, i32 1
  %86 = extractelement <4 x i8> %phi673, i32 1
  %87 = extractelement <4 x i8> %phi678, i32 1
  %88 = extractelement <4 x i8> %phi683, i32 1
  %89 = extractelement <4 x i8> %phi688, i32 1
  %90 = extractelement <4 x i8> %phi693, i32 1
  %91 = extractelement <4 x i8> %phi, i32 2
  %92 = extractelement <4 x i8> %phi623, i32 2
  %93 = extractelement <4 x i8> %phi628, i32 2
  %94 = extractelement <4 x i8> %phi633, i32 2
  %95 = extractelement <4 x i8> %phi638, i32 2
  %96 = extractelement <4 x i8> %phi643, i32 2
  %97 = extractelement <4 x i8> %phi648, i32 2
  %98 = extractelement <4 x i8> %phi653, i32 2
  %99 = extractelement <4 x i8> %phi658, i32 2
  %100 = extractelement <4 x i8> %phi663, i32 2
  %101 = extractelement <4 x i8> %phi668, i32 2
  %102 = extractelement <4 x i8> %phi673, i32 2
  %103 = extractelement <4 x i8> %phi678, i32 2
  %104 = extractelement <4 x i8> %phi683, i32 2
  %105 = extractelement <4 x i8> %phi688, i32 2
  %106 = extractelement <4 x i8> %phi693, i32 2
  %107 = extractelement <4 x i8> %phi, i32 3
  %108 = extractelement <4 x i8> %phi623, i32 3
  %109 = extractelement <4 x i8> %phi628, i32 3
  %110 = extractelement <4 x i8> %phi633, i32 3
  %111 = extractelement <4 x i8> %phi638, i32 3
  %112 = extractelement <4 x i8> %phi643, i32 3
  %113 = extractelement <4 x i8> %phi648, i32 3
  %114 = extractelement <4 x i8> %phi653, i32 3
  %115 = extractelement <4 x i8> %phi658, i32 3
  %116 = extractelement <4 x i8> %phi663, i32 3
  %117 = extractelement <4 x i8> %phi668, i32 3
  %118 = extractelement <4 x i8> %phi673, i32 3
  %119 = extractelement <4 x i8> %phi678, i32 3
  %120 = extractelement <4 x i8> %phi683, i32 3
  %121 = extractelement <4 x i8> %phi688, i32 3
  %122 = extractelement <4 x i8> %phi693, i32 3
  %123 = uitofp i8 %59 to float
  %124 = uitofp i8 %60 to float
  %125 = uitofp i8 %61 to float
  %126 = uitofp i8 %62 to float
  %127 = uitofp i8 %63 to float
  %128 = uitofp i8 %64 to float
  %129 = uitofp i8 %65 to float
  %130 = uitofp i8 %66 to float
  %131 = uitofp i8 %67 to float
  %132 = uitofp i8 %68 to float
  %133 = uitofp i8 %69 to float
  %134 = uitofp i8 %70 to float
  %135 = uitofp i8 %71 to float
  %136 = uitofp i8 %72 to float
  %137 = uitofp i8 %73 to float
  %138 = uitofp i8 %74 to float
  %temp.vect147 = insertelement <16 x float> undef, float %123, i32 0
  %temp.vect148 = insertelement <16 x float> %temp.vect147, float %124, i32 1
  %temp.vect149 = insertelement <16 x float> %temp.vect148, float %125, i32 2
  %temp.vect150 = insertelement <16 x float> %temp.vect149, float %126, i32 3
  %temp.vect151 = insertelement <16 x float> %temp.vect150, float %127, i32 4
  %temp.vect152 = insertelement <16 x float> %temp.vect151, float %128, i32 5
  %temp.vect153 = insertelement <16 x float> %temp.vect152, float %129, i32 6
  %temp.vect154 = insertelement <16 x float> %temp.vect153, float %130, i32 7
  %temp.vect155 = insertelement <16 x float> %temp.vect154, float %131, i32 8
  %temp.vect156 = insertelement <16 x float> %temp.vect155, float %132, i32 9
  %temp.vect157 = insertelement <16 x float> %temp.vect156, float %133, i32 10
  %temp.vect158 = insertelement <16 x float> %temp.vect157, float %134, i32 11
  %temp.vect159 = insertelement <16 x float> %temp.vect158, float %135, i32 12
  %temp.vect160 = insertelement <16 x float> %temp.vect159, float %136, i32 13
  %temp.vect161 = insertelement <16 x float> %temp.vect160, float %137, i32 14
  %temp.vect162 = insertelement <16 x float> %temp.vect161, float %138, i32 15
  %139 = uitofp i8 %75 to float
  %140 = uitofp i8 %76 to float
  %141 = uitofp i8 %77 to float
  %142 = uitofp i8 %78 to float
  %143 = uitofp i8 %79 to float
  %144 = uitofp i8 %80 to float
  %145 = uitofp i8 %81 to float
  %146 = uitofp i8 %82 to float
  %147 = uitofp i8 %83 to float
  %148 = uitofp i8 %84 to float
  %149 = uitofp i8 %85 to float
  %150 = uitofp i8 %86 to float
  %151 = uitofp i8 %87 to float
  %152 = uitofp i8 %88 to float
  %153 = uitofp i8 %89 to float
  %154 = uitofp i8 %90 to float
  %temp.vect163 = insertelement <16 x float> undef, float %139, i32 0
  %temp.vect164 = insertelement <16 x float> %temp.vect163, float %140, i32 1
  %temp.vect165 = insertelement <16 x float> %temp.vect164, float %141, i32 2
  %temp.vect166 = insertelement <16 x float> %temp.vect165, float %142, i32 3
  %temp.vect167 = insertelement <16 x float> %temp.vect166, float %143, i32 4
  %temp.vect168 = insertelement <16 x float> %temp.vect167, float %144, i32 5
  %temp.vect169 = insertelement <16 x float> %temp.vect168, float %145, i32 6
  %temp.vect170 = insertelement <16 x float> %temp.vect169, float %146, i32 7
  %temp.vect171 = insertelement <16 x float> %temp.vect170, float %147, i32 8
  %temp.vect172 = insertelement <16 x float> %temp.vect171, float %148, i32 9
  %temp.vect173 = insertelement <16 x float> %temp.vect172, float %149, i32 10
  %temp.vect174 = insertelement <16 x float> %temp.vect173, float %150, i32 11
  %temp.vect175 = insertelement <16 x float> %temp.vect174, float %151, i32 12
  %temp.vect176 = insertelement <16 x float> %temp.vect175, float %152, i32 13
  %temp.vect177 = insertelement <16 x float> %temp.vect176, float %153, i32 14
  %temp.vect178 = insertelement <16 x float> %temp.vect177, float %154, i32 15
  %155 = uitofp i8 %91 to float
  %156 = uitofp i8 %92 to float
  %157 = uitofp i8 %93 to float
  %158 = uitofp i8 %94 to float
  %159 = uitofp i8 %95 to float
  %160 = uitofp i8 %96 to float
  %161 = uitofp i8 %97 to float
  %162 = uitofp i8 %98 to float
  %163 = uitofp i8 %99 to float
  %164 = uitofp i8 %100 to float
  %165 = uitofp i8 %101 to float
  %166 = uitofp i8 %102 to float
  %167 = uitofp i8 %103 to float
  %168 = uitofp i8 %104 to float
  %169 = uitofp i8 %105 to float
  %170 = uitofp i8 %106 to float
  %temp.vect179 = insertelement <16 x float> undef, float %155, i32 0
  %temp.vect180 = insertelement <16 x float> %temp.vect179, float %156, i32 1
  %temp.vect181 = insertelement <16 x float> %temp.vect180, float %157, i32 2
  %temp.vect182 = insertelement <16 x float> %temp.vect181, float %158, i32 3
  %temp.vect183 = insertelement <16 x float> %temp.vect182, float %159, i32 4
  %temp.vect184 = insertelement <16 x float> %temp.vect183, float %160, i32 5
  %temp.vect185 = insertelement <16 x float> %temp.vect184, float %161, i32 6
  %temp.vect186 = insertelement <16 x float> %temp.vect185, float %162, i32 7
  %temp.vect187 = insertelement <16 x float> %temp.vect186, float %163, i32 8
  %temp.vect188 = insertelement <16 x float> %temp.vect187, float %164, i32 9
  %temp.vect189 = insertelement <16 x float> %temp.vect188, float %165, i32 10
  %temp.vect190 = insertelement <16 x float> %temp.vect189, float %166, i32 11
  %temp.vect191 = insertelement <16 x float> %temp.vect190, float %167, i32 12
  %temp.vect192 = insertelement <16 x float> %temp.vect191, float %168, i32 13
  %temp.vect193 = insertelement <16 x float> %temp.vect192, float %169, i32 14
  %temp.vect194 = insertelement <16 x float> %temp.vect193, float %170, i32 15
  %171 = uitofp i8 %107 to float
  %172 = uitofp i8 %108 to float
  %173 = uitofp i8 %109 to float
  %174 = uitofp i8 %110 to float
  %175 = uitofp i8 %111 to float
  %176 = uitofp i8 %112 to float
  %177 = uitofp i8 %113 to float
  %178 = uitofp i8 %114 to float
  %179 = uitofp i8 %115 to float
  %180 = uitofp i8 %116 to float
  %181 = uitofp i8 %117 to float
  %182 = uitofp i8 %118 to float
  %183 = uitofp i8 %119 to float
  %184 = uitofp i8 %120 to float
  %185 = uitofp i8 %121 to float
  %186 = uitofp i8 %122 to float
  %temp.vect195 = insertelement <16 x float> undef, float %171, i32 0
  %temp.vect196 = insertelement <16 x float> %temp.vect195, float %172, i32 1
  %temp.vect197 = insertelement <16 x float> %temp.vect196, float %173, i32 2
  %temp.vect198 = insertelement <16 x float> %temp.vect197, float %174, i32 3
  %temp.vect199 = insertelement <16 x float> %temp.vect198, float %175, i32 4
  %temp.vect200 = insertelement <16 x float> %temp.vect199, float %176, i32 5
  %temp.vect201 = insertelement <16 x float> %temp.vect200, float %177, i32 6
  %temp.vect202 = insertelement <16 x float> %temp.vect201, float %178, i32 7
  %temp.vect203 = insertelement <16 x float> %temp.vect202, float %179, i32 8
  %temp.vect204 = insertelement <16 x float> %temp.vect203, float %180, i32 9
  %temp.vect205 = insertelement <16 x float> %temp.vect204, float %181, i32 10
  %temp.vect206 = insertelement <16 x float> %temp.vect205, float %182, i32 11
  %temp.vect207 = insertelement <16 x float> %temp.vect206, float %183, i32 12
  %temp.vect208 = insertelement <16 x float> %temp.vect207, float %184, i32 13
  %temp.vect209 = insertelement <16 x float> %temp.vect208, float %185, i32 14
  %temp.vect210 = insertelement <16 x float> %temp.vect209, float %186, i32 15
  %187 = fmul <16 x float> %vector146, %temp.vect162
  %188 = fmul <16 x float> %vector146, %temp.vect178
  %189 = fmul <16 x float> %vector146, %temp.vect194
  %190 = fmul <16 x float> %vector146, %temp.vect210
  %191 = fmul <16 x float> %vector212, %vectorPHI107
  %192 = fmul <16 x float> %vector212, %vectorPHI108
  %193 = fmul <16 x float> %vector212, %vectorPHI109
  %194 = fmul <16 x float> %vector212, %vectorPHI110
  %195 = fadd <16 x float> %187, %191
  %196 = fadd <16 x float> %188, %192
  %197 = fadd <16 x float> %189, %193
  %198 = fadd <16 x float> %190, %194
  %199 = fmul <16 x float> %vector214, %vectorPHI103
  %200 = fmul <16 x float> %vector214, %vectorPHI104
  %201 = fmul <16 x float> %vector214, %vectorPHI105
  %202 = fmul <16 x float> %vector214, %vectorPHI106
  %203 = fsub <16 x float> %195, %199
  %204 = fsub <16 x float> %196, %200
  %205 = fsub <16 x float> %197, %201
  %206 = fsub <16 x float> %198, %202
  %207 = fmul <16 x float> %vector216, %vectorPHI99
  %208 = fmul <16 x float> %vector216, %vectorPHI100
  %209 = fmul <16 x float> %vector216, %vectorPHI101
  %210 = fmul <16 x float> %vector216, %vectorPHI102
  %211 = fsub <16 x float> %203, %207
  %extract218 = extractelement <16 x float> %211, i32 1
  %extract219 = extractelement <16 x float> %211, i32 2
  %extract220 = extractelement <16 x float> %211, i32 3
  %extract221 = extractelement <16 x float> %211, i32 4
  %extract222 = extractelement <16 x float> %211, i32 5
  %extract223 = extractelement <16 x float> %211, i32 6
  %extract224 = extractelement <16 x float> %211, i32 7
  %extract225 = extractelement <16 x float> %211, i32 8
  %extract226 = extractelement <16 x float> %211, i32 9
  %extract227 = extractelement <16 x float> %211, i32 10
  %extract228 = extractelement <16 x float> %211, i32 11
  %extract229 = extractelement <16 x float> %211, i32 12
  %extract230 = extractelement <16 x float> %211, i32 13
  %extract231 = extractelement <16 x float> %211, i32 14
  %extract232 = extractelement <16 x float> %211, i32 15
  %212 = fsub <16 x float> %204, %208
  %extract234 = extractelement <16 x float> %212, i32 1
  %extract235 = extractelement <16 x float> %212, i32 2
  %extract236 = extractelement <16 x float> %212, i32 3
  %extract237 = extractelement <16 x float> %212, i32 4
  %extract238 = extractelement <16 x float> %212, i32 5
  %extract239 = extractelement <16 x float> %212, i32 6
  %extract240 = extractelement <16 x float> %212, i32 7
  %extract241 = extractelement <16 x float> %212, i32 8
  %extract242 = extractelement <16 x float> %212, i32 9
  %extract243 = extractelement <16 x float> %212, i32 10
  %extract244 = extractelement <16 x float> %212, i32 11
  %extract245 = extractelement <16 x float> %212, i32 12
  %extract246 = extractelement <16 x float> %212, i32 13
  %extract247 = extractelement <16 x float> %212, i32 14
  %extract248 = extractelement <16 x float> %212, i32 15
  %213 = fsub <16 x float> %205, %209
  %extract250 = extractelement <16 x float> %213, i32 1
  %extract251 = extractelement <16 x float> %213, i32 2
  %extract252 = extractelement <16 x float> %213, i32 3
  %extract253 = extractelement <16 x float> %213, i32 4
  %extract254 = extractelement <16 x float> %213, i32 5
  %extract255 = extractelement <16 x float> %213, i32 6
  %extract256 = extractelement <16 x float> %213, i32 7
  %extract257 = extractelement <16 x float> %213, i32 8
  %extract258 = extractelement <16 x float> %213, i32 9
  %extract259 = extractelement <16 x float> %213, i32 10
  %extract260 = extractelement <16 x float> %213, i32 11
  %extract261 = extractelement <16 x float> %213, i32 12
  %extract262 = extractelement <16 x float> %213, i32 13
  %extract263 = extractelement <16 x float> %213, i32 14
  %extract264 = extractelement <16 x float> %213, i32 15
  %214 = fsub <16 x float> %206, %210
  %extract266 = extractelement <16 x float> %214, i32 1
  %extract267 = extractelement <16 x float> %214, i32 2
  %extract268 = extractelement <16 x float> %214, i32 3
  %extract269 = extractelement <16 x float> %214, i32 4
  %extract270 = extractelement <16 x float> %214, i32 5
  %extract271 = extractelement <16 x float> %214, i32 6
  %extract272 = extractelement <16 x float> %214, i32 7
  %extract273 = extractelement <16 x float> %214, i32 8
  %extract274 = extractelement <16 x float> %214, i32 9
  %extract275 = extractelement <16 x float> %214, i32 10
  %extract276 = extractelement <16 x float> %214, i32 11
  %extract277 = extractelement <16 x float> %214, i32 12
  %extract278 = extractelement <16 x float> %214, i32 13
  %extract279 = extractelement <16 x float> %214, i32 14
  %extract280 = extractelement <16 x float> %214, i32 15
  %215 = fptoui float %extract218 to i8
  %216 = fptoui float %extract219 to i8
  %217 = fptoui float %extract220 to i8
  %218 = fptoui float %extract221 to i8
  %219 = fptoui float %extract222 to i8
  %220 = fptoui float %extract223 to i8
  %221 = fptoui float %extract224 to i8
  %222 = fptoui float %extract225 to i8
  %223 = fptoui float %extract226 to i8
  %224 = fptoui float %extract227 to i8
  %225 = fptoui float %extract228 to i8
  %226 = fptoui float %extract229 to i8
  %227 = fptoui float %extract230 to i8
  %228 = fptoui float %extract231 to i8
  %229 = fptoui float %extract232 to i8
  %230 = fptoui float %extract234 to i8
  %231 = fptoui float %extract235 to i8
  %232 = fptoui float %extract236 to i8
  %233 = fptoui float %extract237 to i8
  %234 = fptoui float %extract238 to i8
  %235 = fptoui float %extract239 to i8
  %236 = fptoui float %extract240 to i8
  %237 = fptoui float %extract241 to i8
  %238 = fptoui float %extract242 to i8
  %239 = fptoui float %extract243 to i8
  %240 = fptoui float %extract244 to i8
  %241 = fptoui float %extract245 to i8
  %242 = fptoui float %extract246 to i8
  %243 = fptoui float %extract247 to i8
  %244 = fptoui float %extract248 to i8
  %245 = fptoui float %extract250 to i8
  %246 = fptoui float %extract251 to i8
  %247 = fptoui float %extract252 to i8
  %248 = fptoui float %extract253 to i8
  %249 = fptoui float %extract254 to i8
  %250 = fptoui float %extract255 to i8
  %251 = fptoui float %extract256 to i8
  %252 = fptoui float %extract257 to i8
  %253 = fptoui float %extract258 to i8
  %254 = fptoui float %extract259 to i8
  %255 = fptoui float %extract260 to i8
  %256 = fptoui float %extract261 to i8
  %257 = fptoui float %extract262 to i8
  %258 = fptoui float %extract263 to i8
  %259 = fptoui float %extract264 to i8
  %260 = fptoui float %extract266 to i8
  %261 = fptoui float %extract267 to i8
  %262 = fptoui float %extract268 to i8
  %263 = fptoui float %extract269 to i8
  %264 = fptoui float %extract270 to i8
  %265 = fptoui float %extract271 to i8
  %266 = fptoui float %extract272 to i8
  %267 = fptoui float %extract273 to i8
  %268 = fptoui float %extract274 to i8
  %269 = fptoui float %extract275 to i8
  %270 = fptoui float %extract276 to i8
  %271 = fptoui float %extract277 to i8
  %272 = fptoui float %extract278 to i8
  %273 = fptoui float %extract279 to i8
  %274 = fptoui float %extract280 to i8
  %275 = insertelement <4 x i8> undef, i8 %215, i32 0
  %276 = insertelement <4 x i8> undef, i8 %216, i32 0
  %277 = insertelement <4 x i8> undef, i8 %217, i32 0
  %278 = insertelement <4 x i8> undef, i8 %218, i32 0
  %279 = insertelement <4 x i8> undef, i8 %219, i32 0
  %280 = insertelement <4 x i8> undef, i8 %220, i32 0
  %281 = insertelement <4 x i8> undef, i8 %221, i32 0
  %282 = insertelement <4 x i8> undef, i8 %222, i32 0
  %283 = insertelement <4 x i8> undef, i8 %223, i32 0
  %284 = insertelement <4 x i8> undef, i8 %224, i32 0
  %285 = insertelement <4 x i8> undef, i8 %225, i32 0
  %286 = insertelement <4 x i8> undef, i8 %226, i32 0
  %287 = insertelement <4 x i8> undef, i8 %227, i32 0
  %288 = insertelement <4 x i8> undef, i8 %228, i32 0
  %289 = insertelement <4 x i8> undef, i8 %229, i32 0
  %290 = insertelement <4 x i8> %275, i8 %230, i32 1
  %291 = insertelement <4 x i8> %276, i8 %231, i32 1
  %292 = insertelement <4 x i8> %277, i8 %232, i32 1
  %293 = insertelement <4 x i8> %278, i8 %233, i32 1
  %294 = insertelement <4 x i8> %279, i8 %234, i32 1
  %295 = insertelement <4 x i8> %280, i8 %235, i32 1
  %296 = insertelement <4 x i8> %281, i8 %236, i32 1
  %297 = insertelement <4 x i8> %282, i8 %237, i32 1
  %298 = insertelement <4 x i8> %283, i8 %238, i32 1
  %299 = insertelement <4 x i8> %284, i8 %239, i32 1
  %300 = insertelement <4 x i8> %285, i8 %240, i32 1
  %301 = insertelement <4 x i8> %286, i8 %241, i32 1
  %302 = insertelement <4 x i8> %287, i8 %242, i32 1
  %303 = insertelement <4 x i8> %288, i8 %243, i32 1
  %304 = insertelement <4 x i8> %289, i8 %244, i32 1
  %305 = insertelement <4 x i8> %290, i8 %245, i32 2
  %306 = insertelement <4 x i8> %291, i8 %246, i32 2
  %307 = insertelement <4 x i8> %292, i8 %247, i32 2
  %308 = insertelement <4 x i8> %293, i8 %248, i32 2
  %309 = insertelement <4 x i8> %294, i8 %249, i32 2
  %310 = insertelement <4 x i8> %295, i8 %250, i32 2
  %311 = insertelement <4 x i8> %296, i8 %251, i32 2
  %312 = insertelement <4 x i8> %297, i8 %252, i32 2
  %313 = insertelement <4 x i8> %298, i8 %253, i32 2
  %314 = insertelement <4 x i8> %299, i8 %254, i32 2
  %315 = insertelement <4 x i8> %300, i8 %255, i32 2
  %316 = insertelement <4 x i8> %301, i8 %256, i32 2
  %317 = insertelement <4 x i8> %302, i8 %257, i32 2
  %318 = insertelement <4 x i8> %303, i8 %258, i32 2
  %319 = insertelement <4 x i8> %304, i8 %259, i32 2
  %320 = insertelement <4 x i8> %305, i8 %260, i32 3
  %321 = insertelement <4 x i8> %306, i8 %261, i32 3
  %322 = insertelement <4 x i8> %307, i8 %262, i32 3
  %323 = insertelement <4 x i8> %308, i8 %263, i32 3
  %324 = insertelement <4 x i8> %309, i8 %264, i32 3
  %325 = insertelement <4 x i8> %310, i8 %265, i32 3
  %326 = insertelement <4 x i8> %311, i8 %266, i32 3
  %327 = insertelement <4 x i8> %312, i8 %267, i32 3
  %328 = insertelement <4 x i8> %313, i8 %268, i32 3
  %329 = insertelement <4 x i8> %314, i8 %269, i32 3
  %330 = insertelement <4 x i8> %315, i8 %270, i32 3
  %331 = insertelement <4 x i8> %316, i8 %271, i32 3
  %332 = insertelement <4 x i8> %317, i8 %272, i32 3
  %333 = insertelement <4 x i8> %318, i8 %273, i32 3
  %334 = insertelement <4 x i8> %319, i8 %274, i32 3
  %335 = extractelement <16 x i32> %tmp21113, i32 1
  %336 = sext i32 %335 to i64
  %337 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %336
  %338 = extractelement <16 x i32> %tmp21113, i32 2
  %339 = sext i32 %338 to i64
  %340 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %339
  %341 = extractelement <16 x i32> %tmp21113, i32 3
  %342 = sext i32 %341 to i64
  %343 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %342
  %344 = extractelement <16 x i32> %tmp21113, i32 4
  %345 = sext i32 %344 to i64
  %346 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %345
  %347 = extractelement <16 x i32> %tmp21113, i32 5
  %348 = sext i32 %347 to i64
  %349 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %348
  %350 = extractelement <16 x i32> %tmp21113, i32 6
  %351 = sext i32 %350 to i64
  %352 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %351
  %353 = extractelement <16 x i32> %tmp21113, i32 7
  %354 = sext i32 %353 to i64
  %355 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %354
  %356 = extractelement <16 x i32> %tmp21113, i32 8
  %357 = sext i32 %356 to i64
  %358 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %357
  %359 = extractelement <16 x i32> %tmp21113, i32 9
  %360 = sext i32 %359 to i64
  %361 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %360
  %362 = extractelement <16 x i32> %tmp21113, i32 10
  %363 = sext i32 %362 to i64
  %364 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %363
  %365 = extractelement <16 x i32> %tmp21113, i32 11
  %366 = sext i32 %365 to i64
  %367 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %366
  %368 = extractelement <16 x i32> %tmp21113, i32 12
  %369 = sext i32 %368 to i64
  %370 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %369
  %371 = extractelement <16 x i32> %tmp21113, i32 13
  %372 = sext i32 %371 to i64
  %373 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %372
  %374 = extractelement <16 x i32> %tmp21113, i32 14
  %375 = sext i32 %374 to i64
  %376 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %375
  %377 = extractelement <16 x i32> %tmp21113, i32 15
  %378 = sext i32 %377 to i64
  %379 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %378
  br i1 %extract129, label %preload619, label %postload620

preload619:                                       ; preds = %postload692
  %extract217 = extractelement <16 x float> %211, i32 0
  %380 = fptoui float %extract217 to i8
  %extract233 = extractelement <16 x float> %212, i32 0
  %381 = insertelement <4 x i8> undef, i8 %380, i32 0
  %382 = fptoui float %extract233 to i8
  %extract249 = extractelement <16 x float> %213, i32 0
  %383 = insertelement <4 x i8> %381, i8 %382, i32 1
  %384 = fptoui float %extract249 to i8
  %extract265 = extractelement <16 x float> %214, i32 0
  %385 = insertelement <4 x i8> %383, i8 %384, i32 2
  %386 = fptoui float %extract265 to i8
  %387 = extractelement <16 x i32> %tmp21113, i32 0
  %388 = sext i32 %387 to i64
  %389 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %388
  %390 = insertelement <4 x i8> %385, i8 %386, i32 3
  store <4 x i8> %390, <4 x i8> addrspace(1)* %389, align 4
  br label %postload620

postload620:                                      ; preds = %preload619, %postload692
  br i1 %extract130, label %preload624, label %postload625

preload624:                                       ; preds = %postload620
  store <4 x i8> %320, <4 x i8> addrspace(1)* %337, align 4
  br label %postload625

postload625:                                      ; preds = %preload624, %postload620
  br i1 %extract131, label %preload629, label %postload630

preload629:                                       ; preds = %postload625
  store <4 x i8> %321, <4 x i8> addrspace(1)* %340, align 4
  br label %postload630

postload630:                                      ; preds = %preload629, %postload625
  br i1 %extract132, label %preload634, label %postload635

preload634:                                       ; preds = %postload630
  store <4 x i8> %322, <4 x i8> addrspace(1)* %343, align 4
  br label %postload635

postload635:                                      ; preds = %preload634, %postload630
  br i1 %extract133, label %preload639, label %postload640

preload639:                                       ; preds = %postload635
  store <4 x i8> %323, <4 x i8> addrspace(1)* %346, align 4
  br label %postload640

postload640:                                      ; preds = %preload639, %postload635
  br i1 %extract134, label %preload644, label %postload645

preload644:                                       ; preds = %postload640
  store <4 x i8> %324, <4 x i8> addrspace(1)* %349, align 4
  br label %postload645

postload645:                                      ; preds = %preload644, %postload640
  br i1 %extract135, label %preload649, label %postload650

preload649:                                       ; preds = %postload645
  store <4 x i8> %325, <4 x i8> addrspace(1)* %352, align 4
  br label %postload650

postload650:                                      ; preds = %preload649, %postload645
  br i1 %extract136, label %preload654, label %postload655

preload654:                                       ; preds = %postload650
  store <4 x i8> %326, <4 x i8> addrspace(1)* %355, align 4
  br label %postload655

postload655:                                      ; preds = %preload654, %postload650
  br i1 %extract137, label %preload659, label %postload660

preload659:                                       ; preds = %postload655
  store <4 x i8> %327, <4 x i8> addrspace(1)* %358, align 4
  br label %postload660

postload660:                                      ; preds = %preload659, %postload655
  br i1 %extract138, label %preload664, label %postload665

preload664:                                       ; preds = %postload660
  store <4 x i8> %328, <4 x i8> addrspace(1)* %361, align 4
  br label %postload665

postload665:                                      ; preds = %preload664, %postload660
  br i1 %extract139, label %preload669, label %postload670

preload669:                                       ; preds = %postload665
  store <4 x i8> %329, <4 x i8> addrspace(1)* %364, align 4
  br label %postload670

postload670:                                      ; preds = %preload669, %postload665
  br i1 %extract140, label %preload674, label %postload675

preload674:                                       ; preds = %postload670
  store <4 x i8> %330, <4 x i8> addrspace(1)* %367, align 4
  br label %postload675

postload675:                                      ; preds = %preload674, %postload670
  br i1 %extract141, label %preload679, label %postload680

preload679:                                       ; preds = %postload675
  store <4 x i8> %331, <4 x i8> addrspace(1)* %370, align 4
  br label %postload680

postload680:                                      ; preds = %preload679, %postload675
  br i1 %extract142, label %preload684, label %postload685

preload684:                                       ; preds = %postload680
  store <4 x i8> %332, <4 x i8> addrspace(1)* %373, align 4
  br label %postload685

postload685:                                      ; preds = %preload684, %postload680
  br i1 %extract143, label %preload689, label %postload690

preload689:                                       ; preds = %postload685
  store <4 x i8> %333, <4 x i8> addrspace(1)* %376, align 4
  br label %postload690

postload690:                                      ; preds = %preload689, %postload685
  br i1 %extract144, label %preload694, label %postload695

preload694:                                       ; preds = %postload690
  store <4 x i8> %334, <4 x i8> addrspace(1)* %379, align 4
  br label %postload695

postload695:                                      ; preds = %preload694, %postload690
  %391 = add nsw i32 %y.010, 1
  %exitcond18 = icmp eq i32 %391, %height
  %temp281 = insertelement <16 x i1> undef, i1 %exitcond18, i32 0
  %vector282 = shufflevector <16 x i1> %temp281, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond = xor i1 %exitcond18, true
  %temp287 = insertelement <16 x i1> undef, i1 %notCond, i32 0
  %vector288 = shufflevector <16 x i1> %temp287, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr283 = and <16 x i1> %vectorPHI98, %vector282
  %loop_mask54285 = or <16 x i1> %vectorPHI, %who_left_tr283
  %curr_loop_mask286 = or <16 x i1> %loop_mask54285, %who_left_tr283
  %ipred.i1 = bitcast <16 x i1> %curr_loop_mask286 to i16
  %val.i2 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1, i16 %ipred.i1) nounwind
  %tmp.i3 = and i32 %val.i2, 1
  %res.i4 = icmp eq i32 %tmp.i3, 0
  %local_edge289 = and <16 x i1> %vectorPHI98, %vector288
  br i1 %res.i4, label %10, label %._crit_edge

._crit_edge:                                      ; preds = %postload695, %SyncBB
  br i1 %extract290, label %preload784, label %postload785

preload784:                                       ; preds = %._crit_edge
  %check.WI.iter878 = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter878, label %thenBB875, label %elseBB876

thenBB875:                                        ; preds = %preload784
  %"CurrWI++879" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride881" = add nuw i64 %CurrSBIndex..0, 448
  br label %SyncBB

elseBB876:                                        ; preds = %preload784
  call void @llvm.x86.sse2.mfence()
  br label %postload785

postload785:                                      ; preds = %._crit_edge, %elseBB876, %thenBB
  %vector370887 = phi <16 x float> [ %vector370, %elseBB876 ], [ %vector370887, %thenBB ], [ %vector370.le, %._crit_edge ]
  %vector368885 = phi <16 x float> [ %vector368, %elseBB876 ], [ %vector368885, %thenBB ], [ %vector368.le, %._crit_edge ]
  %currBarrier.1 = phi i32 [ 7, %elseBB876 ], [ %currBarrier.1, %thenBB ], [ %currBarrier.2.ph, %._crit_edge ]
  %CurrSBIndex..2 = phi i64 [ 0, %elseBB876 ], [ %"loadedCurrSB+Stride", %thenBB ], [ %CurrSBIndex..0, %._crit_edge ]
  %CurrWI..2 = phi i64 [ 0, %elseBB876 ], [ %"CurrWI++", %thenBB ], [ %CurrWI..0, %._crit_edge ]
  %"&(pSB[currWI].offset)840" = add nuw i64 %CurrSBIndex..2, 384
  %"&pSB[currWI].offset841" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)840"
  %CastToValueType842 = bitcast i8* %"&pSB[currWI].offset841" to <16 x i1>*
  %loadedValue843 = load <16 x i1>* %CastToValueType842, align 16
  %._crit_edge_to_bb.nph308 = and <16 x i1> %loadedValue843, %vector307
  %"&(pSB[currWI].offset)827" = add nuw i64 %CurrSBIndex..2, 320
  %"&pSB[currWI].offset828" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)827"
  %CastToValueType829 = bitcast i8* %"&pSB[currWI].offset828" to <16 x i32>*
  %loadedValue = load <16 x i32>* %CastToValueType829, align 64
  %tmp16311 = add <16 x i32> %vector310, %loadedValue
  %ipred.i5 = bitcast <16 x i1> %._crit_edge_to_bb.nph308 to i16
  %val.i6 = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i5, i16 %ipred.i5) nounwind
  %tmp.i7 = and i32 %val.i6, 1
  %res.i8 = icmp eq i32 %tmp.i7, 0
  br i1 %res.i8, label %.preheader882, label %.loopexit

.preheader882:                                    ; preds = %postload785
  %negIncomingLoopMask78312 = xor <16 x i1> %._crit_edge_to_bb.nph308, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %392

; <label>:392                                     ; preds = %postload783, %.preheader882
  %vectorPHI313 = phi <16 x i1> [ %loop_mask63503, %postload783 ], [ %negIncomingLoopMask78312, %.preheader882 ]
  %vectorPHI315 = phi <16 x i1> [ %local_edge69507, %postload783 ], [ %._crit_edge_to_bb.nph308, %.preheader882 ]
  %indvar = phi i32 [ %indvar.next, %postload783 ], [ 0, %.preheader882 ]
  %vectorPHI316 = phi <16 x float> [ %vectorPHI320, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI317 = phi <16 x float> [ %vectorPHI321, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI318 = phi <16 x float> [ %vectorPHI322, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI319 = phi <16 x float> [ %vectorPHI323, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI320 = phi <16 x float> [ %529, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI321 = phi <16 x float> [ %530, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI322 = phi <16 x float> [ %531, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI323 = phi <16 x float> [ %532, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI324 = phi <16 x float> [ %vectorPHI328, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI325 = phi <16 x float> [ %vectorPHI329, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI326 = phi <16 x float> [ %vectorPHI330, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI327 = phi <16 x float> [ %vectorPHI331, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI328 = phi <16 x float> [ %temp.vect523, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI329 = phi <16 x float> [ %temp.vect539, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI330 = phi <16 x float> [ %temp.vect555, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %vectorPHI331 = phi <16 x float> [ %temp.vect571, %postload783 ], [ zeroinitializer, %.preheader882 ]
  %extract351 = extractelement <16 x i1> %vectorPHI315, i32 0
  %extract352 = extractelement <16 x i1> %vectorPHI315, i32 1
  %extract353 = extractelement <16 x i1> %vectorPHI315, i32 2
  %extract354 = extractelement <16 x i1> %vectorPHI315, i32 3
  %extract355 = extractelement <16 x i1> %vectorPHI315, i32 4
  %extract356 = extractelement <16 x i1> %vectorPHI315, i32 5
  %extract357 = extractelement <16 x i1> %vectorPHI315, i32 6
  %extract358 = extractelement <16 x i1> %vectorPHI315, i32 7
  %extract359 = extractelement <16 x i1> %vectorPHI315, i32 8
  %extract360 = extractelement <16 x i1> %vectorPHI315, i32 9
  %extract361 = extractelement <16 x i1> %vectorPHI315, i32 10
  %extract362 = extractelement <16 x i1> %vectorPHI315, i32 11
  %extract363 = extractelement <16 x i1> %vectorPHI315, i32 12
  %extract364 = extractelement <16 x i1> %vectorPHI315, i32 13
  %extract365 = extractelement <16 x i1> %vectorPHI315, i32 14
  %extract366 = extractelement <16 x i1> %vectorPHI315, i32 15
  %tmp12 = mul i32 %indvar, %tmp
  %temp332 = insertelement <16 x i32> undef, i32 %tmp12, i32 0
  %vector333 = shufflevector <16 x i32> %temp332, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp17334 = add <16 x i32> %tmp16311, %vector333
  %393 = extractelement <16 x i32> %tmp17334, i32 1
  %394 = sext i32 %393 to i64
  %395 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %394
  %396 = extractelement <16 x i32> %tmp17334, i32 2
  %397 = sext i32 %396 to i64
  %398 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %397
  %399 = extractelement <16 x i32> %tmp17334, i32 3
  %400 = sext i32 %399 to i64
  %401 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %400
  %402 = extractelement <16 x i32> %tmp17334, i32 4
  %403 = sext i32 %402 to i64
  %404 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %403
  %405 = extractelement <16 x i32> %tmp17334, i32 5
  %406 = sext i32 %405 to i64
  %407 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %406
  %408 = extractelement <16 x i32> %tmp17334, i32 6
  %409 = sext i32 %408 to i64
  %410 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %409
  %411 = extractelement <16 x i32> %tmp17334, i32 7
  %412 = sext i32 %411 to i64
  %413 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %412
  %414 = extractelement <16 x i32> %tmp17334, i32 8
  %415 = sext i32 %414 to i64
  %416 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %415
  %417 = extractelement <16 x i32> %tmp17334, i32 9
  %418 = sext i32 %417 to i64
  %419 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %418
  %420 = extractelement <16 x i32> %tmp17334, i32 10
  %421 = sext i32 %420 to i64
  %422 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %421
  %423 = extractelement <16 x i32> %tmp17334, i32 11
  %424 = sext i32 %423 to i64
  %425 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %424
  %426 = extractelement <16 x i32> %tmp17334, i32 12
  %427 = sext i32 %426 to i64
  %428 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %427
  %429 = extractelement <16 x i32> %tmp17334, i32 13
  %430 = sext i32 %429 to i64
  %431 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %430
  %432 = extractelement <16 x i32> %tmp17334, i32 14
  %433 = sext i32 %432 to i64
  %434 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %433
  %435 = extractelement <16 x i32> %tmp17334, i32 15
  %436 = sext i32 %435 to i64
  %437 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %436
  br i1 %extract351, label %preload786, label %postload787

preload786:                                       ; preds = %392
  %438 = extractelement <16 x i32> %tmp17334, i32 0
  %439 = sext i32 %438 to i64
  %440 = getelementptr inbounds <4 x i8> addrspace(1)* %input, i64 %439
  %masked_load587 = load <4 x i8> addrspace(1)* %440, align 4
  br label %postload787

postload787:                                      ; preds = %preload786, %392
  %phi788 = phi <4 x i8> [ undef, %392 ], [ %masked_load587, %preload786 ]
  br i1 %extract352, label %preload794, label %postload795

preload794:                                       ; preds = %postload787
  %masked_load588 = load <4 x i8> addrspace(1)* %395, align 4
  br label %postload795

postload795:                                      ; preds = %preload794, %postload787
  %phi796 = phi <4 x i8> [ undef, %postload787 ], [ %masked_load588, %preload794 ]
  br i1 %extract353, label %preload802, label %postload803

preload802:                                       ; preds = %postload795
  %masked_load589 = load <4 x i8> addrspace(1)* %398, align 4
  br label %postload803

postload803:                                      ; preds = %preload802, %postload795
  %phi804 = phi <4 x i8> [ undef, %postload795 ], [ %masked_load589, %preload802 ]
  br i1 %extract354, label %preload810, label %postload811

preload810:                                       ; preds = %postload803
  %masked_load590 = load <4 x i8> addrspace(1)* %401, align 4
  br label %postload811

postload811:                                      ; preds = %preload810, %postload803
  %phi812 = phi <4 x i8> [ undef, %postload803 ], [ %masked_load590, %preload810 ]
  br i1 %extract355, label %preload818, label %postload819

preload818:                                       ; preds = %postload811
  %masked_load591 = load <4 x i8> addrspace(1)* %404, align 4
  br label %postload819

postload819:                                      ; preds = %preload818, %postload811
  %phi820 = phi <4 x i8> [ undef, %postload811 ], [ %masked_load591, %preload818 ]
  br i1 %extract356, label %preload696, label %postload697

preload696:                                       ; preds = %postload819
  %masked_load592 = load <4 x i8> addrspace(1)* %407, align 4
  br label %postload697

postload697:                                      ; preds = %preload696, %postload819
  %phi698 = phi <4 x i8> [ undef, %postload819 ], [ %masked_load592, %preload696 ]
  br i1 %extract357, label %preload704, label %postload705

preload704:                                       ; preds = %postload697
  %masked_load593 = load <4 x i8> addrspace(1)* %410, align 4
  br label %postload705

postload705:                                      ; preds = %preload704, %postload697
  %phi706 = phi <4 x i8> [ undef, %postload697 ], [ %masked_load593, %preload704 ]
  br i1 %extract358, label %preload712, label %postload713

preload712:                                       ; preds = %postload705
  %masked_load594 = load <4 x i8> addrspace(1)* %413, align 4
  br label %postload713

postload713:                                      ; preds = %preload712, %postload705
  %phi714 = phi <4 x i8> [ undef, %postload705 ], [ %masked_load594, %preload712 ]
  br i1 %extract359, label %preload720, label %postload721

preload720:                                       ; preds = %postload713
  %masked_load595 = load <4 x i8> addrspace(1)* %416, align 4
  br label %postload721

postload721:                                      ; preds = %preload720, %postload713
  %phi722 = phi <4 x i8> [ undef, %postload713 ], [ %masked_load595, %preload720 ]
  br i1 %extract360, label %preload728, label %postload729

preload728:                                       ; preds = %postload721
  %masked_load596 = load <4 x i8> addrspace(1)* %419, align 4
  br label %postload729

postload729:                                      ; preds = %preload728, %postload721
  %phi730 = phi <4 x i8> [ undef, %postload721 ], [ %masked_load596, %preload728 ]
  br i1 %extract361, label %preload736, label %postload737

preload736:                                       ; preds = %postload729
  %masked_load597 = load <4 x i8> addrspace(1)* %422, align 4
  br label %postload737

postload737:                                      ; preds = %preload736, %postload729
  %phi738 = phi <4 x i8> [ undef, %postload729 ], [ %masked_load597, %preload736 ]
  br i1 %extract362, label %preload744, label %postload745

preload744:                                       ; preds = %postload737
  %masked_load598 = load <4 x i8> addrspace(1)* %425, align 4
  br label %postload745

postload745:                                      ; preds = %preload744, %postload737
  %phi746 = phi <4 x i8> [ undef, %postload737 ], [ %masked_load598, %preload744 ]
  br i1 %extract363, label %preload752, label %postload753

preload752:                                       ; preds = %postload745
  %masked_load599 = load <4 x i8> addrspace(1)* %428, align 4
  br label %postload753

postload753:                                      ; preds = %preload752, %postload745
  %phi754 = phi <4 x i8> [ undef, %postload745 ], [ %masked_load599, %preload752 ]
  br i1 %extract364, label %preload760, label %postload761

preload760:                                       ; preds = %postload753
  %masked_load600 = load <4 x i8> addrspace(1)* %431, align 4
  br label %postload761

postload761:                                      ; preds = %preload760, %postload753
  %phi762 = phi <4 x i8> [ undef, %postload753 ], [ %masked_load600, %preload760 ]
  br i1 %extract365, label %preload768, label %postload769

preload768:                                       ; preds = %postload761
  %masked_load601 = load <4 x i8> addrspace(1)* %434, align 4
  br label %postload769

postload769:                                      ; preds = %preload768, %postload761
  %phi770 = phi <4 x i8> [ undef, %postload761 ], [ %masked_load601, %preload768 ]
  br i1 %extract366, label %preload776, label %postload777

preload776:                                       ; preds = %postload769
  %masked_load602 = load <4 x i8> addrspace(1)* %437, align 4
  br label %postload777

postload777:                                      ; preds = %preload776, %postload769
  %phi778 = phi <4 x i8> [ undef, %postload769 ], [ %masked_load602, %preload776 ]
  %441 = extractelement <4 x i8> %phi788, i32 0
  %442 = extractelement <4 x i8> %phi796, i32 0
  %443 = extractelement <4 x i8> %phi804, i32 0
  %444 = extractelement <4 x i8> %phi812, i32 0
  %445 = extractelement <4 x i8> %phi820, i32 0
  %446 = extractelement <4 x i8> %phi698, i32 0
  %447 = extractelement <4 x i8> %phi706, i32 0
  %448 = extractelement <4 x i8> %phi714, i32 0
  %449 = extractelement <4 x i8> %phi722, i32 0
  %450 = extractelement <4 x i8> %phi730, i32 0
  %451 = extractelement <4 x i8> %phi738, i32 0
  %452 = extractelement <4 x i8> %phi746, i32 0
  %453 = extractelement <4 x i8> %phi754, i32 0
  %454 = extractelement <4 x i8> %phi762, i32 0
  %455 = extractelement <4 x i8> %phi770, i32 0
  %456 = extractelement <4 x i8> %phi778, i32 0
  %457 = extractelement <4 x i8> %phi788, i32 1
  %458 = extractelement <4 x i8> %phi796, i32 1
  %459 = extractelement <4 x i8> %phi804, i32 1
  %460 = extractelement <4 x i8> %phi812, i32 1
  %461 = extractelement <4 x i8> %phi820, i32 1
  %462 = extractelement <4 x i8> %phi698, i32 1
  %463 = extractelement <4 x i8> %phi706, i32 1
  %464 = extractelement <4 x i8> %phi714, i32 1
  %465 = extractelement <4 x i8> %phi722, i32 1
  %466 = extractelement <4 x i8> %phi730, i32 1
  %467 = extractelement <4 x i8> %phi738, i32 1
  %468 = extractelement <4 x i8> %phi746, i32 1
  %469 = extractelement <4 x i8> %phi754, i32 1
  %470 = extractelement <4 x i8> %phi762, i32 1
  %471 = extractelement <4 x i8> %phi770, i32 1
  %472 = extractelement <4 x i8> %phi778, i32 1
  %473 = extractelement <4 x i8> %phi788, i32 2
  %474 = extractelement <4 x i8> %phi796, i32 2
  %475 = extractelement <4 x i8> %phi804, i32 2
  %476 = extractelement <4 x i8> %phi812, i32 2
  %477 = extractelement <4 x i8> %phi820, i32 2
  %478 = extractelement <4 x i8> %phi698, i32 2
  %479 = extractelement <4 x i8> %phi706, i32 2
  %480 = extractelement <4 x i8> %phi714, i32 2
  %481 = extractelement <4 x i8> %phi722, i32 2
  %482 = extractelement <4 x i8> %phi730, i32 2
  %483 = extractelement <4 x i8> %phi738, i32 2
  %484 = extractelement <4 x i8> %phi746, i32 2
  %485 = extractelement <4 x i8> %phi754, i32 2
  %486 = extractelement <4 x i8> %phi762, i32 2
  %487 = extractelement <4 x i8> %phi770, i32 2
  %488 = extractelement <4 x i8> %phi778, i32 2
  %489 = extractelement <4 x i8> %phi788, i32 3
  %490 = extractelement <4 x i8> %phi796, i32 3
  %491 = extractelement <4 x i8> %phi804, i32 3
  %492 = extractelement <4 x i8> %phi812, i32 3
  %493 = extractelement <4 x i8> %phi820, i32 3
  %494 = extractelement <4 x i8> %phi698, i32 3
  %495 = extractelement <4 x i8> %phi706, i32 3
  %496 = extractelement <4 x i8> %phi714, i32 3
  %497 = extractelement <4 x i8> %phi722, i32 3
  %498 = extractelement <4 x i8> %phi730, i32 3
  %499 = extractelement <4 x i8> %phi738, i32 3
  %500 = extractelement <4 x i8> %phi746, i32 3
  %501 = extractelement <4 x i8> %phi754, i32 3
  %502 = extractelement <4 x i8> %phi762, i32 3
  %503 = extractelement <4 x i8> %phi770, i32 3
  %504 = extractelement <4 x i8> %phi778, i32 3
  %505 = fmul <16 x float> %vector368885, %vectorPHI328
  %506 = fmul <16 x float> %vector368885, %vectorPHI329
  %507 = fmul <16 x float> %vector368885, %vectorPHI330
  %508 = fmul <16 x float> %vector368885, %vectorPHI331
  %509 = fmul <16 x float> %vector370887, %vectorPHI324
  %510 = fmul <16 x float> %vector370887, %vectorPHI325
  %511 = fmul <16 x float> %vector370887, %vectorPHI326
  %512 = fmul <16 x float> %vector370887, %vectorPHI327
  %513 = fadd <16 x float> %505, %509
  %514 = fadd <16 x float> %506, %510
  %515 = fadd <16 x float> %507, %511
  %516 = fadd <16 x float> %508, %512
  %517 = fmul <16 x float> %vector214, %vectorPHI320
  %518 = fmul <16 x float> %vector214, %vectorPHI321
  %519 = fmul <16 x float> %vector214, %vectorPHI322
  %520 = fmul <16 x float> %vector214, %vectorPHI323
  %521 = fsub <16 x float> %513, %517
  %522 = fsub <16 x float> %514, %518
  %523 = fsub <16 x float> %515, %519
  %524 = fsub <16 x float> %516, %520
  %525 = fmul <16 x float> %vector216, %vectorPHI316
  %526 = fmul <16 x float> %vector216, %vectorPHI317
  %527 = fmul <16 x float> %vector216, %vectorPHI318
  %528 = fmul <16 x float> %vector216, %vectorPHI319
  %529 = fsub <16 x float> %521, %525
  %530 = fsub <16 x float> %522, %526
  %531 = fsub <16 x float> %523, %527
  %532 = fsub <16 x float> %524, %528
  %533 = extractelement <16 x i32> %tmp17334, i32 0
  %534 = sext i32 %533 to i64
  %535 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %534
  %536 = extractelement <16 x i32> %tmp17334, i32 1
  %537 = sext i32 %536 to i64
  %538 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %537
  %539 = extractelement <16 x i32> %tmp17334, i32 2
  %540 = sext i32 %539 to i64
  %541 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %540
  %542 = extractelement <16 x i32> %tmp17334, i32 3
  %543 = sext i32 %542 to i64
  %544 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %543
  %545 = extractelement <16 x i32> %tmp17334, i32 4
  %546 = sext i32 %545 to i64
  %547 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %546
  %548 = extractelement <16 x i32> %tmp17334, i32 5
  %549 = sext i32 %548 to i64
  %550 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %549
  %551 = extractelement <16 x i32> %tmp17334, i32 6
  %552 = sext i32 %551 to i64
  %553 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %552
  %554 = extractelement <16 x i32> %tmp17334, i32 7
  %555 = sext i32 %554 to i64
  %556 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %555
  %557 = extractelement <16 x i32> %tmp17334, i32 8
  %558 = sext i32 %557 to i64
  %559 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %558
  %560 = extractelement <16 x i32> %tmp17334, i32 9
  %561 = sext i32 %560 to i64
  %562 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %561
  %563 = extractelement <16 x i32> %tmp17334, i32 10
  %564 = sext i32 %563 to i64
  %565 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %564
  %566 = extractelement <16 x i32> %tmp17334, i32 11
  %567 = sext i32 %566 to i64
  %568 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %567
  %569 = extractelement <16 x i32> %tmp17334, i32 12
  %570 = sext i32 %569 to i64
  %571 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %570
  %572 = extractelement <16 x i32> %tmp17334, i32 13
  %573 = sext i32 %572 to i64
  %574 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %573
  %575 = extractelement <16 x i32> %tmp17334, i32 14
  %576 = sext i32 %575 to i64
  %577 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %576
  %578 = extractelement <16 x i32> %tmp17334, i32 15
  %579 = sext i32 %578 to i64
  %580 = getelementptr inbounds <4 x i8> addrspace(1)* %output, i64 %579
  br i1 %extract351, label %preload789, label %postload790

preload789:                                       ; preds = %postload777
  %masked_load603 = load <4 x i8> addrspace(1)* %535, align 4
  br label %postload790

postload790:                                      ; preds = %preload789, %postload777
  %phi791 = phi <4 x i8> [ undef, %postload777 ], [ %masked_load603, %preload789 ]
  br i1 %extract352, label %preload797, label %postload798

preload797:                                       ; preds = %postload790
  %masked_load604 = load <4 x i8> addrspace(1)* %538, align 4
  br label %postload798

postload798:                                      ; preds = %preload797, %postload790
  %phi799 = phi <4 x i8> [ undef, %postload790 ], [ %masked_load604, %preload797 ]
  br i1 %extract353, label %preload805, label %postload806

preload805:                                       ; preds = %postload798
  %masked_load605 = load <4 x i8> addrspace(1)* %541, align 4
  br label %postload806

postload806:                                      ; preds = %preload805, %postload798
  %phi807 = phi <4 x i8> [ undef, %postload798 ], [ %masked_load605, %preload805 ]
  br i1 %extract354, label %preload813, label %postload814

preload813:                                       ; preds = %postload806
  %masked_load606 = load <4 x i8> addrspace(1)* %544, align 4
  br label %postload814

postload814:                                      ; preds = %preload813, %postload806
  %phi815 = phi <4 x i8> [ undef, %postload806 ], [ %masked_load606, %preload813 ]
  br i1 %extract355, label %preload821, label %postload822

preload821:                                       ; preds = %postload814
  %masked_load607 = load <4 x i8> addrspace(1)* %547, align 4
  br label %postload822

postload822:                                      ; preds = %preload821, %postload814
  %phi823 = phi <4 x i8> [ undef, %postload814 ], [ %masked_load607, %preload821 ]
  br i1 %extract356, label %preload699, label %postload700

preload699:                                       ; preds = %postload822
  %masked_load608 = load <4 x i8> addrspace(1)* %550, align 4
  br label %postload700

postload700:                                      ; preds = %preload699, %postload822
  %phi701 = phi <4 x i8> [ undef, %postload822 ], [ %masked_load608, %preload699 ]
  br i1 %extract357, label %preload707, label %postload708

preload707:                                       ; preds = %postload700
  %masked_load609 = load <4 x i8> addrspace(1)* %553, align 4
  br label %postload708

postload708:                                      ; preds = %preload707, %postload700
  %phi709 = phi <4 x i8> [ undef, %postload700 ], [ %masked_load609, %preload707 ]
  br i1 %extract358, label %preload715, label %postload716

preload715:                                       ; preds = %postload708
  %masked_load610 = load <4 x i8> addrspace(1)* %556, align 4
  br label %postload716

postload716:                                      ; preds = %preload715, %postload708
  %phi717 = phi <4 x i8> [ undef, %postload708 ], [ %masked_load610, %preload715 ]
  br i1 %extract359, label %preload723, label %postload724

preload723:                                       ; preds = %postload716
  %masked_load611 = load <4 x i8> addrspace(1)* %559, align 4
  br label %postload724

postload724:                                      ; preds = %preload723, %postload716
  %phi725 = phi <4 x i8> [ undef, %postload716 ], [ %masked_load611, %preload723 ]
  br i1 %extract360, label %preload731, label %postload732

preload731:                                       ; preds = %postload724
  %masked_load612 = load <4 x i8> addrspace(1)* %562, align 4
  br label %postload732

postload732:                                      ; preds = %preload731, %postload724
  %phi733 = phi <4 x i8> [ undef, %postload724 ], [ %masked_load612, %preload731 ]
  br i1 %extract361, label %preload739, label %postload740

preload739:                                       ; preds = %postload732
  %masked_load613 = load <4 x i8> addrspace(1)* %565, align 4
  br label %postload740

postload740:                                      ; preds = %preload739, %postload732
  %phi741 = phi <4 x i8> [ undef, %postload732 ], [ %masked_load613, %preload739 ]
  br i1 %extract362, label %preload747, label %postload748

preload747:                                       ; preds = %postload740
  %masked_load614 = load <4 x i8> addrspace(1)* %568, align 4
  br label %postload748

postload748:                                      ; preds = %preload747, %postload740
  %phi749 = phi <4 x i8> [ undef, %postload740 ], [ %masked_load614, %preload747 ]
  br i1 %extract363, label %preload755, label %postload756

preload755:                                       ; preds = %postload748
  %masked_load615 = load <4 x i8> addrspace(1)* %571, align 4
  br label %postload756

postload756:                                      ; preds = %preload755, %postload748
  %phi757 = phi <4 x i8> [ undef, %postload748 ], [ %masked_load615, %preload755 ]
  br i1 %extract364, label %preload763, label %postload764

preload763:                                       ; preds = %postload756
  %masked_load616 = load <4 x i8> addrspace(1)* %574, align 4
  br label %postload764

postload764:                                      ; preds = %preload763, %postload756
  %phi765 = phi <4 x i8> [ undef, %postload756 ], [ %masked_load616, %preload763 ]
  br i1 %extract365, label %preload771, label %postload772

preload771:                                       ; preds = %postload764
  %masked_load617 = load <4 x i8> addrspace(1)* %577, align 4
  br label %postload772

postload772:                                      ; preds = %preload771, %postload764
  %phi773 = phi <4 x i8> [ undef, %postload764 ], [ %masked_load617, %preload771 ]
  br i1 %extract366, label %preload779, label %postload780

preload779:                                       ; preds = %postload772
  %masked_load618 = load <4 x i8> addrspace(1)* %580, align 4
  br label %postload780

postload780:                                      ; preds = %preload779, %postload772
  %phi781 = phi <4 x i8> [ undef, %postload772 ], [ %masked_load618, %preload779 ]
  %581 = extractelement <4 x i8> %phi791, i32 0
  %582 = extractelement <4 x i8> %phi799, i32 0
  %583 = extractelement <4 x i8> %phi807, i32 0
  %584 = extractelement <4 x i8> %phi815, i32 0
  %585 = extractelement <4 x i8> %phi823, i32 0
  %586 = extractelement <4 x i8> %phi701, i32 0
  %587 = extractelement <4 x i8> %phi709, i32 0
  %588 = extractelement <4 x i8> %phi717, i32 0
  %589 = extractelement <4 x i8> %phi725, i32 0
  %590 = extractelement <4 x i8> %phi733, i32 0
  %591 = extractelement <4 x i8> %phi741, i32 0
  %592 = extractelement <4 x i8> %phi749, i32 0
  %593 = extractelement <4 x i8> %phi757, i32 0
  %594 = extractelement <4 x i8> %phi765, i32 0
  %595 = extractelement <4 x i8> %phi773, i32 0
  %596 = extractelement <4 x i8> %phi781, i32 0
  %597 = extractelement <4 x i8> %phi791, i32 1
  %598 = extractelement <4 x i8> %phi799, i32 1
  %599 = extractelement <4 x i8> %phi807, i32 1
  %600 = extractelement <4 x i8> %phi815, i32 1
  %601 = extractelement <4 x i8> %phi823, i32 1
  %602 = extractelement <4 x i8> %phi701, i32 1
  %603 = extractelement <4 x i8> %phi709, i32 1
  %604 = extractelement <4 x i8> %phi717, i32 1
  %605 = extractelement <4 x i8> %phi725, i32 1
  %606 = extractelement <4 x i8> %phi733, i32 1
  %607 = extractelement <4 x i8> %phi741, i32 1
  %608 = extractelement <4 x i8> %phi749, i32 1
  %609 = extractelement <4 x i8> %phi757, i32 1
  %610 = extractelement <4 x i8> %phi765, i32 1
  %611 = extractelement <4 x i8> %phi773, i32 1
  %612 = extractelement <4 x i8> %phi781, i32 1
  %613 = extractelement <4 x i8> %phi791, i32 2
  %614 = extractelement <4 x i8> %phi799, i32 2
  %615 = extractelement <4 x i8> %phi807, i32 2
  %616 = extractelement <4 x i8> %phi815, i32 2
  %617 = extractelement <4 x i8> %phi823, i32 2
  %618 = extractelement <4 x i8> %phi701, i32 2
  %619 = extractelement <4 x i8> %phi709, i32 2
  %620 = extractelement <4 x i8> %phi717, i32 2
  %621 = extractelement <4 x i8> %phi725, i32 2
  %622 = extractelement <4 x i8> %phi733, i32 2
  %623 = extractelement <4 x i8> %phi741, i32 2
  %624 = extractelement <4 x i8> %phi749, i32 2
  %625 = extractelement <4 x i8> %phi757, i32 2
  %626 = extractelement <4 x i8> %phi765, i32 2
  %627 = extractelement <4 x i8> %phi773, i32 2
  %628 = extractelement <4 x i8> %phi781, i32 2
  %629 = extractelement <4 x i8> %phi791, i32 3
  %630 = extractelement <4 x i8> %phi799, i32 3
  %631 = extractelement <4 x i8> %phi807, i32 3
  %632 = extractelement <4 x i8> %phi815, i32 3
  %633 = extractelement <4 x i8> %phi823, i32 3
  %634 = extractelement <4 x i8> %phi701, i32 3
  %635 = extractelement <4 x i8> %phi709, i32 3
  %636 = extractelement <4 x i8> %phi717, i32 3
  %637 = extractelement <4 x i8> %phi725, i32 3
  %638 = extractelement <4 x i8> %phi733, i32 3
  %639 = extractelement <4 x i8> %phi741, i32 3
  %640 = extractelement <4 x i8> %phi749, i32 3
  %641 = extractelement <4 x i8> %phi757, i32 3
  %642 = extractelement <4 x i8> %phi765, i32 3
  %643 = extractelement <4 x i8> %phi773, i32 3
  %644 = extractelement <4 x i8> %phi781, i32 3
  %645 = uitofp i8 %581 to float
  %646 = uitofp i8 %582 to float
  %647 = uitofp i8 %583 to float
  %648 = uitofp i8 %584 to float
  %649 = uitofp i8 %585 to float
  %650 = uitofp i8 %586 to float
  %651 = uitofp i8 %587 to float
  %652 = uitofp i8 %588 to float
  %653 = uitofp i8 %589 to float
  %654 = uitofp i8 %590 to float
  %655 = uitofp i8 %591 to float
  %656 = uitofp i8 %592 to float
  %657 = uitofp i8 %593 to float
  %658 = uitofp i8 %594 to float
  %659 = uitofp i8 %595 to float
  %660 = uitofp i8 %596 to float
  %temp.vect371 = insertelement <16 x float> undef, float %645, i32 0
  %temp.vect372 = insertelement <16 x float> %temp.vect371, float %646, i32 1
  %temp.vect373 = insertelement <16 x float> %temp.vect372, float %647, i32 2
  %temp.vect374 = insertelement <16 x float> %temp.vect373, float %648, i32 3
  %temp.vect375 = insertelement <16 x float> %temp.vect374, float %649, i32 4
  %temp.vect376 = insertelement <16 x float> %temp.vect375, float %650, i32 5
  %temp.vect377 = insertelement <16 x float> %temp.vect376, float %651, i32 6
  %temp.vect378 = insertelement <16 x float> %temp.vect377, float %652, i32 7
  %temp.vect379 = insertelement <16 x float> %temp.vect378, float %653, i32 8
  %temp.vect380 = insertelement <16 x float> %temp.vect379, float %654, i32 9
  %temp.vect381 = insertelement <16 x float> %temp.vect380, float %655, i32 10
  %temp.vect382 = insertelement <16 x float> %temp.vect381, float %656, i32 11
  %temp.vect383 = insertelement <16 x float> %temp.vect382, float %657, i32 12
  %temp.vect384 = insertelement <16 x float> %temp.vect383, float %658, i32 13
  %temp.vect385 = insertelement <16 x float> %temp.vect384, float %659, i32 14
  %temp.vect386 = insertelement <16 x float> %temp.vect385, float %660, i32 15
  %661 = uitofp i8 %597 to float
  %662 = uitofp i8 %598 to float
  %663 = uitofp i8 %599 to float
  %664 = uitofp i8 %600 to float
  %665 = uitofp i8 %601 to float
  %666 = uitofp i8 %602 to float
  %667 = uitofp i8 %603 to float
  %668 = uitofp i8 %604 to float
  %669 = uitofp i8 %605 to float
  %670 = uitofp i8 %606 to float
  %671 = uitofp i8 %607 to float
  %672 = uitofp i8 %608 to float
  %673 = uitofp i8 %609 to float
  %674 = uitofp i8 %610 to float
  %675 = uitofp i8 %611 to float
  %676 = uitofp i8 %612 to float
  %temp.vect387 = insertelement <16 x float> undef, float %661, i32 0
  %temp.vect388 = insertelement <16 x float> %temp.vect387, float %662, i32 1
  %temp.vect389 = insertelement <16 x float> %temp.vect388, float %663, i32 2
  %temp.vect390 = insertelement <16 x float> %temp.vect389, float %664, i32 3
  %temp.vect391 = insertelement <16 x float> %temp.vect390, float %665, i32 4
  %temp.vect392 = insertelement <16 x float> %temp.vect391, float %666, i32 5
  %temp.vect393 = insertelement <16 x float> %temp.vect392, float %667, i32 6
  %temp.vect394 = insertelement <16 x float> %temp.vect393, float %668, i32 7
  %temp.vect395 = insertelement <16 x float> %temp.vect394, float %669, i32 8
  %temp.vect396 = insertelement <16 x float> %temp.vect395, float %670, i32 9
  %temp.vect397 = insertelement <16 x float> %temp.vect396, float %671, i32 10
  %temp.vect398 = insertelement <16 x float> %temp.vect397, float %672, i32 11
  %temp.vect399 = insertelement <16 x float> %temp.vect398, float %673, i32 12
  %temp.vect400 = insertelement <16 x float> %temp.vect399, float %674, i32 13
  %temp.vect401 = insertelement <16 x float> %temp.vect400, float %675, i32 14
  %temp.vect402 = insertelement <16 x float> %temp.vect401, float %676, i32 15
  %677 = uitofp i8 %613 to float
  %678 = uitofp i8 %614 to float
  %679 = uitofp i8 %615 to float
  %680 = uitofp i8 %616 to float
  %681 = uitofp i8 %617 to float
  %682 = uitofp i8 %618 to float
  %683 = uitofp i8 %619 to float
  %684 = uitofp i8 %620 to float
  %685 = uitofp i8 %621 to float
  %686 = uitofp i8 %622 to float
  %687 = uitofp i8 %623 to float
  %688 = uitofp i8 %624 to float
  %689 = uitofp i8 %625 to float
  %690 = uitofp i8 %626 to float
  %691 = uitofp i8 %627 to float
  %692 = uitofp i8 %628 to float
  %temp.vect403 = insertelement <16 x float> undef, float %677, i32 0
  %temp.vect404 = insertelement <16 x float> %temp.vect403, float %678, i32 1
  %temp.vect405 = insertelement <16 x float> %temp.vect404, float %679, i32 2
  %temp.vect406 = insertelement <16 x float> %temp.vect405, float %680, i32 3
  %temp.vect407 = insertelement <16 x float> %temp.vect406, float %681, i32 4
  %temp.vect408 = insertelement <16 x float> %temp.vect407, float %682, i32 5
  %temp.vect409 = insertelement <16 x float> %temp.vect408, float %683, i32 6
  %temp.vect410 = insertelement <16 x float> %temp.vect409, float %684, i32 7
  %temp.vect411 = insertelement <16 x float> %temp.vect410, float %685, i32 8
  %temp.vect412 = insertelement <16 x float> %temp.vect411, float %686, i32 9
  %temp.vect413 = insertelement <16 x float> %temp.vect412, float %687, i32 10
  %temp.vect414 = insertelement <16 x float> %temp.vect413, float %688, i32 11
  %temp.vect415 = insertelement <16 x float> %temp.vect414, float %689, i32 12
  %temp.vect416 = insertelement <16 x float> %temp.vect415, float %690, i32 13
  %temp.vect417 = insertelement <16 x float> %temp.vect416, float %691, i32 14
  %temp.vect418 = insertelement <16 x float> %temp.vect417, float %692, i32 15
  %693 = uitofp i8 %629 to float
  %694 = uitofp i8 %630 to float
  %695 = uitofp i8 %631 to float
  %696 = uitofp i8 %632 to float
  %697 = uitofp i8 %633 to float
  %698 = uitofp i8 %634 to float
  %699 = uitofp i8 %635 to float
  %700 = uitofp i8 %636 to float
  %701 = uitofp i8 %637 to float
  %702 = uitofp i8 %638 to float
  %703 = uitofp i8 %639 to float
  %704 = uitofp i8 %640 to float
  %705 = uitofp i8 %641 to float
  %706 = uitofp i8 %642 to float
  %707 = uitofp i8 %643 to float
  %708 = uitofp i8 %644 to float
  %temp.vect419 = insertelement <16 x float> undef, float %693, i32 0
  %temp.vect420 = insertelement <16 x float> %temp.vect419, float %694, i32 1
  %temp.vect421 = insertelement <16 x float> %temp.vect420, float %695, i32 2
  %temp.vect422 = insertelement <16 x float> %temp.vect421, float %696, i32 3
  %temp.vect423 = insertelement <16 x float> %temp.vect422, float %697, i32 4
  %temp.vect424 = insertelement <16 x float> %temp.vect423, float %698, i32 5
  %temp.vect425 = insertelement <16 x float> %temp.vect424, float %699, i32 6
  %temp.vect426 = insertelement <16 x float> %temp.vect425, float %700, i32 7
  %temp.vect427 = insertelement <16 x float> %temp.vect426, float %701, i32 8
  %temp.vect428 = insertelement <16 x float> %temp.vect427, float %702, i32 9
  %temp.vect429 = insertelement <16 x float> %temp.vect428, float %703, i32 10
  %temp.vect430 = insertelement <16 x float> %temp.vect429, float %704, i32 11
  %temp.vect431 = insertelement <16 x float> %temp.vect430, float %705, i32 12
  %temp.vect432 = insertelement <16 x float> %temp.vect431, float %706, i32 13
  %temp.vect433 = insertelement <16 x float> %temp.vect432, float %707, i32 14
  %temp.vect434 = insertelement <16 x float> %temp.vect433, float %708, i32 15
  %709 = fadd <16 x float> %temp.vect386, %529
  %extract436 = extractelement <16 x float> %709, i32 1
  %extract437 = extractelement <16 x float> %709, i32 2
  %extract438 = extractelement <16 x float> %709, i32 3
  %extract439 = extractelement <16 x float> %709, i32 4
  %extract440 = extractelement <16 x float> %709, i32 5
  %extract441 = extractelement <16 x float> %709, i32 6
  %extract442 = extractelement <16 x float> %709, i32 7
  %extract443 = extractelement <16 x float> %709, i32 8
  %extract444 = extractelement <16 x float> %709, i32 9
  %extract445 = extractelement <16 x float> %709, i32 10
  %extract446 = extractelement <16 x float> %709, i32 11
  %extract447 = extractelement <16 x float> %709, i32 12
  %extract448 = extractelement <16 x float> %709, i32 13
  %extract449 = extractelement <16 x float> %709, i32 14
  %extract450 = extractelement <16 x float> %709, i32 15
  %710 = fadd <16 x float> %temp.vect402, %530
  %extract452 = extractelement <16 x float> %710, i32 1
  %extract453 = extractelement <16 x float> %710, i32 2
  %extract454 = extractelement <16 x float> %710, i32 3
  %extract455 = extractelement <16 x float> %710, i32 4
  %extract456 = extractelement <16 x float> %710, i32 5
  %extract457 = extractelement <16 x float> %710, i32 6
  %extract458 = extractelement <16 x float> %710, i32 7
  %extract459 = extractelement <16 x float> %710, i32 8
  %extract460 = extractelement <16 x float> %710, i32 9
  %extract461 = extractelement <16 x float> %710, i32 10
  %extract462 = extractelement <16 x float> %710, i32 11
  %extract463 = extractelement <16 x float> %710, i32 12
  %extract464 = extractelement <16 x float> %710, i32 13
  %extract465 = extractelement <16 x float> %710, i32 14
  %extract466 = extractelement <16 x float> %710, i32 15
  %711 = fadd <16 x float> %temp.vect418, %531
  %extract468 = extractelement <16 x float> %711, i32 1
  %extract469 = extractelement <16 x float> %711, i32 2
  %extract470 = extractelement <16 x float> %711, i32 3
  %extract471 = extractelement <16 x float> %711, i32 4
  %extract472 = extractelement <16 x float> %711, i32 5
  %extract473 = extractelement <16 x float> %711, i32 6
  %extract474 = extractelement <16 x float> %711, i32 7
  %extract475 = extractelement <16 x float> %711, i32 8
  %extract476 = extractelement <16 x float> %711, i32 9
  %extract477 = extractelement <16 x float> %711, i32 10
  %extract478 = extractelement <16 x float> %711, i32 11
  %extract479 = extractelement <16 x float> %711, i32 12
  %extract480 = extractelement <16 x float> %711, i32 13
  %extract481 = extractelement <16 x float> %711, i32 14
  %extract482 = extractelement <16 x float> %711, i32 15
  %712 = fadd <16 x float> %temp.vect434, %532
  %extract484 = extractelement <16 x float> %712, i32 1
  %extract485 = extractelement <16 x float> %712, i32 2
  %extract486 = extractelement <16 x float> %712, i32 3
  %extract487 = extractelement <16 x float> %712, i32 4
  %extract488 = extractelement <16 x float> %712, i32 5
  %extract489 = extractelement <16 x float> %712, i32 6
  %extract490 = extractelement <16 x float> %712, i32 7
  %extract491 = extractelement <16 x float> %712, i32 8
  %extract492 = extractelement <16 x float> %712, i32 9
  %extract493 = extractelement <16 x float> %712, i32 10
  %extract494 = extractelement <16 x float> %712, i32 11
  %extract495 = extractelement <16 x float> %712, i32 12
  %extract496 = extractelement <16 x float> %712, i32 13
  %extract497 = extractelement <16 x float> %712, i32 14
  %extract498 = extractelement <16 x float> %712, i32 15
  %713 = fptoui float %extract436 to i8
  %714 = fptoui float %extract437 to i8
  %715 = fptoui float %extract438 to i8
  %716 = fptoui float %extract439 to i8
  %717 = fptoui float %extract440 to i8
  %718 = fptoui float %extract441 to i8
  %719 = fptoui float %extract442 to i8
  %720 = fptoui float %extract443 to i8
  %721 = fptoui float %extract444 to i8
  %722 = fptoui float %extract445 to i8
  %723 = fptoui float %extract446 to i8
  %724 = fptoui float %extract447 to i8
  %725 = fptoui float %extract448 to i8
  %726 = fptoui float %extract449 to i8
  %727 = fptoui float %extract450 to i8
  %728 = fptoui float %extract452 to i8
  %729 = fptoui float %extract453 to i8
  %730 = fptoui float %extract454 to i8
  %731 = fptoui float %extract455 to i8
  %732 = fptoui float %extract456 to i8
  %733 = fptoui float %extract457 to i8
  %734 = fptoui float %extract458 to i8
  %735 = fptoui float %extract459 to i8
  %736 = fptoui float %extract460 to i8
  %737 = fptoui float %extract461 to i8
  %738 = fptoui float %extract462 to i8
  %739 = fptoui float %extract463 to i8
  %740 = fptoui float %extract464 to i8
  %741 = fptoui float %extract465 to i8
  %742 = fptoui float %extract466 to i8
  %743 = fptoui float %extract468 to i8
  %744 = fptoui float %extract469 to i8
  %745 = fptoui float %extract470 to i8
  %746 = fptoui float %extract471 to i8
  %747 = fptoui float %extract472 to i8
  %748 = fptoui float %extract473 to i8
  %749 = fptoui float %extract474 to i8
  %750 = fptoui float %extract475 to i8
  %751 = fptoui float %extract476 to i8
  %752 = fptoui float %extract477 to i8
  %753 = fptoui float %extract478 to i8
  %754 = fptoui float %extract479 to i8
  %755 = fptoui float %extract480 to i8
  %756 = fptoui float %extract481 to i8
  %757 = fptoui float %extract482 to i8
  %758 = fptoui float %extract484 to i8
  %759 = fptoui float %extract485 to i8
  %760 = fptoui float %extract486 to i8
  %761 = fptoui float %extract487 to i8
  %762 = fptoui float %extract488 to i8
  %763 = fptoui float %extract489 to i8
  %764 = fptoui float %extract490 to i8
  %765 = fptoui float %extract491 to i8
  %766 = fptoui float %extract492 to i8
  %767 = fptoui float %extract493 to i8
  %768 = fptoui float %extract494 to i8
  %769 = fptoui float %extract495 to i8
  %770 = fptoui float %extract496 to i8
  %771 = fptoui float %extract497 to i8
  %772 = fptoui float %extract498 to i8
  %773 = insertelement <4 x i8> undef, i8 %713, i32 0
  %774 = insertelement <4 x i8> undef, i8 %714, i32 0
  %775 = insertelement <4 x i8> undef, i8 %715, i32 0
  %776 = insertelement <4 x i8> undef, i8 %716, i32 0
  %777 = insertelement <4 x i8> undef, i8 %717, i32 0
  %778 = insertelement <4 x i8> undef, i8 %718, i32 0
  %779 = insertelement <4 x i8> undef, i8 %719, i32 0
  %780 = insertelement <4 x i8> undef, i8 %720, i32 0
  %781 = insertelement <4 x i8> undef, i8 %721, i32 0
  %782 = insertelement <4 x i8> undef, i8 %722, i32 0
  %783 = insertelement <4 x i8> undef, i8 %723, i32 0
  %784 = insertelement <4 x i8> undef, i8 %724, i32 0
  %785 = insertelement <4 x i8> undef, i8 %725, i32 0
  %786 = insertelement <4 x i8> undef, i8 %726, i32 0
  %787 = insertelement <4 x i8> undef, i8 %727, i32 0
  %788 = insertelement <4 x i8> %773, i8 %728, i32 1
  %789 = insertelement <4 x i8> %774, i8 %729, i32 1
  %790 = insertelement <4 x i8> %775, i8 %730, i32 1
  %791 = insertelement <4 x i8> %776, i8 %731, i32 1
  %792 = insertelement <4 x i8> %777, i8 %732, i32 1
  %793 = insertelement <4 x i8> %778, i8 %733, i32 1
  %794 = insertelement <4 x i8> %779, i8 %734, i32 1
  %795 = insertelement <4 x i8> %780, i8 %735, i32 1
  %796 = insertelement <4 x i8> %781, i8 %736, i32 1
  %797 = insertelement <4 x i8> %782, i8 %737, i32 1
  %798 = insertelement <4 x i8> %783, i8 %738, i32 1
  %799 = insertelement <4 x i8> %784, i8 %739, i32 1
  %800 = insertelement <4 x i8> %785, i8 %740, i32 1
  %801 = insertelement <4 x i8> %786, i8 %741, i32 1
  %802 = insertelement <4 x i8> %787, i8 %742, i32 1
  %803 = insertelement <4 x i8> %788, i8 %743, i32 2
  %804 = insertelement <4 x i8> %789, i8 %744, i32 2
  %805 = insertelement <4 x i8> %790, i8 %745, i32 2
  %806 = insertelement <4 x i8> %791, i8 %746, i32 2
  %807 = insertelement <4 x i8> %792, i8 %747, i32 2
  %808 = insertelement <4 x i8> %793, i8 %748, i32 2
  %809 = insertelement <4 x i8> %794, i8 %749, i32 2
  %810 = insertelement <4 x i8> %795, i8 %750, i32 2
  %811 = insertelement <4 x i8> %796, i8 %751, i32 2
  %812 = insertelement <4 x i8> %797, i8 %752, i32 2
  %813 = insertelement <4 x i8> %798, i8 %753, i32 2
  %814 = insertelement <4 x i8> %799, i8 %754, i32 2
  %815 = insertelement <4 x i8> %800, i8 %755, i32 2
  %816 = insertelement <4 x i8> %801, i8 %756, i32 2
  %817 = insertelement <4 x i8> %802, i8 %757, i32 2
  %818 = insertelement <4 x i8> %803, i8 %758, i32 3
  %819 = insertelement <4 x i8> %804, i8 %759, i32 3
  %820 = insertelement <4 x i8> %805, i8 %760, i32 3
  %821 = insertelement <4 x i8> %806, i8 %761, i32 3
  %822 = insertelement <4 x i8> %807, i8 %762, i32 3
  %823 = insertelement <4 x i8> %808, i8 %763, i32 3
  %824 = insertelement <4 x i8> %809, i8 %764, i32 3
  %825 = insertelement <4 x i8> %810, i8 %765, i32 3
  %826 = insertelement <4 x i8> %811, i8 %766, i32 3
  %827 = insertelement <4 x i8> %812, i8 %767, i32 3
  %828 = insertelement <4 x i8> %813, i8 %768, i32 3
  %829 = insertelement <4 x i8> %814, i8 %769, i32 3
  %830 = insertelement <4 x i8> %815, i8 %770, i32 3
  %831 = insertelement <4 x i8> %816, i8 %771, i32 3
  %832 = insertelement <4 x i8> %817, i8 %772, i32 3
  br i1 %extract351, label %preload792, label %postload793

preload792:                                       ; preds = %postload780
  %extract435 = extractelement <16 x float> %709, i32 0
  %833 = fptoui float %extract435 to i8
  %extract451 = extractelement <16 x float> %710, i32 0
  %834 = insertelement <4 x i8> undef, i8 %833, i32 0
  %835 = fptoui float %extract451 to i8
  %extract467 = extractelement <16 x float> %711, i32 0
  %836 = insertelement <4 x i8> %834, i8 %835, i32 1
  %837 = fptoui float %extract467 to i8
  %extract483 = extractelement <16 x float> %712, i32 0
  %838 = insertelement <4 x i8> %836, i8 %837, i32 2
  %839 = fptoui float %extract483 to i8
  %840 = insertelement <4 x i8> %838, i8 %839, i32 3
  store <4 x i8> %840, <4 x i8> addrspace(1)* %535, align 4
  br label %postload793

postload793:                                      ; preds = %preload792, %postload780
  br i1 %extract352, label %preload800, label %postload801

preload800:                                       ; preds = %postload793
  store <4 x i8> %818, <4 x i8> addrspace(1)* %538, align 4
  br label %postload801

postload801:                                      ; preds = %preload800, %postload793
  br i1 %extract353, label %preload808, label %postload809

preload808:                                       ; preds = %postload801
  store <4 x i8> %819, <4 x i8> addrspace(1)* %541, align 4
  br label %postload809

postload809:                                      ; preds = %preload808, %postload801
  br i1 %extract354, label %preload816, label %postload817

preload816:                                       ; preds = %postload809
  store <4 x i8> %820, <4 x i8> addrspace(1)* %544, align 4
  br label %postload817

postload817:                                      ; preds = %preload816, %postload809
  br i1 %extract355, label %preload824, label %postload825

preload824:                                       ; preds = %postload817
  store <4 x i8> %821, <4 x i8> addrspace(1)* %547, align 4
  br label %postload825

postload825:                                      ; preds = %preload824, %postload817
  br i1 %extract356, label %preload702, label %postload703

preload702:                                       ; preds = %postload825
  store <4 x i8> %822, <4 x i8> addrspace(1)* %550, align 4
  br label %postload703

postload703:                                      ; preds = %preload702, %postload825
  br i1 %extract357, label %preload710, label %postload711

preload710:                                       ; preds = %postload703
  store <4 x i8> %823, <4 x i8> addrspace(1)* %553, align 4
  br label %postload711

postload711:                                      ; preds = %preload710, %postload703
  br i1 %extract358, label %preload718, label %postload719

preload718:                                       ; preds = %postload711
  store <4 x i8> %824, <4 x i8> addrspace(1)* %556, align 4
  br label %postload719

postload719:                                      ; preds = %preload718, %postload711
  br i1 %extract359, label %preload726, label %postload727

preload726:                                       ; preds = %postload719
  store <4 x i8> %825, <4 x i8> addrspace(1)* %559, align 4
  br label %postload727

postload727:                                      ; preds = %preload726, %postload719
  br i1 %extract360, label %preload734, label %postload735

preload734:                                       ; preds = %postload727
  store <4 x i8> %826, <4 x i8> addrspace(1)* %562, align 4
  br label %postload735

postload735:                                      ; preds = %preload734, %postload727
  br i1 %extract361, label %preload742, label %postload743

preload742:                                       ; preds = %postload735
  store <4 x i8> %827, <4 x i8> addrspace(1)* %565, align 4
  br label %postload743

postload743:                                      ; preds = %preload742, %postload735
  br i1 %extract362, label %preload750, label %postload751

preload750:                                       ; preds = %postload743
  store <4 x i8> %828, <4 x i8> addrspace(1)* %568, align 4
  br label %postload751

postload751:                                      ; preds = %preload750, %postload743
  br i1 %extract363, label %preload758, label %postload759

preload758:                                       ; preds = %postload751
  store <4 x i8> %829, <4 x i8> addrspace(1)* %571, align 4
  br label %postload759

postload759:                                      ; preds = %preload758, %postload751
  br i1 %extract364, label %preload766, label %postload767

preload766:                                       ; preds = %postload759
  store <4 x i8> %830, <4 x i8> addrspace(1)* %574, align 4
  br label %postload767

postload767:                                      ; preds = %preload766, %postload759
  br i1 %extract365, label %preload774, label %postload775

preload774:                                       ; preds = %postload767
  store <4 x i8> %831, <4 x i8> addrspace(1)* %577, align 4
  br label %postload775

postload775:                                      ; preds = %preload774, %postload767
  br i1 %extract366, label %preload782, label %postload783

preload782:                                       ; preds = %postload775
  store <4 x i8> %832, <4 x i8> addrspace(1)* %580, align 4
  br label %postload783

postload783:                                      ; preds = %preload782, %postload775
  %841 = uitofp i8 %441 to float
  %842 = uitofp i8 %442 to float
  %843 = uitofp i8 %443 to float
  %844 = uitofp i8 %444 to float
  %845 = uitofp i8 %445 to float
  %846 = uitofp i8 %446 to float
  %847 = uitofp i8 %447 to float
  %848 = uitofp i8 %448 to float
  %849 = uitofp i8 %449 to float
  %850 = uitofp i8 %450 to float
  %851 = uitofp i8 %451 to float
  %852 = uitofp i8 %452 to float
  %853 = uitofp i8 %453 to float
  %854 = uitofp i8 %454 to float
  %855 = uitofp i8 %455 to float
  %856 = uitofp i8 %456 to float
  %temp.vect508 = insertelement <16 x float> undef, float %841, i32 0
  %temp.vect509 = insertelement <16 x float> %temp.vect508, float %842, i32 1
  %temp.vect510 = insertelement <16 x float> %temp.vect509, float %843, i32 2
  %temp.vect511 = insertelement <16 x float> %temp.vect510, float %844, i32 3
  %temp.vect512 = insertelement <16 x float> %temp.vect511, float %845, i32 4
  %temp.vect513 = insertelement <16 x float> %temp.vect512, float %846, i32 5
  %temp.vect514 = insertelement <16 x float> %temp.vect513, float %847, i32 6
  %temp.vect515 = insertelement <16 x float> %temp.vect514, float %848, i32 7
  %temp.vect516 = insertelement <16 x float> %temp.vect515, float %849, i32 8
  %temp.vect517 = insertelement <16 x float> %temp.vect516, float %850, i32 9
  %temp.vect518 = insertelement <16 x float> %temp.vect517, float %851, i32 10
  %temp.vect519 = insertelement <16 x float> %temp.vect518, float %852, i32 11
  %temp.vect520 = insertelement <16 x float> %temp.vect519, float %853, i32 12
  %temp.vect521 = insertelement <16 x float> %temp.vect520, float %854, i32 13
  %temp.vect522 = insertelement <16 x float> %temp.vect521, float %855, i32 14
  %temp.vect523 = insertelement <16 x float> %temp.vect522, float %856, i32 15
  %857 = uitofp i8 %457 to float
  %858 = uitofp i8 %458 to float
  %859 = uitofp i8 %459 to float
  %860 = uitofp i8 %460 to float
  %861 = uitofp i8 %461 to float
  %862 = uitofp i8 %462 to float
  %863 = uitofp i8 %463 to float
  %864 = uitofp i8 %464 to float
  %865 = uitofp i8 %465 to float
  %866 = uitofp i8 %466 to float
  %867 = uitofp i8 %467 to float
  %868 = uitofp i8 %468 to float
  %869 = uitofp i8 %469 to float
  %870 = uitofp i8 %470 to float
  %871 = uitofp i8 %471 to float
  %872 = uitofp i8 %472 to float
  %temp.vect524 = insertelement <16 x float> undef, float %857, i32 0
  %temp.vect525 = insertelement <16 x float> %temp.vect524, float %858, i32 1
  %temp.vect526 = insertelement <16 x float> %temp.vect525, float %859, i32 2
  %temp.vect527 = insertelement <16 x float> %temp.vect526, float %860, i32 3
  %temp.vect528 = insertelement <16 x float> %temp.vect527, float %861, i32 4
  %temp.vect529 = insertelement <16 x float> %temp.vect528, float %862, i32 5
  %temp.vect530 = insertelement <16 x float> %temp.vect529, float %863, i32 6
  %temp.vect531 = insertelement <16 x float> %temp.vect530, float %864, i32 7
  %temp.vect532 = insertelement <16 x float> %temp.vect531, float %865, i32 8
  %temp.vect533 = insertelement <16 x float> %temp.vect532, float %866, i32 9
  %temp.vect534 = insertelement <16 x float> %temp.vect533, float %867, i32 10
  %temp.vect535 = insertelement <16 x float> %temp.vect534, float %868, i32 11
  %temp.vect536 = insertelement <16 x float> %temp.vect535, float %869, i32 12
  %temp.vect537 = insertelement <16 x float> %temp.vect536, float %870, i32 13
  %temp.vect538 = insertelement <16 x float> %temp.vect537, float %871, i32 14
  %temp.vect539 = insertelement <16 x float> %temp.vect538, float %872, i32 15
  %873 = uitofp i8 %473 to float
  %874 = uitofp i8 %474 to float
  %875 = uitofp i8 %475 to float
  %876 = uitofp i8 %476 to float
  %877 = uitofp i8 %477 to float
  %878 = uitofp i8 %478 to float
  %879 = uitofp i8 %479 to float
  %880 = uitofp i8 %480 to float
  %881 = uitofp i8 %481 to float
  %882 = uitofp i8 %482 to float
  %883 = uitofp i8 %483 to float
  %884 = uitofp i8 %484 to float
  %885 = uitofp i8 %485 to float
  %886 = uitofp i8 %486 to float
  %887 = uitofp i8 %487 to float
  %888 = uitofp i8 %488 to float
  %temp.vect540 = insertelement <16 x float> undef, float %873, i32 0
  %temp.vect541 = insertelement <16 x float> %temp.vect540, float %874, i32 1
  %temp.vect542 = insertelement <16 x float> %temp.vect541, float %875, i32 2
  %temp.vect543 = insertelement <16 x float> %temp.vect542, float %876, i32 3
  %temp.vect544 = insertelement <16 x float> %temp.vect543, float %877, i32 4
  %temp.vect545 = insertelement <16 x float> %temp.vect544, float %878, i32 5
  %temp.vect546 = insertelement <16 x float> %temp.vect545, float %879, i32 6
  %temp.vect547 = insertelement <16 x float> %temp.vect546, float %880, i32 7
  %temp.vect548 = insertelement <16 x float> %temp.vect547, float %881, i32 8
  %temp.vect549 = insertelement <16 x float> %temp.vect548, float %882, i32 9
  %temp.vect550 = insertelement <16 x float> %temp.vect549, float %883, i32 10
  %temp.vect551 = insertelement <16 x float> %temp.vect550, float %884, i32 11
  %temp.vect552 = insertelement <16 x float> %temp.vect551, float %885, i32 12
  %temp.vect553 = insertelement <16 x float> %temp.vect552, float %886, i32 13
  %temp.vect554 = insertelement <16 x float> %temp.vect553, float %887, i32 14
  %temp.vect555 = insertelement <16 x float> %temp.vect554, float %888, i32 15
  %889 = uitofp i8 %489 to float
  %890 = uitofp i8 %490 to float
  %891 = uitofp i8 %491 to float
  %892 = uitofp i8 %492 to float
  %893 = uitofp i8 %493 to float
  %894 = uitofp i8 %494 to float
  %895 = uitofp i8 %495 to float
  %896 = uitofp i8 %496 to float
  %897 = uitofp i8 %497 to float
  %898 = uitofp i8 %498 to float
  %899 = uitofp i8 %499 to float
  %900 = uitofp i8 %500 to float
  %901 = uitofp i8 %501 to float
  %902 = uitofp i8 %502 to float
  %903 = uitofp i8 %503 to float
  %904 = uitofp i8 %504 to float
  %temp.vect556 = insertelement <16 x float> undef, float %889, i32 0
  %temp.vect557 = insertelement <16 x float> %temp.vect556, float %890, i32 1
  %temp.vect558 = insertelement <16 x float> %temp.vect557, float %891, i32 2
  %temp.vect559 = insertelement <16 x float> %temp.vect558, float %892, i32 3
  %temp.vect560 = insertelement <16 x float> %temp.vect559, float %893, i32 4
  %temp.vect561 = insertelement <16 x float> %temp.vect560, float %894, i32 5
  %temp.vect562 = insertelement <16 x float> %temp.vect561, float %895, i32 6
  %temp.vect563 = insertelement <16 x float> %temp.vect562, float %896, i32 7
  %temp.vect564 = insertelement <16 x float> %temp.vect563, float %897, i32 8
  %temp.vect565 = insertelement <16 x float> %temp.vect564, float %898, i32 9
  %temp.vect566 = insertelement <16 x float> %temp.vect565, float %899, i32 10
  %temp.vect567 = insertelement <16 x float> %temp.vect566, float %900, i32 11
  %temp.vect568 = insertelement <16 x float> %temp.vect567, float %901, i32 12
  %temp.vect569 = insertelement <16 x float> %temp.vect568, float %902, i32 13
  %temp.vect570 = insertelement <16 x float> %temp.vect569, float %903, i32 14
  %temp.vect571 = insertelement <16 x float> %temp.vect570, float %904, i32 15
  %indvar.next = add i32 %indvar, 1
  %exitcond = icmp eq i32 %indvar.next, %height
  %temp499 = insertelement <16 x i1> undef, i1 %exitcond, i32 0
  %vector500 = shufflevector <16 x i1> %temp499, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond59 = xor i1 %exitcond, true
  %temp505 = insertelement <16 x i1> undef, i1 %notCond59, i32 0
  %vector506 = shufflevector <16 x i1> %temp505, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr60501 = and <16 x i1> %vectorPHI315, %vector500
  %loop_mask63503 = or <16 x i1> %vectorPHI313, %who_left_tr60501
  %curr_loop_mask65504 = or <16 x i1> %loop_mask63503, %who_left_tr60501
  %ipred.i9 = bitcast <16 x i1> %curr_loop_mask65504 to i16
  %val.i10 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i9, i16 %ipred.i9) nounwind
  %tmp.i11 = and i32 %val.i10, 1
  %res.i12 = icmp eq i32 %tmp.i11, 0
  %local_edge69507 = and <16 x i1> %vectorPHI315, %vector506
  br i1 %res.i12, label %392, label %.loopexit

.loopexit:                                        ; preds = %postload783, %postload785
  %check.WI.iter = icmp ult i64 %CurrWI..2, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB872

thenBB:                                           ; preds = %.loopexit
  %"CurrWI++" = add nuw i64 %CurrWI..2, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..2, 448
  %cond = icmp eq i32 %currBarrier.1, 11
  br i1 %cond, label %SyncBB.outer, label %postload785

SyncBB872:                                        ; preds = %.loopexit
  ret void
}

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @transpose_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x i8> addrspace(1)**
  %1 = load <4 x i8> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x i8> addrspace(1)**
  %4 = load <4 x i8> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to <4 x i8> addrspace(3)**
  %7 = load <4 x i8> addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 96
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  br label %SyncBB24.i

SyncBB24.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %29 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = add i64 %30, %32
  %34 = trunc i64 %33 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %34, i32* %CastToValueType.i, align 4
  %35 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 1
  %36 = load i64* %35, align 8
  %37 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 1
  %38 = load i64* %37, align 8
  %39 = add i64 %36, %38
  %40 = trunc i64 %39 to i32
  %"&(pSB[currWI].offset)71.i" = or i64 %CurrSBIndex..0.i, 4
  %"&pSB[currWI].offset8.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)71.i"
  %CastToValueType9.i = bitcast i8* %"&pSB[currWI].offset8.i" to i32*
  store i32 %40, i32* %CastToValueType9.i, align 4
  %41 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %42 = load i64* %41, align 8
  %43 = trunc i64 %42 to i32
  %44 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 1
  %45 = load i64* %44, align 8
  %46 = trunc i64 %45 to i32
  %47 = mul i32 %40, %10
  %48 = add i32 %47, %34
  %49 = zext i32 %48 to i64
  %50 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %49
  %51 = load <4 x i8> addrspace(1)* %50, align 4
  %52 = mul i32 %46, %16
  %53 = add i32 %52, %43
  %54 = zext i32 %53 to i64
  %55 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %54
  %"&(pSB[currWI].offset)162.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset17.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)162.i"
  %CastToValueType18.i = bitcast i8* %"&pSB[currWI].offset17.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %55, <4 x i8> addrspace(3)** %CastToValueType18.i, align 8
  store <4 x i8> %51, <4 x i8> addrspace(3)* %55, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %SyncBB24.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 448
  br label %SyncBB24.i

SyncBB.i:                                         ; preds = %thenBB27.i, %SyncBB24.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++31.i", %thenBB27.i ], [ 0, %SyncBB24.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride33.i", %thenBB27.i ], [ 0, %SyncBB24.i ]
  %"&pSB[currWI].offset4.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..1.i
  %CastToValueType5.i = bitcast i8* %"&pSB[currWI].offset4.i" to i32*
  %loadedValue.i = load i32* %CastToValueType5.i, align 4
  %56 = mul i32 %loadedValue.i, %13
  %"&(pSB[currWI].offset)113.i" = or i64 %CurrSBIndex..1.i, 4
  %"&pSB[currWI].offset12.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)113.i"
  %CastToValueType13.i = bitcast i8* %"&pSB[currWI].offset12.i" to i32*
  %loadedValue14.i = load i32* %CastToValueType13.i, align 4
  %57 = add i32 %loadedValue14.i, %56
  %"&(pSB[currWI].offset)204.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset21.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)204.i"
  %CastToValueType22.i = bitcast i8* %"&pSB[currWI].offset21.i" to <4 x i8> addrspace(3)**
  %loadedValue23.i = load <4 x i8> addrspace(3)** %CastToValueType22.i, align 8
  %58 = load <4 x i8> addrspace(3)* %loadedValue23.i, align 4
  %59 = zext i32 %57 to i64
  %60 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %59
  store <4 x i8> %58, <4 x i8> addrspace(1)* %60, align 4
  %check.WI.iter30.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter30.i, label %thenBB27.i, label %__transpose_kernel_separated_args.exit

thenBB27.i:                                       ; preds = %SyncBB.i
  %"CurrWI++31.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride33.i" = add nuw i64 %CurrSBIndex..1.i, 448
  br label %SyncBB.i

__transpose_kernel_separated_args.exit:           ; preds = %SyncBB.i
  ret void
}

define void @RecursiveGaussian_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x i8> addrspace(1)**
  %1 = load <4 x i8> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x i8> addrspace(1)**
  %4 = load <4 x i8> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to float*
  %13 = load float* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 28
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 32
  %18 = bitcast i8* %17 to float*
  %19 = load float* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 36
  %21 = bitcast i8* %20 to float*
  %22 = load float* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 40
  %24 = bitcast i8* %23 to float*
  %25 = load float* %24, align 4
  %26 = getelementptr i8* %pBuffer, i64 44
  %27 = bitcast i8* %26 to float*
  %28 = load float* %27, align 4
  %29 = getelementptr i8* %pBuffer, i64 80
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 88
  %33 = bitcast i8* %32 to %struct.PaddedDimId**
  %34 = load %struct.PaddedDimId** %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i64*
  %37 = load i64* %36, align 8
  %38 = getelementptr i8* %pBuffer, i64 112
  %39 = bitcast i8* %38 to i8**
  %40 = load i8** %39, align 8
  %41 = icmp sgt i32 %10, 0
  %42 = insertelement <4 x float> undef, float %13, i32 0
  %43 = shufflevector <4 x float> %42, <4 x float> undef, <4 x i32> zeroinitializer
  %44 = insertelement <4 x float> undef, float %16, i32 0
  %45 = shufflevector <4 x float> %44, <4 x float> undef, <4 x i32> zeroinitializer
  %46 = insertelement <4 x float> undef, float %25, i32 0
  %47 = shufflevector <4 x float> %46, <4 x float> undef, <4 x i32> zeroinitializer
  %48 = insertelement <4 x float> undef, float %28, i32 0
  %49 = shufflevector <4 x float> %48, <4 x float> undef, <4 x i32> zeroinitializer
  %y1.01.i = add i32 %10, -1
  %50 = icmp sgt i32 %y1.01.i, -1
  %51 = insertelement <4 x float> undef, float %19, i32 0
  %52 = shufflevector <4 x float> %51, <4 x float> undef, <4 x i32> zeroinitializer
  %53 = insertelement <4 x float> undef, float %22, i32 0
  %54 = shufflevector <4 x float> %53, <4 x float> undef, <4 x i32> zeroinitializer
  %55 = insertelement <4 x float> undef, float %25, i32 0
  %56 = shufflevector <4 x float> %55, <4 x float> undef, <4 x i32> zeroinitializer
  %57 = insertelement <4 x float> undef, float %28, i32 0
  %58 = shufflevector <4 x float> %57, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp.i = sub i32 0, %7
  %tmp14.i = mul i32 %y1.01.i, %7
  br label %SyncBB31.outer.i

SyncBB31.outer.i:                                 ; preds = %thenBB34.i, %entry
  %CurrWI..0.ph.i = phi i64 [ 0, %entry ], [ %"CurrWI++38.i", %thenBB34.i ]
  %CurrSBIndex..0.ph.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride40.i", %thenBB34.i ]
  %currBarrier.2.ph.i = phi i32 [ 9, %entry ], [ %currBarrier.1.i, %thenBB34.i ]
  br label %SyncBB31.i

SyncBB31.i:                                       ; preds = %thenBB.i, %SyncBB31.outer.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ %CurrWI..0.ph.i, %SyncBB31.outer.i ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %CurrSBIndex..0.ph.i, %SyncBB31.outer.i ]
  %59 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %60 = load i64* %59, align 8
  %61 = getelementptr %struct.PaddedDimId* %31, i64 0, i32 0, i64 0
  %62 = load i64* %61, align 8
  %63 = add i64 %60, %62
  %64 = trunc i64 %63 to i32
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %64, i32* %CastToValueType.i, align 4
  %65 = icmp ult i32 %64, %7
  br i1 %65, label %.preheader.i, label %.loopexit.i

.preheader.i:                                     ; preds = %SyncBB31.i
  br i1 %41, label %bb.nph11.i, label %._crit_edge.i

bb.nph11.i:                                       ; preds = %.preheader.i
  %"&(pSB[currWI].offset)27.i" = add nuw i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset28.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset)27.i"
  %CastToValueType29.i = bitcast i8* %"&pSB[currWI].offset28.i" to i32*
  br label %66

; <label>:66                                      ; preds = %66, %bb.nph11.i
  %y.010.i = phi i32 [ 0, %bb.nph11.i ], [ %102, %66 ]
  %yb.09.i = phi <4 x float> [ zeroinitializer, %bb.nph11.i ], [ %yp.08.i, %66 ]
  %yp.08.i = phi <4 x float> [ zeroinitializer, %bb.nph11.i ], [ %88, %66 ]
  %xp.07.i = phi <4 x float> [ zeroinitializer, %bb.nph11.i ], [ %81, %66 ]
  %tmp19.i = mul i32 %y.010.i, %7
  %loadedValue30.i = load i32* %CastToValueType29.i, align 4
  %tmp21.i = add i32 %loadedValue30.i, %tmp19.i
  %67 = sext i32 %tmp21.i to i64
  %68 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %67
  %69 = load <4 x i8> addrspace(1)* %68, align 4
  %70 = extractelement <4 x i8> %69, i32 0
  %71 = uitofp i8 %70 to float
  %72 = insertelement <4 x float> undef, float %71, i32 0
  %73 = extractelement <4 x i8> %69, i32 1
  %74 = uitofp i8 %73 to float
  %75 = insertelement <4 x float> %72, float %74, i32 1
  %76 = extractelement <4 x i8> %69, i32 2
  %77 = uitofp i8 %76 to float
  %78 = insertelement <4 x float> %75, float %77, i32 2
  %79 = extractelement <4 x i8> %69, i32 3
  %80 = uitofp i8 %79 to float
  %81 = insertelement <4 x float> %78, float %80, i32 3
  %82 = fmul <4 x float> %43, %81
  %83 = fmul <4 x float> %45, %xp.07.i
  %84 = fadd <4 x float> %82, %83
  %85 = fmul <4 x float> %47, %yp.08.i
  %86 = fsub <4 x float> %84, %85
  %87 = fmul <4 x float> %49, %yb.09.i
  %88 = fsub <4 x float> %86, %87
  %89 = extractelement <4 x float> %88, i32 0
  %90 = fptoui float %89 to i8
  %91 = insertelement <4 x i8> undef, i8 %90, i32 0
  %92 = extractelement <4 x float> %88, i32 1
  %93 = fptoui float %92 to i8
  %94 = insertelement <4 x i8> %91, i8 %93, i32 1
  %95 = extractelement <4 x float> %88, i32 2
  %96 = fptoui float %95 to i8
  %97 = insertelement <4 x i8> %94, i8 %96, i32 2
  %98 = extractelement <4 x float> %88, i32 3
  %99 = fptoui float %98 to i8
  %100 = insertelement <4 x i8> %97, i8 %99, i32 3
  %101 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %67
  store <4 x i8> %100, <4 x i8> addrspace(1)* %101, align 4
  %102 = add nsw i32 %y.010.i, 1
  %exitcond18.i = icmp eq i32 %102, %10
  br i1 %exitcond18.i, label %._crit_edge.i, label %66

._crit_edge.i:                                    ; preds = %66, %.preheader.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %37
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 448
  br label %SyncBB31.i

elseBB.i:                                         ; preds = %._crit_edge.i
  call void @llvm.x86.sse2.mfence() nounwind
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB34.i, %elseBB.i
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++38.i", %thenBB34.i ]
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride40.i", %thenBB34.i ]
  %currBarrier.0.i = phi i32 [ 2, %elseBB.i ], [ %currBarrier.1.i, %thenBB34.i ]
  br i1 %50, label %bb.nph.i, label %.loopexit.i

bb.nph.i:                                         ; preds = %SyncBB.i
  %"&(pSB[currWI].offset)23.i" = add nuw i64 %CurrSBIndex..1.i, 16
  %"&pSB[currWI].offset24.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset)23.i"
  %CastToValueType25.i = bitcast i8* %"&pSB[currWI].offset24.i" to i32*
  %loadedValue.i = load i32* %CastToValueType25.i, align 4
  %tmp16.i = add i32 %tmp14.i, %loadedValue.i
  br label %103

; <label>:103                                     ; preds = %103, %bb.nph.i
  %indvar.i = phi i32 [ 0, %bb.nph.i ], [ %indvar.next.i, %103 ]
  %ya.05.i = phi <4 x float> [ zeroinitializer, %bb.nph.i ], [ %yn.04.i, %103 ]
  %yn.04.i = phi <4 x float> [ zeroinitializer, %bb.nph.i ], [ %113, %103 ]
  %xa.03.i = phi <4 x float> [ zeroinitializer, %bb.nph.i ], [ %xn.02.i, %103 ]
  %xn.02.i = phi <4 x float> [ zeroinitializer, %bb.nph.i ], [ %152, %103 ]
  %tmp12.i = mul i32 %indvar.i, %tmp.i
  %tmp17.i = add i32 %tmp16.i, %tmp12.i
  %104 = sext i32 %tmp17.i to i64
  %105 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %104
  %106 = load <4 x i8> addrspace(1)* %105, align 4
  %107 = fmul <4 x float> %52, %xn.02.i
  %108 = fmul <4 x float> %54, %xa.03.i
  %109 = fadd <4 x float> %107, %108
  %110 = fmul <4 x float> %56, %yn.04.i
  %111 = fsub <4 x float> %109, %110
  %112 = fmul <4 x float> %58, %ya.05.i
  %113 = fsub <4 x float> %111, %112
  %114 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %104
  %115 = load <4 x i8> addrspace(1)* %114, align 4
  %116 = extractelement <4 x i8> %115, i32 0
  %117 = uitofp i8 %116 to float
  %118 = insertelement <4 x float> undef, float %117, i32 0
  %119 = extractelement <4 x i8> %115, i32 1
  %120 = uitofp i8 %119 to float
  %121 = insertelement <4 x float> %118, float %120, i32 1
  %122 = extractelement <4 x i8> %115, i32 2
  %123 = uitofp i8 %122 to float
  %124 = insertelement <4 x float> %121, float %123, i32 2
  %125 = extractelement <4 x i8> %115, i32 3
  %126 = uitofp i8 %125 to float
  %127 = insertelement <4 x float> %124, float %126, i32 3
  %128 = fadd <4 x float> %127, %113
  %129 = extractelement <4 x float> %128, i32 0
  %130 = fptoui float %129 to i8
  %131 = insertelement <4 x i8> undef, i8 %130, i32 0
  %132 = extractelement <4 x float> %128, i32 1
  %133 = fptoui float %132 to i8
  %134 = insertelement <4 x i8> %131, i8 %133, i32 1
  %135 = extractelement <4 x float> %128, i32 2
  %136 = fptoui float %135 to i8
  %137 = insertelement <4 x i8> %134, i8 %136, i32 2
  %138 = extractelement <4 x float> %128, i32 3
  %139 = fptoui float %138 to i8
  %140 = insertelement <4 x i8> %137, i8 %139, i32 3
  store <4 x i8> %140, <4 x i8> addrspace(1)* %114, align 4
  %141 = extractelement <4 x i8> %106, i32 0
  %142 = extractelement <4 x i8> %106, i32 1
  %143 = uitofp i8 %141 to float
  %144 = extractelement <4 x i8> %106, i32 2
  %145 = uitofp i8 %142 to float
  %146 = insertelement <4 x float> undef, float %143, i32 0
  %147 = extractelement <4 x i8> %106, i32 3
  %148 = uitofp i8 %144 to float
  %149 = insertelement <4 x float> %146, float %145, i32 1
  %150 = uitofp i8 %147 to float
  %151 = insertelement <4 x float> %149, float %148, i32 2
  %152 = insertelement <4 x float> %151, float %150, i32 3
  %indvar.next.i = add i32 %indvar.i, 1
  %exitcond.i = icmp eq i32 %indvar.next.i, %10
  br i1 %exitcond.i, label %.loopexit.i, label %103

.loopexit.i:                                      ; preds = %103, %SyncBB.i, %SyncBB31.i
  %CurrWI..2.i = phi i64 [ %CurrWI..1.i, %103 ], [ %CurrWI..1.i, %SyncBB.i ], [ %CurrWI..0.i, %SyncBB31.i ]
  %CurrSBIndex..2.i = phi i64 [ %CurrSBIndex..1.i, %103 ], [ %CurrSBIndex..1.i, %SyncBB.i ], [ %CurrSBIndex..0.i, %SyncBB31.i ]
  %currBarrier.1.i = phi i32 [ %currBarrier.0.i, %103 ], [ %currBarrier.0.i, %SyncBB.i ], [ %currBarrier.2.ph.i, %SyncBB31.i ]
  %check.WI.iter37.i = icmp ult i64 %CurrWI..2.i, %37
  br i1 %check.WI.iter37.i, label %thenBB34.i, label %__RecursiveGaussian_kernel_separated_args.exit

thenBB34.i:                                       ; preds = %.loopexit.i
  %"CurrWI++38.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride40.i" = add nuw i64 %CurrSBIndex..2.i, 448
  %cond.i = icmp eq i32 %currBarrier.1.i, 2
  br i1 %cond.i, label %SyncBB.i, label %SyncBB31.outer.i

__RecursiveGaussian_kernel_separated_args.exit:   ; preds = %.loopexit.i
  ret void
}

define void @__Vectorized_.RecursiveGaussian_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x i8> addrspace(1)**
  %1 = load <4 x i8> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x i8> addrspace(1)**
  %4 = load <4 x i8> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 20
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 24
  %12 = bitcast i8* %11 to float*
  %13 = load float* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 28
  %15 = bitcast i8* %14 to float*
  %16 = load float* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 32
  %18 = bitcast i8* %17 to float*
  %19 = load float* %18, align 4
  %20 = getelementptr i8* %pBuffer, i64 36
  %21 = bitcast i8* %20 to float*
  %22 = load float* %21, align 4
  %23 = getelementptr i8* %pBuffer, i64 40
  %24 = bitcast i8* %23 to float*
  %25 = load float* %24, align 4
  %26 = getelementptr i8* %pBuffer, i64 44
  %27 = bitcast i8* %26 to float*
  %28 = load float* %27, align 4
  %29 = getelementptr i8* %pBuffer, i64 80
  %30 = bitcast i8* %29 to %struct.PaddedDimId**
  %31 = load %struct.PaddedDimId** %30, align 8
  %32 = getelementptr i8* %pBuffer, i64 88
  %33 = bitcast i8* %32 to %struct.PaddedDimId**
  %34 = load %struct.PaddedDimId** %33, align 8
  %35 = getelementptr i8* %pBuffer, i64 104
  %36 = bitcast i8* %35 to i64*
  %37 = load i64* %36, align 8
  %38 = getelementptr i8* %pBuffer, i64 112
  %39 = bitcast i8* %38 to i8**
  %40 = load i8** %39, align 8
  %temp215.i = insertelement <16 x float> undef, float %28, i32 0
  %vector216.i = shufflevector <16 x float> %temp215.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp213.i = insertelement <16 x float> undef, float %25, i32 0
  %vector214.i = shufflevector <16 x float> %temp213.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp211.i = insertelement <16 x float> undef, float %16, i32 0
  %vector212.i = shufflevector <16 x float> %temp211.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp145.i = insertelement <16 x float> undef, float %13, i32 0
  %vector146.i = shufflevector <16 x float> %temp145.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp.i = insertelement <16 x i32> undef, i32 %7, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %41 = icmp sgt i32 %10, 0
  %temp93.i = insertelement <16 x i1> undef, i1 %41, i32 0
  %vector94.i = shufflevector <16 x i1> %temp93.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %temp369.i = insertelement <16 x float> undef, float %22, i32 0
  %vector370.i = shufflevector <16 x float> %temp369.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp367.i = insertelement <16 x float> undef, float %19, i32 0
  %vector368.i = shufflevector <16 x float> %temp367.i, <16 x float> undef, <16 x i32> zeroinitializer
  %y1.01.i = add i32 %10, -1
  %42 = icmp sgt i32 %y1.01.i, -1
  %temp306.i = insertelement <16 x i1> undef, i1 %42, i32 0
  %vector307.i = shufflevector <16 x i1> %temp306.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %tmp.i = sub i32 0, %7
  %tmp14.i = mul i32 %y1.01.i, %7
  %temp309.i = insertelement <16 x i32> undef, i32 %tmp14.i, i32 0
  %vector310.i = shufflevector <16 x i32> %temp309.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %temp369.le.i = insertelement <16 x float> undef, float %22, i32 0
  %vector370.le.i = shufflevector <16 x float> %temp369.le.i, <16 x float> undef, <16 x i32> zeroinitializer
  %temp367.le.i = insertelement <16 x float> undef, float %19, i32 0
  %vector368.le.i = shufflevector <16 x float> %temp367.le.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB.outer.i

SyncBB.outer.i:                                   ; preds = %thenBB.i, %entry
  %currBarrier.2.ph.i = phi i32 [ 11, %entry ], [ %currBarrier.1.i, %thenBB.i ]
  %CurrSBIndex..0.ph.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %CurrWI..0.ph.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB875.i, %SyncBB.outer.i
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride881.i", %thenBB875.i ], [ %CurrSBIndex..0.ph.i, %SyncBB.outer.i ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++879.i", %thenBB875.i ], [ %CurrWI..0.ph.i, %SyncBB.outer.i ]
  %43 = getelementptr %struct.PaddedDimId* %34, i64 %CurrWI..0.i, i32 0, i64 0
  %44 = load i64* %43, align 8
  %45 = getelementptr %struct.PaddedDimId* %31, i64 0, i32 0, i64 0
  %46 = load i64* %45, align 8
  %47 = add i64 %44, %46
  %broadcast1.i = insertelement <16 x i64> undef, i64 %47, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %48 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %49 = trunc <16 x i64> %48 to <16 x i32>
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 320
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %49, <16 x i32>* %CastToValueType.i, align 64
  %50 = icmp ult <16 x i32> %49, %vector.i
  %"&(pSB[currWI].offset)836.i" = add nuw i64 %CurrSBIndex..0.i, 384
  %"&pSB[currWI].offset837.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset)836.i"
  %CastToValueType838.i = bitcast i8* %"&pSB[currWI].offset837.i" to <16 x i1>*
  store <16 x i1> %50, <16 x i1>* %CastToValueType838.i, align 16
  %extract290.i = extractelement <16 x i1> %50, i32 0
  %.preheader_to_bb.nph1195.i = and <16 x i1> %50, %vector94.i
  %ipred.i.i = bitcast <16 x i1> %.preheader_to_bb.nph1195.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %tmp.i.i = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %tmp.i.i, 0
  br i1 %res.i.i, label %.preheader883.i, label %._crit_edge.i

.preheader883.i:                                  ; preds = %SyncBB.i
  %negIncomingLoopMask96.i = xor <16 x i1> %.preheader_to_bb.nph1195.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %"&(pSB[currWI].offset)831.i" = add nuw i64 %CurrSBIndex..0.i, 320
  %"&pSB[currWI].offset832.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset)831.i"
  %CastToValueType833.i = bitcast i8* %"&pSB[currWI].offset832.i" to <16 x i32>*
  br label %51

; <label>:51                                      ; preds = %postload695.i, %.preheader883.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask54285.i, %postload695.i ], [ %negIncomingLoopMask96.i, %.preheader883.i ]
  %vectorPHI98.i = phi <16 x i1> [ %local_edge289.i, %postload695.i ], [ %.preheader_to_bb.nph1195.i, %.preheader883.i ]
  %y.010.i = phi i32 [ %432, %postload695.i ], [ 0, %.preheader883.i ]
  %vectorPHI99.i = phi <16 x float> [ %vectorPHI103.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI100.i = phi <16 x float> [ %vectorPHI104.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI101.i = phi <16 x float> [ %vectorPHI105.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI102.i = phi <16 x float> [ %vectorPHI106.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI103.i = phi <16 x float> [ %252, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI104.i = phi <16 x float> [ %253, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI105.i = phi <16 x float> [ %254, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI106.i = phi <16 x float> [ %255, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI107.i = phi <16 x float> [ %temp.vect162.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI108.i = phi <16 x float> [ %temp.vect178.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI109.i = phi <16 x float> [ %temp.vect194.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %vectorPHI110.i = phi <16 x float> [ %temp.vect210.i, %postload695.i ], [ zeroinitializer, %.preheader883.i ]
  %extract129.i = extractelement <16 x i1> %vectorPHI98.i, i32 0
  %extract130.i = extractelement <16 x i1> %vectorPHI98.i, i32 1
  %extract131.i = extractelement <16 x i1> %vectorPHI98.i, i32 2
  %extract132.i = extractelement <16 x i1> %vectorPHI98.i, i32 3
  %extract133.i = extractelement <16 x i1> %vectorPHI98.i, i32 4
  %extract134.i = extractelement <16 x i1> %vectorPHI98.i, i32 5
  %extract135.i = extractelement <16 x i1> %vectorPHI98.i, i32 6
  %extract136.i = extractelement <16 x i1> %vectorPHI98.i, i32 7
  %extract137.i = extractelement <16 x i1> %vectorPHI98.i, i32 8
  %extract138.i = extractelement <16 x i1> %vectorPHI98.i, i32 9
  %extract139.i = extractelement <16 x i1> %vectorPHI98.i, i32 10
  %extract140.i = extractelement <16 x i1> %vectorPHI98.i, i32 11
  %extract141.i = extractelement <16 x i1> %vectorPHI98.i, i32 12
  %extract142.i = extractelement <16 x i1> %vectorPHI98.i, i32 13
  %extract143.i = extractelement <16 x i1> %vectorPHI98.i, i32 14
  %extract144.i = extractelement <16 x i1> %vectorPHI98.i, i32 15
  %tmp19.i = mul i32 %y.010.i, %7
  %temp111.i = insertelement <16 x i32> undef, i32 %tmp19.i, i32 0
  %vector112.i = shufflevector <16 x i32> %temp111.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %loadedValue834.i = load <16 x i32>* %CastToValueType833.i, align 64
  %tmp21113.i = add <16 x i32> %loadedValue834.i, %vector112.i
  %52 = extractelement <16 x i32> %tmp21113.i, i32 1
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %53
  %55 = extractelement <16 x i32> %tmp21113.i, i32 2
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %56
  %58 = extractelement <16 x i32> %tmp21113.i, i32 3
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %59
  %61 = extractelement <16 x i32> %tmp21113.i, i32 4
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %62
  %64 = extractelement <16 x i32> %tmp21113.i, i32 5
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %65
  %67 = extractelement <16 x i32> %tmp21113.i, i32 6
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %68
  %70 = extractelement <16 x i32> %tmp21113.i, i32 7
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %71
  %73 = extractelement <16 x i32> %tmp21113.i, i32 8
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %74
  %76 = extractelement <16 x i32> %tmp21113.i, i32 9
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %77
  %79 = extractelement <16 x i32> %tmp21113.i, i32 10
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %80
  %82 = extractelement <16 x i32> %tmp21113.i, i32 11
  %83 = sext i32 %82 to i64
  %84 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %83
  %85 = extractelement <16 x i32> %tmp21113.i, i32 12
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %86
  %88 = extractelement <16 x i32> %tmp21113.i, i32 13
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %89
  %91 = extractelement <16 x i32> %tmp21113.i, i32 14
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %92
  %94 = extractelement <16 x i32> %tmp21113.i, i32 15
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %95
  br i1 %extract129.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %51
  %97 = extractelement <16 x i32> %tmp21113.i, i32 0
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %98
  %masked_load.i = load <4 x i8> addrspace(1)* %99, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %51
  %phi.i = phi <4 x i8> [ undef, %51 ], [ %masked_load.i, %preload.i ]
  br i1 %extract130.i, label %preload621.i, label %postload622.i

preload621.i:                                     ; preds = %postload.i
  %masked_load572.i = load <4 x i8> addrspace(1)* %54, align 4
  br label %postload622.i

postload622.i:                                    ; preds = %preload621.i, %postload.i
  %phi623.i = phi <4 x i8> [ undef, %postload.i ], [ %masked_load572.i, %preload621.i ]
  br i1 %extract131.i, label %preload626.i, label %postload627.i

preload626.i:                                     ; preds = %postload622.i
  %masked_load573.i = load <4 x i8> addrspace(1)* %57, align 4
  br label %postload627.i

postload627.i:                                    ; preds = %preload626.i, %postload622.i
  %phi628.i = phi <4 x i8> [ undef, %postload622.i ], [ %masked_load573.i, %preload626.i ]
  br i1 %extract132.i, label %preload631.i, label %postload632.i

preload631.i:                                     ; preds = %postload627.i
  %masked_load574.i = load <4 x i8> addrspace(1)* %60, align 4
  br label %postload632.i

postload632.i:                                    ; preds = %preload631.i, %postload627.i
  %phi633.i = phi <4 x i8> [ undef, %postload627.i ], [ %masked_load574.i, %preload631.i ]
  br i1 %extract133.i, label %preload636.i, label %postload637.i

preload636.i:                                     ; preds = %postload632.i
  %masked_load575.i = load <4 x i8> addrspace(1)* %63, align 4
  br label %postload637.i

postload637.i:                                    ; preds = %preload636.i, %postload632.i
  %phi638.i = phi <4 x i8> [ undef, %postload632.i ], [ %masked_load575.i, %preload636.i ]
  br i1 %extract134.i, label %preload641.i, label %postload642.i

preload641.i:                                     ; preds = %postload637.i
  %masked_load576.i = load <4 x i8> addrspace(1)* %66, align 4
  br label %postload642.i

postload642.i:                                    ; preds = %preload641.i, %postload637.i
  %phi643.i = phi <4 x i8> [ undef, %postload637.i ], [ %masked_load576.i, %preload641.i ]
  br i1 %extract135.i, label %preload646.i, label %postload647.i

preload646.i:                                     ; preds = %postload642.i
  %masked_load577.i = load <4 x i8> addrspace(1)* %69, align 4
  br label %postload647.i

postload647.i:                                    ; preds = %preload646.i, %postload642.i
  %phi648.i = phi <4 x i8> [ undef, %postload642.i ], [ %masked_load577.i, %preload646.i ]
  br i1 %extract136.i, label %preload651.i, label %postload652.i

preload651.i:                                     ; preds = %postload647.i
  %masked_load578.i = load <4 x i8> addrspace(1)* %72, align 4
  br label %postload652.i

postload652.i:                                    ; preds = %preload651.i, %postload647.i
  %phi653.i = phi <4 x i8> [ undef, %postload647.i ], [ %masked_load578.i, %preload651.i ]
  br i1 %extract137.i, label %preload656.i, label %postload657.i

preload656.i:                                     ; preds = %postload652.i
  %masked_load579.i = load <4 x i8> addrspace(1)* %75, align 4
  br label %postload657.i

postload657.i:                                    ; preds = %preload656.i, %postload652.i
  %phi658.i = phi <4 x i8> [ undef, %postload652.i ], [ %masked_load579.i, %preload656.i ]
  br i1 %extract138.i, label %preload661.i, label %postload662.i

preload661.i:                                     ; preds = %postload657.i
  %masked_load580.i = load <4 x i8> addrspace(1)* %78, align 4
  br label %postload662.i

postload662.i:                                    ; preds = %preload661.i, %postload657.i
  %phi663.i = phi <4 x i8> [ undef, %postload657.i ], [ %masked_load580.i, %preload661.i ]
  br i1 %extract139.i, label %preload666.i, label %postload667.i

preload666.i:                                     ; preds = %postload662.i
  %masked_load581.i = load <4 x i8> addrspace(1)* %81, align 4
  br label %postload667.i

postload667.i:                                    ; preds = %preload666.i, %postload662.i
  %phi668.i = phi <4 x i8> [ undef, %postload662.i ], [ %masked_load581.i, %preload666.i ]
  br i1 %extract140.i, label %preload671.i, label %postload672.i

preload671.i:                                     ; preds = %postload667.i
  %masked_load582.i = load <4 x i8> addrspace(1)* %84, align 4
  br label %postload672.i

postload672.i:                                    ; preds = %preload671.i, %postload667.i
  %phi673.i = phi <4 x i8> [ undef, %postload667.i ], [ %masked_load582.i, %preload671.i ]
  br i1 %extract141.i, label %preload676.i, label %postload677.i

preload676.i:                                     ; preds = %postload672.i
  %masked_load583.i = load <4 x i8> addrspace(1)* %87, align 4
  br label %postload677.i

postload677.i:                                    ; preds = %preload676.i, %postload672.i
  %phi678.i = phi <4 x i8> [ undef, %postload672.i ], [ %masked_load583.i, %preload676.i ]
  br i1 %extract142.i, label %preload681.i, label %postload682.i

preload681.i:                                     ; preds = %postload677.i
  %masked_load584.i = load <4 x i8> addrspace(1)* %90, align 4
  br label %postload682.i

postload682.i:                                    ; preds = %preload681.i, %postload677.i
  %phi683.i = phi <4 x i8> [ undef, %postload677.i ], [ %masked_load584.i, %preload681.i ]
  br i1 %extract143.i, label %preload686.i, label %postload687.i

preload686.i:                                     ; preds = %postload682.i
  %masked_load585.i = load <4 x i8> addrspace(1)* %93, align 4
  br label %postload687.i

postload687.i:                                    ; preds = %preload686.i, %postload682.i
  %phi688.i = phi <4 x i8> [ undef, %postload682.i ], [ %masked_load585.i, %preload686.i ]
  br i1 %extract144.i, label %preload691.i, label %postload692.i

preload691.i:                                     ; preds = %postload687.i
  %masked_load586.i = load <4 x i8> addrspace(1)* %96, align 4
  br label %postload692.i

postload692.i:                                    ; preds = %preload691.i, %postload687.i
  %phi693.i = phi <4 x i8> [ undef, %postload687.i ], [ %masked_load586.i, %preload691.i ]
  %100 = extractelement <4 x i8> %phi.i, i32 0
  %101 = extractelement <4 x i8> %phi623.i, i32 0
  %102 = extractelement <4 x i8> %phi628.i, i32 0
  %103 = extractelement <4 x i8> %phi633.i, i32 0
  %104 = extractelement <4 x i8> %phi638.i, i32 0
  %105 = extractelement <4 x i8> %phi643.i, i32 0
  %106 = extractelement <4 x i8> %phi648.i, i32 0
  %107 = extractelement <4 x i8> %phi653.i, i32 0
  %108 = extractelement <4 x i8> %phi658.i, i32 0
  %109 = extractelement <4 x i8> %phi663.i, i32 0
  %110 = extractelement <4 x i8> %phi668.i, i32 0
  %111 = extractelement <4 x i8> %phi673.i, i32 0
  %112 = extractelement <4 x i8> %phi678.i, i32 0
  %113 = extractelement <4 x i8> %phi683.i, i32 0
  %114 = extractelement <4 x i8> %phi688.i, i32 0
  %115 = extractelement <4 x i8> %phi693.i, i32 0
  %116 = extractelement <4 x i8> %phi.i, i32 1
  %117 = extractelement <4 x i8> %phi623.i, i32 1
  %118 = extractelement <4 x i8> %phi628.i, i32 1
  %119 = extractelement <4 x i8> %phi633.i, i32 1
  %120 = extractelement <4 x i8> %phi638.i, i32 1
  %121 = extractelement <4 x i8> %phi643.i, i32 1
  %122 = extractelement <4 x i8> %phi648.i, i32 1
  %123 = extractelement <4 x i8> %phi653.i, i32 1
  %124 = extractelement <4 x i8> %phi658.i, i32 1
  %125 = extractelement <4 x i8> %phi663.i, i32 1
  %126 = extractelement <4 x i8> %phi668.i, i32 1
  %127 = extractelement <4 x i8> %phi673.i, i32 1
  %128 = extractelement <4 x i8> %phi678.i, i32 1
  %129 = extractelement <4 x i8> %phi683.i, i32 1
  %130 = extractelement <4 x i8> %phi688.i, i32 1
  %131 = extractelement <4 x i8> %phi693.i, i32 1
  %132 = extractelement <4 x i8> %phi.i, i32 2
  %133 = extractelement <4 x i8> %phi623.i, i32 2
  %134 = extractelement <4 x i8> %phi628.i, i32 2
  %135 = extractelement <4 x i8> %phi633.i, i32 2
  %136 = extractelement <4 x i8> %phi638.i, i32 2
  %137 = extractelement <4 x i8> %phi643.i, i32 2
  %138 = extractelement <4 x i8> %phi648.i, i32 2
  %139 = extractelement <4 x i8> %phi653.i, i32 2
  %140 = extractelement <4 x i8> %phi658.i, i32 2
  %141 = extractelement <4 x i8> %phi663.i, i32 2
  %142 = extractelement <4 x i8> %phi668.i, i32 2
  %143 = extractelement <4 x i8> %phi673.i, i32 2
  %144 = extractelement <4 x i8> %phi678.i, i32 2
  %145 = extractelement <4 x i8> %phi683.i, i32 2
  %146 = extractelement <4 x i8> %phi688.i, i32 2
  %147 = extractelement <4 x i8> %phi693.i, i32 2
  %148 = extractelement <4 x i8> %phi.i, i32 3
  %149 = extractelement <4 x i8> %phi623.i, i32 3
  %150 = extractelement <4 x i8> %phi628.i, i32 3
  %151 = extractelement <4 x i8> %phi633.i, i32 3
  %152 = extractelement <4 x i8> %phi638.i, i32 3
  %153 = extractelement <4 x i8> %phi643.i, i32 3
  %154 = extractelement <4 x i8> %phi648.i, i32 3
  %155 = extractelement <4 x i8> %phi653.i, i32 3
  %156 = extractelement <4 x i8> %phi658.i, i32 3
  %157 = extractelement <4 x i8> %phi663.i, i32 3
  %158 = extractelement <4 x i8> %phi668.i, i32 3
  %159 = extractelement <4 x i8> %phi673.i, i32 3
  %160 = extractelement <4 x i8> %phi678.i, i32 3
  %161 = extractelement <4 x i8> %phi683.i, i32 3
  %162 = extractelement <4 x i8> %phi688.i, i32 3
  %163 = extractelement <4 x i8> %phi693.i, i32 3
  %164 = uitofp i8 %100 to float
  %165 = uitofp i8 %101 to float
  %166 = uitofp i8 %102 to float
  %167 = uitofp i8 %103 to float
  %168 = uitofp i8 %104 to float
  %169 = uitofp i8 %105 to float
  %170 = uitofp i8 %106 to float
  %171 = uitofp i8 %107 to float
  %172 = uitofp i8 %108 to float
  %173 = uitofp i8 %109 to float
  %174 = uitofp i8 %110 to float
  %175 = uitofp i8 %111 to float
  %176 = uitofp i8 %112 to float
  %177 = uitofp i8 %113 to float
  %178 = uitofp i8 %114 to float
  %179 = uitofp i8 %115 to float
  %temp.vect147.i = insertelement <16 x float> undef, float %164, i32 0
  %temp.vect148.i = insertelement <16 x float> %temp.vect147.i, float %165, i32 1
  %temp.vect149.i = insertelement <16 x float> %temp.vect148.i, float %166, i32 2
  %temp.vect150.i = insertelement <16 x float> %temp.vect149.i, float %167, i32 3
  %temp.vect151.i = insertelement <16 x float> %temp.vect150.i, float %168, i32 4
  %temp.vect152.i = insertelement <16 x float> %temp.vect151.i, float %169, i32 5
  %temp.vect153.i = insertelement <16 x float> %temp.vect152.i, float %170, i32 6
  %temp.vect154.i = insertelement <16 x float> %temp.vect153.i, float %171, i32 7
  %temp.vect155.i = insertelement <16 x float> %temp.vect154.i, float %172, i32 8
  %temp.vect156.i = insertelement <16 x float> %temp.vect155.i, float %173, i32 9
  %temp.vect157.i = insertelement <16 x float> %temp.vect156.i, float %174, i32 10
  %temp.vect158.i = insertelement <16 x float> %temp.vect157.i, float %175, i32 11
  %temp.vect159.i = insertelement <16 x float> %temp.vect158.i, float %176, i32 12
  %temp.vect160.i = insertelement <16 x float> %temp.vect159.i, float %177, i32 13
  %temp.vect161.i = insertelement <16 x float> %temp.vect160.i, float %178, i32 14
  %temp.vect162.i = insertelement <16 x float> %temp.vect161.i, float %179, i32 15
  %180 = uitofp i8 %116 to float
  %181 = uitofp i8 %117 to float
  %182 = uitofp i8 %118 to float
  %183 = uitofp i8 %119 to float
  %184 = uitofp i8 %120 to float
  %185 = uitofp i8 %121 to float
  %186 = uitofp i8 %122 to float
  %187 = uitofp i8 %123 to float
  %188 = uitofp i8 %124 to float
  %189 = uitofp i8 %125 to float
  %190 = uitofp i8 %126 to float
  %191 = uitofp i8 %127 to float
  %192 = uitofp i8 %128 to float
  %193 = uitofp i8 %129 to float
  %194 = uitofp i8 %130 to float
  %195 = uitofp i8 %131 to float
  %temp.vect163.i = insertelement <16 x float> undef, float %180, i32 0
  %temp.vect164.i = insertelement <16 x float> %temp.vect163.i, float %181, i32 1
  %temp.vect165.i = insertelement <16 x float> %temp.vect164.i, float %182, i32 2
  %temp.vect166.i = insertelement <16 x float> %temp.vect165.i, float %183, i32 3
  %temp.vect167.i = insertelement <16 x float> %temp.vect166.i, float %184, i32 4
  %temp.vect168.i = insertelement <16 x float> %temp.vect167.i, float %185, i32 5
  %temp.vect169.i = insertelement <16 x float> %temp.vect168.i, float %186, i32 6
  %temp.vect170.i = insertelement <16 x float> %temp.vect169.i, float %187, i32 7
  %temp.vect171.i = insertelement <16 x float> %temp.vect170.i, float %188, i32 8
  %temp.vect172.i = insertelement <16 x float> %temp.vect171.i, float %189, i32 9
  %temp.vect173.i = insertelement <16 x float> %temp.vect172.i, float %190, i32 10
  %temp.vect174.i = insertelement <16 x float> %temp.vect173.i, float %191, i32 11
  %temp.vect175.i = insertelement <16 x float> %temp.vect174.i, float %192, i32 12
  %temp.vect176.i = insertelement <16 x float> %temp.vect175.i, float %193, i32 13
  %temp.vect177.i = insertelement <16 x float> %temp.vect176.i, float %194, i32 14
  %temp.vect178.i = insertelement <16 x float> %temp.vect177.i, float %195, i32 15
  %196 = uitofp i8 %132 to float
  %197 = uitofp i8 %133 to float
  %198 = uitofp i8 %134 to float
  %199 = uitofp i8 %135 to float
  %200 = uitofp i8 %136 to float
  %201 = uitofp i8 %137 to float
  %202 = uitofp i8 %138 to float
  %203 = uitofp i8 %139 to float
  %204 = uitofp i8 %140 to float
  %205 = uitofp i8 %141 to float
  %206 = uitofp i8 %142 to float
  %207 = uitofp i8 %143 to float
  %208 = uitofp i8 %144 to float
  %209 = uitofp i8 %145 to float
  %210 = uitofp i8 %146 to float
  %211 = uitofp i8 %147 to float
  %temp.vect179.i = insertelement <16 x float> undef, float %196, i32 0
  %temp.vect180.i = insertelement <16 x float> %temp.vect179.i, float %197, i32 1
  %temp.vect181.i = insertelement <16 x float> %temp.vect180.i, float %198, i32 2
  %temp.vect182.i = insertelement <16 x float> %temp.vect181.i, float %199, i32 3
  %temp.vect183.i = insertelement <16 x float> %temp.vect182.i, float %200, i32 4
  %temp.vect184.i = insertelement <16 x float> %temp.vect183.i, float %201, i32 5
  %temp.vect185.i = insertelement <16 x float> %temp.vect184.i, float %202, i32 6
  %temp.vect186.i = insertelement <16 x float> %temp.vect185.i, float %203, i32 7
  %temp.vect187.i = insertelement <16 x float> %temp.vect186.i, float %204, i32 8
  %temp.vect188.i = insertelement <16 x float> %temp.vect187.i, float %205, i32 9
  %temp.vect189.i = insertelement <16 x float> %temp.vect188.i, float %206, i32 10
  %temp.vect190.i = insertelement <16 x float> %temp.vect189.i, float %207, i32 11
  %temp.vect191.i = insertelement <16 x float> %temp.vect190.i, float %208, i32 12
  %temp.vect192.i = insertelement <16 x float> %temp.vect191.i, float %209, i32 13
  %temp.vect193.i = insertelement <16 x float> %temp.vect192.i, float %210, i32 14
  %temp.vect194.i = insertelement <16 x float> %temp.vect193.i, float %211, i32 15
  %212 = uitofp i8 %148 to float
  %213 = uitofp i8 %149 to float
  %214 = uitofp i8 %150 to float
  %215 = uitofp i8 %151 to float
  %216 = uitofp i8 %152 to float
  %217 = uitofp i8 %153 to float
  %218 = uitofp i8 %154 to float
  %219 = uitofp i8 %155 to float
  %220 = uitofp i8 %156 to float
  %221 = uitofp i8 %157 to float
  %222 = uitofp i8 %158 to float
  %223 = uitofp i8 %159 to float
  %224 = uitofp i8 %160 to float
  %225 = uitofp i8 %161 to float
  %226 = uitofp i8 %162 to float
  %227 = uitofp i8 %163 to float
  %temp.vect195.i = insertelement <16 x float> undef, float %212, i32 0
  %temp.vect196.i = insertelement <16 x float> %temp.vect195.i, float %213, i32 1
  %temp.vect197.i = insertelement <16 x float> %temp.vect196.i, float %214, i32 2
  %temp.vect198.i = insertelement <16 x float> %temp.vect197.i, float %215, i32 3
  %temp.vect199.i = insertelement <16 x float> %temp.vect198.i, float %216, i32 4
  %temp.vect200.i = insertelement <16 x float> %temp.vect199.i, float %217, i32 5
  %temp.vect201.i = insertelement <16 x float> %temp.vect200.i, float %218, i32 6
  %temp.vect202.i = insertelement <16 x float> %temp.vect201.i, float %219, i32 7
  %temp.vect203.i = insertelement <16 x float> %temp.vect202.i, float %220, i32 8
  %temp.vect204.i = insertelement <16 x float> %temp.vect203.i, float %221, i32 9
  %temp.vect205.i = insertelement <16 x float> %temp.vect204.i, float %222, i32 10
  %temp.vect206.i = insertelement <16 x float> %temp.vect205.i, float %223, i32 11
  %temp.vect207.i = insertelement <16 x float> %temp.vect206.i, float %224, i32 12
  %temp.vect208.i = insertelement <16 x float> %temp.vect207.i, float %225, i32 13
  %temp.vect209.i = insertelement <16 x float> %temp.vect208.i, float %226, i32 14
  %temp.vect210.i = insertelement <16 x float> %temp.vect209.i, float %227, i32 15
  %228 = fmul <16 x float> %vector146.i, %temp.vect162.i
  %229 = fmul <16 x float> %vector146.i, %temp.vect178.i
  %230 = fmul <16 x float> %vector146.i, %temp.vect194.i
  %231 = fmul <16 x float> %vector146.i, %temp.vect210.i
  %232 = fmul <16 x float> %vector212.i, %vectorPHI107.i
  %233 = fmul <16 x float> %vector212.i, %vectorPHI108.i
  %234 = fmul <16 x float> %vector212.i, %vectorPHI109.i
  %235 = fmul <16 x float> %vector212.i, %vectorPHI110.i
  %236 = fadd <16 x float> %228, %232
  %237 = fadd <16 x float> %229, %233
  %238 = fadd <16 x float> %230, %234
  %239 = fadd <16 x float> %231, %235
  %240 = fmul <16 x float> %vector214.i, %vectorPHI103.i
  %241 = fmul <16 x float> %vector214.i, %vectorPHI104.i
  %242 = fmul <16 x float> %vector214.i, %vectorPHI105.i
  %243 = fmul <16 x float> %vector214.i, %vectorPHI106.i
  %244 = fsub <16 x float> %236, %240
  %245 = fsub <16 x float> %237, %241
  %246 = fsub <16 x float> %238, %242
  %247 = fsub <16 x float> %239, %243
  %248 = fmul <16 x float> %vector216.i, %vectorPHI99.i
  %249 = fmul <16 x float> %vector216.i, %vectorPHI100.i
  %250 = fmul <16 x float> %vector216.i, %vectorPHI101.i
  %251 = fmul <16 x float> %vector216.i, %vectorPHI102.i
  %252 = fsub <16 x float> %244, %248
  %extract218.i = extractelement <16 x float> %252, i32 1
  %extract219.i = extractelement <16 x float> %252, i32 2
  %extract220.i = extractelement <16 x float> %252, i32 3
  %extract221.i = extractelement <16 x float> %252, i32 4
  %extract222.i = extractelement <16 x float> %252, i32 5
  %extract223.i = extractelement <16 x float> %252, i32 6
  %extract224.i = extractelement <16 x float> %252, i32 7
  %extract225.i = extractelement <16 x float> %252, i32 8
  %extract226.i = extractelement <16 x float> %252, i32 9
  %extract227.i = extractelement <16 x float> %252, i32 10
  %extract228.i = extractelement <16 x float> %252, i32 11
  %extract229.i = extractelement <16 x float> %252, i32 12
  %extract230.i = extractelement <16 x float> %252, i32 13
  %extract231.i = extractelement <16 x float> %252, i32 14
  %extract232.i = extractelement <16 x float> %252, i32 15
  %253 = fsub <16 x float> %245, %249
  %extract234.i = extractelement <16 x float> %253, i32 1
  %extract235.i = extractelement <16 x float> %253, i32 2
  %extract236.i = extractelement <16 x float> %253, i32 3
  %extract237.i = extractelement <16 x float> %253, i32 4
  %extract238.i = extractelement <16 x float> %253, i32 5
  %extract239.i = extractelement <16 x float> %253, i32 6
  %extract240.i = extractelement <16 x float> %253, i32 7
  %extract241.i = extractelement <16 x float> %253, i32 8
  %extract242.i = extractelement <16 x float> %253, i32 9
  %extract243.i = extractelement <16 x float> %253, i32 10
  %extract244.i = extractelement <16 x float> %253, i32 11
  %extract245.i = extractelement <16 x float> %253, i32 12
  %extract246.i = extractelement <16 x float> %253, i32 13
  %extract247.i = extractelement <16 x float> %253, i32 14
  %extract248.i = extractelement <16 x float> %253, i32 15
  %254 = fsub <16 x float> %246, %250
  %extract250.i = extractelement <16 x float> %254, i32 1
  %extract251.i = extractelement <16 x float> %254, i32 2
  %extract252.i = extractelement <16 x float> %254, i32 3
  %extract253.i = extractelement <16 x float> %254, i32 4
  %extract254.i = extractelement <16 x float> %254, i32 5
  %extract255.i = extractelement <16 x float> %254, i32 6
  %extract256.i = extractelement <16 x float> %254, i32 7
  %extract257.i = extractelement <16 x float> %254, i32 8
  %extract258.i = extractelement <16 x float> %254, i32 9
  %extract259.i = extractelement <16 x float> %254, i32 10
  %extract260.i = extractelement <16 x float> %254, i32 11
  %extract261.i = extractelement <16 x float> %254, i32 12
  %extract262.i = extractelement <16 x float> %254, i32 13
  %extract263.i = extractelement <16 x float> %254, i32 14
  %extract264.i = extractelement <16 x float> %254, i32 15
  %255 = fsub <16 x float> %247, %251
  %extract266.i = extractelement <16 x float> %255, i32 1
  %extract267.i = extractelement <16 x float> %255, i32 2
  %extract268.i = extractelement <16 x float> %255, i32 3
  %extract269.i = extractelement <16 x float> %255, i32 4
  %extract270.i = extractelement <16 x float> %255, i32 5
  %extract271.i = extractelement <16 x float> %255, i32 6
  %extract272.i = extractelement <16 x float> %255, i32 7
  %extract273.i = extractelement <16 x float> %255, i32 8
  %extract274.i = extractelement <16 x float> %255, i32 9
  %extract275.i = extractelement <16 x float> %255, i32 10
  %extract276.i = extractelement <16 x float> %255, i32 11
  %extract277.i = extractelement <16 x float> %255, i32 12
  %extract278.i = extractelement <16 x float> %255, i32 13
  %extract279.i = extractelement <16 x float> %255, i32 14
  %extract280.i = extractelement <16 x float> %255, i32 15
  %256 = fptoui float %extract218.i to i8
  %257 = fptoui float %extract219.i to i8
  %258 = fptoui float %extract220.i to i8
  %259 = fptoui float %extract221.i to i8
  %260 = fptoui float %extract222.i to i8
  %261 = fptoui float %extract223.i to i8
  %262 = fptoui float %extract224.i to i8
  %263 = fptoui float %extract225.i to i8
  %264 = fptoui float %extract226.i to i8
  %265 = fptoui float %extract227.i to i8
  %266 = fptoui float %extract228.i to i8
  %267 = fptoui float %extract229.i to i8
  %268 = fptoui float %extract230.i to i8
  %269 = fptoui float %extract231.i to i8
  %270 = fptoui float %extract232.i to i8
  %271 = fptoui float %extract234.i to i8
  %272 = fptoui float %extract235.i to i8
  %273 = fptoui float %extract236.i to i8
  %274 = fptoui float %extract237.i to i8
  %275 = fptoui float %extract238.i to i8
  %276 = fptoui float %extract239.i to i8
  %277 = fptoui float %extract240.i to i8
  %278 = fptoui float %extract241.i to i8
  %279 = fptoui float %extract242.i to i8
  %280 = fptoui float %extract243.i to i8
  %281 = fptoui float %extract244.i to i8
  %282 = fptoui float %extract245.i to i8
  %283 = fptoui float %extract246.i to i8
  %284 = fptoui float %extract247.i to i8
  %285 = fptoui float %extract248.i to i8
  %286 = fptoui float %extract250.i to i8
  %287 = fptoui float %extract251.i to i8
  %288 = fptoui float %extract252.i to i8
  %289 = fptoui float %extract253.i to i8
  %290 = fptoui float %extract254.i to i8
  %291 = fptoui float %extract255.i to i8
  %292 = fptoui float %extract256.i to i8
  %293 = fptoui float %extract257.i to i8
  %294 = fptoui float %extract258.i to i8
  %295 = fptoui float %extract259.i to i8
  %296 = fptoui float %extract260.i to i8
  %297 = fptoui float %extract261.i to i8
  %298 = fptoui float %extract262.i to i8
  %299 = fptoui float %extract263.i to i8
  %300 = fptoui float %extract264.i to i8
  %301 = fptoui float %extract266.i to i8
  %302 = fptoui float %extract267.i to i8
  %303 = fptoui float %extract268.i to i8
  %304 = fptoui float %extract269.i to i8
  %305 = fptoui float %extract270.i to i8
  %306 = fptoui float %extract271.i to i8
  %307 = fptoui float %extract272.i to i8
  %308 = fptoui float %extract273.i to i8
  %309 = fptoui float %extract274.i to i8
  %310 = fptoui float %extract275.i to i8
  %311 = fptoui float %extract276.i to i8
  %312 = fptoui float %extract277.i to i8
  %313 = fptoui float %extract278.i to i8
  %314 = fptoui float %extract279.i to i8
  %315 = fptoui float %extract280.i to i8
  %316 = insertelement <4 x i8> undef, i8 %256, i32 0
  %317 = insertelement <4 x i8> undef, i8 %257, i32 0
  %318 = insertelement <4 x i8> undef, i8 %258, i32 0
  %319 = insertelement <4 x i8> undef, i8 %259, i32 0
  %320 = insertelement <4 x i8> undef, i8 %260, i32 0
  %321 = insertelement <4 x i8> undef, i8 %261, i32 0
  %322 = insertelement <4 x i8> undef, i8 %262, i32 0
  %323 = insertelement <4 x i8> undef, i8 %263, i32 0
  %324 = insertelement <4 x i8> undef, i8 %264, i32 0
  %325 = insertelement <4 x i8> undef, i8 %265, i32 0
  %326 = insertelement <4 x i8> undef, i8 %266, i32 0
  %327 = insertelement <4 x i8> undef, i8 %267, i32 0
  %328 = insertelement <4 x i8> undef, i8 %268, i32 0
  %329 = insertelement <4 x i8> undef, i8 %269, i32 0
  %330 = insertelement <4 x i8> undef, i8 %270, i32 0
  %331 = insertelement <4 x i8> %316, i8 %271, i32 1
  %332 = insertelement <4 x i8> %317, i8 %272, i32 1
  %333 = insertelement <4 x i8> %318, i8 %273, i32 1
  %334 = insertelement <4 x i8> %319, i8 %274, i32 1
  %335 = insertelement <4 x i8> %320, i8 %275, i32 1
  %336 = insertelement <4 x i8> %321, i8 %276, i32 1
  %337 = insertelement <4 x i8> %322, i8 %277, i32 1
  %338 = insertelement <4 x i8> %323, i8 %278, i32 1
  %339 = insertelement <4 x i8> %324, i8 %279, i32 1
  %340 = insertelement <4 x i8> %325, i8 %280, i32 1
  %341 = insertelement <4 x i8> %326, i8 %281, i32 1
  %342 = insertelement <4 x i8> %327, i8 %282, i32 1
  %343 = insertelement <4 x i8> %328, i8 %283, i32 1
  %344 = insertelement <4 x i8> %329, i8 %284, i32 1
  %345 = insertelement <4 x i8> %330, i8 %285, i32 1
  %346 = insertelement <4 x i8> %331, i8 %286, i32 2
  %347 = insertelement <4 x i8> %332, i8 %287, i32 2
  %348 = insertelement <4 x i8> %333, i8 %288, i32 2
  %349 = insertelement <4 x i8> %334, i8 %289, i32 2
  %350 = insertelement <4 x i8> %335, i8 %290, i32 2
  %351 = insertelement <4 x i8> %336, i8 %291, i32 2
  %352 = insertelement <4 x i8> %337, i8 %292, i32 2
  %353 = insertelement <4 x i8> %338, i8 %293, i32 2
  %354 = insertelement <4 x i8> %339, i8 %294, i32 2
  %355 = insertelement <4 x i8> %340, i8 %295, i32 2
  %356 = insertelement <4 x i8> %341, i8 %296, i32 2
  %357 = insertelement <4 x i8> %342, i8 %297, i32 2
  %358 = insertelement <4 x i8> %343, i8 %298, i32 2
  %359 = insertelement <4 x i8> %344, i8 %299, i32 2
  %360 = insertelement <4 x i8> %345, i8 %300, i32 2
  %361 = insertelement <4 x i8> %346, i8 %301, i32 3
  %362 = insertelement <4 x i8> %347, i8 %302, i32 3
  %363 = insertelement <4 x i8> %348, i8 %303, i32 3
  %364 = insertelement <4 x i8> %349, i8 %304, i32 3
  %365 = insertelement <4 x i8> %350, i8 %305, i32 3
  %366 = insertelement <4 x i8> %351, i8 %306, i32 3
  %367 = insertelement <4 x i8> %352, i8 %307, i32 3
  %368 = insertelement <4 x i8> %353, i8 %308, i32 3
  %369 = insertelement <4 x i8> %354, i8 %309, i32 3
  %370 = insertelement <4 x i8> %355, i8 %310, i32 3
  %371 = insertelement <4 x i8> %356, i8 %311, i32 3
  %372 = insertelement <4 x i8> %357, i8 %312, i32 3
  %373 = insertelement <4 x i8> %358, i8 %313, i32 3
  %374 = insertelement <4 x i8> %359, i8 %314, i32 3
  %375 = insertelement <4 x i8> %360, i8 %315, i32 3
  %376 = extractelement <16 x i32> %tmp21113.i, i32 1
  %377 = sext i32 %376 to i64
  %378 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %377
  %379 = extractelement <16 x i32> %tmp21113.i, i32 2
  %380 = sext i32 %379 to i64
  %381 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %380
  %382 = extractelement <16 x i32> %tmp21113.i, i32 3
  %383 = sext i32 %382 to i64
  %384 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %383
  %385 = extractelement <16 x i32> %tmp21113.i, i32 4
  %386 = sext i32 %385 to i64
  %387 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %386
  %388 = extractelement <16 x i32> %tmp21113.i, i32 5
  %389 = sext i32 %388 to i64
  %390 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %389
  %391 = extractelement <16 x i32> %tmp21113.i, i32 6
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %392
  %394 = extractelement <16 x i32> %tmp21113.i, i32 7
  %395 = sext i32 %394 to i64
  %396 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %395
  %397 = extractelement <16 x i32> %tmp21113.i, i32 8
  %398 = sext i32 %397 to i64
  %399 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %398
  %400 = extractelement <16 x i32> %tmp21113.i, i32 9
  %401 = sext i32 %400 to i64
  %402 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %401
  %403 = extractelement <16 x i32> %tmp21113.i, i32 10
  %404 = sext i32 %403 to i64
  %405 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %404
  %406 = extractelement <16 x i32> %tmp21113.i, i32 11
  %407 = sext i32 %406 to i64
  %408 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %407
  %409 = extractelement <16 x i32> %tmp21113.i, i32 12
  %410 = sext i32 %409 to i64
  %411 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %410
  %412 = extractelement <16 x i32> %tmp21113.i, i32 13
  %413 = sext i32 %412 to i64
  %414 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %413
  %415 = extractelement <16 x i32> %tmp21113.i, i32 14
  %416 = sext i32 %415 to i64
  %417 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %416
  %418 = extractelement <16 x i32> %tmp21113.i, i32 15
  %419 = sext i32 %418 to i64
  %420 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %419
  br i1 %extract129.i, label %preload619.i, label %postload620.i

preload619.i:                                     ; preds = %postload692.i
  %extract217.i = extractelement <16 x float> %252, i32 0
  %421 = fptoui float %extract217.i to i8
  %extract233.i = extractelement <16 x float> %253, i32 0
  %422 = insertelement <4 x i8> undef, i8 %421, i32 0
  %423 = fptoui float %extract233.i to i8
  %extract249.i = extractelement <16 x float> %254, i32 0
  %424 = insertelement <4 x i8> %422, i8 %423, i32 1
  %425 = fptoui float %extract249.i to i8
  %extract265.i = extractelement <16 x float> %255, i32 0
  %426 = insertelement <4 x i8> %424, i8 %425, i32 2
  %427 = fptoui float %extract265.i to i8
  %428 = extractelement <16 x i32> %tmp21113.i, i32 0
  %429 = sext i32 %428 to i64
  %430 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %429
  %431 = insertelement <4 x i8> %426, i8 %427, i32 3
  store <4 x i8> %431, <4 x i8> addrspace(1)* %430, align 4
  br label %postload620.i

postload620.i:                                    ; preds = %preload619.i, %postload692.i
  br i1 %extract130.i, label %preload624.i, label %postload625.i

preload624.i:                                     ; preds = %postload620.i
  store <4 x i8> %361, <4 x i8> addrspace(1)* %378, align 4
  br label %postload625.i

postload625.i:                                    ; preds = %preload624.i, %postload620.i
  br i1 %extract131.i, label %preload629.i, label %postload630.i

preload629.i:                                     ; preds = %postload625.i
  store <4 x i8> %362, <4 x i8> addrspace(1)* %381, align 4
  br label %postload630.i

postload630.i:                                    ; preds = %preload629.i, %postload625.i
  br i1 %extract132.i, label %preload634.i, label %postload635.i

preload634.i:                                     ; preds = %postload630.i
  store <4 x i8> %363, <4 x i8> addrspace(1)* %384, align 4
  br label %postload635.i

postload635.i:                                    ; preds = %preload634.i, %postload630.i
  br i1 %extract133.i, label %preload639.i, label %postload640.i

preload639.i:                                     ; preds = %postload635.i
  store <4 x i8> %364, <4 x i8> addrspace(1)* %387, align 4
  br label %postload640.i

postload640.i:                                    ; preds = %preload639.i, %postload635.i
  br i1 %extract134.i, label %preload644.i, label %postload645.i

preload644.i:                                     ; preds = %postload640.i
  store <4 x i8> %365, <4 x i8> addrspace(1)* %390, align 4
  br label %postload645.i

postload645.i:                                    ; preds = %preload644.i, %postload640.i
  br i1 %extract135.i, label %preload649.i, label %postload650.i

preload649.i:                                     ; preds = %postload645.i
  store <4 x i8> %366, <4 x i8> addrspace(1)* %393, align 4
  br label %postload650.i

postload650.i:                                    ; preds = %preload649.i, %postload645.i
  br i1 %extract136.i, label %preload654.i, label %postload655.i

preload654.i:                                     ; preds = %postload650.i
  store <4 x i8> %367, <4 x i8> addrspace(1)* %396, align 4
  br label %postload655.i

postload655.i:                                    ; preds = %preload654.i, %postload650.i
  br i1 %extract137.i, label %preload659.i, label %postload660.i

preload659.i:                                     ; preds = %postload655.i
  store <4 x i8> %368, <4 x i8> addrspace(1)* %399, align 4
  br label %postload660.i

postload660.i:                                    ; preds = %preload659.i, %postload655.i
  br i1 %extract138.i, label %preload664.i, label %postload665.i

preload664.i:                                     ; preds = %postload660.i
  store <4 x i8> %369, <4 x i8> addrspace(1)* %402, align 4
  br label %postload665.i

postload665.i:                                    ; preds = %preload664.i, %postload660.i
  br i1 %extract139.i, label %preload669.i, label %postload670.i

preload669.i:                                     ; preds = %postload665.i
  store <4 x i8> %370, <4 x i8> addrspace(1)* %405, align 4
  br label %postload670.i

postload670.i:                                    ; preds = %preload669.i, %postload665.i
  br i1 %extract140.i, label %preload674.i, label %postload675.i

preload674.i:                                     ; preds = %postload670.i
  store <4 x i8> %371, <4 x i8> addrspace(1)* %408, align 4
  br label %postload675.i

postload675.i:                                    ; preds = %preload674.i, %postload670.i
  br i1 %extract141.i, label %preload679.i, label %postload680.i

preload679.i:                                     ; preds = %postload675.i
  store <4 x i8> %372, <4 x i8> addrspace(1)* %411, align 4
  br label %postload680.i

postload680.i:                                    ; preds = %preload679.i, %postload675.i
  br i1 %extract142.i, label %preload684.i, label %postload685.i

preload684.i:                                     ; preds = %postload680.i
  store <4 x i8> %373, <4 x i8> addrspace(1)* %414, align 4
  br label %postload685.i

postload685.i:                                    ; preds = %preload684.i, %postload680.i
  br i1 %extract143.i, label %preload689.i, label %postload690.i

preload689.i:                                     ; preds = %postload685.i
  store <4 x i8> %374, <4 x i8> addrspace(1)* %417, align 4
  br label %postload690.i

postload690.i:                                    ; preds = %preload689.i, %postload685.i
  br i1 %extract144.i, label %preload694.i, label %postload695.i

preload694.i:                                     ; preds = %postload690.i
  store <4 x i8> %375, <4 x i8> addrspace(1)* %420, align 4
  br label %postload695.i

postload695.i:                                    ; preds = %preload694.i, %postload690.i
  %432 = add nsw i32 %y.010.i, 1
  %exitcond18.i = icmp eq i32 %432, %10
  %temp281.i = insertelement <16 x i1> undef, i1 %exitcond18.i, i32 0
  %vector282.i = shufflevector <16 x i1> %temp281.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond.i = xor i1 %exitcond18.i, true
  %temp287.i = insertelement <16 x i1> undef, i1 %notCond.i, i32 0
  %vector288.i = shufflevector <16 x i1> %temp287.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr283.i = and <16 x i1> %vectorPHI98.i, %vector282.i
  %loop_mask54285.i = or <16 x i1> %vectorPHI.i, %who_left_tr283.i
  %curr_loop_mask286.i = or <16 x i1> %loop_mask54285.i, %who_left_tr283.i
  %ipred.i1.i = bitcast <16 x i1> %curr_loop_mask286.i to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %tmp.i3.i = and i32 %val.i2.i, 1
  %res.i4.i = icmp eq i32 %tmp.i3.i, 0
  %local_edge289.i = and <16 x i1> %vectorPHI98.i, %vector288.i
  br i1 %res.i4.i, label %51, label %._crit_edge.i

._crit_edge.i:                                    ; preds = %postload695.i, %SyncBB.i
  br i1 %extract290.i, label %preload784.i, label %postload785.i

preload784.i:                                     ; preds = %._crit_edge.i
  %check.WI.iter878.i = icmp ult i64 %CurrWI..0.i, %37
  br i1 %check.WI.iter878.i, label %thenBB875.i, label %elseBB876.i

thenBB875.i:                                      ; preds = %preload784.i
  %"CurrWI++879.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride881.i" = add nuw i64 %CurrSBIndex..0.i, 448
  br label %SyncBB.i

elseBB876.i:                                      ; preds = %preload784.i
  call void @llvm.x86.sse2.mfence() nounwind
  br label %postload785.i

postload785.i:                                    ; preds = %thenBB.i, %elseBB876.i, %._crit_edge.i
  %vector370887.i = phi <16 x float> [ %vector370.i, %elseBB876.i ], [ %vector370887.i, %thenBB.i ], [ %vector370.le.i, %._crit_edge.i ]
  %vector368885.i = phi <16 x float> [ %vector368.i, %elseBB876.i ], [ %vector368885.i, %thenBB.i ], [ %vector368.le.i, %._crit_edge.i ]
  %currBarrier.1.i = phi i32 [ 7, %elseBB876.i ], [ %currBarrier.1.i, %thenBB.i ], [ %currBarrier.2.ph.i, %._crit_edge.i ]
  %CurrSBIndex..2.i = phi i64 [ 0, %elseBB876.i ], [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ %CurrSBIndex..0.i, %._crit_edge.i ]
  %CurrWI..2.i = phi i64 [ 0, %elseBB876.i ], [ %"CurrWI++.i", %thenBB.i ], [ %CurrWI..0.i, %._crit_edge.i ]
  %"&(pSB[currWI].offset)840.i" = add nuw i64 %CurrSBIndex..2.i, 384
  %"&pSB[currWI].offset841.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset)840.i"
  %CastToValueType842.i = bitcast i8* %"&pSB[currWI].offset841.i" to <16 x i1>*
  %loadedValue843.i = load <16 x i1>* %CastToValueType842.i, align 16
  %._crit_edge_to_bb.nph308.i = and <16 x i1> %loadedValue843.i, %vector307.i
  %"&(pSB[currWI].offset)827.i" = add nuw i64 %CurrSBIndex..2.i, 320
  %"&pSB[currWI].offset828.i" = getelementptr inbounds i8* %40, i64 %"&(pSB[currWI].offset)827.i"
  %CastToValueType829.i = bitcast i8* %"&pSB[currWI].offset828.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType829.i, align 64
  %tmp16311.i = add <16 x i32> %vector310.i, %loadedValue.i
  %ipred.i5.i = bitcast <16 x i1> %._crit_edge_to_bb.nph308.i to i16
  %val.i6.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i5.i, i16 %ipred.i5.i) nounwind
  %tmp.i7.i = and i32 %val.i6.i, 1
  %res.i8.i = icmp eq i32 %tmp.i7.i, 0
  br i1 %res.i8.i, label %.preheader882.i, label %.loopexit.i

.preheader882.i:                                  ; preds = %postload785.i
  %negIncomingLoopMask78312.i = xor <16 x i1> %._crit_edge_to_bb.nph308.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %433

; <label>:433                                     ; preds = %postload783.i, %.preheader882.i
  %vectorPHI313.i = phi <16 x i1> [ %loop_mask63503.i, %postload783.i ], [ %negIncomingLoopMask78312.i, %.preheader882.i ]
  %vectorPHI315.i = phi <16 x i1> [ %local_edge69507.i, %postload783.i ], [ %._crit_edge_to_bb.nph308.i, %.preheader882.i ]
  %indvar.i = phi i32 [ %indvar.next.i, %postload783.i ], [ 0, %.preheader882.i ]
  %vectorPHI316.i = phi <16 x float> [ %vectorPHI320.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI317.i = phi <16 x float> [ %vectorPHI321.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI318.i = phi <16 x float> [ %vectorPHI322.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI319.i = phi <16 x float> [ %vectorPHI323.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI320.i = phi <16 x float> [ %570, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI321.i = phi <16 x float> [ %571, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI322.i = phi <16 x float> [ %572, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI323.i = phi <16 x float> [ %573, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI324.i = phi <16 x float> [ %vectorPHI328.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI325.i = phi <16 x float> [ %vectorPHI329.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI326.i = phi <16 x float> [ %vectorPHI330.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI327.i = phi <16 x float> [ %vectorPHI331.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI328.i = phi <16 x float> [ %temp.vect523.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI329.i = phi <16 x float> [ %temp.vect539.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI330.i = phi <16 x float> [ %temp.vect555.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %vectorPHI331.i = phi <16 x float> [ %temp.vect571.i, %postload783.i ], [ zeroinitializer, %.preheader882.i ]
  %extract351.i = extractelement <16 x i1> %vectorPHI315.i, i32 0
  %extract352.i = extractelement <16 x i1> %vectorPHI315.i, i32 1
  %extract353.i = extractelement <16 x i1> %vectorPHI315.i, i32 2
  %extract354.i = extractelement <16 x i1> %vectorPHI315.i, i32 3
  %extract355.i = extractelement <16 x i1> %vectorPHI315.i, i32 4
  %extract356.i = extractelement <16 x i1> %vectorPHI315.i, i32 5
  %extract357.i = extractelement <16 x i1> %vectorPHI315.i, i32 6
  %extract358.i = extractelement <16 x i1> %vectorPHI315.i, i32 7
  %extract359.i = extractelement <16 x i1> %vectorPHI315.i, i32 8
  %extract360.i = extractelement <16 x i1> %vectorPHI315.i, i32 9
  %extract361.i = extractelement <16 x i1> %vectorPHI315.i, i32 10
  %extract362.i = extractelement <16 x i1> %vectorPHI315.i, i32 11
  %extract363.i = extractelement <16 x i1> %vectorPHI315.i, i32 12
  %extract364.i = extractelement <16 x i1> %vectorPHI315.i, i32 13
  %extract365.i = extractelement <16 x i1> %vectorPHI315.i, i32 14
  %extract366.i = extractelement <16 x i1> %vectorPHI315.i, i32 15
  %tmp12.i = mul i32 %indvar.i, %tmp.i
  %temp332.i = insertelement <16 x i32> undef, i32 %tmp12.i, i32 0
  %vector333.i = shufflevector <16 x i32> %temp332.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp17334.i = add <16 x i32> %tmp16311.i, %vector333.i
  %434 = extractelement <16 x i32> %tmp17334.i, i32 1
  %435 = sext i32 %434 to i64
  %436 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %435
  %437 = extractelement <16 x i32> %tmp17334.i, i32 2
  %438 = sext i32 %437 to i64
  %439 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %438
  %440 = extractelement <16 x i32> %tmp17334.i, i32 3
  %441 = sext i32 %440 to i64
  %442 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %441
  %443 = extractelement <16 x i32> %tmp17334.i, i32 4
  %444 = sext i32 %443 to i64
  %445 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %444
  %446 = extractelement <16 x i32> %tmp17334.i, i32 5
  %447 = sext i32 %446 to i64
  %448 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %447
  %449 = extractelement <16 x i32> %tmp17334.i, i32 6
  %450 = sext i32 %449 to i64
  %451 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %450
  %452 = extractelement <16 x i32> %tmp17334.i, i32 7
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %453
  %455 = extractelement <16 x i32> %tmp17334.i, i32 8
  %456 = sext i32 %455 to i64
  %457 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %456
  %458 = extractelement <16 x i32> %tmp17334.i, i32 9
  %459 = sext i32 %458 to i64
  %460 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %459
  %461 = extractelement <16 x i32> %tmp17334.i, i32 10
  %462 = sext i32 %461 to i64
  %463 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %462
  %464 = extractelement <16 x i32> %tmp17334.i, i32 11
  %465 = sext i32 %464 to i64
  %466 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %465
  %467 = extractelement <16 x i32> %tmp17334.i, i32 12
  %468 = sext i32 %467 to i64
  %469 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %468
  %470 = extractelement <16 x i32> %tmp17334.i, i32 13
  %471 = sext i32 %470 to i64
  %472 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %471
  %473 = extractelement <16 x i32> %tmp17334.i, i32 14
  %474 = sext i32 %473 to i64
  %475 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %474
  %476 = extractelement <16 x i32> %tmp17334.i, i32 15
  %477 = sext i32 %476 to i64
  %478 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %477
  br i1 %extract351.i, label %preload786.i, label %postload787.i

preload786.i:                                     ; preds = %433
  %479 = extractelement <16 x i32> %tmp17334.i, i32 0
  %480 = sext i32 %479 to i64
  %481 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %480
  %masked_load587.i = load <4 x i8> addrspace(1)* %481, align 4
  br label %postload787.i

postload787.i:                                    ; preds = %preload786.i, %433
  %phi788.i = phi <4 x i8> [ undef, %433 ], [ %masked_load587.i, %preload786.i ]
  br i1 %extract352.i, label %preload794.i, label %postload795.i

preload794.i:                                     ; preds = %postload787.i
  %masked_load588.i = load <4 x i8> addrspace(1)* %436, align 4
  br label %postload795.i

postload795.i:                                    ; preds = %preload794.i, %postload787.i
  %phi796.i = phi <4 x i8> [ undef, %postload787.i ], [ %masked_load588.i, %preload794.i ]
  br i1 %extract353.i, label %preload802.i, label %postload803.i

preload802.i:                                     ; preds = %postload795.i
  %masked_load589.i = load <4 x i8> addrspace(1)* %439, align 4
  br label %postload803.i

postload803.i:                                    ; preds = %preload802.i, %postload795.i
  %phi804.i = phi <4 x i8> [ undef, %postload795.i ], [ %masked_load589.i, %preload802.i ]
  br i1 %extract354.i, label %preload810.i, label %postload811.i

preload810.i:                                     ; preds = %postload803.i
  %masked_load590.i = load <4 x i8> addrspace(1)* %442, align 4
  br label %postload811.i

postload811.i:                                    ; preds = %preload810.i, %postload803.i
  %phi812.i = phi <4 x i8> [ undef, %postload803.i ], [ %masked_load590.i, %preload810.i ]
  br i1 %extract355.i, label %preload818.i, label %postload819.i

preload818.i:                                     ; preds = %postload811.i
  %masked_load591.i = load <4 x i8> addrspace(1)* %445, align 4
  br label %postload819.i

postload819.i:                                    ; preds = %preload818.i, %postload811.i
  %phi820.i = phi <4 x i8> [ undef, %postload811.i ], [ %masked_load591.i, %preload818.i ]
  br i1 %extract356.i, label %preload696.i, label %postload697.i

preload696.i:                                     ; preds = %postload819.i
  %masked_load592.i = load <4 x i8> addrspace(1)* %448, align 4
  br label %postload697.i

postload697.i:                                    ; preds = %preload696.i, %postload819.i
  %phi698.i = phi <4 x i8> [ undef, %postload819.i ], [ %masked_load592.i, %preload696.i ]
  br i1 %extract357.i, label %preload704.i, label %postload705.i

preload704.i:                                     ; preds = %postload697.i
  %masked_load593.i = load <4 x i8> addrspace(1)* %451, align 4
  br label %postload705.i

postload705.i:                                    ; preds = %preload704.i, %postload697.i
  %phi706.i = phi <4 x i8> [ undef, %postload697.i ], [ %masked_load593.i, %preload704.i ]
  br i1 %extract358.i, label %preload712.i, label %postload713.i

preload712.i:                                     ; preds = %postload705.i
  %masked_load594.i = load <4 x i8> addrspace(1)* %454, align 4
  br label %postload713.i

postload713.i:                                    ; preds = %preload712.i, %postload705.i
  %phi714.i = phi <4 x i8> [ undef, %postload705.i ], [ %masked_load594.i, %preload712.i ]
  br i1 %extract359.i, label %preload720.i, label %postload721.i

preload720.i:                                     ; preds = %postload713.i
  %masked_load595.i = load <4 x i8> addrspace(1)* %457, align 4
  br label %postload721.i

postload721.i:                                    ; preds = %preload720.i, %postload713.i
  %phi722.i = phi <4 x i8> [ undef, %postload713.i ], [ %masked_load595.i, %preload720.i ]
  br i1 %extract360.i, label %preload728.i, label %postload729.i

preload728.i:                                     ; preds = %postload721.i
  %masked_load596.i = load <4 x i8> addrspace(1)* %460, align 4
  br label %postload729.i

postload729.i:                                    ; preds = %preload728.i, %postload721.i
  %phi730.i = phi <4 x i8> [ undef, %postload721.i ], [ %masked_load596.i, %preload728.i ]
  br i1 %extract361.i, label %preload736.i, label %postload737.i

preload736.i:                                     ; preds = %postload729.i
  %masked_load597.i = load <4 x i8> addrspace(1)* %463, align 4
  br label %postload737.i

postload737.i:                                    ; preds = %preload736.i, %postload729.i
  %phi738.i = phi <4 x i8> [ undef, %postload729.i ], [ %masked_load597.i, %preload736.i ]
  br i1 %extract362.i, label %preload744.i, label %postload745.i

preload744.i:                                     ; preds = %postload737.i
  %masked_load598.i = load <4 x i8> addrspace(1)* %466, align 4
  br label %postload745.i

postload745.i:                                    ; preds = %preload744.i, %postload737.i
  %phi746.i = phi <4 x i8> [ undef, %postload737.i ], [ %masked_load598.i, %preload744.i ]
  br i1 %extract363.i, label %preload752.i, label %postload753.i

preload752.i:                                     ; preds = %postload745.i
  %masked_load599.i = load <4 x i8> addrspace(1)* %469, align 4
  br label %postload753.i

postload753.i:                                    ; preds = %preload752.i, %postload745.i
  %phi754.i = phi <4 x i8> [ undef, %postload745.i ], [ %masked_load599.i, %preload752.i ]
  br i1 %extract364.i, label %preload760.i, label %postload761.i

preload760.i:                                     ; preds = %postload753.i
  %masked_load600.i = load <4 x i8> addrspace(1)* %472, align 4
  br label %postload761.i

postload761.i:                                    ; preds = %preload760.i, %postload753.i
  %phi762.i = phi <4 x i8> [ undef, %postload753.i ], [ %masked_load600.i, %preload760.i ]
  br i1 %extract365.i, label %preload768.i, label %postload769.i

preload768.i:                                     ; preds = %postload761.i
  %masked_load601.i = load <4 x i8> addrspace(1)* %475, align 4
  br label %postload769.i

postload769.i:                                    ; preds = %preload768.i, %postload761.i
  %phi770.i = phi <4 x i8> [ undef, %postload761.i ], [ %masked_load601.i, %preload768.i ]
  br i1 %extract366.i, label %preload776.i, label %postload777.i

preload776.i:                                     ; preds = %postload769.i
  %masked_load602.i = load <4 x i8> addrspace(1)* %478, align 4
  br label %postload777.i

postload777.i:                                    ; preds = %preload776.i, %postload769.i
  %phi778.i = phi <4 x i8> [ undef, %postload769.i ], [ %masked_load602.i, %preload776.i ]
  %482 = extractelement <4 x i8> %phi788.i, i32 0
  %483 = extractelement <4 x i8> %phi796.i, i32 0
  %484 = extractelement <4 x i8> %phi804.i, i32 0
  %485 = extractelement <4 x i8> %phi812.i, i32 0
  %486 = extractelement <4 x i8> %phi820.i, i32 0
  %487 = extractelement <4 x i8> %phi698.i, i32 0
  %488 = extractelement <4 x i8> %phi706.i, i32 0
  %489 = extractelement <4 x i8> %phi714.i, i32 0
  %490 = extractelement <4 x i8> %phi722.i, i32 0
  %491 = extractelement <4 x i8> %phi730.i, i32 0
  %492 = extractelement <4 x i8> %phi738.i, i32 0
  %493 = extractelement <4 x i8> %phi746.i, i32 0
  %494 = extractelement <4 x i8> %phi754.i, i32 0
  %495 = extractelement <4 x i8> %phi762.i, i32 0
  %496 = extractelement <4 x i8> %phi770.i, i32 0
  %497 = extractelement <4 x i8> %phi778.i, i32 0
  %498 = extractelement <4 x i8> %phi788.i, i32 1
  %499 = extractelement <4 x i8> %phi796.i, i32 1
  %500 = extractelement <4 x i8> %phi804.i, i32 1
  %501 = extractelement <4 x i8> %phi812.i, i32 1
  %502 = extractelement <4 x i8> %phi820.i, i32 1
  %503 = extractelement <4 x i8> %phi698.i, i32 1
  %504 = extractelement <4 x i8> %phi706.i, i32 1
  %505 = extractelement <4 x i8> %phi714.i, i32 1
  %506 = extractelement <4 x i8> %phi722.i, i32 1
  %507 = extractelement <4 x i8> %phi730.i, i32 1
  %508 = extractelement <4 x i8> %phi738.i, i32 1
  %509 = extractelement <4 x i8> %phi746.i, i32 1
  %510 = extractelement <4 x i8> %phi754.i, i32 1
  %511 = extractelement <4 x i8> %phi762.i, i32 1
  %512 = extractelement <4 x i8> %phi770.i, i32 1
  %513 = extractelement <4 x i8> %phi778.i, i32 1
  %514 = extractelement <4 x i8> %phi788.i, i32 2
  %515 = extractelement <4 x i8> %phi796.i, i32 2
  %516 = extractelement <4 x i8> %phi804.i, i32 2
  %517 = extractelement <4 x i8> %phi812.i, i32 2
  %518 = extractelement <4 x i8> %phi820.i, i32 2
  %519 = extractelement <4 x i8> %phi698.i, i32 2
  %520 = extractelement <4 x i8> %phi706.i, i32 2
  %521 = extractelement <4 x i8> %phi714.i, i32 2
  %522 = extractelement <4 x i8> %phi722.i, i32 2
  %523 = extractelement <4 x i8> %phi730.i, i32 2
  %524 = extractelement <4 x i8> %phi738.i, i32 2
  %525 = extractelement <4 x i8> %phi746.i, i32 2
  %526 = extractelement <4 x i8> %phi754.i, i32 2
  %527 = extractelement <4 x i8> %phi762.i, i32 2
  %528 = extractelement <4 x i8> %phi770.i, i32 2
  %529 = extractelement <4 x i8> %phi778.i, i32 2
  %530 = extractelement <4 x i8> %phi788.i, i32 3
  %531 = extractelement <4 x i8> %phi796.i, i32 3
  %532 = extractelement <4 x i8> %phi804.i, i32 3
  %533 = extractelement <4 x i8> %phi812.i, i32 3
  %534 = extractelement <4 x i8> %phi820.i, i32 3
  %535 = extractelement <4 x i8> %phi698.i, i32 3
  %536 = extractelement <4 x i8> %phi706.i, i32 3
  %537 = extractelement <4 x i8> %phi714.i, i32 3
  %538 = extractelement <4 x i8> %phi722.i, i32 3
  %539 = extractelement <4 x i8> %phi730.i, i32 3
  %540 = extractelement <4 x i8> %phi738.i, i32 3
  %541 = extractelement <4 x i8> %phi746.i, i32 3
  %542 = extractelement <4 x i8> %phi754.i, i32 3
  %543 = extractelement <4 x i8> %phi762.i, i32 3
  %544 = extractelement <4 x i8> %phi770.i, i32 3
  %545 = extractelement <4 x i8> %phi778.i, i32 3
  %546 = fmul <16 x float> %vector368885.i, %vectorPHI328.i
  %547 = fmul <16 x float> %vector368885.i, %vectorPHI329.i
  %548 = fmul <16 x float> %vector368885.i, %vectorPHI330.i
  %549 = fmul <16 x float> %vector368885.i, %vectorPHI331.i
  %550 = fmul <16 x float> %vector370887.i, %vectorPHI324.i
  %551 = fmul <16 x float> %vector370887.i, %vectorPHI325.i
  %552 = fmul <16 x float> %vector370887.i, %vectorPHI326.i
  %553 = fmul <16 x float> %vector370887.i, %vectorPHI327.i
  %554 = fadd <16 x float> %546, %550
  %555 = fadd <16 x float> %547, %551
  %556 = fadd <16 x float> %548, %552
  %557 = fadd <16 x float> %549, %553
  %558 = fmul <16 x float> %vector214.i, %vectorPHI320.i
  %559 = fmul <16 x float> %vector214.i, %vectorPHI321.i
  %560 = fmul <16 x float> %vector214.i, %vectorPHI322.i
  %561 = fmul <16 x float> %vector214.i, %vectorPHI323.i
  %562 = fsub <16 x float> %554, %558
  %563 = fsub <16 x float> %555, %559
  %564 = fsub <16 x float> %556, %560
  %565 = fsub <16 x float> %557, %561
  %566 = fmul <16 x float> %vector216.i, %vectorPHI316.i
  %567 = fmul <16 x float> %vector216.i, %vectorPHI317.i
  %568 = fmul <16 x float> %vector216.i, %vectorPHI318.i
  %569 = fmul <16 x float> %vector216.i, %vectorPHI319.i
  %570 = fsub <16 x float> %562, %566
  %571 = fsub <16 x float> %563, %567
  %572 = fsub <16 x float> %564, %568
  %573 = fsub <16 x float> %565, %569
  %574 = extractelement <16 x i32> %tmp17334.i, i32 0
  %575 = sext i32 %574 to i64
  %576 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %575
  %577 = extractelement <16 x i32> %tmp17334.i, i32 1
  %578 = sext i32 %577 to i64
  %579 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %578
  %580 = extractelement <16 x i32> %tmp17334.i, i32 2
  %581 = sext i32 %580 to i64
  %582 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %581
  %583 = extractelement <16 x i32> %tmp17334.i, i32 3
  %584 = sext i32 %583 to i64
  %585 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %584
  %586 = extractelement <16 x i32> %tmp17334.i, i32 4
  %587 = sext i32 %586 to i64
  %588 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %587
  %589 = extractelement <16 x i32> %tmp17334.i, i32 5
  %590 = sext i32 %589 to i64
  %591 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %590
  %592 = extractelement <16 x i32> %tmp17334.i, i32 6
  %593 = sext i32 %592 to i64
  %594 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %593
  %595 = extractelement <16 x i32> %tmp17334.i, i32 7
  %596 = sext i32 %595 to i64
  %597 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %596
  %598 = extractelement <16 x i32> %tmp17334.i, i32 8
  %599 = sext i32 %598 to i64
  %600 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %599
  %601 = extractelement <16 x i32> %tmp17334.i, i32 9
  %602 = sext i32 %601 to i64
  %603 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %602
  %604 = extractelement <16 x i32> %tmp17334.i, i32 10
  %605 = sext i32 %604 to i64
  %606 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %605
  %607 = extractelement <16 x i32> %tmp17334.i, i32 11
  %608 = sext i32 %607 to i64
  %609 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %608
  %610 = extractelement <16 x i32> %tmp17334.i, i32 12
  %611 = sext i32 %610 to i64
  %612 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %611
  %613 = extractelement <16 x i32> %tmp17334.i, i32 13
  %614 = sext i32 %613 to i64
  %615 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %614
  %616 = extractelement <16 x i32> %tmp17334.i, i32 14
  %617 = sext i32 %616 to i64
  %618 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %617
  %619 = extractelement <16 x i32> %tmp17334.i, i32 15
  %620 = sext i32 %619 to i64
  %621 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %620
  br i1 %extract351.i, label %preload789.i, label %postload790.i

preload789.i:                                     ; preds = %postload777.i
  %masked_load603.i = load <4 x i8> addrspace(1)* %576, align 4
  br label %postload790.i

postload790.i:                                    ; preds = %preload789.i, %postload777.i
  %phi791.i = phi <4 x i8> [ undef, %postload777.i ], [ %masked_load603.i, %preload789.i ]
  br i1 %extract352.i, label %preload797.i, label %postload798.i

preload797.i:                                     ; preds = %postload790.i
  %masked_load604.i = load <4 x i8> addrspace(1)* %579, align 4
  br label %postload798.i

postload798.i:                                    ; preds = %preload797.i, %postload790.i
  %phi799.i = phi <4 x i8> [ undef, %postload790.i ], [ %masked_load604.i, %preload797.i ]
  br i1 %extract353.i, label %preload805.i, label %postload806.i

preload805.i:                                     ; preds = %postload798.i
  %masked_load605.i = load <4 x i8> addrspace(1)* %582, align 4
  br label %postload806.i

postload806.i:                                    ; preds = %preload805.i, %postload798.i
  %phi807.i = phi <4 x i8> [ undef, %postload798.i ], [ %masked_load605.i, %preload805.i ]
  br i1 %extract354.i, label %preload813.i, label %postload814.i

preload813.i:                                     ; preds = %postload806.i
  %masked_load606.i = load <4 x i8> addrspace(1)* %585, align 4
  br label %postload814.i

postload814.i:                                    ; preds = %preload813.i, %postload806.i
  %phi815.i = phi <4 x i8> [ undef, %postload806.i ], [ %masked_load606.i, %preload813.i ]
  br i1 %extract355.i, label %preload821.i, label %postload822.i

preload821.i:                                     ; preds = %postload814.i
  %masked_load607.i = load <4 x i8> addrspace(1)* %588, align 4
  br label %postload822.i

postload822.i:                                    ; preds = %preload821.i, %postload814.i
  %phi823.i = phi <4 x i8> [ undef, %postload814.i ], [ %masked_load607.i, %preload821.i ]
  br i1 %extract356.i, label %preload699.i, label %postload700.i

preload699.i:                                     ; preds = %postload822.i
  %masked_load608.i = load <4 x i8> addrspace(1)* %591, align 4
  br label %postload700.i

postload700.i:                                    ; preds = %preload699.i, %postload822.i
  %phi701.i = phi <4 x i8> [ undef, %postload822.i ], [ %masked_load608.i, %preload699.i ]
  br i1 %extract357.i, label %preload707.i, label %postload708.i

preload707.i:                                     ; preds = %postload700.i
  %masked_load609.i = load <4 x i8> addrspace(1)* %594, align 4
  br label %postload708.i

postload708.i:                                    ; preds = %preload707.i, %postload700.i
  %phi709.i = phi <4 x i8> [ undef, %postload700.i ], [ %masked_load609.i, %preload707.i ]
  br i1 %extract358.i, label %preload715.i, label %postload716.i

preload715.i:                                     ; preds = %postload708.i
  %masked_load610.i = load <4 x i8> addrspace(1)* %597, align 4
  br label %postload716.i

postload716.i:                                    ; preds = %preload715.i, %postload708.i
  %phi717.i = phi <4 x i8> [ undef, %postload708.i ], [ %masked_load610.i, %preload715.i ]
  br i1 %extract359.i, label %preload723.i, label %postload724.i

preload723.i:                                     ; preds = %postload716.i
  %masked_load611.i = load <4 x i8> addrspace(1)* %600, align 4
  br label %postload724.i

postload724.i:                                    ; preds = %preload723.i, %postload716.i
  %phi725.i = phi <4 x i8> [ undef, %postload716.i ], [ %masked_load611.i, %preload723.i ]
  br i1 %extract360.i, label %preload731.i, label %postload732.i

preload731.i:                                     ; preds = %postload724.i
  %masked_load612.i = load <4 x i8> addrspace(1)* %603, align 4
  br label %postload732.i

postload732.i:                                    ; preds = %preload731.i, %postload724.i
  %phi733.i = phi <4 x i8> [ undef, %postload724.i ], [ %masked_load612.i, %preload731.i ]
  br i1 %extract361.i, label %preload739.i, label %postload740.i

preload739.i:                                     ; preds = %postload732.i
  %masked_load613.i = load <4 x i8> addrspace(1)* %606, align 4
  br label %postload740.i

postload740.i:                                    ; preds = %preload739.i, %postload732.i
  %phi741.i = phi <4 x i8> [ undef, %postload732.i ], [ %masked_load613.i, %preload739.i ]
  br i1 %extract362.i, label %preload747.i, label %postload748.i

preload747.i:                                     ; preds = %postload740.i
  %masked_load614.i = load <4 x i8> addrspace(1)* %609, align 4
  br label %postload748.i

postload748.i:                                    ; preds = %preload747.i, %postload740.i
  %phi749.i = phi <4 x i8> [ undef, %postload740.i ], [ %masked_load614.i, %preload747.i ]
  br i1 %extract363.i, label %preload755.i, label %postload756.i

preload755.i:                                     ; preds = %postload748.i
  %masked_load615.i = load <4 x i8> addrspace(1)* %612, align 4
  br label %postload756.i

postload756.i:                                    ; preds = %preload755.i, %postload748.i
  %phi757.i = phi <4 x i8> [ undef, %postload748.i ], [ %masked_load615.i, %preload755.i ]
  br i1 %extract364.i, label %preload763.i, label %postload764.i

preload763.i:                                     ; preds = %postload756.i
  %masked_load616.i = load <4 x i8> addrspace(1)* %615, align 4
  br label %postload764.i

postload764.i:                                    ; preds = %preload763.i, %postload756.i
  %phi765.i = phi <4 x i8> [ undef, %postload756.i ], [ %masked_load616.i, %preload763.i ]
  br i1 %extract365.i, label %preload771.i, label %postload772.i

preload771.i:                                     ; preds = %postload764.i
  %masked_load617.i = load <4 x i8> addrspace(1)* %618, align 4
  br label %postload772.i

postload772.i:                                    ; preds = %preload771.i, %postload764.i
  %phi773.i = phi <4 x i8> [ undef, %postload764.i ], [ %masked_load617.i, %preload771.i ]
  br i1 %extract366.i, label %preload779.i, label %postload780.i

preload779.i:                                     ; preds = %postload772.i
  %masked_load618.i = load <4 x i8> addrspace(1)* %621, align 4
  br label %postload780.i

postload780.i:                                    ; preds = %preload779.i, %postload772.i
  %phi781.i = phi <4 x i8> [ undef, %postload772.i ], [ %masked_load618.i, %preload779.i ]
  %622 = extractelement <4 x i8> %phi791.i, i32 0
  %623 = extractelement <4 x i8> %phi799.i, i32 0
  %624 = extractelement <4 x i8> %phi807.i, i32 0
  %625 = extractelement <4 x i8> %phi815.i, i32 0
  %626 = extractelement <4 x i8> %phi823.i, i32 0
  %627 = extractelement <4 x i8> %phi701.i, i32 0
  %628 = extractelement <4 x i8> %phi709.i, i32 0
  %629 = extractelement <4 x i8> %phi717.i, i32 0
  %630 = extractelement <4 x i8> %phi725.i, i32 0
  %631 = extractelement <4 x i8> %phi733.i, i32 0
  %632 = extractelement <4 x i8> %phi741.i, i32 0
  %633 = extractelement <4 x i8> %phi749.i, i32 0
  %634 = extractelement <4 x i8> %phi757.i, i32 0
  %635 = extractelement <4 x i8> %phi765.i, i32 0
  %636 = extractelement <4 x i8> %phi773.i, i32 0
  %637 = extractelement <4 x i8> %phi781.i, i32 0
  %638 = extractelement <4 x i8> %phi791.i, i32 1
  %639 = extractelement <4 x i8> %phi799.i, i32 1
  %640 = extractelement <4 x i8> %phi807.i, i32 1
  %641 = extractelement <4 x i8> %phi815.i, i32 1
  %642 = extractelement <4 x i8> %phi823.i, i32 1
  %643 = extractelement <4 x i8> %phi701.i, i32 1
  %644 = extractelement <4 x i8> %phi709.i, i32 1
  %645 = extractelement <4 x i8> %phi717.i, i32 1
  %646 = extractelement <4 x i8> %phi725.i, i32 1
  %647 = extractelement <4 x i8> %phi733.i, i32 1
  %648 = extractelement <4 x i8> %phi741.i, i32 1
  %649 = extractelement <4 x i8> %phi749.i, i32 1
  %650 = extractelement <4 x i8> %phi757.i, i32 1
  %651 = extractelement <4 x i8> %phi765.i, i32 1
  %652 = extractelement <4 x i8> %phi773.i, i32 1
  %653 = extractelement <4 x i8> %phi781.i, i32 1
  %654 = extractelement <4 x i8> %phi791.i, i32 2
  %655 = extractelement <4 x i8> %phi799.i, i32 2
  %656 = extractelement <4 x i8> %phi807.i, i32 2
  %657 = extractelement <4 x i8> %phi815.i, i32 2
  %658 = extractelement <4 x i8> %phi823.i, i32 2
  %659 = extractelement <4 x i8> %phi701.i, i32 2
  %660 = extractelement <4 x i8> %phi709.i, i32 2
  %661 = extractelement <4 x i8> %phi717.i, i32 2
  %662 = extractelement <4 x i8> %phi725.i, i32 2
  %663 = extractelement <4 x i8> %phi733.i, i32 2
  %664 = extractelement <4 x i8> %phi741.i, i32 2
  %665 = extractelement <4 x i8> %phi749.i, i32 2
  %666 = extractelement <4 x i8> %phi757.i, i32 2
  %667 = extractelement <4 x i8> %phi765.i, i32 2
  %668 = extractelement <4 x i8> %phi773.i, i32 2
  %669 = extractelement <4 x i8> %phi781.i, i32 2
  %670 = extractelement <4 x i8> %phi791.i, i32 3
  %671 = extractelement <4 x i8> %phi799.i, i32 3
  %672 = extractelement <4 x i8> %phi807.i, i32 3
  %673 = extractelement <4 x i8> %phi815.i, i32 3
  %674 = extractelement <4 x i8> %phi823.i, i32 3
  %675 = extractelement <4 x i8> %phi701.i, i32 3
  %676 = extractelement <4 x i8> %phi709.i, i32 3
  %677 = extractelement <4 x i8> %phi717.i, i32 3
  %678 = extractelement <4 x i8> %phi725.i, i32 3
  %679 = extractelement <4 x i8> %phi733.i, i32 3
  %680 = extractelement <4 x i8> %phi741.i, i32 3
  %681 = extractelement <4 x i8> %phi749.i, i32 3
  %682 = extractelement <4 x i8> %phi757.i, i32 3
  %683 = extractelement <4 x i8> %phi765.i, i32 3
  %684 = extractelement <4 x i8> %phi773.i, i32 3
  %685 = extractelement <4 x i8> %phi781.i, i32 3
  %686 = uitofp i8 %622 to float
  %687 = uitofp i8 %623 to float
  %688 = uitofp i8 %624 to float
  %689 = uitofp i8 %625 to float
  %690 = uitofp i8 %626 to float
  %691 = uitofp i8 %627 to float
  %692 = uitofp i8 %628 to float
  %693 = uitofp i8 %629 to float
  %694 = uitofp i8 %630 to float
  %695 = uitofp i8 %631 to float
  %696 = uitofp i8 %632 to float
  %697 = uitofp i8 %633 to float
  %698 = uitofp i8 %634 to float
  %699 = uitofp i8 %635 to float
  %700 = uitofp i8 %636 to float
  %701 = uitofp i8 %637 to float
  %temp.vect371.i = insertelement <16 x float> undef, float %686, i32 0
  %temp.vect372.i = insertelement <16 x float> %temp.vect371.i, float %687, i32 1
  %temp.vect373.i = insertelement <16 x float> %temp.vect372.i, float %688, i32 2
  %temp.vect374.i = insertelement <16 x float> %temp.vect373.i, float %689, i32 3
  %temp.vect375.i = insertelement <16 x float> %temp.vect374.i, float %690, i32 4
  %temp.vect376.i = insertelement <16 x float> %temp.vect375.i, float %691, i32 5
  %temp.vect377.i = insertelement <16 x float> %temp.vect376.i, float %692, i32 6
  %temp.vect378.i = insertelement <16 x float> %temp.vect377.i, float %693, i32 7
  %temp.vect379.i = insertelement <16 x float> %temp.vect378.i, float %694, i32 8
  %temp.vect380.i = insertelement <16 x float> %temp.vect379.i, float %695, i32 9
  %temp.vect381.i = insertelement <16 x float> %temp.vect380.i, float %696, i32 10
  %temp.vect382.i = insertelement <16 x float> %temp.vect381.i, float %697, i32 11
  %temp.vect383.i = insertelement <16 x float> %temp.vect382.i, float %698, i32 12
  %temp.vect384.i = insertelement <16 x float> %temp.vect383.i, float %699, i32 13
  %temp.vect385.i = insertelement <16 x float> %temp.vect384.i, float %700, i32 14
  %temp.vect386.i = insertelement <16 x float> %temp.vect385.i, float %701, i32 15
  %702 = uitofp i8 %638 to float
  %703 = uitofp i8 %639 to float
  %704 = uitofp i8 %640 to float
  %705 = uitofp i8 %641 to float
  %706 = uitofp i8 %642 to float
  %707 = uitofp i8 %643 to float
  %708 = uitofp i8 %644 to float
  %709 = uitofp i8 %645 to float
  %710 = uitofp i8 %646 to float
  %711 = uitofp i8 %647 to float
  %712 = uitofp i8 %648 to float
  %713 = uitofp i8 %649 to float
  %714 = uitofp i8 %650 to float
  %715 = uitofp i8 %651 to float
  %716 = uitofp i8 %652 to float
  %717 = uitofp i8 %653 to float
  %temp.vect387.i = insertelement <16 x float> undef, float %702, i32 0
  %temp.vect388.i = insertelement <16 x float> %temp.vect387.i, float %703, i32 1
  %temp.vect389.i = insertelement <16 x float> %temp.vect388.i, float %704, i32 2
  %temp.vect390.i = insertelement <16 x float> %temp.vect389.i, float %705, i32 3
  %temp.vect391.i = insertelement <16 x float> %temp.vect390.i, float %706, i32 4
  %temp.vect392.i = insertelement <16 x float> %temp.vect391.i, float %707, i32 5
  %temp.vect393.i = insertelement <16 x float> %temp.vect392.i, float %708, i32 6
  %temp.vect394.i = insertelement <16 x float> %temp.vect393.i, float %709, i32 7
  %temp.vect395.i = insertelement <16 x float> %temp.vect394.i, float %710, i32 8
  %temp.vect396.i = insertelement <16 x float> %temp.vect395.i, float %711, i32 9
  %temp.vect397.i = insertelement <16 x float> %temp.vect396.i, float %712, i32 10
  %temp.vect398.i = insertelement <16 x float> %temp.vect397.i, float %713, i32 11
  %temp.vect399.i = insertelement <16 x float> %temp.vect398.i, float %714, i32 12
  %temp.vect400.i = insertelement <16 x float> %temp.vect399.i, float %715, i32 13
  %temp.vect401.i = insertelement <16 x float> %temp.vect400.i, float %716, i32 14
  %temp.vect402.i = insertelement <16 x float> %temp.vect401.i, float %717, i32 15
  %718 = uitofp i8 %654 to float
  %719 = uitofp i8 %655 to float
  %720 = uitofp i8 %656 to float
  %721 = uitofp i8 %657 to float
  %722 = uitofp i8 %658 to float
  %723 = uitofp i8 %659 to float
  %724 = uitofp i8 %660 to float
  %725 = uitofp i8 %661 to float
  %726 = uitofp i8 %662 to float
  %727 = uitofp i8 %663 to float
  %728 = uitofp i8 %664 to float
  %729 = uitofp i8 %665 to float
  %730 = uitofp i8 %666 to float
  %731 = uitofp i8 %667 to float
  %732 = uitofp i8 %668 to float
  %733 = uitofp i8 %669 to float
  %temp.vect403.i = insertelement <16 x float> undef, float %718, i32 0
  %temp.vect404.i = insertelement <16 x float> %temp.vect403.i, float %719, i32 1
  %temp.vect405.i = insertelement <16 x float> %temp.vect404.i, float %720, i32 2
  %temp.vect406.i = insertelement <16 x float> %temp.vect405.i, float %721, i32 3
  %temp.vect407.i = insertelement <16 x float> %temp.vect406.i, float %722, i32 4
  %temp.vect408.i = insertelement <16 x float> %temp.vect407.i, float %723, i32 5
  %temp.vect409.i = insertelement <16 x float> %temp.vect408.i, float %724, i32 6
  %temp.vect410.i = insertelement <16 x float> %temp.vect409.i, float %725, i32 7
  %temp.vect411.i = insertelement <16 x float> %temp.vect410.i, float %726, i32 8
  %temp.vect412.i = insertelement <16 x float> %temp.vect411.i, float %727, i32 9
  %temp.vect413.i = insertelement <16 x float> %temp.vect412.i, float %728, i32 10
  %temp.vect414.i = insertelement <16 x float> %temp.vect413.i, float %729, i32 11
  %temp.vect415.i = insertelement <16 x float> %temp.vect414.i, float %730, i32 12
  %temp.vect416.i = insertelement <16 x float> %temp.vect415.i, float %731, i32 13
  %temp.vect417.i = insertelement <16 x float> %temp.vect416.i, float %732, i32 14
  %temp.vect418.i = insertelement <16 x float> %temp.vect417.i, float %733, i32 15
  %734 = uitofp i8 %670 to float
  %735 = uitofp i8 %671 to float
  %736 = uitofp i8 %672 to float
  %737 = uitofp i8 %673 to float
  %738 = uitofp i8 %674 to float
  %739 = uitofp i8 %675 to float
  %740 = uitofp i8 %676 to float
  %741 = uitofp i8 %677 to float
  %742 = uitofp i8 %678 to float
  %743 = uitofp i8 %679 to float
  %744 = uitofp i8 %680 to float
  %745 = uitofp i8 %681 to float
  %746 = uitofp i8 %682 to float
  %747 = uitofp i8 %683 to float
  %748 = uitofp i8 %684 to float
  %749 = uitofp i8 %685 to float
  %temp.vect419.i = insertelement <16 x float> undef, float %734, i32 0
  %temp.vect420.i = insertelement <16 x float> %temp.vect419.i, float %735, i32 1
  %temp.vect421.i = insertelement <16 x float> %temp.vect420.i, float %736, i32 2
  %temp.vect422.i = insertelement <16 x float> %temp.vect421.i, float %737, i32 3
  %temp.vect423.i = insertelement <16 x float> %temp.vect422.i, float %738, i32 4
  %temp.vect424.i = insertelement <16 x float> %temp.vect423.i, float %739, i32 5
  %temp.vect425.i = insertelement <16 x float> %temp.vect424.i, float %740, i32 6
  %temp.vect426.i = insertelement <16 x float> %temp.vect425.i, float %741, i32 7
  %temp.vect427.i = insertelement <16 x float> %temp.vect426.i, float %742, i32 8
  %temp.vect428.i = insertelement <16 x float> %temp.vect427.i, float %743, i32 9
  %temp.vect429.i = insertelement <16 x float> %temp.vect428.i, float %744, i32 10
  %temp.vect430.i = insertelement <16 x float> %temp.vect429.i, float %745, i32 11
  %temp.vect431.i = insertelement <16 x float> %temp.vect430.i, float %746, i32 12
  %temp.vect432.i = insertelement <16 x float> %temp.vect431.i, float %747, i32 13
  %temp.vect433.i = insertelement <16 x float> %temp.vect432.i, float %748, i32 14
  %temp.vect434.i = insertelement <16 x float> %temp.vect433.i, float %749, i32 15
  %750 = fadd <16 x float> %temp.vect386.i, %570
  %extract436.i = extractelement <16 x float> %750, i32 1
  %extract437.i = extractelement <16 x float> %750, i32 2
  %extract438.i = extractelement <16 x float> %750, i32 3
  %extract439.i = extractelement <16 x float> %750, i32 4
  %extract440.i = extractelement <16 x float> %750, i32 5
  %extract441.i = extractelement <16 x float> %750, i32 6
  %extract442.i = extractelement <16 x float> %750, i32 7
  %extract443.i = extractelement <16 x float> %750, i32 8
  %extract444.i = extractelement <16 x float> %750, i32 9
  %extract445.i = extractelement <16 x float> %750, i32 10
  %extract446.i = extractelement <16 x float> %750, i32 11
  %extract447.i = extractelement <16 x float> %750, i32 12
  %extract448.i = extractelement <16 x float> %750, i32 13
  %extract449.i = extractelement <16 x float> %750, i32 14
  %extract450.i = extractelement <16 x float> %750, i32 15
  %751 = fadd <16 x float> %temp.vect402.i, %571
  %extract452.i = extractelement <16 x float> %751, i32 1
  %extract453.i = extractelement <16 x float> %751, i32 2
  %extract454.i = extractelement <16 x float> %751, i32 3
  %extract455.i = extractelement <16 x float> %751, i32 4
  %extract456.i = extractelement <16 x float> %751, i32 5
  %extract457.i = extractelement <16 x float> %751, i32 6
  %extract458.i = extractelement <16 x float> %751, i32 7
  %extract459.i = extractelement <16 x float> %751, i32 8
  %extract460.i = extractelement <16 x float> %751, i32 9
  %extract461.i = extractelement <16 x float> %751, i32 10
  %extract462.i = extractelement <16 x float> %751, i32 11
  %extract463.i = extractelement <16 x float> %751, i32 12
  %extract464.i = extractelement <16 x float> %751, i32 13
  %extract465.i = extractelement <16 x float> %751, i32 14
  %extract466.i = extractelement <16 x float> %751, i32 15
  %752 = fadd <16 x float> %temp.vect418.i, %572
  %extract468.i = extractelement <16 x float> %752, i32 1
  %extract469.i = extractelement <16 x float> %752, i32 2
  %extract470.i = extractelement <16 x float> %752, i32 3
  %extract471.i = extractelement <16 x float> %752, i32 4
  %extract472.i = extractelement <16 x float> %752, i32 5
  %extract473.i = extractelement <16 x float> %752, i32 6
  %extract474.i = extractelement <16 x float> %752, i32 7
  %extract475.i = extractelement <16 x float> %752, i32 8
  %extract476.i = extractelement <16 x float> %752, i32 9
  %extract477.i = extractelement <16 x float> %752, i32 10
  %extract478.i = extractelement <16 x float> %752, i32 11
  %extract479.i = extractelement <16 x float> %752, i32 12
  %extract480.i = extractelement <16 x float> %752, i32 13
  %extract481.i = extractelement <16 x float> %752, i32 14
  %extract482.i = extractelement <16 x float> %752, i32 15
  %753 = fadd <16 x float> %temp.vect434.i, %573
  %extract484.i = extractelement <16 x float> %753, i32 1
  %extract485.i = extractelement <16 x float> %753, i32 2
  %extract486.i = extractelement <16 x float> %753, i32 3
  %extract487.i = extractelement <16 x float> %753, i32 4
  %extract488.i = extractelement <16 x float> %753, i32 5
  %extract489.i = extractelement <16 x float> %753, i32 6
  %extract490.i = extractelement <16 x float> %753, i32 7
  %extract491.i = extractelement <16 x float> %753, i32 8
  %extract492.i = extractelement <16 x float> %753, i32 9
  %extract493.i = extractelement <16 x float> %753, i32 10
  %extract494.i = extractelement <16 x float> %753, i32 11
  %extract495.i = extractelement <16 x float> %753, i32 12
  %extract496.i = extractelement <16 x float> %753, i32 13
  %extract497.i = extractelement <16 x float> %753, i32 14
  %extract498.i = extractelement <16 x float> %753, i32 15
  %754 = fptoui float %extract436.i to i8
  %755 = fptoui float %extract437.i to i8
  %756 = fptoui float %extract438.i to i8
  %757 = fptoui float %extract439.i to i8
  %758 = fptoui float %extract440.i to i8
  %759 = fptoui float %extract441.i to i8
  %760 = fptoui float %extract442.i to i8
  %761 = fptoui float %extract443.i to i8
  %762 = fptoui float %extract444.i to i8
  %763 = fptoui float %extract445.i to i8
  %764 = fptoui float %extract446.i to i8
  %765 = fptoui float %extract447.i to i8
  %766 = fptoui float %extract448.i to i8
  %767 = fptoui float %extract449.i to i8
  %768 = fptoui float %extract450.i to i8
  %769 = fptoui float %extract452.i to i8
  %770 = fptoui float %extract453.i to i8
  %771 = fptoui float %extract454.i to i8
  %772 = fptoui float %extract455.i to i8
  %773 = fptoui float %extract456.i to i8
  %774 = fptoui float %extract457.i to i8
  %775 = fptoui float %extract458.i to i8
  %776 = fptoui float %extract459.i to i8
  %777 = fptoui float %extract460.i to i8
  %778 = fptoui float %extract461.i to i8
  %779 = fptoui float %extract462.i to i8
  %780 = fptoui float %extract463.i to i8
  %781 = fptoui float %extract464.i to i8
  %782 = fptoui float %extract465.i to i8
  %783 = fptoui float %extract466.i to i8
  %784 = fptoui float %extract468.i to i8
  %785 = fptoui float %extract469.i to i8
  %786 = fptoui float %extract470.i to i8
  %787 = fptoui float %extract471.i to i8
  %788 = fptoui float %extract472.i to i8
  %789 = fptoui float %extract473.i to i8
  %790 = fptoui float %extract474.i to i8
  %791 = fptoui float %extract475.i to i8
  %792 = fptoui float %extract476.i to i8
  %793 = fptoui float %extract477.i to i8
  %794 = fptoui float %extract478.i to i8
  %795 = fptoui float %extract479.i to i8
  %796 = fptoui float %extract480.i to i8
  %797 = fptoui float %extract481.i to i8
  %798 = fptoui float %extract482.i to i8
  %799 = fptoui float %extract484.i to i8
  %800 = fptoui float %extract485.i to i8
  %801 = fptoui float %extract486.i to i8
  %802 = fptoui float %extract487.i to i8
  %803 = fptoui float %extract488.i to i8
  %804 = fptoui float %extract489.i to i8
  %805 = fptoui float %extract490.i to i8
  %806 = fptoui float %extract491.i to i8
  %807 = fptoui float %extract492.i to i8
  %808 = fptoui float %extract493.i to i8
  %809 = fptoui float %extract494.i to i8
  %810 = fptoui float %extract495.i to i8
  %811 = fptoui float %extract496.i to i8
  %812 = fptoui float %extract497.i to i8
  %813 = fptoui float %extract498.i to i8
  %814 = insertelement <4 x i8> undef, i8 %754, i32 0
  %815 = insertelement <4 x i8> undef, i8 %755, i32 0
  %816 = insertelement <4 x i8> undef, i8 %756, i32 0
  %817 = insertelement <4 x i8> undef, i8 %757, i32 0
  %818 = insertelement <4 x i8> undef, i8 %758, i32 0
  %819 = insertelement <4 x i8> undef, i8 %759, i32 0
  %820 = insertelement <4 x i8> undef, i8 %760, i32 0
  %821 = insertelement <4 x i8> undef, i8 %761, i32 0
  %822 = insertelement <4 x i8> undef, i8 %762, i32 0
  %823 = insertelement <4 x i8> undef, i8 %763, i32 0
  %824 = insertelement <4 x i8> undef, i8 %764, i32 0
  %825 = insertelement <4 x i8> undef, i8 %765, i32 0
  %826 = insertelement <4 x i8> undef, i8 %766, i32 0
  %827 = insertelement <4 x i8> undef, i8 %767, i32 0
  %828 = insertelement <4 x i8> undef, i8 %768, i32 0
  %829 = insertelement <4 x i8> %814, i8 %769, i32 1
  %830 = insertelement <4 x i8> %815, i8 %770, i32 1
  %831 = insertelement <4 x i8> %816, i8 %771, i32 1
  %832 = insertelement <4 x i8> %817, i8 %772, i32 1
  %833 = insertelement <4 x i8> %818, i8 %773, i32 1
  %834 = insertelement <4 x i8> %819, i8 %774, i32 1
  %835 = insertelement <4 x i8> %820, i8 %775, i32 1
  %836 = insertelement <4 x i8> %821, i8 %776, i32 1
  %837 = insertelement <4 x i8> %822, i8 %777, i32 1
  %838 = insertelement <4 x i8> %823, i8 %778, i32 1
  %839 = insertelement <4 x i8> %824, i8 %779, i32 1
  %840 = insertelement <4 x i8> %825, i8 %780, i32 1
  %841 = insertelement <4 x i8> %826, i8 %781, i32 1
  %842 = insertelement <4 x i8> %827, i8 %782, i32 1
  %843 = insertelement <4 x i8> %828, i8 %783, i32 1
  %844 = insertelement <4 x i8> %829, i8 %784, i32 2
  %845 = insertelement <4 x i8> %830, i8 %785, i32 2
  %846 = insertelement <4 x i8> %831, i8 %786, i32 2
  %847 = insertelement <4 x i8> %832, i8 %787, i32 2
  %848 = insertelement <4 x i8> %833, i8 %788, i32 2
  %849 = insertelement <4 x i8> %834, i8 %789, i32 2
  %850 = insertelement <4 x i8> %835, i8 %790, i32 2
  %851 = insertelement <4 x i8> %836, i8 %791, i32 2
  %852 = insertelement <4 x i8> %837, i8 %792, i32 2
  %853 = insertelement <4 x i8> %838, i8 %793, i32 2
  %854 = insertelement <4 x i8> %839, i8 %794, i32 2
  %855 = insertelement <4 x i8> %840, i8 %795, i32 2
  %856 = insertelement <4 x i8> %841, i8 %796, i32 2
  %857 = insertelement <4 x i8> %842, i8 %797, i32 2
  %858 = insertelement <4 x i8> %843, i8 %798, i32 2
  %859 = insertelement <4 x i8> %844, i8 %799, i32 3
  %860 = insertelement <4 x i8> %845, i8 %800, i32 3
  %861 = insertelement <4 x i8> %846, i8 %801, i32 3
  %862 = insertelement <4 x i8> %847, i8 %802, i32 3
  %863 = insertelement <4 x i8> %848, i8 %803, i32 3
  %864 = insertelement <4 x i8> %849, i8 %804, i32 3
  %865 = insertelement <4 x i8> %850, i8 %805, i32 3
  %866 = insertelement <4 x i8> %851, i8 %806, i32 3
  %867 = insertelement <4 x i8> %852, i8 %807, i32 3
  %868 = insertelement <4 x i8> %853, i8 %808, i32 3
  %869 = insertelement <4 x i8> %854, i8 %809, i32 3
  %870 = insertelement <4 x i8> %855, i8 %810, i32 3
  %871 = insertelement <4 x i8> %856, i8 %811, i32 3
  %872 = insertelement <4 x i8> %857, i8 %812, i32 3
  %873 = insertelement <4 x i8> %858, i8 %813, i32 3
  br i1 %extract351.i, label %preload792.i, label %postload793.i

preload792.i:                                     ; preds = %postload780.i
  %extract435.i = extractelement <16 x float> %750, i32 0
  %874 = fptoui float %extract435.i to i8
  %extract451.i = extractelement <16 x float> %751, i32 0
  %875 = insertelement <4 x i8> undef, i8 %874, i32 0
  %876 = fptoui float %extract451.i to i8
  %extract467.i = extractelement <16 x float> %752, i32 0
  %877 = insertelement <4 x i8> %875, i8 %876, i32 1
  %878 = fptoui float %extract467.i to i8
  %extract483.i = extractelement <16 x float> %753, i32 0
  %879 = insertelement <4 x i8> %877, i8 %878, i32 2
  %880 = fptoui float %extract483.i to i8
  %881 = insertelement <4 x i8> %879, i8 %880, i32 3
  store <4 x i8> %881, <4 x i8> addrspace(1)* %576, align 4
  br label %postload793.i

postload793.i:                                    ; preds = %preload792.i, %postload780.i
  br i1 %extract352.i, label %preload800.i, label %postload801.i

preload800.i:                                     ; preds = %postload793.i
  store <4 x i8> %859, <4 x i8> addrspace(1)* %579, align 4
  br label %postload801.i

postload801.i:                                    ; preds = %preload800.i, %postload793.i
  br i1 %extract353.i, label %preload808.i, label %postload809.i

preload808.i:                                     ; preds = %postload801.i
  store <4 x i8> %860, <4 x i8> addrspace(1)* %582, align 4
  br label %postload809.i

postload809.i:                                    ; preds = %preload808.i, %postload801.i
  br i1 %extract354.i, label %preload816.i, label %postload817.i

preload816.i:                                     ; preds = %postload809.i
  store <4 x i8> %861, <4 x i8> addrspace(1)* %585, align 4
  br label %postload817.i

postload817.i:                                    ; preds = %preload816.i, %postload809.i
  br i1 %extract355.i, label %preload824.i, label %postload825.i

preload824.i:                                     ; preds = %postload817.i
  store <4 x i8> %862, <4 x i8> addrspace(1)* %588, align 4
  br label %postload825.i

postload825.i:                                    ; preds = %preload824.i, %postload817.i
  br i1 %extract356.i, label %preload702.i, label %postload703.i

preload702.i:                                     ; preds = %postload825.i
  store <4 x i8> %863, <4 x i8> addrspace(1)* %591, align 4
  br label %postload703.i

postload703.i:                                    ; preds = %preload702.i, %postload825.i
  br i1 %extract357.i, label %preload710.i, label %postload711.i

preload710.i:                                     ; preds = %postload703.i
  store <4 x i8> %864, <4 x i8> addrspace(1)* %594, align 4
  br label %postload711.i

postload711.i:                                    ; preds = %preload710.i, %postload703.i
  br i1 %extract358.i, label %preload718.i, label %postload719.i

preload718.i:                                     ; preds = %postload711.i
  store <4 x i8> %865, <4 x i8> addrspace(1)* %597, align 4
  br label %postload719.i

postload719.i:                                    ; preds = %preload718.i, %postload711.i
  br i1 %extract359.i, label %preload726.i, label %postload727.i

preload726.i:                                     ; preds = %postload719.i
  store <4 x i8> %866, <4 x i8> addrspace(1)* %600, align 4
  br label %postload727.i

postload727.i:                                    ; preds = %preload726.i, %postload719.i
  br i1 %extract360.i, label %preload734.i, label %postload735.i

preload734.i:                                     ; preds = %postload727.i
  store <4 x i8> %867, <4 x i8> addrspace(1)* %603, align 4
  br label %postload735.i

postload735.i:                                    ; preds = %preload734.i, %postload727.i
  br i1 %extract361.i, label %preload742.i, label %postload743.i

preload742.i:                                     ; preds = %postload735.i
  store <4 x i8> %868, <4 x i8> addrspace(1)* %606, align 4
  br label %postload743.i

postload743.i:                                    ; preds = %preload742.i, %postload735.i
  br i1 %extract362.i, label %preload750.i, label %postload751.i

preload750.i:                                     ; preds = %postload743.i
  store <4 x i8> %869, <4 x i8> addrspace(1)* %609, align 4
  br label %postload751.i

postload751.i:                                    ; preds = %preload750.i, %postload743.i
  br i1 %extract363.i, label %preload758.i, label %postload759.i

preload758.i:                                     ; preds = %postload751.i
  store <4 x i8> %870, <4 x i8> addrspace(1)* %612, align 4
  br label %postload759.i

postload759.i:                                    ; preds = %preload758.i, %postload751.i
  br i1 %extract364.i, label %preload766.i, label %postload767.i

preload766.i:                                     ; preds = %postload759.i
  store <4 x i8> %871, <4 x i8> addrspace(1)* %615, align 4
  br label %postload767.i

postload767.i:                                    ; preds = %preload766.i, %postload759.i
  br i1 %extract365.i, label %preload774.i, label %postload775.i

preload774.i:                                     ; preds = %postload767.i
  store <4 x i8> %872, <4 x i8> addrspace(1)* %618, align 4
  br label %postload775.i

postload775.i:                                    ; preds = %preload774.i, %postload767.i
  br i1 %extract366.i, label %preload782.i, label %postload783.i

preload782.i:                                     ; preds = %postload775.i
  store <4 x i8> %873, <4 x i8> addrspace(1)* %621, align 4
  br label %postload783.i

postload783.i:                                    ; preds = %preload782.i, %postload775.i
  %882 = uitofp i8 %482 to float
  %883 = uitofp i8 %483 to float
  %884 = uitofp i8 %484 to float
  %885 = uitofp i8 %485 to float
  %886 = uitofp i8 %486 to float
  %887 = uitofp i8 %487 to float
  %888 = uitofp i8 %488 to float
  %889 = uitofp i8 %489 to float
  %890 = uitofp i8 %490 to float
  %891 = uitofp i8 %491 to float
  %892 = uitofp i8 %492 to float
  %893 = uitofp i8 %493 to float
  %894 = uitofp i8 %494 to float
  %895 = uitofp i8 %495 to float
  %896 = uitofp i8 %496 to float
  %897 = uitofp i8 %497 to float
  %temp.vect508.i = insertelement <16 x float> undef, float %882, i32 0
  %temp.vect509.i = insertelement <16 x float> %temp.vect508.i, float %883, i32 1
  %temp.vect510.i = insertelement <16 x float> %temp.vect509.i, float %884, i32 2
  %temp.vect511.i = insertelement <16 x float> %temp.vect510.i, float %885, i32 3
  %temp.vect512.i = insertelement <16 x float> %temp.vect511.i, float %886, i32 4
  %temp.vect513.i = insertelement <16 x float> %temp.vect512.i, float %887, i32 5
  %temp.vect514.i = insertelement <16 x float> %temp.vect513.i, float %888, i32 6
  %temp.vect515.i = insertelement <16 x float> %temp.vect514.i, float %889, i32 7
  %temp.vect516.i = insertelement <16 x float> %temp.vect515.i, float %890, i32 8
  %temp.vect517.i = insertelement <16 x float> %temp.vect516.i, float %891, i32 9
  %temp.vect518.i = insertelement <16 x float> %temp.vect517.i, float %892, i32 10
  %temp.vect519.i = insertelement <16 x float> %temp.vect518.i, float %893, i32 11
  %temp.vect520.i = insertelement <16 x float> %temp.vect519.i, float %894, i32 12
  %temp.vect521.i = insertelement <16 x float> %temp.vect520.i, float %895, i32 13
  %temp.vect522.i = insertelement <16 x float> %temp.vect521.i, float %896, i32 14
  %temp.vect523.i = insertelement <16 x float> %temp.vect522.i, float %897, i32 15
  %898 = uitofp i8 %498 to float
  %899 = uitofp i8 %499 to float
  %900 = uitofp i8 %500 to float
  %901 = uitofp i8 %501 to float
  %902 = uitofp i8 %502 to float
  %903 = uitofp i8 %503 to float
  %904 = uitofp i8 %504 to float
  %905 = uitofp i8 %505 to float
  %906 = uitofp i8 %506 to float
  %907 = uitofp i8 %507 to float
  %908 = uitofp i8 %508 to float
  %909 = uitofp i8 %509 to float
  %910 = uitofp i8 %510 to float
  %911 = uitofp i8 %511 to float
  %912 = uitofp i8 %512 to float
  %913 = uitofp i8 %513 to float
  %temp.vect524.i = insertelement <16 x float> undef, float %898, i32 0
  %temp.vect525.i = insertelement <16 x float> %temp.vect524.i, float %899, i32 1
  %temp.vect526.i = insertelement <16 x float> %temp.vect525.i, float %900, i32 2
  %temp.vect527.i = insertelement <16 x float> %temp.vect526.i, float %901, i32 3
  %temp.vect528.i = insertelement <16 x float> %temp.vect527.i, float %902, i32 4
  %temp.vect529.i = insertelement <16 x float> %temp.vect528.i, float %903, i32 5
  %temp.vect530.i = insertelement <16 x float> %temp.vect529.i, float %904, i32 6
  %temp.vect531.i = insertelement <16 x float> %temp.vect530.i, float %905, i32 7
  %temp.vect532.i = insertelement <16 x float> %temp.vect531.i, float %906, i32 8
  %temp.vect533.i = insertelement <16 x float> %temp.vect532.i, float %907, i32 9
  %temp.vect534.i = insertelement <16 x float> %temp.vect533.i, float %908, i32 10
  %temp.vect535.i = insertelement <16 x float> %temp.vect534.i, float %909, i32 11
  %temp.vect536.i = insertelement <16 x float> %temp.vect535.i, float %910, i32 12
  %temp.vect537.i = insertelement <16 x float> %temp.vect536.i, float %911, i32 13
  %temp.vect538.i = insertelement <16 x float> %temp.vect537.i, float %912, i32 14
  %temp.vect539.i = insertelement <16 x float> %temp.vect538.i, float %913, i32 15
  %914 = uitofp i8 %514 to float
  %915 = uitofp i8 %515 to float
  %916 = uitofp i8 %516 to float
  %917 = uitofp i8 %517 to float
  %918 = uitofp i8 %518 to float
  %919 = uitofp i8 %519 to float
  %920 = uitofp i8 %520 to float
  %921 = uitofp i8 %521 to float
  %922 = uitofp i8 %522 to float
  %923 = uitofp i8 %523 to float
  %924 = uitofp i8 %524 to float
  %925 = uitofp i8 %525 to float
  %926 = uitofp i8 %526 to float
  %927 = uitofp i8 %527 to float
  %928 = uitofp i8 %528 to float
  %929 = uitofp i8 %529 to float
  %temp.vect540.i = insertelement <16 x float> undef, float %914, i32 0
  %temp.vect541.i = insertelement <16 x float> %temp.vect540.i, float %915, i32 1
  %temp.vect542.i = insertelement <16 x float> %temp.vect541.i, float %916, i32 2
  %temp.vect543.i = insertelement <16 x float> %temp.vect542.i, float %917, i32 3
  %temp.vect544.i = insertelement <16 x float> %temp.vect543.i, float %918, i32 4
  %temp.vect545.i = insertelement <16 x float> %temp.vect544.i, float %919, i32 5
  %temp.vect546.i = insertelement <16 x float> %temp.vect545.i, float %920, i32 6
  %temp.vect547.i = insertelement <16 x float> %temp.vect546.i, float %921, i32 7
  %temp.vect548.i = insertelement <16 x float> %temp.vect547.i, float %922, i32 8
  %temp.vect549.i = insertelement <16 x float> %temp.vect548.i, float %923, i32 9
  %temp.vect550.i = insertelement <16 x float> %temp.vect549.i, float %924, i32 10
  %temp.vect551.i = insertelement <16 x float> %temp.vect550.i, float %925, i32 11
  %temp.vect552.i = insertelement <16 x float> %temp.vect551.i, float %926, i32 12
  %temp.vect553.i = insertelement <16 x float> %temp.vect552.i, float %927, i32 13
  %temp.vect554.i = insertelement <16 x float> %temp.vect553.i, float %928, i32 14
  %temp.vect555.i = insertelement <16 x float> %temp.vect554.i, float %929, i32 15
  %930 = uitofp i8 %530 to float
  %931 = uitofp i8 %531 to float
  %932 = uitofp i8 %532 to float
  %933 = uitofp i8 %533 to float
  %934 = uitofp i8 %534 to float
  %935 = uitofp i8 %535 to float
  %936 = uitofp i8 %536 to float
  %937 = uitofp i8 %537 to float
  %938 = uitofp i8 %538 to float
  %939 = uitofp i8 %539 to float
  %940 = uitofp i8 %540 to float
  %941 = uitofp i8 %541 to float
  %942 = uitofp i8 %542 to float
  %943 = uitofp i8 %543 to float
  %944 = uitofp i8 %544 to float
  %945 = uitofp i8 %545 to float
  %temp.vect556.i = insertelement <16 x float> undef, float %930, i32 0
  %temp.vect557.i = insertelement <16 x float> %temp.vect556.i, float %931, i32 1
  %temp.vect558.i = insertelement <16 x float> %temp.vect557.i, float %932, i32 2
  %temp.vect559.i = insertelement <16 x float> %temp.vect558.i, float %933, i32 3
  %temp.vect560.i = insertelement <16 x float> %temp.vect559.i, float %934, i32 4
  %temp.vect561.i = insertelement <16 x float> %temp.vect560.i, float %935, i32 5
  %temp.vect562.i = insertelement <16 x float> %temp.vect561.i, float %936, i32 6
  %temp.vect563.i = insertelement <16 x float> %temp.vect562.i, float %937, i32 7
  %temp.vect564.i = insertelement <16 x float> %temp.vect563.i, float %938, i32 8
  %temp.vect565.i = insertelement <16 x float> %temp.vect564.i, float %939, i32 9
  %temp.vect566.i = insertelement <16 x float> %temp.vect565.i, float %940, i32 10
  %temp.vect567.i = insertelement <16 x float> %temp.vect566.i, float %941, i32 11
  %temp.vect568.i = insertelement <16 x float> %temp.vect567.i, float %942, i32 12
  %temp.vect569.i = insertelement <16 x float> %temp.vect568.i, float %943, i32 13
  %temp.vect570.i = insertelement <16 x float> %temp.vect569.i, float %944, i32 14
  %temp.vect571.i = insertelement <16 x float> %temp.vect570.i, float %945, i32 15
  %indvar.next.i = add i32 %indvar.i, 1
  %exitcond.i = icmp eq i32 %indvar.next.i, %10
  %temp499.i = insertelement <16 x i1> undef, i1 %exitcond.i, i32 0
  %vector500.i = shufflevector <16 x i1> %temp499.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %notCond59.i = xor i1 %exitcond.i, true
  %temp505.i = insertelement <16 x i1> undef, i1 %notCond59.i, i32 0
  %vector506.i = shufflevector <16 x i1> %temp505.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %who_left_tr60501.i = and <16 x i1> %vectorPHI315.i, %vector500.i
  %loop_mask63503.i = or <16 x i1> %vectorPHI313.i, %who_left_tr60501.i
  %curr_loop_mask65504.i = or <16 x i1> %loop_mask63503.i, %who_left_tr60501.i
  %ipred.i9.i = bitcast <16 x i1> %curr_loop_mask65504.i to i16
  %val.i10.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i9.i, i16 %ipred.i9.i) nounwind
  %tmp.i11.i = and i32 %val.i10.i, 1
  %res.i12.i = icmp eq i32 %tmp.i11.i, 0
  %local_edge69507.i = and <16 x i1> %vectorPHI315.i, %vector506.i
  br i1 %res.i12.i, label %433, label %.loopexit.i

.loopexit.i:                                      ; preds = %postload783.i, %postload785.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..2.i, %37
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.RecursiveGaussian_kernel_separated_args.exit

thenBB.i:                                         ; preds = %.loopexit.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..2.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..2.i, 448
  %cond.i = icmp eq i32 %currBarrier.1.i, 11
  br i1 %cond.i, label %SyncBB.outer.i, label %postload785.i

____Vectorized_.RecursiveGaussian_kernel_separated_args.exit: ; preds = %.loopexit.i
  ret void
}

define void @__Vectorized_.transpose_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <4 x i8> addrspace(1)**
  %1 = load <4 x i8> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to <4 x i8> addrspace(1)**
  %4 = load <4 x i8> addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to <4 x i8> addrspace(3)**
  %7 = load <4 x i8> addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i32*
  %10 = load i32* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 28
  %12 = bitcast i8* %11 to i32*
  %13 = load i32* %12, align 4
  %14 = getelementptr i8* %pBuffer, i64 32
  %15 = bitcast i8* %14 to i32*
  %16 = load i32* %15, align 4
  %17 = getelementptr i8* %pBuffer, i64 64
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 88
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 96
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  br label %SyncBB216.i

SyncBB216.i:                                      ; preds = %thenBB.i, %entry
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %29 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %32 = load i64* %31, align 8
  %33 = add i64 %30, %32
  %broadcast1.i = insertelement <16 x i64> undef, i64 %33, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %34 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %35 = trunc <16 x i64> %34 to <16 x i32>
  %"&(pSB[currWI].offset).i" = add nuw i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset).i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to <16 x i32>*
  store <16 x i32> %35, <16 x i32>* %CastToValueType.i, align 64
  %36 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 1
  %37 = load i64* %36, align 8
  %38 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 1
  %39 = load i64* %38, align 8
  %40 = add i64 %37, %39
  %41 = trunc i64 %40 to i32
  %temp38.i = insertelement <16 x i32> undef, i32 %41, i32 0
  %vector39.i = shufflevector <16 x i32> %temp38.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %"&(pSB[currWI].offset)62.i" = add nuw i64 %CurrSBIndex..0.i, 128
  %"&pSB[currWI].offset63.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)62.i"
  %CastToValueType64.i = bitcast i8* %"&pSB[currWI].offset63.i" to <16 x i32>*
  store <16 x i32> %vector39.i, <16 x i32>* %CastToValueType64.i, align 64
  %42 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %43 = load i64* %42, align 8
  %broadcast11.i = insertelement <16 x i64> undef, i64 %43, i32 0
  %broadcast22.i = shufflevector <16 x i64> %broadcast11.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %44 = add <16 x i64> %broadcast22.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %45 = trunc <16 x i64> %44 to <16 x i32>
  %46 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 1
  %47 = load i64* %46, align 8
  %48 = trunc i64 %47 to i32
  %49 = mul i32 %41, %10
  %temp.i = insertelement <16 x i32> undef, i32 %49, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %50 = add <16 x i32> %vector.i, %35
  %51 = extractelement <16 x i32> %50, i32 0
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %52
  %54 = extractelement <16 x i32> %50, i32 1
  %55 = zext i32 %54 to i64
  %56 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %55
  %57 = extractelement <16 x i32> %50, i32 2
  %58 = zext i32 %57 to i64
  %59 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %58
  %60 = extractelement <16 x i32> %50, i32 3
  %61 = zext i32 %60 to i64
  %62 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %61
  %63 = extractelement <16 x i32> %50, i32 4
  %64 = zext i32 %63 to i64
  %65 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %64
  %66 = extractelement <16 x i32> %50, i32 5
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %67
  %69 = extractelement <16 x i32> %50, i32 6
  %70 = zext i32 %69 to i64
  %71 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %70
  %72 = extractelement <16 x i32> %50, i32 7
  %73 = zext i32 %72 to i64
  %74 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %73
  %75 = extractelement <16 x i32> %50, i32 8
  %76 = zext i32 %75 to i64
  %77 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %76
  %78 = extractelement <16 x i32> %50, i32 9
  %79 = zext i32 %78 to i64
  %80 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %79
  %81 = extractelement <16 x i32> %50, i32 10
  %82 = zext i32 %81 to i64
  %83 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %82
  %84 = extractelement <16 x i32> %50, i32 11
  %85 = zext i32 %84 to i64
  %86 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %85
  %87 = extractelement <16 x i32> %50, i32 12
  %88 = zext i32 %87 to i64
  %89 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %88
  %90 = extractelement <16 x i32> %50, i32 13
  %91 = zext i32 %90 to i64
  %92 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %91
  %93 = extractelement <16 x i32> %50, i32 14
  %94 = zext i32 %93 to i64
  %95 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %94
  %96 = extractelement <16 x i32> %50, i32 15
  %97 = zext i32 %96 to i64
  %98 = getelementptr inbounds <4 x i8> addrspace(1)* %4, i64 %97
  %99 = load <4 x i8> addrspace(1)* %53, align 4
  %100 = load <4 x i8> addrspace(1)* %56, align 4
  %101 = load <4 x i8> addrspace(1)* %59, align 4
  %102 = load <4 x i8> addrspace(1)* %62, align 4
  %103 = load <4 x i8> addrspace(1)* %65, align 4
  %104 = load <4 x i8> addrspace(1)* %68, align 4
  %105 = load <4 x i8> addrspace(1)* %71, align 4
  %106 = load <4 x i8> addrspace(1)* %74, align 4
  %107 = load <4 x i8> addrspace(1)* %77, align 4
  %108 = load <4 x i8> addrspace(1)* %80, align 4
  %109 = load <4 x i8> addrspace(1)* %83, align 4
  %110 = load <4 x i8> addrspace(1)* %86, align 4
  %111 = load <4 x i8> addrspace(1)* %89, align 4
  %112 = load <4 x i8> addrspace(1)* %92, align 4
  %113 = load <4 x i8> addrspace(1)* %95, align 4
  %114 = load <4 x i8> addrspace(1)* %98, align 4
  %115 = mul i32 %48, %16
  %temp18.i = insertelement <16 x i32> undef, i32 %115, i32 0
  %vector19.i = shufflevector <16 x i32> %temp18.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %116 = add <16 x i32> %vector19.i, %45
  %117 = extractelement <16 x i32> %116, i32 0
  %118 = zext i32 %117 to i64
  %119 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %118
  %"&(pSB[currWI].offset)71.i" = add nuw i64 %CurrSBIndex..0.i, 192
  %"&pSB[currWI].offset72.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)71.i"
  %CastToValueType73.i = bitcast i8* %"&pSB[currWI].offset72.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %119, <4 x i8> addrspace(3)** %CastToValueType73.i, align 8
  %120 = extractelement <16 x i32> %116, i32 1
  %121 = zext i32 %120 to i64
  %122 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %121
  %"&(pSB[currWI].offset)80.i" = add nuw i64 %CurrSBIndex..0.i, 200
  %"&pSB[currWI].offset81.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)80.i"
  %CastToValueType82.i = bitcast i8* %"&pSB[currWI].offset81.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %122, <4 x i8> addrspace(3)** %CastToValueType82.i, align 8
  %123 = extractelement <16 x i32> %116, i32 2
  %124 = zext i32 %123 to i64
  %125 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %124
  %"&(pSB[currWI].offset)89.i" = add nuw i64 %CurrSBIndex..0.i, 208
  %"&pSB[currWI].offset90.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)89.i"
  %CastToValueType91.i = bitcast i8* %"&pSB[currWI].offset90.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %125, <4 x i8> addrspace(3)** %CastToValueType91.i, align 8
  %126 = extractelement <16 x i32> %116, i32 3
  %127 = zext i32 %126 to i64
  %128 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %127
  %"&(pSB[currWI].offset)98.i" = add nuw i64 %CurrSBIndex..0.i, 216
  %"&pSB[currWI].offset99.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)98.i"
  %CastToValueType100.i = bitcast i8* %"&pSB[currWI].offset99.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %128, <4 x i8> addrspace(3)** %CastToValueType100.i, align 8
  %129 = extractelement <16 x i32> %116, i32 4
  %130 = zext i32 %129 to i64
  %131 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %130
  %"&(pSB[currWI].offset)107.i" = add nuw i64 %CurrSBIndex..0.i, 224
  %"&pSB[currWI].offset108.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)107.i"
  %CastToValueType109.i = bitcast i8* %"&pSB[currWI].offset108.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %131, <4 x i8> addrspace(3)** %CastToValueType109.i, align 8
  %132 = extractelement <16 x i32> %116, i32 5
  %133 = zext i32 %132 to i64
  %134 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %133
  %"&(pSB[currWI].offset)116.i" = add nuw i64 %CurrSBIndex..0.i, 232
  %"&pSB[currWI].offset117.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)116.i"
  %CastToValueType118.i = bitcast i8* %"&pSB[currWI].offset117.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %134, <4 x i8> addrspace(3)** %CastToValueType118.i, align 8
  %135 = extractelement <16 x i32> %116, i32 6
  %136 = zext i32 %135 to i64
  %137 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %136
  %"&(pSB[currWI].offset)125.i" = add nuw i64 %CurrSBIndex..0.i, 240
  %"&pSB[currWI].offset126.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)125.i"
  %CastToValueType127.i = bitcast i8* %"&pSB[currWI].offset126.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %137, <4 x i8> addrspace(3)** %CastToValueType127.i, align 8
  %138 = extractelement <16 x i32> %116, i32 7
  %139 = zext i32 %138 to i64
  %140 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %139
  %"&(pSB[currWI].offset)134.i" = add nuw i64 %CurrSBIndex..0.i, 248
  %"&pSB[currWI].offset135.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)134.i"
  %CastToValueType136.i = bitcast i8* %"&pSB[currWI].offset135.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %140, <4 x i8> addrspace(3)** %CastToValueType136.i, align 8
  %141 = extractelement <16 x i32> %116, i32 8
  %142 = zext i32 %141 to i64
  %143 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %142
  %"&(pSB[currWI].offset)143.i" = add nuw i64 %CurrSBIndex..0.i, 256
  %"&pSB[currWI].offset144.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)143.i"
  %CastToValueType145.i = bitcast i8* %"&pSB[currWI].offset144.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %143, <4 x i8> addrspace(3)** %CastToValueType145.i, align 8
  %144 = extractelement <16 x i32> %116, i32 9
  %145 = zext i32 %144 to i64
  %146 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %145
  %"&(pSB[currWI].offset)152.i" = add nuw i64 %CurrSBIndex..0.i, 264
  %"&pSB[currWI].offset153.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)152.i"
  %CastToValueType154.i = bitcast i8* %"&pSB[currWI].offset153.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %146, <4 x i8> addrspace(3)** %CastToValueType154.i, align 8
  %147 = extractelement <16 x i32> %116, i32 10
  %148 = zext i32 %147 to i64
  %149 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %148
  %"&(pSB[currWI].offset)161.i" = add nuw i64 %CurrSBIndex..0.i, 272
  %"&pSB[currWI].offset162.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)161.i"
  %CastToValueType163.i = bitcast i8* %"&pSB[currWI].offset162.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %149, <4 x i8> addrspace(3)** %CastToValueType163.i, align 8
  %150 = extractelement <16 x i32> %116, i32 11
  %151 = zext i32 %150 to i64
  %152 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %151
  %"&(pSB[currWI].offset)170.i" = add nuw i64 %CurrSBIndex..0.i, 280
  %"&pSB[currWI].offset171.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)170.i"
  %CastToValueType172.i = bitcast i8* %"&pSB[currWI].offset171.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %152, <4 x i8> addrspace(3)** %CastToValueType172.i, align 8
  %153 = extractelement <16 x i32> %116, i32 12
  %154 = zext i32 %153 to i64
  %155 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %154
  %"&(pSB[currWI].offset)179.i" = add nuw i64 %CurrSBIndex..0.i, 288
  %"&pSB[currWI].offset180.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)179.i"
  %CastToValueType181.i = bitcast i8* %"&pSB[currWI].offset180.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %155, <4 x i8> addrspace(3)** %CastToValueType181.i, align 8
  %156 = extractelement <16 x i32> %116, i32 13
  %157 = zext i32 %156 to i64
  %158 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %157
  %"&(pSB[currWI].offset)188.i" = add nuw i64 %CurrSBIndex..0.i, 296
  %"&pSB[currWI].offset189.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)188.i"
  %CastToValueType190.i = bitcast i8* %"&pSB[currWI].offset189.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %158, <4 x i8> addrspace(3)** %CastToValueType190.i, align 8
  %159 = extractelement <16 x i32> %116, i32 14
  %160 = zext i32 %159 to i64
  %161 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %160
  %"&(pSB[currWI].offset)197.i" = add nuw i64 %CurrSBIndex..0.i, 304
  %"&pSB[currWI].offset198.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)197.i"
  %CastToValueType199.i = bitcast i8* %"&pSB[currWI].offset198.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %161, <4 x i8> addrspace(3)** %CastToValueType199.i, align 8
  %162 = extractelement <16 x i32> %116, i32 15
  %163 = zext i32 %162 to i64
  %164 = getelementptr inbounds <4 x i8> addrspace(3)* %7, i64 %163
  %"&(pSB[currWI].offset)206.i" = add nuw i64 %CurrSBIndex..0.i, 312
  %"&pSB[currWI].offset207.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)206.i"
  %CastToValueType208.i = bitcast i8* %"&pSB[currWI].offset207.i" to <4 x i8> addrspace(3)**
  store <4 x i8> addrspace(3)* %164, <4 x i8> addrspace(3)** %CastToValueType208.i, align 8
  store <4 x i8> %99, <4 x i8> addrspace(3)* %119, align 4
  store <4 x i8> %100, <4 x i8> addrspace(3)* %122, align 4
  store <4 x i8> %101, <4 x i8> addrspace(3)* %125, align 4
  store <4 x i8> %102, <4 x i8> addrspace(3)* %128, align 4
  store <4 x i8> %103, <4 x i8> addrspace(3)* %131, align 4
  store <4 x i8> %104, <4 x i8> addrspace(3)* %134, align 4
  store <4 x i8> %105, <4 x i8> addrspace(3)* %137, align 4
  store <4 x i8> %106, <4 x i8> addrspace(3)* %140, align 4
  store <4 x i8> %107, <4 x i8> addrspace(3)* %143, align 4
  store <4 x i8> %108, <4 x i8> addrspace(3)* %146, align 4
  store <4 x i8> %109, <4 x i8> addrspace(3)* %149, align 4
  store <4 x i8> %110, <4 x i8> addrspace(3)* %152, align 4
  store <4 x i8> %111, <4 x i8> addrspace(3)* %155, align 4
  store <4 x i8> %112, <4 x i8> addrspace(3)* %158, align 4
  store <4 x i8> %113, <4 x i8> addrspace(3)* %161, align 4
  store <4 x i8> %114, <4 x i8> addrspace(3)* %164, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %elseBB.i

thenBB.i:                                         ; preds = %SyncBB216.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 448
  br label %SyncBB216.i

elseBB.i:                                         ; preds = %SyncBB216.i
  %temp36.i = insertelement <16 x i32> undef, i32 %13, i32 0
  %vector37.i = shufflevector <16 x i32> %temp36.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB219.i, %elseBB.i
  %CurrSBIndex..1.i = phi i64 [ 0, %elseBB.i ], [ %"loadedCurrSB+Stride225.i", %thenBB219.i ]
  %CurrWI..1.i = phi i64 [ 0, %elseBB.i ], [ %"CurrWI++223.i", %thenBB219.i ]
  %"&(pSB[currWI].offset)58.i" = add nuw i64 %CurrSBIndex..1.i, 64
  %"&pSB[currWI].offset59.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)58.i"
  %CastToValueType60.i = bitcast i8* %"&pSB[currWI].offset59.i" to <16 x i32>*
  %loadedValue.i = load <16 x i32>* %CastToValueType60.i, align 64
  %165 = mul <16 x i32> %loadedValue.i, %vector37.i
  %"&(pSB[currWI].offset)66.i" = add nuw i64 %CurrSBIndex..1.i, 128
  %"&pSB[currWI].offset67.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)66.i"
  %CastToValueType68.i = bitcast i8* %"&pSB[currWI].offset67.i" to <16 x i32>*
  %loadedValue69.i = load <16 x i32>* %CastToValueType68.i, align 64
  %166 = add <16 x i32> %loadedValue69.i, %165
  %"&(pSB[currWI].offset)75.i" = add nuw i64 %CurrSBIndex..1.i, 192
  %"&pSB[currWI].offset76.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)75.i"
  %CastToValueType77.i = bitcast i8* %"&pSB[currWI].offset76.i" to <4 x i8> addrspace(3)**
  %loadedValue78.i = load <4 x i8> addrspace(3)** %CastToValueType77.i, align 8
  %167 = load <4 x i8> addrspace(3)* %loadedValue78.i, align 4
  %"&(pSB[currWI].offset)84.i" = add nuw i64 %CurrSBIndex..1.i, 200
  %"&pSB[currWI].offset85.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)84.i"
  %CastToValueType86.i = bitcast i8* %"&pSB[currWI].offset85.i" to <4 x i8> addrspace(3)**
  %loadedValue87.i = load <4 x i8> addrspace(3)** %CastToValueType86.i, align 8
  %168 = load <4 x i8> addrspace(3)* %loadedValue87.i, align 4
  %"&(pSB[currWI].offset)93.i" = add nuw i64 %CurrSBIndex..1.i, 208
  %"&pSB[currWI].offset94.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)93.i"
  %CastToValueType95.i = bitcast i8* %"&pSB[currWI].offset94.i" to <4 x i8> addrspace(3)**
  %loadedValue96.i = load <4 x i8> addrspace(3)** %CastToValueType95.i, align 8
  %169 = load <4 x i8> addrspace(3)* %loadedValue96.i, align 4
  %"&(pSB[currWI].offset)102.i" = add nuw i64 %CurrSBIndex..1.i, 216
  %"&pSB[currWI].offset103.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)102.i"
  %CastToValueType104.i = bitcast i8* %"&pSB[currWI].offset103.i" to <4 x i8> addrspace(3)**
  %loadedValue105.i = load <4 x i8> addrspace(3)** %CastToValueType104.i, align 8
  %170 = load <4 x i8> addrspace(3)* %loadedValue105.i, align 4
  %"&(pSB[currWI].offset)111.i" = add nuw i64 %CurrSBIndex..1.i, 224
  %"&pSB[currWI].offset112.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)111.i"
  %CastToValueType113.i = bitcast i8* %"&pSB[currWI].offset112.i" to <4 x i8> addrspace(3)**
  %loadedValue114.i = load <4 x i8> addrspace(3)** %CastToValueType113.i, align 8
  %171 = load <4 x i8> addrspace(3)* %loadedValue114.i, align 4
  %"&(pSB[currWI].offset)120.i" = add nuw i64 %CurrSBIndex..1.i, 232
  %"&pSB[currWI].offset121.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)120.i"
  %CastToValueType122.i = bitcast i8* %"&pSB[currWI].offset121.i" to <4 x i8> addrspace(3)**
  %loadedValue123.i = load <4 x i8> addrspace(3)** %CastToValueType122.i, align 8
  %172 = load <4 x i8> addrspace(3)* %loadedValue123.i, align 4
  %"&(pSB[currWI].offset)129.i" = add nuw i64 %CurrSBIndex..1.i, 240
  %"&pSB[currWI].offset130.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)129.i"
  %CastToValueType131.i = bitcast i8* %"&pSB[currWI].offset130.i" to <4 x i8> addrspace(3)**
  %loadedValue132.i = load <4 x i8> addrspace(3)** %CastToValueType131.i, align 8
  %173 = load <4 x i8> addrspace(3)* %loadedValue132.i, align 4
  %"&(pSB[currWI].offset)138.i" = add nuw i64 %CurrSBIndex..1.i, 248
  %"&pSB[currWI].offset139.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)138.i"
  %CastToValueType140.i = bitcast i8* %"&pSB[currWI].offset139.i" to <4 x i8> addrspace(3)**
  %loadedValue141.i = load <4 x i8> addrspace(3)** %CastToValueType140.i, align 8
  %174 = load <4 x i8> addrspace(3)* %loadedValue141.i, align 4
  %"&(pSB[currWI].offset)147.i" = add nuw i64 %CurrSBIndex..1.i, 256
  %"&pSB[currWI].offset148.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)147.i"
  %CastToValueType149.i = bitcast i8* %"&pSB[currWI].offset148.i" to <4 x i8> addrspace(3)**
  %loadedValue150.i = load <4 x i8> addrspace(3)** %CastToValueType149.i, align 8
  %175 = load <4 x i8> addrspace(3)* %loadedValue150.i, align 4
  %"&(pSB[currWI].offset)156.i" = add nuw i64 %CurrSBIndex..1.i, 264
  %"&pSB[currWI].offset157.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)156.i"
  %CastToValueType158.i = bitcast i8* %"&pSB[currWI].offset157.i" to <4 x i8> addrspace(3)**
  %loadedValue159.i = load <4 x i8> addrspace(3)** %CastToValueType158.i, align 8
  %176 = load <4 x i8> addrspace(3)* %loadedValue159.i, align 4
  %"&(pSB[currWI].offset)165.i" = add nuw i64 %CurrSBIndex..1.i, 272
  %"&pSB[currWI].offset166.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)165.i"
  %CastToValueType167.i = bitcast i8* %"&pSB[currWI].offset166.i" to <4 x i8> addrspace(3)**
  %loadedValue168.i = load <4 x i8> addrspace(3)** %CastToValueType167.i, align 8
  %177 = load <4 x i8> addrspace(3)* %loadedValue168.i, align 4
  %"&(pSB[currWI].offset)174.i" = add nuw i64 %CurrSBIndex..1.i, 280
  %"&pSB[currWI].offset175.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)174.i"
  %CastToValueType176.i = bitcast i8* %"&pSB[currWI].offset175.i" to <4 x i8> addrspace(3)**
  %loadedValue177.i = load <4 x i8> addrspace(3)** %CastToValueType176.i, align 8
  %178 = load <4 x i8> addrspace(3)* %loadedValue177.i, align 4
  %"&(pSB[currWI].offset)183.i" = add nuw i64 %CurrSBIndex..1.i, 288
  %"&pSB[currWI].offset184.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)183.i"
  %CastToValueType185.i = bitcast i8* %"&pSB[currWI].offset184.i" to <4 x i8> addrspace(3)**
  %loadedValue186.i = load <4 x i8> addrspace(3)** %CastToValueType185.i, align 8
  %179 = load <4 x i8> addrspace(3)* %loadedValue186.i, align 4
  %"&(pSB[currWI].offset)192.i" = add nuw i64 %CurrSBIndex..1.i, 296
  %"&pSB[currWI].offset193.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)192.i"
  %CastToValueType194.i = bitcast i8* %"&pSB[currWI].offset193.i" to <4 x i8> addrspace(3)**
  %loadedValue195.i = load <4 x i8> addrspace(3)** %CastToValueType194.i, align 8
  %180 = load <4 x i8> addrspace(3)* %loadedValue195.i, align 4
  %"&(pSB[currWI].offset)201.i" = add nuw i64 %CurrSBIndex..1.i, 304
  %"&pSB[currWI].offset202.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)201.i"
  %CastToValueType203.i = bitcast i8* %"&pSB[currWI].offset202.i" to <4 x i8> addrspace(3)**
  %loadedValue204.i = load <4 x i8> addrspace(3)** %CastToValueType203.i, align 8
  %181 = load <4 x i8> addrspace(3)* %loadedValue204.i, align 4
  %"&(pSB[currWI].offset)210.i" = add nuw i64 %CurrSBIndex..1.i, 312
  %"&pSB[currWI].offset211.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)210.i"
  %CastToValueType212.i = bitcast i8* %"&pSB[currWI].offset211.i" to <4 x i8> addrspace(3)**
  %loadedValue213.i = load <4 x i8> addrspace(3)** %CastToValueType212.i, align 8
  %182 = load <4 x i8> addrspace(3)* %loadedValue213.i, align 4
  %183 = extractelement <16 x i32> %166, i32 0
  %184 = zext i32 %183 to i64
  %185 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %184
  %186 = extractelement <16 x i32> %166, i32 1
  %187 = zext i32 %186 to i64
  %188 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %187
  %189 = extractelement <16 x i32> %166, i32 2
  %190 = zext i32 %189 to i64
  %191 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %190
  %192 = extractelement <16 x i32> %166, i32 3
  %193 = zext i32 %192 to i64
  %194 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %193
  %195 = extractelement <16 x i32> %166, i32 4
  %196 = zext i32 %195 to i64
  %197 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %196
  %198 = extractelement <16 x i32> %166, i32 5
  %199 = zext i32 %198 to i64
  %200 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %199
  %201 = extractelement <16 x i32> %166, i32 6
  %202 = zext i32 %201 to i64
  %203 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %202
  %204 = extractelement <16 x i32> %166, i32 7
  %205 = zext i32 %204 to i64
  %206 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %205
  %207 = extractelement <16 x i32> %166, i32 8
  %208 = zext i32 %207 to i64
  %209 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %208
  %210 = extractelement <16 x i32> %166, i32 9
  %211 = zext i32 %210 to i64
  %212 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %211
  %213 = extractelement <16 x i32> %166, i32 10
  %214 = zext i32 %213 to i64
  %215 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %214
  %216 = extractelement <16 x i32> %166, i32 11
  %217 = zext i32 %216 to i64
  %218 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %217
  %219 = extractelement <16 x i32> %166, i32 12
  %220 = zext i32 %219 to i64
  %221 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %220
  %222 = extractelement <16 x i32> %166, i32 13
  %223 = zext i32 %222 to i64
  %224 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %223
  %225 = extractelement <16 x i32> %166, i32 14
  %226 = zext i32 %225 to i64
  %227 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %226
  %228 = extractelement <16 x i32> %166, i32 15
  %229 = zext i32 %228 to i64
  %230 = getelementptr inbounds <4 x i8> addrspace(1)* %1, i64 %229
  store <4 x i8> %167, <4 x i8> addrspace(1)* %185, align 4
  store <4 x i8> %168, <4 x i8> addrspace(1)* %188, align 4
  store <4 x i8> %169, <4 x i8> addrspace(1)* %191, align 4
  store <4 x i8> %170, <4 x i8> addrspace(1)* %194, align 4
  store <4 x i8> %171, <4 x i8> addrspace(1)* %197, align 4
  store <4 x i8> %172, <4 x i8> addrspace(1)* %200, align 4
  store <4 x i8> %173, <4 x i8> addrspace(1)* %203, align 4
  store <4 x i8> %174, <4 x i8> addrspace(1)* %206, align 4
  store <4 x i8> %175, <4 x i8> addrspace(1)* %209, align 4
  store <4 x i8> %176, <4 x i8> addrspace(1)* %212, align 4
  store <4 x i8> %177, <4 x i8> addrspace(1)* %215, align 4
  store <4 x i8> %178, <4 x i8> addrspace(1)* %218, align 4
  store <4 x i8> %179, <4 x i8> addrspace(1)* %221, align 4
  store <4 x i8> %180, <4 x i8> addrspace(1)* %224, align 4
  store <4 x i8> %181, <4 x i8> addrspace(1)* %227, align 4
  store <4 x i8> %182, <4 x i8> addrspace(1)* %230, align 4
  %check.WI.iter222.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter222.i, label %thenBB219.i, label %____Vectorized_.transpose_kernel_separated_args.exit

thenBB219.i:                                      ; preds = %SyncBB.i
  %"CurrWI++223.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride225.i" = add nuw i64 %CurrSBIndex..1.i, 448
  br label %SyncBB.i

____Vectorized_.transpose_kernel_separated_args.exit: ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, <4 x i8> addrspace(3)*, i32, i32, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__transpose_kernel_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uchar4 __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(3))) *, uint const, uint const, uint const", metadata !"opencl_transpose_kernel_locals_anchor", void (i8*)* @transpose_kernel}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (<4 x i8> addrspace(1)*, <4 x i8> addrspace(1)*, i32, i32, float, float, float, float, float, float, float, float, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__RecursiveGaussian_kernel_separated_args, metadata !1, metadata !1, metadata !"", metadata !"uchar4 const __attribute__((address_space(1))) *, uchar4 __attribute__((address_space(1))) *, int const, int const, float const, float const, float const, float const, float const, float const, float const, float const", metadata !"opencl_RecursiveGaussian_kernel_locals_anchor", void (i8*)* @RecursiveGaussian_kernel}
